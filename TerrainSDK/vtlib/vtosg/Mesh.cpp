//
// Mesh.cpp - Meshes for vtlib-OSG
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/vtLog.h"

#include <osg/LineWidth>


/////////////////////////////////////////////////////////////////////////////
// vtMesh
//

static const FPoint3 sCubeCorners[8] =
{
	FPoint3(-1.0, -1.0, 1.0),	/* 0 */
	FPoint3(-1.0, 1.0, 1.0),	/* 1 */
	FPoint3(1.0, 1.0, 1.0),		/* 2 */
	FPoint3(1.0, -1.0, 1.0),	/* 3 */
	FPoint3(-1.0, -1.0, -1.0),	/* 4 */
	FPoint3(-1.0, 1.0, -1.0),	/* 5 */
	FPoint3(1.0, 1.0, -1.0),	/* 6 */
	FPoint3(1.0, -1.0, -1.0),	/* 7 */
};

static const FPoint3 sPrismCorners[8] =
{
	FPoint3(-0.5, 0, 0.5),		/* 0 */
	FPoint3(-0.5, 1.0, 0.5),	/* 1 */
	FPoint3(0.5, 1.0, 0.5),		/* 2 */
	FPoint3(0.5, 0, 0.5),		/* 3 */
	FPoint3(-0.5, 0, -0.5),		/* 4 */
	FPoint3(-0.5, 1.0, -0.5),	/* 5 */
	FPoint3(0.5, 1.0, -0.5),	/* 6 */
	FPoint3(0.5, 0, -0.5),		/* 7 */
};

static const FPoint3 sCubeNormals[6] =
{
	FPoint3( 0.0, 0.0, 1.0 ),
	FPoint3( -1.0, 0.0, 0.0 ),
	FPoint3( 0.0, 0.0, -1.0 ),
	FPoint3( 1.0, 0.0, 0.0 ),
	FPoint3( 0.0, -1.0, 0.0 ),
	FPoint3( 0.0, 1.0, 0.0 )
};

static const int sCubeIndices[6][4] = {
	7, 4, 5, 6,		 /* back */
	3, 7, 6, 2,		 /* right */
	0, 3, 2, 1,		 /* front */
	4, 0, 1, 5,		 /* left */
	6, 5, 1, 2,		 /* top */
	4, 7, 3, 0,		 /* bottom */
};

static const FPoint2 sSquareTexCoords[4] =
{
	FPoint2(0.0, 1.0),
	FPoint2(1.0, 1.0),
	FPoint2(1.0, 0.0),
	FPoint2(0.0, 0.0)
};

/**
 * Construct a Mesh.
 * A Mesh is a container for a set of vertices and primitives.
 *
 * \param ePrimType The type of primitive this mesh will contain.  Allowed
 *		values are:
 *		- osg::PrimitiveSet::POINTS
 *		- osg::PrimitiveSet::LINES
 *		- osg::PrimitiveSet::LINE_STRIP
 *		- osg::PrimitiveSet::TRIANGLES
 *		- osg::PrimitiveSet::TRIANGLE_STRIP
 *		- osg::PrimitiveSet::TRIANGLE_FAN
 *		- osg::PrimitiveSet::QUADS
 *		- osg::PrimitiveSet::POLYGON
 *
 * \param VertType Flags which indicate what type of information is stored
 *		with each vertex.  This can be any combination of the following bit
 *		flags:
 *		- VT_Normals - a normal per vertex.
 *		- VT_Colors - a color per vertex.
 *		- VT_TexCoords - a texture coordinate (UV) per vertex.
 *
 * \param NumVertices The expected number of vertices that the mesh will
 *		contain.  If more than this number of vertices are added, the mesh
 *		will automatically grow to contain them.  However it is more
 *		efficient if you know the number at creation time and pass it in
 *		this parameter.
 */
vtMesh::vtMesh(PrimType ePrimType, int VertType, int NumVertices)
{
	m_PrimType = ePrimType;
	m_iMatIdx = -1;

	osg::Vec3Array *pVert = new osg::Vec3Array;
	pVert->reserve(NumVertices);
	setVertexArray(pVert);

	if (VertType & VT_Normals)
	{
		osg::Vec3Array *pNorm = new osg::Vec3Array;
		pNorm->reserve(NumVertices);
		setNormalArray(pNorm);
		setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	}
	if (VertType & VT_Colors)
	{
		osg::Vec4Array *pColor = new osg::Vec4Array;
		pColor->reserve(NumVertices);
		setColorArray(pColor);
		setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	}
	if (VertType & VT_TexCoords)
	{
		osg::Vec2Array *pTex = new osg::Vec2Array;
		pTex->reserve(NumVertices);
		setTexCoordArray(0, pTex);
	}

#ifdef USE_OPENGL_BUFFER_OBJECTS
	// This can only be done after the arrays have been set up
	// The buffer objects will be compiled on the next rendering pass
	// So if you change the contents of the vertex, normal, colour, or tex coord arrys
	// after the next rendering pass you need to call the DirtyBufferObjects() function
	// to cause them to recompile.
	// Element Index buffer objects are handled slightly differently by OSG but the
	// same constraint applies if you change any element indices. This occurs when
	// you add geometry definitions to the mesh e.g by calling AddTriangle etc.
	if (VertType & VT_VBO)
		setUseVertexBufferObjects(true);
#endif

	osg::PrimitiveSet *pPrimSet;

	pPrimSet = NULL;
	switch (ePrimType)
	{
	case osg::PrimitiveSet::POINTS:
		pPrimSet = new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, NumVertices);
		break;
	case osg::PrimitiveSet::LINES:
		pPrimSet = new osg::DrawElementsUShort(osg::PrimitiveSet::LINES);
		break;
	case osg::PrimitiveSet::TRIANGLES:
		pPrimSet = new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLES);
		break;
	case osg::PrimitiveSet::QUADS:
		pPrimSet = new osg::DrawElementsUShort(osg::PrimitiveSet::QUADS);
		break;
	default:	// Keep picky compilers quiet.
		break;
	}
	if (NULL != pPrimSet)
		addPrimitiveSet(pPrimSet);
}

