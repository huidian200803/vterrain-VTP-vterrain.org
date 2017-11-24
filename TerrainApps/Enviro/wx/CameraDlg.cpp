//
// Name: CameraDlg.cpp
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "CameraDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "CameraDlg.h"
#include "EnviroGUI.h"
#include "vtdata/LocalCS.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// CameraDlg
//----------------------------------------------------------------------------

// WDR: event table for CameraDlg

BEGIN_EVENT_TABLE(CameraDlg,CameraDlgBase)
	EVT_INIT_DIALOG (CameraDlg::OnInitDialog)

	EVT_TEXT_ENTER( ID_CAMX, CameraDlg::OnTextEnter )
	EVT_TEXT_ENTER( ID_CAMY, CameraDlg::OnTextEnter )
	EVT_TEXT_ENTER( ID_CAMZ, CameraDlg::OnTextEnter )

	EVT_TEXT( ID_FOV, CameraDlg::OnText )
	EVT_TEXT( ID_NEAR, CameraDlg::OnText )
	EVT_TEXT( ID_FAR, CameraDlg::OnText )
	EVT_TEXT( ID_EYE_SEP, CameraDlg::OnText )
	EVT_TEXT( ID_FUSION_DIST, CameraDlg::OnText )
	EVT_TEXT( ID_SPEED, CameraDlg::OnText )
	EVT_TEXT( ID_DAMPING, CameraDlg::OnText )

	EVT_TEXT( ID_LOD_VEG, CameraDlg::OnText )
	EVT_TEXT( ID_LOD_STRUCT, CameraDlg::OnText )
	EVT_TEXT( ID_LOD_ROAD, CameraDlg::OnText )

	EVT_SLIDER( ID_FOVSLIDER, CameraDlg::OnFovSlider )
	EVT_SLIDER( ID_NEARSLIDER, CameraDlg::OnNearSlider )
	EVT_SLIDER( ID_FARSLIDER, CameraDlg::OnFarSlider )
	EVT_SLIDER( ID_EYE_SEPSLIDER, CameraDlg::OnEyeSepSlider )
	EVT_SLIDER( ID_FUSION_DIST_SLIDER, CameraDlg::OnFusionDistSlider )
	EVT_SLIDER( ID_SPEEDSLIDER, CameraDlg::OnSpeedSlider )
	EVT_SLIDER( ID_DAMPINGSLIDER, CameraDlg::OnDampingSlider )

	EVT_SLIDER( ID_SLIDER_VEG, CameraDlg::OnSliderVeg )
	EVT_SLIDER( ID_SLIDER_STRUCT, CameraDlg::OnSliderStruct )
	EVT_SLIDER( ID_SLIDER_ROAD, CameraDlg::OnSliderRoad )
	EVT_CHOICE( ID_SPEED_UNITS, CameraDlg::OnSpeedUnits )
	EVT_CHECKBOX( ID_ACCEL, CameraDlg::OnAccel )
END_EVENT_TABLE()

CameraDlg::CameraDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	CameraDlgBase( parent, id, title, position, size, style )
{
	m_iSpeedUnits = 0;
	m_bSet = true;
	m_pTerrain = NULL;

	AddValidator(this, ID_CAMX, &m_camX);
	AddValidator(this, ID_CAMY, &m_camY);
	AddValidator(this, ID_CAMZ, &m_camZ);

	AddNumValidator(this, ID_FOV, &m_fFov);
	AddNumValidator(this, ID_NEAR, &m_fNear);
	AddNumValidator(this, ID_FAR, &m_fFar);
	AddNumValidator(this, ID_EYE_SEP, &m_fEyeSep);
	AddNumValidator(this, ID_FUSION_DIST, &m_fFusionDist);
	AddNumValidator(this, ID_SPEED, &m_fSpeed);
	AddNumValidator(this, ID_DAMPING, &m_fDamping, 1);

	GetSpeedUnits()->Append(_("Meters/sec"));
	GetSpeedUnits()->Append(_("Km/hour"));
	GetSpeedUnits()->Append(_("Miles/hour"));
	AddValidator(this, ID_SPEED_UNITS, &m_iSpeedUnits);
	AddValidator(this, ID_ACCEL, &m_bAccel);

	AddNumValidator(this, ID_LOD_VEG, &m_fDistVeg);
	AddNumValidator(this, ID_LOD_STRUCT, &m_fDistStruct);
	AddNumValidator(this, ID_LOD_ROAD, &m_fDistRoad);

	AddValidator(this, ID_FOVSLIDER, &m_iFov);
	AddValidator(this, ID_NEARSLIDER, &m_iNear);
	AddValidator(this, ID_FARSLIDER, &m_iFar);
	AddValidator(this, ID_EYE_SEPSLIDER, &m_iEyeSep);
	AddValidator(this, ID_FUSION_DIST_SLIDER, &m_iFusionDist);
	AddValidator(this, ID_SPEEDSLIDER, &m_iSpeed);
	AddValidator(this, ID_DAMPINGSLIDER, &m_iDamping);

	AddValidator(this, ID_SLIDER_VEG, &m_iDistVeg);
	AddValidator(this, ID_SLIDER_STRUCT, &m_iDistStruct);
	AddValidator(this, ID_SLIDER_ROAD, &m_iDistRoad);

	GetSizer()->SetSizeHints(this);
}

