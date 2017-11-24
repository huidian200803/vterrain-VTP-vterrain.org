//
// Globe.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef GLOBEH
#define GLOBEH

#include "vtdata/Icosa.h"
#include "vtdata/Features.h"
#include "vtdata/FilePath.h"
#include "TimeEngines.h"

class vtTerrainScene;
class vtGeomFactory;

/**
 A GIS-style feature layer, on the globe.
 An collection of these is maintained by vtIcoGlobe.
 */
class GlobeLayer : public vtGroup
{
public:
	void DestructGeometry();

	vtFeatureSet *m_pSet;
};

typedef vtArray<GlobeLayer*> GlobeLayerArray;

/** \addtogroup terrain */
/*@{*/

/**
 * vtIcoGlobe is an icosahedral globe.  To use it:
 *  - call Create() with the desired parameters to construct the object
 *  - call GetTop() to get the top node in the globe's scene graph
 *  - add that node to your scene graph
 *
 * The globe has unit radius.
 */
class vtIcoGlobe : public DymaxIcosa, public vtTimeTarget
{
public:
	enum Style
	{
		GEODESIC, INDEPENDENT_GEODESIC, RIGHT_TRIANGLE, DYMAX_UNFOLD
	};

	// construction
	vtIcoGlobe();
	virtual ~vtIcoGlobe();

	void Create(int iTriangleCount, const vtString &strImagePrefix,
		Style style = GEODESIC);
	void Create(int iTriangleCount, vtImage **images, Style style);
	vtTransform *GetTop() { return m_top; }
	vtGroup *GetSurface() { return m_SurfaceGroup; }

	// control globe behavior
	void SetInflation(float f);
	void SetUnfolding(float f);
	void SetCulling(bool bCull);
	void SetLighting(bool bLight);
	void SetTime(const vtTime &time);
	void ShowAxis(bool bShow);
	void SetSeasonalTilt(bool bTilt) { m_bTilt = bTilt; }
	bool GetSeasonalTilt() { return m_bTilt; }
	FQuat GetRotation() { return m_Rotation; }

	// surface features
	GlobeLayerArray &GetGlobeLayers() { return m_GlobeLayers; }
	int AddGlobeFeatures(const char *fname, float fSize);
	void RemoveLayer(GlobeLayer *glay);

	void AddTerrainRectangles(vtTerrainScene *pTerrainScene);
	double AddSurfaceLineToMesh(vtGeomFactory *pMF, const DPoint2 &g1, const DPoint2 &g2);
	double AddSurfaceLineToMesh(vtGeomFactory *pMF, const DLine2 &line);

	void SetEarthMaterials(vtMaterialArray *mats);
	vtMaterialArray *GetEarthMaterials();

protected:
	struct IcoVert
	{
		DPoint3 p;
		FPoint2 uv;
	};

	// Each movable face, composed of 1 to 6 subfaces of an original
	//  icosahedron face.
	struct MFace
	{
		vtTransform *xform;
		vtGeode *geode;
		vtGroup *surfgroup;
		FPoint3 local_origin;
		FPoint3 axis;		// axis of rotation ("hinge") for each face
	};

	void CreateMeshMat(int iTriangleCount);
	vtMaterialArray *CreateMaterialsFromFiles(const vtString &strImagePrefix);
	vtMaterialArray *CreateMaterialsFromImages(vtImage **images);
	void CreateCoreMaterials();
	void CreateNodes();

	int GetMFace(int face, int subface);
	void BuildSphericalFeatures(GlobeLayer *glay, float fSize);
	void BuildSphericalPoints(GlobeLayer *glay, float fSize);
	void BuildSphericalLines(GlobeLayer *glay, float fSize);
	void BuildSphericalPolygons(GlobeLayer *glay, float fSize);
	void BuildFlatFeatures(GlobeLayer *glay, float fSize);
	void BuildFlatPoint(GlobeLayer *glay, int i, float fSize);

	void FindLocalOrigin(int mface);
	void SetMeshConnect(int mface);

	void EstimateTesselation(int iTriangleCount);
	void CreateUnfoldableDymax();
	void CreateNormalSphere();
	void CreateIndependentGeodesicSphere();

	// these methods create a mesh for each face composed of strips
	void add_face1(vtMesh *mesh, int face, bool second);
	void set_face_verts1(vtMesh *geom, int face, float f);

	// these methods use a right-triangle recursion to create faces
	void add_face2(vtMesh *mesh, int face, int mface, int subfaces, bool second);
	void set_face_verts2(vtMesh *geom, int face, float f);
	void add_subface(vtMesh *mesh, int face, int v0, int v1, int v2,
								   bool flip, int depth);
	void refresh_face_positions(vtMesh *mesh, int mface, float f);

	// these methods create several meshes per face
	void create_independent_face(int face, bool second);
	void add_face_independent_meshes(int pair, int face, bool second);

	vtIcoGlobe::Style m_style;

	// Common to all globe styles
	vtTransform	*m_top;
	vtGroup		*m_SurfaceGroup;
	vtGeode		*m_pAxisGeom;
	vtMaterialArray	*m_coremats;
	vtMaterialArray	*m_earthmats;
	std::vector<int> m_globe_mat;
	int			m_red;
	int			m_yellow;
	int			m_white;
	std::vector<vtMesh*> m_mesh;
	int			m_meshes;	// 20 or 22 for normal or unfoldable globe
	bool		m_bUnfolded;
	bool		m_bTilt;
	FQuat		m_Rotation;

	vtMesh		*m_cylinder;

	// TEMP- replace with feature layer soon
	vtGeode *m_pRectangles;

	// for GEODESIC
	vtGeode	*m_GlobeGeom;
	int		m_freq;		// tesselation frequency
	int		m_subfreq;	// tesselation subfrequency

	// for RIGHT_TRIANGLE
	int		m_vert;
	vtArray<IcoVert>	m_rtv[22];	// right-triangle vertices
	int		m_depth;	// tesselation depth

	// for DYMAX_UNFOLD
	MFace	m_mface[22];
	FQuat	m_diff;

	// Features (point, line, polygon..) draped on the globe
	GlobeLayerArray	m_GlobeLayers;
};

vtMovGeode *CreateSimpleEarth(const vtString &strDataPath);

void geo_to_xyz(double radius, const DPoint2 &geo, FPoint3 &p);
void geo_to_xyz(double radius, const DPoint2 &geo, DPoint3 &p);
void xyz_to_geo(double radius, const FPoint3 &p, DPoint3 &geo);

/*@}*/	// Group terrain

#endif	// GLOBEH

