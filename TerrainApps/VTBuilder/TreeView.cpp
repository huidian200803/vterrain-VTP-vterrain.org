//
// TreeView.cpp
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/FilePath.h"
#include "vtdata/vtLog.h"

#include "App.h"
#include "TreeView.h"
#include "MenuEnum.h"	// for LayerTree_Ctrl
#include "Frame.h"
#include "BuilderView.h"
#include "ImageLayer.h"
#include "vtImage.h"
#include "vtui/Helper.h"

DECLARE_APP(BuilderApp)

// Under Windows, the icons are in the .rc file; on Unix, they are included
// from .xpm files.
#ifndef __WXMSW__
#include "building.xpm"
#include "file1.xpm"
#include "file2.xpm"
#include "folder1.xpm"
#include "folder2.xpm"
#include "folder3.xpm"

#include "grid.xpm"
#include "image.xpm"
#include "raw.xpm"
#include "road.xpm"
#include "veg1.xpm"
#include "water.xpm"
#include "util.xpm"
#include "transit.xpm"
#endif

/////////////////////////////////////////////////


// MyTreeCtrl implementation
IMPLEMENT_DYNAMIC_CLASS(MyTreeCtrl, wxTreeCtrl)

MyTreeCtrl::MyTreeCtrl(wxWindow *parent, const wxWindowID id,
	const wxPoint& pos, const wxSize& size,
	long style)
: wxTreeCtrl(parent, id, pos, size, style)
{
	m_reverseSort = false;
	m_imageListNormal = NULL;
	m_bShowPaths = true;
	m_bUser = true;

	CreateImageList(16);

	// Add some items to the tree
	RefreshTreeItems(NULL);
}

void MyTreeCtrl::CreateImageList(int size)
{
	delete m_imageListNormal;

	if ( size == -1 )
	{
		m_imageListNormal = NULL;
		return;
	}

	// Make an image list containing small icons
	m_imageListNormal = new wxImageList(size, size, TRUE);

	// should correspond to TreeCtrlIcon_xxx enum
	wxIcon icons[14];
	icons[0] = wxICON(file1);
	icons[1] = wxICON(file2);
	icons[2] = wxICON(folder1);
	icons[3] = wxICON(folder2);
	icons[4] = wxICON(folder3);
	icons[5] = wxICON(building);
	icons[6] = wxICON(road);
	icons[7] = wxICON(grid);
	icons[8] = wxICON(image);
	icons[9] = wxICON(veg1);
	icons[10] = wxICON(water);
	icons[11] = wxICON(transit);
	icons[12] = wxICON(util);
	icons[13] = wxICON(raw);

	int sizeOrig = icons[0].GetWidth();
	for ( size_t i = 0; i < WXSIZEOF(icons); i++ )
	{
		if ( size == sizeOrig )
			m_imageListNormal->Add(icons[i]);
		else
			m_imageListNormal->Add(wxBitmap(icons[i]).ConvertToImage().Rescale(size, size));
	}

	SetImageList(m_imageListNormal);
}

MyTreeCtrl::~MyTreeCtrl()
{
	delete m_imageListNormal;
}

int MyTreeCtrl::OnCompareItems(const wxTreeItemId& item1,
	const wxTreeItemId& item2)
{
	if ( m_reverseSort )
	{
		// just exchange 1st and 2nd items
		return wxTreeCtrl::OnCompareItems(item2, item1);
	}
	else
	{
		return wxTreeCtrl::OnCompareItems(item1, item2);
	}
}

wxTreeItemId rootId;

wxTreeItemId MyTreeCtrl::AddRootItem(int image, const wxString &text)
{
	wxTreeItemId id = AppendItem(rootId, text, image);
	SetItemBold(id);
	return id;
}

wxString MyTreeCtrl::MakeItemName(vtLayerPtr lp)
{
	wxString str;
	if (lp->GetModified())
		str = _T("(*) ");
	wxString path = lp->GetLayerFilename();

	if (!m_bShowPaths)
		path = StartOfFilenameWX(path);

	str += path;
	return str;
}

