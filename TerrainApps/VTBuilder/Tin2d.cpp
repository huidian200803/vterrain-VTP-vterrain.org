//
// Tin2d.cpp
//
// Copyright (c) 2005-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "Tin2d.h"
#include "Options.h"
#include "ScaledView.h"

#include "vtdata/ElevationGrid.h"
#include "vtdata/Features.h"
#include "vtdata/Triangulate.h"
#include "vtdata/vtLog.h"

////////////////////////////////////////////////////////////////////

vtTin2d::vtTin2d()
{
	m_fEdgeLen = NULL;
	m_bConstrain = false;
}

vtTin2d::~vtTin2d()
{
	FreeEdgeLengths();
}

/**
 Create a TIN from a grid, by simply triangulating all the valid heixels
 in the grid.
 */
vtTin2d::vtTin2d(vtElevationGrid *grid)
{
	m_fEdgeLen = NULL;
	m_bConstrain = false;

	int cols, rows;
	grid->GetDimensions(cols, rows);
	m_crs = grid->GetCRS();

	// This isn't an optimal algorithm, but it's not a common operation, so
	// cpu/mem efficiency isn't vital.
	//
	// First step: naively make vertices from every grid coordinate.
	DPoint2 p;
	for (int x = 0; x < cols; x++)
	{
		for (int y = 0; y < rows; y++)
		{
			grid->GetEarthPoint(x, y, p);
			AddVert(p, grid->GetFValue(x, y));
		}
	}
	for (int x = 0; x < cols-1; x++)
	{
		for (int y = 0; y < rows-1; y++)
		{
			int base = x * rows + y;

			// Only add triangles where the heixels have valid data.
			const bool b1 = (grid->GetFValue(x, y) != INVALID_ELEVATION);
			const bool b2 = (grid->GetFValue(x + 1, y) != INVALID_ELEVATION);
			const bool b3 = (grid->GetFValue(x, y + 1) != INVALID_ELEVATION);
			const bool b4 = (grid->GetFValue(x + 1, y + 1) != INVALID_ELEVATION);
			const int valid = (int) b1 + b2 + b3 + b4;
			if (valid < 3)
				continue;
			else if (valid == 4)
			{
				// Add both triangles
				AddTri(base, base + rows, base+1);
				AddTri(base+1, base + rows, base + rows+1);
			}
			else if (!b1)
				AddTri(base + rows, base + rows + 1, base+1);
			else if (!b2)
				AddTri(base + rows + 1, base + 1, base);
			else if (!b3)
				AddTri(base, base + rows, base + rows + 1);
			else if (!b4)
				AddTri(base + rows, base + 1, base);
		}
	}

	// Remove any unused vertices
	RemoveUnusedVertices();

	ComputeExtents();
}

#define ANSI_DECLARATORS
#define REAL double
extern "C" {
#include "vtdata/triangle/triangle.h"
}

/**
 * Create a TIN from a set of 3D points, using the Triangle library to derive
 *  the set of triangles from the points.
 */
