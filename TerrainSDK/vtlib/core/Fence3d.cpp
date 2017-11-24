//
// Fence3d.cpp
//
// Creates linear structure geometry, drapes it on a terrain.
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/Triangulate.h"
#include "vtdata/vtLog.h"
#include "vtdata/DataPath.h"

#include "Light.h"
#include "Terrain.h"
#include "Fence3d.h"


vtFence3d::vtFence3d() : vtFence()
{
	Init();
}

void vtFence3d::Init()
{
	m_pContainer = NULL;
	m_pFenceGeom = NULL;
	m_pHighlightMesh = NULL;
	m_bBuilt = false;
}

void vtFence3d::AddFencepost(const FPoint3 &p1, int iMatIdx)
{
	// create fencepost block
	vtMesh *pPostMesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_FAN, VT_Normals | VT_TexCoords, 20);

	FPoint3 PostSize(m_Params.m_fPostWidth, m_Params.m_fPostHeight, m_Params.m_fPostDepth);
	pPostMesh->CreateOptimizedBlock(PostSize);

	// scoot over and upwards to put it above ground
	FMatrix4 t;
	t.Identity();
	t.Translate(p1);
	pPostMesh->TransformVertices(t);

	m_pFenceGeom->AddMesh(pPostMesh, iMatIdx);
}

FPoint3 SidewaysVector(const FPoint3 &p0, const FPoint3 &p1)
{
	FPoint3 diff = p1 - p0;
	FPoint3 up(0,1,0);
	FPoint3 cross = diff.Cross(up);
	cross.Normalize();
	return cross;
}

void vtFence3d::AddWireMeshes(const FLine3 &p3)
{
	// special connector type, consisting of 3 wires
	uint i, j, npoints = p3.GetSize();
	if (npoints > 1)
	{
		float wire_height[3] = { 0.42f, 0.66f, 0.91f };

		vtMesh *pWireMesh = new vtMesh(osg::PrimitiveSet::LINE_STRIP, 0, npoints);
		int vidx = 0;
		for (j = 0; j < 3; j++)
		{
			int start = vidx;
			FPoint3 upwards(0, (m_Params.m_fPostHeight * wire_height[j]), 0);
			for (i = 0; i < npoints; i++)
			{
				pWireMesh->AddVertex(p3[i] + upwards);
				vidx++;
			}
			pWireMesh->AddStrip2(npoints, start);
		}
		m_pFenceGeom->AddMesh(pWireMesh, GetMatIndex("Wire"));
	}
}

//
// A single thin strip polygon with a single texture.
//
void vtFence3d::AddFlatConnectionMesh(const FLine3 &p3)
{
	vtMesh *pMesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_STRIP, VT_TexCoords, 100);

	vtMaterialDescriptor *desc = GetMatDescriptor(m_Params.m_ConnectMaterial);
	if (!desc)
	{
		VTLOG1("Warning: could not find material:");
		VTLOG1(m_Params.m_ConnectMaterial);
		VTLOG1("\n");
		return;
	}

	FPoint2 uvscale = desc->GetUVScale();

	float u = 0.0f;
	float vertical_meters = m_Params.m_fConnectTop - m_Params.m_fConnectBottom;
	float v_top;
	if (uvscale.y == -1)
		v_top = 1.0f;
	else
		v_top = vertical_meters / uvscale.y;

	uint i, npoints = p3.GetSize();
	int vidx = 0;
	for (i = 0; i < npoints; i++)
	{
		pMesh->SetVtxPUV(vidx++, p3[i] + FPoint3(0, m_Params.m_fConnectBottom, 0), u, 0.0);
		pMesh->SetVtxPUV(vidx++, p3[i] + FPoint3(0, m_Params.m_fConnectTop, 0), u, v_top);

		if (i < npoints-1)
		{
			// increment u based on the length of each fence segment
			float length_meters = (p3[i+1] - p3[i]).Length();
			u += (length_meters / desc->GetUVScale().x);
		}
	}
	pMesh->AddStrip2(npoints * 2, 0);

	m_pFenceGeom->AddMesh(pMesh, GetMatIndex(desc));
}

