//
// Fence3d.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef FENCE3DH
#define FENCE3DH

#include "vtdata/Fence.h"
#include "vtdata/HeightField.h"
#include "Structure3d.h"

/** \addtogroup struct */
/*@{*/

/**
 * This class extends vtFence with the abiliity to construct and
 * operate on a 3D model of a linear structore (parametric fence or wall.)
 */
class vtFence3d : public vtFence, public vtStructure3d
{
public:
	vtFence3d();

	// implement vtStructure3d methods
	virtual bool CreateNode(vtTerrain *pTerr);
	virtual bool IsCreated();
	virtual vtGeode *GetGeom() { return m_pFenceGeom; }
	virtual osg::Node *GetContained() { return m_pFenceGeom; }
	virtual void DeleteNode();
	virtual void ShowBounds(bool bShow);

	// implement vtFence methods
	virtual void SetParams(const vtLinearParams &params);
	virtual void ProfileChanged();

	void DestroyGeometry();

protected:
	void	Init();
	void	LoadProfile();
	void	AddFencepost(const FPoint3 &p1, int iMatIdx);

	void AddWireMeshes(const FLine3 &p3);
	void AddFlatConnectionMesh(const FLine3 &p3);
	void AddThickConnectionMesh(const FLine3 &p3);
	void AddProfileConnectionMesh(const FLine3 &p3);
	void AddPostExtensions(const FLine3 &p3);

	void	AddFenceMeshes(vtHeightField3d *pHeightField);

	vtGeodePtr	m_pFenceGeom;
	vtMesh		*m_pHighlightMesh;	// The wireframe highlight
	bool		m_bBuilt;
	FLine3		m_Posts3d;
	float		m_fMaxGroundY;
	FLine2		m_Profile;
};

typedef class vtFence *vtFencePtr;

/*@}*/	// Group struct

#endif	// FENCE3DH
