//
// Name:	 EnviroMenus.cpp
// Purpose:  Menu methods for the frame class for the wxEnviro application.
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/numdlg.h"

#ifdef VTUNIX
#include <unistd.h>
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/Contours.h"
#include "vtlib/core/Fence3d.h"
#include "vtlib/core/SkyDome.h"
#include "vtlib/vtosg/SaveImageOSG.h"

#include "vtdata/vtLog.h"
#include "vtdata/FileFilters.h"
#include "vtdata/TripDub.h"
#include "vtdata/Version.h"	// for About box
#include "vtui/Helper.h"	// for progress dialog

#include "EnviroFrame.h"

// dialogs
#include "CameraDlg.h"
#include "DistanceDlg3d.h"
#include "DriveDlg.h"
#include "EphemDlg.h"
#include "FeatureTableDlg3d.h"
#include "LayerDlg.h"
#include "LinearStructDlg3d.h"
#include "LocationDlg.h"
#include "LODDlg.h"
#include "PlantDlg.h"
#include "ScenarioSelectDialog.h"
#include "TextureDlg.h"
#include "UtilDlg.h"
#include "VehicleDlg.h"
#include "VIADlg.h"
#include "VIAGDALOptionsDlg.h"

#include "vtui/ContourDlg.h"
#include "vtui/InstanceDlg.h"
#include "vtui/ProfileDlg.h"
#include "vtui/SizeDlg.h"
#include "vtui/TagDlg.h"

#include "wxosg/BuildingDlg3d.h"
#include "wxosg/SceneGraphDlg.h"
#include "wxosg/TimeDlg.h"

#include "../Options.h"
#include "EnviroGUI.h"	// for GetCurrentTerrain

#include "EnviroApp.h"
#include "EnviroCanvas.h"
#include "menu_id.h"

#if VTP_VISUAL_IMPACT_CALCULATOR
#include "gdal_priv.h"
#endif

DECLARE_APP(EnviroApp);

BEGIN_EVENT_TABLE(EnviroFrame, wxFrame)
EVT_CHAR(EnviroFrame::OnChar)
EVT_MENU(ID_FILE_LAYERS,			EnviroFrame::OnFileLayers)
EVT_MENU(ID_FILE_LAYER_CREATE,		EnviroFrame::OnLayerCreate)
EVT_UPDATE_UI(ID_FILE_LAYER_CREATE,	EnviroFrame::OnUpdateLayerCreate)
EVT_MENU(ID_FILE_LAYER_LOAD,		EnviroFrame::OnLayerLoad)
EVT_UPDATE_UI(ID_FILE_LAYER_LOAD,	EnviroFrame::OnUpdateLayerLoad)
EVT_MENU(ID_FILE_SAVE_TERRAIN,		EnviroFrame::OnSaveTerrain)
EVT_MENU(ID_FILE_SAVE_TERRAIN_AS,	EnviroFrame::OnSaveTerrainAs)
EVT_UPDATE_UI(ID_FILE_SAVE_TERRAIN,	EnviroFrame::OnUpdateSaveTerrain)
EVT_MENU(wxID_EXIT,					EnviroFrame::OnExit)
EVT_CLOSE(EnviroFrame::OnClose)
EVT_IDLE(EnviroFrame::OnIdle)

EVT_MENU(ID_TOOLS_SELECT,			EnviroFrame::OnToolsSelect)
EVT_UPDATE_UI(ID_TOOLS_SELECT,		EnviroFrame::OnUpdateToolsSelect)
EVT_MENU(ID_TOOLS_SELECT_BOX,		EnviroFrame::OnToolsSelectBox)
EVT_UPDATE_UI(ID_TOOLS_SELECT_BOX,	EnviroFrame::OnUpdateToolsSelectBox)
EVT_MENU(ID_TOOLS_SELECT_MOVE,		EnviroFrame::OnToolsSelectMove)
EVT_UPDATE_UI(ID_TOOLS_SELECT_MOVE,	EnviroFrame::OnUpdateToolsSelectMove)
EVT_MENU(ID_TOOLS_FENCES,			EnviroFrame::OnToolsFences)
EVT_UPDATE_UI(ID_TOOLS_FENCES,		EnviroFrame::OnUpdateToolsFences)
EVT_MENU(ID_TOOLS_BUILDINGS,		EnviroFrame::OnToolsBuildings)
EVT_UPDATE_UI(ID_TOOLS_BUILDINGS,	EnviroFrame::OnUpdateToolsBuildings)
EVT_MENU(ID_TOOLS_POWER,			EnviroFrame::OnToolsPower)
EVT_UPDATE_UI(ID_TOOLS_POWER,		EnviroFrame::OnUpdateToolsPower)
EVT_MENU(ID_TOOLS_PLANTS,			EnviroFrame::OnToolsPlants)
EVT_UPDATE_UI(ID_TOOLS_PLANTS,		EnviroFrame::OnUpdateToolsPlants)
EVT_MENU(ID_TOOLS_POINTS,			EnviroFrame::OnToolsPoints)
EVT_UPDATE_UI(ID_TOOLS_POINTS,		EnviroFrame::OnUpdateToolsPoints)
EVT_MENU(ID_TOOLS_INSTANCES,		EnviroFrame::OnToolsInstances)
EVT_UPDATE_UI(ID_TOOLS_INSTANCES,	EnviroFrame::OnUpdateToolsInstances)
EVT_MENU(ID_TOOLS_VEHICLES,			EnviroFrame::OnToolsVehicles)
EVT_UPDATE_UI(ID_TOOLS_VEHICLES,	EnviroFrame::OnUpdateToolsVehicles)
EVT_MENU(ID_TOOLS_MOVE,				EnviroFrame::OnToolsMove)
EVT_UPDATE_UI(ID_TOOLS_MOVE,		EnviroFrame::OnUpdateToolsMove)
EVT_MENU(ID_TOOLS_NAVIGATE,			EnviroFrame::OnToolsNavigate)
EVT_UPDATE_UI(ID_TOOLS_NAVIGATE,	EnviroFrame::OnUpdateToolsNavigate)
EVT_MENU(ID_TOOLS_MEASURE,			EnviroFrame::OnToolsMeasure)
EVT_UPDATE_UI(ID_TOOLS_MEASURE,		EnviroFrame::OnUpdateToolsMeasure)
EVT_MENU(ID_TOOLS_CONSTRAIN,		EnviroFrame::OnToolsConstrain)
EVT_UPDATE_UI(ID_TOOLS_CONSTRAIN,	EnviroFrame::OnUpdateToolsConstrain)
// Visual impact submenu
EVT_MENU(ID_VIA_CALCULATE, EnviroFrame::OnVIACalculate)
EVT_UPDATE_UI(ID_VIA_CALCULATE, EnviroFrame::OnUpdateVIACalculate)
EVT_MENU(ID_VIA_PLOT, EnviroFrame::OnVIAPlot)
EVT_UPDATE_UI(ID_VIA_PLOT, EnviroFrame::OnUpdateVIAPlot)
EVT_MENU(ID_VIA_CLEAR, EnviroFrame::OnVIAClear)
EVT_UPDATE_UI(ID_VIA_CLEAR, EnviroFrame::OnUpdateVIAClear)

EVT_MENU(ID_VIEW_MAINTAIN,			EnviroFrame::OnViewMaintain)
EVT_UPDATE_UI(ID_VIEW_MAINTAIN,		EnviroFrame::OnUpdateViewMaintain)
EVT_MENU(ID_VIEW_WIREFRAME,			EnviroFrame::OnViewWireframe)
EVT_UPDATE_UI(ID_VIEW_WIREFRAME,	EnviroFrame::OnUpdateViewWireframe)
EVT_MENU(ID_VIEW_FULLSCREEN,		EnviroFrame::OnViewFullscreen)
EVT_UPDATE_UI(ID_VIEW_FULLSCREEN,	EnviroFrame::OnUpdateViewFullscreen)
EVT_MENU(ID_VIEW_TOPDOWN,			EnviroFrame::OnViewTopDown)
EVT_UPDATE_UI(ID_VIEW_TOPDOWN,		EnviroFrame::OnUpdateViewTopDown)
EVT_MENU(ID_VIEW_STATS,				EnviroFrame::OnViewStats)
EVT_MENU(ID_VIEW_ELEV_LEGEND,		EnviroFrame::OnViewElevLegend)
EVT_UPDATE_UI(ID_VIEW_ELEV_LEGEND,	EnviroFrame::OnUpdateViewElevLegend)
EVT_MENU(ID_VIEW_COMPASS,			EnviroFrame::OnViewCompass)
EVT_UPDATE_UI(ID_VIEW_COMPASS,		EnviroFrame::OnUpdateViewCompass)
EVT_MENU(ID_VIEW_MAP_OVERVIEW,		EnviroFrame::OnViewMapOverView)
EVT_UPDATE_UI(ID_VIEW_MAP_OVERVIEW,	EnviroFrame::OnUpdateViewMapOverView)
EVT_MENU(ID_VIEW_VERT_LINE,			EnviroFrame::OnViewVertLine)
EVT_UPDATE_UI(ID_VIEW_VERT_LINE,	EnviroFrame::OnUpdateViewVertLine)
EVT_MENU(ID_VIEW_DRIVE,				EnviroFrame::OnViewDrive)
EVT_UPDATE_UI(ID_VIEW_DRIVE,		EnviroFrame::OnUpdateViewDrive)
EVT_MENU(ID_VIEW_SETTINGS,			EnviroFrame::OnViewSettings)
EVT_MENU(ID_VIEW_LOCATIONS,			EnviroFrame::OnViewLocations)
EVT_UPDATE_UI(ID_VIEW_LOCATIONS,	EnviroFrame::OnUpdateViewLocations)
EVT_MENU(ID_VIEW_RESET,				EnviroFrame::OnViewReset)
EVT_MENU(ID_VIEW_SNAPSHOT,			EnviroFrame::OnViewSnapshot)
EVT_MENU(ID_VIEW_SNAP_AGAIN,		EnviroFrame::OnViewSnapAgain)
EVT_MENU(ID_VIEW_SNAP_HIGH,			EnviroFrame::OnViewSnapHigh)
EVT_MENU(ID_VIEW_STATUSBAR,			EnviroFrame::OnViewStatusBar)
EVT_UPDATE_UI(ID_VIEW_STATUSBAR,	EnviroFrame::OnUpdateViewStatusBar)
EVT_MENU(ID_VIEW_SCENARIOS,			EnviroFrame::OnViewScenarios)
EVT_MENU(ID_VIEW_PROFILE,			EnviroFrame::OnViewProfile)
EVT_UPDATE_UI(ID_VIEW_PROFILE,		EnviroFrame::OnUpdateViewProfile)

EVT_MENU(ID_VIEW_SLOWER,		EnviroFrame::OnViewSlower)
EVT_UPDATE_UI(ID_VIEW_SLOWER,	EnviroFrame::OnUpdateViewSlower)
EVT_MENU(ID_VIEW_FASTER,		EnviroFrame::OnViewFaster)
EVT_UPDATE_UI(ID_VIEW_FASTER,	EnviroFrame::OnUpdateViewFaster)
EVT_MENU(ID_NAV_NORMAL,			EnviroFrame::OnNavNormal)
EVT_UPDATE_UI(ID_NAV_NORMAL,	EnviroFrame::OnUpdateNavNormal)
EVT_MENU(ID_NAV_VELO,			EnviroFrame::OnNavVelo)
EVT_UPDATE_UI(ID_NAV_VELO,		EnviroFrame::OnUpdateNavVelo)
EVT_MENU(ID_NAV_GRAB_PIVOT,		EnviroFrame::OnNavGrabPivot)
EVT_UPDATE_UI(ID_NAV_GRAB_PIVOT,EnviroFrame::OnUpdateNavGrabPivot)
EVT_MENU(ID_NAV_PANO,			EnviroFrame::OnNavPano)
EVT_UPDATE_UI(ID_NAV_PANO,		EnviroFrame::OnUpdateNavPano)

EVT_MENU(ID_VIEW_TOOLS_CULTURE,			EnviroFrame::OnViewToolCulture)
EVT_MENU(ID_VIEW_TOOLS_SNAPSHOT,		EnviroFrame::OnViewToolSnapshot)
EVT_MENU(ID_VIEW_TOOLS_TIME,			EnviroFrame::OnViewToolTime)
EVT_UPDATE_UI(ID_VIEW_TOOLS_CULTURE,	EnviroFrame::OnUpdateViewToolCulture)
EVT_UPDATE_UI(ID_VIEW_TOOLS_SNAPSHOT,	EnviroFrame::OnUpdateViewToolSnapshot)
EVT_UPDATE_UI(ID_VIEW_TOOLS_TIME,		EnviroFrame::OnUpdateViewToolTime)

EVT_MENU(ID_SCENE_SCENEGRAPH,	EnviroFrame::OnSceneGraph)
EVT_MENU(ID_SCENE_TERRAIN,		EnviroFrame::OnSceneTerrain)
EVT_UPDATE_UI(ID_SCENE_TERRAIN,	EnviroFrame::OnUpdateSceneTerrain)
EVT_MENU(ID_SCENE_SPACE,		EnviroFrame::OnSceneSpace)
EVT_UPDATE_UI(ID_SCENE_SPACE,	EnviroFrame::OnUpdateSceneSpace)
EVT_MENU(ID_SCENE_SAVE,			EnviroFrame::OnSceneSave)
EVT_MENU(ID_SCENE_EPHEMERIS,	EnviroFrame::OnSceneEphemeris)
#ifdef VTP_NVIDIA_PERFORMANCE_MONITORING
EVT_MENU(ID_SCENE_PERFMON,	    EnviroFrame::OnPerformanceMonitor)
#endif
EVT_MENU(ID_TIME_DIALOG,		EnviroFrame::OnTimeDialog)
EVT_MENU(ID_TIME_STOP,			EnviroFrame::OnTimeStop)
EVT_MENU(ID_TIME_FASTER,		EnviroFrame::OnTimeFaster)

EVT_UPDATE_UI(ID_TIME_DIALOG,	EnviroFrame::OnUpdateInOrbitOrTerrain)
EVT_UPDATE_UI(ID_TIME_STOP,		EnviroFrame::OnUpdateInOrbitOrTerrain)
EVT_UPDATE_UI(ID_TIME_FASTER,	EnviroFrame::OnUpdateInOrbitOrTerrain)

EVT_MENU(ID_TERRAIN_SURFACE,	EnviroFrame::OnSurface)
EVT_MENU(ID_TERRAIN_CULLEVERY,	EnviroFrame::OnCullEvery)
EVT_MENU(ID_TERRAIN_CULLONCE,	EnviroFrame::OnCullOnce)
EVT_MENU(ID_TERRAIN_SKY,		EnviroFrame::OnSky)
EVT_MENU(ID_TERRAIN_OCEAN,		EnviroFrame::OnOcean)
EVT_MENU(ID_TERRAIN_PLANTS,		EnviroFrame::OnPlants)
EVT_MENU(ID_TERRAIN_STRUCTURES,	EnviroFrame::OnStructures)
EVT_MENU(ID_TERRAIN_ROADS,		EnviroFrame::OnRoads)
EVT_MENU(ID_TERRAIN_FOG,		EnviroFrame::OnFog)
EVT_MENU(ID_TERRAIN_INCREASE,	EnviroFrame::OnIncrease)
EVT_MENU(ID_TERRAIN_DECREASE,	EnviroFrame::OnDecrease)
EVT_MENU(ID_TERRAIN_LOD,		EnviroFrame::OnLOD)
EVT_MENU(ID_TERRAIN_FOUNDATIONS, EnviroFrame::OnToggleFoundations)
EVT_MENU(ID_TERRAIN_RESHADE,	EnviroFrame::OnTerrainReshade)
EVT_MENU(ID_TERRAIN_CHANGE_TEXTURE,	EnviroFrame::OnTerrainChangeTexture)
EVT_MENU(ID_TERRAIN_DISTRIB_VEHICLES,	EnviroFrame::OnTerrainDistribVehicles)
EVT_MENU(ID_TERRAIN_WRITE_ELEVATION,	EnviroFrame::OnTerrainWriteElevation)
EVT_MENU(ID_TERRAIN_ADD_CONTOUR,	EnviroFrame::OnTerrainAddContour)

EVT_UPDATE_UI(ID_TERRAIN_SURFACE,	EnviroFrame::OnUpdateSurface)
EVT_UPDATE_UI(ID_TERRAIN_CULLEVERY, EnviroFrame::OnUpdateCullEvery)
EVT_UPDATE_UI(ID_TERRAIN_CULLONCE,	EnviroFrame::OnUpdateIsDynTerrain)
EVT_UPDATE_UI(ID_TERRAIN_SKY,		EnviroFrame::OnUpdateSky)
EVT_UPDATE_UI(ID_TERRAIN_OCEAN,		EnviroFrame::OnUpdateOcean)
EVT_UPDATE_UI(ID_TERRAIN_PLANTS,	EnviroFrame::OnUpdatePlants)
EVT_UPDATE_UI(ID_TERRAIN_STRUCTURES, EnviroFrame::OnUpdateStructures)
EVT_UPDATE_UI(ID_TERRAIN_ROADS,		EnviroFrame::OnUpdateRoads)
EVT_UPDATE_UI(ID_TERRAIN_FOG,		EnviroFrame::OnUpdateFog)
EVT_UPDATE_UI(ID_TERRAIN_INCREASE,	EnviroFrame::OnUpdateLOD)
EVT_UPDATE_UI(ID_TERRAIN_DECREASE,	EnviroFrame::OnUpdateLOD)
EVT_UPDATE_UI(ID_TERRAIN_LOD,		EnviroFrame::OnUpdateLOD)
EVT_UPDATE_UI(ID_TERRAIN_FOUNDATIONS, EnviroFrame::OnUpdateFoundations)
EVT_UPDATE_UI(ID_TERRAIN_RESHADE,	EnviroFrame::OnUpdateIsDynTerrain)
EVT_UPDATE_UI(ID_TERRAIN_CHANGE_TEXTURE, EnviroFrame::OnUpdateIsDynTerrain)
EVT_UPDATE_UI(ID_TERRAIN_WRITE_ELEVATION,	EnviroFrame::OnUpdateIsDynTerrain)

EVT_MENU(ID_EARTH_SHOWSHADING,	EnviroFrame::OnEarthShowShading)
EVT_MENU(ID_EARTH_SHOWAXES,		EnviroFrame::OnEarthShowAxes)
EVT_MENU(ID_EARTH_TILT,			EnviroFrame::OnEarthTilt)
EVT_MENU(ID_EARTH_FLATTEN,		EnviroFrame::OnEarthFlatten)
EVT_MENU(ID_EARTH_UNFOLD,		EnviroFrame::OnEarthUnfold)
EVT_MENU(ID_EARTH_CLOUDS,		EnviroFrame::OnEarthClouds)
EVT_MENU(ID_EARTH_CLOUDS2,		EnviroFrame::OnEarthClouds2)

EVT_UPDATE_UI(ID_EARTH_SHOWSHADING, EnviroFrame::OnUpdateEarthShowShading)
EVT_UPDATE_UI(ID_EARTH_SHOWAXES, EnviroFrame::OnUpdateEarthShowAxes)
EVT_UPDATE_UI(ID_EARTH_TILT,	EnviroFrame::OnUpdateEarthTilt)
EVT_UPDATE_UI(ID_EARTH_FLATTEN, EnviroFrame::OnUpdateEarthFlatten)
EVT_UPDATE_UI(ID_EARTH_UNFOLD,	EnviroFrame::OnUpdateEarthUnfold)
EVT_UPDATE_UI(ID_EARTH_CLOUDS,	EnviroFrame::OnUpdateEarthClouds)
EVT_UPDATE_UI(ID_EARTH_CLOUDS2,	EnviroFrame::OnUpdateEarthClouds)

EVT_MENU(ID_HELP_ABOUT, EnviroFrame::OnHelpAbout)
EVT_MENU(ID_HELP_DOC_LOCAL, EnviroFrame::OnHelpDocLocal)
EVT_MENU(ID_HELP_DOC_ONLINE, EnviroFrame::OnHelpDocOnline)

// Popup
EVT_MENU(ID_POPUP_PROPERTIES, EnviroFrame::OnPopupProperties)
EVT_MENU(ID_POPUP_FLIP, EnviroFrame::OnPopupFlip)
EVT_MENU(ID_POPUP_SET_EAVES, EnviroFrame::OnPopupSetEaves)
EVT_MENU(ID_POPUP_COPY_STYLE, EnviroFrame::OnPopupCopyStyle)
EVT_MENU(ID_POPUP_PASTE_STYLE, EnviroFrame::OnPopupPasteStyle)
EVT_MENU(ID_POPUP_RELOAD, EnviroFrame::OnPopupReload)
EVT_MENU(ID_POPUP_SHADOW, EnviroFrame::OnPopupShadow)
EVT_MENU(ID_POPUP_ADJUST, EnviroFrame::OnPopupAdjust)
EVT_MENU(ID_POPUP_START, EnviroFrame::OnPopupStart)
EVT_MENU(ID_POPUP_DELETE, EnviroFrame::OnPopupDelete)
EVT_MENU(ID_POPUP_URL, EnviroFrame::OnPopupURL)
EVT_MENU(ID_POPUP_VIA, EnviroFrame::OnPopupVIA)
EVT_UPDATE_UI(ID_POPUP_VIA, EnviroFrame::OnUpdatePopupVIA)
EVT_MENU(ID_POPUP_VIA_TARGET, EnviroFrame::OnPopupVIATarget)
EVT_UPDATE_UI(ID_POPUP_VIA_TARGET, EnviroFrame::OnUpdatePopupVIATarget)
END_EVENT_TABLE()

