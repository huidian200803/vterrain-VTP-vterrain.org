//
// Building3d.cpp
//
// The vtBuilding3d class extends vtBuilding with the ability to procedurally
// create 3D geometry of the buildings.
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/vtosg/GeometryUtils.h"
#include "vtdata/DataPath.h"
#include "vtdata/HeightField.h"
#include "vtdata/PolyChecker.h"
#include "vtdata/Triangulate.h"

#include "Light.h"
#include "Terrain.h"
#include "Building3d.h"
#include "FelkelStraightSkeleton.h"


/////////////////////////////////////////////////////////////////////////////

vtBuilding3d::vtBuilding3d() : vtBuilding()
{
	m_pContainer = NULL;
	m_pGeode = NULL;
	m_pHighlight = NULL;
}

vtBuilding3d::~vtBuilding3d()
{
	// meshes will be automatically deleted by the geometry they're in
}

vtBuilding3d &vtBuilding3d::operator=(const vtBuilding &v)
{
	// just call the copy method of the parent class
	*((vtBuilding*)this) = v;
	return *this;
}


//
// Convert the building's reference point into world coordinates.
//
void vtBuilding3d::UpdateWorldLocation(vtHeightField3d *pHeightField)
{
	// Embed the building in the ground such that the lowest corner of its
	// lowest level is at ground level.
	float base_level = CalculateBaseElevation(pHeightField);

	// Find the center of the building in world coordinates (the origin of
	// the building's local coordinate system)
	DPoint2 center;
	GetBaseLevelCenter(center);
	pHeightField->ConvertEarthToSurfacePoint(center, m_center);
	m_center.y = base_level;
}

float vtBuilding3d::GetHeightOfStories()
{
	float height = 0.0f;

	int levs = m_Levels.GetSize();
	for (int i = 0; i < levs; i++)
		height += m_Levels[i]->m_iStories * m_Levels[i]->m_fStoryHeight;

	return height;
}


void vtBuilding3d::DestroyGeometry()
{
	if (!m_pGeode)	// safety check
		return;

	m_pContainer->removeChild(m_pGeode);
	m_pGeode = NULL;
	m_Mesh.clear();
}

void vtBuilding3d::AdjustHeight(vtHeightField3d *pHeightField)
{
	UpdateWorldLocation(pHeightField);
	m_pContainer->SetTrans(m_center);
}

void vtBuilding3d::CreateUpperPolygon(const vtLevel *lev, FPolygon3 &polygon,
									  FPolygon3 &polygon2)
{
	int i, prev, next;
	int rings = polygon.size();

	polygon2 = polygon;

	int base_edge = 0;
	for (int ring = 0; ring < rings; ring++)
	{
		FLine3 &poly = polygon[ring];
		FLine3 &poly2 = polygon2[ring];

		int edges = poly.GetSize();

		for (i = 0; i < edges; i++)
		{
			prev = (i-1 < 0) ? edges-1 : i-1;
			next = (i+1 == edges) ? 0 : i+1;

			FPoint3 p = poly[i];

			int islope1 = lev->GetEdge(base_edge + prev)->m_iSlope;
			int islope2 = lev->GetEdge(base_edge + i)->m_iSlope;
			if (islope1 == 90 && islope2 == 90)
			{
				// easy case
				p.y += lev->m_fStoryHeight;
			}
			else
			{
				float slope1 = (islope1 / 180.0f * PIf);
				float slope2 = (islope2 / 180.0f * PIf);

				// get edge vectors
				FPoint3 vec1 = poly[prev] - poly[i];
				FPoint3 vec2 = poly[next] - poly[i];
				vec1.Normalize();
				vec2.Normalize();

				// get perpendicular (upward pointing) vectors
				FPoint3 perp1, perp2;
				perp1.Set(0, 1, 0);
				perp2.Set(0, 1, 0);

				// create rotation matrices to rotate them upward
				FMatrix4 mat1, mat2;
				mat1.Identity();
				mat1.AxisAngle(vec1, -slope1);
				mat2.Identity();
				mat2.AxisAngle(vec2, slope2);

				// create normals
				FPoint3 norm1, norm2;
				mat1.TransformVector(perp1, norm1);
				mat2.TransformVector(perp2, norm2);

				// vector of plane intersection is cross product of their normals
				FPoint3 inter = norm1.Cross(norm2);
				// Test that intersection vector is pointing into the polygon
				// need a better test if we are going to handle downward sloping roofs
				if (inter.y < 0)
					inter = -inter;	// Reverse vector to point upward

				inter.Normalize();
				inter *= (lev->m_fStoryHeight / inter.y);

				p += inter;
			}
			poly2[i] = p;
		}
		base_edge += edges;
	}
}

