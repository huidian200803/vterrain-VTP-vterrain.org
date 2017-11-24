//
// Implementation of methods for the basic data classes
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "MathTypes.h"
#include "vtLog.h"

int DPolyArray::s_previous_poly = -1;

/**
* Return the index of the polygon at a specified point, or -1 if
* there is no polygon there.
*
* For speed, it first test the polygon which was found last time.
* For spatially linear testing, this can be a 10x speedup.
*/
int DPolyArray::FindPoly(const DPoint2 &p) const
{
	if (s_previous_poly != -1)
	{
		if (at(s_previous_poly).ContainsPoint(p))
			return s_previous_poly;
	}
	int num = size();
	for (int i = 0; i < num; i++)
	{
		const DPolygon2 &poly = at(i);
#if 0
		// possible further speed test: first test against extents
		if (world_x < poly->xmin || world_x > poly->xmax ||
			world_z < poly->zmin || world_z > poly->zmax)
			continue;
#endif
		if (poly.ContainsPoint(p))
		{
			// found
			s_previous_poly = i;
			return i;
		}
	}
	// not found
	s_previous_poly = -1;
	return -1;
}


//
// DLine2 methods
//

void DLine2::Add(const DPoint2 &p)
{
	uint i, size = GetSize();
	for (i=0; i < size; i++)
		GetAt(i) += p;
}

void DLine2::Mult(double factor)
{
	uint i, size = GetSize();
	for (i=0; i < size; i++)
		GetAt(i) *= factor;
}

void  DLine2::InsertPointAfter(int iInsertAfter, const DPoint2 &Point)
{
	int iNumPoints = GetSize();
	int iIndex;
	if (iInsertAfter == iNumPoints - 1)
		Append(Point);
	else
	{
		for (iIndex = iNumPoints - 1; iIndex > iInsertAfter ; iIndex--)
			SetAt(iIndex + 1, GetAt(iIndex));
		SetAt(iInsertAfter + 1, Point);
	}
}

void DLine2::RemovePoint(int i)
{
	RemoveAt(i);
}

void DLine2::ReverseOrder()
{
	DPoint2 p;
	int i, size = GetSize();
	for (i = 0; i < size/2; i++)
	{
		p = GetAt(i);
		SetAt(i, GetAt(size-1-i));
		SetAt(size-1-i, p);
	}
}

/**
 * Given four points A B C D, where B and C are identical, then this
 *  line is a triangle not a rectangle.  Remove the identical points to
 *  produce A B D.  Points are identical if they are within dEpsilon
 *  of each other.
 *
 * \param dEpsilon Distance.
 * \param bClosed If true, treat this as a closed polyline (a simple
 *  polygon) by wrapping around from the last to the first point.
 */
int DLine2::RemoveDegeneratePoints(double dEpsilon, bool bClosed)
{
	int removed = 0;

	// If closed, the wrap around when looking for adjacent points.
	// Otherwise, never consider removing the first or last point.
	int preserve_endpoints = bClosed ? 0 : 1;

	for (int i = preserve_endpoints; i < (int) GetSize() - preserve_endpoints; i++)
	{
		// Compare each point to the previous
		const double dist = (GetAt(i) - GetSafePoint(i-1)).Length();
		if (dist < dEpsilon)
		{
			RemoveAt(i);
			removed++;
			i--;
		}
	}
	return removed;
}

/**
 Given three points A B C, where the distance from B to the line A-C
  is less than dEpsilon units, B is co-linear and should be removed.

 We could use another measure of linearity, the angle between (B-A)
 and (C-A), but didn't do that.
 
 \param dEpsilon Distance.
 \param bClosed If true, treat this as a closed polyline (a simple
   polygon) by wrapping around from the last to the first point.
 */
int DLine2::RemoveColinearPoints(double dEpsilon, bool bClosed)
{
	int removed = 0;

	// If closed, the wrap around when looking for adjacent points.
	// Otherwise, never consider removing the first or last point.
	int preserve_endpoints = bClosed ? 0 : 1;

	for (int i = preserve_endpoints; i < (int) GetSize() - preserve_endpoints; i++)
	{
		const DPoint2 &prev = GetSafePoint(i-1);
		const DPoint2 &next = GetSafePoint(i+1);

		DPoint2 ray = next - prev;
		ray.Normalize();
		const double dist = ray.Cross(GetAt(i) - prev);
		if (fabs(dist) < dEpsilon)
		{
			RemoveAt(i);
			removed++;
			i--;
		}
	}
	return removed;
}

bool DLine2::IsConvex() const
{
	int positive = 0;
	int negative = 0;
	uint length = GetSize();

	for (uint i = 0; i < length; i++)
	{
		DPoint2 &p0 = GetAt(i);
		DPoint2 &p1 = GetSafePoint(i+1);
		DPoint2 &p2 = GetSafePoint(i+2);
		double cross = (p1-p0).Cross(p2-p1);

		if ( cross < 0 )
			negative++;
		else
			positive++;
	}
	return (negative == 0 || positive == 0);
}

/**
* With the assumption that this set of points defines a closed polygon,
* test whether the polygon contains a given point.  Since a simple array
* of points being interpreted as a polygon, this means the data may be
* concave or convex, but not contains holes.
*/
bool DLine2::ContainsPoint(const DPoint2 &p) const
{
	if (GetData() != NULL)
		return CrossingsTest(GetData(), GetSize(), p);
	else
		return false;
}

/**
* Returns the location of the closest point on the line to a given point.
*
* \param Point The input point.
* \param iIndex Index of the first point of the nearest line segment.
* \param dist Distance from the DLine2 to the input point.
* \param Intersection The closest point on the DLine2.
*
* \return True if a closest point was found.
*/
bool DLine2::NearestSegment(const DPoint2 &Point, int &iIndex,
							double &dist, DPoint2 &Intersection) const
{
	int iNumPoints = GetSize();
	int i, closest = -1;
	double dMagnitude;
	double dDistance;
	double dMinDistance = 1E9;
	double dU;
	DPoint2 p0, p1, p2;

	for (i = 0; i < iNumPoints; i++)
	{
		p0 = GetAt(i);
		p1 = GetAt((i + 1) % iNumPoints);
		dMagnitude = SegmentLength(i);
		// Calculate U for standard line equation:
		// values of U between 0.0 and +1.0 mean normal intersects segment
		dU = (((Point.x - p0.x) * (p1.x - p0.x)) +
			((Point.y - p0.y) * (p1.y - p0.y))) / (dMagnitude * dMagnitude);
		if ((dU < 0.0) || (dU > 1.0))
			continue;
		p2.x = p0.x + dU * (p1.x - p0.x);
		p2.y = p0.y + dU * (p1.y - p0.y);
		dDistance = DPoint2(Point - p2).Length();
		if (dDistance < dMinDistance)
		{
			dMinDistance = dDistance;
			closest = i;
			Intersection = p2;
		}
	}
	if (closest != -1)
	{
		iIndex = closest;
		dist = dMinDistance;
		return true;
	}
	return false;
}

/**
* Return the nearest point (of the points which make up the line).
* This is not the same as the closest place on the line, which may
* lie between the defining points; use NearestSegment to find that.
*
* \param Point The input point.
* \param iIndex Index of the first point of the nearest line segment.
* \param dClosest Distance from the DLine2 to the input point.
*/
void DLine2::NearestPoint(const DPoint2 &Point, int &iIndex, double &dClosest) const
{
	uint iNumPoints = GetSize();
	dClosest = 1E9;
	double dDistance;

	for (uint i = 0; i < iNumPoints; i++)
	{
		dDistance = (Point - GetAt(i)).Length();
		if (dDistance < dClosest)
		{
			dClosest = dDistance;
			iIndex = i;
		}
	}
}

/**
* Get a point on the line, safely wrapping around to the end or beginning
* for index values that are out of range.
*/
DPoint2 &DLine2::GetSafePoint(int index) const
{
	int points = GetSize();
	if (index < 0)
		return GetAt(index + points);
	if (index >= points)
		return GetAt(index - points);
	return GetAt(index);
}

