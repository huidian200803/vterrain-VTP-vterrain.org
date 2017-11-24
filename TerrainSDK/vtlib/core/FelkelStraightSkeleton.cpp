//
// FelkelStraightSkeleton.cpp: implementation of the vtStraightSkeleton class.
//
// Copyright (c) 2003-2009 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
// Straight skeleton algorithm and original implementation
// courtesy of Petr Felkel and Stepan Obdrzalek (petr.felkel@tiani.com)
// Re-implemented for the Virtual Terrain Project (vterrain.org)
// by Roger James (www.beardandsandals.co.uk)
//

#include "FelkelStraightSkeleton.h"
#include "vtdata/vtString.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

vtStraightSkeleton::vtStraightSkeleton()
{

}

vtStraightSkeleton::~vtStraightSkeleton()
{

}

CSkeleton& vtStraightSkeleton::MakeSkeleton(ContourVector &contours)
{
	try
	{
		while (m_iq.size ())
			m_iq.pop ();
		m_vl.erase(m_vl.begin(), m_vl.end());
		m_skeleton.erase(m_skeleton.begin(), m_skeleton.end());
		m_boundaryedges.erase(m_boundaryedges.begin(), m_boundaryedges.end());

		for (size_t ci = 0; ci < contours.size(); ci++)
		{
			Contour &points = contours[ci];

			Contour::iterator first = points.begin();
			if (first == points.end())
				break;

			Contour::iterator next = first;

			while (++next != points.end ())
			{
				if (*first == *next)
					points.erase (next);
				else
					first = next;
				next = first;
			}

			int s = points.size();
			CVertexList::iterator start = m_vl.end();
			CVertexList::iterator from = start;
			CVertexList::iterator to = start;

			for (int f = 0; f <= s; f++)
			{
				if (0 == f)
				{
					m_vl.push_back(CVertex(points[0].m_Point, points[s - 1].m_Point, points[s - 1].m_Slope, points[1].m_Point, points[0].m_Slope));
					to = m_vl.end();
					to--;
					start = to;
				}
				else if (f == s)
				{
					from = to;
					to = start;
					m_boundaryedges.push_front(CSkeletonLine(*from, *to));
				}
				else
				{
					from = to;
					m_vl.push_back(CVertex(points[f].m_Point, points[f - 1].m_Point, points[f - 1].m_Slope, points[(f + 1) % s].m_Point, points[f].m_Slope));
					to = m_vl.end();
					to--;
					m_boundaryedges.push_front(CSkeletonLine(*from, *to));
				}
			}
		}

		m_NumberOfBoundaryVertices = m_vl.size();
		m_NumberOfBoundaryEdges = m_boundaryedges.size();

		if (m_vl.size() < 3)
		{
			vtString *str = new vtString;
			str->Format("%s (%d): Eave Polygon too small\n", __FILE__, __LINE__);
			throw str;
		}

		CVertexList::iterator i;

		size_t vn = 0, cn = 0;

		CVertexList::iterator contourBegin;

		for (i = m_vl.begin (); i != m_vl.end (); i++)
		{
			(*i).m_prevVertex = &*m_vl.prev(i);
			(*i).m_nextVertex = &*m_vl.next(i);
			(*i).m_leftVertex = &*i;
			(*i).m_rightVertex = &*i;
			if (vn == 0)
				contourBegin = i;
			if (vn == contours [cn].size () - 1)
			{
				(*i).m_nextVertex = &*contourBegin;
				(*contourBegin).m_prevVertex = &*i;
				vn = 0;
				cn ++;
			}
			else
				vn ++;
		}


#ifdef FELKELDEBUG
		VTLOG("Building initial intersection queue\n");
#endif
		for (i = m_vl.begin(); i != m_vl.end (); i++)
		{
			if (!(*i).m_done)
			{
				CIntersection is(m_vl, *i);
				if (is.m_height != CN_INFINITY)
					m_iq.push(is);
			}
		}

#ifdef FELKELDEBUG
		VTLOG("Processing intersection queue\n");
#endif
		while (m_iq.size ())
		{
			CIntersection i = m_iq.top ();

			m_iq.pop ();

#ifdef FELKELDEBUG
			VTLOG("Processing %d %d left done %d right done %d\n",
				i.m_leftVertex->m_ID, i.m_rightVertex->m_ID, i.m_leftVertex->m_done, i.m_rightVertex->m_done);
#endif
			if ((NULL == i.m_leftVertex) || (NULL == i.m_rightVertex))
			{
				vtString *str = new vtString;
				str->Format("%s (%d): Invalid intersection queue entry\n", __FILE__, __LINE__);
				throw str;
			}
			if (i.m_leftVertex->m_done && i.m_rightVertex->m_done)
				continue;
			if (i.m_leftVertex->m_done || i.m_rightVertex->m_done)
			{
				if (!i.m_leftVertex->m_done)
					m_iq.push(CIntersection (m_vl, *i.m_leftVertex));
				if (!i.m_rightVertex->m_done)
					m_iq.push(CIntersection (m_vl, *i.m_rightVertex));
				continue;
			}

#ifdef FELKELDEBUG
			if (!(i.m_leftVertex->m_prevVertex != i.m_rightVertex))
				VTLOG("%s %d Assert failed\n", __FILE__, __LINE__);
			if (!(i.m_rightVertex->m_nextVertex != i.m_leftVertex))
				VTLOG("%s %d Assert failed\n", __FILE__, __LINE__);
#endif
			if (i.m_type == CIntersection::CONVEX)
				if (i.m_leftVertex->m_prevVertex->m_prevVertex == i.m_rightVertex || i.m_rightVertex->m_nextVertex->m_nextVertex == i.m_leftVertex)
					i.ApplyLast3(m_skeleton, m_vl);
				else
					i.ApplyConvexIntersection(m_skeleton, m_vl, m_iq);
			if (i.m_type == CIntersection :: NONCONVEX)
				i.ApplyNonconvexIntersection(m_skeleton, m_vl, m_iq, cn == 1);
		}

#ifdef FELKELDEBUG
		Dump();
#endif

		FixSkeleton();

#ifdef FELKELDEBUG
		Dump();
#endif
	}
	catch (vtString *str)
	{
		m_skeleton.erase(m_skeleton.begin(), m_skeleton.end());
		VTLOG(*str);
		delete str;
	}

	return m_skeleton;
}

