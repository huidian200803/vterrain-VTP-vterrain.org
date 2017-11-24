//
// Earth View functionality of the Enviro class.
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/GeomFactory.h"
#include "vtlib/core/PickEngines.h"
#include "vtlib/core/Globe.h"
#include "vtlib/core/SkyDome.h"
#include "vtlib/core/Terrain.h"
#include "vtdata/DataPath.h"
#include "vtdata/vtLog.h"

#include "Enviro.h"
#include "Engines.h"
#include "Options.h"

#define SPACE_DARKNESS		0.0f
#define UNFOLD_SPEED		0.01f

// these define the vertex count and height offset of the lon-lat lines
#define LL_COUNT	640
#define LL_RADIUS	1.002

// Although there is no string translation in the core of Enviro (because it
//  is independent of wx or any GUI library) nonetheless we want the text
//  messages to be found by the gettext utility, so we need to enclose
//  anything to be translated in _()
#define _(x) x


void Enviro::FlyToSpace()
{
	VTLOG("FlyToSpace\n");
	if (m_state == AS_Terrain)
	{
		// Make sure the normal camera is active
		SetTopDown(false);

		// remember camera position
		vtTerrain *pT = GetCurrentTerrain();
		vtCamera *pCam = vtGetScene()->GetCamera();
		FMatrix4 mat;
		pCam->GetTransform(mat);
		pT->SetCamLocation(mat);
	}

	// turn off terrain, if any
	SwitchToTerrain(NULL);
	EnableFlyerEngine(false);

	SetState(AS_MovingOut);
	m_iInitStep = 0;
	FreeArc();

	// Layer view needs to stop showing terrain layers
	RefreshLayerView();

	// Inform the GUI that there is no terrain
	SetTerrainToGUI(NULL);
}

void Enviro::SetupGlobe()
{
	VTLOG("SetupGlobe step %d\n", m_iInitStep);

	if (m_iInitStep == 1)
	{
		m_pTerrainPicker->SetEnabled(false);
		SetMessage(_("Creating Globe"));
	}
	if (m_iInitStep == 2)
	{
		if (m_pGlobeContainer == NULL)
		{
			MakeGlobe();
			m_SpaceTrackballState[0].Set(0,0,INITIAL_SPACE_DIST);
			m_SpaceTrackballState[1].Set(0,0,0);
			m_SpaceTrackballState[2].Set(0,0,0);
			m_pRoot->addChild(m_pGlobeContainer);
		}
		SetMessage(_("Switching to Globe"));
	}
	if (m_iInitStep == 3)
	{
		// put the light where the sun should be
		vtTransform *pSunLight = GetSunLightTransform();
		vtLightSource *pLightSource = GetSunLightSource();

		pSunLight->Identity();
		pSunLight->SetTrans(FPoint3(0, 0, -5));

		if (pLightSource)
		{
			// standard bright sunlight
			pLightSource->SetDiffuse(RGBf(3, 3, 3));
			pLightSource->SetAmbient(RGBf(0.5f, 0.5f, 0.5f));
		}
		vtGetScene()->SetBgColor(RGBf(SPACE_DARKNESS, SPACE_DARKNESS, SPACE_DARKNESS));

		m_pGlobeContainer->SetEnabled(true);
		m_pCursorMGeom->Identity();
		m_pCursorMGeom->Scale(.1f);
	}
	if (m_iInitStep == 4)
	{
		vtCamera *pCam = vtGetScene()->GetCamera();
		pCam->SetHither(0.01f);
		pCam->SetYon(50.0f);
		pCam->SetFOV(60 * (PIf / 180.0f));
	}
	if (m_iInitStep == 5)
	{
		SetEarthShading(false);
	}
	if (m_iInitStep == 6)
	{
		m_pTrackball->SetState(m_SpaceTrackballState);
		m_pTrackball->SetEnabled(true);
	}
	if (m_iInitStep == 7)
	{
		SetState(AS_Orbit);
		SetMode(MM_SELECT);
		SetMessage(_("Earth View"), "", 10);	// Show for 10 seconds
		m_pGlobePicker->SetEnabled(true);

		// Layer view needs to update
		RefreshLayerView();

		// Let the GUI know
		SetTimeEngineToGUI(m_pGlobeTime);
	}
	VTLOG("SetupGlobe step %d finished\n", m_iInitStep);
}

// Helper
void GeomAddRectMesh(vtGeode *pGeode, const FRECT &rect, float z, int matidx)
{
	vtMesh *mesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_FAN, VT_TexCoords, 4);
	mesh->AddVertexUV(FPoint3(rect.left, rect.bottom, z), 0, 0);
	mesh->AddVertexUV(FPoint3(rect.right, rect.bottom, z), 1, 0);
	mesh->AddVertexUV(FPoint3(rect.right, rect.top, z), 1, 1);
	mesh->AddVertexUV(FPoint3(rect.left, rect.top, z), 0, 1);
	mesh->AddFan(0, 1, 2, 3);
	pGeode->AddMesh(mesh, matidx);
}