void vtFence3d::AddThickConnectionMesh(const FLine3 &p3)
{
	vtMesh *pMesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_STRIP, VT_TexCoords | VT_Normals, 100);

	// a solid block, with top/left/right sides, made of 3 strips
	vtMaterialDescriptor *desc = GetMatDescriptor(m_Params.m_ConnectMaterial);
	if (!desc)
	{
		VTLOG1("Warning: could not find material: ");
		VTLOG1(m_Params.m_ConnectMaterial);
		VTLOG1("\n");
		return;
	}
	FPoint2 uvscale = desc->GetUVScale();
	float vertical_meters = m_Params.m_fConnectTop - m_Params.m_fConnectBottom;

	float fWidthTop = m_Params.m_fConnectWidth / 2;
	float slope = m_Params.m_iConnectSlope / 180.0f * PIf;

	uint i, j, npoints = p3.GetSize();
	float u = 0.0f;
	float v1, v2;
	for (i = 0; i < 3; i++)
	{
		float y1, y2;
		float z1, z2;
		FPoint3 pos, sideways, normal;

		int start = pMesh->NumVertices();
		for (j = 0; j < npoints; j++)
		{
			// determine side-pointing vector
			if (j == 0)
				sideways = SidewaysVector(p3[j], p3[j+1]);
			else if (j > 0 && j < npoints-1)
			{
				AngleSideVector(p3[j-1], p3[j], p3[j+1], sideways);
				sideways = -sideways;	// We want a vector pointing left, not right
			}
			else if (j == npoints-1)
				sideways = SidewaysVector(p3[j-1], p3[j]);

			// 'extra' elevation is added to maintain a constant top
			float fExtraElevation = 0.0f;
			if (m_Params.m_bConstantTop)
				fExtraElevation = m_fMaxGroundY - p3[j].y;

			float fVertical = vertical_meters + fExtraElevation;
			float fWidthBottom = fWidthTop + fVertical / tan(slope);

			// determine v texture coordinate
			switch (i)
			{
			case 0:		// right side
				v1 = 0.0f;
				if (uvscale.y == -1)
					v2 = 1.0f;
				else
					v2 = fVertical / uvscale.y;
				break;
			case 1:		// top
				v1 = 0.0f;
				if (uvscale.y == -1)
					v2 = 1.0f;
				else
					v2 = m_Params.m_fConnectWidth / uvscale.y;
				break;
			case 2:		// left side
				v2 = 0.0f;
				if (uvscale.y == -1)
					v1 = 1.0f;
				else
					v1 = fVertical / uvscale.y;
				break;
			}

			// determine Y and Z values
			switch (i)
			{
			case 0:	// right side
				y1 = m_Params.m_fConnectBottom;
				y2 = m_Params.m_fConnectTop + fExtraElevation;
				z1 = fWidthBottom;
				z2 = fWidthTop;
				break;
			case 1:	// top
				y1 = m_Params.m_fConnectTop + fExtraElevation;
				y2 = m_Params.m_fConnectTop + fExtraElevation;
				z1 = fWidthTop;
				z2 = -fWidthTop;
				break;
			case 2:	// left side
				y1 = m_Params.m_fConnectTop + fExtraElevation;
				y2 = m_Params.m_fConnectBottom;
				z1 = -fWidthTop;
				z2 = -fWidthBottom;
				break;
			}

			// determine vertex normal (used for shading and thickness)
			switch (i)
			{
			case 0: normal = sideways; break;	// right
			case 1: normal.Set(0,1,0); break;	// top: up
			case 2: normal = -sideways; break;	// left
			}

			pos = p3[j];
			pos.y += y2;
			pos += (sideways * z2);
			pMesh->AddVertexNUV(pos, normal, FPoint2(u, v2));

			pos = p3[j];
			pos.y += y1;
			pos += (sideways * z1);
			pMesh->AddVertexNUV(pos, normal, FPoint2(u, v1));

			if (j < npoints-1)
			{
				// increment u based on the length of each fence segment
				float length_meters = (p3[j+1] - p3[j]).Length();
				u += (length_meters / uvscale.x);
			}
		}
		pMesh->AddStrip2(npoints * 2, start);
	}

	// add cap at beginning
	u = m_Params.m_fConnectWidth / desc->GetUVScale().x;
	v2 = vertical_meters / uvscale.y;

	int start =
	pMesh->AddVertex(pMesh->GetVtxPos(npoints*2*2+1));
	pMesh->AddVertex(pMesh->GetVtxPos(npoints*2*2));
	pMesh->AddVertex(pMesh->GetVtxPos(0));
	pMesh->AddVertex(pMesh->GetVtxPos(1));
	pMesh->SetVtxNormal(start+0, p3[0] - p3[1]);
	pMesh->SetVtxNormal(start+1, p3[0] - p3[1]);
	pMesh->SetVtxNormal(start+2, p3[0] - p3[1]);
	pMesh->SetVtxNormal(start+3, p3[0] - p3[1]);
	pMesh->SetVtxTexCoord(start+0, FPoint2(u, 0.0f));
	pMesh->SetVtxTexCoord(start+1, FPoint2(u, v2));
	pMesh->SetVtxTexCoord(start+2, FPoint2(0.0f, 0.0f));
	pMesh->SetVtxTexCoord(start+3, FPoint2(0.0f, v2));
	pMesh->AddStrip2(4, start);

	// add cap at end
	start =
	pMesh->AddVertex(pMesh->GetVtxPos(npoints*2-2));
	pMesh->AddVertex(pMesh->GetVtxPos(npoints*2-1));
	pMesh->AddVertex(pMesh->GetVtxPos(npoints*3*2-1));
	pMesh->AddVertex(pMesh->GetVtxPos(npoints*3*2-2));
	pMesh->SetVtxNormal(start+0, p3[npoints-1] - p3[npoints-2]);
	pMesh->SetVtxNormal(start+1, p3[npoints-1] - p3[npoints-2]);
	pMesh->SetVtxNormal(start+2, p3[npoints-1] - p3[npoints-2]);
	pMesh->SetVtxNormal(start+3, p3[npoints-1] - p3[npoints-2]);
	pMesh->SetVtxTexCoord(start+0, FPoint2(0.0f, 0.0f));
	pMesh->SetVtxTexCoord(start+1, FPoint2(0.0f, v2));
	pMesh->SetVtxTexCoord(start+2, FPoint2(u, 0.0f));
	pMesh->SetVtxTexCoord(start+3, FPoint2(u, v2));
	pMesh->AddStrip2(4, start);

	m_pFenceGeom->AddMesh(pMesh, GetMatIndex(desc));
}

