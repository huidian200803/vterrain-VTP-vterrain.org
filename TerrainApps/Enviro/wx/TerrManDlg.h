//
// Name: TerrManDlg.h
//
// Copyright (c) 2003-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __TerrManDlg_H__
#define __TerrManDlg_H__

#include "EnviroUI.h"

// WDR: class declarations

//---------------------------------------------------------------------------
// TerrainManagerDlg
//---------------------------------------------------------------------------

class TerrainManagerDlg: public TerrManDlgBase
{
public:
	// constructors and destructors
	TerrainManagerDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for TerrainManagerDlg
	wxButton* GetCopy()  { return (wxButton*) FindWindow( ID_COPY ); }
	wxButton* GetEditParams()  { return (wxButton*) FindWindow( ID_EDIT_PARAMS ); }
	wxButton* GetDelete()  { return (wxButton*) FindWindow( ID_DELETE ); }
	wxButton* GetAddTerrain()  { return (wxButton*) FindWindow( ID_ADD_TERRAIN ); }
	wxTreeCtrl* GetTree()  { return (wxTreeCtrl*) FindWindow( ID_TREECTRL ); }
	void RefreshTreeContents();
	void RefreshTreeText();
	void UpdateEnabling();
	wxString GetCurrentPath();
	wxString GetCurrentTerrainPath();

private:
	// WDR: member variable declarations for TerrainManagerDlg
	wxTreeCtrl *m_pTree;
	int m_iSelect;
	wxTreeItemId m_Root;
	wxTreeItemId m_Selected;

private:
	// WDR: handler declarations for TerrainManagerDlg
	void OnCopy( wxCommandEvent &event );
	void OnEditParams( wxCommandEvent &event );
	void OnDelete( wxCommandEvent &event );
	void OnAddTerrain( wxCommandEvent &event );
	void OnAddPath( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);
	void OnDeleteItem( wxTreeEvent &event );
	void OnSelChanged( wxTreeEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif

