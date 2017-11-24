//
// Roads.cpp
//
// also shorelines and rivers
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/vtLog.h"
#include "vtdata/DataPath.h"

#include "Light.h"
#include "Roads.h"
#include "Content3d.h"	// content manager for sign models

#define ROADSIDE_WIDTH		2.0f
#define ROADSIDE_DEPTH		-ROADSIDE_WIDTH

#define UV_SCALE_ROAD		(.08f)
#define UV_SCALE_SIDEWALK	(1.00f)

#define ROAD_AMBIENT 0.6	// brighter than terrain ambient
#define ROAD_DIFFUSE 0.4

#define ROAD_REZ 2048


////////////////////////////////////////////////////////////////////

//
// return the positive difference of two angles (a - b)
// allow for wrapping around 2 PI
//
float angle_diff(float a, float b)
{
	if (a > b)
		return a-b;
	else
		return PI2f+a-b;
}

//
// helper: given two points along a link, produce a vector
// along to that link, parallel to the ground plane,
// with length corresponding to the supplied width
//
FPoint3 CreateRoadVector(const FPoint3 &p1, const FPoint3 &p2, const float w)
{
	FPoint3 v = p2 - p1;
	v.y = 0;
	v.Normalize();
	v.x *= (w / 2.0f);
	v.z *= (w / 2.0f);
	return v;
}

FPoint3 CreateUnitLinkVector(FPoint3 p1, FPoint3 p2)
{
	FPoint3 v = p2 - p1;
	v.y = 0;
	v.Normalize();
	return v;
}


/////////////////////////////////////////////////////////////////////////

NodeGeom::NodeGeom()
{
	m_iVerts = 0;
}

NodeGeom::~NodeGeom()
{
}

FPoint3 NodeGeom::GetUnitLinkVector(int i)
{
	const FPoint3 &linkpoint = GetAdjacentRoadpoint(i);
	return CreateUnitLinkVector(m_p3, linkpoint);
}

const FPoint3 &NodeGeom::GetAdjacentRoadpoint(int iLinkNumber)
{
	TLink *link = m_connect[iLinkNumber];
	LinkGeom *lg = (LinkGeom*) link;
	if (lg->GetNode(0) == this)
		return lg->m_centerline[1];
	else
		return lg->m_centerline[lg->GetSize() - 2];
}


// statistics
int one = 0, two = 0, many = 0;

void NodeGeom::ComputeIntersectionVertices()
{
	FPoint3 v, v_next, v_prev;
	FPoint3 pn0, pn1;
	float w;			// link width

	SortLinksByAngle();

	// how many links meet here?
	if (NumLinks() == 0)
	{
		; // bogus case (put a breakpoint here)
	}
	else if (NumLinks() == 1)
	{
		// dead end: only need 2 vertices for this node
		m_iVerts = 2;
		m_v.SetSize(2);

		// get info about the link
		LinkGeom *r = GetLink(0);
		w = (r->m_fLeftWidth + r->m_fRightWidth) / 2;

		pn1 = GetAdjacentRoadpoint(0);
		v = CreateRoadVector(m_p3, pn1, w);

		m_v[0].Set(m_p3.x + v.z, m_p3.y, m_p3.z - v.x);
		m_v[1].Set(m_p3.x - v.z, m_p3.y, m_p3.z + v.x);

		one++;
	}
	else if (NumLinks() == 2)
	{
		// only need 2 vertices for this node; no intersection
		m_iVerts = 2;
		m_v.SetSize(2);

		// get info about the links
		w = (GetLink(0)->m_fLeftWidth + GetLink(1)->m_fRightWidth) / 2.0f;

		pn0 = GetAdjacentRoadpoint(0);
		pn1 = GetAdjacentRoadpoint(1);

		v = CreateRoadVector(pn0, pn1, w);

		m_v[0].Set(m_p3.x + v.z, m_p3.y, m_p3.z - v.x);
		m_v[1].Set(m_p3.x - v.z, m_p3.y, m_p3.z + v.x);

		two++;
	}
	else
	{
		// intersection: need 2 vertices for each link meeting here
		m_iVerts = 2 * NumLinks();
		m_v.SetSize(m_iVerts);

		// For each pairs of links, find the places where the link edges
		//  intersect as they approach this node.

		// The following is an array of float triples, used as follows:
		//  x = minimum distance which avoids intersection with next link
		//  y = minimum distance which avoids intersection with previous link
		//  z = greater of x or y.
		FLine3 distance_to_intersection(NumLinks());

		// Go through the links once, collecting the minimum distances
		for (int i = 0; i < NumLinks(); i++)
		{
			// indices of the next and previous links
			const int i_next = (i == NumLinks()-1) ? 0 : i+1;

			const TLink *pL = GetLink(i);
			const TLink *pL_next = GetLink(i_next);

			const float width1 = pL->m_fLeftWidth;
			const float width2 = pL_next->m_fRightWidth;

			const FPoint3 linkv1 = GetUnitLinkVector(i);
			const FPoint3 linkv2 = GetUnitLinkVector(i_next);

			// Use 2D vectors for the following math
			// Compute two vectors: left road edge of this link, right road
			//  edge of the following link, compute where they intersect, in
			//  terms of the ua and ub factors, which are the distance along
			//  each input vector to the intersection point.
			const FPoint2 v1(linkv1.x, linkv1.z);
			const FPoint2 v2(linkv2.x, linkv2.z);

			const float denom = v2.y*v1.x - v2.x*v1.y;
			if (fabs(denom) < 0.01)
			{
				// too parallel, pick a safety value
				distance_to_intersection[i].x = 1.0f;
				distance_to_intersection[i_next].y = 1.0f;
			}
			else
			{
				FPoint2 norm1(linkv1.z, -linkv1.x);
				FPoint2 norm2(linkv2.z, -linkv2.x);
				norm1.Normalize();
				norm2.Normalize();

				const FPoint2 center(m_p3.x, m_p3.z);
				const FPoint2 p1 = center + norm1 * width1;
				const FPoint2 p2 = center - norm2 * width2;

				const float ua = (v2.x*(p1.y - p2.y) - v2.y*(p1.x - p2.x)) / denom;
				const float ub = (v1.x*(p1.y - p2.y) - v1.y*(p1.x - p2.x)) / denom;

				distance_to_intersection[i].x = ua;
				distance_to_intersection[i_next].y = ub;
			}
		}
		// Go through the links again, picking the largest minimum
		for (int i = 0; i < NumLinks(); i++)
		{
			distance_to_intersection[i].z = std::max(distance_to_intersection[i].x,
				distance_to_intersection[i].y);
		}
		// Now we can finally set the two points where this link meets the
		//  intersection without overlapping with the other links
		for (int i = 0; i < NumLinks(); i++)
		{
			const TLink *pL = GetLink(i);
			v = GetUnitLinkVector(i);

			FPoint3 norm(v.z, 0, -v.x);
			norm.Normalize();
			norm *= pL->m_fLeftWidth;

			const float dist = distance_to_intersection[i].z;
			m_v[i * 2 + 0] = m_p3 + norm + (v * dist);
			m_v[i * 2 + 1] = m_p3 - norm + (v * dist);
		}
		many++;
	}
}


