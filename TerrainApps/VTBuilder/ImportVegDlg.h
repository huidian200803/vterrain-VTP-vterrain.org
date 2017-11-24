//
// Name: ImportVegDlg.h
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __ImportVegDlg_H__
#define __ImportVegDlg_H__

#include "VTBuilder_UI.h"
#include "VegLayer.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// ImportVegDlg
//----------------------------------------------------------------------------

class ImportVegDlg: public ImportVegDlgBase
{
public:
	// constructors and destructors
	ImportVegDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// Methods
	void SetShapefileName(const wxString &filename);

	// Data
	int m_fieldindex;
	VegImportFieldType m_datatype;
	wxComboBox *m_pcbField;
	wxRadioButton *m_pDensity, *m_pBiotype1, *m_pBiotype2;
	wxString m_filename;

	// WDR: method declarations for ImportVegDlg
	wxRadioButton* GetDensity()  { return (wxRadioButton*) FindWindow( ID_DENSITY ); }
	wxRadioButton* GetBiotype2()  { return (wxRadioButton*) FindWindow( ID_BIOTYPE2 ); }
	wxRadioButton* GetBiotype1()  { return (wxRadioButton*) FindWindow( ID_BIOTYPE1 ); }
	wxComboBox* GetField()  { return (wxComboBox*) FindWindow( ID_FIELD ); }

private:
	// WDR: member variable declarations for ImportVegDlg
	void OnInitDialog(wxInitDialogEvent& event);

private:
	// WDR: handler declarations for ImportVegDlg
	void OnOK( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __ImportVegDlg_H__

