//
// Dynamic Terrain class
//
// This is the parent class for terrain which can redefine it's
// surface at render time.
//
// Copyright (c) 2001-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "DynTerrain.h"

vtDynTerrainGeom::vtDynTerrainGeom() : vtDynGeom(), vtHeightFieldGrid3d()
{
	m_iPolygonTarget = 20000;

	m_bCulleveryframe = true;
	m_bCullonce = false;

	m_fXLookup = m_fZLookup = NULL;
}

vtDynTerrainGeom::~vtDynTerrainGeom()
{
	delete[] m_fXLookup;
	delete[] m_fZLookup;
}

/**
 * Sets up the vtDynTerrainGeom object.
 * The units, extents, height range, grid dimensions and other
 * properties are copied from the supplied elevation grid.
 *
 * This is generally called from the Init() method of a subclass.
 */
DTErr vtDynTerrainGeom::BasicInit(const vtElevationGrid *pGrid)
{
	// initialize the HeightFieldGrid3D
	const LinearUnits units = pGrid->GetCRS().GetUnits();

	float fMinHeight, fMaxHeight;
	pGrid->GetHeightExtents(fMinHeight, fMaxHeight);

	int cols, rows;
	pGrid->GetDimensions(cols, rows);

	// Set up HeightFieldGrid
	Initialize(units, pGrid->GetEarthExtents(), fMinHeight, fMaxHeight, cols, rows);

	if (fabs(m_WorldExtents.Width()) < 0.000001 ||
		fabs(m_WorldExtents.Height()) < 0.000001)
		return DTErr_EMPTY_EXTENTS;

	// Allocate and set the xz lookup tables
	m_fXLookup = new float[m_iSize.x];
	m_fZLookup = new float[m_iSize.y];

	for (int i = 0; i < m_iSize.x; i++)
		m_fXLookup[i] = m_WorldExtents.left + i * m_fStep.x;
	for (int i = 0; i < m_iSize.y; i++)
		m_fZLookup[i] = m_WorldExtents.bottom - i * m_fStep.y;

	m_iTotalTriangles = m_iSize.x * m_iSize.y * 2;

	return DTErr_OK;
}


// overrides for HeightField
bool vtDynTerrainGeom::FindAltitudeOnEarth(const DPoint2 &p, float &fAltitude, bool bTrue) const
{
	DPoint2 spacing = GetSpacing();
	int iX = (int)((p.x - m_EarthExtents.left) / spacing.x);
	int iY = (int)((p.y - m_EarthExtents.bottom) / spacing.y);

	// safety check
	if (iX < 0 || iX >= m_iSize.x-1 || iY < 0 || iY >= m_iSize.y-1)
	{
		fAltitude = 0.0f;
		return false;
	}

	float alt0, alt1, alt2, alt3;
	alt0 = GetElevation(iX, iY, bTrue);
	alt1 = GetElevation(iX+1, iY, bTrue);
	alt2 = GetElevation(iX+1, iY+1, bTrue);
	alt3 = GetElevation(iX, iY+1, bTrue);

	// find fractional amount (0..1 across quad)
	double fX = (p.x - (m_EarthExtents.left + iX * spacing.x)) / spacing.x;
	double fY = (p.y - (m_EarthExtents.bottom + iY * spacing.y)) / spacing.y;

	// which of the two triangles in the quad is it?
	if (fX + fY < 1)
		fAltitude = (float) (alt0 + fX * (alt1 - alt0) + fY * (alt3 - alt0));
	else
		fAltitude = (float) (alt2 + (1.0-fX) * (alt3 - alt2) + (1.0-fY) * (alt1 - alt2));

	return true;
}