//
// Create the earth globe
//
void Enviro::MakeGlobe()
{
	VTLOG1("MakeGlobe\n");

	m_pGlobeTime = new vtTimeEngine;
	m_pGlobeTime->setName("GlobeTime");
	vtGetScene()->AddEngine(m_pGlobeTime);

	m_pGlobeContainer = new vtGroup;
	m_pGlobeContainer->setName("Globe Container");

	// simple globe
//	m_pGlobeXForm = CreateSimpleEarth(g_Options.m_DataPaths);

	// fancy icosahedral globe
	m_pIcoGlobe = new vtIcoGlobe;
	m_pIcoGlobe->Create(5000, g_Options.m_strEarthImage,
//		vtIcoGlobe::GEODESIC);
//		vtIcoGlobe::RIGHT_TRIANGLE);
		vtIcoGlobe::DYMAX_UNFOLD);
//		vtIcoGlobe::INDEPENDENT_GEODESIC);
	m_pGlobeContainer->addChild(m_pIcoGlobe->GetTop());
	m_pGlobeTime->AddTarget(m_pIcoGlobe);
	m_pDemoGroup = NULL;

	// pass the time along once to orient the earth
	m_pIcoGlobe->SetTime(m_pGlobeTime->GetTime());

	// use a trackball engine for navigation
	//
	VTLOG("\tcreating Trackball\n");
	m_pTrackball = new vtTrackball(INITIAL_SPACE_DIST);
	m_pTrackball->setName("Trackball2");
	m_pTrackball->AddTarget(vtGetScene()->GetCamera());
	m_pTrackball->SetRotateButton(VT_RIGHT, 0, false);
	m_pTrackball->SetZoomButton(VT_RIGHT, VT_SHIFT);
	vtGetScene()->AddEngine(m_pTrackball);

	// stop them from going in too far (they'd see through the earth)
	m_pTrackball->LimitPos(FPoint3(-1E9,-1E9,1.01f), FPoint3(1E9,1E9,1E9));

	// determine where the terrains are, and show them as red rectangles
	//
	LookUpTerrainLocations();
	VTLOG("AddTerrainRectangles\n");
	m_pIcoGlobe->AddTerrainRectangles(vtGetTS());

	// create the GlobePicker engine for picking features on the earth
	//
	m_pGlobePicker = new GlobePicker;
	m_pGlobePicker->setName("GlobePicker");
	m_pGlobePicker->SetGlobe(m_pIcoGlobe);
	vtGetScene()->AddEngine(m_pGlobePicker);
	m_pGlobePicker->AddTarget(m_pCursorMGeom);
	m_pGlobePicker->SetRadius(1.0);
	m_pGlobePicker->SetEnabled(false);

	// create some stars around the earth
	//
	vtStarDome *pStars = new vtStarDome;
	vtString bsc_file = FindFileOnPaths(vtGetDataPath(), "Sky/bsc.data");
	if (bsc_file != "")
	{
		pStars->Create(bsc_file, 5.0f);	// brightness
		vtTransform *pScale = new vtTransform;
		pScale->setName("Star Scaling Transform");
		pScale->Scale(20);
		m_pGlobeContainer->addChild(pScale);
		pScale->addChild(pStars);
	}

	// create some geometry showing various astronomical axes
	vtMaterialArray *pMats = new vtMaterialArray;
	int yellow = pMats->AddRGBMaterial(RGBf(1,1,0), false, false);
	int red = pMats->AddRGBMaterial(RGBf(1,0,0), false, false);
	int green = pMats->AddRGBMaterial(RGBf(0,1,0), false, false);

	m_pSpaceAxes = new vtGeode;
	m_pSpaceAxes->setName("Earth Axes");
	m_pSpaceAxes->SetMaterials(pMats);

	vtMesh *mesh = new vtMesh(osg::PrimitiveSet::LINES, 0, 6);
	mesh->AddLine(FPoint3(0,0,200), FPoint3(0,0,0));
	mesh->AddLine(FPoint3(0,0,1),   FPoint3(-.07f,0,1.1f));
	mesh->AddLine(FPoint3(0,0,1),   FPoint3( .07f,0,1.1f));
	m_pSpaceAxes->AddMesh(mesh, yellow);

	mesh = new vtMesh(osg::PrimitiveSet::LINES, 0, 6);
	mesh->AddLine(FPoint3(1.5f,0,0), FPoint3(-1.5f,0,0));
	mesh->AddLine(FPoint3(-1.5f,0,0), FPoint3(-1.4f, 0.07f,0));
	mesh->AddLine(FPoint3(-1.5f,0,0), FPoint3(-1.4f,-0.07f,0));
	m_pSpaceAxes->AddMesh(mesh, green);

	mesh = new vtMesh(osg::PrimitiveSet::LINES, 0, 6);
	mesh->AddLine(FPoint3(0,2,0), FPoint3(0,-2,0));
	m_pSpaceAxes->AddMesh(mesh, red);

	m_pGlobeContainer->addChild(m_pSpaceAxes);
	m_pSpaceAxes->SetEnabled(false);

	// Lon-lat cursor lines
	m_pEarthLines = new vtGeode;
	m_pEarthLines->setName("Earth Lines");
	int orange = pMats->AddRGBMaterial(RGBf(1,.7,1), false, false, true, 0.6);
	m_pEarthLines->SetMaterials(pMats);

	m_pLineMesh = new vtMesh(osg::PrimitiveSet::LINE_STRIP, 0, 6);
	for (int i = 0; i < LL_COUNT*3; i++)
		m_pLineMesh->AddVertex(FPoint3(0,0,0));
	m_pLineMesh->AddStrip2(LL_COUNT*2, 0);
	m_pLineMesh->AddStrip2(LL_COUNT, LL_COUNT*2);
	m_pLineMesh->AllowOptimize(false);

	m_pEarthLines->AddMesh(m_pLineMesh, orange);
	m_pIcoGlobe->GetSurface()->addChild(m_pEarthLines);
	m_pEarthLines->SetEnabled(false);

	double lon = 14.1;
	double lat = 37.5;
	SetEarthLines(lon, lat);
}