/**
 * Adds a vertex to the mesh.
 *
 * \return The index of the vertex that was added.
 */
int vtMesh::AddVertex(float x, float y, float z)
{
	const int n = NumVertices();
	SetVtxPos(n, FPoint3(x, y, z));
	return n;
}

/**
 * Adds a vertex to the mesh, with a vertex normal.
 *
 * \return The index of the vertex that was added.
 */
int vtMesh::AddVertexN(float x, float y, float z, float nx, float ny, float nz)
{
	const int n = NumVertices();
	SetVtxPos(n, FPoint3(x, y, z));
	SetVtxNormal(n, FPoint3(nx, ny, nz));
	return n;
}

/**
 * Adds a vertex to the mesh, with UV coordinates.
 *
 * \return The index of the vertex that was added.
 */
int vtMesh::AddVertexUV(float x, float y, float z, float u, float v)
{
	const int n = NumVertices();
	SetVtxPos(n, FPoint3(x, y, z));
	SetVtxTexCoord(n, FPoint2(u, v));
	return n;
}

/**
 * Adds a vertex to the mesh.
 *
 * \return The index of the vertex that was added.
 */
int vtMesh::AddVertex(const FPoint3 &p)
{
	const int n = NumVertices();
	SetVtxPos(n, p);
	return n;
}

/**
 * Adds a vertex to the mesh, with a vertex normal.
 *
 * \return The index of the vertex that was added.
 */
int vtMesh::AddVertexN(const FPoint3 &pos, const FPoint3 &norm)
{
	const int n = NumVertices();
	SetVtxPos(n, pos);
	SetVtxNormal(n, norm);
	return n;
}

/**
 * Adds a vertex to the mesh, with UV coordinates.
 *
 * \return The index of the vertex that was added.
 */
int vtMesh::AddVertexUV(const FPoint3 &p, float u, float v)
{
	const int n = NumVertices();
	SetVtxPos(n, p);
	SetVtxTexCoord(n, FPoint2(u, v));
	return n;
}

/**
 * Adds a vertex to the mesh, with UV coordinates.
 *
 * \return The index of the vertex that was added.
 */
int vtMesh::AddVertexUV(const FPoint3 &p, const FPoint2 &uv)
{
	const int n = NumVertices();
	SetVtxPos(n, p);
	SetVtxTexCoord(n, uv);
	return n;
}

/**
 * Adds a vertex to the mesh, with a vertex normal and UV coordinates.
 *
 * \return The index of the vertex that was added.
 */
int vtMesh::AddVertexNUV(const FPoint3 &pos, const FPoint3 &norm, const FPoint2 &uv)
{
	const int n = NumVertices();
	SetVtxPos(n, pos);
	SetVtxNormal(n, norm);
	SetVtxTexCoord(n, uv);
	return n;
}

/**
* Add a triangle.
*  p0, p1, p2 are the indices of the vertices of the triangle.
*/
void vtMesh::AddTri(int p0, int p1, int p2)
{
	PrimType PrimType = getPrimType();
	osg::DrawElements *pDrawElements;
	if (PrimType == osg::PrimitiveSet::TRIANGLES)
	{
		pDrawElements = getPrimitiveSet(0)->getDrawElements();
	}
	else if (PrimType == osg::PrimitiveSet::TRIANGLE_STRIP || PrimType == osg::PrimitiveSet::TRIANGLE_FAN)
	{
		pDrawElements = new osg::DrawElementsUShort(PrimType, 3);
		addPrimitiveSet(pDrawElements);
	}
	pDrawElements->addElement(p0);
	pDrawElements->addElement(p1);
	pDrawElements->addElement(p2);
}

/**
* Add a triangle fan with up to 6 points (center + 5 points).  The first 3
* arguments are required, the rest are optional.  A fan will be created
* with as many point indices as you pass.
*/
void vtMesh::AddFan(int p0, int p1, int p2, int p3, int p4, int p5)
{
	osg::DrawElements *pDrawElements;
	pDrawElements = new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLE_FAN);
	pDrawElements->addElement(p0);
	pDrawElements->addElement(p1);
	if (p2 != -1)
		pDrawElements->addElement(p2);
	if (p3 != -1)
		pDrawElements->addElement(p3);
	if (p4 != -1)
		pDrawElements->addElement(p4);
	if (p5 != -1)
		pDrawElements->addElement(p5);
	addPrimitiveSet(pDrawElements);
}

/**
* Add a triangle fan with any number of points.
*	\param idx An array of vertex indices for the fan.
*	\param iNVerts the number of vertices in the fan.
*/
void vtMesh::AddFan(int *idx, int iNVerts)
{
	osg::DrawElements *pDrawElements;
	pDrawElements = new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLE_FAN);
	pDrawElements->reserveElements(iNVerts);
	for (int i = 0; i < iNVerts; i++)
		pDrawElements->addElement(idx[i]);
	addPrimitiveSet(pDrawElements);
}

/**
* Adds an indexed strip to the mesh.
*
* \param iNVerts The number of vertices in the strip.
* \param pIndices An array of the indices of the vertices in the strip.
*/
void vtMesh::AddStrip(int iNVerts, unsigned short *pIndices)
{
	osg::DrawElements *pDrawElements;
	switch (m_PrimType)
	{
	case osg::PrimitiveSet::LINE_STRIP:
		pDrawElements = new osg::DrawElementsUShort(osg::PrimitiveSet::LINE_STRIP);
		break;
	case osg::PrimitiveSet::TRIANGLE_STRIP:
		pDrawElements = new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLE_STRIP);
		break;
	case osg::PrimitiveSet::QUAD_STRIP:
		pDrawElements = new osg::DrawElementsUShort(osg::PrimitiveSet::QUAD_STRIP);
		break;
	}
	pDrawElements->reserveElements(iNVerts);
	for (int i = 0; i < iNVerts; i++)
		pDrawElements->addElement(pIndices[i]);
	addPrimitiveSet(pDrawElements);
}

/**
 * Adds an indexed strip to the mesh, with the assumption that the indices
 * are in linear order.
 *
 * \param iNVerts The number of vertices in the strip.
 * \param iStartIndex The index that starts the linear sequence.
 */