void EnviroFrame::CreateMenus()
{
	// Make menu bar
	m_pMenuBar = new wxMenuBar;

	// Make menus
	m_pFileMenu = new wxMenu;
	m_pFileMenu->Append(ID_FILE_LAYERS, _("Layers"), _("Layers"));
	m_pFileMenu->AppendSeparator();
	m_pFileMenu->Append(ID_FILE_LAYER_CREATE, _("Create Layer"), _("Create Layer"));
	m_pFileMenu->Append(ID_FILE_LAYER_LOAD, _("Load Layer"), _("Load Layer"));
	m_pFileMenu->AppendSeparator();
	m_pFileMenu->Append(ID_FILE_SAVE_TERRAIN, _("Save Terrain State"));
	m_pFileMenu->Append(ID_FILE_SAVE_TERRAIN_AS, _("Save Terrain State As..."));
	m_pFileMenu->AppendSeparator();
	m_pFileMenu->Append(wxID_EXIT, _("E&xit (Esc)"), _("Exit"));
	m_pMenuBar->Append(m_pFileMenu, _("&File"));

	m_pToolsMenu = new wxMenu;
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_SELECT, _("Select"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_SELECT_BOX, _("Select Box"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_SELECT_MOVE, _("Select and Move"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_FENCES, _("Fences"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_BUILDINGS, _("Buildings"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_POWER, _("Powerlines"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_PLANTS, _("Plants"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_POINTS, _("Points"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_INSTANCES, _("Instances"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_VEHICLES, _("Vehicles"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_MOVE, _("Move Objects"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_NAVIGATE, _("Navigate"));
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_MEASURE, _("Measure Distances\tCtrl+D"));
	m_pToolsMenu->AppendSeparator();
	m_pToolsMenu->AppendCheckItem(ID_TOOLS_CONSTRAIN, _("Constrain building angles"));
#ifdef VTP_VISUAL_IMPACT_CALCULATOR
	wxMenu *pVIAMenu = new wxMenu;
	pVIAMenu->Append(ID_VIA_CALCULATE, _("&Calculate\tCtrl+C"), _("Calculate visual impact factor"));
	pVIAMenu->Append(ID_VIA_PLOT, _("&Plot\tCtrl+P"), _("Produce visual impact plot"));
	pVIAMenu->Append(ID_VIA_CLEAR, _("C&lear"), _("Clear visual impact target and all contributors"));
	m_pToolsMenu->AppendSeparator();
	m_pToolsMenu->Append(ID_VIA_MENU, _("Visual Impact.."), pVIAMenu);
#endif
	m_pMenuBar->Append(m_pToolsMenu, _("&Tools"));

	// shortcuts:
	// Ctrl+A Show Axes
	// Ctrl+D Measure Distances
	// Ctrl+E Flatten
	// Ctrl+F Fullscreen
	// Ctrl+G Go to Terrain
	// Ctrl+I Time
	// Ctrl+L Store/Recall Locations
	// Ctrl+N Save Numbered Snapshot
	// Ctrl+P ePhemeris
	// Ctrl+Q Terrain LOD Info
	// Ctrl+R Reset Camera
	// Ctrl+S Camera - View Settings
	// Ctrl+T Top-Down
	// Ctrl+U Unfold
	// Ctrl+W Wireframe
	//
	// A Maintain height above ground
	// D Toggle Grab-Pivot
	// F Faster
	// S Faster
	// W Navigate w/o mouse button

	m_pSceneMenu = new wxMenu;
	m_pSceneMenu->Append(ID_SCENE_SCENEGRAPH, _("Scene Graph"));
    #ifdef VTP_NVIDIA_PERFORMANCE_MONITORING
    m_pSceneMenu->Append(ID_SCENE_PERFMON, _("Performance Monitor"));
    #endif
	m_pSceneMenu->AppendSeparator();
	m_pSceneMenu->Append(ID_SCENE_TERRAIN, _("Go to Terrain...\tCtrl+G"));
	if (m_bEnableEarth)
	{
		m_pSceneMenu->Append(ID_SCENE_SPACE, _("Go to Space"));
	}
	m_pSceneMenu->AppendSeparator();
	m_pSceneMenu->Append(ID_SCENE_SAVE, _("Save scene graph to .osg"));
	m_pSceneMenu->AppendSeparator();
	m_pSceneMenu->Append(ID_SCENE_EPHEMERIS, _("Ephemeris...\tCtrl+P"));
	m_pSceneMenu->Append(ID_TIME_DIALOG, _("Time...\tCtrl+I"));
	m_pSceneMenu->Append(ID_TIME_STOP, _("Time Stop"));
	m_pSceneMenu->Append(ID_TIME_FASTER, _("Time Faster"));
    m_pMenuBar->Append(m_pSceneMenu, _("&Scene"));

	m_pViewMenu = new wxMenu;
	m_pViewMenu->AppendCheckItem(ID_VIEW_WIREFRAME, _("Wireframe\tCtrl+W"));
	m_pViewMenu->AppendCheckItem(ID_VIEW_FULLSCREEN, _("Fullscreen\tCtrl+F"));
	m_pViewMenu->AppendCheckItem(ID_VIEW_TOPDOWN, _("Top-Down Camera\tCtrl+T"));
	m_pViewMenu->Append(ID_VIEW_STATS, _("Rendering Statistics\tx"));
	m_pViewMenu->AppendCheckItem(ID_VIEW_ELEV_LEGEND, _("Elevation Legend"));
	m_pViewMenu->AppendCheckItem(ID_VIEW_COMPASS, _("Compass"));
	m_pViewMenu->AppendCheckItem(ID_VIEW_MAP_OVERVIEW, _("Overview"));
	m_pViewMenu->AppendCheckItem(ID_VIEW_VERT_LINE, _("Vertical Measure Line"));
	m_pViewMenu->AppendCheckItem(ID_VIEW_DRIVE, _("Drive Vehicle"));
	m_pViewMenu->AppendSeparator();
	m_pViewMenu->Append(ID_VIEW_SETTINGS, _("Camera - View Settings\tCtrl+S"));
	m_pViewMenu->Append(ID_VIEW_LOCATIONS, _("Store/Recall Locations\tCtrl+L"));
	m_pViewMenu->Append(ID_VIEW_RESET, _("Reset to Terrain Center\tCtrl+R"));
	m_pViewMenu->AppendSeparator();
	m_pViewMenu->Append(ID_VIEW_SNAPSHOT, _("Save Window Snapshot"));
	m_pViewMenu->Append(ID_VIEW_SNAP_AGAIN, _("Save Numbered Snapshot\tCtrl+N"));
	m_pViewMenu->Append(ID_VIEW_SNAP_HIGH, _("High-resolution Snapshot"));
	m_pViewMenu->AppendSeparator();
	m_pViewMenu->AppendCheckItem(ID_VIEW_STATUSBAR, _("&Status Bar"));
	m_pViewMenu->Append(ID_VIEW_SCENARIOS, _("Scenarios"));
	m_pMenuBar->Append(m_pViewMenu, _("&View"));

		// submenu
		wxMenu *tbMenu = new wxMenu;
		tbMenu->AppendCheckItem(ID_VIEW_TOOLS_CULTURE, _("Culture"));
		tbMenu->AppendCheckItem(ID_VIEW_TOOLS_SNAPSHOT, _("Snapshot"));
		tbMenu->AppendCheckItem(ID_VIEW_TOOLS_TIME, _("Time"));
		m_pViewMenu->Append(0, _("Toolbars"), tbMenu);

	m_pNavMenu = new wxMenu;
	m_pNavMenu->Append(ID_VIEW_SLOWER, _("Fly Slower (S)"));
	m_pNavMenu->Append(ID_VIEW_FASTER, _("Fly Faster (F)"));
	m_pNavMenu->AppendCheckItem(ID_VIEW_MAINTAIN, _("Maintain height above ground (A)"));
	m_pMenuBar->Append(m_pNavMenu, _("&Navigate"));

		// submenu
		wxMenu *navstyleMenu = new wxMenu;
		navstyleMenu->AppendCheckItem(ID_NAV_NORMAL, _("Normal Terrain Flyer"));
		navstyleMenu->AppendCheckItem(ID_NAV_VELO, _("Flyer with Velocity"));
		navstyleMenu->AppendCheckItem(ID_NAV_GRAB_PIVOT, _("Grab-Pivot"));
//		navstyleMenu->AppendCheckItem(ID_NAV_QUAKE, _T("Keyboard Walk"));
		navstyleMenu->AppendCheckItem(ID_NAV_PANO, _("Panoramic Flyer"));
		m_pNavMenu->Append(0, _("Navigation Style"), navstyleMenu);

	m_pTerrainMenu = new wxMenu;
	m_pTerrainMenu->AppendCheckItem(ID_TERRAIN_SURFACE, _("Show Terrain Surface\tF3"));
	m_pTerrainMenu->AppendCheckItem(ID_TERRAIN_SKY, _("Show Sky\tF4"));
	m_pTerrainMenu->AppendCheckItem(ID_TERRAIN_OCEAN, _("Show Ocean Plane\tF5"));
	m_pTerrainMenu->AppendCheckItem(ID_TERRAIN_PLANTS, _("Show Plants\tF6"));
	m_pTerrainMenu->AppendCheckItem(ID_TERRAIN_STRUCTURES, _("Show Structures\tF7"));
	m_pTerrainMenu->AppendCheckItem(ID_TERRAIN_ROADS, _("Show Roads\tF8"));
	m_pTerrainMenu->AppendCheckItem(ID_TERRAIN_FOG, _("Show Fog\tF9"));
	m_pTerrainMenu->AppendSeparator();
	m_pTerrainMenu->Append(ID_TERRAIN_INCREASE, _("Increase Detail (+)"));
	m_pTerrainMenu->Append(ID_TERRAIN_DECREASE, _("Decrease Detail (-)"));
	m_pTerrainMenu->Append(ID_TERRAIN_LOD, _("Level of Detail Info\tCtrl+Q"));
	m_pTerrainMenu->AppendSeparator();
	m_pTerrainMenu->AppendCheckItem(ID_TERRAIN_FOUNDATIONS, _("Toggle Artificial Foundations"));
	m_pTerrainMenu->Append(ID_TERRAIN_RESHADE, _("&Recalculate Shading\tCtrl+H"));
	m_pTerrainMenu->Append(ID_TERRAIN_CHANGE_TEXTURE, _("&Change Texture"));
	m_pTerrainMenu->Append(ID_TERRAIN_DISTRIB_VEHICLES, _("&Distribute Vehicles (test)"));
	m_pTerrainMenu->Append(ID_TERRAIN_WRITE_ELEVATION, _("Write Elevation to BT"));
	m_pTerrainMenu->Append(ID_TERRAIN_ADD_CONTOUR, _("Add Contours"));
	m_pTerrainMenu->AppendSeparator();
	m_pTerrainMenu->AppendCheckItem(ID_TERRAIN_CULLEVERY, _("Cull every frame"));
	m_pTerrainMenu->Append(ID_TERRAIN_CULLONCE, _("Cull once"));
	m_pMenuBar->Append(m_pTerrainMenu, _("Te&rrain"));

	if (m_bEnableEarth)
	{
		m_pEarthMenu = new wxMenu;
		m_pEarthMenu->AppendCheckItem(ID_EARTH_SHOWSHADING, _("&Show Shading"));
		m_pEarthMenu->AppendCheckItem(ID_EARTH_SHOWAXES, _("Show &Axes\tCtrl+A"));
		m_pEarthMenu->AppendCheckItem(ID_EARTH_TILT, _("Seasonal &Tilt"));
		m_pEarthMenu->AppendCheckItem(ID_EARTH_FLATTEN, _("&Flatten\tCtrl+E"));
		m_pEarthMenu->AppendCheckItem(ID_EARTH_UNFOLD, _("&Unfold\tCtrl+U"));
		m_pEarthMenu->Append(ID_EARTH_CLOUDS, _T("Load Clouds..."));
#if SUPPORT_CURL
		m_pEarthMenu->Append(ID_EARTH_CLOUDS2, _T("Load Clouds from URL..."));
#endif
		m_pMenuBar->Append(m_pEarthMenu, _("&Earth"));
	}

	wxMenu *helpMenu = new wxMenu;
	wxString about = _("About");
	about += _T(" ");
	about += wxString(STRING_APPORG, wxConvUTF8);
	about += _T("...");
	helpMenu->Append(ID_HELP_ABOUT, about);
	helpMenu->Append(ID_HELP_DOC_LOCAL, _("Documentation (local)"));
	helpMenu->Append(ID_HELP_DOC_ONLINE, _("Documentation (online)"));
	m_pMenuBar->Append(helpMenu, _("&Help"));

	SetMenuBar(m_pMenuBar);
}

/////////////////////////////////////////////////////////////////////////////
//
// Handle the menu commands
//

void EnviroFrame::OnExit(wxCommandEvent& event)
{
	VTLOG1("Got Exit event.\n");
	Close(false);	// False means: don't force a close.
}

//////////////////// File menu //////////////////////////

void EnviroFrame::OnFileLayers(wxCommandEvent& event)
{
	wxAuiPaneInfo &info = m_mgr.GetPane(m_pLayerDlg);
	info.Show(!info.IsShown());
	m_mgr.Update();
}

void EnviroFrame::OnLayerCreate(wxCommandEvent& event)
{
	m_pLayerDlg->OnLayerCreate(event);
}

void EnviroFrame::OnUpdateLayerCreate(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
}

void EnviroFrame::OnLayerLoad(wxCommandEvent& event)
{
	m_pLayerDlg->OnLayerLoad(event);
}

void EnviroFrame::OnUpdateLayerLoad(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain || g_App.m_state == AS_Orbit);
}

void EnviroFrame::OnSaveTerrain(wxCommandEvent& event)
{
	// Copy as much state as possible from the active terrain to its parameters
	g_App.StoreTerrainParameters();

	vtTerrain *terr = g_App.GetCurrentTerrain();
	TParams &par = terr->GetParams();
	vtString fname = terr->GetParamFile();
	if (!par.WriteToXML(fname, STR_TPARAMS_FORMAT_NAME))
		DisplayAndLog("Couldn't write file '%s'", (const char *)fname);
}

void EnviroFrame::OnSaveTerrainAs(wxCommandEvent& event)
{
	// Copy as much state as possible from the active terrain to its parameters
	g_App.StoreTerrainParameters();

	vtTerrain *terr = g_App.GetCurrentTerrain();
	TParams &par = terr->GetParams();
	vtString pfile = terr->GetParamFile();

	wxString defDir(ExtractPath(pfile, false), wxConvUTF8);
	wxString defFile(StartOfFilename(pfile), wxConvUTF8);

	EnableContinuousRendering(false);
	wxFileDialog saveFile(NULL, _("Save Terrain State"), defDir, defFile,
		_("Terrain Files (*.xml)|*.xml"), wxFD_SAVE);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	EnableContinuousRendering(true);
	if (!bResult)
		return;

	// Give the user an opportunity to name the new terrain
	vtString tname = terr->GetName();
	wxString def(tname, wxConvUTF8);
	wxString name = wxGetTextFromUser(_("Name for new terrain:"), _("Query"), def, this);
	if (name == _T(""))
		return;

	vtString fname = (const char*)saveFile.GetPath().mb_str(wxConvUTF8);
	terr->SetParamFile(fname);
	terr->SetName((const char *) name.mb_str(wxConvUTF8));
	if (!par.WriteToXML(fname, STR_TPARAMS_FORMAT_NAME))
		DisplayAndLog("Couldn't write file '%s'", (const char *)fname);

	// Update title
	SetTitle(wxGetApp().MakeFrameTitle(terr));
}

void EnviroFrame::OnUpdateSaveTerrain(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
}


//////////////////// View menu //////////////////////////

void EnviroFrame::OnViewMaintain(wxCommandEvent& event)
{
	g_App.SetMaintain(!g_App.GetMaintain());
}

void EnviroFrame::OnUpdateViewMaintain(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.GetMaintain());
}

void EnviroFrame::OnNavNormal(wxCommandEvent& event)
{
	g_App.SetNavType(NT_Normal);
}

void EnviroFrame::OnUpdateNavNormal(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_nav == NT_Normal);
}

void EnviroFrame::OnNavVelo(wxCommandEvent& event)
{
	g_App.SetNavType(NT_Velo);
}

void EnviroFrame::OnUpdateNavVelo(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_nav == NT_Velo);
}

void EnviroFrame::OnNavGrabPivot(wxCommandEvent& event)
{
	g_App.SetNavType(NT_Grab);
}

void EnviroFrame::OnUpdateNavGrabPivot(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_nav == NT_Grab);
}

void EnviroFrame::OnNavPano(wxCommandEvent& event)
{
	g_App.SetNavType(NT_Pano);
}

void EnviroFrame::OnUpdateNavPano(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_nav == NT_Pano);
}

void EnviroFrame::OnViewWireframe(wxCommandEvent& event)
{
	vtGetScene()->SetGlobalWireframe(!vtGetScene()->GetGlobalWireframe());
}

void EnviroFrame::OnUpdateViewWireframe(wxUpdateUIEvent& event)
{
	event.Check(vtGetScene()->GetGlobalWireframe());
}

void EnviroFrame::OnViewFullscreen(wxCommandEvent& event)
{
	SetFullScreen(!m_bFullscreen);
}

void EnviroFrame::OnUpdateViewFullscreen(wxUpdateUIEvent& event)
{
	event.Check(m_bFullscreen);
}

void EnviroFrame::OnViewTopDown(wxCommandEvent& event)
{
	m_bTopDown = !m_bTopDown;
	g_App.SetTopDown(m_bTopDown);
}

void EnviroFrame::OnUpdateViewTopDown(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(m_bTopDown);
}

void EnviroFrame::OnViewStats(wxCommandEvent& event)
{
#ifdef VTP_USE_OSG_STATS
	// Yes, this is a hack, but it doesn't seem that StatsHandler can be cycled
	//  any other way than by key event.
	osgViewer::GraphicsWindow *pGW = vtGetScene()->GetGraphicsWindow();
	if ((NULL != pGW) && pGW->valid())
		pGW->getEventQueue()->keyPress('x');
#endif
}

void EnviroFrame::OnViewElevLegend(wxCommandEvent& event)
{
	g_App.ShowElevationLegend(!g_App.GetShowElevationLegend());
}

void EnviroFrame::OnUpdateViewElevLegend(wxUpdateUIEvent& event)
{
	// enable only for derived-color textured terrain
	bool bEnable = false;
	vtTerrain *curr = g_App.GetCurrentTerrain();
	if (curr)
		bEnable = (curr->GetParams().GetValueInt(STR_TEXTURE) == 3);
	event.Enable(g_App.m_state == AS_Terrain && bEnable);
	event.Check(g_App.GetShowElevationLegend());
}

void EnviroFrame::OnViewCompass(wxCommandEvent& event)
{
	g_App.ShowCompass(!g_App.GetShowCompass());
}

void EnviroFrame::OnUpdateViewCompass(wxUpdateUIEvent& event)
{
	// enable only for terrain view
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.GetShowCompass());
}

void EnviroFrame::OnViewMapOverView(wxCommandEvent& event)
{
	g_App.ShowMapOverview(!g_App.GetShowMapOverview());
}

void EnviroFrame::OnUpdateViewMapOverView(wxUpdateUIEvent& event)
{
	// Only supported in Terrain View for certain texture types
	bool bEnable = false;
	vtTerrain *curr = g_App.GetCurrentTerrain();
	if (curr)
	{
		TextureEnum eTex = curr->GetParams().GetTextureEnum();
		bEnable = (eTex == TE_SINGLE || eTex == TE_DERIVED);
	}
	event.Enable(bEnable);
	event.Check(g_App.GetShowMapOverview());
}

void EnviroFrame::OnViewVertLine(wxCommandEvent& event)
{
	g_App.ShowVerticalLine(!g_App.GetShowVerticalLine());
}

void EnviroFrame::OnUpdateViewVertLine(wxUpdateUIEvent& event)
{
	// Only supported in Terrain View with multiple elevation layers
	bool bEnable = false;
	vtTerrain *curr = g_App.GetCurrentTerrain();
	if (curr)
	{
		int elev_layers = curr->NumLayersOfType(LT_ELEVATION);
		bEnable = (elev_layers > 0);
	}
	event.Enable(bEnable);
	event.Check(g_App.GetShowVerticalLine());
}

void EnviroFrame::OnViewDrive(wxCommandEvent& event)
{
	m_pDriveDlg->Show(!m_pDriveDlg->IsShown());
}

void EnviroFrame::OnUpdateViewDrive(wxUpdateUIEvent& event)
{
	// Only supported in Terrain View for certain texture types
	bool bEnable = (g_App.m_Vehicles.GetSelectedCarEngine() != NULL);
	event.Enable(bEnable);
	event.Check(m_pDriveDlg->IsShown());
}

void EnviroFrame::OnViewSlower(wxCommandEvent& event)
{
	ChangeFlightSpeed(1.0f / 1.8f);
}

void EnviroFrame::OnUpdateViewSlower(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain && g_App.m_mode == MM_NAVIGATE);
}

void EnviroFrame::OnViewFaster(wxCommandEvent& event)
{
	ChangeFlightSpeed(1.8f);
}

void EnviroFrame::OnUpdateViewFaster(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain && g_App.m_mode == MM_NAVIGATE);
}

