//
// Elastic.cpp
//
// Copyright (c) 2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "Terrain.h"
#include "Elastic.h"
#include "GeomFactory.h"

ElasticPolyline::ElasticPolyline()
{
	m_Materials = new vtMaterialArray;
	m_Materials->AddRGBMaterial(RGBf(1, 0.5, 0), true, true);			// orange solid
	m_Materials->AddRGBMaterial(RGBf(1, 1, 0.5), false, false, true);	// light yellow wireframe
	m_Materials->AddRGBMaterial(RGBf(1, 0.7, 1), false, false, false, 0.4);	// translucent

	// Create a marker post to use for each corner of the polyline
	int matidx = 0;		// orange
	float fRadius = 0.1f;
	m_Marker = CreateCylinderGeom(m_Materials, matidx, VT_Normals, 1.0f,
		fRadius, 10, true, false, false, 1);

	m_Container = new vtGroup;
	m_pTerr = NULL;

	// Defaults
	m_fPostHeight = 1.0f;
	m_fLineHeight = 1.0f;
}

void ElasticPolyline::SetTerrain(vtTerrain *pTerr)
{
	// Remove from any other terrain, add to this one
	if (m_Container->getNumParents() != 0)
		m_Container->getParent(0)->removeChild(m_Container.get());

	m_pTerr = pTerr;
	m_pTerr->GetTopGroup()->addChild(m_Container);
}

void ElasticPolyline::SetLineColor(const RGBAf &color)
{
	m_Materials->at(1)->SetDiffuse(color);
}

void ElasticPolyline::SetPolyline(const DLine2 &line)
{
	m_Line = line;
	Realize();
}

void ElasticPolyline::AddPoint(const DPoint2 &point, bool bEnforceRightAngles)
{
	m_Line.Append(point);

	if (bEnforceRightAngles)
		EnforceRightAngles();

	Realize();
}

void ElasticPolyline::RemovePoint(int iPoint)
{
	m_Line.RemovePoint(iPoint);
	Realize();
}

void ElasticPolyline::SetPoint(int iPoint, const DPoint2 &point, bool bEnforceRightAngles)
{
	m_Line.SetAt(iPoint, point);

	if (bEnforceRightAngles)
		EnforceRightAngles();

	Realize();
}

void ElasticPolyline::EnforceRightAngles()
{
	int npoints = m_Line.GetSize();
	if (npoints < 3)
		return;

	DPoint2 p2 = m_Line[npoints-1];
	DPoint2 p1 = m_Line[npoints-2];
	DPoint2 p0 = m_Line[npoints-3];
	DPoint2 vec = p1 - p0;	// Vector along previous edge

	// Normalize carefully; if we're in dange of dividing by zero, it's better
	//  to quietly return
	if (vec.Length() == 0.0)
		return;

	vec.Normalize();
	DPoint2 vec2 = p2 - p1;	// Vector to new point

	double a = vec2.Dot(vec);	// Distance along edge
	DPoint2 result = p1 + (vec * a);
	m_Line[npoints-2] = result;

	vec.Rotate(PID2d);
	double b = vec2.Dot(vec);	// Distance perpendicular to edge
	result += (vec * b);
	m_Line[npoints-1] = result;
}

void ElasticPolyline::Clear()
{
	m_Line.Clear();
	Realize();
}

float vtTerrain::EstimateGroundSpacingAtPoint(const DPoint2 &p) const
{
	// try to guess how finely to tessellate a feature that will be draped on the surface.
	if (m_pDynGeom)
	{
		const FPoint2 &spacing = m_pDynGeom->GetWorldSpacing();
		return std::min(spacing.x, spacing.y) / 2;
	}
	else if (m_pTin)
	{
		// TINs don't have a grid spacing.  In lieu of using a completely
		//  different (more correct) algorithm for draping, just estimate.
		DRECT ext = m_pTin->GetEarthExtents();
		FPoint2 p1, p2;
		m_pHeightField->m_LocalCS.EarthToLocal(ext.left, ext.bottom, p1.x, p1.y);
		m_pHeightField->m_LocalCS.EarthToLocal(ext.right, ext.top, p2.x, p2.y);
		return (p2 - p1).Length() / 1000.0f;
	}
	else if (m_pTiledGeom)
	{
		// There is no ideal way to drape a line on a tileset of tiles
		//  with varying resolution.  For now, just use the highest (LOD0)
		//  grid density.
		const FPoint2 spacing = m_pTiledGeom->GetWorldSpacingAtPoint(p);
		return std::min(spacing.x, spacing.y);
	}
	return 1.0;		// Should not get here.
}

void ElasticPolyline::Realize()
{
	if (!m_pTerr)	// safety check
		return;

	// Clear previous geometry
	m_Container->removeChildren(0, m_Container->getNumChildren());

	// Make markers
	for (uint i = 0; i < m_Line.GetSize(); i++)
	{
		vtTransform *xform = new vtTransform;
		m_pTerr->PlantModelAtPoint(xform, m_Line[i]);
		xform->Scale(1.0f, m_fPostHeight, 1.0f);

		m_Container->addChild(xform);
		xform->addChild(m_Marker);
	}

	const DPoint2 first_point = m_Line[0];
	const float fSpacing = m_pTerr->EstimateGroundSpacingAtPoint(first_point);
	FLine3 tessellated;
	float fTotalLength = m_pTerr->GetHeightField()->LineOnSurface(m_Line, fSpacing, 0.0f, true,
		false, false, tessellated);

	// Make lines between them
	vtGeode *LineGeode = new vtGeode;
	LineGeode->SetMaterials(m_Materials);
	m_Container->addChild(LineGeode);
	vtGeomFactory mf(LineGeode, osg::PrimitiveSet::LINE_STRIP, 0, 30000, 1);

	mf.PrimStart();
	for (uint i = 0; i < tessellated.GetSize(); i++)
		mf.AddVertex(tessellated[i] + FPoint3(0, m_fLineHeight, 0));
	mf.PrimEnd();

	// Make strips between them
	vtGeode *StripGeode = new vtGeode;
	StripGeode->SetMaterials(m_Materials);
	m_Container->addChild(StripGeode);
	vtGeomFactory mf2(StripGeode, osg::PrimitiveSet::TRIANGLE_STRIP, 0, 30000, 2);

	mf2.PrimStart();
	for (uint i = 0; i < tessellated.GetSize(); i++)
	{
		mf2.AddVertex(tessellated[i]);
		mf2.AddVertex(tessellated[i] + FPoint3(0, m_fLineHeight * 0.99f, 0));
	}
	mf2.PrimEnd();
}

