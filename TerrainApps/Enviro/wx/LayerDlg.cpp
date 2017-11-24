//
// Name: LayerDlg.cpp
//
// Copyright (c) 2003-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/vtLog.h"
#include "vtdata/FileFilters.h"
#include "vtlib/vtlib.h"
#include "vtlib/core/Building3d.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/Globe.h"
#include "vtui/Helper.h"	// for progress dialog

#include "EnviroGUI.h"		// for GetCurrentTerrain
#include "EnviroFrame.h"
#include "EnviroCanvas.h"	// for EnableContinuousRendering
#include "StyleDlg.h"
#include "LayerDlg.h"

#include "menu_id.h"

#if defined(__WXGTK__) || defined(__WXMOTIF__) || defined(__WXMAC__)
#  include "building.xpm"
#  include "road.xpm"
#  include "veg1.xpm"
#  include "fence.xpm"
#  include "instance.xpm"
#  include "image.xpm"

#  include "layer_create.xpm"
#  include "layer_open.xpm"
#  include "save.xpm"
#  include "save_as.xpm"
#  include "zoom_to.xpm"
#  include "delete1.xpm"
#  include "eye.xpm"
#  include "shadow.xpm"
#  include "treeview.xpm"
#  include "grid.xpm"
#  include "refresh.xpm"
#endif

#include "wxosg/icons/top.xpm"
#include "icons/raw.xpm"
#include "icons/elevation.xpm"
#include "icons/raw_yellow.xpm"

#define LocalIcon(X) wxIcon( (const char**) X##_xpm )

#define ICON_BUILDING	0
#define ICON_ROAD		1
#define ICON_VEG1		2
#define ICON_RAW		3
#define ICON_FENCE		4
#define ICON_INSTANCE	5
#define ICON_TOP		6
#define ICON_IMAGE		7
#define ICON_ELEV		8
#define ICON_RAW_YELLOW	9

/////////////////////////////

#define ID_LAYER_TREE 10500

// WDR: class implementations

//----------------------------------------------------------------------------
// LayerDlg
//----------------------------------------------------------------------------

// WDR: event table for LayerDlg

BEGIN_EVENT_TABLE(LayerDlg,wxPanel)
	EVT_INIT_DIALOG (LayerDlg::OnInitDialog)
	EVT_TREE_SEL_CHANGED( ID_LAYER_TREE, LayerDlg::OnSelChanged )
	EVT_TREE_ITEM_ACTIVATED( ID_LAYER_TREE, LayerDlg::OnItemActived )
	EVT_CHECKBOX( ID_SHOW_ALL, LayerDlg::OnShowAll )
	EVT_CHECKBOX( ID_LAYER_VISIBLE, LayerDlg::OnVisible )
	EVT_CHECKBOX( ID_LAYER_SHADOW, LayerDlg::OnShadowVisible )

	EVT_MENU( ID_LAYER_CREATE, LayerDlg::OnLayerCreate )
	EVT_MENU( ID_LAYER_LOAD, LayerDlg::OnLayerLoad )
	EVT_MENU( ID_LAYER_SAVE, LayerDlg::OnLayerSave )
	EVT_MENU( ID_LAYER_SAVE_AS, LayerDlg::OnLayerSaveAs )
	EVT_MENU( ID_LAYER_DELETE, LayerDlg::OnLayerRemove )
	EVT_MENU( ID_LAYER_ZOOM_TO, LayerDlg::OnZoomTo )

	EVT_MENU( ID_LAYER_VISIBLE, LayerDlg::OnVisible )
	EVT_MENU( ID_LAYER_TABLE, LayerDlg::OnTable )
	EVT_MENU( ID_LAYER_SHADOW, LayerDlg::OnShadowVisible )
	EVT_MENU( ID_LAYER_REFRESH, LayerDlg::OnRefresh )
	EVT_MENU( ID_SHOW_ALL, LayerDlg::OnShowAll )

	EVT_UPDATE_UI(ID_LAYER_CREATE,	LayerDlg::OnUpdateCreate)
	EVT_UPDATE_UI(ID_LAYER_VISIBLE,	LayerDlg::OnUpdateVisible)
	EVT_UPDATE_UI(ID_LAYER_REFRESH,	LayerDlg::OnUpdateRefresh)
	EVT_UPDATE_UI(ID_LAYER_SHADOW, LayerDlg::OnUpdateShadow)
	EVT_UPDATE_UI(ID_SHOW_ALL,	LayerDlg::OnUpdateShowAll)

END_EVENT_TABLE()

