//
// RoadMap.cpp
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "RoadMap.h"
#include "vtLog.h"
#include "FilePath.h"

#define intSize 4
#define floatSize 4
#define doubleSize 8

// Convenience for writing one thing to file.
#define FWrite(data,size) fwrite(data,size,1,fp)


//diff a - b.  result between PI and -PI.
float diffAngle(float a, float b)
{
	float result = a - b;
	while (result > PIf) {
		result -= PI2f;
	}
	while (result < -PIf) {
		result += PI2f;
	}
	return result;
}

//
// Nodes
//
TNode::TNode()
{
	m_pNext = NULL;
	m_id = -1;
}

TNode::~TNode()
{
}

bool TNode::operator==(TNode &ref)
{
	if (NumLinks() != ref.NumLinks())
		return false;
	if (m_id != ref.m_id)
		return false;
	return true;
}

void TNode::Copy(TNode *node)
{
	m_p = node->m_p;
	m_id = node->m_id;
	m_connect = node->m_connect;
	m_pNext = NULL;	//don't copy this
}

TLink *TNode::GetLink(uint n) const
{
	if (n >= m_connect.size())	// safety check
		return NULL;
	return m_connect[n];
}

int TNode::FindLink(int linkID)
{
	for (size_t i = 0; i < m_connect.size(); i++)
	{
		if (m_connect[i]->m_id == linkID)
			return i;
	}
	return -1;
}

int TNode::AddLink(TLink *pL)
{
	// fill in the entry for the new link
	m_connect.push_back(pL);
	return m_connect.size() - 1;
}

void TNode::DetachLink(TLink *pL)
{
	for (size_t i = 0; i < m_connect.size(); i++)
	{
		if (m_connect[i] == pL)
		{
			// found it
			m_connect.erase(m_connect.begin() + i);
			return;
		}
	}
}

//angles all > 0.
void TNode::DetermineLinkAngles()
{
	DPoint2 pn0, pn1, diff;

	m_fLinkAngle.resize(m_connect.size());

	for (size_t i = 0; i < m_connect.size(); i++)
	{
		pn0 = m_p;
		pn1 = GetAdjacentLinkPoint2d(i);
		diff = pn1 - pn0;

		float angle = atan2f((float)diff.y, (float)diff.x);
		if (angle < 0.0f)
			angle += PI2f;
		m_fLinkAngle[i] = angle;
	}
}

float TNode::GetLinkAngle(uint iLinkNum)
{
	return m_fLinkAngle[iLinkNum];
}

void TNode::SortLinksByAngle()
{
	// first determine the angle of each link
	DetermineLinkAngles();

	// sort links by radial angle (make them counter-clockwise)
	// use a bubble sort
	bool sorted = false;
	while (!sorted)
	{
		sorted = true;
		for (size_t i = 0; i < m_connect.size()-1; i++)
		{
			if (m_fLinkAngle[i] > m_fLinkAngle[i+1])
			{
				// swap entries in connection array
				TLink *tmp1 = m_connect[i];
				m_connect[i] = m_connect[i+1];
				m_connect[i+1] = tmp1;

				// And angle array
				float tmp2 = m_fLinkAngle[i];
				m_fLinkAngle[i] = m_fLinkAngle[i+1];
				m_fLinkAngle[i+1] = tmp2;

				sorted = false;
			}
		}
	}
}

DPoint2 TNode::GetAdjacentLinkPoint2d(int iLinkNum)
{
	TLink *link = m_connect[iLinkNum];
	if (link->GetNode(0) == this)
		return link->GetAt(1);			// link starts here
	else
		return link->GetAt(link->GetSize() - 2);	// link ends here
}

int TNode::GetLinkNum(TLink *link)
{
	for (size_t i = 0; i < m_connect.size(); i++)
	{
		if (m_connect[i] == link)
			return i;
	}
	return -1;
}

