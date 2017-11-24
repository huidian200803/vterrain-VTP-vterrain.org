//
// Name: ColorMapDlg.cpp
//
// This dialog is for defining a set of colors which map onto elevations,
//  to define how the user wants an elevation dataset to be colored.
//
// Copyright (c) 2004-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/colordlg.h>	// for wxColourDialog
#include <wx/imaglist.h>	// for wxImageList
#include "ColorMapDlg.h"
#include "vtdata/FileFilters.h"
#include "vtdata/FilePath.h"
#include "vtui/Helper.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ColorMapDlg
//----------------------------------------------------------------------------

// WDR: event table for ColorMapDlg

BEGIN_EVENT_TABLE(ColorMapDlg, ColorMapDlgBase)
	EVT_INIT_DIALOG (ColorMapDlg::OnInitDialog)
	EVT_LIST_ITEM_SELECTED( ID_COLORLIST, ColorMapDlg::OnItemSelected )
	EVT_LIST_ITEM_DESELECTED( ID_COLORLIST, ColorMapDlg::OnItemSelected )
	EVT_BUTTON( ID_CHANGE_COLOR, ColorMapDlg::OnChangeColor )
	EVT_BUTTON( ID_DELETE_ELEVATION, ColorMapDlg::OnDeleteColor )
	EVT_BUTTON( ID_ADD, ColorMapDlg::OnAdd )
	EVT_BUTTON( ID_SAVE_CMAP, ColorMapDlg::OnSave )
	EVT_BUTTON( ID_SAVE_AS_CMAP, ColorMapDlg::OnSaveAs )
	EVT_BUTTON( ID_LOAD_CMAP, ColorMapDlg::OnLoad )
END_EVENT_TABLE()

ColorMapDlg::ColorMapDlg( wxWindow *parent, wxWindowID id,
	const wxString& title, const wxPoint &position, const wxSize& size, long style ) :
	ColorMapDlgBase( parent, id, title, position, size, style | wxRESIZE_BORDER )
{
	AddValidator(this, ID_CMAP_FILE, &m_strFile);
	AddNumValidator(this, ID_HEIGHT_TO_ADD, &m_fHeight);
	AddValidator(this, ID_RELATIVE, &m_bRelative);
	AddValidator(this, ID_BLEND, &m_bBlend);

	m_iItem = -1;
	m_fHeight = 0.0f;

	GetList()->SetImageList(&m_imlist, wxIMAGE_LIST_SMALL);
	GetList()->InsertColumn(0, _T(""), wxLIST_FORMAT_LEFT, 180);

	GetSizer()->SetSizeHints(this);
}


// WDR: handler implementations for ColorMapDlg

void ColorMapDlg::SetFile(const char *fname)
{
	GetList()->DeleteAllItems();
	m_strFile = _T("");

	if (!m_cmap.Load(fname))
		return;

	m_strFile = wxString(fname, wxConvUTF8);
	m_bRelative = m_cmap.m_bRelative;
	m_bBlend = m_cmap.m_bBlend;

	TransferDataToWindow();
	UpdateItems();
}

void ColorMapDlg::UpdateItems()
{
	GetList()->DeleteAllItems();

	uint i, num = m_cmap.Num();

	// First refresh the color bitmaps
	m_imlist.RemoveAll();
	wxColour color;
	m_imlist.Create(32, 16);
	for (i = 0; i < num; i++)
	{
		const RGBi &c = m_cmap.Color(i);
		color.Set(c.r, c.g, c.b);
		wxBitmap *bitmap = MakeColorBitmap(32, 16, color);
		m_imlist.Add(*bitmap);
		delete bitmap;
	}
	GetList()->SetImageList(&m_imlist, wxIMAGE_LIST_SMALL);

	wxString str;
	for (i = 0; i < num; i++)
	{
		str.Printf(_("%.2f meters"), m_cmap.Elev(i));
		int item = GetList()->InsertItem(i, str, i);
	}
}