void vtFence3d::AddProfileConnectionMesh(const FLine3 &p3)
{
	uint i, j, npoints = p3.GetSize(), prof_points = m_Profile.GetSize();

	// Must have at least 2 points in the profile
	if (prof_points < 2)
		return;

	// Each segment of the profile becomes a long triangle strip.
	// If there are no shared vertices between segments, the number of
	//  vertices is, for a profile of N points and a line of P points:
	//   P * (N-1) * 2, for the sides
	//   N*2, for the end caps (or more if we have to tessellate)
	//
	int iEstimateVerts = npoints * (prof_points-1) * 2 + (prof_points * 2);
	vtMesh *pMesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_STRIP,
		VT_TexCoords | VT_Normals, iEstimateVerts);

	vtMaterialDescriptor *desc = GetMatDescriptor(m_Params.m_ConnectMaterial);
	if (!desc)
	{
		VTLOG1("Warning: could not find material: ");
		VTLOG1(m_Params.m_ConnectMaterial);
		VTLOG1("\n");
		return;
	}
	FPoint2 uvscale = desc->GetUVScale();

	// determine side-pointing vector
	vtArray<float> ExtraElevation(npoints);
	FLine3 sideways(npoints);
	for (j = 0; j < npoints; j++)
	{
		// determine side-pointing vector
		if (j == 0)
			sideways[j] = SidewaysVector(p3[j], p3[j+1]);
		else if (j > 0 && j < npoints-1)
		{
			AngleSideVector(p3[j-1], p3[j], p3[j+1], sideways[j]);
			sideways[j] = -sideways[j];	// We want a vector pointing left, not right
		}
		else if (j == npoints-1)
			sideways[j] = SidewaysVector(p3[j-1], p3[j]);

		ExtraElevation[j] = 0.0f;
		if (m_Params.m_bConstantTop)
			ExtraElevation[j] = m_fMaxGroundY - p3[j].y;
	}

	float u;
	float v1, v2;
	for (i = 0; i < prof_points-1; i++)
	{
		float y1, y2;
		float z1, z2;
		FPoint3 pos, normal;

		// determine v texture coordinate
		float seg_length = m_Profile.SegmentLength(i);
		if (uvscale.y == -1)
		{
			v1 = 0.0f;
			v2 = 1.0f;
		}
		else
		{
			if (i == 0)
			{
				v1 = 0.0f;
				v2 = seg_length / uvscale.y;
			}
			else
			{
				v1 = v2;
				v2 += seg_length / uvscale.y;
			}
		}

		// determine Y and Z values
		y1 = m_Profile[i].y;
		y2 = m_Profile[i+1].y;
		z1 = m_Profile[i].x;
		z2 = m_Profile[i+1].x;

		u = 0.0f;
		int start = pMesh->NumVertices();
		for (j = 0; j < npoints; j++)
		{
			// determine vertex normal (for shading)
			float diffy = y2-y1;
			float diffz = z2-z1;
			float dy = -diffz;
			float dz = diffy;

			FPoint3 n1, n2;
			n1.Set(0, y1, 0);
			n1 += (sideways[j] * z1);
			n2.Set(0, y1 + dy, 0);
			n2 += (sideways[j] * (z1 + dz));

			normal = n2 - n1;
			normal.Normalize();

			// determine the two points of this segment edge, and add them
			pos = p3[j];
			pos.y += y2;
			pos.y += ExtraElevation[j];
			pos += (sideways[j] * z2);
			pMesh->AddVertexNUV(pos, normal, FPoint2(u, v2));

			pos = p3[j];
			pos.y += y1;
			pos.y += ExtraElevation[j];
			pos += (sideways[j] * z1);
			pMesh->AddVertexNUV(pos, normal, FPoint2(u, v1));

			if (j < npoints-1)
			{
				// increment u based on the length of each fence segment
				float length_meters = (p3[j+1] - p3[j]).Length();
				u += (length_meters / uvscale.x);
			}
		}
		pMesh->AddStrip2(npoints * 2, start);
	}

	// We must assume the profile is interpreted as a closed polygon, which
	//  may not be convex.  Hence it must be triangulated.
	FLine2 result;
	Triangulate_f::Process(m_Profile, result);
	uint tcount = result.GetSize()/3;

	int ind[3];
	int line_point;
	FPoint3 normal;

	// add cap at beginning
	line_point = 0;
	normal = p3[0] - p3[1];
	normal.Normalize();
	for (i=0; i<tcount; i++)
	{
		for (j = 0; j < 3; j++)
		{
			FPoint2 p2 = result[i*3+j];

			FPoint3 pos = p3[line_point];
			pos.y += p2.y;
			pos.y += ExtraElevation[line_point];
			pos += (sideways[line_point] * p2.x);

			FPoint2 uv = p2;
			if (uvscale.y != -1)
				uv.Div(uvscale);	// divide meters by [meters/uv] to get uv

			ind[j] = pMesh->AddVertexNUV(pos, normal, uv);
		}
		pMesh->AddTri(ind[0], ind[1], ind[2]);
	}

	// add cap at end
	line_point = npoints-1;
	normal = p3[npoints-1] - p3[npoints-2];
	normal.Normalize();
	for (i=0; i<tcount; i++)
	{
		for (j = 0; j < 3; j++)
		{
			FPoint2 p2 = result[i*3+j];

			FPoint3 pos = p3[line_point];
			pos.y += p2.y;
			pos.y += ExtraElevation[line_point];
			pos += (sideways[line_point] * p2.x);

			FPoint2 uv = p2;
			if (uvscale.y != -1)
				uv.Div(uvscale);	// divide meters by [meters/uv] to get uv

			ind[j] = pMesh->AddVertexNUV(pos, normal, uv);
		}
		pMesh->AddTri(ind[0], ind[2], ind[1]);
	}

	m_pFenceGeom->AddMesh(pMesh, GetMatIndex(desc));
}

