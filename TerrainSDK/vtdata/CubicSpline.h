//
// (Natural) Cubic Spline class.
//
// Originally adapted from some LGPL code found around the net.
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __CUBIC_SPLINE_H__
#define __CUBIC_SPLINE_H__

#include <list>
#include "MathTypes.h"

/**
 * Implements interpolation of a cubic spline curved, defined by a set
 * of control points.  The curve will pass through each control point.
 */
class CubicSpline
{
public:
	CubicSpline();
	virtual ~CubicSpline();

	void Cleanup();
	int AddPoint(const DPoint3 &vec);
	bool Generate();
	bool Interpolate(double par, DPoint3 *vec,
					DPoint3 *dvec = NULL, DPoint3 *ddvec = NULL) const;
	/** Return number of control points that define this curve. */
	int NumPoints() const { return m_iPoints; }

private:
	std::list<DPoint3> m_veclist;

	int m_iPoints;
	DPoint3 *m_pCoeff0;
	DPoint3 *m_pCoeff1;
	DPoint3 *m_pCoeff2;
	DPoint3 *m_pCoeff3;
};

#endif	// __CUBIC_SPLINE_H__