void DLine2::SetSafePoint(int index, const DPoint2 &p)
{
	int points = GetSize();
	if (index < 0)
		SetAt(index + points, p);
	else if (index >= points)
		SetAt(index - points, p);
	else
		SetAt(index, p);
}

/**
* Return the length of the N'th segment in the line, which is the distance
* between points N and N+1.  If the length of the last segment is requested,
* a closed polygon is assumed.
*/
double DLine2::SegmentLength(uint i) const
{
	uint j = (i < GetSize()-1) ? i+1 : 0;
	return (GetAt(j) - GetAt(i)).Length();
}

double DLine2::Length() const
{
	uint i, iNumPoints = GetSize();
	double length = 0.0;

	if (!iNumPoints)
		return 0.0;

	for (i = 0; i < iNumPoints-1; i++)
		length += (GetAt(i+1) - GetAt(i)).Length();

	return length;
}

/** Centroid (centre of gravity/mass) of the polygon
 *
 * Note: I've observed some numerical precision issues with this method.
 * With 7-digit coordinates (e.g. x=2388836, y=4690396) the centroid computed
 * may be several units away from its correct location, in fact even outside
 * the polygon for a convex shape.
 */
DPoint2 DLine2::Centroid() const
{
	DPoint2 pt(0,0);
	double dA = 0.0;
	uint n = GetSize();
	for (uint i=0; i<n; ++i)
	{
		int j = i + 1;
		if (j == n) j = 0;
		double dT = m_Data[i].x * m_Data[j].y - m_Data[j].x * m_Data[i].y;
		pt.x += (m_Data[i].x + m_Data[j].x) * dT;
		pt.y += (m_Data[i].y + m_Data[j].y) * dT;
		dA += dT;
	}
	double dN = 3 * dA;
	pt.x /= dN;
	pt.y /= dN;
	return pt;
}

/** (Approximate) centroid of a polygon, using simple averaging of the
 * vertices.
 */
DPoint2 DLine2::Centroid2() const
{
	DPoint2 result(0,0);
	uint n = GetSize();
	for (uint i=0; i<n; ++i)
		result += m_Data[i];
	result /= n;
	return result;
}

//////////////////////////////////////////////////////////////////////////
// FLine2 methods
//

float FLine2::Area() const
{
	int n = (int) GetSize();
	float A = 0.0f;
	for (int p=n-1,q=0; q<n; p=q++)
		A += GetAt(p).x*GetAt(q).y - GetAt(q).x*GetAt(p).y;
	return A*0.5f;
}

float FLine2::SegmentLength(uint i) const
{
	uint j = (i < GetSize()-1) ? i+1 : 0;
	return (GetAt(j) - GetAt(i)).Length();
}


/**
* Return the nearest point (of the points which make up the line).
* This is not the same as the closest place on the line, which may
* lie between the defining points.
*
* \param Point The input point.
* \param iIndex Index of the first point of the nearest line segment.
* \param fClosest Distance from the FLine2 to the input point.
*/
void FLine2::NearestPoint(const FPoint2 &Point, int &iIndex, float &fClosest) const
{
	uint iNumPoints = GetSize();
	fClosest = 1E9;
	float fDistance;

	for (uint i = 0; i < iNumPoints; i++)
	{
		fDistance = (Point - GetAt(i)).Length();
		if (fDistance < fClosest)
		{
			fClosest = fDistance;
			iIndex = i;
		}
	}
}

/**
* A slightly faster version of NearestPoint which doesn't provide
* the distance to the closest point.
*/
void FLine2::NearestPoint(const FPoint2 &Point, int &iIndex) const
{
	uint iNumPoints = GetSize();
	float fClosest = 1E9;
	float fDistance;

	for (uint i = 0; i < iNumPoints; i++)
	{
		fDistance = (Point - GetAt(i)).LengthSquared();
		if (fDistance < fClosest)
		{
			fClosest = fDistance;
			iIndex = i;
		}
	}
}

bool FLine2::NearestSegment(const FPoint2 &Point, int &iIndex, float &dist, FPoint2 &Intersection) const
{
	int iNumPoints = GetSize();
	int i, closest = -1;
	float fMagnitude;
	float fDistance;
	float fMinDistance = 1E9;
	float fU;
	FPoint2 p0, p1, p2;

	for (i = 0; i < iNumPoints; i++)
	{
		p0 = GetAt(i);
		p1 = GetAt((i + 1) % iNumPoints);
		fMagnitude = SegmentLength(i);
		// Calculate U for standard line equation:
		// values of U between 0.0 and +1.0 mean normal intersects segment
		fU = (((Point.x - p0.x) * (p1.x - p0.x)) +
			((Point.y - p0.y) * (p1.y - p0.y))) / (fMagnitude * fMagnitude);
		if ((fU < 0.0) || (fU > 1.0))
			continue;
		p2.x = p0.x + fU * (p1.x - p0.x);
		p2.y = p0.y + fU * (p1.y - p0.y);
		fDistance = FPoint2(Point - p2).Length();
		if (fDistance < fMinDistance)
		{
			fMinDistance = fDistance;
			closest = i;
			Intersection = p2;
		}
	}
	if (closest != -1)
	{
		iIndex = closest;
		dist = fMinDistance;
		return true;
	}
	return false;
}

void FLine2::InsertPointAfter(int iInsertAfter, const FPoint2 &Point)
{
	int iNumPoints = GetSize();
	int iIndex;
	if (iInsertAfter == iNumPoints - 1)
		Append(Point);
	else
	{
		for (iIndex = iNumPoints - 1; iIndex > iInsertAfter ; iIndex--)
			SetAt(iIndex + 1, GetAt(iIndex));
		SetAt(iInsertAfter + 1, Point);
	}
}

void FLine2::ReverseOrder()
{
	FPoint2 p;
	uint i, size = GetSize();
	for (i = 0; i < size/2; i++)
	{
		p = GetAt(i);
		SetAt(i, GetAt(size-1-i));
		SetAt(size-1-i, p);
	}
}

bool FLine2::IsConvex() const
{
	int positive = 0;
	int negative = 0;
	uint length = GetSize();

	for (uint i = 0; i < length; i++)
	{
		FPoint2 &p0 = GetAt(i);
		FPoint2 &p1 = GetAt((i+1) % length);
		FPoint2 &p2 = GetAt((i+2) % length);
		float cross = (p1-p0).Cross(p2-p1);

		if ( cross < 0 )
			negative++;
		else
			positive++;
	}
	return (negative == 0 || positive == 0);
}

//////////////////////////////////////////////////////////////////////////
// DLine3 methods

void DLine3::Add(const DPoint2 &p)
{
	int size = GetSize();
	for (int i=0; i < size; i++)
	{
		GetAt(i).x += p.x;
		GetAt(i).y += p.y;
	}
}

/**
* Returns the location of the horizontally closest point on the line to a given 2D point.
*
* \param Point The input point.
* \param iIndex Index of the first point of the nearest line segment.
* \param dist Distance from the DLine2 to the input point.
* \param Intersection The closest point on the DLine2.
*
* \return True if a closest point was found.
*/
bool DLine3::NearestSegment2D(const DPoint2 &Point, int &iIndex,
							double &dist, DPoint3 &Intersection) const
{
	int iNumPoints = GetSize();
	int i, closest = -1;
	double dMagnitude;
	double dDistance;
	double dMinDistance = 1E9;
	double dU;
	DPoint2 p0, p1, p2;

	for (i = 0; i < iNumPoints; i++)
	{
		DPoint3 p3_0 = GetAt(i);
		DPoint3 p3_1 = GetAt((i + 1) % iNumPoints);

		p0.Set(p3_0.x, p3_0.y);
		p1.Set(p3_1.x, p3_1.y);
		dMagnitude = (p1 - p0).Length();
		// Calculate U for standard line equation:
		// values of U between 0.0 and +1.0 mean normal intersects segment
		dU = (((Point.x - p0.x) * (p1.x - p0.x)) +
			((Point.y - p0.y) * (p1.y - p0.y))) / (dMagnitude * dMagnitude);
		if ((dU < 0.0) || (dU > 1.0))
			continue;
		p2 = p0 + (p1 - p0) * dU;
		dDistance = (Point - p2).Length();
		if (dDistance < dMinDistance)
		{
			dMinDistance = dDistance;
			closest = i;
			Intersection = p3_0 + (p3_1 - p3_0) * dU;
		}
	}
	if (closest != -1)
	{
		iIndex = closest;
		dist = dMinDistance;
		return true;
	}
	return false;
}

