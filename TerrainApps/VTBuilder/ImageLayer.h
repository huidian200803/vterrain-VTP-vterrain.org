//
// ImageLayer.h
//
// Copyright (c) 2002-2015 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

class vtImage;

#include <map>
#include "Layer.h"
#include "GLTexture.h"

///////////////////////////////////////////////////////////////////////

class vtImageLayer : public vtLayer
{
public:
	vtImageLayer();
	vtImageLayer(const DRECT &area, const IPoint2 &size, const vtCRS &crs, int bitDepth);
	virtual ~vtImageLayer();

	// overrides for vtLayer methods
	bool GetExtent(DRECT &rect);
	void DrawLayer(vtScaledView *pView, UIContext &ui);
	bool TransformCoords(vtCRS &crs);
	bool OnSave(bool progress_callback(int) = NULL);
	bool OnLoad();
	bool AppendDataFrom(vtLayer *pL);
	void GetCRS(vtCRS &crs);
	void SetCRS(const vtCRS &crs);
	void Offset(const DPoint2 &delta);

	// optional overrides
	bool SetExtent(const DRECT &rect);
	void GetPropertyText(wxString &str);

	vtImage *GetImage() const { return m_pImage; }
	DPoint2 GetSpacing() const;
	bool ImportFromFile(const wxString &strFileName, bool progress_callback(int) = NULL);
	bool ImportFromDB(const char *szFileName, bool progress_callback(int) = NULL);
	void ReplaceColor(const RGBi &rgb1, const RGBi &rgb2);

protected:
	void DrawLayerOutline(vtScaledView *pView);

	// One vtImage can have any number of overviews, of which some may be on
	// disk, and others in memory as vtDIBs.
	vtImage	*m_pImage;

	// Each vtDIB we want to draw will need a GLTexture.
	std::map<vtDIB*, GLTexture> m_Textures;
};

// Helper
int GetBitDepthUsingGDAL(const char *fname);
void MakeSampleOffsets(const DPoint2 cellsize, uint N, DLine2 &offsets);
