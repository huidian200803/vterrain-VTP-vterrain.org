//
// EnviroGUI.cpp
// GUI-specific functionality of the Enviro class
//
// Copyright (c) 2003-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtdata/FileFilters.h"
#include "vtdata/vtLog.h"
#include "vtui/Helper.h"
#include "vtui/InstanceDlg.h"
#include "vtui/ProfileDlg.h"

#include "EnviroGUI.h"
#include "EnviroApp.h"
#include "EnviroFrame.h"
#include "EnviroCanvas.h"
#include "../Options.h"

// dialogs
#include "DistanceDlg3d.h"
#include "FeatureTableDlg3d.h"
#include "DriveDlg.h"
#include "LayerDlg.h"
#include "StyleDlg.h"

#if WIN32 || !wxUSE_JOYSTICK
  #include "vtui/Joystick.h"
#else
  #include "wx/joystick.h"
#endif

DECLARE_APP(EnviroApp);

//
// This is a 'singleton', the only instance of the global application object
//
EnviroGUI g_App;

// helper
EnviroFrame *GetFrame()
{
	return dynamic_cast<EnviroFrame *>(wxGetApp().GetTopWindow());
}

EnviroGUI::EnviroGUI()
{
	m_pJFlyer = NULL;
}

EnviroGUI::~EnviroGUI()
{
}

void EnviroGUI::ShowPopupMenu(const IPoint2 &pos)
{
	GetFrame()->ShowPopupMenu(pos);
}

void EnviroGUI::SetTerrainToGUI(vtTerrain *pTerrain)
{
	GetFrame()->SetTerrainToGUI(pTerrain);

	if (pTerrain)
	{
		if (m_pJFlyer)
		{
			float speed = pTerrain->GetParams().GetValueFloat(STR_NAVSPEED);
			m_pJFlyer->SetSpeed(speed);
		}
		ShowMapOverview(pTerrain->GetParams().GetValueBool(STR_OVERVIEW));
		ShowCompass(pTerrain->GetParams().GetValueBool(STR_COMPASS));
	}
	else
	{
		ShowMapOverview(false);
		ShowCompass(false);
	}
}

void EnviroGUI::SetFlightSpeed(float speed)
{
	if (m_pJFlyer)
		m_pJFlyer->SetSpeed(speed);
	Enviro::SetFlightSpeed(speed);
}

const char *AppStateNames[] =
{
	"AS_Initializing",
	"AS_Neutral",
	"AS_Orbit",
	"AS_FlyingIn",
	"AS_SwitchToTerrain",
	"AS_Terrain",
	"AS_MovingOut",
	"AS_Error"
};

void EnviroGUI::SetState(AppState s)
{
	// if entering or leaving terrain or orbit state
	AppState previous = m_state;
	m_state = s;

	if (m_state != previous)
	{
		VTLOG("Changing app state from %s to %s\n", AppStateNames[previous],
			AppStateNames[m_state]);
		GetFrame()->SetTitle(wxGetApp().MakeFrameTitle(GetCurrentTerrain()));
	}

	if ((previous == AS_Terrain && m_state != AS_Terrain) ||
		(previous == AS_Orbit && m_state != AS_Orbit) ||
		(previous != AS_Terrain && m_state == AS_Terrain) ||
		(previous != AS_Orbit && m_state == AS_Orbit))
	{
		GetFrame()->RefreshToolbar();
	}

	if (s == AS_Error)
	{
		// If we encounter an error while trying to open a terrain, don't get
		//  stuck in a progress dialog.
		CloseProgressDialog2();
	}
}

vtString EnviroGUI::GetStringFromUser(const vtString &title, const vtString &msg)
{
	wxString caption(title, wxConvUTF8);
	wxString message(msg, wxConvUTF8);
	wxString str = wxGetTextFromUser(message, caption, _T(""), GetFrame());
	return (vtString) (const char *) str.mb_str(wxConvUTF8);
}

void EnviroGUI::ShowProgress(bool bShow)
{
	if (bShow)
		OpenProgressDialog2(_("Creating Terrain"), false, GetFrame());
	else
		CloseProgressDialog2();
}

void EnviroGUI::SetProgressTerrain(vtTerrain *pTerr)
{
	pTerr->SetProgressCallback(progress_callback_minor);
}