void vtMesh::AddStrip2(int iNVerts, int iStartIndex)
{
	unsigned short *idx = new unsigned short[iNVerts];

	for (int i = 0; i < iNVerts; i++)
		idx[i] = iStartIndex + i;

	AddStrip(iNVerts, idx);
	delete [] idx;
}

/**
* Add a single line primitive to a mesh.
*	\param p0, p1	The indices of the two vertices of the line.
*/
void vtMesh::AddLine(int p0, int p1)
{
	PrimType PrimType = getPrimType();
	osg::DrawElements *pDrawElements;
	if (PrimType == osg::PrimitiveSet::LINES)
	{
		pDrawElements = getPrimitiveSet(0)->getDrawElements();
	}
	else if (PrimType == osg::PrimitiveSet::LINE_STRIP)
	{
		// This seems a bit pointless
		pDrawElements = new osg::DrawElementsUShort(PrimType, 2);
		addPrimitiveSet(pDrawElements);
	}
	pDrawElements->addElement(p0);
	pDrawElements->addElement(p1);
}

/**
* Add a single line primitive to a mesh.
*	\param pos1, pos2	The positions of the two vertices of the line.
*  \return The index of the first vertex added.
*/
int vtMesh::AddLine(const FPoint3 &pos1, const FPoint3 &pos2)
{
	int p0 = AddVertex(pos1);
	int p1 = AddVertex(pos2);
	AddLine(p0, p1);
	return p0;
}

/**
* Add a triangle.
*  p0, p1, p2 are the indices of the vertices of the triangle.
*/
void vtMesh::AddQuad(int p0, int p1, int p2, int p3)
{
	osg::DrawElements *pDrawElements;
	pDrawElements = getPrimitiveSet(0)->getDrawElements();
	pDrawElements->addElement(p0);
	pDrawElements->addElement(p1);
	pDrawElements->addElement(p2);
	pDrawElements->addElement(p3);
}

/**
 * Add a block (rectangular 3d box) to this mesh.  The width, height and
 * depth are specified with the 'size' parameter.
 */
void vtMesh::CreateBlock(const FPoint3& size)
{
	int i, j;
	FPoint3 vtx[8];			/* individual vertex values */

	for (i = 0; i < 8; i++)
	{
		vtx[i].x = size.x * sCubeCorners[i].x;
		vtx[i].y = size.y * sCubeCorners[i].y;
		vtx[i].z = size.z * sCubeCorners[i].z;
	}

	int vidx = 0;
	for (i = 0; i < 6; i++)
	{
		int start = vidx;
		for (j = 0; j < 4; j++)
		{
			AddVertex(vtx[sCubeIndices[i][j]]);
			if (hasVertexNormals())		/* compute normals */
				SetVtxNormal(vidx, -sCubeNormals[i]);
			if (hasVertexTexCoords())	/* compute tex coords */
				SetVtxTexCoord(vidx, sSquareTexCoords[j]);
			vidx++;
		}
		AddFan(start, start+1, start+2, start+3);
	}
}

/**
 *	Adds a 3D block to a vtMesh as a series of 5 triangle fan primitives.
 *  The bottom face is omitted, the base is placed at y=0, and texture
 *  coordinates are provided such that texture bitmaps appear right-side-up
 *  on the side faces.
 */
void vtMesh::CreateOptimizedBlock(const FPoint3& size)
{
	int i, j;
	FPoint3 vtx[8];			/* individual vertex values */
	FPoint3 half = size / 2.0f;

	for (i = 0; i < 8; i++)
	{
		vtx[i].x = half.x * sCubeCorners[i].x;
		vtx[i].y = half.y * sCubeCorners[i].y + (half.y);
		vtx[i].z = half.z * sCubeCorners[i].z;
	}

	int vidx = 0;
	for (i = 0; i < 5; i++)
	{
		int start = vidx;
		for (j = 0; j < 4; j++)
		{
			AddVertex(vtx[sCubeIndices[i][j]]);
			if (hasVertexNormals())		/* compute normals */
				SetVtxNormal(vidx, -sCubeNormals[i]);
			if (hasVertexTexCoords())	/* compute tex coords */
				SetVtxTexCoord(vidx, sSquareTexCoords[j]);
			vidx++;
		}
		AddFan(start, start+1, start+2, start+3);
	}
}

/**
 * Adds a 3D block (extruded rectangle) to a vtMesh as a series of 5 triangle
 * fan primitives.  The bottom face is omitted, the base is placed at /base/,
 * the extrusion is along /vector_up/.  Texture coordinates are provided such
 * that texture bitmaps appear right-side-up on the side faces.
 *
 * Used by vtFence3d to make fenceposts.
 */
void vtMesh::CreatePrism(const FPoint3 &base, const FPoint3 &vector_up,
	const FPoint2 &size1, const FPoint2 &size2)
{
	int i, j;
	FPoint3 vtx[8];		/* individual vertex values */

	for (i = 0; i < 8; i++)
	{
		vtx[i].y = base.y + sPrismCorners[i].y * (vector_up.y);
		if (sPrismCorners[i].y == 0)	// bottom
		{
			vtx[i].x = base.x + size1.x * sPrismCorners[i].x;
			vtx[i].z = base.z + size1.y * sPrismCorners[i].z;
		}
		else
		{
			vtx[i].x = base.x + vector_up.x + size2.x * sPrismCorners[i].x;
			vtx[i].z = base.z + vector_up.z + size2.y * sPrismCorners[i].z;
		}
	}

	int vidx = NumVertices();
	for (i = 0; i < 5; i++)
	{
		int start = vidx;
		for (j = 0; j < 4; j++)
		{
			AddVertex(vtx[sCubeIndices[i][j]]);
			if (hasVertexNormals())		/* compute normals */
				SetVtxNormal(vidx, -sCubeNormals[i]);
			if (hasVertexTexCoords())	/* compute tex coords */
				SetVtxTexCoord(vidx, sSquareTexCoords[j]);
			vidx++;
		}
		AddFan(start, start+1, start+2, start+3);
	}
}

