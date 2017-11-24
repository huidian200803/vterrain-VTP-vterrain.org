//
// Name:		LightDlg.cpp
//
// Copyright (c) 2004-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtui/AutoDialog.h"
#include "vtui/Helper.h"	// for FillWithColor
#include "LightDlg.h"

#include <wx/colordlg.h>	// for wxColourDialog

// WDR: class implementations

//----------------------------------------------------------------------------
// LightDlg
//----------------------------------------------------------------------------

// WDR: event table for LightDlg

BEGIN_EVENT_TABLE(LightDlg, LightDlgBase)
	EVT_CHOICE( ID_LIGHT, LightDlg::OnLight )
	EVT_BUTTON( ID_AMBIENT, LightDlg::OnAmbient )
	EVT_BUTTON( ID_DIFFUSE, LightDlg::OnDiffuse )
	EVT_TEXT_ENTER( ID_DIRX, LightDlg::OnText )
	EVT_TEXT_ENTER( ID_DIRY, LightDlg::OnText )
	EVT_TEXT_ENTER( ID_DIRZ, LightDlg::OnText )
END_EVENT_TABLE()

LightDlg::LightDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	LightDlgBase( parent, id, title, position, size, style )
{
	AddNumValidator(this, ID_DIRX, &m_dir.x);
	AddNumValidator(this, ID_DIRY, &m_dir.y);
	AddNumValidator(this, ID_DIRZ, &m_dir.z);

	GetLight()->SetSelection(0);
}

void LightDlg::UseLight(vtTransform *pMovLight)
{
	m_pMovLight = pMovLight;
	m_pLight = (vtLightSource *) pMovLight->getChild(0);

	uchar r, g, b;
	RGBf col;
	col = m_pLight->GetAmbient();
	r = (uchar) col.r * 255;
	g = (uchar) col.g * 255;
	b = (uchar) col.b * 255;
	m_ambient.Set(r, g, b);

	col = m_pLight->GetDiffuse();
	r = (uchar) col.r * 255;
	g = (uchar) col.g * 255;
	b = (uchar) col.b * 255;
	m_diffuse.Set(r, g, b);

	UpdateColorBitmaps();

	m_dir = m_pMovLight->GetDirection();

	TransferDataToWindow();
}

void LightDlg::UpdateColorBitmaps()
{
	// Case of a single color, simple
	FillWithColorSize(GetAmbient(), 26, 14, m_ambient);
	FillWithColorSize(GetDiffuse(), 26, 14, m_diffuse);
}


// WDR: handler implementations for LightDlg

void LightDlg::OnText( wxCommandEvent &event )
{
	TransferDataFromWindow();
	if (m_pMovLight)
		m_pMovLight->SetDirection(m_dir);
}

void LightDlg::OnDiffuse( wxCommandEvent &event )
{
	m_data.SetChooseFull(true);
	m_data.SetColour(m_diffuse);

	wxColourDialog dlg(this, &m_data);
	if (dlg.ShowModal() == wxID_OK)
	{
		m_data = dlg.GetColourData();
		m_diffuse = m_data.GetColour();

		RGBi result(m_diffuse.Red(), m_diffuse.Green(), m_diffuse.Blue());
		if (m_pLight)
			m_pLight->SetDiffuse(result);
		UpdateColorBitmaps();
	}
}

void LightDlg::OnAmbient( wxCommandEvent &event )
{
	m_data.SetChooseFull(true);
	m_data.SetColour(m_ambient);

	wxColourDialog dlg(this, &m_data);
	if (dlg.ShowModal() == wxID_OK)
	{
		m_data = dlg.GetColourData();
		m_ambient = m_data.GetColour();

		RGBi result(m_ambient.Red(), m_ambient.Green(), m_ambient.Blue());
		if (m_pLight)
			m_pLight->SetAmbient(result);
		UpdateColorBitmaps();
	}
}

void LightDlg::OnLight( wxCommandEvent &event )
{

}

