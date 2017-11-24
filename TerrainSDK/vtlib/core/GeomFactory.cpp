//
// GeomFactory.cpp
//
// Useful classes and functions for working with geometry and meshes.
//
// Copyright (c) 2001-2015 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "GeomFactory.h"
#include "vtdata/HeightField.h"

//////////////////////////////////////////////////////////////

/**
* Constructor.
*
* \param pGeode	The geometry node which will receive the mesh object(s)
*		that this factory will produce.
* \param ePrimType	The type of mesh to produce.
* \param iVertType	The vertex attributes for the meshes to produce.
* \param iMaxVertsPerMesh	The largest number of vertices to allow in a single
*		mesh.  When this number is exceeded, the current mesh will be finished
*		and another mesh begun.
* \param iMatIndex	The material index of the mesh when it is added to the
*		geometry node.
* \param iExpectedVerts If you know how many vertices will be mesh ahead of
*		time, you can save a little time and memory by passing the number.
*/
vtGeomFactory::vtGeomFactory(vtGeode *pGeode, vtMesh::PrimType ePrimType,
	int iVertType, int iMaxVertsPerMesh, int iMatIndex,
	int iExpectedVerts)
{
	m_pGeode = pGeode;
	m_ePrimType = ePrimType;
	m_iVertType = iVertType;
	m_iMaxVertsPerMesh = iMaxVertsPerMesh;
	m_iMatIndex = iMatIndex;

	if (iExpectedVerts == -1)
		m_iExpectedVerts = m_iMaxVertsPerMesh;
	else
		m_iExpectedVerts = iExpectedVerts;

	m_pMesh = NULL;
	m_iPrimStart = -1;
	m_iPrimVerts = -1;
	m_fLineWidth = 1;

	m_bSimple = false;
}

/**
* Alternate, simpler constructor.
*
* \param pMesh The mesh which will receive all the vertices that this factory
*		produces.
*/
vtGeomFactory::vtGeomFactory(vtMesh *pMesh)
{
	m_pGeode = NULL;
	m_pMesh = pMesh;
	m_iPrimStart = -1;
	m_iPrimVerts = -1;
	m_fLineWidth = 1;

	m_bSimple = true;
}

void vtGeomFactory::NewMesh()
{
	m_pMesh = new vtMesh(m_ePrimType, m_iVertType, m_iExpectedVerts);
	m_pGeode->AddMesh(m_pMesh, m_iMatIndex);

	if (m_fLineWidth != 1)
		m_pMesh->SetLineWidth(m_fLineWidth);

	// Keep a list of all the meshes made in this factory
	m_Meshes.push_back(m_pMesh);
}

/** Tell the factory to start a primitive. */
void vtGeomFactory::PrimStart()
{
	if (!m_pMesh)
		NewMesh();
	m_iPrimStart = m_pMesh->NumVertices();
	m_iPrimVerts = 0;
}

/** Tell the factory to add a vertex to the current primitive. */
void vtGeomFactory::AddVertex(const FPoint3 &p)
{
	if (!m_bSimple)
	{
		int count = m_pMesh->NumVertices();
		if (count == m_iMaxVertsPerMesh)
		{
			// repeat vertex; it needs to appear in both meshes
			m_pMesh->AddVertex(p);
			m_iPrimVerts++;

			// close that primitive and start another on a new mesh
			PrimEnd();
			NewMesh();
			PrimStart();
		}
	}
	m_pMesh->AddVertex(p);
	m_iPrimVerts++;
}

/** Tell the factory to end a primitive. */
void vtGeomFactory::PrimEnd()
{
	if (m_iPrimVerts > 0)
		m_pMesh->AddStrip2(m_iPrimVerts, m_iPrimStart);
	m_iPrimStart = -1;
	m_iPrimVerts = -1;
}

void vtGeomFactory::SetLineWidth(float width)
{
	m_fLineWidth = width;
}

void vtGeomFactory::SetMatIndex(int iIdx)
{
	if (iIdx != m_iMatIndex)
	{
		// Material is changing, we must start a new mesh
		PrimEnd();
		m_iMatIndex = iIdx;
		NewMesh();
		PrimStart();
	}
}

/**
* Create geometry for a 2D line by draping the points on a heightfield.
*
* \param pHF	The heightfield to drape on.
* \param line	The 2D line to drape, in Earth coordinates.
* \param fSpacing	The approximate spacing of the surface tessellation, used to
*		decide how finely to tessellate the line.
* \param fOffset	An offset to elevate each point in the resulting geometry,
*		useful for keeping it visibly above the ground.
* \param bInterp	True to interpolate between the vertices of the input
*		line. This is generally desirable when the ground is much more finely
*		spaced than the input line.
* \param bCurve	True to interpret the vertices of the input line as
*		control points of a curve.  The created geometry will consist of
*		a draped line which passes through the control points.
* \param bTrue		True to use the true elevation of the terrain, ignoring
*		whatever scale factor is being used to exaggerate elevation for
*		display.
* \return The approximate length of the resulting 3D line mesh.
*
* \par Example:
\code
DLine2 line = ...;
vtTerrain *pTerr = ...;
vtGeode *pLineGeom = new vtGeode;
pTerr->AddNode(pLineGeom);
vtGeomFactory mf(pLineGeom, osg::PrimitiveSet::LINE_STRIP, 0, 30000, 1);
float length = mf.AddSurfaceLineToMesh(pTerr->GetHeightfield(), dline, 1, 10, true);
\endcode
*/
float vtGeomFactory::AddSurfaceLineToMesh(vtHeightField3d *pHF,
	const DLine2 &line, float fSpacing, float fOffset, bool bInterp, bool bCurve, bool bTrue)
{
	FLine3 tessellated;
	const float fTotalLength = pHF->LineOnSurface(line, fSpacing, fOffset, bInterp,
		bCurve, bTrue, tessellated);

	PrimStart();
	for (uint i = 0; i < tessellated.GetSize(); i++)
		AddVertex(tessellated[i]);
	PrimEnd();

	return fTotalLength;
}