void LayerToolBarFunc(wxToolBar *bar)
{
    bar->SetMargins( 1, 1 );

    bar->AddTool( ID_LAYER_CREATE, _("Create"),	  wxBITMAP(layer_create), wxNullBitmap, wxITEM_NORMAL, _("Create New Layer") );
    bar->AddTool( ID_LAYER_LOAD, _("Load"),		  wxBITMAP(layer_open),	  wxNullBitmap, wxITEM_NORMAL, _("Load Layer") );
    bar->AddTool( ID_LAYER_SAVE, _("Save"),		  wxBITMAP(save),		  wxNullBitmap, wxITEM_NORMAL, _("Save Layer") );
    bar->AddTool( ID_LAYER_SAVE_AS, _("Save As"), wxBITMAP(save_as),	  wxNullBitmap, wxITEM_NORMAL, _("Save Layer As...") );
    bar->AddTool( ID_LAYER_DELETE, _("Delete"),	  wxBITMAP(delete1),	  wxNullBitmap, wxITEM_NORMAL, _("Delete Layer") );
    bar->AddSeparator();
    bar->AddTool( ID_LAYER_ZOOM_TO, _("Zoom"),	  wxBITMAP(zoom_to),	  wxNullBitmap, wxITEM_NORMAL, _("Zoom To") );
    bar->AddTool( ID_LAYER_VISIBLE, _("Visible"), wxBITMAP(eye),		  wxNullBitmap, wxITEM_CHECK, _("Toggle Layer Visibility") );
    bar->AddTool( ID_LAYER_TABLE, wxT(""),		  wxBITMAP(grid),		  wxNullBitmap, wxITEM_NORMAL, _("Table of Features") );
    bar->AddTool( ID_LAYER_SHADOW, _("Shadow"),	  wxBITMAP(shadow),		  wxNullBitmap, wxITEM_CHECK, _("Toggle Structure Shadow") );
    bar->AddTool( ID_LAYER_REFRESH, _("Refresh"), wxBITMAP(refresh),	  wxNullBitmap, wxITEM_NORMAL, _("Refresh") );
    bar->AddSeparator();
    bar->AddTool( ID_SHOW_ALL, _("All"),		  wxBITMAP(treeview),	  wxNullBitmap, wxITEM_CHECK, _("Show all elements of every layer") );

    bar->Realize();
}


LayerDlg::LayerDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	wxPanel( parent, id, position, size, style )
{
	m_pTerrain = NULL;
	m_bShowAll = false;
	m_imageListNormal = NULL;

	const long tbstyle = wxTB_FLAT | wxTB_NODIVIDER;
	//tbstyle |= wxTB_HORZ_TEXT;
	m_pToolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, tbstyle);
	m_pToolbar->SetToolBitmapSize(wxSize(20, 20));
	LayerToolBarFunc(m_pToolbar);

	// tell wxAuiManager to manage this frame
	m_mgr.SetManagedWindow(this);

	wxAuiPaneInfo api;
	api.Name(_T("toolbar"));
	api.ToolbarPane();
	api.Top();
	api.LeftDockable(false);
	api.RightDockable(false);
	wxSize best = m_pToolbar->GetBestSize();
	api.MinSize(best);
	api.Floatable(false);
	m_mgr.AddPane(m_pToolbar, api);

	m_main = new wxPanel( this, -1, wxDefaultPosition, wxDefaultSize, 0 );
	m_mgr.AddPane(m_main, wxAuiPaneInfo().
				  Name(wxT("pane1")).Caption(wxT("pane1")).
				  CenterPane());
	m_mgr.Update();

	wxBoxSizer *item0 = new wxBoxSizer( wxHORIZONTAL );
	m_pTree = new wxTreeCtrl( m_main, ID_LAYER_TREE, wxDefaultPosition,
		wxDefaultSize, wxTR_HAS_BUTTONS|wxTR_LINES_AT_ROOT|wxSUNKEN_BORDER );
	item0->Add( m_pTree, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	wxSizer *top = item0;

	m_main->SetSizer( item0 );

	CreateImageList(16);

	// Note that the sizer for the _main_ pain informs the window of its
	//  size hints.  The top level window (the dialog) has no sizer at all.
	top->SetSizeHints(this);
}

LayerDlg::~LayerDlg()
{
	m_mgr.UnInit();

	delete m_imageListNormal;
}

void LayerDlg::CreateImageList(int size)
{
	delete m_imageListNormal;

	if ( size == -1 )
	{
		m_imageListNormal = NULL;
		return;
	}
	// Make an image list containing small icons
	m_imageListNormal = new wxImageList(size, size, TRUE);

	wxIcon icons[10];
	icons[0] = wxICON(building);		// ICON_BUILDING
	icons[1] = wxICON(road);			// ICON_ROAD
	icons[2] = wxICON(veg1);			// ICON_VEG1
	icons[3] = LocalIcon(raw);			// ICON_RAW
	icons[4] = wxICON(fence);			// ICON_FENCE
	icons[5] = wxICON(instance);		// ICON_INSTANCE
	icons[6] = LocalIcon(top);			// ICON_TOP
	icons[7] = wxICON(image);			// ICON_IMAGE
	icons[8] = LocalIcon(elevation);	// ICON_ELEV
	icons[9] = LocalIcon(raw_yellow);	// ICON_RAW_YELLOW

	int sizeOrig = icons[0].GetWidth();
	for ( size_t i = 0; i < WXSIZEOF(icons); i++ )
	{
		if ( size == sizeOrig )
			m_imageListNormal->Add(icons[i]);
		else
			m_imageListNormal->Add(wxBitmap(wxBitmap(icons[i]).ConvertToImage().Rescale(size, size)));
	}
	m_pTree->SetImageList(m_imageListNormal);
}


void LayerDlg::SetShowAll(bool bTrue)
{
	m_bShowAll = bTrue;
}

//
// For an item in the tree which corresponds to an actual structure,
//  return the node associated with that structure.
//
osg::Node *LayerDlg::GetNodeFromItem(wxTreeItemId item, bool bContainer)
{
	if (!item.IsOk())
		return NULL;

	LayerItemData *data = (LayerItemData *)m_pTree->GetItemData(item);
	if (!data)
		return NULL;
	if (data->m_alay)
		return data->m_alay->GetContainer();
	if (data->m_item == -1)
		return NULL;

	vtStructure3d *str3d = data->m_slay->GetStructure3d(data->m_item);
	vtStructure *str = data->m_slay->at(data->m_item);
	vtStructureType typ = str->GetType();

	if (bContainer && typ != ST_LINEAR)
		return str3d->GetContainer();
	else
		// always get contained geometry for linears; they have no container
		return str3d->GetContained();
}

vtLayer *LayerDlg::GetLayerFromItem(wxTreeItemId item)
{
	LayerItemData *data = GetLayerDataFromItem(item);
	if (!data)
		return NULL;
	return data->m_layer;
}

vtStructureLayer *LayerDlg::GetStructureLayerFromItem(wxTreeItemId item)
{
	LayerItemData *data = GetLayerDataFromItem(item);
	if (!data)
		return NULL;
	if (data->m_item == -1)
		return data->m_slay;
	return NULL;
}

LayerItemData *LayerDlg::GetLayerDataFromItem(wxTreeItemId item)
{
	if (!item.IsOk())
		return NULL;
	LayerItemData *data = (LayerItemData *)m_pTree->GetItemData(item);
	return data;
}

void LayerDlg::RefreshTreeContents()
{
	if (!m_pTree)
		return;

	// start with a blank slate
	m_pTree->DeleteAllItems();

	switch (g_App.m_state)
	{
	case AS_Terrain:
		RefreshTreeTerrain();
		break;
	case AS_Orbit:
		RefreshTreeSpace();
		break;
	default:
		break;
	}
	m_item = m_pTree->GetSelection();
	UpdateEnabling();
}

// Helper
wxString MakeVegLayerString(vtPlantInstanceArray3d &pia)
{
	wxString str(pia.GetFilename(), wxConvUTF8);
	wxString str2;
	str2.Printf(_(" (Plants: %d)"), pia.NumEntities());
	str += str2;
	return str;
}

// Helper
void MakeAbsLayerString(vtAbstractLayer *alay, wxString &str,
						uint &selected)
{
	vtFeatureSet *fset = alay->GetFeatureSet();

	// Safety check
	if (!fset)
	{
		str += _(" (no featureset)");
		return;
	}

	vtString vs = fset->GetFilename();
	str = wxString(vs, wxConvUTF8);

	str += _T(" (");
	str += wxString(OGRGeometryTypeToName(fset->GetGeomType()), wxConvLibc);

	str += _(", Features: ");
	vs.Format("%d", fset->NumEntities());
	str += wxString(vs, wxConvLibc);

	selected = fset->NumSelected();

	if (selected != 0)
	{
		str += _(", Selected: ");
		vs.Format("%d", selected);
		str += wxString(vs, wxConvLibc);
	}
	str += _T(")");
}

void LayerDlg::RefreshTreeTerrain()
{
	if (!m_pTerrain)
		return;

	m_root = m_pTree->AddRoot(_("Layers"), ICON_TOP, ICON_TOP);

	wxString str;
	vtString vs;
	LayerSet &layers = m_pTerrain->GetLayers();
	for (uint i = 0; i < layers.size(); i++)
	{
		vtLayer *layer = layers[i].get();
		LayerItemData *lid;
		int icon;

		// Structure layers
		vtStructureLayer *slay = dynamic_cast<vtStructureLayer*>(layer);
		if (slay)
		{
			str = wxString(slay->GetFilename(), wxConvUTF8);
			icon = ICON_BUILDING;
			wxTreeItemId hLayer = m_pTree->AppendItem(m_root, str, icon, icon);
			if (slay == m_pTerrain->GetStructureLayer())
				m_pTree->SetItemBold(hLayer, true);
			lid = new LayerItemData(slay, i, -1);
			lid->m_text = str;
			lid->m_icon = icon;
			m_pTree->SetItemData(hLayer, lid);

			wxTreeItemId hItem;
			if (m_bShowAll)
			{
				for (uint j = 0; j < slay->size(); j++)
				{
					vtBuilding *bld = slay->GetBuilding(j);
					if (bld)
					{
						vtString value;
						wxString label;
						if (bld->GetValueString("id", value))
						{
							label += _("id: ");
							label += wxString::FromUTF8(value);
						}
						if (bld->GetValueString("name", value))
						{
							if (label != _T(""))
								label += _T(", ");
							label += _("name: ");
							label += _T("'");
							label += wxString::FromUTF8(value);
							label += _T("'");
						}
						if (label == _T(""))
							label = _("Building");

						icon = ICON_BUILDING;
						hItem = m_pTree->AppendItem(hLayer, label, icon, icon);
					}
					if (slay->GetFence(j))
					{
						icon = ICON_FENCE;
						hItem = m_pTree->AppendItem(hLayer, _("Fence"), icon, icon);
					}
					if (vtStructInstance *inst = slay->GetInstance(j))
					{
						vs = inst->GetValueString("filename", true);
						if (vs != "")
						{
							str = _("File ");
							str += wxString((const char *)vs, wxConvUTF8);
						}
						else
						{
							vs = inst->GetValueString("itemname", true);
							str = _("Item ");
							str += wxString((const char *)vs, wxConvUTF8);
						}
						icon = ICON_INSTANCE;
						hItem = m_pTree->AppendItem(hLayer, str, icon, icon);
					}
					lid = new LayerItemData(slay, i, j);
					lid->m_text = str;
					lid->m_icon = icon;
					m_pTree->SetItemData(hItem, lid);
				}
			}
			else
			{
				int bld = 0, fen = 0, inst = 0;
				for (uint j = 0; j < slay->size(); j++)
				{
					if (slay->GetBuilding(j)) bld++;
					if (slay->GetFence(j)) fen++;
					if (slay->GetInstance(j)) inst++;
				}
				if (bld)
				{
					str.Printf(_("Buildings: %d"), bld);
					icon = ICON_BUILDING;
					hItem = m_pTree->AppendItem(hLayer, str, icon, icon);
					lid = new LayerItemData(slay, i, -1);
					lid->m_text = str;
					lid->m_icon = icon;
					m_pTree->SetItemData(hItem, lid);
				}
				if (fen)
				{
					str.Printf(_("Fences: %d"), fen);
					icon = ICON_FENCE;
					hItem = m_pTree->AppendItem(hLayer, str, icon, icon);
					lid = new LayerItemData(slay, i, -1);
					lid->m_text = str;
					lid->m_icon = icon;
					m_pTree->SetItemData(hItem, lid);
				}
				if (inst)
				{
					str.Printf(_("Instances: %d"), inst);
					icon = ICON_INSTANCE;
					hItem = m_pTree->AppendItem(hLayer, str, icon, icon);
					lid = new LayerItemData(slay, i, -1);
					lid->m_text = str;
					lid->m_icon = icon;
					m_pTree->SetItemData(hItem, lid);
				}
			}
			m_pTree->Expand(hLayer);
		}

		// Now, abstract layers
		vtAbstractLayer *alay = dynamic_cast<vtAbstractLayer*>(layer);
		if (alay)
		{
			wxString str;
			uint selected;
			MakeAbsLayerString(alay, str, selected);

			icon = (selected != 0 ? ICON_RAW_YELLOW : ICON_RAW);
			wxTreeItemId hLayer = m_pTree->AppendItem(m_root, str, icon, icon);
			vtFeatureSet *fset = alay->GetFeatureSet();
			lid = new LayerItemData(alay, fset);
			lid->m_text = str;
			lid->m_icon = icon;
			m_pTree->SetItemData(hLayer, lid);
		}
		// Image layers
		vtImageLayer *ilay = dynamic_cast<vtImageLayer*>(layer);
		if (ilay)
		{
			vs = ilay->GetLayerName();
			str = wxString(vs, wxConvUTF8);

			icon = ICON_IMAGE;
			wxTreeItemId hLayer = m_pTree->AppendItem(m_root, str, icon, icon);
			lid = new LayerItemData(ilay);
			lid->m_text = str;
			lid->m_icon = icon;
			m_pTree->SetItemData(hLayer, lid);
		}
		// Vegetation
		vtVegLayer *vlay = dynamic_cast<vtVegLayer*>(layer);
		if (vlay)
		{
			str = MakeVegLayerString(*vlay);

			icon = ICON_VEG1;
			wxTreeItemId hLayer = m_pTree->AppendItem(m_root, str, icon, icon);
			lid = new LayerItemData(vlay);
			lid->m_text = str;
			lid->m_icon = icon;
			m_pTree->SetItemData(hLayer, lid);
		}
		// Elevation
		vtElevLayer *elay = dynamic_cast<vtElevLayer*>(layer);
		if (elay)
		{
			vs = elay->GetLayerName();
			str = wxString(vs, wxConvUTF8);

			icon = ICON_ELEV;
			wxTreeItemId hLayer = m_pTree->AppendItem(m_root, str, icon, icon);
			lid = new LayerItemData(elay);
			lid->m_text = str;
			lid->m_icon = icon;
			m_pTree->SetItemData(hLayer, lid);
		}
	}
	m_pTree->Expand(m_root);
}

