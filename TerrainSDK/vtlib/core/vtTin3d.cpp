//
// vtTin3d.cpp
//
// Class which represents a Triangulated Irregular Network.
//
// Copyright (c) 2002-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/FilePath.h"	// for FindFileOnPaths
#include "vtdata/DataPath.h"
#include "vtdata/vtLog.h"

#include "vtTin3d.h"
#include "SurfaceTexture.h"		// For LoadColorMap
#include "Light.h"

// We will split the TIN into chunks of geometry, each with no more than this many vertices.
const int kMaxChunkVertices = 10000;
const int kColorMapTableSize = 8192;


vtTin3d::vtTin3d()
{
	m_pMats = NULL;
	m_pGeode = NULL;
	m_pDropGeode = NULL;
	m_pColorMap = NULL;
}

vtTin3d::~vtTin3d()
{
	delete m_pColorMap;
}

/**
 * Read the TIN from a file.  This can either be an old-style or new-style
 * .tin format (so far, a VTP-specific format)
 */
bool vtTin3d::Read(const char *fname, bool progress_callback(int))
{
	if (!vtTin::Read(fname, progress_callback))
		return false;

	Initialize(m_crs.GetUnits(), m_EarthExtents, m_fMinHeight, m_fMaxHeight);
	return true;
}

FPoint3 ComputeNormal(const FPoint3 &p1, const FPoint3 &p2, const FPoint3 &p3)
{
	FPoint3 d1 = (p2 - p1);
	FPoint3 d2 = (p3 - p1);
	FPoint3 cross = d1.Cross(d2);
	cross.Normalize();
	return cross;
}

void vtTin3d::SetMaterial(vtMaterialArray *pMats, int mat_idx)
{
	m_pMats = pMats;
	m_MatIndex = mat_idx;
}

void vtTin3d::MakeSurfaceMaterials()
{
	uint iSurfTypes = m_surftypes.size();
	bool bExplicitNormals = HasVertexNormals();

	m_StartOfSurfaceMaterials = m_pMats->size();

	for (uint i = 0; i < iSurfTypes; i++)
	{
		bool bLighting = bExplicitNormals;
		float fAmbient = 0.3f;

		// Might be absolute path
		vtString path = FindFileOnPaths(vtGetDataPath(), m_surftypes[i]);

		// Might be relative path
		if (path == "")
		{
			vtString relpath = "GeoTypical/";
			relpath += m_surftypes[i];
			path = FindFileOnPaths(vtGetDataPath(), relpath);
		}

		osg::Image *image = LoadOsgImage(path);
		int idx = m_pMats->AddTextureMaterial(image, false, bLighting, false, false,
			fAmbient, 1.0f, 1.0f, 0.0f);
		if (idx != -1)
			m_pMats->at(idx)->SetMipMap(true);
	}
}

/**
 High-level method to make materials from layer options.
 */
void vtTin3d::MakeMaterialsFromOptions(const vtTagArray &options, bool bTextureCompression)
{
	VTLOG1("vtTin3d::MakeMaterialsFromOptions\n");

	ScopedLocale normal_numbers(LC_NUMERIC, "C");	// for GetValueFloat

	const vtString color_map_name = options.GetValueString(STR_COLOR_MAP);
	const vtString geotypical_name = options.GetValueString(STR_TEXTURE_GEOTYPICAL);

	const float fScale = options.GetValueFloat(STR_GEOTYPICAL_SCALE);
	const float fOpacity = options.GetValueFloat(STR_OPACITY);

	vtString name = options.GetValueString(STR_COLOR_MAP);

	ColorMap *cmap = NULL;
	osg::Image *image = NULL;
	if (color_map_name != "")
	{
		cmap = LoadColorMap(color_map_name);
	}
	if (geotypical_name != "")
	{
		// Geotypical material
		vtString fname = "Geotypical/";
		fname += geotypical_name;
		vtString path = FindFileOnPaths(vtGetDataPath(), fname);
		image = osgDB::readImageFile((const char *) path);
	}
	MakeMaterials(cmap, image, fScale, fOpacity, bTextureCompression);
}

/**
 Lower-level method to make materials.
 */
void vtTin3d::MakeMaterials(ColorMap *cmap, osg::Image *image, float fScale,
	float fOpacity, bool bTextureCompression)
{
	VTLOG1("vtTin3d::MakeMaterials\n");

	// set up geotypical materials
	m_pMats = new vtMaterialArray;

	// White: used for 1D-textured terrain surface
	bool lighting = true;
	bool culling = false;
	m_MatIndex = m_pMats->AddRGBMaterial(RGBf(1, 1, 1), culling, lighting, false, fOpacity);

	// Grey: used for drop shadow plane
	m_ShadowMatIndex = m_pMats->AddRGBMaterial(RGBf(0.4f, 0.4f, 0.4f), false, false, false);

	vtMaterial *pMat = m_pMats->at(m_MatIndex);

	if (cmap)
	{
		SetColorMap(cmap);

		// Setup TexGen for a 1D texture.
		int unit = pMat->NextAvailableTextureUnit();
		VTLOG("  ColorMap using texture unit %d\n", unit);

		// Rather than look through the color map for each pixel, pre-build
		//  a color lookup table once - should be faster in nearly all cases.
		float fMin, fMax;
		GetHeightExtents(fMin, fMax);
		m_pColorMap->GenerateColorTable(4096, fMin, fMax);

		// A 1D texture to color by elevation: copy from colormap to an image.
		osg::Image *image1d = new osg::Image;
		image1d->allocateImage(4096, 1, 1, GL_RGB, GL_UNSIGNED_BYTE);
		for (int i = 0; i < 4096; i++)
		{
			RGBi rgb = m_pColorMap->m_table[i];

			int x = i, y = 0;
			uchar *buf = image1d->data(x, y);
			buf[0] = rgb.r;
			buf[1] = rgb.g;
			buf[2] = rgb.b;
		}
		pMat->SetTexture1D(image1d, unit);

		// Set TexGen to scale from min to max
		float fRange = fMax - fMin;
		pMat->SetTexGen1D(FPoint3(0, 1/fRange, 0), -fMin / fRange, unit);
		pMat->SetTextureMode(GL_MODULATE, unit);
	}
	if (image)
	{
		int unit = pMat->NextAvailableTextureUnit();
		VTLOG("  GeoTypical using texture unit %d\n", unit);

		pMat->SetTexture2D(image, unit, bTextureCompression);
		pMat->SetClamp(false, unit);
		// A geotypical texture.  Setup TexGen.
		pMat->SetTexGen2D(FPoint2(1.0f/fScale, 1.0f/fScale), FPoint2(0, 0), unit);
		pMat->SetTextureMode(GL_MODULATE, unit);
	}
}

vtMaterial *vtTin3d::GetSurfaceMaterial()
{
	if (m_pMats != NULL && m_MatIndex != -1)
		return m_pMats->at(m_MatIndex);
	return NULL;
}

vtGeode *vtTin3d::CreateGeometry(bool bDropShadowMesh)
{
	VTLOG1("vtTin3d::CreateGeometry\n");

	uint iSurfTypes = m_surftypes.size();
	bool bUseSurfaceTypes = (m_surfidx.size() > 0 && iSurfTypes > 0);
	bool bExplicitNormals = HasVertexNormals();
	bool bUseVertexColors = !bUseSurfaceTypes;

	m_pGeode = new vtGeode;
	m_pGeode->SetMaterials(m_pMats);

	if (bUseSurfaceTypes)
	{
		MakeSurfaceMaterials();
	}

	// Break it up into a series of meshes - this is good for both culling and
	// memory management.
	DPoint3 ep;		// earth point
	FPoint3 wp;		// world point
	FPoint3 p[3], norm;

	// Most TINs are larger in the horizontal dimension than the vertical, so
	// use horizontal extents as the basis of subdivision
	DRECT rect = m_EarthExtents;

	// Make it slightly larger to avoid edge conditions
	rect.Grow(0.000001, 0.000001);

	const DPoint2 EarthSize = rect.SizeExtents();

	int divx, divy;		// number of x and y divisions
	uint dsize = 0;
	Bin *bins = NULL;
	uint tris = NumTris();
	vtArray<vtMesh *> pTypeMeshes;

	uint i, j, k;
	int divs = 4;
	bool acceptable = false;
	while (!acceptable)
	{
		// Take the smaller dimension and split it to ensure a minimum level
		// of subdivision, with the larger dimension proportional
		if (EarthSize.x < EarthSize.y)
		{
			divx = divs;
			divy = (int) (divx * EarthSize.y / EarthSize.x);
		}
		else
		{
			divy = divs;
			divx = (int) (divy * EarthSize.x / EarthSize.y);
		}

		// Create a 2D array of Bins
		dsize = divx*divy;
		bins = new Bin[dsize];

		// See how many triangles would go into each bin
		uint most = 0;
		for (i = 0; i < tris; i++)
		{
			j = i * 3;
			const DPoint2 &gp = (m_vert[m_tri[j]] + m_vert[m_tri[j+1]] + m_vert[m_tri[j+2]]) / 3;
			const int bx = (int) (divx * (gp.x - rect.left) / EarthSize.x);
			const int by = (int) (divy * (gp.y - rect.bottom) / EarthSize.y);

			Bin &bref = bins[bx * divy + by];
			bref.push_back(i);
			const uint newsize = bref.size();
			if (newsize > most)
				most = newsize;
		}
		if (most > kMaxChunkVertices)
		{
			// Too many in one bin, increase the divisions and try again.
			delete [] bins;
			divs = divs * 3 / 2;
		}
		else
			acceptable = true;
	}

	FPoint3 light_dir(0.5, 1, 0);
	light_dir.Normalize();

	uint in_bin;
	int tri, vidx;

	// If the material is textured, it will use TexGen so we don't need texture
	// coordinate per vertex.
	int vert_type = 0;
	if (bUseSurfaceTypes)
		vert_type = VT_TexCoords;

	// We always have normals for lighting
	vert_type |= VT_Normals;

	for (i = 0; i < dsize; i++)
	{
		Bin &bref = bins[i];
		in_bin = bref.size();
		if (!in_bin)
			continue;

		vtMesh *pMesh = NULL;
		if (bUseSurfaceTypes)
		{
			pTypeMeshes.SetSize(iSurfTypes);
			for (j = 0; j < iSurfTypes; j++)
				pTypeMeshes[j] = NULL;
		}
		else
		{
			// simple case: this whole bin goes into one mesh
			pMesh = new vtMesh(osg::PrimitiveSet::TRIANGLES, vert_type, in_bin * 3);
		}

		for (j = 0; j < in_bin; j++)
		{
			tri = bref[j];
			int tribase = tri * 3;

			for (k = 0; k < 3; k++)
			{
				vidx = m_tri[tribase + k];
				ep.Set(m_vert[vidx].x, m_vert[vidx].y, m_z[vidx]);
				m_LocalCS.EarthToLocal(ep, p[k]);
			}
			norm = ComputeNormal(p[0], p[1], p[2]);

			float shade = norm.Dot(light_dir);	// shading 0 (dark) to 1 (light)
			if (shade < 0)
				shade = -shade;

			float fTiling;
			if (bUseSurfaceTypes)
			{
				// We mush pick a mesh based on surface type
				int surftype = m_surfidx[tri];
				if (pTypeMeshes[surftype] == NULL)
					pTypeMeshes[surftype] = new vtMesh(osg::PrimitiveSet::TRIANGLES,
						vert_type, in_bin * 3);
				pMesh = pTypeMeshes[surftype];
				fTiling = m_surftype_tiling[surftype];
			}

			int vert_base = pMesh->NumVertices();
			for (k = 0; k < 3; k++)
			{
				vidx = m_tri[tribase + k];

				// This is where we actually add the vertex
				int vert_index = pMesh->AddVertex(p[k]);
				if (bUseSurfaceTypes)
				{
					FPoint2 uv;
					uv.Set((m_vert[vidx].x - m_EarthExtents.left) / fTiling,
							(m_vert[vidx].y - m_EarthExtents.bottom) / fTiling);
					pMesh->SetVtxTexCoord(vert_index, uv);
				}
				if (bExplicitNormals)
					pMesh->SetVtxNormal(vert_index, m_vert_normal[vidx]);
				else
					pMesh->SetVtxNormal(vert_index, norm);
			}
			pMesh->AddTri(vert_base, vert_base+1, vert_base+2);
		}
		if (bUseSurfaceTypes)
		{
			for (j = 0; j < iSurfTypes; j++)
			{
				if (pTypeMeshes[j] != NULL)
				{
					m_pGeode->AddMesh(pTypeMeshes[j], m_StartOfSurfaceMaterials + j);
					m_Meshes.Append(pTypeMeshes[j]);
				}
			}
		}
		else
		{
			// Simple case
			m_pGeode->AddMesh(pMesh, m_MatIndex);
			m_Meshes.Append(pMesh);
		}
	}

	// Free up temp arrays
	delete [] bins;

	if (bDropShadowMesh)
	{
		vtMesh *pBaseMesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_FAN, 0, 4);

		ep.Set(m_EarthExtents.left - 10, m_EarthExtents.bottom - 10, m_fMinHeight - 5);
		m_LocalCS.EarthToLocal(ep, wp);
		pBaseMesh->AddVertex(wp);

		ep.Set(m_EarthExtents.right + 10, m_EarthExtents.bottom - 10, m_fMinHeight - 5);
		m_LocalCS.EarthToLocal(ep, wp);
		pBaseMesh->AddVertex(wp);

		ep.Set(m_EarthExtents.right + 10, m_EarthExtents.top + 10, m_fMinHeight - 5);
		m_LocalCS.EarthToLocal(ep, wp);
		pBaseMesh->AddVertex(wp);

		ep.Set(m_EarthExtents.left - 10, m_EarthExtents.top + 10, m_fMinHeight - 5);
		m_LocalCS.EarthToLocal(ep, wp);
		pBaseMesh->AddVertex(wp);

		pBaseMesh->AddFan(0, 1, 2, 3);
		m_pGeode->AddMesh(pBaseMesh, m_ShadowMatIndex);
	}

	// The TIN is a large geometry which should not attempt to cast a shadow,
	//  because shadow algos tend to support only small regions of casters.
	m_pGeode->SetCastShadow(false);

	return m_pGeode;
}

/**
 * Returns true if the point was over the TIN, false otherwise.
 */
bool vtTin3d::FindAltitudeAtPoint(const FPoint3 &input, float &fAltitude,
								  bool bTrue, int iCultureFlags, FPoint3 *vNormal) const
{
	// Look on culture first
	if (iCultureFlags != 0 && m_pCulture != NULL)
	{
		if (m_pCulture->FindAltitudeOnCulture(input, fAltitude, bTrue, iCultureFlags))
			return true;
	}
	return vtTin::FindAltitudeAtPoint(input, fAltitude, bTrue, iCultureFlags, vNormal);

#if 0
	// We could test against the 3D triangles with the code below, but in
	//  practice, i found it is actually faster to just use the parent class
	//  (vtTin) to test against the 2D triangles.

	FPoint2 p(input.x, input.z);

	FPoint3 wp1, wp2, wp3;
	FPoint2 p1, p2, p3;
	bool good;

	for (uint m = 0; m < m_Meshes.GetSize(); m++)
	{
		vtMesh *mesh = m_Meshes[m];
		int tris = mesh->NumVertices() / 3;
		for (int i = 0; i < tris; i++)
		{
			// get world points
			wp1 = mesh->GetVtxPos(i*3+0);
			wp2 = mesh->GetVtxPos(i*3+1);
			wp3 = mesh->GetVtxPos(i*3+2);
			// convert to 2d
			p1.Set(wp1.x, wp1.z);
			p2.Set(wp2.x, wp2.z);
			p3.Set(wp3.x, wp3.z);

			if (!PointInTriangle(p, p1, p2, p3))
				continue;

			// compute barycentric coordinates with respect to the triangle
			float bary[3], val;
			good = BarycentricCoords(p1, p2, p3, p, bary);
			if (!good)
				continue;

			// compute barycentric combination of function values at vertices
			val = bary[0]*wp1.y +
				  bary[1]*wp2.y +
				  bary[2]*wp3.y;
			fAltitude = val;
			return true;
		}
	}
	fAltitude = 0;
	return false;
#endif
}


/*
 * Algorithm from 'Fast, Minimum Storage Ray-Triangle Intersection',
 * Thomas Möller and Ben Trumbore, 1997
 * Adapted to use C++ and the vtdata math classes.
 */
#define EPSILON 0.000001
bool intersect_triangle(const FPoint3 &orig, const FPoint3 &dir,
			const FPoint3 &vert0, const FPoint3 &vert1, const FPoint3 &vert2,
			float &t, float &u, float &v)
{
	FPoint3 edge1, edge2, tvec, pvec, qvec;
	float det,inv_det;

	/* find vectors for two edges sharing vert0 */
	edge1 = vert1 - vert0;
	edge2 = vert2 - vert0;

	/* begin calculating determinant - also used to calculate U parameter */
	pvec = dir.Cross(edge2);

	/* if determinant is near zero, ray lies in plane of triangle */
	det = edge1.Dot(pvec);

	if (det < EPSILON)
		return false;

	/* calculate distance from vert0 to ray origin */
	tvec = orig - vert0;

	/* calculate U parameter and test bounds */
	u = tvec.Dot(pvec);
	if (u < 0.0 || u > det)
		return false;

	/* prepare to test V parameter */
	qvec = tvec.Cross(edge1);

	 /* calculate V parameter and test bounds */
	v = dir.Dot(qvec);
	if (v < 0.0 || u + v > det)
		return false;

	/* calculate t, scale parameters, ray intersects triangle */
	t = edge2.Dot(qvec);
	inv_det = 1.0f / det;
	t *= inv_det;
	u *= inv_det;
	v *= inv_det;

	return true;
}

bool vtTin3d::CastRayToSurface(const FPoint3 &point, const FPoint3 &dir,
							   FPoint3 &result) const
{
	FPoint3 wp1, wp2, wp3;
	float t, u, v, closest = 1E9;
	int i;

	for (uint m = 0; m < m_Meshes.GetSize(); m++)
	{
		vtMesh *mesh = m_Meshes[m];

		// Try to quickly avoid some meshes by testing against their bounding
		// spheres.
		FBox3 box;
		mesh->GetBoundBox(box);
		FSphere sph(box);

		int iQuantity;
		FPoint3 interpoints[2];
		if (!RaySphereIntersection(point, dir, sph, iQuantity, interpoints))
		{
			continue;
		}

		int tris = mesh->NumVertices() / 3;
		for (i = 0; i < tris; i++)
		{
			// get world points
			wp1 = mesh->GetVtxPos(i*3+0);
			wp2 = mesh->GetVtxPos(i*3+1);
			wp3 = mesh->GetVtxPos(i*3+2);
			if (intersect_triangle(point, dir, wp1, wp2, wp3, t, u, v))
			{
				if (t < closest)
					closest = t;
			}
		}
	}
	if (closest == 1E9)
		return false;

	result = point + (dir * closest);
	return true;
}

FPoint3 vtTin3d::FindVectorToClosestVertex(const FPoint3 &pos)
{
	FPoint3 vert, diff, closest_diff;
	float len, minlen = 1E9;

	for (uint m = 0; m < m_Meshes.GetSize(); m++)
	{
		vtMesh *mesh = m_Meshes[m];
		int points = mesh->NumVertices();
		for (int i = 0; i < points; i++)
		{
			vert = mesh->GetVtxPos(i);
			diff = vert - pos;
			len = diff.Length();
			if (len < minlen)
			{
				minlen = len;
				closest_diff = diff;
			}
		}
	}
	return closest_diff;
}

