//
// Name: RawDlg.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __RawDlg_H__
#define __RawDlg_H__

#include "vtdata/MathTypes.h"
#include "vtdata/vtCRS.h"

#include "VTBuilder_UI.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// RawDlg
//----------------------------------------------------------------------------

class RawDlg: public RawDlgBase
{
public:
	// constructors and destructors
	RawDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for RawDlg
	wxButton* GetCrs()  { return (wxButton*) FindWindow( ID_CRS ); }
	wxButton* GetExtents()  { return (wxButton*) FindWindow( ID_EXTENTS ); }
	wxTextCtrl* GetSpacing()  { return (wxTextCtrl*) FindWindow( ID_SPACING ); }
	void OnInitDialog(wxInitDialogEvent& event);

	int m_iBytes;
	int m_iWidth;
	int m_iHeight;
	float m_fVUnits;
	float m_fSpacing;
	bool m_bBigEndian;

	bool m_bExtSpacing;
	bool m_bExtExact;
	bool m_bCrsSimple;
	bool m_bCrsCurrent;
	bool m_bCrsExact;

	DRECT m_extents;
	vtCRS m_crs;
	vtCRS m_original;

private:
	// WDR: member variable declarations for RawDlg
	void UpdateEnabling();
	void UpdateExtents();
	void UpdateProjection();

private:
	// WDR: handler declarations for RawDlg
	void OnCRS( wxCommandEvent &event );
	void OnExtents( wxCommandEvent &event );
	void OnRadio( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif
