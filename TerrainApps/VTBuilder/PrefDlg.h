//
// Name: PrefDlg.h
//
// Copyright (c) 2007-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __PrefDlg_H__
#define __PrefDlg_H__

#include "VTBuilder_UI.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// PrefDlg
//----------------------------------------------------------------------------

class PrefDlg: public PrefDlgBase
{
public:
	// constructors and destructors
	PrefDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for PrefDlg
	void UpdateEnable();

public:
	// WDR: member variable declarations for PrefDlg
	bool b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15;
	int i1, i2, i3, i4;

private:
	// WDR: handler declarations for PrefDlg
	void OnInitDialog(wxInitDialogEvent& event);
	void OnCheck( wxCommandEvent &event );
	void OnRadio( wxCommandEvent &event );
	void OnOK( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif //  __PrefDlg_H__