bool vtBuilding3d::CreateGeometry(vtHeightField3d *pHeightField)
{
#if VTP_USE_EXPERIMENTAL_BUILDING_GEOMETRY_GENERATOR
	UpdateWorldLocation(pHeightField);

	osg::ref_ptr<OSGGeomUtils::GeometryBuilder> pGenerator =
		new OSGGeomUtils::GeometryBuilder(*this);
	m_pGeode = pGenerator->Generate();
#else

	UpdateWorldLocation(pHeightField);

	// TEMP: we can handle complex polys now - i think
	// PolyChecker PolyChecker;
	// if (!PolyChecker.IsSimplePolygon(GetLocalFootprint(0)))
	//	 return false;

	// create the edges (walls and roof)
	float fHeight = 0.0f;
	const int iLevels = NumLevels();

	int level_show = -1, edge_show = -1;
	GetValueInt("level", level_show);
	GetValueInt("edge", edge_show);

	for (int i = 0; i < iLevels; i++)
	{
		const vtLevel *lev = m_Levels[i];
		const FPolygon3 &foot = GetLocalFootprint(i);
		const uint edges = lev->NumEdges();

		// safety check
		if (foot[0].GetSize() < 3)
			return false;

		if (lev->IsHorizontal())
		{
			// make flat roof
			AddFlatRoof(foot, lev);
		}
		else if (lev->IsUniform())
		{
			int iHighlightEdge = level_show == i ? edge_show : -1;
			CreateUniformLevel(i, fHeight, iHighlightEdge);
			fHeight += lev->m_iStories * lev->m_fStoryHeight;
		}
		else if (lev->HasSlopedEdges() && edges > 4)
		{
			// For complicated roofs with sloped edges which meet at a
			// roofline of uneven height, we need a sophisticated
			// straight-skeleton solution like Petr Felkel's
			float fRoofHeight = MakeFelkelRoof(foot, lev);
			if (fRoofHeight < 0.0)
			{
				VTLOG("Failed to make Felkel roof - reverting to flat roof\n");
				AddFlatRoof(foot, lev);
			}
			fHeight += fRoofHeight;
		}
		else
		{
			// Build a 'flat roof' for the floor
			AddFlatRoof(foot, lev);

			FPolygon3 poly = foot;
			FPolygon3 poly2;

			// Build a set of walls for each storey of the level
			for (uint j = 0; j < lev->m_iStories; j++)
			{
				for (uint r = 0; r < poly.size(); r++)
				{
					for (uint k = 0; k < poly[r].GetSize(); k++)
					{
						poly[r][k].y = fHeight;
					}
				}
				CreateUpperPolygon(lev, poly, poly2);
				int edge_start = 0;
				for (uint r = 0; r < poly.size(); r++)
				{
					for (uint k = edge_start; k < edge_start + poly[r].GetSize(); k++)
					{
						const bool bShowEdge = (level_show == i && edge_show == k);
						CreateEdgeGeometry(lev, poly, poly2, k, bShowEdge);
					}
					edge_start += poly[r].GetSize();
				}
				fHeight += lev->m_fStoryHeight;
			}
		}
	}

#if 0 // testing
	const FLine3 &roof = GetLocalFootprint(iLevels-1);	// roof: top level
	vtLevel *roof_lev = m_Levels[iLevels-1];
	float roof_height = (roof_lev->m_fStoryHeight * roof_lev->m_iStories);
#endif

	// wrap in a shape and set materials
	m_pGeode = new vtGeode;
	m_pGeode->setName("building-geom");
	vtMaterialArray *pShared = GetSharedMaterialArray();
	m_pGeode->SetMaterials(pShared);

	for (uint j = 0; j < m_Mesh.size(); j++)
	{
		vtMesh *mesh = m_Mesh[j].m_pMesh;
		const int index = m_Mesh[j].m_iMatIdx;
		m_pGeode->AddMesh(mesh, index);
	}
#endif

	// resize bounding box
	if (m_pHighlight)
	{
		const bool bEnabled = m_pHighlight->GetEnabled();

		m_pContainer->removeChild(m_pHighlight);
		m_pHighlight = NULL;
		ShowBounds(bEnabled);
	}
	return true;
}

vtGeode *vtBuilding3d::CreateHighlight()
{
	vtGeode *geode = new vtGeode;
	vtMaterialArray *pShared = GetSharedMaterialArray();
	geode->SetMaterials(pShared);

	vtMesh *mesh = new vtMesh(osg::PrimitiveSet::LINE_STRIP, 0, 40);
	const int mat_idx = GetMatIndex("Highlight", RGBf(1,1,0));

	geode->AddMesh(mesh, mat_idx);
	mesh->SetLineWidth(2.0f);

	unsigned short idx[MAX_WALLS];
	for (uint i = 0; i < NumLevels(); i++)
	{
		const FPolygon3 &foot = GetLocalFootprint(i);

		// safety check
		if (foot[0].GetSize() < 3)
			continue;

		const int outer_corners = foot[0].GetSize();

		for (int j = 0; j <= outer_corners; j++)
		{
			FPoint3 p = foot[0][j % outer_corners];
			p *= 1.01f;		// Slightly larger to not intersect the building
			idx[j] = mesh->AddVertex(p);
		}
		mesh->AddStrip(outer_corners + 1, idx);
	}
	// Center of top level
	const FLine3 &top_foot = GetLocalFootprint(NumLevels()-1).at(0);
	FPoint3 top_center(0, 0, 0);
	for (uint i = 0; i < top_foot.GetSize(); i++)
		top_center += top_foot[i];
	top_center /= top_foot.GetSize();
	top_center.y += 10.0;
	int top_idx = mesh->AddVertex(top_center);

	const FPolygon3 &foot0 = GetLocalFootprint(0);
	for (uint j = 0; j < foot0[0].GetSize(); j++)
	{
		int count = 0;
		for (uint i = 0; i < NumLevels(); i++)
		{
			const FLine3 &foot = GetLocalFootprint(i).at(0);
			if (j >= foot.GetSize())
				continue;		// Safety check
			FPoint3 p = foot[j];
			p *= 1.01f;
			idx[count++] = mesh->AddVertex(p);
		}
		idx[count++] = top_idx;
		mesh->AddStrip(count, idx);
	}

	return geode;
}

////////////////////////////////////////////////////////////////////////////

//
// Since each set of primitives with a specific material requires its own
// mesh, this method looks up or creates a mesh as needed, per color/material.
//
vtMesh *vtBuilding3d::FindMatMesh(const vtString &Material,
								  const RGBi &color, vtMesh::PrimType ePrimType)
{
	int mi;
	int VertType;
	RGBf fcolor = color;

	// wireframe is a special case, used for highlight materials
	if (ePrimType == osg::PrimitiveSet::LINE_STRIP)
	{
		mi = GetMatIndex(BMAT_NAME_HIGHLIGHT, fcolor);
		VertType = 0;
	}
	else
	{
		// otherwise, find normal stored material
		if (&Material == NULL)
			mi = GetMatIndex(BMAT_NAME_PLAIN, fcolor);
		else
			mi = GetMatIndex(Material, fcolor);
		VertType = VT_Normals | VT_TexCoords;
	}

	int i, size = m_Mesh.size();
	for (i = 0; i < size; i++)
	{
		if (m_Mesh[i].m_iMatIdx == mi && m_Mesh[i].m_ePrimType == ePrimType)
			return m_Mesh[i].m_pMesh;
	}
	// didn't find it, so we need to make it
	MatMesh mm;
	mm.m_iMatIdx = mi;
	mm.m_ePrimType = ePrimType;

	// Potential Optimization: should calculate how many vertices the building
	// will take.  Even the simplest building will use 20 vertices, for now
	// just use 40 as a reasonable starting point for each mesh.

	mm.m_pMesh = new vtMesh(ePrimType, VertType, 40);
	m_Mesh.push_back(mm);
	return mm.m_pMesh;
}

