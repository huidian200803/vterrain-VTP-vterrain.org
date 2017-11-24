//
// Name:	 EnviroFrame.cpp
// Purpose:  The frame class for the wxEnviro application.
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef VTUNIX
#include <unistd.h>
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/PickEngines.h"
#include "vtlib/core/SMTerrain.h"
#include "vtlib/core/SRTerrain.h"
#include "vtlib/vtosg/ScreenCaptureHandler.h"

#include "vtdata/vtLog.h"
#include "vtdata/Content.h"
#include "vtdata/DataPath.h"
#include "vtdata/FileFilters.h"
#include "vtui/Helper.h"	// for progress dialog
#include "wxosg/GraphicsWindowWX.h"

#include "EnviroFrame.h"
#include "StatusBar.h"

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
#include "StyleDlg.h"
#include "TinTextureDlg.h"
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

#ifdef VTP_NVIDIA_PERFORMANCE_MONITORING
#include "PerformanceMonitor.h"
#endif

#include "../Options.h"
#include "EnviroGUI.h"	// for GetCurrentTerrain

#include "EnviroApp.h"
#include "EnviroCanvas.h"
#include "menu_id.h"
#include "StatusBar.h"

// Toolbar buttons for non-MSW platforms
#if defined(__WXGTK__) || defined(__WXMOTIF__) || defined(__WXMAC__)
#  include "Enviro_32x32.xpm"
#  include "axes.xpm"
#  include "bld_corner.xpm"
#  include "building.xpm"
#  include "camera.xpm"
#  include "distance.xpm"
#  include "ephemeris.xpm"
#  include "faster.xpm"
#  include "fence.xpm"
#  include "instances.xpm"
#  include "layers.xpm"
#  include "loc.xpm"
#  include "maintain.xpm"
#  include "move.xpm"
#  include "nav.xpm"
#  include "nav_fast.xpm"
#  include "nav_set.xpm"
#  include "nav_slow.xpm"
#  include "placemark.xpm"
#  include "route.xpm"
#  include "scenario.xpm"
#  include "select.xpm"
#  include "select_box.xpm"
#  include "select_move.xpm"
#  include "sgraph.xpm"
#  include "snap.xpm"
#  include "snap_num.xpm"
#  include "space.xpm"
#  include "stop.xpm"
#  include "sun.xpm"
#  include "terrain.xpm"
#  include "tilt.xpm"
#  include "time.xpm"
#  include "tree.xpm"
#  include "unfold.xpm"
#  include "vehicles.xpm"
#  include "view_profile.xpm"
#endif


DECLARE_APP(EnviroApp);

// Helper
class InstanceDlg3d: public InstanceDlg
{
public:
	InstanceDlg3d(EnviroFrame *frame, wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos, const wxSize& size, long style) :
		InstanceDlg( parent, id, title, pos, size, style )
	{
		m_pFrame = frame;
	}
	virtual void OnCreate()
	{
		m_pFrame->CreateInstance(m_pos, GetTagArray());
	}
	EnviroFrame *m_pFrame;
};


/////////////////////////////////////////////////////////////////////////////
//
// Frame constructor
//
EnviroFrame::EnviroFrame(wxFrame *parent, const wxString& title, const wxPoint& pos,
	const wxSize& size, long style, bool bVerticalToolbar, bool bEnableEarth) :
		wxFrame(parent, -1, title, pos, size, style)
{
	VTLOG1("Frame constructor.\n");
	m_bCloseOnIdle = false;

	// tell wxAuiManager to manage this frame
	m_mgr.SetManagedWindow(this);

	// Give the frame an icon
	VTLOG1(" Setting icon\n");
#ifdef ICON_NAME
	SetIcon(wxIcon(wxString(ICON_NAME, wxConvUTF8)));
#else
	SetIcon(wxICON(Enviro));
#endif

#if wxUSE_DRAG_AND_DROP
	SetDropTarget(new DnDFile);
#endif

	m_bCulleveryframe = true;
	m_bAlwaysMove = false;
	m_bFullscreen = false;
	m_bTopDown = false;
	m_ToggledMode = MM_SELECT;
	m_bEnableEarth = bEnableEarth;
	m_bEarthLines = false;
	m_bVerticalToolbar = bVerticalToolbar;

	m_pStatusBar = NULL;
	m_pToolbar = NULL;
}