//traffic control
bool TNode::SetIntersectType(uint linkNum, IntersectionType type)
{
	if (linkNum >= m_connect.size())
		return false;

	m_connect[linkNum]->SetIntersectionType(this, type);
	return true;
}

IntersectionType TNode::GetIntersectType(uint linkNum)
{
	if (linkNum >= m_connect.size())
		return IT_NONE;

	return m_connect[linkNum]->GetIntersectionType(this);
}

bool TNode::HasLights()
{
	for (size_t i = 0; i < m_connect.size(); i++)
	{
		if (GetIntersectType(i) == IT_LIGHT)
			return true;
	}
	return false;
}

bool TNode::IsControlled()
{
	for (size_t i = 0; i < m_connect.size(); i++)
	{
		if (GetIntersectType(i) != IT_NONE)
			return true;
	}
	return false;
}

void TNode::AdjustForLights()
{
	if (!HasLights())
		return;

	for (size_t i = 0; i< m_connect.size(); i++) {
		SetIntersectType(i, IT_LIGHT);
	}
//if the intersection has signal lights, determine light relationships.
	switch (m_connect.size()) {
	case 0:
	case 1:
	case 2:
		//leave colors alone
		break;
	case 3:
	case 4:
		SortLinksByAngle();
		float bestAngle = PI2f;
		int bestChoiceA = 0;
		int bestChoiceB = 0;
		float newAngle;

		//go through all link pairs and see what is the difference of their angles.
		//we're shooting for difference of PI.
		for (size_t i = 0; i < m_connect.size() - 1; i++)
		{
			//since angles are sorted, angle i < angle j (not sure if that helps.)
			for (size_t j = i+1; j < m_connect.size(); j++)
			{
				if (i != j)
				{
					newAngle  = m_fLinkAngle[j] - (m_fLinkAngle[i] + PIf);
					//get absolute value
					if (newAngle < 0) {
						newAngle = -newAngle;
					}
					//get smallest angle
					if (newAngle > PIf) {
						newAngle = PI2f - newAngle;
					}
					//get absolute value
					if (newAngle < 0) {
						newAngle = -newAngle;
					}
					printf("%i:%f, %i:%f, %f, %f",
						(int) i, m_fLinkAngle[i],
						(int) j, m_fLinkAngle[j],
						newAngle, bestAngle);
					if (newAngle < bestAngle) {
						bestChoiceA = i;
						bestChoiceB = j;
						bestAngle = newAngle;
					}
				}
			}
		}
		break;
	}
}


//
// Links
//
TLink::TLink()
{
	// Provide default values
	m_fLeftWidth = 1.0f;
	m_fRightWidth = 1.0f;
	m_iLanes = 0;
	m_Surface = SURFT_PAVED;
	m_iHwy = -1;
	m_pNext = NULL;
	m_id = 0;
	m_iFlags = (RF_FORWARD|RF_REVERSE);	// by default, links are bidirectional
	m_fHeight[0] = 0;
	m_fHeight[1] = 0;
	m_pNode[0] = NULL;
	m_pNode[1] = NULL;
	m_fSidewalkWidth = SIDEWALK_WIDTH;
	m_fCurbHeight = CURB_HEIGHT;
	m_fMarginWidth = MARGIN_WIDTH;
	m_fLaneWidth = LANE_WIDTH;
	m_fParkingWidth = PARKING_WIDTH;
}

//
// Copy constructor
//
TLink::TLink(TLink &ref)
{
	*this = ref;
//	DLine2::DLine2(ref);	// need to do this?
}

TLink::~TLink()
{
}

bool TLink::operator==(TLink &ref)
{
	return (m_fLeftWidth == ref.m_fLeftWidth &&
		m_fRightWidth == ref.m_fRightWidth &&
		m_iLanes == ref.m_iLanes &&
		m_Surface == ref.m_Surface &&
		m_iHwy == ref.m_iHwy &&
		m_iFlags == ref.m_iFlags);
}