void MyTreeCtrl::RefreshTreeItems(Builder *pBuilder)
{
	VTLOG1("Refreshing Tree Items:");

	// Deleting the previous items can call OnSelChanged, which can cause VTB
	//  to forget the active layer, so indicate that this is not user input.
	m_bUser = false;
	DeleteAllItems();
	m_bUser = true;

	rootId = AddRoot(_("Layers"));
	SetItemBold(rootId);

	int	image, imageSel;

	wxTreeItemId elevId =	AddRootItem(MyTreeCtrl::TreeCtrlIcon_Grid, _("Elevation"));
	SetItemData(elevId, new MyTreeItemData(LT_ELEVATION));

	wxTreeItemId imageId =	AddRootItem(MyTreeCtrl::TreeCtrlIcon_Image, _("Images"));
	SetItemData(imageId, new MyTreeItemData(LT_IMAGE));

	wxTreeItemId buildId =	AddRootItem(MyTreeCtrl::TreeCtrlIcon_Building, _("Structures"));
	SetItemData(buildId, new MyTreeItemData(LT_STRUCTURE));

	wxTreeItemId roadId =	AddRootItem(MyTreeCtrl::TreeCtrlIcon_Road, _("Roads"));
	SetItemData(roadId, new MyTreeItemData(LT_ROAD));

	wxTreeItemId vegId =	AddRootItem(MyTreeCtrl::TreeCtrlIcon_Veg1, _("Vegetation"));
	SetItemData(vegId, new MyTreeItemData(LT_VEG));

	wxTreeItemId waterId =	AddRootItem(MyTreeCtrl::TreeCtrlIcon_Water, _("Water"));
	SetItemData(waterId, new MyTreeItemData(LT_WATER));

	wxTreeItemId utilityId = AddRootItem(MyTreeCtrl::TreeCtrlIcon_Utility, _("Utilities"));
	SetItemData(utilityId, new MyTreeItemData(LT_UTILITY));

	wxTreeItemId rawId =	AddRootItem(MyTreeCtrl::TreeCtrlIcon_Raw, _("Raw"));
	SetItemData(rawId, new MyTreeItemData(LT_RAW));

	image = TreeCtrlIcon_File;
	imageSel = TreeCtrlIcon_FileSelected;
	vtLayerPtr lp;
	int iLayers = 0;
	if (pBuilder) iLayers = pBuilder->NumLayers();
	wxTreeItemId hSelectedItem;
	for (int i = 0; i < iLayers; i++)
	{
		lp = pBuilder->GetLayer(i);

		wxString str = MakeItemName(lp);

		wxTreeItemId hItem;
		switch (lp->GetType())
		{
		case LT_ELEVATION:
			hItem = AppendItem(elevId, str, image, imageSel);
			break;
		case LT_IMAGE:
			hItem = AppendItem(imageId, str, image, imageSel);
			break;
		case LT_ROAD:
			hItem = AppendItem(roadId, str, image, imageSel);
			break;
		case LT_STRUCTURE:
			hItem = AppendItem(buildId, str, image, imageSel);
			break;
		case LT_VEG:
			hItem = AppendItem(vegId, str, image, imageSel);
			break;
		case LT_WATER:
			hItem = AppendItem(waterId, str, image, imageSel);
			break;
		case LT_UTILITY:
			hItem = AppendItem(utilityId, str, image, imageSel);
			break;
		case LT_RAW:
			hItem = AppendItem(rawId, str, image, imageSel);
			break;
		case LT_UNKNOWN:
		case LAYER_TYPES:
			// Keep picky compilers quiet.
			break;
		}
		if (hItem.IsOk())
		{
			SetItemData(hItem, new MyTreeItemData(lp));

			if (lp == pBuilder->GetActiveLayer())
				hSelectedItem = hItem;
			if (!lp->GetVisible())
			{
				SetItemFont(hItem, *wxITALIC_FONT);
				SetItemTextColour(hItem, wxColour(80,80,80));
			}
		}
	}
	VTLOG(" %d layers.\n", iLayers);

	// Expand the groups after they have all their items.  It doesn't work
	//  to expand before, because then not all items are shown.
	Expand(elevId);
	Expand(imageId);
	Expand(roadId);
	Expand(buildId);
	Expand(vegId);
	Expand(waterId);
	Expand(utilityId);
	Expand(rawId);

	// Wait until all the groups are expanded, before highlighting the
	//  selected item, so that it will definitely be in view.
	if (hSelectedItem.IsOk())
		SelectItem(hSelectedItem);
}