vtTin2d::vtTin2d(vtFeatureSetPoint3D *set)
{
	m_fEdgeLen = NULL;
	m_bConstrain = false;

	struct triangulateio in, out;
	int i;
	DPoint3 p3;

	// point list
	in.numberofpoints = set->NumEntities();
	in.pointlist = (REAL *) malloc(in.numberofpoints * 2 * sizeof(REAL));
	in.numberofpointattributes = 1;
	in.pointattributelist = (REAL *) malloc(in.numberofpoints * sizeof(REAL));

	for ( i = 0; i < in.numberofpoints; ++i )
	{
		set->GetPoint(i, p3);
		in.pointlist[2*i] = p3.x;
		in.pointlist[2*i + 1] = p3.y;

		in.pointattributelist[i] = p3.z;
	}

	in.pointmarkerlist = (int *) NULL;
	in.numberoftriangles = 0;

	// no segment list
	in.numberofsegments = 0;
	in.segmentlist = (int *) NULL;
	in.segmentmarkerlist = (int *) NULL;

	// no holes or regions
	in.numberofholes = 0;
	in.holelist = (REAL *) NULL;
	in.numberofregions = 0;
	in.regionlist = (REAL *) NULL;

	// prep the output structures
	out.pointlist = (REAL *) NULL;        // Not needed if -N switch used.
	out.pointattributelist = (REAL *) NULL;
	out.pointmarkerlist = (int *) NULL;   // Not needed if -N or -B switch used.
	out.trianglelist = (int *) NULL;      // Not needed if -E switch used.
	out.triangleattributelist = (REAL *) NULL;
	out.neighborlist = (int *) NULL;      // Needed only if -n switch used.
	out.segmentlist = (int *) NULL;
	out.segmentmarkerlist = (int *) NULL;
	out.edgelist = (int *) NULL;          // Needed only if -e switch used.
	out.edgemarkerlist = (int *) NULL;    // Needed if -e used and -B not used.

	// Triangulate the points.  Switches are chosen:
	// number everything from zero (z),
	triangulate("z", &in, &out, NULL);

	// now copy the triangle results back into vtdata structures
	for ( i = 0; i < out.numberofpoints; ++i )
	{
		float z = out.pointattributelist[i];
		AddVert(DPoint2(out.pointlist[2*i], out.pointlist[2*i + 1]), z);
	}
	for ( i = 0; i < out.numberoftriangles; ++i )
	{
		int n1 = out.trianglelist[i * 3];
		int n2 = out.trianglelist[i * 3 + 1];
		int n3 = out.trianglelist[i * 3 + 2];
		AddTri(n1, n2, n3);
	}
	// free mem allocated to the "Triangle" structures
	free(in.pointlist);
	free(in.pointattributelist);

	free(out.pointlist);
	free(out.pointattributelist);
	free(out.pointmarkerlist);
	free(out.trianglelist);
	free(out.triangleattributelist);
	free(out.neighborlist);
	free(out.segmentlist);
	free(out.segmentmarkerlist);
	free(out.edgelist);
	free(out.edgemarkerlist);

	ComputeExtents();
	// Adopt CRS from the featureset
	m_crs = set->GetAtCRS();
}

/**
 Construct a TIN from polygons.  The polygons features are triangulated and
 assigned a height, either from the feature fields, or a fixed height.

 \param set			The polygons featureset to use.
 \param iFieldNum	The number of the field to use; for example, if the fields
	are "Area", "Distance", "Height", and "Height" contains the value you want,
	then pass 2.  To use an fixed height instead for all triangles, pass -1.
 \param fHeight		The absolute height to use, if iFieldNum is -1.
 */
vtTin2d::vtTin2d(vtFeatureSetPolygon *set, int iFieldNum, float fHeight)
{
	VTLOG1("Construct TIN from Polygons\n");

	m_fEdgeLen = NULL;
	m_bConstrain = false;

	int num = set->NumEntities();
	for (int i = 0; i < num; ++i)
	{
		DPolygon2 &dpoly = set->GetPolygon(i);

		// Get z value
		float z;
		if (iFieldNum == -1)
			z = fHeight;
		else
			z = set->GetFloatValue(i, iFieldNum);

		DLine2 result;
#if VTDEBUG
	VTLOG(" Polygon %d/%d: %d rings, first ring %d points\n",
		i, num, dpoly.size(), dpoly[0].GetSize());
#endif
		CallTriangle(dpoly, result);
//		CallPoly2Tri(dpoly, result);	// TEST
		int res = result.GetSize();
		int base = NumVerts();
		for (int j = 0; j < res; j++)
		{
			AddVert(result[j], z);
		}
		for (int j = 0; j < res/3; j++)
		{
			AddTri(base + j*3, base + j*3+1, base + j*3+2);
		}
	}
	ComputeExtents();
	// Adopt CRS from the featureset
	m_crs = set->GetAtCRS();

	VTLOG1("Construct TIN from Polygons: done\n");
}

