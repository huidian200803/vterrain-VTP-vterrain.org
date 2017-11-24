//
// Name: TextureDlg.cpp
//
// Copyright (c) 2006-2011 Virtual Terrain Project
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

#include "TextureDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// TextureDlg
//----------------------------------------------------------------------------

// WDR: event table for TextureDlg

BEGIN_EVENT_TABLE(TextureDlg,TextureDlgBase)
	EVT_INIT_DIALOG (TextureDlg::OnInitDialog)

	// Texture
	EVT_RADIOBUTTON( ID_SINGLE, TextureDlg::OnRadio )
	EVT_RADIOBUTTON( ID_DERIVED, TextureDlg::OnRadio )

	EVT_COMBOBOX( ID_TFILE_SINGLE, TextureDlg::OnComboTFileSingle )
	EVT_BUTTON( ID_EDIT_COLORS, TextureDlg::OnEditColors )

	EVT_RADIOBUTTON( ID_SINGLE, TextureDlg::OnRadio )
	EVT_RADIOBUTTON( ID_DERIVED, TextureDlg::OnRadio )
	EVT_COMBOBOX( ID_TFILE_SINGLE, TextureDlg::OnComboTFileSingle )
	EVT_BUTTON( ID_EDIT_COLORS, TextureDlg::OnEditColors )
END_EVENT_TABLE()

TextureDlg::TextureDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	TextureDlgBase( parent, id, title, position, size, style )
{
	// texture
	AddValidator(this, ID_TFILE_SINGLE, &m_strTextureSingle);
	AddValidator(this, ID_CHOICE_COLORS, &m_strColorMap);

	AddValidator(this, ID_MIPMAP, &m_bMipmap);
	AddValidator(this, ID_16BIT, &m_b16bit);
	AddValidator(this, ID_PRELIGHT, &m_bPreLight);
	AddValidator(this, ID_CAST_SHADOWS, &m_bCastShadows);
	AddNumValidator(this, ID_LIGHT_FACTOR, &m_fPreLightFactor, 2);

	GetSizer()->SetSizeHints(this);
}

void TextureDlg::SetParams(const TParams &Params)
{
	VTLOG("TextureDlg::SetParams\n");
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	// texture
	m_iTexture =		Params.GetTextureEnum();

	// single
	m_strTextureSingle = wxString(Params.GetValueString(STR_TEXTUREFILE), wxConvUTF8);

	// derived
	m_strColorMap = wxString(Params.GetValueString(STR_COLOR_MAP), wxConvUTF8);

	m_bMipmap =			Params.GetValueBool(STR_MIPMAP);
	m_b16bit =			Params.GetValueBool(STR_REQUEST16BIT);
	m_bPreLight =		Params.GetValueBool(STR_PRELIGHT);
	m_fPreLightFactor = Params.GetValueFloat(STR_PRELIGHTFACTOR);
	m_bCastShadows =	Params.GetValueBool(STR_CAST_SHADOWS);
}

//
// get the values from the dialog into the supplied paramter structure
//
void TextureDlg::GetParams(TParams &Params)
{
	VTLOG("TextureDlg::GetParams\n");
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	// texture
	Params.SetTextureEnum((enum TextureEnum)m_iTexture);

	// single
	Params.SetValueString(STR_TEXTUREFILE, (const char *) m_strTextureSingle.mb_str(wxConvUTF8));

	// derived
	Params.SetValueString(STR_COLOR_MAP, (const char *) m_strColorMap.mb_str(wxConvUTF8));

	Params.SetValueBool(STR_MIPMAP, m_bMipmap);
	Params.SetValueBool(STR_REQUEST16BIT, m_b16bit);
	Params.SetValueBool(STR_PRELIGHT, m_bPreLight);
	Params.SetValueFloat(STR_PRELIGHTFACTOR, m_fPreLightFactor);
	Params.SetValueBool(STR_CAST_SHADOWS, m_bCastShadows);
}

void TextureDlg::UpdateEnableState()
{
	FindWindow(ID_TFILE_SINGLE)->Enable(m_iTexture == TE_SINGLE);
	FindWindow(ID_CHOICE_COLORS)->Enable(m_iTexture == TE_DERIVED);
	FindWindow(ID_EDIT_COLORS)->Enable(m_iTexture == TE_DERIVED);
}

void TextureDlg::UpdateColorMapChoice()
{
	// fill the "colormap" control with available colormap files
	m_choice_colors->Clear();
	vtStringArray &paths = vtGetDataPath();
	for (uint i = 0; i < paths.size(); i++)
		AddFilenamesToChoice(m_choice_colors, paths[i] + "GeoTypical", "*.cmt");

	int sel = m_choice_colors->FindString(m_strColorMap);
	if (sel != -1)
		m_choice_colors->SetSelection(sel);
}


// WDR: handler implementations for TextureDlg

void TextureDlg::OnEditColors( wxCommandEvent &event )
{
	TransferDataFromWindow();

	if (m_strColorMap.IsEmpty())
	{
		wxMessageBox(_("Please select a filename."));
		return;
	}

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

void TextureDlg::OnComboTFileSingle( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
}

void TextureDlg::OnRadio( wxCommandEvent &event )
{
	if (m_bSetting || !event.IsChecked())
		return;
	TransferDataFromWindow();
	UpdateEnableState();
}

void TextureDlg::OnInitDialog(wxInitDialogEvent& event)
{
	VTLOG("TextureDlg::OnInitDialog\n");

	bool bShowProgress = (vtGetDataPath().size() > 1);
	if (bShowProgress)
		OpenProgressDialog(_("Looking for files on data paths"), _T(""), false, this);

	m_bSetting = true;

	uint i;
	int sel;

	// Clear drop-down controls before putting values into them
	m_tfile_single->Clear();

	vtStringArray &paths = vtGetDataPath();

	// fill the "single texture filename" control with available image files
	for (i = 0; i < paths.size(); i++)
	{
		if (bShowProgress)
			UpdateProgressDialog(i * 100 / paths.size(), wxString(paths[i], wxConvUTF8));

		// Gather all possible texture image filenames
		AddFilenamesToComboBox(m_tfile_single, paths[i] + "GeoSpecific", "*.bmp");
		AddFilenamesToComboBox(m_tfile_single, paths[i] + "GeoSpecific", "*.jpg");
		AddFilenamesToComboBox(m_tfile_single, paths[i] + "GeoSpecific", "*.jpeg");
		AddFilenamesToComboBox(m_tfile_single, paths[i] + "GeoSpecific", "*.png");
		AddFilenamesToComboBox(m_tfile_single, paths[i] + "GeoSpecific", "*.tif");
	}
	sel = m_tfile_single->FindString(m_strTextureSingle);
	if (sel != -1)
		m_tfile_single->SetSelection(sel);

	UpdateColorMapChoice();

	wxWindow::OnInitDialog(event);

	UpdateEnableState();

	if (bShowProgress)
		CloseProgressDialog();

	m_bSetting = false;
}

bool TextureDlg::TransferDataToWindow()
{
	m_bSetting = true;

	m_single->SetValue(m_iTexture == TE_SINGLE);
	m_derived->SetValue(m_iTexture == TE_DERIVED);

	bool result = wxDialog::TransferDataToWindow();
	m_bSetting = false;

	return result;
}

bool TextureDlg::TransferDataFromWindow()
{
	if (m_single->GetValue()) m_iTexture = TE_SINGLE;
	if (m_derived->GetValue()) m_iTexture = TE_DERIVED;

	return wxDialog::TransferDataFromWindow();
}

