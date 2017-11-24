//
// Name: LocationDlg.cpp
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "LocationDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtlib/vtosg/ScreenCaptureHandler.h"

#include "vtdata/FileFilters.h"
#include "vtdata/FilePath.h"
#include "vtdata/vtLog.h"
#include "vtui/Helper.h"

#include "EnviroCanvas.h"		// for EnableContinuousRendering
#include "LocationDlg.h"

void BlockingMessageBox(const wxString &msg)
{
	EnableContinuousRendering(false);
	wxMessageBox(msg);
	EnableContinuousRendering(true);
}

// WDR: class implementations

//----------------------------------------------------------------------------
// LocationDlg
//----------------------------------------------------------------------------

// WDR: event table for LocationDlg

BEGIN_EVENT_TABLE(LocationDlg,LocationDlgBase)
	EVT_LISTBOX( ID_LOCLIST, LocationDlg::OnLocList )
	EVT_BUTTON( ID_RECALL, LocationDlg::OnRecall )
	EVT_BUTTON( ID_STORE, LocationDlg::OnStore )
	EVT_BUTTON( ID_STOREAS, LocationDlg::OnStoreAs )
	EVT_BUTTON( ID_SAVE, LocationDlg::OnSave )
	EVT_BUTTON( ID_LOAD, LocationDlg::OnLoad )
	EVT_LISTBOX_DCLICK( ID_LOCLIST, LocationDlg::OnListDblClick )
	EVT_BUTTON( ID_REMOVE, LocationDlg::OnRemove )

	EVT_BUTTON( ID_NEW_ANIM, LocationDlg::OnNewAnim )
	EVT_BUTTON( ID_SAVE_ANIM, LocationDlg::OnSaveAnim )
	EVT_BUTTON( ID_LOAD_ANIM, LocationDlg::OnLoadAnim )
	EVT_BUTTON( ID_PLAY, LocationDlg::OnPlay )
	EVT_BUTTON( ID_RECORD1, LocationDlg::OnRecord1 )
	EVT_BUTTON( ID_STOP, LocationDlg::OnStop )
	EVT_BUTTON( ID_PLAY_TO_DISK, LocationDlg::OnPlayToDisk )
	EVT_CHECKBOX( ID_LOOP, LocationDlg::OnCheckbox )
	EVT_CHECKBOX( ID_CONTINUOUS, LocationDlg::OnCheckbox )
	EVT_CHECKBOX( ID_SMOOTH, LocationDlg::OnCheckbox )
	EVT_CHECKBOX( ID_POS_ONLY, LocationDlg::OnCheckbox )
	EVT_BUTTON( ID_RESET, LocationDlg::OnReset )
	EVT_SLIDER( ID_SPEEDSLIDER, LocationDlg::OnSpeedSlider )
	EVT_TEXT( ID_SPEED, LocationDlg::OnText )
	EVT_RADIOBUTTON( ID_RECORD_LINEAR, LocationDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RECORD_INTERVAL, LocationDlg::OnRadio )
	EVT_CHECKBOX( ID_ACTIVE, LocationDlg::OnActive )
	EVT_SLIDER( ID_ANIM_POS, LocationDlg::OnAnimPosSlider )
	EVT_TREE_SEL_CHANGED( ID_ANIMTREE, LocationDlg::OnTreeSelChanged )
	EVT_TREE_KEY_DOWN( ID_ANIMTREE, LocationDlg::OnTreeKeyDown )
END_EVENT_TABLE()

LocationDlg::LocationDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	LocationDlgBase( parent, id, title, position, size, style )
{
	m_bActive = true;
	m_bLoop = true;
	m_bContinuous = false;
	m_bSmooth = true;
	m_bPosOnly = false;
	m_iAnim = -1;
	m_iPos = 0;
	m_fSpeed = 1.0f;
	m_bSetting = false;
	m_fRecordSpacing = 1.0f;
	m_bRecordLinear = true;
	m_bRecordInterval = false;

	m_pSaver = NULL;
	m_pAnimPaths = NULL;

	m_pLocList = GetLoclist();

	// Work around wxFormDesigner's lack of support for limiting to smallest size
	GetSizer()->SetSizeHints(this);

	AddValidator(this, ID_ACTIVE, &m_bActive);
	AddValidator(this, ID_LOOP, &m_bLoop);
	AddValidator(this, ID_CONTINUOUS, &m_bContinuous);
	AddValidator(this, ID_SMOOTH, &m_bSmooth);
	AddValidator(this, ID_POS_ONLY, &m_bPosOnly);
	AddNumValidator(this, ID_SPEED, &m_fSpeed, 2);
	AddValidator(this, ID_SPEEDSLIDER, &m_iSpeed);
	AddNumValidator(this, ID_RECORD_SPACING, &m_fRecordSpacing);
	AddValidator(this, ID_ANIM_POS, &m_iPos);

	AddValidator(this, ID_RECORD_LINEAR, &m_bRecordLinear);
	AddValidator(this, ID_RECORD_INTERVAL, &m_bRecordInterval);

	RefreshButtons();
	UpdateEnabling();
}