void Enviro::MakeDemoGlobe()
{
	m_pDemoGroup = new vtGroup;
	m_pDemoGroup->setName("DemoGroup");
	vtIcoGlobe *Globe2 = new vtIcoGlobe;
	Globe2->Create(1000, vtString(""), vtIcoGlobe::GEODESIC);
	//Globe2->SetInflation(1.0f);
	vtTransform *trans = new vtTransform;
	trans->setName("2nd Globe Scaler");
	m_pGlobeContainer->addChild(m_pDemoGroup);
	m_pDemoGroup->addChild(trans);
	trans->addChild(Globe2->GetTop());
	trans->Scale(1.006f);
	m_pGlobeTime->AddTarget((vtTimeTarget *)Globe2);

	// Planetwork globe is around 3 PM GMT, summer over the north atlantic
	vtTime time;
	time.SetDate(2000, 6, 20);
	time.SetTimeOfDay(15,0,0);
	m_pGlobeTime->SetTime(time);

	vtGeode *geode = new vtGeode;
	vtMaterialArray *mats = new vtMaterialArray;
	mats->AddTextureMaterial(LoadOsgImage("Planetwork/logo3.png"), false, false, true);
	mats->AddTextureMaterial(LoadOsgImage("Planetwork/logo2.png"), false, false, true);
	geode->SetMaterials(mats);

	float width = 1.9, height = .22;
	FRECT rect(-width/2, height/2, width/2, -height/2);
	GeomAddRectMesh(geode, rect, 1.15, 0);
	rect += FPoint2(0.01, -0.01);
	GeomAddRectMesh(geode, rect, 1.14, 1);
	m_pDemoGroup->addChild(geode);
	Globe2->SetTime(m_pGlobeTime->GetTime());

	int i;
	vtMaterialArray *rainbow = new vtMaterialArray;
	bool bLighting = false;
	rainbow->AddRGBMaterial(RGBf(0.5,0,0),		false, bLighting, false, 0.5f);
	rainbow->AddRGBMaterial(RGBf(0.5,0.5,0),	false, bLighting, false, 0.5f);
	rainbow->AddRGBMaterial(RGBf(0,0.5,0),		false, bLighting, false, 0.5f);
	rainbow->AddRGBMaterial(RGBf(0,0.5,0.5),	false, bLighting, false, 0.5f);
	rainbow->AddRGBMaterial(RGBf(0,0,0.5),		false, bLighting, false, 0.5f);
	rainbow->AddRGBMaterial(RGBf(0.5,0,0.5),	false, bLighting, false, 0.5f);
	for (i = 0; i < 6; i++)
	{
		vtMaterial *mat = rainbow->at(i);
		mat->SetTransparent(true, true);
	}
	m_pDemoTrails = new vtGeode;
	m_pDemoTrails->setName("Trails");
	m_pDemoTrails->SetMaterials(rainbow);
	Globe2->GetTop()->addChild(m_pDemoTrails);

	vtString users = FindFileOnPaths(vtGetDataPath(), "PointData/vtp-users-040129.shp");
	if (users != "")
	{
		vtFeatureLoader loader;
		vtFeatureSet *feat1 = loader.LoadFromSHP(users);
		vtFeatureSetPoint2D *ft = (vtFeatureSetPoint2D *) feat1;

		int half = ft->NumEntities() / 2;
		int foo = 0;
		for (i = 0; i < half; i++)
		{
			DPoint2 p1 = ft->GetPoint(i);
			DPoint2 p2 = ft->GetPoint(i+half);
			if (p1 == DPoint2(0,0) || p2 == DPoint2(0,0))
				continue;
			if (p1.y > 0 && p2.y > 0)
			{
				foo++;
				if ((foo%20)<19)
					continue;
			}
			if (p1.y < 0 && p2.y < 0)
				continue;

			vtGeomFactory mf(m_pDemoTrails, osg::PrimitiveSet::LINE_STRIP, 0, 7000, i%6);
			Globe2->AddSurfaceLineToMesh(&mf, p1, p2);
		}
		delete feat1;
	}

	// Stark lighting, no ambient
	vtLightSource *pLight = GetSunLightSource();
	if (pLight)
	{
		pLight->SetDiffuse(RGBf(1, 1, 1));
		pLight->SetAmbient(RGBf(0, 0, 0));
	}
	SetEarthShading(true);
}

