//
// Building.cpp
//
// Implements the vtBuilding class which represents a single built structure.
// This is can be a single building, or any single artificial structure
// such as a wall or fence.
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "Building.h"
#include "vtLog.h"
#include "HeightField.h"
#include "LocalCS.h"
#include "MaterialDescriptor.h"

// Defaults
#define STORY_HEIGHT	3.0f
#define WINDOW_WIDTH	1.3f
#define WINDOW_BOTTOM	0.4f
#define WINDOW_TOP		0.9f
#define DOOR_WIDTH		1.0f
#define DOOR_TOP		0.7f

/////////////////////////////////////

vtEdgeFeature::vtEdgeFeature()
{
	SetDefaults();
}

vtEdgeFeature::vtEdgeFeature(int code, float width, float vf1, float vf2)
{
	m_code = code;
	m_width = width;
	m_vf1 = vf1;
	m_vf2 = vf2;
}

void vtEdgeFeature::SetDefaults()
{
	m_code = WFC_WALL;
	m_width = -1.0f;
	m_vf1 = 0.0f;
	m_vf2 = 1.0f;
}


/////////////////////////////////////

vtEdge::vtEdge()
{
	m_Color.Set(255,0,0);		// default color: red
	m_iSlope = 90;		// vertical
	m_pMaterial = GetGlobalMaterials()->FindName(BMAT_NAME_PLAIN);
}

vtEdge::vtEdge(const vtEdge &lhs)
{
	m_Color = lhs.m_Color;
	m_iSlope = lhs.m_iSlope;
	for (uint i = 0; i < lhs.m_Features.size(); i++)
		m_Features.push_back(lhs.m_Features[i]);
	m_pMaterial = lhs.m_pMaterial;
	m_Facade = lhs.m_Facade;
}

void vtEdge::Set(int iDoors, int iWindows, const char *material)
{
	vtEdgeFeature wall, window, door;

	window.m_code = WFC_WINDOW;
	window.m_width = WINDOW_WIDTH;
	window.m_vf1 = WINDOW_BOTTOM;
	window.m_vf2 = WINDOW_TOP;

	door.m_code = WFC_DOOR;
	door.m_width = DOOR_WIDTH;
	door.m_vf1 = 0.0f;
	door.m_vf2 = DOOR_TOP;

	m_Features.clear();
	m_Features.reserve((iDoors + iWindows) * 2 + 1);
	m_Features.push_back(wall);

	bool do_door, do_window, flip = false;
	while (iDoors || iWindows)
	{
		do_door = do_window = false;
		if (iDoors && iWindows)
		{
			if (flip)
				do_door = true;
			else
				do_window = true;
		}
		else if (iDoors)
			do_door = true;
		else if (iWindows)
			do_window = true;

		if (do_door)
		{
			m_Features.push_back(door);
			iDoors--;
		}
		if (do_window)
		{
			m_Features.push_back(window);
			iWindows--;
		}
		m_Features.push_back(wall);
	}
	m_pMaterial = GetGlobalMaterials()->FindName(material);
}

void vtEdge::AddFeature(int code, float width, float vf1, float vf2)
{
	m_Features.push_back(vtEdgeFeature(code, width, vf1, vf2));
}

int vtEdge::NumFeaturesOfCode(int code) const
{
	int i, count = 0, size = m_Features.size();
	for (i = 0; i < size; i++)
	{
		if (m_Features[i].m_code == code)
			count++;
	}
	return count;
}

float vtEdge::FixedFeaturesWidth() const
{
	float width = 0.0f;
	for (uint i = 0; i < m_Features.size(); i++)
	{
		const float fwidth = m_Features[i].m_width;
		if (fwidth > 0)
			width += fwidth;
	}
	return width;
}

float vtEdge::ProportionTotal() const
{
	float width = 0.0f;
	for (uint i = 0; i < m_Features.size(); i++)
	{
		const float fwidth = m_Features[i].m_width;
		if (fwidth < 0)
			width += fwidth;
	}
	return width;
}

bool vtEdge::IsUniform() const
{
	const int windows = NumFeaturesOfCode(WFC_WINDOW);
	const int doors = NumFeaturesOfCode(WFC_DOOR);
	const int walls = NumFeaturesOfCode(WFC_WALL);
	if (doors > 0)
		return false;
	if (walls != (windows + 1))
		return false;
	if (m_iSlope != 90)
		return false;
	if (m_pMaterial != NULL && *m_pMaterial != BMAT_NAME_SIDING)
		return false;
	return true;
}

/////////////////////////////////////

vtLevel::vtLevel()
{
	m_iStories = 1;
	m_fStoryHeight = STORY_HEIGHT;
}

vtLevel::~vtLevel()
{
	m_Foot.clear();
	DeleteEdges();
}

void vtLevel::DeleteEdges()
{
	for (uint i = 0; i < m_Edges.GetSize(); i++)
		delete m_Edges[i];
	m_Edges.SetSize(0);
}

vtLevel &vtLevel::operator=(const vtLevel &v)
{
	m_iStories = v.m_iStories;
	m_fStoryHeight = v.m_fStoryHeight;

	DeleteEdges();
	m_Edges.SetSize(v.m_Edges.GetSize());
	for (uint i = 0; i < v.m_Edges.GetSize(); i++)
	{
		vtEdge *pnew = new vtEdge(*v.m_Edges[i]);
		m_Edges.SetAt(i, pnew);
	}
	m_Foot = v.m_Foot;
	m_LocalFootprint = v.m_LocalFootprint;
	return *this;
}

void vtLevel::DeleteEdge(int iEdge)
{
	delete m_Edges[iEdge];
	m_Edges.RemoveAt(iEdge);
	m_Foot.RemovePoint(iEdge);
}

// Split an edge at the indicated point and clone into two edges
bool vtLevel::AddEdge(const int iEdge, const DPoint2 &Point)
{
	int iNumEdges = m_Edges.GetSize();
	int iIndex;

	vtEdge *pEdge = new vtEdge(*GetEdge(iEdge));

	if (NULL == pEdge)
		return false;

	if (iEdge == iNumEdges - 1)
		m_Edges.Append(pEdge);
	else
	{
		for (iIndex = iNumEdges - 1; iIndex > iEdge ; iIndex--)
			m_Edges.SetAt(iIndex + 1, m_Edges[iIndex]);
		m_Edges.SetAt(iEdge + 1, pEdge);
	}
	m_Foot.InsertPointAfter(iEdge, Point);
	return true;
}

