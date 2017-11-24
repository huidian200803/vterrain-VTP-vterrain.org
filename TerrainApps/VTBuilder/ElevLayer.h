//
// ElevLayer.h
//
// Copyright (c) 2001-2015 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

#include "wx/image.h"
#include "vtdata/ElevationGrid.h"
#include "vtdata/HeightField.h"
#include "vtdata/vtDIB.h"
#include "Layer.h"
#include "ElevDrawOptions.h"
#include "TilingOptions.h"
#include "GLTexture.h"

#define SHADING_BIAS	200

class vtElevationGrid;
class vtFeatureSet;
class vtTin2d;
class vtFeatureSetPoint3D;
class vtFeatureSetPolygon;
class vtHeightField;

//////////////////////////////////////////////////////////

class vtElevLayer : public vtLayer
{
public:
	vtElevLayer();
	vtElevLayer(const DRECT &area, const IPoint2 &size,
		bool bFloats, float fScale, const vtCRS &crs);
	vtElevLayer(vtElevationGrid *grid);
	virtual ~vtElevLayer();

	// overrides
	bool GetExtent(DRECT &rect);
	void DrawLayer(vtScaledView *pView, UIContext &ui);
	bool TransformCoords(vtCRS &crs);
	bool OnSave(bool progress_callback(int) = NULL);
	bool OnLoad();
	void GetCRS(vtCRS &crs);
	void SetCRS(const vtCRS &crs);

	// optional overrides
	bool SetExtent(const DRECT &rect);
	void GetPropertyText(wxString &str);
	wxString GetFileExtension();
	bool AskForSaveFilename();
	bool GetAreaExtent(DRECT &rect);

	int GetMemoryUsed() const;
	int MemoryNeededToLoad() const;
	void FreeData();
	bool HasData();

	void OnLeftDown(BuilderView *pView, UIContext &ui);
	void OnLeftUp(BuilderView *pView, UIContext &ui);
	void OnMouseMove(BuilderView *pView, UIContext &ui);

	bool AppendDataFrom(vtLayer *pL);
	void ReRender();
	void ReImage();
	bool IsGrid() { return m_pGrid != NULL; }
	void SetPreferGZip(bool val) { m_bPreferGZip = val; }

	void SetupDefaults();

	// heightfield operations
	vtHeightField *GetHeightField();
	void Offset(const DPoint2 &p);
	float GetElevation(const DPoint2 &p);
	bool GetHeightExtents(float &fMinHeight, float &fMaxHeight) const;
	bool ImportFromFile(const wxString &strFileName, bool progress_callback(int) = NULL,
		vtElevError *err = NULL);
	bool CreateFromPoints(vtFeatureSet *set, const IPoint2 &size, float fDistanceRatio);

	// grid operations
	void SetGrid(vtElevationGrid *grid);
	vtElevationGrid	*GetGrid() { return m_pGrid; }
	const vtElevationGrid *GetGrid() const { return m_pGrid; }
	int RemoveElevRange(float zmin, float zmax, const DRECT *area = NULL);
	int SetUnknown(float fValue, const DRECT *area = NULL);
	void DetermineMeterSpacing();
	bool WriteElevationTileset(TilingOptions &opts, BuilderView *pView);
	bool ImportFromDB(const char *szFileName, bool progress_callback(int));

	// TIN operations
	void SetTin(vtTin2d *pTin);
	vtTin2d *GetTin() { return m_pTin; }
	void MergeSharedVerts(bool bSilent = false);
	void SetupTinTriangleBins(int target_triangles_per_bin);

	// Drawing
	void SetupBitmap();
	void RenderBitmap();
	void DrawLayerBitmap(vtScaledView *pView);
	void DrawLayerOutline(vtScaledView *pView);
	static void SetupDefaultColors(ColorMap &cmap);

	static ElevDrawOptions m_draw;
	static bool m_bDefaultGZip;

	// only this many elevation files may be loaded, the rest are paged out on an LRU basis
	static int m_iElevMemLimit;

	bool NeedsDraw();

protected:
	// We can store either a grid or a TIN, so at most one of these two
	//  pointers will be set:
	vtElevationGrid	*m_pGrid;
	vtTin2d *m_pTin;

	bool	m_bNeedsDraw;
	bool	m_bBitmapRendered;	// Rendered, and copied to m_Texture
	float	m_fSpacing;
	bool	m_bPreferGZip;	// user wants their elevation treated as a .gz file
	DPoint2 mTrim1, mTrim2;

	vtDIB	m_Bitmap;
	IPoint2 m_ImageSize;
	GLTexture m_Texture;
};

// Helpers
FPoint3 LightDirection(float angle, float direction);
bool MatchTilingToResolution(const DRECT &original_area, const DPoint2 &resolution,
							int &iTileSize, bool bGrow, bool bShrink, DRECT &new_area,
							IPoint2 &tiling);
bool ElevCacheOpen(vtElevLayer *pLayer, const char *fname, vtElevError *err);
bool ElevCacheLoadData(vtElevLayer *elev);
void ElevCacheRemove(vtElevLayer *elev);