CSkeleton& vtStraightSkeleton::MakeSkeleton(Contour &points)
{
	ContourVector vv;

	vv.push_back (points);

	return MakeSkeleton(vv);
}

CSkeleton vtStraightSkeleton::CompleteWingedEdgeStructure(ContourVector &contours)
{
	// Save current skeleton
	int iOldSize = m_skeleton.size();
	int i;
	CSkeleton::iterator si;

	for (size_t ci = 0; ci < contours.size(); ci++)
	{
		Contour& points = contours[ci];
		for (size_t pi = 0; pi < points.size(); pi++)
		{
			C3DPoint& LowerPoint = points[pi].m_Point;
			C3DPoint& HigherPoint = points[(pi+1)%points.size()].m_Point;


			// Find a matching empty lower left
			for (i = 0, si = m_skeleton.begin(); i < iOldSize; i++, si++)
			{
				CSkeletonLine& Line = *si;
				if ((Line.m_lower.m_vertex->m_point == LowerPoint) && (Line.m_lower.LeftID() == -1))
					break;
			}
			if (i == iOldSize)
			{
				VTLOG("CompleteWingedEdgeStructure - Failed to find matching empty lower left\n");
				return CSkeleton();
			}
			CSkeletonLine& OldLowerLeft = *si;

			// Find a matching empty lower right
			for (i = 0, si = m_skeleton.begin(); i < iOldSize; i++, si++)
			{
				CSkeletonLine& Line = *si;
				if ((Line.m_lower.m_vertex->m_point == HigherPoint) && (Line.m_lower.RightID() == -1))
					break;
			}
			if (i == iOldSize)
			{
				VTLOG("CompleteWingedEdgeStructure - Failed to find matching empty lower right\n");
				return CSkeleton();
			}
			CSkeletonLine& OldLowerRight = *si;

			m_skeleton.push_back(CSkeletonLine(*OldLowerLeft.m_lower.m_vertex, *OldLowerRight.m_lower.m_vertex));

			CSkeletonLine& NewEdge = m_skeleton.back();

			NewEdge.m_lower.m_right = &OldLowerLeft;
			OldLowerLeft.m_lower.m_left = &NewEdge;
			NewEdge.m_higher.m_left = &OldLowerRight;
			OldLowerRight.m_lower.m_right = &NewEdge;
		}
	}
#ifdef FELKELDEBUG
	Dump();
#endif
	return m_skeleton;
}

