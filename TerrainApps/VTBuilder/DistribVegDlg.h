//
// Name: DistribVegDlg.h
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __DistribVegDlg_H__
#define __DistribVegDlg_H__

#include "VTBuilder_UI.h"
#include "VegGenOptions.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// DistribVegDlg
//----------------------------------------------------------------------------

class DistribVegDlg: public DistribVegDlgBase
{
public:
	// constructors and destructors
	DistribVegDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	int m_iChoiceSpecies;
	int m_iChoiceBiotype;
	int m_iChoiceBiotypeLayer;
	int m_iChoiceDensityLayer;
	int m_iRandomFrom, m_iRandomTo;

	bool m_bSpecies1, m_bSpecies2, m_bSpecies3;
	bool m_bDensity1, m_bDensity2, m_bDensity3;
	bool m_bSize1, m_bSize2;

	VegGenOptions	m_opt;

	// WDR: method declarations for DistribVegDlg
	wxTextCtrl* GetTextFixedDensity()  { return (wxTextCtrl*) FindWindow( ID_TEXT_FIXED_DENSITY ); }
	wxTextCtrl* GetTextFixedSize()  { return (wxTextCtrl*) FindWindow( ID_TEXT_FIXED_SIZE ); }
	wxSpinCtrl* GetSpinRandomTo()  { return (wxSpinCtrl*) FindWindow( ID_SPIN_RANDOM_TO ); }
	wxSpinCtrl* GetSpinRandomFrom()  { return (wxSpinCtrl*) FindWindow( ID_SPIN_RANDOM_FROM ); }
	wxChoice* GetChoiceDensityLayer()  { return (wxChoice*) FindWindow( ID_CHOICE_DENSITY_LAYER ); }
	wxChoice* GetChoiceBiotypeLayer()  { return (wxChoice*) FindWindow( ID_CHOICE_BIOTYPE_LAYER ); }
	wxChoice* GetChoiceBiotype()  { return (wxChoice*) FindWindow( ID_CHOICE_BIOTYPE ); }
	wxChoice* GetChoiceSpecies()  { return (wxChoice*) FindWindow( ID_CHOICE_SPECIES ); }
	wxRadioButton* GetSize2()  { return (wxRadioButton*) FindWindow( ID_SIZE2 ); }
	wxRadioButton* GetSize1()  { return (wxRadioButton*) FindWindow( ID_SIZE1 ); }
	wxRadioButton* GetDensity3()  { return (wxRadioButton*) FindWindow( ID_DENSITY3 ); }
	wxRadioButton* GetDensity2()  { return (wxRadioButton*) FindWindow( ID_DENSITY2 ); }
	wxRadioButton* GetDensity1()  { return (wxRadioButton*) FindWindow( ID_DENSITY1 ); }
	wxRadioButton* GetSpecies3()  { return (wxRadioButton*) FindWindow( ID_SPECIES3 ); }
	wxRadioButton* GetSpecies2()  { return (wxRadioButton*) FindWindow( ID_SPECIES2 ); }
	wxRadioButton* GetSpecies1()  { return (wxRadioButton*) FindWindow( ID_SPECIES1 ); }

private:
	// WDR: member variable declarations for DistribVegDlg
	void UpdateEnabling();

private:
	// WDR: handler declarations for DistribVegDlg
	void OnInitDialog(wxInitDialogEvent& event);
	void OnRadio( wxCommandEvent &event );
	void OnOK( wxCommandEvent &event );

	bool TransferDataFromWindow();

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __DistribVegDlg_H__

