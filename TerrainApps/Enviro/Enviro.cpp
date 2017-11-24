//
// class Enviro: Main functionality of the Enviro application
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/CarEngine.h"
#include "vtlib/core/Fence3d.h"
#include "vtlib/core/GeomFactory.h"
#include "vtlib/core/Globe.h"
#include "vtlib/core/SkyDome.h"
#include "vtlib/core/Building3d.h"
#include "vtlib/core/PagedLodGrid.h"
#include "vtlib/core/PickEngines.h"
#include "vtlib/core/TiledGeom.h"
#include "vtlib/core/MapOverviewEngine.h"
#include "vtdata/vtLog.h"
#include "vtdata/PolyChecker.h"
#include "vtdata/DataPath.h"

#include "Engines.h"
#include "Enviro.h"
#include "Options.h"
#include "Hawaii.h"

// Although there is no string translation in the core of Enviro (because it is
//  independent of wx or any GUI library) nonetheless we want the text messages
//  to be found by the gettext utility, so we need to enclose anything to be
//  translated in _()
#define _(x) x

#define ORTHO_HITHER	50	// 50m above highest point on terrain


///////////////////////////////////////////////////////////

Enviro::Enviro() : vtTerrainScene()
{
	m_mode = MM_NONE;
	m_state = AS_Initializing;
	m_iInitStep = 0;

	m_FenceParams.Defaults();

	m_bOnTerrain = false;
	m_bEarthShade = false;

	m_pGlobeContainer = NULL;
	m_bGlobeFlat = false;
	m_fFlattening = 1.0f;
	m_fFlattenDir = 0.0f;
	m_bGlobeUnfolded = false;
	m_fFolding = 0.0f;
	m_fFoldDir = 0.0f;
	m_pIcoGlobe = NULL;
	m_pOverlayGlobe = NULL;
	m_pSpaceAxes = NULL;
	m_pEarthLines = NULL;

	m_bTopDown = false;
	m_pTerrainPicker = NULL;
	m_pGlobePicker = NULL;
	m_pCursorMGeom = NULL;

	m_pArc = NULL;
	m_pArcMats = NULL;
	m_fArcLength = 0.0;
	m_fDistToolHeight = 5.0f;
	m_bMeasurePath = false;

	m_fMessageTime = 0.0f;
	m_pHUD = NULL;
	m_pHUDMessage = NULL;

	// plants
	m_pSpeciesList = NULL;
	m_bPlantsLoaded = false;
	m_PlantOpt.m_iMode = 0;
	m_PlantOpt.m_iSpecies = -1;
	m_PlantOpt.m_fHeight = 100.0f;
	m_PlantOpt.m_iVariance = 20;
	m_PlantOpt.m_fSpacing = 2.0f;

	m_bDragging = false;
	m_bDragUpDown = false;
	m_bSelectedStruct = false;
	m_bSelectedPlant = false;
	m_bSelectedVehicle = false;
	m_pControlEng = NULL;

	// HUD
	m_pLegendGeom = NULL;
	m_bCreatedLegend = false;

	m_pCompassSizer = NULL;
	m_pCompassGeom = NULL;
	m_bCreatedCompass = false;
	m_bDragCompass = false;

	m_pWindowBoxMesh = NULL;

	m_pMapOverview = NULL;
	m_bFlyIn = false;
	m_iFlightStage = 0;

	m_Elastic.SetPostHeight(4.0f);
	m_Elastic.SetLineHeight(2.0f);

	// There are more buildings with all right angles than those without..
	m_bConstrainAngles = true;
	m_bShowVerticalLine = false;
}

Enviro::~Enviro()
{
}

void Enviro::Startup()
{
	VTSTARTLOG("debug.txt");
	VTLOG1("\nEnviro\nBuild:");
#if VTDEBUG
	VTLOG1(" Debug");
#else
	VTLOG1(" Release");
#endif
#if UNICODE
	VTLOG1(" Unicode");
#endif
	VTLOG("\n\n");

	// set up the datum list we will use
	SetupEPSGDatums();
}

void Enviro::Shutdown()
{
	VTLOG1("Shutdown.\n");

	delete m_pSpeciesList;
	m_pSpeciesList = NULL;

	m_pTopDownCamera = NULL;

	// Clean up the rest of the TerrainScene container
	vtGetScene()->SetRoot(NULL);
	CleanupScene();

	delete m_pIcoGlobe;
	m_pIcoGlobe = NULL;

	delete m_pOverlayGlobe;
	m_pOverlayGlobe = NULL;
}

void Enviro::StartupArgument(int i, const char *str)
{
	VTLOG("Command line %d: %s\n", i, str);

	if (!strcmp(str, "-fullscreen"))
		g_Options.m_bFullscreen = true;

	else if(!strncmp(str, "-terrain=", 9))
		g_Options.m_strInitTerrain = str+9;

	else if(!strncmp(str, "-elev=", 6))
		g_Options.m_strUseElevation = str+6;

	else if(!strncmp(str, "-location=", 10))
	{
		g_Options.m_strInitLocation = str+10;
		// trim quotes
		g_Options.m_strInitLocation.Remove('\"');
	}

	else if(!strncmp(str, "-neutral", 8))
		g_Options.m_bStartInNeutral = true;
}

void Enviro::LoadAllTerrainDescriptions()
{
	VTLOG("LoadAllTerrainDescriptions...\n");

	for (uint i = 0; i < vtGetDataPath().size(); i++)
		LoadTerrainDescriptions(vtGetDataPath()[i]);

	VTLOG(" Done.\n");
}

void Enviro::LoadTerrainDescriptions(const vtString &path)
{
	int count = 0;
	VTLOG("  On path '%s':\n", (const char *) path);

	const vtString directory = path + "Terrains";
	for (dir_iter it((const char *)directory); it != dir_iter(); ++it)
	{
		//VTLOG("\t file: %s\n", it.filename().c_str());
		if (it.is_hidden() || it.is_directory())
			continue;

		std::string name1 = it.filename();
		vtString name = name1.c_str();

		// Only look for ".xml" files which describe a terrain
		vtString ext = GetExtension(name, false);
		if (ext.CompareNoCase(".xml") != 0)
			continue;

		// Some terrain .xml files want to use a different Terrain class
		int dot = name.Find('.');
		vtString before_dot;
		if (dot == -1)
			before_dot = name;
		else
			before_dot = name.Left(dot);

		// This is where you can tell Enviro to contruct your own terrain
		//  class, for a particular config file, rather than the default
		//  vtTerrain.
		vtTerrain *pTerr;
		if (before_dot == "Hawai`i" || before_dot == "Hawai'i" ||
			before_dot == "Hawaii" || before_dot == "Honoka'a" ||
			before_dot == "Kealakekua" || before_dot == "Kamuela")
			pTerr = new IslandTerrain;
		else
			pTerr = new vtTerrain;

		pTerr->SetTextureCompression(g_Options.m_bTextureCompression);
		pTerr->SetParamFile(directory + "/" + name);
		if (pTerr->LoadParams())
		{
			//vtString sn = pTerr->GetParams().GetValueString(STR_NAME);
			//VTLOG("Terrain name: '%s'\n", (const char *) sn);
			AppendTerrain(pTerr);
		}
		else
			VTLOG1("\t Couldn't read.\n");
		count++;
	}
	VTLOG("\t%d terrains.\n", count);
}

void Enviro::LoadGlobalContent()
{
	// Load the global content file, if there is one
	VTLOG("Looking for global content file '%s'\n", (const char *)g_Options.m_strContentFile);
	vtString fname = FindFileOnPaths(vtGetDataPath(), g_Options.m_strContentFile);
	if (fname != "")
	{
		bool success = true;
		vtContentManager3d &con = vtGetContent();
		VTLOG1("  Loading content file.\n");
		try {
			con.ReadXML(fname);
		}
		catch (xh_io_exception &e)
		{
			success = false;
			string str = e.getFormattedMessage();
			VTLOG("  Error: %s\n", str.c_str());
		}
		if (success)
			VTLOG("   Load successful, %d items\n", con.NumItems());
		else
			VTLOG1("   Load not successful.\n");
	}
	else
		VTLOG1("  Couldn't find it.\n");
}

void Enviro::StartControlEngine()
{
	VTLOG1("StartControlEngine\n");

	m_pControlEng = new ControlEngine;
	m_pControlEng->setName("Control Engine");
	m_pControlEng->m_pEnvironment = this;
	vtGetScene()->AddEngine(m_pControlEng);
}

void Enviro::DoControl()
{
	if (m_fMessageTime != 0.0f)
	{
		if ((vtGetTime() - m_fMessageStart) > m_fMessageTime)
		{
			SetMessage("");
			m_fMessageTime = 0.0f;
		}
	}
	if (m_state == AS_Initializing)
	{
		m_iInitStep++;

		VTLOG("AS_Initializing initstep=%d\n", m_iInitStep);

		if (m_iInitStep == 1)
		{
			SetupScene1();
			return;
		}
		if (m_iInitStep == 2)
		{
			SetupScene2();
			return;
		}
		if (m_iInitStep == 3)
		{
			SetupScene3();
			return;
		}
		if (g_Options.m_bStartInNeutral)
		{
			SetState(AS_Neutral);
		}
		else if (g_Options.m_bEarthView)
		{
			FlyToSpace();
			return;
		}
		else if (g_Options.m_strUseElevation != "")
		{
			// Make a default terrain with a specific elevation grid or TIN
			vtTerrain *pTerr = new vtTerrain;
			pTerr->GetParams().SetValueString(STR_ELEVFILE, g_Options.m_strUseElevation);
			pTerr->GetParams().SetValueInt(STR_TEXTURE, 3);		//  3=derived
			pTerr->GetParams().SetValueString(STR_COLOR_MAP, "default_absolute.cmt");
			pTerr->GetParams().SetValueString(STR_INITTIME, "104 2 21 9 0 0");

			const vtString &s = g_Options.m_strUseElevation;
			if (s.Find(".itf") != -1 || s.Find(".ITF") != -1)
				pTerr->GetParams().SetValueInt(STR_SURFACE_TYPE, 1);	// 1 = tin
			else
				pTerr->GetParams().SetValueInt(STR_SURFACE_TYPE, 0);	// 0 = grid
			RequestTerrain(pTerr);
			return;
		}
		else
		{
			if (!RequestTerrain(g_Options.m_strInitTerrain))
			{
				SetMessage(_("Terrain not found"));
				SetState(AS_Error);
			}
			return;
		}
	}
	if (m_state == AS_FlyingIn)
	{
		if (m_iFlightStage == 1)
		{
			FlyInStage1();
			return;
		}
		if (m_iFlightStage == 2)
		{
			FlyInStage2();
			return;
		}
	}
	if (m_state == AS_SwitchToTerrain)
	{
		m_iInitStep++;
		SetupTerrain(m_pTargetTerrain);
		if (m_bFlyIn && m_iInitStep >= 10)
		{
			// Finished constructing, can smoothly fly in now
			ShowProgress(false);
			StartFlyIn();
		}
	}
	if (m_state == AS_MovingOut)
	{
		m_iInitStep++;
		SetupGlobe();
	}
	if (m_state == AS_Orbit)
		DoControlOrbit();
	if (m_state == AS_Terrain)
		DoControlTerrain();
}

void Enviro::DoControlTerrain()
{
	vtTerrain *terr = GetCurrentTerrain();
	if (!terr)
		return;

	// Update structure paging and shadows
	vtLodGrid *plg = terr->GetStructureGrid();
	vtPagedStructureLodGrid *pslg = terr->GetStructureLodGrid();
	if (pslg)
	{
		int remaining = terr->DoStructurePaging();
		if (remaining != 0)
		{
			vtString msg;
			msg.Format("Structure queue: %d\n", remaining);
			SetHUDMessageText(msg);
		}
		else
			SetHUDMessageText("");
	}
}

bool Enviro::RequestTerrain(const char *name)
{
	VTLOG("RequestTerrain (%s)\n", name);
	vtTerrain *pTerr = FindTerrainByName(name);
	if (!pTerr)
		return false;

	if (!IsAcceptable(pTerr))
		return false;

	if (pTerr)
	{
		RequestTerrain(pTerr);
		return true;
	}
	else
		return false;
}

void Enviro::RequestTerrain(vtTerrain *pTerr)
{
	VTLOG("RequestTerrain %lx\n", pTerr);

	FreeArc();

	if (m_state == AS_Orbit)
	{
		// remember camera position
		m_pTrackball->GetState(m_SpaceTrackballState);
	}
	if (m_state == AS_Terrain)
	{
		// remember camera position
		vtTerrain *pT = GetCurrentTerrain();
		vtCamera *pCam = vtGetScene()->GetCamera();
		FMatrix4 mat;
		pCam->GetTransform(mat);
		pT->SetCamLocation(mat);
	}

	// If it's not a tileset, and we're coming in from space, fly in
	if (m_state == AS_Orbit && g_Options.m_bFlyIn)
		m_bFlyIn = true;

	SetState(AS_SwitchToTerrain);
	m_pTargetTerrain = pTerr;
	m_iInitStep = 0;

	if (g_Options.m_bShowProgress)
	{
		ShowProgress(true);
		SetProgressTerrain(pTerr);
	}
}