void EnviroFrame::CreateUI()
{
	VTLOG1("Frame window: creating menus and toolbars.\n");
	CreateMenus();

	// Create StatusBar
	m_pStatusBar = new MyStatusBar(this);
	SetStatusBar(m_pStatusBar);
	m_pStatusBar->Show();
	m_pStatusBar->UpdateText();
	PositionStatusBar();

#ifdef VTP_NVIDIA_PERFORMANCE_MONITORING
	// Stop crash in update toolbar
	m_pCameraDlg = NULL;
	m_pLocationDlg = NULL;
	m_pLODDlg = NULL;
	m_pPerformanceMonitorDlg = NULL;
#endif

	// An array of values to tell wxWidgets how to make our OpenGL context.
	std::vector<int> gl_attribs;
	gl_attribs.push_back(WX_GL_RGBA);			// Full color
	gl_attribs.push_back(WX_GL_DOUBLEBUFFER);
	gl_attribs.push_back(WX_GL_BUFFER_SIZE);	// 24-bit Z-buffer
	gl_attribs.push_back(24);
	gl_attribs.push_back(WX_GL_DEPTH_SIZE);
	gl_attribs.push_back(24);
	VTLOG("Anti-aliasing multisamples: %d\n", g_Options.m_iMultiSamples);
#if wxCHECK_VERSION(2, 9, 0)
	if (g_Options.m_iMultiSamples > 0)
	{
		gl_attribs.push_back(WX_GL_SAMPLE_BUFFERS);
		gl_attribs.push_back(1);
		gl_attribs.push_back(WX_GL_SAMPLES);
		gl_attribs.push_back(g_Options.m_iMultiSamples);
	}
#endif
	if (g_Options.m_bStereo && g_Options.m_iStereoMode == 1)	// 1 = Quad-buffer stereo
	{
		gl_attribs.push_back(WX_GL_STEREO);
	}
	// Zero terminates the array
	gl_attribs.push_back(0);

	VTLOG("Frame window: creating view canvas.\n");
	m_canvas = new EnviroCanvas(this, -1, wxPoint(0, 0), wxSize(-1, -1), 0,
		_T("vtGLCanvas"), &gl_attribs.front());

	// Show the frame
	VTLOG("Showing the main frame\n");
	Show(true);

	VTLOG("Constructing dialogs\n");
	m_pBuildingDlg = new BuildingDlg3d(this, -1, _("Building Properties"));
	m_pCameraDlg = new CameraDlg(this, -1, _("Camera-View"));
	m_pDistanceDlg = new DistanceDlg3d(this, -1, _("Distance"));
	m_pEphemDlg = new EphemDlg(this, -1, _("Ephemeris"));
	m_pFenceDlg = new LinearStructureDlg3d(this, -1, _("Linear Structures"));
	m_pInstanceDlg = new InstanceDlg3d(this, this, -1, _("Instances"),
		wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pTagDlg = new TagDlg(this, -1, _("Tags"), wxDefaultPosition,
		wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pTagDlg->SetSize(440,80);
	m_pLODDlg = new LODDlg(this, -1, _("Terrain LOD Info"), wxDefaultPosition,
		wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	// m_pLODDlg->Show();	// Enable this to see the LOD dialog immediately

	m_pPlantDlg = new PlantDlg(this, -1, _("Plants"));
	m_pPlantDlg->ShowOnlyAvailableSpecies(g_Options.m_bOnlyAvailableSpecies);
	m_pPlantDlg->SetLang(wxGetApp().GetLanguageCode());

	m_pLocationDlg = new LocationDlg(this, -1, _("Locations"),
			wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pSceneGraphDlg = new SceneGraphDlg(this, -1, _("Scene Graph"),
			wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pSceneGraphDlg->SetSize(450, 600);
	m_pTimeDlg = new TimeDlg(this, -1, _("Time"));
	m_pUtilDlg = new UtilDlg(this, -1, _("Utility Poles"));
	m_pScenarioSelectDialog = new ScenarioSelectDialog(this, -1, _("Scenarios"));
	m_pVehicleDlg = new VehicleDlg(this, -1, _("Vehicles"));
	m_pDriveDlg = new DriveDlg(this);
	m_pProfileDlg = NULL;
	#ifdef VTP_NVIDIA_PERFORMANCE_MONITORING
    m_pPerformanceMonitorDlg = new CPerformanceMonitorDialog(this, wxID_ANY, _("Performance Monitor"));
    #endif
	m_pVIADlg = new VIADlg(this);

#if wxVERSION_NUMBER < 2900		// before 2.9.0
	if (m_canvas)
		m_canvas->SetCurrent();
#else
	// Still need to do a "SetCurrent" here? It's more complicated now in wx 2.9.x,
	//  and it's probably already taken care of by GraphicsWindowWX?
#endif

	m_mgr.AddPane(m_canvas, wxAuiPaneInfo().
				  Name(wxT("canvas")).Caption(wxT("Canvas")).
				  CenterPane());
	m_mgr.Update();

	m_pLayerDlg = new LayerDlg(this, -1, _("Layers"), wxDefaultPosition,
		wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pLayerDlg->SetSize(500, -1);

	m_mgr.AddPane(m_pLayerDlg, wxAuiPaneInfo().
				  Name(_T("layers")).Caption(_("Layers")).
				  Left());
	m_mgr.Update();
}

EnviroFrame::~EnviroFrame()
{
	VTLOG("Deleting Frame\n");

	m_mgr.UnInit();

	DeleteCanvas();

	delete m_pSceneGraphDlg;
	delete m_pPlantDlg;
	delete m_pFenceDlg;
	delete m_pTagDlg;
	delete m_pUtilDlg;
	delete m_pCameraDlg;
	delete m_pLocationDlg;
	delete m_pInstanceDlg;
	delete m_pLayerDlg;
	#ifdef VTP_NVIDIA_PERFORMANCE_MONITORING
    delete m_pPerformanceMonitorDlg;
    #endif
	delete m_pVIADlg;

	delete m_pStatusBar;
	delete m_pToolbar;
	SetStatusBar(NULL);
}

void EnviroFrame::DeleteCanvas()
{
	// Tell our graphics context that there is no canvas.
	GraphicsWindowWX *pGW = (GraphicsWindowWX*) vtGetScene()->GetGraphicsWindow();
	if (pGW) {
		pGW->CloseOsgContext();
		pGW->SetCanvas(NULL);
	}
	if (m_canvas)
	{
		m_canvas->m_bRunning = false;
		delete m_canvas;
		m_canvas = NULL;
	}
}

void EnviroFrame::CreateToolbar()
{
	long style = wxTB_FLAT | wxTB_NODIVIDER;	// wxTB_DOCKABLE is GTK-only
	if (m_bVerticalToolbar)
		style |= wxTB_VERTICAL;

	delete m_pToolbar;

	// Create
	m_pToolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
					               style);
	m_pToolbar->SetMargins(1, 1);
	m_pToolbar->SetToolBitmapSize(wxSize(20, 20));

	// populate the toolbar with buttons
	RefreshToolbar();

	wxAuiPaneInfo api;
	api.Name(_T("toolbar"));
	api.ToolbarPane();
	if (m_bVerticalToolbar)
	{
		api.GripperTop();
		api.Left();
		api.TopDockable(false);
		api.BottomDockable(false);
	}
	else
	{
		api.Top();
		api.LeftDockable(false);
		api.RightDockable(false);
	}
	wxSize best = m_pToolbar->GetBestSize();
	api.MinSize(best);
	api.Floatable(false);
	m_mgr.AddPane(m_pToolbar, api);
	m_mgr.Update();
}

void EnviroFrame::RefreshToolbar()
{
	if (!m_pToolbar)	// safety check
		return;

	// remove any existing buttons
	int count = m_pToolbar->GetToolsCount();
#ifdef __WXMAC__
	// Nino says: I spent a long time with this one, the issue was definitely
	// deep in wxMac, but I don't remember, nor want to repeat it :).  Can we
	// #ifdef __WXMAC__ for the time being to revisit it after Xcode is working?
	while (count >= 1)
	{
		m_pToolbar->DeleteToolByPos(count-1);
		count = m_pToolbar->GetToolsCount();
	}
	m_pToolbar->Realize();
	AddTool(ID_TOOLS_SELECT, wxBITMAP(select), _("Select"), true);
#else
	if (!count)
	{
		AddTool(ID_TOOLS_SELECT, wxBITMAP(select), _("Select"), true);
		count = 1;
	}
	while (count > 1)
	{
		m_pToolbar->DeleteToolByPos(count-1);
		count = m_pToolbar->GetToolsCount();
	}
#endif

	bool bEarth = (g_App.m_state == AS_Orbit);
	bool bTerr = (g_App.m_state == AS_Terrain);

	if (bTerr)
	{
		AddTool(ID_TOOLS_SELECT_BOX, wxBITMAP(select_box), _("Select Box"), true);
		if (g_Options.m_bShowToolsCulture)
		{
			AddTool(ID_TOOLS_SELECT_MOVE, wxBITMAP(select_move), _("Select and Move"), true);
			AddTool(ID_TOOLS_MOVE, wxBITMAP(move), _("Move Objects"), true);
			AddTool(ID_TOOLS_FENCES, wxBITMAP(fence), _("Create Fences"), true);
			AddTool(ID_TOOLS_BUILDINGS, wxBITMAP(building), _("Create Buildings"), true);
			AddTool(ID_TOOLS_POWER, wxBITMAP(route), _("Create Powerlines"), true);
			AddTool(ID_TOOLS_PLANTS, wxBITMAP(tree), _("Create Plants"), true);
		}
		AddTool(ID_TOOLS_POINTS, wxBITMAP(placemark), _("Create Points"), true);
		if (g_Options.m_bShowToolsCulture)
		{
			AddTool(ID_TOOLS_INSTANCES, wxBITMAP(instances), _("Create Instances"), true);
			AddTool(ID_TOOLS_VEHICLES, wxBITMAP(vehicles), _("Create Vehicles"), true);
		}
		AddTool(ID_TOOLS_NAVIGATE, wxBITMAP(nav), _("Navigate"), true);
	}
	if (bTerr || bEarth)
	{
		AddTool(ID_TOOLS_MEASURE, wxBITMAP(distance), _("Measure Distance"), true);
	}
	if (bTerr)
	{
		m_pToolbar->AddSeparator();
		AddTool(ID_VIEW_PROFILE, wxBITMAP(view_profile), _("Elevation Profile"), true);
		AddTool(ID_TOOLS_CONSTRAIN, wxBITMAP(bld_corner), _("Constrain Angles"), true);
	}
	if (g_Options.m_bShowToolsSnapshot)
	{
		m_pToolbar->AddSeparator();
		AddTool(ID_VIEW_SNAPSHOT, wxBITMAP(snap), _("Snapshot"), false);
		AddTool(ID_VIEW_SNAP_AGAIN, wxBITMAP(snap_num), _("Numbered Snapshot"), false);
	}
	if (bTerr || bEarth)
	{
		m_pToolbar->AddSeparator();
		AddTool(ID_FILE_LAYERS, wxBITMAP(layers), _("Show Layer Dialog"), false);
	}
	if (bTerr)
	{
		m_pToolbar->AddSeparator();
		AddTool(ID_VIEW_MAINTAIN, wxBITMAP(maintain), _("Maintain Height"), true);
		AddTool(ID_VIEW_FASTER, wxBITMAP(nav_fast), _("Fly Faster"), false);
		AddTool(ID_VIEW_SLOWER, wxBITMAP(nav_slow), _("Fly Slower"), false);
		AddTool(ID_VIEW_SETTINGS, wxBITMAP(nav_set), _("Camera Dialog"), false);
		AddTool(ID_VIEW_LOCATIONS, wxBITMAP(loc), _("Locations"), false);
	}
	if (m_bEnableEarth)
	{
		m_pToolbar->AddSeparator();
		if (bTerr)
		{
			AddTool(ID_SCENE_EPHEMERIS, wxBITMAP(ephemeris), _("Ephemeris"), false);
			AddTool(ID_SCENE_SPACE, wxBITMAP(space), _("Go to Space"), false);
		}
		AddTool(ID_SCENE_TERRAIN, wxBITMAP(terrain), _("Go to Terrain"), false);
		if (bEarth)
		{
			m_pToolbar->AddSeparator();
			AddTool(ID_EARTH_SHOWSHADING, wxBITMAP(sun), _("Show Sunlight"), true);
			AddTool(ID_EARTH_SHOWAXES, wxBITMAP(axes), _("Axes"), true);
			AddTool(ID_EARTH_TILT, wxBITMAP(tilt), _("Tilt"), true);
			AddTool(ID_EARTH_UNFOLD, wxBITMAP(unfold), _("Unfold"), true);
		}
	}
	if (g_Options.m_bShowToolsTime)
	{
		m_pToolbar->AddSeparator();
		AddTool(ID_TIME_DIALOG, wxBITMAP(time), _("Time"), false);
		AddTool(ID_TIME_FASTER, wxBITMAP(faster), _("Time Faster"), false);
		AddTool(ID_TIME_STOP, wxBITMAP(stop), _("Time Stop"), false);
	}

	m_pToolbar->AddSeparator();
	AddTool(ID_SCENE_SCENEGRAPH, wxBITMAP(sgraph), _("Scene Graph"), false);

	VTLOG1("Realize toolbar.\n");
	m_pToolbar->Realize();

	// "commit" all changes made to wxAuiManager
	wxAuiPaneInfo &api = m_mgr.GetPane(wxT("toolbar"));
	if (api.IsOk())
	{
		wxSize best = m_pToolbar->GetBestSize();
		api.MinSize(best);
		m_mgr.Update();
	}
}

void EnviroFrame::AddTool(int id, const wxBitmap &bmp, const wxString &tooltip, bool tog)
{
	m_pToolbar->AddTool(id, tooltip, bmp, tooltip,
		tog ? wxITEM_CHECK : wxITEM_NORMAL);
}

/////////////////////////////////////////////////////////////////////////////
//
// wx Event handlers
//

void EnviroFrame::OnChar(wxKeyEvent& event)
{
	static NavType prev = NT_Normal;
	vtTerrain *pTerr = g_App.GetCurrentTerrain();
	long key = event.GetKeyCode();

	// Keyboard shortcuts ("accelerators")
	switch (key)
	{
	case 27:
		// Esc: cancel a building or linear we're making
		if (g_App.m_mode == MM_BUILDINGS && g_App.IsMakingElastic())
			g_App.CancelElastic();
		else if (g_App.m_mode == MM_LINEARS && g_App.IsMakingElastic())
			g_App.CancelElastic();
		else	// or exit the application
		{
			// It's not safe to close immediately, as that will kill the canvas,
			//  and it might some Canvas event that caused us to close.  So,
			//  simply stop rendering, and delay closing until the next Idle event.
			m_canvas->m_bRunning = false;
			m_bCloseOnIdle = true;
		}
		break;

	case ' ':
		if (g_App.m_state == AS_Terrain)
			ToggleNavigate();
		break;

	case 'f':
		ChangeFlightSpeed(1.8f);
		break;
	case 's':
		ChangeFlightSpeed(1.0f / 1.8f);
		break;
	case 'a':
		g_App.SetMaintain(!g_App.GetMaintain());
		break;

	case '+':
		SetTerrainDetail(GetTerrainDetail()+1000);
		break;
	case '-':
		SetTerrainDetail(GetTerrainDetail()-1000);
		break;

	case 'd':
		// Toggle grab-pivot
		if (g_App.m_nav == NT_Grab)
			g_App.SetNavType(prev);
		else
		{
			prev = g_App.m_nav;
			g_App.SetNavType(NT_Grab);
		}
		break;

	case 'w':
		m_bAlwaysMove = !m_bAlwaysMove;
		if (g_App.m_pTFlyer != NULL)
			g_App.m_pTFlyer->SetAlwaysMove(m_bAlwaysMove);
		break;

	case '[':
		{
			float exag = pTerr->GetVerticalExag();
			exag /= 1.01;
			pTerr->SetVerticalExag(exag);
		}
		break;
	case ']':
		{
			float exag = pTerr->GetVerticalExag();
			exag *= 1.01;
			pTerr->SetVerticalExag(exag);
		}
		break;

	case 'e':
		m_bEarthLines = !m_bEarthLines;
		g_App.ShowEarthLines(m_bEarthLines);
		break;

	case 'y':
		// Example code: modify the terrain by using the (slow) approach of using
		//  vtTerrain methods GetInitialGrid and UpdateElevation.
		{
			vtTerrain *pTerr = g_App.GetCurrentTerrain();
			if (pTerr && pTerr->GetParams().GetValueBool(STR_ALLOW_GRID_SCULPTING))
			{
				vtElevationGrid	*grid = pTerr->GetInitialGrid();
				if (grid)
				{
					clock_t t1 = clock();
					// Raise an area of the terrain
					int cols, rows;
					grid->GetDimensions(cols, rows);
					for (int i  = cols / 4; i < cols / 2; i++)
						for (int j = rows / 4; j < rows / 2; j++)
						{
							grid->SetFValue(i, j, grid->GetFValue(i, j) + 40);
						}
					pTerr->UpdateElevation();
					clock_t t2 = clock();
					VTLOG(" Modify1: %.3f sec\n", (float)(t2-t1)/CLOCKS_PER_SEC);

					// Update the shading and culture
					pTerr->ReshadeTexture(vtGetTS()->GetSunLightTransform());
					DRECT area;
					area.SetToZero();
					pTerr->RedrapeCulture(area);
				}
			}
		}
		break;
	case 'Y':
		// Example code: modify the terrain by using the (fast) approach of using
		//  vtDynTerrainGeom::SetElevation.
		{
			vtTerrain *pTerr = g_App.GetCurrentTerrain();
			if (pTerr)
			{
				vtDynTerrainGeom *dyn = pTerr->GetDynTerrain();
				if (dyn)
				{
					clock_t t1 = clock();
					// Raise an area of the terrain
					int cols, rows;
					dyn->GetDimensions(cols, rows);
					for (int i  = cols / 4; i < cols / 2; i++)
						for (int j = rows / 4; j < rows / 2; j++)
						{
							dyn->SetElevation(i, j, dyn->GetElevation(i, j) + 40);
						}
					clock_t t2 = clock();
					VTLOG(" Modify2: %.3f sec\n", (float)(t2-t1)/CLOCKS_PER_SEC);

					// Update the shading and culture
					pTerr->ReshadeTexture(vtGetTS()->GetSunLightTransform());
					DRECT area;
					area.SetToZero();
					pTerr->RedrapeCulture(area);
				}
			}
		}
		break;
	case 'u':
		// Example code: modify a small area of terrain around the mouse pointer.
		{
			vtTerrain *pTerr = g_App.GetCurrentTerrain();
			if (pTerr)
			{
				vtDynTerrainGeom *dyn = pTerr->GetDynTerrain();
				if (dyn)
				{
					// Get 3D cursor location in grid coordinates
					FPoint3 fpos;
					g_App.m_pTerrainPicker->GetCurrentPoint(fpos);
					IPoint2 ipos;
					dyn->WorldToGrid(fpos, ipos);
					for (int x  = -4; x < 4; x++)
						for (int y = -4; y < 4; y++)
						{
							float val = dyn->GetElevation(ipos.x + x, ipos.y + y);
							dyn->SetElevation(ipos.x + x, ipos.y + y, val + 40);
						}

					// Update the (entire) shading and culture
					pTerr->ReshadeTexture(vtGetTS()->GetSunLightTransform());
					DRECT area;
					area.SetToZero();
					pTerr->RedrapeCulture(area);
				}
			}
		}
		break;
	case 'D':	// Shift-D
		// dump camera info
		g_App.DumpCameraInfo();
		break;

	case 2:	// Ctrl-B
		// toggle demo
		g_App.ToggleDemo();
		break;

	case WXK_F11:
		DoTestCode();
		break;

	case WXK_F12:
		m_pSceneGraphDlg->Show(true);
		break;

	case WXK_DELETE:
		DeleteAllSelected();
		break;

	default:
		event.Skip();
		break;
	}
}

void EnviroFrame::OnClose(wxCloseEvent &event)
{
	VTLOG1("Got Close event.\n");
	bool bReally = true;

	bool bUnsavedChanges = false;
	vtTerrain *terr = g_App.GetCurrentTerrain();
	if (terr)
	{
		LayerSet &set = terr->GetLayers();
		for (uint i = 0; i < set.size(); i++)
		{
			vtLayer *lay = set[i];
			if (lay->GetModified())
				bUnsavedChanges = true;
		}
	}

	if (event.CanVeto() && bUnsavedChanges)
	{
		// Pause rendering
		m_canvas->m_bRunning = false;
		int ret = wxMessageBox(_("Really Exit?"), _T("Enviro"), wxYES_NO);
		if (ret == wxNO)
		{
			event.Veto();
			bReally = false;
			// Resume rendering
			m_canvas->m_bRunning = true;
		}
	}
	if (bReally)
	{
		DeleteCanvas();
		Destroy();
	}
}

void EnviroFrame::OnIdle(wxIdleEvent& event)
{
	// Check if we were requested to close on the next Idle event.
	if (m_bCloseOnIdle)
	{
		m_bCloseOnIdle = false;
		Close(false);	// False means: don't force a close.
	}
	else
		event.Skip();
}

#ifdef __WXMSW__
// Catch special events, or calls an appropriate default window procedure
WXLRESULT EnviroFrame::MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
{
	if (nMsg == WM_ENTERMENULOOP)
	{
		//VTLOG1("WM_ENTERMENULOOP\n");
		EnableContinuousRendering(false);
	}
	else if (nMsg == WM_EXITMENULOOP)
	{
		//VTLOG1("WM_EXITMENULOOP\n");
		EnableContinuousRendering(true);
	}
	else if (nMsg == WM_ENTERSIZEMOVE)
	{
		//VTLOG1("WM_ENTERSIZEMOVE\n");
		EnableContinuousRendering(false);
	}
	else if (nMsg == WM_EXITSIZEMOVE)
	{
		//VTLOG1("WM_EXITSIZEMOVE\n");
		EnableContinuousRendering(true);
	}
	return wxFrame::MSWWindowProc(nMsg, wParam, lParam);
}
#endif	// __WXMSW__


/////////////////////////////////////////////////////////////////////////////
//
// Utility methods
//

void EnviroFrame::SetMode(MouseMode mode)
{
	// Show/hide the modeless dialogs as appropriate
	m_pUtilDlg->Show(mode == MM_POWER);
	m_pInstanceDlg->Show(mode == MM_INSTANCES);
	m_pDistanceDlg->Show(mode == MM_MEASURE);
	m_pVehicleDlg->Show(mode == MM_VEHICLES);

	g_App.SetMode(mode);

	if (mode == MM_LINEARS)
		OpenFenceDialog();
	else
		m_pFenceDlg->Show(false);

	// Show/hide plant dialog
	if (mode == MM_PLANTS)
	{
		VTLOG1("Calling Plant dialog\n");

		// Make sure the species file and appearances are available
		g_App.LoadSpeciesList();
		g_App.GetCurrentTerrain()->SetSpeciesList(g_App.GetSpeciesList());
		g_App.ActivateAVegetationLayer();

		m_pPlantDlg->SetSpeciesList(g_App.GetSpeciesList());
		m_pPlantDlg->SetDlgPlantOptions(g_App.GetPlantOptions());
	}
	m_pPlantDlg->Show(mode == MM_PLANTS);

	if (mode == MM_LINEARS || mode == MM_BUILDINGS || mode == MM_INSTANCES)
	{
		g_App.ActivateAStructureLayer();
	}
}

#include <osgShadow/ShadowedScene>
#include <osgShadow/ShadowMap>
#include <osgShadow/ShadowTexture>
#include <osgDB/ReadFile>

void EnviroFrame::LoadClouds(const char *fname)
{
	ImagePtr input = osgDB::readImageFile(fname);
	if (input.valid())
	{
		int depth = input->getPixelSizeInBits();
		if (depth != 8)
			DisplayAndLog("That isn't an 8-bit cloud image.");
		else
		{
			// For transparency, convert the 8-bit (from black to white) to a
			//  32-bit (RGB is white, Alpha is 0-255)
			uint w = input->s();
			uint h = input->t();

			vtImagePtr img2 = new vtImage;
			img2->Allocate(IPoint2(w, h), 32);
			RGBAi rgba(255,255,255,0);
			for (uint i = 0; i < w; i++)
				for (uint j = 0; j < h; j++)
				{
					rgba.a = GetPixel8(input, i, j);
					img2->SetPixel32(i, j, rgba);
				}

			OpenProgressDialog(_T("Processing Images"), _T(""), false, this);
			g_App.MakeOverlayGlobe(img2, progress_callback);
			CloseProgressDialog();
		}
	}
	else
		DisplayAndLog("Couldn't read input file.");
}

void EnviroFrame::ToggleNavigate()
{
	MouseMode current = g_App.m_mode;
	if (current == MM_NAVIGATE && m_ToggledMode != MM_NAVIGATE)
		SetMode(m_ToggledMode);
	else
	{
		m_ToggledMode = current;
		SetMode(MM_NAVIGATE);
	}
}

void EnviroFrame::ChangeFlightSpeed(float factor)
{
	float speed = g_App.GetFlightSpeed();
	g_App.SetFlightSpeed(speed * factor);

	VTLOG("Change flight speed to %f\n", speed * factor);

	m_pCameraDlg->GetValues();
	m_pCameraDlg->ValuesToSliders();
	m_pCameraDlg->TransferToWindow();
	m_pCameraDlg->Refresh();

	// Also set spacenavigator speed.  Scaling from mouse to spacenav is
	//  approximate, based on the magnitude and number of INPUT events the
	//  spacenav appears to send, 100x seems to be rough correlation.
	m_canvas->SetSpaceNavSpeed(speed * factor / 100);
}

void EnviroFrame::SetTerrainDetail(int iMetric)
{
	vtTerrain *pTerr = g_App.GetCurrentTerrain();
	if (!pTerr)
		return;

	vtDynTerrainGeom *pDyn = pTerr->GetDynTerrain();
	if (pDyn)
		return pDyn->SetPolygonTarget(iMetric);

	vtTiledGeom *pTiled = pTerr->GetTiledGeom();
	if (pTiled)
		return pTiled->SetVertexTarget(iMetric);
}

int EnviroFrame::GetTerrainDetail()
{
	const vtTerrain *pTerr = g_App.GetCurrentTerrain();
	if (!pTerr)
		return 0;

	const vtDynTerrainGeom *pDyn = pTerr->GetDynTerrain();
	if (pDyn)
		return pDyn->GetPolygonTarget();

	const vtTiledGeom *pTiled = pTerr->GetTiledGeom();
	if (pTiled)
		return pTiled->GetVertexTarget();

	return 0;
}

void EnviroFrame::ChangePagingRange(float prange)
{
	vtTerrain *pTerr = g_App.GetCurrentTerrain();
	if (!pTerr) return;
	vtTiledGeom *pTiled = pTerr->GetTiledGeom();
	if (pTiled)
		pTiled->SetPagingRange(prange);
}

void EnviroFrame::SetFullScreen(bool bFull)
{
	m_bFullscreen = bFull;
#ifdef __WXMSW__
	if (m_bFullscreen)
	{
		ShowFullScreen(true, wxFULLSCREEN_NOMENUBAR |
//							 wxFULLSCREEN_NOTOOLBAR |	// leave toolbar visible
			wxFULLSCREEN_NOSTATUSBAR |
			wxFULLSCREEN_NOBORDER |
			wxFULLSCREEN_NOCAPTION );
	}
	else
		ShowFullScreen(false);
#else
  /*  FIXME - ShowFullScreen not implemented in wxGTK 2.2.5.  */
  /*   Do full-screen another way.                           */
#endif
}

void EnviroFrame::CreateInstance(const DPoint2 &pos, vtTagArray *tags)
{
	// Just pass it along to the app.
	g_App.CreateInstanceAt(pos, tags);
}

void EnviroFrame::Snapshot(bool bNumbered)
{
	VTLOG1("EnviroFrame::Snapshot\n");

	wxString use_name;
	if (!bNumbered || (bNumbered && m_strSnapshotFilename == _T("")))
	{
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
		if (bNumbered)
		{
			m_strSnapshotFilename = saveFile.GetPath();
			m_iSnapshotNumber = 0;
		}
		else
			use_name = saveFile.GetPath();
	}
	if (bNumbered)
	{
		// Append the number of the snapshot to the filename
		wxString start, number, extension;
		start = m_strSnapshotFilename.BeforeLast(_T('.'));
		extension = m_strSnapshotFilename.AfterLast(_T('.'));
		number.Printf(_T("_%03d."), m_iSnapshotNumber);
		m_iSnapshotNumber++;
		use_name = start + number + extension;
	}

	std::string Filename(use_name.mb_str(wxConvUTF8));
	CScreenCaptureHandler::SetupScreenCapture(Filename);
}

void EnviroFrame::ParseCommandLine(const char *cmdstart, char **argv, char *args, int *numargs, int *numchars)
{
	static const char NULCHAR = '\0';
	static const char SPACECHAR = ' ';
	static const char TABCHAR = '\t';
	static const char DQUOTECHAR = '\"';
	static const char SLASHCHAR = '\\';
	const char *p;
	int inquote;			// 1 = inside quotes
	int copychar;			// 1 = copy char to *args
	unsigned numslash;		// num of backslashes seen

	*numchars = 0;
	*numargs = 0;

	p = cmdstart;

	inquote = 0;

	/* loop on each argument */
	for(;;)
	{
		if ( *p )
		{
			while (*p == SPACECHAR || *p == TABCHAR)
				++p;
		}

		if (*p == NULCHAR)
			break;              // end of args

		// scan an argument
		if (argv)
			*argv++ = args;     // store ptr to arg
		++*numargs;

		// loop through scanning one argument
		for (;;)
		{
			copychar = 1;
			// Rules: 2N backslashes + " ==> N backslashes and begin/end quote
			// 2N+1 backslashes + " ==> N backslashes + literal "
			// N backslashes ==> N backslashes
			numslash = 0;
			while (*p == SLASHCHAR)
			{
				// count number of backslashes for use below
				++p;
				++numslash;
			}
			if (*p == DQUOTECHAR)
			{
				// if 2N backslashes before, start/end quote, otherwise
				// copy literally
				if (numslash % 2 == 0)
				{
					if (inquote)
					{
						if (p[1] == DQUOTECHAR)
							p++;    // Double quote inside quoted string
						else        // skip first quote char and copy second
							copychar = 0;
					}
					else
						copychar = 0;       // don't copy quote

					inquote = !inquote;
				}
				numslash /= 2;          // divide numslash by two
			}

			// copy slashes
			while (numslash--)
			{
				if (args)
					*args++ = SLASHCHAR;
				++*numchars;
			}

			// if at end of arg, break loop
			if (*p == NULCHAR || (!inquote && (*p == SPACECHAR || *p == TABCHAR)))
				break;

			// copy character into argument
			if (copychar)
			{
				if (args)
					*args++ = *p;
				++*numchars;
			}
			++p;
		}

		// null-terminate the argument
		if (args)
			*args++ = NULCHAR;          // terminate string
		++*numchars;
	}

	/* We put one last argument in -- a null ptr */
	if (argv)
		*argv++ = NULL;
	++*numargs;
}

//
// Called when the GUI needs to be informed of a new terrain
//
void EnviroFrame::SetTerrainToGUI(vtTerrain *pTerrain)
{
	// Some dialogs need to be informed of the current terrain (even if it is NULL)
	m_pCameraDlg->SetTerrain(pTerrain);
	m_pEphemDlg->SetTerrain(pTerrain);
	m_pScenarioSelectDialog->SetTerrain(pTerrain);
	m_pLayerDlg->SetTerrain(pTerrain);
	m_pLODDlg->SetTerrain(pTerrain);

	if (pTerrain)
	{
		m_pLocationDlg->SetLocSaver(pTerrain->GetLocSaver());
		m_pLocationDlg->SetAnimContainer(pTerrain->GetAnimContainer());

		m_pInstanceDlg->SetCRS(pTerrain->GetCRS());
		m_pDistanceDlg->SetCRS(pTerrain->GetCRS());

		// Fill instance dialog with global and terrain-specific content
		m_pInstanceDlg->ClearContent();
		m_pInstanceDlg->AddContent(&vtGetContent());
		if (pTerrain->m_Content.NumItems() != 0)
			m_pInstanceDlg->AddContent(&pTerrain->m_Content);

		// Also switch the time dialog to the time engine of the terrain,
		//  not the globe.
		SetTimeEngine(vtGetTS()->GetTimeEngine());

		// If there is paging involved, Inform the LOD dialog
		vtTiledGeom *geom = pTerrain->GetTiledGeom();
		if (geom && m_pLODDlg)
			m_pLODDlg->SetPagingRange(geom->prange_min, geom->prange_max);

		bool bAllowRoll = pTerrain->GetParams().GetValueBool(STR_ALLOW_ROLL);
		m_canvas->SetSpaceNavAllowRoll(bAllowRoll);
	}
	else
	{
		vtCRS geo;
		OGRErr err = geo.SetGeogCSFromDatum(EPSG_DATUM_WGS84);
		if (err == OGRERR_NONE)
			m_pDistanceDlg->SetCRS(geo);
	}
	// Update Title
	SetTitle(wxGetApp().MakeFrameTitle(pTerrain));
}

//
// Called when the Earth View has been constructed
//
void EnviroFrame::SetTimeEngine(vtTimeEngine *pEngine)
{
	m_pTimeDlg->SetTimeEngine(pEngine);
	// poke it once to let the time dialog know
	pEngine->SetTime(pEngine->GetTime());
}

void EnviroFrame::EarthPosUpdated(const DPoint3 &pos)
{
	m_pInstanceDlg->SetLocation(DPoint2(pos.x, pos.y));
}

void EnviroFrame::CameraChanged()
{
	// we are dealing with a new camera, so update with its values
	if (m_pCameraDlg && m_pCameraDlg->IsShown())
		m_pCameraDlg->CameraChanged();
}

void EnviroFrame::OnDrop(const wxString &str)
{
	vtString utf8 = (const char *) str.ToUTF8();

	if (!str.Right(4).CmpNoCase(_T(".kml")))
	{
		g_App.ImportModelFromKML(utf8);
	}
	else
		LoadLayer(utf8);
}

void EnviroFrame::LoadLayer(vtString &fname)
{
	VTLOG("EnviroFrame::LoadLayer '%s'\n", (const char *) fname);
	bool success = false;

	if (g_App.m_state == AS_Terrain)
	{
		vtTerrain *pTerr = g_App.GetCurrentTerrain();

		OpenProgressDialog(_("Loading..."), wxString::FromUTF8(fname), false, this);
		pTerr->SetProgressCallback(progress_callback);

		success = LoadTerrainLayer(fname);

		CloseProgressDialog();
	}
	else if (g_App.m_state == AS_Orbit)
	{
		// earth view
		int ret = g_App.AddGlobeAbstractLayer(fname);
		if (ret == -1)
			wxMessageBox(_("Couldn't open"));
		else if (ret == -2)
			wxMessageBox(_("That file isn't point data."));
		else
			success = true;
	}
	if (success)
		m_pLayerDlg->RefreshTreeContents();
}

bool EnviroFrame::LoadTerrainLayer(vtString &fname)
{
	VTLOG("LoadTerrainLayer '%s'\n", (const char *) fname);

	vtTerrain *pTerr = g_App.GetCurrentTerrain();
	bool success = false;

	vtString ext = GetExtension(fname);
	if (!ext.CompareNoCase(".vtst"))
	{
		MakeRelativeToDataPath(fname, "BuildingData");

		vtStructureLayer *st_layer = pTerr->NewStructureLayer();
		st_layer->SetLayerName(fname);
		if (!st_layer->Load())
		{
			VTLOG("\tCouldn't load structures.\n");
			// Removing the layer deletes it, by dereference.
			pTerr->GetLayers().Remove(st_layer);
		}
		else
		{
			pTerr->CreateStructures(st_layer);
			success = true;
		}
	}
	else if (!ext.CompareNoCase(".vf"))
	{
		MakeRelativeToDataPath(fname, "PlantData");

		vtVegLayer *v_layer = pTerr->LoadVegetation(fname);
		if (v_layer)
			success = true;
	}
	else if (!ext.CompareNoCase(".shp"))
	{
		bool bRelative = MakeRelativeToDataPath(fname, "PointData");
		if (!bRelative)
			bRelative = MakeRelativeToDataPath(fname, "");
		if (bRelative)
			VTLOG("Shortened path to '%s'\n", (const char *) fname);

		vtAbstractLayer *ab_layer = pTerr->NewAbstractLayer();
		ab_layer->SetLayerName(fname);

		// TODO here: progress dialog on load?
		if (ab_layer->Load(pTerr->GetCRS(), NULL))
		{
			VTLOG("Successfully read features from file '%s'\n", (const char *) fname);

			// Abstract layers aren't constructed yet, giving us a chance to ask
			// for styling.
			vtTagArray &props = ab_layer->Props();

			StyleDlg dlg(NULL, -1, _("Style"), wxDefaultPosition, wxDefaultSize,
				wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
			dlg.SetFeatureSet(ab_layer->GetFeatureSet());
			dlg.SetOptions(props);
			if (dlg.ShowModal() == wxID_OK)
			{
				// Copy all the style attributes to the new featureset
				VTLOG1("  Setting featureset properties.\n");
				dlg.GetOptions(props);

				pTerr->CreateAbstractLayerVisuals(ab_layer);
				success = true;
			}
		}
		if (!success)
		{
			VTLOG("Couldn't read features from file '%s'\n", (const char *) fname);
			pTerr->RemoveLayer(ab_layer);
		}
	}
	else if (!ext.CompareNoCase(".itf"))
	{
		MakeRelativeToDataPath(fname, "Elevation");

		vtTagArray tags;
		tags.SetValueString("Type", TERR_LTYPE_ELEVATION);
		tags.SetValueString("Filename", fname);
		tags.SetValueFloat(STR_OPACITY, 1.0f);
		tags.SetValueFloat(STR_GEOTYPICAL_SCALE, 10.0f);

		TinTextureDlg dlg(this, -1, _("TIN Texture"));
		dlg.SetOptions(tags);
		if (dlg.ShowModal() == wxID_OK)
		{
			dlg.GetOptions(tags);
			success = pTerr->CreateElevLayerFromTags(tags);
		}
	}
	else if (!ext.CompareNoCase(".tif"))
	{
		MakeRelativeToDataPath(fname, "GeoSpecific");

		vtImageLayer *im_layer = pTerr->NewImageLayer();
		im_layer->Props().SetValueString("Filename", fname);
		if (!im_layer->Load())
		{
			VTLOG("\tCouldn't load image layer.\n");
			// Removing the layer deletes it, by dereference.
			pTerr->GetLayers().Remove(im_layer);
		}
		else
		{
			pTerr->AddMultiTextureOverlay(im_layer);
			success = true;
		}
	}
	return success;
}

void EnviroFrame::UpdateStatus()
{
	if (m_pStatusBar)
		m_pStatusBar->UpdateText();

	if (m_pCameraDlg && m_pCameraDlg->IsShown())
		m_pCameraDlg->CheckAndUpdatePos();

	if (m_pLocationDlg && m_pLocationDlg->IsShown())
		m_pLocationDlg->Update();
}

void EnviroFrame::UpdateLODInfo()
{
	if (!m_pLODDlg)
		return;

	vtTerrain *terr = g_App.GetCurrentTerrain();
	if (!terr)
		return;
	vtTiledGeom *geom = terr->GetTiledGeom();
	if (geom)
	{
		float fmin = log(TILEDGEOM_RESOLUTION_MIN);
		float fmax = log(TILEDGEOM_RESOLUTION_MAX);
		float scale = 300 / (fmax -fmin);
		float log0 = log(geom->m_fLResolution);
		float log1 = log(geom->m_fResolution);
		float log2 = log(geom->m_fHResolution);
		m_pLODDlg->Refresh((log0-fmin) * scale,
			(log1-fmin) * scale,
			(log2-fmin) * scale,
			geom->m_iVertexTarget, geom->m_iVertexCount,
			geom->GetPagingRange());

		m_pLODDlg->DrawTilesetState(geom, vtGetScene()->GetCamera());
	}
	vtDynTerrainGeom *dyn = terr->GetDynTerrain();
	if (dyn)
	{
		SRTerrain *sr = dynamic_cast<SRTerrain*>(dyn);
		if (sr)
		{
			m_pLODDlg->Refresh(log(sr->m_fLResolution)*17,
				log(sr->m_fResolution)*17,
				log(sr->m_fHResolution)*17,
				sr->GetPolygonTarget(), sr->NumDrawnTriangles(), -1);
		}
		SMTerrain *sm = dynamic_cast<SMTerrain*>(dyn);
		if (sm)
		{
			m_pLODDlg->Refresh(-1,
				log((sm->GetQualityConstant()-0.002f)*10000)*40, -1,
				sm->GetPolygonTarget(), sm->NumDrawnTriangles(), -1);
		}
	}
	vtPagedStructureLodGrid *pPSLG = terr->GetStructureLodGrid();
	if (pPSLG)
		m_pLODDlg->DrawStructureState(pPSLG, terr->GetStructurePageOutDistance());
}

//
// Show the feature table dialog for a given feature set.
//
FeatureTableDlg3d *EnviroFrame::ShowTable(vtAbstractLayer *alay)
{
	vtFeatureSet *set = alay->GetFeatureSet();
	FeatureTableDlg3d *table = NULL;
	for (uint i = 0; i < m_FeatureDlgs.size(); i++)
	{
		if (m_FeatureDlgs[i]->GetFeatureSet() == set)
			table = m_FeatureDlgs[i];
	}
	if (!table)
	{
		table = new FeatureTableDlg3d(this, -1, _T(""), wxDefaultPosition,
			wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
		table->SetFeatureSet(set);
		table->SetLayer(alay);
		m_FeatureDlgs.push_back(table);
	}
	table->Show();

	// The dialog might have been already open, but hidden behind other
	//  windows.  We want to help the user see it.
	table->Raise();

	return table;
}

//
// When a feature set is deleted, be sure to also remove the corresponding
//  feature table dialog.
//
void EnviroFrame::OnSetDelete(vtFeatureSet *set)
{
	for (uint i = 0; i < m_FeatureDlgs.size(); i++)
	{
		if (m_FeatureDlgs[i]->GetFeatureSet() == set)
		{
			delete m_FeatureDlgs[i];
			m_FeatureDlgs.erase(m_FeatureDlgs.begin()+i);
			return;
		}
	}
}
void EnviroFrame::DeleteAllSelected()
{
	vtTerrain *pTerr = g_App.GetCurrentTerrain();

	int structs = 0, plants = 0, features = 0;

	vtStructureLayer *st_layer = pTerr->GetStructureLayer();
	if (st_layer)
	{
		structs = pTerr->DeleteSelectedStructures(st_layer);
		if (structs)
			st_layer->SetModified();
	}
	vtVegLayer *v_layer = pTerr->GetVegLayer();
	if (v_layer)
	{
		plants = pTerr->DeleteSelectedPlants(v_layer);
		if (plants)
			v_layer->SetModified();
	}
	vtAbstractLayer *a_layer = pTerr->GetAbstractLayer();
	if (a_layer)
	{
		features = pTerr->DeleteSelectedFeatures(a_layer);
		if (features)
			a_layer->SetModified();
	}

	// layer dialog needs to reflect the change
	if ((plants != 0 || features != 0) && structs == 0)
		m_pLayerDlg->UpdateTreeTerrain();		// we only need to update
	else if (structs != 0)
		m_pLayerDlg->RefreshTreeContents();		// we need full refresh
}


///////////////////////////////////////////////////////////////////

class EnviroProfileCallback : public ProfileCallback
{
public:
	EnviroProfileCallback() {}
	float GetElevation(const DPoint2 &p)
	{
		vtTerrain *terr = g_App.GetCurrentTerrain();
		if (terr)
		{
			FPoint3 w;
			terr->GetHeightField()->m_LocalCS.EarthToLocal(p, w.x, w.z);
			terr->GetHeightField()->FindAltitudeAtPoint(w, w.y, true);
			return w.y;
		}
		return INVALID_ELEVATION;
	}
	float GetCultureHeight(const DPoint2 &p)
	{
		vtTerrain *terr = g_App.GetCurrentTerrain();
		if (terr)
		{
			FPoint3 w;
			terr->GetHeightField()->m_LocalCS.EarthToLocal(p, w.x, w.z);
			bool success = terr->FindAltitudeOnCulture(w, w.y, true, CE_STRUCTURES);
			if (success)
				return w.y;
		}
		return INVALID_ELEVATION;
	}
	virtual bool HasCulture() { return true; }
};

ProfileDlg *EnviroFrame::ShowProfileDlg()
{
	if (!m_pProfileDlg)
	{
		// Create new Feature Info Dialog
		m_pProfileDlg = new ProfileDlg(this, wxID_ANY, _("Elevation Profile"),
				wxPoint(120, 80), wxSize(730, 500), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

		EnviroProfileCallback *callback = new EnviroProfileCallback;
		m_pProfileDlg->SetCallback(callback);

		m_pProfileDlg->SetCRS(g_App.GetCurrentTerrain()->GetCRS());
	}
	m_pProfileDlg->Show(true);
	return m_pProfileDlg;
}

void EnviroFrame::OpenFenceDialog()
{
	// inform the dialog about the materials
	m_pFenceDlg->SetStructureMaterials(GetGlobalMaterials());
	m_pFenceDlg->Show(true);
}

void EnviroFrame::CarveTerrainToFitNode(osg::Node *node)
{
	vtTerrain *terr = g_App.GetCurrentTerrain();
	if (!terr)
		return;
	vtDynTerrainGeom *dyn = terr->GetDynTerrain();
	if (!dyn)
		return;

	FSphere sph;
	GetBoundSphere(node, sph, true);

	int changed = 0;
	int cols, rows;
	dyn->GetDimensions(cols, rows);
	const FPoint3 yvec(0,100,0);

	for (int c = 0; c < cols; c++)
	{
		for (int r = 0; r < rows; r++)
		{
			FPoint3 wpos;
			dyn->GetWorldLocation(c, r, wpos);

			if (wpos.x < (sph.center.x - sph.radius))
				continue;
			if (wpos.x > (sph.center.x + sph.radius))
				continue;
			if (wpos.z < (sph.center.z - sph.radius))
				continue;
			if (wpos.z > (sph.center.z + sph.radius))
				continue;

			// Shoot a ray upwards through the terrain surface point
			vtHitList HitList;
			int iNumHits = vtIntersect(node, wpos - yvec, wpos + yvec, HitList);
			if (iNumHits)
			{
				FPoint3 pos = HitList.front().point;

				dyn->SetElevation(c, r, pos.y);
				changed++;
			}
		}
	}
	if (changed != 0)
	{
		wxString msg;
		msg.Printf(_T("Adjusted %d heixels.  Re-shade the terrain?"), changed);
		int res = wxMessageBox(msg, _T(""), wxYES_NO, this);

		if (res == wxYES)
		{
			// Update the (entire) shading and culture
			EnableContinuousRendering(false);
			OpenProgressDialog(_("Recalculating Shading"), _T(""), false, this);

			terr->ReshadeTexture(vtGetTS()->GetSunLightTransform(), progress_callback);
			DRECT area;
			area.SetToZero();
			terr->RedrapeCulture(area);

			CloseProgressDialog();
			EnableContinuousRendering(true);
		}
	}
}

#if wxUSE_DRAG_AND_DROP
///////////////////////////////////////////////////////////////////////
// Drag-and-drop functionality
//
bool DnDFile::OnDropFiles(wxCoord, wxCoord, const wxArrayString& filenames)
{
	size_t nFiles = filenames.GetCount();
	EnviroFrame *frame = GetFrame();
	for ( size_t n = 0; n < nFiles; n++ )
	{
		wxString str = filenames[n];
		frame->OnDrop(str);
	}
	return TRUE;
}
#endif

//
// Test code
//
void EnviroFrame::DoTestCode()
{
	vtTerrain *pTerr = g_App.GetCurrentTerrain();
	if (!pTerr)
		return;
	pTerr->WriteStructuresToOBJ("c:/temp/structures.obj");
#if 0
	SetMode(MM_SLOPE);
#endif
#if 0
	// Shadow tests
	const int ReceivesShadowTraversalMask = 0x1;
	const int CastsShadowTraversalMask = 0x2;

	osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;

	shadowedScene->setReceivesShadowTraversalMask(ReceivesShadowTraversalMask);
	shadowedScene->setCastsShadowTraversalMask(CastsShadowTraversalMask);

#if 0
	osg::ref_ptr<osgShadow::ShadowMap> sm = new osgShadow::ShadowMap;
	shadowedScene->setShadowTechnique(sm.get());
	int mapres = 1024;
	sm->setTextureSize(osg::Vec2s(mapres,mapres));
#else
	osg::ref_ptr<osgShadow::ShadowTexture> sm = new osgShadow::ShadowTexture;
	shadowedScene->setShadowTechnique(sm.get());
#endif

	osg::Group* cessna1 = (osg::Group*) osgDB::readNodeFile("cessna.osg");
	if (!cessna1)
		return;
	cessna1->setNodeMask(CastsShadowTraversalMask);
	cessna1->getChild(0)->setNodeMask(CastsShadowTraversalMask);

	osg::Group* cessna2 = (osg::Group*) osgDB::readNodeFile("cessna.osg");
	if (!cessna2)
		return;
	int flags_off = ~(CastsShadowTraversalMask | ReceivesShadowTraversalMask);
	cessna2->setNodeMask(flags_off);
	cessna2->getChild(0)->setNodeMask(flags_off);

	osg::MatrixTransform* positioned = new osg::MatrixTransform;
	positioned->setDataVariance(osg::Object::STATIC);
	positioned->setMatrix(osg::Matrix::rotate(osg::inDegrees(-90.0f),0.0f,1.0f,0.0f)
		*osg::Matrix::translate(40,40,0));
	positioned->addChild(cessna1);

//osg::ref_ptr<osg::Group> shadowedScene = new osg::Group;
	shadowedScene->addChild(positioned);
	shadowedScene->addChild(cessna2);

	//	osg::ref_ptr<osg::Group> container = new osg::Group;
	//	container->addChild(positioned);
	//    container->addChild(cessna2);

	vtGroup *vtg = GetCurrentTerrain()->GetTerrainGroup();
	vtg->GetOsgGroup()->addChild(shadowedScene.get());
	//	vtg->GetOsgGroup()->addChild(container.get());

	vtLogGraph(shadowedScene.get());
#endif
#if 0
	if (pTerr && g_App.m_bSelectedStruct)
	{
		vtStructureArray3d *sa = pTerr->GetStructureLayer();
		int i = 0;
		while (!sa->GetAt(i)->IsSelected())
			i++;
		vtBuilding3d *bld = sa->GetBuilding(i);
		// (Do something to the building as a test)
		sa->ConstructStructure(bld);
	}
#endif
#if 0
	{
		// Read points from a text file, create OBJ file with geometry at that locations
		FILE *fp = fopen("test.txt", "r");
		if (!fp) return;

		char buf[80];
		float depth, x, y;

		// Add the geometry and materials to the shape
		vtGeode *pGeode = new vtGeode;
		vtMaterialArray *pMats = new vtMaterialArray;
		pMats->AddRGBMaterial(RGBf(1.0f, 1.0f, 1.0f), false, false, false);
		pGeode->SetMaterials(pMats);

		vtMesh *mesh = new vtMesh(osg::PrimitiveSet::TRIANGLES, VT_Normals | VT_Colors, 4000);

		int line = 0;
		fgets(buf, 80, fp);	// skip first
		while (fgets(buf, 80, fp) != NULL)
		{
			sscanf(buf, "%f\t%f\t%f", &depth, &x, &y);
			int idx = mesh->NumVertices();
			for (int i = 0; i < 20; i++)
			{
				double angle = (double)i / 20.0 * PI2d;
				FPoint3 vec;
				vec.x = x/2 * cos(angle);
				vec.y = 0.0f;
				vec.z = y/2 * sin(angle);

				// normal
				FPoint3 norm = vec;
				norm.Normalize();

				// color
				RGBAf col(1.0f, 1.0f, 1.0f, 1.0f);
				if (x > y)
				{
					float frac = (x-y)/1.5f;	// typical: 0 - 1.2
					col.g -= frac;
					col.b -= frac;
				}
				else if (y > x)
				{
					float frac = (y-x)/1.5f;	// typical: 0 - 1.2
					col.r -= frac;
					col.g -= frac;
				}

				int add = mesh->AddVertexN(vec.x, /*650*/-depth, vec.z,
					norm.x, norm.y, norm.z);
				mesh->SetVtxColor(add, col);
			}
			if (line != 0)
			{
				for (int i = 0; i < 20; i++)
				{
					int next = (i+1)%20;
					mesh->AddTri(idx-20 + i, idx + i,    idx-20 + next);
					mesh->AddTri(idx    + i, idx + next, idx-20 + next);
				}
			}
			line++;
		}
		pGeode->AddMesh(mesh, 0);
		WriteGeomToOBJ(pGeode, "bore.obj");

		vtTransform *model = new vtTransform;
		model->addChild(pGeode);
		DPoint3 pos;
		g_App.m_pTerrainPicker->GetCurrentEarthPos(pos);
		GetCurrentTerrain()->AddNode(model);
		GetCurrentTerrain()->PlantModelAtPoint(model, DPoint2(pos.x, pos.y));
	}
#endif
}

