//
// SRTerrain class : a subclass of vtDynTerrainGeom which encapsulates
//  Stefan Roettger's CLOD algorithm.
//
// Utilizes: Roettger's MINI library implementation
// http://stereofx.org/#Terrain
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef SRTERRAINH
#define SRTERRAINH

#include "DynTerrain.h"

/** \addtogroup dynterr */
/*@{*/

/*!
	The SRTerrain class implements Stefan Roettger's algorithm for
	regular-grid terrain LOD.  It was adapted directly from his sample
	implementation and correspondence with him.
*/
class SRTerrain : public vtDynTerrainGeom
{
public:
	SRTerrain();

	// initialization
	DTErr Init(const vtElevationGrid *pGrid, float fZScale);

	// overrides
	void DoRender();
	void DoCulling(const vtCamera *pCam);
	float GetElevation(int iX, int iZ, bool bTrue = false) const;
	void SetElevation(int iX, int iZ, float fValue, bool bTrue = false);
	void GetWorldLocation(int iX, int iZ, FPoint3 &p, bool bTrue = false) const;
	void SetVerticalExag(float fExag);
	float GetVerticalExag() const { return m_fHeightScale; }
	void SetPolygonTarget(int iCount);

	// Dynamic elevation
	DTErr ReInit(const vtElevationGrid *pGrid);

	void LoadSingleMaterial();

	int		m_iBlockSize;

	float m_fResolution;
	float m_fHResolution;
	float m_fLResolution;

protected:
	// rendering
	void RenderSurface();
	void RenderPass();

	// cleanup
	virtual ~SRTerrain();

private:
	class ministub *m_pMini;

	IPoint2 m_window_size;
	FPoint3 m_eyepos_ogl;
	float m_fFOVY;
	float m_fAspect;
	float m_fNear, m_fFar;
	FPoint3 eye_up, eye_forward;

	float m_fHeightScale;
	float m_fMaximumScale;
	float m_fDrawScale;
	bool m_bFloat;
};

/*@}*/	// Group dynterr

#endif	// SRTerrain
