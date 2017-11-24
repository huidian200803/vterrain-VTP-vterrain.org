//
// Globe.cpp
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "Engine.h"
#include "Terrain.h"
#include "TerrainScene.h"	// For iterating terrains
#include "GeomFactory.h"

#include "vtdata/shapelib/shapefil.h"

#include "Globe.h"
#include "vtdata/vtLog.h"	// for logging debug message
#include "vtdata/DataPath.h"

// Helper
vtTransform *WireAxis(RGBf color, float len);


/////////////////////////////////////////////////////

vtIcoGlobe::vtIcoGlobe()
{
	m_top = NULL;
	m_SurfaceGroup = NULL;
	m_pAxisGeom = NULL;
	m_coremats = NULL;
	m_earthmats = NULL;
	m_bUnfolded = false;
	m_bTilt = false;
	m_cylinder = NULL;
}

vtIcoGlobe::~vtIcoGlobe()
{
	for (uint i = 0; i < m_GlobeLayers.GetSize(); i++)
		delete m_GlobeLayers[i];
}


/**
 * Create the globe's geometry and nodes.
 *
 * \param iTriangleCount The desired triangle count of the entire globe.  The
 *		class will attempt to match this value as closely as possible with
 *		the indicated tessellation.
 * \param strImagePrefix The base of the filename for the set of icosahedral
 *		surface textures.  For example, if your textures have the name
 *		"geosphere_*.jpg", pass "geosphere_"
 * \param style Tessellation style, can be one of:
 *		- GEODESIC	A classic geodesic tiling based on subdividing the edges
 *			of the icosahedron and gnomonically projecting them to the sphere.
 *		- RIGHT_TRIANGLE	An alternative approach where each face is
 *			divided into right triangles recursively.
 *		- DYMAX_UNFOLD	Same as RIGHT_TRIANGLE but the faces are also placed
 *			on seperate geometries so that the globe can be unfolded in the
 *			Dymaxion style.
 */
void vtIcoGlobe::Create(int iTriangleCount, const vtString &strImagePrefix, Style style)
{
	VTLOG("vtIcoGlobe::Create\n");

	m_style = style;
	CreateMeshMat(iTriangleCount);
	CreateCoreMaterials();
	SetEarthMaterials(CreateMaterialsFromFiles(strImagePrefix));
	CreateNodes();
}

void vtIcoGlobe::Create(int iTriangleCount, vtImage **images, Style style)
{
	VTLOG("vtIcoGlobe::Create\n");

	m_style = style;
	CreateMeshMat(iTriangleCount);
	CreateCoreMaterials();
	SetEarthMaterials(CreateMaterialsFromImages(images));
	CreateNodes();
}

void vtIcoGlobe::SetEarthMaterials(vtMaterialArray *mats)
{
	m_earthmats = mats;
	// TODO: set the materials on all the component earth geometries
}

vtMaterialArray *vtIcoGlobe::GetEarthMaterials()
{
	return m_earthmats;
}

void vtIcoGlobe::CreateMeshMat(int iTriangleCount)
{
	EstimateTesselation(iTriangleCount);

	// Estimate number of meshes, and number of vertices per mesh
	int numvtx_per_mesh = 0;
	vtMesh::PrimType prim_type;
	if (m_style == GEODESIC)
	{
		prim_type = osg::PrimitiveSet::TRIANGLE_STRIP;
		m_meshes = 20;
		numvtx_per_mesh = (m_freq + 1) * (m_freq + 2) / 2;

		// 1 material per face pair
		m_globe_mat.resize(10);
	}
	else if (m_style == INDEPENDENT_GEODESIC)
	{
		prim_type = osg::PrimitiveSet::TRIANGLES;

		m_meshes = 20 * m_freq * m_freq;
		m_subfreq = 1;	// 1 for now
		numvtx_per_mesh = (m_subfreq + 1) * (m_subfreq + 2) / 2;

		// 1 material per mesh pair
		m_globe_mat.resize(10 * m_freq * m_freq);
	}
	else if (m_style == RIGHT_TRIANGLE || m_style == DYMAX_UNFOLD)
	{
		prim_type = osg::PrimitiveSet::TRIANGLE_STRIP;

		m_meshes = (m_style == RIGHT_TRIANGLE) ? 20 : 22;
		numvtx_per_mesh = 1 + 2 * ((int) pow((double)3, m_depth+1));

		// 1 material per face pair
		m_globe_mat.resize(10);
	}

	for (int i = 0; i < m_meshes; i++)
	{
		int vertex_type = VT_Normals | VT_TexCoords;
		vtMesh *mesh = new vtMesh(prim_type, vertex_type, numvtx_per_mesh);

		// We will be inflating, unfolding etc. which requires disabling the
		//  default display lists.
		mesh->AllowOptimize(false);
		m_mesh.push_back(mesh);
	}
}

void vtIcoGlobe::CreateCoreMaterials()
{
	m_coremats = new vtMaterialArray;
	m_red = m_coremats->AddRGBMaterial(RGBf(1.0f, 0.0f, 0.0f),	// red
					 false, false, true);
	m_yellow = m_coremats->AddRGBMaterial(RGBf(1.0f, 1.0f, 0.0f),	// yellow
					 false, false, false);
	m_white = m_coremats->AddRGBMaterial(RGBf(0.2f, 0.2f, 0.2f),
					 true, true, true, 1);
	m_coremats->at(m_white)->SetTransparent(true, true);
}

vtMaterialArray *vtIcoGlobe::CreateMaterialsFromFiles(const vtString &strImagePrefix)
{
	vtMaterialArray *mats = new vtMaterialArray;

#if 0
	if (m_style == INDEPENDENT_GEODESIC)
	{
		int mat = 0;
		for (int pair = 0; pair < 10; pair++)
		{
			for (int j = 0; j < m_freq; j++)
			for (int i = 0; i < m_freq; i++)
			{
				// TEMP for testing: pretty colors
				float r = (pair+1) * (1.0f / 10);
				float g = (j+1) * (1.0f / m_freq);
				float b = (i+1) * (1.0f / m_freq);
				m_globe_mat[mat++] = mats->AddRGBMaterial(RGBf(r, g, b),
						true, true);
			}
		}
		return;
	}
#endif
	vtString base;
	vtString fname;
	vtString fullpath;

	bool bCulling = true;
	bool bLighting = false;
	int pair, index;
	for (pair = 0; pair < 10; pair++)
	{
		if (strImagePrefix == "")
		{
			m_globe_mat[pair] = m_white;
			continue;
		}
		int f1 = icosa_face_pairs[pair][0];
		int f2 = icosa_face_pairs[pair][1];

		base = "WholeEarth/";
		base += strImagePrefix;
		base += "_";

		fname.Format("%s%02d%02d.jpg", (const char *)base, f1+1, f2+1);
		VTLOG("\t texture: %s\n", (const char *)fname);

		fullpath = FindFileOnPaths(vtGetDataPath(), (const char *)fname);
		if (fullpath == "")
		{
			// try again with png
			fname.Format("%s%02d%02d.png", (const char *)base, f1+1, f2+1);
			VTLOG("\t texture: %s\n", (const char *)fname);
			fullpath = FindFileOnPaths(vtGetDataPath(), (const char *)fname);
		}
		if (fullpath == "")
		{
			VTLOG("\t\tnot found on data paths.\n");
			index = -1;
		}
		else
		{
			ImagePtr img = osgDB::readImageFile((const char *)fullpath);
			if (img.valid())
			{
				index = mats->AddTextureMaterial(img,
							bCulling, bLighting,
							GetDepth(img) == 32, false,	// transp, additive
							0.1f, 1.0f, 1.0f, 0.0f);	// ambient, diffuse, alpha, emmisive
				mats->at(index)->SetClamp(false);
			}
			else
				index = -1;
		}
		if (index == -1)
		{
			VTLOG("\t\ttexture load failed, using red material.\n");
			m_globe_mat[pair] = m_red;
		}
		else
			m_globe_mat[pair] = index;
	}
	return mats;
}