void MyTreeCtrl::RefreshTreeStatus(Builder *pBuilder)
{
	VTLOG("(Refreshing Tree Status)\n");

	wxTreeItemId root = GetRootItem();
	wxTreeItemId parent, item;

	wxTreeItemIdValue cookie;
	for (parent = GetFirstChild(root, cookie); parent; parent = GetNextChild(root, cookie))
	{
		wxTreeItemIdValue cookie2;
		for (item = GetFirstChild(parent, cookie2); item; item = GetNextChild(parent, cookie2))
		{
			MyTreeItemData *data = (MyTreeItemData *)GetItemData(item);
			if (data)
			{
				SetItemText(item, MakeItemName(data->m_pLayer));
				if (data->m_pLayer == pBuilder->GetActiveLayer())
					SelectItem(item);
				if (data->m_pLayer->GetVisible())
				{
					SetItemFont(item, *wxNORMAL_FONT);
					SetItemTextColour(item, wxColour(0,0,0));
				}
				else
				{
					SetItemFont(item, *wxITALIC_FONT);
					SetItemTextColour(item, wxColour(80,80,80));
				}
			}
		}
	}
}


// avoid repetition
#define TREE_EVENT_HANDLER(name)			\
void MyTreeCtrl::name(wxTreeEvent& event)	\
{											\
	event.Skip();							\
}

TREE_EVENT_HANDLER(OnBeginRDrag)
TREE_EVENT_HANDLER(OnDeleteItem)
TREE_EVENT_HANDLER(OnGetInfo)
TREE_EVENT_HANDLER(OnItemExpanded)
TREE_EVENT_HANDLER(OnItemExpanding)
TREE_EVENT_HANDLER(OnItemCollapsed)
TREE_EVENT_HANDLER(OnSelChanging)
TREE_EVENT_HANDLER(OnTreeKeyDown)

#undef TREE_EVENT_HANDLER

void MyTreeCtrl::OnSelChanged(wxTreeEvent& event)
{
	if (!m_bUser)	// only listen for actual user input
		return;

	wxTreeItemId item = event.GetItem();
	if (!item.IsOk())
		return;

	MyTreeItemData *data = (MyTreeItemData *)GetItemData(item);
	vtLayerPtr lp = NULL;
	if (data)
		lp = data->m_pLayer;

	MainFrame *frame = GetMainFrame();
	vtLayerPtr last = g_bld->GetActiveLayer();
	if (lp != last)
		frame->GetView()->SetActiveLayer(lp);

	LayerType curr_ltype = lp ? lp->GetType() : LT_UNKNOWN;
	LayerType last_ltype = last ? last->GetType() : LT_UNKNOWN;
	if (curr_ltype != last_ltype)
		frame->RefreshToolbars();
}

void MyTreeCtrl::OnItemRightClick(wxTreeEvent& event)
{
	// for testing, prevent the user from collapsing the first child folder
	wxTreeItemId itemId = event.GetItem();

	MyTreeItemData *data = (MyTreeItemData *)GetItemData(itemId);
	if (data)
	{
		if (data->m_pLayer)
			m_clicked_layer_type = data->m_pLayer->GetType();
		else
			m_clicked_layer_type = data->m_type;

		wxMenu *popmenu = new wxMenu;
		if (data->m_pLayer)
		{
			popmenu->Append(ID_POPUP_PROPS, _("Properties"));
			popmenu->Append(ID_POPUP_TO_TOP, _("Move to Top (draw first)"));
			popmenu->Append(ID_POPUP_TO_BOTTOM, _("Move to Bottom (draw last)"));

			vtImageLayer *pIL = dynamic_cast<vtImageLayer*>(data->m_pLayer);
			if (pIL && pIL->GetImage() && pIL->GetImage()->GetBitmap() != NULL)
			{
				popmenu->AppendSeparator();
				popmenu->Append(ID_POPUP_OVR_DISK, _("Create Overviews on Disk"));
				popmenu->Append(ID_POPUP_OVR_MEM, _("Create Overviews in Memory"));
			}
			popmenu->AppendSeparator();
		}
		popmenu->Append(ID_POPUP_SHOWALL, _("Show All"));
		popmenu->Append(ID_POPUP_HIDEALL, _("Hide All"));
		PopupMenu(popmenu);
		delete popmenu;
	}
}

void MyTreeCtrl::OnBeginDrag(wxTreeEvent& event)
{
}

void MyTreeCtrl::OnEndDrag(wxTreeEvent& event)
{
}

void MyTreeCtrl::OnBeginLabelEdit(wxTreeEvent& event)
{
#if 0
	wxLogMessage("OnBeginLabelEdit");

	// for testing, prevent this items label editing
	wxTreeItemId itemId = event.GetItem();
	if ( IsTestItem(itemId) )
	{
		wxMessageBox("You can't edit this item.");

		event.Veto();
	}
#endif
}

void MyTreeCtrl::OnEndLabelEdit(wxTreeEvent& event)
{
#if 0
	wxLogMessage("OnEndLabelEdit");

	// don't allow anything except letters in the labels
	if ( !event.GetLabel().IsWord() )
	{
		wxMessageBox("The label should contain only letters.");

		event.Veto();
	}
#endif
}

void MyTreeCtrl::OnItemCollapsing(wxTreeEvent& event)
{
#if 0
	wxLogMessage("OnItemCollapsing");

	// for testing, prevent the user from collapsing the first child folder
	wxTreeItemId itemId = event.GetItem();
	if ( IsTestItem(itemId) )
	{
		wxMessageBox("You can't collapse this item.");

		event.Veto();
	}
#endif
}

void MyTreeCtrl::OnItemActivated(wxTreeEvent& event)
{
#if 0
	// show some info about this item
	wxTreeItemId itemId = event.GetItem();
	MyTreeItemData *item = (MyTreeItemData *)GetItemData(itemId);

	if ( item != NULL )
	{
		item->ShowInfo(this);
	}
	wxLogMessage("OnItemActivated");
#endif
}

void MyTreeCtrl::OnRMouseDClick(wxMouseEvent& event)
{
#if 0
	wxTreeItemId id = HitTest(event.GetPosition());
	if ( !id )
		wxLogMessage("No item under mouse");
	else
	{
		MyTreeItemData *item = (MyTreeItemData *)GetItemData(id);
		if ( item )
			wxLogMessage("Item '%s' under mouse", item->GetDesc());
	}
#endif
}

/////////////////////////////////////////////////

BEGIN_EVENT_TABLE(MyTreeCtrl, wxTreeCtrl)
EVT_TREE_BEGIN_DRAG(LayerTree_Ctrl, MyTreeCtrl::OnBeginDrag)
EVT_TREE_BEGIN_RDRAG(LayerTree_Ctrl, MyTreeCtrl::OnBeginRDrag)
EVT_TREE_END_DRAG(LayerTree_Ctrl, MyTreeCtrl::OnEndDrag)
EVT_TREE_BEGIN_LABEL_EDIT(LayerTree_Ctrl, MyTreeCtrl::OnBeginLabelEdit)
EVT_TREE_END_LABEL_EDIT(LayerTree_Ctrl, MyTreeCtrl::OnEndLabelEdit)
EVT_TREE_DELETE_ITEM(LayerTree_Ctrl, MyTreeCtrl::OnDeleteItem)
EVT_TREE_ITEM_EXPANDED(LayerTree_Ctrl, MyTreeCtrl::OnItemExpanded)
EVT_TREE_ITEM_EXPANDING(LayerTree_Ctrl, MyTreeCtrl::OnItemExpanding)
EVT_TREE_ITEM_COLLAPSED(LayerTree_Ctrl, MyTreeCtrl::OnItemCollapsed)
EVT_TREE_ITEM_COLLAPSING(LayerTree_Ctrl, MyTreeCtrl::OnItemCollapsing)
EVT_TREE_ITEM_RIGHT_CLICK(LayerTree_Ctrl, MyTreeCtrl::OnItemRightClick)
EVT_TREE_SEL_CHANGED(LayerTree_Ctrl, MyTreeCtrl::OnSelChanged)
EVT_TREE_SEL_CHANGING(LayerTree_Ctrl, MyTreeCtrl::OnSelChanging)
EVT_TREE_KEY_DOWN(LayerTree_Ctrl, MyTreeCtrl::OnTreeKeyDown)
EVT_TREE_ITEM_ACTIVATED(LayerTree_Ctrl, MyTreeCtrl::OnItemActivated)
EVT_RIGHT_DCLICK(MyTreeCtrl::OnRMouseDClick)
END_EVENT_TABLE()