/**
 * Adds triangle/quad strips to this mesh, suitable for a (topologically)
 * rectangular grid.
 *
 * \param xsize Number of vertices in the first dimension.
 * \param ysize Number of vertices in the second dimension.
 * \param bReverseNormals Reverse the vertex order so the normals point the other way.
 */
void vtMesh::CreateRectangularMesh(int xsize, int ysize, bool bReverseNormals)
{
	int i, j;

	unsigned short *strip = new unsigned short[xsize*2];
	for (j = 0; j < ysize - 1; j++)
	{
		int start = j * xsize;
		int len = 0;
		for (i = 0; i < xsize; i++)
		{
			if (bReverseNormals)
			{
				// reverse the vertex order so the normals point the other way
				strip[len++] = start + i + xsize;
				strip[len++] = start + i;
			}
			else
			{
				strip[len++] = start + i;
				strip[len++] = start + i + xsize;
			}
		}
		AddStrip(xsize*2, strip);
	}
	delete [] strip;
}

/**
 * Adds geometry for an ellipsoid to this mesh.
 *
 * The geometry is created with efficient triangle strips.  If the mesh
 * has vertex normals, outward-pointing normals are created for lighting.
 * If the mesh has vertex coordinates, then UVs are set as follows:
 * U ranges from 0 to 1 around the circumference, and V ranges from 0
 * to 1 from the top to the bottom.  For a hemisphere, V ranges from
 * 0 at the top to 1 at the base.
 *
 * \param center Position of the center, pass FPoint3(0,0,0) to center on the origin.
 * \param size The width, height and depth of the ellipsoid.
 * \param res The resolution (number of quads used in the tesselation)
 *		from top to bottom (north pole to south pole).
 * \param hemi Create only the top of the ellipsoid (e.g. a hemisphere).
 * \param bNormalsIn Use a vertex order in the mesh so that the normals point
 *		in, instead of out.  This is useful for, example, a backface-culled
 *		sphere that you want to see from the inside, instead of the outside.
 */
void vtMesh::CreateEllipsoid(const FPoint3 &center, const FPoint3 &size,
								 int res, bool hemi, bool bNormalsIn)
{
	int		i, j;
	int		vidx;
	FPoint3 v;
	float	theta_step, phi_step;
	int		theta_res, phi_res;
	float	phi_range;

	if (hemi)
	{
		phi_res = res/2;
		phi_range = PID2f;
	}
	else
	{
		phi_res = res;
		phi_range = PIf;
	}
	theta_res = res * 2;

	theta_step = PI2f / theta_res;
	phi_step = phi_range / phi_res;
	for (j = 0; j <= phi_res; j++)
	{
		float phi = j * phi_step;

		for (i = 0; i <= theta_res; i++)
		{
			float theta = i * theta_step;
			v.x = sinf(theta) * sinf(phi);
			v.y = cosf(phi);
			v.z = cosf(theta) * sinf(phi);

			FPoint3 p(size.x * v.x, size.y * v.y, size.z * v.z);
			vidx = AddVertex(center + p);
			if (hasVertexNormals())		/* compute normals */
				SetVtxNormal(vidx, v);
			if (hasVertexTexCoords())	/* compute tex coords */
			{
				FPoint2 t((float)i / theta_res, 1.0f - (float)j / phi_res);
				SetVtxTexCoord(vidx, t);
			}
		}
	}
	CreateRectangularMesh(theta_res+1, phi_res+1, bNormalsIn);
}

/**
 * Adds an cylinder to this mesh.
 *
 * \param height The height of the cylinder.
 * \param radius The radius of the cylinder.
 * \param res The resolution (number of side of the cylinder).
 * \param bTop True to create the top of the cylinder.
 * \param bBottom True to create the bottom of the cylinder.  You could set
 *		this to false, for example, if the cylinder is going to sit on a
 *		flat surface where you will never see its bottom.
 * \param bCentered True to create a cylinder centered around its origin,
 *		false for a cylinder with its base at the origin that extends outward.
 * \param direction An orientation, 0-2 corresponds to X, Y, Z.  Default is 1 (Y).
 */
void vtMesh::CreateCylinder(float height, float radius, int res,
								bool bTop, bool bBottom, bool bCentered, int direction)
{
	// One way to model a cylinder is as a triangle/quad strip and
	// a pair of triangle fans for the top and bottom, although that
	// requires 2 kinds of primitive, so we would need 2 vtMesh objects.
	// Instead, we use triangle strips for both the sides and top/bottom.

	// In order to do vertex-based shading, we will have to provide correct
	// vertex normals for the sides and top/bottom separately.  Unfortunately
	// this doubles the number of vertices, since each vertex can only
	// have a single normal.  So we may have to make 2 passes through vertex
	// construction.

	int		a, b, passes;
	int		i, j, k;
	int		vidx;
	FPoint3 p, norm;

	float	theta_step = PI2f / res;
	int		verts_per_pass = res * 2;

	if ((bTop || bBottom) && (hasVertexNormals()))
		passes = 2;
	else
		passes = 1;

	for (a = 0; a < passes; a++)
	{
		for (b = 0; b < 2; b++)
		{
			if (bCentered)
				p.y = b ? height/2 : -height/2;
			else
				p.y = b ? height : 0;

			for (i = 0; i < res; i++)
			{
				float theta = i * theta_step;
				p.x = cosf(theta) * radius;
				p.z = sinf(theta) * radius;
				vidx = AddVertex(p);

				if (hasVertexNormals())	/* compute normals */
				{
					if (a == 0)		// first pass, outward normals for sides
					{
						norm.x = cosf(theta);
						norm.y = 0;
						norm.z = sinf(theta);
					}
					else if (a == 1)	// second pass, top/bottom normals
					{
						norm.x = 0;
						norm.y = b ? 1.0f : -1.0f;
						norm.z = 0;
					}
					SetVtxNormal(vidx, norm);
				}
			}
		}
	}
	// Create sides
	unsigned short *indices = new unsigned short[(res+1) * 2];
	j = 0;
	k = 0;
	for (i = 0; i < res+1; i++)
	{
		indices[j++] = k;
		indices[j++] = k+res;
		k++;
		if (k == res)
			k = 0;
	}
	AddStrip(j, indices);

	// create top and bottom
	int offset = (passes == 2 ? verts_per_pass : 0);
	if (bBottom)
	{
		j = 0;
		for (i = 0; i < res/2; i++)
		{
			indices[j++] = offset + i;
			k = res - 1 - i;
			if (i >= k)
				break;
			indices[j++] = offset + k;
		}
		AddStrip(j, indices);
	}

	if (bTop)
	{
		j = 0;
		for (i = 0; i < res/2; i++)
		{
			indices[j++] = offset + res + i;
			k = res - 1 - i;
			if (i >= k)
				break;
			indices[j++] = offset + res + k;
		}
		AddStrip(j, indices);
	}
	delete indices;
}