//
// Create the earth globe
//
void Enviro::MakeOverlayGlobe(vtImage *input, bool progress_callback(int))
{
	VTLOG1("MakeOverlayGlobe\n");

	// Make dymaxion overlay images
	vtImage *output[10];

	const IPoint2 input_size = input->GetSize();
	int depth = input->GetDepth();
	int output_size = 512;

	DymaxIcosa ico;

	uchar value;
	RGBi rgb;
	RGBAi rgba;
	double u, v;
	double lon, lat;
	DPoint3 uvw;
	for (int i = 0; i < 10; i++)
	{
		output[i] = new vtImage;
		output[i]->Allocate(IPoint2(output_size, output_size), depth);

		if (progress_callback != NULL)
			progress_callback((i+1)*10);

		for (int x = 0; x < output_size; x++)
		{
			for (int y = 0; y < output_size; y++)
			{
				int face;
				if (y < output_size-1-x)
				{
					face = icosa_face_pairs[i][0];
					u = (double)x / output_size;
					v = (double)y / output_size;
				}
				else
				{
					face = icosa_face_pairs[i][1];
					u = (double)(output_size-1-x) / output_size;
					v = (double)(output_size-1-y) / output_size;
				}
				uvw.x = u;
				uvw.y = v;
				ico.FaceUVToGeo(face, uvw, lon, lat);

				int source_x = (int) (lon / PI2d * input_size.x);
				int source_y = (int) (lat / PId * input_size.y);

				if (depth == 8)
				{
					value = input->GetPixel8(source_x, source_y);
					output[i]->SetPixel8(x, output_size-1-y, value);
				}
				else if (depth == 24)
				{
					input->GetPixel24(source_x, source_y, rgb);
					output[i]->SetPixel24(x, output_size-1-y, rgb);
				}
				else if (depth == 32)
				{
					input->GetPixel32(source_x, source_y, rgba);
					output[i]->SetPixel32(x, output_size-1-y, rgba);
				}
			}
		}
	}


	// fancy icosahedral globe
	m_pOverlayGlobe = new vtIcoGlobe;
	m_pOverlayGlobe->Create(5000, output,
//		vtIcoGlobe::GEODESIC);
//		vtIcoGlobe::RIGHT_TRIANGLE);
		vtIcoGlobe::DYMAX_UNFOLD);
//		vtIcoGlobe::INDEPENDENT_GEODESIC);

	vtTransform *trans = new vtTransform;
	trans->setName("Overlay Globe Scaler");
	trans->Scale(1.005f);

	m_pIcoGlobe->GetTop()->addChild(trans);
	trans->addChild(m_pOverlayGlobe->GetTop());
}

void Enviro::SetEarthLines(double lon, double lat)
{
	int i;
	DPoint2 p;
	FPoint3 p3;

	// lat line
	p.y = lat;
	for (i = 0; i < LL_COUNT+LL_COUNT; i++)
	{
		p.x = (double)i / (LL_COUNT+LL_COUNT-1) * 360;
		geo_to_xyz(LL_RADIUS, p, p3);
		m_pLineMesh->SetVtxPos(i, p3);
	}
	// lon line
	p.x = lon;
	for (i = 0; i < LL_COUNT; i++)
	{
		p.y = -90 + (double)i / (LL_COUNT-1) * 180;
		geo_to_xyz(LL_RADIUS, p, p3);
		m_pLineMesh->SetVtxPos(LL_COUNT+LL_COUNT+i, p3);
	}
}

void Enviro::ShowEarthLines(bool bShow)
{
	if (m_pEarthLines)
		m_pEarthLines->SetEnabled(bShow);
}