void vtLevel::SetFootprint(const DLine2 &dl)
{
	// Safety check: Make sure there is at least an outer polygon
	if (m_Foot.size() == 0)
		m_Foot.resize(1);

	int prev = m_Foot[0].GetSize();
	m_Foot[0] = dl;
	int curr = dl.GetSize();
	if (curr != prev)
		RebuildEdges(curr);
}

void vtLevel::SetFootprint(const DPolygon2 &poly)
{
	int prev = m_Foot.NumTotalVertices();
	m_Foot = poly;
	int curr = m_Foot.NumTotalVertices();
	if (curr != prev)
		RebuildEdges(curr);
}

void vtLevel::SetEdgeMaterial(const char *matname)
{
	const vtString *str = GetGlobalMaterials()->FindName(matname);
	for (uint i = 0; i < m_Edges.GetSize(); i++)
		m_Edges[i]->m_pMaterial = str;
}

void vtLevel::SetEdgeMaterial(const vtString *matname)
{
	for (uint i = 0; i < m_Edges.GetSize(); i++)
		m_Edges[i]->m_pMaterial = matname;
}

void vtLevel::SetEdgeColor(RGBi color)
{
	for (uint i = 0; i < m_Edges.GetSize(); i++)
		m_Edges[i]->m_Color = color;
}

vtEdge *vtLevel::GetEdge(uint i) const
{
	if (i < m_Edges.GetSize())		// safety check
		return m_Edges[i];
	else
		return NULL;
}

float vtLevel::GetEdgeLength(uint iIndex) const
{
	int i = iIndex;
	const int ring = m_Foot.WhichRing(i);
	if (ring == -1)
		return 0.0f;
	const DLine2 &dline = m_Foot[ring];
	const int j = i+1 == dline.GetSize() ? 0 : i+1;
	return (float) (dline[j] - dline[i]).Length();
}

float vtLevel::GetLocalEdgeLength(uint iIndex) const
{
	int i = iIndex;
	const int ring = m_LocalFootprint.WhichRing(i);
	if (ring == -1)
		return 0.0f;
	const FLine3 &fline = m_LocalFootprint[ring];
	const int j = i+1 == fline.GetSize() ? 0 : i+1;
	return (float) (fline[j] - fline[i]).Length();
}

void vtLevel::RebuildEdges(uint n)
{
	DeleteEdges();
	for (uint i = 0; i < n; i++)
	{
		vtEdge *pnew = new vtEdge;
		pnew->Set(0, 0, BMAT_NAME_SIDING);
		m_Edges.Append(pnew);
	}
}

void vtLevel::ResizeEdgesToMatchFootprint()
{
	int curr = m_Foot.NumTotalVertices();
	m_Edges.SetSize(curr);
}

bool vtLevel::HasSlopedEdges() const
{
	for (uint i = 0; i < m_Edges.GetSize(); i++)
	{
		if (m_Edges[i]->m_iSlope != 90)
			return true;
	}
	return false;
}

bool vtLevel::IsHorizontal() const
{
	for (uint i = 0; i < m_Edges.GetSize(); i++)
	{
		if (m_Edges[i]->m_iSlope == 0)
			return true;
	}
	return false;
}

void vtLevel::SetRoofType(RoofType rt, int iSlope)
{
	int i, edges = NumEdges();

	if (rt == ROOF_FLAT)
	{
		// all edges are horizontal
		for (i = 0; i < edges; i++)
			m_Edges[i]->m_iSlope = 0;
		m_fStoryHeight = 0.0f;
	}
	if (rt == ROOF_SHED)
	{
		// all edges are vertical
		for (i = 0; i < edges; i++)
			m_Edges[i]->m_iSlope = 90;
		// except for the first edge
		m_Edges[0]->m_iSlope = iSlope;

		DetermineHeightFromSlopes();
	}
	if (rt == ROOF_GABLE)
	{
		// Algorithm for guessing which edges makes up the gable roof:
		if (NumEdges() == 4)
		{
			// In the case of a rectangular footprint, assume that the
			// shorter edge has the gable
			if (GetEdgeLength(1) > GetEdgeLength(0))
			{
				m_Edges[0]->m_iSlope = 90;
				m_Edges[1]->m_iSlope = iSlope;
				m_Edges[2]->m_iSlope = 90;
				m_Edges[3]->m_iSlope = iSlope;
			}
			else
			{
				m_Edges[0]->m_iSlope = iSlope;
				m_Edges[1]->m_iSlope = 90;
				m_Edges[2]->m_iSlope = iSlope;
				m_Edges[3]->m_iSlope = 90;
			}
		}
		else
		{
			// Assume that only convex edges can be gables, and no more than
			// one edge in a row is a gable.  All other edges are hip.
			bool last_gable = false;
			for (i = 0; i < edges; i++)
			{
				if (IsEdgeConvex(i) && !last_gable &&
					!((i == edges - 1) && (m_Edges[0]->m_iSlope == 90)))
				{
					m_Edges[i]->m_iSlope = 90;
					last_gable = true;
				}
				else
				{
					m_Edges[i]->m_iSlope = iSlope;
					last_gable = false;
				}
			}
		}
		DetermineHeightFromSlopes();
	}
	if (rt == ROOF_HIP)
	{
		for (i = 0; i < edges; i++)
			m_Edges[i]->m_iSlope = iSlope;

		DetermineHeightFromSlopes();
	}
}

void vtLevel::SetEaveLength(float fMeters)
{
	int i, edges = NumEdges();

	for (i = 0; i < edges; i++)
	{
		vtEdge *edge = m_Edges[i];
		float rise = m_fStoryHeight;
		// sin(slope) = rise/length
		// length = rise/sin(slope)
		float length = rise / sinf(edge->m_iSlope / 180.0f * PIf);
		edge->m_Features[0].m_vf1 = -(fMeters / length);
	}
}

