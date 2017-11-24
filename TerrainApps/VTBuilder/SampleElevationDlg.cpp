//
// Name: SampleElevationDlg.cpp
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/FileFilters.h"
#include "vtui/AutoDialog.h"

#include "SampleElevationDlg.h"
#include "RenderOptionsDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// SampleElevationDlg
//----------------------------------------------------------------------------

// WDR: event table for SampleElevationDlg

BEGIN_EVENT_TABLE(SampleElevationDlg, SampleElevationDlgBase)
	EVT_INIT_DIALOG (SampleElevationDlg::OnInitDialog)
	EVT_BUTTON( ID_SMALLER, SampleElevationDlg::OnSmaller )
	EVT_BUTTON( ID_BIGGER, SampleElevationDlg::OnBigger )
	EVT_CHECKBOX( ID_CONSTRAIN, SampleElevationDlg::OnConstrain )
	EVT_TEXT( ID_SIZEX, SampleElevationDlg::OnSizeXY )
	EVT_TEXT( ID_SIZEY, SampleElevationDlg::OnSizeXY )
	EVT_TEXT( ID_SPACINGX, SampleElevationDlg::OnSpacingXY )
	EVT_TEXT( ID_SPACINGY, SampleElevationDlg::OnSpacingXY )
	EVT_RADIOBUTTON( ID_FLOATS, SampleElevationDlg::OnFloats )
	EVT_RADIOBUTTON( ID_SHORTS, SampleElevationDlg::OnShorts )
	EVT_RADIOBUTTON( ID_RADIO_CREATE_NEW, SampleElevationDlg::OnRadioOutput )
	EVT_RADIOBUTTON( ID_RADIO_TO_FILE, SampleElevationDlg::OnRadioOutput )
	EVT_BUTTON( ID_DOTDOTDOT, SampleElevationDlg::OnDotDotDot )
END_EVENT_TABLE()

SampleElevationDlg::SampleElevationDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	SampleElevationDlgBase( parent, id, title, position, size, style )
{
	m_bSetting = false;

	m_bNewLayer = true;
	m_bToFile = false;
	m_bFillGaps = true;

	// output options
	AddValidator(this, ID_RADIO_CREATE_NEW, &m_bNewLayer);
	AddValidator(this, ID_RADIO_TO_FILE, &m_bToFile);

	AddValidator(this, ID_TEXT_TO_FILE, &m_strToFile);

	// sampling
	spacing1 = AddNumValidator(this, ID_SPACINGX, &m_fSpacingX);
	spacing2 = AddNumValidator(this, ID_SPACINGY, &m_fSpacingY);
	AddNumValidator(this, ID_SIZEX, &m_Size.x);
	AddNumValidator(this, ID_SIZEY, &m_Size.y);
	AddValidator(this, ID_CONSTRAIN, &m_bConstraint);

	// output grid
	AddValidator(this, ID_FLOATS, &m_bFloats);
	AddNumValidator(this, ID_VUNITS, &m_fVUnits);
	AddValidator(this, ID_FILL_GAPS, &m_bFillGaps);

	// informations
	AddNumValidator(this, ID_AREAX, &m_fAreaX);
	AddNumValidator(this, ID_AREAY, &m_fAreaY);

	AddNumValidator(this, ID_ESTX, &m_fEstX);
	AddNumValidator(this, ID_ESTY, &m_fEstY);

	GetSizer()->SetSizeHints(this);
}

void SampleElevationDlg::OnInitDialog(wxInitDialogEvent& event)
{
	m_bNewLayer = true;
	m_bToFile = false;

	m_power = 8;
	m_bConstraint = false;
	m_fVUnits = 1.0f;

	m_fAreaX = m_area.Width();
	m_fAreaY = m_area.Height();

	// initial value: based on estimate spacing
	m_fSpacingX = m_fEstX;
	m_fSpacingY = m_fEstY;
	m_Size.x = ((int) (m_fAreaX / m_fSpacingX + 0.5)) + 1;
	m_Size.y = ((int) (m_fAreaY / m_fSpacingY + 0.5)) + 1;

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;

	EnableBasedOnConstraint();

	GetShorts()->SetValue(!m_bFloats);
}

void SampleElevationDlg::RecomputeSize()
{
	if (m_bConstraint)  // powers of 2 + 1
		m_Size.x = m_Size.y = (1 << m_power) + 1;

	m_fSpacingX = m_fAreaX / (m_Size.x - 1);
	m_fSpacingY = m_fAreaY / (m_Size.y - 1);
}

// WDR: handler implementations for SampleElevationDlg

void SampleElevationDlg::OnDotDotDot( wxCommandEvent &event )
{
	wxString filter;
	filter += FSTRING_BT;
	filter += _T("|");
	filter += FSTRING_BTGZ;

	// ask the user for a filename
	wxFileDialog saveFile(NULL, _("Save Elevation"), _T(""), _T(""), filter, wxFD_SAVE);
	saveFile.SetFilterIndex(0);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	wxString name = saveFile.GetPath();

	// work around incorrect extension(s) that wxFileDialog added
	bool bPreferGZip = (saveFile.GetFilterIndex() == 1);

	if (!name.Right(3).CmpNoCase(_T(".gz")))
		name = name.Left(name.Len()-3);
	if (!name.Right(3).CmpNoCase(_T(".bt")))
		name = name.Left(name.Len()-3);

	if (bPreferGZip)
		name += _T(".bt.gz");
	else
		name += _T(".bt");

	m_strToFile = name;

	// update controls
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void SampleElevationDlg::OnRadioOutput( wxCommandEvent &event )
{
	TransferDataFromWindow();
	EnableBasedOnConstraint();
	RecomputeSize();
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void SampleElevationDlg::OnShorts( wxCommandEvent &event )
{
	GetVUnits()->Enable(true);
}

void SampleElevationDlg::OnFloats( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	m_fVUnits = 1.0f;
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
	EnableBasedOnConstraint();
}

void SampleElevationDlg::OnSpacingXY( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	m_Size.x = (int) (m_fAreaX / m_fSpacingX)+1;
	m_Size.y = (int) (m_fAreaY / m_fSpacingY)+1;

	m_bSetting = true;
	spacing1->Enable(false);
	spacing2->Enable(false);
	TransferDataToWindow();
	spacing1->Enable(true);
	spacing2->Enable(true);
	m_bSetting = false;
}

void SampleElevationDlg::OnSizeXY( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	RecomputeSize();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void SampleElevationDlg::OnConstrain( wxCommandEvent &event )
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

void SampleElevationDlg::EnableBasedOnConstraint()
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

	GetVUnits()->Enable(!m_bFloats);
}

void SampleElevationDlg::OnBigger( wxCommandEvent &event )
{
	m_power++;
	RecomputeSize();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void SampleElevationDlg::OnSmaller( wxCommandEvent &event )
{
	m_power--;
	RecomputeSize();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