LocationDlg::~LocationDlg()
{
}

void LocationDlg::SetLocSaver(vtLocationSaver *saver)
{
	m_pSaver = saver;
	RefreshList();
	RefreshButtons();
}

void LocationDlg::SetAnimContainer(vtAnimContainer *ac)
{
	m_pAnimPaths = ac;
	m_iAnim = -1;
	RefreshAnims();
	UpdateSlider();
	UpdateEnabling();
}

void LocationDlg::Update()
{
	RefreshAnimsText();
	UpdateSlider();
}

void LocationDlg::RefreshList()
{
	m_pLocList->Clear();
	if (!m_pSaver)
		return;

	wxString str;
	int num = m_pSaver->NumLocations();
	for (int i = 0; i < num; i++)
	{
		vtLocation *loc = m_pSaver->GetLocation(i);

		str.Printf(_T("%d. %ls"), i+1, loc->m_strName.c_str());
		m_pLocList->Append(str);
	}
}

void LocationDlg::RefreshAnims()
{
	GetAnimTree()->DeleteAllItems();
	m_root = GetAnimTree()->AddRoot(_T("Root"));

	if (!m_pAnimPaths)
		return;

	wxString str;
	uint i, num = m_pAnimPaths->size();

	for (i = 0; i < num; i++)
	{
		wxTreeItemId id = GetAnimTree()->AppendItem(m_root, _T("anim"));

		vtAnimPath *anim = GetAnim(i);
		vtAnimPath::TimeControlPointMap &tcm = anim->GetTimeControlPointMap();
		vtAnimPath::TimeControlPointMap::iterator iter;

		int count = 0;
		for (iter = tcm.begin(); iter != tcm.end(); iter++)
		{
			anim->GetFirstTime();
			str.Printf(_("%d: time %lf"), count, iter->first);
			GetAnimTree()->AppendItem(id, str);
			count++;
		}
	}

	RefreshAnimsText();
}

void LocationDlg::RefreshAnimsText()
{
	wxTreeItemIdValue cookie;
	wxTreeItemId id;

	uint i, num = m_pAnimPaths->size();
	for (i = 0; i < num; i++)
	{
		vtAnimEntry &entry = m_pAnimPaths->at(i);
		vtAnimPath *anim = GetAnim(i);
		vtAnimPathEngine *eng = GetEngine(i);

		if (id.IsOk())
			id = GetAnimTree()->GetNextChild(m_root, cookie);
		else
			id = GetAnimTree()->GetFirstChild(m_root, cookie);

		wxString str(entry.m_Name, wxConvUTF8);
		wxString str2;
		str2.Printf(_T(" (%.1f/%.1f, %d)"), eng->GetTime(),
			(float) anim->GetLastTime(), anim->NumPoints());
		str += str2;
		GetAnimTree()->SetItemText(id, str);
	}
}

void LocationDlg::UpdateSlider()
{
	if (m_iAnim != -1)
	{
		// time slider
		float fTotalTime = GetAnim(m_iAnim)->GetLastTime();
		float fTime = GetEngine(m_iAnim)->GetTime();
		m_iPos = (int) (fTime / fTotalTime * 1000);
		GetAnimPos()->SetValue(m_iPos);
	}
}

void LocationDlg::UpdateEnabling()
{
	GetAnimPos()->Enable(m_iAnim != -1);
	GetSaveAnim()->Enable(m_iAnim != -1);
	GetActive()->Enable(m_iAnim != -1);
	GetSpeed()->Enable(m_iAnim != -1);
	GetReset()->Enable(m_iAnim != -1);
	GetPlay()->Enable(m_iAnim != -1);
	GetRecord1()->Enable(m_iAnim != -1);
	GetStop()->Enable(m_iAnim != -1);
	GetLoop()->Enable(m_iAnim != -1);
	GetContinuous()->Enable(m_iAnim != -1);
	GetSmooth()->Enable(m_iAnim != -1 && GetAnim(m_iAnim)->NumPoints() > 2);
	GetPosOnly()->Enable(m_iAnim != -1);

	GetRecordInterval()->Enable(m_iAnim != -1);
	GetRecordLinear()->Enable(m_iAnim != -1);

	GetRecordSpacing()->Enable(m_bRecordInterval);
	GetPlayToDisk()->Enable(m_iAnim != -1);
}

