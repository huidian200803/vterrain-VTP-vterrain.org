//
// Name:     TagDlg.h
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __TagDlg_H__
#define __TagDlg_H__

#include "CManager_UI.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// TagDlg
//----------------------------------------------------------------------------

class TagDlg: public TagDlgBase
{
public:
	// constructors and destructors
	TagDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for TagDlg
	wxChoice *GetTagName() { return (wxChoice *) FindWindow(ID_TAGNAME); }
	wxTextCtrl *GetTagText() { return (wxTextCtrl *) FindWindow(ID_TAGTEXT); }

	wxString m_strName;
	wxString m_strValue;

private:
	// WDR: member variable declarations for TagDlg

private:
	// WDR: handler declarations for TagDlg
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __TagDlg_H__
