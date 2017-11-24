//
// Name:	 Contours.cpp
// Purpose:  Contour-related code, which interfaces vtlib to the
//	QuikGrid library.
//
// Copyright (c) 2004-2009 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

#if SUPPORT_QUIKGRID

#include "vtdata/QuikGrid.h"
#include "vtlib/core/Contours.h"
#include "vtlib/core/GeomFactory.h"
#include "vtlib/core/TiledGeom.h"

//
// This callback function will receive points output from QuikGrid.
//
void ReceiveContourPoint2(void *context, float x, float y, bool bStart)
{
	vtContourConverter *cc = (vtContourConverter *) context;
	cc->Coord(x, y, bStart);
}


/////////////////////////////////////////////////////////////////////////////
// class vtContourConverter

vtContourConverter::vtContourConverter()
{
	m_pMF = NULL;
	m_pGrid = NULL;
	m_pGeode = NULL;
	m_pLS = NULL;
}

vtContourConverter::~vtContourConverter()
{
	delete m_pMF;
	delete m_pGrid;
}

bool vtContourConverter::SetupTerrain(vtTerrain *pTerr)
{
	// Make a note of this terrain and its attributes
	m_pTerrain = pTerr;
	m_pHF = pTerr->GetHeightFieldGrid3d();
	int tileLod0Size = 0;
	if (!m_pHF)
	{
		vtTiledGeom *tiledGeom = pTerr->GetTiledGeom();
		if (!tiledGeom)
			return false;

		m_ext = tiledGeom->GetEarthExtents();
		//get highest LOD
		int minLod = 0;
		for(int i = 0; i < tiledGeom->rows * tiledGeom->rows; i++)
			if (tiledGeom->m_elev_info.lodmap.m_min[i] > minLod)
				minLod = tiledGeom->m_elev_info.lodmap.m_min[i];

		tileLod0Size = 1 << minLod;
		m_spacing = DPoint2(m_ext.Width() / (tiledGeom->cols * tileLod0Size), m_ext.Height() / (tiledGeom->rows *tileLod0Size));
	}
	else
	{
		m_ext = m_pHF->GetEarthExtents();
		m_spacing = m_pHF->GetSpacing();
	}

	// copy data from our grid to a QuikGrid object
	int nx, ny;
	if (m_pHF)
		m_pHF->GetDimensions(nx, ny);
	else
	{
		vtTiledGeom *tiledGeom = pTerr->GetTiledGeom();
		nx = tiledGeom->cols * tileLod0Size + 1;
		ny = tiledGeom->rows * tileLod0Size + 1;
	}
	// we can't allocate too much memory, so reduce the resolution if too large
	DPoint2 scale(1.0, 1.0);
	while ((nx/scale.x) > 8192)
		scale.x *= 2.0;
	while ((ny/scale.y) > 8192)
		scale.y *= 2.0;

	m_spacing.x *= scale.x;
	m_spacing.y *= scale.y;
	nx /= scale.x;
	ny /= scale.y;

	m_pGrid = new SurfaceGrid(nx,ny);
	int i, j;
	if (m_pHF)
	{
		for (i = 0; i < nx; i++)
		{
			for (j = 0; j < ny; j++)
			{
				// use the true elevation, for true contours
				m_pGrid->zset(i, j, m_pHF->GetElevation(i, j, true));
			}
		}
	}
	else
	{
		vtTiledGeom *tiledGeom = pTerr->GetTiledGeom();
		float topBottom = tiledGeom->m_WorldExtents.top - tiledGeom->m_WorldExtents.bottom;
		float rightLeft = tiledGeom->m_WorldExtents.right - tiledGeom->m_WorldExtents.left;
		float xwidth = rightLeft / nx;
		float ywidth = topBottom / ny;
		float altitude;
		for (i = 0; i < nx; i++)
		{
			for (j = 0; j < ny; j++)
			{
				// use the true elevation, for true contours
				tiledGeom->FindAltitudeAtPoint(FPoint3(i*xwidth, 0, j*ywidth),altitude, true);
				m_pGrid->zset(i, j, altitude);
			}
		}
	}
	return true;
}

/**
 * Set up the class to create geometry on a terrain.
 *
 * \param pTerr The terrain you will generate the contour lines on.
 * \param color The colors of the generated lines.
 * \param fHeight The height above the terrain to drape the lines.  Generally
 *		you will want to use a small offset value here, to keep the lines from
 *		colliding with the terrain itself.
 * \return A geometry node which contains the contours.
 */
