//
// Building.h
//
// Implements the vtBuilding class which represents a single built structure.
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef BUILDINGH
#define BUILDINGH

#include <stdio.h>
#include "LocalCS.h"
#include "MathTypes.h"
#include "vtCRS.h"
#include "Structure.h"
#include "ogrsf_frmts.h"

class vtHeightField;

////////////////////////////////////////////////////

#define MINIMUM_BASEMENT_SIZE 0.5 // Mininum size of an automatically generated basement layer in a building

#define DEFAULT_BUILDING_SIZE 10.0  // Default size for buildings imported from points

enum RoofType
{
	ROOF_FLAT, ROOF_SHED, ROOF_GABLE, ROOF_HIP, ROOF_UNKNOWN, NUM_ROOFTYPES
};

enum BldColor
{
	BLD_BASIC,
	BLD_ROOF,
};

// Edge feature codes
#define WFC_WALL		1
#define WFC_GAP			2
#define WFC_POST		3
#define WFC_WINDOW		4
#define WFC_DOOR		5

class vtEdgeFeature
{
public:
	vtEdgeFeature();
	vtEdgeFeature(int code, float width = -1.0f, float vf1 = 0.0f, float vf2 = 1.0f);

	void SetDefaults();

	// classification code - door, window, wall etc.
	int		m_code;

	// style (to classify window/door/post style)
	// TODO

	// overall color of this feature, or "-1 -1 -1" for default color
	RGBi	m_color;

	// width: positive values mean meters,
	//	negative values mean proportional scaling factors
	float	m_width;

	// vertical factors: from base, start/end of feature, normalized (0-1)
	float	m_vf1;
	float	m_vf2;
};

/**
 * Each building level (vtLevel) is composed of a number of edges (vtEdge).
 * There should be one vtEdge for each edge of the level's footprint.
 *
 * Each edge is in turn composed of a number of vtEdgeFeature objects,
 * which represent such things as walls, doors, and windows.  An edge
 * can also simply consist of a Facade texture map.
 */
class vtEdge
{
public:
	vtEdge();
	vtEdge(const vtEdge &lhs);

	void Set(int doors, int windows, const char *material);
	void AddFeature(int code, float width = -1.0f, float vf1 = 0.0f, float vf2 = 1.0f);
	size_t NumFeatures() const { return m_Features.size(); }
	int NumFeaturesOfCode(int code) const;
	float FixedFeaturesWidth() const;
	float ProportionTotal() const;
	bool IsUniform() const;
	float SlopeRadians() const { return m_iSlope / 180.0f * PIf; }

	// color
	RGBi	m_Color;	// overall edge color

	// slope in degrees: 90 is vertical, 0 is horizontal
	int	m_iSlope;

	// members
	const vtString		*m_pMaterial;
	std::vector<vtEdgeFeature> m_Features;
	vtString			 m_Facade;
};

#define MAX_WALLS	360	// the largest number of walls
						// (largest number of points in a poly-shaped building)

/**
 * The vtLevel class represents a level of a building.  In the VTP's
 * parametric building representation, each building is composed of one
 * or more levels, and each level is defined horizontally by a number
 * of edges (vtEdge) and vertical by a story height and number of stories.
 * The edges contain further description of the appearance of each side
 * of this level of the building.
 *
 * Generally, a simple building will have two levels: one for the walls,
 * and one for the roof.  Some buildings may have an additional level for
 * base, such as a cement foundation.
 *
 * Each level can have its own footprint polygon, although in simple cases
 * they will all be the same polygon.
 */
class vtLevel
{
	friend class vtBuilding;
public:
	vtLevel();
	vtLevel(const vtLevel &from) { *this = from; }
	~vtLevel();

	// assignment operator
	vtLevel &operator=(const vtLevel &v);

	void DeleteEdge(int iEdge);
	bool AddEdge(const int iEdge, const DPoint2 &Point);
	int NumEdges() const { return m_Edges.GetSize(); }
	vtEdge *GetEdge(uint i) const;
	float GetEdgeLength(uint i) const;
	float GetLocalEdgeLength(uint i) const;
	const vtString GetOverallEdgeMaterial();
	bool GetOverallEdgeColor(RGBi &color);
	RoofType GuessRoofType() const;
	void FlipFootprintDirection();

	bool HasSlopedEdges() const;
	bool IsHorizontal() const;
	bool IsEdgeConvex(int i);
	bool IsCornerConvex(int i);
	bool IsUniform() const;

	void SetEdgeMaterial(const char *matname);
	void SetEdgeMaterial(const vtString *matname);
	void SetEdgeColor(RGBi color);
	void SetRoofType(RoofType rt, int iSlopeDegrees);
	void SetEaveLength(float fMeters);

