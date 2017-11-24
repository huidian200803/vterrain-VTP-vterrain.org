//
// Name: PropDlg.h
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __PropDlg_H__
#define __PropDlg_H__

#include "CManager_UI.h"
#include "vtdata/vtString.h"

class vtItem;
class vtModel;

// WDR: class declarations

//----------------------------------------------------------------------------
// PropPanel
//----------------------------------------------------------------------------

class PropPanel: public PropPanelBase
{
public:
	// constructors and destructors
	PropPanel( wxWindow *parent, wxWindowID id = -1,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxTAB_TRAVERSAL | wxNO_BORDER );

	// WDR: method declarations for PropDlg
	wxListCtrl* GetTaglist()  { return (wxListCtrl*) FindWindow( ID_TAGLIST ); }
	wxChoice* GetTypeChoice()  { return (wxChoice*) FindWindow( ID_TYPECHOICE ); }
	wxChoice* GetSubtypeChoice()  { return (wxChoice*) FindWindow( ID_SUBTYPECHOICE ); }
	void SetCurrentItem(vtItem *item);
	void UpdateFromControls();
	void UpdateTagList();
	void UpdateSubtypes();

private:
	// WDR: member variable declarations for PropDlg
	struct Pair { vtString type, subtype; };
	std::vector<Pair> m_types;

	wxString	m_strItem;
	wxString	m_strType, m_strSubtype;
	vtItem	  *m_pCurrentItem;
	wxListCtrl  *m_pTagList;
	wxChoice	*m_pTypeChoice;
	wxChoice	*m_pSubtypeChoice;
	bool		m_bUpdating;

private:
	// WDR: handler declarations for PropDlg
	void OnTagEdit( wxCommandEvent &event );
	void OnRemoveTag( wxCommandEvent &event );
	void OnAddTag( wxCommandEvent &event );
	void OnChoiceType( wxCommandEvent &event );
	void OnChoiceSubtype( wxCommandEvent &event );
	void OnTextItem( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __PropDlg_H__
