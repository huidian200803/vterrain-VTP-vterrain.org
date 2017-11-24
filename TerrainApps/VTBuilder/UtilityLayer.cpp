//
// UtilityLayer.cpp
//
// Copyright (c) 2001-2015 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "Builder.h"
#include "UtilityLayer.h"
#include "ScaledView.h"

//////////////////////////////////////////////////////////////////////////

vtUtilityLayer::vtUtilityLayer() : vtLayer(LT_UTILITY)
{
	SetLayerFilename(_T("Untitled.osm"));
}

bool vtUtilityLayer::GetExtent(DRECT &rect)
{
	if (m_Poles.empty())
		return false;

	GetPoleExtents(rect);

	return true;
}

void vtUtilityLayer::DrawLayer(vtScaledView *pView, UIContext &ui)
{
	uint i;
	uint npoles = m_Poles.size();
	uint nlines = m_Lines.size();

	if (!npoles)
		return;

	glColor3f(0, 1, 0);

	m_size = 3;

	glColor3f(0, 0, 1);
	for (i = 0; i < npoles; i++)
	{
		// draw each Pole
		vtPole *pole = m_Poles[i];
		DrawPole(pView, pole);
	}
	glColor3f(0, 1, 0);
	DLine2 polyline;
	for (i = 0; i < nlines; i++)
	{
		vtLine *line = m_Lines[i];
		line->MakePolyline(polyline);
		pView->DrawPolyLine(polyline, false);
	}
}

void vtUtilityLayer::DrawPole(vtScaledView *pView, vtPole *pole)
{
	pView->DrawXHair(pole->m_p, m_size);
}

bool vtUtilityLayer::OnSave(bool progress_callback(int))
{
	return WriteOSM(GetLayerFilename().mb_str(wxConvUTF8));
}

bool vtUtilityLayer::OnLoad()
{
	wxString strExt = GetLayerFilename().AfterLast('.');

//	if (!strExt.CmpNoCase("utl"))
//		return ReadUTL(m_strFilename);
	return false;
}

void vtUtilityLayer::GetCRS(vtCRS &crs)
{
	crs = m_crs;
}

void vtUtilityLayer::SetCRS(const vtCRS &crs)
{
	vtUtilityMap::SetCRS(crs);
}

bool vtUtilityLayer::TransformCoords(vtCRS &crs)
{
	if (crs == m_crs)
		return true;

	return vtUtilityMap::TransformTo(crs);
}

bool vtUtilityLayer::AppendDataFrom(vtLayer *pL)
{
	// safety check
	if (pL->GetType() != LT_UTILITY)
		return false;

//	vtUtilityLayer *pFrom = (vtUtilityLayer *)pL;
	// TODO
	return true;
}

void vtUtilityLayer::Offset(const DPoint2 &delta)
{
	vtUtilityMap::Offset(delta);
}

void vtUtilityLayer::GetPropertyText(wxString &strIn)
{
	wxString str;
	str.Printf(_("Nodes (poles/towers): %d\n"), m_Poles.size());
	strIn += str;

	str.Printf(_("Lines: %d\n"), m_Lines.size());
	strIn += str;
}

void vtUtilityLayer::DeselectAll()
{
}

void vtUtilityLayer::InvertSelection()
{
}