bool vtLevel::IsEdgeConvex(int i)
{
	// get the 2 corner indices of this edge
	int edges = NumEdges();
	int c1 = i;
	int c2 = (i+1 == edges) ? 0 : i+1;

	return (IsCornerConvex(c1) && IsCornerConvex(c2));
}

bool vtLevel::IsCornerConvex(int i)
{
	int ring = m_Foot.WhichRing(i);
	if (ring == -1)
		return false;
	const DLine2 &dline = m_Foot[ring];

	// get the 2 adjacent corner indices
	int edges = dline.GetSize();
	int c1 = (i-1 < 0) ? edges-1 : i-1;
	int c2 = i;
	int c3 = (i+1 == edges) ? 0 : i+1;

	// get edge vectors
	DPoint2 v1 = dline[c2] - dline[c1];
	DPoint2 v2 = dline[c3] - dline[c2];

	// if dot product is positive, it's convex
	double xprod = v1.Cross(v2);
	return (xprod > 0);
}

/**
 * Returns true if this level consists of edges with identical,
 * evenly spaced windows.
 */
bool vtLevel::IsUniform() const
{
	for (uint i = 0; i < m_Edges.GetSize(); i++)
	{
		if (m_Edges[i]->IsUniform() == false)
			return false;
	}
	return true;
}

void vtLevel::DetermineLocalFootprint(float fHeight, const LocalCS &local_cs)
{
	const uint rings = m_Foot.size();
	FPoint3 lp;

	m_LocalFootprint.resize(rings);
	for (unsigned ring = 0; ring < rings; ring++)
	{
		const DLine2 &dline2 = m_Foot[ring];
		FLine3 &fline3 = m_LocalFootprint[ring];

		const uint edges = dline2.GetSize();
		fline3.SetSize(edges);
		for (unsigned i = 0; i < edges; i++)
		{
			const DPoint2 &p = dline2[i];
			local_cs.EarthToLocal(p, lp.x, lp.z);
			lp.y = fHeight;
			fline3.SetAt(i, lp);
		}
	}
}

void vtLevel::GetEdgePlane(uint i, FPlane &plane)
{
	vtEdge *edge = m_Edges[i];
	int islope = edge->m_iSlope;
	float slope = (islope / 180.0f * PIf);

	int index = i;
	int ring = m_LocalFootprint.WhichRing(index);
	FLine3 &loop = m_LocalFootprint[ring];
	uint ring_edges = loop.GetSize();
	int next = (index+1 == ring_edges) ? 0 : index+1;

	// get edge vector
	FPoint3 vec = loop[next] - loop[index];
	vec.Normalize();

	// get perpendicular (upward pointing) vector
	FPoint3 perp;
	perp.Set(0, 1, 0);

	// create rotation matrix to rotate it upward
	FMatrix4 mat;
	mat.Identity();
	mat.AxisAngle(vec, slope);

	// create normal
	FPoint3 norm;
	mat.TransformVector(perp, norm);

	plane.Set(loop[index], norm);
}

//
// Look at the sloped edges to see if they meet at a particular point;
// if so, set that as the Level's story height.  Return true on success.
//
bool vtLevel::DetermineHeightFromSlopes()
{
	// In order to find a roof point, we need 3 adjacent edges whose
	// edges intersect.
	int i, edges = NumEdges();

	bool bFoundASolution = false;
	FPlane *planes = new FPlane[edges];
	float fMinHeight = 1E10;
	for (i = 0; i < edges; i++)
	{
		GetEdgePlane(i, planes[i]);
	}
	for (i = 0; i < edges; i++)
	{
		int i0 = i;
		int i1 = (i+1)%edges;
		int i2 = (i+2)%edges;
		vtEdge *edge0 = m_Edges[i0];
		vtEdge *edge1 = m_Edges[i1];
		vtEdge *edge2 = m_Edges[i2];
		if (edge0->m_iSlope == 90 &&
			edge1->m_iSlope == 90 &&
			edge2->m_iSlope == 90)
		{
			// skip this one; 3 vertical edges aren't useful
			continue;
		}
		FPoint3 point;
		bool valid = PlaneIntersection(planes[i0], planes[i1], planes[i2], point);
		if (valid)
		{
			// take this point as the height of the roof
			float fHeight = (point.y - m_LocalFootprint[0][0].y);

			if (fHeight < 0)	// shouldn't happen, but just a safety check
				continue;

			if (fHeight < fMinHeight)
				fMinHeight = fHeight;
			bFoundASolution = true;
		}
	}
	if (bFoundASolution)
		m_fStoryHeight = fMinHeight;
	delete [] planes;
	return bFoundASolution;
}

//
// Look at the materials of this level's edges.  If they all use the
// same material, return it.  Otherwise, return "Multiple".
//
const vtString vtLevel::GetOverallEdgeMaterial()
{
	const vtString *most = NULL;

	int edges = NumEdges();
	for (int i = 0; i < edges; i++)
	{
		vtEdge *pEdge = GetEdge(i);
		const vtString *mat = pEdge->m_pMaterial;
		if (mat == NULL)
			continue;
		if (most == NULL)
			most = mat;
		else if (*most != *mat)
			return "Multiple";
	}
	if (most)
		return *most;
	else
		return "None";
}

//
// Look at the materials of this level's edges.  If they all use the
// same material, return it.  Otherwise, return BMAT_UNKNOWN.
//
bool vtLevel::GetOverallEdgeColor(RGBi &color)
{
	RGBi col1(-1, -1, -1);

	int edges = NumEdges();
	for (int i = 0; i < edges; i++)
	{
		vtEdge *pEdge = GetEdge(i);
		RGBi col2 = pEdge->m_Color;
		if (col1.r == -1)
			col1 = col2;
		else if (col1 != col2)
			return false;
	}
	color = col1;
	return true;
}

