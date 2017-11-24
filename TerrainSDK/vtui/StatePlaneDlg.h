//
// Name: StatePlaneDlg.h
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __StatePlaneDlg_H__
#define __StatePlaneDlg_H__

#include "vtui_UI.h"
#include "AutoDialog.h"
#include "vtdata/vtCRS.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// StatePlaneDlg
//----------------------------------------------------------------------------

class StatePlaneDlg: public StatePlaneDlgBase
{
public:
	// constructors and destructors
	StatePlaneDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for StatePlaneDlg
	wxListBox* GetStatePlanes()  { return (wxListBox*) FindWindow( ID_STATEPLANES ); }
	wxRadioButton* GetNad27()  { return (wxRadioButton*) FindWindow( ID_NAD27 ); }
	wxRadioButton* GetNad83()  { return (wxRadioButton*) FindWindow( ID_NAD83 ); }
	wxRadioButton* GetFeetUs()  { return (wxRadioButton*) FindWindow( ID_RADIO_FEET_US ); }

public:
	// WDR: member variable declarations for StatePlaneDlg
	int m_iStatePlane;
	bool m_bNAD27;
	bool m_bNAD83;
	bool m_bMeters;
	bool m_bFeet;
	bool m_bFeetUS;
	bool m_bSetting;

private:
	// WDR: handler declarations for StatePlaneDlg
	void OnNad83( wxCommandEvent &event );
	void OnNad27( wxCommandEvent &event );
	void OnListBox( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __StatePlaneDlg_H__