//
// Given a node and a link, return the two points that the link
// will need in order to hook up with the node.
//
void NodeGeom::FindVerticesForLink(TLink *pL, bool bStart, FPoint3 &p0, FPoint3 &p1)
{
	if (NumLinks() == 1)
	{
		p0 = m_v[0];
		p1 = m_v[1];
	}
	else if (NumLinks() == 2)
	{
		if (pL == m_connect[0])
		{
			p0 = m_v[1];
			p1 = m_v[0];
		}
		else
		{
			p0 = m_v[0];
			p1 = m_v[1];
		}
	}
	else
	{
		for (int i = 0; i < NumLinks(); i++)
		{
			if (m_connect[i] == pL)
			{
				p0 = m_v[i*2];
				p1 = m_v[i*2+1];
				return;
			}
		}
		// Should not get here!  This node does not reference the link passed
		; // (put a breakpoint here)
	}
}


vtMesh *NodeGeom::GenerateGeometry(const VirtualTexture &vt)
{
	if (NumLinks() < 3)
		return NULL;

	FPoint2 uv;

	const FPoint3 upvector(0.0f, 1.0f, 0.0f);

	vtMesh *pMesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_FAN, VT_TexCoords | VT_Normals, NumLinks()*2 + 1);
	int verts = 0;

	vt.Adapt(FPoint2(0.5f, 0.5f), uv);

	pMesh->SetVtxPUV(verts, m_p3, uv.x, uv.y);
	pMesh->SetVtxNormal(verts, upvector);
	verts++;

	for (int j = 0; j < NumLinks(); j++)
	{
		vt.Adapt(FPoint2(0.0f, 1.0f), uv);
		pMesh->SetVtxPUV(verts, m_v[j*2+1], uv.x, uv.y);

		vt.Adapt(FPoint2(1.0f, 1.0f), uv);
		pMesh->SetVtxPUV(verts+1, m_v[j*2], uv.x, uv.y);

		pMesh->SetVtxNormal(verts, upvector);
		pMesh->SetVtxNormal(verts+1, upvector);
		verts += 2;
	}

	// create triangles
	verts = 0;
	int idx[100];
	idx[verts++] = 0;
	for (int j = 0; j < NumLinks(); j++)
	{
		idx[verts++] = (j*2+1);
		idx[verts++] = (j*2+2);
	}
	idx[verts++] = 1;	// close it
	pMesh->AddFan(idx, verts);
	return pMesh;
}


////////////////////////////////////////////////////////////////////////

RoadBuildInfo::RoadBuildInfo(int iCoords)
{
	center.SetSize(iCoords);
	crossvector.SetSize(iCoords);
	fvLength.SetSize(iCoords);
	verts = vert_index = 0;
}


////////////////////////////////////////////////////////////////////////

LinkGeom::LinkGeom()
{
}

LinkGeom::~LinkGeom()
{
}

