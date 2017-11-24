//
// Name: ProfileEditDlg.h
//
// Copyright (c) 2006-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __ProfileEditDlg_H__
#define __ProfileEditDlg_H__

#include "vtui_UI.h"
#include "vtdata/MathTypes.h"

class ProfDlgView : public wxScrolledWindow
{
public:
	ProfDlgView(wxWindow *parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize, long style = wxHSCROLL | wxVSCROLL, const wxString& name = _T(""));

	void OnSize(wxSizeEvent& event);
	void NewSize(const wxSize &s);
	void OnDraw(wxDC &dc);
	void OnMouseEvent(wxMouseEvent &event);

	FLine2 m_profile;
	vtArray<wxPoint> m_screen;
	wxPoint m_org;
	float m_scale;
	int m_mode;
	bool m_bDragging;
	int m_iDragging;

private:
	DECLARE_EVENT_TABLE()
};

//----------------------------------------------------------------------------
// ProfileEditDlg
//----------------------------------------------------------------------------

class ProfileEditDlg: public ProfileEditDlgBase
{
public:
	// constructors and destructors
	ProfileEditDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for ProfileEditDlg
	void UpdateEnabling();
	wxButton* GetSave()  { return (wxButton*) FindWindow( ID_SAVE_PROF ); }

	void SetFilename(const char *fname);
	void CheckClockwisdom();

private:
	// WDR: member variable declarations for ProfileEditDlg
	ProfDlgView *m_pView;
	wxString	m_strFilename;

private:
	// WDR: handler declarations for ProfileEditDlg
	void OnOK( wxCommandEvent &event );
	void OnLoad( wxCommandEvent &event );
	void OnSaveAs( wxCommandEvent &event );
	void OnSave( wxCommandEvent &event );
	void OnRemove( wxCommandEvent &event );
	void OnMove( wxCommandEvent &event );
	void OnAdd( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __ProfileEditDlg_H__