void TLink::CopyAttributesFrom(TLink *rhs)
{
	m_fLeftWidth =		rhs->m_fLeftWidth;
	m_fRightWidth =		rhs->m_fRightWidth;
	m_iLanes =			rhs->m_iLanes;
	m_Surface =			rhs->m_Surface;
	m_iHwy =			rhs->m_iHwy;
	m_iFlags =			rhs->m_iFlags;
	m_id =				rhs->m_id;
	m_fSidewalkWidth =	rhs->m_fSidewalkWidth;
	m_fCurbHeight =		rhs->m_fCurbHeight;
	m_fMarginWidth =	rhs->m_fMarginWidth;
	m_fLaneWidth =		rhs->m_fLaneWidth;
	m_fParkingWidth =	rhs->m_fParkingWidth;
	m_fHeight[0] =		rhs->m_fHeight[0];
	m_fHeight[1] =		rhs->m_fHeight[1];
}

void TLink::SetFlag(int flag, bool value)
{
	if (value)
		m_iFlags |= flag;
	else
		m_iFlags &= ~flag;
}

int TLink::GetFlag(int flag)
{
	return (m_iFlags & flag) != 0;
}

int TLink::GetSidewalk()
{
	int value = 0;
	if (m_iFlags & RF_SIDEWALK_LEFT) value |= 1;
	if (m_iFlags & RF_SIDEWALK_RIGHT) value |= 2;
	return value;
}

int TLink::GetParking()
{
	int value = 0;
	if (m_iFlags & RF_PARKING_LEFT) value |= 1;
	if (m_iFlags & RF_PARKING_RIGHT) value |= 2;
	return value;
}

/**
 * Find closest lateral distance from a given point to the link.
 */
double TLink::DistanceToPoint(const DPoint2 &point, bool bAllowEnds)
{
	double a, b;
	DPoint2 dummy1;
	int dummy2;
	float dummy3;
	GetLinearCoordinates(point, a, b, dummy1, dummy2, dummy3, bAllowEnds);
	return fabs(b);
}

/**
 * Produces the "linear coordinates" a and b, where a is the distance
 * along the link, and b is the signed lateral distance orthogonal to
 * the link at that point.
 *
 * \param p The input point.
 * \param result_a	The resulting distance along the link.
 * \param result_b  The signed lateral (or absolute) distance to the link.
 * \param closest	The closest point on the link.
 * \param linkpoint	Index into the links points just before the closest point.
 * \param fractional Fractional distance between this link point and the next.
 * \param bAllowEnds	If true, then for cases where the the closest
 *		point is either end of the link, the distance to that point
 *		is returned.  Otherwise, only laterial distances are returned.
 */
double TLink::GetLinearCoordinates(const DPoint2 &p, double &result_a,
								  double &result_b, DPoint2 &closest,
								  int &linkpoint, float &fractional,
								  bool bAllowEnds)
{
	double u, b, length, traversed = 0, min_dist = 1E10;
	DPoint2 p1, p2, diff, vec_a, vec_b, delta;

	int points = GetSize();
	for (int i=0; i < points-1; i++)
	{
		p1 = GetAt(i);
		p2 = GetAt(i+1);

		// check distance to line segment
		diff = p - p1;
		vec_a = p2 - p1;
		vec_b.Set(vec_a.y, -vec_a.x);

		length = vec_a.Length();
		vec_a /= length;
		vec_b.Normalize();
		u = diff.Dot(vec_a);

		// if u >1 or u <0, then point is not closest to line within the
		// given line segment.  use the end points.
		if ((bAllowEnds || (i > 0)) && u < 0)
//		if (u < 0)
		{
			u = 0;
			b = (p - p1).Length();
		}
		else if ((bAllowEnds || (i < points-2)) && u > length)
//		else if (u > length)
		{
			u = length;
			b = (p - p2).Length();
		}
		else
		{
			b = diff.Dot(vec_b);
		}

		if (fabs(b) < min_dist)
		{
			min_dist = fabs(b);
			result_a = traversed + u;
			result_b = b;
			closest = p1 + (vec_a * u);
			linkpoint = i;
			fractional = (float) (u/length);
		}
		traversed += length;
	}

	// 'traversed' now contains the total length of the link
	return traversed;
}