void LinkGeom::SetupBuildInfo(RoadBuildInfo &bi)
{
	float length = 0.0f;

	//  for each point in the link, determine coordinates
	uint j, size = GetSize();
	for (j = 0; j < size; j++)
	{
		FPoint3 left, right;

		if (j > 0)
		{
			// increment 2D length along link
			FPoint2 v2;
			v2.x = m_centerline[j].x - m_centerline[j-1].x;
			v2.y = m_centerline[j].z - m_centerline[j-1].z;
			length += v2.Length();
		}
		bi.fvLength[j] = length;

		// At sharp corners, we must widen the link to keep each segment the
		//  desired width.
		float wider = 1.0f;

		// we will add 2 vertices to the link mesh
		FPoint3 p0, p1;
		if (j == 0)
		{
			// add 2 vertices at this point, copied from the start node
			GetNode(0)->FindVerticesForLink(this, true, right, left);	// true means start
			//wider = (right-left).Length() / (m_fLeftWidth + m_fRightWidth);
		}
		if (j > 0 && j < size-1)
		{
			// add 2 vertices at this point, directed at the previous and next points
			FPoint3 bisector;
			wider = AngleSideVector(m_centerline[j-1], m_centerline[j], m_centerline[j+1], bisector);

			left = m_centerline[j] - bisector;
			right = m_centerline[j] + bisector;
		}
		if (j == size-1)
		{
			// add 2 vertices at this point, copied from the end node
			GetNode(1)->FindVerticesForLink(this, false, left, right);	// false means end
			//wider = (right-left).Length() / (m_fLeftWidth + m_fRightWidth);
		}
		bi.crossvector[j] = right - left;
//		bi.center[j] = left + (bi.crossvector[j] * 0.5f);
//		bi.center[j] = m_centerline[j];
		bi.center[j] = (left + right) / 2;
		bi.crossvector[j].Normalize();
		bi.crossvector[j] *= wider;
	}
}

void LinkGeom::AddRoadStrip(vtMesh *pMesh, RoadBuildInfo &bi,
							float offset_left, float offset_right,
							float height_left, float height_right,
							VirtualTexture &vt,
							float u1, float u2, float uv_scale,
							normal_direction nd)
{
	FPoint3 local0, local1, normal;
	float texture_v;
	FPoint2 uv;

	for (uint j = 0; j < GetSize(); j++)
	{
		texture_v = bi.fvLength[j] * uv_scale;

		local0 = bi.center[j] + (bi.crossvector[j] * offset_left);
		local1 = bi.center[j] + (bi.crossvector[j] * offset_right);
		local0.y += height_left;
		local1.y += height_right;

		if (nd == ND_UP)
			normal.Set(0.0f, 1.0f, 0.0f);	// up
		else if (nd == ND_LEFT)
			normal = (bi.crossvector[j] * -1.0f);	// left
		else
			normal = bi.crossvector[j];		// right

		vt.Adapt(FPoint2(u2, texture_v), uv);
		pMesh->AddVertexUV(local1, uv);

		vt.Adapt(FPoint2(u1, texture_v), uv);
		pMesh->AddVertexUV(local0, uv);

		pMesh->SetVtxNormal(bi.verts, normal);
		pMesh->SetVtxNormal(bi.verts+1, normal);
		bi.verts += 2;
	}
	// create tristrip
	pMesh->AddStrip2(GetSize() * 2, bi.vert_index);
	bi.vert_index += (GetSize() * 2);
}

void LinkGeom::GenerateGeometry(vtRoadMap3d *rmgeom)
{
	if (GetSize() < 2)	// safety check
		return;

	bool do_roadside = true;
	switch (m_Surface)
	{
	case SURFT_NONE:
		break;
	case SURFT_GRAVEL:
	case SURFT_TRAIL:
	case SURFT_2TRACK:
	case SURFT_DIRT:
		do_roadside = false;
		break;
	case SURFT_PAVED:
	case SURFT_RAILROAD:
	case SURFT_STONE:
		break;
	}
	do_roadside = false;	// temporary override
	if (m_iHwy > 0)
		m_iFlags |= RF_MARGIN;

	// calculate total vertex count for this geometry
	int total_vertices = GetSize() * 2;	// main surface
	if (m_iFlags & RF_MARGIN)
		total_vertices += (GetSize() * 2 * 2);	// 2 margin strips
	if (m_iFlags & RF_PARKING_LEFT)
		total_vertices += (GetSize() * 2 * 1);	// 1 parking strip
	if (m_iFlags & RF_PARKING_RIGHT)
		total_vertices += (GetSize() * 2 * 1);	// 1 parking strip
	if (m_iFlags & RF_SIDEWALK_LEFT)
		total_vertices += (GetSize() * 2 * 2);	// 2 sidewalk strips
	if (m_iFlags & RF_SIDEWALK_RIGHT)
		total_vertices += (GetSize() * 2 * 2);	// 2 sidewalk strips
	if (do_roadside)
		total_vertices += (GetSize() * 2 * 2);		// 2 roadside strips

	vtMesh *pMesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_STRIP, VT_TexCoords | VT_Normals,
		total_vertices);

	RoadBuildInfo bi(GetSize());
	SetupBuildInfo(bi);

	const float center_width = m_iLanes * m_fLaneWidth;
	float offset = -(center_width / 2);
	if (m_iFlags & RF_MARGIN)
		offset -= m_fMarginWidth;
	if (m_iFlags & RF_PARKING_LEFT)
		offset -= m_fParkingWidth;
	if (m_iFlags & RF_SIDEWALK_LEFT)
		offset -= m_fSidewalkWidth;
	if (do_roadside)
		offset -= ROADSIDE_WIDTH;

