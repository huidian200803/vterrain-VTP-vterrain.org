//
// The main Frame window of the VTBuilder application
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/stdpaths.h>

#include "vtdata/ElevationGrid.h"
#include "vtdata/FilePath.h"
#include "vtdata/vtDIB.h"
#include "vtdata/vtLog.h"
#include "vtdata/DataPath.h"
#include "xmlhelper/exception.hpp"
#include <fstream>
#include <float.h>	// for FLT_MIN

#include "Frame.h"
#include "TreeView.h"
#include "MenuEnum.h"
#include "App.h"
#include "BuilderView.h"
#include "VegGenOptions.h"
#include "vtImage.h"
#include "Options.h"

#include "vtui/Helper.h"
#include "vtui/ProfileDlg.h"

// Layers
#include "ElevLayer.h"
#include "ImageLayer.h"
#include "RawLayer.h"
#include "RoadLayer.h"
#include "StructLayer.h"
#include "UtilityLayer.h"
#include "VegLayer.h"
// Dialogs
#include "DistanceDlg2d.h"
#include "FeatInfoDlg.h"
#include "OptionsDlg.h"
#include "vtui/InstanceDlg.h"
#include "vtui/LinearStructDlg.h"
#include "vtui/ProjectionDlg.h"

#if defined(__WXGTK__) || defined(__WXMOTIF__) || defined(__WXMAC__)
#  include "bld_add_points.xpm"
#  include "bld_delete_points.xpm"
#  include "bld_edit.xpm"
#  include "bld_corner.xpm"
#  include "distance.xpm"
#  include "edit_crossing.xpm"
#  include "edit_delete.xpm"
#  include "edit_offset.xpm"
#  include "elev_box.xpm"
#  include "elev_resample.xpm"

#  include "image_resample.xpm"
#  include "info.xpm"
#  include "instances.xpm"

#  include "layer_export.xpm"
#  include "layer_import.xpm"
#  include "layer_new.xpm"
#  include "layer_open.xpm"
#  include "layer_save.xpm"
#  include "layer_show.xpm"
#  include "layer_up.xpm"
#  include "layer_down.xpm"

#  include "proj_new.xpm"
#  include "proj_open.xpm"
#  include "proj_save.xpm"

#  include "rd_direction.xpm"
#  include "rd_edit.xpm"
#  include "rd_select_node.xpm"
#  include "rd_select_road.xpm"
#  include "rd_select_whole.xpm"
#  include "rd_shownodes.xpm"

#  include "select.xpm"
#  include "str_add_linear.xpm"
#  include "str_edit_linear.xpm"
#  include "raw_add_point.xpm"
#  include "tin_trim.xpm"

#  include "table.xpm"
#  include "twr_edit.xpm"

#  include "view_hand.xpm"
#  include "view_mag.xpm"
#  include "view_minus.xpm"
#  include "view_plus.xpm"
#  include "view_zoomall.xpm"
#  include "view_zoomexact.xpm"
#  include "view_zoom_layer.xpm"
#  include "view_profile.xpm"
#  include "view_options.xpm"

#	include "VTBuilder.xpm"
#endif

// Singletons
DECLARE_APP(BuilderApp)

//////////////////////////////////////////////////////////////////

/** You can get at the main frame object from anywhere in the application. */
MainFrame *GetMainFrame()
{
	return (MainFrame *) wxGetApp().GetTopWindow();
}


//////////////////////////////////////////////////////////////////
// Frame constructor
//
MainFrame::MainFrame(wxFrame *frame, const wxString& title,
	const wxPoint& pos, const wxSize& size) :
		wxFrame(frame, wxID_ANY, title, pos, size)
{
	VTLOG("  MainFrame constructor: enter\n");

	// tell wxAuiManager to manage this frame
	m_mgr.SetManagedWindow(this);

	// Inform builder object that this is the top window
	m_pParentWindow = this;

	// init app data
	m_pView = NULL;
	m_pActiveLayer = NULL;
	m_SpeciesListDlg = NULL;
	m_BioRegionDlg = NULL;
	m_pFeatInfoDlg = NULL;
	m_pDistanceDlg = NULL;
	m_pProfileDlg = NULL;
	m_pLinearStructureDlg = NULL;
	m_pInstanceDlg = NULL;
	m_bAdoptFirstCRS = true;
	m_pToolbar = NULL;
	m_pMapServerDlg = NULL;
	for (int i = 0; i < LAYER_TYPES; i++)
		m_pLayBar[i] = NULL;

	// frame icon
	SetIcon(wxICON(vtbuilder));
	VTLOG1("  MainFrame constructor: exit\n");
}

MainFrame::~MainFrame()
{
	VTLOG1("Frame destructor\n");
}

void MainFrame::CreateView()
{
	VTLOG1("CreateView\n");
	m_pView = new BuilderView(this, wxID_ANY,
			wxPoint(0, 0), wxSize(200, 400), _T("") );

	m_mgr.AddPane(m_pView, wxAuiPaneInfo().
				  Name(wxT("view")).Caption(wxT("View")).
				  CenterPane().Show(true));
	m_mgr.Update();
	VTLOG1(" refreshing view\n");
	m_pView->Refresh();
}

void MainFrame::ZoomAll()
{
	VTLOG("Zoom All\n");
	m_pView->ZoomToRect(GetExtents(), 0.1f);
}