/**
* Return the nearest (2D distance) point (of the points which make up the
* line). This is not the same as the closest place on the line, which may
* lie between the defining points.
*
* \param Point The input point.
* \param iIndex Index of the first point of the nearest line segment.
* \param dClosest Distance from the DLine2 to the input point.
*/
void DLine3::NearestPoint2D(const DPoint2 &Point, int &iIndex, double &dClosest) const
{
	uint iNumPoints = GetSize();
	dClosest = 1E9;
	double dDistance;
	DPoint2 p2;

	for (uint i = 0; i < iNumPoints; i++)
	{
		p2.x = GetAt(i).x;
		p2.y = GetAt(i).y;
		dDistance = (Point - p2).Length();
		if (dDistance < dClosest)
		{
			dClosest = dDistance;
			iIndex = i;
		}
	}
}

/**
* With the assumption that this set of points defines a closed polygon,
* test whether the polygon contains a given point.  Since a simple array
* of points being interpreted as a polygon, this means the data may be
* concave or convex, but not contains holes.  The third (Z) coordinate
* is ignored.
*/
bool DLine3::ContainsPoint2D(const DPoint2 &p) const
{
	if (GetData() != NULL)
		return CrossingsTest(GetData(), GetSize(), p);
	else
		return false;
}


//////////////////////////////////////////////////////////////////////////
// FLine3 methods

void FLine3::ReverseOrder()
{
	FPoint3 p;
	int i, size = GetSize();
	for (i = 0; i < size/2; i++)
	{
		p = GetAt(i);
		SetAt(i, GetAt(size-1-i));
		SetAt(size-1-i, p);
	}
}


/////////////////////////////////////////////////////////////////////////////
// DRECT methods
//

/**
 * Fast, naive test for polyline/polygon inside rectangle.
 * \return true if any point of the poly is inside the rectangle.
 */
bool DRECT::ContainsLine(const DLine2 &line) const
{
	for (uint i = 0; i < line.GetSize(); i++)
	{
		if (!ContainsPoint(line[i]))
			return false;
	}
	return true;
}

bool DRECT::ContainsLine(const DLine3 &line) const
{
	for (uint i = 0; i < line.GetSize(); i++)
	{
		if (!ContainsPoint(line[i]))
			return false;
	}
	return true;
}


/////////////////////////////////////////////////////////////////////////////
// DPolygon2 methods
//

uint DPolygon2::NumTotalVertices() const
{
	uint total = 0, r;
	for (r = 0; r < size(); r++)
		total += at(r).GetSize();
	return total;
}

bool DPolygon2::ComputeExtents(DRECT &rect) const
{
	if (size() == 0)
		return false;

	rect.SetInsideOut();
	for (uint ringnum = 0; ringnum < size(); ringnum++)
	{
		const DLine2 &ring = at(ringnum);
		for (unsigned i = 0; i < ring.GetSize(); i++)
			rect.GrowToContainPoint(ring[i]);
	}
	return true;
}

bool DPolygon2::ContainsPoint(const DPoint2 &p) const
{
	// We don't have a point-in-polygon test which actually takes multiple
	//  rings as input.  So, there are two options, both inefficient:
	//
	// 1. Unwind the rings into a temporary array with all the vertices
	//	present, and test against that.
	//
	// 2. Test against each ring: inside the outside ring, and outside of
	//	each internal ring.  (This is what GEOS does.)
	//
	// Here we take option 1.  TODO: make this faster and more efficient!!

	if (size() > 1)
	{
#if 0
		// Option 1 - although this _should_ work, in practice, it fails
		DLine2 all_vertices;
		GetAsDLine2(all_vertices);

		return all_vertices.ContainsPoint(p);
#endif
		// Option 2
		bool outer_ring = at(0).ContainsPoint(p);
		if (!outer_ring)
			return false;
		for (uint i = 1; i < size(); i++)
		{
			// It seems that we would have to reverse the order of the inner
			//  ring vertices, but in practice, it works fine without doing so.
			bool inner_ring = at(i).ContainsPoint(p);
			if (inner_ring)
				return false;
		}
		return true;
	}
	else
		return at(0).ContainsPoint(p);
}

/**
 * Normally the polygon is stored as a series of rings.  Sometimes it is
 * necessary to access the polygon as a single array of points instead.
 *
 * This method fills a provided DLine2 with all the points of all rings
 * of the polygon.
 */
void DPolygon2::GetAsDLine2(DLine2 &dline) const
{
	uint i, total = 0, ringnum;

	for (ringnum = 0; ringnum < size(); ringnum++)
		total += (at(ringnum).GetSize() + 1);

	dline.SetMaxSize(total);

	for (ringnum = 0; ringnum < size(); ringnum++)
	{
		const DLine2 &ring = at(ringnum);
		for (i = 0; i < ring.GetSize(); i++)
			dline.Append(ring[i]);

		// close each ring by repeating the first point of the ring
		dline.Append(ring[0]);
	}
}

int DPolygon2::WhichRing(int &iVtxNum) const
{
	for (uint ring = 0; ring < size(); ring++)
	{
		uint size = at(ring).GetSize();
		if ((uint)iVtxNum < size)
			return ring;
		iVtxNum -= size;
	}
	return -1;
}

/**
* Return the nearest point (of the points which make up the line).
* This is not the same as the closest place on the line, which may
* lie between the defining points; use NearestSegment to find that.
*
* \param Point The input point.
* \param iIndex Index of the first point of the nearest line segment.
* \param dClosest Distance from the DLine2 to the input point.
*/
void DPolygon2::NearestPoint(const DPoint2 &Point, int &iIndex, double &dClosest) const
{
	dClosest = 1E9;

	int test_index;
	double test_distance;

	uint rings = size();
	int ring_start = 0;
	for (uint ring = 0; ring < rings; ring++)
	{
		const DLine2 &loop = at(ring);
		loop.NearestPoint(Point, test_index, test_distance);
		if (test_distance < dClosest)
		{
			dClosest = test_distance;
			iIndex = ring_start + test_index;
		}
		ring_start += loop.GetSize();
	}
}

/**
* Returns the location of the closest point on the polygon to a given point.
*
* \param Point The input point.
* \param iIndex Index of the first point of the nearest line segment.
* \param dist Distance from the DPolygon2 to the input point.
* \param Intersection The closest point on the DPolygon2.
*
* \return True if a closest point was found.
*/
bool DPolygon2::NearestSegment(const DPoint2 &Point, int &iIndex,
							   double &dist, DPoint2 &Intersection) const
{
	int closest = -1;
	double dClosest = 1E9;

	int test_index;
	double test_distance;
	DPoint2 test_intersection;

	uint rings = size();
	int ring_start = 0;
	for (uint ring = 0; ring < rings; ring++)
	{
		const DLine2 &loop = at(ring);
		loop.NearestSegment(Point, test_index, test_distance, test_intersection);
		if (test_distance < dClosest)
		{
			dClosest = test_distance;
			closest = ring_start + test_index;
			Intersection = test_intersection;
		}
		ring_start += loop.GetSize();
	}
	if (closest != -1)
	{
		iIndex = closest;
		dist = dClosest;
		return true;
	}
	return false;
}

/**
 * Add the given amount to all coordinates of the polygon.  Spatially, this
 * offsets the location of the polygon.
 */
void DPolygon2::Add(const DPoint2 &p)
{
	for (uint ringnum = 0; ringnum < size(); ringnum++)
	{
		DLine2 &ring = at(ringnum);
		for (uint i = 0; i < ring.GetSize(); i++)
			ring[i] += p;
	}
}

/**
* Multiplies (scales) all the coordinates of the polygon.
*/
void DPolygon2::Mult(double factor)
{
	for (uint ringnum = 0; ringnum < size(); ringnum++)
	{
		DLine2 &ring = at(ringnum);
		for (uint i = 0; i < ring.GetSize(); i++)
			ring[i].Mult(factor, factor);
	}
}

