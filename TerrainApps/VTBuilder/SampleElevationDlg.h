//
// Name: SampleElevationDlg.h
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __SampleElevationDlg_H__
#define __SampleElevationDlg_H__

#include "VTBuilder_UI.h"
#include "vtdata/MathTypes.h"
#include "vtui/AutoDialog.h"
#include "ElevDrawOptions.h"

class BuilderView;

// WDR: class declarations

//----------------------------------------------------------------------------
// SampleElevationDlg
//----------------------------------------------------------------------------

class SampleElevationDlg: public SampleElevationDlgBase
{
public:
	// constructors and destructors
	SampleElevationDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for SampleElevationDlg
	wxButton* GetDotDotDot()  { return (wxButton*) FindWindow( ID_DOTDOTDOT ); }
	wxTextCtrl* GetTextToFile()  { return (wxTextCtrl*) FindWindow( ID_TEXT_TO_FILE ); }
	wxRadioButton* GetRadioToFile()  { return (wxRadioButton*) FindWindow( ID_RADIO_TO_FILE ); }
	wxRadioButton* GetRadioCreateNew()  { return (wxRadioButton*) FindWindow( ID_RADIO_CREATE_NEW ); }
	wxRadioButton* GetShorts()  { return (wxRadioButton*) FindWindow( ID_SHORTS ); }
	wxTextCtrl* GetVUnits()  { return (wxTextCtrl*) FindWindow( ID_VUNITS ); }
	wxTextCtrl* GetSpacingY()  { return (wxTextCtrl*) FindWindow( ID_SPACINGY ); }
	wxTextCtrl* GetSpacingX()  { return (wxTextCtrl*) FindWindow( ID_SPACINGX ); }
	wxTextCtrl* GetSizeY()  { return (wxTextCtrl*) FindWindow( ID_SIZEY ); }
	wxTextCtrl* GetSizeX()  { return (wxTextCtrl*) FindWindow( ID_SIZEX ); }
	wxButton* GetBigger()  { return (wxButton*) FindWindow( ID_BIGGER ); }
	wxButton* GetSmaller()  { return (wxButton*) FindWindow( ID_SMALLER ); }
	wxCheckBox* GetConstrain()  { return (wxCheckBox*) FindWindow( ID_CONSTRAIN ); }

	void SetView(BuilderView *pView) { m_pView = pView; }
	void RecomputeSize();
	void EnableBasedOnConstraint();

	bool m_bNewLayer;
	bool m_bToFile;
	wxString m_strToFile;

	double  m_fSpacingX;
	double  m_fSpacingY;
	IPoint2 m_Size;
	bool	m_bConstraint;

	double  m_fAreaX;
	double  m_fAreaY;
	double  m_fEstX;
	double  m_fEstY;

	bool	m_bFloats;
	float   m_fVUnits;

	DRECT   m_area;
	bool	m_bFillGaps;

private:
	// WDR: member variable declarations for SampleElevationDlg
	wxNumericValidator *spacing1, *spacing2;
	BuilderView *m_pView;
	int		m_power;
	bool	m_bSetting;

private:
	// WDR: handler declarations for SampleElevationDlg
	void OnDotDotDot( wxCommandEvent &event );
	void OnRadioOutput( wxCommandEvent &event );
	void OnShorts( wxCommandEvent &event );
	void OnFloats( wxCommandEvent &event );
	void OnSpacingXY( wxCommandEvent &event );
	void OnSizeXY( wxCommandEvent &event );
	void OnConstrain( wxCommandEvent &event );
	void OnBigger( wxCommandEvent &event );
	void OnSmaller( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __SampleElevationDlg_H__

