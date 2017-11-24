//
// UtilityLayer.h
//
// Copyright (c) 2001-2015 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

#include "Layer.h"
#include "vtdata/UtilityMap.h"

//////////////////////////////////////////////////////////

class vtUtilityLayer : public vtLayer, public vtUtilityMap
{
public:
	vtUtilityLayer();

	bool GetExtent(DRECT &rect);
	void DrawLayer(vtScaledView *pView, UIContext &ui);
	bool TransformCoords(vtCRS &crs);
	bool OnSave(bool progress_callback(int) = NULL);
	bool OnLoad();
	void GetCRS(vtCRS &crs);
	void SetCRS(const vtCRS &crs);
	bool AppendDataFrom(vtLayer *pL);
	void Offset(const DPoint2 &delta);
	void GetPropertyText(wxString &str);

	void DeselectAll();
	void InvertSelection();

protected:
	void DrawPole(vtScaledView *pView, vtPole *pole);

	int m_size;
};