void DPolygon2::ReverseOrder()
{
	for (uint ringnum = 0; ringnum < size(); ringnum++)
	{
		DLine2 &ring = at(ringnum);
		ring.ReverseOrder();
	}
}

/**
 * The insertion point can be on the outer ring, or any inner ring.
 */
void  DPolygon2::InsertPointAfter(int iInsertAfter, const DPoint2 &Point)
{
	for (uint ring = 0; ring < size(); ring++)
	{
		uint size = at(ring).GetSize();
		if ((uint)iInsertAfter < size)
		{
			// remove the point from this ring
			at(ring).InsertPointAfter(iInsertAfter, Point);
			return;
		}
		iInsertAfter -= size;
	}
}

/**
 * The Nth point can be on the outer ring, or any inner ring.
 */
void DPolygon2::RemovePoint(int N)
{
	for (uint ring = 0; ring < size(); ring++)
	{
		uint size = at(ring).GetSize();
		if ((uint)N < size)
		{
			// remove the point from this ring
			at(ring).RemovePoint(N);
			return;
		}
		N -= size;
	}
}

/**
 For each ring of the polygon, remove any points which are degenerate (less than
 dEpsilon apart).
 \return The number of points that were removed.
 */
int DPolygon2::RemoveDegeneratePoints(double dEpsilon)
{
	int removed = 0;
	for (uint ring = 0; ring < size(); ring++)
	{
		DLine2 &dline = at(ring);
		removed += dline.RemoveDegeneratePoints(dEpsilon, true);
		if (dline.GetSize() < 3)
		{
			int bad = 1;
		}
	}
	return removed;
}

/**
 For each ring of the polygon, remove any points which are co-linear, defined
 as point p(n) which is less than dEpsilon displaced from the line from P(n-1) to P(n+1)
 \return The number of points that were removed.
 */
int DPolygon2::RemoveColinearPoints(double dEpsilon)
{
	int removed = 0;
	for (uint ring = 0; ring < size(); ring++)
	{
		DLine2 &dline = at(ring);
		removed += dline.RemoveColinearPoints(dEpsilon, true);
		if (dline.GetSize() < 3)
		{
			int bad = 1;
		}
	}
	return removed;
}


/////////////////////////////////////////////////////////////////////////////
// FPolygon3 methods
//

/**
* Add the given amount to all coordinates of the polygon.  Spatially, this
* offsets the location of the polygon.
*/
void FPolygon3::Add(const FPoint3 &p)
{
	for (uint ringnum = 0; ringnum < size(); ringnum++)
	{
		FLine3 &ring = at(ringnum);
		for (uint i = 0; i < ring.GetSize(); i++)
			ring[i] += p;
	}
}

/**
* Multiplies (scales) all the coordinates of the polygon.
*/
void FPolygon3::Mult(float factor)
{
	for (uint ringnum = 0; ringnum < size(); ringnum++)
	{
		FLine3 &ring = at(ringnum);
		for (uint i = 0; i < ring.GetSize(); i++)
			ring[i] *= factor;
	}
}

void FPolygon3::ReverseOrder()
{
	for (uint ringnum = 0; ringnum < size(); ringnum++)
	{
		FLine3 &ring = at(ringnum);
		ring.ReverseOrder();
	}
}

uint FPolygon3::NumTotalVertices() const
{
	uint total = 0, r;
	for (r = 0; r < size(); r++)
		total += at(r).GetSize();
	return total;
}

int FPolygon3::WhichRing(int &iVtxNum) const
{
	for (uint ring = 0; ring < size(); ring++)
	{
		uint size = at(ring).GetSize();
		if ((uint)iVtxNum < size)
			return ring;
		iVtxNum -= size;
	}
	return -1;
}


/////////////////////////////////////////////////////////////////////////////
// FQuat methods

void FQuat::AxisAngle(const FPoint3 &axis, float angle)
{
	float inversenorm  = 1.0f/axis.Length();
	float coshalfangle = cosf( 0.5f*angle );
	float sinhalfangle = sinf( 0.5f*angle );

	x = axis.x * sinhalfangle * inversenorm;
	y = axis.y * sinhalfangle * inversenorm;
	z = axis.z * sinhalfangle * inversenorm;
	w = coshalfangle;
}

/**
* Set quaternion to be equivalent to specified matrix.
*/
void FQuat::SetFromMatrix(const FMatrix3 &mat)
{
	// Source: Gamasutra, Rotating Objects Using Quaternions
	//http://www.gamasutra.com/features/19980703/quaternions_01.htm
	float  tr, s;
	float  tq[4];
	int	i, j, k;

	int nxt[3] = {1, 2, 0};

	tr = mat(0,0) + mat(1,1) + mat(2,2);

	// check the diagonal
	if (tr > 0.0)
	{
		s = (float)sqrt (tr + 1.0);
		w = s / 2.0f;
		s = 0.5f / s;
		x = (mat(2,1) - mat(1,2)) * s;
		y = (mat(0,2) - mat(2,0)) * s;
		z = (mat(1,0) - mat(0,1)) * s;
	}
	else
	{
		// diagonal is negative
		i = 0;
		if (mat(1,1) > mat(0,0))
			i = 1;
		if (mat(2,2) > mat(i,i))
			i = 2;
		j = nxt[i];
		k = nxt[j];

		s = (float)sqrt ((mat(i,i) - (mat(j,j) + mat(k,k))) + 1.0);

		tq[i] = s * 0.5f;

		if (s != 0.0f)
			s = 0.5f / s;

		tq[3] = (mat(k,j) - mat(j,k)) * s;
		tq[j] = (mat(j,i) + mat(i,j)) * s;
		tq[k] = (mat(k,i) + mat(i,k)) * s;

		x = tq[0];
		y = tq[1];
		z = tq[2];
		w = tq[3];
	}
}

void FQuat::SetFromVectors(const FPoint3 &forward, const FPoint3 &up)
{
	FMatrix3 m3;
	m3.SetFromVectors(forward, up);
	SetFromMatrix(m3);
}

void FQuat::SetFromVector(const FPoint3 &direction)
{
	FMatrix3 m3;
	m3.MakeOrientation(direction);
	SetFromMatrix(m3);
}

/**
* Get the equivalent matrix for this quaternion.
*/
void FQuat::GetMatrix(FMatrix3 &mat) const
{
	// Source: Gamasutra, Rotating Objects Using Quaternions
	//http://www.gamasutra.com/features/19980703/quaternions_01.htm
	float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

	// calculate coefficients
	x2 = x + x;
	y2 = y + y;
	z2 = z + z;

	xx = x * x2;
	xy = x * y2;
	xz = x * z2;

	yy = y * y2;
	yz = y * z2;
	zz = z * z2;

	wx = w * x2;
	wy = w * y2;
	wz = w * z2;

	mat.Set(0, 0, 1.0f - (yy + zz));
	mat.Set(0, 1, xy - wz);
	mat.Set(0, 2, xz + wy);

	mat.Set(1, 0, xy + wz);
	mat.Set(1, 1, 1.0f - (xx + zz));
	mat.Set(1, 2, yz - wx);

	mat.Set(2, 0, xz - wy);
	mat.Set(2, 1, yz + wx);
	mat.Set(2, 2, 1.0f - (xx + yy));
}

/**
* Spherical Linear Interpolation.
* As f goes from 0 to 1, the quaternion goes from "from" to "to".
*/
void FQuat::Slerp(const FQuat &from, const FQuat &to, double f)
{
	/// Reference: Shoemake at SIGGRAPH 89, See also:
	/// http://www.gamasutra.com/features/19980703/quaternions_01.htm

	const double epsilon = 0.00001;
	double omega, cosomega, sinomega, scale_from, scale_to;

	FQuat quatTo(to);

	// this is a dot product
	cosomega = from.x*to.x + from.y*to.y + from.z*to.z + from.w*to.w;

	if (cosomega < 0.0)
	{
		cosomega = -cosomega;
		quatTo.Set(-to.x, -to.y, -to.z, -to.w);
	}

	if ((1.0 - cosomega) > epsilon)
	{
		omega = acos(cosomega);		// 0 <= omega <= Pi (for acos)
		sinomega = sin(omega);
		scale_from = sin((1.0-f)*omega)/sinomega;
		scale_to = sin(f*omega)/sinomega;
	}
	else
	{
		// The ends of the vectors are very close, we can use simple linear
		// interpolation - no need to worry about "spherical" interpolation
		scale_from = 1.0 - f;
		scale_to = f;
	}
	x = (float) (from.x * scale_from + quatTo.x * scale_to);
	y = (float) (from.y * scale_from + quatTo.y * scale_to);
	z = (float) (from.z * scale_from + quatTo.z * scale_to);
	w = (float) (from.w * scale_from + quatTo.w * scale_to);
}