void MainFrame::SetupUI()
{
	m_statbar = new MyStatusBar(this);
	SetStatusBar(m_statbar);
	m_statbar->Show();
	m_statbar->SetTexts(this);
	PositionStatusBar();

	CreateMenus();
	CreateToolbar();

#if wxUSE_DRAG_AND_DROP
	SetDropTarget(new DnDFile);
#endif

	m_pTree = new MyTreeCtrl(this, LayerTree_Ctrl,
			wxPoint(0, 0), wxSize(200, 400),
			wxTR_HIDE_ROOT | wxNO_BORDER);

	m_mgr.AddPane(m_pTree, wxAuiPaneInfo().
				  Name(_T("layers")).Caption(_("Layers")).
				  Left());
	m_mgr.Update();

	CreateView();
	m_pView->Show(FALSE);

	// Read INI file after creating the view
	if (!ReadXML("VTBuilder.xml"))
	{
		// fall back on older ini file
		if (!ReadINI("VTBuilder.ini"))
		{
			// No options files available. Use defaults.
			SetOptionDefaults();
		}
	}

	// Safety checks
	CheckOptionBounds();

	// Apply all the options, from g_Options the rest of the application
	ApplyOptions();

	RefreshToolbars();

	vtCRS crs;
	crs.SetWellKnownGeogCS("WGS84");
	SetCRS(crs);
	RefreshStatusBar();

	// Load structure defaults
	bool foundmaterials = LoadGlobalMaterials();
	if (!foundmaterials)
		DisplayAndLog("The building materials file (Culture/materials.xml) was not found\n"
			" on your Data Path.  Without this file, materials will not be handled\n"
			" correctly.  Please check your Data Paths to avoid this problem.");

	SetupDefaultStructures(FindFileOnPaths(vtGetDataPath(), "BuildingData/DefaultStructures.vtst"));

	// Load content files, which might be referenced by structure layers
	LookForContentFiles();

#if 0
	long style = 0;//wxCAPTION | wxCLOSE_BOX;
	m_pScaleBar = new vtScaleBar(this, wxID_ANY, wxPoint(0,0), wxSize(400,30), style,
		_T("ScaleBarPanel"));
	m_mgr.AddPane(m_pScaleBar, wxAuiPaneInfo().
				  Name(wxT("scalebar")).Caption(wxT("Scale Bar")).
				  Dockable(false).Float().MinSize(400,30).MaxSize(400,60).Resizable(false));
	m_mgr.Update();
#endif

	// Again, for good measure
	m_mgr.Update();

	SetProgressDialogParent(this);

	SetStatusText(_("Ready"));
}

void MainFrame::OnClose(wxCloseEvent &event)
{
	VTLOG("Frame OnClose\n");
	int num = NumModifiedLayers();
	if (num > 0)
	{
		wxString str;
		str.Printf(_("There are %d layers modified but unsaved.\n Are you sure you want to exit?"), num);
		if (wxMessageBox(str, _("Warning"), wxYES_NO) == wxNO)
		{
			event.Veto();
			return;
		}
	}

	if (m_pFeatInfoDlg != NULL)
	{
		// For some reason, destroying the list control in the feature
		//  dialog is dangerous if allowed to occur naturally, but it is
		//  safe to do it at this point.
		m_pFeatInfoDlg->Clear();
	}

	Destroy();

	WriteXML("VTBuilder.xml");
	m_mgr.UnInit();
}

void MainFrame::ManageToolbar(const wxString &name, wxToolBar *bar, bool show)
{
	wxAuiPaneInfo api;
	api.Name(name);
	api.ToolbarPane();
	api.Top();
	api.LeftDockable(false);
	api.RightDockable(false);
	api.Show(show);
	m_mgr.AddPane(bar, api);
}

wxToolBar *MainFrame::NewToolbar()
{
	int style = (wxTB_FLAT | wxTB_NODIVIDER);
	wxToolBar *bar = new wxToolBar(this, wxID_ANY, wxDefaultPosition,
		wxDefaultSize, style);
	bar->SetMargins(1, 1);
	bar->SetToolBitmapSize(wxSize(20, 20));
	return bar;
}

void MainFrame::CreateToolbar()
{
	// tool bar
	m_pToolbar = NewToolbar();
	m_pLayBar[LT_RAW] = NewToolbar();
	m_pLayBar[LT_ELEVATION] = NewToolbar();
	m_pLayBar[LT_IMAGE] = NewToolbar();
	m_pLayBar[LT_ROAD] = NewToolbar();
	m_pLayBar[LT_STRUCTURE] = NewToolbar();
	m_pLayBar[LT_VEG] = NewToolbar();
	m_pLayBar[LT_UTILITY] = NewToolbar();

	AddMainToolbars();

	ManageToolbar(_T("toolbar"), m_pToolbar, true);
	ManageToolbar(_T("toolbar_raw"), m_pLayBar[LT_RAW], false);
	ManageToolbar(_T("toolbar_elev"), m_pLayBar[LT_ELEVATION], false);
	ManageToolbar(_T("toolbar_image"), m_pLayBar[LT_IMAGE], false);
	ManageToolbar(_T("toolbar_road"), m_pLayBar[LT_ROAD], false);
	ManageToolbar(_T("toolbar_struct"), m_pLayBar[LT_STRUCTURE], false);
	ManageToolbar(_T("toolbar_veg"), m_pLayBar[LT_VEG], false);
	ManageToolbar(_T("toolbar_util"), m_pLayBar[LT_UTILITY], false);
	m_mgr.Update();
}

void MainFrame::RefreshToolbars()
{
	vtLayer *lay = GetActiveLayer();
	LayerType lt = LT_UNKNOWN;
	if (lay)
		lt = lay->GetType();

	m_pMenuBar->EnableTop(m_iLayerMenu[LT_ELEVATION], lt == LT_ELEVATION);
	m_pMenuBar->EnableTop(m_iLayerMenu[LT_IMAGE], lt == LT_IMAGE);
	m_pMenuBar->EnableTop(m_iLayerMenu[LT_ROAD], lt == LT_ROAD);
	m_pMenuBar->EnableTop(m_iLayerMenu[LT_UTILITY], lt == LT_UTILITY);
//	m_pMenuBar->EnableTop(m_iLayerMenu[LT_VEG], lt == LT_VEG);
	m_pMenuBar->EnableTop(m_iLayerMenu[LT_STRUCTURE], lt == LT_STRUCTURE);
	m_pMenuBar->EnableTop(m_iLayerMenu[LT_RAW], lt == LT_RAW);

	for (int i = 0; i < LAYER_TYPES; i++)
	{
		wxToolBar *bar = m_pLayBar[i];
		if (bar)
		{
			wxAuiPaneInfo &info = m_mgr.GetPane(bar);
			info.Show(lt == i);
		}
	}
	m_mgr.Update();
}