void EnviroFrame::OnViewSettings(wxCommandEvent& event)
{
	m_pCameraDlg->Show(true);
}

void EnviroFrame::OnViewLocations(wxCommandEvent& event)
{
	m_pLocationDlg->Show(true);
}

void EnviroFrame::OnViewReset(wxCommandEvent& event)
{
	g_App.ResetCamera();
}

void EnviroFrame::OnUpdateViewLocations(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
}

void EnviroFrame::OnViewSnapshot(wxCommandEvent& event)
{
	Snapshot(false); // do ask for explicit filename always
}

void EnviroFrame::OnViewSnapAgain(wxCommandEvent& event)
{
	Snapshot(true); // number, and don't ask for filename if we already have one
}

void EnviroFrame::OnViewSnapHigh(wxCommandEvent& event)
{
	VTLOG1("EnviroFrame::OnViewSnapHigh\n");

	vtScene *scene = vtGetScene();
	IPoint2 original_size = scene->GetWindowSize();

	SizeDlg dlg(this, -1, _("High-resolution Snapshot"));
	dlg.SetBase(original_size);
	dlg.SetRatioRange(1.0f, 4.0f);
	dlg.GetTextCtrl()->SetValue(_("If the size requested is greater than your graphics card supports, the size will be clamped to the card's limit"));
	if (dlg.ShowModal() != wxID_OK)
		return;

	int  aPixWidth = dlg.m_Current.x;
    int  aPixHeight = dlg.m_Current.y;
	VTLOG("\tSize: %d %d\n", aPixWidth, aPixHeight);

	// save current directory
	wxString path = wxGetCwd();

	wxString filter = FSTRING_JPEG _T("|") FSTRING_BMP _T("|") FSTRING_PNG
		_T("|") FSTRING_TIF;
	EnableContinuousRendering(false);
	wxFileDialog saveFile(NULL, _("Save View Snapshot"), _T(""), _T(""),
		filter, wxFD_SAVE);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	EnableContinuousRendering(true);
	if (!bResult)
	{
		wxSetWorkingDirectory(path);	// restore
		return;
	}

	std::string Filename(saveFile.GetPath().mb_str(wxConvUTF8));
	if (!CSaveImageOSG::SaveImage(Filename, aPixWidth, aPixHeight))
		wxMessageBox(_T("Sorry - could not save image. Yuor graphics card software may need updating"),_T("ERROR"));
}

void EnviroFrame::OnViewStatusBar(wxCommandEvent& event)
{
	GetStatusBar()->Show(!GetStatusBar()->IsShown());
	SendSizeEvent();
}

void EnviroFrame::OnUpdateViewStatusBar(wxUpdateUIEvent& event)
{
	event.Check(GetStatusBar()->IsShown());
}

void EnviroFrame::OnViewScenarios(wxCommandEvent& event)
{
	m_pScenarioSelectDialog->Show(true);
}

void EnviroFrame::OnViewProfile(wxCommandEvent& event)
{
	if (m_pProfileDlg && m_pProfileDlg->IsShown())
		m_pProfileDlg->Hide();
	else
	{
		// this might be the first time it's displayed, so we need to get
		//  the point values from the distance tool
		ProfileDlg *dlg = ShowProfileDlg();
		if (m_pDistanceDlg)
		{
			DPoint2 p1, p2;
			m_pDistanceDlg->GetPoints(p1, p2);
			dlg->SetPoints(p1, p2);
		}
	}
}

void EnviroFrame::OnUpdateViewProfile(wxUpdateUIEvent& event)
{
	event.Check(m_pProfileDlg && m_pProfileDlg->IsShown());
	event.Enable(g_App.m_state == AS_Terrain);
}

void EnviroFrame::OnViewToolCulture(wxCommandEvent& event)
{
	g_Options.m_bShowToolsCulture = !g_Options.m_bShowToolsCulture;
	RefreshToolbar();
}
void EnviroFrame::OnUpdateViewToolCulture(wxUpdateUIEvent& event)
{
	event.Check(g_Options.m_bShowToolsCulture);
}
void EnviroFrame::OnViewToolSnapshot(wxCommandEvent& event)
{
	g_Options.m_bShowToolsSnapshot = !g_Options.m_bShowToolsSnapshot;
	RefreshToolbar();
}
void EnviroFrame::OnUpdateViewToolSnapshot(wxUpdateUIEvent& event)
{
	event.Check(g_Options.m_bShowToolsSnapshot);
}
void EnviroFrame::OnViewToolTime(wxCommandEvent& event)
{
	g_Options.m_bShowToolsTime = !g_Options.m_bShowToolsTime;
	RefreshToolbar();
}
void EnviroFrame::OnUpdateViewToolTime(wxUpdateUIEvent& event)
{
	event.Check(g_Options.m_bShowToolsTime);
}


///////////////////// Tools menu //////////////////////////

void EnviroFrame::OnToolsSelect(wxCommandEvent& event)
{
	SetMode(MM_SELECT);
}

void EnviroFrame::OnUpdateToolsSelect(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain || g_App.m_state == AS_Orbit);
	event.Check(g_App.m_mode == MM_SELECT);
}

void EnviroFrame::OnToolsSelectBox(wxCommandEvent& event)
{
	SetMode(MM_SELECTBOX);
}

void EnviroFrame::OnUpdateToolsSelectBox(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_SELECTBOX);
}

void EnviroFrame::OnToolsSelectMove(wxCommandEvent& event)
{
	SetMode(MM_SELECTMOVE);
}

void EnviroFrame::OnUpdateToolsSelectMove(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_SELECTMOVE);
}

void EnviroFrame::OnToolsFences(wxCommandEvent& event)
{
	SetMode(MM_LINEARS);
}

void EnviroFrame::OnUpdateToolsFences(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_LINEARS);
}

void EnviroFrame::OnToolsBuildings(wxCommandEvent& event)
{
	SetMode(MM_BUILDINGS);
}

void EnviroFrame::OnUpdateToolsBuildings(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_BUILDINGS);
}

void EnviroFrame::OnToolsPower(wxCommandEvent& event)
{
	SetMode(MM_POWER);
}

void EnviroFrame::OnUpdateToolsPower(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_POWER);
}

void EnviroFrame::OnToolsPlants(wxCommandEvent& event)
{
	SetMode(MM_PLANTS);
}

void EnviroFrame::OnUpdateToolsPlants(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_PLANTS);
}

void EnviroFrame::OnToolsPoints(wxCommandEvent& event)
{
	vtAbstractLayer *alay = g_App.GetLabelLayer();
	if (!alay)
		alay = CreateNewAbstractPointLayer(g_App.GetCurrentTerrain());
	if (!alay)
		return;
	SetMode(MM_ADDPOINTS);
}

void EnviroFrame::OnUpdateToolsPoints(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_ADDPOINTS);
}

void EnviroFrame::OnToolsInstances(wxCommandEvent& event)
{
	SetMode(MM_INSTANCES);
}

void EnviroFrame::OnUpdateToolsInstances(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_INSTANCES);
}