vtGeode *vtContourConverter::Setup(vtTerrain *pTerr, const RGBf &color, float fHeight)
{
	if (!pTerr)
		return NULL;

	if (!SetupTerrain(pTerr))
		return NULL;

	m_fHeight = fHeight;

	// Create material and geometry to contain the vector geometry
	vtMaterialArrayPtr pMats = new vtMaterialArray;
	pMats->AddRGBMaterial(color, false, false, true);

	m_pGeode = new vtGeode;
	m_pGeode->setName("Contour Geometry");
	m_pGeode->SetMaterials(pMats);

	m_pMF = new vtGeomFactory(m_pGeode, osg::PrimitiveSet::LINE_STRIP, 0, 30000, 0);

	return m_pGeode;
}


/**
 * Set up the class to create line features on a terrain.
 *
 * \param pTerr The terrain you will generate the contour lines on.
 * \param fset The featureset to receive the polylines.
 * \return True if successful.
 */
bool vtContourConverter::Setup(vtTerrain *pTerr, vtFeatureSetLineString *fset)
{
	if (!pTerr)
		return false;

	if (!SetupTerrain(pTerr))
		return false;

	m_pLS = fset;
	return true;
}


/**
 * Generate a contour line to be draped on the terrain.
 *
 * \param fAlt The altitude (elevation) of the line to be generated.
 */
void vtContourConverter::GenerateContour(float fAlt)
{
	SetQuikGridCallbackFunction(ReceiveContourPoint2, this);
	m_fAltitude = fAlt;
	Contour(*m_pGrid, fAlt);
}

/**
 * Generate a set of contour lines to be draped on the terrain.
 *
 * \param fInterval  The vertical spacing between the contours.  For example,
 *		if the elevation range of your data is from 50 to 350 meters, then
 *		an fIterval of 100 will place contour bands at 100,200,300 meters.
 */
void vtContourConverter::GenerateContours(float fInterval)
{
	float fMin, fMax;
	vtHeightField *hf;
	if (m_pHF)
		hf = m_pHF;
	else
		hf = m_pTerrain->GetTiledGeom();
	if (!hf)
		return;

	hf->GetHeightExtents(fMin, fMax);
	int start = (int) (fMin / fInterval) + 1;
	int stop = (int) (fMax / fInterval);

	SetQuikGridCallbackFunction(ReceiveContourPoint2, this);
	for (int i = start; i <= stop; i++)
	{
		m_fAltitude = i * fInterval;
		Contour(*m_pGrid, i * fInterval);
	}
}

void vtContourConverter::Coord(float x, float y, bool bStart)
{
	if (bStart)
		Flush();

	DPoint2 p2;
	p2.x = m_ext.left + x * m_spacing.x;
	p2.y = m_ext.bottom + y * m_spacing.y;
	m_line.Append(p2);
}

/**
 * Finishes the contour generation process.  Call once when you are done
 * using the class to generate contours.
 */
void vtContourConverter::Finish()
{
	Flush();

	if (m_pMF)
	{
		// Add the geometry to the terrain's scaled features, so that it will scale
		//  up/down with the terrain's vertical exaggeration.
		m_pTerrain->GetScaledFeatures()->addChild(m_pGeode);
	}
}

void vtContourConverter::Flush()
{
	// we may have some degenerate geometry; we need at least three points
	if (m_line.GetSize() > 2)
	{
		// confirm they are not all the same
		DPoint2 p2 = m_line[0];
		bool same = true;
		for (uint i = 1; i < m_line.GetSize(); i++)
		{
			if (m_line[i] != p2)
			{
				same = false;
				break;
			}
		}
		if (!same)
		{
			if (m_pMF)
			{
				const bool bInterpolate = false;		// no need; it already hugs the ground
				const bool bCurve = false;				// no need; it's already quite smooth
				const bool bUseTrueElevation = true;	// use true elevation, not scaled
				const float fSpacing = 0.0f;			// Doesn't matter, no interpolation.

				m_pMF->AddSurfaceLineToMesh(m_pTerrain->GetHeightField(),
					m_line, fSpacing, m_fHeight, bInterpolate, bCurve, bUseTrueElevation);
			}
			else if (m_pLS)
			{
				int record = m_pLS->AddPolyLine(m_line);
				m_pLS->SetValue(record, 0, m_fAltitude);
			}
		}
	}
	m_line.Clear();
}

#endif // SUPPORT_QUIKGRID