void MainFrame::AddMainToolbars()
{
	ADD_TOOL(m_pToolbar, ID_FILE_NEW, wxBITMAP(proj_new), _("New Project"));
	ADD_TOOL(m_pToolbar, ID_FILE_OPEN, wxBITMAP(proj_open), _("Open Project"));
	ADD_TOOL(m_pToolbar, ID_FILE_SAVE, wxBITMAP(proj_save), _("Save Project"));
	ADD_TOOL(m_pToolbar, ID_VIEW_OPTIONS, wxBITMAP(view_options), _("View Options"));
	m_pToolbar->AddSeparator();
	ADD_TOOL(m_pToolbar, ID_LAYER_NEW, wxBITMAP(layer_new), _("New Layer"));
	ADD_TOOL(m_pToolbar, ID_LAYER_OPEN, wxBITMAP(layer_open), _("Open Layer"));
	ADD_TOOL(m_pToolbar, ID_LAYER_SAVE, wxBITMAP(layer_save), _("Save Layer"));
	ADD_TOOL(m_pToolbar, ID_LAYER_IMPORT, wxBITMAP(layer_import), _("Import Data"));
	m_pToolbar->AddSeparator();
	ADD_TOOL(m_pToolbar, ID_EDIT_DELETE, wxBITMAP(edit_delete), _("Delete"));
	ADD_TOOL(m_pToolbar, ID_EDIT_OFFSET, wxBITMAP(edit_offset), _("Offset"));
	ADD_TOOL2(m_pToolbar, ID_VIEW_SHOWLAYER, wxBITMAP(layer_show), _("Layer Visibility"), wxITEM_CHECK);
	ADD_TOOL(m_pToolbar, ID_VIEW_LAYER_UP, wxBITMAP(layer_up), _("Layer Up"));
	ADD_TOOL(m_pToolbar, ID_VIEW_LAYER_DOWN, wxBITMAP(layer_down), _("Layer Down"));
	m_pToolbar->AddSeparator();
	ADD_TOOL(m_pToolbar, ID_VIEW_ZOOMIN, wxBITMAP(view_plus), _("Zoom In"));
	ADD_TOOL(m_pToolbar, ID_VIEW_ZOOMOUT, wxBITMAP(view_minus), _("Zoom Out"));
	ADD_TOOL(m_pToolbar, ID_VIEW_ZOOMALL, wxBITMAP(view_zoomall), _("Zoom All"));
	ADD_TOOL(m_pToolbar, ID_VIEW_ZOOM_LAYER, wxBITMAP(view_zoom_layer), _("Zoom To Layer"));
	m_pToolbar->AddSeparator();
	ADD_TOOL2(m_pToolbar, ID_VIEW_MAGNIFIER, wxBITMAP(view_mag), _("Magnifier"), wxITEM_CHECK);
	ADD_TOOL2(m_pToolbar, ID_VIEW_PAN, wxBITMAP(view_hand), _("Pan"), wxITEM_CHECK);
	ADD_TOOL2(m_pToolbar, ID_VIEW_DISTANCE, wxBITMAP(distance), _("Distance"), wxITEM_CHECK);
	ADD_TOOL2(m_pToolbar, ID_VIEW_SETAREA, wxBITMAP(elev_box), _("Area Tool"), wxITEM_CHECK);
	ADD_TOOL2(m_pToolbar, ID_VIEW_PROFILE, wxBITMAP(view_profile), _("Elevation Profile"), wxITEM_CHECK);
	m_pToolbar->AddSeparator();
	ADD_TOOL(m_pToolbar, ID_AREA_SAMPLE_ELEV, wxBITMAP(elev_resample), _("Sample Elevation"));
	ADD_TOOL(m_pToolbar, ID_AREA_SAMPLE_IMAGE, wxBITMAP(image_resample), _("Sample Imagery"));
	m_pToolbar->Realize();

	// Raw
	ADD_TOOL2(m_pLayBar[LT_RAW], ID_FEATURE_SELECT, wxBITMAP(select), _("Select Features"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_RAW], ID_FEATURE_PICK, wxBITMAP(info), _("Pick Features"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_RAW], ID_FEATURE_TABLE, wxBITMAP(table), _("Table"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_RAW], ID_RAW_ADDPOINTS, wxBITMAP(raw_add_point), _("Add Points with Mouse"), wxITEM_CHECK);
	m_pLayBar[LT_RAW]->Realize();

	// Elevation
	ADD_TOOL2(m_pLayBar[LT_ELEVATION], ID_ELEV_SELECT, wxBITMAP(select), _("Select Elevation"), wxITEM_CHECK);
	ADD_TOOL(m_pLayBar[LT_ELEVATION], ID_VIEW_FULLVIEW, wxBITMAP(view_zoomexact), _("Zoom to Full Detail"));
	ADD_TOOL2(m_pLayBar[LT_ELEVATION], ID_ELEV_TRIMTIN, wxBITMAP(tin_trim), _("Trim TIN triangles by line segment"), wxITEM_CHECK);
	m_pLayBar[LT_ELEVATION]->Realize();

	// Image
	ADD_TOOL(m_pLayBar[LT_IMAGE], ID_VIEW_FULLVIEW, wxBITMAP(view_zoomexact), _("Zoom to Full Detail"));
	m_pLayBar[LT_IMAGE]->Realize();

	// Road
	ADD_TOOL2(m_pLayBar[LT_ROAD], ID_ROAD_SELECTROAD, wxBITMAP(rd_select_road), _("Select Roads"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_ROAD], ID_ROAD_SELECTNODE, wxBITMAP(rd_select_node), _("Select Nodes"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_ROAD], ID_ROAD_SELECTWHOLE, wxBITMAP(rd_select_whole), _("Select Whole Roads"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_ROAD], ID_ROAD_DIRECTION, wxBITMAP(rd_direction), _("Set Road Direction"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_ROAD], ID_ROAD_EDIT, wxBITMAP(rd_edit), _("Edit Road Points"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_ROAD], ID_ROAD_SHOWNODES, wxBITMAP(rd_shownodes), _("Show Nodes"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_ROAD], ID_EDIT_CROSSINGSELECTION, wxBITMAP(edit_crossing), _("Crossing Selection"), wxITEM_CHECK);
	m_pLayBar[LT_ROAD]->Realize();

	// Structure
	ADD_TOOL2(m_pLayBar[LT_STRUCTURE], ID_FEATURE_SELECT, wxBITMAP(select), _("Select Features"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_STRUCTURE], ID_STRUCTURE_EDIT_BLD, wxBITMAP(bld_edit), _("Edit Buildings"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_STRUCTURE], ID_STRUCTURE_ADD_POINTS, wxBITMAP(bld_add_points), _("Add points to building footprints"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_STRUCTURE], ID_STRUCTURE_DELETE_POINTS, wxBITMAP(bld_delete_points), _("Delete points from building footprints"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_STRUCTURE], ID_STRUCTURE_ADD_LINEAR, wxBITMAP(str_add_linear), _("Add Linear Structures"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_STRUCTURE], ID_STRUCTURE_EDIT_LINEAR, wxBITMAP(str_edit_linear), _("Edit Linear Structures"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_STRUCTURE], ID_STRUCTURE_CONSTRAIN, wxBITMAP(bld_corner), _("Constrain Angles"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_STRUCTURE], ID_STRUCTURE_ADD_INST, wxBITMAP(instances), _("Add Instances"), wxITEM_CHECK);
	m_pLayBar[LT_STRUCTURE]->Realize();

	// Veg
	ADD_TOOL2(m_pLayBar[LT_VEG], ID_FEATURE_SELECT, wxBITMAP(select), _("Select Plants"), wxITEM_CHECK);
	m_pLayBar[LT_VEG]->Realize();

	// Utility
	ADD_TOOL2(m_pLayBar[LT_UTILITY], ID_TOWER_ADD,wxBITMAP(rd_select_node), _("Add Tower"), wxITEM_CHECK);
	m_pLayBar[LT_UTILITY]->AddSeparator();
	ADD_TOOL2(m_pLayBar[LT_UTILITY], ID_TOWER_SELECT,wxBITMAP(select),_("Select Towers"), wxITEM_CHECK);
	ADD_TOOL2(m_pLayBar[LT_UTILITY], ID_TOWER_EDIT, wxBITMAP(twr_edit), _("Edit Towers"), wxITEM_CHECK);
	m_pLayBar[LT_UTILITY]->Realize();
}


////////////////////////////////////////////////////////////////
// Application Methods

void MainFrame::SetCRS(const vtCRS &p)
{
	Builder::SetCRS(p);

	// inform the world map view
	if (GetView())
		GetView()->SetWMProj(p);

	// inform the dialogs that care, if they're open
	if (m_pDistanceDlg)
		m_pDistanceDlg->SetCRS(m_crs);
	if (m_pInstanceDlg)
		m_pInstanceDlg->SetCRS(m_crs);
	if (m_pProfileDlg)
		m_pProfileDlg->SetCRS(m_crs);
}

bool MainFrame::AddLayerWithCheck(vtLayer *pLayer, bool bRefresh)
{
	bool result = Builder::AddLayerWithCheck(pLayer, bRefresh);
	if (result && bRefresh)
	{
		// refresh the view
		ZoomAll();
		RefreshToolbars();
		RefreshTreeView();
		RefreshStatusBar();
	}
	return result;
}