void vtStraightSkeleton::FixSkeleton()
{
	// Search the skeleton list for consecutive pairs of incorrectly linked lines
	CSkeleton::iterator s1 = m_skeleton.begin();
	for (uint i = 0; i < m_skeleton.size() - 2; i++, s1++)
	{
		CSkeletonLine& Lower = *s1++;
		CSkeletonLine& Higher1 = *s1++;
		CSkeletonLine& Higher2 = *s1;

		if ((Higher1.m_higher.RightID() == -1) &&
			(Higher1.m_lower.LeftID() == -1) &&
			(Higher2.m_higher.LeftID() == -1) &&
			(Higher2.m_lower.RightID() == -1) &&
			(Higher1.m_higher.VertexID() == Higher2.m_lower.VertexID()) &&
			(Higher1.m_lower.VertexID() == Higher2.m_higher.VertexID())) // I don't think I cam make this test much tighter !!!
		{
			CSkeletonLine* pLeft = Lower.m_higher.m_left;
			CSkeletonLine* pRight = Lower.m_higher.m_right;
			const CVertex* pVertex = Lower.m_higher.m_vertex;
			if ((NULL == pLeft) || (NULL == pRight) || (NULL == pVertex))
			{
				vtString *str = new vtString;
				str->Format("%s (%d): Problem fixing skeleton\n", __FILE__, __LINE__);
				throw str;
			}
			// Fix up the left side
			if ((pLeft->m_lower.VertexID() == pVertex->m_ID) || (pLeft->m_lower.VertexID() == pVertex->m_ID + 1))
			{
				// Fix up lower end
				pLeft->m_lower.m_vertex = pVertex;
				pLeft->m_lower.m_left = pRight;
				if (pLeft->m_lower.RightID() != Lower.m_ID)
				{
					vtString *str = new vtString;
					str->Format("%s (%d): Left Lower Right ID != Lower ID\n", __FILE__, __LINE__);
					throw str;
				}
			}
			else if ((pLeft->m_higher.VertexID() == pVertex->m_ID) || (pLeft->m_higher.VertexID() == pVertex->m_ID + 1))
			{
				// Fix up upper end
				pLeft->m_higher.m_vertex = pVertex;
				pLeft->m_higher.m_left = pRight;
				if (pLeft->m_higher.RightID() != Lower.m_ID)
				{
					vtString *str = new vtString;
					str->Format("%s (%d): Left Higher Right ID != Lower ID\n", __FILE__, __LINE__);
					throw str;
				}
			}
			else
			{
				vtString *str = new vtString;
				str->Format("%s (%d): Problem fixing left side\n", __FILE__, __LINE__);
				throw str;
			}
			// Fix up the right side
			if ((pRight->m_lower.VertexID() == pVertex->m_ID) || (pRight->m_lower.VertexID() == pVertex->m_ID + 1))
			{
				// Fix up lower end
				pRight->m_lower.m_vertex = pVertex;
				pRight->m_lower.m_right = pLeft;
				if (pRight->m_lower.LeftID() != Lower.m_ID)
				{
					vtString *str = new vtString;
					str->Format("%s (%d): Right Lower Left ID != Lower ID\n", __FILE__, __LINE__);
					throw str;
				}
			}
			else if ((pRight->m_higher.VertexID() == pVertex->m_ID) || (pRight->m_higher.VertexID() == pVertex->m_ID + 1))
			{
				// Fix up upper end
				pRight->m_higher.m_vertex = pVertex;
				pRight->m_higher.m_right = pLeft;
				if (pRight->m_higher.LeftID() != Lower.m_ID)
				{
					vtString *str = new vtString;
					str->Format("%s (%d): Right Higher Left ID != Lower ID\n", __FILE__, __LINE__);
					throw str;
				}
			}
			else
			{
				vtString *str = new vtString;
				str->Format("%s (%d): FixSkeleton - Problem fixing right side\n", __FILE__, __LINE__);
				throw str;
			}
		}
		s1--;
		s1--;
	}
}

#ifdef FELKELDEBUG
void vtStraightSkeleton::Dump()
{
	int i;

	VTLOG("Skeleton:\n");

	i = 0;
	for (CSkeleton::iterator s1 = m_skeleton.begin(); s1 != m_skeleton.end(); s1++)
	{
		CSkeletonLine& db = (*s1);
		VTLOG("ID: %d lower leftID %d rightID %d vertexID %d (%f %f %f)\nhigher leftID %d rightID %d vertexID %d (%f %f %f)\n",
			db.m_ID,
			db.m_lower.LeftID(),
			db.m_lower.RightID(),
			db.m_lower.VertexID(), db.m_lower.m_vertex->m_point.m_x, db.m_lower.m_vertex->m_point.m_y, db.m_lower.m_vertex->m_point.m_z,
			db.m_higher.LeftID(),
			db.m_higher.RightID(),
			db.m_higher.VertexID(), db.m_higher.m_vertex->m_point.m_x, db.m_higher.m_vertex->m_point.m_y, db.m_higher.m_vertex->m_point.m_z);
	}
}
#endif