#if 0
	// create left roadside strip
	if (do_roadside)
	{
		AddRoadStrip(pMesh, bi,
					offset, offset+ROADSIDE_WIDTH,
					ROADSIDE_DEPTH,
					(m_iFlags & RF_SIDEWALK) ? m_fCurbHeight : 0.0f,
					rmgeom->m_vt[],
					0.02f, 0.98f, UV_SCALE_ROAD,
					ND_UP);
		offset += ROADSIDE_WIDTH;
	}
#endif

	// create left sidwalk
	if (m_iFlags & RF_SIDEWALK_LEFT)
	{
		AddRoadStrip(pMesh, bi,
					offset,
					offset + m_fSidewalkWidth,
					m_fCurbHeight, m_fCurbHeight,
					rmgeom->m_vt[VTI_SIDEWALK],
					0.0f, 0.93f, UV_SCALE_SIDEWALK,
					ND_UP);
		offset += m_fSidewalkWidth;
		AddRoadStrip(pMesh, bi,
					offset,
					offset,
					m_fCurbHeight, 0.0f,
					rmgeom->m_vt[VTI_SIDEWALK],
					0.93f, 1.0f, UV_SCALE_SIDEWALK,
					ND_RIGHT);
	}
	// create left parking lane
	if (m_iFlags & RF_PARKING_LEFT)
	{
		AddRoadStrip(pMesh, bi,
					offset,
					offset + m_fParkingWidth,
					0.0f, 0.0f,
					rmgeom->m_vt[VTI_1LANE],
					0.0f, 1.0f, UV_SCALE_ROAD,
					ND_UP);
		offset += m_fParkingWidth;
	}
	// create left margin
	if (m_iFlags & RF_MARGIN)
	{
		AddRoadStrip(pMesh, bi,
					offset,
					offset + m_fMarginWidth,
					0.0f, 0.0f,
					rmgeom->m_vt[VTI_MARGIN],
					0.0f, 1.0f, UV_SCALE_ROAD,
					ND_UP);
		offset += m_fMarginWidth;
	}

	// create main road surface
	AddRoadStrip(pMesh, bi,
				-center_width/2, center_width/2,
				0.0f, 0.0f,
				rmgeom->m_vt[m_vti],
				0.0f, 1.0f, UV_SCALE_ROAD,
				ND_UP);
	offset = center_width/2;

	// create right margin
	if (m_iFlags & RF_MARGIN)
	{
		AddRoadStrip(pMesh, bi,
					offset,
					offset + m_fMarginWidth,
					0.0f, 0.0f,
					rmgeom->m_vt[VTI_MARGIN],
					1.0f, 0.0f, UV_SCALE_ROAD,
					ND_UP);
		offset += m_fMarginWidth;
	}
	// create left parking lane
	if (m_iFlags & RF_PARKING_RIGHT)
	{
		AddRoadStrip(pMesh, bi,
					offset,
					offset + m_fParkingWidth,
					0.0f, 0.0f,
					rmgeom->m_vt[VTI_1LANE],
					0.0f, 1.0f, UV_SCALE_ROAD,
					ND_UP);
		offset += m_fParkingWidth;
	}

	// create right sidwalk
	if (m_iFlags & RF_SIDEWALK_RIGHT)
	{
		AddRoadStrip(pMesh, bi,
					offset,
					offset,
					0.0f, m_fCurbHeight,
					rmgeom->m_vt[VTI_SIDEWALK],
					1.0f, 0.93f, UV_SCALE_SIDEWALK,
					ND_LEFT);
		AddRoadStrip(pMesh, bi,
					offset,
					offset + m_fSidewalkWidth,
					m_fCurbHeight, m_fCurbHeight,
					rmgeom->m_vt[VTI_SIDEWALK],
					0.93f, 0.0f, UV_SCALE_SIDEWALK,
					ND_UP);
		offset += m_fSidewalkWidth;
	}

#if 0
	if (do_roadside)
	{
		// create left roadside strip
		AddRoadStrip(pMesh, bi,
					offset, offset+ROADSIDE_WIDTH,
					(m_iFlags & RF_SIDEWALK) ? m_fCurbHeight : 0.0f,
					ROADSIDE_DEPTH,
					MATIDX_ROADSIDE,
					0.98f, 0.02f, UV_SCALE_ROAD,
					ND_UP);
	}
#endif

	// set lane coordinates
	m_Lanes.resize(m_iLanes);
	for (uint i = 0; i < m_iLanes; i++)
	{
		m_Lanes[i].SetSize(GetSize());
	}
	for (uint j = 0; j < GetSize(); j++)
	{
		for (int i = 0; i < m_iLanes; i++)
		{
			float offset = -((float)(m_iLanes-1) / 2.0f) + i;
			offset *= m_fLaneWidth;
			FPoint3 offset_diff = bi.crossvector[j] * offset;
			m_Lanes[i].SetAt(j, bi.center[j] + offset_diff);
		}
	}

	assert(total_vertices == bi.verts);
	rmgeom->AddMeshToGrid(pMesh, rmgeom->m_vt[m_vti].m_idx);
}