vtMaterialArray *vtIcoGlobe::CreateMaterialsFromImages(vtImage **images)
{
	vtMaterialArray *mats = new vtMaterialArray;
	bool bCulling = true;
	bool bLighting = false;
	for (int pair = 0; pair < 10; pair++)
	{
		vtImage *img = images[pair];
		int index = mats->AddTextureMaterial(img,
					 bCulling, bLighting,
					 img->GetDepth() == 32, false,	// transp, additive
					 0.1f, 1.0f, 1.0f, 0.0f);		// ambient, diffuse, alpha, emmisive
		mats->at(index)->SetClamp(true);
		m_globe_mat[pair] = index;
	}
	return mats;
}

void vtIcoGlobe::CreateNodes()
{
	m_top = new vtTransform;
	m_top->setName("GlobeXForm");

	// Create groups to contain the surface features
	m_SurfaceGroup = new vtGroup;
	m_SurfaceGroup->setName("SurfaceGroup");
	m_top->addChild(m_SurfaceGroup);

	if (m_style == DYMAX_UNFOLD)
		CreateUnfoldableDymax();
	else if (m_style == INDEPENDENT_GEODESIC)
		CreateIndependentGeodesicSphere();
	else
		CreateNormalSphere();
}

/**
 * Set the amount of inflation of the globe.
 *
 * \param f Ranges from 0 (icosahedron) to 1 (sphere)
 */
void vtIcoGlobe::SetInflation(float f)
{
	for (int i = 0; i < m_meshes; i++)
	{
		if (m_style == GEODESIC)
			set_face_verts1(m_mesh[i], i, f);
		else if (m_style == RIGHT_TRIANGLE || m_style == DYMAX_UNFOLD)
			set_face_verts2(m_mesh[i], i, f);
	}
}

/**
 * Set the amount of unfolding of the globe.
 *
 * \param f Ranges from 0 (sphere/icosahedron) to 1 (entirely unfolded flat)
 */
void vtIcoGlobe::SetUnfolding(float f)
{
	// only possible on unfoldable globes
	if (m_style != DYMAX_UNFOLD)
		return;

	m_bUnfolded = (f != 0.0f);
	int i;

	m_SurfaceGroup->SetEnabled(!m_bUnfolded);
	for (i = 0; i < 22; i++)
		m_mface[i].surfgroup->SetEnabled(f == 1.0f);

	float dih = (float) DihedralAngle();
	for (i = 1; i < 22; i++)
	{
		FPoint3 pos = m_mface[i].xform->GetTrans();
		m_mface[i].xform->Identity();
		m_mface[i].xform->SetTrans(pos);
		m_mface[i].xform->RotateLocal(m_mface[i].axis, -f * dih);
	}

	// gradually undo the current top rotation
	FQuat qnull(0,0,0,1);
	FQuat q1, q2;
	FMatrix3 m3;
	FMatrix4 m4;

	q1.Slerp(qnull, m_Rotation, 1-f);
	q1.GetMatrix(m3);
	m4 = m3;
	m_top->SetTransform(m3);

	// interpolate from No rotation to the desired rotation
	q2.Slerp(qnull, m_diff, f);
	q2.GetMatrix(m3);
	m4 = m3;
	m_mface[0].xform->SetTransform(m3);
}

void vtIcoGlobe::SetCulling(bool bCull)
{
	int pair;
	for (pair = 0; pair < 10; pair++)
	{
		vtMaterial *mat = m_earthmats->at(m_globe_mat[pair]);
		mat->SetCulling(bCull);
	}
}

void vtIcoGlobe::SetLighting(bool bLight)
{
	for (uint i = 0; i < m_earthmats->size(); i++)
	{
		vtMaterial *pApp = m_earthmats->at(m_globe_mat[i]);
		pApp->SetLighting(bLight);
	}
}

/*
 * Set the time for the globe.  The globe will orient itself to properly
 * reflect the indicate date and time, including seasonal shift of the
 * polar axis and time of day.
 *
 * \param time A time value (assumed to be Greenwich Mean Time).
 */
void vtIcoGlobe::SetTime(const vtTime &time)
{
	float second_of_day = (float) time.GetSecondOfDay();
	float fraction_of_day = second_of_day / (24 * 60 * 60);
	float rotation = fraction_of_day * PI2f;

	// match with actual globe
	rotation += PID2f;

	FMatrix4 tmp, m4;
	m4.Identity();

	if (m_bTilt)
	{
		FPoint3 xvector(1,0,0), seasonal_axis;

		// Seasonal axis rotation (days since winter solstice, approximate)
		float season = (time.GetTM().tm_yday + 10) / 365.0f * PI2f;
		tmp.AxisAngle(FPoint3(0,1,0), season);
		tmp.Transform(xvector, seasonal_axis);

		// The earth's axis is tilted with respect to the plane of its orbit
		// at an angle of about 23.4 degrees.  Tilting the northern
		// hemisphere away from the sun (-tilt) puts this at the winter
		// solstice.
		float tilt = 23.4 / 180.0f * PIf;
		tmp.AxisAngle(seasonal_axis, -tilt);
		m4.PreMult(tmp);
	}

	// rotation around axis
	tmp.AxisAngle(FPoint3(0,1,0), rotation);
	m4.PreMult(tmp);

	// store rotation to a quaternion
	FMatrix3 m3 = m4;
	m_Rotation.SetFromMatrix(m3);

	// don't do time rotation on unfolded(ing) globes
	if (!m_bUnfolded)
		m_top->SetTransform(m4);
}

void vtIcoGlobe::ShowAxis(bool bShow)
{
	if (m_pAxisGeom)
		m_pAxisGeom->SetEnabled(bShow);
}


///////////////////////////////////////////////////////////////////////
// Surface feature methods

int vtIcoGlobe::AddGlobeFeatures(const char *fname, float fSize)
{
	vtFeatureLoader loader;

	vtFeatureSet *feat = loader.LoadFromSHP(fname);

	if (!feat)
		return -1;

	GlobeLayer *gl = new GlobeLayer;
	gl->m_pSet = feat;

	m_GlobeLayers.Append(gl);

	BuildSphericalFeatures(gl, fSize);
	BuildFlatFeatures(gl, fSize);

	return feat->NumEntities();
}

