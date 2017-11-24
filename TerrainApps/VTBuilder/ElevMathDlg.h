//
// ElevMathDlg.h
//
// Copyright (c) 2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ELEV_MATH_DLG_H
#define ELEV_MATH_DLG_H

#include "VTBuilder_UI.h"
#include "ElevLayer.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// ElevMathDlg
//----------------------------------------------------------------------------

class ElevMathDlg: public ElevMathDlgBase
{
public:
	// constructors and destructors
	ElevMathDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	int m_iLayer1;
	int m_iLayer2;
	int m_iOperation;

	vtElevLayer *m_pLayer1;
	vtElevLayer *m_pLayer2;
	DRECT m_extent;
	DPoint2 m_spacing;
	IPoint2 m_grid_size;

	std::vector<vtElevLayer*> m_layers;
	bool m_bSetting;

protected:
	// WDR: method declarations for ElevMathDlg
	void Update();

private:
	// WDR: handler declarations for ElevMathDlg
	void OnChoice( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif	// ELEV_MATH_DLG_H