void MainFrame::RemoveLayer(vtLayer *lp)
{
	if (!lp)
		return;

	Builder::RemoveLayer(lp);

	// if it was being shown in the feature info dialog, reset that dialog
	if (m_pFeatInfoDlg && m_pFeatInfoDlg->GetLayer() == lp)
	{
		m_pFeatInfoDlg->SetLayer(NULL);
		m_pFeatInfoDlg->SetFeatureSet(NULL);
	}
	m_pView->Refresh();
	RefreshTreeView();
	RefreshToolbars();
}

void MainFrame::SetActiveLayer(vtLayer *lp, bool refresh)
{
	LayerType last = m_pActiveLayer ? m_pActiveLayer->GetType() : LT_UNKNOWN;

	Builder::SetActiveLayer(lp);
	if (refresh)
		m_pTree->RefreshTreeStatus(this);

	// change mouse mode based on layer type
	if (lp == NULL)
		m_pView->SetMode(LB_Mag);

	if (lp != NULL)
	{
		if (lp->GetType() == LT_ELEVATION && last != LT_ELEVATION)
			m_pView->SetMode(LB_TSelect);

		if (lp->GetType() == LT_ROAD && last != LT_ROAD)
			m_pView->SetMode(LB_Link);

		if (lp->GetType() == LT_STRUCTURE && last != LT_STRUCTURE)
			m_pView->SetMode(LB_FSelect);

		if (lp->GetType() == LT_UTILITY && last != LT_UTILITY)
			m_pView->SetMode(LB_FSelect);

		if (lp->GetType() == LT_RAW && last != LT_RAW)
			m_pView->SetMode(LB_FSelect);
	}
}

void MainFrame::RefreshView()
{
	m_pView->Refresh();
}


//
// read / write ini file
//
bool MainFrame::ReadINI(const char *fname)
{
	FILE *fpIni = vtFileOpen(fname, "rb+");

	if (!fpIni)
		return false;

	int ShowMap, ShowElev, ShadeQuick, DoUTM, ShowPaths, DrawWidth,
		CastShadows, ShadeDot=0, Angle=30, Direction=45, dummy;
	float Ambient = 0.1f;
	float Gamma = 0.8f;
	int quiet = fscanf(fpIni, "%d %d %d %d %d %d %d %d %d %d %d %f %f", &ShowMap,
		&ShowElev, &ShadeQuick, &dummy, &DoUTM, &ShowPaths, &DrawWidth,
		&CastShadows, &ShadeDot, &Angle, &Direction, &Ambient, &Gamma);

	vtElevLayer::m_draw.m_bShowElevation = (ShowElev != 0);
	vtElevLayer::m_draw.m_bShadingQuick = (ShadeQuick != 0);
	vtElevLayer::m_draw.m_bShadingDot = (ShadeDot != 0);
	vtElevLayer::m_draw.m_bCastShadows = (CastShadows != 0);
	vtElevLayer::m_draw.m_iCastAngle = Angle;
	vtElevLayer::m_draw.m_iCastDirection = Direction;
	vtElevLayer::m_draw.m_fAmbient = Ambient;
	vtElevLayer::m_draw.m_fGamma = Gamma;
	m_pView->SetShowMap(ShowMap != 0);
	m_pView->m_bShowUTMBounds = (DoUTM != 0);
	m_pTree->SetShowPaths(ShowPaths != 0);
	vtRoadLayer::SetDrawWidth(DrawWidth != 0);

	char buf[4000];
	if (fscanf(fpIni, "\n%s\n", buf) == 1)
	{
		wxString str(buf, wxConvUTF8);
		m_mgr.LoadPerspective(str, false);
	}

	return true;
}

//
// read / write options file
//
bool MainFrame::ReadXML(const char *fname)
{
	bool success = g_Options.LoadFromXML(fname);
	if (success)
	{
		vtExtractArray(g_Options.GetValueString("ProjectMRU"), m_ProjectFiles, '|');
		vtExtractArray(g_Options.GetValueString("LayerMRU"), m_LayerFiles, '|');
		vtExtractArray(g_Options.GetValueString("ImportMRU"), m_ImportFiles, '|');
	}
	return success;
}

void MainFrame::ApplyOptions()
{
	if (!m_pView)
		return;

	// Apply all the options, from g_Options the rest of the application
	m_pView->SetShowMap(g_Options.GetValueBool(TAG_SHOW_MAP));
	m_pView->m_bShowUTMBounds = g_Options.GetValueBool(TAG_SHOW_UTM);
	m_pTree->SetShowPaths(g_Options.GetValueBool(TAG_SHOW_PATHS));
	vtRoadLayer::SetDrawWidth(g_Options.GetValueBool(TAG_ROAD_DRAW_WIDTH));

	m_statbar->m_bShowMinutes = g_Options.GetValueBool(TAG_SHOW_MINUTES);
	m_statbar->m_ShowVertUnits = (LinearUnits) g_Options.GetValueInt(TAG_SHOW_VERT_UNITS);

	vtImage::bTreatBlackAsTransparent = g_Options.GetValueBool(TAG_BLACK_TRANSP);
	vtElevLayer::m_bDefaultGZip = g_Options.GetValueBool(TAG_DEFAULT_GZIP_BT);
	vtElevLayer::m_draw.SetFromTags(g_Options);
	if (g_Options.GetValueBool(TAG_DELAY_LOAD_GRID))
		vtElevLayer::m_iElevMemLimit = g_Options.GetValueInt(TAG_MAX_MEM_GRID);
	else
		vtElevLayer::m_iElevMemLimit = -1;

	vtString str;
	if (g_Options.GetValueString("UI_Layout", str))
	{
		wxString str2(str, wxConvUTF8);
		m_mgr.LoadPerspective(str2, false);
	}
}

