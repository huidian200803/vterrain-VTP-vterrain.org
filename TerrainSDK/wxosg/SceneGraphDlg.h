//
// Name: SceneGraphDlg.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __SceneGraphDlg_H__
#define __SceneGraphDlg_H__

#include "wx/imaglist.h"
#include "wxosg_UI.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// SceneGraphDlg
//----------------------------------------------------------------------------

class SceneGraphDlg: public SceneGraphDlgBase
{
public:
	// constructors and destructors
	SceneGraphDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	~SceneGraphDlg();

	void RefreshTreeContents();

protected:
	// WDR: method declarations for SceneGraphDlg
	wxButton* GetZoomto()  { return (wxButton*) FindWindow( ID_ZOOMTO ); }
	wxCheckBox* GetEnabled()  { return (wxCheckBox*) FindWindow( ID_ENABLED ); }
	wxTreeCtrl* GetScenetree()  { return (wxTreeCtrl*) FindWindow( ID_SCENETREE ); }

	void CreateImageList(int size = 16);
	void AddNodeItemsRecursively(wxTreeItemId hParentItem, osg::Node *pNode, int depth);
	void AddEnginesRecursively(wxTreeItemId hParentItem, vtEngine *pEng, int depth);
private:
	// WDR: member variable declarations for SceneGraphDlg
	wxImageList *m_imageListNormal;
	wxButton	*m_pZoomTo;
	wxCheckBox  *m_pEnabled;
	wxTreeCtrl  *m_pTree;
	vtEngine	*m_pSelectedEngine;
	osg::Node	*m_pSelectedNode;

private:
	// WDR: handler declarations for SceneGraphDlg
	void OnInitDialog(wxInitDialogEvent& event);
	void OnRefresh( wxCommandEvent &event );
	void OnLog( wxCommandEvent &event );
	void OnZoomTo( wxCommandEvent &event );
	void OnEnabled( wxCommandEvent &event );
	void OnTreeSelChanged( wxTreeEvent &event );
	void OnChar(wxKeyEvent& event);

	bool m_bFirst;

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __SceneGraphDlg_H__

