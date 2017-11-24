//
// GeomUtil.h
//
// Useful classes and functions for working with geometry and meshes.
//
// Copyright (c) 2001-2015 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

/** \addtogroup sg */
/*@{*/

/**
 * The vtDimension class creates a 'dimension' object, which draws an arrow
 * connecting two points in space along with a text label, which is useful
 * for describing the length of the line (or any other string).
 */
class vtDimension : public vtTransform
{
public:
	vtDimension(const FPoint3 &p1, const FPoint3 &p2, float height,
		const RGBf &line_color, const RGBf &text_color,
		osgText::Font *font, const char *message);

	void SetText(const char *text);

	vtGeode *m_pGeode;
	vtMaterialArrayPtr m_pMats;
	vtMesh *m_pLines;
	vtTextMesh *m_pLabel, *m_pLabel2;
};


// Helper functions
vtGeode *Create3DCursor(float fSize, float fSmall, float fAlpha = 0.5f);
vtGeode *CreateBoundSphereGeode(const FSphere &sphere, int res = 24);
vtMesh *CreateSphereMesh(const FSphere &sphere, int res = 24);
vtGeode *CreatePlaneGeom(const vtMaterialArray *pMats, int iMatIdx,
						int Axis1, int Axis2, int Axis3,
						const FPoint2 &min1, const FPoint2 &max1,
						float fTiling, int steps);
vtGeode *CreateBlockGeom(const vtMaterialArray *pMats, int iMatIdx,
						const FPoint3 &size);
void AddLineMesh(vtGeode *pGeode, int iMatIdx, const FPoint3 &p0, const FPoint3 &p1);
vtGeode *CreateSphereGeom(const vtMaterialArray *pMats, int iMatIdx, int iVertType,
						 float fRadius, int res);
vtGeode *CreateCylinderGeom(const vtMaterialArray *pMats, int iMatIdx, int iVertType,
						   float hHeight, float fRadius, int res,
						   bool bTop = true, bool bBottom = true,
						   bool bCentered = true, int direction = 1);
vtGeode *CreateLineGridGeom(const vtMaterialArray *pMats, int iMatIdx,
					   const FPoint3 &min1, const FPoint3 &max1, int steps);

/**
	A dynamic visual display of a wireframe bounding box, which can be very
	useful for debugging things involving extents in the 3D scene.  To use:
\code
	// Create
	vtDynBoundBox *box = new vtDynBoundBox(RGBf(0,0,1));	// blue box
	parent->addChild(box->pGeode);

	// Use it to show a node's bounding box
	FBox3 bb;
	GetNodeBoundBox(node, bb);
	box->SetBox(bb);
\endcode
 */
class vtDynBoundBox
{
public:
	vtDynBoundBox(const RGBf &color);
	void SetBox(const FBox3 &box);
	vtGeode *pGeode;
	vtMesh *pMesh;
};

// Writing Wavefront OBJ format
class vtOBJFile
{
public:
	bool Begin(const vtMaterialArray *materials, const char *filename);
	void WriteGeode(vtGeode *geode, const FPoint3 &vertexOffset);
	void Close();

protected:
	FILE *m_fp;
	int m_VertsWritten;
};
bool WriteGeodeToOBJ(vtGeode *geode, const char *filename);

/*@}*/	// Group sg