void ColorMapDlg::OnInitDialog(wxInitDialogEvent& event)
{
	UpdateEnabling();
	return wxDialog::OnInitDialog(event);
}

void ColorMapDlg::UpdateEnabling()
{
	GetChangeColor()->Enable(m_iItem != -1);
	GetDeleteColor()->Enable(m_iItem != -1);
	GetSave()->Enable(m_strFile != _T(""));
}

void ColorMapDlg::OnLoad( wxCommandEvent &event )
{
	wxFileDialog loadFile(NULL, _("Load ColorMap"), _T(""), _T(""),
		FSTRING_CMT, wxFD_OPEN);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	wxString str = loadFile.GetPath();
	vtString fname = (const char *) str.mb_str(wxConvUTF8);
	if (m_cmap.Load(fname))
	{
		m_strFile = str;
		TransferDataToWindow();
		UpdateItems();
	}
}

void ColorMapDlg::OnSave( wxCommandEvent &event )
{
	TransferDataFromWindow();
	m_cmap.m_bRelative = m_bRelative;
	m_cmap.m_bBlend = m_bBlend;

	vtString fname = (const char *) m_strFile.mb_str(wxConvUTF8);
	m_cmap.Save(fname);
}

void ColorMapDlg::OnSaveAs( wxCommandEvent &event )
{
	TransferDataFromWindow();
	m_cmap.m_bRelative = m_bRelative;
	m_cmap.m_bBlend = m_bBlend;

	// If previously saved/loaded from a file, use that as the default
	wxString default_dir, default_file;
	vtString previous = (const char *) m_strFile.mb_str(wxConvUTF8);
	if (previous != "")
	{
		default_file = StartOfFilenameWX(m_strFile);
		default_dir = wxString(ExtractPath(previous, false), wxConvUTF8);
	}

	wxFileDialog saveFile(NULL, _("Save ColorMap"), default_dir, default_file,
		FSTRING_CMT, wxFD_SAVE);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	wxString str = saveFile.GetPath();
	vtString fname = (const char *) str.mb_str(wxConvUTF8);
	if (m_cmap.Save(fname))
	{
		m_strFile = str;
		TransferDataToWindow();
	}
}

bool ColorMapDlg::AskColor(RGBi &rgb)
{
	m_ColorData.SetColour(wxColour(rgb.r, rgb.g, rgb.b));
	m_ColorData.SetChooseFull(true);

	wxColourDialog dlg(this, &m_ColorData);
	if (dlg.ShowModal() == wxID_OK)
	{
		m_ColorData = dlg.GetColourData();
		wxColour c1 = m_ColorData.GetColour();
		rgb.Set(c1.Red(), c1.Green(), c1.Blue());
		return true;
	}
	return false;
}

void ColorMapDlg::OnAdd( wxCommandEvent &event )
{
	TransferDataFromWindow();

	RGBi result;
	if (AskColor(result))
	{
		m_cmap.Add(m_fHeight, result);
		UpdateItems();
	}
}

void ColorMapDlg::OnDeleteColor( wxCommandEvent &event )
{
	m_cmap.RemoveAt(m_iItem);
	UpdateItems();
	if (GetList()->GetItemCount() > 0)
	{
		GetList()->SetItemState(m_iItem, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		m_iItem = GetList()->GetNextItem(-1, wxLIST_NEXT_ALL,
										wxLIST_STATE_SELECTED);
		if (m_iItem != -1)
			GetList()->EnsureVisible(m_iItem);
	}
	else
		m_iItem = -1;
	UpdateEnabling();
}

void ColorMapDlg::OnChangeColor( wxCommandEvent &event )
{
	RGBi rgb = m_cmap.Color(m_iItem);
	if (AskColor(rgb))
	{
		m_cmap.SetColor(m_iItem, rgb);
		UpdateItems();
	}
}

void ColorMapDlg::OnItemSelected( wxListEvent &event )
{
	m_iItem = GetList()->GetNextItem(-1, wxLIST_NEXT_ALL,
									 wxLIST_STATE_SELECTED);
	UpdateEnabling();
}