bool vtDynTerrainGeom::FindAltitudeAtPoint(const FPoint3 &p, float &fAltitude,
						bool bTrue, int iCultureFlags, FPoint3 *vNormal) const
{
	// Look on culture first
	if (iCultureFlags != 0 && m_pCulture != NULL)
	{
		if (m_pCulture->FindAltitudeOnCulture(p, fAltitude, bTrue, iCultureFlags))
			return true;
	}

	int iX = (int)((p.x - m_WorldExtents.left) / m_fStep.x);
	int iZ = (int)(-(p.z - m_WorldExtents.bottom) / m_fStep.y);

	// safety check
	bool bogus = false;
	if (iX < 0 || iX > m_iSize.x-1 || iZ < 0 || iZ > m_iSize.y-1)
		bogus = true;
	else if (iX == m_iSize.x-1 || iZ == m_iSize.y-1)
	{
		if (p.x == m_WorldExtents.right || p.z == m_WorldExtents.top)
		{
			// right on the edge: allow this point, but don't interpolate
			fAltitude = GetElevation(iX, iZ, bTrue);
			if (vNormal) vNormal->Set(0,1,0);
			return true;
		}
		else
			bogus = true;
	}
	if (bogus)
	{
		fAltitude = 0.0f;
		if (vNormal) vNormal->Set(0, 1, 0);
		return false;
	}

	if (vNormal != NULL)
	{
		FPoint3 p0, p1, p2, p3;
		GetWorldLocation(iX, iZ, p0, bTrue);
		GetWorldLocation(iX+1, iZ, p1, bTrue);
		GetWorldLocation(iX+1, iZ+1, p2, bTrue);
		GetWorldLocation(iX, iZ+1, p3, bTrue);

		// find fractional amount (0..1 across quad)
		float fX = (float)  (p.x - p0.x) / m_fStep.x;
		float fZ = (float) -(p.z - p0.z) / m_fStep.y;

		// which way is this quad split?
		if ((iX + iZ) & 1)
		{
			// which of the two triangles in the quad is it?
			if (fX + fZ < 1)
			{
				fAltitude = p0.y + fX * (p1.y - p0.y) + fZ * (p3.y - p0.y);
				vNormal->UnitNormal(p0, p1, p3);
			}
			else
			{
				fAltitude = p2.y + (1.0f-fX) * (p3.y - p2.y) + (1.0f-fZ) * (p1.y - p2.y);
				vNormal->UnitNormal(p2, p3, p1);
			}
		}
		else
		{
			if (fX > fZ)
			{
				fAltitude = p0.y + fX * (p1.y - p0.y) + fZ * (p2.y - p1.y);
				vNormal->UnitNormal(p1, p2, p0);
			}
			else
			{
				fAltitude = p0.y + fX * (p2.y - p3.y) + fZ * (p3.y - p0.y);
				vNormal->UnitNormal(p3, p0, p2);
			}
		}
	}
	else
	{
		// It's faster to simpler to operate only the elevations, if we don't
		//  need to compute a normal vector.
		const float alt0 = GetElevation(iX, iZ, bTrue);
		const float alt1 = GetElevation(iX+1, iZ, bTrue);
		const float alt2 = GetElevation(iX+1, iZ+1, bTrue);
		const float alt3 = GetElevation(iX, iZ+1, bTrue);

		// find fractional amount (0..1 across quad)
		const float fX = (p.x - (m_WorldExtents.left + iX * m_fStep.x)) / m_fStep.x;
		const float fY = (p.z - (m_WorldExtents.bottom - iZ * m_fStep.y)) / -m_fStep.y;

		// which way is this quad split?
		if ((iX + iZ) & 1)
		{
			// which of the two triangles in the quad is it?
			if (fX + fY < 1)
				fAltitude = (float) (alt0 + fX * (alt1 - alt0) + fY * (alt3 - alt0));
			else
				fAltitude = (float) (alt2 + (1.0f-fX) * (alt3 - alt2) + (1.0f-fY) * (alt1 - alt2));
		}
		else
		{
			if (fX > fY)
				fAltitude = (float) (alt0 + fX * (alt1 - alt0) + fY * (alt2 - alt1));
			else
				fAltitude = (float) (alt0 + fX * (alt2 - alt3) + fY * (alt3 - alt0));
		}
	}
	return true;
}


void vtDynTerrainGeom::SetCull(bool bOnOff)
{
	m_bCulleveryframe = bOnOff;
}

void vtDynTerrainGeom::CullOnce()
{
	m_bCullonce = true;
}

/**
 * Set the polygon target, which is the number of polygons (generally
 * triangles) which the CLOD algorithm will try to maintain.
 */
void vtDynTerrainGeom::SetPolygonTarget(int iCount)
{
	m_iPolygonTarget = iCount;

	// safety check
	if (m_iPolygonTarget < 1000)
		m_iPolygonTarget = 1000;
}

/**
 * Get the polygon target, which is the number of polygons (generally
 * triangles) which the CLOD algorithm will try to maintain.
 */
int vtDynTerrainGeom::GetPolygonTarget() const
{
	return m_iPolygonTarget;
}

/**
 * Return the number of triangles that were rendered by this dynamic terrain
 * last frame.
 */
int vtDynTerrainGeom::NumDrawnTriangles() const
{
	return m_iDrawnTriangles;
}

///////////////////////////////////////////////////////////////////////
//
// Overrides for vtDynGeom
//
void vtDynTerrainGeom::DoCalcBoundBox(FBox3 &box)
{
	// derive bounding box from known terrain origin and size

	// units are those of the coordinate space below the transform
	box.Set(0,				   m_fMinHeight, 0,
			(float)m_iSize.x, m_fMaxHeight, (float)m_iSize.y);
}

void vtDynTerrainGeom::DoCull(const vtCamera *pCam)
{
	// make sure we cull at least every 300 ms
	bool bCullThisFrame = false;
#if 0
	static float last_time = 0.0f;
	float this_time = vtGetTime();
	if ((this_time - last_time) > 0.3f)
	{
		bCullThisFrame = true;
		last_time = this_time;
	}
#endif
	if (m_bCulleveryframe || m_bCullonce || bCullThisFrame)
	{
		DoCulling(pCam);
		m_bCullonce = false;
	}
}

//////////////////////////////////////////////////////////////////////

void vtDynTerrainGeom::SetupTexGen(float fTiling)
{
	GLfloat sPlane[4] = { fTiling * 1.0f / (m_iSize.x-1), 0.0f, 0.0f, 0.0f };
	GLfloat tPlane[4] = { 0.0f, 0.0f, fTiling * 1.0f / (m_iSize.y-1), 0.0f };

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_S, GL_OBJECT_PLANE, sPlane);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, tPlane);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_2D);
}

void vtDynTerrainGeom::DisableTexGen()
{
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
}

//GLboolean ca, va, na, ia, ta;

void vtDynTerrainGeom::PreRender() const
{
	// get ready to count drawn triangles
	vtDynTerrainGeom *pHack = (vtDynTerrainGeom *)this;
	pHack->m_iDrawnTriangles = 0;
}

void vtDynTerrainGeom::PostRender() const
{
}