void EnviroFrame::OnToolsVehicles(wxCommandEvent& event)
{
	SetMode(MM_VEHICLES);
}

void EnviroFrame::OnUpdateToolsVehicles(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_VEHICLES);
}

void EnviroFrame::OnToolsMove(wxCommandEvent& event)
{
	SetMode(MM_MOVE);
}

void EnviroFrame::OnUpdateToolsMove(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_MOVE);
}

void EnviroFrame::OnToolsNavigate(wxCommandEvent& event)
{
	SetMode(MM_NAVIGATE);
}

void EnviroFrame::OnUpdateToolsNavigate(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_mode == MM_NAVIGATE);
}

void EnviroFrame::OnToolsMeasure(wxCommandEvent& event)
{
	SetMode(MM_MEASURE);
}

void EnviroFrame::OnUpdateToolsMeasure(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain || g_App.m_state == AS_Orbit);
	event.Check(g_App.m_mode == MM_MEASURE);
}

void EnviroFrame::OnToolsConstrain(wxCommandEvent& event)
{
	g_App.m_bConstrainAngles = !g_App.m_bConstrainAngles;
}

void EnviroFrame::OnUpdateToolsConstrain(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
	event.Check(g_App.m_bConstrainAngles);
}

// Visual impact submenu
void EnviroFrame::OnVIACalculate(wxCommandEvent& event)
{
#if VTP_VISUAL_IMPACT_CALCULATOR
	wxString Message = _("Your 3d driver does not support off screen rendering.\n");
	Message += _("If the main 3d window is obscured by any other windows\n");
	Message += _("(including popup dialogs). The accuracy of the visual impact\n");
	Message += _("calculation will be impaired.");

	if (vtGetScene()->GetVisualImpactCalculator().UsingLiveFrameBuffer())
		wxMessageBox(Message);
	m_pVIADlg->Show(true);
#else
	wxMessageBox(_("Not available."));
#endif
}

void EnviroFrame::OnUpdateVIACalculate(wxUpdateUIEvent& event)
{
	bool bFound = false;
	vtTerrain *pTerr = g_App.GetCurrentTerrain();
	if (NULL != pTerr)
	{
		LayerSet& Layers = pTerr->GetLayers();

		int iNumberOfLayers = Layers.size();

		for (int i = 0; i < iNumberOfLayers; i++)
		{
			vtStructureArray3d *pStructures = dynamic_cast<vtStructureArray3d*>(Layers[i].get());
			if (NULL != pStructures)
			{
				int iNumberOfStructures = pStructures->size();
				for (int j = 0; (j < iNumberOfStructures) && !bFound; j++)
				{
					vtStructure3d *pStructure3d = pStructures->GetStructure3d(j);
					if (pStructure3d->GetVIAContributor())
						bFound = true;
				}
			}
		}
	}
	event.Enable(bFound);
}

void EnviroFrame::OnVIAPlot(wxCommandEvent& event)
{
#if VTP_VISUAL_IMPACT_CALCULATOR
	wxFileDialog RasterFileDialog(this,
								_T("Output raster file"),
								_T(""), _T("viaplot.tif"),
								_T(""), wxFD_SAVE);
	CVIAGDALOptionsDlg OptionsDlg(this);
	GDALDriverManager *pDriverManager;
	int iNumDrivers;
	int i;
	bool bFirst = true;
	wxString Filter;
	int *pDriverIndices = NULL;
	int iDriverIndexOffset = 0;
	int iDriverIndex;
	int iDefaultFilter = 0;

	g_GDALWrapper.RequestGDALFormats();

	pDriverManager = GetGDALDriverManager();
	iNumDrivers = pDriverManager->GetDriverCount();
	if (NULL == (pDriverIndices = new int[iNumDrivers]))
		return;
	for (i = 0; i < iNumDrivers; i++)
	{
		GDALDriver* pDriver = pDriverManager->GetDriver(i);
		char ** ppMetaData = pDriver->GetMetadata();
		const char *pExtension;
		const char *pLongname;

		if (CSLFetchBoolean(ppMetaData, GDAL_DCAP_CREATE, FALSE)
			&& (NULL != (pLongname = CSLFetchNameValue(ppMetaData, GDAL_DMD_LONGNAME))))
		{
			pExtension = CSLFetchNameValue(ppMetaData, GDAL_DMD_EXTENSION);
			if (bFirst)
				bFirst = false;
			else
				Filter += wxT("|");
			Filter += wxString(pLongname, wxConvUTF8);
			Filter += wxT("|*.");
			Filter += wxString(pExtension, wxConvUTF8);
			pDriverIndices[iDriverIndexOffset] = i;
#ifdef WIN32
			if (0 == strnicmp(pLongname, "GeoTIFF", 7))
#else
			if (0 == strncasecmp(pLongname, "GeoTIFF", 7))
#endif
				iDefaultFilter = i;
			iDriverIndexOffset++;
		}
	}

	RasterFileDialog.SetWildcard(Filter);
	RasterFileDialog.SetFilterIndex(iDefaultFilter);

	if (wxID_OK != RasterFileDialog.ShowModal())
	{
		delete pDriverIndices;
		return;
	}

	iDriverIndex = pDriverIndices[RasterFileDialog.GetFilterIndex()];
	delete pDriverIndices;

	OptionsDlg.Setup(iDriverIndex);

	if (wxID_OK != OptionsDlg.ShowModal())
		return;

	GDALDriver *pDriver = GetGDALDriverManager()->GetDriver(iDriverIndex);
	if (NULL == pDriver)
		return;

	int iNumArgs;
	int iNumChars;
	char **ppArgv = NULL;
	char *pArgs = NULL;
	int iType;

	float fXSampleInterval, fYSampleInterval, fScaleFactor;
	double Temp;
	OptionsDlg.m_XSampleInterval.ToDouble(&Temp);
	fXSampleInterval = Temp;
	OptionsDlg.m_YSampleInterval.ToDouble(&Temp);
	fYSampleInterval = Temp;
	OptionsDlg.m_ScaleFactor.ToDouble(&Temp);
	fScaleFactor = Temp;

	ParseCommandLine((const char *)OptionsDlg.m_CreationOptions.mb_str(wxConvUTF8), NULL, NULL, &iNumArgs, &iNumChars);
	ppArgv = new char*[iNumArgs];
	pArgs = new char[iNumChars];
	ParseCommandLine((const char *)OptionsDlg.m_CreationOptions.mb_str(wxConvUTF8), ppArgv, pArgs, &iNumArgs, &iNumChars);

	for (iType = 1; iType < GDT_TypeCount; iType++)
		if (GDALGetDataTypeName((GDALDataType)iType) != NULL
				&& EQUAL(GDALGetDataTypeName((GDALDataType)iType), (const char *)OptionsDlg.m_DataType.mb_str(wxConvUTF8)))
			break;

	DRECT EarthExtents = vtGetTS()->GetCurrentTerrain()->GetHeightField()->GetEarthExtents();

	int iXSize = (int)((EarthExtents.right - EarthExtents.left)/fXSampleInterval);
	int iYSize = (int)((EarthExtents.top - EarthExtents.bottom)/fYSampleInterval);

	GDALDataset *pDataset = pDriver->Create((const char *)RasterFileDialog.GetPath().mb_str(wxConvUTF8), iXSize, iYSize, 1, (GDALDataType)iType, ppArgv);
	delete [] ppArgv;
	delete [] pArgs;
	if (NULL == pDataset)
		return;

	// Set up geo stuff;
	char *pWKT;
	double Transform[6] = {0.0};
	Transform[0] = EarthExtents.left;
	Transform[1] = fXSampleInterval;
	Transform[3] = EarthExtents.top;
	Transform[5] = -fYSampleInterval;
	pDataset->SetGeoTransform(Transform);
	g_App.GetCurrentTerrain()->GetCRS().exportToWkt(&pWKT);
	pDataset->SetCRS(pWKT);
	CPLFree(pWKT);
	GDALRasterBand *pRasterBand = pDataset->GetRasterBand(1);
	if (NULL == pRasterBand)
		return;

	EnableContinuousRendering(false);
	OpenProgressDialog(_T("Visual Impact Analysis"), _T("Plotting Visual Impact Factor"), true);
	bool bRet = vtGetScene()->GetVisualImpactCalculator().Plot(pRasterBand,
													fScaleFactor,
													fXSampleInterval,
													fYSampleInterval,
													progress_callback);
	CloseProgressDialog();
	if (bRet)
		wxMessageBox(_("Visual Impact Plot successful"));
	else
		wxMessageBox(_("Visual Impact Plot unsuccessful"));

	EnableContinuousRendering(true);
	delete pDataset; // This flushes and closes the dataset
#endif  // VTP_VISUAL_IMPACT_CALCULATOR
}
void EnviroFrame::OnUpdateVIAPlot(wxUpdateUIEvent& event)
{
	bool bFoundContributor = false;
	bool bFoundTarget = false;
	vtTerrain *pTerr = g_App.GetCurrentTerrain();
	if (NULL != pTerr)
	{
		LayerSet &Layers = pTerr->GetLayers();
		int iNumberOfLayers = Layers.size();

		for (int i = 0; i < iNumberOfLayers; i++)
		{
			vtStructureArray3d *pStructures = dynamic_cast<vtStructureArray3d*>(Layers[i].get());
			if (NULL != pStructures)
			{
				int iNumberOfStructures = pStructures->size();
				for (int j = 0; (j < iNumberOfStructures) && !(bFoundContributor && bFoundTarget); j++)
				{
					vtStructure3d *pStructure3d = pStructures->GetStructure3d(j);
					if (pStructure3d->GetVIAContributor())
						bFoundContributor = true;
					if (pStructure3d->GetVIATarget())
						bFoundTarget = true;
				}
			}
		}
	}
	event.Enable(bFoundContributor && bFoundTarget);
}

void EnviroFrame::OnVIAClear(wxCommandEvent& event)
{
	vtTerrain *pTerr = g_App.GetCurrentTerrain();
	if (NULL != pTerr)
	{
		LayerSet &Layers = pTerr->GetLayers();
		int iNumberOfLayers = Layers.size();

		for (int i = 0; i < iNumberOfLayers; i++)
		{
			vtStructureArray3d *pStructures = dynamic_cast<vtStructureArray3d*>(Layers[i].get());
			if (NULL != pStructures)
			{
				int iNumberOfStructures = pStructures->size();
				for (int j = 0; j < iNumberOfStructures; j++)
				{
					vtStructure3d *pStructure3d = pStructures->GetStructure3d(j);
					pStructure3d->SetVIATarget(false);
					pStructure3d->SetVIAContributor(false);
				}
			}
		}
	}
}

void EnviroFrame::OnUpdateVIAClear(wxUpdateUIEvent& event)
{
	bool bFoundContributor = false;
	bool bFoundTarget = false;
	vtTerrain *pTerr = g_App.GetCurrentTerrain();
	if (NULL != pTerr)
	{
		LayerSet &Layers = pTerr->GetLayers();
		int iNumberOfLayers = Layers.size();

		for (int i = 0; i < iNumberOfLayers; i++)
		{
			vtStructureArray3d *pStructures = dynamic_cast<vtStructureArray3d*>(Layers[i].get());
			if (NULL != pStructures)
			{
				int iNumberOfStructures = pStructures->size();
				for (int j = 0; (j < iNumberOfStructures) && !(bFoundContributor || bFoundTarget); j++)
				{
					vtStructure3d *pStructure3d = pStructures->GetStructure3d(j);
					if (pStructure3d->GetVIAContributor())
						bFoundContributor = true;
					if (pStructure3d->GetVIATarget())
						bFoundTarget = true;
				}
			}
		}
	}
	event.Enable(bFoundContributor || bFoundTarget);
}



/////////////////////// Scene menu ///////////////////////////

void EnviroFrame::OnSceneGraph(wxCommandEvent& event)
{
	m_pSceneGraphDlg->Show(true);
}

#ifdef VTP_NVIDIA_PERFORMANCE_MONITORING
void EnviroFrame::OnPerformanceMonitor(wxCommandEvent& event)
{
	m_pPerformanceMonitorDlg->Show(true);
}
#endif

void EnviroFrame::OnSceneTerrain(wxCommandEvent& event)
{
	wxString str;

	// When switching terrains, highlight the current on
	vtTerrain *pTerr = g_App.GetCurrentTerrain();

	// Or, if in Earth view, highlight a terrain that's already been created
	if (!pTerr && g_App.m_state == AS_Orbit)
	{
		for (uint i = 0; i < vtGetTS()->NumTerrains(); i++)
		{
			vtTerrain *t = vtGetTS()->GetTerrain(i);
			if (t->IsCreated())
			{
				pTerr = t;
				break;
			}
		}
	}
	// Get the name from the terrain, if we found one
	if (pTerr)
		str = wxString(pTerr->GetName(), wxConvUTF8);

	if (wxGetApp().AskForTerrainName(this, str))
		g_App.RequestTerrain(str.mb_str(wxConvUTF8));
}

