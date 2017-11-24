//
// DynTerrain class : Dynamically rendering terrain
//
// Copyright (c) 2001-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef DYNTERRAINH
#define DYNTERRAINH

#include "vtdata/ElevationGrid.h"
#include "vtdata/HeightField.h"

/** \defgroup dynterr Dynamic Terrain (CLOD)
 * These classes are used for dynamically rendered terrain surfaces, including
 * implementations of Continuous Level of Detail (CLOD).
 */
/*@{*/

// A set of errors that may be returned from the dynamic terrain classes.
enum DTErr
{
	DTErr_OK,
	DTErr_EMPTY_EXTENTS,
	DTErr_NOTSQUARE,
	DTErr_NOTPOWER2,
	DTErr_NOMEM
};

/**
 * This class provides a framework for implementing any kind of dynamic
 * geometry for a heightfield terrain grid.  It is the parent class which
 * contains common fuctionality used by each of the terrain CLOD
 * implementations.
 */
class vtDynTerrainGeom : public vtDynGeom, public vtHeightFieldGrid3d
{
public:
	vtDynTerrainGeom();

	// Each subclass must implement this initialization method
	virtual DTErr Init(const vtElevationGrid *pGrid, float fZScale) = 0;
	virtual void Init2() {}
	DTErr BasicInit(const vtElevationGrid *pGrid);
	virtual void SetVerticalExag(float fExag) {}
	virtual float GetVerticalExag() const = 0;

	virtual void SetPolygonTarget(int iPolygonCount);
	int GetPolygonTarget() const;

	int NumDrawnTriangles() const;

	void SetupTexGen(float fTiling);
	void DisableTexGen();

	// overrides for vtDynGeom
	void DoCalcBoundBox(FBox3 &box);
	void DoCull(const vtCamera *pCam);

	// overrides for HeightField
	bool FindAltitudeOnEarth(const DPoint2 &p, float &fAltitude, bool bTrue = false) const;

	// overrides for HeightField3d
	bool FindAltitudeAtPoint(const FPoint3 &p3, float &fAltitude,
		bool bTrue = false, int iCultureFlags = 0,
		FPoint3 *vNormal = NULL) const;

	// overridables
	virtual void DoCulling(const vtCamera *pCam) = 0;
	virtual void SetElevation(int i, int j, float fValue, bool bTrue = false) {}

	// control
	void SetCull(bool bOnOff);
	void CullOnce();

	void PreRender() const;
	void PostRender() const;

	// statistics
	int m_iTotalTriangles;
	int m_iDrawnTriangles;

protected:
	// tables for quick conversion from x,y index to output X,Z coordinates
	float	*m_fXLookup, *m_fZLookup;

	// this determines the overall level of detail
	int		m_iPolygonTarget;

	// flags
	bool m_bCulleveryframe;
	bool m_bCullonce;

protected:
	~vtDynTerrainGeom();
};
typedef osg::ref_ptr<vtDynTerrainGeom> vtDynTerrainGeomPtr;

/*@}*/	// Group dynterr

#endif	// DYNTERRAINH

