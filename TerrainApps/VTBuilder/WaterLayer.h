//
// WaterLayer.h
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef WATERLAYER_H
#define WATERLAYER_H

#include "vtdata/shapelib/shapefil.h"
#include "vtdata/RoadMap.h"
#include "Layer.h"

class vtDIB;

//////////////////////////////////////////////////////////

class vtWaterFeature : public DLine2
{
public:
	vtWaterFeature() : DLine2() { m_bIsBody = false; }
	bool m_bIsBody;
};

class vtWaterLayer : public vtLayer
{
public:
	vtWaterLayer();
	~vtWaterLayer();

	bool GetExtent(DRECT &rect);
	void DrawLayer(vtScaledView *pView, UIContext &ui);
	bool TransformCoords(vtCRS &crs);
	bool OnSave(bool progress_callback(int) = NULL);
	bool OnLoad();
	bool AppendDataFrom(vtLayer *pL);
	void GetCRS(vtCRS &crs);
	void SetCRS(const vtCRS &crs);
	void Offset(const DPoint2 &p);
	void GetPropertyText(wxString &str);

	void AddFeature(const DLine2 &dline, bool bIsBody = false);
	vtWaterFeature &GetFeature(int i) { return (vtWaterFeature &) m_Lines[i]; }

	void PaintDibWithWater(vtDIB *dib);

	void AddElementsFromDLG(vtDLGFile *pDlg);
	void AddElementsFromSHP(const wxString &filename, const vtCRS &crs);

	// Import from SDTS via OGR
	void AddElementsFromOGR(class GDALDataset *datasource,
		bool progress_callback(int) = NULL);

protected:
	// data for rivers and water bodies
	// eventually, should have vector+width data for rivers, area data for bodies
	// for now, just use plain vectors for everything
	DLine2Array		m_Lines;
	std::vector<bool>	m_IsBody;
	vtCRS	m_crs;
};

#endif

