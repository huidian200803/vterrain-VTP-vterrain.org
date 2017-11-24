//
// Name: OptionsDlg.cpp
//
// Copyright (c) 2004-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "OptionsDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
	  #include "wx/wx.h"
#endif

#include "vtdata/DataPath.h"
#include "vtui/Helper.h"	// for AddFilenamesToChoice
#include "OptionsDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// OptionsDlg
//----------------------------------------------------------------------------

// WDR: event table for OptionsDlg

BEGIN_EVENT_TABLE(OptionsDlg,OptionsDlgBase)
	EVT_INIT_DIALOG (OptionsDlg::OnInitDialog)
	EVT_CHECKBOX( ID_FULLSCREEN, OptionsDlg::OnCheck )
	EVT_CHECKBOX( ID_STEREO, OptionsDlg::OnCheck )
	EVT_CHECKBOX( ID_DIRECT_PICKING, OptionsDlg::OnCheck )
	EVT_CHECKBOX( ID_TERRAIN_PROGRESS, OptionsDlg::OnCheck )
	EVT_BUTTON( wxID_OK, OptionsDlg::OnOK )
END_EVENT_TABLE()

OptionsDlg::OptionsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	OptionsDlgBase( parent, id, title, position, size, style )
{
	AddValidator(this, ID_FULLSCREEN, &m_bFullscreen);
	AddValidator(this, ID_STEREO, &m_bStereo);
	AddNumValidator(this, ID_WINX, &m_WinPos.x);
	AddNumValidator(this, ID_WINY, &m_WinPos.y);
	AddNumValidator(this, ID_WIN_XSIZE, &m_WinSize.x);
	AddNumValidator(this, ID_WIN_YSIZE, &m_WinSize.y);
	AddValidator(this, ID_SIZE_INSIDE, &m_bLocationInside);

//  AddValidator(this, ID_HTML_PANE, &m_bHtmlpane);
//  AddValidator(this, ID_FLOATING, &m_bFloatingToolbar);
	AddValidator(this, ID_TEXTURE_COMPRESSION, &m_bTextureCompression);
//  AddValidator(this, ID_SHADOWS, &m_bShadows);
	AddValidator(this, ID_DISABLE_MIPMAPS, &m_bDisableMipmaps);

	AddValidator(this, ID_DIRECT_PICKING, &m_bDirectPicking);
	AddNumValidator(this, ID_SELECTION_CUTOFF, &m_fSelectionCutoff, 2);
	AddNumValidator(this, ID_SELECTION_RADIUS, &m_fMaxPickableInstanceRadius, 2);

	AddNumValidator(this, ID_PLANTSIZE, &m_fPlantScale, 2);
	AddValidator(this, ID_ONLY_AVAILABLE_SPECIES, &m_bOnlyAvailableSpecies);

	AddValidator(this, ID_CHOICE_CONTENT, &m_iContentFile);
	AddValidator(this, ID_CHOICE_CONTENT, &m_strContentFile);

	AddValidator(this, ID_TERRAIN_PROGRESS, &m_bShowProgress);
	AddValidator(this, ID_FLY_IN, &m_bFlyIn);
	AddValidator(this, ID_ENABLE_JOYSTICK, &m_bUseJoystick);
	AddValidator(this, ID_ENABLE_SPACENAV, &m_bUseSpaceNav);

	GetSizer()->SetSizeHints(this);
}


void OptionsDlg::GetOptionsFrom(EnviroOptions &opt)
{
	m_bFullscreen = opt.m_bFullscreen;
	m_bStereo = opt.m_bStereo;
	m_iStereoMode = opt.m_iStereoMode;
	m_iMultiSamples = opt.m_iMultiSamples;
	m_WinPos = opt.m_WinPos;
	m_WinSize = opt.m_WinSize;
	m_bLocationInside = opt.m_bLocationInside;

//  m_bHtmlpane = opt.m_bHtmlpane;
//  m_bFloatingToolbar = opt.m_bFloatingToolbar;
	m_bTextureCompression = opt.m_bTextureCompression;
	m_bDisableMipmaps = opt.m_bDisableModelMipmaps;

	m_bDirectPicking = opt.m_bDirectPicking;
	m_fSelectionCutoff = opt.m_fSelectionCutoff;
	m_fMaxPickableInstanceRadius = opt.m_fMaxPickableInstanceRadius;

	m_fPlantScale = opt.m_fPlantScale;
//  m_bShadows = opt.m_bShadows;
	m_bOnlyAvailableSpecies = opt.m_bOnlyAvailableSpecies;

	m_strContentFile = wxString(opt.m_strContentFile, wxConvUTF8);
	m_bShowProgress = opt.m_bShowProgress;
	m_bFlyIn = opt.m_bFlyIn;
	m_bUseJoystick = opt.m_bUseJoystick;
	m_bUseSpaceNav = opt.m_bUseSpaceNav;
}

