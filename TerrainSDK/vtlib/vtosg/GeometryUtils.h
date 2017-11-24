//
// GeometryUtils.h
//
// Copyright (c) 2011-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_GEOMETRYUTILSH
#define VTOSG_GEOMETRYUTILSH

#include "vtlib/core/Structure3d.h"

class vtBuilding3d;
class vtLevel;

namespace OSGGeomUtils {

osg::PrimitiveSet* MakeAPrimitiveSet(const int VertexType, 
	const osg::PrimitiveSet::Type Type, const osg::PrimitiveSet::Mode Mode);

class Arrays
{
public:
	int m_VertexType;
	osg::ref_ptr<osg::Vec3Array> m_Vertices;
	osg::ref_ptr<osg::Vec3Array> m_Normals;
	osg::ref_ptr<osg::Vec2Array> m_TexCoords;
};

class DATriangles : public osg::DrawArrays, public Arrays
{
public:
	DATriangles(const int VertexType, const osg::PrimitiveSet::Mode Mode);
};

class DALLineStrip : public osg::DrawArrayLengths, public Arrays
{
public:
	DALLineStrip(const int VertexType, const osg::PrimitiveSet::Mode Mode);
};

class DALTriangleFan : public osg::DrawArrayLengths, public Arrays
{
public:
	DALTriangleFan(const int VertexType, const osg::PrimitiveSet::Mode Mode);
};

class PrimSetCacheEntry : public osg::Referenced
{
public:
	PrimSetCacheEntry(osg::PrimitiveSet* PrimitiveSet) : m_pPrimitiveSet(PrimitiveSet) {}
	osg::ref_ptr<osg::PrimitiveSet> m_pPrimitiveSet;
protected:
	~PrimSetCacheEntry() {}
};

class PrimSetCache : public std::vector<osg::ref_ptr<PrimSetCacheEntry> >,
					 public osg::Referenced
{
public:
	osg::PrimitiveSet* FindOrCreatePrimitive(const int VertexType,
		const vtMaterial* pMaterial, const osg::PrimitiveSet::Type Type,
		const osg::PrimitiveSet::Mode Mode);
protected:
	~PrimSetCache() {}
};

class StateSetCacheEntry : public osg::Referenced
{
public:
	StateSetCacheEntry(osg::StateSet& StateSet) : m_StateSet(StateSet)
	{
		m_pPrimSetCache = new PrimSetCache;
	}
	osg::StateSet& m_StateSet;
	osg::ref_ptr<PrimSetCache> m_pPrimSetCache;
protected:
	~StateSetCacheEntry() {}
};

class StateSetCache : public std::vector<osg::ref_ptr<StateSetCacheEntry> >,
					  public osg::Referenced
{
public:
	osg::PrimitiveSet* FindOrCreatePrimitive(const int VertexType,
		vtMaterial* pMaterial, const osg::PrimitiveSet::Type Type,
		const osg::PrimitiveSet::Mode Mode);
protected:
	~StateSetCache() {}
};

class VertexTypeCacheEntry : public osg::Referenced
{
public:
	VertexTypeCacheEntry(const int VertexType) : m_VertexType(VertexType)
	{
		m_pStateSetCache = new StateSetCache;
	}
	int m_VertexType;
	osg::ref_ptr<StateSetCache> m_pStateSetCache;
protected:
	~VertexTypeCacheEntry() {}
};

class PrimitiveCache : public std::vector<osg::ref_ptr<VertexTypeCacheEntry> >,
					   public osg::Referenced
{
public:
	DATriangles* FindOrCreateDATriangles(const int VertexType,
		vtMaterial* pMaterial);
	DALLineStrip* FindOrCreateDALLineStrip(const int VertexType, vtMaterial* pMaterial);
	DALTriangleFan* FindOrCreateDALTriangleFan(const int VertexType, vtMaterial* pMaterial);
	osg::PrimitiveSet* FindOrCreatePrimitive(const int VertexType, vtMaterial* pMaterial,
		const osg::PrimitiveSet::Type Type, const osg::PrimitiveSet::Mode Mode);
	vtGeode* Realise(bool bUseVertexBufferObjects = false) const;
protected:
	~PrimitiveCache() {}
};

class GeometryBuilder : public osg::Referenced,
						public vtStructure3d // subclass vtStructure3d to get hold of shared material functions 
{
public:
	GeometryBuilder(const vtBuilding3d& Building) : m_Building(Building) {}
	vtGeode* Generate();
protected: 
	void AddFlatRoof(const FPolygon3 &pp, const vtLevel *pLev);
	void CreateUniformLevel(int iLevel, float fHeight, int iHighlightEdge);
	bool MakeFacade(vtEdge *pEdge, FLine3 &quad, int stories);
	osg::Vec3 Normal(const vtVec3 &p0, const vtVec3 &p1, const vtVec3 &p2);
	void AddWallSection(vtEdge *pEdge, bool bUniform,
		const FLine3 &quad, float vf1, float vf2, float hf1 = -1.0f);
	void AddHighlightSection(vtEdge *pEdge, const FLine3 &quad);
	float MakeFelkelRoof(const FPolygon3 &EavePolygons, const vtLevel *pLev);
	bool Collinear2d(const FPoint3& Previous, const FPoint3& Current, const FPoint3& Next);
	void CreateUpperPolygon(const vtLevel *lev, FPolygon3 &polygon, FPolygon3 &polygon2);
	void CreateEdgeGeometry(const vtLevel *pLev, const FPolygon3 &polygon1,
									const FPolygon3 &polygon2, int iEdge, bool bShowEdge);
	void AddWallNormal(vtEdge *pEdge, vtEdgeFeature *pFeat, const FLine3 &quad);
	void AddWindowSection(vtEdge *pEdge, vtEdgeFeature *pFeat, const FLine3 &quad);
	void AddDoorSection(vtEdge *pEdge, vtEdgeFeature *pFeat, const FLine3 &quad);
	int FindVertex(const FPoint3 &Point, const FLine3 &RoofSection3D, const std::vector<int> &iaVertices);
	~GeometryBuilder() {}
	const vtBuilding3d& m_Building;
	osg::ref_ptr<PrimitiveCache> m_pPrimitiveCache;

	// abstract memebers
	osg::Node *GetContained(void) { return NULL; }
	bool CreateNode(vtTerrain *) { return false; }
	bool IsCreated(void) { return false; }
	void DeleteNode(void) {}
};

class osg::Geometry* FindOrCreateGeometryObject(osg::Geode *pGeode,
	vtMaterial& Material, const int ArraysRequired);
class osg::PrimitiveSet* FindOrCreatePrimitiveSet(osg::Geometry* pGeometry,
	const osg::PrimitiveSet::Mode Mode, const osg::PrimitiveSet::Type Type);

}; // namespace OSGGeomUtils

#endif	// VTOSG_GEOMETRYUTILSH

