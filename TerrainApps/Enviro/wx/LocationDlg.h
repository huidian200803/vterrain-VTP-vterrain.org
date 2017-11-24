//
// Name:		LocationDlg.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __LocationDlg_H__
#define __LocationDlg_H__

#include "EnviroUI.h"
#include "vtlib/core/Location.h"
#include "vtlib/core/AnimPath.h"
#include "vtui/AutoDialog.h"

class vtLocationSaver;

#define PF_LOOP	 1
#define PF_INTERP   2
#define PF_CONTIN   4
#define PF_POSONLY  8
#define PF_SPEED	16
#define PF_ALL	  (PF_LOOP|PF_INTERP|PF_CONTIN|PF_POSONLY|PF_SPEED)

// WDR: class declarations

//----------------------------------------------------------------------------
// LocationDlg
//----------------------------------------------------------------------------

class LocationDlg: public LocationDlgBase
{
public:
	// constructors and destructors
	LocationDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	~LocationDlg();

	// WDR: method declarations for LocationDlg
	wxButton* GetPlayToDisk()  { return (wxButton*) FindWindow( ID_PLAY_TO_DISK ); }
	wxTextCtrl* GetSpeed()  { return (wxTextCtrl*) FindWindow( ID_SPEED ); }
	wxTextCtrl* GetRecordSpacing()  { return (wxTextCtrl*) FindWindow( ID_RECORD_SPACING ); }
	wxRadioButton* GetRecordInterval()  { return (wxRadioButton*) FindWindow( ID_RECORD_INTERVAL ); }
	wxRadioButton* GetRecordLinear()  { return (wxRadioButton*) FindWindow( ID_RECORD_LINEAR ); }
	wxCheckBox* GetActive()  { return (wxCheckBox*) FindWindow( ID_ACTIVE ); }
	wxSlider* GetAnimPos()  { return (wxSlider*) FindWindow( ID_ANIM_POS ); }
	wxBitmapButton* GetReset()  { return (wxBitmapButton*) FindWindow( ID_RESET ); }
	wxCheckBox* GetLoop()  { return (wxCheckBox*) FindWindow( ID_LOOP ); }
	wxCheckBox* GetSmooth()  { return (wxCheckBox*) FindWindow( ID_SMOOTH ); }
	wxCheckBox* GetContinuous()  { return (wxCheckBox*) FindWindow( ID_CONTINUOUS ); }
	wxCheckBox* GetPosOnly()  { return (wxCheckBox*) FindWindow( ID_POS_ONLY ); }
	wxButton* GetSaveAnim()  { return (wxButton*) FindWindow( ID_SAVE_ANIM ); }
	wxButton* GetStop()  { return (wxButton*) FindWindow( ID_STOP ); }
	wxButton* GetRecord1()  { return (wxButton*) FindWindow( ID_RECORD1 ); }
	wxButton* GetPlay()  { return (wxButton*) FindWindow( ID_PLAY ); }
	wxTreeCtrl* GetAnimTree()  { return (wxTreeCtrl*) FindWindow( ID_ANIMTREE ); }

	wxButton* GetStoreas()  { return (wxButton*) FindWindow( ID_STOREAS ); }
	wxButton* GetStore()  { return (wxButton*) FindWindow( ID_STORE ); }
	wxButton* GetRecall()  { return (wxButton*) FindWindow( ID_RECALL ); }
	wxListBox* GetLoclist()  { return (wxListBox*) FindWindow( ID_LOCLIST ); }
	wxButton* GetRemove()  { return (wxButton*) FindWindow( ID_REMOVE ); }

	void Update();
	void DeleteItem(wxListBox *pBox);

	void RefreshList();
	void SetLocSaver(vtLocationSaver *saver);
	void SetAnimContainer(vtAnimContainer *ac);
	void RefreshButtons();
	void RecallFrom(const char *locname);

	void RefreshAnims();
	void RefreshAnimsText();
	void UpdateSlider();
	void UpdateEnabling();
	void SlidersToValues();
	void ValuesToSliders();
	void AppendAnimPath(vtAnimPath *anim, const char *name);
	vtAnimPath *CreateAnimPath();

	void SetValues(int which = PF_ALL);
	void GetValues();
	void TransferToWindow();

private:
	// WDR: member variable declarations for LocationDlg
	vtLocationSaver *m_pSaver;

	vtAnimContainer *m_pAnimPaths;

	vtAnimPath *GetAnim(int i) { return m_pAnimPaths->at(i).m_pAnim; }
	vtAnimPathEngine *GetEngine(int i) { return m_pAnimPaths->at(i).m_pEngine; }

	bool m_bActive;
	bool m_bLoop;
	bool m_bContinuous;
	bool m_bSmooth;
	bool m_bPosOnly;
	int m_iAnim;
	int m_iPos;
	float m_fSpeed;
	int m_iSpeed;
	bool m_bSetting;
	float m_fRecordSpacing;
	bool m_bRecordLinear;
	bool m_bRecordInterval;

	wxListBox* m_pLocList;
	wxTreeItemId m_root, m_current;

private:
	// WDR: handler declarations for LocationDlg
	void OnTreeKeyDown( wxTreeEvent &event );
	void OnTreeSelChanged( wxTreeEvent &event );
	void OnAnimPosSlider( wxCommandEvent &event );
	void OnActive( wxCommandEvent &event );
	void OnRadio( wxCommandEvent &event );
	void OnText( wxCommandEvent &event );
	void OnSpeedSlider( wxCommandEvent &event );
	void OnReset( wxCommandEvent &event );
	void OnCheckbox( wxCommandEvent &event );
	void OnStop( wxCommandEvent &event );
	void OnPlayToDisk( wxCommandEvent &event );
	void OnRecord1( wxCommandEvent &event );
	void OnPlay( wxCommandEvent &event );
	void OnLoadAnim( wxCommandEvent &event );
	void OnSaveAnim( wxCommandEvent &event );
	void OnNewAnim( wxCommandEvent &event );
	void OnRemove( wxCommandEvent &event );
	void OnListDblClick( wxCommandEvent &event );
	void OnLoad( wxCommandEvent &event );
	void OnSave( wxCommandEvent &event );
	void OnStoreAs( wxCommandEvent &event );
	void OnStore( wxCommandEvent &event );
	void OnRecall( wxCommandEvent &event );
	void OnLocList( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __LocationDlg_H__

