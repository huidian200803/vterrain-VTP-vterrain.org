//
// Name: SizeDlg.h
//
// Copyright (c) 2008-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __SizeDlg_H__
#define __SizeDlg_H__

#include "vtdata/MathTypes.h"
#include "vtui/AutoDialog.h"
#include "vtui_UI.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// SizeDlg
//----------------------------------------------------------------------------

class SizeDlg: public SizeDlgBase
{
public:
	// constructors and destructors
	SizeDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for SizeDlg
	wxTextCtrl* GetTextCtrl()  { return (wxTextCtrl*) FindWindow( ID_TEXTCTRL ); }
	wxSlider* GetSliderRatio()  { return (wxSlider*) FindWindow( ID_SLIDER_RATIO ); }
	wxTextCtrl* GetTextY()  { return (wxTextCtrl*) FindWindow( ID_TEXT_Y ); }
	wxTextCtrl* GetTextX()  { return (wxTextCtrl*) FindWindow( ID_TEXT_X ); }

	void SetBase(const IPoint2 &size);
	void SetRatioRange(float fMin, float fMax);
	void Update();

	IPoint2 m_Base, m_Current;
	int m_iRatio;
	float m_fRatio, m_fRatioMin, m_fRatioMax;

private:
	// WDR: member variable declarations for SizeDlg

private:
	// WDR: handler declarations for SizeDlg
	void OnSlider( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __SizeDlg_H__