void FQuat::Invert()
{
	float l2 = LengthSquared();
	x = -x / l2;
	y = -y / l2;
	z = -z / l2;
	w = w / l2;
}

const FQuat FQuat::operator*(const FQuat &q) const
{
	return FQuat(q.w*x + q.x*w + q.y*z - q.z*y,
		q.w*y - q.x*z + q.y*w + q.z*x,
		q.w*z + q.x*y - q.y*x + q.z*w,
		q.w*w - q.x*x - q.y*y - q.z*z );
}
FQuat &FQuat::operator*=(const FQuat &q)
{
	float fx = q.w*x + q.x*w + q.y*z - q.z*y;
	float fy = q.w*y - q.x*z + q.y*w + q.z*x;
	float fz = q.w*z + q.x*y - q.y*x + q.z*w;
	w =		   q.w*w - q.x*x - q.y*y - q.z*z;
	z = fz;
	y = fy;
	x = fx;
	return (*this);
}

const FQuat FQuat::operator/(const FQuat &q) const
{
	return ( (*this) * q.Inverse() );
}

FQuat &FQuat::operator/=(const FQuat &q)
{
	(*this) = (*this) * q.Inverse();
	return (*this);
}


/////////////////////////////////////////////////////////////////////////////
// FPQ

void FPQ::FromMatrix(const FMatrix4 &matrix)
{
	FMatrix3 m3 = matrix;
	q.SetFromMatrix(m3);
	p = matrix.GetTrans();
}

void FPQ::ToMatrix(FMatrix4 &matrix)
{
	FMatrix3 m3;
	q.GetMatrix(m3);
	matrix.SetFromMatrix3(m3);
	matrix.SetTrans(p);
}

void FPQ::Interpolate(const FPQ &from, const FPQ &to, float f)
{
	p = from.p + (to.p - from.p) * f;
	q.Slerp(from.q, to.q, f);
}


/////////////////////////////////////////////////////////////////////////////
// RGBi

void RGBi::Crop()
{
	if (r < 0) r = 0;
	else if (r > 255) r = 255;
	if (g < 0) g = 0;
	else if (g > 255) g = 255;
	if (b < 0) b = 0;
	else if (b > 255) b = 255;
}


/////////////////////////////////////////////////////////////////////////////
// RGBAi

void RGBAi::Crop()
{
	if (r < 0) r = 0;
	else if (r > 255) r = 255;
	if (g < 0) g = 0;
	else if (g > 255) g = 255;
	if (b < 0) b = 0;
	else if (b > 255) b = 255;
	if (a < 0) a = 0;
	else if (a > 255) a = 255;
}

///////////////////////////////////////////////////////////////////////
// FPlane

FPlane::FPlane(const FPoint3& p, const FPoint3& q, const FPoint3& r)
{
	// Construct from three points
	float fpx = p.x - r.x;
	float fpy = p.y - r.y;
	float fpz = p.z - r.z;
	float fqx = q.x - r.x;
	float fqy = q.y - r.y;
	float fqz = q.z - r.z;

	x = fpy * fqz - fqy * fpz;
	y = fpz * fqx - fqz * fpx;
	z = fpx * fqy - fqx * fpy;
	w = - x * r.x - y * r.y - z * r.z;
}

FPlane::FPlane(const FPoint3& Point, const FPoint3& Normal)
{
	// Construct from point and normal
	x = Normal.x;
	y = Normal.y;
	z = Normal.z;
	w = - x * Normal.x - y * Normal.y - z * Normal.z;
}

/**
* Find the intersection of two planes. which in the general case is a line.
* The line is provided as a ray (origin and direction).
*
* \return status, which is either INTERSECTING, COPLANAR, or PARALLEL.  In
*		the case of INTERSECTING, then the Origin and Direction parameters
*		describe the line of intersection.
*/
const FPlane::IntersectionType FPlane::Intersection(const FPlane &Plane,
													FPoint3 &Origin, FPoint3 &Direction, float fEpsilon) const
{
	double a = x;
	double b = y;
	double c = z;
	double d = w;
	double p = Plane.x;
	double q = Plane.y;
	double r = Plane.z;
	double s = Plane.w;
	double Determinant;

	Determinant = a * q - p * b;
	if (fabs(Determinant - 0.0) > fEpsilon)
	{
		Origin.Set((float)((b * s - d * q) / Determinant), (float)((p * d - a * s) / Determinant), 0.0f);
		Direction.Set((float)(b * r - c * q), (float)(p * c - a * r), (float)Determinant);
		return INTERSECTING;
	}

	Determinant = a * r - p * c;
	if (fabs(Determinant - 0.0) > fEpsilon)
	{
		Origin.Set((float)((c * s - d * r) / Determinant), 0.0f, (float)((p * d - a * s) / Determinant));
		Direction.Set((float)(c * q - b * r), (float)Determinant, (float)(p * b - a * q));
		return INTERSECTING;
	}

	Determinant = b * r - c * q;
	if (fabs(Determinant - 0.0) > fEpsilon)
	{
		Origin.Set(0.0f, (float)((c * s - d * r) / Determinant), (float)((d * q - b * s) / Determinant));
		Direction.Set((float)Determinant, (float)(c * p - a * r), (float)(a * q - b * p));
		return INTERSECTING;
	}

	if (a != 0.0 || p != 0.0)
	{
		if (fabs(a * s - p * d) <= fEpsilon)
			return COPLANAR;
		else
			return PARALLEL;
	}
	if (b != 0.0 || q != 0.0)
	{
		if (fabs(b * s - q * d) <= fEpsilon)
			return COPLANAR;
		else
			return PARALLEL;
	}
	if (c != 0.0 || r != 0.0)
	{
		if (fabs(c * s - r * d) <= fEpsilon)
			return COPLANAR;
		else
			return PARALLEL;
	}
	return PARALLEL;
}

/**
* Compute Ray-Plane intersection.
*
* \param Origin, Direction	The position and direction that define the ray.
* \param fDistance			The distance along the ray to the intersection point.
* \param Intersection		The intersection point.
* \param fEpsilon			Small value to test for numeric equivalenccy.
*
* \return The intersection result (PARALLEL, FACING_AWAY, or INTERSECTING)
*/
const FPlane::IntersectionType FPlane::RayIntersection(const FPoint3& Origin,
													   const FPoint3& Direction, float &fDistance, FPoint3& Intersection,
													   float fEpsilon) const
{
	// Intersection of ray with plane
	float NdotV = Dot(Direction);

	if (fabs(0.0 - NdotV) <= fEpsilon)
		return PARALLEL;

	fDistance = -(Dot(Origin) + w) / NdotV;

	if (fDistance < 0.0)
		return FACING_AWAY;

	Intersection = Origin + Direction * fDistance;

	return INTERSECTING;
}

const FPlane::IntersectionType FPlane::LineIntersection(const FPoint3& Origin, const FPoint3& Direction, FPoint3& Intersection, float fEpsilon) const
{
	float Numerator = x * Origin.x + y * Origin.y + z * Origin.z + w;
	float Denominator = x * Direction.x + y * Direction.y + z * Direction.z;

	if (fabs(0.0 - Denominator) <= fEpsilon)
	{
		if (fabs(0.0 - Numerator) <= fEpsilon)
			return COPLANAR;
		else
			return PARALLEL;
	}
	Intersection.x = (Denominator * Origin.x - Numerator * Direction.x) / Denominator;
	Intersection.y = (Denominator * Origin.y - Numerator * Direction.y) / Denominator;
	Intersection.z = (Denominator * Origin.z - Numerator * Direction.z) / Denominator;
	return INTERSECTING;
}