void vtMesh::CreateTetrahedron(const FPoint3 &center, float fRadius)
{
	float A = fRadius * 1.632993161858f;
	float B = fRadius;
	float D = fRadius * 0.333333333333f;
	float F = fRadius * 0.9428090415834f;
	float G = fRadius * 0.4714045207904f;
	FPoint3 p0(-A/2, -D, G);
	FPoint3 p1( A/2, -D, G);
	FPoint3 p2( 0, B, 0);
	FPoint3 p3( 0, -D, -F);

	p0 += center;
	p1 += center;
	p2 += center;
	p3 += center;

	int vidx;
	if (hasVertexNormals())
	{
		// We need distinct vertices for each triangle, so they can have
		//  different normals.  This means 12 vertices, 4 faces * 3 corners.
		vidx = AddVertex(p0); AddVertex(p1); AddVertex(p2);
		AddTri(vidx, vidx+1, vidx+2);

		vidx = AddVertex(p1); AddVertex(p3); AddVertex(p2);
		AddTri(vidx, vidx+1, vidx+2);

		vidx = AddVertex(p3); AddVertex(p0); AddVertex(p2);
		AddTri(vidx, vidx+1, vidx+2);

		vidx = AddVertex(p0); AddVertex(p3); AddVertex(p1);
		AddTri(vidx, vidx+1, vidx+2);
	}
	else
	{
		// With no shading, all we need is 4 vertices for the 4 faces.
		vidx = AddVertex(p0); AddVertex(p1); AddVertex(p2); AddVertex(p3);
		AddTri(vidx+0, vidx+1, vidx+2);
		AddTri(vidx+1, vidx+3, vidx+2);
		AddTri(vidx+3, vidx+0, vidx+2);
		AddTri(vidx+0, vidx+3, vidx+1);
	}
}


/**
 * Adds the vertices and a fan primitive for a single flat rectangle.
 */
void vtMesh::AddRectangleXZ(float xsize, float zsize)
{
	int vidx =
	AddVertexUV(-xsize/2, 0.0f, -zsize/2,	0.0f, 0.0f);
	AddVertexUV( xsize/2, 0.0f, -zsize/2,	1.0f, 0.0f);
	AddVertexUV( xsize/2, 0.0f,  zsize/2,	1.0f, 1.0f);
	AddVertexUV(-xsize/2, 0.0f,  zsize/2,	0.0f, 1.0f);
	AddFan(vidx, vidx+1, vidx+2, vidx+3);
}

/**
 * Adds the vertices and a fan primitive for a single flat rectangle.
 */
void vtMesh::AddRectangleXY(float x, float y, float xsize, float ysize,
								float z, bool bCentered)
{
	int vidx;
	if (bCentered)
	{
		vidx =
		AddVertexUV(-xsize/2, -ysize/2,	z, 0.0f, 0.0f);
		AddVertexUV( xsize/2, -ysize/2,	z, 1.0f, 0.0f);
		AddVertexUV( xsize/2,  ysize/2,	z, 1.0f, 1.0f);
		AddVertexUV(-xsize/2,  ysize/2,	z, 0.0f, 1.0f);
	}
	else
	{
		vidx =
		AddVertexUV(x,		 y,			z, 0.0f, 0.0f);
		AddVertexUV(x+xsize, y,			z, 1.0f, 0.0f);
		AddVertexUV(x+xsize, y+ysize,	z, 1.0f, 1.0f);
		AddVertexUV(x,		 y+ysize,	z, 0.0f, 1.0f);
	}
	if (getPrimType() == osg::PrimitiveSet::TRIANGLE_FAN)
		AddFan(vidx, vidx+1, vidx+2, vidx+3);
	else if (getPrimType() == osg::PrimitiveSet::QUADS)
		AddQuad(vidx, vidx+1, vidx+2, vidx+3);
}

/**
 * Adds an conical surface to this mesh.  This is a subset of a full cone,
 * bounded by start/end factors along the two degrees of freedom of the
 * surface of the cone.  The default orientation of the cone is with the
 * tip pointing up (radius increasing downward).
 *
 * \param tip The top point of the cone.
 * \param radial_angle This is the angle between the cone's edge and its
 *		center axis, in radians.  Expected range is 0 to PI/2.  Small values
 *		indicate a sharp, pointed cone, large values indicate a blunt cone.
 *		The slope of the cone's edge is tan(radial_angle).
 * \param theta1, theta2 Start and end values for the theta value, which
 *		ranges from 0 to 2*PI around the central axis of the cone.
 * \param r1, r2 Start and end values for the cone's radius.  These range
 *		from 0 (at the tip of the cone) and increase downward.
 * \param res Resolution, number of polygons along each side of the
 *		surface mesh.
 */
void vtMesh::CreateConicalSurface(const FPoint3 &tip, double radial_angle,
									double theta1, double theta2,
									double r1, double r2, int res)
{
	int i, j, vidx;
	double tan_cr = tan(radial_angle);
	double theta, theta_step = (theta2 - theta1) / (res - 1);
	double r, r_step = (r2 - r1) / (res - 1);

	FPoint3 p, norm;

	r = r1;
	for (i = 0; i < res; i++)
	{
		theta = theta1;
		for (j = 0; j < res; j++)
		{
			p.x = (float) (tip.x + cos(theta) * r);
			p.z = (float) (tip.z - sin(theta) * r);
			p.y = (float) (tip.y - (r / tan_cr));
			vidx = AddVertex(p);

			if (hasVertexNormals())
			{
				// compute vertex normal for lighting
				norm.x = (float) (cos(theta) * r);
				norm.y = 0.0f;
				norm.z = (float) (sin(theta) * r);
				norm.Normalize();
				SetVtxNormal(vidx, norm);
			}

			theta += theta_step;
		}
		r += r_step;
	}
	CreateRectangularMesh(res, res);
}

