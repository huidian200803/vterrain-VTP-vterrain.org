//
// Name: OptionsDlg.h
//
// Copyright (c) 2004-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __OptionsDlg_H__
#define __OptionsDlg_H__

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "EnviroUI.h"
#include "vtui/AutoDialog.h"
#include "../Options.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// OptionsDlg
//----------------------------------------------------------------------------

/**
 Dialog to edit Enviro's global options, EnviroOptions.
 */
class OptionsDlg: public OptionsDlgBase
{
public:
	// constructors and destructors
	OptionsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for OptionsDlg
	wxTextCtrl* GetSelectionRadius()  { return (wxTextCtrl*) FindWindow( ID_SELECTION_RADIUS ); }
	wxTextCtrl* GetSelectionCutoff()  { return (wxTextCtrl*) FindWindow( ID_SELECTION_CUTOFF ); }
	wxCheckBox* GetSizeInside()  { return (wxCheckBox*) FindWindow( ID_SIZE_INSIDE ); }
	wxTextCtrl* GetWinYsize()  { return (wxTextCtrl*) FindWindow( ID_WIN_YSIZE ); }
	wxTextCtrl* GetWinXsize()  { return (wxTextCtrl*) FindWindow( ID_WIN_XSIZE ); }
	wxTextCtrl* GetWiny()  { return (wxTextCtrl*) FindWindow( ID_WINY ); }
	wxTextCtrl* GetWinx()  { return (wxTextCtrl*) FindWindow( ID_WINX ); }
	wxChoice* GetContent()  { return (wxChoice*) FindWindow( ID_CHOICE_CONTENT ); }

	void GetOptionsFrom(EnviroOptions &opt);
	void PutOptionsTo(EnviroOptions &opt);
	void UpdateEnabling();

private:
	// WDR: member variable declarations for OptionsDlg
	bool	m_bFullscreen;
	bool	m_bStereo;
	int		m_iStereoMode;
	int		m_iMultiSamples;
	IPoint2 m_WinPos, m_WinSize;
	bool	m_bLocationInside;

//  bool	m_bHtmlpane;
//  bool	m_bFloatingToolbar;
	bool	m_bTextureCompression;
	bool	m_bDisableMipmaps;

	bool	m_bDirectPicking;
	float   m_fSelectionCutoff;
	float   m_fMaxPickableInstanceRadius;

	float   m_fPlantScale;
//  bool	m_bShadows;
	bool	m_bOnlyAvailableSpecies;

	int		m_iContentFile;
	wxString   m_strContentFile;
	bool	m_bShowProgress;
	bool	m_bFlyIn;
	bool	m_bUseJoystick;
	bool	m_bUseSpaceNav;

private:
	// WDR: handler declarations for OptionsDlg
	void OnCheck( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);
	void OnOK( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __OptionsDlg_H__