FPoint3 LinkGeom::FindPointAlongRoad(float fDistance)
{
	FPoint3 v;

	float length = 0.0f;

	if (fDistance <= 0) {
		static int c = 0;
		c++;
		return m_centerline[0];
	}
	// compute 2D length of this link, by adding up the 2d link segment lengths
	for (uint j = 0; j < GetSize()-1; j++)
	{
		// consider length of next segment
		v.x = m_centerline[j+1].x - m_centerline[j].x;
		v.y = 0;
		v.z = m_centerline[j+1].z - m_centerline[j].z;
		length = v.Length();
		if (fDistance <= length)
		{
			float fraction = fDistance / length;
			FPoint3 p0, p1, diff;
			p0 = m_centerline[j];
			v *= fraction;
			return p0 + v;
		}
		fDistance -= length;
	}
	// if we pass the end of line, just return the last point
	return m_centerline[GetSize()-1];
}

//
// Return the 2D length of this link segment in world units
//
float LinkGeom::Length()
{
	FPoint3 v;
	v.y = 0;
	float length = 0.0f;

	// compute 2D length of this link, by adding up the 2d link segment lengths
	for (uint j = 0; j < GetSize(); j++)
	{
		if (j > 0)
		{
			// increment length
			v.x = m_centerline[j].x - m_centerline[j-1].x;
			v.z = m_centerline[j].z - m_centerline[j-1].z;
			float l = v.Length();
			if (l < 0) {
				assert(false);
			}
			length += l;
		}
	}
	return length;
}

///////////////////////////////////////////////////////////////////

vtRoadMap3d::vtRoadMap3d()
{
	m_pTransform = NULL;
	m_pGroup = NULL;
	m_pMats = NULL;
	m_fGroundOffset = 1.0f;
}

vtRoadMap3d::~vtRoadMap3d()
{
}

void vtRoadMap3d::ComputeIntersectionVertices()
{
	for (NodeGeom *pN = GetFirstNode(); pN; pN = pN->GetNext())
		pN->ComputeIntersectionVertices();
}


int clusters_used = 0;	// for statistical purposes

void vtRoadMap3d::AddMeshToGrid(vtMesh *pMesh, int iMatIdx)
{
	// which cluster does it belong to?
	int a, b;

	FBox3 bound;
	pMesh->GetBoundBox(bound);
	FPoint3 center = bound.Center();

	a = (int)((center.x - m_extents.min.x) / m_extent_range.x * ROAD_CLUSTER);
	b = (int)((center.z - m_extents.min.z) / m_extent_range.z * ROAD_CLUSTER);

	// safety check: if the following is true, then the geometry
	// has somehow gotten mangled, so it's producing extents
	// outside of what they should be, go no further
	assert(a >= 0 && a < ROAD_CLUSTER && b >= 0 && b < ROAD_CLUSTER);

	vtGeode *pGeode;
	if (m_pRoads[a][b])
	{
		pGeode = (vtGeode *) m_pRoads[a][b]->getChild(0);
	}
	else
	{
		m_pRoads[a][b] = new vtLOD;
		m_pGroup->addChild(m_pRoads[a][b]);

		FPoint3 lod_center;
		lod_center.x = m_extents.min.x + ((m_extent_range.x / ROAD_CLUSTER) * (a + 0.5f));
		lod_center.y = m_extents.min.y + (m_extent_range.y / 2.0f);
		lod_center.z = m_extents.min.z + ((m_extent_range.z / ROAD_CLUSTER) * (b + 0.5f));
		m_pRoads[a][b]->SetCenter(lod_center);

#if 0
		vtGeode *pSphere = CreateSphereGeom(m_pMats, m_mi_red, 1000.0f, 8);
		vtMovGeode *pSphere2 = new vtMovGeode(pSphere);
		m_pGroup->addChild(pSphere2);
		pSphere2->SetTrans(lod_center);
#endif

		pGeode = new vtGeode;
		pGeode->setName("road");
		pGeode->SetMaterials(m_pMats);

		// Visible from 0 to the desired distance
		m_pRoads[a][b]->addChild(pGeode, 0.0f, m_fLodDistance);

		clusters_used++;
	}
	pGeode->AddMesh(pMesh, iMatIdx);
}