//
// Edges are created from a series of features ("panels", "sections")
//
void vtBuilding3d::CreateEdgeGeometry(const vtLevel *pLev, const FPolygon3 &polygon1,
									  const FPolygon3 &polygon2, int iEdge, bool bShowEdge)
{
	// Get edge from complete list
	vtEdge	*pEdge = pLev->GetEdge(iEdge);

	// Then determine which ring its on
	int ring = polygon1.WhichRing(iEdge);
	const FLine3 &poly1 = polygon1[ring];
	const FLine3 &poly2 = polygon2[ring];

	// number of edges in this ring
	int num_edges = poly1.GetSize();
	int i = iEdge, j = (i+1)%num_edges;

	FLine3 quad(4);

	// start with the whole wall section
	quad[0] = poly1[i];
	quad[1] = poly1[j];
	quad[2] = poly2[i];
	quad[3] = poly2[j];

	// length of the edge
	FPoint3 dir1 = quad[1] - quad[0];
	FPoint3 dir2 = quad[3] - quad[2];
	float total_length1 = dir1.Length();
	float total_length2 = dir2.Length();
	if (total_length1 > 0.0f)
		dir1.Normalize();
	if (total_length2 > 0.0f)
		dir2.Normalize();

	if (bShowEdge)
		AddHighlightSection(pEdge, quad);

	// How wide should each feature be?
	// Determine how much space we have for the proportional features after
	// accounting for the fixed-width features
	const float fixed_width = pEdge->FixedFeaturesWidth();
	const float total_prop = pEdge->ProportionTotal();
	const float dyn_width = total_length1 - fixed_width;

	if (pEdge->m_Facade != "")
	{
		// If we can successfully construct the facade, we don't need to
		//  use the edge features.
		if (MakeFacade(pEdge, quad, 1))
			return;
	}

	// build the edge features.
	// point[0] is the first starting point of a panel.
	for (i = 0; i < (int)pEdge->NumFeatures(); i++)
	{
		vtEdgeFeature &feat = pEdge->m_Features[i];

		// determine real width
		float meter_width = 0.0f;
		if (feat.m_width >= 0)
			meter_width = feat.m_width;
		else
			meter_width = (feat.m_width / total_prop) * dyn_width;
		quad[1] = quad[0] + dir1 * meter_width;
		quad[3] = quad[2] + dir2 * (meter_width * total_length2 / total_length1);

		if (feat.m_code == WFC_WALL)
		{
			AddWallNormal(pEdge, &feat, quad);
		}
		if (feat.m_code == WFC_GAP)
		{
			// do nothing
		}
		if (feat.m_code == WFC_POST)
		{
			// TODO
		}
		if (feat.m_code == WFC_WINDOW)
		{
			AddWindowSection(pEdge, &feat, quad);
		}
		if (feat.m_code == WFC_DOOR)
		{
			AddDoorSection(pEdge, &feat, quad);
		}
		quad[0] = quad[1];
		quad[2] = quad[3];
	}
}

/**
 * Creates geometry for a highlighted area (an edge).
 */
void vtBuilding3d::AddHighlightSection(vtEdge *pEdge,
	const FLine3 &quad)
{
	// determine 4 points at corners of wall section
	FPoint3 p0 = quad[0];
	FPoint3 p1 = quad[1];
	FPoint3 p3 = quad[2];
	FPoint3 p2 = quad[3];

	vtMesh *mesh = FindMatMesh(BMAT_NAME_PLAIN, RGBi(255,255,255), osg::PrimitiveSet::LINE_STRIP);

	// determine normal (not used for shading)
	FPoint3 norm = Normal(p0,p1,p2);

	int start =
		mesh->AddVertex(p0 + norm);
	mesh->AddVertex(p1 + norm);
	mesh->AddVertex(p2 + norm);
	mesh->AddVertex(p3 + norm);
	mesh->AddVertex(p0 + norm);
	mesh->AddFan(start, start+1, start+2, start+3, start+4);

	start = mesh->AddVertex(p0);
	mesh->AddVertex(p0 + norm);
	mesh->AddFan(start, start+1);

	start = mesh->AddVertex(p1);
	mesh->AddVertex(p1 + norm);
	mesh->AddFan(start, start+1);

	start = mesh->AddVertex(p2);
	mesh->AddVertex(p2 + norm);
	mesh->AddFan(start, start+1);

	start = mesh->AddVertex(p3);
	mesh->AddVertex(p3 + norm);
	mesh->AddFan(start, start+1);

	norm *= 0.95f;
	mesh = FindMatMesh(BMAT_NAME_PLAIN, RGBi(255,0,0), osg::PrimitiveSet::LINE_STRIP);
	start =
		mesh->AddVertex(p0 + norm);
	mesh->AddVertex(p1 + norm);
	mesh->AddVertex(p2 + norm);
	mesh->AddVertex(p3 + norm);
	mesh->AddVertex(p0 + norm);
	mesh->AddFan(start, start+1, start+2, start+3, start+4);
}

/**
 * Builds a wall, given material index, starting and end points, height, and
 * starting height.
 */
