//
// Roads.h
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TERRAINROADSH
#define TERRAINROADSH

/** \defgroup transp Transportation
 * These classes are used for transportation - roads, trails and rails.
 * Transportation in the vtlib library consists of subclassing the vtdata
 * transportation classes to extend them with the ability to create and
 * operate on 3D geometry of the links.
 */
/*@{*/

#include "vtdata/FilePath.h"
#include "vtdata/RoadMap.h"
#include "vtdata/HeightField.h"
#include "LodGrid.h"

#define ROAD_CLUSTER	16

/**
 * A 'virtual' texture is a subset of an another texture; defined by
 * rectangular UV extents that refer to an area of source texels.  It is
 * used by the RoadMap classes to pack a number of textures into a single
 * "roadway" texture, but could certainly by extended into a general-purpose
 * class in the future.
 *
 * One thing to watch out for is that a virtual texture won't necessarily
 * tile (repeat) as expected, since it make have other virtual textures
 * adjacent to it in the source texture.
 */
class VirtualTexture
{
public:
	int		m_idx;	// material index
	FRECT	m_rect;	// region of the base texture to use

	void Adapt(const FPoint2 &in, FPoint2 &out) const
	{
		out.x = m_rect.left + in.x * (m_rect.right - m_rect.left);
		out.y = m_rect.bottom + in.y * (m_rect.top - m_rect.bottom);
	}
};

//
// virtual texture indices
//
enum RoadVTIndices
{
	VTI_MARGIN,
	VTI_SIDEWALK,
	VTI_1LANE,
	VTI_2LANE1WAY,
	VTI_2LANE2WAY,
	VTI_3LANE1WAY,
	VTI_3LANE2WAY,
	VTI_4LANE1WAY,
	VTI_4LANE2WAY,
	VTI_RAIL,
	VTI_4WD,
	VTI_TRAIL,
	VTI_GRAVEL,
	VTI_STONE,
	VTI_PAVEMENT,
	VTI_TOTAL
};

/**
 * A Node is a place where 2 or more links meet.  NodeGeom extents Node
 * with 3D geometry.
 */
class NodeGeom : public TNode
{
public:
	NodeGeom();
	virtual ~NodeGeom();

	class LinkGeom *GetLink(int n)
	{
		return (class LinkGeom *)m_connect[n];
	}
	void ComputeIntersectionVertices();
	void FindVerticesForLink(TLink *pR, bool bStart, FPoint3 &p0, FPoint3 &p1);
	vtMesh *GenerateGeometry(const VirtualTexture &vt);
	FPoint3 GetUnitLinkVector(int i);
	const FPoint3 &GetAdjacentRoadpoint(int iLinkNumber);
	NodeGeom *GetNext() { return (NodeGeom*) m_pNext; }

	int m_iVerts;
	FLine3 m_v;		// vertices of the polygon
	FPoint3 m_p3;
};

//
// Holds information about a roadway, used internally by vtRoadMap3d during
// road construction.
//
class RoadBuildInfo
{
public:
	RoadBuildInfo(int iCoords);
	FLine3 center;
	FLine3 crossvector;
	vtArray<float> fvLength;

	int verts;
	int vert_index;
};

enum normal_direction {
	ND_UP,
	ND_LEFT,
	ND_RIGHT
};

/**
 * A link is series of points, connecting one node to another.  LinkGeom
 * extents Link with 3D geometry.
 */
class LinkGeom : public TLink
{
public:
	LinkGeom();
	virtual ~LinkGeom();

	FPoint3 FindPointAlongRoad(float fDistance);
	float	Length();	// find 2D length in world units

	// link-construction methods
	void SetupBuildInfo(RoadBuildInfo &bi);
	void AddRoadStrip(vtMesh *pGeom, RoadBuildInfo &bi,
					float offset_left, float offset_right,
					float height_left, float height_right,
					VirtualTexture &vt,
					float u1, float u2, float uv_scale,
					normal_direction nd);
	void GenerateGeometry(class vtRoadMap3d *rmgeom);

	NodeGeom *GetNode(int n) { return (NodeGeom *)m_pNode[n]; }
	LinkGeom *GetNext() { return (LinkGeom *)m_pNext; }

	int m_vti;
	FLine3 m_centerline;

	/* Lanes lines, which define the centerline of each trafficable lane,
	 * are created by vtRoadMap3d for traffic simulation/visualization
	 * purposes.
	 */
	std::vector<FLine3> m_Lanes;
};


/**
 * vtRoadMap3d extents vtRoadMap with 3D geometry.
 */
class vtRoadMap3d : public vtRoadMap, public osg::Referenced
{
public:
	vtRoadMap3d();
	~vtRoadMap3d();

	// overrides for virtual methods
	NodeGeom *GetFirstNode() { return (NodeGeom *) m_pFirstNode; }
	LinkGeom *GetFirstLink() { return (LinkGeom *) m_pFirstLink; }
	NodeGeom *NewNode() { return new NodeGeom; }
	LinkGeom *NewLink() { return new LinkGeom; }
	NodeGeom *AddNewNode()
	{
		NodeGeom *node = new NodeGeom;
		AddNode(node);
		return node;
	}
	LinkGeom *AddNewLink()
	{
		LinkGeom *link = new LinkGeom;
		AddLink(link);
		return link;
	}

	void DrapeOnTerrain(vtHeightField3d *pHeightField);
	void ComputeIntersectionVertices();
	void AddMeshToGrid(vtMesh *pMesh, int iMatIdx);
	vtTransform *GenerateGeometry(bool do_texture, bool bHwy, bool bPaved,
		bool bDirt, bool progress_callback(int) = NULL);
	void GenerateSigns(vtLodGrid *pLodGrid);
	vtGroup *GetGroup() { return m_pGroup; }
	void SetHeightOffGround(float fHeight);
	float GetHeightOffGround() { return m_fGroundOffset; }
	void DetermineSurfaceAppearance();

	void SetLodDistance(float fDistance);
	float GetLodDistance();

public:
	// virtual textures
	VirtualTexture	m_vt[VTI_TOTAL];

	// material indices
	int		m_mi_roadside;
	int		m_mi_pavement;
	int		m_mi_roads;
	int		m_mi_4wd;
	int		m_mi_trail;
	int		m_mi_gravel;
	int		m_mi_red;

protected:
	int _CreateMaterial(const char *texture_filename, bool bTransparency);
	void _CreateMaterials(bool do_texture);
	void _GatherExtents();

	vtTransform	*m_pTransform;	// For elevating the roads above the terrain.
	vtGroup		*m_pGroup;
	vtMaterialArrayPtr m_pMats;

	vtLOD		*m_pRoads[ROAD_CLUSTER][ROAD_CLUSTER];
	FBox3		m_extents;
	FPoint3		m_extent_range;
	float		m_fLodDistance;		// in meters
	float		m_fGroundOffset;	// in meters
};
typedef osg::ref_ptr<vtRoadMap3d> vtRoadMap3dPtr;

// Useful typedefs
typedef NodeGeom *NodeGeomPtr;
typedef LinkGeom *LinkGeomPtr;

/*@}*/  // transp

#endif

