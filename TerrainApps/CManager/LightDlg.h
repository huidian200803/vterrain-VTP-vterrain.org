//
// Name: LightDlg.h
//
// Copyright (c) 2004-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __LightDlg_H__
#define __LightDlg_H__

#include "CManager_UI.h"

#if wxCHECK_VERSION(2, 9, 0)
  #include <wx/colourdata.h>
#else
  // The older include
  #include <wx/cmndata.h>
#endif

// WDR: class declarations

//----------------------------------------------------------------------------
// LightDlg
//----------------------------------------------------------------------------

class LightDlg: public LightDlgBase
{
public:
	// constructors and destructors
	LightDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void UseLight(vtTransform *pLight);
	void UpdateColorBitmaps();

	// WDR: method declarations for LightDlg
	wxTextCtrl* GetDirz()  { return (wxTextCtrl*) FindWindow( ID_DIRZ ); }
	wxTextCtrl* GetDiry()  { return (wxTextCtrl*) FindWindow( ID_DIRY ); }
	wxTextCtrl* GetDirx()  { return (wxTextCtrl*) FindWindow( ID_DIRX ); }
	wxBitmapButton* GetDiffuse()  { return (wxBitmapButton*) FindWindow( ID_DIFFUSE ); }
	wxBitmapButton* GetAmbient()  { return (wxBitmapButton*) FindWindow( ID_AMBIENT ); }
	wxChoice* GetLight()  { return (wxChoice*) FindWindow( ID_LIGHT ); }

private:
	// WDR: member variable declarations for LightDlg
	vtTransform *m_pMovLight;
	vtLightSource *m_pLight;
	FPoint3 m_dir;

	wxColour m_ambient, m_diffuse;
	wxColourData m_data;

private:
	// WDR: handler declarations for LightDlg
	void OnText( wxCommandEvent &event );
	void OnDiffuse( wxCommandEvent &event );
	void OnAmbient( wxCommandEvent &event );
	void OnLight( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};




#endif
