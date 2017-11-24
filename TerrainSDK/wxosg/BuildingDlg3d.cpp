//
// Name:	BuildingDlg3d.cpp
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "BuildingDlg3d.h"
#include "vtlib/core/TerrainScene.h"	// for GetCurrentTerrain
#include "vtlib/core/Building3d.h"
#include "vtlib/core/Fence3d.h"

extern void EnableContinuousRendering(bool bTrue);

// WDR: class implementations

//----------------------------------------------------------------------------
// BuildingDlg3d
//----------------------------------------------------------------------------

BuildingDlg3d::BuildingDlg3d( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	BuildingDlg( parent, id, title, position, size, style )
{
}

void BuildingDlg3d::Setup(vtBuilding3d *bld3d)
{
	vtTerrain *pTerr = vtGetTS()->GetCurrentTerrain();
	m_pBuilding3d = bld3d;
	BuildingDlg::Setup(pTerr->GetStructureLayer(), bld3d);
}

void BuildingDlg3d::OnOK( wxCommandEvent &event )
{
	BuildingDlg::OnOK(event);

	vtTerrain *pTerr = vtGetTS()->GetCurrentTerrain();
	pTerr->GetStructureLayer()->ConstructStructure(m_pBuilding3d);
}

void BuildingDlg3d::EnableRendering(bool bEnable)
{
	EnableContinuousRendering(bEnable);
}

void BuildingDlg3d::Modified()
{
	// When a building is modified, we must reconstruct its 3D geometry
	vtTerrain *pTerr = vtGetTS()->GetCurrentTerrain();
	vtStructureLayer *slay = pTerr->GetStructureLayer();
	slay->ConstructStructure(m_pBuilding3d);

	// We might also have to rebuild any linear structures around or on it
	const DLine2 &foot = m_pBuilding3d->GetOuterFootprint(0);
	for (uint i = 0; i < slay->size(); i++)
	{
		vtFence3d *fen = slay->GetFence(i);
		if (fen)
		{
			bool bInside = false;
			const DLine2 &pts = fen->GetFencePoints();
			for (uint j = 0; j < pts.GetSize(); j++)
				if (foot.ContainsPoint(pts[j]))
					bInside = true;
			if (bInside)
				slay->ConstructStructure(fen);
		}
	}
}
