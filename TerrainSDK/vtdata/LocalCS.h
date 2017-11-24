//
// LocalCS.h
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTDATA_LOCAL_CS_H
#define VTDATA_LOCAL_CS_H

// global conversion factor
#define WORLD_SCALE				1.0f	// 1 meter = 1.0 units

#include "MathTypes.h"
#include "Units.h"

/**
 This class represents a mapping between real earth coordinates (geographic or
 projected, elevation in meters) and a local, meters-based 3D coordinate system,
 which uses the right-handed OpenGL axis convention (X right, Y up, Z backwards)
 */
class LocalCS
{
public:
	LocalCS();

	void Setup(LinearUnits units, const DRECT &earthextents);
	void Setup(LinearUnits units, const DPoint2 &origin);

	void LocalToEarth(float x, float z, double &ex, double &ey) const;
	void LocalToEarth(float x, float z, DPoint2 &earth) const;
	void LocalToEarth(const FPoint3 &world, DPoint3 &earth) const;

	void EarthToLocal(double ex, double ey, float &x, float &z) const;
	void EarthToLocal(const DPoint2 &earth, float &x, float &z) const;
	void EarthToLocal(const DPoint3 &earth, FPoint3 &world) const;
	void EarthToLocal(const DRECT &earth, FRECT &world) const;

	void VectorLocalToEarth(float x, float z, DPoint2 &earth) const;
	void VectorEarthToLocal(const DPoint2 &earth, float &x, float &z) const;

	LinearUnits GetUnits() const { return m_Units; }

protected:
	LinearUnits m_Units;
	DPoint2	m_EarthOrigin;
	DPoint2	m_Scale;
};

#endif // VTDATA_LOCAL_CS_H

