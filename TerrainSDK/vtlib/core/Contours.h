//
// Contours.h
//
// Copyright (c) 2004-2015 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

#if SUPPORT_QUIKGRID

#include "Terrain.h"
class SurfaceGrid;
class vtGeomFactory;

/** \defgroup utility Utility classes
 */
/*@{*/

/**
 * This class provides the ability to easily construct contour lines
 * on a terrain.  It does so by calling the QuikGrid library to generate
 * contour vectors, then converts those vectors into 3D line geometry
 * draped on the terrain.
 *
 * \par Here is an example of how to use it:
	\code
	vtContourConverter cc;
	cc.Setup(pTerrain, RGBf(1,1,0), 10);
	cc.GenerateContours(100);
	cc.Finish();
	\endcode
 *
 * \par Or, you can generate specific contour lines:
	\code
	vtContourConverter cc;
	cc.Setup(pTerrain, RGBf(1,1,0), 10);
	cc.GenerateContour(75);
	cc.GenerateContour(125);
	cc.GenerateContour(250);
	cc.Finish();
	\endcode
 *
 * \par If you keep a pointer to the geometry, you can toggle or delete it later:
	\code
	vtContourConverter cc;
	vtGeode *geode = cc.Setup(pTerrain, RGBf(1,1,0), 10);
	[...]
	geom->SetEnabled(bool);	// set visibility
	[...]
	pTerrain->GetScaledFeatures()->RemoveChild(geom);
	geom->Release();		// delete
	\endcode
 */
class vtContourConverter
{
public:
	vtContourConverter();
	~vtContourConverter();

	// There are two ways to use the converter:
	/// Setup to generate geometry directly on a terrain
	vtGeode *Setup(vtTerrain *pTerr, const RGBf &color, float fHeight);
	/// Setup to generate line features
	bool Setup(vtTerrain *pTerr, vtFeatureSetLineString *fset);

	void GenerateContour(float fAlt);
	void GenerateContours(float fAInterval);
	void Finish();
	void Coord(float x, float y, bool bStart);

protected:
	bool SetupTerrain(vtTerrain *pTerr);
	void Flush();

	SurfaceGrid *m_pGrid;

	vtTerrain *m_pTerrain;
	vtHeightFieldGrid3d *m_pHF;
	DRECT m_ext;
	DPoint2 m_spacing;
	float m_fAltitude;
	float m_fHeight;
	DLine2	m_line;

	// These are used if building geometry directly
	vtGeode *m_pGeode;
	vtGeomFactory *m_pMF;

	// This is used if building line features
	vtFeatureSetLineString *m_pLS;
};

/*@}*/  // utility

#endif // SUPPORT_QUIKGRID
