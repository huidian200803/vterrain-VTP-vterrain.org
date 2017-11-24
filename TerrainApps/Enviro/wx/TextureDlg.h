//
// Name: TextureDlg.h
//
// Copyright (c) 2006-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __TextureDlg_H__
#define __TextureDlg_H__

#include "EnviroUI.h"
#include "vtui/AutoDialog.h"
#include "vtlib/core/TParams.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// TextureDlg
//----------------------------------------------------------------------------

class TextureDlg: public TextureDlgBase
{
public:
	// constructors and destructors
	TextureDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void SetParams(const TParams &Params);
	void GetParams(TParams &Params);
	void UpdateEnableState();
	void UpdateColorMapChoice();

private:
	// texture
	int	 m_iTexture;
	wxString m_strTextureSingle;
	wxString m_strColorMap;
	bool	m_bMipmap;
	bool	m_b16bit;
	bool	m_bPreLight;
	float   m_fPreLightFactor;
	bool	m_bCastShadows;

	bool	m_bSetting;

private:
	// WDR: handler declarations for TextureDlg
	void OnEditColors( wxCommandEvent &event );
	void OnComboTFileSingle( wxCommandEvent &event );
	void OnRadio( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

	bool TransferDataToWindow();
	bool TransferDataFromWindow();

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __TextureDlg_H__

