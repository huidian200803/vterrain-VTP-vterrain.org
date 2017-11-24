//
// Plants3d.h
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTLIB_PLANTS3D_H
#define VTLIB_PLANTS3D_H

/** \defgroup veg Vegetation
 * These classes are used for vegetation.  Vegetation in the vtlib library
 * consists of subclassing the vtdata vegetation classes to extend them with
 * the ability to create and operate on 3D geometry of the plants.
 */
/*@{*/

#include "vtdata/Plants.h"

class vtHeightField3d;

#if SUPPORT_XFROG
// Greenworks Xfrog:  See http://www.xfrog.com for details.  Not currently
//  supported as we don't currently have a way to interface to their engine.
#include "\dism\xfrog2dism\xfrog2dism.h"
#endif

class PlantShaderDrawable : public osg::Drawable
{
public:
	PlantShaderDrawable() { setUseDisplayList(false); }

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	PlantShaderDrawable(const PlantShaderDrawable& PlantShaderDrawable,
						const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY) :
		osg::Drawable(PlantShaderDrawable,copyop) {}

	META_Object(osg,PlantShaderDrawable);

	typedef std::vector<osg::Vec4> VecVec4;

	virtual void drawImplementation(osg::RenderInfo &renderInfo) const
	{
		for (VecVec4::const_iterator itr = _psizelist.begin();
			itr != _psizelist.end(); ++itr)
		{
			renderInfo.getState()->Color((*itr)[0],(*itr)[1],(*itr)[2],(*itr)[3]);
			_geometry->draw(renderInfo);
		}
	}

	virtual osg::BoundingBox computeBoundingBox() const
	{
#if OSG_VERSION_GREATER_THAN(3, 3, 0)
		osg::BoundingBox geom_box = _geometry->getBoundingBox();
#else
		osg::BoundingBox geom_box = _geometry->getBound();
#endif
		osg::BoundingBox bb;
		for (VecVec4::const_iterator itr = _psizelist.begin();
			 itr != _psizelist.end(); ++itr)
		{
			bb.expandBy(geom_box.corner(0)*(*itr)[3] +
				osg::Vec3( (*itr)[0], (*itr)[1], (*itr)[2] ));
			bb.expandBy(geom_box.corner(7)*(*itr)[3] +
				osg::Vec3( (*itr)[0], (*itr)[1], (*itr)[2] ));
		}
		return bb;
	}
	void setGeometry(osg::Geometry* geometry)
	{
		_geometry = geometry;
	}
	void addPlant(const osg::Vec4& pos_height)
	{
		_psizelist.push_back(pos_height);
	}

protected:
	virtual ~PlantShaderDrawable() {}
	osg::ref_ptr<osg::Geometry> _geometry;
	VecVec4 _psizelist;
};

/**
 * This class extends vtPlantAppearance with the ability to construct 3D
 * geometry for an appearance.
 */
class vtPlantAppearance3d : public vtPlantAppearance
{
public:
	vtPlantAppearance3d(AppearType type, const char *filename,
		float width, float height,
		float shadow_radius, float shadow_darkness);
	~vtPlantAppearance3d();
	vtPlantAppearance3d(const vtPlantAppearance &v);

	void CheckAvailability();
	void LoadAndCreate();
	bool GenerateGeom(vtTransform *container);
	bool IsAvailable() { return m_bAvailable; }

	static float s_fPlantScale;
	static bool  s_bPlantShadows;

	// Shader support
	osg::StateSet *GetOrCreateShaderStateset();

protected:
	vtMesh *CreateTreeMesh(float fTreeScale);
	void _Defaults();

	osg::ref_ptr<vtMaterialArray> m_pMats;
	vtGeodePtr		m_pGeode;
	osg::ref_ptr<vtMesh> m_pMesh;
	int				m_iMatIdx;
	osg::ref_ptr<osg::Node>	m_pExternal;
#if SUPPORT_XFROG
	CFrogModel	*m_pFrogModel;
#endif
	bool m_bAvailable;
	bool m_bCreated;

	// Shader support
	osg::StateSet *m_pShaderStateset;
};

/**
 * This class extends vtPlantSpecies with the ability to manage 3D
 * appearances of the species.
 */
class vtPlantSpecies3d : public vtPlantSpecies
{
public:
	vtPlantSpecies3d();

	// copy
	vtPlantSpecies3d &operator=(const vtPlantSpecies &v);

	vtPlantAppearance3d *GetAppearanceByHeight(float fHeight);
	vtPlantAppearance3d *GetRandomAppearance();
	vtPlantAppearance3d *GetAppearance(int i) const { return (vtPlantAppearance3d *)m_Apps[i]; };

	void CheckAvailability();
	int NumAvailableInstances();

	virtual void AddAppearance(AppearType type, const char *filename, float width, float height,
		float shadow_radius, float shadow_darkness);
};