void EnviroFrame::OnUpdateSceneTerrain(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain ||
		g_App.m_state == AS_Orbit ||
		g_App.m_state == AS_Neutral);
}

void EnviroFrame::OnSceneSpace(wxCommandEvent& event)
{
	g_App.FlyToSpace();
}

void EnviroFrame::OnUpdateSceneSpace(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
}

void EnviroFrame::OnSceneSave(wxCommandEvent& event)
{
	vtGroup *pRoot = vtGetTS()->GetTop();
#if (OPENSCENEGRAPH_MAJOR_VERSION==2 && OPENSCENEGRAPH_MINOR_VERSION>=2) || OPENSCENEGRAPH_MAJOR_VERSION>2
	osgDB::Registry::instance()->writeNode(*pRoot, std::string("scene.osg"), NULL);
#else
	osgDB::Registry::instance()->writeNode(*pRoot, "scene.osg");
#endif
}

void EnviroFrame::OnSceneEphemeris(wxCommandEvent& event)
{
	vtTerrainScene *ts = vtGetTS();
	vtTerrain *terr = g_App.GetCurrentTerrain();
	TParams &param = terr->GetParams();
	vtSkyDome *sky = ts->GetSkyDome();

	m_pEphemDlg->m_bSky = sky->GetEnabled();
	m_pEphemDlg->m_strSkyTexture = wxString(param.GetValueString(STR_SKYTEXTURE), wxConvUTF8);
	m_pEphemDlg->m_bOceanPlane = terr->GetFeatureVisible(TFT_OCEAN);
	m_pEphemDlg->m_fOceanPlaneLevel = param.GetValueFloat(STR_OCEANPLANELEVEL);
	m_pEphemDlg->m_bFog = terr->GetFog();
	m_pEphemDlg->m_fFogDistance = param.GetValueFloat(STR_FOGDISTANCE) * 1000;
	RGBi col = terr->GetBgColor();
	// shadows
	m_pEphemDlg->m_bShadows = terr->GetShadows();
	vtShadowOptions opt;
	terr->GetShadowOptions(opt);
	m_pEphemDlg->m_fDarkness = opt.fDarkness;
	m_pEphemDlg->m_bShadowsEveryFrame = opt.bShadowsEveryFrame;
	m_pEphemDlg->m_bShadowLimit = opt.bShadowLimit;
	m_pEphemDlg->m_fShadowRadius = opt.fShadowRadius;

	m_pEphemDlg->m_BgColor.Set(col.r, col.g, col.b);
	m_pEphemDlg->m_iWindDir = param.GetValueInt("WindDirection");
	m_pEphemDlg->m_fWindSpeed = param.GetValueFloat("WindSpeed");
	m_pEphemDlg->ValuesToSliders();

	m_pEphemDlg->Show();
#if 0
	sky->SetEnabled(dlg.m_bSky);
	ts->UpdateSkydomeForTerrain(terr);
	terr->SetFeatureVisible(TFT_OCEAN, dlg.m_bOceanPlane);
	terr->SetWaterLevel(dlg.m_fOceanPlaneLevel);
	terr->SetFog(dlg.m_bFog);
	terr->SetFogDistance(dlg.m_fFogDistance);
	col.Set(dlg.m_BgColor.Red(), dlg.m_BgColor.Green(), dlg.m_BgColor.Blue());
	terr->SetBgColor(col);
	vtGetScene()->SetBgColor(col);
	g_App.SetWind(dlg.m_iWindDir, dlg.m_fWindSpeed);
#endif
}

void EnviroFrame::OnUpdateSceneEphemeris(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Terrain);
}

void EnviroFrame::OnTimeDialog(wxCommandEvent& event)
{
	m_pTimeDlg->Show(true);
}

void EnviroFrame::OnTimeStop(wxCommandEvent& event)
{
	g_App.SetSpeed(0.0f);
}

void EnviroFrame::OnTimeFaster(wxCommandEvent& event)
{
	float x = g_App.GetSpeed();
	if (x == 0.0f)
		g_App.SetSpeed(150.0f);
	else
		g_App.SetSpeed(x*1.5f);
}


/////////////////////// Terrain menu ///////////////////////////

void EnviroFrame::OnSurface(wxCommandEvent& event)
{
	vtTerrain *t = g_App.GetCurrentTerrain();
	if (!t) return;
	bool on = t->GetFeatureVisible(TFT_TERRAINSURFACE);

	t->SetFeatureVisible(TFT_TERRAINSURFACE, !on);
}

void EnviroFrame::OnUpdateSurface(wxUpdateUIEvent& event)
{
	vtTerrain *t = g_App.GetCurrentTerrain();
	event.Check(t && t->GetFeatureVisible(TFT_TERRAINSURFACE));
	event.Enable(t != NULL);
}

void EnviroFrame::OnSky(wxCommandEvent& event)
{
	vtSkyDome *sky = vtGetTS()->GetSkyDome();
	if (!sky) return;
	bool on = sky->GetEnabled();
	sky->SetEnabled(!on);
}

void EnviroFrame::OnUpdateSky(wxUpdateUIEvent& event)
{
	vtSkyDome *sky = vtGetTS()->GetSkyDome();
	if (!sky) return;
	bool on = sky->GetEnabled();
	event.Check(on);
	event.Enable(g_App.GetCurrentTerrain() != NULL);
}

void EnviroFrame::OnOcean(wxCommandEvent& event)
{
	vtTerrain *t = g_App.GetCurrentTerrain();
	if (t) t->SetFeatureVisible(TFT_OCEAN, !t->GetFeatureVisible(TFT_OCEAN));
}

void EnviroFrame::OnUpdateOcean(wxUpdateUIEvent& event)
{
	vtTerrain *t = g_App.GetCurrentTerrain();
	bool on = false;
	if (t)
		on = t->GetFeatureVisible(TFT_OCEAN);
	event.Enable(t != NULL);
	event.Check(on);
}

void EnviroFrame::OnPlants(wxCommandEvent& event)
{
	vtTerrain *t = g_App.GetCurrentTerrain();
	if (t) t->SetFeatureVisible(TFT_VEGETATION, !t->GetFeatureVisible(TFT_VEGETATION));
}

void EnviroFrame::OnUpdatePlants(wxUpdateUIEvent& event)
{
	vtTerrain *t = g_App.GetCurrentTerrain();
	bool on = false;
	if (t)
		on = t->GetFeatureVisible(TFT_VEGETATION);
	event.Enable(t != NULL);
	event.Check(on);
}

void EnviroFrame::OnStructures(wxCommandEvent& event)
{
	vtTerrain *t = g_App.GetCurrentTerrain();
	if (t) t->SetFeatureVisible(TFT_STRUCTURES, !t->GetFeatureVisible(TFT_STRUCTURES));
}

void EnviroFrame::OnUpdateStructures(wxUpdateUIEvent& event)
{
	vtTerrain *t = g_App.GetCurrentTerrain();
	bool on = false;
	if (t)
		on = t->GetFeatureVisible(TFT_STRUCTURES);
	event.Enable(t != NULL);
	event.Check(on);
}

void EnviroFrame::OnRoads(wxCommandEvent& event)
{
	vtTerrain *t = g_App.GetCurrentTerrain();
	if (t) t->SetFeatureVisible(TFT_ROADS, !t->GetFeatureVisible(TFT_ROADS));
}

void EnviroFrame::OnFog(wxCommandEvent& event)
{
	vtTerrain *t = g_App.GetCurrentTerrain();
	if (t) t->SetFog(!t->GetFog());
}

void EnviroFrame::OnUpdateRoads(wxUpdateUIEvent& event)
{
	vtTerrain *t = g_App.GetCurrentTerrain();
	bool on = false;
	if (t)
		on = t->GetFeatureVisible(TFT_ROADS);
	event.Enable(t != NULL);
	event.Check(on);
}

void EnviroFrame::OnUpdateFog(wxUpdateUIEvent& event)
{
	vtTerrain *t = g_App.GetCurrentTerrain();
	event.Check(t && t->GetFog());
	event.Enable(t != NULL);
}

void EnviroFrame::OnIncrease(wxCommandEvent& event)
{
	SetTerrainDetail(GetTerrainDetail()+1000);
}

void EnviroFrame::OnDecrease(wxCommandEvent& event)
{
	SetTerrainDetail(GetTerrainDetail()-1000);
}

void EnviroFrame::OnLOD(wxCommandEvent& event)
{
	m_pLODDlg->Show();
}

void EnviroFrame::OnUpdateLOD(wxUpdateUIEvent& event)
{
	vtTerrain *t = g_App.GetCurrentTerrain();
	event.Enable(t &&
		(t->GetDynTerrain() != NULL || t->GetTiledGeom() != NULL));
}

static bool s_bBuilt = false;

void EnviroFrame::OnToggleFoundations(wxCommandEvent& event)
{
	s_bBuilt = !s_bBuilt;

	vtTerrain *pTerr = g_App.GetCurrentTerrain();
	vtStructureArray3d *sa = pTerr->GetStructureLayer();

	if (s_bBuilt)
	{
		OpenProgressDialog(_("Adding Foundations"), _T(""));
		sa->AddFoundations(pTerr->GetHeightField(), progress_callback);
	}
	else
	{
		OpenProgressDialog(_("Removing Foundations"), _T(""));
		sa->RemoveFoundations();
	}
	int i, size = sa->size(), selected = sa->NumSelected();
	for (i = 0; i < size; i++)
	{
		progress_callback(i * 99 / size);

		vtStructure *s = sa->at(i);
		if (selected > 0 && !s->IsSelected())
			continue;
		if (s->GetType() != ST_BUILDING)
			continue;
		sa->ConstructStructure(i);
	}
	CloseProgressDialog();
}

void EnviroFrame::OnUpdateFoundations(wxUpdateUIEvent& event)
{
	vtTerrain *t = g_App.GetCurrentTerrain();
	event.Enable(t && t->GetStructureLayer() && t->GetStructureLayer()->size() > 0);
	event.Check(s_bBuilt);
}

void EnviroFrame::OnTerrainReshade(wxCommandEvent& event)
{
	VTLOG1("EnviroFrame::OnTerrainReshade\n");

	vtTerrain *pTerr = g_App.GetCurrentTerrain();
	if (!pTerr)
		return;

	EnableContinuousRendering(false);
	OpenProgressDialog(_("Recalculating Shading"), _T(""), false, this);
	pTerr->ReshadeTexture(vtGetTS()->GetSunLightTransform(), progress_callback);
	CloseProgressDialog();
	EnableContinuousRendering(true);

	// Also update the overview, if there is one.
	if (g_App.GetShowMapOverview())
		g_App.TextureHasChanged();
}

void EnviroFrame::OnTerrainChangeTexture(wxCommandEvent& event)
{
	VTLOG1("EnviroFrame::OnTerrainChangeTexture\n");

	vtTerrain *pTerr = g_App.GetCurrentTerrain();
	if (!pTerr)
		return;

	EnableContinuousRendering(false);

	TextureDlg dlg(this, -1, _("Change Texture"));
	dlg.SetParams(pTerr->GetParams());
	if (dlg.ShowModal() == wxID_OK)
	{
		dlg.GetParams(pTerr->GetParams());

		OpenProgressDialog(_("Changing Texture"), _T(""), false, this);
		pTerr->RecreateTexture(vtGetTS()->GetSunLightTransform(), progress_callback);
		CloseProgressDialog();

		// Also update the overview, if there is one.
		if (g_App.GetShowMapOverview())
			g_App.TextureHasChanged();
	}

	EnableContinuousRendering(true);
	m_canvas->Refresh(false);
}

void EnviroFrame::OnUpdateIsTerrainView(wxUpdateUIEvent& event)
{
	vtTerrain *t = g_App.GetCurrentTerrain();
	event.Enable(t && g_App.m_state == AS_Terrain);
}