void Enviro::SetSpaceAxes(bool bShow)
{
	if (m_state == AS_Orbit)
	{
		if (m_pSpaceAxes)
			m_pSpaceAxes->SetEnabled(bShow);
		if (m_pIcoGlobe)
			m_pIcoGlobe->ShowAxis(bShow);
	}
	else if (m_state == AS_Terrain)
	{
		vtSkyDome *sky = vtGetTS()->GetSkyDome();
		if (sky)
			sky->ShowMarkers(bShow);
	}
}

bool Enviro::GetSpaceAxes()
{
	if (m_state == AS_Orbit)
	{
		if (m_pSpaceAxes)
			return m_pSpaceAxes->GetEnabled();
	}
	else if (m_state == AS_Terrain)
	{
		vtSkyDome *sky = vtGetTS()->GetSkyDome();
		if (sky)
			return sky->MarkersShown();
	}
	return false;
}

void Enviro::LookUpTerrainLocations()
{
	VTLOG("LookUpTerrainLocations\n");

	// look up the earth location of each known terrain
	for (uint i = 0; i < NumTerrains(); i++)
	{
		vtTerrain *pTerr = GetTerrain(i);
		VTLOG("looking up: %s\n  ", (const char *) pTerr->GetName());

		bool success = pTerr->GetGeoExtentsFromMetadata();
		if (success)
		{
			DPoint2 nw, se;
			nw = pTerr->m_Corners_geo[1];
			se = pTerr->m_Corners_geo[3];
			VTLOG("\t\t(%.2lf,%.2lf) - (%.2lf,%.2lf)\n", nw.x, nw.y, se.x, se.y);
		}
		else
		{
			VTLOG("\t\tCouldn't determine terrain corners.\n");
			continue;
		}
	}
	VTLOG("\tLookUpTerrainLocations: done\n");
}

int Enviro::AddGlobeAbstractLayer(const char *fname)
{
	// Size: 0.0015f  works OK for the VTP recipients, 0.0005f for GeoURL s size works OK for the VTP recipients
	int num_added = m_pIcoGlobe->AddGlobeFeatures(fname, 0.003f);
	if (num_added != -1)
		RefreshLayerView();
	return num_added;
}

void Enviro::RemoveGlobeAbstractLayer(GlobeLayer *glay)
{
	m_pIcoGlobe->RemoveLayer(glay);
}

void Enviro::DoControlOrbit()
{
	if (m_fFlattenDir != 0.0f)
	{
		m_fFlattening += m_fFlattenDir;
		if (m_fFlattenDir > 0.0f && m_fFlattening > 1.0f)
		{
			m_fFlattening = 1.0f;
			m_fFlattenDir = 0.0f;
			m_bGlobeFlat = false;
		}
		if (m_fFlattenDir < 0.0f && m_fFlattening < 0.0f)
		{
			m_fFlattening = 0.0f;
			m_fFlattenDir = 0.0f;
			m_bGlobeFlat = true;
		}
		m_pIcoGlobe->SetInflation(m_fFlattening);
	}
	if (m_fFoldDir != 0.0f)
	{
		m_fFolding += m_fFoldDir;
		if (m_fFoldDir > 0.0f && m_fFolding > 1.0f)
		{
			// all the way flattened
			m_pFlatFlyer->SetEnabled(true);

			m_fFolding = 1.0f;
			m_fFoldDir = 0.0f;
			m_bGlobeUnfolded = true;
			m_bGlobeFlat = true;
		}
		if (m_fFoldDir < 0.0f && m_fFolding < 0.0f)
		{
			m_fFolding = 0.0f;
			m_fFoldDir = 0.0f;
			m_bGlobeUnfolded = false;
			m_bGlobeFlat = false;

			// Leave Flat View
			m_pTrackball->SetEnabled(true);

			// Turn globe culling back on
			m_pIcoGlobe->SetCulling(true);
		}
		m_pIcoGlobe->SetUnfolding(m_fFolding);

		// deflate as we unfold
		m_fFlattening = 1.0f - m_fFolding;
		m_pIcoGlobe->SetInflation(m_fFlattening);

		FPQ pq;
		pq.Interpolate(m_SpaceLoc, m_FlatLoc, m_fFolding);
		FMatrix4 m4;
		pq.ToMatrix(m4);
		m_pNormalCamera->SetTransform(m4);
	}

	if (m_bOnTerrain)
	{
		// Attempt to scale the 3d cursor, rather than keeping it the
		//  same size in world space (it would be too small in the distance)
		float fAltitude = m_pTrackball->GetRadius() - 1;
		m_pGlobePicker->SetTargetScale(fAltitude * 0.06f);
	}
}