void Enviro::SetupTerrain(vtTerrain *pTerr)
{
	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	VTLOG("SetupTerrain step %d\n", m_iInitStep);
	if (m_iInitStep == 1)
	{
		vtString name_quotes = vtString("'") + pTerr->GetName() + "'";
		SetMessage(_("Setting up Terrain "), name_quotes);
		UpdateProgress(m_strMessage1, m_strMessage2, m_iInitStep * 100 / 16, 0);
	}
	else if (m_iInitStep == 2)
	{
		if (pTerr->IsCreated())
		{
			m_iInitStep = 13;	// already made, skip ahead
			return;
		}
		else
			SetMessage(_("Loading Elevation"));
		UpdateProgress(m_strMessage1, m_strMessage2, m_iInitStep * 100 / 16, 0);
	}
	else if (m_iInitStep == 3)
	{
		OnCreateTerrain(pTerr);

		const int veg_layers = pTerr->GetParams().NumLayersOfType(TERR_LTYPE_VEGETATION);
		VTLOG("There are %d vegetation layers\n", veg_layers);
		if (veg_layers > 0)
		{
			// We'll need vegetation for this terrain, so load the species
			//  file and check which appearances are available
			LoadSpeciesList();
		}

		pTerr->SetSpeciesList(m_pSpeciesList);
		pTerr->CreateStep1();

		// connect the terrain's engines
		m_pTerrainEngines->AddChild(pTerr->GetEngineGroup());

		if (!pTerr->CreateStep2())
		{
			SetState(AS_Error);
			SetMessage("", pTerr->GetLastError());	// Don't try to translate error
			return;
		}
		SetMessage(_("Loading/Coloring/Prelighting Textures"));
		UpdateProgress(m_strMessage1, m_strMessage2, m_iInitStep * 100 / 16, 0);
	}
	else if (m_iInitStep == 4)
	{
		if (m_pSkyDome)
		{
			// Tell the skydome where on the planet we are
			VTLOG1("Placing skydome.\n");
			DPoint2 geo = pTerr->GetCenterGeoLocation();
			m_pSkyDome->SetGeoLocation(geo);

			// Set time to that of the new terrain
			m_pSkyDome->SetTime(pTerr->GetInitialTime());
		}

		if (!pTerr->CreateStep3(GetSunLightTransform(), GetSunLightSource()))
		{
			SetState(AS_Error);
			SetMessage("", pTerr->GetLastError());
			return;
		}
		SetMessage(_("Processing Elevation"));
		UpdateProgress(m_strMessage1, m_strMessage2, m_iInitStep * 100 / 16, 0);
	}
	else if (m_iInitStep == 5)
	{
		if (!pTerr->CreateStep4())
		{
			SetState(AS_Error);
			SetMessage("", pTerr->GetLastError());
			return;
		}
		SetMessage(_("Building CLOD"));
		UpdateProgress(m_strMessage1, m_strMessage2, 25, 0);
	}
	else if (m_iInitStep == 6)
	{
		if (!pTerr->CreateStep5())
		{
			SetState(AS_Error);
			SetMessage("", pTerr->GetLastError());
			return;
		}
		SetMessage(_("Creating Structures"));
		UpdateProgress(m_strMessage1, m_strMessage2, m_iInitStep * 100 / 16, 0);
	}
	else if (m_iInitStep == 7)
	{
		pTerr->CreateStep6();

		SetMessage(_("Creating Roads"));
		UpdateProgress(m_strMessage1, m_strMessage2, m_iInitStep * 100 / 16, 0);
	}
	else if (m_iInitStep == 8)
	{
		pTerr->CreateStep7();

		SetMessage(_("Creating Vegetation"));
		UpdateProgress(m_strMessage1, m_strMessage2, m_iInitStep * 100 / 16, 0);
	}
	else if (m_iInitStep == 9)
	{
		pTerr->CreateStep8();

		SetMessage(_("Creating Water, UtilityMaps, HUD"));
		UpdateProgress(m_strMessage1, m_strMessage2, m_iInitStep * 100 / 16, 0);
	}
	else if (m_iInitStep == 10)
	{
		pTerr->CreateStep9();

		SetMessage(_("Creating Abstract Layers"));
		UpdateProgress(m_strMessage1, m_strMessage2, m_iInitStep * 100 / 16, 0);
	}
	else if (m_iInitStep == 11)
	{
		pTerr->CreateStep10();

		SetMessage(_("Creating Image Layers"));
		UpdateProgress(m_strMessage1, m_strMessage2, m_iInitStep * 100 / 16, 0);
	}
	else if (m_iInitStep == 12)
	{
		pTerr->CreateStep11();

		SetMessage(_("Creating Elevation Layers"));
		UpdateProgress(m_strMessage1, m_strMessage2, m_iInitStep * 100 / 16, 0);
	}
	else if (m_iInitStep == 13)
	{
		pTerr->CreateStep12();

		SetMessage(_("Setting Camera"));
		UpdateProgress(m_strMessage1, m_strMessage2, m_iInitStep * 100 / 16, 0);
	}
	else if (m_iInitStep == 14)
	{
		// If we were in Earth View, hide the globe and disable the trackball
		if (m_pGlobeContainer != NULL)
		{
			m_pGlobeContainer->SetEnabled(false);
			m_pGlobePicker->SetEnabled(false);
		}
		if (m_pTrackball)
			m_pTrackball->SetEnabled(false);

		SetMessage(_("Switching to Terrain"));
		UpdateProgress(m_strMessage1, m_strMessage2, m_iInitStep * 100 / 16, 0);
	}
	else if (m_iInitStep == 15)
	{
		// make the terrain active
		SwitchToTerrain(pTerr);

		// Set hither and yon
		m_pNormalCamera->SetHither(pTerr->GetParams().GetValueFloat(STR_HITHER));
		m_pNormalCamera->SetYon(500000.0f);

		// ensure that sunlight is active
		GetSunLightTransform()->SetEnabled(true);

		m_pTerrainPicker->SetEnabled(true);
		SetMode(MM_NAVIGATE);
		UpdateProgress(m_strMessage1, m_strMessage2, m_iInitStep * 100 / 16, 0);
	}
	else if (m_iInitStep == 16)
	{
		SetMessage(_("Welcome to "), pTerr->GetName(), 5.0f);

		VTLOG(" seconds since app start: %.2f\n", (float)clock()/CLOCKS_PER_SEC);

		ShowProgress(false);

		SetState(AS_Terrain);

		// Layer view needs to update
		RefreshLayerView();

		m_Elastic.SetTerrain(pTerr);
	}
}

void Enviro::FormatCoordString(vtString &str, const DPoint3 &coord, LinearUnits units, bool seconds)
{
	DPoint3 pos = coord;
	if (units == LU_DEGREES)
	{
		int deg1 = (int) pos.x;
		pos.x -= (deg1);
		int min1 = (int) (pos.x * 60);
		pos.x -= (min1 / 60.0);
		int sec1 = (int) (pos.x * 3600);
		if (deg1 < 0) deg1 = -deg1;
		if (min1 < 0) min1 = -min1;
		if (sec1 < 0) sec1 = -sec1;
		char ew = coord.x > 0.0f ? 'E' : 'W';

		int deg2 = (int) pos.y;
		pos.y -= (deg2);
		int min2 = (int) (pos.y * 60);
		pos.y -= (min2 / 60.0);
		int sec2 = (int) (pos.y * 3600);
		if (deg2 < 0) deg2 = -deg2;
		if (min2 < 0) min2 = -min2;
		if (sec2 < 0) sec2 = -sec2;
		char ns = coord.y > 0.0f ? 'N' : 'S';

		if (seconds)
			str.Format("%3d:%02d:%02d %c, %3d:%02d:%02d %c", deg1, min1, sec1, ew, deg2, min2, sec2, ns);
		else
			str.Format("%3d:%02d %c, %3d:%02d %c", deg1, min1, ew, deg2, min2, ns);
	}
	else
	{
		str.Format("%7.1d, %7.1d", (int) coord.x, (int) coord.y);
	}
}


//
// Check the terrain under the 3D cursor (for Terrain View).
//
void Enviro::DoCursorOnTerrain()
{
	m_bOnTerrain = false;
	DPoint3 earthpos;

	if (m_pTerrainPicker != NULL)
		m_bOnTerrain = m_pTerrainPicker->GetCurrentEarthPos(earthpos);
	if (m_bOnTerrain)
	{
		m_EarthPos = earthpos;

		// Attempt to scale the 3d cursor, for ease of use.
		// Rather than keeping it the same size in world space (it would be too
		// small in the distance) or the same size in screen space (would look
		// confusing without the spatial distance cue) we compromise and scale
		// it based on the square root of distance.
		FPoint3 gpos;
		if (m_pTerrainPicker->GetCurrentPoint(gpos))
		{
			const FPoint3 campos = vtGetScene()->GetCamera()->GetTrans();
			const float distance = (gpos - campos).Length();
			const float sc = (float) sqrt(distance) / 1.0f;
			const FPoint3 pos = m_pCursorMGeom->GetTrans();
			m_pCursorMGeom->Identity();
			m_pCursorMGeom->Scale(sc);
			m_pCursorMGeom->SetTrans(pos);
		}

		// Inform GUI, in case it cares.
		EarthPosUpdated();

		if (m_bShowVerticalLine)
			UpdateVerticalLine();
	}
}


void Enviro::SetupScene1()
{
	VTLOG1("SetupScene1\n");

	// Set some global properties
	m_fCatenaryFactor = g_Options.m_fCatenaryFactor;
	g_bDisableMipmaps = g_Options.m_bDisableModelMipmaps;

	vtScene *pScene = vtGetScene();
	vtCamera *pCamera = pScene->GetCamera();
	if (pCamera) pCamera->setName("Standard Camera");

	m_pRoot = BeginTerrainScene();
	pScene->SetRoot(m_pRoot);
}

void Enviro::SetupScene2()
{
	VTLOG1("SetupScene2\n");

	m_pNavEngines = new vtEngine;
	m_pNavEngines->setName("Navigation Engines");
	vtGetScene()->GetRootEngine()->AddChild(m_pNavEngines);

	// Make navigation engines
	m_pOrthoFlyer = new vtOrthoFlyer(1.0f);
	m_pOrthoFlyer->setName("Orthographic View Flyer");
	m_pOrthoFlyer->SetEnabled(false);
	m_pNavEngines->AddChild(m_pOrthoFlyer);

	m_pQuakeFlyer = new QuakeFlyer(1.0f);
	m_pQuakeFlyer->setName("Quake-Style Flyer");
	m_pQuakeFlyer->SetEnabled(false);
	m_pNavEngines->AddChild(m_pQuakeFlyer);

	m_pVFlyer = new VFlyer(1.0f);
	m_pVFlyer->setName("Velocity-Gravity Flyer");
	m_pVFlyer->SetEnabled(false);
	m_pNavEngines->AddChild(m_pVFlyer);

	m_pTFlyer = new vtTerrainFlyer(1.0f);
	m_pTFlyer->setName("Terrain-following Flyer");
	m_pTFlyer->SetEnabled(false);
	m_pNavEngines->AddChild(m_pTFlyer);

	m_pGFlyer = new GrabFlyer(1.0f);
	m_pGFlyer->setName("Grab-Pivot Flyer");
	m_pGFlyer->SetEnabled(false);
	m_pNavEngines->AddChild(m_pGFlyer);

	m_pFlatFlyer = new FlatFlyer;
	m_pFlatFlyer->setName("Flat Flyer");
	m_pFlatFlyer->SetEnabled(false);
	m_pNavEngines->AddChild(m_pFlatFlyer);

	m_pPanoFlyer = new vtPanoFlyer(1.0f);
	m_pPanoFlyer->setName("Panoramic Flyer");
	m_pPanoFlyer->SetEnabled(false);
	m_pNavEngines->AddChild(m_pPanoFlyer);

	m_nav = NT_Normal;

	// create picker object and picker engine
	vtGeode *pCursor = Create3DCursor(1.0, g_Options.m_fCursorThickness);
	m_pCursorMGeom = new vtMovGeode(pCursor);
	m_pCursorMGeom->setName("Cursor");

	GetTop()->addChild(m_pCursorMGeom);
	m_pTerrainPicker = new TerrainPicker;
	m_pTerrainPicker->setName("TerrainPicker");
	vtGetScene()->AddEngine(m_pTerrainPicker);

	m_pTerrainPicker->AddTarget(m_pCursorMGeom);
	m_pTerrainPicker->SetEnabled(false); // turn off at startup

	// Connect to the GrabFlyer
	m_pGFlyer->SetTerrainPicker(m_pTerrainPicker);

	VTLOG("Setting up Cameras\n");
	m_pNormalCamera = vtGetScene()->GetCamera();

	// Create second camera (for Top-Down view)
	if (m_pTopDownCamera == NULL)
	{
		VTLOG("Creating Top-Down Camera\n");
		m_pTopDownCamera = new vtCamera;
		m_pTopDownCamera->SetOrtho(true);
		m_pTopDownCamera->setName("Top-Down Camera");
		m_pOrthoFlyer->AddTarget(m_pTopDownCamera);
	}

	m_pQuakeFlyer->AddTarget(m_pNormalCamera);
	m_pVFlyer->AddTarget(m_pNormalCamera);
	m_pTFlyer->AddTarget(m_pNormalCamera);
	m_pGFlyer->AddTarget(m_pNormalCamera);
	m_pFlatFlyer->AddTarget(m_pNormalCamera);
	m_pPanoFlyer->AddTarget(m_pNormalCamera);

	// An engine to keep the camera above the terrain, comes after the other
	//  engines which could move the camera.
	m_pHeightEngine = new vtHeightConstrain(1.0f);
	m_pHeightEngine->setName("Height Constrain Engine");
	m_pHeightEngine->AddTarget(m_pNormalCamera);
	vtGetScene()->GetRootEngine()->AddChild(m_pHeightEngine);

	// This HUD group will contain geometry such as the legend
	m_pHUD = new vtHUD;
	m_pHUD->setName("HUD");
	m_pRoot->addChild(m_pHUD);
	m_pHUDMaterials = new vtMaterialArray;

	// A font is need by the HUD message text, and the elevation legend.
	m_pArial = osgText::readFontFile("Arial.ttf");
}

//
// Load the species file and check which appearances are available
//
void Enviro::LoadSpeciesList()
{
	if (m_bPlantsLoaded)
		return;

	VTLOG1("LoadSpeciesList\n");

	// First look for species.xml with terrain name prepended, otherwise fall
	//  back on just "species.xml"
	vtString species_fname = "PlantData/" + g_Options.m_strInitTerrain + "-species.xml";
	vtString species_path = FindFileOnPaths(vtGetDataPath(), species_fname);
	if (species_path == "")
		species_path = FindFileOnPaths(vtGetDataPath(), "PlantData/species.xml");

	if (species_path == "")
	{
		VTLOG1(" not found.\n");
		return;
	}

	vtSpeciesList sp_list;
	vtString errmsg;
	if (sp_list.ReadXML(species_path, &errmsg))
	{
		VTLOG(" Using species file: '%s'\n", (const char *) species_path);
		m_pSpeciesList = new vtSpeciesList3d;
		*m_pSpeciesList = sp_list;

		// global options
		vtPlantAppearance3d::s_fPlantScale = g_Options.m_fPlantScale;
		vtPlantAppearance3d::s_bPlantShadows = g_Options.m_bShadows;

		// Don't load all the plant appearances now, just check which are available
		int available = m_pSpeciesList->CheckAvailability();
		VTLOG(" %d plant appearances available.\n", available);

		m_bPlantsLoaded = true;
	}
	else
	{
		VTLOG1("Error loading species: ");
		VTLOG1(errmsg);
		VTLOG1("\n");
	}
}

