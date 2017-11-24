//
// Name: CameraDlg.h
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __CameraDlg_H__
#define __CameraDlg_H__

#include "EnviroUI.h"
#include "vtui/AutoDialog.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// CameraDlg
//----------------------------------------------------------------------------

class CameraDlg: public CameraDlgBase
{
public:
	// constructors and destructors
	CameraDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for CameraDlg
	wxStaticText* GetFovText()  { return (wxStaticText*) FindWindow( ID_FOV_TEXT ); }
	wxCheckBox* GetAccel()  { return (wxCheckBox*) FindWindow( ID_ACCEL ); }
	wxSlider* GetFusionDistSlider()  { return (wxSlider*) FindWindow( ID_FUSION_DIST_SLIDER ); }
	wxTextCtrl* GetFusionDist()  { return (wxTextCtrl*) FindWindow( ID_FUSION_DIST ); }
	wxSlider* GetEyeSepSlider()  { return (wxSlider*) FindWindow( ID_EYE_SEPSLIDER ); }
	wxTextCtrl* GetEyeSep()  { return (wxTextCtrl*) FindWindow( ID_EYE_SEP ); }
	wxSlider* GetFovSlider()  { return (wxSlider*) FindWindow( ID_FOVSLIDER ); }
	wxTextCtrl* GetFov()  { return (wxTextCtrl*) FindWindow( ID_FOV ); }
	wxChoice* GetSpeedUnits()  { return (wxChoice*) FindWindow( ID_SPEED_UNITS ); }
	void SlidersToValues(int w);
	void ValuesToSliders();
	void GetValues();
	void SetValues();
	void TransferToWindow();
	void SetSliderControls();

	void SetTerrain(class vtTerrain *pTerr) { m_pTerrain = pTerr; }
	void CameraChanged();
	void CheckAndUpdatePos();

private:
	// WDR: member variable declarations for CameraDlg
	wxString m_camX, m_camY, m_camZ, m_camWidth;

	int m_iFov;
	int m_iNear;
	int m_iFar;
	int m_iEyeSep;
	int m_iFusionDist;
	int m_iSpeed;
	int m_iDamping;
	int m_iDistVeg;
	int m_iDistStruct;
	int m_iDistRoad;

	bool m_bAccel;
	int m_iSpeedUnits;

	float m_fFov;
	float m_fNear;
	float m_fFar;
	float m_fEyeSep;
	float m_fFusionDist;
	float m_fSpeed;
	float m_fDamping;
	float m_fDistVeg;
	float m_fDistStruct;
	float m_fDistRoad;

	bool m_bSet;
	DPoint3 m_EarthPos;
	bool m_bOrtho;
	class vtTerrain *m_pTerrain;

private:
	// WDR: handler declarations for CameraDlg
	void OnAccel( wxCommandEvent &event );
	void OnSpeedUnits( wxCommandEvent &event );
	void OnSpeedSlider( wxCommandEvent &event );
	void OnDampingSlider( wxCommandEvent &event );
	void OnFarSlider( wxCommandEvent &event );
	void OnEyeSepSlider( wxCommandEvent &event );
	void OnFusionDistSlider( wxCommandEvent &event );
	void OnNearSlider( wxCommandEvent &event );
	void OnFovSlider( wxCommandEvent &event );

	void OnSliderVeg( wxCommandEvent &event );
	void OnSliderStruct( wxCommandEvent &event );
	void OnSliderRoad( wxCommandEvent &event );

	void OnText( wxCommandEvent &event );
	void OnTextEnter( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif // __CameraDlg_H__
