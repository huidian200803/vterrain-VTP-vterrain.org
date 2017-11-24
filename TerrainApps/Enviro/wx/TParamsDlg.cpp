//
// Name: TParamsDlg.cpp
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "TParamsDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/colordlg.h>
#include <wx/dir.h>

#include "vtlib/vtlib.h"
#include "vtlib/core/Location.h"

#include "vtdata/DataPath.h"
#include "vtdata/FileFilters.h"
#include "vtdata/FilePath.h"		// for FindFileOnPaths
#include "vtdata/vtLog.h"
#include "vtui/ColorMapDlg.h"
#include "vtui/Helper.h"			// for AddFilenamesToChoice
#include "wxosg/TimeDlg.h"

#include "StyleDlg.h"
#include "TextureDlg.h"
#include "TinTextureDlg.h"
#include "TParamsDlg.h"

#include "ScenarioParamsDialog.h"

//---------------------------------------------------------------------------

/**
 * wxListBoxEventHandler is a roudabout way of catching events on our
 * listboxes, to implement the "Delete" key operation on them.
 */
class wxListBoxEventHandler: public wxEvtHandler
{
public:
	wxListBoxEventHandler(TParamsDlg *dlg, wxListBox *pBox)
	{
		m_pDlg = dlg;
		m_pBox = pBox;
	}
	void OnChar(wxKeyEvent& event)
	{
		if (event.GetKeyCode() == WXK_DELETE)
		{
			int sel = m_pBox->GetSelection();
			int count = m_pBox->GetCount();
			if (sel != -1 && sel < count-1)
			{
				m_pDlg->DeleteItem(m_pBox);
				m_pDlg->TransferDataToWindow();
			}
		}
		event.Skip();
	}

private:
	TParamsDlg *m_pDlg;
	wxListBox *m_pBox;
	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(wxListBoxEventHandler, wxEvtHandler)
	EVT_CHAR(wxListBoxEventHandler::OnChar)
END_EVENT_TABLE()


// WDR: class implementations

//----------------------------------------------------------------------------
// TParamsDlg
//----------------------------------------------------------------------------

// WDR: event table for TParamsDlg

BEGIN_EVENT_TABLE(TParamsDlg,TParamsDlgBase)
	EVT_INIT_DIALOG (TParamsDlg::OnInitDialog)

	// Primary Elevation
	EVT_RADIOBUTTON( ID_USE_GRID, TParamsDlg::OnCheckBoxElevType )
	EVT_RADIOBUTTON( ID_USE_TIN, TParamsDlg::OnCheckBoxElevType )
	EVT_RADIOBUTTON( ID_USE_TILESET, TParamsDlg::OnCheckBoxElevType )
	EVT_RADIOBUTTON( ID_USE_EXTERNAL, TParamsDlg::OnCheckBoxElevType )
	EVT_CHOICE( ID_LODMETHOD, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_TILE_THREADING, TParamsDlg::OnCheckBox )
	EVT_BUTTON( ID_PRIMARY_TEXTURE, TParamsDlg::OnPrimaryTexture )
	EVT_BUTTON( ID_SET_TIN_TEXTURE, TParamsDlg::OnSetTinTexture )

	// Extra
	EVT_BUTTON( ID_SET_TEXTURE, TParamsDlg::OnSetTexture )

	// Plants and Roads
	EVT_CHECKBOX( ID_ROADS, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_CHECK_STRUCTURE_SHADOWS, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_SHADOW_LIMIT, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_CHECK_STRUCTURE_PAGING, TParamsDlg::OnCheckBox )

	// Ephemeris
	EVT_CHECKBOX( ID_OCEANPLANE, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_WATER, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_DEPRESSOCEAN, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_SKY, TParamsDlg::OnCheckBox )
	EVT_CHECKBOX( ID_FOG, TParamsDlg::OnCheckBox )
	EVT_BUTTON( ID_BGCOLOR, TParamsDlg::OnBgColor )
	EVT_BUTTON( ID_SET_INIT_TIME, TParamsDlg::OnSetInitTime )

	// Abstracts
	EVT_BUTTON( ID_STYLE, TParamsDlg::OnStyle )

	// HUD
	EVT_BUTTON( ID_OVERLAY_DOTDOTDOT, TParamsDlg::OnOverlay )

	// Camera
	EVT_TEXT( ID_LOCFILE, TParamsDlg::OnChoiceLocFile )
	EVT_CHOICE( ID_INIT_LOCATION, TParamsDlg::OnChoiceInitLocation )

	// Scenario
	EVT_BUTTON( ID_NEW_SCENARIO, TParamsDlg::OnNewScenario )
	EVT_BUTTON( ID_DELETE_SCENARIO, TParamsDlg::OnDeleteScenario )
	EVT_BUTTON( ID_EDIT_SCENARIO, TParamsDlg::OnEditScenario )
	EVT_BUTTON( ID_MOVEUP_SCENARIO, TParamsDlg::OnMoveUpScenario )
	EVT_BUTTON( ID_MOVEDOWN_SCENARIO, TParamsDlg::OnMoveDownSceanario )
	EVT_LISTBOX( ID_SCENARIO_LIST, TParamsDlg::OnScenarioListEvent )
	EVT_CHOICE( ID_CHOICE_SCENARIO, TParamsDlg::OnChoiceScenario )

	// Clickable listboxes
	EVT_LISTBOX_DCLICK( ID_ELEVFILES, TParamsDlg::OnListDblClickElev )
	EVT_LISTBOX_DCLICK( ID_PLANTFILES, TParamsDlg::OnListDblClickPlants )
	EVT_LISTBOX_DCLICK( ID_STRUCTFILES, TParamsDlg::OnListDblClickStructure )
	EVT_LISTBOX_DCLICK( ID_RAWFILES, TParamsDlg::OnListDblClickRaw )
	EVT_LISTBOX_DCLICK( ID_ANIM_PATHS, TParamsDlg::OnListDblClickAnimPaths )
	EVT_LISTBOX_DCLICK( ID_IMAGEFILES, TParamsDlg::OnListDblClickImage )

END_EVENT_TABLE()

TParamsDlg::TParamsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	TParamsDlgBase( parent, id, title, position, size, style | wxRESIZE_BORDER )
{
	VTLOG("TParamsDlg: Constructing.\n");

	m_bSetting = false;

	// Work around wxFormDesigner's lack of support for limiting to smallest size
	GetSizer()->SetSizeHints(this);

	// make sure that validation gets down to the child windows
	// including the children of the notebook
	wxNotebook *notebook = (wxNotebook*) FindWindow( ID_NOTEBOOK );
	notebook->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);

	m_pPreLightFactor = GetLightFactor();

	m_pSingle = GetSingle();
	m_pDerived = GetDerived();

	m_iOverlayX = 0;
	m_iOverlayY = 0;
	m_bOverview = false;
	m_bCompass = false;

	// Create Validators To Attach C++ Members To WX Controls

	// overall name
	AddValidator(this, ID_TNAME, &m_strTerrainName);

	// elevation: grid clod
	AddValidator(this, ID_USE_GRID, &m_bGrid);
	AddValidator(this, ID_LODMETHOD, &m_iLodMethod);
	AddNumValidator(this, ID_TRI_COUNT, &m_iTriCount);

	// tin
	AddValidator(this, ID_USE_TIN, &m_bTin);