void Enviro::SetCurrentNavigator(vtTerrainFlyer *pE)
{
	if (m_pCurrentFlyer != NULL)
	{
		const char *name = m_pCurrentFlyer->getName();
		VTLOG("Disabling '%s'\n", name);

		m_pCurrentFlyer->SetEnabled(false);
	}
	m_pCurrentFlyer = pE;
	if (m_pCurrentFlyer == NULL)
		VTLOG1("No navigator now enabled.\n");
	else
	{
		const char *name = m_pCurrentFlyer->getName();
		VTLOG(" Enabling '%s'\n", name);

		m_pCurrentFlyer->SetEnabled(true);
	}
}

void Enviro::EnableFlyerEngine(bool bEnable)
{
	if (bEnable && !m_bTopDown)
	{
		// enable normal navigation
		if (m_nav == NT_Normal)
			SetCurrentNavigator(m_pTFlyer);
		if (m_nav == NT_Velo)
			SetCurrentNavigator(m_pVFlyer);
		if (m_nav == NT_Grab)
			SetCurrentNavigator(m_pGFlyer);
//		if (m_nav == NT_Quake)
//			SetCurrentNavigator(m_pQuakeFlyer);
		if (m_nav == NT_Pano || m_nav == NT_Dummy)
			SetCurrentNavigator(m_pPanoFlyer);
	}
	else
		SetCurrentNavigator(NULL);

	if (m_bTopDown)
	{
		VTLOG("Enable OrthoFlyer: %d\n", bEnable);
		m_pOrthoFlyer->SetEnabled(bEnable);
	}
}

void Enviro::SetNavType(NavType nav)
{
	if (m_mode == MM_NAVIGATE)
		EnableFlyerEngine(false);
	m_nav = nav;
	if (m_mode == MM_NAVIGATE)
		EnableFlyerEngine(true);
}

void Enviro::SetMaintain(bool bOn)
{
	m_pHeightEngine->SetMaintain(bOn);
	m_pHeightEngine->SetMaintainHeight(0);
}

bool Enviro::GetMaintain()
{
	if (m_pHeightEngine == NULL)
		return false;
	return m_pHeightEngine->GetMaintain();
}

void Enviro::ResetCamera()
{
	VTLOG1("ResetCamera\n");
	if (m_pCurrentTerrain)
		m_pNormalCamera->SetTransform(m_pCurrentTerrain->GetCamLocation());
}

void Enviro::SwitchToTerrain(vtTerrain *pTerrain)
{
	VTLOG("Enviro::SwitchToTerrain '%s'\n",
		pTerrain ? (const char *) pTerrain->GetName() : "none");

	if (m_pCurrentTerrain)
	{
		vtGroup *pOverlay = m_pCurrentTerrain->GetOverlay();
		if (pOverlay)
			m_pHUD->GetContainer()->removeChild(pOverlay);
	}

	// Inform the container that this new terrain is current
	SetCurrentTerrain(pTerrain);

	// safety check
	if (!pTerrain)
	{
		ShowMapOverview(false);
		return;
	}
	const vtHeightField3d *pHF = pTerrain->GetHeightField();
	if (!pHF)
		return;

	// Inform the UI that this new terrain is current
	const TParams &param = pTerrain->GetParams();
	SetNavType((enum NavType) param.GetValueInt(STR_NAVSTYLE));

	EnableFlyerEngine(true);

	// Inform the terrain's location saver of the camera
	pTerrain->GetLocSaver()->SetTransform(m_pNormalCamera);

	// inform the navigation engines of the new terrain
	float speed = param.GetValueFloat(STR_NAVSPEED);
	if (m_pCurrentFlyer != NULL)
	{
		m_pCurrentFlyer->AddTarget(m_pNormalCamera);
		m_pCurrentFlyer->SetEnabled(true);
		m_pCurrentFlyer->SetExag(param.GetValueBool(STR_ACCEL));
	}
	m_pTFlyer->SetSpeed(speed);
	m_pVFlyer->SetSpeed(speed);
	m_pPanoFlyer->SetSpeed(speed);
	m_pOrthoFlyer->SetSpeed(speed);

	m_pVFlyer->SetDamping(param.GetValueFloat(STR_NAVDAMPING));

	// TODO: a more elegant way of keeping all nav engines current
	m_pQuakeFlyer->SetHeightField(pHF);
	m_pVFlyer->SetHeightField(pHF);
	m_pTFlyer->SetHeightField(pHF);
	m_pGFlyer->SetHeightField(pHF);
	m_pPanoFlyer->SetHeightField(pHF);
	// also the height constraint engine
	m_pHeightEngine->SetHeightField(pHF);
	m_pHeightEngine->SetMinGroundOffset(param.GetValueFloat(STR_MINHEIGHT));

	bool bAllowRoll = param.GetValueBool(STR_ALLOW_ROLL);
	m_pTFlyer->SetDOF(vtFlyer::DOF_ROLL, bAllowRoll);

	// Set the top-down viewpoint to a point over the center of the new
	//  terrain, with near and far planes derived from the height extents.
	m_pTopDownCamera->Identity();
	FPoint3 middle;
	pHF->GetCenter(middle);		// Gets XZ center

	float fMin, fMax;
	pHF->GetHeightExtents(fMin, fMax);
	fMax *= param.GetValueFloat(STR_VERTICALEXAG);
	fMax += 1;	// beware flat terrain: safety buffer of 1 meter
	middle.y = fMax + ORTHO_HITHER;		// highest value + hither
	m_pTopDownCamera->SetTrans(middle);

	// point it straight down
	m_pTopDownCamera->RotateLocal(TRANS_XAxis, -PID2f);
	m_pTopDownCamera->SetHither(ORTHO_HITHER);
	m_pTopDownCamera->SetYon(fMax - fMin + ORTHO_HITHER + ORTHO_HITHER);

	// pick an arbitrary amount of detail to show initially:
	//  1/4 the terrain width, with proportional speed
	m_pTopDownCamera->SetWidth(middle.x / 2);
	m_pOrthoFlyer->SetSpeed(middle.x / 5);

	if (m_pTerrainPicker != NULL)
		m_pTerrainPicker->SetHeightField(pHF);

	m_fDistToolHeight = param.GetValueFloat(STR_DIST_TOOL_HEIGHT);

	vtGroup *pOverlay = pTerrain->GetOverlay();
	if (pOverlay)
		m_pHUD->GetContainer()->addChild(pOverlay);

	if (!IsFlyingInFromSpace())
		SelectInitialViewpoint(pTerrain);

	// Inform the GUI that the terrain has changed
	SetTerrainToGUI(pTerrain);

	// Inform the map overview
	if (m_pMapOverview)
		m_pMapOverview->SetTerrain(pTerrain);

	// If there is an initial scenario, show it
	vtString scenario_name;
	if (param.GetValueString(STR_INIT_SCENARIO, scenario_name))
	{
		for (uint snum = 0; snum < param.m_Scenarios.size(); snum++)
		{
			if (scenario_name == param.m_Scenarios[snum].GetValueString(STR_SCENARIO_NAME))
				pTerrain->ActivateScenario(snum);
		}
	}
}

void Enviro::SelectInitialViewpoint(vtTerrain *pTerrain)
{
	// If we've already been here, use the viewpoint we had last time
	if (pTerrain->IsVisited())
	{
		// Get stored location
		const FMatrix4 &mat = pTerrain->GetCamLocation();
		FPoint3 trans = mat.GetTrans();
		VTLOG("   Got stored viewpoint for terrain '%s' as position %.1f, %.1f, %.1f\n",
			(const char *) pTerrain->GetName(), trans.x, trans.y, trans.z);
		m_pNormalCamera->SetTransform(mat);
		return;
	}

	// First visit.
	bool bGotStoredViewpoint = false;
	VTLOG1("First visit to this terrain, looking up initial location.\n");
	if (g_Options.m_strInitLocation != "")
	{
		// may have been given on command line
		pTerrain->GetLocSaver()->RecallFrom(g_Options.m_strInitLocation);
		g_Options.m_strInitLocation = "";
		bGotStoredViewpoint = true;
	}
	else
	{
		// or, it may be a location in the terrain parameters
		vtString vname = pTerrain->GetParams().GetValueString(STR_INITLOCATION);
		if (pTerrain->GetLocSaver()->RecallFrom(vname))
			bGotStoredViewpoint = true;
	}
	if (!bGotStoredViewpoint)
	{
		// Initial default location for camera for this terrain: Try center
		//  of heightfield, just above the ground, looking north.
		const vtHeightField3d *pHF = pTerrain->GetHeightField();
		FPoint3 middle;
		FMatrix4 mat;

		VTLOG1(" Placing the camera at the center of the terrain:\n");
		pHF->GetCenter(middle);
		VTLOG(" Center: %f %f %f\n", middle.x, middle.y, middle.z);
		pHF->FindAltitudeAtPoint(middle, middle.y);
		VTLOG(" Altitude at that point: %f\n", middle.y);
		float minheight = pTerrain->GetParams().GetValueFloat(STR_MINHEIGHT);
		middle.y += minheight;
		VTLOG(" plus minimum height (%f) is %f\n", minheight, middle.y);
		mat.Identity();
		mat.SetTrans(middle);

		// Use it now
		m_pNormalCamera->SetTransform(mat);
	}
	// Remember it for later
	FMatrix4 mat;
	m_pNormalCamera->GetTransform(mat);
	pTerrain->SetCamLocation(mat);

	pTerrain->Visited(true);
}

//
// Copy as much state as possible from the active terrain to its parameters
//
void Enviro::StoreTerrainParameters()
{
	vtTimeEngine *te = GetTimeEngine();
	vtTerrain *terr = GetCurrentTerrain();
	TParams &par = terr->GetParams();
	vtCamera *cam = vtGetScene()->GetCamera();
	vtSkyDome *sky = GetSkyDome();

	par.SetValueFloat(STR_VERTICALEXAG, terr->GetVerticalExag());

	par.SetValueInt(STR_NAVSTYLE, GetNavType());
	par.SetValueFloat(STR_NAVSPEED, GetFlightSpeed());
	par.SetValueFloat(STR_NAVDAMPING, GetNavDamping());
	//par.SetValueString(STR_LOCFILE);
	//par.SetValueString(STR_INITLOCATION);
	par.SetValueFloat(STR_HITHER, cam->GetHither());

	if (par.GetValueInt(STR_SURFACE_TYPE) == 0)	// single grid
	{
		vtDynTerrainGeom *dtg = terr->GetDynTerrain();
		par.SetValueInt(STR_TRICOUNT, dtg->GetPolygonTarget());
	}
	else if (par.GetValueInt(STR_SURFACE_TYPE) == 2)	// tileset
	{
		vtTiledGeom *tg = terr->GetTiledGeom();
		par.SetValueInt(STR_VERTCOUNT, tg->GetVertexTarget());
	}

	par.SetValueBool(STR_TIMEON, te->GetSpeed() != 0.0f);
	par.SetValueString(STR_INITTIME, te->GetTime().GetAsString());
	par.SetValueFloat(STR_TIMESPEED, te->GetSpeed());

	par.SetValueBool(STR_FOG, terr->GetFog());
	//par.SetValueFloat(STR_FOGDISTANCE);	// already set dynamically
	par.SetValueBool(STR_SKY, sky->GetEnabled());
	// par.SetValueString(STR_SKYTEXTURE);

	par.SetValueBool(STR_OCEANPLANE, terr->GetFeatureVisible(TFT_OCEAN));
	//par.SetValueFloat(STR_OCEANPLANELEVEL);	// already set dynamically
	//par.SetValueBool(STR_DEPRESSOCEAN);
	//par.SetValueFloat(STR_DEPRESSOCEANLEVEL);
	const RGBi col = terr->GetBgColor();
	par.SetValueRGBi(STR_BGCOLOR, col);

	par.SetValueFloat(STR_STRUCTDIST, terr->GetLODDistance(TFT_STRUCTURES));
	par.SetValueFloat(STR_ROADDISTANCE, terr->GetLODDistance(TFT_ROADS));
	par.SetValueFloat(STR_VEGDISTANCE, terr->GetLODDistance(TFT_VEGETATION));

	par.SetValueBool(STR_OVERVIEW, GetShowMapOverview());
	par.SetValueBool(STR_COMPASS, GetShowCompass());

	// Layers: copy back from the terrain's set of layers, to the set of
	//  layers in the parameters.
	const LayerSet &set = terr->GetLayers();
	par.m_Layers.clear();
	for (uint i = 0; i < set.size(); i++)
		par.m_Layers.push_back(set[i]->Props());
}

/**
  Display a message in the UI. The surrounding flavor of Enviro might show this
  in the status bar of the frame window (for wxEnviro) or as a HUD text (for
  a pure OpenGL version of Enviro).

  \param str1 The first part of the message, which should be translated.
  \param str2 The second part of the message, which should not be translated.
	The two parts will be concatenated.
  \param fTime How long the message should appear, in seconds.
*/
void Enviro::SetMessage(const vtString &str1, const vtString &str2, float fTime)
{
	VTLOG(" SetMessage: '%s' '%s'\n", (const char *) str1, (const char *) str2);

	if ((str1 + str2) != "" && fTime != 0.0f)
	{
		m_fMessageStart = vtGetTime();
		m_fMessageTime = fTime;
	}
	m_strMessage1 = str1;
	m_strMessage2 = str2;
}

void Enviro::SetFlightSpeed(float speed)
{
	if (m_bTopDown && m_pOrthoFlyer != NULL)
		m_pOrthoFlyer->SetSpeed(speed);
	else if (m_pCurrentFlyer != NULL)
		m_pCurrentFlyer->SetSpeed(speed);
}

