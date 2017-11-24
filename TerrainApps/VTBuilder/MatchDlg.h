//
// Name: MatchDlg.h
//
// Copyright (c) 2007-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __MatchDlg_H__
#define __MatchDlg_H__

#include "VTBuilder_UI.h"
#include "vtdata/MathTypes.h"

class BuilderView;

// WDR: class declarations

//----------------------------------------------------------------------------
// MatchDlg
//----------------------------------------------------------------------------

class MatchDlg: public MatchDlgBase
{
public:
	// constructors and destructors
	MatchDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for MatchDlg
	wxCheckBox* GetShrink()  { return (wxCheckBox*) FindWindow( ID_SHRINK ); }
	wxCheckBox* GetGrow()  { return (wxCheckBox*) FindWindow( ID_GROW ); }
	void SetArea(const DRECT &area, bool bIsGeo);
	wxChoice* GetMatchLayer()  { return (wxChoice*) FindWindow( ID_MATCH_LAYER ); }
	void SetView(BuilderView *pView) { m_pView = pView; }

public:
	// WDR: member variable declarations for MatchDlg
	wxString m_strExtent1;
	wxString m_strExtent2;
	int m_iLayer;
	wxString m_strLayerRes;
	bool m_bGrow, m_bShrink;
	int m_iTileSize;
	wxString m_strTiling;

	bool m_bIsGeo;
	DPoint2 m_spacing;
	IPoint2 m_tile;
	DRECT m_area, m_original;
	wxString m_fs;  // Format string depends on coordinate scheme

	BuilderView *m_pView;

private:
	// WDR: handler declarations for MatchDlg
	void OnShrink( wxCommandEvent &event );
	void OnGrow( wxCommandEvent &event );
	void OnMatchLayer( wxCommandEvent &event );
	void OnSpinUp( wxSpinEvent &event );
	void OnSpinDown( wxSpinEvent &event );

	void UpdateValues();
	void UpdateLayers();
	void UpdateGuess();
	void GetLayerSpacing();

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __MatchDlg_H__

