//
// Name: SampleImageDlg.cpp
//
// Copyright (c) 2003-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/FileFilters.h"
#include "vtui/AutoDialog.h"
#include "vtui/Helper.h"

#include "SampleImageDlg.h"
#include "BuilderView.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// SampleImageDlg
//----------------------------------------------------------------------------

// WDR: event table for SampleImageDlg

BEGIN_EVENT_TABLE(SampleImageDlg, SampleImageDlgBase)
	EVT_INIT_DIALOG (SampleImageDlg::OnInitDialog)
	EVT_BUTTON( ID_SMALLER, SampleImageDlg::OnSmaller )
	EVT_BUTTON( ID_BIGGER, SampleImageDlg::OnBigger )
	EVT_CHECKBOX( ID_CONSTRAIN, SampleImageDlg::OnConstrain )
	EVT_TEXT( ID_SIZEX, SampleImageDlg::OnSizeXY )
	EVT_TEXT( ID_SIZEY, SampleImageDlg::OnSizeXY )
	EVT_TEXT( ID_SPACINGX, SampleImageDlg::OnSpacingXY )
	EVT_TEXT( ID_SPACINGY, SampleImageDlg::OnSpacingXY )
	EVT_RADIOBUTTON( ID_RADIO_CREATE_NEW, SampleImageDlg::OnRadioOutput )
	EVT_RADIOBUTTON( ID_RADIO_TO_FILE, SampleImageDlg::OnRadioOutput )
	EVT_BUTTON( ID_DOTDOTDOT, SampleImageDlg::OnDotDotDot )
END_EVENT_TABLE()

SampleImageDlg::SampleImageDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	SampleImageDlgBase( parent, id, title, position, size, style )
{
	m_power = 8;
	m_bConstraint = false;
	m_bSetting = false;

	m_bNewLayer = true;
	m_bToFile = false;

	// output options
	AddValidator(this, ID_RADIO_CREATE_NEW, &m_bNewLayer);
	AddValidator(this, ID_RADIO_TO_FILE, &m_bToFile);

	AddValidator(this, ID_TEXT_TO_FILE, &m_strToFile);

	// sampling
	AddNumValidator(this, ID_SPACINGX, &m_fSpacingX);
	AddNumValidator(this, ID_SPACINGY, &m_fSpacingY);
	AddNumValidator(this, ID_SIZEX, &m_Size.x);
	AddNumValidator(this, ID_SIZEY, &m_Size.y);
	AddValidator(this, ID_CONSTRAIN, &m_bConstraint);

	// informations
	AddNumValidator(this, ID_AREAX, &m_fAreaX);
	AddNumValidator(this, ID_AREAY, &m_fAreaY);

	AddNumValidator(this, ID_ESTX, &m_fEstX);
	AddNumValidator(this, ID_ESTY, &m_fEstY);

	GetSizer()->SetSizeHints(this);
}

// WDR: handler implementations for SampleImageDlg

void SampleImageDlg::OnInitDialog(wxInitDialogEvent& event)
{
	m_fAreaX = m_area.Width();
	m_fAreaY = m_area.Height();

	// initial value: based on estimate spacing
	m_fSpacingX = m_fEstX;
	m_fSpacingY = m_fEstY;

	// don't just truncate, round slightly to avoid precision issues
	m_Size.x = (int) (m_fAreaX / m_fSpacingX + 0.5);
	m_Size.y = (int) (m_fAreaY / m_fSpacingY + 0.5);

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;

	EnableBasedOnConstraint();
}

void SampleImageDlg::RecomputeSize()
{
	if (m_bConstraint)  // powers of 2 + 1
		m_Size.x = m_Size.y = (1 << m_power);

	m_fSpacingX = m_fAreaX / m_Size.x;
	m_fSpacingY = m_fAreaY / m_Size.y;
}

// WDR: handler implementations for SampleImageDlg

void SampleImageDlg::OnSpacingXY( wxCommandEvent &event )
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

void SampleImageDlg::OnSizeXY( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	RecomputeSize();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void SampleImageDlg::OnConstrain( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	if (m_bConstraint)
	{
		// round up to a value at least as great as the current size
		m_power = 1;
		while (((1 << m_power) + 1) < m_Size.x ||
			   ((1 << m_power) + 1) < m_Size.y)
			m_power++;
	}
	RecomputeSize();
	EnableBasedOnConstraint();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void SampleImageDlg::EnableBasedOnConstraint()
{
	GetTextToFile()->Enable(m_bToFile);
	GetDotDotDot()->Enable(m_bToFile);

	GetConstrain()->Enable(true);
	GetSmaller()->Enable(m_bConstraint);
	GetBigger()->Enable(m_bConstraint);

	GetSizeX()->SetEditable(!m_bConstraint);
	GetSizeY()->SetEditable(!m_bConstraint);
	GetSpacingX()->SetEditable(!m_bConstraint);
	GetSpacingY()->SetEditable(!m_bConstraint);
}

void SampleImageDlg::OnBigger( wxCommandEvent &event )
{
	m_power++;
	RecomputeSize();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void SampleImageDlg::OnSmaller( wxCommandEvent &event )
{
	m_power--;
	RecomputeSize();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void SampleImageDlg::OnRadioOutput( wxCommandEvent &event )
{
	TransferDataFromWindow();
	EnableBasedOnConstraint();
	RecomputeSize();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void SampleImageDlg::OnDotDotDot( wxCommandEvent &event )
{
	wxString filter = _("All Files|*.*");
	AddType(filter, FSTRING_TIF);
	AddType(filter, FSTRING_JPEG);
	AddType(filter, FSTRING_ECW);

	// ask the user for a filename
	wxFileDialog saveFile(NULL, _("Save Imagery"), _T(""), _T(""), filter, wxFD_SAVE);
	saveFile.SetFilterIndex(0);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	m_strToFile = saveFile.GetPath();

	// update controls
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}