/**
 * This class extends vtSpeciesList with the ability to construct and
 * manage 3D representations of the plants.
 */
class vtSpeciesList3d : public vtSpeciesList
{
public:
	vtSpeciesList3d();

	// copy operator
	vtSpeciesList3d &operator=(const vtSpeciesList &v);

	int CheckAvailability();
	void CreatePlantSurfaces();

	// override a method of vtSpeciesList
	vtPlantSpecies3d *GetSpecies(uint i) const;
};

/**
 * This class works in parallel with vtPlantInstance to contain the 3D
 * geometry for a plant instance.  An array of these objects are
 * maintained by vtPlantInstanceArray3d.
 */
class vtPlantInstance3d
{
public:
	vtPlantInstance3d();

	void ShowBounds(bool bShow);
	void ReleaseContents();

	vtTransform *m_pContainer;
	vtGeode		*m_pHighlight;	// The wireframe highlight
};

/**
 * A simple structure (used by value) which stores just the information about 
 * each plant that we need to place it into a Cell and construct it.
 */
struct vtPlantInstanceShader : public osg::Referenced
{
	osg::Vec3 m_pos;
	float m_size;
	short m_species_id;
};

typedef std::vector< vtPlantInstanceShader > TreeList;
typedef std::map<vtPlantAppearance3d*, PlantShaderDrawable*> PlantShaderMap;
namespace osg { class GroupLOD; }

/**
 * A helper class used by vtPlantInstanceArray3d to distribute its
 * plants in a tree of LOD nodes, for efficient culling.
 */
class PlantCell : public osg::Referenced
{
public:
    typedef std::vector< osg::ref_ptr<PlantCell> > CellList;

    PlantCell() : _parent(0) {}
    PlantCell(osg::BoundingBox& bb) : _parent(0), _bb(bb) {}
        
    void addCell(PlantCell* cell) { cell->_parent=this; _cells.push_back(cell); }
	void reserveTrees(uint number) { _trees.reserve(number); }
    void addTree(vtPlantInstanceShader tree) { _trees.push_back(tree); }
    bool contains(const osg::Vec3& position) const { return _bb.contains(position); }
        
    void computeBound();
    bool divide(unsigned int maxNumTreesPerCell=10);
    bool divide(bool xAxis, bool yAxis, bool zAxis);
    void bin();

    PlantCell*        _parent;
    osg::BoundingBox  _bb;
    CellList          _cells;
    TreeList          _trees;

	// Shader support: each potential plant appearance (texture) needs its own
	// drawable per cell, so we use a map here to keep track of the drawables.
	PlantShaderMap m_ShaderDrawables;

	// The Cell only points to its nodes, it doesn't own (refcount) them; they
	// are held in the scenegraph instead.
	osg::GroupLOD *m_group;
};

/**
 * This class extends vtPlantInstanceArray with the ability to construct and
 * manage 3D representations of the plants.
 *
 * \ingroup veg
 */
class vtPlantInstanceArray3d : public vtPlantInstanceArray
{
public:
	vtPlantInstanceArray3d();
	~vtPlantInstanceArray3d();

	int CreatePlantNodes(bool progress_dialog(int) = NULL);
	bool CreatePlantNode(uint i);
	int NumOffTerrain() const { return m_iOffTerrain; }

	vtTransform *GetPlantNode(uint i) const;
	vtPlantInstance3d *GetInstance3d(uint i) const;

	/// Indicate the heightfield which will be used for the plants in this array
	void SetHeightField(vtHeightField3d *hf) { m_pHeightField = hf; }

	/// Indicate the Plant List to use
	vtSpeciesList3d *GetSpeciesList() { return (vtSpeciesList3d *) m_pSpeciesList; }

	void SetEnabled(bool en);
	bool GetEnabled() const;

	/// Deselect all plants including turning off their visual highlights
	void VisualDeselectAll();

	/// Select a single plant, and visually highlight it
	void VisualSelect(uint i);

	void OffsetSelectedPlants(const DPoint2 &offset);

	void ReleasePlantGeometry(uint i);
	void DeletePlant(uint i);

	void UpdateTransform(uint i);

	bool FindPlantFromNode(osg::Node *pNode, int &iOffset);

	// Shader support
	void AddShaderGeometryForPlant(PlantCell *cell, vtPlantInstanceShader &pis);
	PlantShaderDrawable *MakePlantShaderDrawable(PlantCell *cell, vtPlantAppearance3d *ps);
	osg::Node *CreateCellNodes(PlantCell *cell);
	int CreatePlantShaderNodes(bool progress_dialog(int) = NULL);

	vtGroupPtr m_group;

protected:
	vtArray<vtPlantInstance3d*>	m_Instances3d;
	vtHeightField3d		*m_pHeightField;
	int					m_iOffTerrain;
};

/*@}*/	// Group veg

#endif	// VTLIB_PLANTS3D_H

