//
// NodeOSG.h
//
// Extend the behavior of OSG scene graph nodes.
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_NODEH
#define VTOSG_NODEH

#include <osg/Fog>
#include <osg/Geode>
#include <osg/LightSource>
#include <osg/LOD>
#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osg/Version>
#include <osgShadow/ShadowedScene>

#define VTLISPSM 0

class vtCamera;

/** \addtogroup sg */
/*@{*/


///////////////////////////////////////////////////////////////////////////////
// vtlib's Extension class provide some additional functionality to OSG's
//  Node, Group, and MatrixTransform classes.
//

/**
 Extends osg::Node with methods to easily enable/disable the node, and control
 whether it casts a shadow.
 */
struct NodeExtension
{
	NodeExtension();

	void SetEnabled(bool bOn);
	bool GetEnabled() const;

	/// Set this node to cast a shadow, if it is under a vtShadow node.  Default is false.
	void SetCastShadow(bool b);

	/// Get whether this node casts a shadow.
	bool GetCastShadow();

	/// Get the Bounding Sphere of the node
	void GetBoundSphere(FSphere &sphere, bool bGlobal = false);

	// Implementation
	void SetOsgNode(osg::Node *n);
	osg::Node *m_pNode;
	bool m_bCastShadow;
};

/**
 Extends OSG's MatrixTransform with methods to easily operate on its matrix.
 */
struct TransformExtension: public NodeExtension
{
	void SetOsgTransform(osg::MatrixTransform *xform) { m_pNode = m_pTransform = xform; }

	/** Set this transform to identity (no scale, rotation, or translation). */
	void Identity();

	/** Set the translation component of the transform */
	void SetTrans(const FPoint3 &pos);

	/** Get the translation component of the transform */
	FPoint3 GetTrans() const;

	/** Apply a relative offset (translation) to the transform, in the frame
		of its parent. */
	void Translate(const FPoint3 &pos);

	/** Apply a relative offset (translation) to the transform, in its own
		frame of reference. */
	void TranslateLocal(const FPoint3 &pos);

	/** Rotate around a given axis by a given angle, in radians. */
	void Rotate(const FPoint3 &axis, double angle);

	/** Similar to Rotate2, but operates in the local frame of reference. */
	void RotateLocal(const FPoint3 &axis, double angle);

	/** Similar to Rotate2, but the rotation occurs in the parent's frame of
		reference. */
	void RotateParent(const FPoint3 &axis, double angle);

	/** Get the orientation (rotate relative to default) of the transform */
	FQuat GetOrient() const;

	/** Get the direction, which by convention is shorthand for the -Z axis
		of the local frame.  For oriented objects such as the camera, this
		is the direction the object is facing. */
	FPoint3 GetDirection() const;

	/** Set the direction, which by convention is shorthand for the -Z axis
		of the local frame.  For oriented objects such as the camera, this
		is the direction the object is facing. */
	void SetDirection(const FPoint3 &point, bool bPitch = true);

	/** Scale (stretch) by the given factor in all dimensions. */
	void Scale(float factor);
	/** Scale (stretch) by given factors in the x,y,z dimensions. */
	void Scale(float x, float y, float z);

	/** Set the entire transform with a 4x4 matrix. */
	void SetTransform(const FMatrix4 &mat);
	/** Get the entire transform as a 4x4 matrix. */
	void GetTransform(FMatrix4 &mat) const;

	/** Rotate the object such that it points toward a given point.  By
		convention, this means that the object's -Z axis points in the
		desired direction. */
	void PointTowards(const FPoint3 &point, bool bPitch = true);

	osg::MatrixTransform *m_pTransform;
};


///////////////////////////////////////////////////////////////////////////////
// Standalone methods which operate on a node
//

bool FindAncestor(osg::Node *node, osg::Node *parent);
osg::Node *FindDescendent(osg::Group *node, const char *pName);

void InsertNodeAbove(osg::Node *node, osg::Group *newnode);
void InsertNodeBelow(osg::Group *group, osg::Group *newnode);

void LocalToWorld(osg::Node *node, FPoint3 &point);
void GetBoundSphere(osg::Node *node, FSphere &sphere, bool bGlobal = false);
FSphere GetGlobalBoundSphere(osg::Node *node);
bool ContainsParticleSystem(osg::Node *node);

void SetEnabled(osg::Node *node, bool bOn);
bool GetEnabled(osg::Node *node);
bool NodeIsEnabled(osg::Node *node);

void ApplyVertexRotation(osg::Node *node, const FPoint3 &axis, float angle);
void ApplyVertexTransform(osg::Node *node, const FMatrix4 &mat);