//
// Refresh only the state (text) of the items: don't destroy or create any.
//
void LayerDlg::UpdateTreeTerrain()
{
	if (!m_pTerrain)
		return;

	if (!m_root.IsOk())
		return;

	wxTreeItemIdValue cookie;
	wxTreeItemId id;
	int count = 0;
	for (id = m_pTree->GetFirstChild(m_root, cookie); id.IsOk();
		id = m_pTree->GetNextChild(m_root, cookie))
	{
		LayerItemData *data = GetLayerDataFromItem(id);

		if (data)
		{
			wxString str;

			// Hightlight the active layer in Bold
			if (data->m_layer && data->m_layer == m_pTerrain->GetActiveLayer())
			{
				if (!m_pTree->IsBold(id))
					m_pTree->SetItemBold(id, true);
			}
			else
			{
				if (m_pTree->IsBold(id))
					m_pTree->SetItemBold(id, false);
			}

			// Refresh the vegetation count
			if (data->m_vlay)
			{
				str = MakeVegLayerString(*(data->m_vlay));
				if (str != data->m_text)
				{
					data->m_text = str;
					m_pTree->SetItemText(id, str);
				}
				m_pTree->SetItemImage(id, ICON_VEG1);
			}

			// Update text and icon for abstract layers
			if (data->m_alay)
			{
				uint selected;
				MakeAbsLayerString(data->m_alay, str, selected);
				int icon = (selected != 0 ? ICON_RAW_YELLOW : ICON_RAW);

				if (str != data->m_text)
				{
					data->m_text = str;
					m_pTree->SetItemText(id, str);
				}
				if (icon != data->m_icon)
				{
					data->m_icon = icon;
					m_pTree->SetItemImage(id, icon);
				}
			}
		}
		count++;
	}
}

void LayerDlg::RefreshTreeSpace()
{
	vtIcoGlobe *globe = g_App.GetGlobe();
	if (!globe)
		return;

	wxTreeItemId hRoot = m_pTree->AddRoot(_("Layers"), ICON_TOP, ICON_TOP);

	GlobeLayerArray &glayers = globe->GetGlobeLayers();
	for (uint i = 0; i < glayers.GetSize(); i++)
	{
		GlobeLayer *glay = glayers[i];
		vtFeatureSet *feat = glay->m_pSet;

		wxString str(feat->GetFilename(), wxConvUTF8);
		wxTreeItemId hItem = m_pTree->AppendItem(hRoot, str, ICON_RAW, ICON_RAW);
		m_pTree->SetItemData(hItem, new LayerItemData(glay));

		OGRwkbGeometryType type = feat->GetGeomType();
		int num = feat->NumEntities();
		str.Printf(_T("%d "), num);
		if (type == wkbPoint)
			str += _T("Point");
		if (type == wkbPoint25D)
			str += _T("PointZ");
		if (type == wkbLineString)
			str += _T("Arc");
		if (type == wkbPolygon)
			str += _T("Polygon");
		str += _T(" Feature");
		if (num != 1)
			str += _T("s");
		m_pTree->AppendItem(hItem, str, ICON_RAW, ICON_RAW);
		m_pTree->Expand(hItem);
	}
	m_pTree->Expand(hRoot);
}

// WDR: handler implementations for LayerDlg