bool MainFrame::WriteXML(const char *fname)
{
	if (!m_pView)
		return false;

	// Gather all the options into g_Options
	g_Options.SetValueBool(TAG_SHOW_MAP, m_pView->GetShowMap());
	g_Options.SetValueBool(TAG_SHOW_UTM, m_pView->m_bShowUTMBounds);
	g_Options.SetValueBool(TAG_SHOW_PATHS, m_pTree->GetShowPaths());
	g_Options.SetValueBool(TAG_ROAD_DRAW_WIDTH, vtRoadLayer::GetDrawWidth());

	g_Options.SetValueBool(TAG_SHOW_MINUTES, m_statbar->m_bShowMinutes);
	g_Options.SetValueInt(TAG_SHOW_VERT_UNITS, m_statbar->m_ShowVertUnits);

	vtElevLayer::m_draw.SetToTags(g_Options);

	wxString str = m_mgr.SavePerspective();
	g_Options.SetValueString("UI_Layout", (const char *) str.mb_str(wxConvUTF8));

	// Write it to XML
	return g_Options.WriteToXML(fname, "Options");
}

void MainFrame::RefreshTreeView()
{
	if (m_pTree)
		m_pTree->RefreshTreeItems(this);
}

void MainFrame::RefreshTreeStatus()
{
	if (m_pTree)
		m_pTree->RefreshTreeStatus(this);
}

void MainFrame::RefreshStatusBar()
{
	m_statbar->SetTexts(this);
}

