//
// HeightDlg.cpp
//
// Copyright (c) 2002-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#include "HeightDlg.h"
#include "BuildingDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// CHeightDialog
//----------------------------------------------------------------------------

// WDR: event table for CHeightDialog

BEGIN_EVENT_TABLE(CHeightDialog, AutoDialog)
	EVT_BUTTON( ID_RECALCULATEHEIGHTS, CHeightDialog::OnRecalculateHeights )
	EVT_BUTTON( wxID_OK, CHeightDialog::OnOK )
	EVT_TEXT( ID_BASELINEOFFSET, CHeightDialog::OnBaselineOffset )
	EVT_GRID_CELL_LEFT_CLICK( CHeightDialog::OnLeftClickGrid )
	EVT_GRID_CELL_CHANGE( CHeightDialog::OnGridCellChange )
	EVT_CLOSE( CHeightDialog::OnClose )
	EVT_BUTTON( wxID_CANCEL, CHeightDialog::OnCancel )
END_EVENT_TABLE()

CHeightDialog::CHeightDialog( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	HeightDialogFunc( this, TRUE );

	// TODO? --- find an easy way of adding a KILL_FOCUS handler to this ctrl
	AddNumValidator(this, ID_BASELINEOFFSET, &m_fBaselineOffset);
}

void CHeightDialog::Setup(vtBuilding  * const pBuilding, vtHeightField *pHeightField)
{
	int i;
	double dBaseline = pBuilding->CalculateBaseElevation(pHeightField);
	wxGridCellFloatEditor GCFE;

	m_bGridModified = false;

	m_pBuilding = pBuilding;
	m_OldBuilding = *pBuilding;
	m_pHeightField = pHeightField;
	m_pHeightGrid = GetHeightgrid();
	m_pBaselineOffset = GetBaselineoffset();

	// Set up the baseline offset
	m_fBaselineOffset = m_pBuilding->GetElevationOffset();

	m_NumLevels = m_pBuilding->GetNumLevels();
	m_BottomRow = m_NumLevels - 1;
	m_dBaseLine = dBaseline;

	for (i = 0; i < (m_NumLevels - 1); i++)
		dBaseline += m_pBuilding->GetLevel(i)->m_fStoryHeight * m_pBuilding->GetLevel(i)->m_iStories;

	m_pHeightGrid->SetColLabelSize(40);

	// Set up the grid
	m_pHeightGrid->SetColLabelValue(BASELINE_COL, _T("Absolute\nBaseline"));
	m_pHeightGrid->SetColFormatFloat(BASELINE_COL);
	m_pHeightGrid->SetColLabelValue(RELATIVE_COL, _T("Relative\nBaseline"));
	m_pHeightGrid->SetColFormatFloat(RELATIVE_COL);
	m_pHeightGrid->SetColLabelValue(ACTIVE_COL, _T("Editable"));
	m_pHeightGrid->SetColFormatBool(ACTIVE_COL);
	m_pHeightGrid->SetColLabelValue(STOREYS_COL, _T("Storeys"));
	m_pHeightGrid->SetColFormatNumber(STOREYS_COL);
	m_pHeightGrid->SetColLabelValue(HEIGHT_COL, _T("Height"));
	m_pHeightGrid->SetColFormatFloat(HEIGHT_COL);
	m_pHeightGrid->AppendRows(m_NumLevels);

	wxString str;
	for (i = 0; i < m_NumLevels; i++)
	{
		int iCurrentLevel = (m_NumLevels - 1) - i;
		vtLevel  *pLevel = m_pBuilding->GetLevel(iCurrentLevel);
		float StoreyHeight = pLevel->m_fStoryHeight;
		int Storeys = pLevel->m_iStories;

		if (i != 0)
			dBaseline -= StoreyHeight * Storeys;

		str.Printf(_T("%d"), iCurrentLevel);
		m_pHeightGrid->SetRowLabelValue(i, str);

		str.Printf(_T("%f"), dBaseline);
		m_pHeightGrid->SetCellValue(i, BASELINE_COL, str);
		m_pHeightGrid->SetCellEditor(i, BASELINE_COL, GCFE.Clone());
		str.Printf(_T("%f"), dBaseline - m_dBaseLine);
		m_pHeightGrid->SetCellValue(i, RELATIVE_COL, str);
		m_pHeightGrid->SetCellEditor(i, RELATIVE_COL, GCFE.Clone());
		m_pHeightGrid->SetReadOnly(i, ACTIVE_COL);
		m_pHeightGrid->SetReadOnly(i, BASELINE_COL);
		m_pHeightGrid->SetCellValue(i, ACTIVE_COL, _T("0"));

		str.Printf(_T("%d"), Storeys);
		m_pHeightGrid->SetReadOnly(i, STOREYS_COL);
		m_pHeightGrid->SetCellValue(i, STOREYS_COL, str);

		str.Printf(_T("%f"), StoreyHeight);
		m_pHeightGrid->SetReadOnly(i, HEIGHT_COL);
		m_pHeightGrid->SetCellValue(i, HEIGHT_COL, str);
	}
}