float Enviro::GetFlightSpeed()
{
	if (m_bTopDown && m_pOrthoFlyer != NULL)
		return m_pOrthoFlyer->GetSpeed();
	else if (m_pCurrentFlyer != NULL)
		return m_pCurrentFlyer->GetSpeed();
	else
	{
		// We may not be navigating, but determine the speed of the navigator
		// that would be active if we were.
		if (m_nav == NT_Normal)
			return m_pTFlyer->GetSpeed();
		if (m_nav == NT_Velo)
			return m_pVFlyer->GetSpeed();
		if (m_nav == NT_Grab)
			return m_pGFlyer->GetSpeed();
		if (m_nav == NT_Pano || m_nav == NT_Dummy)
			return m_pPanoFlyer->GetSpeed();
	}
	return 0.0f;
}

void Enviro::SetNavDamping(float factor)
{
	m_pVFlyer->SetDamping(factor);
}

float Enviro::GetNavDamping()
{
	return m_pVFlyer->GetDamping();
}

void Enviro::SetFlightAccel(bool bAccel)
{
	if (m_pCurrentFlyer)
		m_pCurrentFlyer->SetExag(bAccel);
}

bool Enviro::GetFlightAccel()
{
	if (m_pCurrentFlyer)
		return m_pCurrentFlyer->GetExag();
	else
		return false;
}

void Enviro::SetMode(MouseMode mode)
{
	VTLOG("SetMode %d\n", mode);

	if (m_pCursorMGeom)
	{
		switch (mode)
		{
		case MM_NAVIGATE:
			m_pCursorMGeom->SetEnabled(false);
			EnableFlyerEngine(true);
			break;
		case MM_SELECT:
		case MM_SELECTMOVE:
			m_pCursorMGeom->SetEnabled(!g_Options.m_bDirectPicking);
			EnableFlyerEngine(false);
			break;
		case MM_SELECTBOX:
			m_pCursorMGeom->SetEnabled(false);
			EnableFlyerEngine(false);
			break;
		case MM_LINEARS:
		case MM_BUILDINGS:
		case MM_POWER:
		case MM_PLANTS:
		case MM_ADDPOINTS:
		case MM_INSTANCES:
		case MM_VEHICLES:
		case MM_MOVE:
		case MM_MEASURE:
		case MM_SLOPE:
			m_pCursorMGeom->SetEnabled(true);
			EnableFlyerEngine(false);
			break;
		}
	}
	m_mode = mode;
}

void Enviro::SetTopDown(bool bTopDown)
{
	static bool bWas;

	m_bTopDown = bTopDown;

	if (bTopDown)
	{
		vtGetScene()->SetCamera(m_pTopDownCamera);
		bWas = m_pSkyDome->GetEnabled();
		m_pSkyDome->SetEnabled(false);
	}
	else
	{
		vtGetScene()->SetCamera(m_pNormalCamera);
		m_pSkyDome->SetEnabled(bWas);
		m_pOrthoFlyer->SetEnabled(false);
	}

	// set mode again, to put everything in the right state
	SetMode(m_mode);

	// inform the UI that we have switched cameras
	CameraChanged();
}

void Enviro::DumpCameraInfo()
{
	const vtCamera *cam = m_pNormalCamera;
	const FPoint3 pos = cam->GetTrans();
	const FPoint3 dir = cam->GetDirection();
	VTLOG("Camera: pos %f %f %f, dir %f %f %f\n",
		pos.x, pos.y, pos.z, dir.x, dir.y, dir.z);
}

void Enviro::SetSpeed(float x)
{
	if (m_state == AS_Orbit && m_pGlobeTime)
		m_pGlobeTime->SetSpeed(x);
	else if (m_state == AS_Terrain)
		GetTimeEngine()->SetSpeed(x);
}

float Enviro::GetSpeed()
{
	if (m_state == AS_Orbit && m_pGlobeTime)
		return m_pGlobeTime->GetSpeed();
	else if (m_state == AS_Terrain)
		return GetTimeEngine()->GetSpeed();
	return 0;
}

bool Enviro::OnMouse(vtMouseEvent &event)
{
	// check for what is under the 3D cursor
	if (m_state == AS_Orbit)
		DoCursorOnEarth();
	else if (m_state == AS_Terrain &&
		(m_pCursorMGeom->GetEnabled() || g_Options.m_bDirectPicking))
		DoCursorOnTerrain();

	// give the child classes first chance to take this event
	bool bCancel = OnMouseEvent(event);
	if (bCancel)
		return true;

	if (!OnMouseCompass(event))
		return false;

	if (event.type == VT_DOWN)
	{
		if (event.button == VT_LEFT)
		{
			if (m_state == AS_Terrain)
				OnMouseLeftDownTerrain(event);
			else if (m_state == AS_Orbit)
				OnMouseLeftDownOrbit(event);
		}
		else if (event.button == VT_RIGHT)
			OnMouseRightDown(event);
	}
	if (event.type == VT_MOVE)
	{
		OnMouseMove(event);
	}
	if (event.type == VT_UP)
	{
		if (event.button == VT_LEFT)
			OnMouseLeftUp(event);
		if (event.button == VT_RIGHT)
			OnMouseRightUp(event);
	}
	m_MouseLast = event.pos;
	return true;
}

void Enviro::OnMouseLeftDownTerrain(vtMouseEvent &event)
{
	if (m_mode != MM_SELECT &&
		m_mode != MM_SELECTMOVE &&
		m_mode != MM_SELECTBOX)
	{
		// All other modes require that we are pointing at some spot on the ground.
		if (!m_bOnTerrain)
			return;
	}

	// Many operations only need the 2D point
	const DPoint2 p2(m_EarthPos.x, m_EarthPos.y);
	vtTerrain *pTerr = GetCurrentTerrain();

	if (m_mode == MM_LINEARS || m_mode == MM_BUILDINGS)
	{
		// Add to a structure being drawn.
		AddElasticPoint(p2);
	}
	if (m_mode == MM_POWER)
	{
		if (!m_bActiveUtilLine)
			StartPowerline();

		pTerr->AddPoleToLine(m_pCurUtilLine, p2, m_sStructType);
	}
	if (m_mode == MM_PLANTS)
	{
		// try planting a tree there
		if (pTerr->IsGeographicCRS())
			VTLOG("Create a plant at %.8lf,%.8lf:", m_EarthPos.x, m_EarthPos.y);
		else
			VTLOG("Create a plant at %.2lf,%.2lf:", m_EarthPos.x, m_EarthPos.y);
		bool success = PlantATree(p2);
		VTLOG(" %s.\n", success ? "yes" : "no");
	}
	if (m_mode == MM_ADDPOINTS)
	{
		vtString str = GetStringFromUser("Created labeled point feature", "Label:");
		if (str != "")
		{
			vtAbstractLayer *alay = GetLabelLayer();
			vtFeatureSetPoint2D *pset = dynamic_cast<vtFeatureSetPoint2D*>(alay->GetFeatureSet());
			if (pset)
			{
				// add a single 2D point
				int rec = pset->AddPoint(p2);
				int field = pset->GetFieldIndex("Label");
				pset->SetValueFromString(rec, field, str);

				// create its 3D visual
				alay->CreateFeatureVisual(rec);
			}
			UpdateLayerView();
		}
	}
	if (m_mode == MM_INSTANCES)
		CreateInstance();

	if (m_mode == MM_VEHICLES)
		CreateGroundVehicle(m_VehicleOpt);

	if (m_mode == MM_SELECT || m_mode == MM_SELECTMOVE)
		OnMouseLeftDownTerrainSelect(event);

	if (m_mode == MM_SELECTBOX)
	{
		m_bDragging = true;
		m_MouseDown = event.pos;
	}

	if (m_mode == MM_MOVE)
		OnMouseLeftDownTerrainMove(event);

	if (m_mode == MM_MEASURE)
	{
		m_bDragging = true;

		if (m_bMeasurePath)
		{
			// Path mode - set initial segment
			int len = m_distance_path.GetSize();
			if (len == 0)
			{
				// begin new path
				m_fArcLength = 0.0;
				m_EarthPosDown = m_EarthPos;
				m_distance_path.Append(p2);
			}
			// default: add point to the path
			m_distance_path.Append(p2);

			SetTerrainMeasure(m_distance_path);
		}
		else
		{
			m_EarthPosDown = m_EarthPos;
			m_fArcLength = 0.0;
			SetTerrainMeasure(p2, p2);
		}
		UpdateDistanceTool();
	}
	if (m_mode == MM_SLOPE)
	{
		// TODO
	}
}

void Enviro::OnMouseSelectRayPick(vtMouseEvent &event)
{
	VTLOG("Click, raypick at %d %d, ", event.pos.x, event.pos.y);

	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureArray3d *pActiveStructures = pTerr->GetStructureLayer();

	if (!(event.flags & VT_CONTROL) && (pActiveStructures != NULL))
	{
		pActiveStructures->VisualDeselectAll();
		m_bSelectedStruct = false;
	}

	vtVegLayer *v_layer = pTerr->GetVegLayer();
	if (v_layer)
		v_layer->VisualDeselectAll();
	m_bSelectedPlant = false;

	vtUtilityMap3d &util_map = pTerr->GetUtilityMap();
	m_bSelectedUtil = false;

	// Get ray intersection with near and far planes
	FPoint3 Near, Dir;
	vtGetScene()->CameraRay(event.pos, Near, Dir);

	// Dir is unit-length direction vector, so scale it up to the
	//  distance we want to test.
	Dir *= 10000.0f;	// 10km should be enough for visible objects

	vtHitList HitList;
	int iNumHits = vtIntersect(pTerr->GetTopGroup(), Near, Near+Dir, HitList);
	if (iNumHits == 0)
	{
		VTLOG("no hits\n");
		return;
	}
	else
		VTLOG("%d hits\n", iNumHits);

	// Check for structures
	int iOffset;
	vtStructureLayer *slay;
	slay = pTerr->GetLayers().FindStructureFromNode(HitList.front().geode, iOffset);
	if (slay)
	{
		VTLOG("  Found structure ");
		vtBuilding3d *pBuilding = slay->GetBuilding(iOffset);
		vtStructInstance3d *pInstance = slay->GetInstance(iOffset);
		vtFence3d *pFence = slay->GetFence(iOffset);

		if (NULL != pBuilding)
		{
			// Found a procedural building
			VTLOG("(building)\n");
			pBuilding->ToggleSelect();
			if (pBuilding->IsSelected())
			{
				pBuilding->ShowBounds(true);
				m_bDragging = true;
			}
			else
				pBuilding->ShowBounds(false);
		}
		else if (NULL != pInstance)
		{
			// Found a structure instance
			VTLOG("(instance)\n");
			pInstance->ToggleSelect();
			if (pInstance->IsSelected())
			{
				pInstance->ShowBounds(true);
				if ((event.flags & VT_SHIFT) != 0)
				{
					m_StartRotation = pInstance->GetRotation();
					m_bRotating = true;
				}
				else
					m_bDragging = true;
			}
			else
				pInstance->ShowBounds(false);
		}
		else if (NULL != pFence)
		{
			// Found a linear structure
			VTLOG("(fence)\n");
			pFence->ToggleSelect();
			if (pFence->IsSelected())
				pFence->ShowBounds(true);
			else
				pFence->ShowBounds(false);
		}
		else
			VTLOG("(unknown)\n");
		if (pActiveStructures != slay)
		{
			// Switching to a different structure set
			pActiveStructures->VisualDeselectAll();
			pTerr->SetActiveLayer(slay);
			ShowLayerView();
			UpdateLayerView();
		}
		// This is inefficient it would be better to maintain a live count if possible
		if (pTerr->GetStructureLayer()->NumSelected())
			m_bSelectedStruct = true;
		else
			m_bSelectedStruct = false;
	}
	// Check for plants
	else if (v_layer && v_layer->FindPlantFromNode(HitList.front().geode, iOffset))
	{
		VTLOG("  Found plant\n");
		v_layer->VisualSelect(iOffset);
		m_bDragging = true;
		m_bSelectedPlant = true;
	}
	// Check for routes
	else if (util_map.FindPoleFromNode(HitList.front().geode, iOffset))
	{
		VTLOG("  Found route\n");
		m_bDragging = true;
		m_bSelectedUtil = true;
		m_pSelUtilPole = util_map.GetPole(iOffset);
	}
	else
		VTLOG("  Unable to identify node\n");

	if (m_bDragging)
		VTLOG("Now dragging.\n");
	if (m_bRotating)
		VTLOG("Now rotating.\n");
}

void Enviro::OnMouseSelectCursorPick(vtMouseEvent &event)
{
	VTLOG("Click, cursor pick, ");

	// See if camera ray intersects something?  NO, it's simpler and
	//  easier for the user to just test whether the ground cursor is
	//  near a something's origin.
	DPoint2 gpos(m_EarthPos.x, m_EarthPos.y);

	// De-select
	vtTerrain *pTerr = GetCurrentTerrain();
	if (!(event.flags & VT_CONTROL))
	{
		pTerr->DeselectAllStructures();
		pTerr->DeselectAllPlants();
	}

	const LocalCS &conv = pTerr->GetLocalCS();

	// SelectionCutoff is in meters, but the picking functions work in
	//  Earth coordinates.  Try to convert it to earth horiz units.
	DPoint2 eoffset;
	conv.VectorLocalToEarth(g_Options.m_fSelectionCutoff, 0, eoffset);
	double epsilon = eoffset.x;
	VTLOG("epsilon %lf, ", epsilon);

	VTLOG("|XY= %lf, %lf, %lf|\n", m_EarthPos.x, m_EarthPos.y, m_EarthPos.z); // BobMaX

	vtAbstractLayer *ab_layer = pTerr->GetAbstractLayer();
	if (ab_layer)
	{
		// If the current layer is abstract, only pick within it.
		vtFeatureSet *fset = ab_layer->GetFeatureSet();
		vtFeatureSetPoint2D *pset2 = dynamic_cast<vtFeatureSetPoint2D*>(fset);

		// Control key extends selection, otherwise deselect all.
		if (!(event.flags & VT_CONTROL))
		{
			for (uint j = 0; j < fset->NumEntities(); j++)
			{
				vtFeature *feat = fset->GetFeature(j);
				vtVisual *viz = ab_layer->GetViz(feat);
				if (viz)
					fset->Select(j, false);
			}
		}

		int index = -1;
		double dist;
		if (pset2)
			index = pset2->FindClosestPoint(gpos, epsilon, &dist);
		if (index >= 0)
		{
			VTLOG("abstract feature at dist.\n", dist);
			fset->Select(index, true);
		}
			
		// Make the selected meshes yellow
		ab_layer->UpdateVisualSelection();
		return;
	}

	// We also want to use a small (2m) buffer around linear features, so they
	//  can be picked even if they are inside/on top of a building.
	conv.VectorLocalToEarth(2.0f, 0, eoffset);
	double linear_buffer = eoffset.x;

	// Look at the distance to each type of object
	double dist1 = 1E9, dist2 = 1E9, dist3 = 1E9, dist4 = 1E9;

	// Check Structures
	vtStructureLayer *st_layer;	// layer that contains the closest structure
	int structure;				// index of closest structure
	bool result1 = pTerr->FindClosestStructure(gpos, epsilon, structure,
		&st_layer, dist1, g_Options.m_fMaxPickableInstanceRadius,
		(float) linear_buffer);
	if (result1)
		VTLOG("structure at dist %lf, ", dist1);
	m_bSelectedStruct = false;

	// Check Plants
	vtVegLayer *v_layer;
	m_bSelectedPlant = false;
	int plant_index;
	bool result2 = pTerr->FindClosestPlant(gpos, epsilon, plant_index, &v_layer);
	if (result2)
	{
		dist2 = (gpos - v_layer->GetPoint(plant_index)).Length();
		VTLOG("plant at dist %lf, ", dist2);
	}

	// Check Utilities
	vtUtilityMap3d &util_map = pTerr->GetUtilityMap();
	m_bSelectedUtil = false;
	bool result3 = util_map.FindClosestUtilPole(gpos, epsilon, m_pSelUtilPole, dist3);

	// Check Vehicles
	m_bSelectedVehicle = false;
	FPoint3 wpos;
	conv.EarthToLocal(m_EarthPos, wpos);
	m_Vehicles.VisualDeselectAll();
	int vehicle = m_Vehicles.FindClosestVehicle(wpos, dist4);
	if (dist4 > g_Options.m_fSelectionCutoff)
		vehicle = -1;

	bool click_struct = (result1 && dist1 < dist2 && dist1 < dist3 && dist1 < dist4);
	bool click_plant = (result2 && dist2 < dist1 && dist2 < dist3 && dist2 < dist4);
	bool click_route = (result3 && dist3 < dist1 && dist3 < dist2 && dist3 < dist4);
	bool click_vehicle = (vehicle!=-1 && dist4 < dist1 && dist4 < dist2 && dist4 < dist3);

	if (click_struct)
	{
		VTLOG(" struct is closest.\n");
		vtStructure *str = st_layer->at(structure);
		vtStructure3d *str3d = st_layer->GetStructure3d(structure);
		if (str->GetType() != ST_INSTANCE && str3d->GetGeom() == NULL)
		{
			VTLOG("  Warning: unconstructed structure.\n");
		}
		else
		{
			str->Select(true);
			str3d->ShowBounds(true);

			vtStructInstance *inst = str->GetInstance();
			vtFence *fen = str->GetFence();

			// Reset dragging state
			m_bRotating = false;
			m_pDraggingFence = NULL;
			m_iDraggingFencePoint = -1;		// no grab

			if (inst != NULL && (event.flags & VT_SHIFT) != 0)
			{
				m_StartRotation = inst->GetRotation();
				m_bRotating = true;
			}
			else if (fen != NULL)
			{
				// perhaps we have clicked on a fence control point
				double dist;
				const int idx = fen->GetNearestPointIndex(gpos, dist);
				if (idx != -1 && dist < 2.0f)	// distance cutoff
				{
					m_pDraggingFence = dynamic_cast<vtFence3d*>(str3d);
					m_iDraggingFencePoint = idx;	// grab
					m_bDragging = true;
				}
			}
			else
			{
				m_bDragging = true;
				// Press 'alt' key to drag vertically instead of horizontally
				if ((event.flags & VT_ALT) != 0)
					m_bDragUpDown = true;
			}
			m_bSelectedStruct = true;
		}
		if (st_layer != pTerr->GetActiveLayer())
		{
			// active structure set (layer) has changed due to picking
			pTerr->SetActiveLayer(st_layer);
			ShowLayerView();
			UpdateLayerView();
		}
	}
	else if (click_plant)
	{
		VTLOG(" plant is closest.\n");
		v_layer->VisualSelect(plant_index);
		m_bDragging = true;
		m_bSelectedPlant = true;

		if (v_layer != pTerr->GetActiveLayer())
		{
			// active layer has changed due to picking
			pTerr->SetActiveLayer(v_layer);
			ShowLayerView();
			UpdateLayerView();
		}
	}
	else if (click_route)
	{
		m_bDragging = true;
		m_bSelectedUtil = true;
	}
	else if (click_vehicle)
	{
		VTLOG(" vehicle is closest.\n");
		m_Vehicles.VisualSelect(vehicle);
		if ((event.flags & VT_SHIFT) != 0)
		{
			m_StartRotation = m_Vehicles.GetSelectedCarEngine()->GetRotation();
			m_bRotating = true;
		}
		else
			m_bDragging = true;
		m_bSelectedVehicle = true;
	}
	else
		VTLOG(" nothing.\n");
}

void Enviro::OnMouseLeftDownTerrainSelect(vtMouseEvent &event)
{
	if (g_Options.m_bDirectPicking)
		OnMouseSelectRayPick(event);
	else
		OnMouseSelectCursorPick(event);

	m_EarthPosDown = m_EarthPosLast = m_EarthPos;
	m_MouseDown = event.pos;
}

void Enviro::OnMouseLeftDownTerrainMove(vtMouseEvent &event)
{
	m_EarthPosDown = m_EarthPosLast = m_EarthPos;
	m_MouseDown = event.pos;

	// In move mode, always start dragging
	m_bDragging = true;

	// Press 'alt' key to drag vertically instead of horizontally
	if ((event.flags & VT_ALT) != 0)
		m_bDragUpDown = true;
}

void Enviro::OnMouseLeftUp(vtMouseEvent &event)
{
	m_bDragging = m_bDragUpDown = m_bRotating = false;

	if (m_state == AS_Orbit && m_mode == MM_MEASURE && m_bDragging)
		UpdateEarthArc();

	if (m_mode == MM_SELECTBOX)
		OnMouseLeftUpBox(event);
}

void Enviro::OnMouseLeftUpBox(vtMouseEvent &event)
{
	// Hide the rubber box
	SetWindowBox(IPoint2(0,0), IPoint2(0,0));

	// Do selection in window (HUD) coordinates
	// Frustum's origin is lower left, mouse origin is upper left
	IPoint2 winsize = vtGetScene()->GetWindowSize();

	FRECT select_box((float) m_MouseDown.x, (float) winsize.y-1-m_MouseDown.y,
					 (float) event.pos.x,	(float) winsize.y-1-event.pos.y);
	select_box.Sort();

	// We can use the view matrix to project each 3d object onto the window,
	//  to compare them against our box.
	FMatrix4 viewmat;
	vtGetScene()->ComputeViewMatrix(viewmat);

	vtTerrain *terr = GetCurrentTerrain();
	float fVerticalExag = terr->GetVerticalExag();
	LayerSet &layers = terr->GetLayers();
	for (uint i = 0; i < layers.size(); i++)
	{
		vtAbstractLayer *alay = dynamic_cast<vtAbstractLayer*>(layers[i].get());
		if (!alay)
			continue;

		vtFeatureSet *fset = alay->GetFeatureSet();
		vtFeatureSetPoint2D *pset2 = dynamic_cast<vtFeatureSetPoint2D*>(fset);
		vtFeatureSetPoint3D *pset3 = dynamic_cast<vtFeatureSetPoint3D*>(fset);
		if (pset2 || pset3)
		{
			FBox3 bbox;
			for (uint j = 0; j < fset->NumEntities(); j++)
			{
				vtFeature *feat = fset->GetFeature(j);
				vtVisual *viz = alay->GetViz(feat);
				if (!viz)
					continue;

				// Control key extends selection
				if (!(event.flags & VT_CONTROL))
					fset->Select(j, false);

				bool bSelected = false;
				for (uint k = 0; k < viz->m_meshes.size(); k++)
				{
					vtMesh *mesh = viz->m_meshes[k];
					mesh->GetBoundBox(bbox);
					FPoint3 center = bbox.Center();

					// Account for potential vertical exaggeration
					center.y *= fVerticalExag;

					// Project 3d pos to 2d window pos
					FPoint3 frustump = viewmat.PreMult(center);

					// If inside the window box
					if (select_box.ContainsPoint(frustump.x, frustump.y) &&
						frustump.z > 0 &&
						frustump.z < 1)		// and in front of camera
					{
						bSelected = true;
					}
				}
				if (bSelected)
					fset->Select(j, true);
			}
			// Make the selected meshes yellow
			alay->UpdateVisualSelection();
		}
	}
	UpdateLayerView();
}

void Enviro::OnMouseRightDown(vtMouseEvent &event)
{
	if (m_state == AS_Terrain)
	{
		if (m_mode == MM_BUILDINGS && m_Elastic.NumPoints() > 0)
			FinishBuilding();

		if (m_mode == MM_LINEARS && m_Elastic.NumPoints() > 0)
			FinishLinear();
	}
}

void Enviro::OnMouseRightUp(vtMouseEvent &event)
{
	if (m_state == AS_Terrain)
	{
		if (m_mode == MM_POWER)
			FinishPowerline();

		if (m_mode == MM_SELECT || m_mode == MM_SELECTMOVE)
		{
			vtTerrain *terr = GetCurrentTerrain();
			vtStructureArray3d *sa = terr->GetStructureLayer();
			vtVegLayer *vlay = terr->GetVegLayer();
			vtAbstractLayer *alay = terr->GetAbstractLayer();

			bool show = false;
			if (sa && sa->NumSelected() != 0)
				show = true;
			if (vlay && vlay->NumSelected() != 0)
				show = true;
			if (m_Vehicles.GetSelected() != -1)
				show = true;
			if (alay && alay->GetFeatureSet() && alay->GetFeatureSet()->NumSelected() != 0)
				show = true;
			if (show)
				ShowPopupMenu(event.pos);
		}
		if (m_mode == MM_SELECTBOX)
		{
			if (NumSelectedAbstractFeatures() > 0)
				ShowPopupMenu(event.pos);
		}
	}
}

void Enviro::OnMouseMove(vtMouseEvent &event)
{
	if (m_state == AS_Terrain)
		OnMouseMoveTerrain(event);

	if (m_state == AS_Orbit && m_mode == MM_MEASURE && m_bDragging)
		UpdateEarthArc();
}

void Enviro::OnMouseMoveTerrain(vtMouseEvent &event)
{
	// Many operations only need a 2D point
	const DPoint2 p2(m_EarthPos.x, m_EarthPos.y);

	if ((m_mode == MM_SELECTMOVE || m_mode == MM_MOVE) &&
		(m_bDragging || m_bRotating))
	{
		const DPoint3 delta = m_EarthPos - m_EarthPosLast;
		const DPoint2 ground_delta(delta.x, delta.y);

		//VTLOG("ground_delta %f, %f\n", delta.x, delta.y);

		const float fNewRotation = m_StartRotation + (event.pos.x - m_MouseDown.x) / 100.0f;

		vtTerrain *pTerr = GetCurrentTerrain();

		vtStructureLayer *st_layer = pTerr->GetStructureLayer();
		if (st_layer && st_layer->NumSelected() > 0)
		{
			if (m_bDragging)
			{
				if (m_bDragUpDown)
				{
					// Moving a whole structure (building or instance)
					float fDelta = (m_MouseLast.y - event.pos.y) / 20.0f;
					st_layer->OffsetSelectedStructuresVertical(fDelta);
				}
				else if (m_pDraggingFence != NULL)
				{
					// Dragging a linear structure point
					DLine2 &pts = m_pDraggingFence->GetFencePoints();
					pts[m_iDraggingFencePoint] += ground_delta;
					m_pDraggingFence->CreateNode(pTerr);
				}
				else
				{
					// Moving a whole structure (building or instance)
					st_layer->OffsetSelectedStructures(ground_delta);
				}
			}
			else if (m_bRotating)
			{
				for (int sel = st_layer->GetFirstSelected(); sel != -1; sel = st_layer->GetNextSelected())
				{
					vtStructInstance *inst = st_layer->at(sel)->GetInstance();
					vtStructInstance3d *str3d = st_layer->GetInstance(sel);

					inst->SetRotation(fNewRotation);
					str3d->UpdateTransform(pTerr->GetHeightField());
				}
			}
			st_layer->SetModified();
		}
		if (m_bDragging)
		{
			if (m_bSelectedPlant)
			{
				vtVegLayer *vlay = pTerr->GetVegLayer();
				if (vlay)
					vlay->OffsetSelectedPlants(ground_delta);
				vlay->SetModified();
			}
			if (m_bSelectedUtil)
			{
				m_pSelUtilPole->Offset(ground_delta);
				pTerr->RebuildUtilityGeometry();
			}
			if (m_bSelectedVehicle)
			{
				CarEngine *eng = m_Vehicles.GetSelectedCarEngine();
				if (eng)
				{
					const LocalCS &conv = pTerr->GetLocalCS();
					eng->SetEarthPos(conv, eng->GetEarthPos(conv) + ground_delta);
				}
			}
		}
		else if (m_bRotating)
		{
			if (m_bSelectedVehicle)
			{
				CarEngine *eng = m_Vehicles.GetSelectedCarEngine();
				if (eng)
					eng->SetRotation(fNewRotation);
			}
		}
		m_EarthPosLast = m_EarthPos;
	}
	if (m_mode == MM_SELECTBOX && m_bDragging)
	{
		SetWindowBox(m_MouseDown, event.pos);
	}
	if (m_mode == MM_MEASURE && m_bDragging && m_bOnTerrain)
	{
//		VTLOG("MouseMove, MEASURE & Drag & OnTerrain: %.1lf, %.1lf\n", m_EarthPos.x, m_EarthPos.y);
		if (m_bMeasurePath)
		{
			const uint npoints = m_distance_path.GetSize();
			if (npoints > 1)
				m_distance_path[npoints-1] = p2;
			SetTerrainMeasure(m_distance_path);
		}
		else
		{
			DPoint2 p1(m_EarthPosDown.x, m_EarthPosDown.y);
			SetTerrainMeasure(p1, p2);
		}
		UpdateDistanceTool();
	}
	if (m_mode == MM_LINEARS)
	{
		const uint npoints = m_Elastic.NumPoints();
		if (npoints > 1)
			m_Elastic.SetPoint(npoints-1, p2, false);
	}
	if (m_mode == MM_BUILDINGS)
	{
		const uint npoints = m_Elastic.NumPoints();
		if (npoints > 1)
			m_Elastic.SetPoint(npoints-1, p2, m_bConstrainAngles);
	}
	if (m_mode == MM_SLOPE && m_bDragging && m_bOnTerrain)
	{
		// Optional TODO
	}
}