void EnviroFrame::OnTerrainDistribVehicles(wxCommandEvent& event)
{
	vtTerrain *pTerr = g_App.GetCurrentTerrain();
	if (!pTerr)
		return;

	if (pTerr->GetRoadMap() == NULL)
	{
		wxMessageBox(_("There are no roads to put the vehicles on.\n"));
		return;
	}

	int numv = 0;
	vtContentManager3d &con = vtGetContent();
	for (uint i = 0; i < con.NumItems(); i++)
	{
		vtItem *item = con.GetItem(i);
		const char *type = item->GetValueString("type");
		int wheels = item->GetValueInt("num_wheels");
		if (type && vtString(type) == "ground vehicle" && wheels == 4)
			numv++;
	}
	if (numv == 0)
	{
		wxMessageBox(_("Could not find any ground vehicles in the content file.\n"));
		return;
	}

	wxString msg;
	msg.Printf(_("There are %d types of ground vehicle available."), numv);
	int num = wxGetNumberFromUser(msg, _("Vehicles:"), _("Distribute Vehicles"), 10, 1, 99);
	if (num == -1)
		return;

	g_App.CreateSomeTestVehicles(pTerr);
}

void EnviroFrame::OnTerrainWriteElevation(wxCommandEvent& event)
{
	vtTerrain *pTerr = g_App.GetCurrentTerrain();
	if (!pTerr)
		return;

	EnableContinuousRendering(false);
	wxFileDialog saveFile(NULL, _("Write Elevation to BT"), _T(""), _T(""),
		FSTRING_BT, wxFD_SAVE);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	EnableContinuousRendering(true);
	if (!bResult)
		return;

	EnableContinuousRendering(false);
	OpenProgressDialog(_("Write Elevation to BT"), saveFile.GetPath(), false, this);

	// Get the properties of the dynamic terrain, make an elevation grid like it
	vtDynTerrainGeom *dtg = pTerr->GetDynTerrain();

	const vtCRS &crs = pTerr->GetCRS();
	const IPoint2 &size = dtg->GetDimensions();
	const  DRECT area = dtg->GetEarthExtents();
	bool bFloat = true;
	vtElevationGrid grid(area, size, bFloat, crs);

	// Copy the data to the (temporary) elevation grid
	for (int i = 0; i < size.x; i++)
	{
		progress_callback(i * 99 / size.x);
		for (int j = 0; j < size.y; j++)
		{
			const float val = dtg->GetElevation(i, j, true);
			grid.SetFValue(i, j, val);
		}
	}

	// Write to disk, the temporary is freed when it goes out of scope
	vtString fname = (const char*)saveFile.GetPath().mb_str(wxConvUTF8);
	grid.SaveToBT(fname, progress_callback);

	CloseProgressDialog();
	EnableContinuousRendering(true);
}

void EnviroFrame::OnTerrainAddContour(wxCommandEvent& event)
{
#if SUPPORT_QUIKGRID
	vtTerrain *pTerr = g_App.GetCurrentTerrain();
	if (!pTerr)
		return;

	EnableContinuousRendering(false);
	ContourDlg dlg(this, -1, _("Add Contours"));

	dlg.LayerChoice()->Clear();
	LayerSet &layers = pTerr->GetLayers();
	for (uint i = 0; i < layers.size(); i++)
	{
		vtAbstractLayer *alay = dynamic_cast<vtAbstractLayer*>(layers[i].get());
		if (!alay)
			continue;
		vtString vname = alay->GetLayerName();
		if (alay->GetFeatureSet()->GetGeomType() == wkbLineString)
			dlg.LayerChoice()->Append(wxString(vname, wxConvUTF8));
	}

	bool bResult = (dlg.ShowModal() == wxID_OK);
	EnableContinuousRendering(true);
	if (!bResult)
		return;

	vtAbstractLayer *alay;
	if (dlg.m_bCreate)
	{
		// create new (abstract polyline) layer to receive contour lines
		alay = CreateNewAbstractLineLayer(pTerr, true);
	}
	else
	{
		// get the existing layer from the dialog's choice, by name
		wxString wname = dlg.m_strLayer;
		vtLayer *lay = layers.FindByName((const char *)wname.mb_str(wxConvUTF8));
		if (!lay) return;
		alay = dynamic_cast<vtAbstractLayer*>(lay);
	}
	if (!alay) return;
	vtFeatureSetLineString *pSet = (vtFeatureSetLineString *) alay->GetFeatureSet();

	vtContourConverter cc;
	if (!cc.Setup(pTerr, pSet))
		return;

	if (dlg.m_bSingle)
		cc.GenerateContour(dlg.m_fElevSingle);
	else
		cc.GenerateContours(dlg.m_fElevEvery);
	cc.Finish();

	// show the geometry
	pTerr->CreateAbstractLayerVisuals(alay);

	// and show it in the layers dialog
	m_pLayerDlg->RefreshTreeContents();	// full refresh
#endif
}

void EnviroFrame::OnUpdateIsDynTerrain(wxUpdateUIEvent& event)
{
	vtTerrain *t = g_App.GetCurrentTerrain();
	event.Enable(t && t->GetDynTerrain());
}

void EnviroFrame::OnCullEvery(wxCommandEvent& event)
{
	vtTerrain *t = g_App.GetCurrentTerrain();
	if (!t) return;

	m_bCulleveryframe = !m_bCulleveryframe;
	t->GetDynTerrain()->SetCull(m_bCulleveryframe);
}

void EnviroFrame::OnUpdateCullEvery(wxUpdateUIEvent& event)
{
	vtTerrain *t = g_App.GetCurrentTerrain();
	event.Enable(t && t->GetDynTerrain());
	event.Check(m_bCulleveryframe);
}

void EnviroFrame::OnCullOnce(wxCommandEvent& event)
{
	vtTerrain *t = g_App.GetCurrentTerrain();
	if (!t) return;
	vtDynTerrainGeom *pTerr = t->GetDynTerrain();
	if (!pTerr) return;

	pTerr->CullOnce();
}


////////////////// Earth Menu //////////////////////

void EnviroFrame::OnEarthShowShading(wxCommandEvent& event)
{
	g_App.SetEarthShading(!g_App.GetEarthShading());
}

void EnviroFrame::OnUpdateEarthShowShading(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit);
	event.Check(g_App.GetEarthShading());
}

void EnviroFrame::OnEarthShowAxes(wxCommandEvent& event)
{
	g_App.SetSpaceAxes(!g_App.GetSpaceAxes());
}

void EnviroFrame::OnUpdateEarthShowAxes(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit || g_App.m_state == AS_Terrain);
	event.Check(g_App.GetSpaceAxes());
}

void EnviroFrame::OnEarthTilt(wxCommandEvent& event)
{
	g_App.SetEarthTilt(!g_App.GetEarthTilt());
}

void EnviroFrame::OnUpdateEarthTilt(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit);
	event.Check(g_App.GetEarthTilt());
}

void EnviroFrame::OnUpdateEarthFlatten(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit);
	event.Check(g_App.GetEarthShape());
}

void EnviroFrame::OnUpdateEarthUnfold(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit);
	event.Check(g_App.GetEarthUnfold());
}

void EnviroFrame::OnUpdateInOrbit(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit);
}

void EnviroFrame::OnUpdateInOrbitOrTerrain(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit || g_App.m_state == AS_Terrain);
}

void EnviroFrame::OnEarthFlatten(wxCommandEvent& event)
{
	g_App.SetEarthShape(!g_App.GetEarthShape());
}

void EnviroFrame::OnEarthUnfold(wxCommandEvent& event)
{
	g_App.SetEarthUnfold(!g_App.GetEarthUnfold());
}

void EnviroFrame::OnEarthClouds(wxCommandEvent& event)
{
	wxFileDialog loadFile(NULL, _("Load"), _T(""), _T(""),
		FSTRING_JPEG, wxFD_OPEN);
	if (loadFile.ShowModal() != wxID_OK)
		return;
	vtString fname = (const char *) loadFile.GetPath().mb_str(wxConvUTF8);
	LoadClouds(fname);
}

// Helper
vtString GetTempFolderName()
{
	vtString path;

	const char *temp = getenv("TEMP");
	if (temp)
		path = temp;
	else
#if WIN32
		path = "C:/TEMP";
#else
		path = "/tmp";
#endif
	return path;
}

void EnviroFrame::OnEarthClouds2(wxCommandEvent& event)
{
#if SUPPORT_CURL
	wxString defval = _T("http://xplanet.explore-the-world.net/clouds_2048.jpg");
	wxString str = wxGetTextFromUser(_T("Enter URL of overlay image"),
		_T("URL"), defval, this);

	if (str == _T(""))
		return;
	vtString url = (const char*)str.mb_str();

	vtBytes data;
	ReqContext cl;
	cl.SetProgressCallback(progress_callback);

	OpenProgressDialog(_T("Downloading"), wxString::FromUTF8((const char *) url), false, this);
	bool success = cl.GetURL(url, data);
	CloseProgressDialog();

	if (!success)
		return;

	vtString tfile = GetTempFolderName() + "/temp.jpg";
	FILE *fp = vtFileOpen(tfile, "wb");
	if (!fp)
		return;
	fwrite(data.Get(), data.Len(), 1, fp);
	fclose(fp);

	LoadClouds(tfile);
#endif
}

void EnviroFrame::OnUpdateEarthClouds(wxUpdateUIEvent& event)
{
	event.Enable(g_App.m_state == AS_Orbit && g_App.GetOverlayGlobe() == NULL);
}


//////////////////// Help menu //////////////////////////

void EnviroFrame::OnHelpAbout(wxCommandEvent& event)
{
	EnableContinuousRendering(false);

	wxString str(STRING_APPORG "\n\n", wxConvUTF8);
#ifdef ENVIRO_NATIVE
	str += _("The runtime environment for the Virtual Terrain Project.\n\n");
	str += _("Please read the HTML documentation and license.\n\n");
	str += _("Send feedback to: ben@vterrain.org\n");
	str += _T("\nVersion: ");
	str += _T(VTP_VERSION);
	str += _T("\n");
	str += _("Build date: ");
	str += wxString(__DATE__, *wxConvCurrent);
#else
	str += _T("Based on the Virtual Terrain Project 3D Runtime Environment.\n");
#endif

	str += _T("\n\n");
	str += wxString("Application framework: wxWidgets v" wxVERSION_NUM_DOT_STRING "\n", wxConvUTF8);

	wxString str2 = _("About ");
	str2 += wxString(STRING_APPORG, wxConvUTF8);
	wxMessageBox(str, str2);

	EnableContinuousRendering(true);
}

void EnviroFrame::OnHelpDocLocal(wxCommandEvent &event)
{
	vtString local_lang_code = (const char *) wxGetApp().GetLanguageCode().mb_str(wxConvUTF8);
	local_lang_code = local_lang_code.Left(2);

	// Launch default web browser with documentation pages
	LaunchAppDocumentation("Enviro", local_lang_code);
}

void EnviroFrame::OnHelpDocOnline(wxCommandEvent &event)
{
	// Launch default web browser with documentation pages
	wxLaunchDefaultBrowser(_T("http://vterrain.org/Doc/Enviro/"));
}


///////////////////////////////////////////////////////////////////
// Popup Menus