vtTransform *vtRoadMap3d::GenerateGeometry(bool do_texture,
	bool bHwy, bool bPaved, bool bDirt, bool progress_callback(int))
{
	VTLOG("   vtRoadMap3d::GenerateGeometry\n");
	VTLOG("   Nodes %d, Links %d\n", NumNodes(), NumLinks());

	_CreateMaterials(do_texture);

	m_pGroup = new vtGroup;
	m_pGroup->setName("Roads");

	m_pTransform = new vtTransform;
	m_pTransform->addChild(m_pGroup);
	m_pTransform->SetTrans(FPoint3(0, m_fGroundOffset, 0));

	// We wrap the roads' geometry with an array of simple LOD nodes
	int a, b;
	for (a = 0; a < ROAD_CLUSTER; a++)
		for (b = 0; b < ROAD_CLUSTER; b++)
		{
			m_pRoads[a][b] = NULL;
		}

	_GatherExtents();

#if 0
	vtGeode *pGeode = CreateLineGridGeom(m_pMats, 0,
						   m_extents.min, m_extents.max, ROAD_CLUSTER);
	m_pGroup->addChild(pGeode);
#endif

	vtMesh *pMesh;
	int count = 0, total = NumLinks() + NumNodes();
	for (LinkGeom *pL = GetFirstLink(); pL; pL = pL->GetNext())
	{
		// Decide whether to construct this link
		bool include = false;
		if (bHwy && bPaved && bDirt)
			include = true;
		else
		{
			bool bIsDirt = (pL->m_Surface == SURFT_2TRACK || pL->m_Surface == SURFT_DIRT);
			if (bHwy && pL->m_iHwy != -1)
				include = true;
			if (bPaved && !bIsDirt)
				include = true;
			if (bDirt && bIsDirt)
				include = true;
		}
		if (include)
			pL->GenerateGeometry(this);
		count++;
		if (progress_callback != NULL)
			progress_callback(count * 100 / total);
	}
	count = 0;
	for (NodeGeom *pN = GetFirstNode(); pN; pN = pN->GetNext())
	{
		// What material to use?  We used to simply use "pavement", but that is
		// bad when they are e.g. trail or stone.  Now, we will try to guess
		// what to use by looking at the links here.
		int node_vti = VTI_PAVEMENT;
		for (int i = 0; i < pN->NumLinks(); i++)
		{
			LinkGeom *pL = pN->GetLink(i);
			switch (pL->m_vti)
			{
			case VTI_RAIL:
			case VTI_4WD:
			case VTI_TRAIL:
			case VTI_GRAVEL:
			case VTI_STONE:
				node_vti = pL->m_vti;
			}
		}
		VirtualTexture &vt = m_vt[node_vti];

		pMesh = pN->GenerateGeometry(vt);
		if (pMesh)
			AddMeshToGrid(pMesh, vt.m_idx);
		count++;
		if (progress_callback != NULL)
			progress_callback(count * 100 / total);
	}

	// return the top group, ready to be added to scene graph
	return m_pTransform;
}

int vtRoadMap3d::_CreateMaterial(const char *texture_filename, bool bTransparency)
{
	vtString fname = "GeoTypical/";
	fname += texture_filename;
	vtString path = FindFileOnPaths(vtGetDataPath(), fname);
	if (path == "")
		return -1;

	osg::Image *image = LoadOsgImage(path);
	if (!image)
		return -1;

	int material_index = m_pMats->AddTextureMaterial(image, true, true, bTransparency,
		false, ROAD_AMBIENT, ROAD_DIFFUSE, 1.0f, TERRAIN_EMISSIVE);

	if (material_index == -1)
		return -1;

	// All the road textures repeat, so don't clamp.
	vtMaterial *pMaterial = m_pMats->at(material_index);
	pMaterial->SetClamp(false);

	return material_index;
}

