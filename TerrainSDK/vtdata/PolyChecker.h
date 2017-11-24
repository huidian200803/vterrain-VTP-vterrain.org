//
// PolyChecker.h: interface for the PolyChecker class by Roger James.
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef POLYCHECKER_H
#define POLYCHECKER_H

#include "MathTypes.h"

// I really need to stop defining this!
#define  POLYEPSILON 0.000001

class PolyChecker
{
public:
	PolyChecker();
	virtual ~PolyChecker();

	bool Xor(bool x, bool y);
	bool Intersect(const DPoint2 &a, const DPoint2 &b, const DPoint2 &c, const DPoint2 &d);
	bool IntersectProp(const DPoint2 &a, const DPoint2 &b, const DPoint2 &c, const DPoint2 &d);
	bool Between(const DPoint2 &a, const DPoint2 &b, const DPoint2 &c);
	bool Left(const DPoint2 &a, const DPoint2 &b, const DPoint2 &c);
	bool LeftOn(const DPoint2 &a, const DPoint2 &b, const DPoint2 &c);
	bool Collinear(const DPoint2 &a, const DPoint2 &b, const DPoint2 &c);
	double Area2(const DPoint2 &a, const DPoint2 &b, const DPoint2 &c);
	int AreaSign(const DPoint2 &a, const DPoint2 &b, const DPoint2 &c, double dEpsilon = POLYEPSILON);
	bool IsSimplePolygon(const DLine2 &vertices);
	bool IsSimplePolygon(const DLine3 &vertices);
	bool IsClockwisePolygon(const DLine2 &vertices);
	bool IsClockwisePolygon(const DLine3 &vertices);

	bool Intersect(const FPoint2 &a, const FPoint2 &b, const FPoint2 &c, const FPoint2 &d);
	bool IntersectProp(const FPoint2 &a, const FPoint2 &b, const FPoint2 &c, const FPoint2 &d);
	bool Between(const FPoint2 &a, const FPoint2 &b, const FPoint2 &c);
	bool Left(const FPoint2 &a, const FPoint2 &b, const FPoint2 &c);
	bool LeftOn(const FPoint2 &a, const FPoint2 &b, const FPoint2 &c);
	bool Collinear(const FPoint2 &a, const FPoint2 &b, const FPoint2 &c);
	float Area2(const FPoint2 &a, const FPoint2 &b, const FPoint2 &c);
	int AreaSign(const FPoint2 &a, const FPoint2 &b, const FPoint2 &c, float dEpsilon = POLYEPSILON);
	bool IsSimplePolygon(const FLine2 &vertices);
	bool IsSimplePolygon(const FLine3 &vertices);
	bool IsClockwisePolygon(const FLine2 &vertices);
	bool IsClockwisePolygon(const FLine3 &vertices);
};

#endif // POLYCHECKER_H