void EnviroFrame::ShowPopupMenu(const IPoint2 &pos)
{
	// Currently, you can pop up properties on a single structure, or
	//  a single plant.  If both are selected, the structure is used.
	VTLOG1("Creating popup, ");

	vtTerrain *pTerr = g_App.GetCurrentTerrain();
	vtStructureLayer *slay = pTerr->GetStructureLayer();
	vtVegLayer *vlay = pTerr->GetVegLayer();
	vtAbstractLayer *alay = pTerr->GetAbstractLayer();

	wxMenu *popmenu = new wxMenu;

	int structures_selected = slay ? slay->NumSelected() : 0;
	int buildings_selected = slay ? slay->NumSelectedOfType(ST_BUILDING) : 0;
	int linears_selected = slay ? slay->NumSelectedOfType(ST_LINEAR) : 0;
	int instances_selected = slay ? slay->NumSelectedOfType(ST_INSTANCE) : 0;
	int plants_selected = vlay ? vlay->NumSelected() : 0;
	int features_selected = alay ? alay->GetFeatureSet()->NumSelected() : 0;

	wxMenuItem *item = popmenu->Append(ID_POPUP_PROPERTIES, _("Properties"));
	if (structures_selected == 0 && plants_selected == 0 && features_selected == 0)
		item->Enable(false);

	// Can't display properties for more than one structure
	if (buildings_selected == 1)
		popmenu->Append(ID_POPUP_COPY_STYLE, _("Copy Style"));

	if (buildings_selected > 0)
	{
		if (g_App.HaveBuildingStyle())
			popmenu->Append(ID_POPUP_PASTE_STYLE, _("Paste Style"));
		popmenu->Append(ID_POPUP_FLIP, _("Flip Footprint Direction"));
		popmenu->Append(ID_POPUP_SET_EAVES, _("Set Eaves"));
	}

	if (instances_selected > 0)
	{
		popmenu->Append(ID_POPUP_RELOAD, _("Reload from Disk"));
		popmenu->Append(ID_POPUP_ADJUST, _("Adjust Terrain Surface to Fit"));
	}

	if (structures_selected > 0)
		popmenu->Append(ID_POPUP_SHADOW, _("Toggle Shadow"));

	if (structures_selected == 1)
	{
		// It might have a URL, also
		vtTag *tag = slay->GetFirstSelectedStructure()->FindTag("url");
		if (tag)
		{
			popmenu->AppendSeparator();
			popmenu->Append(ID_POPUP_URL, _("URL"));
		}
#ifdef VTP_VISUAL_IMPACT_CALCULATOR
		// Visual Impact Assessment
        popmenu->AppendCheckItem(ID_POPUP_VIA,
			_("&Visual Impact Contributor\tCtrl+V"),
			_("Set this structure as a contributor to the VIA calculation"));
        popmenu->AppendCheckItem(ID_POPUP_VIA_TARGET,
			_("Visual Impact &Target\tCtrl+T"),
			_("Set this structure as the viewer target for VIA plots"));
#endif
	}
	if (features_selected == 1)
	{
		// It might have a URL, also
		vtFeatureSet *fset = alay->GetFeatureSet();
		if (fset->GetField("url"))
		{
			popmenu->AppendSeparator();
			popmenu->Append(ID_POPUP_URL, _("URL"));
		}
	}
	if (plants_selected != 0)
	{
		// We could add some plant-specific commands here
	}

	if (g_App.m_Vehicles.GetSelected() != -1)
	{
		popmenu->Append(ID_POPUP_START, _("Start Driving"));
	}

	popmenu->AppendSeparator();
	popmenu->Append(ID_POPUP_DELETE, _("Delete"));

	VTLOG1("show.\n");
	m_canvas->PopupMenu(popmenu, pos.x, pos.y);
	delete popmenu;
}

void EnviroFrame::OnPopupProperties(wxCommandEvent& event)
{
	// Currently, you can pop up properties on a single structure, or
	//  a single plant.  If both are selected, the structure is used.

	VTLOG1("OnPopupProperties.\n");
	vtTerrain *pTerr = g_App.GetCurrentTerrain();
	vtStructureArray3d *sa = pTerr->GetStructureLayer();
	if (sa)
	{
		VTLOG("Active: structure layer\n");
		int sel = sa->GetFirstSelected();
		if (sel != -1)
		{
			vtBuilding3d *bld = sa->GetBuilding(sel);
			if (bld)
			{
				m_pBuildingDlg->Setup(bld);
				m_pBuildingDlg->Show(true);
			}
			vtFence3d *fen = sa->GetFence(sel);
			if (fen)
			{
				// Editing of fence properties
				m_pFenceDlg->SetOptions(fen->GetParams());
				OpenFenceDialog();
			}
			vtStructInstance3d *inst = sa->GetInstance(sel);
			if (inst)
			{
				// Display of instance properties
				m_pTagDlg->Show(true);
				m_pTagDlg->SetTags(inst);
			}
			return;
		}
	}

	vtVegLayer *vlay = pTerr->GetVegLayer();
	if (vlay && vlay->NumSelected() != 0)
	{
		VTLOG("Active: veg layer\n");
		int found = -1;
		uint count = vlay->NumEntities();
		for (uint i = 0; i < count; i++)
		{
			if (vlay->IsSelected(i))
			{
				found = i;
				break;
			}
		}
		if (found != -1)
		{
			// Show properties for this plant
			PlantingOptions &opt = g_App.GetPlantOptions();

			float size;
			short species_id;
			vlay->GetPlant(found, size, species_id);
			opt.m_iSpecies = species_id;
			opt.m_fHeight = size;

			m_pPlantDlg->SetSpeciesList(g_App.GetSpeciesList());
			m_pPlantDlg->SetDlgPlantOptions(opt);
			m_pPlantDlg->Show(true);
		}
	}

	vtAbstractLayer *alay = pTerr->GetAbstractLayer();
	if (alay)
	{
		vtFeatureSet *fset = alay->GetFeatureSet();
		VTLOG("Active: abstract layer\n");
		uint count = fset->NumSelected();
		if (count)
		{
			// Use the feature table dialog to show attributes of the selected features.
			FeatureTableDlg3d *table = ShowTable(alay);
			table->ShowSelected();
		}
	}
}

void EnviroFrame::OnPopupCopyStyle(wxCommandEvent& event)
{
	g_App.CopyBuildingStyle();
}

void EnviroFrame::OnPopupPasteStyle(wxCommandEvent& event)
{
	g_App.PasteBuildingStyle();
}

void EnviroFrame::OnPopupFlip(wxCommandEvent& event)
{
	g_App.FlipBuildingFooprints();
}

void EnviroFrame::OnPopupSetEaves(wxCommandEvent& event)
{
	static float fLength = 1.0f;

	wxString str;
	str.Printf(_T("%g"), fLength);
	str = wxGetTextFromUser(_("Eave length in meters"), _("Set Eaves"), str, this);
	if (str != _T(""))
	{
		fLength = atof(str.mb_str(wxConvUTF8));
		g_App.SetBuildingEaves(fLength);
	}
}

void EnviroFrame::OnPopupReload(wxCommandEvent& event)
{
	vtStructureLayer *structures = g_App.GetCurrentTerrain()->GetStructureLayer();
	if (!structures)
		return;

	for (uint i = 0; i < structures->size(); i++)
	{
		if (structures->at(i)->IsSelected())
		{
			vtStructInstance3d *inst = structures->GetInstance(i);
			if (inst)
				structures->ConstructStructure(structures->GetStructure3d(i));
		}
	}
}

void EnviroFrame::OnPopupShadow(wxCommandEvent& event)
{
	vtTerrain *pTerr = g_App.GetCurrentTerrain();
	vtStructureLayer *structures = pTerr->GetStructureLayer();

	for (uint i = 0; i < structures->size(); i++)
	{
		vtStructure *str = structures->at(i);
		if (!str->IsSelected())
			continue;

		// toggle
		bool bShow = !structures->GetStructure3d(i)->GetCastShadow();
		structures->GetStructure3d(i)->SetCastShadow(bShow);

		// remember state
		if (!bShow)
			str->SetValueBool("shadow", false);
		else
			// shadows are on by default, so don't store shadow=true
			str->RemoveTag("shadow");
	}
	pTerr->ForceShadowUpdate();
}

void EnviroFrame::OnPopupAdjust(wxCommandEvent& event)
{
	vtStructureLayer *structures = g_App.GetCurrentTerrain()->GetStructureLayer();

	for (uint i = 0; i < structures->size(); i++)
	{
		if (structures->at(i)->IsSelected())
		{
			vtStructInstance3d *inst = structures->GetInstance(i);
			if (inst)
				CarveTerrainToFitNode(inst->GetContainer());
		}
	}
}

void EnviroFrame::OnPopupStart(wxCommandEvent& event)
{
	g_App.m_Vehicles.SetVehicleSpeed(g_App.m_Vehicles.GetSelected(), 1.0f);
}

void EnviroFrame::OnPopupDelete(wxCommandEvent& event)
{
	DeleteAllSelected();
}

void EnviroFrame::OnPopupURL(wxCommandEvent& event)
{
	vtString url;
	vtStructureArray3d *sa = g_App.GetCurrentTerrain()->GetStructureLayer();
	vtAbstractLayer *alay = g_App.GetCurrentTerrain()->GetAbstractLayer();
	if (sa)
	{
		vtStructure *struc = sa->at(sa->GetFirstSelected());
		url = struc->GetValueString("url");
	}
	else if (alay)
	{
		vtFeatureSet *fset = alay->GetFeatureSet();
		uint size = fset->NumEntities();
		for (uint i = 0; i < size; i++)
		{
			if (fset->IsSelected(i))
			{
				fset->GetValueAsString(i, fset->GetFieldIndex("Url"), url);
			}
		}
	}
	if (url != "")
		wxLaunchDefaultBrowser(wxString(url, wxConvUTF8));
}

void EnviroFrame::OnPopupVIA(wxCommandEvent& event)
{
#if VTP_VISUAL_IMPACT_CALCULATOR
	vtTerrain *pTerr = g_App.GetCurrentTerrain();
	vtStructureArray3d *pStructures = pTerr->GetStructureLayer();
	vtStructure3d *pStructure3d;

	if (pStructures->NumSelected() != 1)
		return;

	pStructure3d = pStructures->GetStructure3d(pStructures->GetFirstSelected());
	if (NULL == pStructure3d)
        return;
	if (pStructure3d->GetVIAContributor())
	{
		pStructure3d->SetVIAContributor(false);
		vtGetScene()->GetVisualImpactCalculator().RemoveVisualImpactContributor(pStructure3d->GetContainer());
	}
	else
	{
		pStructure3d->SetVIAContributor(true);
		vtGetScene()->GetVisualImpactCalculator().AddVisualImpactContributor(pStructure3d->GetContainer());
	}
#endif
}

void EnviroFrame::OnUpdatePopupVIA(wxUpdateUIEvent& event)
{
#if VTP_VISUAL_IMPACT_CALCULATOR
	vtTerrain *pTerr = g_App.GetCurrentTerrain();

	if (NULL == pTerr)
		event.Enable(false);
	else
	{
		vtStructureArray3d *pStructures = pTerr->GetStructureLayer();

		if (NULL == pStructures)
			event.Enable(false);
		else
		{
            vtStructure3d *pStructure3d;

			if (pStructures->NumSelected() != 1)
				event.Enable(false);
			else
			{
                pStructure3d = pStructures->GetStructure3d(pStructures->GetFirstSelected());
				if ((NULL != pStructure3d) && pStructure3d->GetVIAContributor())
					event.Check(true);
				else
					event.Check(false);
/* Need to work out why this is in my original code
				if ((NULL == dynamic_cast<vtStructure3d*>(pStr)) || (NULL == dynamic_cast<vtStructure3d*>(pStr)->GetContainer()))
					event.Enable(false);
				else
					event.Enable(true);
*/
			}
		}
	}
#endif
}

void EnviroFrame::OnPopupVIATarget(wxCommandEvent& event)
{
#if VTP_VISUAL_IMPACT_CALCULATOR
	vtTerrain *pTerr = g_App.GetCurrentTerrain();
	vtStructureArray3d *pStructures = pTerr->GetStructureLayer();
	int count = pStructures->size();
	vtStructure3d *pStructure3d;
	vtTransform *pTransform;
	FSphere sphere;

	if (pStructures->NumSelected() != 1)
		return;

	pStructure3d = pStructures->GetStructure3d(pStructures->GetFirstSelected());

	if (NULL == pStructure3d)
        return;

	if (pStructure3d->GetVIATarget())
	{
		pStructure3d->SetVIATarget(false);
		return;
	}

	for (int i = 0; i < count; i++)
	{
        vtStructure3d *pStructure3d = pStructures->GetStructure3d(i);
        if (NULL != pStructure3d)
            pStructure3d->SetVIATarget(false);
	}

	pTransform = pStructure3d->GetContainer();
	if (pTransform)
	{
		// Get the model centre
		pTransform->GetBoundSphere(sphere);
		vtGetScene()->GetVisualImpactCalculator().SetVisualImpactTarget(sphere.center + pTransform->GetTrans());
		pStructure3d->SetVIATarget(true);
	}
#endif
}

void EnviroFrame::OnUpdatePopupVIATarget(wxUpdateUIEvent& event)
{
#if VTP_VISUAL_IMPACT_CALCULATOR
	vtTerrain *pTerr = g_App.GetCurrentTerrain();

	if (NULL == pTerr)
		event.Enable(false);
	else
	{
		vtStructureArray3d *pStructures = pTerr->GetStructureLayer();

		if (NULL == pStructures)
			event.Enable(false);
		else
		{
			vtStructure3d *pStructure3d;

			if (pStructures->NumSelected() != 1)
				event.Enable(false);
			else
			{
				pStructure3d = pStructures->GetStructure3d(pStructures->GetFirstSelected());
				if (NULL == pStructure3d)
					event.Enable(false);
				else
				{
					if (pStructure3d->GetVIATarget())
						event.Check(true);
					else
						event.Check(false);
/* Need to work out why this is in my original code
					if ((NULL == dynamic_cast<vtStructure3d*>(pStr)) || (NULL == dynamic_cast<vtStructure3d*>(pStr)->GetContainer()))
						event.Enable(false);
					else
						event.Enable(true);
*/
				}
			}
		}
	}
#endif
}