#define FOV_MIN	2.0f
#define FOV_RANGE	128.0f
#define WIDTH_MIN	0.0f
#define WIDTH_MAX	6.0f
#define WIDTH_RANGE	(WIDTH_MAX-(WIDTH_MIN))
#define CLIP_MIN	0.0f
#define CLIP_MAX	6.0f
#define CLIP_RANGE	(CLIP_MAX-(CLIP_MIN)) // 1.0 to 1000000 meters
#define SPEED_MIN	-1.0f
#define SPEED_MAX	4.0f
#define SPEED_RANGE	(SPEED_MAX-(SPEED_MIN)) // 0.1 to 10000 meters/sec
#define DIST_MIN	1.0f
#define DIST_MAX	5.0f
#define DIST_RANGE	(DIST_MAX-(DIST_MIN)) // 10 to 100000 meters
#define SEP_MIN		-2.5f
#define SEP_MAX		-0.5f
#define SEP_RANGE	(SEP_MAX-(SEP_MIN))
#define DAMP_MIN	0.5f
#define DAMP_RANGE	19.5f

void CameraDlg::SlidersToValues(int id)
{
	if (id == ID_FOV)
	{
		if (m_bOrtho)
			m_fFov = powf(10, (WIDTH_MIN + m_iFov * WIDTH_RANGE / 100));
		else
			m_fFov = FOV_MIN + (m_iFov * FOV_RANGE / 100);
	}
	if (id == ID_NEAR)		m_fNear =   powf(10, (CLIP_MIN + m_iNear * CLIP_RANGE / 100));
	if (id == ID_FAR)		m_fFar =	 powf(10, (CLIP_MIN + m_iFar * CLIP_RANGE / 100));
	if (id == ID_EYE_SEP)	m_fEyeSep = powf(10, (SEP_MIN + m_iEyeSep * SEP_RANGE / 100));
	if (id == ID_FUSION_DIST)m_fFusionDist = powf(10, (DIST_MIN + m_iFusionDist * DIST_RANGE / 100));
	if (id == ID_SPEED)		m_fSpeed =  powf(10, (SPEED_MIN + m_iSpeed * SPEED_RANGE / 100));
	if (id == ID_DAMPING)	m_fDamping =  DAMP_MIN + (m_iDamping * DAMP_RANGE / 100);

	if (id == ID_LOD_VEG)	 m_fDistVeg =	 powf(10, (DIST_MIN + m_iDistVeg * DIST_RANGE / 100));
	if (id == ID_LOD_STRUCT) m_fDistStruct = powf(10, (DIST_MIN + m_iDistStruct * DIST_RANGE / 100));
	if (id == ID_LOD_ROAD)	 m_fDistRoad =   powf(10, (DIST_MIN + m_iDistRoad * DIST_RANGE / 100));

	// safety check to prevent user from putting Near > Far
	if (m_fNear >= m_fFar)
		m_fNear = m_fFar -1;
}

void CameraDlg::ValuesToSliders()
{
	if (m_bOrtho)
		m_iFov =	(int) ((log10f(m_fFov) - WIDTH_MIN) / WIDTH_RANGE * 100);
	else
		m_iFov =	(int) ((m_fFov - FOV_MIN) / FOV_RANGE * 100);

	m_iNear =		(int) ((log10f(m_fNear) - CLIP_MIN) / CLIP_RANGE * 100);
	m_iFar =		(int) ((log10f(m_fFar) - CLIP_MIN) / CLIP_RANGE * 100);
	m_iEyeSep =		(int) ((log10f(m_fEyeSep) - SEP_MIN) / SEP_RANGE * 100);
	m_iFusionDist =	(int) ((log10f(m_fFusionDist) - DIST_MIN) / DIST_RANGE * 100);
	m_iSpeed =		(int) ((log10f(m_fSpeed) - SPEED_MIN) / SPEED_RANGE * 100);
	m_iDamping =	(int) ((m_fDamping - DAMP_MIN) / DAMP_RANGE * 100);

	m_iDistVeg =	(int) ((log10f(m_fDistVeg) - DIST_MIN) / DIST_RANGE * 100);
	m_iDistStruct = (int) ((log10f(m_fDistStruct) - DIST_MIN) / DIST_RANGE * 100);
	m_iDistRoad =   (int) ((log10f(m_fDistRoad) - DIST_MIN) / DIST_RANGE * 100);
}

