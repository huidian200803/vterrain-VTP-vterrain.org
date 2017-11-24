//
// Route.cpp
//
// Creates a route (a series of utility structures, e.g. an electrical
// transmission line), creates geometry, drapes on a terrain
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
//////////////////////////////////////////////////////////////////////

#include "vtlib/vtlib.h"

#include "vtdata/HeightField.h"

#include "Light.h"
#include "LodGrid.h"
#include "TerrainScene.h"	// for LoadUtilStructure
#include "UtilityMap3d.h"

#define NUM_WIRE_SEGMENTS	160
#define METERS_PER_FOOT		0.3048f	// meters per foot


///////////////////

vtPole3d::vtPole3d()
{
	m_pUtilStruct = NULL;
	dRadAzimuth = 0.0f;
	m_pTrans = NULL;
}

bool vtPole3d::CreateGeometry(const vtHeightField3d *pHF)
{
	bool bAdd = false;
	if (!m_pTrans)
	{
		if (!m_pUtilStruct)
		{
			// Load structure for the indicated node.
			// sPath identifies the path to the Route data
			vtUtilStruct *struc = vtGetTS()->LoadUtilStructure(m_sStructName);
			if (struc)
				m_pUtilStruct = struc;
		}

		if (!m_pUtilStruct)
			return false;

		osg::Node *tower = m_pUtilStruct->m_pTower;
		if (tower)
		{
			m_pTrans = new vtTransform;
			m_pTrans->addChild(tower);
			bAdd = true;
		}
	}

	// set orientation
	m_pTrans->Identity();
	m_pTrans->RotateLocal(FPoint3(0,1,0), dRadAzimuth);

	FPoint3 wpos;
	pHF->m_LocalCS.EarthToLocal(m_p.x, m_p.y, wpos.x, wpos.z);
	pHF->FindAltitudeAtPoint(wpos, wpos.y);
	m_pTrans->SetTrans(wpos);

	return bAdd;
}

void vtPole3d::DestroyGeometry()
{
	if (m_pTrans)
	{
		osg::Group *parent = m_pTrans->getParent(0);
		if (parent)
			parent->removeChild(m_pTrans);

		m_pTrans = NULL;
	}
}

///////////////////

vtLine3d::vtLine3d()
{
	m_bBuilt = false;

	m_pWireGeom = new vtGeode;
	m_pWireGeom->setName("Catenary wires");
}

//
// Builds (or rebuilds) the geometry for a route.
//
void vtLine3d::CreateGeometry(vtHeightField3d *pHeightField, int matidx)
{
	if (m_bBuilt)
		DestroyGeometry();

	// create surface and shape
	for (uint i = 1; i < NumPoles(); i++)
		CreateCatenaryWire(i, pHeightField, matidx);

	m_bBuilt = true;
}

void vtLine3d::DestroyGeometry()
{
	// Destroy the meshes so they can be re-made
	m_pWireGeom->RemoveAllMeshes();
	m_bBuilt = false;
}

void vtLine3d::ComputePoleRotations()
{
	int i, poles = m_poles.size();
	DPoint2 curr, diff_last, diff_next, diff_use;

	if (poles < 2)
		return;

	for (i = 0; i < poles; i++)
	{
		curr = m_poles[i]->m_p;
		if (i > 0)
		{
			diff_last = curr - m_poles[i-1]->m_p;
			diff_last.Normalize();
			diff_use = diff_last;
		}
		if (i < poles-1)
		{
			diff_next = m_poles[i+1]->m_p - curr;
			diff_next.Normalize();
			diff_use = diff_next;
		}
		if (i > 0 && i < poles-1)
		{
			// has a node before and after, average the angles
			diff_use = diff_last + diff_next;
		}
		diff_use.Normalize();
		double angle = atan2(diff_use.y, diff_use.x);
		GetPole(i)->dRadAzimuth = angle;
	}
}

