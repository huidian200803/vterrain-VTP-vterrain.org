//
// GeomFactory.h
//
// Helper class for working with geometry and meshes.
//
// Copyright (c) 2001-2015 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

/** \addtogroup sg */
/*@{*/

/**
 * The vtGeomFactory class makes it easy to create geometry with a lot
 * of vertices.  You simply provide vertices, and it will create as
 * many geometries as necessary to contain them all.
 *
 * \par Example:
	This example produces a line strip with 10000 vertices.  The factory
	is told to limit the number of vertices in a single primitive to 3000,
	so it will automatically produce four geometries with 3000, 3000, 3000, and
	1000 vertices each.  The geometries are automatically added to the indicated
	geode.
	\code
	{
		vtGeode *pLineGeom = new vtGeode;
		vtGeomFactory mf(pLineGeom, osg::PrimitiveSet::LINE_STRIP, 0, 3000, 1);
		mf.PrimStart();
		for (int i = 0; i < 10000; i++)
			mf.AddVertex(FPoint3(i,i,i));
		mf.PrimEnd();
	}
	\endcode
 */
class vtGeomFactory
{
public:
	vtGeomFactory(vtGeode *pGeode, vtMesh::PrimType ePrimType,
		int iVertType, int iMaxVertsPerMesh, int iMatIndex, int iExpectedVerts = -1);
	vtGeomFactory(vtMesh *pMesh);

	void PrimStart();
	void AddVertex(const FPoint3 &p);
	void PrimEnd();

	void SetLineWidth(float width);
	void SetMatIndex(int iIdx);

	float AddSurfaceLineToMesh(vtHeightField3d *pHF, const DLine2 &line,
		float fSpacing, float fOffset, bool bInterp = true, bool bCurve = false,
		bool bTrue = false);

	uint Meshes() { return m_Meshes.size(); }
	vtMesh *Mesh(int index) { return m_Meshes[index]; }

protected:
	void NewMesh();

	std::vector<vtMesh*> m_Meshes;
	vtGeode *m_pGeode;
	vtMesh::PrimType m_ePrimType;
	int m_iVertType;
	int m_iMaxVertsPerMesh;
	int m_iExpectedVerts;
	int m_iMatIndex;

	vtMesh *m_pMesh;
	int m_iPrimStart;
	int m_iPrimVerts;
	float m_fLineWidth;

	bool m_bSimple;
};

/*@}*/	// Group sg