//
// Check the terrain under the 3D cursor (for Earth View).
//
void Enviro::DoCursorOnEarth()
{
	m_bOnTerrain = false;
	DPoint3 earthpos;

	if (m_pGlobePicker != NULL)
		m_bOnTerrain = m_pGlobePicker->GetCurrentEarthPos(earthpos);
	if (m_bOnTerrain)
	{
		m_EarthPos = earthpos;

		// Update Earth Lines
		SetEarthLines(m_EarthPos.x, m_EarthPos.y);
	}
	vtString str1, str2;
	FormatCoordString(str1, m_EarthPos, LU_DEGREES);
	str2 = "Cursor ";
	str2 += str1;

	// The ideal relation, between trackball radius (height above earth) and
	//  rotation speed, is probably a curve, but here we fit it with a simple
	//  line, slope determined empirically
	float radius = m_pTrackball->GetRadius();
	IPoint2 winsize = vtGetScene()->GetWindowSize();
	m_pTrackball->SetRotScale((radius-1.0f) / winsize.x * 280);
}


vtTerrain *Enviro::FindTerrainOnEarth(const DPoint2 &p)
{
	vtTerrain *t, *smallest = NULL;
	double diag, smallest_diag = 1E7;

	for (uint i = 0; i < NumTerrains(); i++)
	{
		t = GetTerrain(i);
		if (t->m_Corners_geo.ContainsPoint(p))
		{
			// normally, doing comparison on latlon coordinates wouldn't be
			// meaningful, but in this case we know that the two areas compared
			// are overlapping and therefore numerically similar
			diag = (t->m_Corners_geo[2] - t->m_Corners_geo[0]).Length();
			if (diag < smallest_diag)
			{
				smallest_diag = diag;
				smallest = t;
			}
		}
	}
	return smallest;
}

void Enviro::OnMouseLeftDownOrbit(vtMouseEvent &event)
{
	// from orbit, check if we've clicked on a terrain
	if (!m_bOnTerrain)
		return;
	if (m_mode == MM_SELECT)
	{
		VTLOG("Calling FindTerrainOnEarth(%lf, %lf)\n", m_EarthPos.x, m_EarthPos.y);
		vtTerrain *pTerr = FindTerrainOnEarth(DPoint2(m_EarthPos.x, m_EarthPos.y));
		if (pTerr)
			RequestTerrain(pTerr);
	}
	if (m_mode == MM_MEASURE)
	{
		m_bDragging = true;
		if (m_bMeasurePath)
		{
			DPoint2 g1(m_EarthPos.x, m_EarthPos.y);
			// Path mode - set initial segment
			int len = m_distance_path.GetSize();
			if (len == 0)
			{
				// begin new path
				m_EarthPosDown = m_EarthPos;
				m_distance_path.Append(g1);
			}
			// default: add point to the path
			m_distance_path.Append(g1);
		}
		else
		{
			m_EarthPosDown = m_EarthPos;
			UpdateEarthArc();
		}
	}
}

bool Enviro::GetEarthShading()
{
	return m_bEarthShade;
}

void Enviro::SetEarthShading(bool bShade)
{
	m_bEarthShade = bShade;

	vtTransform *pMovableLight = GetSunLightTransform();

	pMovableLight->SetEnabled(bShade);
	m_pIcoGlobe->SetLighting(bShade);
	if (m_pOverlayGlobe)
		m_pOverlayGlobe->SetLighting(bShade);
}

void Enviro::SetEarthShape(bool bFlat)
{
	if (m_bGlobeFlat)
		m_fFlattenDir = 0.03f;
	else
		m_fFlattenDir = -0.03f;
}

void Enviro::SetEarthTilt(bool bTilt)
{
	if (!m_pIcoGlobe)
		return;
	m_pIcoGlobe->SetSeasonalTilt(bTilt);

	// remind the earth of the time/date to refresh orientation
	m_pIcoGlobe->SetTime(m_pGlobeTime->GetTime());
}

bool Enviro::GetEarthTilt()
{
	if (!m_pIcoGlobe)
		return false;
	return m_pIcoGlobe->GetSeasonalTilt();
}

void Enviro::SetEarthUnfold(bool bUnfold)
{
	if (!m_bGlobeUnfolded)
	{
		// Enter Flat View
		FMatrix4 m4;
		m_pNormalCamera->GetTransform(m4);
		m_SpaceLoc.FromMatrix(m4);

		m_FlatLoc.p.Set(0.85f,-0.75f,5.6);
		m_FlatLoc.q.Init();

//		m_pNormalCamera->SetTrans(m_FlatPos);
		m_pNormalCamera->PointTowards(FPoint3(0.85f,-0.75f,0));
		m_pTrackball->SetEnabled(false);

		// turn off axes
		SetSpaceAxes(false);

		// turn off globe shading and culling
		m_pIcoGlobe->SetCulling(false);
		SetEarthShading(false);

		m_fFoldDir = UNFOLD_SPEED;
	}
	else
	{
		if (m_fFolding == 1.0f)
			m_FlatLoc.p = m_pNormalCamera->GetTrans();
		m_fFoldDir = -UNFOLD_SPEED;

		m_pFlatFlyer->SetEnabled(false);
	}
}