//
// Using pole numbers i and i-1, string catenaries between them
//
void vtLine3d::CreateCatenaryWire(int iPoleIndex, vtHeightField3d *pHeightField,
	int matidx)
{
	const int numiterations = NUM_WIRE_SEGMENTS;

	const vtPole3d *n0 = GetPole(iPoleIndex - 1);
	const vtPole3d *n1 = GetPole(iPoleIndex);

	const vtUtilStruct *st0 = n0->m_pUtilStruct;
	const vtUtilStruct *st1 = n1->m_pUtilStruct;

	// safety check
	if (!st0 || !st1)
		return;

	const DPoint2 p0 = n0->m_p;
	const DPoint2 p1 = n1->m_p;

	FPoint3 fp0, fp1;
	pHeightField->ConvertEarthToSurfacePoint(p0, fp0);
	pHeightField->ConvertEarthToSurfacePoint(p1, fp1);

	FMatrix4 rot;
	rot.Identity();
	FPoint3 axisY(0, 1, 0);
	FPoint3 offset, wire0, wire1;

	vtMesh *pWireMesh;
	for (int j = 0; j < st1->m_iNumWires; j++)
	{
		pWireMesh = new vtMesh(osg::PrimitiveSet::LINE_STRIP, 0, numiterations+1);

		offset = st0->m_fpWireAtt1[j];
		rot.AxisAngle(axisY, n0->dRadAzimuth);
		rot.Transform(offset, wire0);
		FPoint3 wire_start = fp0 + wire0;

		pWireMesh->AddVertex(wire_start);

		offset = st1->m_fpWireAtt2[j];
		rot.AxisAngle(axisY, n1->dRadAzimuth);
		rot.Transform(offset, wire1);
		FPoint3 wire_end = fp1 + wire1;

		_DrawCat(pHeightField, wire_start, wire_end, vtGetTS()->m_fCatenaryFactor,
			numiterations, pWireMesh);

		pWireMesh->AddVertex(wire_end);

		pWireMesh->AddStrip2(numiterations+1, 0);
		m_pWireGeom->AddMesh(pWireMesh, matidx);
	}
}

//
// Draw catenary curve between conductor attachment points
//	from the current tower to the previous tower.
//
void vtLine3d::_DrawCat(vtHeightField3d *pHeightField, const FPoint3 &pt0,
	const FPoint3 &pt1, double catenary, int iNumSegs, vtMesh *pWireMesh)
{
	FPoint3 diff = pt1-pt0;
	FPoint3 ptNew;
	int i;
	double xz = sqrt(diff.z*diff.z+diff.x*diff.x);	// distance in the xz plane
	double y = diff.y;
	FPoint3 step = diff / (float) iNumSegs;

	// Calculate the parabolic constants.
	double parabolicConst = (xz / 2) - (y * (catenary / xz));

	FPoint3 ptCur(0,0,0);

	// Iterate along the xz-plane
	for (i = 0; i < iNumSegs-1; i++)
	{
		ptCur.x += step.x;
		ptCur.z += step.z;
		const double dist = sqrt(ptCur.x*ptCur.x + ptCur.z*ptCur.z);

		ptCur.y = (float) ((dist / (2*catenary)) * (dist - (2*parabolicConst)));

		ptNew = pt0 + ptCur;
		float ground;
		pHeightField->FindAltitudeAtPoint(ptNew, ground);
		ground += 0.5;
		if (ptNew.y < ground)
			ptNew.y = ground;

		pWireMesh->AddVertex(ptNew);
	}
}


///////////////////////////////////////////////////////////////////////////////

vtUtilityMap3d::vtUtilityMap3d()
{
	m_pTopGroup = NULL;
}

vtGroup *vtUtilityMap3d::Setup()
{
	m_pTopGroup = new vtGroup;
	m_pTopGroup->setName("UtilityMap");

	CreateMaterials();

	return m_pTopGroup;
}

vtPole *vtUtilityMap3d::NewPole()
{
	return new vtPole3d;
}

vtLine *vtUtilityMap3d::NewLine()
{
	vtLine3d *line = new vtLine3d;

	// Connect the line to the map
	m_pTopGroup->addChild(line->GetGeom());
	line->GetGeom()->SetMaterials(m_pMaterials);

	return line;
}

