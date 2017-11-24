//
// Name:		SelectDlg.h
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __SelectDlg_H__
#define __SelectDlg_H__

#include "VTBuilder_UI.h"

class vtRawLayer;

// WDR: class declarations

//----------------------------------------------------------------------------
// SelectDlg
//----------------------------------------------------------------------------

class SelectDlg: public SelectDlgBase
{
public:
	// constructors and destructors
	SelectDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for SelectDlg
	wxComboBox* GetComboValue()  { return (wxComboBox*) FindWindow( ID_COMBO_VALUE ); }
	wxChoice* GetCondition()  { return (wxChoice*) FindWindow( ID_CONDITION ); }
	wxListBox* GetField()  { return (wxListBox*) FindWindow( ID_FIELD ); }
	void FillValuesControl();
	void SetRawLayer(vtRawLayer *pRL);

	int			m_iField;
	int			m_iCondition;
	wxString	m_strValue;

private:
	// WDR: member variable declarations for SelectDlg
	vtRawLayer	*m_pLayer;
	bool		m_bSetting;
	int			m_iFauxFields;

private:
	// WDR: handler declarations for SelectDlg
	void OnChoiceField( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);
	void OnOK( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};




#endif
