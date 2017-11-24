//
// QuikGrid.h
//
// This code provides an interface to the QuikGrid LGPL library
//  (http://www.perspectiveedge.com/source.html)
//
// In particular, it solves the problem that QuikGrid can only link to
//  a single callback function for contour output.
//
// Copyright (c) 2007-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

#include "config_vtdata.h"	// To ensure we have SUPPORT_QUIKGRID

#if SUPPORT_QUIKGRID

// QuikGrid headers
#include "surfgrid.h"
#include "contour.h"

#include "HeightField.h"
#include "Features.h"

typedef void (*ContourCallback)(void *context, float x, float y, bool bStart);

void SetQuikGridCallbackFunction(ContourCallback fn, void *context);

class ContourConverter
{
public:
	ContourConverter();
	~ContourConverter();

	/// Setup to generate line features
	bool Setup(vtHeightFieldGrid3d *pHFGrid, vtFeatureSetLineString *fset);

	void GenerateContour(float fAlt);
	void GenerateContours(float fAInterval);
	void Finish();
	void Coord(float x, float y, bool bStart);

protected:
	bool SetupGrid(vtHeightFieldGrid3d *pHGGrid);
	void Flush();

	class SurfaceGrid *m_pGrid;

	vtHeightFieldGrid3d *m_pHF;
	DRECT m_ext;
	DPoint2 m_spacing;
	float m_fAltitude;
	float m_fHeight;
	DLine2	m_line;

	vtFeatureSetLineString *m_pLS;
};

#endif // SUPPORT_QUIKGRID
