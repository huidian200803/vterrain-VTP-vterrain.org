//
// Name: EphemDlg.cpp
//
// Copyright (c) 2007-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/colordlg.h>

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/SkyDome.h"
#include "vtdata/DataPath.h"
#include "vtdata/vtLog.h"
#include "EphemDlg.h"
#include "EnviroGUI.h"    // for g_App
#include "vtui/Helper.h"    // for FillWithColor

// WDR: class implementations

//----------------------------------------------------------------------------
// EphemDlg
//----------------------------------------------------------------------------

// WDR: event table for EphemDlg

BEGIN_EVENT_TABLE(EphemDlg,EphemDlgBase)
EVT_INIT_DIALOG (EphemDlg::OnInitDialog)
EVT_CHECKBOX( ID_OCEANPLANE, EphemDlg::OnCheckBox )
EVT_CHECKBOX( ID_SKY, EphemDlg::OnCheckBox )
EVT_COMBOBOX( ID_SKYTEXTURE, EphemDlg::OnSkyTexture )
EVT_CHECKBOX( ID_FOG, EphemDlg::OnCheckBox )

EVT_CHECKBOX( ID_SHADOWS, EphemDlg::OnCheckBox )
EVT_TEXT( ID_AMBIENT_BIAS, EphemDlg::OnDarkness )
EVT_SLIDER( ID_SLIDER_AMBIENT_BIAS, EphemDlg::OnSliderDarkness )
EVT_CHECKBOX( ID_SHADOWS_EVERY_FRAME, EphemDlg::OnCheckBox )
EVT_CHECKBOX( ID_SHADOW_LIMIT, EphemDlg::OnCheckBox )
EVT_TEXT( ID_SHADOW_LIMIT_RADIUS, EphemDlg::OnDarkness )

EVT_BUTTON( ID_BGCOLOR, EphemDlg::OnBgColor )
EVT_TEXT( ID_OCEANPLANEOFFSET, EphemDlg::OnOceanPlaneOffset )
EVT_TEXT( ID_FOG_DISTANCE, EphemDlg::OnFogDistance )
EVT_TEXT( ID_TEXT_WIND_DIRECTION, EphemDlg::OnWindDirection )
EVT_TEXT( ID_TEXT_WIND_SPEED, EphemDlg::OnWindSpeed )
EVT_SLIDER( ID_SLIDER_FOG_DISTANCE, EphemDlg::OnSliderFogDistance )
EVT_SLIDER( ID_SLIDER_WIND_DIRECTION, EphemDlg::OnSliderWindDirection )
EVT_SLIDER( ID_SLIDER_WIND_SPEED, EphemDlg::OnSliderWindSpeed)
END_EVENT_TABLE()

EphemDlg::EphemDlg(wxWindow *parent, wxWindowID id, const wxString &title,
				   const wxPoint &position, const wxSize& size, long style) :
	EphemDlgBase( parent, id, title, position, size, style )
{
	m_bSetting = false;
	m_iWindDirSlider = 0;
	m_iWindSpeedSlider = 0;
	m_iWindDir = 0;
	m_fWindSpeed = 0;
	m_pTerrain = NULL;

	AddValidator(this, ID_SKY, &m_bSky);
	AddValidator(this, ID_SKYTEXTURE, &m_strSkyTexture);

	AddValidator(this, ID_OCEANPLANE, &m_bOceanPlane);
	AddNumValidator(this, ID_OCEANPLANEOFFSET, &m_fOceanPlaneLevel);

	// fog
	AddValidator(this, ID_FOG, &m_bFog);
	AddNumValidator(this, ID_FOG_DISTANCE, &m_fFogDistance);
	AddValidator(this, ID_SLIDER_FOG_DISTANCE, &m_iFogDistance);

	// shadows
	AddValidator(this, ID_SHADOWS, &m_bShadows);
	AddNumValidator(this, ID_AMBIENT_BIAS, &m_fDarkness);
	AddValidator(this, ID_SLIDER_AMBIENT_BIAS, &m_iDarkness);
	AddValidator(this, ID_SHADOWS_EVERY_FRAME, &m_bShadowsEveryFrame);
	AddValidator(this, ID_SHADOW_LIMIT, &m_bShadowLimit);
	AddNumValidator(this, ID_SHADOW_LIMIT_RADIUS, &m_fShadowRadius);

	AddNumValidator(this, ID_TEXT_WIND_DIRECTION, &m_iWindDir);
	AddValidator(this, ID_SLIDER_WIND_DIRECTION, &m_iWindDirSlider);

	AddNumValidator(this, ID_TEXT_WIND_SPEED, &m_fWindSpeed);
	AddValidator(this, ID_SLIDER_WIND_SPEED, &m_iWindSpeedSlider);

	GetSizer()->SetSizeHints(this);
}

void EphemDlg::UpdateEnableState()
{
	FindWindow(ID_OCEANPLANEOFFSET)->Enable(m_bOceanPlane);
	m_skytexture->Enable(m_bSky);
	GetFogDistance()->Enable(m_bFog);
	GetSliderFogDistance()->Enable(m_bFog);
	GetDarkness()->Enable(m_bShadows);
	GetSliderDarkness()->Enable(m_bShadows);

	// can have fog and shadows at the same time
	GetFog()->Enable(!m_bShadows);
}

#define DIST_MIN 1.698970004336		// 50 m
#define DIST_MAX 4.903089986992		// 80 km
#define DIST_RANGE (DIST_MAX-DIST_MIN)

#define BIAS_MIN 0.0f
#define BIAS_MAX 1.0f
#define BIAS_RANGE (BIAS_MAX-BIAS_MIN)