void LocationDlg::AppendAnimPath(vtAnimPath *anim, const char *name)
{
	vtAnimPathEngine *engine = new vtAnimPathEngine(anim);
	engine->setName("AnimPathEngine");
	engine->AddTarget(m_pSaver->GetTransform());
	engine->SetEnabled(false);

	vtAnimEntry entry;
	entry.m_pAnim = anim;
	entry.m_pEngine = engine;
	entry.m_Name = name;
	m_pAnimPaths->AppendEntry(entry);
}

vtAnimPath *LocationDlg::CreateAnimPath()
{
	vtAnimPath *anim = new vtAnimPath;

	// Ensure that anim knows the CRS.
	anim->SetCRS(m_pSaver->GetAtCRS(), m_pSaver->GetLocalCS());

	return anim;
}

#define SPEED_MIN   0.0f
#define SPEED_MAX   4.0f
#define SPEED_RANGE (SPEED_MAX-(SPEED_MIN)) // 1 to 10000 meters/sec

void LocationDlg::SlidersToValues()
{
	m_fSpeed =  powf(10, (SPEED_MIN + m_iSpeed * SPEED_RANGE / 100));
}

void LocationDlg::ValuesToSliders()
{
	m_iSpeed =  (int) ((log10f(m_fSpeed) - SPEED_MIN) / SPEED_RANGE * 100);
}

void LocationDlg::SetValues(int which)
{
	if (m_iAnim == -1)
		return;

	vtAnimPath *anim = GetAnim(m_iAnim);
	if (which & PF_LOOP)
		anim->SetLoop(m_bLoop);
	if (which & PF_INTERP)
		anim->SetInterpMode(m_bSmooth ? vtAnimPath::CUBIC_SPLINE : vtAnimPath::LINEAR);
	vtAnimPathEngine *engine = GetEngine(m_iAnim);
	if (which & PF_CONTIN)
		engine->SetContinuous(m_bContinuous);
	if (which & PF_POSONLY)
		engine->SetPosOnly(m_bPosOnly);
//  engine->SetEnabled(m_bActive);
	if (which & PF_SPEED)
		engine->SetSpeed(m_fSpeed);
}

void LocationDlg::GetValues()
{
	if (m_iAnim == -1)
		return;

	vtAnimPath *anim = GetAnim(m_iAnim);
	m_bSmooth = (anim->GetInterpMode() == vtAnimPath::CUBIC_SPLINE);
	m_bLoop = anim->GetLoop();

	vtAnimPathEngine *engine = GetEngine(m_iAnim);
	m_bContinuous = engine->GetContinuous();
	m_bPosOnly = engine->GetPosOnly();
	m_fSpeed = engine->GetSpeed();
//  m_bActive = engine->GetEnabled();
}

void LocationDlg::TransferToWindow()
{
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}


// WDR: handler implementations for LocationDlg

void LocationDlg::OnTreeKeyDown( wxTreeEvent &event )
{
	if (event.GetKeyCode() != WXK_DELETE)
	{
		event.Skip();
		return;
	}
	wxTreeItemId selid = m_current;
	wxTreeItemId parent = GetAnimTree()->GetItemParent(selid);
	if (parent == m_root)
	{
		// delete anim
		m_pAnimPaths->erase(m_pAnimPaths->begin() + m_iAnim);
		m_iAnim = -1;
		RefreshAnims();
		UpdateEnabling();
	}
	else
	{
		// delete point
		wxTreeItemIdValue cookie;
		wxTreeItemId id;
		int count = 0;
		for (id = GetAnimTree()->GetFirstChild(parent, cookie);
			id.IsOk();
			id = GetAnimTree()->GetNextChild(parent, cookie))
		{
			if (id == selid)
				break;
			count++;
		}

		vtAnimPath *anim = GetAnim(m_iAnim);
		anim->RemovePoint(count);

		GetAnimTree()->Delete(m_current);
	}
}