const FPlane::IntersectionType FPlane::ThreePlanesIntersection(const FPlane& Plane1, const FPlane& Plane2, FPoint3& Intersection, float fEpsilon) const
{
	FPoint3 Origin;
	FPoint3 Direction;

	IntersectionType Type = Plane1.Intersection(Plane2, Origin, Direction, fEpsilon);

	if (INTERSECTING == Type)
	{
		// Planes have an intersection line
		Type = LineIntersection(Origin, Direction, Intersection, fEpsilon);
		return Type;
	}
	else
		return Type;
}


/////////////////////////////////////////////////////////////////////////////
// ScopedLocale

ScopedLocale::ScopedLocale(int category, const char *locale_string)
{
	// Store and override
	m_old_locale = setlocale(category, NULL);
	setlocale(category, locale_string);
}

ScopedLocale::~ScopedLocale()
{
	// Restore
	if (m_old_locale.size() > 0)
		setlocale(LC_NUMERIC, m_old_locale.c_str());
}


///////////////////////////
// useful helper functions
//

float random_offset(float x)
{
	return (((float)rand()/RAND_MAX) - 0.5f) * x;
}

float random(float x)
{
	return ((float)rand()/RAND_MAX) * x;
}

/**
* Compute base-2 logarithm of an integer.
* There's probably an even simpler way to do this with sneaky
* logical ops or without a loop, but this works.
*/
int vt_log2(int n)
{
	int temp = n, i;
	for (i = 0; temp > 1; i++)
		temp >>= 1;
	return i;
}

/**
* Compute base-2 logarithm of an float.
*/
const float LN_2 = 0.693147180559945f;
float vt_log2f(float n)
{
	return logf(n) / LN_2;
}

/**
 Given three points in a polyline, determine a side vector will will offset
 the polyline, to the left, by a unit width.
 */
double AngleSideVector(const DPoint2 &p0, const DPoint2 &p1, const DPoint2 &p2,
					  DPoint2 &sideways)
{
	// Look at vectors to previous and next points
	const DPoint2 v0 = (p1-p0).Normalize();
	const DPoint2 v1 = (p2-p1).Normalize();

	// we flip axes to turn the path vector 90 degrees (normal to path)
	DPoint2 bisector(-(v0.y + v1.y), v0.x + v1.x);
	bisector.Normalize();

	double wider;
	const double dot = v0.Dot(-v1);
	if (dot <= -0.99 || dot >= 0.99)
	{
		// close enough to colinear, no need to widen
		wider = 1.0f;
	}
	else
	{
		// factor to widen this corner is proportional to the angle
		double angle = acos(dot);
		wider = (float) (1.0 / sin(angle / 2));
		bisector *= wider;
	}
	sideways = bisector;
	return wider;
}

/**
 Given three points in a polyline, determine a side vector will will offset
 the polyline by a unit width.
 */
float AngleSideVector(const FPoint3 &p0, const FPoint3 &p1, const FPoint3 &p2,
					  FPoint3 &sideways)
{
	// Look at vectors to previous and next points
	const FPoint3 v0 = (p1-p0).Normalize();
	const FPoint3 v1 = (p2-p1).Normalize();

	// we flip axes to turn the path vector 90 degrees (normal to path)
	FPoint3 bisector(v0.z + v1.z, 0, -(v0.x + v1.x));
	bisector.Normalize();

	float wider;
	const float dot = v0.Dot(-v1);
	if (dot <= -0.97 || dot >= 0.97)
	{
		// close enough to colinear, no need to widen
		wider = 1.0f;
	}
	else
	{
		// factor to widen this corner is proportional to the angle
		float angle = acos(dot);
		wider = (float) (1.0 / sin(angle / 2));
		bisector *= wider;
	}
	sideways = bisector;
	return wider;
}

/*
* ======= Crossings algorithm ============================================
*
* Adapted from: Graphics Gems IV
*
* Shoot a test ray along +X axis.  The strategy, from MacMartin, is to
* compare vertex Y values to the testing point's Y and quickly discard
* edges which are entirely to one side of the test ray.
*/
/**
* Point-in-polygon test.
*
* Tests whether the 2D polygon \a pgon with \a numverts number of vertices
* contains the test point \a point.
*
* \return 1 if inside, 0 if outside.
*/
bool CrossingsTest(const DPoint2 *pgon, int numverts, const DPoint2 &point)
{
	register int j;
	register double ty, tx;
	register bool inside_flag, yflag0, yflag1, xflag0;
	const DPoint2 *vertex0, *vertex1;

	tx = point.x;
	ty = point.y;

	vertex0 = pgon + (numverts-1);
	/* get test bit for above/below X axis */
	yflag0 = (vertex0->y >= ty);
	vertex1 = pgon;

	inside_flag = false;
	for (j = numverts+1; --j;)
	{
		yflag1 = (vertex1->y >= ty);
		/* check if endpoints straddle (are on opposite sides) of X axis
		* (i.e. the Y's differ); if so, +X ray could intersect this edge.
		*/
		if (yflag0 != yflag1)
		{
			xflag0 = (vertex0->x >= tx);
			/* check if endpoints are on same side of the Y axis (i.e. X's
			* are the same); if so, it's easy to test if edge hits or misses.
			*/
			if (xflag0 == (vertex1->x >= tx))
			{
				/* if edge's X values both right of the point, must hit */
				if (xflag0) inside_flag = !inside_flag;
			}
			else
			{
				/* compute intersection of pgon segment with +X ray, note
				* if >= point's X; if so, the ray hits it.
				*/
				if ((vertex1->x - (vertex1->y-ty) *
					(vertex0->x - vertex1->x)/(vertex0->y - vertex1->y)) >= tx) {
						inside_flag = !inside_flag;
				}
			}
		}
		/* move to next pair of vertices, retaining info as possible */
		yflag0 = yflag1;
		vertex0 = vertex1;
		vertex1 += 1;
	}

	return inside_flag;
}
/**
* Another version of CrossingsTest that accepts 3D rather than 2D points.
* Only the first two components (X and Y) are tested, so this allows you to
* do a 2D test with a 3D polygon.
*/
bool CrossingsTest(const DPoint3 *pgon, int numverts, const DPoint2 &point)
{
	register int j;
	register double ty, tx;
	register bool inside_flag, yflag0, yflag1, xflag0;
	const DPoint3 *vertex0, *vertex1;

	tx = point.x;
	ty = point.y;

	vertex0 = pgon + (numverts-1);
	/* get test bit for above/below X axis */
	yflag0 = (vertex0->y >= ty);
	vertex1 = pgon;

	inside_flag = false;
	for (j = numverts+1; --j;)
	{
		yflag1 = (vertex1->y >= ty);
		/* check if endpoints straddle (are on opposite sides) of X axis
		* (i.e. the Y's differ); if so, +X ray could intersect this edge.
		*/
		if (yflag0 != yflag1)
		{
			xflag0 = (vertex0->x >= tx);
			/* check if endpoints are on same side of the Y axis (i.e. X's
			* are the same); if so, it's easy to test if edge hits or misses.
			*/
			if (xflag0 == (vertex1->x >= tx))
			{
				/* if edge's X values both right of the point, must hit */
				if (xflag0) inside_flag = !inside_flag;
			}
			else
			{
				/* compute intersection of pgon segment with +X ray, note
				* if >= point's X; if so, the ray hits it.
				*/
				if ((vertex1->x - (vertex1->y-ty) *
					(vertex0->x - vertex1->x)/(vertex0->y - vertex1->y)) >= tx) {
						inside_flag = !inside_flag;
				}
			}
		}
		/* move to next pair of vertices, retaining info as possible */
		yflag0 = yflag1;
		vertex0 = vertex1;
		vertex1 += 1;
	}

	return inside_flag;
}