void vtRoadMap3d::_CreateMaterials(bool do_texture)
{
	m_pMats = new vtMaterialArray;

	// road textures
	if (do_texture)
	{
		// TODO: put these literals in a config file somewhere, if needed.
		m_mi_roadside =	_CreateMaterial("roadside_32.png", true);
		m_mi_pavement =	_CreateMaterial("pavement_256.jpg", false);
		m_mi_roads =	_CreateMaterial("roadset_2k.jpg", false);
		m_mi_4wd =		_CreateMaterial("road_4wd2.png", true);
		m_mi_trail =	_CreateMaterial("trail2.png", true);
		m_mi_gravel =	_CreateMaterial("gravel1.png", true);

		m_vt[VTI_MARGIN].m_idx = m_mi_roads;
		m_vt[VTI_MARGIN].m_rect.SetRect(960.0f/ROAD_REZ, 1, 992.0f/ROAD_REZ, 0);

		m_vt[VTI_SIDEWALK].m_idx = m_mi_roads;
		m_vt[VTI_SIDEWALK].m_rect.SetRect(512.0f/ROAD_REZ, 1, 640.0f/ROAD_REZ, 0);

		m_vt[VTI_1LANE].m_idx = m_mi_roads;
		m_vt[VTI_1LANE].m_rect.SetRect(451.0f/ROAD_REZ, 1, 511.0f/ROAD_REZ, 0);

		m_vt[VTI_2LANE1WAY].m_idx = m_mi_roads;
		m_vt[VTI_2LANE1WAY].m_rect.SetRect(4.0f/ROAD_REZ, 1, 124.0f/ROAD_REZ, 0);

		m_vt[VTI_2LANE2WAY].m_idx = m_mi_roads;
		m_vt[VTI_2LANE2WAY].m_rect.SetRect(640.0f/ROAD_REZ, 1, 768.0f/ROAD_REZ, 0);

		m_vt[VTI_3LANE1WAY].m_idx = m_mi_roads;
		m_vt[VTI_3LANE1WAY].m_rect.SetRect(2.0f/ROAD_REZ, 1, 190.0f/ROAD_REZ, 0);

		m_vt[VTI_3LANE2WAY].m_idx = m_mi_roads;
		m_vt[VTI_3LANE2WAY].m_rect.SetRect(768.0f/ROAD_REZ, 1, .0f/ROAD_REZ, 0);

		m_vt[VTI_4LANE1WAY].m_idx = m_mi_roads;
		m_vt[VTI_4LANE1WAY].m_rect.SetRect(0.0f/ROAD_REZ, 1, 256.0f/ROAD_REZ, 0);

		m_vt[VTI_4LANE2WAY].m_idx = m_mi_roads;
		m_vt[VTI_4LANE2WAY].m_rect.SetRect(256.0f/ROAD_REZ, 1, 512.0f/ROAD_REZ, 0);

		m_vt[VTI_RAIL].m_idx = m_mi_roads;
		m_vt[VTI_RAIL].m_rect.SetRect(992.0f/ROAD_REZ, 1, 1248.0f/ROAD_REZ, 0);

		m_vt[VTI_STONE].m_idx = m_mi_roads;
		m_vt[VTI_STONE].m_rect.SetRect(1248.0f/ROAD_REZ, 1, 1440.0f/ROAD_REZ, 0);

		m_vt[VTI_4WD].m_idx = m_mi_4wd;
		m_vt[VTI_4WD].m_rect.SetRect(0, 0, 1, 1);

		m_vt[VTI_TRAIL].m_idx = m_mi_trail;
		m_vt[VTI_TRAIL].m_rect.SetRect(0, 0, 1, 1);

		m_vt[VTI_GRAVEL].m_idx = m_mi_gravel;
		m_vt[VTI_GRAVEL].m_rect.SetRect(0, 0, 1, 1);

		m_vt[VTI_PAVEMENT].m_idx = m_mi_pavement;
		m_vt[VTI_PAVEMENT].m_rect.SetRect(0, 0, 1, 1);
	}
	else
	{
		m_mi_roadside = m_pMats->AddRGBMaterial(RGBf(0.8f, 0.6f, 0.4f), true, false);	// 0 brown roadside
		m_mi_roads = m_pMats->AddRGBMaterial(RGBf(0.0f, 1.0f, 0.0f), true, false);	// 1 green
		m_mi_4wd = m_pMats->AddRGBMaterial(RGBf(0.5f, 0.5f, 0.5f), true, false);	// 2 grey
		m_mi_trail = m_pMats->AddRGBMaterial(RGBf(1.0f, 0.3f, 1.0f), true, false);	// 3 light purple
	}
	m_mi_red = m_pMats->AddRGBMaterial(RGBf(1.0f, 0.0f, 0.0f),
		true, true, false, 0.4f);	// red-translucent
}

//
// stoplights and stopsigns
//
void vtRoadMap3d::GenerateSigns(vtLodGrid *pLodGrid)
{
	if (!pLodGrid)
		return;

	vtContentManager3d &con = vtGetContent();
	osg::Node *stopsign = con.CreateNodeFromItemname("American Stopsign");
	osg::Node *stoplight = con.CreateNodeFromItemname("Stoplight (right)");

	if (!stopsign || !stoplight)
	{
		VTLOG("Couldn't find stopsign and stoplight.\n");
		return;
	}
	for (NodeGeom *pN = GetFirstNode(); pN; pN = pN->GetNext())
	{
		for (int r = 0; r < pN->NumLinks(); r++)
		{
			osg::Node *shape = NULL;
			if (pN->GetIntersectType(r) == IT_STOPSIGN && stopsign)
			{
				shape = (osg::Node *) stopsign->clone(osg::CopyOp::SHALLOW_COPY);
			}
			if (pN->GetIntersectType(r) == IT_LIGHT && stoplight)
			{
				shape = (osg::Node *) stoplight->clone(osg::CopyOp::SHALLOW_COPY);
			}
			if (!shape) continue;

			vtTransform *trans = new vtTransform;
			trans->addChild(shape);

			LinkGeom *link = pN->GetLink(r);
			FPoint3 unit = pN->GetUnitLinkVector(r);
			FPoint3 perp(unit.z, unit.y, -unit.x);
			FPoint3 offset;

			// Turn the sign (yaw) to face the oncoming traffic
			trans->RotateLocal(FPoint3(0,1,0), pN->GetLinkAngle(r) + PID2f);

			if (pN->GetIntersectType(r) == IT_STOPSIGN)
			{
				offset = pN->m_p3 + (unit * 6.0f) + (perp * (link->m_fRightWidth));
			}
			if (pN->GetIntersectType(r) == IT_LIGHT)
			{
				offset = pN->m_p3 - (unit * 6.0f) + (perp * (link->m_fRightWidth));
			}
			trans->Translate(offset);
			pLodGrid->AddToGrid(trans);
		}
	}
}

