//
// Name:		ContourDlg.cpp
//
// Copyright (c) 2009-2011 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "ContourDlg.h"
#include "AutoDialog.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ContourDlg
//----------------------------------------------------------------------------

// WDR: event table for ContourDlg

BEGIN_EVENT_TABLE(ContourDlg,ContourDlgBase)
	EVT_RADIOBUTTON( ID_RADIO_SINGLE, ContourDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO_EVERY, ContourDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO_CREATE, ContourDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO_ADD, ContourDlg::OnRadio )
END_EVENT_TABLE()

ContourDlg::ContourDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	ContourDlgBase( parent, id, title, position, size, style )
{
	m_fElevSingle = 1000;
	m_fElevEvery = 1000;
	m_bSingle = true;
	m_bCreate = true;
	m_strLayer = _T("");

	AddNumValidator(this, ID_ELEV1, &m_fElevSingle);
	AddNumValidator(this, ID_ELEV2, &m_fElevEvery);
	AddValidator(this, ID_RADIO_SINGLE, &m_bSingle);
	AddValidator(this, ID_RADIO_CREATE, &m_bCreate);
	AddValidator(this, ID_CHOICE_LAYER, &m_strLayer);

	UpdateEnabling();

	GetSizer()->SetSizeHints(this);
}

// WDR: handler implementations for ContourDlg

void ContourDlg::OnRadio( wxCommandEvent &event )
{
	TransferDataFromWindow();
	UpdateEnabling();
}

void ContourDlg::UpdateEnabling()
{
	m_elev->Enable(m_bSingle);
	m_elev2->Enable(!m_bSingle);
	m_choice_layer->Enable(!m_bCreate);
}