/**
* 2D point in triangle containment test.
*
* \return true if the point is inside the triangle, otherwise false.
*/
bool PointInTriangle(const FPoint2 &p, const FPoint2 &p1, const FPoint2 &p2,
					 const FPoint2 &p3)
{
	const float fAB = (p.y-p1.y)*(p2.x-p1.x) - (p.x-p1.x)*(p2.y-p1.y);
	const float fBC = (p.y-p2.y)*(p3.x-p2.x) - (p.x-p2.x)*(p3.y-p2.y);
	const float fCA = (p.y-p3.y)*(p1.x-p3.x) - (p.x-p3.x)*(p1.y-p3.y);

	return (fAB * fBC >= 0) && (fBC * fCA >= 0) && (fAB * fCA >= 0);
}

/**
* 2D point in triangle containment test.
*
* \return true if the point is inside the triangle, otherwise false.
*/
bool PointInTriangle(const DPoint2 &p, const DPoint2 &p1, const DPoint2 &p2,
					 const DPoint2 &p3)
{
	const double fAB = (p.y-p1.y)*(p2.x-p1.x) - (p.x-p1.x)*(p2.y-p1.y);
	const double fBC = (p.y-p2.y)*(p3.x-p2.x) - (p.x-p2.x)*(p3.y-p2.y);
	const double fCA = (p.y-p3.y)*(p1.x-p3.x) - (p.x-p3.x)*(p1.y-p3.y);

	return (fAB * fBC >= 0) && (fBC * fCA >= 0) && (fAB * fCA >= 0);
}

/**
* Compute the 3 barycentric coordinates of a 2D point in a 2D triangle.
*
* \return false if a problem was encountered (e.g. degenerate triangle),
* otherwise true.
*/
bool BarycentricCoords(const FPoint2 &p1, const FPoint2 &p2,
					   const FPoint2 &p3, const FPoint2 &p, float fBary[3])
{
	FPoint2 vec13 = p1 - p3;
	FPoint2 vec23 = p2 - p3;
	FPoint2 vecp3 = p - p3;

	float m11 = vec13.Dot(vec13);
	float m12 = vec13.Dot(vec23);
	float m22 = vec23.Dot(vec23);
	float fR0 = vec13.Dot(vecp3);
	float fR1 = vec23.Dot(vecp3);
	float fDet = m11*m22 - m12*m12;
	if (fDet == 0.0f)
		return false;
	float fInvDet = 1.0f/fDet;

	fBary[0] = (m22*fR0 - m12*fR1)*fInvDet;
	fBary[1] = (m11*fR1 - m12*fR0)*fInvDet;
	fBary[2] = 1.0f - fBary[0] - fBary[1];
	return true;
}


/**
* Compute the 3 barycentric coordinates of a 2D point in a 2D triangle.
*
* \return false if a problem was encountered (e.g. degenerate triangle),
* otherwise true.
*/
bool BarycentricCoords(const DPoint2 &p1, const DPoint2 &p2,
					   const DPoint2 &p3, const DPoint2 &p, double fBary[3])
{
	DPoint2 vec13 = p1 - p3;
	DPoint2 vec23 = p2 - p3;
	DPoint2 vecp3 = p - p3;

	double m11 = vec13.Dot(vec13);
	double m12 = vec13.Dot(vec23);
	double m22 = vec23.Dot(vec23);
	double fR0 = vec13.Dot(vecp3);
	double fR1 = vec23.Dot(vecp3);
	double fDet = m11*m22 - m12*m12;
	if (fDet == 0.0f)
		return false;
	double fInvDet = 1.0f/fDet;

	fBary[0] = (m22*fR0 - m12*fR1)*fInvDet;
	fBary[1] = (m11*fR1 - m12*fR0)*fInvDet;
	fBary[2] = 1.0f - fBary[0] - fBary[1];
	return true;
}

/**
* Find the intersection of 3 planes.
*
* \return true if there was a valid intersection point.  The function
* will fail and return false for any degenerate case, e.g. when any two
* of the planes are parallel.
*/
bool PlaneIntersection(const FPlane &plane1, const FPlane &plane2,
					   const FPlane &plane3, FPoint3 &result)
{
	FPoint3 n1 = plane1;
	FPoint3 n2 = plane2;
	FPoint3 n3 = plane3;
	float d1 = -plane1.w;
	float d2 = -plane2.w;
	float d3 = -plane3.w;

	// formula for result: P = (d1(n2 X n3) + d2(n3 X n1) + d3(n1 X n2)) / n1 . (n2 X n3)
	FPoint3 numer = n2.Cross(n3) * d1 + n3.Cross(n1) * d2 + n1.Cross(n2) * d3;
	float denom = n1.Dot(n2.Cross(n3));

	// The following epsilon value was chosen carefully from empirical
	// results.  If you have buildings whose roofs are being drawn
	// incorrectly, this value can be increased, but do so with care.
	if (fabsf(denom) < 1E-7)
		return false;

	result = numer / denom;
	return true;
}

/**
* Find the closest distance from a point to a line segment.
*
* \param p1, p2	The points which define the line segment.
* \param p3		The point to which to measure distance.
*/
double DistancePointToLine(const DPoint2 &p1, const DPoint2 &p2, const DPoint2 &p3)
{
	DPoint2 vec = (p2 - p1);
	double len2 = vec.LengthSquared();
	double u = ((p3.x - p1.x)*(p2.x - p1.x) + (p3.y - p1.y)*(p2.y - p1.y)) / len2;

	// constrain u to remain within the line segment
	if (u < 0) u = 0;
	if (u > 1) u = 1;

	DPoint2 closest = p1 + (vec * u);
	return (p3 - closest).Length();
}

/**
* Given the four vectors defining your points A1, A2, B1, and B2, the points
* result1 and result2 on the lines A1A2 and B1B2, respectively, which are
* closest together, and their distance, d, apart.
*/
float DistanceLineToLine(const FPoint3 &A1, const FPoint3 &A2,
						 const FPoint3 &B1, const FPoint3 &B2,
						 FPoint3 &result1, FPoint3 &result2)
{
	FPoint3 U = A2-A1;
	FPoint3 V = B2-B1;
	FPoint3 W = U.Cross(V);
	result1 = A1 + U * (((B1-A1).Cross(V)).Dot(W) / W.Dot(W));
	result2 = B1 + V * (((B1-A1).Cross(U)).Dot(W) / W.Dot(W));
	float d = (result2-result1).Length();
	return d;
}

/** Calculate closest distance from one line segment to another.
* Input:  two 3D line segments A1>A2 and B1->B2
* Return: the points result1 and result2 on the lines A1A2 and B1B2,
* respectively, which are closest together, and their distance apart.
*/
#define dot(a,b) a.Dot(b)
#define SMALL_NUM 0.0001f
float DistanceSegmentToSegment(const FPoint3 &A1, const FPoint3 &A2,
							   const FPoint3 &B1, const FPoint3 &B2,
							   FPoint3 &result1, FPoint3 &result2)
{
	FPoint3   u = A2 - A1;
	FPoint3   v = B2 - B1;
	FPoint3   w = A1 - B1;
	float    a = dot(u,u);        // always >= 0
	float    b = dot(u,v);
	float    c = dot(v,v);        // always >= 0
	float    d = dot(u,w);
	float    e = dot(v,w);
	float    D = a*c - b*b;       // always >= 0
	float    sc, sN, sD = D;      // sc = sN / sD, default sD = D >= 0
	float    tc, tN, tD = D;      // tc = tN / tD, default tD = D >= 0

	// compute the line parameters of the two closest points
	if (D < SMALL_NUM) { // the lines are almost parallel
		sN = 0.0;        // force using point P0 on segment S1
		sD = 1.0;        // to prevent possible division by 0.0 later
		tN = e;
		tD = c;
	}
	else {                // get the closest points on the infinite lines
		sN = (b*e - c*d);
		tN = (a*e - b*d);
		if (sN < 0.0) {       // sc < 0 => the s=0 edge is visible
			sN = 0.0;
			tN = e;
			tD = c;
		}
		else if (sN > sD) {  // sc > 1 => the s=1 edge is visible
			sN = sD;
			tN = e + b;
			tD = c;
		}
	}

	if (tN < 0.0) {           // tc < 0 => the t=0 edge is visible
		tN = 0.0;
		// recompute sc for this edge
		if (-d < 0.0)
			sN = 0.0;
		else if (-d > a)
			sN = sD;
		else {
			sN = -d;
			sD = a;
		}
	}
	else if (tN > tD) {      // tc > 1 => the t=1 edge is visible
		tN = tD;
		// recompute sc for this edge
		if ((-d + b) < 0.0)
			sN = 0;
		else if ((-d + b) > a)
			sN = sD;
		else {
			sN = (-d + b);
			sD = a;
		}
	}
	// finally do the division to get sc and tc
	sc = (fabs(sN) < SMALL_NUM ? 0.0f : sN / sD);
	tc = (fabs(tN) < SMALL_NUM ? 0.0f : tN / tD);

	// get the difference of the two closest points
	FPoint3   dP = w + (u * sc) - (v * tc);  // = S1(sc) - S2(tc)

	// and the two closest points themselves
	result1 = A1 + u * sc;
	result2 = B1 + v * tc;

	return dP.Length();   // return the closest distance
}

