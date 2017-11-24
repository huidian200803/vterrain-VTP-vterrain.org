//
// LodGrid.cpp
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/LocalCS.h"
#include "vtdata/HeightField.h"
#include "vtdata/vtString.h"

#include "LodGrid.h"

#define CellIndex(a,b) ((a*m_dim)+b)

/////////////////////////////////////////////////////////////////////////////
// Base LOD Grid

vtLodGrid::vtLodGrid()
{
	m_dim = 100;
	m_fLODDistance = 0.0f;
	m_pHeightField = NULL;
}

void vtLodGrid::DetermineCell(const FPoint3 &pos, int &a, int &b)
{
	a = (int) ((pos.x - m_origin.x) / m_step.x);
	b = (int) ((pos.z - m_origin.z) / m_step.z);
}

bool vtLodGrid::AddToGrid(osg::Node *pNode)
{
	osg::BoundingSphere sph = pNode->getBound();

	osg::Group *pGroup = FindCellParent(s2v(sph.center()));
	if (pGroup)
	{
		pGroup->addChild(pNode);
		return true;
	}
	else
		return false;
}

void vtLodGrid::RemoveFromGrid(osg::Node *pNode)
{
	osg::Group *pGroup = FindCellParent(s2v(pNode->getBound().center()));
	if (pGroup && pGroup->getChildIndex(pNode) != pGroup->getNumChildren())
		pGroup->removeChild(pNode);
	else
	{
		/*
		 * Search through all of the LOD grid's cells looking
		 * for the node, so we can handle even cases where the object may have
		 * moved out of its original cell.
		 */
		int a, b;
		for (a = 0; a < m_dim; a++)
		{
			for (b = 0; b < m_dim; b++)
			{
				osg::Group *group = GetCell(a, b);
				if (group && group->getChildIndex(pNode) != group->getNumChildren())
				{
					group->removeChild(pNode);
					return;
				}
			}
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// Simple LOD Grid

vtSimpleLodGrid::vtSimpleLodGrid()
{
	m_pCells = NULL;
}

vtSimpleLodGrid::~vtSimpleLodGrid()
{
	// free our pointers to children; they are removed separately
	free(m_pCells);
	m_pCells = NULL;
}

void vtSimpleLodGrid::Setup(const FPoint3 &origin, const FPoint3 &size,
				 int iDimension, float fLODDistance, vtHeightField3d *pHF)
{
	m_origin = origin;
	m_size = size;
	m_dim = iDimension;
	m_fLODDistance = fLODDistance;
	m_step = m_size / (float)m_dim;

	// wrap with an array of simple LOD nodes
	m_pCells = (vtLOD **)malloc(m_dim * m_dim * sizeof(vtLOD *));

	int a, b;
	for (a = 0; a < m_dim; a++)
	{
		for (b = 0; b < m_dim; b++)
		{
			m_pCells[CellIndex(a,b)] = NULL;
		}
	}
	m_pHeightField = pHF;
}

void vtSimpleLodGrid::AllocateCell(int a, int b)
{
	int i = CellIndex(a,b);

	m_pCells[i] = new vtLOD;
	vtString name;
	name.Format("LOD cell %d %d", a, b);
	m_pCells[i]->setName(name);

	// The child of this LOD is only visible up to LODDistance away
	m_pCells[i]->setRange(0, 0.0f, m_fLODDistance);

	// determine LOD center
	FPoint3 lod_center;
	lod_center.x = m_origin.x + ((m_size.x / m_dim) * (a + 0.5f));
	lod_center.y = m_origin.y + (m_size.y / 2.0f);
	lod_center.z = m_origin.z + ((m_size.z / m_dim) * (b + 0.5f));
	if (m_pHeightField)
		m_pHeightField->FindAltitudeAtPoint(lod_center, lod_center.y);

	vtGroup *group = new vtGroup;
	group->setName("LOD group");
	m_pCells[i]->SetCenter(lod_center);
	m_pCells[i]->addChild(group);

	addChild(m_pCells[i]);
}


osg::Group *vtSimpleLodGrid::GetCell(int a, int b)
{
	int i = CellIndex(a, b);
	vtLOD *pCell = m_pCells[i];
	if (!pCell)
		return NULL;
	return pCell;
}

osg::Group *vtSimpleLodGrid::FindCellParent(const FPoint3 &point)
{
	int a, b;

	DetermineCell(point, a, b);
	if (a < 0 || a >= m_dim || b < 0 || b >= m_dim)
		return NULL;

	int i = CellIndex(a, b);
	if (!m_pCells[i])
		AllocateCell(a, b);

	return (osg::Group *) (m_pCells[i]->getChild(0));
}

void vtSimpleLodGrid::SetDistance(float fLODDistance)
{
	m_fLODDistance = fLODDistance;

	for (int a = 0; a < m_dim; a++)
	{
		for (int b = 0; b < m_dim; b++)
		{
			vtLOD *lod = m_pCells[CellIndex(a,b)];
			if (lod)
				lod->setRange(0, 0.0f, m_fLODDistance);
		}
	}
}