//
// try to guess type of roof from looking at slopes of edges of
// this level
//
RoofType vtLevel::GuessRoofType() const
{
	int sloped = 0, vert = 0, hori = 0;
	int i, edges = NumEdges();
	for (i = 0; i < edges; i++)
	{
		vtEdge *edge = GetEdge(i);
		if (edge->m_iSlope == 0)
			hori++;
		else if (edge->m_iSlope == 90)
			vert++;
		else
			sloped++;
	}
	if (hori)
		return ROOF_FLAT;

	if (sloped == 1 && vert == edges-1)
		return ROOF_SHED;

	if (sloped == edges)
		return ROOF_HIP;

	if (sloped > 0 && vert > 0)
		return ROOF_GABLE;

	return ROOF_UNKNOWN;
}

void vtLevel::FlipFootprintDirection()
{
	m_Foot.ReverseOrder();
}


/////////////////////////////////////////////////////////////////////////////
// vtBuilding class implementation

vtBuilding::vtBuilding() : vtStructure()
{
	SetType(ST_BUILDING);
	m_pCRS = NULL;
}

vtBuilding::~vtBuilding()
{
	DeleteLevels();
}

/**
 * Delete all the levels for this building.
 */
void vtBuilding::DeleteLevels()
{
	for (uint i = 0; i < m_Levels.GetSize(); i++)
		delete m_Levels[i];
	m_Levels.SetSize(0);
}

/**
 * Asignment operator, which makes an explicit copy the entire building
 * including each level.
 */
vtBuilding &vtBuilding::operator=(const vtBuilding &v)
{
	// copy parent data
	vtStructure::CopyFrom(v);

	// copy class data
	DeleteLevels();
	for (uint i = 0; i < v.m_Levels.GetSize(); i++)
		m_Levels.Append(new vtLevel(* v.m_Levels[i]));

	m_pCRS = v.m_pCRS;

	return *this;
}

/**
 * Flips the direction of the footprint, which is either clockwise or
 * counterclockwise when viewed from above.  This affects the footprints
 * of all levels.
 */
void vtBuilding::FlipFootprintDirection()
{
	// Flip the direction (clockwisdom) of each level
	for (uint i = 0; i < m_Levels.GetSize(); i++)
		m_Levels[i]->FlipFootprintDirection();

	// keep 2d and 3d in synch
	DetermineLocalFootprints();
}

/**
 * Calculate the elevation at which this building should be placed
 * on a given heightfield.
 */
float vtBuilding::CalculateBaseElevation(vtHeightField *pHeightField)
{
	const DLine2 &Footprint = m_Levels[0]->GetOuterFootprint();
	int iSize = Footprint.GetSize();
	float fLowest = 1E9f;

	float fAltitude;
	for (int i = 0; i < iSize; i++)
	{
		pHeightField->FindAltitudeOnEarth(Footprint[i], fAltitude);
		if (fAltitude < fLowest)
			fLowest = fAltitude;
	}
	return fLowest + m_fElevationOffset;
}

/**
 * Transform the coodinates of this building (the footprints of
 * each level) by the given coordinate transformation.
 */
void vtBuilding::TransformCoords(OCTransform *trans)
{
	uint i, j;
	DPoint2 p;

	for (i = 0; i < m_Levels.GetSize(); i++)
	{
		vtLevel *pLev = m_Levels[i];

		DPolygon2 foot = pLev->GetFootprint();
		for (uint r = 0; r < foot.size(); r++)
		{
			DLine2 &footline = foot[r];
			uint iSize = footline.GetSize();
			for (j = 0; j < iSize; j++)
			{
				p = footline[j];
				trans->Transform(1, &p.x, &p.y);
				footline[j] = p;
			}
		}
		pLev->SetFootprint(foot);
	}
}

/**
 * Sets the base footprint of the building to be a circle.  A circle
 * is represented by a 20-sided polygonal footprint.
 *
 * \param center The location of the building's center.
 * \param fRad The radius of the building.
 */
void vtBuilding::SetCircle(const DPoint2 &center, float fRad)
{
	DLine2 fp;
	int i;
	for (i = 0; i < 20; i++)
	{
		double angle = i * PI2d / 20;
		DPoint2 vec(cos(angle) * fRad, sin(angle) * fRad);
		fp.Append(center + vec);
	}
	SetFootprint(0, fp);
}

/**
 * Set the colors of the building.
 *
 * \param which Can be either BLD_BASIC (the overall color of the building)
 *			or BLD_ROOF (the overall color of the roof).
 * \param color The color to set.
 */
void vtBuilding::SetColor(BldColor which, const RGBi &color)
{
	int i, levs = m_Levels.GetSize();
	for (i = 0; i < levs; i++)
	{
		vtLevel *pLev = m_Levels[i];
		int j, edges = pLev->NumEdges();
		for (j = 0; j < edges; j++)
		{
			vtEdge *edge = pLev->GetEdge(j);
			if (edge->m_iSlope < 90)
			{
				if (which == BLD_ROOF)
					edge->m_Color = color;
			}
			else
			{
				if (which == BLD_BASIC)
					edge->m_Color = color;
			}
		}
	}
}

/**
 * Get the color of the building.  In the case of multi-colored buildings,
 *  note that this method returns only the first color encountered.
 *
 * \param which Can be either BLD_BASIC (color of the building)	or BLD_ROOF
 *		(color of the roof).
 */
RGBi vtBuilding::GetColor(BldColor which) const
{
	int i, levs = m_Levels.GetSize();
	for (i = 0; i < levs; i++)
	{
		vtLevel *pLev = m_Levels[i];
		int j, edges = pLev->NumEdges();
		for (j = 0; j < edges; j++)
		{
			vtEdge *edge = pLev->GetEdge(j);
			if (edge->m_iSlope < 90)
			{
				if (which == BLD_ROOF)
					return edge->m_Color;
			}
			else
			{
				if (which == BLD_BASIC)
					return edge->m_Color;
			}
		}
	}
	return RGBi(0,0,0);
}

/**
 * Set the height of the building in stories.  If the building has no levels,
 * two will be created: for the walls and the roof.  If the number of stories
 * is greater than before, the additional stories are added to the top-most
 * non-roof level.  If lesser, stories and levels are removed from the top
 * down until the desired number is met.
 *
 * \param iStories Number of stories to set.
 */