void CHeightDialog::OnLeftClickGrid( wxGridEvent &event )
{
	int iRow = event.GetRow();
	int iCol = event.GetCol();
	long lValue;
	// Check to if user has clicked on "locked column"
	// but not on bottom row
	if ((iCol == ACTIVE_COL) && (iRow != m_BottomRow))
	{
		bool bValidate = !m_pHeightGrid->IsCellEditControlEnabled();

		m_bGridModified = true;

		m_pHeightGrid->GetCellValue(iRow, ACTIVE_COL).ToLong(&lValue);

		if (lValue)
			m_pHeightGrid->SetCellValue(iRow, ACTIVE_COL, _T("0"));
		else
			m_pHeightGrid->SetCellValue(iRow, ACTIVE_COL, _T("1"));

		// Force a move cursor before updating the read only attribute
		// this avoids orphaned cell editors
		m_pHeightGrid->SetGridCursor(iRow, iCol);

		if (lValue)
			m_pHeightGrid->SetReadOnly(iRow, BASELINE_COL);
		else
			m_pHeightGrid->SetReadOnly(iRow, BASELINE_COL, false);

		if (bValidate)
			ValidateGrid();
	}
	event.Skip();
}

void CHeightDialog::OnGridCellChange( wxGridEvent &event )
{
	m_bGridModified = true;
	double dNewValue;
	wxString Str;
	int iCurrentRow = event.GetRow();

	switch(event.GetCol())
	{
		case BASELINE_COL:
			m_pHeightGrid->GetCellValue(iCurrentRow, BASELINE_COL).ToDouble(&dNewValue);
			Str.Printf(_T("%f"), dNewValue - m_dBaseLine);
			m_pHeightGrid->SetCellValue(iCurrentRow, RELATIVE_COL, Str);
			break;
		case RELATIVE_COL:
			m_pHeightGrid->GetCellValue(iCurrentRow, RELATIVE_COL).ToDouble(&dNewValue);
			Str.Printf(_T("%f"), dNewValue + m_dBaseLine);
			m_pHeightGrid->SetCellValue(iCurrentRow, BASELINE_COL, Str);
			break;
	}
	ValidateGrid();
	event.Skip();
}


void CHeightDialog::ValidateGrid()
{
	double dBaseLine =  m_dBaseLine;

	// Validate the storey heights
	for (int i = 1; i < m_NumLevels; i++)
	{
		long lValue;
		int iCurrentRow = m_NumLevels - 1 - i;

		if (iCurrentRow != m_BottomRow)
			m_pHeightGrid->GetCellValue(iCurrentRow, ACTIVE_COL).ToLong(&lValue);
		else
			lValue = 1;
		if (lValue)
		{
			double dNewBaseLine;

			m_pHeightGrid->GetCellValue(iCurrentRow, BASELINE_COL).ToDouble(&dNewBaseLine);
			if (dNewBaseLine < dBaseLine)
			{
				wxMessageBox(_T("Active baselines must be greater or equal to next lower active baseline"),
							_T("Error"),
							wxOK|wxICON_ERROR,
							this);
			}
			dBaseLine = dNewBaseLine;
		}
	}
}

// WDR: handler implementations for CHeightDialog

void CHeightDialog::OnCancel( wxCommandEvent &event )
{
	*m_pBuilding = m_OldBuilding;
	m_pBuilding->DetermineLocalFootprints();
	((BuildingDlg*)GetParent())->Modified();
	//Pass on to base class
	event.Skip();
}

