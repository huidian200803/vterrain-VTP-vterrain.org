//
// BruteTerrain class : A brute-force rendered terrain
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef CUSTOMTERRAINH
#define CUSTOMTERRAINH

/** \addtogroup dynterr */
/*@{*/

#include "DynTerrain.h"

/**
 This class provides a simplistic example of how to add a terrain-rendering
 implementation to the vtlib library.  It contains the bare skeleton of
 terrain rendering code, taking an elevation grid as input and doing a
 naive, brute-force rendering all the geometry via OpenGL each frame.

 To add, or wrap, your own terrain rendering algorithm, simply make a
 copy of BruteTerrain and rename it, then fill in the methods with your
 own functionality.
 */
class BruteTerrain : public vtDynTerrainGeom
{
public:
	BruteTerrain();

	// initialization
	DTErr Init(const vtElevationGrid *pGrid, float fZScale);

	// overrides
	void DoRender();
	void DoCulling(const vtCamera *pCam);
	float GetElevation(int iX, int iZ, bool bTrue = false) const;
	void GetWorldLocation(int iX, int iZ, FPoint3 &p, bool bTrue = false) const;
	float GetVerticalExag() const { return m_fZScale; }

protected:
	virtual ~BruteTerrain();

	// rendering
	void RenderSurface();
	void RenderPass();
	void LoadSingleMaterial();

private:
	float *m_pData;			// the elevation height array
	float m_fZScale;
};

/*@}*/	// Group dynterr

#endif
