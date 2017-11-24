//
// Name: SizeDlg.cpp
//
// Copyright (c) 2008-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "SizeDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// SizeDlg
//----------------------------------------------------------------------------

// WDR: event table for SizeDlg

BEGIN_EVENT_TABLE(SizeDlg,SizeDlgBase)
EVT_SLIDER( ID_SLIDER_RATIO, SizeDlg::OnSlider )
END_EVENT_TABLE()

SizeDlg::SizeDlg(wxWindow *parent, wxWindowID id, const wxString &title,
				 const wxPoint &position, const wxSize& size, long style) :
	SizeDlgBase( parent, id, title, position, size, style )
{
	m_fRatio = 1.0f;
	m_iRatio = 0;

	AddValidator(this, ID_SLIDER_RATIO, &m_iRatio);
	AddNumValidator(this, ID_TEXT_X, &m_Current.x);
	AddNumValidator(this, ID_TEXT_Y, &m_Current.y);

	GetSliderRatio()->SetFocus();

	GetSizer()->SetSizeHints(this);
}

void SizeDlg::SetBase(const IPoint2 &size)
{
	m_Base = size;
	Update();
}

void SizeDlg::SetRatioRange(float fMin, float fMax)
{
	m_fRatioMin = fMin;
	m_fRatioMax = fMax;
	Update();
}

void SizeDlg::Update()
{
	m_Current = m_Base * m_fRatio;
}

// WDR: handler implementations for SizeDlg

void SizeDlg::OnSlider( wxCommandEvent &event )
{
	TransferDataFromWindow();
	m_fRatio = m_fRatioMin + ((float) m_iRatio / 320.0f) * (m_fRatioMax - m_fRatioMin);
	Update();
	TransferDataToWindow();
}