	// tileset
	AddValidator(this, ID_USE_TILESET, &m_bTileset);
	AddValidator(this, ID_FILENAME_TILES, &m_strFilenameTileset);
	AddNumValidator(this, ID_VTX_COUNT, &m_iVertCount);
	AddValidator(this, ID_TILE_THREADING, &m_bTileThreading);
	AddValidator(this, ID_TFILE_TILESET, &m_strTextureTileset);
	AddNumValidator(this, ID_TEX_LOD, &m_fTextureLODFactor);
	AddValidator(this, ID_TEXTURE_GRADUAL, &m_bTextureGradual);

	// external
	AddValidator(this, ID_USE_EXTERNAL, &m_bExternal);
	AddValidator(this, ID_TT_EXTERNAL_DATA, &m_strExternalData);

	AddValidator(this, ID_FILENAME, &m_strFilename);
	AddValidator(this, ID_FILENAME_TIN, &m_strFilenameTin);

	AddNumValidator(this, ID_VERTEXAG, &m_fVerticalExag, 2);

	// nav
	AddNumValidator(this, ID_MINHEIGHT, &m_fMinHeight, 2);
	AddValidator(this, ID_NAV_STYLE, &m_iNavStyle);
	AddNumValidator(this, ID_NAVSPEED, &m_fNavSpeed, 2);
	AddNumValidator(this, ID_DAMPING, &m_fDamping, 1);
	AddValidator(this, ID_LOCFILE, &m_strLocFile);
	AddValidator(this, ID_INIT_LOCATION, &m_iInitLocation);
	AddNumValidator(this, ID_HITHER, &m_fHither);
	AddValidator(this, ID_ACCEL, &m_bAccel);
	AddValidator(this, ID_ALLOW_ROLL, &m_bAllowRoll);

	// time
	AddValidator(this, ID_TIMEMOVES, &m_bTimeOn);
	AddValidator(this, ID_TEXT_INIT_TIME, &m_strInitTime);
	AddNumValidator(this, ID_TIMESPEED, &m_fTimeSpeed, 2);

	// culture page
	AddNumValidator(this, ID_VEGDISTANCE, &m_iVegDistance);
	AddValidator(this, ID_TREES_USE_SHADERS, &m_bTreesUseShaders);

	AddValidator(this, ID_ROADS, &m_bRoads);
	AddValidator(this, ID_ROADFILE, &m_strRoadFile);
	AddValidator(this, ID_HIGHWAYS, &m_bHwy);
	AddValidator(this, ID_PAVED, &m_bPaved);
	AddValidator(this, ID_DIRT, &m_bDirt);
	AddNumValidator(this, ID_ROADHEIGHT, &m_fRoadHeight);
	AddNumValidator(this, ID_ROADDISTANCE, &m_fRoadDistance);
	AddValidator(this, ID_TEXROADS, &m_bTexRoads);
	AddValidator(this, ID_ROADCULTURE, &m_bRoadCulture);

	AddValidator(this, ID_CONTENT_FILE, &m_strContent);
	AddNumValidator(this, ID_STRUCT_DISTANCE, &m_iStructDistance);

	// shadows
	AddValidator(this, ID_CHECK_STRUCTURE_SHADOWS, &m_bStructureShadows);
	AddValidator(this, ID_CHOICE_SHADOW_REZ, &m_iShadowRez);
	AddNumValidator(this, ID_DARKNESS, &m_fDarkness, 2);
	AddValidator(this, ID_SHADOWS_DEFAULT_ON, &m_bShadowsDefaultOn);
	AddValidator(this, ID_SHADOWS_EVERY_FRAME, &m_bShadowsEveryFrame);
	AddValidator(this, ID_SHADOW_LIMIT, &m_bLimitShadowArea);
	AddNumValidator(this, ID_SHADOW_LIMIT_RADIUS, &m_fShadowRadius);

	// paging
	AddValidator(this, ID_CHECK_STRUCTURE_PAGING, &m_bPagingStructures);
	AddNumValidator(this, ID_PAGING_MAX_STRUCTURES, &m_iPagingStructureMax);
	AddNumValidator(this, ID_PAGE_OUT_DISTANCE, &m_fPagingStructureDist, 1);
//	AddValidator(this, ID_VEHICLES, &m_bVehicles);

	// Ephemeris
	AddValidator(this, ID_SKY, &m_bSky);
	AddValidator(this, ID_SKYTEXTURE, &m_strSkyTexture);
	AddValidator(this, ID_OCEANPLANE, &m_bOceanPlane);
	AddNumValidator(this, ID_OCEANPLANEOFFSET, &m_fOceanPlaneLevel);
	AddValidator(this, ID_WATER, &m_bWater);
	AddValidator(this, ID_FILENAME_WATER, &m_strFilenameWater);
	AddValidator(this, ID_DEPRESSOCEAN, &m_bDepressOcean);
	AddNumValidator(this, ID_DEPRESSOCEANOFFSET, &m_fDepressOceanLevel);
	AddValidator(this, ID_FOG, &m_bFog);
	AddNumValidator(this, ID_FOG_DISTANCE, &m_fFogDistance);

	// HUD
	AddValidator(this, ID_OVERLAY_FILE, &m_strOverlayFile);
	AddNumValidator(this, ID_OVERLAY_X, &m_iOverlayX);
	AddNumValidator(this, ID_OVERLAY_Y, &m_iOverlayY);
	AddValidator(this, ID_CHECK_OVERVIEW, &m_bOverview);
	AddValidator(this, ID_CHECK_COMPASS, &m_bCompass);

	// It's somewhat roundabout, but this lets us capture events on the
	// listbox controls without having to subclass.
	m_elev_files->PushEventHandler(new wxListBoxEventHandler(this, m_elev_files));
	m_plant_files->PushEventHandler(new wxListBoxEventHandler(this, m_plant_files));
	m_structure_files->PushEventHandler(new wxListBoxEventHandler(this, m_structure_files));
	m_raw_files->PushEventHandler(new wxListBoxEventHandler(this, m_raw_files));
	m_anim_paths->PushEventHandler(new wxListBoxEventHandler(this, m_anim_paths));
	m_image_files->PushEventHandler(new wxListBoxEventHandler(this, m_image_files));
}

TParamsDlg::~TParamsDlg()
{
	m_elev_files->PopEventHandler(true);
	m_plant_files->PopEventHandler(true);
	m_structure_files->PopEventHandler(true);
	m_raw_files->PopEventHandler(true);
	m_anim_paths->PopEventHandler(true);
	m_image_files->PopEventHandler(true);
}