float TLink::Length()
{
	double dist = 0;
	DPoint2 tmp;
	for (uint i = 1; i < GetSize(); i++)
	{
		tmp = GetAt(i-1) - GetAt(i);
		dist += tmp.Length();
	}
	return (float) dist;
}

void TLink::EstimateWidth(bool bIncludeSidewalk)
{
	const float center_width = m_iLanes * m_fLaneWidth;

	m_fLeftWidth = m_fRightWidth = center_width / 2;

	if (GetFlag(RF_PARKING_LEFT))
		m_fLeftWidth += m_fParkingWidth;
	if (GetFlag(RF_PARKING_RIGHT))
		m_fRightWidth += m_fParkingWidth;

	if (GetFlag(RF_MARGIN))
	{
		m_fLeftWidth += (m_fMarginWidth * 2);
		m_fRightWidth += (m_fMarginWidth * 2);
	}

	if (bIncludeSidewalk)
	{
		if (GetFlag(RF_SIDEWALK_LEFT))
			m_fLeftWidth += m_fSidewalkWidth;
		if (GetFlag(RF_SIDEWALK_RIGHT))
			m_fRightWidth += m_fSidewalkWidth;
	}
}


//
// RoadMap class
//

vtRoadMap::vtRoadMap()
{
	// provide inital values for extent
	m_bValidExtents = false;

	m_pFirstLink = NULL;
	m_pFirstNode = NULL;
}


vtRoadMap::~vtRoadMap()
{
	DeleteElements();
}

void vtRoadMap::DeleteElements()
{
	TLink *nextR;
	while (m_pFirstLink)
	{
		nextR = m_pFirstLink->GetNext();
		delete m_pFirstLink;
		m_pFirstLink = nextR;
	}

	TNode *nextN;
	while (m_pFirstNode)
	{
		nextN = m_pFirstNode->GetNext();
		delete m_pFirstNode;
		m_pFirstNode = nextN;
	}
}

TNode *vtRoadMap::FindNodeByID(int id)
{
	for (TNode *pN = m_pFirstNode; pN; pN = pN->GetNext())
	{
		if (pN->m_id == id)
			return pN;
	}
	return NULL;
}

/**
 * Find the node closest to the indicated point.  Ignore nodes more than
 * epsilon units away from the point.
 */
TNode *vtRoadMap::FindNodeAtPoint(const DPoint2 &point, double epsilon)
{
	TNode *closest = NULL;
	double result, dist = 1E9;

	// a target rectangle, to quickly cull points too far away
	DRECT target(point.x-epsilon, point.y+epsilon, point.x+epsilon, point.y-epsilon);
	for (TNode *curNode = GetFirstNode(); curNode; curNode = curNode->GetNext())
	{
		if (!target.ContainsPoint(curNode->Pos()))
			continue;
		result = (curNode->Pos() - point).Length();
		if (result < dist)
		{
			closest = curNode;
			dist = result;
		}
	}
	return closest;
}

int	vtRoadMap::NumLinks() const
{
	int count = 0;
	for (TLink *pL = m_pFirstLink; pL; pL = pL->GetNext())
		count++;
	return count;
}

int	vtRoadMap::NumNodes() const
{
	int count = 0;
	for (TNode *pN = m_pFirstNode; pN; pN = pN->GetNext())
		count++;
	return count;
}

DRECT vtRoadMap::GetMapExtent()
{
	if (!m_bValidExtents)
		ComputeExtents();

	if (m_bValidExtents)
		return m_extents;
	else
		return DRECT(0,0,0,0);
}