void vtFence3d::AddPostExtensions(const FLine3 &p3)
{
	uint i, j, npoints = p3.GetSize();

	bool bLeft = false, bRight = false, bWires = (npoints > 1);
	if (m_Params.m_PostExtension == "left")
		bLeft = true;
	if (m_Params.m_PostExtension == "right")
		bRight = true;
	if (m_Params.m_PostExtension == "double")
		bLeft = bRight = true;

	// create extension prism(s)
	vtMesh *pMesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_FAN, VT_Normals, 20);

	FPoint2 size(m_Params.m_fPostWidth, m_Params.m_fPostWidth);
	FPoint2 size1 = size * 0.9f;
	FPoint2 size2 = size * 0.5f;
	FPoint3 sideways, upward;

	float wire_height[3] = { 0.3f, 0.6f, 0.9f };

	vtMesh *pWiresLeft, *pWiresRight;
	if (bLeft && bWires)
		pWiresLeft = new vtMesh(osg::PrimitiveSet::LINE_STRIP, 0, npoints*3);
	if (bRight && bWires)
		pWiresRight = new vtMesh(osg::PrimitiveSet::LINE_STRIP, 0, npoints*3);

	for (i = 0; i < npoints; i++)
	{
		FPoint3 base = p3[i] + FPoint3(0.0f, m_Params.m_fPostHeight, 0.0f);

		// determine side-pointing normal
		if (i == 0)
			sideways = SidewaysVector(p3[i], p3[i+1]);
		else if (i > 0 && i < npoints-1)
		{
			AngleSideVector(p3[i-1], p3[i], p3[i+1], sideways);
			sideways = -sideways;	// We want a vector pointing left, not right
		}
		else if (i == npoints-1)
			sideways = SidewaysVector(p3[i-1], p3[i]);

		if (bLeft && bWires)
		{
			upward = -sideways + FPoint3(0,1,0);
			upward.SetLength(0.35);	// roughly 1 foot is standard
			pMesh->CreatePrism(base,
				upward, size1, size2);

			for (j = 0; j < 3; j++)
				pWiresLeft->SetVtxPos(j*npoints + i, base + upward * wire_height[j]);
		}
		if (bRight && bWires)
		{
			upward = sideways + FPoint3(0,1,0);
			upward.SetLength(0.35);	// roughly 1 foot is standard
			pMesh->CreatePrism(base, upward, size1, size2);

			for (j = 0; j < 3; j++)
				pWiresRight->SetVtxPos(j*npoints + i, base + upward * wire_height[j]);
		}
	}
	m_pFenceGeom->AddMesh(pMesh, GetMatIndex("Steel"));

	if (bLeft && bWires)
	{
		pWiresLeft->AddStrip2(npoints, 0);
		pWiresLeft->AddStrip2(npoints, npoints);
		pWiresLeft->AddStrip2(npoints, npoints*2);
		m_pFenceGeom->AddMesh(pWiresLeft, GetMatIndex("Wire"));
	}
	if (bRight && bWires)
	{
		pWiresRight->AddStrip2(npoints, 0);
		pWiresRight->AddStrip2(npoints, npoints);
		pWiresRight->AddStrip2(npoints, npoints*2);
		m_pFenceGeom->AddMesh(pWiresRight, GetMatIndex("Wire"));
	}
}

