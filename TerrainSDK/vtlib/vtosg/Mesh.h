//
// Mesh.h for OSG
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

#include <osg/PrimitiveSet>
#include <osg/Geometry>

#include <osgText/Font>
#include <osgText/Text>

//#define USE_OPENGL_BUFFER_OBJECTS

// Vertex values
#define VT_Normals		1
#define VT_Colors		2
#define VT_TexCoords	4
#ifdef USE_OPENGL_BUFFER_OBJECTS
#define VT_VBO			8
#endif
class vtImage;

/** \addtogroup sg */
/*@{*/

/**
 * A Mesh is a set of graphical primitives (such as lines, triangles,
 *	or fans).
 * \par
 * The vtMesh class allows you to define and access a Mesh, including many
 * functions useful for creating and dynamically changing Meshes.
 * To add the vtMesh to the visible scene graph, add it to a vtGeode node.
 * \par
 */
class vtMesh : public osg::Geometry
{
	friend class vtGeode;

public:
	typedef osg::PrimitiveSet::Mode PrimType;

	// Constructor.
	vtMesh(PrimType ePrimType, int VertType, int NumVertices);

	// Adding vertices
	int AddVertex(float x, float y, float z);
	int AddVertexN(float x, float y, float z, float nx, float ny, float nz);
	int AddVertexUV(float x, float y, float z, float u, float v);

	int AddVertex(const FPoint3 &p);
	int AddVertexN(const FPoint3 &p, const FPoint3 &n);
	int AddVertexUV(const FPoint3 &p, float u, float v);
	int AddVertexUV(const FPoint3 &p, const FPoint2 &uv);
	int AddVertexNUV(const FPoint3 &p, const FPoint3 &n, const FPoint2 &uv);

	// Adding primitives.
	void AddTri(int p0, int p1, int p2);
	void AddFan(int p0, int p1, int p2 = -1, int p3 = -1, int p4 = -1, int p5 = -1);
	void AddFan(int *idx, int iNVerts);
	void AddStrip(int iNVerts, unsigned short *pIndices);
	void AddStrip2(int iNVerts, int iStartIndex);
	void AddLine(int p0, int p1);
	int  AddLine(const FPoint3 &pos1, const FPoint3 &pos2);
	void AddQuad(int p0, int p1, int p2, int p3);

	// Accessors
	PrimType getPrimType() const { return m_PrimType; }
#if 0
	// TODO - why can't we just do this? Because there may be many primsets?
	PrimType getPrimType() const { return (PrimType) getPrimSet()->getMode(); }
#endif
	void SetMatIndex(int i) { m_iMatIdx = i; }
	int GetMatIndex() const { return m_iMatIdx; }

	// Create multiple primitives.
	void CreateBlock(const FPoint3& size);
	void CreateOptimizedBlock(const FPoint3& size);
	void CreatePrism(const FPoint3 &base, const FPoint3 &vector_up,
		const FPoint2 &size1, const FPoint2 &size2);
	void CreateRectangularMesh(int xsize, int ysize, bool bReverseNormals = false);
	void CreateEllipsoid(const FPoint3 &center, const FPoint3 &size,
		int res, bool hemi = false, bool bNormalsIn = false);
	void CreateCylinder(float height, float radius, int res,
		bool bTop = true, bool bBottom = true, bool bCentered = true,
		int direction = 1);
	void CreateTetrahedron(const FPoint3 &center, float fRadius);
	void AddRectangleXZ(float xsize, float zsize);
	void AddRectangleXY(float x, float y, float xsize, float ysize,
		float z=0.0f, bool bCentered=false);
	void CreateConicalSurface(const FPoint3 &tip, double radial_angle,
							  double theta1, double theta2,
							  double r1, double r2, int res = 40);
	void CreateRectangle(int iQuads1, int iQuads2,
		int Axis1, int Axis2, int Axis3,
		const FPoint2 &min1, const FPoint2 &max1, float fLevel, float fTiling);

	// Modify vertices.
	void TransformVertices(const FMatrix4 &mat);

	// Access properties.
	uint NumVertices() const;
	int NumPrims() const;
	void GetBoundBox(FBox3 &box) const;
	int GetPrimLen(int i) const;