/**
 * Mouse handler should return false if it absorbed the event, true if the
 * event should continue to propagate.
 */
bool Enviro::OnMouseCompass(vtMouseEvent &event)
{
	if (m_pCompassSizer)
	{
		if (m_bDragCompass)
		{
			if (event.type == VT_UP)
				m_bDragCompass = false;

			if (event.type == VT_MOVE)
			{
				const FPoint2 center = m_pCompassSizer->GetWindowCenter();
				const FPoint2 pos(event.pos.x, event.pos.y);
				const FPoint2 diff = (pos - center);
				const float angle = atan2(diff.y, diff.x);
				const float delta = angle - m_fDragAngle;
				m_fDragAngle = angle;

				vtCamera *cam = vtGetScene()->GetCamera();
				cam->RotateParent(FPoint3(0,1.0f,0), delta);
			}
		}
		else if (event.type == VT_DOWN)
		{
			const FPoint2 center = m_pCompassSizer->GetWindowCenter();
			const FPoint2 pos(event.pos.x, event.pos.y);
			const FPoint2 diff = (pos - center);
			const float dist = diff.Length();
			if (dist > 20 && dist < 64)
			{
				m_bDragCompass = true;
				m_fDragAngle = atan2(diff.y, diff.x);
			}
		}

		if (m_bDragCompass)
			return false;
	}
	return true;
}


void Enviro::SetupArcMaterials()
{
	if (!m_pArcMats)
	{
		m_pArcMats = new vtMaterialArray;
		m_pArcMats->AddRGBMaterial(RGBf(1, 1, 0), false, false); // yellow
		m_pArcMats->AddRGBMaterial(RGBf(1, 0, 0), false, false); // red
		m_pArcMats->AddRGBMaterial(RGBf(1, 0.5f, 0), true, true); // orange lit
	}
}

void Enviro::SetupArcMesh()
{
	SetupArcMaterials();

	// create geometry container, if needed
	if (!m_pArc)
	{
		m_pArc = new vtGeode;
		if (m_state == AS_Orbit)
			m_pIcoGlobe->GetTop()->addChild(m_pArc);
		else if (m_state == AS_Terrain)
			GetCurrentTerrain()->GetTopGroup()->addChild(m_pArc);
		m_pArc->SetMaterials(m_pArcMats);
	}

	// re-create mesh if not the first time
	FreeArcMesh();
}

void Enviro::FreeArc()
{
	FreeArcMesh();
	if (m_pArc)
		m_pArc = NULL;
}

void Enviro::FreeArcMesh()
{
	if (m_pArc)
		m_pArc->RemoveAllMeshes();
}

void Enviro::SetTerrainMeasure(const DPoint2 &g1, const DPoint2 &g2)
{
	// place the arc for the distance measuring tool on the terrain
	SetupArcMesh();

	DLine2 dline;
	dline.Append(g1);
	dline.Append(g2);

	vtTerrain *pTerr = GetCurrentTerrain();
	vtGeomFactory mf(m_pArc, osg::PrimitiveSet::LINE_STRIP, 0, 30000, 1);
	mf.SetLineWidth(2);

	const float fSpacing = pTerr->EstimateGroundSpacingAtPoint(dline[0]);

	m_fArcLength = mf.AddSurfaceLineToMesh(pTerr->GetHeightField(), dline,
		fSpacing, m_fDistToolHeight, true);
}

void Enviro::SetTerrainMeasure(const DLine2 &path)
{
	// place the arc for the distance measuring tool on the terrain
	SetupArcMesh();

	vtTerrain *pTerr = GetCurrentTerrain();
	vtGeomFactory mf(m_pArc, osg::PrimitiveSet::LINE_STRIP, 0, 30000, 1);
	mf.SetLineWidth(2);

	const float fSpacing = pTerr->EstimateGroundSpacingAtPoint(path[0]);

	m_fArcLength = mf.AddSurfaceLineToMesh(pTerr->GetHeightField(), path,
		fSpacing, m_fDistToolHeight, true);
}

void Enviro::SetDistanceToolMode(bool bPath)
{
	// if switching modes, reset
	bool bNeedReset = (m_bMeasurePath != bPath);

	m_bMeasurePath = bPath;

	if (bNeedReset)
		ResetDistanceTool();
}

void Enviro::SetDistanceTool(const DLine2 &line)
{
	m_distance_path = line;
	SetTerrainMeasure(m_distance_path);
	ShowDistance(m_distance_path, m_fArcLength, FLT_MIN);
}

void Enviro::ResetDistanceTool()
{
	m_distance_path.Clear();
	m_fArcLength = 0.0;
	if (m_bMeasurePath)
		ShowDistance(DLine2(), FLT_MIN, FLT_MIN);
	else
		ShowDistance(DPoint2(0,0), DPoint2(0,0), FLT_MIN, FLT_MIN);

	// remove visible terrain line
	FreeArcMesh();
}

void Enviro::UpdateDistanceTool()
{
	if (m_bMeasurePath)
		ShowDistance(m_distance_path, m_fArcLength, m_EarthPos.z - m_EarthPosDown.z);
	else
	{
		DPoint2 g1(m_EarthPosDown.x, m_EarthPosDown.y);
		DPoint2 g2(m_EarthPos.x, m_EarthPos.y);
		ShowDistance(g1, g2, m_fArcLength, m_EarthPos.z - m_EarthPosDown.z);
	}
}

// Wind
void Enviro::SetWind(int iDirection, float fSpeed)
{
	// Store these properties on the terrain
	vtTerrain *terr = GetCurrentTerrain();
	TParams &params = terr->GetParams();
	params.SetValueInt("WindDirection", iDirection);
	params.SetValueFloat("WindSpeed", fSpeed);
}


////////////////////////////////////////////////////////////////
// Elastic

const double kMinimumEdgeLengthDegrees = 2e-6;
const double kMinimumEdgeLengthMeters = 0.2;

void Enviro::AddElasticPoint(const DPoint2 &p)
{
	// Try to prevent the user from making bad geometry with points too close together
	const int npoints = m_Elastic.NumPoints();
	if ( npoints >= 3)
	{
		const DPoint2 p0 = m_Elastic.GetPolyline().GetAt(npoints-3);
		const DPoint2 p1 = m_Elastic.GetPolyline().GetAt(npoints-2);
		const DPoint2 p2 = m_Elastic.GetPolyline().GetAt(npoints-1);

		const bool bIsGeo = GetCurrentTerrain()->IsGeographicCRS();
		const double dMin = (bIsGeo ? kMinimumEdgeLengthDegrees : kMinimumEdgeLengthMeters);

		if ((p0 - p1).Length() < dMin || (p1 - p2).Length() < dMin)
		{
			// too close
			VTLOG1(" too close, omitting point.\n");
			return;
		}
	}
	// we use two points to begin with
	if (npoints == 0)
		m_Elastic.AddPoint(p);
	m_Elastic.AddPoint(p);
}

bool Enviro::IsMakingElastic()
{
	return (m_Elastic.NumPoints() > 0);
}

void Enviro::CancelElastic()
{
	m_Elastic.Clear();
}


////////////////////////////////////////////////////////////////
// Linear Structures

void Enviro::FinishLinear()
{
	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureLayer *st_layer = pTerr->GetStructureLayer();
	if (!st_layer)
		return;

	// Must have at least 2 points.
	if (m_Elastic.NumPoints() < 2)
		return;

	// Close and create new fence in the current structure array
	vtFence3d *fence = (vtFence3d*) st_layer->AddNewFence();
	st_layer->SetModified(true);

	fence->SetParams(m_FenceParams);
	fence->SetFencePoints(m_Elastic.GetPolyline());

	// Hide the temporary markers which showed the polyline
	m_Elastic.Clear();

	// Construct it and add it to the terrain
	fence->CreateNode(pTerr);
	pTerr->AddNodeToStructGrid(fence->GetGeom());

	// update count shown in layer view
	RefreshLayerView();
}

void Enviro::SetFenceOptions(const vtLinearParams &param, bool bProfileChanged)
{
	VTLOG1("SetFenceOptions\n");
	m_FenceParams = param;

	vtTerrain *pTerr = GetCurrentTerrain();
	if (!pTerr)
		return;

	vtStructureArray3d *structures = pTerr->GetStructureLayer();
	for (uint i = 0; i < structures->size(); i++)
	{
		vtStructure *str = structures->at(i);
		if (!str->IsSelected() || str->GetType() != ST_LINEAR)
			continue;
		vtFence3d *pFence = structures->GetFence(i);
		pFence->SetParams(param);
		if (bProfileChanged)
			pFence->ProfileChanged();
		pFence->CreateNode(pTerr);	// re-create
	}
}


////////////////////////////////////////////////////////////////
// Buildings

void Enviro::FinishBuilding()
{
	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureLayer *st_layer = pTerr->GetStructureLayer();
	if (!st_layer)
		return;

	if (m_bConstrainAngles)
	{
		// We can't have all right angles and an odd number of points.
		const int num = m_Elastic.NumPoints();
		if (num & 1)
			m_Elastic.RemovePoint(num-1);

		// To ensure that we have right angles all around, act as if the user
		//  clicked back on the original point
		m_Elastic.AddPoint(m_Elastic.GetPolyline().GetAt(0), m_bConstrainAngles);
		m_Elastic.RemovePoint(m_Elastic.NumPoints()-1);
	}

	// Must have at least 3 points.
	if (m_Elastic.NumPoints() < 3)
		return;

	// Hide the temporary markers which showed the polyline
	DLine2 line = m_Elastic.GetPolyline();
	m_Elastic.Clear();

	// Close and create new building in the current structure array
	vtBuilding3d *pbuilding = (vtBuilding3d*) st_layer->AddNewBuilding();
	st_layer->SetModified(true);

	// Force footprint anticlockwise
	PolyChecker PolyChecker;
	if (PolyChecker.IsClockwisePolygon(line))
		line.ReverseOrder();
	pbuilding->SetFootprint(0, line);

	// Describe the appearance of the new building
	pbuilding->SetNumStories(2);
	pbuilding->SetRoofType(ROOF_HIP);
	pbuilding->SetColor(BLD_BASIC, RGBi(255,255,255));
	pbuilding->SetColor(BLD_ROOF, RGBi(230,200,170));

	// Construct it and add it to the terrain
	pbuilding->CreateNode(pTerr);
	pTerr->AddNodeToStructGrid(pbuilding->GetContainer());
	RefreshLayerView();
}

void Enviro::FlipBuildingFooprints()
{
	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureLayer *structures = pTerr->GetStructureLayer();

	vtStructure *str;
	vtBuilding3d *bld;
	for (uint i = 0; i < structures->size(); i++)
	{
		str = structures->at(i);
		if (!str->IsSelected())
			continue;

		bld = structures->GetBuilding(i);
		if (!bld)
			continue;
		bld->FlipFootprintDirection();
		structures->ConstructStructure(structures->GetStructure3d(i));
	}
}

void Enviro::SetBuildingEaves(float fLength)
{
	vtStructureLayer *structures = GetCurrentTerrain()->GetStructureLayer();

	for (uint i = 0; i < structures->size(); i++)
	{
		if (structures->at(i)->IsSelected())
		{
			vtBuilding3d *bld = structures->GetBuilding(i);
			if (bld)
			{
				bld->SetEaves(fLength);
				structures->ConstructStructure(structures->GetStructure3d(i));
			}
		}
	}
}

void Enviro::CopyBuildingStyle()
{
	const vtTerrain *pTerr = GetCurrentTerrain();
	const vtStructureArray3d *sa = pTerr->GetStructureLayer();
	const vtBuilding *bld = sa->GetFirstSelectedStructure()->GetBuilding();

	// Make a copy of the building
	m_BuildingStyle = *bld;
}

void Enviro::PasteBuildingStyle()
{
	vtStructureLayer *structures = GetCurrentTerrain()->GetStructureLayer();
	for (uint i = 0; i < structures->size(); i++)
	{
		const vtStructure *str = structures->at(i);
		if (str->IsSelected())
		{
			vtBuilding3d *bld = structures->GetBuilding(i);
			if (bld)
			{
				bool do_height = false;
				bld->CopyStyleFrom(&m_BuildingStyle, do_height);
				structures->ConstructStructure(structures->GetStructure3d(i));
			}
		}
	}
}

bool Enviro::HaveBuildingStyle()
{
	return (m_BuildingStyle.NumLevels() != 0);
}


////////////////////////////////////////////////////////////////
// Utility Map (currently, power transmission lines)

void Enviro::StartPowerline()
{
	VTLOG1("StartPowerline\n");
	vtLine3d *line = GetCurrentTerrain()->NewLine();
	m_pCurUtilLine = line;
	m_bActiveUtilLine = true;
}

void Enviro::FinishPowerline()
{
	VTLOG1("FinishPowerline\n");
	m_bActiveUtilLine = false;
}