void vtFence3d::AddFenceMeshes(vtHeightField3d *pHeightField)
{
	// Trigger the creation of any materials we may need
	GetMatIndex("");

	uint i, j;
	uint numfencepts = m_pFencePts.GetSize();

	FLine3 p3;

	FPoint3 diff, fp;
	FPoint3 PostSize(m_Params.m_fPostWidth, m_Params.m_fPostHeight,
		m_Params.m_fPostDepth);

	// All culture (roads and buildings) can be draped on
	int iIncludeCulture = CE_ALL;

	// first, project the posts from earth to world
	m_Posts3d.SetSize(numfencepts);
	for (i = 0; i < numfencepts; i++)
		pHeightField->ConvertEarthToSurfacePoint(m_pFencePts[i], m_Posts3d[i], iIncludeCulture);

	// Find highest point
	m_fMaxGroundY = -1E8;
	for (i = 0; i < numfencepts; i++)
		if (m_Posts3d[i].y > m_fMaxGroundY)
			m_fMaxGroundY = m_Posts3d[i].y;

	if (m_Params.m_PostType != "none")
	{
		// has posts
		// determine where the fence posts go
		for (i = 0; i < numfencepts; i++)
		{
			if (i == numfencepts-1)
			{
				p3.Append(m_Posts3d[i]);
				continue;
			}
			// get start and end group points for this section
			FPoint3 wpos1 = m_Posts3d[i];
			FPoint3 wpos2 = m_Posts3d[i+1];

			// look at world distance (approximate meters, _not_ earth
			//  coordinates, which might be in e.g. feet or degrees)
			diff = wpos2 - wpos1;
			float distance = sqrt(diff.x*diff.x+diff.z*diff.z);
			uint segments = (uint) (distance / m_Params.m_fPostSpacing);
			if (segments < 1) segments = 1;
			FPoint3 diff_per_segment = diff / (float) segments;

			for (j = 0; j < segments; j++)
			{
				fp = wpos1 + (diff_per_segment * (float)j);

				if (i > 0 && i < numfencepts-1)
				{
					// randomly offset by up to 4% of fence spacing, for "realism"
					fp.x += random_offset(0.04f * m_Params.m_fPostSpacing);
					fp.z += random_offset(0.04f * m_Params.m_fPostSpacing);
				}
				// false: true elevation, true: include culture (structures and roads)
				pHeightField->FindAltitudeAtPoint(fp, fp.y, false, CE_ALL);
				p3.Append(fp);
			}
		}
		// generate the posts
		// Look first for post materials (type 3)
		int iMatIdx = GetMatIndex(m_Params.m_PostType, RGBf(), 3);

		// If that didn't work, look for any material by that name
		if (iMatIdx == -1)
			int iMatIdx = GetMatIndex(m_Params.m_PostType);
		for (i = 0; i < p3.GetSize(); i++)
			AddFencepost(p3[i], iMatIdx);
	}
	else
	{
		// no post spacing to consider, so just use the input vertices
		p3.SetSize(numfencepts);
		for (i = 0; i < numfencepts; i++)
			p3[i] = m_Posts3d[i];
	}

	if (m_Params.m_PostExtension != "none")
		AddPostExtensions(p3);

	// if not enough points, nothing connections to create
	if (p3.GetSize() < 2)
		return;

	if (m_Params.m_iConnectType == 0)	// none
	{
		// nothing to do
	}
	else if (m_Params.m_iConnectType == 1)	// wire
	{
		AddWireMeshes(p3);
	}

	if (m_Params.m_ConnectMaterial == "none")
		return;

	if (m_Params.m_iConnectType == 2)	// simple
	{
		if (m_Params.m_fConnectWidth == 0.0f)
			AddFlatConnectionMesh(p3);
		else if (m_Params.m_fConnectWidth > 0.0f)
			AddThickConnectionMesh(p3);
	}
	else if (m_Params.m_iConnectType == 3)	// profile
	{
		AddProfileConnectionMesh(p3);
	}
}