void vtRoadMap::ComputeExtents()
{
	if (NumLinks() == 0)
	{
		m_bValidExtents = false;
		return;
	}

	// iterate through all elements accumulating extents
	m_extents.SetInsideOut();
	for (TLink *pL = m_pFirstLink; pL; pL = pL->GetNext())
	{
		// links are a subclass of line, so we can treat them as lines
		const DLine2 *dl = pL;
		m_extents.GrowToContainLine(*dl);
	}
	m_bValidExtents = true;
}

//
// remove unused nodes, return the number removed
//
int vtRoadMap::RemoveUnusedNodes()
{
	VTLOG("   vtRoadMap::RemoveUnusedNodes: ");

	TNode *prev = NULL, *next;
	TNode *pN = m_pFirstNode;
	int total = 0, unused = 0;

	while (pN)
	{
		total++;
		next = pN->GetNext();
		if (pN->NumLinks() == 0)
		{
			// delete it
			if (prev)
				prev->SetNext(next);
			else
				m_pFirstNode = next;
			delete pN;
			unused++;
		}
		else
			prev = pN;
		pN = next;
	}
	VTLOG("   %d of %d removed\n", unused, total);
	return unused;
}

/**
 * Remove a node, which also deletes it.
 */
void vtRoadMap::RemoveNode(TNode *pNode)
{
	TNode *prev = NULL, *next;
	TNode *pN = m_pFirstNode;

	while (pN)
	{
		next = pN->GetNext();
		if (pNode == pN)
		{
			// delete it
			if (prev)
				prev->SetNext(next);
			else
				m_pFirstNode = next;
			delete pN;
			break;
		}
		else
			prev = pN;
		pN = next;
	}
}

/**
 * Remove a link, which also deletes it.
 */
void vtRoadMap::RemoveLink(TLink *pLink)
{
	TLink *prev = NULL, *next;
	TLink *pL = m_pFirstLink;

	while (pL)
	{
		next = pL->GetNext();
		if (pLink == pL)
		{
			// delete it
			if (prev)
				prev->SetNext(next);
			else
				m_pFirstLink = next;
			delete pL;
			break;
		}
		else
			prev = pL;
		pL = next;
	}
}

/**
 * Read an RMF (Road Map File)
 * Returns true if operation sucessful.
 */
