//
// Name: TinTextureDlg.h
//
// Copyright (c) 2006-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __TinTextureDlg_H__
#define __TinTextureDlg_H__

#include "EnviroUI.h"
#include "vtui/AutoDialog.h"
#include "vtlib/core/TParams.h"

//----------------------------------------------------------------------------
// TinTextureDlg
//----------------------------------------------------------------------------

class TinTextureDlg: public TinTextureDlgBase
{
public:
	// constructors and destructors
	TinTextureDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void SetOptions(const vtTagArray &layer_tags);
	void GetOptions(vtTagArray &layer_tags);

private:
	void UpdateColorMapChoice();
	void ValuesToSliders();
	void UpdateEnableState();

	// texture
	bool		m_bDeriveColor;
	wxString	m_strColorMap;
	bool		m_bGeotypical;
	wxString	m_strTextureGeotypical;
	float		m_fGeotypicalScale;
	int			m_iOpacity;
	float		m_fOpacity;

	bool	m_bSetting;

	// WDR: handler declarations for TinTextureDlg
	void OnEditColors( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);
	void OnText( wxCommandEvent &event );
	void OnOpacitySlider( wxCommandEvent &event );
	void OnCheckBox( wxCommandEvent &event );

	bool TransferDataToWindow();
	bool TransferDataFromWindow();

	DECLARE_EVENT_TABLE()
};

#endif	// __TinTextureDlg_H__