void OptionsDlg::PutOptionsTo(EnviroOptions &opt)
{
	opt.m_bFullscreen = m_bFullscreen;
	opt.m_bStereo = m_bStereo;
	opt.m_iStereoMode = m_iStereoMode;
	opt.m_iMultiSamples = m_iMultiSamples;
	opt.m_WinPos = m_WinPos;
	opt.m_WinSize = m_WinSize;
	opt.m_bLocationInside = m_bLocationInside;

//  opt.m_bHtmlpane = m_bHtmlpane;
//  opt.m_bFloatingToolbar = m_bFloatingToolbar;
	opt.m_bTextureCompression = m_bTextureCompression;
	opt.m_bDisableModelMipmaps = m_bDisableMipmaps;

	opt.m_bDirectPicking = m_bDirectPicking;
	opt.m_fSelectionCutoff = m_fSelectionCutoff;
	opt.m_fMaxPickableInstanceRadius = m_fMaxPickableInstanceRadius;

	opt.m_fPlantScale = m_fPlantScale;
//  opt.m_bShadows = m_bShadows;
	opt.m_bOnlyAvailableSpecies = m_bOnlyAvailableSpecies;

	opt.m_strContentFile = m_strContentFile.mb_str(wxConvUTF8);
	opt.m_bShowProgress = m_bShowProgress;
	opt.m_bFlyIn = m_bFlyIn;
	opt.m_bUseJoystick = m_bUseJoystick;
	opt.m_bUseSpaceNav = m_bUseSpaceNav;
}

void OptionsDlg::UpdateEnabling()
{
	m_stereo1->Enable(m_bStereo);
	m_stereo2->Enable(m_bStereo);
	m_stereo3->Enable(m_bStereo);
	m_stereo4->Enable(m_bStereo);

	GetWinx()->Enable(!m_bFullscreen);
	GetWiny()->Enable(!m_bFullscreen);
	GetWinXsize()->Enable(!m_bFullscreen);
	GetWinYsize()->Enable(!m_bFullscreen);
	GetSizeInside()->Enable(!m_bFullscreen);

	GetSelectionCutoff()->Enable(!m_bDirectPicking);
	GetSelectionRadius()->Enable(!m_bDirectPicking);
}

// WDR: handler implementations for OptionsDlg

void OptionsDlg::OnOK(wxCommandEvent &event)
{
	if (m_stereo1->GetValue()) m_iStereoMode = 0;
	if (m_stereo2->GetValue()) m_iStereoMode = 1;
	if (m_stereo3->GetValue()) m_iStereoMode = 2;
	if (m_stereo4->GetValue()) m_iStereoMode = 3;

	if (m_samples0->GetValue()) m_iMultiSamples = 0;
	if (m_samples4->GetValue()) m_iMultiSamples = 4;
	if (m_samples8->GetValue()) m_iMultiSamples = 8;
	if (m_samples16->GetValue()) m_iMultiSamples = 16;

	event.Skip();
}

void OptionsDlg::OnCheck( wxCommandEvent &event )
{
	TransferDataFromWindow();
	UpdateEnabling();
}

void OptionsDlg::OnInitDialog(wxInitDialogEvent& event)
{
	// Populate Content files choices
	GetContent()->Clear();
	vtStringArray &paths = vtGetDataPath();
	for (uint i = 0; i < paths.size(); i++)
	{
		vtString path = paths[i];
		AddFilenamesToChoice(GetContent(), path, "*.vtco");
	}
	m_iContentFile = GetContent()->FindString(m_strContentFile);
	if (m_iContentFile != -1)
		GetContent()->SetSelection(m_iContentFile);

	UpdateEnabling();

	if (m_iStereoMode == 0) m_stereo1->SetValue(true);
	if (m_iStereoMode == 1) m_stereo2->SetValue(true);
	if (m_iStereoMode == 2) m_stereo3->SetValue(true);
	if (m_iStereoMode == 3) m_stereo4->SetValue(true);

	if (m_iMultiSamples == 0) m_samples0->SetValue(true);
	if (m_iMultiSamples == 4) m_samples4->SetValue(true);
	if (m_iMultiSamples == 8) m_samples8->SetValue(true);
	if (m_iMultiSamples == 16) m_samples16->SetValue(true);

	event.Skip();
}