void LayerDlg::OnLayerRemove( wxCommandEvent &event )
{
	LayerItemData *data = GetLayerDataFromItem(m_item);
	if (!data)
		return;

	if (data->m_layer != NULL)
	{
		// Inform table views, don't show a layer that's going away
		if (data->m_alay)
			g_App.OnSetDelete(data->m_alay->GetFeatureSet());

		OpenProgressDialog(_T("Deleting layer"), _T(""), false, this);
		m_pTerrain->RemoveLayer(data->m_layer, progress_callback);
		CloseProgressDialog();
	}
	else if (data->m_glay != NULL)
	{
		g_App.RemoveGlobeAbstractLayer(data->m_glay);
	}
	RefreshTreeContents();
}

void LayerDlg::OnLayerCreate( wxCommandEvent &event )
{
	if (!m_pTerrain)
		return;

	wxArrayString choices;
	choices.Add(_("Abstract (Points with labels)"));
	choices.Add(_("Structure"));
	choices.Add(_("Vegetation"));
	int index = wxGetSingleChoiceIndex(_("Layer type:"), _("Create new layer"), choices, this);
	if (index == 0)
	{
		if (CreateNewAbstractPointLayer(m_pTerrain))
			RefreshTreeContents();
	}
	else if (index == 1)
	{
		// make a new structure layer
		vtStructureLayer *slay = m_pTerrain->NewStructureLayer();
		slay->SetFilename("Untitled.vtst");
		m_pTerrain->SetActiveLayer(slay);
		RefreshTreeContents();
	}
	else if (index == 2)
	{
		// make a new veg layer
		vtVegLayer *vlay = m_pTerrain->NewVegLayer();
		vlay->SetFilename("Untitled.vf");
		m_pTerrain->SetActiveLayer(vlay);
		RefreshTreeContents();
	}
}

//Helper
bool SaveAbstractLayer(vtAbstractLayer *alay, bool bAskFilename)
{
	vtFeatureSet *fset = alay->GetFeatureSet();
	vtString fname = fset->GetFilename();

	if (bAskFilename)
	{
		// save current directory
		wxString path = wxGetCwd();

		wxString default_file(StartOfFilename(fname), wxConvUTF8);
		wxString default_dir(ExtractPath(fname, false), wxConvUTF8);

		EnableContinuousRendering(false);
		wxFileDialog saveFile(NULL, _("Save Abstract Data"), default_dir,
			default_file, FSTRING_SHP, wxFD_SAVE);
		bool bResult = (saveFile.ShowModal() == wxID_OK);
		EnableContinuousRendering(true);
		if (!bResult)
		{
			wxSetWorkingDirectory(path);	// restore
			return false;
		}
		wxString str = saveFile.GetPath();
		fname = str.mb_str(wxConvUTF8);
		fset->SetFilename(fname);
	}
	bool success = fset->SaveToSHP(fname);

	if (success)
		alay->SetModified(false);
	else
		wxMessageBox(_("Couldn't save layer."));

	return success;
}

void LayerDlg::OnLayerLoad( wxCommandEvent &event )
{
	VTLOG1("LayerDlg::OnLayerLoad\n");

	wxString filter = _("Layer Formats|");
	AddType(filter, FSTRING_SHP);
	if (g_App.m_state == AS_Terrain)
	{
		AddType(filter, FSTRING_VTST);
		AddType(filter, FSTRING_VF);
		AddType(filter, FSTRING_TIN);
	}

	wxFileDialog loadFile(NULL, _("Load Layer"), _T(""), _T(""), filter, wxFD_OPEN);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	wxString str = loadFile.GetPath();
	vtString fname = (const char *) str.mb_str(wxConvUTF8);

	VTLOG1(" File dialog: ");
	VTLOG1(fname);
	VTLOG1("\n");

	GetFrame()->LoadLayer(fname);
}

void LayerDlg::OnLayerSave( wxCommandEvent &event )
{
	VTLOG1("LayerDlg::OnLayerSave\n");

	LayerItemData *data = GetLayerDataFromItem(m_item);
	if (!data)
		return;

	bool bSaved = false;
	if (data->m_type == LT_STRUCTURE && data->m_slay != NULL)
		bSaved = g_App.SaveStructures(false);	// don't ask for filename

	if (data->m_type == LT_VEG)
		bSaved = g_App.SaveVegetation(false);	// don't ask for filename

	if (data->m_type == LT_ABSTRACT)
		bSaved = SaveAbstractLayer(data->m_alay, false);	// don't ask for filename

	// Update the (*) next to the modified layer name
	if (bSaved)
		RefreshTreeContents();
}

void LayerDlg::OnLayerSaveAs( wxCommandEvent &event )
{
	VTLOG1("LayerDlg::OnLayerSaveAs\n");

	LayerItemData *data = GetLayerDataFromItem(m_item);
	if (!data)
		return;

	bool bSaved = false;
	if (data->m_type == LT_STRUCTURE)
		bSaved = g_App.SaveStructures(true);		// ask for filename

	if (data->m_type == LT_VEG)
		bSaved = g_App.SaveVegetation(true);	// ask for filename

	if (data->m_type == LT_ABSTRACT)
		bSaved = SaveAbstractLayer(data->m_alay, true);	// ask for filename

	// The filename may have changed
	if (bSaved)
		RefreshTreeContents();
}