//
// Set the values in the dialog from the supplied paramter structure.
// Note that TParams uses UTF8 for all its strings, so we need to use
//  from_utf8() when copying to wxString.
//
void TParamsDlg::SetParams(const TParams &Params)
{
	VTLOG("TParamsDlg::SetParams\n");
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	// Store main copy of the parameters
	m_Params = Params;

	// overall name
	m_strTerrainName = wxString(m_Params.GetValueString(STR_NAME), wxConvUTF8);

	// elevation
	m_bGrid =			m_Params.GetValueInt(STR_SURFACE_TYPE) == 0;
	m_bTin =			m_Params.GetValueInt(STR_SURFACE_TYPE) == 1;
	m_bTileset =		m_Params.GetValueInt(STR_SURFACE_TYPE) == 2;
	m_bExternal =		m_Params.GetValueInt(STR_SURFACE_TYPE) == 3;

	if (m_bExternal)
		m_strExternalData = wxString(m_Params.GetValueString(STR_ELEVFILE), wxConvUTF8);
	if (m_bGrid)
		m_strFilename = wxString(m_Params.GetValueString(STR_ELEVFILE), wxConvUTF8);
	if (m_bTin)
		m_strFilenameTin = wxString(m_Params.GetValueString(STR_ELEVFILE), wxConvUTF8);
	if (m_bTileset)
	{
		m_strFilenameTileset = wxString(m_Params.GetValueString(STR_ELEVFILE), wxConvUTF8);
		m_strTextureTileset = wxString(m_Params.GetValueString(STR_TEXTUREFILE), wxConvUTF8);
	}

	m_iLodMethod =		m_Params.GetLodMethod();
	m_iTriCount =		m_Params.GetValueInt(STR_TRICOUNT);
	m_iVertCount =		m_Params.GetValueInt(STR_VERTCOUNT);
	m_bTileThreading =	m_Params.GetValueBool(STR_TILE_THREADING);
	m_fTextureLODFactor =	m_Params.GetValueFloat(STR_TEXURE_LOD_FACTOR);
	m_bTextureGradual =	m_Params.GetValueBool(STR_TEXTURE_GRADUAL);
	m_fVerticalExag =   m_Params.GetValueFloat(STR_VERTICALEXAG);

	/// navigation
	m_fMinHeight =		m_Params.GetValueFloat(STR_MINHEIGHT);
	m_iNavStyle =		m_Params.GetValueInt(STR_NAVSTYLE);
	m_fNavSpeed =		m_Params.GetValueFloat(STR_NAVSPEED);
	m_fDamping =		m_Params.GetValueFloat(STR_NAVDAMPING);
	m_strLocFile = wxString(m_Params.GetValueString(STR_LOCFILE), wxConvUTF8);
	m_strInitLocation = wxString(m_Params.GetValueString(STR_INITLOCATION), wxConvUTF8);
	m_fHither =			m_Params.GetValueFloat(STR_HITHER);
	m_bAccel =			m_Params.GetValueBool(STR_ACCEL);
	m_bAllowRoll =		m_Params.GetValueBool(STR_ALLOW_ROLL);
	m_AnimPaths =		m_Params.m_AnimPaths;

	// time
	m_bTimeOn =			m_Params.GetValueBool(STR_TIMEON);
	m_InitTime.SetFromString(m_Params.GetValueString(STR_INITTIME));
	m_fTimeSpeed =		m_Params.GetValueFloat(STR_TIMESPEED);

	// culture
	m_bRoads =			m_Params.GetValueBool(STR_ROADS);
	m_strRoadFile = wxString(m_Params.GetValueString(STR_ROADFILE), wxConvUTF8);
	m_bHwy =			m_Params.GetValueBool(STR_HWY);
	m_bPaved =			m_Params.GetValueBool(STR_PAVED);
	m_bDirt =			m_Params.GetValueBool(STR_DIRT);
	m_fRoadHeight =		m_Params.GetValueFloat(STR_ROADHEIGHT);
	m_fRoadDistance =	m_Params.GetValueFloat(STR_ROADDISTANCE);
	m_bTexRoads =		m_Params.GetValueBool(STR_TEXROADS);
	m_bRoadCulture =	m_Params.GetValueBool(STR_ROADCULTURE);

	m_iVegDistance =	m_Params.GetValueInt(STR_VEGDISTANCE);
	m_bTreesUseShaders = m_Params.GetValueBool(STR_TREES_USE_SHADERS);

	m_bFog =			m_Params.GetValueBool(STR_FOG);
	m_fFogDistance =	m_Params.GetValueFloat(STR_FOGDISTANCE);

	// Layers and structure stuff
	m_strContent = wxString(m_Params.GetValueString(STR_CONTENT_FILE), wxConvUTF8);
	m_Layers = m_Params.m_Layers;
	m_iStructDistance = m_Params.GetValueInt(STR_STRUCTDIST);
	// shadows
	m_bStructureShadows =	 m_Params.GetValueBool(STR_STRUCT_SHADOWS);
	m_iShadowRez =	 vt_log2(m_Params.GetValueInt(STR_SHADOW_REZ))-8;
	m_fDarkness =			 m_Params.GetValueFloat(STR_SHADOW_DARKNESS);
	m_bShadowsDefaultOn =	 m_Params.GetValueBool(STR_SHADOWS_DEFAULT_ON);
	m_bShadowsEveryFrame =	 m_Params.GetValueBool(STR_SHADOWS_EVERY_FRAME);
	m_bLimitShadowArea =	 m_Params.GetValueBool(STR_LIMIT_SHADOW_AREA);
	m_fShadowRadius =		 m_Params.GetValueFloat(STR_SHADOW_RADIUS);
	// paging
	m_bPagingStructures =	 m_Params.GetValueBool(STR_STRUCTURE_PAGING);
	m_iPagingStructureMax =	 m_Params.GetValueInt(STR_STRUCTURE_PAGING_MAX);
	m_fPagingStructureDist = m_Params.GetValueFloat(STR_STRUCTURE_PAGING_DIST);

//	m_bVehicles =	   m_Params.GetValueBool(STR_VEHICLES);
//  m_fVehicleSize =	m_Params.GetValueFloat(STR_VEHICLESIZE);
//  m_fVehicleSpeed =   m_Params.GetValueFloat(STR_VEHICLESPEED);

	m_bSky =			m_Params.GetValueBool(STR_SKY);
	m_strSkyTexture = wxString(m_Params.GetValueString(STR_SKYTEXTURE), wxConvUTF8);
	m_bOceanPlane =	 m_Params.GetValueBool(STR_OCEANPLANE);
	m_fOceanPlaneLevel = m_Params.GetValueFloat(STR_OCEANPLANELEVEL);
	m_bWater =	 m_Params.GetValueBool(STR_WATER);
	m_strFilenameWater =	 wxString(m_Params.GetValueString(STR_WATERFILE), wxConvUTF8);
	m_bDepressOcean =   m_Params.GetValueBool(STR_DEPRESSOCEAN);
	m_fDepressOceanLevel = m_Params.GetValueFloat(STR_DEPRESSOCEANLEVEL);
	RGBi col =			m_Params.GetValueRGBi(STR_BGCOLOR);
	m_BgColor.Set(col.r, col.g, col.b);

	m_strUtilFile = wxString(m_Params.GetValueString(STR_UTILITY_FILE), wxConvUTF8);

	vtString fname;
	if (m_Params.GetOverlay(fname, m_iOverlayX, m_iOverlayY))
		m_strOverlayFile = wxString(fname, wxConvUTF8);
	m_bOverview =	m_Params.GetValueBool(STR_OVERVIEW);
	m_bCompass =	m_Params.GetValueBool(STR_COMPASS);

	// Scenarios
	m_strInitScenario = wxString(m_Params.GetValueString(STR_INIT_SCENARIO), wxConvUTF8);
	m_Scenarios = m_Params.m_Scenarios;

	// Safety checks
	if (m_iTriCount < 500 || m_iTriCount > 100000)
		m_iTriCount = 10000;
	if (m_fTextureLODFactor < .1f) m_fTextureLODFactor = .1f;
	if (m_fTextureLODFactor > 1) m_fTextureLODFactor = 1;

	VTLOG("   Finished SetParams\n");
}