void Enviro::SetPowerOptions(const vtString &sStructType)
{
	m_sStructType = sStructType;
}


////////////////////////////////////////////////
// Plants

void Enviro::SetPlantOptions(const PlantingOptions &opt)
{
	m_PlantOpt = opt;
	if (m_mode == MM_SELECT || m_mode == MM_SELECTMOVE)
	{
		vtVegLayer *vlay = GetCurrentTerrain()->GetVegLayer();
		if (vlay)
		{
			for (uint i = 0; i < vlay->NumEntities(); i++)
			{
				if (vlay->IsSelected(i))
				{
					vlay->SetPlant(i, opt.m_fHeight, opt.m_iSpecies);
					vlay->CreatePlantNode(i);
				}
			}
		}
	}
}

void Enviro::SetVehicleOptions(const VehicleOptions &opt)
{
	m_VehicleOpt = opt;
}

/**
 * Plant a tree at the given location (in earth coordinates)
 */
bool Enviro::PlantATree(const DPoint2 &epos)
{
	if (!m_pSpeciesList)
		return false;

	vtTerrain *pTerr = GetCurrentTerrain();
	if (!pTerr)
		return false;

	// check distance from other plants
	vtVegLayer *vlay = pTerr->GetVegLayer();
	if (!vlay)
		return false;

	const int size = vlay->NumEntities();
	double len, closest = 1E8;

	bool bValidLocation = true;
	if (m_PlantOpt.m_fSpacing > 0.0f)
	{
		// Spacing is in meters, but the picking functions work in
		//  Earth coordinates.  Try to convert it to earth horiz units.
		DPoint2 eoffset;
		const LocalCS &conv = pTerr->GetLocalCS();
		conv.VectorLocalToEarth(m_PlantOpt.m_fSpacing, 0, eoffset);
		double mininum_spacing = eoffset.x;

		for (int i = 0; i < size; i++)
		{
			len = (epos - vlay->GetPoint(i)).Length();
			if (len < closest)
				closest = len;
		}
		if (closest < mininum_spacing)
			bValidLocation = false;
		VTLOG(" closest plant %.2fm,%s planting..", closest, bValidLocation ? "" : " not");
	}
	if (!bValidLocation)
		return false;

	float height = m_PlantOpt.m_fHeight;
	const float variance = m_PlantOpt.m_iVariance / 100.0f;
	height *= (1.0f + random(variance*2) - variance);
	if (!pTerr->AddPlant(vlay, epos, m_PlantOpt.m_iSpecies, height))
		return false;

	vlay->SetModified();

	// If there is a GUI, let it update to show one more plant
	UpdateLayerView();
	return true;
}


//// Instances

void Enviro::CreateInstance()
{
	vtTagArray *tags = GetInstanceFromGUI();
	if (!tags)
		return;
	CreateInstanceAt(DPoint2(m_EarthPos.x, m_EarthPos.y), tags);
}

// create a new Instance object
void Enviro::CreateInstanceAt(const DPoint2 &pos, vtTagArray *tags)
{
	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureLayer *st_layer = pTerr->GetStructureLayer();
	if (!st_layer)
		return;

	vtStructInstance3d *inst = (vtStructInstance3d *) st_layer->AddNewInstance();

	inst->CopyTagsFrom(*tags);
	inst->SetPoint(pos);
	VTLOG("Create Instance at %.7g, %.7g: ", pos.x, pos.y);

	const int index = st_layer->size() - 1;
	bool success = pTerr->CreateStructure(st_layer, index);
	if (success)
	{
		VTLOG(" succeeded.\n");
		st_layer->SetModified();
		RefreshLayerView();
	}
	else
	{
		// creation failed
		VTLOG(" failed.\n");
		ShowMessage("Could not create instance.");
		inst->Select(true);
		st_layer->DeleteSelected();
		return;
	}
}

/**
 There are two message strings set by reference.

 The first string should contain the part of the message that will be
 translated (like "Cursor: ").
 
 The second string can contain the part of the message that should not be
 translated (like "1152.5, 12351.4")

 The two strings will be concatenated later.
 */
void Enviro::DescribeCoordinatesTerrain(vtString &str1, vtString &str2)
{
	// Ground cursor
	DPoint3 epos;
	const bool bOn = m_pTerrainPicker->GetCurrentEarthPos(epos);
	if (bOn)
	{
		str1 = _("Cursor: ");
		vtTerrain *pTerr = GetCurrentTerrain();
		if (pTerr)
		{
			const LocalCS &conv = pTerr->GetLocalCS();
			FormatCoordString(str2, epos, conv.GetUnits(), true);
		}
		else
			str2 = "";
	}
	else
	{
		str1 = _("Cursor: Not on ground");
		str2 = "";
	}
}

void Enviro::DescribeCLOD(vtString &str)
{
	str = "";

	if (m_state != AS_Terrain)
		return;
	const vtTerrain *t = GetCurrentTerrain();
	if (!t)
		return;
	const vtDynTerrainGeom *dtg = t->GetDynTerrain();
	if (!dtg)
		return;

	// McNally and Roettger CLOD algos use a triangle/vertex count target.
	//  The older implementations use a floating point factor relating to
	//  error/detail.
	//
	LodMethodEnum method = t->GetParams().GetLodMethod();
	if (method == LM_MCNALLY || method == LM_ROETTGER)
	{
		str.Format("CLOD: target %d, drawn %d ", dtg->GetPolygonTarget(),
			dtg->NumDrawnTriangles());
	}
}

/**
 \param which 0 for the "fps" field,
			  1 for the "cursor" field (location of the cursor),
			  2 for the "cursor val" (value under the cursor).
 \param str1 The first part of the string (translated).
 \param str2 The seconds part of the string (untranslated).
 */
void Enviro::GetStatusString(int which, vtString &str1, vtString &str2)
{
	vtScene *scene = vtGetScene();

	vtString str;
	if (which == 0)
	{
		str1 = _("fps");

		// Fps: get framerate
		const float fps = scene->GetFrameRate();

		// only show 3 significant digits
		if (fps < 10)
			str2.Format(" %1.2f", fps);
		else if (fps < 80)
			str2.Format(" %2.1f", fps);
		else
			str2.Format(" %3.0f", fps);
	}
	if (which == 1)
	{
		if (m_state == AS_Orbit)
			DescribeCoordinatesEarth(str1, str2);
		else if (m_state == AS_Terrain)
			DescribeCoordinatesTerrain(str1, str2);
	}
	if (which == 2)
	{
		DPoint3 epos;

		if (m_state == AS_Orbit)
		{
			m_pGlobePicker->GetCurrentEarthPos(epos);
			vtTerrain *pTerr = FindTerrainOnEarth(DPoint2(epos.x, epos.y));
			if (pTerr)
			{
				str1 = "";
				str2 = pTerr->GetName();	// Don't try to translate the name.
			}
		}
		else if (m_state == AS_Terrain)
		{
			const bool bOn = m_pTerrainPicker->GetCurrentEarthPos(epos);
			if (bOn)
			{
				float exag;
				{
					// Avoid trouble with '.' and ',' in Europe
					ScopedLocale normal_numbers(LC_NUMERIC, "C");
					// Report true elevation, without vertical exaggeration
					exag = GetCurrentTerrain()->GetVerticalExag();
				}
				epos.z /= exag;
				str1 = _("Elev: ");				// Part to translate
				str2.Format("%.1f", epos.z);	// Part to not translate
			}
			else
			{
				str1 = _("Not on ground");
				str2 = "";
			}
		}
	}
}

void Enviro::ActivateAStructureLayer()
{
	vtTerrain *terr = GetCurrentTerrain();
	if (!terr)
		return;

	vtLayer *active = terr->GetActiveLayer();
	terr->SetActiveLayer(terr->GetOrCreateLayerOfType(LT_STRUCTURE));

	// If it changed, refresh
	if (terr->GetActiveLayer() != active)
		RefreshLayerView();
}

void Enviro::ActivateAVegetationLayer()
{
	vtTerrain *terr = GetCurrentTerrain();
	if (!terr)
		return;

	vtLayer *active = terr->GetActiveLayer();
	terr->SetActiveLayer(terr->GetOrCreateLayerOfType(LT_VEG));

	// If it changed, refresh
	if (terr->GetActiveLayer() != active)
		RefreshLayerView();
}

// Handle the map overview option
void Enviro::ShowMapOverview(bool bShow)
{
	if (bShow && !m_pMapOverview)
		CreateMapOverview();
	if (m_pMapOverview)
		m_pMapOverview->ShowMapOverview(bShow);
}

bool Enviro::GetShowMapOverview()
{
	if (m_pMapOverview)
		return m_pMapOverview->GetShowMapOverview();
	return false;
}

void Enviro::CreateMapOverview()
{
	// setup the mapoverview engine
	if (!m_pMapOverview)
	{
		m_pMapOverview = new MapOverviewEngine(GetCurrentTerrain());
		m_pMapOverview->setName("Map overview engine");
		vtGetScene()->AddEngine(m_pMapOverview);
	}
}

void Enviro::TextureHasChanged()
{
	// Texture has changed, so update the overview
	if (m_pMapOverview && m_pMapOverview->GetShowMapOverview())
	{
		m_pMapOverview->SetTerrain(GetCurrentTerrain());
	}
}

void Enviro::ShowElevationLegend(bool bShow)
{
	if (bShow && !m_bCreatedLegend)
		CreateElevationLegend();
	if (m_pLegendGeom)
		m_pLegendGeom->SetEnabled(bShow);
}

bool Enviro::GetShowElevationLegend()
{
	if (m_pLegendGeom)
		return m_pLegendGeom->GetEnabled();
	return false;
}

void Enviro::ShowCompass(bool bShow)
{
	if (bShow && !m_bCreatedCompass)
		CreateCompass();
	if (m_pCompassGeom)
		m_pCompassGeom->SetEnabled(bShow);
}

bool Enviro::GetShowCompass()
{
	if (m_pCompassGeom)
		return m_pCompassGeom->GetEnabled();
	return false;
}

void Enviro::UpdateCompass()
{
	const vtCamera *cam = vtGetScene()->GetCamera();
	if (!cam)
		return;
	const FPoint3 dir = cam->GetDirection();
	const float theta = atan2(dir.z, dir.x) + PID2f;
	if (m_pCompassSizer)
	{
		m_pCompassSizer->SetRotation(theta);
		IPoint2 size = vtGetScene()->GetWindowSize();
		m_pCompassSizer->OnWindowSize(size.x, size.y);
	}
}

void Enviro::SetHUDMessageText(const char *message)
{
	// The HUD always has a place to put status messages to the user.
	if (!m_pHUDMessage && m_pArial)
	{
		vtGeode *geode = new vtGeode;
		geode->setName("Message");
		m_pHUD->GetContainer()->addChild(geode);
		m_pHUDMessage = new vtTextMesh(m_pArial, 18);
		m_pHUDMessage->SetPosition(FPoint3(3,3,0));
		geode->AddTextMesh(m_pHUDMessage, 0);
	}
	if (m_pHUDMessage)
		m_pHUDMessage->SetText(message);
}

void Enviro::ShowVerticalLine(bool bShow)
{
	if (bShow)
		MakeVerticalLine();
	m_bShowVerticalLine = bShow;
}

bool Enviro::GetShowVerticalLine()
{
	return m_bShowVerticalLine;
}

void Enviro::MakeVerticalLine()
{
	// Share materials with the Arc
	SetupArcMaterials();

	// create geometry container, if needed
	if (!m_pVertLine)
	{
		m_pVertLine = new vtGeode;
		if (m_state == AS_Terrain)
			GetCurrentTerrain()->GetTopGroup()->addChild(m_pVertLine);
		m_pVertLine->SetMaterials(m_pArcMats);
	}
}

void Enviro::UpdateVerticalLine()
{
	MakeVerticalLine();
	m_pVertLine->RemoveAllMeshes();

	vtTerrain *pTerr = GetCurrentTerrain();
	LayerSet &layers = pTerr->GetLayers();

	FPoint3 world_pos;
	if (!m_pTerrainPicker->GetCurrentPoint(world_pos))
		return;

	std::vector<float> samples;
	std::vector<FPoint3> normals;
	float fMin = world_pos.y, fMax = world_pos.y;
	for (uint i = 0; i < layers.size(); i++)
	{
		if (layers[i]->GetType() == LT_ELEVATION)
		{
			vtElevLayer *pEL = dynamic_cast<vtElevLayer *>(layers[i].get());
			const vtTin *tin = pEL->GetTin();

			float fAlt;
			int triangle;
			const DPoint2 p2(m_EarthPos.x, m_EarthPos.y);
			if (tin->FindTriangleOnEarth(p2, fAlt, triangle, true))
			{
				samples.push_back(fAlt);

				FPoint3 normal = tin->GetTriangleNormal(triangle);
				normals.push_back(normal);

				if (fAlt < fMin) fMin = fAlt;
				if (fAlt > fMax) fMax = fAlt;
			}
		}
	}

	// One yellow bar going through all the points.
	vtMesh *mesh = new vtMesh(osg::PrimitiveSet::LINES, 0, 2);
	mesh->AddLine(FPoint3(world_pos.x, fMin, world_pos.z),
				  FPoint3(world_pos.x, fMax, world_pos.z));
	m_pVertLine->AddMesh(mesh, 0);	// yellow
	mesh->SetLineWidth(2);

	// A red cross at the surface of each elevation layer.
	mesh = new vtMesh(osg::PrimitiveSet::LINES, 0, samples.size() * 4);
	m_pVertLine->AddMesh(mesh, 1);	// red
	mesh->SetLineWidth(3);

	// Make them large enough to see at distance
	const float distance = (m_pNormalCamera->GetTrans() - world_pos).Length();
	float length = sqrt(distance);
	if (length < 3) length = 3.0f;

	for (uint j = 0; j < samples.size(); j++)
	{
		const FPoint3 &normal = normals[j];
		const FPoint3 right(1, 0, 0);
		FPoint3 v1 = normal.Cross(right);
		v1.Normalize();
		FPoint3 v2 = normal.Cross(v1);

		const FPoint3 p(world_pos.x, samples[j], world_pos.z);
		v1 *= length;
		v2 *= length;
		mesh->AddLine(p - v1, p + v1);
		mesh->AddLine(p - v2, p + v2);
	}
}

