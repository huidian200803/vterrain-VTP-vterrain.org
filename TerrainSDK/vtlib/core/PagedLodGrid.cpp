//
// PagedLodGrid.cpp
//
// Copyright (c) 2007-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "Structure3d.h"

#include "vtdata/LocalCS.h"
#include "vtdata/HeightField.h"
#include "vtdata/vtLog.h"

#include "PagedLodGrid.h"

#include <algorithm>	// for sort

vtPagedStructureLOD::vtPagedStructureLOD() : vtLOD()
{
	m_iNumConstructed = 0;
	m_bAddedToQueue = false;

	SetCenter(FPoint3(0, 0, 0));

	SetOsgNode(this);
}

void vtPagedStructureLOD::SetRange(float range)
{
	m_fRange = range;
}

/**
 * \param fDistance The distance in meters to check against.
 * \param bLoad If true, and this cell is within the distance, and it isn't
 *		loaded, then load it.
 */
bool vtPagedStructureLOD::TestVisible(float fDistance, bool bLoad)
{
	if (fDistance < m_fRange)
	{
		// Check if this group has any unbuilt structures
		if (bLoad && !m_bAddedToQueue &&
			m_iNumConstructed != m_StructureRefs.size() &&
			m_pGrid->m_LoadingEnabled)
		{
			AppendToQueue();
			m_bAddedToQueue = true;
		}
		return true;
	}
	return false;
}

void vtPagedStructureLOD::AppendToQueue()
{
	int count = 0;
	for (uint i = 0; i < m_StructureRefs.size(); i++)
	{
		StructureRef &ref = m_StructureRefs[i];

		// Don't queue structures from layers that aren't visible
		if (ref.pArray->GetEnabled() == false)
			continue;

		if (m_pGrid->AddToQueue(this, ref.pArray, ref.iIndex))
			count++;
	}
	if (count > 0)
		VTLOG("Added %d buildings to queue.\n", count);

	// We have just added a lump of structures, sort them by distance
	m_pGrid->SortQueue();
}

void vtPagedStructureLOD::Add(vtStructureArray3d *pArray, int iIndex)
{
	StructureRef ref;
	ref.pArray = pArray;
	ref.iIndex = iIndex;
	m_StructureRefs.push_back(ref);
}

void vtPagedStructureLOD::Remove(vtStructureArray3d *pArray, int iIndex)
{
	StructureRefVector::iterator it = m_StructureRefs.begin();
	while (it != m_StructureRefs.end())
	{
		if (it->pArray == pArray && it->iIndex == iIndex)
		{
			vtStructure3d *s3d = it->pArray->GetStructure3d(it->iIndex);
			if (s3d->IsCreated())
				m_iNumConstructed--;
			it = m_StructureRefs.erase(it);
		}
		else
			it++;
	}
}


///////////////////////////////////////////////////////////////////////
// vtPagedStructureLodGrid

#define CellIndex(a,b) ((a*m_dim)+b)

vtPagedStructureLodGrid::vtPagedStructureLodGrid()
{
	m_pCells = NULL;
	m_LoadingEnabled = true;
	m_iLoadCount = 0;
}

