//
// HeightDlg.h
//
// Copyright (c) 2002-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __HeightDlg_H__
#define __HeightDlg_H__

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "vtui_UI.h"
#include "vtui/AutoDialog.h"
#include "vtui/wxString2.h"
#include "vtdata/Building.h"
#include "vtdata/HeightField.h"


// WDR: class declarations

//---------------------------------------------------------------------------
// CHeightDialog
//---------------------------------------------------------------------------

class CHeightDialog: public AutoDialog
{
public:
	// constructors and destructors
	CHeightDialog( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void Setup(vtBuilding * const pBuilding, vtHeightField *pHeightField);
	void OnLeftClickGrid( wxGridEvent &event );
	void OnGridCellChange( wxGridEvent &event );

	// WDR: method declarations for CHeightDialog
	wxGrid* GetHeightgrid()  { return (wxGrid*) FindWindow( ID_HEIGHTGRID ); }
	wxTextCtrl* GetBaselineoffset()  { return (wxTextCtrl*) FindWindow( ID_BASELINEOFFSET ); }

protected:
	// WDR: member variable declarations for CHeightDialog

protected:
	void ValidateGrid();
	// WDR: handler declarations for CHeightDialog
	void OnCancel( wxCommandEvent &event );
	void OnBaselineOffset( wxCommandEvent &event );
	void OnOK( wxCommandEvent &event );
	void OnRecalculateHeights( wxCommandEvent &event );
	void OnClose(wxCloseEvent& event);

private:
	enum {BASELINE_COL = 0, RELATIVE_COL, ACTIVE_COL, STOREYS_COL, HEIGHT_COL};
	enum {SCALE_EVENLY = 0, SCALE_FROM_BOTTOM, SCALE_FROM_TOP};
	bool m_bGridModified;
	wxGrid *m_pHeightGrid;
	vtBuilding *m_pBuilding;
	vtHeightField *m_pHeightField;
	wxTextCtrl* m_pBaselineOffset;
	float m_fBaselineOffset;
	int m_BottomRow;
	int m_NumLevels;
	double m_dBaseLine;
	vtBuilding m_OldBuilding;
	DECLARE_EVENT_TABLE()
};




#endif
