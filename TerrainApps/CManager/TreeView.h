//
// TreeView.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/treectrl.h"
#include "wx/imaglist.h"
#include "wx/image.h"

class vtFrame;
class vtItem;
class vtModel;

class MyTreeItemData : public wxTreeItemData
{
public:
	MyTreeItemData(vtModel *pModel) { m_pModel = pModel; m_pItem = NULL; }
	MyTreeItemData(vtItem *pItem) { m_pModel = NULL; m_pItem = pItem; }
	vtItem *m_pItem;
	vtModel *m_pModel;
};

class MyTreeCtrl : public wxTreeCtrl
{
public:
	enum
	{
		TreeCtrlIcon_Content,
		TreeCtrlIcon_Item,
		TreeCtrlIcon_ItemSelected,
		TreeCtrlIcon_Model,
		TreeCtrlIcon_ModelSelected
	};

	MyTreeCtrl() { }
	MyTreeCtrl(wxWindow *parent, const wxWindowID id,
			   const wxPoint& pos, const wxSize& size,
			   long style);
	virtual ~MyTreeCtrl();

	void OnBeginDrag(wxTreeEvent& event);
	void OnBeginRDrag(wxTreeEvent& event);
	void OnEndDrag(wxTreeEvent& event);
	void OnBeginLabelEdit(wxTreeEvent& event);
	void OnEndLabelEdit(wxTreeEvent& event);
	void OnDeleteItem(wxTreeEvent& event);
	void OnGetInfo(wxTreeEvent& event);
	void OnSetInfo(wxTreeEvent& event);
	void OnItemExpanded(wxTreeEvent& event);
	void OnItemExpanding(wxTreeEvent& event);
	void OnItemCollapsed(wxTreeEvent& event);
	void OnItemCollapsing(wxTreeEvent& event);
	void OnSelChanged(wxTreeEvent& event);
	void OnSelChanging(wxTreeEvent& event);
	void OnTreeKeyDown(wxTreeEvent& event);
	void OnItemActivated(wxTreeEvent& event);
	void OnRMouseDClick(wxMouseEvent& event);

	void CreateImageList(int size = 16);

	wxTreeItemId AddRootItem(int image, const char *text);
	void RefreshTreeItems(vtFrame *pFrame);
	void RefreshTreeStatus(vtFrame *pFrame);

	void DoEnsureVisible() { EnsureVisible(m_lastItem); }

	void DoToggleIcon(const wxTreeItemId& item);

protected:

private:
	wxImageList *m_imageListNormal;
	wxTreeItemId m_lastItem;				// for OnEnsureVisible()
	bool		 m_bUpdating;

	DECLARE_EVENT_TABLE()
};