//
// get the values from the dialog into the supplied parameter structure
//
void TParamsDlg::GetParams(TParams &Params)
{
	VTLOG("TParamsDlg::GetParams\n");
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	// overall name
	m_Params.SetValueString(STR_NAME, (const char *) m_strTerrainName.mb_str(wxConvUTF8));

	// elevation
	if (m_bGrid)
	{
		m_Params.SetValueInt(STR_SURFACE_TYPE, 0);
		m_Params.SetValueString(STR_ELEVFILE, (const char *) m_strFilename.mb_str(wxConvUTF8));
	}
	m_Params.SetLodMethod((enum LodMethodEnum) m_iLodMethod);
	m_Params.SetValueInt(STR_TRICOUNT, m_iTriCount);
	if (m_bTin)
	{
		m_Params.SetValueInt(STR_SURFACE_TYPE, 1);
		m_Params.SetValueString(STR_ELEVFILE, (const char *) m_strFilenameTin.mb_str(wxConvUTF8));
	}
	if (m_bTileset)
	{
		m_Params.SetValueInt(STR_SURFACE_TYPE, 2);
		m_Params.SetValueString(STR_ELEVFILE, (const char *) m_strFilenameTileset.mb_str(wxConvUTF8));
	}
	m_Params.SetValueInt(STR_VERTCOUNT, m_iVertCount);
	m_Params.SetValueBool(STR_TILE_THREADING, m_bTileThreading);
	m_Params.SetValueBool(STR_TEXTURE_GRADUAL, m_bTextureGradual);
	m_Params.SetValueFloat(STR_TEXURE_LOD_FACTOR, m_fTextureLODFactor);
	if (m_bExternal)
	{
		m_Params.SetValueInt(STR_SURFACE_TYPE, 3);
		m_Params.SetValueString(STR_ELEVFILE, (const char *) m_strExternalData.mb_str(wxConvUTF8));
	}
	m_Params.SetValueFloat(STR_VERTICALEXAG, m_fVerticalExag);

	// navigation
	m_Params.SetValueFloat(STR_MINHEIGHT, m_fMinHeight);
	m_Params.SetValueInt(STR_NAVSTYLE, m_iNavStyle);
	m_Params.SetValueFloat(STR_NAVSPEED, m_fNavSpeed);
	m_Params.SetValueFloat(STR_NAVDAMPING, m_fDamping);
	m_Params.SetValueString(STR_LOCFILE, (const char *) m_strLocFile.mb_str(wxConvUTF8));
	m_Params.SetValueString(STR_INITLOCATION, (const char *) m_strInitLocation.mb_str(wxConvUTF8));
	m_Params.SetValueFloat(STR_HITHER, m_fHither);
	m_Params.SetValueBool(STR_ACCEL, m_bAccel);
	m_Params.SetValueBool(STR_ALLOW_ROLL, m_bAllowRoll);
	m_Params.m_AnimPaths = m_AnimPaths;

	// time
	m_Params.SetValueBool(STR_TIMEON, m_bTimeOn);
	m_Params.SetValueString(STR_INITTIME, m_InitTime.GetAsString());
	m_Params.SetValueFloat(STR_TIMESPEED, m_fTimeSpeed);

	// tileset
	if (m_bTileset)
		m_Params.SetValueString(STR_TEXTUREFILE, (const char *) m_strTextureTileset.mb_str(wxConvUTF8));

	// culture
	m_Params.SetValueBool(STR_ROADS, m_bRoads);
	m_Params.SetValueString(STR_ROADFILE, (const char *) m_strRoadFile.mb_str(wxConvUTF8));
	m_Params.SetValueBool(STR_HWY, m_bHwy);
	m_Params.SetValueBool(STR_PAVED, m_bPaved);
	m_Params.SetValueBool(STR_DIRT, m_bDirt);
	m_Params.SetValueFloat(STR_ROADHEIGHT, m_fRoadHeight);
	m_Params.SetValueFloat(STR_ROADDISTANCE, m_fRoadDistance);
	m_Params.SetValueBool(STR_TEXROADS, m_bTexRoads);
	m_Params.SetValueBool(STR_ROADCULTURE, m_bRoadCulture);

	m_Params.SetValueInt(STR_VEGDISTANCE, m_iVegDistance);
	m_Params.SetValueBool(STR_TREES_USE_SHADERS, m_bTreesUseShaders);

	m_Params.SetValueBool(STR_FOG, m_bFog);
	m_Params.SetValueFloat(STR_FOGDISTANCE, m_fFogDistance);
	// (fog color not exposed in UI)

	// Layers and structure stuff
	m_Params.SetValueString(STR_CONTENT_FILE, (const char *) m_strContent.mb_str(wxConvUTF8));
	m_Params.m_Layers = m_Layers;

	m_Params.SetValueInt(STR_STRUCTDIST, m_iStructDistance);
	m_Params.SetValueBool(STR_STRUCT_SHADOWS, m_bStructureShadows);
	// shadows
	m_Params.SetValueInt(STR_SHADOW_REZ, 1 << (m_iShadowRez+8));
	m_Params.SetValueFloat(STR_SHADOW_DARKNESS, m_fDarkness);
	m_Params.SetValueBool(STR_SHADOWS_DEFAULT_ON, m_bShadowsDefaultOn);
	m_Params.SetValueBool(STR_SHADOWS_EVERY_FRAME, m_bShadowsEveryFrame);
	m_Params.SetValueBool(STR_LIMIT_SHADOW_AREA, m_bLimitShadowArea);
	m_Params.SetValueFloat(STR_SHADOW_RADIUS, m_fShadowRadius);
	// paging
	m_Params.SetValueBool(STR_STRUCTURE_PAGING, m_bPagingStructures);
	m_Params.SetValueInt(STR_STRUCTURE_PAGING_MAX, m_iPagingStructureMax);
	m_Params.SetValueFloat(STR_STRUCTURE_PAGING_DIST, m_fPagingStructureDist);

//	m_Params.SetValueBool(STR_VEHICLES, m_bVehicles);
//  m_Params.SetValueFloat(STR_VEHICLESIZE, m_fVehicleSize);
//  m_Params.SetValueFloat(STR_VEHICLESPEED, m_fVehicleSpeed);

	m_Params.SetValueBool(STR_SKY, m_bSky);
	m_Params.SetValueString(STR_SKYTEXTURE, (const char *) m_strSkyTexture.mb_str(wxConvUTF8));

	m_Params.SetValueBool(STR_OCEANPLANE, m_bOceanPlane);
	m_Params.SetValueFloat(STR_OCEANPLANELEVEL, m_fOceanPlaneLevel);
	m_Params.SetValueBool(STR_WATER, m_bWater);
	m_Params.SetValueString(STR_WATERFILE, (const char *) m_strFilenameWater.mb_str(wxConvUTF8));
	m_Params.SetValueBool(STR_DEPRESSOCEAN, m_bDepressOcean);
	m_Params.SetValueFloat(STR_DEPRESSOCEANLEVEL, m_fDepressOceanLevel);
	RGBi col(m_BgColor.Red(), m_BgColor.Green(), m_BgColor.Blue());
	m_Params.SetValueRGBi(STR_BGCOLOR, col);

	m_Params.SetValueString(STR_UTILITY_FILE, (const char *) m_strUtilFile.mb_str(wxConvUTF8));

	// HUD
	m_Params.SetOverlay((const char *) m_strOverlayFile.mb_str(wxConvUTF8), m_iOverlayX, m_iOverlayY);
	m_Params.SetValueBool(STR_OVERVIEW, m_bOverview);
	m_Params.SetValueBool(STR_COMPASS, m_bCompass);

	// Scenarios
	m_Params.SetValueString(STR_INIT_SCENARIO, (const char *) m_strInitScenario.mb_str(wxConvUTF8));
	m_Params.m_Scenarios = m_Scenarios;

	// Sent back our copy of the parameters.
	Params = m_Params;
}