void CHeightDialog::OnBaselineOffset( wxCommandEvent &event )
{
	float fOldOffset = m_fBaselineOffset;
	double dAdjustment;
	double dBaseline;
	int i;
	wxString str;

	TransferDataFromWindow();
	m_pBuilding->SetElevationOffset(m_fBaselineOffset);
	dAdjustment = m_fBaselineOffset - fOldOffset;

	for (i = 0; i < m_NumLevels; i++)
	{
		m_pHeightGrid->GetCellValue(i, BASELINE_COL).ToDouble(&dBaseline);
		dBaseline += dAdjustment;
		str.Printf(_T("%f"), dBaseline);
		m_pHeightGrid->SetCellValue(i, BASELINE_COL, str);
	}
	((BuildingDlg*)GetParent())->Modified();
}

void CHeightDialog::OnOK( wxCommandEvent &event )
{
	if (m_bGridModified)
		OnRecalculateHeights(event);

	// Passs on to base class
	event.Skip();
//	wxDialog::OnOK(event);
}

void CHeightDialog::OnRecalculateHeights( wxCommandEvent &event )
{
	int i, j;
	int iBaseLevel = 0;
	double dBaseLine =  m_dBaseLine;
	vtLevel *pLevel = m_pBuilding->GetLevel(0);
	float fCurrentHeight = pLevel->m_fStoryHeight * pLevel->m_iStories;
	wxString str;

	if (m_pHeightGrid->IsCellEditControlEnabled())
		m_pHeightGrid->DisableCellEditControl();
	else
		ValidateGrid();


	if (!m_bGridModified)
		return;

	// Recalculate the storey heights
	for (i = 1; i < m_NumLevels; i++)
	{
		long lValue;
		int iCurrentRow = m_NumLevels - 1 - i;

		pLevel = m_pBuilding->GetLevel(i);

		if (iCurrentRow != m_BottomRow)
			m_pHeightGrid->GetCellValue(iCurrentRow, ACTIVE_COL).ToLong(&lValue);
		else
			lValue = 1;
		if (lValue)
		{
			double dNewBaseLine;

			// Scale the intervening heights to match
			m_pHeightGrid->GetCellValue(iCurrentRow, BASELINE_COL).ToDouble(&dNewBaseLine);
			if (dNewBaseLine < dBaseLine)
			{
				// For the time being just give up
				wxMessageDialog(this,
								_T("Active baselines must be greater or equal to next lower active baseline"),
								_T("Error"),
								wxOK|wxICON_ERROR);
				return;
			}
			if (fCurrentHeight != (float)(dNewBaseLine - dBaseLine))
			{
				double dIntermediateBaseLine = dBaseLine;
				float fStoreyHeight;
				float fScaleFactor = (float)(dNewBaseLine - dBaseLine) / fCurrentHeight;

				for (j = iBaseLevel; j < i; j++)
				{
					fStoreyHeight = m_pBuilding->GetLevel(j)->m_fStoryHeight *= fScaleFactor;
					str.Printf(_T("%f"), fStoreyHeight);
					m_pHeightGrid->SetCellValue(m_NumLevels - 1 - j, HEIGHT_COL, str);
					dIntermediateBaseLine += fStoreyHeight * m_pBuilding->GetLevel(j)->m_iStories;
					if (j != (i - 1))
					{
						str.Printf(_T("%f"), dIntermediateBaseLine);
						m_pHeightGrid->SetCellValue(m_NumLevels - 2 - j, BASELINE_COL, str);
						str.Printf(_T("%f"), dIntermediateBaseLine - m_dBaseLine);
						m_pHeightGrid->SetCellValue(m_NumLevels - 2 - j, RELATIVE_COL, str);
					}
				}
			}
			dBaseLine = dNewBaseLine;
			iBaseLevel = i;
			fCurrentHeight = 0;
		}
		fCurrentHeight += pLevel->m_fStoryHeight * pLevel->m_iStories;
	}

	m_pBuilding->DetermineLocalFootprints();
	m_bGridModified = false;
	((BuildingDlg*)GetParent())->Modified();
}

void CHeightDialog::OnClose(wxCloseEvent& event)
{
	// User has dismiised the dialog via the system menu or the window manager
	// assume that this is a cancel and restore the original settings
	*m_pBuilding = m_OldBuilding;
	m_pBuilding->DetermineLocalFootprints();
	((BuildingDlg*)GetParent())->Modified();
	event.Skip();
}