void Enviro::UpdateEarthArc()
{
//	VTLOG("UpdateEarthArc %.1lf %.1lf,  %.1lf %.1lf\n", m_EarthPosDown.x, m_EarthPosDown.y, m_EarthPos.x, m_EarthPos.y);
	if (m_bMeasurePath)
	{
		DPoint2 g2(m_EarthPos.x, m_EarthPos.y);
		uint len = m_distance_path.GetSize();
		if (len > 1)
			m_distance_path[len-1] = g2;

		SetDisplayedArc(m_distance_path);
		ShowDistance(m_distance_path, FLT_MIN, FLT_MIN);
	}
	else
	{
		DPoint2 epos1(m_EarthPosDown.x, m_EarthPosDown.y);
		DPoint2 epos2(m_EarthPos.x, m_EarthPos.y);
		SetDisplayedArc(epos1, epos2);
		ShowDistance(epos1, epos2, FLT_MIN, FLT_MIN);
	}
}

void Enviro::SetDisplayedArc(const DPoint2 &g1, const DPoint2 &g2)
{
	SetupArcMesh();

	vtGeomFactory mf(m_pArc, osg::PrimitiveSet::LINE_STRIP, 0, 30000, 0);
	double angle = m_pIcoGlobe->AddSurfaceLineToMesh(&mf, g1, g2);

	// estimate horizontal distance (angle * radius)
	m_fArcLength = angle * EARTH_RADIUS;
}

void Enviro::SetDisplayedArc(const DLine2 &path)
{
	SetupArcMesh();

	vtGeomFactory mf(m_pArc, osg::PrimitiveSet::LINE_STRIP, 0, 30000, 0);
	double angle = m_pIcoGlobe->AddSurfaceLineToMesh(&mf, path);

	// estimate horizontal distance (angle * radius)
	m_fArcLength = angle * EARTH_RADIUS;
}

void Enviro::DescribeCoordinatesEarth(vtString &str1, vtString &str2)
{
	// give location of 3D cursor
	str1 = _("Cursor: ");

	DPoint3 epos;
	m_pGlobePicker->GetCurrentEarthPos(epos);

	FormatCoordString(str2, epos, LU_DEGREES);
}

// this was a quick hack for the PW conference.  if we ever need a real
// 'logo' functionality, it should be re-done.
void Enviro::ToggleDemo()
{
	if (!m_pDemoGroup) return;
	static int st = 1;

	st++;
	if (st == 3)
		st = 0;
	if (st == 0)
	{
		m_pDemoGroup->SetEnabled(false);
		m_pDemoTrails->SetEnabled(false);
	}
	if (st == 1)
	{
		m_pDemoGroup->SetEnabled(true);
		m_pDemoTrails->SetEnabled(false);
	}
	if (st == 2)
	{
		m_pDemoGroup->SetEnabled(true);
		m_pDemoTrails->SetEnabled(true);
	}
}

//
// For flying in from earth to terrain, we need to switch between them at some
//  point, for example, 50 km above the ground
//
#define MINIMUM_TRANSITION_HEIGHT_ABOVE_EARTH	50000

void Enviro::StartFlyIn()
{
	SetState(AS_FlyingIn);
	m_iFlightStage = 1;
	m_iFlightStep = 0;

	// Decide how high above the earth we should be at the transition point.
	//  This is largely due to the horizontal extent of the terrain.
	DPoint2 nw, se;
	nw = m_pTargetTerrain->m_Corners_geo[1];
	se = m_pTargetTerrain->m_Corners_geo[3];

	// A bit of trigonometry
	double diff_latitude = nw.y - se.y;
	double diff_meters = diff_latitude * METERS_PER_LATITUDE;
	double fov_y = m_pNormalCamera->GetVertFOV();
	m_fTransitionHeight = (float) ((diff_meters/2) / tan(fov_y / 2));

	// We should account for the terrain's height here - zooming into the
	//  Himalaya is significantly different - but we don't have that height
	//  because the terrain (generally) isn't loaded yet.

	if (m_fTransitionHeight < MINIMUM_TRANSITION_HEIGHT_ABOVE_EARTH)
		m_fTransitionHeight = MINIMUM_TRANSITION_HEIGHT_ABOVE_EARTH;

	// Determine how to spin the trackball to put us directly over the terrain
	m_pTrackball->GetState(m_TrackStart);

	m_FlyInCenter = (nw + se) / 2;

	// Account for offset between trackball and longitude
	DPoint2 center = m_FlyInCenter;
	center.x = 270.0 - center.x;
		if (center.x > 360.0) center.x -= 360.0;
		if (center.x < 0) center.x += 360.0;

	// Account for the earth's rotation
	FQuat rot = m_pIcoGlobe->GetRotation().Inverse();

	FPoint3 p1, p2;
	DPoint3 center3d;

	geo_to_xyz(1.0, center, p1);
	FMatrix3 mat;
	rot.GetMatrix(mat);
	mat.Transform(p1, p2);
	xyz_to_geo(1.0, p2, center3d);

		if (center3d.x > 360.0) center3d.x -= 360.0;
		if (center3d.x < 0) center3d.x += 360.0;

	FPoint3 TrackPosEnd;
	TrackPosEnd.x = (float) (center3d.x / 180 * PIf);
	TrackPosEnd.y = (float) (center3d.y / 180 * PIf);
	TrackPosEnd.z = 1.0f + (m_fTransitionHeight  / EARTH_RADIUS);
	m_TrackPosDiff = TrackPosEnd - m_TrackStart[0];

	// Hide the earth cursor on our way in
	m_pGlobePicker->SetEnabled(false);
	m_pCursorMGeom->SetEnabled(false);
}