void LocationDlg::OnTreeSelChanged( wxTreeEvent &event )
{
	int previous = m_iAnim;

	m_current = event.GetItem();
	wxTreeItemId selid = m_current;
	if (selid.IsOk())
	{
		// If they click on a subitem (point), look at its parent
		wxTreeItemId parent = GetAnimTree()->GetItemParent(selid);
		if (parent != m_root)
			selid = parent;

		// Look through the tree to find the index
		wxTreeItemIdValue cookie;
		wxTreeItemId id;
		int count = 0;
		for (id = GetAnimTree()->GetFirstChild(m_root, cookie);
			id.IsOk();
			id = GetAnimTree()->GetNextChild(m_root, cookie))
		{
			if (id == selid)
				m_iAnim = count;
			count++;
		}
	}
	else
		m_iAnim = -1;

	if (previous != m_iAnim)
	{
		UpdateEnabling();
		GetValues();
		UpdateSlider();
		ValuesToSliders();
		TransferToWindow();
	}
}

void LocationDlg::OnAnimPosSlider( wxCommandEvent &event )
{
	if (m_iAnim == -1)
		return;

	TransferDataFromWindow();

	float fTotalTime = GetAnim(m_iAnim)->GetLastTime();
	vtAnimPathEngine *eng = GetEngine(m_iAnim);
	eng->SetTime(m_iPos * fTotalTime / 1000);
	if (m_bActive)
		eng->UpdateTargets();
}

void LocationDlg::OnActive( wxCommandEvent &event )
{
	TransferDataFromWindow();
}

void LocationDlg::OnRadio( wxCommandEvent &event )
{
	TransferDataFromWindow();
	UpdateEnabling();
}

void LocationDlg::OnText( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	ValuesToSliders();
	SetValues(PF_SPEED);
	FindWindow(ID_SPEEDSLIDER)->GetValidator()->TransferToWindow();
}

void LocationDlg::OnSpeedSlider( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	SlidersToValues();
	SetValues(PF_SPEED);
	TransferToWindow();
}

void LocationDlg::OnReset( wxCommandEvent &event )
{
	vtAnimPathEngine *engine = GetEngine(m_iAnim);
	engine->Reset();
	if (m_bActive)
		engine->UpdateTargets();
}

void LocationDlg::OnCheckbox( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	SetValues(PF_ALL);
	RefreshAnimsText();
}

void LocationDlg::OnStop( wxCommandEvent &event )
{
	vtAnimPathEngine *engine = GetEngine(m_iAnim);
	engine->SetEnabled(false);
}

void LocationDlg::OnRecord1( wxCommandEvent &event )
{
	TransferDataFromWindow();

	vtAnimPathEngine *engine = GetEngine(m_iAnim);
	vtTransform *xform = (vtTransform *) engine->GetTarget();
	vtAnimPath *path = engine->GetAnimationPath();

	FPoint3 pos = xform->GetTrans();
	FQuat rot = xform->GetOrient();
	ControlPoint cp(pos, rot);

	float fTime;
	if (path->NumPoints() == 0)
		fTime = 0;
	else
	{
		if (m_bRecordInterval)
		{
			// Record Interval: Append an element whose time is the desired number
			//  of seconds after the last point.
			fTime = path->GetLastTime() + m_fRecordSpacing;
		}
		else
		{
			// RecordLinear: Append an element whose time is derived from the linear
			//  distance from the last point.
			ControlPoint &prev_cp =
			path->GetTimeControlPointMap().rbegin()->second;
			float dist = (pos - prev_cp.m_Position).Length();

			// convert directly at 1 meter/second
			fTime = path->GetLastTime() + dist;
		}
	}
	path->Insert(fTime, cp);
	path->ProcessPoints();

	wxString str;
	str.Printf(_("%d: time %lf"), path->NumPoints()-1, fTime);

	// Find the current animation
	wxTreeItemId CurrentAnimation = GetAnimTree()->GetItemParent(m_current);
	if (CurrentAnimation == m_root)
		CurrentAnimation = m_current;
	GetAnimTree()->AppendItem(CurrentAnimation, str);

	RefreshAnimsText();
	UpdateEnabling();   // Smooth might be allowed now
}

void LocationDlg::OnPlay( wxCommandEvent &event )
{
	vtAnimPathEngine *engine = GetEngine(m_iAnim);
	engine->SetEnabled(true);
}

