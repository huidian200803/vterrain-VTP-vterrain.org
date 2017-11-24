//
// Name: MatchDlg.cpp
//
// Copyright (c) 2007-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "MatchDlg.h"
#include "Frame.h"
#include "BuilderView.h"
#include "ElevLayer.h"
#include "ImageLayer.h"

#include "vtdata/ElevationGrid.h"
#include "vtui/AutoDialog.h"
#include "vtui/Helper.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// MatchDlg
//----------------------------------------------------------------------------

// WDR: event table for MatchDlg

BEGIN_EVENT_TABLE(MatchDlg, MatchDlgBase)
	EVT_SPIN_UP( ID_SIZE_SPIN, MatchDlg::OnSpinUp )
	EVT_SPIN_DOWN( ID_SIZE_SPIN, MatchDlg::OnSpinDown )
	EVT_CHOICE( ID_MATCH_LAYER, MatchDlg::OnMatchLayer )
	EVT_CHECKBOX( ID_GROW, MatchDlg::OnGrow )
	EVT_CHECKBOX( ID_SHRINK, MatchDlg::OnShrink )
END_EVENT_TABLE()

MatchDlg::MatchDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	MatchDlgBase( parent, id, title, position, size, style )
{
	m_pView = NULL;

	AddValidator(this, ID_EXTENT1, &m_strExtent1);
	AddValidator(this, ID_EXTENT2, &m_strExtent2);
	AddValidator(this, ID_MATCH_LAYER, &m_iLayer);
	AddValidator(this, ID_LAYER_RES, &m_strLayerRes);
	AddValidator(this, ID_GROW, &m_bGrow);
	AddValidator(this, ID_SHRINK, &m_bShrink);
	AddNumValidator(this, ID_TILE_SIZE, &m_iTileSize);
	AddValidator(this, ID_TILING, &m_strTiling);

	UpdateLayers();
	GetLayerSpacing();

	GetSizer()->SetSizeHints(this);
}

void MatchDlg::SetArea(const DRECT &area, bool bIsGeo)
{
	// Set the area extents, and set some initial default values
	m_area = area;
	m_original = area;
	m_bIsGeo = bIsGeo;
	m_tile.Set(2, 2);
	m_iTileSize = 1024;
	m_bGrow = true;
	m_bShrink = true;

	// If geographic (degrees), format the coordinates with more decimals
	if (m_bIsGeo)
		m_fs = _T("%4.8f");
	else
		m_fs = _T("%8.1f");

	UpdateGuess();
	UpdateValues();
}

void MatchDlg::UpdateValues()
{
	// Format the strings with the extent and tiling values
	m_strExtent1.Printf(_T("(")+m_fs+_T(", ")+m_fs+_T("), (")+m_fs+_T(", ")+m_fs+_T(")"),
		m_original.left, m_original.bottom, m_original.Width(), m_original.Height());

	m_strExtent2.Printf(_T("(")+m_fs+_T(", ")+m_fs+_T("), (")+m_fs+_T(", ")+m_fs+_T(")"),
		m_area.left, m_area.bottom, m_area.Width(), m_area.Height());

	m_strTiling.Printf(_T("%d x %d"), m_tile.x, m_tile.y);
}

void MatchDlg::UpdateLayers()
{
	// (Re-)fill the 'Layers' control with all available layers
	GetMatchLayer()->Clear();
	for (uint i = 0; i < g_bld->NumLayers(); i++)
	{
		vtLayer *lay = g_bld->GetLayer(i);

		wxString name = StartOfFilenameWX(lay->GetLayerFilename());
		GetMatchLayer()->Append(name);
	}
	GetMatchLayer()->SetSelection(0);
	m_iLayer = 0;
}

void MatchDlg::GetLayerSpacing()
{
	// Look at the layer selected in the dialog
	vtLayer *lay = g_bld->GetLayer(m_iLayer);

	m_strLayerRes = _("n/a");
	m_spacing.Set(0,0);

	// Get resolution (ground spacing) from it
	if (lay->GetType() == LT_ELEVATION)
	{
		vtElevLayer *elay = (vtElevLayer *)lay;
		if (elay->GetGrid())
			m_spacing = elay->GetGrid()->GetSpacing();
	}
	if (lay->GetType() == LT_IMAGE)
		m_spacing = ((vtImageLayer *)lay)->GetSpacing();

	// Update the string displayed to the user
	if (m_spacing != DPoint2(0,0))
	{
		m_strLayerRes.Printf(_T("%.2f, %.2f"), m_spacing.x, m_spacing.y);
	}
}

void MatchDlg::UpdateGuess()
{
	// Based on what the user specified for Tile LOD0 Size, estimate how
	//  closely a set of tiles can match the original area.  This is affected
	//  by whether the user allows us to increase or decrease the area.

	MatchTilingToResolution(m_original, m_spacing, m_iTileSize, m_bGrow, m_bShrink,
		m_area, m_tile);

	// Show to the user, visually
	if (m_pView)
		m_pView->SetGridMarks(m_area, m_tile.x, m_tile.y, -1, -1);
}

// WDR: handler implementations for MatchDlg

void MatchDlg::OnShrink( wxCommandEvent &event )
{
	TransferDataFromWindow();
	if (!m_bShrink)
		m_bGrow = true;
	UpdateGuess();
	UpdateValues();
	TransferDataToWindow();
}

void MatchDlg::OnGrow( wxCommandEvent &event )
{
	TransferDataFromWindow();
	if (!m_bGrow)
		m_bShrink = true;
	UpdateGuess();
	UpdateValues();
	TransferDataToWindow();
}

void MatchDlg::OnMatchLayer( wxCommandEvent &event )
{
	TransferDataFromWindow();
	GetLayerSpacing();
	UpdateGuess();
	UpdateValues();
	TransferDataToWindow();
}

void MatchDlg::OnSpinUp( wxSpinEvent &event )
{
	m_iTileSize <<= 1;
	if (m_iTileSize > 4096)
		m_iTileSize = 4096;
	UpdateGuess();
	UpdateValues();
	TransferDataToWindow();
}

void MatchDlg::OnSpinDown( wxSpinEvent &event )
{
	m_iTileSize >>= 1;
	if (m_iTileSize < 1)
		m_iTileSize = 1;
	UpdateGuess();
	UpdateValues();
	TransferDataToWindow();
}