void CameraDlg::GetValues()
{
	vtScene *scene = vtGetScene();
	vtCamera *pCam = scene->GetCamera();

	m_bOrtho = pCam->IsOrtho();
	m_fNear = pCam->GetHither();
	m_fFar = pCam->GetYon();

	float speed = g_App.GetFlightSpeed();
	switch (m_iSpeedUnits)
	{
	case 0: // m/s
		m_fSpeed = speed;
		break;
	case 1: // Km/h
		m_fSpeed = speed / 1000 * (60*60);  // m -> km, sec -> hour
		break;
	case 2: // Miles/h
		m_fSpeed = speed / 1000 * (60*60);  // m -> km, sec -> hour
		m_fSpeed /= 1.609347;   // km -> miles
		break;
	}
	m_bAccel = g_App.GetFlightAccel();
	m_fDamping = g_App.GetNavDamping();

	if (m_pTerrain)
	{
		m_fDistVeg =	m_pTerrain->GetLODDistance(TFT_VEGETATION);
		m_fDistStruct = m_pTerrain->GetLODDistance(TFT_STRUCTURES);
		m_fDistRoad =   m_pTerrain->GetLODDistance(TFT_ROADS);
	}
	if (m_bOrtho)
	{
		m_fFov = pCam->GetWidth();
		GetFovText()->SetLabel(_("View Width (meters)"));
		GetAccel()->Enable(false);
	}
	else
	{
		m_fFov = pCam->GetFOV() * 180.0f / PIf;
		GetFovText()->SetLabel(_("Horizontal FOV (degrees)"));
		GetAccel()->Enable(true);
	}
	bool bStereo = scene->IsStereo();
	GetEyeSepSlider()->Enable(bStereo);
	GetEyeSep()->Enable(bStereo);
	GetFusionDistSlider()->Enable(bStereo);
	GetFusionDist()->Enable(bStereo);
	m_fEyeSep = scene->GetStereoSeparation();
	m_fFusionDist = scene->GetStereoFusionDistance();
}

void CameraDlg::SetValues()
{
	if (!m_bSet)
		return;

	vtCamera *pCam = vtGetScene()->GetCamera();
	pCam->SetHither(m_fNear);
	pCam->SetYon(m_fFar);

	if (m_bOrtho)
		pCam->SetWidth(m_fFov);
	else
		pCam->SetFOV(m_fFov / 180.0f * PIf);
	float speed=0;
	switch (m_iSpeedUnits)
	{
	case 0: // m/s
		speed = m_fSpeed;
		break;
	case 1: // Km/h
		speed = m_fSpeed * 1000 / (60*60);  // km -> m, hour -> sec
		break;
	case 2: // Miles/h
		speed = m_fSpeed * 1000 / (60*60);  // km -> m, hour -> sec
		speed *= 1.609347;   // miles -> km
		break;
	}
	g_App.SetFlightSpeed(speed);
	g_App.SetFlightAccel(m_bAccel);
	g_App.SetNavDamping(m_fDamping);

	if (m_pTerrain)
	{
		m_pTerrain->SetLODDistance(TFT_VEGETATION, m_fDistVeg);
		m_pTerrain->SetLODDistance(TFT_STRUCTURES, m_fDistStruct);
		m_pTerrain->SetLODDistance(TFT_ROADS, m_fDistRoad);
	}
	vtGetScene()->SetStereoSeparation(m_fEyeSep);
	vtGetScene()->SetStereoFusionDistance(m_fFusionDist);
}

void CameraDlg::CameraChanged()
{
	// we are dealing with a new camera, so update with its values
	GetValues();
	ValuesToSliders();
	TransferToWindow();
}

void CameraDlg::CheckAndUpdatePos()
{
	vtCamera *cam = vtGetScene()->GetCamera();
	const FPoint3 WorldPos = cam->GetTrans();
	m_pTerrain->GetLocalCS().LocalToEarth(WorldPos, m_EarthPos);

	bool bTransfer = false;
	wxString newx, newy, newz;
	newx.Printf(_T("%.7g"), m_EarthPos.x);
	newy.Printf(_T("%.7g"), m_EarthPos.y);
	newz.Printf(_T("%.7g"), m_EarthPos.z);
	if (newx != m_camX || newy != m_camY || newz != m_camZ)
	{
		// new to refresh values
		m_camX = newx;
		m_camY = newy;
		m_camZ = newz;
		bTransfer = true;
	}
	if (m_bOrtho)
	{
		float newWidth = cam->GetWidth();
		if (newWidth != m_fFov)
		{
			m_fFov = newWidth;
			ValuesToSliders();
			bTransfer = true;
		}
	}
	if (bTransfer)
		TransferToWindow();
}