void LayerDlg::OnZoomTo( wxCommandEvent &event )
{
	VTLOG1("LayerDlg::OnZoomTo\n");

	osg::Node *pThing = GetNodeFromItem(m_item, true);	// get container
	if (pThing)
	{
		FSphere sphere = GetGlobalBoundSphere(pThing);   // get global bounds
		vtCamera *pCam = vtGetScene()->GetCamera();

		// Put the camera a bit back from the sphere; sufficiently so that
		//  the whole volume of the bounding sphere is visible.
		float smallest = min(pCam->GetFOV(), pCam->GetVertFOV());
		float alpha = smallest / 2.0f;
		float distance = sphere.radius / tanf(alpha);
		pCam->Identity();
		pCam->Rotate(FPoint3(1,0,0), -PID2f/2);	// tilt down a little
		pCam->Translate(sphere.center);
		pCam->TranslateLocal(FPoint3(0.0f, 0.0f, distance));
	}
}

void LayerDlg::OnShadowVisible( wxCommandEvent &event)
{
	bool bShow = event.IsChecked();

	LayerItemData *data = GetLayerDataFromItem(m_item);
	if (!data)
		return;
	vtStructureLayer *slay = data->m_slay;
	if (slay != NULL)
	{
		if (data->m_item != -1)
		{
			// individual item is selected
			vtStructure3d *str3d = slay->GetStructure3d(data->m_item);
			str3d->SetCastShadow(bShow);

			vtStructure *str = slay->at(data->m_item);
			// remember state
			if (!bShow)
				str->SetValueBool("shadow", false);
			else
				// shadows are on by default, so don't store shadow=true
				str->RemoveTag("shadow");

			return;
		}
		else
			// whole layer is selected
			slay->SetShadows(bShow);
	}
}

void LayerDlg::OnUpdateShadow(wxUpdateUIEvent& event)
{
	LayerItemData *data = GetLayerDataFromItem(m_item);
	if (!data)
	{
		event.Enable(false);
		return;
	}
	vtStructureLayer *slay = data->m_slay;
	if (!slay)
	{
		// only structures cast shadows
		event.Enable(false);
		return;
	}
	if (data->m_item != -1)
	{
		// individual item is selected
		vtStructure3d *str3d = slay->GetStructure3d(data->m_item);
		event.Enable(true);
		event.Check(str3d->GetCastShadow());
	}
	else
	{
		// whole layer is selected
		int count = slay->size();
		if (count)
		{
			// just use first item
			vtStructure3d *str3d = slay->GetStructure3d(0);
			event.Enable(true);
			event.Check(str3d->GetCastShadow());
		}
		else
			event.Enable(false);
	}
}

void LayerDlg::OnVisible( wxCommandEvent &event )
{
	bool bVis = event.IsChecked();

	if (g_App.m_state == AS_Terrain)
	{
		// Structure layers are special because they have items which can be
		// individually shown/hidden:
		vtStructureLayer *slay = GetStructureLayerFromItem(m_item);
		osg::Node *pThing = GetNodeFromItem(m_item);
		if (pThing && slay != NULL)
		{
			SetEnabled(pThing, bVis);
			return;
		}

		// General case is to show/hide the whole layer.
		vtLayer *lay = GetLayerFromItem(m_item);
		if (lay)
			lay->SetVisible(bVis);
	}
	else if (g_App.m_state == AS_Orbit)
	{
		LayerItemData *data = GetLayerDataFromItem(m_item);
		if (data && data->m_glay)
			data->m_glay->SetEnabled(bVis);
	}
}

void LayerDlg::OnTable( wxCommandEvent &event )
{
	LayerItemData *data = GetLayerDataFromItem(m_item);
	if (data && data->m_alay)
		GetFrame()->ShowTable(data->m_alay);
}

void LayerDlg::OnRefresh( wxCommandEvent &event )
{
	bool bVis = event.IsChecked();

	if (g_App.m_state == AS_Terrain)
	{
		vtLayer *lay = GetLayerFromItem(m_item);
		if (lay)
		{
			vtAbstractLayer *alay = dynamic_cast<vtAbstractLayer*>(lay);
			if (alay)
				alay->Reload();
		}
	}
	else if (g_App.m_state == AS_Orbit)
	{
		// TODO
		//LayerItemData *data = GetLayerDataFromItem(m_item);
		//if (data && data->m_glay)
		//	data->m_glay->SetEnabled(bVis);
	}
}

void LayerDlg::OnUpdateVisible(wxUpdateUIEvent& event)
{
	if (!IsShown())
		return;

	if (g_App.m_state == AS_Terrain)
	{
		vtStructureLayer *slay = GetStructureLayerFromItem(m_item);
		osg::Node *pThing = GetNodeFromItem(m_item);
		if (pThing && slay != NULL)
		{
			event.Check(GetEnabled(pThing));
			return;
		}
		vtLayer *lay = GetLayerFromItem(m_item);
		if (lay)
		{
			event.Check(lay->GetVisible());
		}
		event.Enable(pThing != NULL || lay != NULL);
	}
	else if (g_App.m_state == AS_Orbit)
	{
		LayerItemData *data = GetLayerDataFromItem(m_item);
		if (data && data->m_glay)
			event.Check(data->m_glay->GetEnabled());
		event.Enable(data && data->m_glay);
	}
}

void LayerDlg::OnUpdateRefresh(wxUpdateUIEvent& event)
{
	if (!IsShown())
		return;
	if (g_App.m_state == AS_Terrain)
	{
		vtLayer *lay = GetLayerFromItem(m_item);
		vtAbstractLayer *alay = dynamic_cast<vtAbstractLayer*>(lay);
		event.Enable(alay != NULL);
	}
	else
		event.Enable(false);
}

void LayerDlg::OnUpdateCreate(wxUpdateUIEvent& event)
{
	if (!IsShown())
		return;
	event.Enable(g_App.m_state == AS_Terrain);
}


void LayerDlg::OnShowAll( wxCommandEvent &event )
{
	// Check to see if this might be more than they expected
	if (g_App.m_state == AS_Terrain && m_bShowAll == false && event.IsChecked())
	{
		// Count all the structures in all the layers
		int total = 0;
		if (!m_pTerrain)
			return;
		LayerSet &layers = m_pTerrain->GetLayers();
		for (uint i = 0; i < layers.size(); i++)
		{
			vtStructureLayer *slay = dynamic_cast<vtStructureLayer*>(layers[i].get());
			if (slay)
				total += slay->size();
		}
		if (total > 5000)
		{
			wxString str;
			str.Printf(_("There are %d structures.  Are you sure you want to display them all?"), total);
			int res = wxMessageBox(str, _("Warning"), wxYES_NO);
			if (res == wxNO)
				return;
		}
	}

	m_bShowAll = event.IsChecked();
	RefreshTreeContents();
}

void LayerDlg::OnUpdateShowAll(wxUpdateUIEvent& event)
{
	event.Check(m_bShowAll);
	event.Enable(g_App.m_state == AS_Terrain);
}

void LayerDlg::OnSelChanged( wxTreeEvent &event )
{
	m_item = event.GetItem();

	LayerItemData *data = GetLayerDataFromItem(m_item);
	if (data && data->m_layer != NULL)
	{
		vtLayer *newlay = data->m_layer;
		vtLayer *oldlay = m_pTerrain->GetActiveLayer();
		if (newlay != oldlay)
		{
			m_pTerrain->SetActiveLayer(newlay);
			UpdateTreeTerrain();
		}
	}

	UpdateEnabling();
}

void LayerDlg::OnItemActived( wxTreeEvent &event )
{
	// The item was activated (double click, enter, space)
	OnZoomTo(event);
}

void LayerDlg::UpdateEnabling()
{
	bool bRemovable = false, bSaveable = false;

	LayerItemData *data = GetLayerDataFromItem(m_item);
	osg::Node *pThing = GetNodeFromItem(m_item);
	vtStructureLayer *slay = GetStructureLayerFromItem(m_item);

	if (data != NULL)
	{
		// We can save a structure layer if it is selected
		if (data->m_type == LT_STRUCTURE && slay)
			bRemovable = bSaveable = true;

		// We can save always save or remove an abstract layer
		if (data->m_type == LT_ABSTRACT)
			bRemovable = bSaveable = true;

		// We can save a vegetation layer
		if (data->m_type == LT_VEG)
			bRemovable = bSaveable = true;

		if (data->m_type == LT_ELEVATION)
		{
			bRemovable = true;
			bSaveable = false;
		}
	}

	m_pToolbar->EnableTool(ID_LAYER_TABLE, (data && data->m_alay));

	m_pToolbar->EnableTool(ID_LAYER_DELETE, bRemovable);

	m_pToolbar->EnableTool(ID_LAYER_SAVE, bSaveable);
	m_pToolbar->EnableTool(ID_LAYER_SAVE_AS, bSaveable);

	m_pToolbar->EnableTool(ID_LAYER_ZOOM_TO, pThing != NULL);
}