void vtBuilding::SetNumStories(int iStories)
{
	vtLevel *pLev;

	int previous = NumStories();
	if (previous == iStories)
		return;

	// this method assume each building must have at least two levels: one
	// for the walls and one for the roof.
	int levels = m_Levels.GetSize();
	if (levels == 0)
	{
		CreateLevel();
		levels++;
	}
	if (levels == 1)
	{
		vtLevel *pFirstLev = GetLevel(0);
		pLev = CreateLevel();
		pLev->SetFootprint(pFirstLev->GetFootprint());
		pLev->SetRoofType(ROOF_FLAT, 0);
		levels++;
	}
	previous = NumStories(); // Just in case it changed

	// increase if necessary
	if (iStories > previous)
	{
		// get top non-roof level
		pLev = m_Levels[levels-2];
		// added some stories
		pLev->m_iStories += (iStories - previous);
	}
	// decrease if necessary
	while (NumStories() > iStories)
	{
		// get top non-roof level
		pLev = m_Levels[levels-2];
		pLev->m_iStories--;
		if (pLev->m_iStories == 0)
		{
			delete pLev;
			m_Levels.RemoveAt(levels - 2);
			levels--;
		}
	}

	// keep 2d and 3d in synch
	DetermineLocalFootprints();
}

/**
 * Get the total number of stories of this building.  The top level is assumed
 *  to be a roof and does not count toward the total.
 */
int vtBuilding::NumStories() const
{
	// this method assume each building must have at least two levels: one
	// for the walls and one for the roof.
	int stories = 0;
	uint levs = m_Levels.GetSize();
	if (levs > 0)
	{
		for (uint i = 0; i < levs - 1; i++)
			stories += m_Levels[i]->m_iStories;
	}
	return stories;
}

float vtBuilding::GetTotalHeight() const
{
	float h = 0.0f;
	for (uint i = 0; i < m_Levels.GetSize(); i++)
	{
		vtLevel *lev = m_Levels[i];
		h += (lev->m_fStoryHeight * lev->m_iStories);
	}
	return h;
}

/**
 * Set the footprint of the given level of the building.
 *
 * \param lev The level, from 0 for the base level and up.
 * \param foot The footprint.
 */
void vtBuilding::SetFootprint(int lev, const DLine2 &foot)
{
	int levs = NumLevels();
	if (lev >= levs)
		CreateLevel();

	m_Levels[lev]->SetFootprint(foot);

	// keep 2d and 3d in synch
	DetermineLocalFootprints();
}

/**
 * Set the footprintf of the given level of the building.
 *
 * \param lev The level, from 0 for the base level and up.
 * \param poly The footprint.
 */
void vtBuilding::SetFootprint(int lev, const DPolygon2 &poly)
{
	int levs = NumLevels();
	if (lev >= levs)
		CreateLevel();

	m_Levels[lev]->SetFootprint(poly);

	// keep 2d and 3d in synch
	DetermineLocalFootprints();
}

/**
 * Set the type of roof for this building.  In cases of ambiguity, such as
 *  setting a gable roof, the method will try to make intelligent guesses
 *  about where to put the roof angles based the length of the roof edges.
 *
 * \param rt Roof type, one of:
 *		- ROOF_FLAT
 *		- ROOF_SHED
 *		- ROOF_GABLE
 *		- ROOF_HIP
 * \param iSlope For a non-flat roof, this is the slope in degrees of the
 *		sloped edges. This varies from 0 (horizontal) to 90 (vertical).
 * \param iLev (optional) The number of the level to assume is the roof.
 *		If omitted, the top level is assumed to be the roof.
 */
void vtBuilding::SetRoofType(RoofType rt, int iSlope, int iLev)
{
	int i, edges;

	if (NumLevels() < 2)
	{
		// should not occur - this method is intended for buildings with roofs
		return;
	}

	// if there is a roof level, attempt to set its edge angles to match
	// the desired roof type
	vtLevel *pLev, *below;
	if (iLev == -1)
	{
		pLev = GetLevel(NumLevels()-1);
		below = GetLevel(NumLevels()-2);
	}
	else
	{
		pLev = GetLevel(iLev);
		below = GetLevel(iLev-1);
	}

	// If roof level has no edges then give it some
	edges = pLev->NumEdges();
	if (0 == edges)
	{
		pLev->SetFootprint(below->GetFootprint());
	}
	edges = pLev->NumEdges();

	// provide default slopes for the roof sections
	if (iSlope == -1)
	{
		if (rt == ROOF_SHED)
			iSlope = 4;
		if (rt == ROOF_GABLE || rt == ROOF_HIP)
			iSlope = 15;
	}

	pLev->SetRoofType(rt, iSlope);

	// all horizontal edges of the roof should default to the same material
	// as the wall section below them
	for (i = 0; i < edges; i++)
	{
		vtEdge *edge0 = below->GetEdge(i);
		vtEdge *edge1 = pLev->GetEdge(i);
		if (edge1->m_iSlope == 90)
		{
			edge1->m_pMaterial = edge0->m_pMaterial;
			edge1->m_Color = edge0->m_Color;
		}
	}
}

RoofType vtBuilding::GetRoofType() const
{
	// try to guess type of roof from looking at slopes of edges of
	// the top level
	const vtLevel *pLev = GetLevel(NumLevels()-1);

	return pLev->GuessRoofType();
}

RGBi vtBuilding::GuessRoofColor() const
{
	const vtLevel *pRoof = GetLevel(NumLevels()-1);
	for (int i = 0; i < pRoof->NumEdges(); i++)
	{
		if (pRoof->GetEdge(i)->m_iSlope != 90)
			return pRoof->GetEdge(i)->m_Color;
	}
	return RGBi(255, 255, 255);
}

void vtBuilding::SetRoofColor(const RGBi &rgb)
{
	vtLevel *pRoof = GetLevel(NumLevels()-1);
	for (int i = 0; i < pRoof->NumEdges(); i++)
	{
		if (pRoof->GetEdge(i)->m_iSlope != 90)
			pRoof->GetEdge(i)->m_Color = rgb;
	}
}

bool vtBuilding::GetBaseLevelCenter(DPoint2 &p) const
{
	DRECT rect;
	if (!GetExtents(rect))
		return false;
	rect.GetCenter(p);
	return true;
}

