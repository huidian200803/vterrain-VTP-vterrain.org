//
// Name: UtilDlg.h
//
// Copyright (c) 2004-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __UtilDlg_H__
#define __UtilDlg_H__

#include "EnviroUI.h"
#include "vtui/AutoDialog.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// UtilDlg
//----------------------------------------------------------------------------

class UtilDlg: public UtilDlgBase
{
public:
	// constructors and destructors
	UtilDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void OnInitDialog(wxInitDialogEvent& event);

	// WDR: method declarations for UtilDlg
	wxChoice *GetStructtype()  { return (wxChoice*) FindWindow( ID_STRUCTTYPE ); }

private:
	// WDR: member variable declarations for UtilDlg
	wxChoice *m_pChoice;
	int m_iType;

private:
	// WDR: handler declarations for UtilDlg
	void OnStructType( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __UtilDlg_H__