class PlayToDiskEngine : public vtEngine
{
public:
	void Eval()
	{
		vtScene *scene = vtGetScene();
		if (step > 0)
		{
			wxString msg;
			msg.Printf(_T("Output %.2f/%.2f"), fStep * step, fTotal);
			if (UpdateProgressDialog((int) (99 * fStep * step / fTotal), msg) == true)
			{
				// user pressed cancel
				scene->SetPostDrawEngine(NULL);
				CloseProgressDialog();
				return;
			}

			// We can't grab the screen directly, we must use an OSG callback
			// to capture after the next draw.
			vtString fname;
			fname.Format("image_%04d.png", step-1);

			std::string Filename = (const char *)(directory+fname);
			CScreenCaptureHandler::SetupScreenCapture(Filename);
		}
		// Show the next frame time
		engine->SetTime(fStep * step);
		engine->UpdateTargets();

		// Advance to next frame
		step++;
		if (fStep * step > fTotal)
		{
			// We're finished
			scene->SetPostDrawEngine(NULL);
			CloseProgressDialog();
			return;
		}
	}
	bool bReady;
	vtString directory;
	int step;
	float fStep, fTotal;
	vtAnimPathEngine *engine;
};

void LocationDlg::OnPlayToDisk( wxCommandEvent &event )
{
	if (m_iAnim == -1)
	{
		wxMessageBox(_("Please select an animpath to record."));
		return;
	}

	// Ask for directory to place the images
	wxDirDialog getDir(NULL, _("Output directory for the images"));
	bool bResult = (getDir.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	wxString dir = getDir.GetPath();

	// Make sure there is a trailing slash
	if (dir.Length() > 1)
	{
		char ch = dir.GetChar(dir.Length()-1);
		if (ch != '/' && ch != '\\')
			dir += _T("/");
	}

	// Ask for unit of (animation) time for each frame
	wxString value;
	value.Printf(_T("%.1f"), 0.1f);
	wxString step = wxGetTextFromUser(_("Animation time step for each frame, in seconds:"),
		_("Animation"), value, this);
	if (step == _T(""))
		return;
	float fStep = atof(step.mb_str(wxConvUTF8));

	vtAnimPathEngine *engine = GetEngine(m_iAnim);
	vtAnimPath *path = engine->GetAnimationPath();

	wxString msg;
	msg.Printf(_("The animation of %.2f seconds will be recorded as %d frames (%.2f/sec)"),
		path->GetLastTime(), (int) (path->GetLastTime()/fStep), 1.0f/fStep);
	if (wxMessageBox(msg, _("Animation"), wxOK+wxCANCEL) == wxCANCEL)
		return;

	OpenProgressDialog(_("Output"), _T(""), true, this);

	PlayToDiskEngine *eng = new PlayToDiskEngine;
	eng->bReady = false;
	eng->directory = dir.mb_str(wxConvUTF8);
	eng->step = 0;
	eng->fStep = fStep;
	eng->fTotal = path->GetLastTime();
	eng->engine = engine;

	// If we use a regular engine, it is possible (due to timing) that it
	//  will capture the window contents at the wrong time (a blank window).
	// So, we add it to the list of engines that are called _after_ draw.
	vtGetScene()->SetPostDrawEngine(eng);
}

void LocationDlg::OnLoadAnim( wxCommandEvent &event )
{
	wxString filter = _("Polyline Data Sources");
	filter += _T("|");
	AddType(filter, FSTRING_VTAP);
	AddType(filter, FSTRING_SHP);
	AddType(filter, FSTRING_DXF);
	AddType(filter, FSTRING_IGC);

	wxFileDialog loadFile(NULL, _("Load Animation Path"), _T(""), _T(""),
		filter, wxFD_OPEN);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	vtAnimPath *anim;
	bool bSuccess;
	wxString str = loadFile.GetPath();
	vtString filename = (const char *) str.mb_str(wxConvUTF8);
	if (GetExtension(filename) == ".vtap")
	{
		anim = CreateAnimPath();
		bSuccess = anim->Read(filename);
	}
	else
	{
		vtFeatureLoader loader;
		vtFeatureSet *pSet = loader.LoadFrom(filename);
		if (!pSet)
			return;
		anim = CreateAnimPath();
		bSuccess = anim->CreateFromLineString(m_pSaver->GetAtCRS(), pSet);
		delete pSet;
	}
	if (bSuccess)
	{
		AppendAnimPath(anim, filename);
		RefreshAnims();
	}
	else
		delete anim;
}

void LocationDlg::OnNewAnim( wxCommandEvent &event )
{
	vtAnimPath *anim = CreateAnimPath();
	AppendAnimPath(anim, "New Anim");
	RefreshAnims();
}

void LocationDlg::OnSaveAnim( wxCommandEvent &event )
{
	if (m_iAnim == -1)
		return;
	vtAnimPath *path = GetAnim(m_iAnim);

	wxFileDialog saveFile(NULL, _("Save AnimPath"), _T(""), _T(""),
		FSTRING_VTAP, wxFD_SAVE);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	wxString filepath = saveFile.GetPath();
	path->Write(filepath.mb_str(wxConvUTF8));
}

void LocationDlg::OnRemove( wxCommandEvent &event )
{
	int num = m_pLocList->GetSelection();
	if (num >= 0 && num < m_pSaver->NumLocations())
	   m_pSaver->Remove(num);
	RefreshList();
	RefreshButtons();
}

void LocationDlg::OnListDblClick( wxCommandEvent &event )
{
	int num = m_pLocList->GetSelection();
	if (num >= 0 && num < m_pSaver->NumLocations())
	{
		bool success = m_pSaver->RecallFrom(num);
		if (!success)
			BlockingMessageBox(_("Couldn't recall point, probably a coordinate system transformation problem."));
	}
}

void LocationDlg::OnLoad( wxCommandEvent &event )
{
	wxFileDialog loadFile(NULL, _("Load Locations"), _T(""), _T(""),
		FSTRING_LOC, wxFD_OPEN);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	wxString path = loadFile.GetPath();
	vtString upath = (const char *) path.mb_str(wxConvUTF8);
	if (m_pSaver->Read(upath))
	{
		RefreshList();
		RefreshButtons();
	}
}

void LocationDlg::OnSave( wxCommandEvent &event )
{
	wxString default_file, default_dir;

	// If the locations were previously saved/loaded from a file, use that
	//  file as the default in the Save dialog.
	vtString previous = m_pSaver->GetFilename();
	if (previous != "")
	{
		default_file = wxString(StartOfFilename(previous), wxConvUTF8);
		default_dir = wxString(ExtractPath(previous, false), wxConvUTF8);
	}

	wxFileDialog saveFile(NULL, _("Save Locations"), default_dir, default_file,
		FSTRING_LOC, wxFD_SAVE);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	wxString path = saveFile.GetPath();
	vtString upath = (const char *) path.mb_str(wxConvUTF8);
	if (!m_pSaver->Write(upath))
		return;  // couldn't write
}

void LocationDlg::OnStoreAs( wxCommandEvent &event )
{
	int num = m_pSaver->NumLocations();

	wxString str;
	str.Printf(_("Location %d"), num+1);
	wxTextEntryDialog dlg(NULL, _("Type a name for the new location:"),
		_("Location Name"), str);
	if (dlg.ShowModal() != wxID_OK)
		return;

	str = dlg.GetValue();
#if SUPPORT_WSTRING
	bool success = m_pSaver->StoreTo(num, str.wc_str());
#else
	bool success = m_pSaver->StoreTo(num, (const char *) str.ToUTF8());
#endif
	if (success)
	{
		RefreshList();
		m_pLocList->SetSelection(num);
		RefreshButtons();
	}
	else
		BlockingMessageBox(_("Couldn't store point, probably a coordinate system transformation problem."));
}

void LocationDlg::OnStore( wxCommandEvent &event )
{
	int num = m_pLocList->GetSelection();
	if (num >= 0 && num < m_pSaver->NumLocations())
	{
		bool success = m_pSaver->StoreTo(num);
		if (!success)
			BlockingMessageBox(_("Couldn't store point, probably a coordinate system transformation problem."));
	}
}

void LocationDlg::OnRecall( wxCommandEvent &event )
{
	int num = m_pLocList->GetSelection();
	if (num >= 0 && num < m_pSaver->NumLocations())
	{
		bool success = m_pSaver->RecallFrom(num);
		if (!success)
			BlockingMessageBox(_("Couldn't recall point, probably a coordinate system transformation problem."));
	}
}

void LocationDlg::OnLocList( wxCommandEvent &event )
{
	RefreshButtons();
}

void LocationDlg::RefreshButtons()
{
	int num = m_pLocList->GetSelection();
	GetStore()->Enable(num != -1);
	GetRecall()->Enable(num != -1);
	GetRemove()->Enable(num != -1);
}

void LocationDlg::RecallFrom(const char *locname)
{
	if (m_pSaver->RecallFrom(locname))
		VTLOG(" Recalled location '%s'\n", (const char *)locname);
	else
		VTLOG(" Couldn't recall location '%s'\n", (const char *)locname);
}