vtPole3d *vtUtilityMap3d::AddPole(const DPoint2 &epos, const char *structname)
{
	vtPole3d *pole = (vtPole3d *) AddNewPole();
	pole->m_p = epos;
	pole->m_sStructName = structname;

	return pole;
}

vtLine3d *vtUtilityMap3d::AddLine()
{
	return (vtLine3d *) AddNewLine();
}

/**
 Find the util pole which is closest to the given point, if it is within
 'error' distance.  The pole and distance are returned by reference.
 */
bool vtUtilityMap3d::FindClosestUtilPole(const DPoint2 &point, double error,
	vtPole3d* &found_pole, double &closest) const
{
	found_pole = NULL;

	if (m_Poles.empty())
		return false;

	closest = 1E8;
	for (uint i = 0; i < NumPoles(); i++)
	{
		vtPole3d *pole = GetPole(i);

		const double dist = (pole->m_p - point).Length();
		if (dist > error)
			continue;
		if (dist < closest)
		{
			found_pole = pole;
			closest = dist;
		}
	}
	return (found_pole != NULL);
}

void vtUtilityMap3d::BuildGeometry(vtLodGrid *pLodGrid, vtHeightField3d *pHeightField)
{
	// generate the structures (poles/towers/etc.)
	for (uint i = 0; i < NumPoles(); i++)
	{
		if (GetPole(i)->CreateGeometry(pHeightField))
			pLodGrid->AddToGrid(GetPole(i)->m_pTrans);
	}

	for (uint i = 0; i < NumLines(); i++)
	{
		GetLine(i)->CreateGeometry(pHeightField, m_mi_wire);
	}
}

bool vtUtilityMap3d::FindPoleFromNode(osg::Node *pNode, int &iPoleIndex) const
{
	iPoleIndex = -1;

	for (uint i = 0; i < NumPoles(); i++)
	{
		if (FindAncestor(pNode, GetPole(i)->m_pTrans))
		{
			iPoleIndex = i;
			break;
		}
	}
	return (iPoleIndex != -1);
}

void vtUtilityMap3d::ComputePoleRotations()
{
	for (uint i = 0; i < NumLines(); i++)
	{
		GetLine(i)->ComputePoleRotations();
	}
}

void vtUtilityMap3d::ComputePoleStructures()
{
	// From the OSM tags for each pole, try to determine an appropriate pole
	// structure to use, from our limited number of tower models.
	for (uint i = 0; i < NumPoles(); i++)
	{
		vtPole3d *pole = GetPole(i);
		for (uint j = 0; j < pole->NumTags(); j++)
		{
			vtTag *tag = pole->GetTag(j);

			if (tag->name == "design" && tag->value == "barrel")
				pole->m_sStructName = "Lattice Tower";

			if (tag->name == "design" && tag->value == "h-frame")
				pole->m_sStructName = "H-Frame Tangent";

			if (tag->name == "design" && tag->value == "h-frame_one-level")
				pole->m_sStructName = "H-Frame Tangent";

			if (tag->name == "tower:type" && tag->value == "termination")
			{
				if (pole->m_sStructName == "Lattice Tower")
					pole->m_sStructName = "Lattice Deadend";

				if (pole->m_sStructName == "H-Frame Tangent")
					pole->m_sStructName = "H-Frame Hold Down";
			}
		}
		// If we haven't matched anything, fall back on a default.
		if (pole->m_sStructName == "")
			pole->m_sStructName = "Steel Poles Tangent";
	}
}

void vtUtilityMap3d::CreateMaterials()
{
	m_pMaterials = new vtMaterialArray;

	// add wire material (0)
	m_mi_wire = m_pMaterials->AddRGBMaterial(RGBf(0.0f, 0.0f, 0.0f), // diffuse
		false, true, false,		// culling, lighting, wireframe
		1.0f);					// alpha
	m_pMaterials->at(m_mi_wire)->SetAmbient(RGBf(1.0f, 1.0f, 1.0f)); // bright white
}

