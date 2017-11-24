//
// Name: GeocodeDlg.h
//
// Copyright (c) 2005-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __GeocodeDlg_H__
#define __GeocodeDlg_H__

#include "VTBuilder_UI.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// GeocodeDlg
//----------------------------------------------------------------------------

class GeocodeDlg: public GeocodeDlgBase
{
public:
	// constructors and destructors
	GeocodeDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	bool m_bGeocodeUS;
	bool m_bGazetteer;
	bool m_bGNS;

	wxString m_strData;
	wxString m_strGaz;
	wxString m_strZip;
	wxString m_strGNS;

	// WDR: method declarations for GeocodeDlg
	wxButton* GetGetFileZip()  { return (wxButton*) FindWindow( ID_GET_FILE_ZIP ); }
	wxTextCtrl* GetFileZip()  { return (wxTextCtrl*) FindWindow( ID_FILE_ZIP ); }
	wxTextCtrl* GetFileGns()  { return (wxTextCtrl*) FindWindow( ID_FILE_GNS ); }
	wxTextCtrl* GetFileGaz()  { return (wxTextCtrl*) FindWindow( ID_FILE_GAZ ); }
	wxCheckBox* GetCheckUse3()  { return (wxCheckBox*) FindWindow( ID_CHECK_USE3 ); }
	wxCheckBox* GetCheckUse2()  { return (wxCheckBox*) FindWindow( ID_CHECK_USE2 ); }
	wxCheckBox* GetCheckUse1()  { return (wxCheckBox*) FindWindow( ID_CHECK_USE1 ); }
	wxTextCtrl* GetFileData()  { return (wxTextCtrl*) FindWindow( ID_FILE_DATA ); }

private:
	// WDR: member variable declarations for GeocodeDlg

private:
	// WDR: handler declarations for GeocodeDlg
	void OnGetFileZip( wxCommandEvent &event );
	void OnGetFileGNS( wxCommandEvent &event );
	void OnGetFileGaz( wxCommandEvent &event );
	void OnGetFileData( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};




#endif