void EnviroGUI::UpdateProgress(const char *msg1, const char *msg2, int amount1, int amount2)
{
	wxString ws1(msg1, wxConvUTF8);
	wxString ws2(msg2, wxConvUTF8);

	// Try to translate the first part; a translation might be available.
	// If the string is not found in any of the loaded message catalogs,
	// the original string is returned.
	if (ws1 != _T(""))
		ws1 = wxGetTranslation(ws1);

	// Concatenate
	UpdateProgressDialog2(amount1, amount2, ws1 + ws2);
}

void EnviroGUI::AddVehicle(CarEngine *eng)
{
	GetFrame()->GetDriveDlg()->SetCarEngine(eng);
}

void EnviroGUI::RemoveVehicle(CarEngine *eng)
{
	GetFrame()->GetDriveDlg()->SetCarEngine(NULL);
}

void EnviroGUI::SetScenario(int num)
{
	GetFrame()->SetScenario(num);
}

void EnviroGUI::RefreshLayerView()
{
	LayerDlg *dlg = GetFrame()->m_pLayerDlg;
	dlg->RefreshTreeContents();
}

void EnviroGUI::UpdateLayerView()
{
	LayerDlg *dlg = GetFrame()->m_pLayerDlg;
	dlg->UpdateTreeTerrain();
}

void EnviroGUI::ShowLayerView()
{
	LayerDlg *dlg = GetFrame()->m_pLayerDlg;
	dlg->Show(true);
}

void EnviroGUI::CameraChanged()
{
	GetFrame()->CameraChanged();
}

void EnviroGUI::EarthPosUpdated()
{
	GetFrame()->EarthPosUpdated(m_EarthPos);
}

void EnviroGUI::ShowDistance(const DPoint2 &p1, const DPoint2 &p2,
							 double fGround, double fVertical)
{
	GetFrame()->m_pDistanceDlg->SetPoints(p1, p2, false);
	GetFrame()->m_pDistanceDlg->SetGroundAndVertical(fGround, fVertical, true);

	if (GetFrame()->m_pProfileDlg)
		GetFrame()->m_pProfileDlg->SetPoints(p1, p2);
}

void EnviroGUI::ShowDistance(const DLine2 &path,
							 double fGround, double fVertical)
{
	GetFrame()->m_pDistanceDlg->SetPath(path, false);
	GetFrame()->m_pDistanceDlg->SetGroundAndVertical(fGround, fVertical, true);

	if (GetFrame()->m_pProfileDlg)
		GetFrame()->m_pProfileDlg->SetPath(path);
}

vtTagArray *EnviroGUI::GetInstanceFromGUI()
{
	return GetFrame()->m_pInstanceDlg->GetTagArray();
}

bool EnviroGUI::OnMouseEvent(vtMouseEvent &event)
{
	return GetFrame()->OnMouseEvent(event);
}

void EnviroGUI::SetupScene3()
{
	GetFrame()->Setup3DScene();

#if wxUSE_JOYSTICK || WIN32
	if (g_Options.m_bUseJoystick)
	{
		m_pJFlyer = new vtJoystickEngine;
		m_pJFlyer->setName("Joystick");
		vtGetScene()->AddEngine(m_pJFlyer);
		m_pJFlyer->AddTarget(m_pNormalCamera);
	}
#endif
}

void EnviroGUI::SetTimeEngineToGUI(class vtTimeEngine *pEngine)
{
	GetFrame()->SetTimeEngine(pEngine);
}

//////////////////////////////////////////////////////////////////////

bool EnviroGUI::SaveVegetation(bool bAskFilename)
{
	VTLOG1("EnviroGUI::SaveVegetation\n");

	vtTerrain *pTerr = GetCurrentTerrain();
	vtVegLayer *vlay = pTerr->GetVegLayer();

	if (!vlay)
		return false;

	vtString fname = vlay->GetFilename();

	if (bAskFilename)
	{
		// save current directory
		wxString path = wxGetCwd();

		wxString default_file(StartOfFilename(fname), wxConvUTF8);
		wxString default_dir(ExtractPath(fname, false), wxConvUTF8);

		EnableContinuousRendering(false);
		wxFileDialog saveFile(NULL, _("Save Vegetation Data"), default_dir,
			default_file, FSTRING_VF, wxFD_SAVE);
		bool bResult = (saveFile.ShowModal() == wxID_OK);
		EnableContinuousRendering(true);
		if (!bResult)
		{
			wxSetWorkingDirectory(path);	// restore
			return false;
		}
		wxString str = saveFile.GetPath();
		fname = str.mb_str(wxConvUTF8);
		vlay->SetFilename(fname);
	}
	bool success = vlay->WriteVF(fname);
	if (success)
		vlay->SetModified(false);
	return true;
}