/**
 * Adds an rectangular surface to this mesh.
 * The rectangle will lie in the first two axes given, facing toward the
 * third.  Axes can be specified by number.  For example, to produce a
 * rectangle in the XZ plane facing along the Y axis, you would pass 0, 2, 1.
 *
 * \param iQuads1 The number of quads along the first axis.
 * \param iQuads2 The number of quads along the second axis.
 * \param Axis1 The first axis (X=0, Y=1, Z=2)
 * \param Axis2 The second axis (X=0, Y=1, Z=2)
 * \param Axis3 The third axis (X=0, Y=1, Z=2)
 * \param min1 The lower-left-hand corner of the rectangle's position
 * \param max1 The size of the rectangle.
 * \param fLevel The value of the rectangle on the third axis.
 * \param fTiling UV tiling.  Set to 1 for UV coordinate of O..1.
 */
void vtMesh::CreateRectangle(int iQuads1, int iQuads2,
		int Axis1, int Axis2, int Axis3,
		const FPoint2 &min1, const FPoint2 &max1, float fLevel, float fTiling)
{
	int iVerts1 = iQuads1 + 1;
	int iVerts2 = iQuads2 + 1;

	FPoint2 size = max1 - min1;
	FPoint3 pos, normal;

	pos[Axis3] = fLevel;
	normal[Axis1] = 0;
	normal[Axis2] = 0;
	normal[Axis3] = 1;

	for (int i = 0; i < iVerts1; i++)
	{
		pos[Axis1] = min1.x + (size.x / iQuads1) * i;
		for (int j = 0; j < iVerts2; j++)
		{
			pos[Axis2] = min1.y + (size.y / iQuads2) * j;

			int vidx = AddVertex(pos);

			if (hasVertexNormals())
				SetVtxNormal(vidx, normal);

			if (hasVertexTexCoords())		/* compute tex coords */
			{
				FPoint2 tc((float) i/iQuads1 * fTiling,
							(float) j/iQuads2 * fTiling);
				SetVtxTexCoord(vidx, tc);
			}
		}
	}
	CreateRectangularMesh(iVerts1, iVerts2);
}

/**
 * Transform all the vertices of the mesh by the indicated matrix.
 */
void vtMesh::TransformVertices(const FMatrix4 &mat)
{
	int i, num = NumVertices();
	FPoint3 p, p2;

	for (i = 0; i < num; i++)
	{
		p = GetVtxPos(i);
		mat.Transform(p, p2);
		SetVtxPos(i, p2);
	}
	if (hasVertexNormals())
	{
		for (i = 0; i < num; i++)
		{
			p = GetVtxNormal(i);
			mat.TransformVector(p, p2);
			SetVtxNormal(i, p2);
		}
	}
}

uint vtMesh::NumVertices() const
{
	return getVerts()->size();
}

int vtMesh::NumPrims() const
{
	switch (m_PrimType)
	{
	case osg::PrimitiveSet::POINTS:
	case osg::PrimitiveSet::LINES:
	case osg::PrimitiveSet::TRIANGLES:
	case osg::PrimitiveSet::QUADS:
		// Fixed-length primitives: There is only one PrimSet.
		return getPrimSet()->getNumPrimitives();
		break;
	case osg::PrimitiveSet::LINE_STRIP:
	case osg::PrimitiveSet::TRIANGLE_STRIP:
	case osg::PrimitiveSet::TRIANGLE_FAN:
	case osg::PrimitiveSet::QUAD_STRIP:
		// Variable-length primitives: one PrimSet per primitive.
		return getNumPrimitiveSets();
	}
	// Invalid/unknown primtype should not happen.
	assert(0);
	return -1;
}

// Override with ability to get OSG bounding box
void vtMesh::GetBoundBox(FBox3 &box) const
{
#if OSG_VERSION_GREATER_THAN(3, 3, 0)
	const osg::BoundingBox &osg_box = getBoundingBox();
#else
	const osg::BoundingBox &osg_box = getBound();
#endif
	s2v(osg_box, box);
}

#if 0	// alternate implementation
/**
* Computes the bounding box of the mesh, based on the extent of its
* vertices.  Result is placed in the 'box' parameter.  If there
* are no vertices, then 'box' is unchanged.
*/
void vtMesh::GetBoundBox(FBox3 &box) const
{
	int num = NumVertices();
	if (num == 0)
		return;

	box.min.Set(1E8, 1E8, 1E8);
	box.max.Set(-1E8, -1E8, -1E8);

	for (int i = 0; i < num; i++)
	{
		FPoint3 p = GetVtxPos(i);

		if (p.x < box.min.x) box.min.x = p.x;
		if (p.y < box.min.y) box.min.y = p.y;
		if (p.z < box.min.z) box.min.z = p.z;

		if (p.x > box.max.x) box.max.x = p.x;
		if (p.y > box.max.y) box.max.y = p.y;
		if (p.z > box.max.z) box.max.z = p.z;
	}
}
#endif

int vtMesh::GetPrimLen(int i) const
{
	// For strip primitives, there will be one primset per strip.
	const osg::PrimitiveSet *pset = getPrimitiveSet(i);
	const osg::DrawElementsUShort *deus = dynamic_cast<const osg::DrawElementsUShort*>(pset);
	assert(deus);
	return (int)deus->size();
}

/**
 * Set the position of a vertex.
 *	\param i	Index of the vertex.
 *	\param p	The position.
 */
void vtMesh::SetVtxPos(uint i, const FPoint3 &p)
{
	// It is better to reserve the number of vertices that will be needed
	// (with the NumVertices argument to the vtMesh constructor) but if the user
	// doesn't do that, this resize will grow the array for them.
	if (i >= (int)getVerts()->size())
		getVerts()->resize(i + 1);
	getVerts()->at(i) = osg::Vec3(p.x, p.y, p.z);

#ifdef USE_OPENGL_BUFFER_OBJECTS
	getVerts()->dirty();
#endif
}