void vtIcoGlobe::RemoveLayer(GlobeLayer *glay)
{
	glay->DestructGeometry();
	int idx = m_GlobeLayers.Find(glay);
	if (idx != -1)
		m_GlobeLayers.RemoveAt(idx);
}

void vtIcoGlobe::BuildSphericalFeatures(GlobeLayer *glay, float fSize)
{
	if (glay->m_pSet->GetGeomType() == wkbPoint)
		BuildSphericalPoints(glay, fSize);

	if (glay->m_pSet->GetGeomType() == wkbLineString)
		BuildSphericalLines(glay, fSize);

	if (glay->m_pSet->GetGeomType() == wkbPolygon)
		BuildSphericalPolygons(glay, fSize);
}

void vtIcoGlobe::BuildSphericalPoints(GlobeLayer *glay, float fSize)
{
	vtFeatureSet *feat = glay->m_pSet;
	int i, j, size;
	vtArray<FSphere> spheres;

	size = feat->NumEntities();
	spheres.SetSize(size);

	vtFeatureSetPoint2D *pSetP2 = dynamic_cast<vtFeatureSetPoint2D*>(feat);
	if (!pSetP2)
		return;

	DPoint2 p;
	for (i = 0; i < size; i++)
	{
		pSetP2->GetPoint(i, p);

		if (p.x == 0.0 && p.y == 0.0)	// ignore some
			continue;

		FPoint3 loc;
		geo_to_xyz(1.0, p, loc);

		spheres[i].center = loc;
		spheres[i].radius = fSize;
	}

	FPoint3 diff;

	// volume of sphere, 4/3 PI r^3
	// surface area of sphere, 4 PI r^2
	// area of circle of sphere as seen from distance, PI r^2
	int merges;
	do {
		merges = 0;
		// Try merging overlapping points together, so that information
		// is not lost in the overlap.
		// To consider: do we combine the blobs based on their 2d radius,
		// their 2d area, their 3d radius, or their 3d volume?  See
		// Tufte, http://www.edwardtufte.com/
		// Implemented here: preserve 2d area
		for (i = 0; i < size-1; i++)
		{
			for (j = i+1; j < size; j++)
			{
				if (spheres[i].radius == 0.0f || spheres[j].radius == 0.0f)
					continue;

				diff = spheres[i].center - spheres[j].center;

				// if one sphere contains the center of the other
				if (diff.Length() < spheres[i].radius ||
					diff.Length() < spheres[j].radius)
				{
					// combine
					float area1 = PIf * spheres[i].radius * spheres[i].radius;
					float area2 = PIf * spheres[j].radius * spheres[j].radius;
					float combined = (area1 + area2);
					float newrad = sqrtf( combined / PIf );
					// larger eats the smaller
					if (area1 > area2)
					{
						spheres[i].radius = newrad;
						spheres[j].radius = 0.0f;
					}
					else
					{
						spheres[j].radius = newrad;
						spheres[i].radius = 0.0f;
					}
					merges++;
					break;
				}
			}
		}
	}
	while (merges != 0);

	// Now create and place the little geometry objects to represent the
	// point data.

#if 0
	// create simple hemisphere mesh
	int res = 6;
	vtMesh *mesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_STRIP, 0, res*res*2);
	FPoint3 scale(1.0f, 1.0f, 1.0f);
	mesh->CreateEllipsoid(scale, res, true);
#else
	// create cylinder mesh instead
	int res = 14;
	int verts = res * 2;
	vtMesh *mesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_STRIP, 0, verts);
	mesh->CreateCylinder(1.0f, 1.0f, res, true, false, false);
#endif

	// use Area to show amount, otherwise height
	bool bArea = true;

	// create and place the geometries
	size = spheres.GetSize();
	for (i = 0; i < size; i++)
	{
		if (spheres[i].radius == 0.0f)
			continue;

		vtGeode *geode = new vtGeode;
		geode->SetMaterials(m_coremats);
		geode->AddMesh(mesh, m_yellow);

		vtMovGeode *mgeom = new vtMovGeode(geode);
		mgeom->setName("GlobeShape");

		mgeom->PointTowards(spheres[i].center);
		mgeom->RotateLocal(FPoint3(1,0,0), -PID2f);
		mgeom->SetTrans(spheres[i].center);
		if (bArea)
		{
			// scale just the radius of the cylinder
			mgeom->Scale(spheres[i].radius, 0.001f, spheres[i].radius);
		}
		else
		{
			// scale just the height of the cylinder
			double area = PIf * spheres[i].radius * spheres[i].radius;
			mgeom->Scale(0.002f, (float)area*1000, 0.002f);
		}
		m_SurfaceGroup->addChild(mgeom);
		glay->addChild(mgeom);
	}
}

void vtIcoGlobe::BuildSphericalLines(GlobeLayer *glay, float fSize)
{
	vtFeatureSet *feat = glay->m_pSet;
	vtFeatureSetLineString *pSetLS = dynamic_cast<vtFeatureSetLineString*>(feat);
	if (!pSetLS)
		return;

	int i, size;
	size = feat->NumEntities();

	vtGeode *geode = new vtGeode;
	geode->setName("spherical lines");
	geode->SetMaterials(m_coremats);
	m_SurfaceGroup->addChild(geode);
	glay->addChild(geode);

	vtGeomFactory mf(geode, osg::PrimitiveSet::LINE_STRIP, 0, 30000, m_yellow);
	for (i = 0; i < size; i++)
	{
		const DLine2 &line = pSetLS->GetPolyLine(i);
		AddSurfaceLineToMesh(&mf, line);
	}
}

void vtIcoGlobe::BuildSphericalPolygons(GlobeLayer *glay, float fSize)
{
	vtFeatureSet *feat = glay->m_pSet;
	vtFeatureSetPolygon *pSetPoly = dynamic_cast<vtFeatureSetPolygon*>(feat);
	if (!pSetPoly)
		return;

	int i, size;
	size = feat->NumEntities();

	vtGeode *geode = new vtGeode;
	geode->setName("spherical lines");
	geode->SetMaterials(m_coremats);
	m_SurfaceGroup->addChild(geode);
	glay->addChild(geode);

	vtGeomFactory mf(geode, osg::PrimitiveSet::LINE_STRIP, 0, 30000, m_yellow);
	for (i = 0; i < size; i++)
	{
		const DPolygon2 &poly = pSetPoly->GetPolygon(i);
		for (uint ring = 0; ring < poly.size(); ring++)
		{
			const DLine2 &line = poly[ring];
			AddSurfaceLineToMesh(&mf, line);
		}
	}
}

void vtIcoGlobe::BuildFlatFeatures(GlobeLayer *glay, float fSize)
{
	vtFeatureSet *feat = glay->m_pSet;
	if (feat->GetGeomType() == wkbPoint)
	{
		if (!m_cylinder)
		{
			// create cylinder mesh
			int res = 14;
			int verts = res * 2;
			m_cylinder = new vtMesh(osg::PrimitiveSet::TRIANGLE_STRIP, 0, verts);
			m_cylinder->CreateCylinder(1.0f, 1.0f, res, true, false, false);
		}

		int i, size;
		size = feat->NumEntities();

		for (i = 0; i < size; i++)
			BuildFlatPoint(glay, i, fSize);
	}
}

void vtIcoGlobe::BuildFlatPoint(GlobeLayer *glay, int i, float fSize)
{
	vtFeatureSet *feat = glay->m_pSet;
	vtFeatureSetPoint2D *pSetP2 = dynamic_cast<vtFeatureSetPoint2D*>(feat);
	if (!pSetP2)
		return;

	// create and place the geometries
	DPoint2 p;
	int face, subface;
	DPoint3 p_out;

	pSetP2->GetPoint(i, p);

	if (p.x == 0.0 && p.y == 0.0)	// ignore some
		return;

	FPoint3 offset;
	GeoToFacePoint(p, face, subface, p_out);

	int mface = GetMFace(face, subface);

	p_out -= m_mface[mface].local_origin;

	vtGeode *geode = new vtGeode;
	geode->SetMaterials(m_coremats);
	geode->AddMesh(m_cylinder, m_yellow);

	vtMovGeode *mgeom = new vtMovGeode(geode);
	mgeom->setName("GlobeShape");

//	mgeom->RotateLocal(FPoint3(1,0,0), -PID2f);
	DPoint3 v0 = m_verts[icosa_face_v[face][0]];
	DPoint3 v1 = m_verts[icosa_face_v[face][1]];
	DPoint3 v2 = m_verts[icosa_face_v[face][2]];
	DPoint3 facenorm = (v0 + v1 + v2).Normalize();

	mgeom->PointTowards(facenorm);
	mgeom->RotateLocal(FPoint3(1,0,0), -PID2f);

	mgeom->SetTrans(p_out);

	// scale just the radius of the cylinder
	mgeom->Scale(fSize, 0.001f, fSize);

	m_mface[mface].surfgroup->addChild(mgeom);
	glay->addChild(mgeom);
}

void vtIcoGlobe::AddTerrainRectangles(vtTerrainScene *pTerrainScene)
{
	FPoint3 p;

	m_pRectangles = new vtGeode;
	m_pRectangles->setName("terrain extents");
	m_pRectangles->SetMaterials(m_coremats);
	m_SurfaceGroup->addChild(m_pRectangles);

	vtGeomFactory mf(m_pRectangles, osg::PrimitiveSet::LINE_STRIP, 0, 30000, m_red);
	for (uint a = 0; a < pTerrainScene->NumTerrains(); a++)
	{
		vtTerrain *pTerr = pTerrainScene->GetTerrain(a);
		// skip if undefined
		if (pTerr->m_Corners_geo.GetSize() == 0)
			continue;

		int numvtx = 4;

		int i, j;
		DPoint2 p1, p2;
		for (i = 0; i < 4; i++)
		{
			j = (i+1) % 4;
			p1 = pTerr->m_Corners_geo[i];
			p2 = pTerr->m_Corners_geo[j];
			AddSurfaceLineToMesh(&mf, p1, p2);
		}
	}
}

double vtIcoGlobe::AddSurfaceLineToMesh(vtGeomFactory *pMF, const DPoint2 &g1, const DPoint2 &g2)
{
	// first determine how many points we should use for a smooth arc
	DPoint3 p1, p2;
	geo_to_xyz(1.0, g1, p1);
	geo_to_xyz(1.0, g2, p2);
	double dot = p1.Dot(p2);
	double angle = acos(dot);
	int points = (int) (angle * 3000);
	if (points < 2)
		points = 2;

	pMF->PrimStart();
	if (points == 2)
	{
		// simple case
		pMF->AddVertex(p1*1.0002);
		pMF->AddVertex(p2*1.0002);
	}
	else
	{
		// calculate the axis of rotation
		DPoint3 cross = p1.Cross(p2);
		cross.Normalize();
		double angle_spacing = angle / (points-1);
		DMatrix4 rot4;
		rot4.AxisAngle(cross, angle_spacing);
		DMatrix3 rot3;
		rot3.SetByMatrix4(rot4);

		// curved arc on great-circle path
		for (int i = 0; i < points; i++)
		{
			FPoint3 fp = p1 * 1.0002;
			pMF->AddVertex(fp);
			rot3.Transform(p1, p2);
			p1 = p2;
		}
	}
	pMF->PrimEnd();
	return angle;
}

double vtIcoGlobe::AddSurfaceLineToMesh(vtGeomFactory *pMF, const DLine2 &line)
{
	DPoint2 g1, g2;
	DPoint3 p1, p2;
	double scale = 1.0002;
	int length = 0;
	DMatrix3 rot3;

	pMF->PrimStart();
	int i, j, size = line.GetSize();

	for (i = 0; i < size-1; i++)
	{
		g1 = line[i];
		g2 = line[i+1];

		// for each pair of points, determine how many more points are needed
		//  for a smooth arc
		geo_to_xyz(1.0, g1, p1);
		geo_to_xyz(1.0, g2, p2);
		double angle = acos(p1.Dot(p2));
		int segments = (int) (angle * 2000);
		if (segments < 1)
			segments = 1;

		if (segments > 1)
		{
			// calculate the axis of rotation
			DPoint3 cross = p1.Cross(p2);
			cross.Normalize();
			rot3.AxisAngle(cross, angle / segments);
		}

		// curved arc on great-circle path
		for (j = 0; j < segments; j++)
		{
			FPoint3 fp = p1 * 1.0002;
			pMF->AddVertex(fp);
			length++;

			if (j < segments-1)
			{
				rot3.Transform(p1, p2);
				p1 = p2;
			}
		}
	}

	// last vertex
	if (size > 1)
	{
		g2 = line[size-1];
		geo_to_xyz(1.0, g2, p2);
		pMF->AddVertex(p2 * scale);
		length++;
	}

	pMF->PrimEnd();
	return 0.0;
}

///////////////////////////////////////////////////////////////////////
// Internal methods
//

// This array describes the configuration and topology of the subfaces in
// the flattened dymaxion map.
struct dymax_info
{
	int face;
	int subfaces;
	int parent_face;
	int parent_mface;
	int parentedge;
};

dymax_info dymax_subfaces[22] =
{
	{  0, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1, -1, -1, -1 }, // 0

	{  1, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1,  0, 0, 2 },	// 1
	{  4, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1,  0, 0, 0 },	// 2
	{  5, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1,  0, 0, 1 },	// 3

	{  7, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1,  1, 1, 1 },	// 4
	{  2, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1,  1, 1, 2 },	// 5

	{  6, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1,  7, 4, 2 },	// 6
	{  8,               1<<4 | 1<<3 | 1<<2 | 1<<1,  7, 4, 0 },	// 7
	{  9, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1,  2, 5, 1 },	// 8
	{  3, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1,  2, 5, 2 },	// 9

	{ 16, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1,  8, 7, 1 },	// 10
	{  8, 1<<6 | 1<<5,                              9, 8, 2 },	// 11
	{ 11, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1,  3, 9, 1 },	// 12

	{ 15,               1<<4 | 1<<3 | 1<<2,        16, 10, 2 },	// 13
	{ 10, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1, 11, 12, 2 },	// 14
	{ 12, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1, 11, 12, 0 },	// 15

	{ 17, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1, 10, 14, 1 },	// 16
	{ 18, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1, 12, 15, 1 },	// 17
	{ 13, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1, 12, 15, 2 },	// 18

	{ 19, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1, 18, 17, 0 },	// 19

	{ 15, 1<<6 | 1<<5 |                      1<<1, 19, 19, 0 },	// 20
	{ 14, 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1, 19, 19, 1 },	// 21
};

int vtIcoGlobe::GetMFace(int face, int subface)
{
	int submask = 1<<(subface+1);
	for (int i = 0; i < 22; i++)
	{
		if (dymax_subfaces[i].face == face &&
			(dymax_subfaces[i].subfaces & submask) != 0)
			return i;
	}
	return 0;
}

int GetMaterialForFace(int face, bool &which)
{
	for (int i = 0; i < 10; i++)
	{
		if (icosa_face_pairs[i][0] == face)
		{
			which = false;
			return i;
		}
		if (icosa_face_pairs[i][1] == face)
		{
			which = true;
			return i;
		}
	}
	return -1;
}

void vtIcoGlobe::EstimateTesselation(int iTriangleCount)
{
	int per_face = iTriangleCount / 20;
	if (m_style == GEODESIC || m_style == INDEPENDENT_GEODESIC)
	{
		// Frequency for a traditional geodesic tiling gives (frequency ^ 2)
		// triangles.  Find what frequency most closely matches the desired
		// triangle count.
		double exact = sqrt((double)per_face);
		int iLess = (int) floor(exact);
		int iMore = (int) ceil(exact);
		if ((iMore*iMore - per_face) < (per_face - iLess*iLess))
			m_freq = iMore;
		else
			m_freq = iLess;
	}
	else if (m_style == RIGHT_TRIANGLE || m_style == DYMAX_UNFOLD)
	{
		// Recursive right-triangle subdivision gives (2 * 3 ^ (depth+1))
		// triangles.  Find what depth most closely matches the desired
		// triangle count.
		int a, b = 6;
		for (a = 1; a < 10; a++)
		{
			b *= 3;
			if (b > per_face) break;
		}
		if ((b - per_face) < (per_face - b/3))
			m_depth = a;
		else
			m_depth = a-1;
	}
}

//
// argument "f" goes from 0 (icosahedron) to 1 (sphere)
//
void vtIcoGlobe::set_face_verts1(vtMesh *mesh, int face, float f)
{
	int i, j;

	DPoint3 v0 = m_verts[icosa_face_v[face][0]];
	DPoint3 v1 = m_verts[icosa_face_v[face][1]];
	DPoint3 v2 = m_verts[icosa_face_v[face][2]];
	DPoint3 vec0 = (v1 - v0)/m_freq, vec1 = (v2 - v0)/m_freq;
	DPoint3 p0, p1, p2, norm;
	FPoint3 fp0;
	double len, mag = 1.0;

	int idx = 0;
	for (j = 0; j <= m_freq; j++)
	{
		for (i = 0; i <= (m_freq-j); i++)
		{
			p0 = v0 + (vec0 * i) + (vec1 * j);

			// do interpolation between icosa face (f=0) and sphere (f=1)
			len = p0.Length();
			p0 = p0 / len * (f * mag + (1 - f) * len);

			// convert doubles -> floats
			fp0 = p0;

			mesh->SetVtxPos(idx, fp0);
			// for a spheroid (f == 1), this is correct:
			mesh->SetVtxNormal(idx, fp0);

			idx += 1;
		}
	}
	if (f != 1.0f)
	{
		// recalculate vertex normals from the current vertex locations
		idx = 0;
		DPoint3 d0, d1;
		for (j = 0; j <= m_freq; j++)
		{
			for (i = 0; i <= (m_freq-j); i++)
			{
				p0 = v0 + (vec0 * i) + (vec1 * j);
				p1 = v0 + (vec0 * (i+1)) + (vec1 * j);
				p2 = v0 + (vec0 * i) + (vec1 * (j+1));

				len = p0.Length();
				p0 = p0 / len * (f * mag + (1 - f) * len);
				len = p1.Length();
				p1 = p1 / len * (f * mag + (1 - f) * len);
				len = p2.Length();
				p2 = p2 / len * (f * mag + (1 - f) * len);

				d0 = (p1 - p0);
				d1 = (p2 - p0);
				d0.Normalize();
				d1.Normalize();

				norm = d0.Cross(d1);

				fp0 = norm;	// convert doubles to floats
				mesh->SetVtxNormal(idx, fp0);

				idx += 1;
			}
		}
	}
}

//
// argument "f" goes from 0 (icosahedron) to 1 (sphere)
//
void vtIcoGlobe::set_face_verts2(vtMesh *mesh, int mface, float f)
{
	refresh_face_positions(m_mesh[mface], mface, f);

	if (f == 1.0f)
	{
		// sphere normals pointing straight outwards
		for (uint i = 0; i < m_rtv[mface].GetSize(); i++)
			mesh->SetVtxNormal(i, m_rtv[mface][i].p);
	}
	else
		m_mesh[mface]->SetNormalsFromPrimitives();
}

// \param second True for the second triangle in a face pair; affects UV coords.
void vtIcoGlobe::add_face1(vtMesh *mesh, int face, bool second)
{
	int i, j;

	DPoint3 v0 = m_verts[icosa_face_v[face][0]];
	DPoint3 v1 = m_verts[icosa_face_v[face][1]];
	DPoint3 v2 = m_verts[icosa_face_v[face][2]];
	DPoint3 vec0 = (v1 - v0)/m_freq, vec1 = (v2 - v0)/m_freq;
	DPoint3 p0;
	FPoint3 vp0;

	// two passes
	// first pass: create the vertices
	int idx = 0;
	int vtx_base = 0;
	for (j = 0; j <= m_freq; j++)
	{
		for (i = 0; i <= (m_freq-j); i++)
		{
			p0 = v0 + (vec0 * i) + (vec1 * j);

			// convert doubles -> floats
			vp0 = p0;

			mesh->SetVtxPos(idx, vp0);
			// for a spheroid (f == 1), this is correct:
			mesh->SetVtxNormal(idx, vp0);

			FPoint2 coord;
			if (second)
			{
				coord.x = 1.0f - (float)i / m_freq;
				coord.y = 1.0f - (float)j / m_freq;
			}
			else
			{
				coord.x = (float)i / m_freq;
				coord.y = (float)j / m_freq;
			}
			mesh->SetVtxTexCoord(idx, coord);

			idx += 1;
		}
	}
	// Next pass: the strips
	int row_start = 0;
	unsigned short *indices = new unsigned short[m_freq * 2 + 1];

	// Here we create the tristrips moving right to left, because that
	//  gives us the desired right-handed order of the strip vertices.
	for (j = 0; j < m_freq; j++)
	{
		int count = 0;
		int row_len = (m_freq-j) + 1;

		// Number of vertices in this strip: row_len * 2 - 1;

		indices[count++] = vtx_base + row_start + row_len - 1;
		for (i = row_len-2; i >= 0; i--)
		{
			indices[count++] = vtx_base + row_start + i + row_len;
			indices[count++] = vtx_base + row_start + i;
		}
		mesh->AddStrip(count, indices);

		row_start += row_len;
	}
	delete [] indices;
}