bool EnviroGUI::SaveStructures(bool bAskFilename)
{
	VTLOG1("EnviroGUI::SaveStructures\n");

	vtStructureLayer *st_layer = GetCurrentTerrain()->GetStructureLayer();
	vtString fname = st_layer->GetFilename();
	if (bAskFilename)
	{
		// save current directory
		wxString path = wxGetCwd();

		wxString default_file(StartOfFilename(fname), wxConvUTF8);
		wxString default_dir(ExtractPath(fname, false), wxConvUTF8);

		EnableContinuousRendering(false);
		wxFileDialog saveFile(NULL, _("Save Built Structures Data"),
			default_dir, default_file, FSTRING_VTST,
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		bool bResult = (saveFile.ShowModal() == wxID_OK);
		EnableContinuousRendering(true);
		if (!bResult)
		{
			wxSetWorkingDirectory(path);	// restore
			return false;
		}
		wxString str = saveFile.GetPath();
		fname = str.mb_str(wxConvUTF8);
		st_layer->SetFilename(fname);
	}
	bool success = false;
	try {
		success = st_layer->WriteXML(fname);
	}
	catch (xh_io_exception &e)
	{
		string str = e.getFormattedMessage();
		VTLOG("  Error: %s\n", str.c_str());
		wxMessageBox(wxString(str.c_str(), wxConvUTF8), _("Error"));
	}
	if (success)
		st_layer->SetModified(false);
	return success;
}

void EnviroGUI::OnSetDelete(vtFeatureSet *set)
{
	return GetFrame()->OnSetDelete(set);
}

bool EnviroGUI::IsAcceptable(vtTerrain *pTerr)
{
	return GetFrame()->IsAcceptable(pTerr);
}

void EnviroGUI::OnCreateTerrain(vtTerrain *pTerr)
{
	GetFrame()->OnCreateTerrain(pTerr);
}

void EnviroGUI::ShowMessage(const vtString &str)
{
	VTLOG1("EnviroGUI::ShowMessage '");
	VTLOG1(str);
	VTLOG1("'\n");

	EnableContinuousRendering(false);

	wxString str2(str, wxConvUTF8);
	wxMessageBox(str2);

	EnableContinuousRendering(true);
}

///////////////////////////////////////////////////////////////////////

#if wxUSE_JOYSTICK || WIN32

vtJoystickEngine::vtJoystickEngine()
{
	m_fSpeed = 1.0f;
	m_fLastTime = 0.0f;

	m_pStick = new wxJoystick;
	if (!m_pStick->IsOk())
	{
		delete m_pStick;
		m_pStick = NULL;
	}
}
void vtJoystickEngine::Eval()
{
	if (!m_pStick)
		return;

	float fTime = vtGetTime(), fElapsed = fTime - m_fLastTime;

	vtTransform *pTarget = (vtTransform*) GetTarget();
	if (pTarget)
	{
		wxPoint p = m_pStick->GetPosition();
		int buttons = m_pStick->GetButtonState();
		float dx = ((float)p.x / 32768) - 1.0f;
		float dy = ((float)p.y / 32768) - 1.0f;

		// use a small dead zone to avoid drift
		const float dead_zone = 0.04f;

		if (buttons & wxJOY_BUTTON2)
		{
			// move up down left right
			if (fabs(dx) > dead_zone)
				pTarget->TranslateLocal(FPoint3(dx * m_fSpeed * fElapsed, 0.0f, 0.0f));
			if (fabs(dy) > dead_zone)
				pTarget->Translate(FPoint3(0.0f, dy * m_fSpeed * fElapsed, 0.0f));
		}
		else if (buttons & wxJOY_BUTTON3)
		{
			// pitch up down, yaw left right
			if (fabs(dx) > dead_zone)
				pTarget->RotateParent(FPoint3(0,1,0), -dx * fElapsed);
			if (fabs(dy) > dead_zone)
				pTarget->RotateLocal(FPoint3(1,0,0), dy * fElapsed);
		}
		else
		{
			// move forward-backward, turn left-right
			if (fabs(dy) > dead_zone)
				pTarget->TranslateLocal(FPoint3(0.0f, 0.0f, dy * m_fSpeed * fElapsed));
			if (fabs(dx) > dead_zone)
				pTarget->RotateParent(FPoint3(0,1,0), -dx * fElapsed);
		}
	}
	m_fLastTime = fTime;
}

#endif  // wxUSE_JOYSTICK || WIN32


///////////////////////////////////////////////////////////////////////
// Helpers

vtAbstractLayer *CreateNewAbstractPointLayer(vtTerrain *pTerr, bool bAskStyle)
{
	// Make a new abstract layer (points)
	vtFeatureSetPoint2D *pSet = new vtFeatureSetPoint2D;
	pSet->SetFilename("Untitled.shp");
	pSet->AddField("Label", FT_String);
	pSet->AddField("Url", FT_String);

	// Inherit CRS.
	pSet->SetCRS(pTerr->GetCRS());

	// Ask style for the new point layer.
	vtTagArray props;
	props.SetValueBool("ObjectGeometry", false, true);
	props.SetValueBool("Labels", true, true);
	props.SetValueRGBi("LabelColor", RGBi(255,255,0), true);
	props.SetValueFloat("LabelHeight", 10.0f, true);
	props.SetValueInt("TextFieldIndex", 0, true);
	props.SetValueBool("LabelOutline", true, true);

	if (bAskStyle)
	{
		StyleDlg dlg(NULL, -1, _("Style"));
		dlg.SetFeatureSet(pSet);
		dlg.SetOptions(props);
		if (dlg.ShowModal() != wxID_OK)
		{
			delete pSet;
			return NULL;
		}
		dlg.GetOptions(props);
	}

	// wrap the features in an abstract layer
	vtAbstractLayer *ab_layer = new vtAbstractLayer;
	ab_layer->SetFeatureSet(pSet);
	ab_layer->AddProps(props);

	// Add the new layer to the terrain.
	pTerr->GetLayers().push_back(ab_layer);
	pTerr->SetActiveLayer(ab_layer);

	// Construct it once so it is set up for future visuals.
	pTerr->CreateAbstractLayerVisuals(ab_layer);

	// and show it in the layers dialog
	GetFrame()->m_pLayerDlg->RefreshTreeContents();	// full refresh

	return ab_layer;
}

vtAbstractLayer *CreateNewAbstractLineLayer(vtTerrain *pTerr, bool bAskStyle)
{
	// make a new abstract layer (points)
	vtFeatureSetLineString *pSet = new vtFeatureSetLineString;
	pSet->SetFilename("Untitled.shp");
	pSet->AddField("Elevation", FT_Float);

	// Ask style for the new point layer
	vtTagArray props;
	props.SetValueBool("LineGeometry", true, true);
	props.SetValueRGBi("LineGeomColor", RGBi(255,255,0), true);
	props.SetValueFloat("LineGeomHeight", 10.0f, true);
	props.SetValueBool("Labels", false, true);

	if (bAskStyle)
	{
		StyleDlg dlg(NULL, -1, _("Style"));
		dlg.SetFeatureSet(pSet);
		dlg.SetOptions(props);
		// Show the "Line Geometry" page, which is most relevant for a line layer
		dlg.GetNotebook()->SetSelection(1);
		if (dlg.ShowModal() != wxID_OK)
		{
			delete pSet;
			return NULL;
		}
		dlg.GetOptions(props);
	}

	// wrap the features in an abstract layer
	vtAbstractLayer *ab_layer = new vtAbstractLayer();
	ab_layer->SetFeatureSet(pSet);
	ab_layer->SetProps(props);

	// add the new layer to the terrain
	pTerr->GetLayers().push_back(ab_layer);
	pTerr->SetActiveLayer(ab_layer);

	// Construct it once so it is set up for future visuals.
	pTerr->CreateAbstractLayerVisuals(ab_layer);

	// and show it in the layers dialog
	GetFrame()->m_pLayerDlg->RefreshTreeContents();	// full refresh

	return ab_layer;
}