void TParamsDlg::UpdateEnableState()
{
	m_filename->Enable(m_bGrid);
	m_primary_texture->Enable(m_bGrid);
	m_lodmethod->Enable(m_bGrid);
	m_tri_count->Enable(m_bGrid);

	m_filename_tin->Enable(m_bTin);
	m_set_tin_texture->Enable(m_bTin);

	m_filename_tileset->Enable(m_bTileset);
	m_vtx_count->Enable(m_bTileset);
	m_tile_threading->Enable(m_bTileset);
	m_tfile_tileset->Enable(m_bTileset);
	m_tex_lod->Enable(m_bTileset);
	m_texture_gradual->Enable(m_bTileset);

	m_tt_external_data->Enable(m_bExternal);

	FindWindow(ID_TEXTURE_GRADUAL)->Enable(m_bTileset && m_bTileThreading);
	FindWindow(ID_TEX_LOD)->Enable(m_bTileset);

	FindWindow(ID_ROADFILE)->Enable(m_bRoads);
	FindWindow(ID_ROADHEIGHT)->Enable(m_bRoads);
	FindWindow(ID_ROADDISTANCE)->Enable(m_bRoads);
	FindWindow(ID_TEXROADS)->Enable(m_bRoads);
	FindWindow(ID_ROADCULTURE)->Enable(m_bRoads);
	FindWindow(ID_HIGHWAYS)->Enable(m_bRoads);
	FindWindow(ID_PAVED)->Enable(m_bRoads);
	FindWindow(ID_DIRT)->Enable(m_bRoads);

	FindWindow(ID_CHOICE_SHADOW_REZ)->Enable(m_bStructureShadows);
	FindWindow(ID_DARKNESS)->Enable(m_bStructureShadows);
	FindWindow(ID_SHADOWS_DEFAULT_ON)->Enable(m_bStructureShadows);
	FindWindow(ID_SHADOWS_EVERY_FRAME)->Enable(m_bStructureShadows);
	FindWindow(ID_SHADOW_LIMIT)->Enable(m_bStructureShadows);
	FindWindow(ID_SHADOW_LIMIT_RADIUS)->Enable(m_bStructureShadows && m_bLimitShadowArea);
	FindWindow(ID_PAGING_MAX_STRUCTURES)->Enable(m_bPagingStructures);
	FindWindow(ID_PAGE_OUT_DISTANCE)->Enable(m_bPagingStructures);

	FindWindow(ID_OCEANPLANEOFFSET)->Enable(m_bOceanPlane);
	FindWindow(ID_FILENAME_WATER)->Enable(m_bWater);
	FindWindow(ID_DEPRESSOCEANOFFSET)->Enable(m_bDepressOcean);
	m_skytexture->Enable(m_bSky);
	GetFogDistance()->Enable(m_bFog);

	int iSelected = m_scenario_list->GetSelection();
	if (iSelected != wxNOT_FOUND)
	{
		GetEditScenario()->Enable(true);
		GetDeleteScenario()->Enable(true);
		if (iSelected != (m_scenario_list->GetCount() - 1))
			GetMovedownScenario()->Enable(true);
		else
			GetMovedownScenario()->Enable(false);
	   if (iSelected != 0)
			GetMoveupScenario()->Enable(true);
		else
			GetMoveupScenario()->Enable(false);
	}
	else
	{
		GetEditScenario()->Enable(false);
		GetDeleteScenario()->Enable(false);
		GetMoveupScenario()->Enable(false);
		GetMovedownScenario()->Enable(false);
	}
}

void TParamsDlg::RefreshLocationFields()
{
	m_init_location->Clear();
	m_init_location->Append(_("(default)"));

	vtString locfile = (const char *)m_strLocFile.mb_str(wxConvUTF8);
	if (locfile == "")
		return;

	vtString fname = "Locations/";
	fname += locfile;
	vtString path = FindFileOnPaths(vtGetDataPath(), fname);
	if (path == "")
		return;
	vtLocationSaver saver;
	if (!saver.Read(path))
		return;

	int num = saver.NumLocations();
	for (int i = 0; i < num; i++)
	{
		vtLocation *loc = saver.GetLocation(i);
		wxString str(loc->m_strName.c_str(), wxConvUTF8);
		m_init_location->Append(str);
	}
	if (num)
	{
		if (m_iInitLocation < 0)
			m_iInitLocation = 0;
		if (m_iInitLocation > num-1)
			m_iInitLocation = num-1;
	}
}

void TParamsDlg::DeleteItem(wxListBox *pBox)
{
	vtString fname = (const char *) pBox->GetStringSelection().mb_str(wxConvUTF8);

	// might be a layer
	int idx = FindLayerByFilename(fname);
	if (idx != -1)
		m_Layers.erase(m_Layers.begin()+idx);

	// or an animpath
	for (uint i = 0; i < m_AnimPaths.size(); i++)
	{
		if (!fname.Compare(m_AnimPaths[i]))
		{
			m_AnimPaths.erase(m_AnimPaths.begin()+i);
			break;
		}
	}
}

int TParamsDlg::FindLayerByFilename(const vtString &fname)
{
	vtString layer_fname;
	for (uint i = 0; i < m_Layers.size(); i++)
	{
		if (m_Layers[i].GetValueString("Filename", layer_fname))
		{
			if (fname == layer_fname)
				return (int) i;
		}
	}
	return -1;
}


// WDR: handler implementations for TParamsDlg

void TParamsDlg::OnBgColor( wxCommandEvent &event )
{
	wxColourData data;
	data.SetChooseFull(true);
	data.SetColour(m_BgColor);

	wxColourDialog dlg(this, &data);
	if (dlg.ShowModal() == wxID_OK)
	{
		wxColourData data2 = dlg.GetColourData();
		m_BgColor = data2.GetColour();
		UpdateColorControl();
	}
}

