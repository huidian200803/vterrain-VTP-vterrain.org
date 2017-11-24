//
// Name: RenderDlg.cpp
//
// Copyright (c) 2004-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/colordlg.h>
#include "RenderDlg.h"

#include "vtdata/DataPath.h"
#include "vtdata/FileFilters.h"
#include "vtdata/FilePath.h"
#include "vtui/AutoDialog.h"
#include "vtui/ColorMapDlg.h"
#include "vtui/Helper.h"		// for AddFilenamesToChoice

// WDR: class implementations

//----------------------------------------------------------------------------
// RenderDlg
//----------------------------------------------------------------------------

// WDR: event table for RenderDlg

BEGIN_EVENT_TABLE(RenderDlg, RenderDlgBase)
	EVT_INIT_DIALOG (RenderDlg::OnInitDialog)
	EVT_RADIOBUTTON( ID_RADIO_CREATE_NEW, RenderDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO_TO_FILE, RenderDlg::OnRadio )
	EVT_RADIOBUTTON( ID_GEOTIFF, RenderDlg::OnRadio )
	EVT_RADIOBUTTON( ID_JPEG, RenderDlg::OnRadio )
	EVT_BUTTON( ID_DOTDOTDOT, RenderDlg::OnDotdotdot )
	EVT_CHECKBOX( ID_CONSTRAIN, RenderDlg::OnConstrain )
	EVT_CHECKBOX( ID_CONSTRAIN, RenderDlg::OnConstrain )
	EVT_BUTTON( ID_SMALLER, RenderDlg::OnSmaller )
	EVT_BUTTON( ID_BIGGER, RenderDlg::OnBigger )
	EVT_BUTTON( ID_EDIT_COLORS, RenderDlg::OnEditColors )
	EVT_BUTTON( ID_COLOR_NODATA, RenderDlg::OnColorNODATA )
END_EVENT_TABLE()

RenderDlg::RenderDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	RenderDlgBase( parent, id, title, position, size, style )
{
	m_power = 8;
	m_bConstraint = false;
	m_bToFile = false;
	m_bJPEG = false;
	m_ColorNODATA.Set(255,0,0);

	m_Size.x = 256;
	m_Size.y = 256;

	// sampling
	AddValidator(this, ID_RADIO_TO_FILE, &m_bToFile);
	AddValidator(this, ID_TEXT_TO_FILE, &m_strToFile);
	AddValidator(this, ID_JPEG, &m_bJPEG);

	AddValidator(this, ID_CHOICE_COLORS, &m_strColorMap);
	AddValidator(this, ID_SHADING, &m_bShading);

	AddNumValidator(this, ID_SIZEX, &m_Size.x);
	AddNumValidator(this, ID_SIZEY, &m_Size.y);
	AddValidator(this, ID_CONSTRAIN, &m_bConstraint);

	UpdateEnabling();

	GetSizer()->SetSizeHints(this);
}

void RenderDlg::OnInitDialog(wxInitDialogEvent& event)
{
	UpdateColorMapChoice();
	FillWithColorSize(GetColorNodata(), 32, 18, m_ColorNODATA);

	m_bSetting = true;
	wxDialog::OnInitDialog(event);
	m_bSetting = false;
}

void RenderDlg::RecomputeSize()
{
	if (m_bConstraint)  // powers of 2 + 1
		m_Size.x = m_Size.y = (1 << m_power);
}

void RenderDlg::UpdateEnabling()
{
	GetTextToFile()->Enable(m_bToFile);
	GetJpeg()->Enable(m_bToFile);
	GetGeotiff()->Enable(m_bToFile);
	GetDotdotdot()->Enable(m_bToFile);

	GetSmaller()->Enable(m_bConstraint);
	GetBigger()->Enable(m_bConstraint);
	GetSizeX()->SetEditable(!m_bConstraint);
	GetSizeY()->SetEditable(!m_bConstraint);
}

void RenderDlg::UpdateColorMapChoice()
{
	GetColorMap()->Clear();
	for (uint i = 0; i < vtGetDataPath().size(); i++)
	{
		// fill the "colormap" control with available colormap files
		AddFilenamesToChoice(GetColorMap(), vtGetDataPath()[i] + "GeoTypical", "*.cmt");
	}
	int sel = GetColorMap()->FindString(m_strColorMap);
	if (sel != -1)
		GetColorMap()->SetSelection(sel);
	else if (GetColorMap()->GetCount() > 0)
		GetColorMap()->SetSelection(0);
}

// WDR: handler implementations for RenderDlg

void RenderDlg::OnColorNODATA( wxCommandEvent &event )
{
	wxColourData ColorData;
	ColorData.SetChooseFull(true);

	// Set the existing color to the dialog
	wxColour Color;
	Color.Set(m_ColorNODATA.r, m_ColorNODATA.g, m_ColorNODATA.b);
	ColorData.SetColour(Color);

	wxColourDialog dlg(this, &ColorData);
	if (dlg.ShowModal() == wxID_OK)
	{
		// Get the color from the dialog
		ColorData = dlg.GetColourData();
		Color = ColorData.GetColour();
		m_ColorNODATA.Set(Color.Red(), Color.Green(), Color.Blue());
		FillWithColorSize(GetColorNodata(), 32, 18, m_ColorNODATA);
	}
}

void RenderDlg::OnEditColors( wxCommandEvent &event )
{
	TransferDataFromWindow();

	ColorMapDlg dlg(this, -1, _("ColorMap"));

	// Look on data paths, to give a complete path to the dialog
	if (m_strColorMap != _T(""))
	{
		vtString name = "GeoTypical/";
		name += m_strColorMap.mb_str(wxConvUTF8);
		name = FindFileOnPaths(vtGetDataPath(), name);
		if (name == "")
		{
			wxMessageBox(_("Couldn't locate file."));
			return;
		}
		dlg.SetFile(name);
	}
	dlg.ShowModal();

	// They may have added or removed some color map files on the data path
	UpdateColorMapChoice();
}


void RenderDlg::OnDotdotdot( wxCommandEvent &event )
{
	wxString filter;
	filter += FSTRING_JPEG;
	filter += _T("|");
	filter += FSTRING_TIF;

	// ask the user for a filename
	wxFileDialog saveFile(NULL, _("Specify image file"), _T(""), _T(""),
		filter, wxFD_SAVE);
	saveFile.SetFilterIndex(0);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	wxString name = saveFile.GetPath();

	// work around incorrect extension(s) that wxFileDialog added
	if (!name.Right(4).CmpNoCase(_T(".jpg")))
		name = name.Left(name.Len()-4);
	if (!name.Right(4).CmpNoCase(_T(".tif")))
		name = name.Left(name.Len()-4);

	if (m_bJPEG)
		name += _T(".jpeg");
	else
		name += _T(".tif");

	m_strToFile = name;

	// update controls
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void RenderDlg::OnRadio( wxCommandEvent &event )
{
	TransferDataFromWindow();

	RemoveFileExtensions(m_strToFile);
	if (m_strToFile != _T(""))
	{
		if (m_bJPEG)
			m_strToFile += _T(".jpeg");
		else
			m_strToFile += _T(".tif");
	}

	// update controls
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;

	UpdateEnabling();
}

void RenderDlg::OnConstrain( wxCommandEvent &event )
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
	UpdateEnabling();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void RenderDlg::OnSmaller( wxCommandEvent &event )
{
	m_power--;
	RecomputeSize();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void RenderDlg::OnBigger( wxCommandEvent &event )
{
	m_power++;
	RecomputeSize();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