void vtBuilding3d::AddWallSection(vtEdge *pEdge, bool bUniform,
	const FLine3 &quad, float vf1, float vf2, float hf1)
{
	// determine 4 points at corners of wall section
	FPoint3 up1 = (quad[2] - quad[0]);
	FPoint3 up2 = (quad[3] - quad[1]);
	FPoint3 p0 = quad[0] + (up1 * vf1);
	FPoint3 p1 = quad[1] + (up2 * vf1);
	FPoint3 p3 = quad[0] + (up1 * vf2);
	FPoint3 p2 = quad[1] + (up2 * vf2);

	vtMesh *mesh;
	if (bUniform)
		mesh = FindMatMesh(BMAT_NAME_WINDOWWALL, pEdge->m_Color, osg::PrimitiveSet::TRIANGLE_FAN);
	else
		mesh = FindMatMesh(*pEdge->m_pMaterial, pEdge->m_Color, osg::PrimitiveSet::TRIANGLE_FAN);

	// determine normal and primary axes of the face
	FPoint3 norm = Normal(p0, p1, p2);
	FPoint3 axis0, axis1;
	axis0 = p1 - p0;
	axis0.Normalize();
	axis1 = norm.Cross(axis0);

	// determine UVs - special case for window-wall texture
	FPoint2 uv0, uv1, uv2, uv3;
	if (bUniform)
	{
		uv0.Set(0, 0);
		uv1.Set(hf1, 0);
		uv2.Set(hf1, vf2);
		uv3.Set(0, vf2);
	}
	else
	{
		float u1 = (p1 - p0).Dot(axis0);
		float u2 = (p2 - p0).Dot(axis0);
		float u3 = (p3 - p0).Dot(axis0);
		float v2 = (p2 - p0).Dot(axis1);
		vtMaterialDescriptor *md = GetMatDescriptor(*pEdge->m_pMaterial, pEdge->m_Color);
		uv0.Set(0, 0);
		uv1.Set(u1, 0);
		uv2.Set(u2, v2);
		uv3.Set(u3, v2);
		if (md != NULL)
		{
			// divide meters by [meters/uv] to get uv
			FPoint2 UVScale = md->GetUVScale();
			uv0.Div(UVScale);
			uv1.Div(UVScale);
			uv2.Div(UVScale);
			uv3.Div(UVScale);
		}
	}

	int start =
		mesh->AddVertexNUV(p0, norm, uv0);
	mesh->AddVertexNUV(p1, norm, uv1);
	mesh->AddVertexNUV(p2, norm, uv2);
	mesh->AddVertexNUV(p3, norm, uv3);

	mesh->AddFan(start, start+1, start+2, start+3);
}

void vtBuilding3d::AddWallNormal(vtEdge *pEdge, vtEdgeFeature *pFeat,
	const FLine3 &quad)
{
	float vf1 = pFeat->m_vf1;
	float vf2 = pFeat->m_vf2;
	AddWallSection(pEdge, false, quad, vf1, vf2);
}

/**
 * Builds a door section.  will also build the wall above the door to ceiling
 * height.
 */
void vtBuilding3d::AddDoorSection(vtEdge *pEdge, vtEdgeFeature *pFeat,
	const FLine3 &quad)
{
	float vf1 = 0;
	float vf2 = pFeat->m_vf2;

	// determine 4 points at corners of section
	FPoint3 up1 = (quad[2] - quad[0]);
	FPoint3 up2 = (quad[3] - quad[1]);
	FPoint3 p0 = quad[0] + (up1 * vf1);
	FPoint3 p1 = quad[1] + (up2 * vf1);
	FPoint3 p3 = quad[0] + (up1 * vf2);
	FPoint3 p2 = quad[1] + (up2 * vf2);

	vtMesh *mesh = FindMatMesh(BMAT_NAME_DOOR, pEdge->m_Color, osg::PrimitiveSet::TRIANGLE_FAN);

	// determine normal (flat shading, all vertices have the same normal)
	FPoint3 norm = Normal(p0, p1, p2);

	int start =
		mesh->AddVertexNUV(p0, norm, FPoint2(0.0f, 0.0f));
	mesh->AddVertexNUV(p1, norm, FPoint2(1.0f, 0.0f));
	mesh->AddVertexNUV(p2, norm, FPoint2(1.0f, 1.0f));
	mesh->AddVertexNUV(p3, norm, FPoint2(0.0f, 1.0f));

	mesh->AddFan(start, start+1, start+2, start+3);

	//add wall above door
	AddWallSection(pEdge, false, quad, vf2, 1.0f);
}

//builds a window section.  builds the wall below and above a window too.
void vtBuilding3d::AddWindowSection(vtEdge *pEdge, vtEdgeFeature *pFeat,
	const FLine3 &quad)
{
	const float vf1 = pFeat->m_vf1;
	const float vf2 = pFeat->m_vf2;

	// build wall to base of window.
	AddWallSection(pEdge, false, quad, 0, vf1);

	// build wall above window
	AddWallSection(pEdge, false, quad, vf2, 1.0f);

	// determine 4 points at corners of section
	const FPoint3 up1 = (quad[2] - quad[0]);
	const FPoint3 up2 = (quad[3] - quad[1]);
	const FPoint3 p0 = quad[0] + (up1 * vf1);
	const FPoint3 p1 = quad[1] + (up2 * vf1);
	const FPoint3 p3 = quad[0] + (up1 * vf2);
	const FPoint3 p2 = quad[1] + (up2 * vf2);

	vtMesh *mesh = FindMatMesh(BMAT_NAME_WINDOW, pEdge->m_Color, osg::PrimitiveSet::TRIANGLE_FAN);

	// determine normal (flat shading, all vertices have the same normal)
	const FPoint3 norm = Normal(p0,p1,p2);

	const int start = mesh->AddVertexNUV(p0, norm, FPoint2(0.0f, 0.0f));
	mesh->AddVertexNUV(p1, norm, FPoint2(1.0f, 0.0f));
	mesh->AddVertexNUV(p2, norm, FPoint2(1.0f, 1.0f));
	mesh->AddVertexNUV(p3, norm, FPoint2(0.0f, 1.0f));

	mesh->AddFan(start, start+1, start+2, start+3);
}