	void SetVtxPos(uint, const FPoint3&);
	FPoint3 GetVtxPos(uint i) const;

	void SetVtxNormal(uint, const FPoint3&);
	FPoint3 GetVtxNormal(uint i) const;

	void SetVtxColor(uint, const RGBAf&);
	RGBAf GetVtxColor(uint i) const;

	void SetVtxTexCoord(uint, const FPoint2&);
	FPoint2 GetVtxTexCoord(uint i) const;

	void SetLineWidth(float fWidth);

	void SetVtxPUV(uint i, const FPoint3 &pos, float u, float v)
	{
		SetVtxPos(i, pos);
		SetVtxTexCoord(i, FPoint2(u, v));
	}
	void SetVtxPN(uint i, const FPoint3 &pos, const FPoint3 &norm)
	{
		SetVtxPos(i, pos);
		SetVtxNormal(i, norm);
	}

	// Control rendering optimization ("display lists")
	void AllowOptimize(bool bAllow);
	void ReOptimize();

	void SetNormalsFromPrimitives();

	bool hasVertexNormals() const { return getNormalArray() != NULL; }
	bool hasVertexColors() const { return getColorArray() != NULL; }
	bool hasVertexTexCoords() const { return getTexCoordArray(0) != NULL; }

protected:
	// Implementation
	void _AddStripNormals();
	void _AddPolyNormals();
	void _AddTriangleNormals();
	void _AddQuadNormals();

	osg::PrimitiveSet *getPrimSet() { return getPrimitiveSet(0); }
	const osg::PrimitiveSet *getPrimSet() const { return getPrimitiveSet(0); }

	osg::Vec3Array *getVerts() { return (osg::Vec3Array*) getVertexArray(); }
	const osg::Vec3Array *getVerts() const { return (const osg::Vec3Array*) getVertexArray(); }

	osg::Vec3Array *getNormals() { return (osg::Vec3Array*) getNormalArray(); }
	const osg::Vec3Array *getNormals() const { return (const osg::Vec3Array*) getNormalArray(); }

	osg::Vec4Array *getColors() { return (osg::Vec4Array*) getColorArray(); }
	const osg::Vec4Array *getColors() const { return (const osg::Vec4Array*) getColorArray(); }

	osg::Vec2Array *getTexCoords() { return (osg::Vec2Array*) getTexCoordArray(0); }
	const osg::Vec2Array *getTexCoords() const { return (const osg::Vec2Array*) getTexCoordArray(0); }

	int m_iMatIdx;

	// OSG Geometry objects can have many kinds of primitives in them, but
	// vtMesh expects them to all the same kind, so we store that here.
	// TODO: Could we remove this and simply get the type of the first PrimSet?
	PrimType m_PrimType;
};

/** A Font for use with vtTextMesh. */
typedef osg::ref_ptr<osgText::Font> vtFontPtr;

/**
 * A Text Mesh is a special kind of Mesh which contains text instead of
 * general geometry primitives.  vtTextMesh is used similarly with vtMesh:
 * you create them and add them to vtGeode objects to add them to the scene.
 */
class vtTextMesh : public osgText::Text
{
public:
	vtTextMesh(osgText::Font *font, float fSize = 1, bool bCenter = false);

	// Override with ability to get OSG bounding box
	void GetBoundBox(FBox3 &box) const;

	/// Set the text string that this TextMesh should display
	void SetText(const char *text);

	/// Set the (wide) text string that this TextMesh should display
	void SetText(const wchar_t *text);
#if SUPPORT_WSTRING
	/// Set the (wide) text string that this TextMesh should display
	void SetText(const std::wstring &text);
#endif
	/// Set the position (XYZ translation) of the text in its own frame
	void SetPosition(const FPoint3 &pos);

	/// Set the rotation (Quaternion) of the text in its own frame
	void SetRotation(const FQuat &rot);

	/// Set the text alignment: 0 for XY plane, 1 for XZ, 2 for YZ
	void SetAlignment(int align);

	/// Set the color
	void SetColor(const RGBAf &rgba);

	void SetMatIndex(int i) { m_iMatIdx = i; }
	int GetMatIndex() const { return m_iMatIdx; }

protected:
	int	m_iMatIdx;
};

/*@}*/	// Group sg