void TParamsDlg::OnInitDialog(wxInitDialogEvent& event)
{
	VTLOG1("TParamsDlg::OnInitDialog\n");

	bool bShowProgress = (vtGetDataPath().size() > 1);
	if (bShowProgress)
		OpenProgressDialog(_("Looking for files on data paths"), _T(""), false, this);

	m_bReady = false;
	m_bSetting = true;

	uint i;
	int sel;

	// Clear drop-down controls before putting values into them
	m_filename->Clear();
	m_filename_tin->Clear();
	m_filename_tileset->Clear();
	m_tfile_tileset->Clear();
	m_filename_water->Clear();

	vtStringArray &paths = vtGetDataPath();

	for (i = 0; i < paths.size(); i++)
	{
		if (bShowProgress)
			UpdateProgressDialog(i * 100 / paths.size(), wxString(paths[i], wxConvUTF8));

		// Gather all possible texture image filenames
		AddFilenamesToStringArray(m_TextureFiles, paths[i] + "GeoSpecific", "*.bmp");
		AddFilenamesToStringArray(m_TextureFiles, paths[i] + "GeoSpecific", "*.jpg");
		AddFilenamesToStringArray(m_TextureFiles, paths[i] + "GeoSpecific", "*.jpeg");
		AddFilenamesToStringArray(m_TextureFiles, paths[i] + "GeoSpecific", "*.png");
		AddFilenamesToStringArray(m_TextureFiles, paths[i] + "GeoSpecific", "*.tif");

		// fill the "Grid filename" control with available files
		AddFilenamesToComboBox(m_filename, paths[i] + "Elevation", "*.bt*");

		// fill the "TIN filename" control with available files
		AddFilenamesToComboBox(m_filename_tin, paths[i] + "Elevation", "*.tin");
		AddFilenamesToComboBox(m_filename_tin, paths[i] + "Elevation", "*.itf");

		// fill the "Tileset filename" control with available files
		AddFilenamesToComboBox(m_filename_tileset, paths[i] + "Elevation", "*.ini");

		// fill the "texture Tileset filename" control with available files
		AddFilenamesToComboBox(m_tfile_tileset, paths[i] + "GeoSpecific", "*.ini");

		// fill the Location files
		AddFilenamesToComboBox(m_locfile, paths[i] + "Locations", "*.loc");

		// fill in Road files
		AddFilenamesToComboBox(m_roadfile, paths[i] + "RoadData", "*.rmf");

		// fill in Content file
		AddFilenamesToComboBox(m_content_file, paths[i], "*.vtco");

		// fill in Water (TIN) files
		AddFilenamesToComboBox(m_filename_water, paths[i] + "Elevation", "*.itf");

		// fill in Sky files
		AddFilenamesToComboBox(m_skytexture, paths[i] + "Sky", "*.bmp");
		AddFilenamesToComboBox(m_skytexture, paths[i] + "Sky", "*.png");
		AddFilenamesToComboBox(m_skytexture, paths[i] + "Sky", "*.jpg");
	}

	sel = m_filename->FindString(m_strFilename);
	if (sel != -1)
		m_filename->SetSelection(sel);

	sel = m_filename_tin->FindString(m_strFilenameTin);
	if (sel != -1)
		m_filename_tin->SetSelection(sel);
		
	sel = m_filename_tileset->FindString(m_strFilenameTileset);
	if (sel != -1)
		m_filename_tileset->SetSelection(sel);

	sel = m_tfile_tileset->FindString(m_strTextureTileset);
	if (sel != -1)
		m_tfile_tileset->SetSelection(sel);

	sel = m_locfile->FindString(m_strLocFile);
	if (sel != -1)
		m_locfile->SetSelection(sel);

	sel = m_roadfile->FindString(m_strRoadFile);
	if (sel != -1)
		m_roadfile->SetSelection(sel);
		
	sel = m_content_file->FindString(m_strContent);
	if (sel != -1)
		m_content_file->SetSelection(sel);
		
	sel = m_filename_water->FindString(m_strFilenameWater);
	if (sel != -1)
		m_filename_water->SetSelection(sel);
		
	sel = m_skytexture->FindString(m_strSkyTexture);
	if (sel != -1)
		m_skytexture->SetSelection(sel);
		
	// The following must match the ordering in the enum LodMethodEnum:
	m_lodmethod->Clear();
	m_lodmethod->Append(_T("Roettger"));
	m_lodmethod->Append(_T("---"));
	m_lodmethod->Append(_T("McNally"));
	m_lodmethod->Append(_T("---"));
	m_lodmethod->Append(_("Custom"));
	// add your own LOD method here!

	m_lodmethod->SetSelection(m_iLodMethod);

	m_choice_shadow_rez->Clear();
	m_choice_shadow_rez->Append(_T("256"));
	m_choice_shadow_rez->Append(_T("512"));
	m_choice_shadow_rez->Append(_T("1024"));
	m_choice_shadow_rez->Append(_T("2048"));
	m_choice_shadow_rez->Append(_T("4096"));

	m_nav_style->Clear();
	m_nav_style->Append(_("Normal Terrain Flyer"));
	m_nav_style->Append(_("Terrain Flyer with Velocity"));
	m_nav_style->Append(_("Grab-Pivot"));
//  m_nav_style->Append(_("Quake-Style Walk"));
	m_nav_style->Append(_("Panoramic Flyer"));

	RefreshLocationFields();

//  DetermineTerrainSizeFromBT();
//  DetermineSizeFromBMP();

//  OnChangeMem();

	GetUseGrid()->SetValue(m_bGrid);
	GetUseTin()->SetValue(m_bTin);
	GetUseTileset()->SetValue(m_bTileset);
	GetUseExternal()->SetValue(m_bExternal);

	UpdateTimeString();

	m_iInitLocation = m_init_location->FindString(m_strInitLocation);
	if (m_iInitLocation == -1)
		m_iInitLocation = 0;

	UpdateColorControl();

	UpdateScenarioChoices();

	wxWindow::OnInitDialog(event);

	UpdateEnableState();

	if (bShowProgress)
		CloseProgressDialog();

	m_bReady = true;
	VTLOG1("TParamsDlg::OnInitDialog done.\n");
}

bool TParamsDlg::TransferDataToWindow()
{
	m_bSetting = true;

	uint i;
	m_elev_files->Clear();
	m_plant_files->Clear();
	m_structure_files->Clear();
	m_raw_files->Clear();
	m_image_files->Clear();
	for (i = 0; i < m_Layers.size(); i++)
	{
		vtString ltype = m_Layers[i].GetValueString("Type");
		vtString fname = m_Layers[i].GetValueString("Filename");
		wxString fname2(fname, wxConvUTF8);

		if (ltype == TERR_LTYPE_VEGETATION)
			m_plant_files->Append(fname2);
		if (ltype == TERR_LTYPE_STRUCTURE)
			m_structure_files->Append(fname2);
		if (ltype == TERR_LTYPE_ABSTRACT)
			m_raw_files->Append(fname2);
		if (ltype == TERR_LTYPE_IMAGE)
			m_image_files->Append(fname2);
		if (ltype == TERR_LTYPE_ELEVATION)
			m_elev_files->Append(fname2);
	}
	m_elev_files->Append(_("(double-click to add files)"));
	m_plant_files->Append(_("(double-click to add files)"));
	m_structure_files->Append(_("(double-click to add files)"));
	m_raw_files->Append(_("(double-click to add files)"));
	m_image_files->Append(_("(double-click to add files)"));

	m_anim_paths->Clear();
	for (i = 0; i < m_AnimPaths.size(); i++)
		m_anim_paths->Append(wxString(m_AnimPaths[i], wxConvUTF8));
	m_anim_paths->Append(_("(double-click to add files)"));

	m_scenario_list->Clear();
	for (i = 0; i < m_Scenarios.size(); i++)
	{
		wxString str(m_Scenarios[i].GetValueString(STR_SCENARIO_NAME), wxConvUTF8);
		m_scenario_list->Append(str);
	}

	bool result = wxDialog::TransferDataToWindow();
	m_bSetting = false;

	return result;
}

bool TParamsDlg::TransferDataFromWindow()
{
	return wxDialog::TransferDataFromWindow();
}

void TParamsDlg::UpdateColorControl()
{
	FillWithColor(GetColorBitmap(), m_BgColor);
}

void TParamsDlg::OnComboTFileSingle( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
}

void TParamsDlg::OnCheckBoxElevType( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	UpdateEnableState();
}

void TParamsDlg::OnCheckBox( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	UpdateEnableState();
}

