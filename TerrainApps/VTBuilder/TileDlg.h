//
// Name: TileDlg.h
//
// Copyright (c) 2005-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __TileDlg_H__
#define __TileDlg_H__

#include "VTBuilder_UI.h"
#include "vtdata/ElevationGrid.h"
#include "TilingOptions.h"

class BuilderView;

// WDR: class declarations

//----------------------------------------------------------------------------
// TileDlg
//----------------------------------------------------------------------------

class TileDlg: public TileDlgBase
{
public:
	// constructors and destructors
	TileDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for TileDlg
	wxChoice* GetChoiceLod0Size()  { return (wxChoice*) FindWindow( ID_CHOICE_LOD0_SIZE ); }

	void SetTilingOptions(TilingOptions &opt);
	void GetTilingOptions(TilingOptions &opt) const;
	void SetArea(const DRECT &area);
	void UpdateInfo();
	void SetElevation(bool bElev);
	void SetView(BuilderView *pView) { m_pView = pView; }
	void UpdateEnables();

public:
	// WDR: member variable declarations for TileDlg
	wxString m_strToFile;
	int m_iColumns;
	int m_iRows;
	int m_iLODChoice;
	int m_iLOD0Size;
	int m_iNumLODs;

	bool m_bElev;

	int m_iTotalX;
	int m_iTotalY;
	double m_fAreaX;
	double m_fAreaY;
	double m_fEstX;
	double m_fEstY;
	double m_fCurX;
	double m_fCurY;

	bool m_bOmitFlatTiles;
	bool m_bMaskUnknown;
	bool m_bImageAlpha;
	bool m_bCompressNone;
	bool m_bCompressOGL;
	bool m_bCompressSquishFast;
	bool m_bCompressSquishSlow;
	bool m_bCompressJPEG;

	DRECT m_area;
	bool m_bSetting;
	BuilderView *m_pView;

private:
	// WDR: handler declarations for TileDlg
	void OnFilename( wxCommandEvent &event );
	void OnLODSize( wxCommandEvent &event );
	void OnSize( wxCommandEvent &event );
	void OnDotDotDot( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __TileDlg_H__