void vtBuilding::Offset(const DPoint2 &p)
{
	for (uint i = 0; i < m_Levels.GetSize(); i++)
	{
		vtLevel *lev = m_Levels[i];
		DPolygon2 &foot = lev->GetFootprint();
		foot.Add(p);
	}

	// keep 2d and 3d in synch
	DetermineLocalFootprints();
}

//
// Get an extent rectangle around the building.
// It doesn't need to be exact.
//
bool vtBuilding::GetExtents(DRECT &rect) const
{
	uint levs = m_Levels.GetSize();
	if (levs == 0)
		return false;

	rect.SetInsideOut();
	for (uint i = 0; i < levs; i++)
	{
		vtLevel *lev = m_Levels[i];
		if (lev->GetFootprint().size() != 0)	// safety check
			rect.GrowToContainLine(lev->GetOuterFootprint());
	}
	return true;
}

vtLevel *vtBuilding::CreateLevel()
{
	vtLevel *pLev = new vtLevel;
	m_Levels.Append(pLev);

	// We don't have to call DetermineLocalFootprints(), because the new level is empty.
	return pLev;
}

vtLevel *vtBuilding::CreateLevel(const DPolygon2 &footprint)
{
	vtLevel *pLev = new vtLevel;
	pLev->SetFootprint(footprint);

	m_Levels.Append(pLev);

	// keep 2d and 3d in synch
	DetermineLocalFootprints();

	return pLev;
}

void vtBuilding::InsertLevel(int iLev, vtLevel *pLev)
{
	int levels = NumLevels();
	m_Levels.SetSize(levels+1);
	for (int i = levels; i > iLev; i--)
	{
		m_Levels[i] = m_Levels[i-1];
	}
	m_Levels[iLev] = pLev;

	// keep 2d and 3d in synch
	DetermineLocalFootprints();
}

void vtBuilding::DeleteLevel(int iLev)
{
	int levels = NumLevels();
	for (int i = iLev; i < levels-1; i++)
	{
		m_Levels[i] = m_Levels[i+1];
	}
	m_Levels.SetSize(levels-1);

	// keep 2d and 3d in synch
	DetermineLocalFootprints();
}

void vtBuilding::SetRectangle(const DPoint2 &center, float fWidth,
							  float fDepth, float fRotation)
{
	vtLevel *pLev;

	// this function requires at least one level to exist
	if (m_Levels.GetSize() == 0)
	{
		pLev = new vtLevel;
		pLev->m_iStories = 1;
		m_Levels.Append(pLev);
	}
	else
		pLev = m_Levels[0];

	// if rotation is unset, default to none
	if (fRotation == -1.0f)
		fRotation = 0.0f;

	DPoint2 pt(fWidth / 2.0, fDepth / 2.0);
	DPoint2 corner[4];
	corner[0].Set(-pt.x, -pt.y);
	corner[1].Set(pt.x, -pt.y);
	corner[2].Set(pt.x, pt.y);
	corner[3].Set(-pt.x, pt.y);
	corner[0].Rotate(fRotation);
	corner[1].Rotate(fRotation);
	corner[2].Rotate(fRotation);
	corner[3].Rotate(fRotation);

	DLine2 dl;
	dl.Append(center + corner[0]);
	dl.Append(center + corner[1]);
	dl.Append(center + corner[2]);
	dl.Append(center + corner[3]);

	pLev->SetFootprint(dl);
}

/**
 * Find the closest distance from a given point to the interior of a
 * building's lowest footprint.  If the point is inside the footprint,
 * the value 0.0 is returned.
 */
double vtBuilding::GetDistanceToInterior(const DPoint2 &point) const
{
	vtLevel *lev = m_Levels[0];

	// Ignore holes - a small shortcut, could be be addressed later
	const DLine2 &foot = lev->GetOuterFootprint();
	if (foot.ContainsPoint(point))
		return 0.0;

	int i, edges = foot.GetSize();
	double dist, closest = 1E8;
	for (i = 0; i < edges; i++)
	{
		DPoint2 p0 = foot[i];
		DPoint2 p1 = foot[(i+1)%edges];
		dist = DistancePointToLine(p0, p1, point);
		if (dist < closest)
			closest = dist;
	}
	return closest;
}