void vtPagedStructureLodGrid::Setup(const FPoint3 &origin, const FPoint3 &size,
				 int iDimension, float fLODDistance, vtHeightField3d *pHF)
{
	m_origin = origin;
	m_size = size;
	m_dim = iDimension;
	m_fLODDistance = fLODDistance;
	m_step = m_size / (float)m_dim;

	// wrap with an array of simple LOD nodes
	m_pCells = (vtPagedStructureLOD **)malloc(m_dim * m_dim * sizeof(vtPagedStructureLOD *));

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

void vtPagedStructureLodGrid::Cleanup()
{
	// get rid of children first
	removeChildren(0, getNumChildren());

	// free all our pointers to them
	free(m_pCells);
	m_pCells = NULL;
}


void vtPagedStructureLodGrid::AllocateCell(int a, int b)
{
	int i = CellIndex(a,b);

	m_pCells[i] = new vtPagedStructureLOD;
	vtString name;
	name.Format("LOD cell %d %d", a, b);
	m_pCells[i]->setName(name);

	m_pCells[i]->SetRange(m_fLODDistance);

	// determine LOD center
	FPoint3 lod_center;
	lod_center.x = m_origin.x + ((m_size.x / m_dim) * (a + 0.5f));
	lod_center.y = m_origin.y + (m_size.y / 2.0f);
	lod_center.z = m_origin.z + ((m_size.z / m_dim) * (b + 0.5f));
	if (m_pHeightField)
		m_pHeightField->FindAltitudeAtPoint(lod_center, lod_center.y);
	m_pCells[i]->SetCenter(lod_center);

	// and a radius to give the LOD a bounding sphere, for efficient culling
	FPoint2 diag(m_size.x / m_dim, m_size.z / m_dim);
	float diagonal = diag.Length();
	float radius = diagonal/2;

	// Increase it a little, because some structures might visually extend outside
	//  the minimal bounding sphere
	radius *= 1.6f;

	m_pCells[i]->setRadius(radius);
	m_pCells[i]->SetGrid(this);

	addChild(m_pCells[i]);
}

osg::Group *vtPagedStructureLodGrid::GetCell(int a, int b)
{
	int i = CellIndex(a, b);
	return m_pCells[i];
}

vtPagedStructureLOD *vtPagedStructureLodGrid::FindPagedCellParent(const FPoint3 &point)
{
	int a, b;

	DetermineCell(point, a, b);
	if (a < 0 || a >= m_dim || b < 0 || b >= m_dim)
		return NULL;

	const int i = CellIndex(a, b);
	if (!m_pCells[i])
		AllocateCell(a, b);

	return m_pCells[i];
}

osg::Group *vtPagedStructureLodGrid::FindCellParent(const FPoint3 &point)
{
	return FindPagedCellParent(point);
}

void vtPagedStructureLodGrid::SetDistance(float fLODDistance)
{
	m_fLODDistance = fLODDistance;

	for (int a = 0; a < m_dim; a++)
	{
		for (int b = 0; b < m_dim; b++)
		{
			vtPagedStructureLOD *lod = m_pCells[CellIndex(a,b)];
			if (lod)
				lod->SetRange(m_fLODDistance);
		}
	}
}

/**
 * For a given vtStructure, find the lod group parent for it, using the
 * structure's earth extents.
 */
vtPagedStructureLOD *vtPagedStructureLodGrid::FindGroup(vtStructure *str)
{
	DRECT rect;
	if (str->GetExtents(rect))
	{
		float xmin, xmax, zmin, zmax;
		m_pHeightField->m_LocalCS.EarthToLocal(rect.left, rect.bottom, xmin, zmin);
		m_pHeightField->m_LocalCS.EarthToLocal(rect.right, rect.top, xmax, zmax);

		const FPoint3 mid((xmin+xmax) / 2, 0.0f, (zmin+zmax)/2);

		return FindPagedCellParent(mid);
	}
	return NULL;
}

bool vtPagedStructureLodGrid::AppendToGrid(vtStructureArray3d *pArray, int iIndex)
{
	// Get 2D extents from the unbuild structure
	vtStructure *str = pArray->at(iIndex);
	vtPagedStructureLOD *pGroup = FindGroup(str);
	if (pGroup)
	{
		pGroup->Add(pArray, iIndex);
		return true;
	}
	return false;
}

void vtPagedStructureLodGrid::RemoveFromGrid(vtStructureArray3d *pArray, int iIndex)
{
	// Get 2D extents from the unbuild structure
	vtStructure *str = pArray->at(iIndex);
	vtPagedStructureLOD *pGroup = FindGroup(str);
	if (pGroup)
		pGroup->Remove(pArray, iIndex);
}

vtPagedStructureLOD *vtPagedStructureLodGrid::GetPagedCell(int a, int b)
{
	return m_pCells[CellIndex(a,b)];
}

void vtPagedStructureLodGrid::DeconstructCell(vtPagedStructureLOD *pLOD)
{
	int count = 0;

	StructureRefVector &refs = pLOD->m_StructureRefs;
	//VTLOG("Deconstruction check on %d structures: ", indices.GetSize());
	for (uint i = 0; i < refs.size(); i++)
	{
		StructureRef &ref = refs[i];
		vtStructure3d *str3d = ref.pArray->GetStructure3d(ref.iIndex);
		osg::Node *node = str3d->GetContainer();
		if (!node)
			node = str3d->GetGeom();
		if (!node)
			continue;
		pLOD->removeChild(node);
		str3d->DeleteNode();
		count++;
	}
	//VTLOG("%d decon.\n", count);
	pLOD->m_iNumConstructed = 0;
	pLOD->m_bAddedToQueue = false;
}

void vtPagedStructureLodGrid::RemoveCellFromQueue(vtPagedStructureLOD *pLOD)
{
	if (!pLOD->m_bAddedToQueue)
		return;
	if (pLOD->m_iNumConstructed == pLOD->m_StructureRefs.size())
		return;

	const StructureRefVector &refs = pLOD->m_StructureRefs;
	int count = 0;
	for (uint i = 0; i < refs.size(); i++)
	{
		if (RemoveFromQueue(refs[i].pArray, refs[i].iIndex))
			count++;
	}
	if (count != 0)
		VTLOG("Dequeued %d of %d.\n", count, refs.size());
	pLOD->m_bAddedToQueue = false;
}

void vtPagedStructureLodGrid::CullFarawayStructures(const FPoint3 &CamPos,
													int iMaxStructures,
													float fDistance)
{
	m_iTotalConstructed = 0;
	for (int a = 0; a < m_dim; a++)
	{
		for (int b = 0; b < m_dim; b++)
		{
			vtPagedStructureLOD *lod = m_pCells[CellIndex(a,b)];
			if (lod) m_iTotalConstructed += lod->getNumChildren();
		}
	}
	// If we have too many or have items in the queue
	if (m_iTotalConstructed > iMaxStructures || m_Queue.size() > 0)
	{
		//VTLOG("CullFarawayStructures: %d in Queue, ", m_Queue.size());
		int total = 0, removed = 0;
		// Delete/dequeue the ones that are very far
		FPoint3 center;
		for (int a = 0; a < m_dim; a++)
		{
			for (int b = 0; b < m_dim; b++)
			{
				vtPagedStructureLOD *lod = m_pCells[CellIndex(a,b)];
				if (!lod)
					continue;

				total++;
				lod->GetCenter(center);
				float dist = (center - CamPos).Length();

				// If very far, delete the structures entirely
				if (lod->m_iNumConstructed != 0 &&
					m_iTotalConstructed > iMaxStructures &&
					dist > fDistance)
					DeconstructCell(lod);

				// If it has fallen out of the frustum, remove them
				//  from the queue
				if (dist > m_fLODDistance)
				{
					RemoveCellFromQueue(lod);
					removed++;
				}
			}
		}
		//VTLOG(" %d cells, %d cell removed\n", total, removed);
	}
}

bool operator<(const QueueEntry& a, const QueueEntry& b)
{
	// Reverse-sort, to put smallest values (closest points) at the end
	//  of the list so they can be efficiently removed
	return a.fDistance > b.fDistance;
}

void vtPagedStructureLodGrid::SortQueue()
{
	vtCamera *cam = vtGetScene()->GetCamera();
	FPoint3 CamPos = cam->GetTrans();
	FPoint3 CamDir = cam->GetDirection();

	// Prioritization is by distance.
	// We can measure horizontal distance, which is faster.
	DPoint3 cam_epos, cam_epos2;
	m_pHeightField->m_LocalCS.LocalToEarth(CamPos, cam_epos);
	m_pHeightField->m_LocalCS.LocalToEarth(CamPos+CamDir, cam_epos2);
	DPoint2 cam_pos(cam_epos.x, cam_epos.y);
	DPoint2 cam_dir(cam_epos2.x - cam_epos.x, cam_epos2.y - cam_epos.y);
	cam_dir.Normalize();

	DPoint2 p;
	for (uint i = 0; i < m_Queue.size(); i++)
	{
		QueueEntry &e = m_Queue[i];
		vtStructure *st = e.pStructureArray->at(e.iStructIndex);
		vtBuilding *bld = st->GetBuilding();
		vtStructInstance *inst = st->GetInstance();
		if (bld)
			p = bld->GetOuterFootprint(0).Centroid();
		else if (inst)
			p = inst->GetPoint();
		else
			continue;

		// Calculate distance
		DPoint2 diff = p-cam_pos;
		e.fDistance = (float) diff.Length();

		// Is point behind the camera?  If so, give it lowest priority
		if (diff.Dot(cam_dir) < 0)
			e.fDistance += 1E5;
	}
	std::sort(m_Queue.begin(), m_Queue.end());
}

void vtPagedStructureLodGrid::ClearQueue(vtStructureArray3d *pArray)
{
	QueueVector::iterator it = m_Queue.begin();
	while (it != m_Queue.end())
	{
		if (it->pStructureArray == pArray)
			it = m_Queue.erase(it);
		else
			it++;
	}
}

/**
 * In case the paging grid did not load some structure before (because the
 *  structures were hidden), tell it to check again.
 * You should call this when a structure layer becomes enabled (un-hidden).
 */
void vtPagedStructureLodGrid::RefreshPaging(vtStructureArray3d *pArray)
{
	for (uint i = 0; i < pArray->size(); i++)
	{
		// Get 2D extents from the unbuild structure
		vtStructure *str = pArray->at(i);
		vtPagedStructureLOD *pGroup = FindGroup(str);
		if (pGroup)
			pGroup->m_bAddedToQueue = false;
	}
}

void vtPagedStructureLodGrid::DoPaging(const FPoint3 &CamPos,
									   int iMaxStructures, float fDeleteDistance)
{
	static float last_cull = 0.0f, last_load = 0.0f, last_prioritize = 0.0f;
	float current = vtGetTime();

	if (current - last_prioritize > 1.0f)
	{
		// Do a re-priortization every 1 second.
		SortQueue();
		last_prioritize = current;
	}
	else if (current - last_cull > 0.25f)
	{
		// Do a paging cleanup pass every 1/4 of a second
		// Unload/unqueue anything excessive
		CullFarawayStructures(CamPos, iMaxStructures, fDeleteDistance);
		last_cull = current;
	}
	else if (current - last_load > 0.01f && !m_Queue.empty())
	{
		// Do loading every other available frame
		last_load = current;

		// Check if the camera is not moving; if so, construct more.
		int construct;
		static FPoint3 last_campos;
		if (CamPos == last_campos)
			construct = 5;
		else
			construct = 1;

		for (int i = 0; i < construct && m_Queue.size() > 0; i++)
		{
			// Gradually load anything that needs loading
			const QueueEntry &e = m_Queue.back();
			ConstructByIndex(e.pLOD, e.pStructureArray, e.iStructIndex);
			m_Queue.pop_back();
		}
		last_campos = CamPos;
	}
}

void vtPagedStructureLodGrid::ConstructByIndex(vtPagedStructureLOD *pLOD,
											   vtStructureArray3d *pArray,
											   uint iStructIndex)
{
	bool bSuccess = pArray->ConstructStructure(iStructIndex);
	if (bSuccess)
	{
		vtStructure3d *str3d = pArray->GetStructure3d(iStructIndex);
		vtTransform *pTrans = str3d->GetContainer();
		if (pTrans)
			pLOD->addChild(pTrans);
		else
		{
			vtGeode *pGeode = str3d->GetGeom();
			if (pGeode)
				pLOD->addChild(pGeode);
		}
		pLOD->m_iNumConstructed ++;

		// Keep track of overall number of loads
		m_iLoadCount++;
	}
	else
	{
		VTLOG("Error: couldn't construct index %d\n", iStructIndex);
		vtStructInstance *si = pArray->GetInstance(iStructIndex);
		if (si)
		{
			const char *fname = si->GetValueString("filename", true);
			VTLOG("\tinstance fname: '%s'\n", fname ? fname : "null");
		}
	}
}

bool vtPagedStructureLodGrid::AddToQueue(vtPagedStructureLOD *pLOD,
										 vtStructureArray3d *pArray, int iIndex)
{
	// Check if it's already built
	vtStructure3d *str3d = pArray->GetStructure3d(iIndex);
	if (str3d && str3d->IsCreated())
		return false;

	// Check if it's already in the queue
	for (QueueVector::iterator it = m_Queue.begin(); it != m_Queue.end(); it++)
	{
		if (it->pStructureArray == pArray && it->iStructIndex == iIndex)
			return false;
	}

	// If not, add it
	QueueEntry e;
	e.pLOD = pLOD;
	e.pStructureArray = pArray;
	e.iStructIndex = iIndex;
	e.fDistance = 1E9;
	m_Queue.push_back(e);
	return true;
}

bool vtPagedStructureLodGrid::RemoveFromQueue(vtStructureArray3d *pArray, int iIndex)
{
	// Check if it's in the queue
	for (QueueVector::iterator it = m_Queue.begin(); it != m_Queue.end(); it++)
	{
		if (it->pStructureArray == pArray && it->iStructIndex == iIndex)
		{
			m_Queue.erase(it);
			return true;
		}
	}
	return false;
}