void vtIcoGlobe::add_face2(vtMesh *mesh, int face, int mface, int subfaces, bool second)
{
	int i;
	IcoVert v0, v1, v2, e0, e1, e2, center;

	v0.p = m_verts[icosa_face_v[face][0]];
	v1.p  = m_verts[icosa_face_v[face][1]];
	v2.p  = m_verts[icosa_face_v[face][2]];

	// find edges and center
	e0.p = (v0.p + v1.p) / 2;
	e1.p = (v1.p + v2.p) / 2;
	e2.p = (v2.p + v0.p) / 2;
	center.p = (v0.p + v1.p + v2.p) / 3;

	FPoint2 center_uv;
	if (second == false)
	{
		v0.uv.Set(0, 0);
		v1.uv.Set(1, 0);
		v2.uv.Set(0, 1);
	}
	else
	{
		v0.uv.Set(1, 1);
		v1.uv.Set(0, 1);
		v2.uv.Set(1, 0);
	}
	// find edge and center uv
	e0.uv = (v0.uv + v1.uv) / 2;
	e1.uv = (v1.uv + v2.uv) / 2;
	e2.uv = (v2.uv + v0.uv) / 2;
	center.uv = (v0.uv + v1.uv + v2.uv) / 3;

	m_rtv[mface].Append(v0);
	m_rtv[mface].Append(v1);
	m_rtv[mface].Append(v2);
	m_rtv[mface].Append(e0);	// 3
	m_rtv[mface].Append(e1);	// 4
	m_rtv[mface].Append(e2);	// 5
	m_rtv[mface].Append(center); // 6
	m_vert = 7;

	for (i = 0; i < 7; i++)
		mesh->AddVertexNUV(m_rtv[mface][i].p, m_rtv[mface][i].p, m_rtv[mface][i].uv);

	// iterate the 6 subfaces
	if (subfaces & (1<<1))
		add_subface(mesh, mface, 0, 3, 6, false, m_depth);
	if (subfaces & (1<<2))
		add_subface(mesh, mface, 1, 3, 6, true, m_depth);

	if (subfaces & (1<<3))
		add_subface(mesh, mface, 1, 4, 6, false, m_depth);
	if (subfaces & (1<<4))
		add_subface(mesh, mface, 2, 4, 6, true, m_depth);

	if (subfaces & (1<<5))
		add_subface(mesh, mface, 2, 5, 6, false, m_depth);
	if (subfaces & (1<<6))
		add_subface(mesh, mface, 0, 5, 6, true, m_depth);

	// now, deal with curvature
	refresh_face_positions(mesh, mface, 1);
}

void vtIcoGlobe::refresh_face_positions(vtMesh *mesh, int mface, float f)
{
	int i;
	double len;
	FPoint3 fp;
	int total = m_rtv[mface].GetSize();
	for (i = 0; i < total; i++)
	{
		// do interpolation between icosa face and sphere
		len = m_rtv[mface][i].p.Length();
		fp = m_rtv[mface][i].p / len * (f + (1 - f) * len);

		if (m_style == DYMAX_UNFOLD)
		{
			fp -= m_mface[mface].local_origin;
		}

		mesh->SetVtxPos(i, fp);
	}
}

void vtIcoGlobe::add_subface(vtMesh *mesh, int face, int v0, int v1, int v2,
						   bool flip, int depth)
{
	if (depth > 0)
	{
		IcoVert p3, p4;

		p3.p = m_rtv[face][v1].p - (m_rtv[face][v1].p - m_rtv[face][v0].p) / 3;
		p4.p = (m_rtv[face][v0].p + m_rtv[face][v2].p) / 2;

		p3.uv = m_rtv[face][v1].uv - (m_rtv[face][v1].uv - m_rtv[face][v0].uv) / 3;
		p4.uv = (m_rtv[face][v0].uv + m_rtv[face][v2].uv) / 2;

		int v3 = m_vert++;
		int v4 = m_vert++;

		m_rtv[face].Append(p3);
		m_rtv[face].Append(p4);

		mesh->AddVertexNUV(m_rtv[face][v3].p, m_rtv[face][v3].p, m_rtv[face][v3].uv);
		mesh->AddVertexNUV(m_rtv[face][v4].p, m_rtv[face][v4].p, m_rtv[face][v4].uv);

		add_subface(mesh, face, v0, v4, v3, !flip, depth - 1);
		add_subface(mesh, face, v2, v4, v3,  flip, depth - 1);
		add_subface(mesh, face, v2, v1, v3, !flip, depth - 1);
		return;
	}

	unsigned short Indices[3];
	if (flip)
	{
		Indices[0] = v2;
		Indices[1] = v1;
		Indices[2] = v0;
	}
	else
	{
		Indices[0] = v0;
		Indices[1] = v1;
		Indices[2] = v2;
	}
	mesh->AddStrip(3, Indices);
}


void vtIcoGlobe::FindLocalOrigin(int mface)
{
	int parent_face = dymax_subfaces[mface].parent_face;
	int edge = dymax_subfaces[mface].parentedge;

	DPoint3 v0 = m_verts[icosa_face_v[parent_face][0]];
	DPoint3 v1 = m_verts[icosa_face_v[parent_face][1]];
	DPoint3 v2 = m_verts[icosa_face_v[parent_face][2]];

	// find the center of each edge
	DPoint3 ec[3], axis[3];
	ec[0] = (v0 + v1) / 2;
	ec[1] = (v1 + v2) / 2;
	ec[2] = (v2 + v0) / 2;
	axis[0] = v1 - v0;
	axis[1] = v2 - v1;
	axis[2] = v0 - v2;

	m_mface[mface].local_origin = ec[edge];
	m_mface[mface].axis = axis[edge];
	m_mface[mface].axis.Normalize();
}

void vtIcoGlobe::SetMeshConnect(int mface)
{
	int parent_mface = dymax_subfaces[mface].parent_mface;

	vtTransform *xform = m_mface[mface].xform;
	vtMesh *mesh = m_mesh[mface];

	// attach heirarchy
	m_mface[parent_mface].xform->addChild(xform);

	// translate vertices to set origin of this mface
	int i;
	int verts = mesh->NumVertices();
	FPoint3 pos;

	FPoint3 edge_center = m_mface[mface].local_origin;

	for (i = 0; i < verts; i++)
	{
		pos = mesh->GetVtxPos(i);
		pos -= edge_center;
		mesh->SetVtxPos(i, pos);
	}
	if (parent_mface == 0)
		xform->Translate(edge_center);
	else
		xform->Translate(edge_center - m_mface[parent_mface].local_origin);
}