void Enviro::FlyInStage1()
{
	m_iFlightStep++;
	FPoint3 curtrack[3];
	curtrack[0] = m_TrackStart[0] +
		(m_TrackPosDiff * (float)m_iFlightStep / 100);
	curtrack[1] = m_TrackStart[1];
	curtrack[2] = m_TrackStart[2];
	m_pTrackball->SetState(curtrack);

	if (m_iFlightStep == 100)
	{
		// Done with stage 1, prepare stage 2
		m_bFlyIn = false;
		m_iFlightStage = 2;
		m_iFlightStep = 0;

		m_FlyInAnim.Clear();

		// Set special high initial camera location to match where the
		//  trackball left us in earth view
		DPoint3 earth_geo(m_FlyInCenter.x, m_FlyInCenter.y,
			m_fTransitionHeight);

		const vtCRS &tcrs = m_pTargetTerrain->GetCRS();
		vtCRS gcrs;
		CreateSimilarGeographicCRS(tcrs, gcrs);
		ScopedOCTransform trans(CreateCoordTransform(&gcrs, &tcrs));
		DPoint3 earth_local = earth_geo;
		trans->Transform(1, &earth_local.x, &earth_local.y);

		const LocalCS &conv = m_pTargetTerrain->GetLocalCS();
		FPoint3 world;
		conv.EarthToLocal(earth_local, world);

		FPQ Flight2Start, Flight2End;
		Flight2Start.p = world;
		Flight2Start.q.AxisAngle(FPoint3(1,0,0), -PID2f);

		ControlPoint cp1(Flight2Start.p, Flight2Start.q);
		m_FlyInAnim.Insert(0.0, cp1);

		// End is the terrain's initial/current location
		FMatrix4 mat = m_pTargetTerrain->GetCamLocation();
		Flight2End.FromMatrix(mat);

		FPoint3 fall = Flight2Start.p - Flight2End.p;
		ControlPoint cp2(Flight2End.p + (fall*0.2), Flight2Start.q);
		m_FlyInAnim.Insert(60.0, cp2);

		ControlPoint cp3(Flight2End.p, Flight2End.q);
		m_FlyInAnim.Insert(100.0, cp3);

		// Start the camera at the start
		Flight2Start.ToMatrix(mat);
		m_pNormalCamera->SetTransform(mat);

		// Now, all at once, we've got turn off the globe and turn on the
		//  terrain, with as immediate a transition as possible.
		if (m_pGlobeContainer != NULL)
			m_pGlobeContainer->SetEnabled(false);

		// Don't let the trackball mess with the camera anymore
		if (m_pTrackball)
			m_pTrackball->SetEnabled(false);

		// make terrain active
		SwitchToTerrain(m_pTargetTerrain);

		// Set hither and yon
		m_pNormalCamera->SetHither(m_pTargetTerrain->GetParams().GetValueFloat(STR_HITHER));
		m_pNormalCamera->SetYon(500000.0f);

		// ensure that sunlight is active
		GetSunLightTransform()->SetEnabled(true);

		m_pTerrainPicker->SetEnabled(true);
		SetMode(MM_NAVIGATE);

		SetMessage(_("Welcome to "), m_pTargetTerrain->GetName(), 5.0f);

		// Layer view needs to update
		RefreshLayerView();

		// SetState(AS_FlyingIn);

		return;
	}
}

void Enviro::FlyInStage2()
{
	m_iFlightStep++;

	ControlPoint cp;
	if (m_FlyInAnim.GetInterpolatedControlPoint(m_iFlightStep, cp))
	{
		FMatrix4 matrix;
		cp.GetMatrix(matrix);
		m_pNormalCamera->SetTransform(matrix);
	}
	if (m_iFlightStep == 100)
	{
		m_iFlightStep = 0;
		m_iFlightStage = 0;
		m_iInitStep = 0;
		SetState(AS_Terrain);
	}
}