void vtBuilding3d::AddFlatRoof(const FPolygon3 &pp, const vtLevel *pLev)
{
	const FPoint3 up(0.0f, 1.0f, 0.0f);	// vector pointing up
	const int rings = pp.size();
	const int outer_corners = pp[0].GetSize();
	FPoint2 uv;

	const vtEdge *pEdge = pLev->GetEdge(0);
	const vtString& Material = *pEdge->m_pMaterial;
	vtMesh *mesh = FindMatMesh(Material, pEdge->m_Color, osg::PrimitiveSet::TRIANGLES);
	const vtMaterialDescriptor *md = GetMatDescriptor(Material, pEdge->m_Color);

	if (outer_corners > 4 || rings > 1)
	{
		// roof consists of a polygon which must be split into triangles
		//  Invoke the triangulator to triangulate this polygon.
#if 1
		// Use 'Triangle'
		const FLine3 &outer = pp[0];
		float roof_y = outer[0].y;
		DPolygon2 foot2d;
		ProjectionXZ(pp, foot2d);

		// Triangle has been known to behave poorly with redundant vertices
		//  We are in meters now, so we can use a centimeter epsilon.
		int removed = foot2d.RemoveDegeneratePoints(0.08);
		if (removed)
			VTLOG("Skipped %d redundant vertices.\n", removed);

		// a polyline to hold the answer in sets of three points
		DLine2 result2d;
		CallTriangle(foot2d, result2d);

		FLine3 result;
		ProjectionXZ(result2d, roof_y, result);
#else
		// Use older, simpler triangulator
		FLine3 outer = pp[0];
		FLine2 roof;
		roof.SetMaxSize(outer_corners);
		for (i = 0; i < outer_corners; i++)
			roof.Append(FPoint2(outer[i].x, outer[i].z));
		float roof_y = outer[0].y;
		FLine2 result;
		Triangulate_f::Process(roof, result);
#endif

		// use the results.
		int tcount = result.GetSize()/3;
		int ind[3];
		FPoint2 gp;
		FPoint3 p;

		for (int i=0; i<tcount; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				p = result[i*3+j];
				uv.Set(p.x, p.z);
				if (md)
					uv.Div(md->GetUVScale());	// divide meters by [meters/uv] to get uv
				ind[j] = mesh->AddVertexNUV(p, up, uv);
			}
			mesh->AddTri(ind[0], ind[1], ind[2]);
		}
	}
	else
	{
		int idx[MAX_WALLS];
		for (int i = 0; i < outer_corners; i++)
		{
			FPoint3 p = pp[0][i];
			uv.Set(p.x, p.z);
			if (md)
				uv.Div(md->GetUVScale());	// divide meters by [meters/uv] to get uv
			idx[i] = mesh->AddVertexNUV(p, up, uv);
		}
		if (outer_corners > 2)
			mesh->AddTri(idx[0], idx[1], idx[2]);
		if (outer_corners > 3)
			mesh->AddTri(idx[2], idx[3], idx[0]);
	}
}