void vtBuilding::WriteXML(GZOutput &out, bool bDegrees) const
{
	const char *coord_format = "%.9lg";	// up to 9 significant digits

	gfprintf(out, "\t<Building");
	if (m_fElevationOffset != 0.0)
		gfprintf(out, " ElevationOffset=\"%.2f\"", m_fElevationOffset);
	if (m_bAbsolute)
		gfprintf(out, " Absolute=\"true\"");
	gfprintf(out, ">\n");

	int i, j, k;
	int levels = NumLevels();
	for (i = 0; i < levels; i++)
	{
		const vtLevel *lev = GetLevel(i);
		gfprintf(out, "\t\t<Level FloorHeight=\"%f\" StoryCount=\"%d\">\n",
			lev->m_fStoryHeight, lev->m_iStories);

		gfprintf(out, "\t\t\t<Footprint>\n");
		gfprintf(out, "\t\t\t\t<gml:Polygon>\n");

		// Every footprint polygon has at least one outer boundary
		gfprintf(out, "\t\t\t\t\t<gml:outerBoundaryIs>\n");
		gfprintf(out, "\t\t\t\t\t\t<gml:LinearRing>\n");
		gfprintf(out, "\t\t\t\t\t\t\t<gml:coordinates>");

		const DPolygon2 &pfoot = lev->GetFootprint();
		const DLine2 &outer = pfoot[0];
		int points = outer.GetSize();
		for (j = 0; j < points; j++)
		{
			gfprintf(out, coord_format, outer[j].x);
			gfprintf(out, ",");
			gfprintf(out, coord_format, outer[j].y);
			if (j != points-1)
				gfprintf(out, " ");
		}

		gfprintf(out, "</gml:coordinates>\n");
		gfprintf(out, "\t\t\t\t\t\t</gml:LinearRing>\n");
		gfprintf(out, "\t\t\t\t\t</gml:outerBoundaryIs>\n");

		// If we have a compound footprint, write inner rings separately
		int rings = pfoot.size();
		for (int iring = 1; iring < rings; iring++)
		{
			gfprintf(out, "\t\t\t\t\t<gml:innerBoundaryIs>\n");
			gfprintf(out, "\t\t\t\t\t\t<gml:LinearRing>\n");
			gfprintf(out, "\t\t\t\t\t\t\t<gml:coordinates>");

			const DLine2 &inner = pfoot[iring];
			int points = inner.GetSize();
			for (j = 0; j < points; j++)
			{
				gfprintf(out, coord_format, inner[j].x);
				gfprintf(out, ",");
				gfprintf(out, coord_format, inner[j].y);
				if (j != points-1)
					gfprintf(out, " ");
			}
			gfprintf(out, "</gml:coordinates>\n");
			gfprintf(out, "\t\t\t\t\t\t</gml:LinearRing>\n");
			gfprintf(out, "\t\t\t\t\t</gml:innerBoundaryIs>\n");
		}

		gfprintf(out, "\t\t\t\t</gml:Polygon>\n");
		gfprintf(out, "\t\t\t</Footprint>\n");

		int edges = lev->NumEdges();
		for (j = 0; j < edges; j++)
		{
			vtEdge *edge = lev->GetEdge(j);
			gfprintf(out, "\t\t\t<Edge");

			if (edge->m_pMaterial)
				gfprintf(out, " Material=\"%s\"",
					(const char *)*edge->m_pMaterial);

			gfprintf(out, " Color=\"%02x%02x%02x\"",
				edge->m_Color.r, edge->m_Color.g, edge->m_Color.b);

			if (edge->m_iSlope != 90)
				gfprintf(out, " Slope=\"%d\"", edge->m_iSlope);

			if (!edge->m_Facade.IsEmpty())
				gfprintf(out, " Facade=\"%s\"", (pcchar)edge->m_Facade);

			gfprintf(out, ">\n");

			int features = edge->NumFeatures();
			for (k = 0; k < features; k++)
			{
				gfprintf(out, "\t\t\t\t<EdgeElement");

				const vtEdgeFeature &feat = edge->m_Features[k];
				gfprintf(out, " Type=\"%s\"",
					vtBuilding::GetEdgeFeatureString(feat.m_code));

				if (feat.m_vf1 != 0.0f)
					gfprintf(out, " Begin=\"%.3f\"", feat.m_vf1);

				if (feat.m_vf2 != 1.0f)
					gfprintf(out, " End=\"%.3f\"", feat.m_vf2);

				gfprintf(out, "/>\n");
			}

			gfprintf(out, "\t\t\t</Edge>\n");
		}
		gfprintf(out, "\t\t</Level>\n");
	}

	WriteTags(out);
	gfprintf(out, "\t</Building>\n");
}

void vtBuilding::AddDefaultDetails()
{
	// requires at least 2 levels to operate
	int numlevels = NumLevels();
	while (numlevels < 2)
	{
		CreateLevel();
		numlevels = NumLevels();
	}

	// add some default windows/doors
	for (int i = 0; i < numlevels - 1; i++)
	{
		vtLevel *lev = m_Levels[i];
		const int edges = lev->NumEdges();
		for (int j = 0; j < edges; j++)
		{
			vtEdge *edge = lev->GetEdge(j);
			const int doors = 0;
			const int windows = (int) (lev->GetLocalEdgeLength(j) / 6.0f);
			edge->Set(doors, windows, BMAT_NAME_SIDING);
		}
	}

	// process roof level
	vtLevel *roof = m_Levels[numlevels - 1];
	int edges = roof->NumEdges();
	if (0 == edges)
		roof->SetFootprint(m_Levels[0]->GetFootprint());
	edges = roof->NumEdges();

	for (int j = 0; j < edges; j++)
	{
		vtEdge *edge = roof->GetEdge(j);
		edge->m_iSlope = 0;		// flat roof
	}
	DetermineLocalFootprints();
}

void vtBuilding::DetermineLocalFootprints()
{
	DPoint2 center;
	GetBaseLevelCenter(center);

	// The local conversion will be use to make the local footprints.
	LocalCS local_cs;
	local_cs.Setup(m_pCRS->GetUnits(), center);

	int i;
	int levs = m_Levels.GetSize();
	float fHeight = 0.0f;

	for (i = 0; i < levs; i++)
	{
		vtLevel *lev = m_Levels[i];
		lev->DetermineLocalFootprint(fHeight, local_cs);
		fHeight += (lev->m_iStories * lev->m_fStoryHeight);
	}
}

const char *vtBuilding::GetEdgeFeatureString(int edgetype)
{
	switch (edgetype)
	{
	case WFC_WALL: return "Wall"; break;
	case WFC_GAP: return "Gap"; break;
	case WFC_POST: return "Post"; break;
	case WFC_WINDOW: return "Window"; break;
	case WFC_DOOR: return "Door"; break;
	}
	return "Bad Value";
}

int vtBuilding::GetEdgeFeatureValue(const char *value)
{
	if (!strcmp(value, "Wall"))
		return WFC_WALL;
	else if (!strcmp(value, "Gap"))
		return WFC_GAP;
	else if (!strcmp(value, "Post"))
		return WFC_POST;
	else if (!strcmp(value, "Window"))
		return WFC_WINDOW;
	else if (!strcmp(value, "Door"))
		return WFC_DOOR;
	return 0;
}

bool vtBuilding::IsContainedBy(const DRECT &rect) const
{
	// It's easier to select buildings using their centers, than using their extents.
	DPoint2 center;
	GetBaseLevelCenter(center);
	return rect.ContainsPoint(center);
}

void vtBuilding::SwapLevels(int lev1, int lev2)
{
	vtLevel *pTemp = m_Levels[lev1];
	m_Levels[lev1] = m_Levels[lev2];
	m_Levels[lev2] = pTemp;

	// keep 2d and 3d in sync
	DetermineLocalFootprints();
}

void vtBuilding::SetEaves(float fLength)
{
	// Assume that the top level is the roof
	vtLevel *roof = m_Levels[m_Levels.GetSize()-1];
	if (roof->NumEdges() <= 4)
		SetEavesSimple(fLength);
	else
		SetEavesFelkel(fLength);
}

