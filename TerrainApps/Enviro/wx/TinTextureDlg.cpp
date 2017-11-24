//
// Name: TinTextureDlg.cpp
//
// Copyright (c) 2006-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/vtLog.h"
#include "vtdata/DataPath.h"
#include "vtdata/FilePath.h"	// for FindFileOnPaths
#include "vtui/Helper.h"		// for AddFilenamesToChoice
#include "vtui/ColorMapDlg.h"

#include "TinTextureDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// TinTextureDlg
//----------------------------------------------------------------------------

// WDR: event table for TinTextureDlg

BEGIN_EVENT_TABLE(TinTextureDlg,TinTextureDlgBase)
	EVT_INIT_DIALOG (TinTextureDlg::OnInitDialog)

	EVT_CHECKBOX( ID_DERIVE_COLOR, TinTextureDlg::OnCheckBox )
	EVT_BUTTON( ID_EDIT_COLORS, TinTextureDlg::OnEditColors )

	EVT_CHECKBOX( ID_GEOTYPICAL, TinTextureDlg::OnCheckBox )
	EVT_TEXT( ID_OPACITY, TinTextureDlg::OnText )
	EVT_SLIDER( ID_OPACITY_SLIDER, TinTextureDlg::OnOpacitySlider )
END_EVENT_TABLE()

TinTextureDlg::TinTextureDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	TinTextureDlgBase( parent, id, title, position, size, style )
{
	// texture
	AddValidator(this, ID_DERIVE_COLOR, &m_bDeriveColor);
	AddValidator(this, ID_CHOICE_COLORS, &m_strColorMap);
	AddValidator(this, ID_GEOTYPICAL, &m_bGeotypical);
	AddValidator(this, ID_TFILE_GEOTYPICAL, &m_strTextureGeotypical);
	AddNumValidator(this, ID_GEOTYPICAL_SCALE, &m_fGeotypicalScale);
	AddNumValidator(this, ID_OPACITY, &m_fOpacity, 2);
	AddValidator(this, ID_OPACITY_SLIDER, &m_iOpacity);

	GetSizer()->SetSizeHints(this);
}

void TinTextureDlg::SetOptions(const vtTagArray &layer_tags)
{
	VTLOG("TinTextureDlg::SetOptions\n");
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	// derived
	m_strColorMap = wxString(layer_tags.GetValueString(STR_COLOR_MAP), wxConvUTF8);
	m_bDeriveColor = (m_strColorMap != _T(""));
	m_strTextureGeotypical = wxString(layer_tags.GetValueString(STR_TEXTURE_GEOTYPICAL), wxConvUTF8);
	m_bGeotypical = (m_strTextureGeotypical != _T(""));
	m_fGeotypicalScale = layer_tags.GetValueFloat(STR_GEOTYPICAL_SCALE);
	m_fOpacity = layer_tags.GetValueFloat(STR_OPACITY);

	ValuesToSliders();
	UpdateEnableState();
}

void TinTextureDlg::GetOptions(vtTagArray &layer_tags)
{
	VTLOG("TinTextureDlg::GetOptions\n");
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	// derived
	if (m_bDeriveColor)
		layer_tags.SetValueString(STR_COLOR_MAP, (const char *) m_strColorMap.mb_str(wxConvUTF8));
	else
		layer_tags.SetValueString(STR_COLOR_MAP, "");

	if (m_bGeotypical)
		layer_tags.SetValueString(STR_TEXTURE_GEOTYPICAL, (const char *) m_strTextureGeotypical.mb_str(wxConvUTF8));
	else
		layer_tags.SetValueString(STR_TEXTURE_GEOTYPICAL, "");

	layer_tags.SetValueFloat(STR_GEOTYPICAL_SCALE, m_fGeotypicalScale);
	layer_tags.SetValueFloat(STR_OPACITY, m_fOpacity);
}

void TinTextureDlg::UpdateColorMapChoice()
{
	m_choice_colors->Clear();
	vtStringArray &paths = vtGetDataPath();
	for (uint i = 0; i < paths.size(); i++)
	{
		// fill the "colormap" control with available colormap files
		AddFilenamesToChoice(m_choice_colors, paths[i] + "GeoTypical", "*.cmt");
	}
	int sel = m_choice_colors->FindString(m_strColorMap);
	if (sel != -1)
		m_choice_colors->SetSelection(sel);
}


// WDR: handler implementations for TinTextureDlg

void TinTextureDlg::OnEditColors( wxCommandEvent &event )
{
	TransferDataFromWindow();

	// Look on data paths, to give a complete path to the dialog
	vtString name = "GeoTypical/";
	name += m_strColorMap.mb_str(wxConvUTF8);
	name = FindFileOnPaths(vtGetDataPath(), name);
	if (name == "")
	{
		wxMessageBox(_("Couldn't locate file."));
		return;
	}

	ColorMapDlg dlg(this, -1, _("ColorMap"));
	dlg.SetFile(name);
	dlg.ShowModal();

	// They may have added or removed some color map files on the data path
	UpdateColorMapChoice();
}

void TinTextureDlg::OnInitDialog(wxInitDialogEvent& event)
{
	VTLOG("TinTextureDlg::OnInitDialog\n");

	m_bSetting = true;

	m_tfile_geotypical->Clear();

	vtStringArray &paths = vtGetDataPath();
	for (uint i = 0; i < paths.size(); i++)
	{
		// fill the "geotypical" control with available bitmap files
		AddFilenamesToComboBox(m_tfile_geotypical, paths[i] + "GeoTypical", "*.bmp");
		AddFilenamesToComboBox(m_tfile_geotypical, paths[i] + "GeoTypical", "*.jpg");
		AddFilenamesToComboBox(m_tfile_geotypical, paths[i] + "GeoTypical", "*.png");
	}

	// Look for existing values.
	int sel = m_tfile_geotypical->FindString(m_strTextureGeotypical);
	if (sel != -1)
		m_tfile_geotypical->SetSelection(sel);

	UpdateColorMapChoice();
	wxWindow::OnInitDialog(event);
	m_bSetting = false;
}

bool TinTextureDlg::TransferDataToWindow()
{
	m_bSetting = true;
	bool result = wxDialog::TransferDataToWindow();
	m_bSetting = false;

	return result;
}

bool TinTextureDlg::TransferDataFromWindow()
{
	return wxDialog::TransferDataFromWindow();
}

#define OPACITY_MIN	0.0f
#define OPACITY_RANGE 1.0f

void TinTextureDlg::ValuesToSliders()
{
	m_iOpacity =	(int) ((m_fOpacity - OPACITY_MIN) / OPACITY_RANGE * 100);
}

void TinTextureDlg::OnText( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	ValuesToSliders();
	m_opacity_slider->GetValidator()->TransferToWindow();
}

void TinTextureDlg::OnOpacitySlider( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	m_fOpacity = OPACITY_MIN + (m_iOpacity * OPACITY_RANGE / 100);
	TransferDataToWindow();
}

void TinTextureDlg::OnCheckBox( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	UpdateEnableState();
}

void TinTextureDlg::UpdateEnableState()
{
	m_choice_colors->Enable(m_bDeriveColor);
	m_edit_colors->Enable(m_bDeriveColor);

	m_tfile_geotypical->Enable(m_bGeotypical);
	m_geotypical_scale->Enable(m_bGeotypical);
}
