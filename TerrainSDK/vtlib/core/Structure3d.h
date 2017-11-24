//
// Structure3d.h
//
// Implements the vtStructure3d class which extends vtStructure with the
// ability to create 3D geometry.
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef STRUCTURE3DH
#define STRUCTURE3DH

/** \addtogroup struct */
/*@{*/

#include "vtdata/StructArray.h"
#include "MaterialDescriptor3d.h"

class vtBuilding3d;
class vtFence3d;
class vtTerrain;
class vtTransform;


/**
 * This class contains the extra methods needed by a vtStructure to
 * support the ability to construct and manage a 3D representation.
 */
class vtStructure3d
{
public:
	vtStructure3d() { m_pContainer = NULL; m_bIsVIAContributor = false; m_bIsVIATarget = false; }

	vtTransform *GetContainer() { return m_pContainer.get(); }
	virtual osg::Node *GetContained() = 0;

	/// Create the node(s) and position them on the indicated heightfield
	virtual bool CreateNode(vtTerrain *pTerr) = 0;
	virtual bool IsCreated() = 0;

	/// Access the Geometry node for this structure, if it has one
	virtual vtGeode *GetGeom() { return NULL; }
	virtual void DeleteNode() = 0;

	/// Pass true to turn on a wireframe hightlight geometry for this instance
	virtual void ShowBounds(bool bShow) {}

	void SetCastShadow(bool b);
	bool GetCastShadow();

	// Visual Impact
	const bool GetVIAContributor() const { return m_bIsVIAContributor; }
	const bool GetVIATarget() const { return m_bIsVIATarget; }
	void SetVIAContributor(const bool bVIAContributor) { m_bIsVIAContributor = bVIAContributor; }
	void SetVIATarget(const bool bVIATarget) { m_bIsVIATarget = bVIATarget; }

public:
	// A number of convenience methods access the global materials table.
	static void InitializeMaterialArrays();

	// Get the material descriptors
	static vtMaterialDescriptorArray3d& GetMaterialDescriptors()
	{
		return s_MaterialDescriptors;
	}

protected:
	// You can get a material index directly from its name.
	static int GetMatIndex(const vtString &Material, const RGBf &inputColor = RGBf(), int iType = -1)
	{
		return s_MaterialDescriptors.GetMatIndex(Material, inputColor, iType);
	}
	// Or you can get its descriptor, then use that to get its material index.
	static vtMaterialDescriptor *GetMatDescriptor(const vtString &name,
		const RGBf &color = RGBf(), int type = -1)
	{
		return s_MaterialDescriptors.GetMatDescriptor(name, color, type);
	}
	static int GetMatIndex(vtMaterialDescriptor *desc, const RGBf &color = RGBf())
	{
		return s_MaterialDescriptors.GetMatIndex(desc, color);
	}
	static vtMaterialArray *GetSharedMaterialArray()
	{
		return s_MaterialDescriptors.GetMatArray();
	}

protected:
	vtTransformPtr m_pContainer;	// The transform which is used to position the object

	static vtMaterialDescriptorArray3d s_MaterialDescriptors;
	static bool s_bMaterialsInitialized;

	// Visual Impact
    bool m_bIsVIAContributor;
	bool m_bIsVIATarget;
};


/**
 * This class extends vtStructInstance with the ability to construct and
 * manage 3D representations of the instance.
 */
class vtStructInstance3d : public vtStructInstance, public vtStructure3d
{
public:
	vtStructInstance3d();

	// implement vtStructInstance methods
	virtual double DistanceToPoint(const DPoint2 &p, float fMaxRadius) const;

	// implement vtStructure3d methods
	/// Create the node(s) and position them on the indicated heightfield
	virtual bool CreateNode(vtTerrain *pTerr);
	virtual bool IsCreated();
	virtual void DeleteNode();
	virtual void ShowBounds(bool bShow);
	virtual osg::Node *GetContained() { return m_pModel.get(); }

	/// (Re-)position the instance on the indicated heightfield
	void UpdateTransform(vtHeightField3d *pHeightField);

	/// Attempt to reload from disk
	void Reload();

protected:
	vtGeode		*m_pHighlight;	// The wireframe highlight
	osg::ref_ptr<osg::Node> m_pModel; // the contained model
	double		m_RadiusInEarthCoords;
};


/**
 * This class extends vtStructureArray with the ability to construct and
 * manage 3D representations of the structures.
 */
class vtStructureArray3d : public vtStructureArray, public vtEnabledBase
{
public:
	vtStructureArray3d();

	virtual vtBuilding *NewBuilding();
	virtual vtFence *NewFence();
	virtual vtStructInstance *NewInstance();

	vtStructure3d *GetStructure3d(int i);
	vtBuilding3d *GetBuilding(int i) const { return (vtBuilding3d *) at(i)->GetBuilding(); }
	vtFence3d *GetFence(int i) { return (vtFence3d *) at(i)->GetFence(); }
	vtStructInstance3d *GetInstance(int i) { return (vtStructInstance3d *) at(i)->GetInstance(); }

	/// Indicate the heightfield which will be used for the structures in this array
	void SetTerrain(vtTerrain *pTerr) { m_pTerrain = pTerr; }

	/// Construct an individual structure, return true if successful
	bool ConstructStructure(vtStructure3d *str);
	bool ConstructStructure(int index);
	void OffsetSelectedStructures(const DPoint2 &offset);
	void OffsetSelectedStructuresVertical(float offset);

	/// Deselect all structures including turning off their visual highlights
	void VisualDeselectAll();

	/// Enable (set visibility of) the structures
	void SetEnabled(bool bTrue);

	/// Set shadows on/off for all structures
	void SetShadows(bool bTrue);

	// Be informed of edit hightlighting
	virtual void SetEditedEdge(vtBuilding *bld, int lev, int edge);

	// Be informed when a structure is deleted
	virtual void DestroyStructure(int i);

	// Experimental
	void WriteStructuresToOBJ(const char *filename);

protected:
	vtTerrain *m_pTerrain;
};

/*@}*/	// Group struct

#endif // STRUCTURE3DH

