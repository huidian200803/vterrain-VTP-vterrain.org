//
// TerrainLayers.h
//
// Copyright (c) 2006-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TERRAINLAYERSH
#define TERRAINLAYERSH

#include "vtdata/LayerBase.h"
#include "Structure3d.h"	// for vtStructureArray3d
#include "Plants3d.h"		// for vtPlantInstanceArray3d

/** \addtogroup terrain */
/*@{*/

/**
 * Simple abstraction class to describe all vtlib terrain layers.
 */
class vtLayer : public osg::Referenced, public vtLayerBase
{
public:
	vtLayer(LayerType type) : vtLayerBase(type) {}

	virtual void SetLayerName(const vtString &fname) = 0;
	virtual vtString GetLayerName() = 0;

	/// Set the properties for this layer, which includes style.
	void SetProps(const vtTagArray &props) { m_Props = props; }
	void AddProps(const vtTagArray &props) { m_Props.CopyTagsFrom(props); }

	/// Get the properties for this layer, which includes style.
	vtTagArray &Props() { return m_Props; }
	const vtTagArray &Props() const { return m_Props; }

protected:
	// A set of properties that can provide additional information, such as
	//  style information for visual display.
	vtTagArray	m_Props;
};
typedef osg::ref_ptr<vtLayer> vtLayerPtr;

/**
 * This class encapsulates vtStructureArray3d as a terrain layer.
 */
class vtStructureLayer : public vtStructureArray3d, public vtLayer
{
public:
	vtStructureLayer();

	bool Load(bool progress_callback(int) = NULL);

	void SetLayerName(const vtString &fname);
	vtString GetLayerName() { return GetFilename(); }
	void SetVisible(bool vis)
	{
		SetEnabled(vis);
		vtLayerBase::SetVisible(vis);
	}
};

/**
 * This class encapsulates vtPlantInstanceArray3d as a terrain layer.
 */
class vtVegLayer : public vtPlantInstanceArray3d, public vtLayer
{
public:
	vtVegLayer();

	void SetLayerName(const vtString &fname) { SetFilename(fname); }
	vtString GetLayerName() { return GetFilename(); }
	void SetVisible(bool vis)
	{
		SetEnabled(vis);
		vtLayerBase::SetVisible(vis);
	}
};

class vtMultiTexture;

/**
 * A vtlib image layer is a bitmap draped on the terrain surface.
 */
class vtImageLayer : public vtLayer
{
public:
	vtImageLayer();
	~vtImageLayer();

	void SetLayerName(const vtString &fname) { /* not applicable */ }
	vtString GetLayerName() { return m_pImage->GetFilename().c_str(); }
	void SetVisible(bool vis);
	bool GetVisible();
	bool Load(bool progress_callback(int) = NULL);

	vtImageGeoPtr m_pImage;
	osg::ref_ptr<vtMultiTexture> m_pMultiTexture;
};

class vtTin3d;

/**
 * An elevation heightfield as a terrain layer.
 * Currently, it is always a TIN (vtTin3d).
 */
class vtElevLayer : public vtLayer
{
public:
	vtElevLayer();
	~vtElevLayer();

	bool Load(bool progress_callback(int) = NULL);
	void MakeMaterials(bool bTextureCompression);
	vtTransform *CreateGeometry();

	void SetLayerName(const vtString &fname);
	vtString GetLayerName();
	void SetVisible(bool vis);
	vtTin3d *GetTin() { return m_pTin.get(); }

	vtTransform *GetTopNode() { return m_pTransform; }

protected:
	vtGeode *m_pGeode;
	vtTransform *m_pTransform;

	osg::ref_ptr<vtTin3d> m_pTin;
};

/** The set of all layers which are on a terrain. */
class LayerSet : public std::vector<vtLayerPtr>
{
public:
	void Remove(vtLayer *lay);
	vtLayer *FindByName(const vtString &name);

	vtStructureLayer *FindStructureFromNode(osg::Node *pNode, int &iOffset);
};

/*@}*/	// Group terrain

#endif // TERRAINLAYERSH