/**
 * Get the position of a vertex.
 */
FPoint3 vtMesh::GetVtxPos(uint i) const
{
	FPoint3 p;
	s2v(getVerts()->at(i), p);
	return p;
}

/**
 * Set the normal of a vertex.  This is used for lighting, if the mesh is used
 *	with a material with lighting enabled.  Generally you will	want to use a
 *  vector of unit length.
 *
 *	\param i	Index of the vertex.
 *	\param norm	The normal vector.
 */
void vtMesh::SetVtxNormal(uint i, const FPoint3 &norm)
{
	if (i >= (int) getNormals()->size())
		getNormals()->resize(i + 1);

	getNormals()->at(i) = osg::Vec3(norm.x, norm.y, norm.z);
#ifdef USE_OPENGL_BUFFER_OBJECTS
	getNormals()->dirty();
#endif
}

/**
 * Get the normal of a vertex.
 */
FPoint3 vtMesh::GetVtxNormal(uint i) const
{
	FPoint3 p;
	s2v(getNormals()->at(i), p);
	return p;
}

/**
 * Set the color of a vertex.  This color multiplies with the color of the
 *	material used with the mesh, so if you want the vertex color to be
 *	dominant, use a white material.
 *
 *	\param i		Index of the vertex.
 *	\param color	The color.
 */
void vtMesh::SetVtxColor(uint i, const RGBAf &color)
{
	if (!hasVertexColors())
		return;

	osg::Vec4 s;
	v2s(color, s);

	if (i >= (int) getColors()->size())
		getColors()->resize(i + 1);

	getColors()->at(i) = s;
#ifdef USE_OPENGL_BUFFER_OBJECTS
	getColors()->dirty();
#endif
}

/**
 * Get the color of a vertex.
 */
RGBAf vtMesh::GetVtxColor(uint i) const
{
	if (hasVertexColors())
	{
		RGBAf p;
		s2v(getColors()->at(i), p);
		return p;
	}
	return RGBf(0,0,0);
}

/**
 * Set the texture coordinates of a vertex.  Generally these values are
 *	in the range of 0 to 1, although you can use higher values if you want
 *	repeating tiling.  The components of the texture coordinates are
 *  usually called "u" and "v".
 *
 *	\param i	Index of the vertex.
 *	\param uv	The texture coordinate.
 */
void vtMesh::SetVtxTexCoord(uint i, const FPoint2 &uv)
{
	if (!hasVertexTexCoords())
		return;

	osg::Vec2 s;
	v2s(uv, s);

	// Not sure whether I need this
	if (i >= (int) getTexCoords()->size())
		getTexCoords()->resize(i + 1);

	getTexCoords()->at(i) = s;
#ifdef USE_OPENGL_BUFFER_OBJECTS
	getTexCoords()->dirty();
#endif
}

/**
 * Get the texture coordinates of a vertex.
 */
FPoint2 vtMesh::GetVtxTexCoord(uint i) const
{
	if (hasVertexTexCoords())
	{
		FPoint2 p;
		s2v(getTexCoords()->at(i), p);
		return p;
	}
	return FPoint2(0,0);
}