void EphemDlg::ValuesToSliders()
{
	m_iWindDirSlider = m_iWindDir / 2;
	m_iWindSpeedSlider = (int) (m_fWindSpeed / 4 * 15);
	m_iFogDistance = (int) ((log10f(m_fFogDistance) - DIST_MIN) / DIST_RANGE * 100);
	m_iDarkness = (int) ((m_fDarkness - BIAS_MIN) / BIAS_RANGE * 100);
}

void EphemDlg::SlidersToValues()
{
	m_iWindDir = m_iWindDirSlider * 2;
	m_fWindSpeed = (float) m_iWindSpeedSlider * 4 / 15;
	m_fFogDistance = powf(10, (DIST_MIN + m_iFogDistance * DIST_RANGE / 100));
	m_fDarkness = BIAS_MIN + (m_iDarkness * BIAS_RANGE / 100);
}

void EphemDlg::SetSliderControls()
{
	FindWindow(ID_SLIDER_WIND_DIRECTION)->GetValidator()->TransferToWindow();
	FindWindow(ID_SLIDER_WIND_SPEED)->GetValidator()->TransferToWindow();
	FindWindow(ID_SLIDER_FOG_DISTANCE)->GetValidator()->TransferToWindow();
	FindWindow(ID_SLIDER_AMBIENT_BIAS)->GetValidator()->TransferToWindow();
}

void EphemDlg::UpdateColorControl()
{
	FillWithColorSize(GetColorBitmap(), 32, 18, m_BgColor);
}

void EphemDlg::SetToScene()
{
	vtTerrainScene *ts = vtGetTS();
	TParams &param = m_pTerrain->GetParams();
	vtSkyDome *sky = ts->GetSkyDome();

	param.SetValueBool(STR_SKY, m_bSky);
	param.SetValueString(STR_SKYTEXTURE, (const char *) m_strSkyTexture.mb_str(wxConvUTF8));
	ts->UpdateSkydomeForTerrain(m_pTerrain);
	m_pTerrain->SetFeatureVisible(TFT_OCEAN, m_bOceanPlane);
	m_pTerrain->SetWaterLevel(m_fOceanPlaneLevel);
	m_pTerrain->SetFog(m_bFog);
	m_pTerrain->SetFogDistance(m_fFogDistance);
	// shadows
	m_pTerrain->SetShadows(m_bShadows);
	vtShadowOptions opt;
	m_pTerrain->GetShadowOptions(opt);
	opt.fDarkness = m_fDarkness;
	opt.bShadowsEveryFrame = m_bShadowsEveryFrame;
	opt.bShadowLimit = m_bShadowLimit;
	opt.fShadowRadius = m_fShadowRadius;
	m_pTerrain->SetShadowOptions(opt);

	RGBi col(m_BgColor.Red(), m_BgColor.Green(), m_BgColor.Blue());
	m_pTerrain->SetBgColor(col);
	vtGetScene()->SetBgColor(col);
	g_App.SetWind(m_iWindDir, m_fWindSpeed);
}

// WDR: handler implementations for EphemDlg

void EphemDlg::OnInitDialog(wxInitDialogEvent& event)
{
	VTLOG("EphemDlg::OnInitDialog\n");
	m_bSetting = true;

	vtStringArray &paths = vtGetDataPath();
	uint i;
	int sel;

	m_skytexture->Clear();
	for (i = 0; i < paths.size(); i++)
	{
		// fill in Sky files
		AddFilenamesToComboBox(m_skytexture, paths[i] + "Sky", "*.bmp");
		AddFilenamesToComboBox(m_skytexture, paths[i] + "Sky", "*.png");
		AddFilenamesToComboBox(m_skytexture, paths[i] + "Sky", "*.jpg");
		sel = m_skytexture->FindString(m_strSkyTexture);
		if (sel != -1)
			m_skytexture->SetSelection(sel);
	}
	UpdateColorControl();
	wxWindow::OnInitDialog(event);
	UpdateEnableState();
	m_bSetting = false;
}

void EphemDlg::OnSkyTexture( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	SetToScene();
}

void EphemDlg::OnSliderFogDistance( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	SlidersToValues();
	SetToScene();
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void EphemDlg::OnSliderDarkness( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	SlidersToValues();
	SetToScene();
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void EphemDlg::OnSliderWindSpeed( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	SlidersToValues();
	g_App.SetWind(m_iWindDir, m_fWindSpeed);
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void EphemDlg::OnSliderWindDirection( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	SlidersToValues();
	g_App.SetWind(m_iWindDir, m_fWindSpeed);
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void EphemDlg::OnWindSpeed( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	ValuesToSliders();
	g_App.SetWind(m_iWindDir, m_fWindSpeed);
	SetSliderControls();
}

void EphemDlg::OnWindDirection( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	ValuesToSliders();
	g_App.SetWind(m_iWindDir, m_fWindSpeed);
	SetSliderControls();
}

void EphemDlg::OnFogDistance( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	ValuesToSliders();
	SetToScene();
	SetSliderControls();
}

void EphemDlg::OnDarkness( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	ValuesToSliders();
	SetToScene();
	SetSliderControls();
}

void EphemDlg::OnOceanPlaneOffset( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	SetToScene();
}

void EphemDlg::OnBgColor( wxCommandEvent &event )
{
	wxColourData data;
	data.SetChooseFull(true);
	data.SetColour(m_BgColor);

	wxColourDialog dlg(this, &data);
	if (dlg.ShowModal() == wxID_OK)
	{
		wxColourData data2 = dlg.GetColourData();
		m_BgColor = data2.GetColour();
		UpdateColorControl();
		SetToScene();
	}
}

void EphemDlg::OnCheckBox( wxCommandEvent &event )
{
	TransferDataFromWindow();
	UpdateEnableState();
	SetToScene();
}


