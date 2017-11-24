//
// Name:		ContourDlg.h
//
// Copyright (c) 2009-2011 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#ifndef __ContourDlg_H__
#define __ContourDlg_H__

#include "vtui_UI.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// ContourDlg
//----------------------------------------------------------------------------

class ContourDlg: public ContourDlgBase
{
public:
	// constructors and destructors
	ContourDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for ContourDlg
	wxChoice *LayerChoice() { return m_choice_layer; }
	void UpdateEnabling();

	float m_fElevSingle;
	float m_fElevEvery;
	bool m_bSingle;
	bool m_bCreate;
	wxString m_strLayer;

private:
	// WDR: member variable declarations for ContourDlg

private:
	// WDR: handler declarations for ContourDlg
	void OnRadio( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __ContourDlg_H__