void CameraDlg::TransferToWindow()
{
	m_bSet = false;
	TransferDataToWindow();
	m_bSet = true;
}

void CameraDlg::SetSliderControls()
{
	FindWindow(ID_FOVSLIDER)->GetValidator()->TransferToWindow();
	FindWindow(ID_NEARSLIDER)->GetValidator()->TransferToWindow();
	FindWindow(ID_FARSLIDER)->GetValidator()->TransferToWindow();
	FindWindow(ID_EYE_SEPSLIDER)->GetValidator()->TransferToWindow();
	FindWindow(ID_SPEEDSLIDER)->GetValidator()->TransferToWindow();
	FindWindow(ID_DAMPINGSLIDER)->GetValidator()->TransferToWindow();

	FindWindow(ID_SLIDER_VEG)->GetValidator()->TransferToWindow();
	FindWindow(ID_SLIDER_STRUCT)->GetValidator()->TransferToWindow();
	FindWindow(ID_SLIDER_ROAD)->GetValidator()->TransferToWindow();
}

// WDR: handler implementations for CameraDlg

void CameraDlg::OnAccel( wxCommandEvent &event )
{
	TransferDataFromWindow();
	SetValues();
}

void CameraDlg::OnSpeedUnits( wxCommandEvent &event )
{
	TransferDataFromWindow();
	GetValues();
	ValuesToSliders();
	TransferToWindow();
}

void CameraDlg::OnInitDialog(wxInitDialogEvent& event)
{
	GetValues();
	ValuesToSliders();
	m_bSet = false;
	wxDialog::OnInitDialog(event);  // calls TransferDataToWindow
	m_bSet = true;
}

void CameraDlg::OnFovSlider( wxCommandEvent &event )
{
	if (!m_bSet)
		return;
	TransferDataFromWindow();
	SlidersToValues(ID_FOV);
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnNearSlider( wxCommandEvent &event )
{
	if (!m_bSet)
		return;
	TransferDataFromWindow();
	SlidersToValues(ID_NEAR);
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnFarSlider( wxCommandEvent &event )
{
	if (!m_bSet)
		return;
	TransferDataFromWindow();
	SlidersToValues(ID_FAR);
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnEyeSepSlider( wxCommandEvent &event )
{
	if (!m_bSet)
		return;
	TransferDataFromWindow();
	SlidersToValues(ID_EYE_SEP);
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnFusionDistSlider( wxCommandEvent &event )
{
	if (!m_bSet)
		return;
	TransferDataFromWindow();
	SlidersToValues(ID_FUSION_DIST);
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnSpeedSlider( wxCommandEvent &event )
{
	if (!m_bSet)
		return;
	TransferDataFromWindow();
	SlidersToValues(ID_SPEED);
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnDampingSlider( wxCommandEvent &event )
{
	if (!m_bSet)
		return;
	TransferDataFromWindow();
	SlidersToValues(ID_DAMPING);
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnSliderVeg( wxCommandEvent &event )
{
	if (!m_bSet)
		return;
	TransferDataFromWindow();
	SlidersToValues(ID_LOD_VEG);
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnSliderStruct( wxCommandEvent &event )
{
	if (!m_bSet)
		return;
	TransferDataFromWindow();
	SlidersToValues(ID_LOD_STRUCT);
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnSliderRoad( wxCommandEvent &event )
{
	if (!m_bSet)
		return;
	TransferDataFromWindow();
	SlidersToValues(ID_LOD_ROAD);
	SetValues();
	TransferToWindow();
}

void CameraDlg::OnText( wxCommandEvent &event )
{
	if (!m_bSet)
		return;
	TransferDataFromWindow();
	ValuesToSliders();
	SetValues();
	SetSliderControls();
}

void CameraDlg::OnTextEnter( wxCommandEvent &event )
{
	TransferDataFromWindow();

	m_EarthPos.x = atof(m_camX.mb_str(wxConvUTF8));
	m_EarthPos.y = atof(m_camY.mb_str(wxConvUTF8));
	m_EarthPos.z = atof(m_camZ.mb_str(wxConvUTF8));

	FPoint3 WorldPos;
	m_pTerrain->GetLocalCS().EarthToLocal(m_EarthPos, WorldPos);
	vtGetScene()->GetCamera()->SetTrans(WorldPos);
}

