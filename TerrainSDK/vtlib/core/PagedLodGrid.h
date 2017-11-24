//
// PagedLodGrid.h
//
// Copyright (c) 2007-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef PAGEDLODGRIDH
#define PAGEDLODGRIDH

#include "LodGrid.h"

class vtStructure;
class vtStructure3d;
class vtStructureArray3d;

/*
 Implementation scene graph:
			A
		   / \
		  B   B
		 /|\  |\
		C C C C C

 A = vtPagedStructureLodGrid, contans an array of cells consisting of:
 B = vtPagedStructureLOD, which has any number of:
 C = vtStructure/vtStructure3d, which produces a vtTransform when built.
 */
class vtPagedStructureLodGrid;

struct StructureRef {
	vtStructureArray3d *pArray;
	uint iIndex;
};
typedef std::vector<StructureRef> StructureRefVector;

/**
 * A vtPagedStructureLOD node controls the visibility of its child nodes.
 *
 * You set a single distance value (range) for all the children, which
 * is the distance from the camera at which all nodes will be rendered.
 */
class vtPagedStructureLOD : public vtLOD
{
public:
	vtPagedStructureLOD();

	void SetRange(float range);
	bool TestVisible(float fDistance, bool bLoad);

	void Add(vtStructureArray3d *pArray, int iIndex);
	void Remove(vtStructureArray3d *pArray, int iIndex);
	void SetGrid(vtPagedStructureLodGrid *g) { m_pGrid = g; }
	void AppendToQueue();

	StructureRefVector m_StructureRefs;
	int m_iNumConstructed;
	bool m_bAddedToQueue;

	// Implement OSG's traversal with our own logic
	virtual void traverse(osg::NodeVisitor& nv)
	{
		switch(nv.getTraversalMode())
		{
		case(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN):
			std::for_each(_children.begin(),_children.end(),osg::NodeAcceptOp(nv));
			break;
		case(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN):
			{
				// 'Active' children are those within the given distance
				float distance = nv.getDistanceToEyePoint(getCenter(),true);

				// _visitorType might be NODE_VISITOR (in cases such as
				//  intersection testing) or CULL_VISITOR (during rendering).
				//  We only want do visibility testing / page loading during
				//  rendering.

				// Test distance and contruct geometry if needed
				if (TestVisible(distance,
					nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR))
				{
					// Tell OSG to traverse all children
					std::for_each(_children.begin(),_children.end(),osg::NodeAcceptOp(nv));
				}
				break;
			}
		default:
			break;
		}
	}

protected:
	float m_fRange;
	virtual ~vtPagedStructureLOD() {}

	// Pointer up to container
	vtPagedStructureLodGrid *m_pGrid;
};

struct QueueEntry {
	vtPagedStructureLOD *pLOD;
	vtStructureArray3d *pStructureArray;
	uint iStructIndex;
	float fDistance;
};
typedef std::vector<QueueEntry> QueueVector;

/**
 * vtPagedStructureLodGrid provides a more complex implementation of vtLodGrid.
 *
 * Like vtSimpleLodGrid, it consists of a 2D grid of LOD nodes, each covering
 * a rectangular cell, which causes that cell's children to only be shown
 * when within a given distance.  Additionally, the cells can contain
 * structures (vtStructure) which are not constructed until the cell is
 * visible.
 */
class vtPagedStructureLodGrid : public vtLodGrid
{
public:
	vtPagedStructureLodGrid();
	void Setup(const FPoint3 &origin, const FPoint3 &size,
		int iDimension, float fLODDistance, vtHeightField3d *pHF = NULL);
	void Cleanup();

	// methods
	void SetDistance(float fLODDistance);
	bool AppendToGrid(vtStructureArray3d *sa, int iIndex);
	void RemoveFromGrid(vtStructureArray3d *sa, int iIndex);

	vtPagedStructureLOD *GetPagedCell(int a, int b);

	void DoPaging(const FPoint3 &CamPos, int iMaxStructures, float fDeleteDistance);
	bool AddToQueue(vtPagedStructureLOD *pLOD, vtStructureArray3d *pArray, int iIndex);
	bool RemoveFromQueue(vtStructureArray3d *pArray, int iIndex);
	uint GetQueueSize() { return m_Queue.size(); }
	void SortQueue();
	void ClearQueue(vtStructureArray3d *pArray);
	void RefreshPaging(vtStructureArray3d *pArray);

	void EnableLoading(bool b) { m_LoadingEnabled = b; }
	bool m_LoadingEnabled;

	int GetLoadCount() { return m_iLoadCount; }
	void ResetLoadCount() { m_iLoadCount = 0; }
	int GetTotalConstructed() { return m_iTotalConstructed; }

	vtPagedStructureLOD *FindGroup(vtStructure *str);
	void ConstructByIndex(vtPagedStructureLOD *pLOD, vtStructureArray3d *pArray,
		uint iStructIndex);

protected:
	void CullFarawayStructures(const FPoint3 &CamPos,
		int iMaxStructures, float fDistance);
	void DeconstructCell(vtPagedStructureLOD *pLOD);
	void RemoveCellFromQueue(vtPagedStructureLOD *pLOD);

	vtPagedStructureLOD **m_pCells;
	int m_iLoadCount, m_iTotalConstructed;

	osg::Group *FindCellParent(const FPoint3 &point);
	vtPagedStructureLOD *FindPagedCellParent(const FPoint3 &point);
	void AllocateCell(int a, int b);
	osg::Group *GetCell(int a, int b);

	QueueVector m_Queue;
};

#endif // PAGEDLODGRIDH