void vtIcoGlobe::CreateUnfoldableDymax()
{
	int i;
	for (i = 0; i < 22; i++)
	{
		m_mface[i].xform = new vtTransform;
		m_mface[i].surfgroup = new vtGroup;
		m_mface[i].surfgroup->SetEnabled(false);
		m_mface[i].geode = new vtGeode;
		m_mface[i].xform->addChild(m_mface[i].geode);
		m_mface[i].xform->addChild(m_mface[i].surfgroup);

		vtString str;
		str.Format("IcoFace %d", i);
		m_mface[i].xform->setName(str);

		int face = dymax_subfaces[i].face;
		int subfaces = dymax_subfaces[i].subfaces;

		bool which;
		int mat = GetMaterialForFace(face, which);

		add_face2(m_mesh[i], face, i, subfaces, which);

		m_mface[i].geode->SetMaterials(m_earthmats);
		m_mface[i].geode->AddMesh(m_mesh[i], m_globe_mat[mat]);
	}
	m_top->addChild(m_mface[0].xform);

	m_mface[0].local_origin.Set(0,0,0);
	for (i = 1; i < 22; i++)
		FindLocalOrigin(i);
	for (i = 1; i < 22; i++)
		SetMeshConnect(i);

	// Determine necessary rotation to orient flat map toward viewer.
	FQuat qface;
	DPoint3 v0 = m_verts[icosa_face_v[0][0]];
	DPoint3 v1 = m_verts[icosa_face_v[0][1]];
	DPoint3 v2 = m_verts[icosa_face_v[0][2]];

	// Create a rotation to turn the globe so that a specific edge
	//  is pointed down -X for proper map orientation
	DPoint3 edge = v2 - v0;
	edge.Normalize();

	// compose face norm and face quaternion
	DPoint3 fnorm = (v0 + v1 + v2).Normalize();
	qface.SetFromVectors(edge, fnorm);

	// desired vector points down -X
	FQuat qdesired;
	qdesired.SetFromVectors(FPoint3(-1,0,0),FPoint3(0,0,1));

	// determine rotational difference
	m_diff = qface.Inverse() * qdesired;

#if 0
	// scaffolding mesh for debugging
	vtMesh *sm = new vtMesh(GL_LINES, VT_Colors, 12);
	sm->AddVertex(v0*1.0001f);
	sm->AddVertex(v1*1.0001f);
	sm->AddVertex(v2*1.0001f);
	sm->AddVertex(v0*1.0001f+fnorm);
	sm->SetVtxColor(0, RGBf(1,0,0));
	sm->SetVtxColor(1, RGBf(0,1,0));
	sm->SetVtxColor(2, RGBf(0,0,1));
	sm->SetVtxColor(3, RGBf(1,1,0));
	sm->AddLine(0,1);
	sm->AddLine(0,2);
	sm->AddLine(0,3);
	m_geom[0]->AddMesh(sm, m_red);
	sm->Release();
#endif

	// Show axis of rotation (north and south poles)
	vtMaterialArray *pMats = new vtMaterialArray;
	int green = pMats->AddRGBMaterial(RGBf(0,1,0), false, false);
	m_pAxisGeom = new vtGeode;
	m_pAxisGeom->setName("AxisGeom");
	m_pAxisGeom->SetMaterials(pMats);
	m_pAxisGeom->SetEnabled(false);

	vtMesh *pMesh = new vtMesh(osg::PrimitiveSet::LINES, 0, 6);
	pMesh->AddVertex(FPoint3(0,2,0));
	pMesh->AddVertex(FPoint3(0,-2,0));
	pMesh->AddLine(0,1);
	m_pAxisGeom->AddMesh(pMesh, green);
	m_top->addChild(m_pAxisGeom);

#if 0
	axis = WireAxis(RGBf(1,1,1), 1.1f);
	m_top->addChild(axis);
#endif
}

void vtIcoGlobe::CreateNormalSphere()
{
	// Create the meshes
	int pair;
	for (pair = 0; pair < 10; pair++)
	{
		int f1 = icosa_face_pairs[pair][0];
		int f2 = icosa_face_pairs[pair][1];

		if (m_style == GEODESIC)
		{
			add_face1(m_mesh[f1], f1, false);
			add_face1(m_mesh[f2], f2, true);
		}
		else if (m_style == RIGHT_TRIANGLE)
		{
			int all = 1<<6 | 1<<5 | 1<<4 | 1<<3 | 1<<2 | 1<<1;
			add_face2(m_mesh[f1], f1, f1, all, false);
			add_face2(m_mesh[f2], f2, f2, all, true);
		}
	}

	// Create a geom to contain the meshes
	m_GlobeGeom = new vtGeode;
	m_GlobeGeom->setName("GlobeGeom");
	m_GlobeGeom->SetMaterials(m_earthmats);
	m_top->addChild(m_GlobeGeom);

	for (pair = 0; pair < 10; pair++)
	{
		int f1 = icosa_face_pairs[pair][0];
		int f2 = icosa_face_pairs[pair][1];
		m_GlobeGeom->AddMesh(m_mesh[f1], m_globe_mat[pair]);
		m_GlobeGeom->AddMesh(m_mesh[f2], m_globe_mat[pair]);
	}
}

void vtIcoGlobe::create_independent_face(int face, bool second)
{
	int i, j;

	DPoint3 v0 = m_verts[icosa_face_v[face][0]];
	DPoint3 v1 = m_verts[icosa_face_v[face][1]];
	DPoint3 v2 = m_verts[icosa_face_v[face][2]];
	DPoint3 vec0 = (v1 - v0)/m_freq, vec1 = (v2 - v0)/m_freq;
	DPoint3 p0;
	FPoint3 vp0;

	int verts_per_face = (m_freq+1) * (m_freq+2) / 2;
	FPoint3 *verts = new FPoint3[verts_per_face];

	// first pass: determine vertex locations
	int idx = 0;
	int vtx_base = 0;
	for (j = 0; j <= m_freq; j++)
	{
		for (i = 0; i <= (m_freq-j); i++)
		{
			p0 = v0 + (vec0 * i) + (vec1 * j);

			// make spherical
			p0.Normalize();

			verts[idx] = p0;
			idx += 1;
		}
	}

	FPoint2 uv[2][3];
	uv[0][0].Set(0, 0);
	uv[0][1].Set(1, 0);
	uv[0][2].Set(0, 1);
	uv[1][0].Set(1, 1);
	uv[1][1].Set(0, 1);
	uv[1][2].Set(1, 0);

	// second pass: set the vertices of the meshes
	int row_start = 0;
	idx = 0;
	int mesh_num = face * m_freq * m_freq;
	vtMesh *mesh;
	for (j = 0; j < m_freq; j++)
	{
		int row_len = (m_freq-j) + 1;
		for (i = 0; i < (m_freq-j); i++)
		{
			mesh = m_mesh[mesh_num];

			mesh->SetVtxPos(0, verts[row_start + i]);
			mesh->SetVtxPos(1, verts[row_start + i + 1]);
			mesh->SetVtxPos(2, verts[row_start + i + row_len]);

			mesh->SetVtxNormal(0, verts[row_start + i]);
			mesh->SetVtxNormal(1, verts[row_start + i + 1]);
			mesh->SetVtxNormal(2, verts[row_start + i + row_len]);

			for (int k = 0; k < 3; k++)
				mesh->SetVtxTexCoord(k, second ? uv[1][k] : uv[0][k]);

			mesh->AddTri(0, 1, 2);

			mesh_num++;

			if (i < (m_freq-j-1))
			{
				mesh = m_mesh[mesh_num];

				mesh->SetVtxPos(0, verts[row_start + i + row_len + 1]);
				mesh->SetVtxPos(1, verts[row_start + i + row_len]);
				mesh->SetVtxPos(2, verts[row_start + i + 1]);

				mesh->SetVtxNormal(0, verts[row_start + i + row_len + 1]);
				mesh->SetVtxNormal(1, verts[row_start + i + row_len]);
				mesh->SetVtxNormal(2, verts[row_start + i + 1]);

				for (int k = 0; k < 3; k++)
					mesh->SetVtxTexCoord(k, second ? uv[0][k] : uv[1][k]);

				mesh->AddTri(0, 1, 2);

				mesh_num++;
			}
		}
		row_start += row_len;
	}
	delete [] verts;
}