float vtBuilding3d::MakeFelkelRoof(const FPolygon3 &EavePolygons, const vtLevel *pLev)
{
	vtStraightSkeleton StraightSkeleton;
	CSkeleton Skeleton;
	float fMaxHeight = 0.0;
	ContourVector RoofEaves(EavePolygons.size());
	int i;
	CSkeletonLine *pStartEdge;
	CSkeletonLine *pEdge;
	CSkeletonLine *pNextEdge;
	bool bEdgeReversed;
	float EaveY = EavePolygons[0][0].y;
#ifdef FELKELDEBUG
	float DebugX;
	float DebugY;
	float DebugZ;
#endif

	// Make a roof using felkels straight skeleton algorithm

	// First of all build the eave footprint.
	ContourVector::iterator itV = RoofEaves.begin();
	for (FPolygon3::const_iterator itP = EavePolygons.begin(); itP != EavePolygons.end(); itP++, itV++)
	{
		int iVertices = (*itP).GetSize();
		for (i = 0; i < iVertices; i++)
		{
			FPoint3 CurrentPoint = (*itP)[i];
			FPoint3 NextPoint = (*itP)[(i+1)%iVertices];
			FPoint3 PreviousPoint = (*itP)[(iVertices+i-1)%iVertices];
			int iSlope = pLev->GetEdge(i)->m_iSlope;
			if (iSlope > 89)
				iSlope = 90;
			else if (iSlope < 1)
				iSlope = 0;
			int iPrevSlope = pLev->GetEdge((iVertices+i-1)%iVertices)->m_iSlope;
			if (iPrevSlope > 89)
				iPrevSlope = 90;
			else if (iPrevSlope < 1)
				iPrevSlope = 0;
			// If edges are in line and slopes are different then
			if ((iPrevSlope != iSlope)
				&& Collinear2d(PreviousPoint, CurrentPoint, NextPoint))
			{
#ifdef FELKELDEBUG
				VTLOG("Adding dummy eave segment at %d\n", i);
#endif
				// Duplicate the current edge vector
				FPoint3 OldEdge = NextPoint - CurrentPoint;
				FPoint3 NewEdge;
				int iNewSlope;
				if (iSlope > iPrevSlope)
				{
					// Rotate new vertex inwards (clockwise)
					NewEdge.x = OldEdge.z;
					NewEdge.z = -OldEdge.x;
					iNewSlope = iPrevSlope;
				}
				else
				{
					// Rotate new vertext outwards (anticlockwise)
					NewEdge.x = -OldEdge.z;
					NewEdge.z = OldEdge.x;
					iNewSlope = iSlope;
				}
				// Scale to .01 of a co-ord unit
				NewEdge.Normalize();
				NewEdge = NewEdge/100.0f;
				NewEdge += CurrentPoint;
				(*itV).push_back(CEdge(NewEdge.x, 0, NewEdge.z,
					iNewSlope / 180.0f * PIf, pLev->GetEdge(i)->m_pMaterial,
					pLev->GetEdge(i)->m_Color));
			}
			(*itV).push_back(CEdge(CurrentPoint.x, 0, CurrentPoint.z,
				iSlope / 180.0f * PIf, pLev->GetEdge(i)->m_pMaterial,
				pLev->GetEdge(i)->m_Color));
		}
	}

	// Now build the skeleton
	StraightSkeleton.MakeSkeleton(RoofEaves);

	if (0 == StraightSkeleton.m_skeleton.size())
		return -1.0;

	// Merge the original eaves back into the skeleton
	Skeleton = StraightSkeleton.CompleteWingedEdgeStructure(RoofEaves);

	if (0 == Skeleton.size())
		return -1.0;

#ifdef FELKELDEBUG
	VTLOG("Building Geometry\n");
#endif
	// TODO - texture co-ordinates
	// Build the geometry
	for (size_t ci = 0; ci < RoofEaves.size(); ci++)
	{
		Contour& points = RoofEaves[ci];
		for (size_t pi = 0; pi < points.size(); pi++)
		{
			// For each boundary edge zip round the polygon anticlockwise
			// and build the vertex array
			const vtString bmat = *points[pi].m_pMaterial;
			vtMesh *pMesh = FindMatMesh(bmat, points[pi].m_Color, osg::PrimitiveSet::TRIANGLES);
			vtMaterialDescriptor *pMd = GetMatDescriptor(bmat, points[pi].m_Color);
			FPoint2 UVScale;
			if (NULL != pMd)
				UVScale = pMd->GetUVScale();
			else
				UVScale = FPoint2(1.0, 1.0);
			FLine3 RoofSection3D;
			FLine3 TriangulatedRoofSection3D;
			int iTriangleCount = 0;
			FPoint3 PanelNormal;
			FPoint3 UAxis;
			FPoint3 VAxis;
			FPoint3 TextureOrigin;
			int i, j;
			std::vector<int> iaVertices;

			C3DPoint& p1 = points[pi].m_Point;
			C3DPoint& p2 = points[(pi+1)%points.size()].m_Point;
			// Find the starting edge
			CSkeleton::iterator s1;
			for (s1 = Skeleton.begin(); s1 != Skeleton.end(); s1++)
			{
				if (((*s1).m_lower.m_vertex->m_point == p1) &&
					((*s1).m_higher.m_vertex->m_point == p2))
					break;
			}
			if (s1 == Skeleton.end())
				break;

			pStartEdge = &(*s1);
			pEdge = pStartEdge;
			bEdgeReversed = false;
#ifdef FELKELDEBUG
			VTLOG("Building panel\n");
#endif
			uint iNumberofPoints = 0;
			do
			{
				if (iNumberofPoints++ > Skeleton.size())
				{
					VTLOG("MakeFelkelRoof - Roof geometry too complex - giving up\n");
					return -1.0;
				}
				if (bEdgeReversed)
				{
#ifdef FELKELDEBUG
					DebugX = pEdge->m_higher.m_vertex->m_point.m_x;
					DebugY = pEdge->m_higher.m_vertex->m_point.m_y;
					DebugZ = pEdge->m_higher.m_vertex->m_point.m_z;
#endif
					if (pEdge->m_higher.m_vertex->m_point.m_z > (double)fMaxHeight)
						fMaxHeight = (float) pEdge->m_higher.m_vertex->m_point.m_z;
					RoofSection3D.Append(FPoint3(pEdge->m_higher.m_vertex->m_point.m_x, pEdge->m_higher.m_vertex->m_point.m_y + EaveY, pEdge->m_higher.m_vertex->m_point.m_z));
					pNextEdge = pEdge->m_higher.m_right;

					// Safety check: This can happen with poorly-formed input data.
					if (pNextEdge == NULL)
						return -1.0;

//					if (pEdge->m_higher.m_vertex->m_point != pNextEdge->m_higher.m_vertex->m_point)
					if (pEdge->m_higher.VertexID() != pNextEdge->m_higher.VertexID())
						bEdgeReversed = true;
					else
						bEdgeReversed = false;
				}
				else
				{
#ifdef FELKELDEBUG
					DebugX = pEdge->m_lower.m_vertex->m_point.m_x;
					DebugY = pEdge->m_lower.m_vertex->m_point.m_y;
					DebugZ = pEdge->m_lower.m_vertex->m_point.m_z;
#endif
					if (pEdge->m_lower.m_vertex->m_point.m_z > (double)fMaxHeight)
						fMaxHeight = (float) pEdge->m_lower.m_vertex->m_point.m_z;
					RoofSection3D.Append(FPoint3(pEdge->m_lower.m_vertex->m_point.m_x, pEdge->m_lower.m_vertex->m_point.m_y + EaveY, pEdge->m_lower.m_vertex->m_point.m_z));
					pNextEdge = pEdge->m_lower.m_right;

					// Safety check: This can happen with poorly-formed input data.
					if (pNextEdge == NULL)
						return -1.0;

//					if (pEdge->m_lower.m_vertex->m_point != pNextEdge->m_higher.m_vertex->m_point)
					if (pEdge->m_lower.VertexID() != pNextEdge->m_higher.VertexID())
						bEdgeReversed = true;
					else
						bEdgeReversed = false;
				}
#ifdef FELKELDEBUG
				VTLOG("Adding point (ID %d) x %e y %e z %e\n", pEdge->m_ID, DebugX, DebugY, DebugZ);
#endif
				pEdge = pNextEdge;
			}
			// For some reason the pointers dont end up quite the same
			// I will work it out someday
			while (pEdge->m_ID != pStartEdge->m_ID);


			// Remove any vertices that are the same
			for (i = 0; i < (int)RoofSection3D.GetSize(); i++)
			{
				FPoint3& Point = RoofSection3D[i];

				for (j = i + 1; j < (int)RoofSection3D.GetSize(); j++)
				{
					FPoint3& NextPoint = RoofSection3D[j];
					if (NextPoint == Point)
					{
						RoofSection3D.RemoveAt(j);
						j--;
					}
				}
			}


			// determine normal and primary axes of the face
			j = RoofSection3D.GetSize();
			PanelNormal = Normal(RoofSection3D[1], RoofSection3D[0], RoofSection3D[j-1]);
			UAxis = FPoint3(RoofSection3D[j-1] - RoofSection3D[0]).Normalize();
			VAxis = PanelNormal.Cross(UAxis);
			TextureOrigin = RoofSection3D[0];
#ifdef FELKELDEBUG
			VTLOG("Panel normal x %e y %e z %e\n", PanelNormal.x, PanelNormal.y, PanelNormal.z);
#endif
			// Build transform to rotate plane parallel to the xz plane.
			// N.B. this only work with angles from the plane normal to the y axis
			// in the rangle 0 to pi/2 (this is ok for roofs). If you want
			// it to work over a greater range you will have to mess with the sign of the cosine
			// of this angle.
			float fHypot = sqrtf(PanelNormal.x * PanelNormal.x + PanelNormal.z * PanelNormal.z);
			FMatrix3 Transform;
			Transform.SetRow(0, PanelNormal.x * PanelNormal.y / fHypot, PanelNormal.x, -PanelNormal.z / fHypot);
			Transform.SetRow(1, -fHypot, PanelNormal.y, 0);
			Transform.SetRow(2, PanelNormal.z * PanelNormal.y / fHypot, PanelNormal.z, PanelNormal.x / fHypot);

			// Build vertex list
			for (i = 0; i < j; i++)
			{
				FPoint3 Vertex = RoofSection3D[i];
				FPoint2 UV = FPoint2((Vertex - TextureOrigin).Dot(UAxis), (Vertex - TextureOrigin).Dot(VAxis));
				UV.Div(UVScale);
				iaVertices.push_back(pMesh->AddVertexNUV(Vertex, PanelNormal, UV));
			}

			if ((PanelNormal.x != 0) && (PanelNormal.z != 0))
			{
				for (i = 0; i < j; i++)
				{
					// Source and dest cannot be the same
					FPoint3 Temp = RoofSection3D[i];
					Transform.Transform(Temp, RoofSection3D[i]);
				}
			}

			if (!Triangulate_f::Process(RoofSection3D, TriangulatedRoofSection3D))
				return -1.0;

			iTriangleCount = TriangulatedRoofSection3D.GetSize() / 3;

			for (i = 0; i < iTriangleCount; i++)
			{
				int iaIndex[3];

				for (j = 0; j < 3; j++)
				{
					FPoint3 Point = TriangulatedRoofSection3D[i * 3 + j];
					if (-1 == (iaIndex[j] = FindVertex(Point, RoofSection3D, iaVertices)))
						return -1.0;
				}
				pMesh->AddTri(iaIndex[0], iaIndex[2], iaIndex[1]);
#ifdef FELKELDEBUG
				VTLOG("AddTri1 %d %d %d\n", iaIndex[0], iaIndex[2], iaIndex[1]);
#endif
			}
		}
	}

	return fMaxHeight;
}

bool vtBuilding3d::Collinear2d(const FPoint3& Previous, const FPoint3& Current, const FPoint3& Next)
{
	FPoint3 l1 = Previous - Current;
	FPoint3 l2 = Next - Current;

	l1.y = 0;
	l2.y = 0;

	l1.Normalize();
	l2.Normalize();

	float CosTheta = l1.Dot(l2);
	if (CosTheta < -1.0)
		CosTheta = -1.0;
	else if (CosTheta > 1.0)
		CosTheta = 1.0;
	float fTheta = acosf(CosTheta) / PIf * 180;

	if (fabs(fTheta - 180.0) < 1.0)
		return true;
	else
		return false;
}

int vtBuilding3d::FindVertex(const FPoint3 &Point, const FLine3 &RoofSection3D,
	const std::vector<int> &iaVertices)
{
	int iSize = RoofSection3D.GetSize();

	int i;
	for (i = 0; i < iSize; i++)
	{
		if ((Point.x == RoofSection3D[i].x) &&
			(Point.y == RoofSection3D[i].y) &&
			(Point.z == RoofSection3D[i].z))
			break;
	}
	if (i < iSize)
		return iaVertices[i];
	else
	{
		VTLOG("FindVertex - vertex not found\n");
		return -1;
	}
}

//
// Walls which consist of regularly spaced windows and 'siding' material
// can be modelled far more efficiently.  This is very useful for rendering
// speed for large scenes in which the user doesn't have or doesn't care
// about the exact material/windows of the buildings.  We create
// optimized geometry in which each whole wall is a single quad.
//
void vtBuilding3d::CreateUniformLevel(int iLevel, float fHeight,
	int iHighlightEdge)
{
	vtLevel *pLev = m_Levels[iLevel];

	const FPolygon3 &polygon1 = GetLocalFootprint(iLevel);

	int i;
	int base_edge = 0;
	for (uint ring = 0; ring < polygon1.size(); ring++)
	{
		FLine3 poly1 = polygon1[ring];
		FLine3 poly2;

		int edges = poly1.GetSize();
		for (i = 0; i < edges; i++)
			poly1[i].y = fHeight;

		poly2 = poly1;
		for (i = 0; i < edges; i++)
			poly2[i].y += pLev->m_fStoryHeight;

		for (i = 0; i < edges; i++)
		{
			int a = i, b = (a+1)%edges;

			FLine3 quad(4);

			vtEdge	*pEdge = pLev->GetEdge(base_edge+i);

			// do the whole wall section
			quad[0] = poly1[a];
			quad[1] = poly1[b];
			quad[2] = poly2[a];
			quad[3] = poly2[b];

			if (pEdge->m_Facade != "")
			{
				float extraheight = pLev->m_fStoryHeight * (pLev->m_iStories-1);
				quad[2].y += extraheight;
				quad[3].y += extraheight;
				// If we can successfully construct the facade, we don't need to
				//  use the edge features.
				if (MakeFacade(pEdge, quad, pLev->m_iStories))
					continue;
			}
			quad[2] = poly2[a];
			quad[3] = poly2[b];

			float h1 = 0.0f;
			float h2 = (float) pLev->m_iStories;
			float hf1 = (float) pEdge->NumFeaturesOfCode(WFC_WINDOW);
			AddWallSection(pEdge, true, quad, h1, h2, hf1);

			if (base_edge+i == iHighlightEdge)
			{
				for (uint j = 0; j < pLev->m_iStories; j++)
				{
					AddHighlightSection(pEdge, quad);
					quad[0].y += pLev->m_fStoryHeight;
					quad[1].y += pLev->m_fStoryHeight;
					quad[2].y += pLev->m_fStoryHeight;
					quad[3].y += pLev->m_fStoryHeight;
				}
			}
		}
		base_edge += edges;
	}
}

