//
// Name: ExtentDlg.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __ExtentDlg_H__
#define __ExtentDlg_H__

#include "VTBuilder_UI.h"
#include "vtdata/MathTypes.h"

// WDR: class declarations

//---------------------------------------------------------------------------
// ExtentDlg
//---------------------------------------------------------------------------

class ExtentDlg: public ExtentDlgBase
{
public:
	// constructors and destructors
	ExtentDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for ExtentDlg
	wxCheckBox* GetDMS()  { return (wxCheckBox*) FindWindow( ID_DMS ); }
	void SetArea(DRECT area, bool bMeters);
	void FormatExtent(wxString &str, double value);
	double GetValueFrom(const wxString &str);

	wxString   m_strAll;
	wxString   m_strEast;
	wxString   m_strNorth;
	wxString   m_strSouth;
	wxString   m_strWest;

	DRECT m_area;
	bool m_bDegrees;
	bool m_bSetting;
	bool m_bDMS;	// Degrees Minutes Seconds
	wxString m_fs;  // Format string depends on coordinate scheme

private:
	// WDR: member variable declarations for ExtentDlg

private:
	// WDR: handler declarations for ExtentDlg
	void OnExtentAll( wxCommandEvent &event );
	void OnExtentS( wxCommandEvent &event );
	void OnExtentE( wxCommandEvent &event );
	void OnExtentW( wxCommandEvent &event );
	void OnExtentN( wxCommandEvent &event );
	void OnDMS( wxCommandEvent &event );

	void FormatStrings(int which);
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __ExtentDlg_H__