void vtFence3d::DestroyGeometry()
{
	if (m_pFenceGeom.valid())
	{
		// Destroy the meshes so they can be re-made
		m_pFenceGeom->RemoveAllMeshes();
		m_pHighlightMesh = NULL;
	}
	m_bBuilt = false;
}

/////////////////////////////////////
// Implement vtStructure3d methods

/**
 * Build (or rebuild) the geometry for a fence.
 */
bool vtFence3d::CreateNode(vtTerrain *pTerr)
{
	bool bHighlighted = (m_pHighlightMesh != NULL);
	if (m_bBuilt)
	{
		// was build before; destroy meshes in order to re-build
		DestroyGeometry();
	}
	else
	{
		m_pFenceGeom = new vtGeode;
		m_pFenceGeom->setName("Fence");
		m_pFenceGeom->SetMaterials(GetSharedMaterialArray());
	}

	// create surface and shape
	AddFenceMeshes(pTerr->GetHeightField());

	if (bHighlighted)
		ShowBounds(true);

	m_bBuilt = true;
	return true;
}

bool vtFence3d::IsCreated()
{
	return (m_pFenceGeom.valid());
}

void vtFence3d::DeleteNode()
{
	DestroyGeometry();
	m_pFenceGeom = NULL;
}

void vtFence3d::ShowBounds(bool bShow)
{
	if (m_pHighlightMesh)
	{
		// remove previous
		m_pFenceGeom->RemoveMesh(m_pHighlightMesh);
		m_pHighlightMesh = NULL;
	}
	if (bShow)
	{
		uint i, npoints = m_pFencePts.GetSize();

		// Create border around the feature, also some lines as handles for
		//  the control points.
		// Must be tall enough to be visible above all the posts and profile.
		float height = max(m_Params.m_fPostHeight, m_Params.m_fConnectTop);
		float width = m_Params.m_fConnectWidth;
		if (m_Params.m_iConnectType == 3)
		{
			for (i = 0; i < m_Profile.GetSize(); i++)
			{
				float x = fabs(m_Profile[i].x), y = m_Profile[i].y;
				if (y > height) height = y;
				if (x > width) width = x;
			}
		}
		height += 1.0f;
		width += 1.0f;

		// border around the feature
		m_pHighlightMesh = new vtMesh(osg::PrimitiveSet::LINE_STRIP, 0, npoints*2);
		FPoint3 sideways, up(0,1,0);
		for (i = 0; i < npoints; i++)
		{
			// determine normal
			if (i == 0)
				sideways = SidewaysVector(m_Posts3d[i], m_Posts3d[i+1]);
			else if (i > 0 && i < npoints-1)
			{
				AngleSideVector(m_Posts3d[i-1], m_Posts3d[i], m_Posts3d[i+1], sideways);
				sideways = -sideways;	// We want a vector pointing left, not right
			}
			else if (i == npoints-1)
				sideways = SidewaysVector(m_Posts3d[i-1], m_Posts3d[i]);

			sideways.SetLength(width);

			m_pHighlightMesh->AddVertex(m_Posts3d[i] - sideways + up);
			m_pHighlightMesh->AddVertex(m_Posts3d[i] + sideways + up);
		}

		std::vector<unsigned short> idx;
		for (i = 0; i < npoints; i++) idx.push_back(i*2);
		for (i = 0; i < npoints; i++) idx.push_back((npoints*2)-1 - i*2);
		idx.push_back(0);
		m_pHighlightMesh->AddStrip(idx.size(), &idx.front());

		for (i = 0; i < npoints; i++)
		{
			float extra_height = 0.0f;
			if (m_Params.m_bConstantTop)
				extra_height = m_fMaxGroundY - m_Posts3d[i].y;

			int v0 = m_pHighlightMesh->AddVertex(m_Posts3d[i]);
			int v1 = m_pHighlightMesh->AddVertex(m_Posts3d[i] + FPoint3(0,height+extra_height,0));
			m_pHighlightMesh->AddLine(v0, v1);
		}

		// Use yellow highlight material
		int highlight_matidx = GetMatIndex("Highlight", RGBf(1,1,0));

		m_pFenceGeom->AddMesh(m_pHighlightMesh, highlight_matidx);
	}
}

void vtFence3d::SetParams(const vtLinearParams &params)
{
	// Reload profile, if we know it is different than before
	bool bReload = (params.m_iConnectType == 3 &&
		params.m_ConnectProfile != m_Params.m_ConnectProfile);

	m_Params = params;

	if (bReload)
		LoadProfile();
}

void vtFence3d::ProfileChanged()
{
	// When we know the profile has changed, reload it.
	LoadProfile();
}

void vtFence3d::LoadProfile()
{
	vtString path = FindFileOnPaths(vtGetDataPath(),
		"BuildingData/" + m_Params.m_ConnectProfile);
	if (path != "")
		LoadFLine2FromSHP(path, m_Profile);
	else
		m_Profile.Clear();
}
