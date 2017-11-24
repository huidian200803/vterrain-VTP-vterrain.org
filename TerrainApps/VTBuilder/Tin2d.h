//
// Tin2d.h
//
// Copyright (c) 2005-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

#include "vtdata/vtTin.h"

class vtElevationGrid;
class vtFeatureSetPoint3D;
class vtFeatureSetPolygon;
class vtScaledView;

#include <set>

struct IntPair
{
	IntPair() {}
	bool operator <(const IntPair &b) const
	{
		if (v0 < b.v0)
			return true;
		else if (v0 > b.v0)
			return false;
		else
		{
			if (v1 < b.v1)
				return true;
			else
				return false;
		}
	}
	bool operator==(const IntPair &b)
	{
		return (v0 == b.v0 && v1 == b.v1);
	}
	IntPair(int i0, int i1) { v0 = i0; v1 = i1; }
	int v0, v1;
};

struct Outline : public std::set<IntPair>
{
	void AddUniqueEdge(const IntPair &b)
	{
		iterator it = find(b);
		if (it != end())
			erase(it);
		else
			insert(b);
	}
};

/**
 Extend the vtTin class with functionality for doing 2D (actually, 2.5D)
 operations on it, in the Builder environment.
 */
class vtTin2d : public vtTin
{
public:
	vtTin2d();
	~vtTin2d();

	vtTin2d(vtElevationGrid *grid);
	vtTin2d(vtFeatureSetPoint3D *set);
	vtTin2d(vtFeatureSetPolygon *set, int iFieldNum, float fHeight = 0.0f);

	void DrawTin(vtScaledView *pView);
	void ComputeEdgeLengths();
	void CullLongEdgeTris();
	void FreeEdgeLengths();
	void SetConstraint(bool bConstrain, double fMaxEdge);
	void MakeOutline();
	int GetMemoryUsed() const;

	double *m_fEdgeLen;
	bool m_bConstrain;
	double m_fMaxEdge;

	Outline m_edges;
};