void vtLogGraph(osg::Node *node, bool bExtents = false, bool bRefCounts = false, int indent=0);
void WriteDotFile(osg::Group *node, const char *filename);


///////////////////////////////////////////////////////////////////////////////
// File I/O
//

/// Load a 3D model file
osg::Node *vtLoadModel(const char *filename, bool bAllowCache = true,
					   bool bDisableMipmaps = false);
bool vtSaveModel(osg::Node *node, const char *filename);
extern bool g_bDisableMipmaps;	// set to disable ALL mipmaps


///////////////////////////////////////////////////////////////////////////////
// Node classes
//

typedef osg::ref_ptr<osg::Node> NodePtr;

/**
 * Represents a Group (a node that can have children) in the vtlib Scene Graph.
 */
class vtGroup : public osg::Group, public NodeExtension
{
public:
	vtGroup();
};
typedef osg::ref_ptr<vtGroup> vtGroupPtr;
typedef osg::ref_ptr<osg::Group> GroupPtr;

/**
 * A Transform node allows you to apply a transform (scale, rotate, translate)
 * to all its child nodes.
 */
class vtTransform : public osg::MatrixTransform, public TransformExtension
{
public:
	vtTransform();
};
typedef osg::ref_ptr<vtTransform> vtTransformPtr;

/**
 * A Fog node allows you to apply a fog to all its child nodes.
 */
class vtFog : public osg::Group, public NodeExtension
{
public:
	vtFog();

	static RGBf s_white;
	void SetFog(bool bOn, float start = 0, float end = 10000, const RGBf &color = s_white, osg::Fog::Mode eType = osg::Fog::LINEAR);

protected:
	osg::ref_ptr<osg::StateSet> m_pFogStateSet;
	osg::ref_ptr<osg::Fog> m_pFog;
};
typedef osg::ref_ptr<vtFog> vtFogPtr;


class vtHeightField3d;
class vtLodGrid;

/**
	A Shadow node allows you to apply shadows within all its child nodes.
	Add it to your scene graph, and all descendents will receive a cast
	shadow.  Only certain nodes, which are set with vtNode::SetCastShadow,
	will cast a shadow.
 */
class vtShadow : public osgShadow::ShadowedScene, public NodeExtension
{
public:
	vtShadow(const int ShadowTextureUnit, int LightNumber);

	/// Set the darkness of the shadow, from 0 to 1.  Only supported on newer 3D cards.
	void SetDarkness(float bias);
	/// Get the darkness, from 0 to 1.
	float GetDarkness();

	void AddAdditionalTerrainTextureUnit(const uint Unit, const uint Mode);
	void RemoveAdditionalTerrainTextureUnit(const uint Unit);
	void RemoveAllAdditionalTerrainTextureUnits();

	/// A single texture is used for the shadow.  It's resolution defaults to 1024.
	void SetShadowTextureResolution(const uint ShadowTextureResolution);

	/// The shadow may be recalculated every frame, or for improved performance, only when desired
	void SetRecalculateEveryFrame(const bool RecalculateEveryFrame);
	bool GetRecalculateEveryFrame() const;

	void SetShadowSphereRadius(const float ShadowSphereRadius);
	void SetHeightField3d(vtHeightField3d *pHeightField3d);
	void AddLodGridToIgnore(vtLodGrid* pLodGrid);
	void ForceShadowUpdate();

	void SetDebugHUD(vtGroup *pGroup);

protected:
	const int m_ShadowTextureUnit;
	const int m_LightNumber;
};
typedef osg::ref_ptr<vtShadow> vtShadowPtr;

/**
 * A Light node is placed into the scene graph to illuminate all lit geometry
 * (geometry that has lighting enabled and has vertex normals).
 *
 * If you want a light, you should create a vtLightSource node and add it to your
 * scene graph.  To move or orient the lightsource, make it a child of a vtTransform
 * node.  The light will illuminate the entire scene.
 */
class vtLightSource : public osg::LightSource, public NodeExtension
{
public:
	vtLightSource(int LightNumber);

	void SetDiffuse(const RGBf &color);
	RGBf GetDiffuse() const;
	void SetAmbient(const RGBf &color);
	RGBf GetAmbient() const;
	void SetSpecular(const RGBf &color);
	RGBf GetSpecular() const;

protected:
	//  to ensure that reference counting is respected.
	virtual ~vtLightSource() {}
};

/** A vtGeode is a "Geometry Node "which can contain any number of visible
 vtMesh objects.
 \par
 A vtGeode also manages a set of Materials (vtMaterial).  Each contained mesh
 is assigned one of these materials, by index.
 \par
 This separation (Group/Mesh) provides the useful ability to define a vtMesh
 once in memory, and have multiple vtGeode nodes which contain it, which
 permits a large number of visual instances (each with potentially different
 material and transform) with very little memory cost.
 */
class vtGeode : public osg::Geode, public NodeExtension
{
public:
	vtGeode();

	vtGeode *CloneGeode();
	void CloneFromGeode(const vtGeode *rhs);

	/** Add a mesh to this geometry.
		\param pMesh The mesh to add
		\param iMatIdx The material index for this mesh, which is an index
			into the material array of the geometry. */
	void AddMesh(vtMesh *pMesh, int iMatIdx);

	/** Remove a mesh from the geomtry.  Has no effect if the mesh is not
		currently contained. */
	void RemoveMesh(vtMesh *pMesh);

	/** Remove all meshes from the geomtry. They are refcounted so there is no
		need to worry about freeing them. */
	void RemoveAllMeshes();

	/** Add a text mesh to this geometry.
		\param pMesh The mesh to add
		\param iMatIdx The material index for this mesh, which is an index
			into the material array of the geometry.
		\param bOutline true to put a dark outline around the text for contrast. */
	void AddTextMesh(vtTextMesh *pMesh, int iMatIdx, bool bOutline = true);

	/** Return the number of contained meshes. */
	uint NumMeshes() const;

	/** Return a contained vtMesh by index. */
	vtMesh *GetMesh(int i) const;

	/** Return a contained vtTextMesh by index. */
	vtTextMesh *GetTextMesh(int i) const;

	virtual void SetMaterials(const class vtMaterialArray *mats);
	const vtMaterialArray	*GetMaterials() const;

	vtMaterial *GetMaterial(int idx);

	void SetMeshMatIndex(vtMesh *pMesh, int iMatIdx);

	// OSG implementation
	osg::ref_ptr<const vtMaterialArray> m_pMaterialArray;

protected:
	// Destructor is protected so that people will use Release() instead,
	//  to ensure that reference counting is respected.
	virtual ~vtGeode() {}
};
typedef osg::ref_ptr<vtGeode> vtGeodePtr;

/**
 * A utility class which simply wraps some geometry (vtGeode) inside a
 * transform (vtTransform) so that you can move it.
 */
class vtMovGeode : public vtTransform
{
public:
	vtMovGeode(vtGeode *pContained) : vtTransform()
	{
		m_pGeode = pContained;
		addChild(m_pGeode);
	}
	vtGeode	*m_pGeode;
};
typedef osg::ref_ptr<vtMovGeode> vtMovGeodePtr;

/**
 * An LOD node controls the visibility of its child nodes.
 *
 * You should set a distance range for each child, which determines
 * at what distance from the camera the node should be rendered.
 */
class vtLOD : public osg::LOD, public NodeExtension
{
public:
	vtLOD();

	/// Set the location of the LOD's center.  Distance from this center determines detail.
	void SetCenter(const FPoint3 &center) { setCenter(v2s(center)); }

	/// Get the location of the LOD's center
	void GetCenter(FPoint3 &center) { s2v(getCenter(), center); }

protected:
	virtual ~vtLOD() {}
};

/**
 * A Camera is analogous to a physical camera: it describes the location
 * and direction from which the scene is rendered.  It can either be a
 * perspective or orthographic camera, and it is very easy to control
 * since it inherits all the methods of a transform (vtTransform).
 *
 * Although the camera is a node, this is purely for convenience.  You
 * do not have to place the camera node in your scene graph.  You may,
 * however, tell your scene (vtScene) which camera to use.  The scene
 * produces a default camera which is used unless you tell it otherwise.
 */
class vtCamera : public vtTransform
{
public:
	vtCamera();

	void SetHither(float f);
	float GetHither() const;
	void SetYon(float f);
	float GetYon() const;
	void SetFOV(float f);
	float GetFOV() const;
	float GetVertFOV() const;

	void SetOrtho(bool bOrtho);
	bool IsOrtho() const;
	void SetWidth(float f);
	float GetWidth() const;

	void ZoomToSphere(const FSphere &sphere, float fPitch = 0.0f);

protected:
	float m_fFOV;
	float m_fHither;
	float m_fYon;

	bool m_bOrtho;
	float m_fWidth;

	virtual ~vtCamera() {}
};
typedef osg::ref_ptr<vtCamera> vtCameraPtr;

/**
 * A HUD ("heads-up display") is a group whose whose children are transformed
 * to be drawn in window coordinates, rather than world coordinates.
 */
class vtHUD : public osg::Projection, public NodeExtension
{
public:
	vtHUD(bool bPixelCoords = true);

	void SetWindowSize(int w, int h);
	osg::Group *GetContainer() { return modelview_abs.get(); }

protected:
	osg::ref_ptr<osg::MatrixTransform> modelview_abs;
	bool m_bPixelCoords;
};


