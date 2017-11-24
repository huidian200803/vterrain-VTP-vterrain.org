//
// Name: RenderOptionsDlg.h
//
// Copyright (c) 2006-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __RenderOptionsDlg_H__
#define __RenderOptionsDlg_H__

#include "VTBuilder_UI.h"

#include "ElevDrawOptions.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// RenderOptionsDlg
//----------------------------------------------------------------------------

class RenderOptionsDlg: public RenderOptionsDlgBase
{
public:
	// constructors and destructors
	RenderOptionsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for RenderOptionsDlg
	wxSpinCtrl* GetSpinCastDirection()  { return (wxSpinCtrl*) FindWindow( ID_SPIN_CAST_DIRECTION ); }
	wxSpinCtrl* GetSpinCastAngle()  { return (wxSpinCtrl*) FindWindow( ID_SPIN_CAST_ANGLE ); }
	wxCheckBox* GetCastShadows()  { return (wxCheckBox*) FindWindow( ID_CHECK_SHADOWS ); }
	wxRadioButton* GetRadioShadingDot()  { return (wxRadioButton*) FindWindow( ID_RADIO_SHADING_DOT ); }
	wxRadioButton* GetRadioShadingQuick()  { return (wxRadioButton*) FindWindow( ID_RADIO_SHADING_QUICK ); }
	wxRadioButton* GetRadioShadingNone()  { return (wxRadioButton*) FindWindow( ID_RADIO_SHADING_NONE ); }
	wxButton* GetEditColors()  { return (wxButton*) FindWindow( ID_EDIT_COLORS ); }
	wxChoice* GetChoiceColors()  { return (wxChoice*) FindWindow( ID_CHOICE_COLORS ); }

	void SetOptions(ElevDrawOptions &opt);
	void UpdateColorMapChoice();
	void UpdateEnables();

	ElevDrawOptions m_opt;
	wxString m_strColorMap;
	bool m_bNoShading;

private:
	// WDR: member variable declarations for RenderOptionsDlg

private:
	// WDR: handler declarations for RenderOptionsDlg
	void OnChoiceColors( wxCommandEvent &event );
	void OnRadio( wxCommandEvent &event );
	void OnEditColors( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __RenderOptionsDlg_H__

