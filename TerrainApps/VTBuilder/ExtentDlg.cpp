//
// Name: ExtentDlg.cpp
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "ExtentDlg.h"
#include "vtui/AutoDialog.h"
#include "vtdata/vtString.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ExtentDlg
//----------------------------------------------------------------------------

// WDR: event table for ExtentDlg

BEGIN_EVENT_TABLE(ExtentDlg,ExtentDlgBase)
	EVT_INIT_DIALOG (ExtentDlg::OnInitDialog)
	EVT_TEXT_ENTER( ID_EXTENT_N, ExtentDlg::OnExtentN )
	EVT_TEXT_ENTER( ID_EXTENT_W, ExtentDlg::OnExtentW )
	EVT_TEXT_ENTER( ID_EXTENT_E, ExtentDlg::OnExtentE )
	EVT_TEXT_ENTER( ID_EXTENT_S, ExtentDlg::OnExtentS )
	EVT_TEXT( ID_EXTENT_ALL, ExtentDlg::OnExtentAll )
	EVT_CHECKBOX( ID_DMS, ExtentDlg::OnDMS )
END_EVENT_TABLE()

ExtentDlg::ExtentDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	ExtentDlgBase( parent, id, title, position, size, style )
{
	m_bSetting = false;
	m_bDMS = false;

	AddValidator(this, ID_EXTENT_ALL, &m_strAll);
	AddValidator(this, ID_EXTENT_E, &m_strEast);
	AddValidator(this, ID_EXTENT_N, &m_strNorth);
	AddValidator(this, ID_EXTENT_S, &m_strSouth);
	AddValidator(this, ID_EXTENT_W, &m_strWest);
	AddValidator(this, ID_DMS, &m_bDMS);

	GetSizer()->SetSizeHints(this);
}

void ExtentDlg::SetArea(DRECT area, bool bDegrees)
{
	m_area = area;
	m_bDegrees = bDegrees;
	if (m_bDegrees)
		m_fs = _T("%4.8lf");
	else
		m_fs = _T("%8.3lf");
}

void ExtentDlg::FormatExtent(wxString &str, double value)
{
	if (m_bDMS)
	{
		bool sign = value > 0;
		value = fabs(value);
		int degrees = (int) value;
		value = (value - degrees) * 60;
		int minutes = (int) value;
		value = (value - minutes) * 60;
		double seconds = value;

		str.Printf(_T("%s%d %d %.2lf"), sign?"":"-", degrees, minutes, seconds);
	}
	else
		str.Printf(m_fs, value);
}

double ExtentDlg::GetValueFrom(const wxString &str)
{
	if (m_bDMS)
	{
		vtString cstr = (const char *) str.mb_str(wxConvUTF8);
		int degrees, minutes;
		double seconds;
		sscanf(cstr, "%d %d %lf", &degrees, &minutes, &seconds);
		bool negative = (degrees < 0);
		if (negative)
			degrees = -degrees;
		double value = degrees + (minutes / 60.0) + (seconds / 3600.0);
		if (negative)
			value = -value;
		return value;
	}
	else
		return atof(str.mb_str(wxConvUTF8));
}

void ExtentDlg::FormatStrings(int which)
{
	m_bSetting = true;
	if (which == 1)
	{
		FormatExtent(m_strWest, m_area.left);
		FormatExtent(m_strEast, m_area.right);
		FormatExtent(m_strNorth, m_area.top);
		FormatExtent(m_strSouth, m_area.bottom);
	}
	if (which == 2)
	{
		m_strAll.Printf(_T("(")+m_fs+_T(", ")+m_fs+_T("), (")+m_fs+_T(", ")+m_fs+_T(")"),
			m_area.left, m_area.bottom, m_area.Width(), m_area.Height());
	}
	TransferDataToWindow();
	m_bSetting = false;
}

// WDR: handler implementations for ExtentDlg

void ExtentDlg::OnInitDialog(wxInitDialogEvent& event)
{
	GetDMS()->Enable(m_bDegrees);

	FormatStrings(1);
	FormatStrings(2);

	wxWindow::OnInitDialog(event);
}

void ExtentDlg::OnExtentAll( wxCommandEvent &event )
{
	if (m_bSetting) return;

	TransferDataFromWindow();
	double d1, d2, d3, d4;
	int ret = sscanf(m_strAll.mb_str(wxConvUTF8), "(%lf, %lf), (%lf, %lf)",
		&d1, &d2, &d3, &d4);
	if (ret == 4)
	{
		m_area.left = d1;
		m_area.right = d1 + d3;
		m_area.bottom = d2;
		m_area.top = d2 + d4;
		FormatStrings(1);
	}
}

void ExtentDlg::OnExtentS( wxCommandEvent &event )
{
	if (m_bSetting) return;

	TransferDataFromWindow();
	m_area.bottom = GetValueFrom(m_strSouth);
	FormatStrings(2);
}

void ExtentDlg::OnExtentE( wxCommandEvent &event )
{
	if (m_bSetting) return;

	TransferDataFromWindow();
	m_area.right = GetValueFrom(m_strEast);
	FormatStrings(2);
}

void ExtentDlg::OnExtentW( wxCommandEvent &event )
{
	if (m_bSetting) return;

	TransferDataFromWindow();
	m_area.left = GetValueFrom(m_strWest);
	FormatStrings(2);
}

void ExtentDlg::OnExtentN( wxCommandEvent &event )
{
	if (m_bSetting) return;

	TransferDataFromWindow();
	m_area.top = GetValueFrom(m_strNorth);
	FormatStrings(2);
}

void ExtentDlg::OnDMS( wxCommandEvent &event )
{
	if (m_bSetting) return;

	TransferDataFromWindow();
	FormatStrings(1);
}

