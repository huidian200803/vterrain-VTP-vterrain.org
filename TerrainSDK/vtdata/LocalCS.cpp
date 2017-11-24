//
// LocalProjection.cpp
//
// This library has a concept of current conversion from earth to world
// coordinates which is represented by the LocalCS class.
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "MathTypes.h"
#include "vtCRS.h"
#include "LocalCS.h"

////////////////////////////////////////////////////////////////////////////

LocalCS::LocalCS()
{
	m_EarthOrigin.Set(0, 0);
}

void LocalCS::Setup(LinearUnits units, const DRECT &earthextents)
{
	m_Units = units;

	m_EarthOrigin.Set(earthextents.left, earthextents.bottom);
	if (units == LU_DEGREES)
	{
		double middle = (earthextents.bottom + earthextents.top) / 2;
		double fMetersPerLongitude = EstimateDegreesToMeters(middle);
		m_Scale.x = fMetersPerLongitude;
		m_Scale.y = METERS_PER_LATITUDE;
	}
	else
	{
		m_Scale.x = m_Scale.y = GetMetersPerUnit(units);
	}
}

void LocalCS::Setup(LinearUnits units, const DPoint2 &origin)
{
	m_Units = units;

	m_EarthOrigin = origin;
	if (units == LU_DEGREES)
	{
		double middle = origin.y / 2;
		double fMetersPerLongitude = EstimateDegreesToMeters(middle);
		m_Scale.x = fMetersPerLongitude;
		m_Scale.y = METERS_PER_LATITUDE;
	}
	else
	{
		m_Scale.x = m_Scale.y = GetMetersPerUnit(units);
	}
}

void LocalCS::EarthToLocal(double ex, double ey,
												  float &x, float &z) const
{
	x = (float) ((ex - m_EarthOrigin.x) * m_Scale.x);
	z = (float) -((ey - m_EarthOrigin.y) * m_Scale.y);
}

void LocalCS::LocalToEarth(float x, float z,
												  double &ex, double &ey) const
{
	ex = m_EarthOrigin.x + (x / m_Scale.x);
	ey = m_EarthOrigin.y + (-z / m_Scale.y);
}

/**
 * Convert from the coordinate system of the virtual world (x,y,z) to actual
 * earth coodinates (map coordinates, altitude in meters)
 */
void LocalCS::LocalToEarth(const FPoint3 &world, DPoint3 &earth) const
{
	LocalToEarth(world.x, world.z, earth.x, earth.y);
	earth.z = world.y;
}

/**
 * Convert from the coordinate system of the virtual world (x,y,z) to actual
 * earth coodinates (map coordinates, altitude in meters)
 */
void LocalCS::LocalToEarth(float x, float z, DPoint2 &earth) const
{
	LocalToEarth(x, z, earth.x, earth.y);
}

/**
 * Convert from earth coodinates (map coordinates, altitude in meters) to
 * the coordinate system of the virtual world (x,y,z)
 */
void LocalCS::EarthToLocal(const DPoint2 &earth, float &x, float &z) const
{
	EarthToLocal(earth.x, earth.y, x, z);
}

/**
 * Convert from earth coodinates (map coordinates, altitude in meters) to
 * the coordinate system of the virtual world (x,y,z)
 */
void LocalCS::EarthToLocal(const DPoint3 &earth, FPoint3 &world) const
{
	EarthToLocal(earth.x, earth.y, world.x, world.z);
	world.y = (float) earth.z;
}

/**
 * Convert from earth coodinates (map coordinates, altitude in meters) to
 * the coordinate system of the virtual world (x,y,z)
 */
void LocalCS::EarthToLocal(const DRECT &earth, FRECT &world) const
{
	EarthToLocal(earth.left, earth.bottom, world.left, world.bottom);
	EarthToLocal(earth.right, earth.top, world.right, world.top);
}

/**
 * Convert a vector from the coordinate system of the virtual world (x,y,z)
 * to actual earth coodinates (map coordinates, altitude in meters)
 */
void LocalCS::VectorLocalToEarth(float x, float z, DPoint2 &earth) const
{
	earth.x = (x / m_Scale.x);
	earth.y = (-z / m_Scale.y);
}

/**
 * Convert a vector from earth coodinates (map coordinates, altitude in
 * meters) to the coordinate system of the virtual world (x,y,z)
 */
void LocalCS::VectorEarthToLocal(const DPoint2 &earth, float &x, float &z) const
{
	x = (float) (earth.x * m_Scale.x);
	z = (float) -(earth.y * m_Scale.y);
}

