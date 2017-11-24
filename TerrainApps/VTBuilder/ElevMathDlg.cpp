//
// ElevMathDlg.cpp
//
// Copyright (c) 2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "ElevMathDlg.h"
#include "Builder.h"
#include "vtui/AutoDialog.h"
#include "vtui/Helper.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ElevMathDlg
//----------------------------------------------------------------------------

// WDR: event table for ElevMathDlg

BEGIN_EVENT_TABLE(ElevMathDlg, ElevMathDlgBase)
	EVT_INIT_DIALOG (ElevMathDlg::OnInitDialog)
	EVT_CHOICE(ID_LAYER1, ElevMathDlg::OnChoice)
	EVT_CHOICE(ID_LAYER2, ElevMathDlg::OnChoice)
END_EVENT_TABLE()

ElevMathDlg::ElevMathDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	ElevMathDlgBase( parent, id, title, position, size, style )
{
	GetSizer()->SetSizeHints(this);

	AddValidator(m_layer1, &m_iLayer1);
	AddValidator(m_layer2, &m_iLayer2);
	AddValidator(m_operation, &m_iOperation);
	AddNumValidator(m_spacing_x, &(m_spacing.x));
	AddNumValidator(m_spacing_y, &(m_spacing.y));
	AddNumValidator(m_grid_x, &(m_grid_size.x));
	AddNumValidator(m_grid_y, &(m_grid_size.y));

	m_bSetting = false;
}

// WDR: handler implementations for ElevMathDlg

void ElevMathDlg::OnChoice( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();

	Update();
}

void ElevMathDlg::Update()
{
	m_pLayer1 = m_layers[m_iLayer1];
	m_pLayer2 = m_layers[m_iLayer2];

	if (m_pLayer1->IsGrid())
		m_spacing = m_pLayer1->GetGrid()->GetSpacing();
	else if (m_pLayer2->IsGrid())
		m_spacing = m_pLayer2->GetGrid()->GetSpacing();
	else
		m_spacing.Set(1, 1);

	// Compute an extent which encompasses both input layers.
	m_extent.SetInsideOut();
	DRECT r1, r2;
	m_pLayer1->GetExtent(r1);
	m_pLayer2->GetExtent(r2);
	m_extent.GrowToContainRect(r1);
	m_extent.GrowToContainRect(r2);

	// Consider grid size.
	m_grid_size.x = m_extent.Width() / m_spacing.x;
	m_grid_size.y = m_extent.Height() / m_spacing.y;

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void ElevMathDlg::OnInitDialog(wxInitDialogEvent& event)
{
	// Setup left side with common name and max height per species.
	m_layer1->Clear();
	m_layers.clear();

	for (uint i = 0; i < g_bld->NumLayers(); i++)
	{
		vtLayer *lay = g_bld->GetLayer(i);
		if (lay->GetType() == LT_ELEVATION)
		{
			m_layers.push_back(dynamic_cast<vtElevLayer*>(lay));

			m_layer1->AppendString(StartOfFilenameWX(lay->GetLayerFilename()));
			m_layer2->AppendString(StartOfFilenameWX(lay->GetLayerFilename()));
		}
	}
	m_layer1->SetSelection(0);
	m_layer2->SetSelection(1);
	m_iLayer1 = 0;
	m_iLayer2 = 1;

	m_bSetting = true;
	wxDialog::OnInitDialog(event);
	m_bSetting = false;

	Update();
}