void vtIcoGlobe::CreateIndependentGeodesicSphere()
{
	// Create a geom to contain the meshes
	m_GlobeGeom = new vtGeode;
	m_GlobeGeom->setName("GlobeGeom");
	m_GlobeGeom->SetMaterials(m_earthmats);
	m_top->addChild(m_GlobeGeom);

	// fill in the vertices and triangles of the meshes
	for (int pair = 0; pair < 10; pair++)
	{
		int f1 = icosa_face_pairs[pair][0];
		int f2 = icosa_face_pairs[pair][1];

		create_independent_face(f1, false);
		create_independent_face(f2, true);

		// There are freq*freq materials for every *pair* of faces, each which
		//  applies to two meshes.
		add_face_independent_meshes(pair, f1, false);
		add_face_independent_meshes(pair, f2, true);
	}
}

void vtIcoGlobe::add_face_independent_meshes(int pair, int face, bool second)
{
	int mat_base = pair * m_freq * m_freq;
	int mesh_base = face * m_freq * m_freq;

	int mesh = 0;
	for (int j = 0; j < m_freq; j++)
	{
		for (int i = 0; i < (m_freq-j); i++)
		{
			int mat = j * m_freq + i;
			if (second)
				mat = (m_freq-1-j) * m_freq + (m_freq-1-i);

			m_GlobeGeom->AddMesh(m_mesh[mesh_base + mesh], m_globe_mat[mat_base + mat]);
			mesh++;

			if (i < (m_freq-j-1))
			{
				m_GlobeGeom->AddMesh(m_mesh[mesh_base + mesh], m_globe_mat[mat_base + mat]);
				mesh++;
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////
// GlobeLayer class
//

void GlobeLayer::DestructGeometry()
{
	removeChildren(0, getNumChildren());
}


///////////////////////////////////////////////////////////////////////
// Sphere Helpers
//

void geo_to_xyz(double radius, const DPoint2 &geo, FPoint3 &p)
{
	DPoint3 dp;
	geo_to_xyz(radius, geo, dp);
	p = dp;
}

void geo_to_xyz(double radius, const DPoint2 &geo, DPoint3 &p)
{
	// Convert spherical polar coordinates to cartesian coordinates
	// The angles are given in degrees
	double theta = geo.x / 180.0 * PId;
	double phi = geo.y / 180.0 * PId;

	phi += PID2d;
	theta = PI2d - theta;

	p.x = sin(phi) * cos(theta) * radius;
	p.z = sin(phi) * sin(theta) * radius;
	p.y = -cos(phi) * radius;
}

double radians(double degrees)
{
	return degrees / 180.0 * PId;
}

void xyz_to_geo(double radius, const FPoint3 &p, DPoint3 &geo)
{
	double x = p.x / radius;
	double y = p.z / radius;
	double z = -p.y / radius;

	double a=0, lat, lng=0;
	lat = acos(z);
	if (x>0.0 && y>0.0) a = radians(0.0);
	if (x<0.0 && y>0.0) a = radians(180.0);
	if (x<0.0 && y<0.0) a = radians(180.0);
	if (x>0.0 && y<0.0) a = radians(360.0);
	if (x==0.0 && y>0.0) lng = radians(90.0);
	if (x==0.0 && y<0.0) lng = radians(270.0);
	if (x>0.0 && y==0.0) lng = radians(0.0);
	if (x<0.0 && y==0.0) lng = radians(180.0);
	if (x!=0.0 && y!=0.0) lng = atan(y/x) + a;

	lng = PI2d - lng;
	lat = lat - PID2d;

	// convert angles to degrees
	geo.x = lng * 180.0 / PId;
	geo.y = lat * 180.0 / PId;

	// keep in expected range
	if (geo.x > 180.0) geo.x -= 360.0;

	// we don't know elevation (yet)
	geo.z = 0.0f;
}


///////////////////////////////////////////////////////////////////////
// Helper functions

//
// Create a moveable wireframe axis geometry.
//
vtTransform *WireAxis(RGBf color, float len)
{
	vtGeode *geode = new vtGeode;
	geode->setName("axis");

	vtMaterialArray *pMats = new vtMaterialArray;
	int index = pMats->AddRGBMaterial(color, false, false);
	geode->SetMaterials(pMats);

	vtMesh *mesh = new vtMesh(osg::PrimitiveSet::LINES, 0, 6);
	mesh->AddVertex(FPoint3(-len,0,0));
	mesh->AddVertex(FPoint3( len,0,0));
	mesh->AddVertex(FPoint3(0,-len,0));
	mesh->AddVertex(FPoint3(0, len,0));
	mesh->AddVertex(FPoint3(0,0,-len));
	mesh->AddVertex(FPoint3(0,0, len));
	mesh->AddLine(0,1);
	mesh->AddLine(2,3);
	mesh->AddLine(4,5);
	geode->AddMesh(mesh, index);
	vtTransform *trans = new vtTransform;
	trans->addChild(geode);
	return trans;
}

vtMovGeode *CreateSimpleEarth(const vtString &strDataPath)
{
	// create simple texture-mapped sphere
	vtMesh *mesh = new vtMesh(osg::PrimitiveSet::QUADS, VT_Normals | VT_TexCoords, 20*20*2);
	int res = 20;
	FPoint3 size(1.0f, 1.0f, 1.0f);
	mesh->CreateEllipsoid(FPoint3(0,0,0), size, res);

	// fix up the texture coordinates
	int numvtx = mesh->NumVertices();
	for (int i = 0; i < numvtx; i++)
	{
		FPoint2 coord;
		coord.y = 1.0f - ((float) i / (res * res));
		coord.x = 1.0f - ((float (i%res)) / res);
		mesh->SetVtxTexCoord(i, coord);
	}

	vtGeode *geode = new vtGeode;
	vtMovGeode *mgeom = new vtMovGeode(geode);
	mgeom->setName("GlobeGeom");

	vtMaterialArray *pMats = new vtMaterialArray;
	bool bCulling = false;
	bool bLighting = false;
	bool bTransp = false;

	osg::Image *image = LoadOsgImage(strDataPath + "WholeEarth/earth2k_free.jpg");
	pMats->AddTextureMaterial(image, bCulling, bLighting, bTransp);
	geode->SetMaterials(pMats);

	geode->AddMesh(mesh, 0);

	return mgeom;
}