/**
* Set the line width, in pixels, for this mesh's geometry.
*
* You should call this method _after_ the mesh has been added to some
*  geometry with vtGeode::AddMesh()
* (this requirement was found with the OSG flavor of vtlib.)
*/
void vtMesh::SetLineWidth(float fWidth)
{
	osg::LineWidth *lws = new osg::LineWidth;
	osg::StateSet *ss = getOrCreateStateSet();

	lws->setWidth(fWidth);
	ss->setAttributeAndModes(lws,
		osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
}

/**
 * Set whether to allow rendering optimization of this mesh.  With OpenGL,
 *	this optimization is called a "display list", which increases the speed
 *	of rendering by creating a special representation of the mesh the first
 *	time it is drawn.  The tradeoff is that subsequent changes to the mesh
 *	are not applied unless you call ReOptimize().
 *
 *	\param bAllow	True to allow optimization.  The default is true.
 */
void vtMesh::AllowOptimize(bool bAllow)
{
	setUseDisplayList(bAllow);
}

/**
 * For a mesh with rendering optimization enabled, forces an update of the
 *	optimized representation.
 */
void vtMesh::ReOptimize()
{
	dirtyDisplayList();
	dirtyBound();
}

/**
 * Set the normals of the vertices by combining the normals of the surrounding
 * faces.  This requires going through all the primitives to average their
 * contribution to each vertex.
 */
void vtMesh::SetNormalsFromPrimitives()
{
	// TODO: Add support for TRIANGLE_FAN
	if (getPrimType() != osg::PrimitiveSet::TRIANGLES &&
		getPrimType() != osg::PrimitiveSet::QUADS &&
		getPrimType() != osg::PrimitiveSet::TRIANGLE_STRIP &&
		getPrimType() != osg::PrimitiveSet::POLYGON)
		return;

	osg::Vec3Array *norms = getNormals();
	norms->resize(getVerts()->size());
	for (auto itr : *norms)
		itr.set(0, 0, 0);

	switch (getPrimType())
	{
	case osg::PrimitiveSet::TRIANGLES:		_AddTriangleNormals(); break;
	case osg::PrimitiveSet::TRIANGLE_STRIP:	_AddStripNormals(); break;
	case osg::PrimitiveSet::QUADS:			_AddQuadNormals(); break;
	case osg::PrimitiveSet::POLYGON:		_AddPolyNormals(); break;
	default:	// Keep picky compilers quiet.
		break;
	}

	for (auto itr : *norms)
		itr.normalize();
}

void vtMesh::_AddStripNormals()
{
	uint numPrimitiveSets = getNumPrimitiveSets();
	unsigned short v0 = 0, v1 = 0, v2 = 0;
	osg::Vec3 p0, p1, p2, d0, d1, norm;
	osg::Vec3Array *norms = getNormals();

	for (uint i = 0; i < numPrimitiveSets; i++)
	{
		osg::DrawElements *pDrawElements = getPrimitiveSet(i)->getDrawElements();

		// This actually just returns the size of the element list in this case
		const uint len = pDrawElements->getNumIndices();

		for (uint j = 0; j < len; j++)
		{
			v0 = v1; p0 = p1;
			v1 = v2; p1 = p2;
			v2 = pDrawElements->index(j);
			p2 = getVerts()->at(v2);
			if (j >= 2)
			{
				d0 = (p1 - p0);
				d1 = (p2 - p0);
				d0.normalize();
				d1.normalize();

				norm = d0^d1;

				norms->at(v0) += norm;
				norms->at(v1) += norm;
				norms->at(v2) += norm;
			}
		}
	}
}

void vtMesh::_AddPolyNormals()
{
	uint i, j, len;
	uint NumPrimitiveSets = getNumPrimitiveSets();
	unsigned short v0 = 0, v1 = 0, v2 = 0;
	osg::Vec3 p0, p1, p2, d0, d1, norm;
	osg::Vec3Array *norms = getNormals();

	for (i = 0; i < NumPrimitiveSets; i++)
	{
		osg::DrawElements *pDrawElements = getPrimitiveSet(i)->getDrawElements();
		len = pDrawElements->getNumIndices(); // This actually just returns the size of the element list in this case
		// ensure this poly has enough verts to define a surface
		if (len >= 3)
		{
			v0 = pDrawElements->index(0);
			v1 = pDrawElements->index(1);
			v2 = pDrawElements->index(2);
			p0 = getVerts()->at(v0);
			p1 = getVerts()->at(v1);
			p2 = getVerts()->at(v2);

			d0 = (p1 - p0);
			d1 = (p2 - p0);
			d0.normalize();
			d1.normalize();

			norm = d0^d1;

			for (j = 0; j < len; j++)
			{
				uint v = pDrawElements->index(j);
				getNormals()->at(v) += norm;
			}
		}
	}
}

void vtMesh::_AddTriangleNormals()
{
	int tris = NumPrims();
	unsigned short v0, v1, v2;
	osg::Vec3 p0, p1, p2, d0, d1, norm;

	osg::DrawElements *pDrawElements = getPrimitiveSet(0)->getDrawElements();

	for (int i = 0; i < tris; i++)
	{
		v0 = pDrawElements->index(i*3);
		v1 = pDrawElements->index(i*3+1);
		v2 = pDrawElements->index(i*3+2);
		p0 = getVerts()->at(v0);
		p1 = getVerts()->at(v1);
		p2 = getVerts()->at(v2);

		d0 = (p1 - p0);
		d1 = (p2 - p0);
		d0.normalize();
		d1.normalize();

		norm = d0^d1;

		getNormals()->at(v0) += norm;
		getNormals()->at(v1) += norm;
		getNormals()->at(v2) += norm;
	}
}

void vtMesh::_AddQuadNormals()
{
	int quads = NumPrims();
	unsigned short v0, v1, v2, v3;
	osg::Vec3 p0, p1, p2, d0, d1, norm;

	osg::Vec3Array *norms = getNormals();

	osg::DrawElements *pDrawElements = getPrimitiveSet(0)->getDrawElements();

	for (int i = 0; i < quads; i++)
	{
		v0 = pDrawElements->index(i*4);
		v1 = pDrawElements->index(i*4+1);
		v2 = pDrawElements->index(i*4+2);
		v3 = pDrawElements->index(i*4+3);
		p0 = getVerts()->at(v0);
		p1 = getVerts()->at(v1);
		p2 = getVerts()->at(v2);

		d0 = (p1 - p0);
		d1 = (p2 - p0);
		d0.normalize();
		d1.normalize();

		norm = d0^d1;

		norms->at(v0) += norm;
		norms->at(v1) += norm;
		norms->at(v2) += norm;
		norms->at(v3) += norm;
	}
}


/////////////////////////////////////////////////////////////////////////////
// Text

/**
 * Construct a TextMesh object.
 *
 * \param font The font that will be used to draw the text.
 * \param fSize Size (height) in world units of the text rectangle.
 * \param bCenter If true, the origin of the text rectangle is at
 *			it's bottom center.  Otherwise, bottom left.
 */
vtTextMesh::vtTextMesh(osgText::Font *font, float fSize, bool bCenter)
{
	// Set the Font reference width and height resolution in texels.
	setFont(font);
	setFontResolution(32,32);

	// Set the rendered character size in object coordinates.
	setCharacterSize(fSize);

	if (bCenter)
		setAlignment(osgText::Text::CENTER_BOTTOM);

	// We'd like to turn off lighting for the text, but we can't, because
	//  the OSG Text object fiddles with its own StateSet.  Instead, we do
	//  it in vtGeode::AddTextMesh().
}

// Override with ability to get OSG bounding box
void vtTextMesh::GetBoundBox(FBox3 &box) const
{
#if OSG_VERSION_GREATER_THAN(3, 3, 0)
	const osg::BoundingBox &osg_box = getBoundingBox();
#else
	const osg::BoundingBox &osg_box = getBound();
#endif
	s2v(osg_box, box);
}

void vtTextMesh::SetText(const char *text)
{
	setText(text);
}

void vtTextMesh::SetText(const wchar_t *text)
{
	setText(text);
}

#if SUPPORT_WSTRING
void vtTextMesh::SetText(const std::wstring &text)
{
	setText(text.c_str());
}
#endif

void vtTextMesh::SetPosition(const FPoint3 &pos)
{
	setPosition(osg::Vec3(pos.x, pos.y, pos.z));
}

void vtTextMesh::SetRotation(const FQuat &rot)
{
	setRotation(osg::Quat(rot.x, rot.y, rot.z, rot.w));
}

void vtTextMesh::SetAlignment(int align)
{
	osgText::Text::AxisAlignment osga;

	if (align == 0)
		osga = osgText::Text::XY_PLANE;
	else if (align == 1)
		osga = osgText::Text::XZ_PLANE;
	else if (align == 2)
		osga = osgText::Text::YZ_PLANE;
	else return;

	setAxisAlignment(osga);
}

void vtTextMesh::SetColor(const RGBAf &rgba)
{
	setColor(osg::Vec4(rgba.r, rgba.g, rgba.b, rgba.a));
}
