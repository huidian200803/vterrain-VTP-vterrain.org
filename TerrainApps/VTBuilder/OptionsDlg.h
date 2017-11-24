//
// Name: OptionsDlg.h
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __OptionsDlg_H__
#define __OptionsDlg_H__

#include "VTBuilder_UI.h"
#include "ElevDrawOptions.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// OptionsDlg
//----------------------------------------------------------------------------

class OptionsDlg: public OptionsDlgBase
{
public:
	// constructors and destructors
	OptionsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for OptionsDlg
	wxButton* GetRenderOptions()  { return (wxButton*) FindWindow( ID_RENDER_OPTIONS ); }
	wxRadioButton* GetRadioColor()  { return (wxRadioButton*) FindWindow( ID_RADIO_COLOR ); }
	wxRadioButton* GetRadioOutlineOnly()  { return (wxRadioButton*) FindWindow( ID_RADIO_OUTLINE_ONLY ); }
	wxChoice* GetElevUnit()  { return (wxChoice*) FindWindow( ID_ELEVUNIT ); }

	void SetElevDrawOptions(const ElevDrawOptions &opt);
	void GetElevDrawOptions(ElevDrawOptions &opt);

	bool m_bShowMinutes;
	int  m_iElevUnits;

	bool m_bShowOutlines;
	ElevDrawOptions m_opt;
	bool m_bHideUnknown;
	bool m_bDrawTinSimple;

	bool m_bShowRoadWidth;
	bool m_bDrawRawSimple;
	bool m_bShowPath;

private:
	// WDR: member variable declarations for OptionsDlg

private:
	void UpdateEnables();

	// WDR: handler declarations for OptionsDlg
	void OnRenderOptions( wxCommandEvent &event );
	void OnRadio( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __OptionsDlg_H__