	uint m_iStories;
	float	m_fStoryHeight;

	void SetFootprint(const DLine2 &dl);
	void SetFootprint(const DPolygon2 &poly);
	void ResizeEdgesToMatchFootprint();

	DPolygon2 &GetFootprint() { return m_Foot; }
	const DPolygon2 &GetFootprint() const { return m_Foot; }

	// outer ring
	DLine2 &GetOuterFootprint() { return m_Foot[0]; }
	const DLine2 &GetOuterFootprint() const { return m_Foot[0]; }

	void DetermineLocalFootprint(float fHeight, const LocalCS &local_cs);
	const FPolygon3 &GetLocalFootprint() const { return m_LocalFootprint; }

private:
	void RebuildEdges(uint n);
	void GetEdgePlane(uint i, FPlane &plane);
	bool DetermineHeightFromSlopes();
	void DeleteEdges();

	vtArray<vtEdge *> m_Edges;

	// footprint of the stories in this level
	DPolygon2	m_Foot;

	// footprint in the local CS of this building
	FPolygon3	m_LocalFootprint;
};

/**
 * This class contains a parametric representation of a building.
 *
 * The geometry is described by a series of levels (vtLevel), each of which
 * contains a set of edges (vtEdge), which in turn are composed of edge
 * features (vtEdgeFeature).  These components can vary in color, material,
 * size, and number, which gives a high-level representation of a building,
 * which can be used to construct a reasonable (visually similar) model of
 * the building.
 */
class vtBuilding : public vtStructure
{
public:
	vtBuilding();
	~vtBuilding();

	// copy
	vtBuilding &operator=(const vtBuilding &v);

	// footprint methods
	void SetFootprint(int i, const DLine2 &dl);
	void SetFootprint(int i, const DPolygon2 &poly);

	const DPolygon2 &GetFootprint(int i) const { return m_Levels[i]->GetFootprint(); }
	const DLine2 &GetOuterFootprint(int i) const { return m_Levels[i]->GetOuterFootprint(); }
	bool GetBaseLevelCenter(DPoint2 &p) const;

	void SetRectangle(const DPoint2 &center, float fWidth, float fDepth,
		float fRotation = 0.0f);
	void SetCircle(const DPoint2 &center, float fRad);

	void FlipFootprintDirection();
	float CalculateBaseElevation(vtHeightField *pHeightField);
	void TransformCoords(OCTransform *trans);

	// roof methods
	void SetRoofType(RoofType rt, int iSlope = -1, int iLev = -1);
	RoofType GetRoofType() const;
	RGBi GuessRoofColor() const;
	void SetRoofColor(const RGBi &rgb);

	void SetColor(BldColor which, const RGBi &color);
	RGBi GetColor(BldColor which) const;

	void SetNumStories(int i);
	int NumStories() const;
	float GetTotalHeight() const;

	uint NumLevels() const { return m_Levels.GetSize(); }
	vtLevel *GetLevel(int i) { return (i < (int)m_Levels.GetSize()) ? m_Levels[i] : NULL; }
	const vtLevel *GetLevel(int i) const { return (i < (int)m_Levels.GetSize()) ? m_Levels[i] : NULL; }
	vtLevel *CreateLevel(const DPolygon2 &footprint);
	vtLevel *CreateLevel();
	void InsertLevel(int iLev, vtLevel *pLev);
	void DeleteLevel(int iLev);

	bool GetExtents(DRECT &rect) const;
	void Offset(const DPoint2 &delta);
	double GetDistanceToInterior(const DPoint2 &point) const;

	void WriteXML(GZOutput &out, bool bDegrees) const;
	void AddDefaultDetails();
	void DetermineLocalFootprints();
	const FPolygon3 &GetLocalFootprint(int i) const { return m_Levels[i]->GetLocalFootprint(); }

	static const char *GetEdgeFeatureString(int edgetype);
	static int		   GetEdgeFeatureValue(const char *value);

	bool IsContainedBy(const DRECT &rect) const;
	void SwapLevels(int lev1, int lev2);
	void SetEaves(float fLength);
	void SetEavesSimple(float fLength);
	void SetEavesFelkel(float fLength);
	void CopyStyleFrom(const vtBuilding * const pSource, bool bDoHeight);

	void SetCRS(vtCRS *crs) { m_pCRS = crs; }

protected:
	// information about each story
	vtArray<vtLevel *> m_Levels;

	// Every building needs a link back up to its containing array, because
	// that's where its CRS is stored, which it needs to know for LocalCoords.
	// This is more simple and efficient than storing a CRS on every building.
	vtCRS *m_pCRS;

private:
	void DeleteLevels();
};

typedef vtBuilding *vtBuildingPtr;

///////////////////////////////////////////////////////////////////////

#endif // BUILDINGH

