//
// Name: StartupDlg.h
//
// Copyright (c) 2001-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __StartupDlg_H__
#define __StartupDlg_H__

#include "EnviroUI.h"
#include "vtui/AutoDialog.h"
#include "../Options.h"

class EnviroOptions;

// WDR: class declarations

//----------------------------------------------------------------------------
// StartupDlg
//----------------------------------------------------------------------------

class StartupDlg: public StartupDlgBase
{
public:
	// constructors and destructors
	StartupDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void GetOptionsFrom(EnviroOptions &opt);
	void PutOptionsTo(EnviroOptions &opt);
	void UpdateState();
	void RefreshTerrainChoices();

private:
	// WDR: member variable declarations for StartupDlg
	EnviroOptions	m_opt;

	bool		m_bStartEarth;
	bool		m_bStartTerrain;
	wxString	m_strTName;
	wxString	m_strEarthImage;

private:
	// WDR: handler declarations for StartupDlg
	void OnInitDialog(wxInitDialogEvent& event);
	void OnTnameChoice( wxCommandEvent &event );
	void OnTerrMan( wxCommandEvent &event );
	void OnTerrain( wxCommandEvent &event );
	void OnEarthView( wxCommandEvent &event );
	void OnOpenGLInfo( wxCommandEvent &event );
	void OnOptions( wxCommandEvent &event );
	void OnEditProp( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __StartupDlg_H__