//
// This function is used to find all files in a given directory,
// and if they match a wildcard, add them to a string array.
//
void AddFilenamesToArray(wxArrayString &array, const wxString &dirname,
	const wxString &wildcard)
{
	// We could just call this:
	// wxDir::GetAllFiles(dirname, &array, wildcard, wxDIR_FILES);
	// However, that gets the full path names.  We only want the filenames.

	if (!wxDir::Exists(dirname))
		return;
	wxDir dir(dirname);
	wxString filename;
	bool cont = dir.GetFirst(&filename, wildcard, wxDIR_FILES);
	while ( cont )
	{
		array.Add(filename);
		cont = dir.GetNext(&filename);
	}
}

void TParamsDlg::OnListDblClickElev( wxCommandEvent &event )
{
	uint i;
	wxArrayString strings;

	for (i = 0; i < vtGetDataPath().size(); i++)
	{
		wxString path(vtGetDataPath()[i], wxConvUTF8);
		path += _T("Elevation");
		AddFilenamesToArray(strings, path, _T("*.itf"));
	}

	wxString result = wxGetSingleChoice(_("One of the following to add:"),
		_("Choose an elevation file"), strings, this);

	if (result.Cmp(_T(""))) // user selected something
	{
		TransferDataFromWindow();
		vtTagArray lay;
		lay.SetValueString("Type", TERR_LTYPE_ELEVATION, true);
		lay.SetValueString("Filename", (const char *) result.mb_str(wxConvUTF8), true);

		// Defaults
		lay.SetValueString(STR_COLOR_MAP, "default_relative.cmt");
		lay.SetValueString(STR_TEXTURE_GEOTYPICAL, "");
		lay.SetValueFloat(STR_GEOTYPICAL_SCALE, 10.0);
		lay.SetValueFloat(STR_OPACITY, 1.0);
	
		m_Layers.push_back(lay);
		TransferDataToWindow();
	}
}

void TParamsDlg::OnListDblClickPlants( wxCommandEvent &event )
{
	uint i;
	wxArrayString strings;

	for (i = 0; i < vtGetDataPath().size(); i++)
	{
		wxString path(vtGetDataPath()[i], wxConvUTF8);
		path += _T("PlantData");
		AddFilenamesToArray(strings, path, _T("*.vf*"));
		AddFilenamesToArray(strings, path, _T("*.shp"));
	}

	wxString result = wxGetSingleChoice(_("One of the following to add:"),
		_("Choose a plant file"), strings, this);

	if (result.Cmp(_T(""))) // user selected something
	{
		TransferDataFromWindow();
		vtTagArray lay;
		lay.SetValueString("Type", TERR_LTYPE_VEGETATION, true);
		lay.SetValueString("Filename", (const char *) result.mb_str(wxConvUTF8), true);
		m_Layers.push_back(lay);
		TransferDataToWindow();
	}
}

void TParamsDlg::OnListDblClickStructure( wxCommandEvent &event )
{
	uint i;
	wxArrayString strings;

	for (i = 0; i < vtGetDataPath().size(); i++)
	{
		wxString path(vtGetDataPath()[i], wxConvUTF8);
		path += _T("BuildingData");
		AddFilenamesToArray(strings, path, _T("*.vtst*"));
	}

	wxString result = wxGetSingleChoice(_("One of the following to add:"),
		_("Choose a structure file"), strings, this);

	if (result.Cmp(_T(""))) // user selected something
	{
		TransferDataFromWindow();
		vtTagArray lay;
		lay.SetValueString("Type", TERR_LTYPE_STRUCTURE, true);
		lay.SetValueString("Filename", (const char *) result.mb_str(wxConvUTF8), true);
		m_Layers.push_back(lay);
		TransferDataToWindow();
	}
}

void TParamsDlg::OnListDblClickRaw( wxCommandEvent &event )
{
	// Ask user for Raw feature file
	//  Default to the data path
	wxString defdir;
	if (vtGetDataPath().size() > 0)
		defdir = wxString((const char *) vtGetDataPath().at(0), wxConvUTF8);

	wxString filter = _("Feature files|");
	AddType(filter, FSTRING_SHP);
	AddType(filter, FSTRING_IGC);
	AddType(filter, FSTRING_DXF);
	wxFileDialog loadFile(NULL, _("Load features"), defdir, _T(""),
		filter, wxFD_OPEN);

	bool bResult = (loadFile.ShowModal() == wxID_OK);

	if (bResult) // user selected something
	{
		wxString result = loadFile.GetPath();

		vtTagArray lay;
		lay.SetValueString("Type", TERR_LTYPE_ABSTRACT, true);
		lay.SetValueString("Filename", (const char *) result.mb_str(wxConvUTF8), true);
		m_Layers.push_back(lay);
		TransferDataToWindow();
	}
}

void TParamsDlg::OnListDblClickAnimPaths( wxCommandEvent &event )
{
	uint i;
	wxArrayString strings;

	for (i = 0; i < vtGetDataPath().size(); i++)
	{
		wxString path(vtGetDataPath()[i], *wxConvCurrent);
		path += _T("Locations");
		AddFilenamesToArray(strings, path, _T("*.vtap"));
	}

	wxString result = wxGetSingleChoice(_("One of the following to add:"), _("Choose an animpath file"),
		strings, this);

	if (result.Cmp(_T(""))) // user selected something
	{
		m_AnimPaths.push_back(vtString(result.mb_str(wxConvUTF8)));
		TransferDataToWindow();
	}
}

void TParamsDlg::OnListDblClickImage( wxCommandEvent &event )
{
	uint i;
	wxArrayString strings;

	for (i = 0; i < vtGetDataPath().size(); i++)
	{
		wxString path(vtGetDataPath()[i], wxConvUTF8);
		path += _T("GeoSpecific");
		AddFilenamesToArray(strings, path, _T("*.bmp"));
		AddFilenamesToArray(strings, path, _T("*.jpg"));
		AddFilenamesToArray(strings, path, _T("*.jpeg"));
		AddFilenamesToArray(strings, path, _T("*.png"));
		AddFilenamesToArray(strings, path, _T("*.tif"));
	}

	wxString result = wxGetSingleChoice(_("One of the following to add:"),
		_("Choose an image file"), strings, this);

	if (result.Cmp(_T(""))) // user selected something
	{
		TransferDataFromWindow();
		vtTagArray lay;
		lay.SetValueString("Type", TERR_LTYPE_IMAGE, true);
		lay.SetValueString("Filename", (const char *) result.mb_str(wxConvUTF8), true);
		m_Layers.push_back(lay);
		TransferDataToWindow();
	}
}

void TParamsDlg::OnChoiceLocFile( wxCommandEvent &event )
{
	if (m_bSetting || !m_bReady) return;

	wxString prev = m_strLocFile;
	TransferDataFromWindow();
	if (m_strLocFile != prev)
	{
		RefreshLocationFields();
		m_bSetting = true;
		TransferDataToWindow();
		m_bSetting = false;
	}
}

void TParamsDlg::OnChoiceInitLocation( wxCommandEvent &event )
{
	TransferDataFromWindow();
	m_strInitLocation = m_init_location->GetString(m_iInitLocation);
}

void TParamsDlg::OnSetInitTime( wxCommandEvent &event )
{
	TransferDataFromWindow();

	TimeDlg dlg(this, -1, _("Set Initial Time"));
	dlg.AddOkCancel();
	dlg.SetTime(m_InitTime);
	if (dlg.ShowModal() == wxID_OK)
	{
		dlg.GetTime(m_InitTime);
		UpdateTimeString();
		m_bSetting = true;
		TransferDataToWindow();
		m_bSetting = false;
	}
}

