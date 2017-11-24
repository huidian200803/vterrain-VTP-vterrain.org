//
// TreeView.cpp
//
// Copyright (c) 2001-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"

#include "app.h"
#include "TreeView.h"
#include "menu_id.h"
#include "frame.h"

#include "vtdata/Content.h"
#include "vtdata/vtLog.h"

DECLARE_APP(vtApp)

// We use xpm icons on all platforms
#  include "wxosg/icons/geode.xpm"
#  include "wxosg/icons/group.xpm"
#  include "wxosg/icons/top.xpm"
#  include "icons/group2.xpm"
#  include "icons/geom2.xpm"

#define LocalIcon(X) wxIcon( (const char**) X##_xpm )


//////////////////////////////////////////////////////////////////////////

// MyTreeCtrl implementation

MyTreeCtrl::MyTreeCtrl(wxWindow *parent, const wxWindowID id, const wxPoint& pos,
					   const wxSize& size, long style)
		  : wxTreeCtrl(parent, id, pos, size, style)
{
	m_imageListNormal = NULL;
	m_bUpdating = false;

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
	wxIcon icons[5];
	icons[0] = LocalIcon(top);		// Content
	icons[1] = LocalIcon(group);	// Item
	icons[2] = LocalIcon(group2);	// ItemSelected
	icons[3] = LocalIcon(geode);	// Model
	icons[4] = LocalIcon(geom2);	// ModelSelected

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

wxTreeItemId rootId;

void MyTreeCtrl::RefreshTreeItems(vtFrame *pFrame)
{
	VTLOG(" RefreshTreeItems\n");

	m_bUpdating = true;

	DeleteAllItems();

	rootId = AddRoot(_T("Content"), MyTreeCtrl::TreeCtrlIcon_Content);
	SetItemBold(rootId);

	if (!pFrame)
	{
		m_bUpdating = false;
		return;
	}

	vtContentManager *pMan = &(pFrame->m_Man);

	uint i, j;
	uint iItems = pMan->NumItems();
	for (i = 0; i < iItems; i++)
	{
		vtItem *pItem = pMan->GetItem(i);

		wxString str(pItem->m_name, wxConvUTF8);

		wxTreeItemId hItem;
		hItem = AppendItem(rootId, str, TreeCtrlIcon_Item, TreeCtrlIcon_ItemSelected);

		SetItemData(hItem, new MyTreeItemData(pItem));

		if (pItem == pFrame->m_pCurrentItem)
			SelectItem(hItem);

		for (j = 0; j < pItem->NumModels(); j++)
		{
			vtModel *pModel = pItem->GetModel(j);

			str = wxString(pModel->m_filename, wxConvUTF8);

			wxTreeItemId hItem2;
			hItem2 = AppendItem(hItem, str, TreeCtrlIcon_Model, TreeCtrlIcon_ModelSelected);

			SetItemData(hItem2, new MyTreeItemData(pModel));

			if (pModel == pFrame->m_pCurrentModel)
				SelectItem(hItem2);
		}
		Expand(hItem);
	}

	Expand(rootId);
	m_bUpdating = false;
}

void MyTreeCtrl::RefreshTreeStatus(vtFrame *pFrame)
{
	m_bUpdating = true;

	wxTreeItemId root = GetRootItem();
	wxTreeItemId parent, item;
	wxTreeItemIdValue  cookie, cookie2;

	for (parent = GetFirstChild(root, cookie); parent.IsOk(); parent = GetNextChild(root, cookie))
	{
		for (item = GetFirstChild(parent, cookie2); item.IsOk(); item = GetNextChild(parent, cookie2))
		{
			MyTreeItemData *data = (MyTreeItemData *)GetItemData(item);
			if (data)
			{
				if (data->m_pItem == pFrame->m_pCurrentItem)
					SelectItem(item);
				if (data->m_pModel == pFrame->m_pCurrentModel)
					SelectItem(item);
			}
		}
	}
	m_bUpdating = false;
}

// avoid repetition
#define TREE_EVENT_HANDLER(name)			\
void MyTreeCtrl::name(wxTreeEvent& event)	\
{											\
 /*	wxLogMessage(#name); */					\
	event.Skip();							\
}

TREE_EVENT_HANDLER(OnBeginRDrag)
TREE_EVENT_HANDLER(OnDeleteItem)
TREE_EVENT_HANDLER(OnGetInfo)
TREE_EVENT_HANDLER(OnSetInfo)
TREE_EVENT_HANDLER(OnItemExpanded)
TREE_EVENT_HANDLER(OnItemExpanding)
TREE_EVENT_HANDLER(OnItemCollapsed)
TREE_EVENT_HANDLER(OnSelChanging)
TREE_EVENT_HANDLER(OnTreeKeyDown)

#undef TREE_EVENT_HANDLER

void MyTreeCtrl::OnSelChanged(wxTreeEvent& event)
{
	// don't inform the rest of the interface, if it's currently informing us
	// that's a bad feedback loop
	if (m_bUpdating)
		return;

	wxTreeItemId item = event.GetItem();
	if (!IsSelected(item))
		return;

	bool bSelection = event.IsSelection();

	MyTreeItemData *data = (MyTreeItemData *)GetItemData(item);
	vtModel *mod = NULL;
	vtItem *ite = NULL;
	if (data)
	{
		mod = data->m_pModel;
		ite = data->m_pItem;
		if (mod)
		{
			wxTreeItemId pitem = GetItemParent(item);
			data = (MyTreeItemData *)GetItemData(pitem);
			ite = data->m_pItem;
		}
	}

	GetMainFrame()->SetCurrentItemAndModel(ite, mod);
}

void MyTreeCtrl::OnBeginDrag(wxTreeEvent& event)
{
}

void MyTreeCtrl::OnEndDrag(wxTreeEvent& event)
{
}

void MyTreeCtrl::OnBeginLabelEdit(wxTreeEvent& event)
{
	VTLOG("OnBeginLabelEdit\n");

	// If not an item prevent this items label editing
	wxTreeItemId itemId = event.GetItem();
	MyTreeItemData *data = (MyTreeItemData *)GetItemData(itemId);
	if (data != NULL && data->m_pItem != NULL && data->m_pModel == NULL)
	{
		// allow edit
	}
	else
	{
		VTLOG("You can't edit this item.\n");
		event.Veto();
	}
}

void MyTreeCtrl::OnEndLabelEdit(wxTreeEvent& event)
{
	VTLOG("OnEndLabelEdit\n");

	wxString result = event.GetLabel();
	wxTreeItemId itemId = event.GetItem();
	MyTreeItemData *data = (MyTreeItemData *)GetItemData(itemId);
	if (data != NULL && data->m_pItem != NULL)
	{
//		RefreshTreeStatus(frame);	// no need; tree already updated
		vtString label = (const char *) result.mb_str(wxConvUTF8);
		GetMainFrame()->SetItemName(data->m_pItem, label);
	}
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
	EVT_TREE_BEGIN_DRAG(ID_TREECTRL, MyTreeCtrl::OnBeginDrag)
	EVT_TREE_BEGIN_RDRAG(ID_TREECTRL, MyTreeCtrl::OnBeginRDrag)
	EVT_TREE_END_DRAG(ID_TREECTRL, MyTreeCtrl::OnEndDrag)
	EVT_TREE_BEGIN_LABEL_EDIT(ID_TREECTRL, MyTreeCtrl::OnBeginLabelEdit)
	EVT_TREE_END_LABEL_EDIT(ID_TREECTRL, MyTreeCtrl::OnEndLabelEdit)
	EVT_TREE_DELETE_ITEM(ID_TREECTRL, MyTreeCtrl::OnDeleteItem)
	EVT_TREE_SET_INFO(ID_TREECTRL, MyTreeCtrl::OnSetInfo)
	EVT_TREE_ITEM_EXPANDED(ID_TREECTRL, MyTreeCtrl::OnItemExpanded)
	EVT_TREE_ITEM_EXPANDING(ID_TREECTRL, MyTreeCtrl::OnItemExpanding)
	EVT_TREE_ITEM_COLLAPSED(ID_TREECTRL, MyTreeCtrl::OnItemCollapsed)
	EVT_TREE_ITEM_COLLAPSING(ID_TREECTRL, MyTreeCtrl::OnItemCollapsing)
	EVT_TREE_SEL_CHANGED(ID_TREECTRL, MyTreeCtrl::OnSelChanged)
	EVT_TREE_SEL_CHANGING(ID_TREECTRL, MyTreeCtrl::OnSelChanging)
	EVT_TREE_KEY_DOWN(ID_TREECTRL, MyTreeCtrl::OnTreeKeyDown)
	EVT_TREE_ITEM_ACTIVATED(ID_TREECTRL, MyTreeCtrl::OnItemActivated)
	EVT_RIGHT_DCLICK(MyTreeCtrl::OnRMouseDClick)
END_EVENT_TABLE()