/* return values */
#define DONT_INTERSECT 0
#define DO_INTERSECT   1
#define PARALLEL       2
bool SameSigns(double a, double b) { return ((a < 0 && b < 0) || (a > 0 && b > 0)); }

/**
 * Test whether two 2D line segment intersect or not.
 * Based on the algorithm 'Faster Line Segment Intersection' by Franklin
 * Antonio
 *
 * \return 0 if the segments do not intersect, 1 if they do,
 *		2 if the segments are parallel.
 */
int LineSegmentsIntersect(const DPoint2 &p1, const DPoint2 &p2,
						 const DPoint2 &p3, const DPoint2 &p4, DPoint2 *result)
{

	double Ax, Bx, Cx, Ay, By, Cy, d, e, f, num, offset;
	double x1lo, x1hi, y1lo, y1hi;

	Ax = p2.x-p1.x;
	Bx = p3.x-p4.x;

	if(Ax<0) {						/* X bound box test*/
		x1lo = p2.x;
		x1hi = p1.x;
	} else {
		x1hi = p2.x;
		x1lo = p1.x;
	}

	if(Bx>0) {
		if(x1hi < p4.x || p3.x < x1lo) return DONT_INTERSECT;
	} else {
		if(x1hi < p3.x || p4.x < x1lo) return DONT_INTERSECT;
	}

	Ay = p2.y - p1.y;
	By = p3.y - p4.y;

	if(Ay<0) {				/* Y bound box test*/
		y1lo = p2.y;
		y1hi = p1.y;
	} else {
		y1hi = p2.y;
		y1lo = p1.y;
	}

	if(By>0) {
		if(y1hi < p4.y || p3.y < y1lo) return DONT_INTERSECT;
	} else {
		if(y1hi < p3.y || p4.y < y1lo) return DONT_INTERSECT;
	}

	Cx = p1.x - p3.x;
	Cy = p1.y - p3.y;
	d = By*Cx - Bx*Cy;		/* alpha numerator*/
	f = Ay*Bx - Ax*By;		/* both denominator*/
	if (f>0) {				/* alpha tests*/
		if(d<0 || d>f) return DONT_INTERSECT;
	} else {
		if(d>0 || d<f) return DONT_INTERSECT;
	}

	e = Ax*Cy - Ay*Cx;		/* beta numerator*/
	if (f>0) {				/* beta tests*/
		if(e<0 || e>f) return DONT_INTERSECT;
	} else {
		if(e>0 || e<f) return DONT_INTERSECT;
	}

	if (fabs(f) < 1E-4)
		return PARALLEL;

	if (result != NULL)
	{
		/* compute intersection coordinates */
		num = d*Ax;									/* numerator */
		offset = SameSigns(num,f) ? f/2 : -f/2;	/* round direction*/
		result->x = p1.x + (num+offset) / f;			/* intersection x */

		num = d*Ay;
		offset = SameSigns(num,f) ? f/2 : -f/2;
		result->y = p1.y + (num+offset) / f;			/* intersection y */
	}
	return DO_INTERSECT;
}

void vtLogMatrix(const FMatrix4 &mat)
{
	VTLOG("Mat: %f %f %f %f\n"
		"	 %f %f %f %f\n"
		"	 %f %f %f %f\n"
		"	 %f %f %f %f\n",
		mat(0,0), mat(1,0), mat(2,0), mat(3,0),
		mat(0,1), mat(1,1), mat(2,1), mat(3,1),
		mat(0,2), mat(1,2), mat(2,2), mat(3,2),
		mat(0,3), mat(1,3), mat(2,3), mat(3,3));
}

void vtLogMatrix(const FMatrix3 &mat)
{
	VTLOG("Mat: %f %f %f\n"
		"	 %f %f %f\n"
		"	 %f %f %f\n",
		mat(0,0), mat(1,0), mat(2,0),
		mat(0,1), mat(1,1), mat(2,1),
		mat(0,2), mat(1,2), mat(2,2));
}

//
// Ray-Sphere intersection
//
bool RaySphereIntersection(const FPoint3 &origin, const FPoint3 &dir,
						   const FSphere &sphere, int &iQuantity, FPoint3 point[2])
{
	// set up quadratic Q(t) = a*t^2 + 2*b*t + c
	FPoint3 kDiff = origin - sphere.center;
	double fA = dir.LengthSquared();
	double fB = kDiff.Dot(dir);
	double fC = kDiff.LengthSquared() -
		sphere.radius*sphere.radius;

	double afT[2];
	double fDiscr = fB*fB - fA*fC;
	if ( fDiscr < 0.0 )
	{
		iQuantity = 0;
	}
	else if ( fDiscr > 0.0 )
	{
		double fRoot = sqrt(fDiscr);
		double fInvA = 1.0/fA;
		afT[0] = (-fB - fRoot)*fInvA;
		afT[1] = (-fB + fRoot)*fInvA;

		if ( afT[0] >= 0.0 )
			iQuantity = 2;
		else if ( afT[1] >= 0.0 )
			iQuantity = 1;
		else
			iQuantity = 0;
	}
	else
	{
		afT[0] = -fB/fA;
		iQuantity = ( afT[0] >= 0.0 ? 1 : 0 );
	}

	for (int i = 0; i < iQuantity; i++)
		point[i] = origin + dir * (float) afT[i];

	return iQuantity > 0;
}

/**
 * To convert between 3D and 2D entities, project to and from the 3D
 * coordinate system's XZ plane.
 */
void ProjectionXZ(const FLine3 &fline3, DLine2 &dline2)
{
	uint size = fline3.GetSize();
	dline2.SetSize(size);
	for (uint i = 0; i < size; i++)
	{
		const FPoint3 &fp3 = fline3[i];
		dline2[i].Set(fp3.x, -fp3.z);
	}
}

void ProjectionXZ(const FPolygon3 &fpoly3, DPolygon2 &dpoly2)
{
	uint rings = fpoly3.size();
	dpoly2.resize(rings);
	for (uint i = 0; i < rings; i++)
		ProjectionXZ(fpoly3[i], dpoly2[i]);
}

void ProjectionXZ(const DLine2 &dline2, float fY, FLine3 &fline3)
{
	uint size = dline2.GetSize();
	fline3.SetSize(size);
	for (uint i = 0; i < size; i++)
	{
		const DPoint2 &dp2 = dline2[i];
		fline3[i].Set((float) dp2.x, fY, (float) -dp2.y);
	}
}

void ProjectionXZ(const DPolygon2 &dpoly2, float fY, FPolygon3 &fpoly3)
{
	uint rings = dpoly2.size();
	fpoly3.resize(rings);
	for (uint i = 0; i < rings; i++)
		ProjectionXZ(dpoly2[i], fY, fpoly3[i]);
}

double AreaOfTriangle(const DPoint2 &A, const DPoint2 &B, const DPoint2 &C)
{
	const double num = A.x * (B.y - C.y) + B.x * (C.y - A.y) + C.x * (A.y - B.y);
	return fabs(num / 2.0);
}

double AreaOfTriangle(const DPoint3 &A, const DPoint3 &B, const DPoint3 &C)
{
	const DPoint3 cross = (B - A).Cross(C - A);
	return cross.Length() / 2.0;
}