void vtBuilding::SetEavesSimple(float fLength)
{
	// Assume that the top level is the roof
	vtLevel *roof = m_Levels[m_Levels.GetSize()-1];
	float height = roof->m_fStoryHeight;

	for (int i = 0; i < roof->NumEdges(); i++)
	{
		vtEdge *edge = roof->GetEdge(i);

		// Ignore vertical walls and flat roofs
		if (edge->m_iSlope == 90 || edge->m_iSlope == 0)
			continue;

		// Assume only one edge feature on each edge of a roof.
		if (edge->NumFeatures() != 1)
			continue;

		// Simple trigonometry to convert the height, angle and length of the
		// eave (in meters) to produce the length as a fraction.
		float angle = edge->SlopeRadians();
		float fraction = fLength / (height / sin(angle));

		edge->m_Features[0].m_vf1 = -fraction;
	}
}

void vtBuilding::SetEavesFelkel(float fLength)
{
	// Assume that the top level is the roof
	vtLevel *roof = m_Levels[m_Levels.GetSize()-1];

	// Convert meters to local units
	DPolygon2 &foot = roof->GetFootprint();
	DLine2 &line = foot[0];
	DLine2 offset(line.GetSize());
	for (int i = 0; i < (int) line.GetSize(); i++)
	{
		AngleSideVector(line.GetSafePoint(i-1), line[i], line.GetSafePoint(i+1), offset[i]);
	}
	for (int i = 0; i < (int) line.GetSize(); i++)
	{
		// Offset points to the left, subtract it to go to the right, which on a
		// counter-clockwise polygon, expands the polygon.
		line[i] -= (offset[i] * fLength);
	}

	// keep 2d and 3d in sync
	DetermineLocalFootprints();
}

void vtBuilding::CopyStyleFrom(const vtBuilding * const pSource, bool bDoHeight)
{
	SetElevationOffset(pSource->GetElevationOffset());

	DPolygon2 foot;

	// If we will copy height information, then we will make as many levels as
	// needed to match the source.
	uint from_levels = pSource->NumLevels();
	uint copy_levels;
	if (bDoHeight)
		copy_levels = from_levels;
	else
		copy_levels = NumLevels();

	// Copy the roof angles first.
	SetRoofType(pSource->GetRoofType());

	for (uint i = 0; i < copy_levels; i++)
	{
		vtLevel *pLevel = GetLevel(i);
		if (pLevel)
			foot = pLevel->GetFootprint();
		else
			pLevel = CreateLevel(foot);

		int from_level;
		const vtLevel *pFromLevel;
		if (bDoHeight)
		{
			from_level = i;
			pFromLevel = pSource->GetLevel(i);

			pLevel->m_iStories = pFromLevel->m_iStories;
			pLevel->m_fStoryHeight = pFromLevel->m_fStoryHeight;
		}
		else
		{
			// The target building may have more more levels than the default.
			//  Try to guess an appropriate corresponding level.
			if (i == 0)
				from_level = 0;
			else if (i == copy_levels-1)
				from_level = from_levels - 1;
			else
			{
				from_level = i;
				if (from_levels > 2)
					from_level = 1;
				else
					from_level = 0;
			}
			pFromLevel = pSource->GetLevel(from_level);
		}
		int from_edges = pFromLevel->NumEdges();
		int to_edges = pLevel->NumEdges();

		// Now that we have a source and target level, iterate through the edges.
		if (i == copy_levels - 1 && i > 0)
		{
			// Handle roof specially: do the sloped edges.
			vtLevel *below = GetLevel(i - 1);
			RGBi color;
			const vtString *material;
			float vf1;		// Used for roof overhang / eaves.
			for (int j = 0; j < from_edges; j++)
			{
				vtEdge *from_edge = pFromLevel->GetEdge(j);
				if (from_edge->m_iSlope != 90)
				{
					color = from_edge->m_Color;
					material = from_edge->m_pMaterial;
					vf1 = from_edge->m_Features[0].m_vf1;
				}
			}
			for (int j = 0; j < to_edges; j++)
			{
				vtEdge *to_edge = pLevel->GetEdge(j);
				if (to_edge->m_iSlope != 90)
				{
					to_edge->m_Color = color;
					to_edge->m_pMaterial = material;
					to_edge->m_Features[0].m_vf1 = vf1;
				}
				else
				{
					to_edge->m_Color = below->GetEdge(j)->m_Color;
					to_edge->m_pMaterial = below->GetEdge(j)->m_pMaterial;
				}
			}
			continue;
		}
		// The non-roof case:
		for (int j = 0; j < to_edges; j++)
		{
			const int k = j % from_edges;

			vtEdge *from_edge = pFromLevel->GetEdge(k);
			vtEdge *to_edge = pLevel->GetEdge(j);

			to_edge->m_Color = from_edge->m_Color;
			to_edge->m_pMaterial = from_edge->m_pMaterial;

			// Try to create similar edge features (walls, windows, doors)
			if (from_edge->IsUniform())
			{
				const int doors = 0;
				int windows = (int) (pLevel->GetLocalEdgeLength(j) / 6.0f);
				if (windows < 1)
					windows = 1;
				to_edge->Set(doors, windows, BMAT_NAME_SIDING);
			}
			else if (from_edge->NumFeatures() == 1)
			{
				to_edge->m_Features.clear();
				to_edge->m_Features.push_back(from_edge->m_Features[0]);
			}
			else
			{
				// General case: not uniform, more than one feature
				const int num_features1 = from_edge->NumFeatures();
				const float length1 = pFromLevel->GetLocalEdgeLength(k);
				float features_per_meter = (float) num_features1 / length1;

				const float length2 = pLevel->GetLocalEdgeLength(j);
				int num_features2 = (int) (features_per_meter * length2);
				if (num_features2 < 1)
					num_features2 = 1;

				to_edge->m_Features.clear();
				for (int m = 0; m < num_features2; m++)
					to_edge->m_Features.push_back(from_edge->m_Features[m % num_features1]);
			}
		}
	}
}