void vtTin2d::MakeOutline()
{
	// Find all the unique edges (all internal edges appear twice)
	for (uint i = 0; i < NumTris(); i++)
	{
		int v0 = m_tri[i*3+0];
		int v1 = m_tri[i*3+1];
		int v2 = m_tri[i*3+2];
		if (v0 < v1)
			m_edges.AddUniqueEdge(IntPair(v0, v1));
		else
			m_edges.AddUniqueEdge(IntPair(v1, v0));
		if (v1 < v2)
			m_edges.AddUniqueEdge(IntPair(v1, v2));
		else
			m_edges.AddUniqueEdge(IntPair(v2, v1));
		if (v2 < v0)
			m_edges.AddUniqueEdge(IntPair(v2, v0));
		else
			m_edges.AddUniqueEdge(IntPair(v0, v2));
	}
}

int vtTin2d::GetMemoryUsed() const
{
	int bytes = 0;

	bytes += sizeof(vtTin2d);
	bytes += sizeof(DPoint2) * m_vert.GetSize();
	bytes += sizeof(int) * m_tri.size();
	bytes += sizeof(float) * m_z.size();

	if (m_fEdgeLen)
		bytes += sizeof(double) * NumTris();

	if (m_trianglebins)
		bytes += m_trianglebins->GetMemoryUsed();

	return bytes;
}

void vtTin2d::DrawTin(vtScaledView *pView)
{
	// Dark purple lines
	glColor3f(0.5f, 0, 0.5f);

	bool bDrawSimple = g_Options.GetValueBool(TAG_DRAW_TIN_SIMPLE);
	if (bDrawSimple)
	{
		if (!m_edges.size())
		{
			// extract an outline
			MakeOutline();
		}
		// Just draw the online
		for (Outline::iterator it = m_edges.begin(); it != m_edges.end(); it++)
		{
			// TODO: be much more efficient
			pView->DrawLine(m_vert[it->v0], m_vert[it->v1]);
		}
	}
	else
	{
		// Draw every triangle
		// TODO: be WAY more efficient
		FPoint2 p2;
		uint tris = NumTris();
		for (uint i = 0; i < tris; i++)
		{
			if (m_bConstrain)
			{
				if (m_fEdgeLen[i] > m_fMaxEdge)
					continue;
			}
			int v0 = m_tri[i*3+0];
			int v1 = m_tri[i*3+1];
			int v2 = m_tri[i*3+2];

			pView->DrawLine(m_vert[v0], m_vert[v1]);
			pView->DrawLine(m_vert[v1], m_vert[v2]);
			pView->DrawLine(m_vert[v2], m_vert[v0]);
		}
	}
#if 0
	// For testing purposes, draw the vertices as well
	uint points = NumVerts();
	for (i = 0; i < tris; i++)
	{
		pView->screen(m_vert[i], g_screenbuf[0]);
		pDC->DrawPoint(g_screenbuf[0]);
	}
#endif
}

void vtTin2d::SetConstraint(bool bConstrain, double fMaxEdge)
{
	m_bConstrain = bConstrain;
	m_fMaxEdge = fMaxEdge;
}

void vtTin2d::ComputeEdgeLengths()
{
	int nTris = NumTris();
	m_fEdgeLen = new double[nTris];
	for (int i = 0; i < nTris; i++)
		m_fEdgeLen[i] = GetTriMaxEdgeLength(i);
}

void vtTin2d::CullLongEdgeTris()
{
	if (!m_fEdgeLen)
		return;
	int nTris = NumTris();
	int b1, b2;
	int to = 0;
	int kept = 0;
	for (int i = 0; i < nTris; i++)
	{
		b1 = i * 3;
		if (m_fEdgeLen[i] < m_fMaxEdge)
		{
			// keep
			b2 = to * 3;
			m_tri[b2] = m_tri[b1];
			m_tri[b2+1] = m_tri[b1+1];
			m_tri[b2+2] = m_tri[b1+2];
			to++;
			kept++;
		}
	}
	m_tri.resize(kept*3);
}

void vtTin2d::FreeEdgeLengths()
{
	delete m_fEdgeLen;
	m_fEdgeLen = NULL;
}