bool vtRoadMap::ReadRMF(const char *filename)
{
	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	char buffer[12];
	FILE *fp = vtFileOpen(filename, "rb");
	if (!fp)
	{
		VTLOG("Couldn't open RMF file: '%s'\n", filename);
		return false;
	}

	int numNodes, numLinks, i, j, dummy, quiet;
	TNode *tmpNode;
	TLink *tmpLink;

	// Is it a RMF File? and check version number
	if (fread(buffer,11,1,fp) != 1)
		return false;
	buffer[11] = 0;

	if (strncmp(buffer, RMFVERSION_STRING, 7))
	{
		// Not an RMF file!
		VTLOG1("That file does not appear to be a valid RMF file.\n");
		fclose(fp);
		return false;
	}
	double version = atof(buffer+7);
	VTLOG("Reading RMF file, version: %.1f.\n", version);

	if (version < RMFVERSION_SUPPORTED)
	{
		// not recent version
		VTLOG("Sorry, we can only read version %.1f or newer.\n",
			version, RMFVERSION_SUPPORTED);
		fclose(fp);
		return false;
	}

	// Erasing existing network
	DeleteElements();

	// Projection
	if (version < 1.9)
	{
		int proj_type=1, iUTMZone;
		int iDatum = EPSG_DATUM_WGS84;
		if (version >= 1.8f)
		{
			int iUTM;
			quiet = fread(&iUTM, intSize, 1, fp);
			proj_type = (iUTM != 0);
		}
		quiet = fread(&iUTMZone, intSize, 1, fp);
		if (version >= 1.8f)
		{
			quiet = fread(&iDatum, intSize, 1, fp);
		}
		m_crs.SetSimple(proj_type == 1, iUTMZone, iDatum);
	}
	else
	{
		quiet = fread(&dummy, 4, 1, fp);
		short length = (short) dummy;
		char wkt_buf[2000], *wkt = wkt_buf;
		quiet = fread(wkt_buf, length, 1, fp);
		OGRErr err = m_crs.importFromWkt((char **) &wkt);
		if (err != OGRERR_NONE)
			return false;
	}

	// Extents
	if (version < 1.9)
	{
		int le, ri, to, bo;
		quiet = fread(&ri, intSize, 1, fp);
		quiet = fread(&to, intSize, 1, fp);
		quiet = fread(&le, intSize, 1, fp);
		quiet = fread(&bo, intSize, 1, fp);
		m_extents.left = le;
		m_extents.right = ri;
		m_extents.top = to;
		m_extents.bottom = bo;
	}
	else
	{
		quiet = fread(&m_extents.left, doubleSize, 1, fp);
		quiet = fread(&m_extents.right, doubleSize, 1, fp);
		quiet = fread(&m_extents.bottom, doubleSize, 1, fp);
		quiet = fread(&m_extents.top, doubleSize, 1, fp);
	}
	m_bValidExtents = true;

	//get number of nodes and links
	quiet = fread(&numNodes, intSize, 1, fp);
	quiet = fread(&numLinks, intSize, 1, fp);

	quiet = fread(buffer,7,1,fp);
	if (strcmp(buffer, "Nodes:"))
	{
		fclose(fp);
		return false;
	}

	// Use a temporary array for fast lookup
	TNodePtr *pNodeLookup = new TNodePtr[numNodes+1];

	// Read the nodes
	int ivalue;
	for (i = 1; i <= numNodes; i++)
	{
		tmpNode = AddNewNode();
		quiet = fread(&(tmpNode->m_id), intSize, 1, fp);
		if (version < 1.8f)
		{
			int x, y;
			quiet = fread(&x, intSize, 1, fp);
			quiet = fread(&y, intSize, 1, fp);
			tmpNode->SetPos(DPoint2(x, y));
		}
		else
		{
			quiet = fread(&tmpNode->Pos().x, doubleSize, 1, fp);
			quiet = fread(&tmpNode->Pos().y, doubleSize, 1, fp);
		}
		// add to quick lookup table
		pNodeLookup[i] = tmpNode;
	}

	quiet = fread(buffer,7,1,fp);
	if (strcmp(buffer, "Roads:"))
	{
		fclose(fp);
		return false;
	}

	// Read the links
	float ftmp;
	int itmp;
	int node_numbers[2];
	for (i = 1; i <= numLinks; i++)
	{
		tmpLink = AddNewLink();
		quiet = fread(&(tmpLink->m_id), intSize, 1, fp);	//id
		if (version < 1.89)
		{
			quiet = fread(&itmp, intSize, 1, fp);			//highway number
			tmpLink->m_iHwy = (short) itmp;
			quiet = fread(&ftmp, floatSize, 1, fp);	//width
			quiet = fread(&itmp, intSize, 1, fp);			//number of lanes
			tmpLink->m_iLanes = (unsigned short) itmp;
			quiet = fread(&itmp, intSize, 1, fp);			//surface type
			tmpLink->m_Surface =  (SurfaceType) itmp;
			quiet = fread(&itmp, intSize, 1, fp);			//FLAG
			tmpLink->m_iFlags = (short) (itmp >> 16);
		}
		else
		{
			quiet = fread(&(dummy), 4, 1, fp);			//highway number
			tmpLink->m_iHwy = (short) dummy;
			quiet = fread(&ftmp, floatSize, 1, fp);	//width
			quiet = fread(&(dummy), 4, 1, fp);			//number of lanes
			tmpLink->m_iLanes = (short) dummy;
			quiet = fread(&dummy, 4, 1, fp);			//surface type
			tmpLink->m_Surface =  (SurfaceType) dummy;
			quiet = fread(&(dummy), 4, 1, fp);			//FLAG
			tmpLink->m_iFlags = dummy;
		}

		if (version < 1.89)
		{
			quiet = fread(&ftmp, floatSize, 1, fp);		//height of link at node 0
			quiet = fread(&ftmp, floatSize, 1, fp);		//height of link at node 1
		}

		if (version >= 2.0)
		{
			quiet = fread(&(tmpLink->m_fSidewalkWidth), floatSize, 1, fp);	// sidewalk width
			quiet = fread(&(tmpLink->m_fCurbHeight), floatSize, 1, fp);		// curb height
			quiet = fread(&(tmpLink->m_fMarginWidth), floatSize, 1, fp);	// margin width
			quiet = fread(&(tmpLink->m_fLaneWidth), floatSize, 1, fp);		// lane width
			quiet = fread(&(tmpLink->m_fParkingWidth), floatSize, 1, fp);	// parking width
		}
		int size;
		quiet = fread(&size, intSize, 1, fp);	// number of coordinates making the link
		tmpLink->SetSize(size);

		for (j = 0; j < size; j++)
		{
			if (version < 1.8f)
			{
				quiet = fread(&ivalue, intSize, 1, fp);
				(*tmpLink)[j].x = ivalue;
				quiet = fread(&ivalue, intSize, 1, fp);
				(*tmpLink)[j].y = ivalue;
			}
			else
			{
				quiet = fread(&((*tmpLink)[j].x), doubleSize, 1, fp);
				quiet = fread(&((*tmpLink)[j].y), doubleSize, 1, fp);
			}
		}

		// The start/end nodes
		quiet = fread(node_numbers, intSize, 2, fp);
		if (node_numbers[0] < 1 || node_numbers[0] > numNodes ||
			node_numbers[1] < 1 || node_numbers[1] > numNodes)
			return false;
		tmpLink->ConnectNodes(pNodeLookup[node_numbers[0]],
							  pNodeLookup[node_numbers[1]]);
	}

	// Read traffic control information
	quiet = fread(buffer,9, 1, fp);
	if (strcmp(buffer, "Traffic:"))
	{
		fclose(fp);
		return false;
	}

	for (i = 0; i < numNodes; i++)
	{
		int id, numLinks;

		quiet = fread(&id, intSize, 1, fp);  //node ID
		// safety check
		if (id < 1 || id > numNodes)
		{
			fclose(fp);
			return false;
		}

		tmpNode = pNodeLookup[id];
		quiet = fread(&dummy, intSize, 1, fp);
		quiet = fread(&numLinks, intSize, 1, fp);

		//get specifics for each link at the intersection:
		for (j = 0; j < numLinks; j++)
		{
			//match link number
			IntersectionType type;
			int lStatus;
			//read in data
			quiet = fread(&id, intSize, 1, fp);  //link ID
			quiet = fread(&type, intSize, 1, fp);
			quiet = fread(&lStatus, intSize, 1, fp);
			//now figure out which links at the node get what behavior
			id = tmpNode->FindLink(id);
			if (id >= 0)
			{
				tmpNode->SetIntersectType(id, type);
			}
		}
	}

	// We don't need the lookup table any more
	delete [] pNodeLookup;

	//are we at end of file?
	quiet = fread(buffer,8, 1, fp);
	fclose(fp);
	if (strcmp(buffer, "End RMF"))
		return false;

	return true;
}