bool vtBuilding3d::MakeFacade(vtEdge *pEdge, FLine3 &quad, int stories)
{
	// Paint a facade on this edge
	// Add the facade image to the materials array
	// Assume quad is ordered 0,1,3,2
	MatMesh mm;
	FPoint3 norm = Normal(quad[0],quad[1],quad[3]);

	vtString fname = "Facade/";
	fname += pEdge->m_Facade;
	fname = FindFileOnPaths(vtGetDataPath(), (pcchar)fname);
	if (fname == "")
	{
		// Older files may have their facades in 'BuildingModels'
		fname = "BuildingModels/";
		fname += pEdge->m_Facade;
		fname = FindFileOnPaths(vtGetDataPath(), (pcchar)fname);
	}
	if (fname == "")
	{
		VTLOG(" Couldn't find facade texture '%s'\n", (const char*)pEdge->m_Facade);
		return false;
	}

	osg::Image *image = LoadOsgImage(fname);
	if (!image)
		return false;

	vtMaterialArray *mats = GetSharedMaterialArray();
	mm.m_iMatIdx = mats->AddTextureMaterial(image,
			true, true, false, false,
			TERRAIN_AMBIENT,
			TERRAIN_DIFFUSE,
			1.0f,		// alpha
			TERRAIN_EMISSIVE);
	mats->at(mm.m_iMatIdx)->SetClamp(false);	// Facades can repeat upwards.

	// Create a mesh for the new material and add this to the mesh array
	mm.m_pMesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_FAN, VT_Normals | VT_TexCoords, 6);
	m_Mesh.push_back(mm);

	// Calculate the vertices and add them to the mesh
	float v = (float) stories;
	int start = mm.m_pMesh->AddVertexNUV(quad[0], norm, FPoint2(0.0f, 0.0f));
	mm.m_pMesh->AddVertexNUV(quad[1], norm, FPoint2(1.0f, 0.0f));
	mm.m_pMesh->AddVertexNUV(quad[3], norm, FPoint2(1.0f, v));
	mm.m_pMesh->AddVertexNUV(quad[2], norm,  FPoint2(0.0f, v));

	mm.m_pMesh->AddFan(start, start+1, start+2, start+3);
	return true;
}

FPoint3 vtBuilding3d::Normal(const FPoint3 &p0, const FPoint3 &p1, const FPoint3 &p2)
{
	FPoint3 a = p0 - p1;
	FPoint3 b = p2 - p1;
	FPoint3 norm = b.Cross(a);
	norm.Normalize();
	return norm;
}

//
// Randomize buildings characteristics
//
void vtBuilding3d::Randomize(int iStories)
{
	RGBi color;

	color = GetColor(BLD_BASIC);
	if (color.r == -1 && color.g == -1 && color.b == -1)
	{
		// unset color
		// random pastel color
		uchar r, g, b;
		r = (uchar) (128 + random(127));
		g = (uchar) (128 + random(127));
		b = (uchar) (128 + random(127));
		SetColor(BLD_BASIC, RGBi(r, g, b));
	}

	color = GetColor(BLD_ROOF);
	if (color.r == -1 && color.g == -1 && color.b == -1)
	{
		// unset color
		// random roof color
		int r = rand() %5;
		switch (r) {
			case 0: color.Set(255, 255, 250); break;	//off-white
			case 1: color.Set(153, 51, 51); break;		//reddish
			case 2: color.Set(153, 153, 255); break;	//blue-ish
			case 3: color.Set(153, 255, 153); break;	//green-ish
			case 4: color.Set(178, 102, 51); break;		//brown
		}

		SetColor(BLD_ROOF, color);
	}
}


/**
 * Creates the geometry for the building.
 * Capable of several levels of detail (defaults to full detail).
 * If the geometry was already built previously, it is destroyed and re-created.
 *
 * \param pTerr The terrain on which to plant the building.
 */
bool vtBuilding3d::CreateNode(vtTerrain *pTerr)
{
	if (m_pContainer)
	{
		// was build before; re-build geometry
		DestroyGeometry();
	}
	else
	{
		// constructing for the first time
		m_pContainer = new vtTransform;
		m_pContainer->setName("building container");
	}
	if (!CreateGeometry(pTerr->GetHeightField()))
		return false;
	m_pContainer->addChild(m_pGeode);
	m_pContainer->SetTrans(m_center);
	return true;
}

bool vtBuilding3d::IsCreated()
{
	return (m_pContainer != NULL);
}

void vtBuilding3d::DeleteNode()
{
	if (m_pContainer)
	{
		if (m_pHighlight)
		{
			m_pContainer->removeChild(m_pHighlight);
			m_pHighlight = NULL;
		}
		DestroyGeometry();
		m_pContainer = NULL;
	}
}

/**
 * Display some bounding wires around the object to highlight it.
 */
void vtBuilding3d::ShowBounds(bool bShow)
{
	if (bShow)
	{
		if (!m_pHighlight)
		{
			// The highlight geometry doesn't exist, so create it.
#if 0
			// Use bounding sphere; this is OK but quite large, so it's hard to
			// tell which building if there are several close together.
			FSphere sphere;
			s2v(m_pGeode->getBound(), sphere);

			m_pHighlight = CreateBoundSphereGeode(sphere);
#elif 0
			// Use the axis-aligned bounding box, which looks funny when square
			// buildings are not aligned with the world axes.
			FBox3 box;
			GetNodeBoundBox(m_pGeode, box);
			vtDynBoundBox *dbox = new vtDynBoundBox(RGBf(1,1,0));
			dbox->SetBox(box);
			m_pHighlight = dbox->pGeode;
#else
			// Use the building's geometry itself.
			m_pHighlight = CreateHighlight();
#endif
			m_pHighlight->SetCastShadow(false);		// no shadow
			m_pContainer->addChild(m_pHighlight);
		}
		m_pHighlight->SetEnabled(true);
	}
	else
	{
		if (m_pHighlight)
			m_pHighlight->SetEnabled(false);
	}
}