void vtRoadMap3d::SetHeightOffGround(float fHeight)
{
	m_fGroundOffset = fHeight;
	if (m_pTransform)
		m_pTransform->SetTrans(FPoint3(0, m_fGroundOffset, 0));
}

void vtRoadMap3d::_GatherExtents()
{
	// Find extents of area covered by links
	m_extents.InsideOut();

	// Examine the range of the roadmap area
	for (LinkGeom *pL = GetFirstLink(); pL; pL = pL->GetNext())
		m_extents.GrowToContainLine(pL->m_centerline);

	// Expand slightly for safety - in case we allow dragging link nodes
	//  interactively in the future.
	FPoint3 diff = m_extents.max - m_extents.min;
	m_extents.min -= (diff / 20.0f);
	m_extents.max += (diff / 20.0f);
	m_extent_range = m_extents.max - m_extents.min;
}


void vtRoadMap3d::DetermineSurfaceAppearance()
{
	// Pre-process some link attributes
	for (LinkGeom *pL = GetFirstLink(); pL; pL = pL->GetNext())
	{
		// set material index based on surface type, number of lanes, and direction
		bool two_way = (pL->m_iFlags & RF_FORWARD) &&
					   (pL->m_iFlags & RF_REVERSE);
		switch (pL->m_Surface)
		{
		case SURFT_NONE:
//			pL->m_vti = 3;
			pL->m_vti = 0;
			break;
		case SURFT_GRAVEL:
			pL->m_vti = VTI_GRAVEL;
			break;
		case SURFT_TRAIL:
			pL->m_vti = VTI_TRAIL;
			break;
		case SURFT_2TRACK:
		case SURFT_DIRT:
			pL->m_vti = VTI_4WD;
			break;
		case SURFT_PAVED:
			switch (pL->m_iLanes)
			{
			case 1:
				pL->m_vti = VTI_1LANE;
				break;
			case 2:
				pL->m_vti = two_way ? VTI_2LANE2WAY : VTI_2LANE1WAY;
				break;
			case 3:
				pL->m_vti = two_way ? VTI_3LANE2WAY : VTI_3LANE1WAY;
				break;
			case 4:
			case 5:
			case 6:
				pL->m_vti = two_way ? VTI_4LANE2WAY : VTI_4LANE1WAY;
				break;
			default:
				// Catch any freakish roads with >6 lanes.
				pL->m_vti = two_way ? VTI_4LANE2WAY : VTI_4LANE1WAY;
				break;
			}
			break;
		case SURFT_RAILROAD:
			pL->m_vti = VTI_RAIL;
			break;
		case SURFT_STONE:
			pL->m_vti = VTI_STONE;
			break;
		default:
			pL->m_vti = 0;
			break;
		}
	}
}

void vtRoadMap3d::SetLodDistance(float fDistance)
{
	m_fLodDistance = fDistance;

	if (m_pGroup)
	{
		for (int a = 0; a < ROAD_CLUSTER; a++)
			for (int b = 0; b < ROAD_CLUSTER; b++)
			{
				if (m_pRoads[a][b])
					m_pRoads[a][b]->setRange(0, 0.0f, m_fLodDistance);
			}
	}
}

float vtRoadMap3d::GetLodDistance()
{
	return m_fLodDistance;
}

void vtRoadMap3d::DrapeOnTerrain(vtHeightField3d *pHeightField)
{
	FPoint3 p;
	NodeGeom *pN;

#if 0
	// This code attempts to identify cases where a node actually
	// represents something like an overpass: two links that don't
	// actually connect.  However, it's better to take care of this
	// as a preprocess, rather than at runtime.
	float height;
	for (pN = GetFirstNode(); pN; pN = (NodeGeom *)pN->m_pNext)
	{
		bool all_same_height = true;
		height = pN->GetLink(0)->GetHeightAt(pN);
		for (int r = 1; r < pN->NumLinks(); r++)
		{
			if (pN->GetLink(r)->GetHeightAt(pN) != height)
			{
				all_same_height = false;
				break;
			}
		}
		if (!all_same_height)
		{
			pNew = new NodeGeom;
			for (r = 1; r < pN->NumLinks(); r++)
			{
				LinkGeom *pL = pN->GetLink(r);
				if (pL->GetHeightAt(pN) != height)
				{
					pN->DetachRoad(pL);
					pNew->AddRoad(pL);
				}
			}
		}
	}
#endif
	for (pN = GetFirstNode(); pN; pN = pN->GetNext())
	{
		pHeightField->ConvertEarthToSurfacePoint(pN->Pos(), pN->m_p3);
#if 0
		if (pN->NumLinks() > 0)
		{
			height = pN->GetLink(0)->GetHeightAt(pN);
			pN->m_p3.y += height;
		}
#endif
	}
	for (LinkGeom *pL = GetFirstLink(); pL; pL = pL->GetNext())
	{
		pL->m_centerline.SetSize(pL->GetSize());
		for (uint j = 0; j < pL->GetSize(); j++)
		{
			pHeightField->ConvertEarthToSurfacePoint(pL->GetAt(j), p);
			pL->m_centerline[j] = p;
		}
		// ignore width from file - imply from properties
		pL->EstimateWidth();
	}
}