///////////////////////////////////////////////////////////////////////////////
// Dynamic geometry

/*
 * We create our own OSG drawable in order to override the draw method.
 */
class OsgDynMesh : public osg::Drawable
{
public:
	OsgDynMesh();

	// overrides
	virtual osg::Object* cloneType() const { return new OsgDynMesh; }
	virtual osg::Object* clone(const osg::CopyOp &foo) const { return new OsgDynMesh; }
	virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const OsgDynMesh*>(obj)!=NULL; }
	virtual const char* className() const { return "OsgDynMesh"; }

	// Implement OSG::Drawable's computeBound.
	virtual osg::BoundingBox computeBoundingBox() const;
	virtual void drawImplementation(osg::RenderInfo& renderInfo) const;

	class vtDynGeom		*m_pDynGeom;
	osg::State			*m_pDrawState;

protected:
	virtual ~OsgDynMesh() {}
};


// Visibility return codes from vtDynGeom::IsVisible
#define	VT_Visible		1
#define	VT_AllVisible	2

/**
 * vtDynGeom extends the vtGeode class with the ability to have dynamic geometry
 * which changes every frame.  The most prominent use of this feature is to do
 * Continuous Level of Detail (CLOD) for terrain.
 * \par
 * To implement, you must create your own subclass and override the following
 * methods:
 * - DoRender()
 * - DoCalcBoundBox()
 * - DoCull()
 * \par
 * Many helpful methods are provided to make doing your own view culling very easy:
 * - IsVisible(sphere)
 * - IsVisible(triangle)
 * - IsVisible(point)
 * \par
 * \see vtDynTerrainGeom
 */
class vtDynGeom : public vtGeode
{
public:
	vtDynGeom();

	// Tests a sphere or triangle, and return one of:
	//	0				- not in view
	//  VT_Visible		- partly in view
	//  VT_AllVisible	- entirely in view
	//
	int IsVisible(const FSphere &sphere) const;
	int IsVisible(const FPoint3 &point0,
				  const FPoint3 &point1,
				  const FPoint3 &point2,
				  const float fTolerance = 0.0f) const;
	int IsVisible(const FPoint3 &point, float radius) const;

	// Tests a single point, returns true if in view
	bool IsVisible(const FPoint3 &point) const;

	void ApplyMaterial(vtMaterial *mat);

	// vt methods (must be overriden)
	virtual void DoRender() = 0;
	virtual void DoCalcBoundBox(FBox3 &box) = 0;
	virtual void DoCull(const vtCamera *pCam) = 0;

	// The current clipping planes
	FPlane		m_cullPlanes[6];

protected:
	OsgDynMesh	*m_pDynMesh;
};
typedef osg::ref_ptr<vtDynGeom> vtDynGeomPtr;


///////////////////////////////////////////////////////////////////////////////
// Intersection testing

/**
 * This class describes a single point at which vtIntersect has determined
 * a line has intersected some geometry.  At this point, vtHit tells
 * you the node that was hit, the 3D point of intersection, and the
 * distance from the start of the line.
 */
struct vtHit
{
	bool operator < (const vtHit &i) const { return distance < i.distance; }
	osg::Geode *geode;
	FPoint3 point;
	float distance;
};

typedef std::vector<vtHit> vtHitList;
int vtIntersect(osg::Node *pTop, const FPoint3 &start, const FPoint3 &end,
				vtHitList &hitlist, bool bLocalCoords = false, bool bNativeNodes = true);


///////////////////////////////////////////////////////////////////////////////
// Statistics: exact bounds and primitive counts

/** Contains information about the primitives in a set of geometry */
struct vtPrimInfo
{
	/// Number of vertices which the geometry will draw.
	int Vertices;
	/// Total number of primitives of all types.
	int Primitives;
	/// Number of vertices stored in memory, which may appear more than once in indexed primitives.
	int MemVertices;

	/// Number of Point primitives.
	int Points;
	/// Number of Triangle Strip primitives.
	int TriStrips;
	/// Number of Triangle Fan primitives.
	int TriFans;
	/// Number of total Triangles in all the primitives.
	int Triangles;
	/// Number of Quad primitives.
	int Quads;
	/// Number of Quad Strip primitives.
	int QuadStrips;
	/// Number of Polygon primitives.
	int Polygons;
	/// Number of Line Strip primitives.
	int LineStrips;
	/// Number of Line Segments in all the primitives.
	int LineSegments;
};

void GetNodeBoundBox(osg::Node *node, FBox3 &box);
void GetNodePrimCounts(osg::Node *node, vtPrimInfo &info);

/*@}*/	// Group sg

#endif	// VTOSG_NODEH