//
// returns true if operation sucessful.
//
bool vtRoadMap::WriteRMF(const char *filename)
{
	int i;

	TNode *curNode = GetFirstNode();
	TLink *curLink = GetFirstLink();
	int numNodes = NumNodes();
	int numLinks = NumLinks();

	// must have nodes, or saving will fail
	if (numNodes == 0)
		return false;

	FILE *fp = vtFileOpen(filename, "wb");
	if (!fp)
	{
		// wxString str = wxString::Format("Error opening file: %s", filename);
		return false;
	}

	// go through and set id numbers (1-based) for the nodes and links
	i= 1;
	while (curNode) {
		curNode->m_id = i++;
		curNode = curNode->GetNext();
	}
	i=1;
	while (curLink) {
		curLink->m_id = i++;
		curLink = curLink->GetNext();
	}

	curNode = GetFirstNode();
	curLink = GetFirstLink();

	FWrite(RMFVERSION_STRING, 11);

	// Projection
	char *wkt;
	OGRErr err = m_crs.exportToWkt(&wkt);
	if (err != OGRERR_NONE)
		return false;
	short len = (short) strlen(wkt);
	FWrite(&len, 4);
	FWrite(wkt, len);
	OGRFree(wkt);

	// Extents
	FWrite(&m_extents.left, doubleSize);
	FWrite(&m_extents.right, doubleSize);
	FWrite(&m_extents.bottom, doubleSize);
	FWrite(&m_extents.top, doubleSize);
	FWrite(&numNodes, intSize);  // number of nodes
	FWrite(&numLinks, intSize);  // number of links
	FWrite("Nodes:",7);
	//write nodes
	while (curNode)
	{
		FWrite(&(curNode->m_id), intSize);		// id
		FWrite(&curNode->Pos().x, doubleSize);	// coordinate
		FWrite(&curNode->Pos().y, doubleSize);
		curNode = curNode->GetNext();
	}
	FWrite("Roads:",7);
	//write links
	while (curLink)
	{
		float fWidth = curLink->GetTotalWidth();
		FWrite(&(curLink->m_id), intSize);			//id
		FWrite(&(curLink->m_iHwy), 4);				//highway number
		FWrite(&(fWidth), floatSize);				//width
		FWrite(&(curLink->m_iLanes), 4);			//number of lanes
		FWrite(&(curLink->m_Surface), 4);			//surface type
		FWrite(&(curLink->m_iFlags), 4);			//FLAG
		FWrite(&(curLink->m_fSidewalkWidth), floatSize);	// sidewalk width
		FWrite(&(curLink->m_fCurbHeight), floatSize);	// curb height
		FWrite(&(curLink->m_fMarginWidth), floatSize);	// margin width
		FWrite(&(curLink->m_fLaneWidth), floatSize);	// lane width width
		FWrite(&(curLink->m_fParkingWidth), floatSize);	// parking width

		int size = curLink->GetSize();
		FWrite(&size, intSize);//number of coordinates making the link
		for (i = 0; i < size; i++)
		{
			//coordinates that make the link
			FWrite(&curLink->GetAt(i).x, doubleSize);
			FWrite(&curLink->GetAt(i).y, doubleSize);
		}
		//nodes (endpoints)
		FWrite(&(curLink->GetNode(0)->m_id), intSize);	//what link is at the end point?
		FWrite(&(curLink->GetNode(1)->m_id), intSize);

		curLink = curLink->GetNext();
	}

	int dummy = 0;

	//write traffic control information
	FWrite("Traffic:",9);
	curNode = GetFirstNode();
	while (curNode)
	{
		// safety check
		if (curNode->m_id < 1 || curNode->m_id > numNodes)
			return false;

		int num_links = curNode->NumLinks();
		FWrite(&(curNode->m_id), intSize);	//node ID
		FWrite(&(dummy), intSize); //node traffic behavior
		FWrite(&(num_links), intSize); //node traffic behavior
		for (i = 0; i < num_links; i++) {
			IntersectionType type = curNode->GetIntersectType(i);
			int lStatus = 0;
			FWrite(&(curNode->GetLink(i)->m_id), intSize);  //link ID
			FWrite(&type, intSize);  //get the intersection type associated with that link
			FWrite(&lStatus,intSize);
		}
		curNode = curNode->GetNext();
	}

	//EOF
	FWrite("End RMF", 8);
	fclose(fp);

	return true;
}