FeatInfoDlg	*MainFrame::ShowFeatInfoDlg()
{
	if (!m_pFeatInfoDlg)
	{
		// Create new Feature Info Dialog
		m_pFeatInfoDlg = new FeatInfoDlg(this, wxID_ANY, _("Feature Info"),
				wxPoint(120, 80), wxSize(600, 200), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
		m_pFeatInfoDlg->SetView(GetView());
	}
	m_pFeatInfoDlg->Show(true);
	return m_pFeatInfoDlg;
}


DistanceDlg2d *MainFrame::ShowDistanceDlg()
{
	if (!m_pDistanceDlg)
	{
		// Create new Distance Dialog
		m_pDistanceDlg = new DistanceDlg2d(this, wxID_ANY, _("Distance Tool"),
				wxPoint(200, 200), wxSize(600, 200), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
		m_pDistanceDlg->SetCRS(m_crs);
		m_pDistanceDlg->SetFrame(this);
	}
	m_pDistanceDlg->Show(true);
	return m_pDistanceDlg;
}

void MainFrame::UpdateDistance(const DPoint2 &p1, const DPoint2 &p2)
{
	DistanceDlg2d *pDlg = ShowDistanceDlg();
	if (pDlg)
	{
		pDlg->SetPoints(p1, p2, true);
		float h1 = GetHeightFromTerrain(p1);
		float h2 = GetHeightFromTerrain(p2);
		float diff = FLT_MIN;
		if (h1 != INVALID_ELEVATION && h2 != INVALID_ELEVATION)
			diff = h2 - h1;
		if (pDlg)
			pDlg->SetGroundAndVertical(FLT_MIN, diff, false);
	}

	ProfileDlg *pDlg2 = m_pProfileDlg;
	if (pDlg2)
		pDlg2->SetPoints(p1, p2);
}

void MainFrame::UpdateDistance(const DLine2 &path)
{
	DistanceDlg2d *pDlg = ShowDistanceDlg();
	if (pDlg)
		pDlg->SetPath(path, true);

	ProfileDlg *pDlg2 = m_pProfileDlg;
	if (pDlg2)
		pDlg2->SetPath(path);
}

void MainFrame::ClearDistance()
{
	UpdateDistance(DPoint2(0,0), DPoint2(0,0));
	UpdateDistance(DLine2());

	// erase previous
	GetView()->ClearDistanceTool();
}


class LinearStructureDlg2d: public LinearStructureDlg
{
public:
	LinearStructureDlg2d(wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos, const wxSize& size, long style) :
	LinearStructureDlg(parent, id, title, pos, size, style) {}
	void OnSetOptions(const vtLinearParams &opt)
	{
		g_bld->m_LSOptions = opt;
	}
	MainFrame *m_pFrame;
};

LinearStructureDlg *MainFrame::ShowLinearStructureDlg(bool bShow)
{
	if (bShow && !m_pLinearStructureDlg)
	{
		// Create new Distance Dialog
		m_pLinearStructureDlg = new LinearStructureDlg2d(this, -1,
			_("Linear Structures"), wxPoint(120, 80), wxSize(600, 200),
			wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
		m_pLinearStructureDlg->m_pFrame = this;
		m_pLinearStructureDlg->SetStructureMaterials(GetGlobalMaterials());
	}
	if (m_pLinearStructureDlg)
		m_pLinearStructureDlg->Show(bShow);
	return m_pLinearStructureDlg;
}

class InstanceDlg2d: public InstanceDlg
{
public:
	InstanceDlg2d( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos, const wxSize& size, long style) :
		InstanceDlg( parent, id, title, pos, size, style ) {}
	virtual void OnCreate()
	{
		if (!m_pFrame)
			return;
		m_pFrame->CreateInstance(m_pos, GetTagArray());
	}
	MainFrame *m_pFrame;
};

InstanceDlg *MainFrame::ShowInstanceDlg(bool bShow)
{
	if (bShow && !m_pInstanceDlg)
	{
		// Create new Instance Dialog
		InstanceDlg2d *dlg = new InstanceDlg2d(this, -1, _("Structure Instances"),
			wxPoint(120, 80), wxSize(600, 200), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
		dlg->m_pFrame = this;
		m_pInstanceDlg = dlg;

		for (uint i = 0; i < m_contents.size(); i++)
			m_pInstanceDlg->AddContent(m_contents[i]);
		m_pInstanceDlg->SetCRS(m_crs);
	}
	if (m_pInstanceDlg)
		m_pInstanceDlg->Show(bShow);
	return m_pInstanceDlg;
}

void MainFrame::CreateInstance(const DPoint2 &pos, vtTagArray *tags)
{
	vtStructureLayer *slayer = g_bld->GetActiveStructureLayer();
	if (!slayer)
		return;

	vtStructInstance *inst = slayer->AddNewInstance();
	inst->SetPoint(pos);
	inst->CopyTagsFrom(*tags);

	g_bld->ResolveInstanceItem(inst);

	slayer->SetModified(true);
	GetView()->Refresh();
}

class BuildingProfileCallback : public ProfileCallback
{
public:
	void Begin()
	{
		m_elevs.clear();
		m_frame->ElevLayerArray(m_elevs);
	}
	float GetElevation(const DPoint2 &p)
	{
		return ElevLayerArrayValue(m_elevs, p);
	}
	float GetCultureHeight(const DPoint2 &p)
	{
		return INVALID_ELEVATION;
	}
	MainFrame *m_frame;
	std::vector<vtElevLayer*> m_elevs;
};

ProfileDlg *MainFrame::ShowProfileDlg()
{
	if (!m_pProfileDlg)
	{
		// Create new Feature Info Dialog
		m_pProfileDlg = new ProfileDlg(this, wxID_ANY, _("Elevation Profile"),
				wxPoint(120, 80), wxSize(730, 500), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
		BuildingProfileCallback *callback = new BuildingProfileCallback;
		callback->m_frame = this;
		m_pProfileDlg->SetCallback(callback);
		m_pProfileDlg->SetCRS(m_crs);
	}
	m_pProfileDlg->Show(true);

	// This might be the first time it's displayed, so we need to get
	//  the point values from the distance tool
	GetView()->UpdateDistance();

	return m_pProfileDlg;
}

void MainFrame::OnSelectionChanged()
{
	if (m_pFeatInfoDlg && m_pFeatInfoDlg->IsShown())
	{
		vtRawLayer *pRL = GetActiveRawLayer();
		m_pFeatInfoDlg->SetFeatureSet(pRL->GetFeatureSet());
		m_pFeatInfoDlg->ShowSelected();
	}
}

void MainFrame::UpdateFeatureDialog(vtRawLayer *raw, vtFeatureSet *pSet, int iEntity)
{
	FeatInfoDlg	*fdlg = ShowFeatInfoDlg();
	fdlg->SetLayer(raw);
	fdlg->SetFeatureSet(pSet);
	pSet->DePickAll();

	// Handle point features specially: there might be more than one feature
	// under the cursor at that point.
	vtFeatureSetPoint2D *pSetP2 = dynamic_cast<vtFeatureSetPoint2D *>(pSet);
	if (pSetP2)
	{
		DPoint2 &p2 = pSetP2->GetPoint(iEntity);
		std::vector<int> found;
		pSetP2->FindAllPointsAtLocation(p2, found);

		for (uint i = 0; i < found.size(); i++)
			pSetP2->Pick(found[i]);
	}
	else
	{
		// Otherwise just pick the single feature.
		pSet->Pick(iEntity);
	}

	fdlg->ShowPicked();
}

void MainFrame::OnSetMode(LBMode m)
{
	// Show this dialog only in AddLinear mode
	ShowLinearStructureDlg(m == LB_AddLinear);

	// Show this dialog only in AddInstance mode
	ShowInstanceDlg(m == LB_AddInstance);
}


////////////////////////////////////////////////////////////////
// Project operations

bool MainFrame::LoadProject(const wxString &strPathName)
{
	vtString fname = (const char *) strPathName.mb_str(wxConvUTF8);
	VTLOG("Loading project: '%s'\n", (const char *) fname);

	// avoid trying to draw while we're loading the project
	m_bDrawDisabled = true;

	bool success = Builder::LoadProject(fname, m_pView);

	// refresh the view
	m_bDrawDisabled = false;

	RefreshTreeView();
	RefreshToolbars();

	return success;
}

bool MainFrame::SaveProject(const wxString &strPathName) const
{
	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	// Write project file.
	vtString fname = (const char *) strPathName.mb_str(wxConvUTF8);
	FILE *fp = vtFileOpen(fname, "wb");
	if (!fp)
		return false;

	// Write CRS info.
	char *wkt;
	m_crs.exportToWkt(&wkt);
	fprintf(fp, "Projection %s\n", wkt);
	OGRFree(wkt);

	if (m_strSpeciesFilename != "")
	{
		fprintf(fp, "PlantList %s\n", (const char *) m_strSpeciesFilename);
	}

	if (m_strBiotypesFilename != "")
	{
		fprintf(fp, "BioTypes %s\n", (const char *) m_strBiotypesFilename);
	}

	// Write list of layers.
	int iLayers = m_Layers.size();
	fprintf(fp, "layers: %d\n", iLayers);

	vtLayer *lp;
	for (int i = 0; i < iLayers; i++)
	{
		lp = m_Layers[i];

		bool bNative = lp->IsNative();

		fprintf(fp, "type %d, %s", lp->GetType(), bNative ? "native" : "import");
		if (!lp->GetVisible())
			fprintf(fp, " hidden");
		fprintf(fp, "\n");

		wxString fname = lp->GetLayerFilename();
		if (!bNative)
		{
			if (lp->GetImportedFrom() != _T(""))
				fname = lp->GetImportedFrom();
		}
		fprintf(fp, "%s\n", (const char *) fname.mb_str(wxConvUTF8));
	}

	// Write area tool.
	fprintf(fp, "area %lf %lf %lf %lf\n", m_area.left, m_area.top,
		m_area.right, m_area.bottom);

	// Write view location.
	double	scale;
	DPoint2 offset;
	m_pView->GetViewParams(scale, offset);
	fprintf(fp, "view_params %lf %lf %lf\n", scale, offset.x, offset.y);

	// Done.
	fclose(fp);

	return true;
}

void MainFrame::ShowOptionsDialog()
{
	OptionsDlg dlg(this, -1, _("View Options"));

	dlg.m_bShowMinutes = m_statbar->m_bShowMinutes;
	dlg.m_iElevUnits = (int)(m_statbar->m_ShowVertUnits) - 1;

	dlg.SetElevDrawOptions(vtElevLayer::m_draw);

	bool bDrawTinSimple = g_Options.GetValueBool(TAG_DRAW_TIN_SIMPLE);
	bool bDrawRawSimple = g_Options.GetValueBool(TAG_DRAW_RAW_SIMPLE);

	dlg.m_bShowRoadWidth = vtRoadLayer::GetDrawWidth();
	dlg.m_bDrawTinSimple = bDrawTinSimple;
	dlg.m_bDrawRawSimple = bDrawRawSimple;
	dlg.m_bShowPath = m_pTree->GetShowPaths();

	if (dlg.ShowModal() != wxID_OK)
		return;

	bool bNeedRefresh = false;

	m_statbar->m_bShowMinutes = dlg.m_bShowMinutes;
	m_statbar->m_ShowVertUnits = (LinearUnits) (dlg.m_iElevUnits + 1);

	ElevDrawOptions opt;
	dlg.GetElevDrawOptions(opt);

	if (vtElevLayer::m_draw != opt)
	{
		vtElevLayer::m_draw = opt;

		// tell them to redraw themselves
		for (uint i = 0; i < m_Layers.size(); i++)
		{
			vtLayer *lp = m_Layers[i];
			if (lp->GetType() == LT_ELEVATION)
			{
				vtElevLayer *elp = (vtElevLayer *) lp;
				elp->ReRender();
				bNeedRefresh = true;
			}
		}
	}

	bool bWidth = dlg.m_bShowRoadWidth;
	if (vtRoadLayer::GetDrawWidth() != bWidth && LayersOfType(LT_ROAD) > 0)
		bNeedRefresh = true;
	vtRoadLayer::SetDrawWidth(bWidth);

	g_Options.SetValueBool(TAG_DRAW_TIN_SIMPLE, dlg.m_bDrawTinSimple);
	g_Options.SetValueBool(TAG_DRAW_RAW_SIMPLE, dlg.m_bDrawRawSimple);

	// If settings changed, redraw the view
	if (dlg.m_bDrawTinSimple != bDrawTinSimple && LayersOfType(LT_ELEVATION) > 0)
		bNeedRefresh = true;

	if (dlg.m_bDrawRawSimple != bDrawRawSimple && LayersOfType(LT_RAW) > 0)
		bNeedRefresh = true;

	if (dlg.m_bShowPath != m_pTree->GetShowPaths())
	{
		m_pTree->SetShowPaths(dlg.m_bShowPath);
		RefreshTreeView();
	}

	if (bNeedRefresh)
		m_pView->Refresh();
}

void MainFrame::OnDrop(const wxString &str)
{
	if (!str.Right(3).CmpNoCase(_T("vtb")))
		LoadProject(str);
	else
	{
		Builder::LoadResult result = LoadLayer(str);
		if (result == Builder::NOT_NATIVE)
		{
			// try importing
			if (ImportDataFromArchive(LT_UNKNOWN, str, true))
			{
				// succeeded, bring to the top of the MRU
				AddToMRU(m_ImportFiles, (const char *) str.mb_str(wxConvUTF8));
			}
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
	MainFrame *frame = GetMainFrame();
	for ( size_t n = 0; n < nFiles; n++ )
	{
		wxString str = filenames[n];
		frame->OnDrop(str);
	}
	return TRUE;
}
#endif

//////////////////
// Keyboard shortcuts

void MainFrame::OnChar(wxKeyEvent& event)
{
	m_pView->OnChar(event);
}

void MainFrame::OnKeyDown(wxKeyEvent& event)
{
	m_pView->OnChar(event);
}

void MainFrame::OnMouseWheel(wxMouseEvent& event)
{
	m_pView->OnMouseWheel(event);
}