void Enviro::CreateElevationLegend()
{
	// Must have a color-mapped texture on the terrain to show a legend
	ColorMap *cmap = GetCurrentTerrain()->GetTextureColorMap();
	if (!cmap)
		return;

	// Define the size and shape of the legend: input values
	const int ticks = 8;
	const IPoint2 border(10, 18);
	const IPoint2 base(10, 10);
	const IPoint2 size(140, 230);
	const int fontsize = 16;

	// Derived values
	const IPoint2 in_base = base + border;
	const IPoint2 in_size(size.x - (border.x*2), size.y - (border.y*2));
	const int vert_space = in_size.y / (ticks-1);
	const int cbar_left = in_base.x + (in_size.x * 6 / 10);
	const int cbar_right = in_base.x + in_size.x;

	const int white = m_pHUDMaterials->AddRGBMaterial(RGBf(1, 1, 1), false, false); // white
	const int grey = m_pHUDMaterials->AddRGBMaterial(RGBf(.2, .2, .2), false, false); // dark grey

	m_pLegendGeom = new vtGeode;
	m_pLegendGeom->setName("Legend");
	m_pLegendGeom->SetMaterials(m_pHUDMaterials);

	// Solid rectangle behind it
	vtMesh *mesh4 = new vtMesh(osg::PrimitiveSet::QUADS, 0, 4);
	mesh4->AddRectangleXY((float) base.x, (float) base.y, (float) size.x, (float) size.y, -1.0f);
	m_pLegendGeom->AddMesh(mesh4, grey);

	float fMin, fMax;
	GetCurrentTerrain()->GetHeightField()->GetHeightExtents(fMin, fMax);

	// Big band of color
	cmap->GenerateColorTable(in_size.y, fMin, fMax);
	vtMesh *mesh1 = new vtMesh(osg::PrimitiveSet::TRIANGLE_STRIP, VT_Colors, (in_size.y + 1)*2);
	for (int i = 0; i < in_size.y + 1; i++)
	{
		const FPoint3 p1((float) cbar_left,  (float) in_base.y + i, 0.0f);
		const FPoint3 p2((float) cbar_right, (float) in_base.y + i, 0.0f);
		const int idx = mesh1->AddLine(p1, p2);
		mesh1->SetVtxColor(idx, (RGBf) cmap->m_table[i]);
		mesh1->SetVtxColor(idx+1, (RGBf) cmap->m_table[i]);
	}
	mesh1->AddStrip2((in_size.y + 1)*2, 0);
	m_pLegendGeom->AddMesh(mesh1, white);

	// Small white tick marks
	vtMesh *mesh2 = new vtMesh(osg::PrimitiveSet::LINES, 0, ticks*2);
	for (int i = 0; i < ticks; i++)
	{
		const FPoint3 p1((float) cbar_left-border.x*2, (float) in_base.y + i*vert_space, 0.0f);
		const FPoint3 p2((float) cbar_left,			   (float) in_base.y + i*vert_space, 0.0f);
		mesh2->AddLine(p1, p2);
	}
	m_pLegendGeom->AddMesh(mesh2, white);

	// Text labels
	for (int i = 0; i < ticks; i++)
	{
		vtTextMesh *mesh3 = new vtTextMesh(m_pArial, (float) fontsize, false);
		vtString str;
		str.Format("%4.1f", fMin + (fMax - fMin) / (ticks-1) * i);
		mesh3->SetText(str);
		const FPoint3 p1((float) in_base.x, (float) in_base.y + i*vert_space - (fontsize*1/3), 0.0f);
		mesh3->SetPosition(p1);

		m_pLegendGeom->AddTextMesh(mesh3, white);
	}

	m_pHUD->GetContainer()->addChild(m_pLegendGeom);
	m_bCreatedLegend = true;
}

void Enviro::CreateCompass()
{
	// Define the size and shape of the compass
	const IPoint2 base(310, 10);
	const IPoint2 size(128, 128);

	VTLOG1("Loading compass..\n");
	vtString path = FindFileOnPaths(vtGetDataPath(), "compass.png");
	if (path == "")
	{
		VTLOG1(" not found.\n");
		return;
	}

	osg::ref_ptr<vtImageSprite> CompassSprite = new vtImageSprite;
	bool success = CompassSprite->Create(path, true);	// blending = true
	if (!success)
		return;

	m_pCompassSizer = new vtSpriteSizer(CompassSprite, -133, -5, -5, -133);
	m_pCompassSizer->setName("Sizer for Compass");
	vtGetScene()->AddEngine(m_pCompassSizer);

	m_pCompassGeom = CompassSprite->GetGeode();
	m_pCompassGeom->setName("Compass");
	m_pHUD->GetContainer()->addChild(m_pCompassGeom);
	m_bCreatedCompass = true;
}

void Enviro::SetWindowBox(const IPoint2 &p1, const IPoint2 &p2)
{
	if (!m_pWindowBoxMesh)
	{
		// create a yellow wireframe polygon we can stretch later
		const int yellow = m_pHUDMaterials->AddRGBMaterial(RGBf(1,1,0), false, false, true);
		vtGeode *geode = new vtGeode;
		geode->setName("Selection Box");
		geode->SetMaterials(m_pHUDMaterials);
		m_pWindowBoxMesh = new vtMesh(osg::PrimitiveSet::POLYGON, 0, 4);
		m_pWindowBoxMesh->AddVertex(0,0,0);
		m_pWindowBoxMesh->AddVertex(1,0,0);
		m_pWindowBoxMesh->AddVertex(1,1,0);
		m_pWindowBoxMesh->AddVertex(0,1,0);
		m_pWindowBoxMesh->AddStrip2(4, 0);
		geode->AddMesh(m_pWindowBoxMesh, yellow);
		m_pHUD->GetContainer()->addChild(geode);
	}
	// Invert the coordinates Y, because mouse origin is upper left, and
	//  HUD origin is lower left
	const IPoint2 winsize = vtGetScene()->GetWindowSize();

	m_pWindowBoxMesh->SetVtxPos(0, FPoint3((float) p1.x, (float) winsize.y-1-p1.y, 0.0f));
	m_pWindowBoxMesh->SetVtxPos(1, FPoint3((float) p2.x, (float) winsize.y-1-p1.y, 0.0f));
	m_pWindowBoxMesh->SetVtxPos(2, FPoint3((float) p2.x, (float) winsize.y-1-p2.y, 0.0f));
	m_pWindowBoxMesh->SetVtxPos(3, FPoint3((float) p1.x, (float) winsize.y-1-p2.y, 0.0f));
	m_pWindowBoxMesh->ReOptimize();
}


////////////////////////////////////////////////////////////////////////
// Vehicles

CarEngine *Enviro::CreateGroundVehicle(const VehicleOptions &opt)
{
	// Create test vehicle
	vtTerrain *pTerr = GetCurrentTerrain();
	if (!pTerr)
		return NULL;

	DPoint3 epos;
	bool bOn = m_pTerrainPicker->GetCurrentEarthPos(epos);
	if (!bOn)
		return NULL;

	Vehicle *car = m_VehicleManager.CreateVehicle(opt.m_Itemname, opt.m_Color);
	if (!car)
		return NULL;
	pTerr->addNode(car);

	pTerr->PlantModelAtPoint(car, DPoint2(epos.x, epos.y));

	CarEnginePtr pEng = new CarEngine(car, pTerr->GetHeightField());
	if (!pEng->Valid())
		return NULL;

	// add the engine to the terrain's engines
	pTerr->AddEngine(pEng);

	// add the vehicle to the terrain's vehicles
	m_Vehicles.AddEngine(pEng);

	// notify framework
	AddVehicle(pEng);

	pEng->setName("drive");
	return pEng.get();
}

void Enviro::CreateSomeTestVehicles(vtTerrain *pTerrain)
{
	vtRoadMap3d *pRoadMap = pTerrain->GetRoadMap();

	// How many four-wheel land vehicles are there in the content catalog?
	vtStringArray vnames;
	vtContentManager3d &con = vtGetContent();
	for (uint i = 0; i < con.NumItems(); i++)
	{
		vtItem *item = con.GetItem(i);
		const char *type = item->GetValueString("type");
		int wheels = item->GetValueInt("num_wheels");
		if (type && vtString(type) == "ground vehicle" && wheels == 4)
		{
			vnames.push_back(item->m_name);
		}
	}
	const uint numv = vnames.size();

	// put one of each at the center of the terrain
	const DPoint2 center = pTerrain->GetHeightField()->GetEarthExtents().GetCenter();

	// add some test vehicles
	for (uint i = 0; i < numv; i++)
	{
		const RGBf color(1.0f, 1.0f, 1.0f);	// white

		// Create some of each land vehicle type
		Vehicle *car = m_VehicleManager.CreateVehicle(vnames[i], color);
		if (car)
		{
			pTerrain->addNode(car);
			pTerrain->PlantModelAtPoint(car, center + DPoint2(i*10, 0));

			CarEnginePtr pEng = new CarEngine(car, pTerrain->GetHeightField());
			if (!pEng->Valid())
				continue;

			pEng->setName(vtString("drive") + vnames[i]);

			// add the engine to the terrain's engines
			pTerrain->AddEngine(pEng);

			// add the vehicle to the terrain's vehicles
			m_Vehicles.AddEngine(pEng);

			// notify framework
			AddVehicle(pEng);
		}
	}
}


////////////////////////////////////////////////////////////////////////
// Import

class KMLVisitor : public XMLVisitor
{
public:
	KMLVisitor()
	{
		bInPlacemark = false;
	}
	void startElement(const char *name, const XMLAttributes &atts)
	{
		m_data = "";
		if (strcmp(name, "Placemark") == 0)
			bInPlacemark = true;
	}
	void endElement (const char *name)
	{
		if (strcmp(name, "Placemark") == 0)
			bInPlacemark = false;

		if (bInPlacemark)
		{
			const char *str = m_data.c_str();
			if (strcmp(name, "longitude") == 0)
				m_pos.x = atof(str);
			if (strcmp(name, "latitude") == 0)
				m_pos.y = atof(str);
			if (strcmp(name, "href") == 0)
				m_href = str;
		}
	}
	void data(const char *s, int length) { m_data.append(string(s, length)); }

	DPoint2 m_pos;
	bool bInPlacemark;
	vtString m_href;
protected:
	std::string m_data;
};

bool Enviro::ImportModelFromKML(const char *kmlfile)
{
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	VTLOG("Trying to import a model from KML file '%s'\n", kmlfile);

	KMLVisitor visitor;
	try
	{
		readXML(std::string(kmlfile), visitor);
	}
	catch (xh_io_exception &ex)
	{
		const string msg = ex.getFormattedMessage();
		VTLOG(" XML problem: %s\n", msg.c_str());
		return false;
	}

	// create a new Instance object
	vtTerrain *pTerr = GetCurrentTerrain();
	if (!pTerr)
		return false;

	// Ensure there is at least one structure layer, then use it
	pTerr->GetOrCreateLayerOfType(LT_STRUCTURE);
	vtStructureLayer *st_layer = pTerr->GetStructureLayer();
	vtStructInstance3d *inst = (vtStructInstance3d *) st_layer->AddNewInstance();

	const vtCRS &tcrs = pTerr->GetCRS();
	DPoint2 p = visitor.m_pos;
	if (tcrs.IsGeographic() == false)
	{
		// Must transform from KML's CRS (WGS84 geo) to the terrain's CRS
		vtCRS wgs84_geo;
		wgs84_geo.SetGeogCSFromDatum(EPSG_DATUM_WGS84);
		ScopedOCTransform trans(CreateCoordTransform(&wgs84_geo, &tcrs));
		if (!trans)
		{
			VTLOG1(" Couldn't transform coordinates\n");
			return false;
		}
		trans->Transform(1, &p.x, &p.y);
	}
	inst->SetPoint(p);

	// Beware the (common) case of a relative path, which will be relative
	//  to where the kml file was
	vtString pa = visitor.m_href;
	if (!PathIsAbsolute(pa))
	{
		vtString local = PathLevelUp(kmlfile);
		pa = local + "/" + pa;
	}

	inst->SetValueString("filename", pa, true);
	VTLOG("  Filename: '%s'\n", (const char *)pa);
	VTLOG("  at %.7g, %.7g: ", p.x, p.y);

	const int index = st_layer->size() - 1;
	bool success = pTerr->CreateStructure(st_layer, index);
	if (success)
	{
		VTLOG1(" succeeded.\n");
		st_layer->SetModified();
		RefreshLayerView();
	}
	else
	{
		// creation failed
		VTLOG1(" failed.\n");
		ShowMessage("Could not create instance.");
		inst->Select(true);
		st_layer->DeleteSelected();
		return false;
	}
	return true;
}


////////////////////////////////////////////////////////////////////////
// Abstract Layers

// Find an appropriate point layer for labels.
vtAbstractLayer *Enviro::GetLabelLayer() const
{
	const vtTerrain *pTerr = GetCurrentTerrain();
	if (!pTerr)
		return NULL;
	const LayerSet &layers = pTerr->GetLayers();
	for (uint i = 0; i < layers.size(); i++)
	{
		vtAbstractLayer *alay = dynamic_cast<vtAbstractLayer*>(layers[i].get());
		if (!alay)
			continue;
		if (alay->GetFeatureSet()->GetGeomType() == wkbPoint &&
			alay->GetFeatureSet()->GetField("Label") != NULL)
			return alay;
	}
	return NULL;
}

int Enviro::NumSelectedAbstractFeatures() const
{
	const vtTerrain *pTerr = GetCurrentTerrain();
	if (!pTerr)
		return 0;
	const LayerSet &layers = pTerr->GetLayers();
	int count = 0;
	for (uint i = 0; i < layers.size(); i++)
	{
		vtAbstractLayer *alay = dynamic_cast<vtAbstractLayer*>(layers[i].get());
		if (alay)
			count += alay->GetFeatureSet()->NumSelected();
	}
	return count;
}


////////////////////////////////////////////////////////////////////////

void ControlEngine::Eval()
{
	if (m_pEnvironment)
		m_pEnvironment->DoControl();
}

