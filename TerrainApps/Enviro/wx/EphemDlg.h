//
// Name: EphemDlg.h
//
// Copyright (c) 2007-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __EphemDlg_H__
#define __EphemDlg_H__

#include "EnviroUI.h"
#include "vtui/AutoDialog.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// EphemDlg
//----------------------------------------------------------------------------

class EphemDlg: public EphemDlgBase
{
public:
	// constructors and destructors
	EphemDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for EphemDlg
	wxSlider* GetSliderDarkness()  { return (wxSlider*) FindWindow( ID_SLIDER_AMBIENT_BIAS ); }
	wxTextCtrl* GetDarkness()  { return (wxTextCtrl*) FindWindow( ID_AMBIENT_BIAS ); }
	wxCheckBox* GetShadows()  { return (wxCheckBox*) FindWindow( ID_SHADOWS ); }
	wxSlider* GetSliderFogDistance()  { return (wxSlider*) FindWindow( ID_SLIDER_FOG_DISTANCE ); }
	wxTextCtrl* GetFogDistance()  { return (wxTextCtrl*) FindWindow( ID_FOG_DISTANCE ); }
	wxCheckBox* GetFog()  { return (wxCheckBox*) FindWindow( ID_FOG ); }
	wxStaticBitmap* GetColorBitmap()  { return (wxStaticBitmap*) FindWindow( ID_COLOR3 ); }

	void SetTerrain(class vtTerrain *pTerr) { m_pTerrain = pTerr; }
	void UpdateEnableState();
	void UpdateColorControl();
	void ValuesToSliders();
	void SlidersToValues();
	void SetSliderControls();
	void SetToScene();

	bool m_bSky;
	wxString m_strSkyTexture;
	bool m_bOceanPlane;
	float m_fOceanPlaneLevel;
	// fog
	bool m_bFog;
	float m_fFogDistance;
	// shadows
	bool m_bShadows;
	float m_fDarkness;
	int m_iDarkness;
	bool m_bShadowsEveryFrame;
	bool m_bShadowLimit;
	float m_fShadowRadius;

	wxColor m_BgColor;
	int m_iWindDir;
	float m_fWindSpeed;

private:
	// WDR: member variable declarations for EphemDlg
	bool m_bSetting;

	int m_iFogDistance;
	int m_iWindDirSlider;
	int m_iWindSpeedSlider;
	class vtTerrain *m_pTerrain;

private:
	// WDR: handler declarations for EphemDlg
	void OnSkyTexture( wxCommandEvent &event );
	void OnFogDistance( wxCommandEvent &event );
	void OnDarkness( wxCommandEvent &event );
	void OnShadowLimit( wxCommandEvent &event );
	void OnOceanPlaneOffset( wxCommandEvent &event );
	void OnSliderFogDistance( wxCommandEvent &event );
	void OnSliderDarkness( wxCommandEvent &event );
	void OnSliderWindSpeed( wxCommandEvent &event );
	void OnSliderWindDirection( wxCommandEvent &event );
	void OnWindSpeed( wxCommandEvent &event );
	void OnWindDirection( wxCommandEvent &event );
	void OnBgColor( wxCommandEvent &event );
	void OnCheckBox( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif
