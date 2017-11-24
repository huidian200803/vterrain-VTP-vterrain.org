//
// RoadFix.cpp
//
// contains methods of RoadMapEdit used for fixing and
//  cleaning the roadmap topology
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/vtLog.h"

#include "RoadMapEdit.h"
#include "assert.h"


//helper
float fmin(float a, float b, float c, float d)
{
	if (a < b && a < c && a < d)
		return a;
	if (b < c && b < d)
		return b;
	if (c < d)
		return c;
	return d;
}

//makes sure road end points same as node point
void NodeEdit::EnforceLinkEndpoints()
{
	//for the roads that now end in pN2, move it's end point as well.
	for (int k = 0; k < NumLinks(); k++)
	{
		TLink *r = GetLink(k);
		if (r->GetNode(0) == this)
			r->SetAt(0, m_p);
		else if (r->GetNode(1) == this)
			r->SetAt(r->GetSize() - 1, m_p);
		else
			assert(0);	// bad case!

		((LinkEdit*)r)->m_fLength = r->Length();
	}
}


// merge nodes less than 8m apart
#define TOLERANCE_METERS (8.0f)
#define TOLERANCE_DEGREES (TOLERANCE_METERS/110000)

//
// Since the original data is scattered over many source files,
// any road which crosses a DLG file boundary will be split
// by two nodes, one on each edge of the two files.
//
// This routine will merge any two nodes which are sufficiently
// close together.
//
// Warning: some degerate roads may result.
//
// Return the number removed.
//
int RoadMapEdit::MergeRedundantNodes(bool bDegrees, bool progress_callback(int))
{
	NodeEdit *prev = NULL, *next;
	DPoint2 diff;
	int removed = 0;

	int nodes = NumNodes();
	float total = (float)nodes * nodes / 2;
	int count1 = 0, count = 0, count_tick, count_last_tick = 0;
	float tick_size = (float)total / 100;
	double tolerance, tolerance_squared;

	if (bDegrees)
		tolerance = TOLERANCE_DEGREES;
	else
		tolerance = TOLERANCE_METERS;
	tolerance_squared = tolerance * tolerance;

	NodeEdit *pN, *pN2;

	for (pN = GetFirstNode(); pN && pN->GetNext(); pN = next)
	{
		count1++;
		next = pN->GetNext();
		bool remove = false;
		for (pN2 = next; pN2; pN2 = pN2->GetNext())
		{
			diff = pN2->Pos() - pN->Pos();
			if (diff.LengthSquared() < tolerance_squared)
			{
				remove = true;
				break;
			}
		}
		count += (nodes - count1);
		count_tick = (int) (count / tick_size);
		if (count_tick > count_last_tick)
		{
			count_last_tick = count_tick;
			progress_callback(count_tick);
		}
		if (remove)
		{
			// we've got a pair that need to be merged
			//new point is placed between the 2 original points
			pN2->SetPos((pN2->Pos() + pN->Pos()) / 2.0f);

			// we're going to remove the "pN" node
			// inform any roads which may have referenced it
			ReplaceNode(pN, pN2);

			// to remove pN, link around it
			if (prev)
				prev->SetNext(next);
			else
				m_pFirstNode = next;
			delete pN;

			// for the roads that now end in pN2, move their end points
			pN2->EnforceLinkEndpoints();
			removed++;
		}
		else
			prev = pN;
	}
	VTLOG(" Removed %i nodes\n", removed);
	return removed;
}


//
// Remove links which start and end at the same node, yet don't
// have enough points (less than 3) to be a valid loop
//
// There are many other ways a link can be degenerate, but we
// don't check for them (yet).
//
// Returns the number of links removed.
//
int RoadMapEdit::RemoveDegenerateLinks()
{
	int count = 0;

	LinkEdit *prevL = NULL, *nextL;

	for (LinkEdit *pL = GetFirstLink(); pL; pL = nextL)
	{
		bool bad = false;
		nextL = pL->GetNext();

		// Does it start and end at same node?
		if (pL->GetNode(0) == pL->GetNode(1))
		{
			// Does it have enough points to be a valid loop?
			if (pL->GetSize() < 3)
				bad = true;

			// Is it exceedingly short?
			if (pL->Length() < pL->GetTotalWidth())
				bad = true;
		}
		if (bad)
		{
			// remove it
			if (prevL)
				prevL->SetNext(nextL);
			else
				m_pFirstLink = nextL;

			// notify the nodes that the road is gone
			pL->GetNode(0)->DetachLink(pL);
			pL->GetNode(1)->DetachLink(pL);

			delete pL;
			count++;
		}
		else
			prevL = pL;
	}
	VTLOG(" Removed %i degenerate links.\n", count);
	return count;
}

/*RemoveUnnecessaryNodes
  eliminates unnecessary nodes:
	-has 2 roads intersecting it
	-both roads are the same type
	-intersection is uncontrolled
*/
int RoadMapEdit::RemoveUnnecessaryNodes()
{
	// TODO
	return 0;
}


//
// Does two cleanup steps on the points of the road:
//  1. if the road's endpoint does not match the coordinate of
//		that node, then add a point to make it so
//  2. if the road has any two redundant points (same coordinate)
//		then remove one of them.
// Returns the number of road points affected.
//
int RoadMapEdit::CleanLinkPoints(double epsilon)
{
	int count = 0;

	for (NodeEdit *pN = GetFirstNode(); pN && pN->GetNext(); pN = pN->GetNext())
		pN->EnforceLinkEndpoints();

	for (LinkEdit *pR = GetFirstLink(); pR; pR = pR->GetNext())
	{
		for (uint i = 1; i < pR->GetSize(); i++)
		{
			const DPoint2 &p1 = pR->GetAt(i-1);
			const DPoint2 &p2 = pR->GetAt(i);

			double dist = (p2 - p1).Length();
			if (dist < epsilon)
			{
				// the point is redundant and should be removed
				pR->RemovePoint(i-1);
				pR->Dirtied();
				count++;
				pR->m_fLength = pR->Length();

				// there may be multiple redundant points
				i--;
			}
		}
	}
	return count;
}


//
// deletes roads that either:
//		have the same start and end nodes and have less than 4 points
//		have less than 2 points, regardless of start or end nodes.
//
int RoadMapEdit::DeleteDanglingLinks()
{
	NodeEdit *pN1, *pN2;
	LinkEdit *pR = GetFirstLink();
	LinkEdit *prev = NULL, *next;
	int count = 0;
	while (pR)
	{
		next = pR->GetNext();
		pN1 = pR->GetNode(0);
		pN2 = pR->GetNode(1);
		if ((pN1 == pN2 && pR->GetSize() <4) ||
			(pR->GetSize() < 2))
		{
			//delete the road!
			if (prev)
				prev->SetNext(next);
			else
				m_pFirstLink = next;

			pN1->DetachLink(pR);
			pN2->DetachLink(pR);
			delete pR;
			count++;
		}
		else
			prev = pR;

		pR = next;
	}
	return count;
}


// fix when two different roads meet at the same node along the same path
int RoadMapEdit::FixOverlappedLinks(bool bDegrees)
{
	int fixed = 0, roads;
	DPoint2 p0, p1, diff;
	int i, j;
	LinkEdit *pR1=NULL, *pR2=NULL;

	double tolerance;
	if (bDegrees)
		tolerance = TOLERANCE_DEGREES/8;
	else
		tolerance = TOLERANCE_METERS/8;

	for (NodeEdit *pN = GetFirstNode(); pN && pN->GetNext(); pN = pN->GetNext())
	{
		roads = pN->NumLinks();

		if (roads < 2) continue;

		bool bad = false;
		for (i = 0; i < roads-1 && !bad; i++)
		{
			pR1 = pN->GetLink(i);
			p0 = pN->GetAdjacentLinkPoint2d(i);
			for (j = i+1; j < roads; j++)
			{
				pR2 = pN->GetLink(j);

				if (pR1 == pR2) continue;	// don't worry about loops

				p1 = pN->GetAdjacentLinkPoint2d(j);
				diff = (p1 - p0);
				if (fabs(diff.x) < 1.0f && fabs(diff.y) < 1.0f)
				{
					bad = true;
					break;
				}
			}
		}
		if (!bad) continue;

		// now fix it, by removing the two points of the 2 roads that
		// overlap as they approach the node
		if (pR1->GetNode(0) == pN)
			pR1->RemovePoint(1);			// roads starts here
		else
			pR1->RemovePoint(pR1->GetSize()-2);	// road ends here
		if (pR2->GetNode(0) == pN)
			pR2->RemovePoint(1);			// roads starts here
		else
			pR2->RemovePoint(pR2->GetSize()-2);	// road ends here
		fixed++;
		((LinkEdit*)pR1)->m_fLength = pR1->Length();
		((LinkEdit*)pR2)->m_fLength = pR2->Length();

	}
	return fixed;
}

//
// Returns the absolute difference between 2 angles.
//
float angle_diff(float a1, float a2)
{
	// transform both angles to [0, 2pi]
	while (a1 < 0.0f) a1 += PI2f;
	while (a1 > PI2f) a1 -= PI2f;
	while (a2 < 0.0f) a2 += PI2f;
	while (a2 > PI2f) a2 -= PI2f;

	float diff = a1 - a2;
	if (diff > PIf) diff -= PI2f;
	if (diff < -PIf) diff += PI2f;
	return fabsf(diff);
}

//
// deletes really close parallel (roughly) roads, where one of the roads go nowhere.
//
int RoadMapEdit::FixExtraneousParallels()
{
	int removed = 0, i, j, roads;
	LinkEdit *pR1=NULL, *pR2=NULL;

	for (NodeEdit *pN = GetFirstNode(); pN && pN->GetNext(); pN = pN->GetNext())
	{
		roads = pN->NumLinks();

		if (roads < 3) continue;

		bool bad = false;
		pN->DetermineLinkAngles();
		for (i = 0; i < roads-1 && !bad; i++)
		{
			pR1 = pN->GetLink(i);
			for (j = i+1; j < roads; j++)
			{
				pR2 = pN->GetLink(j);
				if (pR2 == pR1) break;		// ignore loops
				float diff = angle_diff(pN->GetLinkAngle(i), pN->GetLinkAngle(j));
				if (diff < 0.3f)
				{
					// pR1 and pR2 are suspiciously close
					bad = true;
					break;
				}
			}
		}
		if (bad)
		{
			int leads_to[2];
			if (pR1->GetNode(0) == pN)
				leads_to[0] = pR1->GetNode(1)->NumLinks();
			else
				leads_to[0] = pR1->GetNode(0)->NumLinks();
			if (pR2->GetNode(0) == pN)
				leads_to[1] = pR2->GetNode(1)->NumLinks();
			else
				leads_to[1] = pR2->GetNode(0)->NumLinks();
			if (leads_to[0] == 1 && leads_to[1] > 1)
			{
				// delete R1
				DeleteSingleLink(pR1);
				removed++;
			}
			else if (leads_to[0] > 1 && leads_to[1] == 1)
			{
				// delete R2;
				DeleteSingleLink(pR2);
				removed++;
			}
			else
			{
				// dangerous case: both road look valid, yet meet at
				// a very close angle
				;	// a spot to put a breakpoint
			}
		}
	}
	return removed;
}

