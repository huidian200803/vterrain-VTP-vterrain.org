//
// UtilityMap3d.h
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTLIB_CORE_UTILITYMAP3D_H
#define VTLIB_CORE_UTILITYMAP3D_H

#include "vtdata/UtilityMap.h"

class vtHeightField3d;
class vtMesh;
class vtUtilStruct;

/**
 * This class represents a 'node' in a network of utility structures.
 * Generally this will have a physical structure associated with it,
 * the type of which is described by the class vtUtilStruct.
 */
class vtPole3d : public vtPole
{
public:
	vtPole3d();
	bool CreateGeometry(const vtHeightField3d *pHF);
	void DestroyGeometry();

	double dRadAzimuth;				// azimuth (orientation to next node)
	vtUtilStruct *m_pUtilStruct;	// Utility Structure to use.
	vtString m_sStructName;			// Structure Family

	vtTransformPtr m_pTrans;
};

/**
 * A vtRoute is a set of nodes (vtPole3d) describing the path of
 * utility components (such as a power line).
 */
class vtLine3d : public vtLine
{
public:
	vtLine3d();

	void CreateGeometry(vtHeightField3d *pHeightField, int matidx);
	void DestroyGeometry();

	vtPole3d *GetPole(uint iter) const
	{
		return (iter < m_poles.size() ? (vtPole3d*) m_poles[iter] : NULL);
	}
	vtGeode *GetGeom() const { return m_pWireGeom; }
	void ComputePoleRotations();

protected:
	void CreateCatenaryWire(int iPoleIndex, vtHeightField3d *pHeightField, int matidx);
	void _DrawCat(vtHeightField3d *pHeightField, const FPoint3 &pt0,
		const FPoint3 &pt1, double catenary, int iNumSegs, vtMesh *pWireMesh);

	vtGeode	*m_pWireGeom;

	bool m_bBuilt;	// True when meshes are built.
};

/**
 * vtUtilityMap3d is a container for a set of vtRoute objects.
 */
class vtUtilityMap3d : public vtUtilityMap
{
public:
	vtUtilityMap3d();

	vtGroup *Setup();

	virtual vtPole *NewPole();
	virtual vtLine *NewLine();

	vtPole3d *GetPole(uint iter) const
	{
		return (iter < m_Poles.size() ? (vtPole3d*) m_Poles[iter] : NULL);
	}
	vtLine3d *GetLine(uint iter) const
	{
		return (iter < m_Lines.size() ? (vtLine3d*) m_Lines[iter] : NULL);
	}

	vtPole3d *AddPole(const DPoint2 &epos, const char *structname);
	vtLine3d *AddLine();

	bool FindClosestUtilPole(const DPoint2 &point, double error,
		vtPole3d* &found_pole, double &closest) const;
	void BuildGeometry(vtLodGrid *pLodGrid, vtHeightField3d *pHeightField);
	bool FindPoleFromNode(osg::Node *pNode, int &iPoleIndex) const;
	void ComputePoleRotations();
	void ComputePoleStructures();

protected:
	vtGroup *m_pTopGroup;

	void CreateMaterials();

	// all lines share the same set of materials
	vtMaterialArray *m_pMaterials;
	int m_mi_wire;
};

#endif  // VTLIB_CORE_UTILITYMAP3D_H