void TParamsDlg::OnStyle( wxCommandEvent &event )
{
	vtString str = (const char *) m_raw_files->GetStringSelection().mb_str(wxConvUTF8);
	int idx = FindLayerByFilename(str);
	if (idx == -1)
		return;

	StyleDlg dlg(this, -1, _("Feature Style"), wxDefaultPosition,
		wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	dlg.SetOptions(m_Layers[idx]);
	if (dlg.ShowModal() == wxID_OK)
	{
		dlg.GetOptions(m_Layers[idx]);
	}
}

void TParamsDlg::OnOverlay( wxCommandEvent &event )
{
	TransferDataFromWindow();

	wxString filter = _("Image Files|");
	AddType(filter, FSTRING_PNG);
	AddType(filter, FSTRING_JPEG);
	AddType(filter, FSTRING_BMP);
	wxFileDialog loadFile(NULL, _("Overlay Image File"), _T(""), _T(""),
		filter, wxFD_OPEN);
	if (m_strOverlayFile != _T(""))
		loadFile.SetPath(m_strOverlayFile);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	m_strOverlayFile = loadFile.GetPath();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void TParamsDlg::UpdateTimeString()
{
	m_strInitTime = wxString(asctime(&m_InitTime.GetTM()), wxConvLibc);

	// asctime has a weird habit of putting a LF at the end
	m_strInitTime.Trim();
}

void TParamsDlg::UpdateScenarioChoices()
{
	m_choice_scenario->Clear();
	for (uint i = 0; i < m_Scenarios.size(); i++)
	{
		vtString vs = m_Scenarios[i].GetValueString(STR_SCENARIO_NAME);
		m_choice_scenario->Append(wxString(vs, wxConvUTF8));
	}
	m_choice_scenario->SetStringSelection(m_strInitScenario);
}

void TParamsDlg::OnNewScenario( wxCommandEvent &event )
{
	wxString ScenarioName = wxGetTextFromUser(_("Enter Scenario Name"),
		_("New Scenario"));

	if (!ScenarioName.IsEmpty())
	{
		ScenarioParams Scenario;

		Scenario.SetValueString(STR_SCENARIO_NAME,
			(const char *) ScenarioName.mb_str(wxConvUTF8), true);
		m_Scenarios.push_back(Scenario);
		m_scenario_list->SetSelection(m_scenario_list->Append(ScenarioName));
		UpdateScenarioChoices();
		UpdateEnableState();
	}
}

void TParamsDlg::OnDeleteScenario( wxCommandEvent &event )
{
	int iSelected = m_scenario_list->GetSelection();

	if (iSelected != wxNOT_FOUND)
	{
		m_scenario_list->Delete(iSelected);
		m_Scenarios.erase(m_Scenarios.begin() + iSelected);
		UpdateScenarioChoices();
		UpdateEnableState();
	}
}

void TParamsDlg::OnEditScenario( wxCommandEvent &event )
{
	ScenarioParamsDialog ScenarioParamsDialog(this, -1, _("Scenario Parameters"));
	int iSelected = m_scenario_list->GetSelection();

	if (iSelected != wxNOT_FOUND)
	{
		vtStringArray lnames;
		for (uint i = 0; i < m_Layers.size(); i++)
			lnames.push_back(m_Layers[i].GetValueString("Filename"));

		ScenarioParamsDialog.SetAvailableLayers(lnames);
		ScenarioParamsDialog.SetParams(m_Scenarios[iSelected]);

		if (wxID_OK == ScenarioParamsDialog.ShowModal())
		{
			if (ScenarioParamsDialog.IsModified())
			{
				m_Scenarios[iSelected] = ScenarioParamsDialog.GetParams();
				wxString str(m_Scenarios[iSelected].GetValueString(STR_SCENARIO_NAME), wxConvUTF8);
				m_scenario_list->SetString(iSelected, str);
				UpdateScenarioChoices();
			}
		}
	}
}

void TParamsDlg::OnMoveUpScenario( wxCommandEvent &event )
{
	int iSelected = m_scenario_list->GetSelection();

	if ((iSelected != wxNOT_FOUND) && (iSelected != 0))
	{
		ScenarioParams TempParams = m_Scenarios[iSelected];
		wxString TempString = m_scenario_list->GetString(iSelected);
		m_scenario_list->Delete(iSelected);
// Bug in wxWidgets
//		m_scenario_list->SetSelection(m_scenario_list->Insert(TempString, iSelected - 1));
		m_scenario_list->Insert(TempString, iSelected - 1);
		m_scenario_list->SetSelection(iSelected - 1);
		m_Scenarios.erase(m_Scenarios.begin() + iSelected);
		m_Scenarios.insert(m_Scenarios.begin() + iSelected - 1,TempParams);
		UpdateEnableState();
	}
}

void TParamsDlg::OnMoveDownSceanario( wxCommandEvent &event )
{
	int iSelected = m_scenario_list->GetSelection();

	if ((iSelected != wxNOT_FOUND) && (iSelected != (m_scenario_list->GetCount() - 1)))
	{
		ScenarioParams TempParams = m_Scenarios[iSelected];
		wxString TempString = m_scenario_list->GetString(iSelected);
		m_scenario_list->Delete(iSelected);
// Bug in wxWidgets
//		m_scenario_list->SetSelection(m_scenario_list->Insert(TempString, iSelected + 1));
		m_scenario_list->Insert(TempString, iSelected + 1);
		m_scenario_list->SetSelection(iSelected + 1);
		m_Scenarios.erase(m_Scenarios.begin() + iSelected);
		m_Scenarios.insert(m_Scenarios.begin() + iSelected + 1,TempParams);
		UpdateEnableState();
	}
}

void TParamsDlg::OnScenarioListEvent( wxCommandEvent &event )
{
	UpdateEnableState();
}

void TParamsDlg::OnChoiceScenario( wxCommandEvent &event )
{
	m_strInitScenario = m_choice_scenario->GetStringSelection();
}

void TParamsDlg::OnPrimaryTexture( wxCommandEvent &event )
{
	// Show texture dialog
	TextureDlg dlg(this, -1, _("Primary Texture"));
	dlg.SetParams(m_Params);
	if (dlg.ShowModal() == wxID_OK)
	{
		dlg.GetParams(m_Params);
	}
}

void TParamsDlg::OnSetTinTexture( wxCommandEvent &event )
{
	// Show texture dialog for a primary TIN surface
	TinTextureDlg dlg(this, -1, _("TIN Texture"));
	dlg.SetOptions(m_Params);
	if (dlg.ShowModal() == wxID_OK)
	{
		dlg.GetOptions(m_Params);
	}
}

void TParamsDlg::OnSetTexture( wxCommandEvent &event )
{
	// Show texture dialog for an elevation layer (TIN surface)
	vtString str = (const char *) m_elev_files->GetStringSelection().mb_str(wxConvUTF8);
	int idx = FindLayerByFilename(str);
	if (idx == -1)
		return;

	TinTextureDlg dlg(this, -1, _("TIN Texture"));
	dlg.SetOptions(m_Layers[idx]);
	if (dlg.ShowModal() == wxID_OK)
	{
		dlg.GetOptions(m_Layers[idx]);
	}
}


