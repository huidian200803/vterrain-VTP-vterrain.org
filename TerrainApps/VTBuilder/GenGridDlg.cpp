//
// Name: GenGridDlg.cpp
//
// Copyright (c) 2007-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
	#pragma implementation "GenGridDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "GenGridDlg.h"
#include "vtui/AutoDialog.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// GenGridDlg
//----------------------------------------------------------------------------

// WDR: event table for GenGridDlg

BEGIN_EVENT_TABLE(GenGridDlg,GenGridDlgBase)
	EVT_TEXT( ID_SPACINGX, GenGridDlg::OnSpacingXY )
	EVT_TEXT( ID_SPACINGY, GenGridDlg::OnSpacingXY )
	EVT_TEXT( ID_SIZEY, GenGridDlg::OnSizeXY )
	EVT_TEXT( ID_SIZEY, GenGridDlg::OnSizeXY )
END_EVENT_TABLE()

GenGridDlg::GenGridDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	bool bIsGeo, const wxPoint &position, const wxSize& size, long style ) :
	GenGridDlgBase( parent, id, title, position, size, style )
{
	m_bSetting = false;

	// sampling
	int digits;
	if (bIsGeo)
		digits = 9;
	else
		digits = 3;
	AddNumValidator(this, ID_SPACINGX, &m_fSpacingX, digits);
	AddNumValidator(this, ID_SPACINGY, &m_fSpacingY, digits);
	AddNumValidator(this, ID_SIZEX, &m_Size.x);
	AddNumValidator(this, ID_SIZEY, &m_Size.y);
	AddNumValidator(this, ID_TEXT_DIST_CUTOFF, &m_fDistanceCutoff);

	GetSizer()->SetSizeHints(this);
}

void GenGridDlg::RecomputeSize()
{
	m_fSpacingX = m_fAreaX / m_Size.x;
	m_fSpacingY = m_fAreaY / m_Size.y;
}


// WDR: handler implementations for GenGridDlg

void GenGridDlg::OnSizeXY( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	RecomputeSize();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void GenGridDlg::OnSpacingXY( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	m_Size.x = (int) (m_fAreaX / m_fSpacingX);
	m_Size.y = (int) (m_fAreaY / m_fSpacingY);

	m_bSetting = true;
	GetSizeX()->GetValidator()->TransferToWindow();
	GetSizeY()->GetValidator()->TransferToWindow();
	m_bSetting = false;
}


