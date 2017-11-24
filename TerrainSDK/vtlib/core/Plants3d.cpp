//
// Plants3d.cpp
//
// Implementation for the following classes:
//
// vtPlantAppearance3d
// vtPlantSpecies3d
// vtSpeciesList3d
// vtPlantInstance3d
// vtPlantInstanceArray3d
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/vtosg/GroupLOD.h"

#include "vtdata/vtLog.h"
#include "vtdata/DataPath.h"
#include "vtdata/FilePath.h"
#include "vtdata/HeightField.h"
#include "Plants3d.h"
#include "Light.h"
#include "GeomUtil.h"	// for CreateBoundSphereGeode

#define SHADOW_HEIGHT		0.1f	// distance above groundpoint in meters

float vtPlantAppearance3d::s_fPlantScale = 1.0f;
bool vtPlantAppearance3d::s_bPlantShadows = false;


/////////////////////////////////////////////////////////////////////////////
// vtPlantAppearance3d
//

vtPlantAppearance3d::vtPlantAppearance3d(AppearType type, const char *filename,
	 float width, float height, float shadow_radius, float shadow_darkness)
	: vtPlantAppearance(type, filename, width, height, shadow_radius, shadow_darkness)
{
	_Defaults();
}

vtPlantAppearance3d::~vtPlantAppearance3d()
{
#if SUPPORT_XFROG
	delete m_pFrogModel;
#endif
}

vtPlantAppearance3d::vtPlantAppearance3d(const vtPlantAppearance &v)
{
	_Defaults();

	m_eType = v.m_eType;
	m_filename = v.m_filename;
	m_width	= v.m_width;
	m_height = v.m_height;
	m_shadow_radius	= v.m_shadow_radius;
	m_shadow_darkness = v.m_shadow_darkness;
}

void vtPlantAppearance3d::_Defaults()
{
	m_pGeode = NULL;
	m_pMats = NULL;
	m_pMesh = NULL;
#if SUPPORT_XFROG
	m_pFrogModel = NULL;
#endif
	m_pExternal = NULL;
	m_bAvailable = false;
	m_bCreated = false;
	m_pShaderStateset = NULL;
}

// Helper
vtString FindPlantModel(const vtString &filename)
{
	vtString name = "PlantModels/";
	name += filename;
	return FindFileOnPaths(vtGetDataPath(), name);
}

void vtPlantAppearance3d::CheckAvailability()
{
	if (m_eType == AT_BILLBOARD || m_eType == AT_MODEL)
	{
		// check if file exists and is readable
		vtString fname = FindPlantModel(m_filename);
		if (fname == "")
			return;

		FILE *fp = vtFileOpen(fname, "rb");
		if (fp != NULL)
		{
			m_bAvailable = true;
			fclose(fp);
		}
#if 1
		else
			VTLOG(" C0uldn't find appearance: %s\n", (const char *)fname);
#endif
	}
	else if (m_eType == AT_XFROG)
	{
	}
}

void vtPlantAppearance3d::LoadAndCreate()
{
	// only need to create once
	if (m_bCreated)
		return;

	if (m_eType == AT_BILLBOARD)
	{
		VTLOG("\tLoading plant texture '%s' ", (const char *) m_filename);
		vtString fname = FindPlantModel(m_filename);

		m_pMats = new vtMaterialArray;

		// create textured appearance
		osg::Image *image = LoadOsgImage(fname);
		m_iMatIdx = m_pMats->AddTextureMaterial(image,
			false, true, true, false,	// cull, lighting, transp, additive
			TREE_AMBIENT, TREE_DIFFUSE,
			1.0f,		// alpha (material is opaque, alpha is in the texture)
			TREE_EMISSIVE);

		if (m_iMatIdx == -1)
			VTLOG(" Failed.\n");
		else
		{
			VTLOG(" OK.\n");
			m_bCreated = true;
		}

		// create a surface object to represent the tree
		m_pMesh = CreateTreeMesh(s_fPlantScale);
		m_pGeode = new vtGeode;
		m_pGeode->SetMaterials(m_pMats);
		m_pGeode->AddMesh(m_pMesh, m_iMatIdx);
	}
	else if (m_eType == AT_XFROG)
	{
#if SUPPORT_XFROG
		// xfrog plant
		vtString fname = FindPlantModel(vtGetDataPath(), m_filename);
		m_pFrogModel = new CFrogModel(fname, m_filename);
#endif
	}
	else if (m_eType == AT_MODEL)
	{
		m_pExternal = vtLoadModel(m_filename);
		if (!m_pExternal)
		{
			vtString fname = FindPlantModel(m_filename);
			if (fname != "")
				m_pExternal = vtLoadModel(fname);
		}
		if (m_pExternal != NULL)
			m_bCreated = true;
	}
}

/**
 * Create an object to represent a textured plant billboard.
 * Makes two intersecting polygons (4 triangles).
 */
vtMesh *vtPlantAppearance3d::CreateTreeMesh(float fTreeScale)
{
	// How many vertices we'll need for this mesh
	const int vtx_count = 8;

	// make a mesh
	vtMesh *pTreeMesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_FAN, VT_TexCoords, vtx_count);

	// size of textured, upright portion
	const float w2 = (m_width * fTreeScale) / 2.0f;
	const float h = m_height * fTreeScale;

	// The billboard geometry: 2 squares
	pTreeMesh->SetVtxPUV(0, FPoint3(-w2, 0.0f, 0), 0.0f, 0.0f);
	pTreeMesh->SetVtxPUV(1, FPoint3( w2, 0.0f, 0), 1.0f, 0.0f);
	pTreeMesh->SetVtxPUV(2, FPoint3(-w2, h, 0), 0.0f, 1.0f);
	pTreeMesh->SetVtxPUV(3, FPoint3( w2, h, 0), 1.0f, 1.0f);
	//
	pTreeMesh->SetVtxPUV(4, FPoint3(0, 0.0f, -w2), 0.0f, 0.0f);
	pTreeMesh->SetVtxPUV(5, FPoint3(0, 0.0f,  w2), 1.0f, 0.0f);
	pTreeMesh->SetVtxPUV(6, FPoint3(0, h, -w2), 0.0f, 1.0f);
	pTreeMesh->SetVtxPUV(7, FPoint3(0, h,  w2), 1.0f, 1.0f);

	// 2 fans
	pTreeMesh->AddFan(0, 1, 3, 2);
	pTreeMesh->AddFan(4, 5, 7, 6);

	return pTreeMesh;
}

bool vtPlantAppearance3d::GenerateGeom(vtTransform *container)
{
	if (m_eType == AT_BILLBOARD)
	{
		container->addChild(m_pGeode);
		return true;
	}
	else if (m_eType == AT_XFROG)
	{
#if SUPPORT_XFROG
		pGeode = m_pFrogModel->CreateShape(1.0f);
		float factor = s_fPlantScale;
		pGeode->Scale(factor, factor, factor);
#endif
	}
	else if (m_eType == AT_MODEL)
	{
		if (m_pExternal)
		{
			container->addChild(m_pExternal);
			return true;
		}
	}
	return false;
}

osg::StateSet *MakeTextureStatesetForPlantBillboard(const char *fname)
{
	osg::Texture2D *tex = new osg::Texture2D;
	tex->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP );
	tex->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP );
	tex->setImage(osgDB::readImageFile(fname));

	osg::StateSet *dstate = new osg::StateSet;
	dstate->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON );
	dstate->setTextureAttribute(0, new osg::TexEnv );
	dstate->setAttributeAndModes( new osg::BlendFunc, osg::StateAttribute::ON );

	osg::AlphaFunc *alphaFunc = new osg::AlphaFunc;
	alphaFunc->setFunction(osg::AlphaFunc::GEQUAL,0.05f);
	dstate->setAttributeAndModes( alphaFunc, osg::StateAttribute::ON );
	dstate->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
	dstate->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

	return dstate;
}

osg::StateSet *vtPlantAppearance3d::GetOrCreateShaderStateset()
{
	// We may have already created it
	if (m_pShaderStateset)
		return m_pShaderStateset;

	// We don't have to call LoadAndCreate on the appearance, because we'll be
	// created it a different way.
	vtString fname = FindPlantModel(m_filename);
	if (fname == "")
		return NULL;

	VTLOG(" Making shader stateset/drawable for plant '%s'\n", (const char *)fname);

	osg::StateSet *stateset = MakeTextureStatesetForPlantBillboard(fname);

	osg::Program* program = new osg::Program;
	stateset->setAttribute(program);

	///////////////////////////////////////////////////////////////////
	// vertex shader using just Vec4 coefficients
	char vertexShaderSource[] = 
		"varying vec2 texcoord;\n"
		"\n"
		"void main(void)\n"
		"{\n"
		"	vec3 position = gl_Vertex.xyz * gl_Color.w + gl_Color.xyz;\n"
		"	gl_Position	 = gl_ModelViewProjectionMatrix * vec4(position,1.0);\n"
		"	gl_FrontColor = vec4(1.0,1.0,1.0,1.0);\n"
		"	texcoord = gl_MultiTexCoord0.st;\n"
		"}\n";

	//////////////////////////////////////////////////////////////////
	// fragment shader
	//
	char fragmentShaderSource[] = 
		"uniform sampler2D baseTexture; \n"
		"varying vec2 texcoord; \n"
		"\n"
		"void main(void) \n"
		"{ \n"
		"	gl_FragColor = texture2D(baseTexture, texcoord); \n"
		"}\n";

	osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource);
	program->addShader(vertex_shader);

	osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource);
	program->addShader(fragment_shader);
	
	osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
	stateset->addUniform(baseTextureSampler);

	// Store it
	m_pShaderStateset = stateset;
	return m_pShaderStateset;
}


/////////////////////////////////////////////////////////////////////////////
// vtPlantSpecies3d
//

vtPlantSpecies3d::vtPlantSpecies3d() : vtPlantSpecies()
{
}

vtPlantSpecies3d &vtPlantSpecies3d::operator=(const vtPlantSpecies &v)
{
	uint i;

	m_szSciName = v.GetSciName();
	m_fMaxHeight = v.GetMaxHeight();

	uint cnames = v.NumCommonNames();
	m_CommonNames.resize(cnames);
	for (i = 0; i < cnames; i++)
		m_CommonNames[i] = v.GetCommonName(i);

	uint apps = v.NumAppearances();
	for (i = 0; i < apps; i++)
	{
		vtPlantAppearance3d *pa3d = new vtPlantAppearance3d(*(v.GetAppearance(i)));
		m_Apps.Append(pa3d);
	}
	return *this;
}

/**
 * Return the species' appearance which is closest to a given height
 */
vtPlantAppearance3d *vtPlantSpecies3d::GetAppearanceByHeight(float fHeight)
{
	uint i, size = m_Apps.GetSize();

	// simple case: if only one appearance, nothing random is possible
	if (size == 1)
		return (vtPlantAppearance3d *) m_Apps[0];

	// find the appearance closest to that height
	float closest_diff = 1E9;
	float closest_value=0;
	for (i = 0; i < size; i++)
	{
		vtPlantAppearance *pa = m_Apps[i];
		float diff = fabsf(pa->m_height - fHeight);
		if (diff < closest_diff)
		{
			closest_diff = diff;
			closest_value = pa->m_height;
		}
	}

	// If there is more than one appearance with the same height,
	//  find them and pick one of them at random.
	std::vector<vtPlantAppearance *> close;
	for (i = 0; i < size; i++)
	{
		vtPlantAppearance *pa = m_Apps[i];
		if (pa->m_height == closest_value)
			close.push_back(pa);
	}
	if (close.size() == 1)
	{
		// simple case
		return (vtPlantAppearance3d *) close[0];
	}
	else
	{
		int which = rand() % close.size();
		return (vtPlantAppearance3d *) close[which];
	}
}

/**
 * Picks a random height, and return the species' appearance
 *  which is closest to that height.
 */
vtPlantAppearance3d *vtPlantSpecies3d::GetRandomAppearance()
{
	// pick a random height
	float height = random(m_fMaxHeight);

	return GetAppearanceByHeight(height);
}

void vtPlantSpecies3d::CheckAvailability()
{
#if VTDEBUG
	VTLOG(" species '%s' checking availability of %d appearances\n",
		(const char *) m_szSciName, NumAppearances());
#endif
	for (uint i = 0; i < NumAppearances(); i++)
		GetAppearance(i)->CheckAvailability();
}

int vtPlantSpecies3d::NumAvailableInstances()
{
	int num = 0;
	for (uint i = 0; i < NumAppearances(); i++)
		if (GetAppearance(i)->IsAvailable())
			num++;
	return num;
}

void vtPlantSpecies3d::AddAppearance(AppearType type, const char *filename,
	float width, float height, float shadow_radius, float shadow_darkness)
{
	vtPlantAppearance3d *pApp = new vtPlantAppearance3d(type, filename,
		width, height, shadow_radius, shadow_darkness);
	m_Apps.Append(pApp);
}


/////////////////////////////////////////////////////////////////////////////
// vtSpeciesList3d
//

vtSpeciesList3d::vtSpeciesList3d()
{
}

// copy operator
vtSpeciesList3d &vtSpeciesList3d::operator=(const vtSpeciesList &v)
{
	int sp = v.NumSpecies();
	for (int i = 0; i < sp; i++)
	{
		vtPlantSpecies *pOld = v.GetSpecies(i);
		vtPlantSpecies3d *pNew = new vtPlantSpecies3d;
		*pNew = *pOld;
		m_Species.Append((vtPlantSpecies *) pNew);
	}
	return *this;
}


vtPlantSpecies3d *vtSpeciesList3d::GetSpecies(uint i) const
{
	if (i >= m_Species.GetSize())
		return NULL;
	return (vtPlantSpecies3d *) m_Species[i];
}


////////////

int vtSpeciesList3d::CheckAvailability()
{
	VTLOG("vtSpeciesList3d checking availability of %d species.\n",
		NumSpecies());

	int num = 0;
	for (uint i = 0; i < NumSpecies(); i++)
	{
		GetSpecies(i)->CheckAvailability();
		num += GetSpecies(i)->NumAvailableInstances();
	}
	return num;
}

/**
 * Create all of the appearances for all the species in this species list.
 */
void vtSpeciesList3d::CreatePlantSurfaces()
{
	for (uint i = 0; i < NumSpecies(); i++)
	{
		vtPlantSpecies3d *pSpecies = GetSpecies(i);
		int iApps = pSpecies->NumAppearances();
		for (int j = 0; j < iApps; j++)
		{
			vtPlantAppearance3d *pApp = pSpecies->GetAppearance(j);
			pApp->LoadAndCreate();
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// vtPlantInstance3d
//

vtPlantInstance3d::vtPlantInstance3d()
{
	m_pContainer = NULL;
	m_pHighlight = NULL;
}

void vtPlantInstance3d::ShowBounds(bool bShow)
{
	if (bShow)
	{
		if (!m_pHighlight)
		{
			// the highlight geometry doesn't exist, so create it
			// get bounding sphere
			osg::Node *contents = m_pContainer->getChild(0);
			if (contents)
			{
				FSphere sphere;
				GetBoundSphere(contents, sphere);

				m_pHighlight = CreateBoundSphereGeode(sphere);
				m_pContainer->addChild(m_pHighlight);
			}
		}
		m_pHighlight->SetEnabled(true);
	}
	else
	{
		if (m_pHighlight)
			m_pHighlight->SetEnabled(false);
	}
}

void vtPlantInstance3d::ReleaseContents()
{
	if (!m_pContainer)	// safety check
		return;

	int ch = m_pContainer->getNumChildren();
	for (int i = 0; i < ch; i++)
	{
		osg::Node *node = m_pContainer->getChild(ch-1-i);
		if (node != m_pHighlight)	// don't delete the highlight
			m_pContainer->removeChild(node);
	}
	m_pContainer->Identity();
}


/////////////////////////////////////////////////////////////////////////////
// Cell used to divide the PlantInstances into a heirarchy of LOD nodes,
// for culling by distance.
//

void PlantCell::computeBound()
{
	_bb.init();
	for(CellList::iterator citr=_cells.begin();
		citr!=_cells.end();
		++citr)
	{
		(*citr)->computeBound();
		_bb.expandBy((*citr)->_bb);
	}

	for(TreeList::iterator titr=_trees.begin();
		titr!=_trees.end();
		++titr)
	{
		_bb.expandBy((*titr).m_pos);
	}
}

bool PlantCell::divide(unsigned int maxNumTreesPerCell)
{
	if (_trees.size() <= maxNumTreesPerCell)
		return false;

	computeBound();

	float radius = _bb.radius();
	float divide_distance = radius*0.7f;
	if (divide((_bb.xMax()-_bb.xMin())>divide_distance,
			   (_bb.yMax()-_bb.yMin())>divide_distance,
			   (_bb.zMax()-_bb.zMin())>divide_distance))
	{
		// recusively divide the new cells till maxNumTreesPerCell is met.
		for(CellList::iterator citr=_cells.begin(); citr!=_cells.end(); ++citr)
		{
			(*citr)->divide(maxNumTreesPerCell);
		}
		return true;
   }
   else
		return false;
}

bool PlantCell::divide(bool xAxis, bool yAxis, bool zAxis)
{
	if (!(xAxis || yAxis || zAxis))
		return false;

	if (_cells.empty())
		_cells.push_back(new PlantCell(_bb));

	if (xAxis)
	{
		unsigned int numCellsToDivide = _cells.size();
		for(unsigned int i=0; i < numCellsToDivide; ++i)
		{
			PlantCell* orig_cell = _cells[i].get();
			PlantCell* new_cell = new PlantCell(orig_cell->_bb);

			float xCenter = (orig_cell->_bb.xMin()+orig_cell->_bb.xMax())*0.5f;
			orig_cell->_bb.xMax() = xCenter;
			new_cell->_bb.xMin() = xCenter;

			_cells.push_back(new_cell);
		}
	}
	if (yAxis)
	{
		unsigned int numCellsToDivide = _cells.size();
		for(unsigned int i=0; i < numCellsToDivide; ++i)
		{
			PlantCell* orig_cell = _cells[i].get();
			PlantCell* new_cell = new PlantCell(orig_cell->_bb);

			float yCenter = (orig_cell->_bb.yMin()+orig_cell->_bb.yMax())*0.5f;
			orig_cell->_bb.yMax() = yCenter;
			new_cell->_bb.yMin() = yCenter;

			_cells.push_back(new_cell);
		}
	}
	if (zAxis)
	{
		unsigned int numCellsToDivide = _cells.size();
		for(unsigned int i=0; i < numCellsToDivide; ++i)
		{
			PlantCell* orig_cell = _cells[i].get();
			PlantCell* new_cell = new PlantCell(orig_cell->_bb);

			float zCenter = (orig_cell->_bb.zMin()+orig_cell->_bb.zMax())*0.5f;
			orig_cell->_bb.zMax() = zCenter;
			new_cell->_bb.zMin() = zCenter;

			_cells.push_back(new_cell);
		}
	}
	bin();
	return true;
}

void PlantCell::bin()
{   
	// put trees in appropriate cells.
	TreeList treesNotAssigned;
	for(TreeList::iterator titr=_trees.begin(); titr!=_trees.end(); ++titr)
	{
		const vtPlantInstanceShader &tree = *titr;
		bool assigned = false;
		for(CellList::iterator citr=_cells.begin();
			citr!=_cells.end() && !assigned;
			++citr)
		{
			if ((*citr)->contains(tree.m_pos))
			{
				(*citr)->addTree(tree);
				assigned = true;
			}
		}
		if (!assigned) treesNotAssigned.push_back(tree);
	}

	// put the unassigned trees back into the original local tree list.
	_trees.swap(treesNotAssigned);


	// prune empty cells.
	CellList cellsNotEmpty;
	for(CellList::iterator citr=_cells.begin(); citr!=_cells.end(); ++citr)
	{
		if (!((*citr)->_trees.empty()))
		{
			cellsNotEmpty.push_back(*citr);
		}
	}
	_cells.swap(cellsNotEmpty);
}

/////////////////////////////////////////////////////////////////////////////
// vtPlantInstanceArray3d
//

vtPlantInstanceArray3d::vtPlantInstanceArray3d()
{
	m_pHeightField = NULL;
	m_pSpeciesList = NULL;
}

vtPlantInstanceArray3d::~vtPlantInstanceArray3d()
{
	int i, num = m_Instances3d.GetSize();
	for (i = 0; i < num; i++)
	{
		vtPlantInstance3d *pi = m_Instances3d[i];
		delete pi;
	}
}

vtPlantInstance3d *vtPlantInstanceArray3d::GetInstance3d(uint i) const
{
	if (i >= m_Instances3d.GetSize())
		return NULL;
	return m_Instances3d[i];
}

int vtPlantInstanceArray3d::CreatePlantNodes(bool progress_dialog(int))
{
	uint i, size = NumEntities();
	int created = 0;
	m_iOffTerrain = 0;

	m_Instances3d.SetSize(size);
	for (i = 0; i < size; i++)
	{
		// Clear value first, in case it doesn't construct.
		m_Instances3d.SetAt(i, NULL);

		if (CreatePlantNode(i))
			created++;

		if (progress_dialog != NULL && ((i%4000)==0))
			progress_dialog(i * 100 / size);
	}
	return created;
}

osg::Geometry *MakeOrthogonalQuads(float w, float h)
{
	// set up the coords
	osg::Vec3Array &v = *(new osg::Vec3Array(8));
	osg::Vec2Array &t = *(new osg::Vec2Array(8));
	
	// This provides a random orientation, but due to the way we're re-using
	// the geometry, it's only random per PlantCell.
	float rotation = random(osg::PI/2.0f);
	float sw = sinf(rotation)*w*0.5f;
	float cw = cosf(rotation)*w*0.5f;

	v[0].set(-sw, 0.0f, -cw);
	v[1].set(+sw, 0.0f,  cw);
	v[2].set(+sw, h,	 cw);
	v[3].set(-sw, h,	-cw);

	v[4].set(-cw, 0.0f,  sw);
	v[5].set(+cw, 0.0f, -sw);
	v[6].set(+cw, h,	-sw);
	v[7].set(-cw, h,	 sw);

	t[0].set(0.0f, 0.0f);
	t[1].set(1.0f, 0.0f);
	t[2].set(1.0f, 1.0f);
	t[3].set(0.0f, 1.0f);
				   
	t[4].set(0.0f, 0.0f);
	t[5].set(1.0f, 0.0f);
	t[6].set(1.0f, 1.0f);
	t[7].set(0.0f, 1.0f);

	osg::Geometry *geom = new osg::Geometry;
	geom->setVertexArray(&v);
	geom->setTexCoordArray(0, &t);
	geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,8));
	return geom;
}

void vtPlantInstanceArray3d::AddShaderGeometryForPlant(PlantCell *cell,
	vtPlantInstanceShader &pi)
{
	vtPlantSpecies3d *ps = GetSpeciesList()->GetSpecies(pi.m_species_id);
	if (!ps)
		return;

	vtPlantAppearance3d *pa = ps->GetAppearanceByHeight(pi.m_size);
	if (!pa)
		return;

	// Do we already have a shader geometry for this species appearance?
	PlantShaderDrawable *psd;
	PlantShaderMap::iterator it = cell->m_ShaderDrawables.find(pa);
	if (it == cell->m_ShaderDrawables.end())
	{
		// Create and add to map
		psd = MakePlantShaderDrawable(cell, pa);
		cell->m_ShaderDrawables[pa] = psd;
	}
	else
		psd = it->second;

	psd->addPlant(osg::Vec4(pi.m_pos.x(), pi.m_pos.y(), pi.m_pos.z(), pi.m_size));
}

PlantShaderDrawable *vtPlantInstanceArray3d::MakePlantShaderDrawable(PlantCell *cell,
	vtPlantAppearance3d *pa)
{
	osg::StateSet *stateset = pa->GetOrCreateShaderStateset();

	// We will scale later by height, so make our quad with height=1.0 with
	// proportional width.
	const float width = pa->m_width / pa->m_height;
	osg::Geometry* two_quads = MakeOrthogonalQuads(width, 1.0f);

	PlantShaderDrawable *shader_drawable = new PlantShaderDrawable;
	shader_drawable->setGeometry(two_quads);

	osg::Geode* geode = new osg::Geode;
	geode->setStateSet(stateset);
	geode->addDrawable(shader_drawable);

	// Add the geode to the InstanceArray's scenegraph, which is later added to
	// the terrain's scenegraph.
	cell->m_group->addChild(geode);
	cell->computeBound();
	cell->m_group->setDistance(cell->_bb.radius());

	return shader_drawable;
}

void LogCellGraph(const PlantCell *cell, int indent)
{
	for (int i = 0; i < indent; i++) VTLOG1(" ");
	VTLOG("Cell %d children %d trees\n", cell->_cells.size(), cell->_trees.size());

	for (uint i = 0; i < cell->_cells.size(); i++)
		LogCellGraph(cell->_cells[i].get(), indent + 1);
}

osg::Node *vtPlantInstanceArray3d::CreateCellNodes(PlantCell *cell)
{
	bool needGroup = !(cell->_cells.empty());
	bool needTrees = !(cell->_trees.empty());

	cell->m_group = new osg::GroupLOD;

	if (needTrees)
	{
		for (TreeList::iterator itr=cell->_trees.begin();
			itr!=cell->_trees.end(); ++itr)
		{
			vtPlantInstanceShader& tree = *itr;
			AddShaderGeometryForPlant(cell, tree);
		}
	}
	else if (needGroup)
	{
		for (PlantCell::CellList::iterator itr=cell->_cells.begin();
			itr!=cell->_cells.end(); ++itr)
		{
			cell->m_group->addChild(CreateCellNodes(itr->get()));
		}
	}

	cell->computeBound();
	cell->m_group->setDistance(cell->_bb.radius());
	cell->m_group->setCenter(cell->_bb.center());

	return cell->m_group;
}

// Divide the cells until there are no more than this many plants in each cell.
const int kMaxPlantsPerCell = 4000;

int vtPlantInstanceArray3d::CreatePlantShaderNodes(bool progress_dialog(int))
{
	VTLOG1(" Creating OpenGL shader based vegetation...\n");

	FPoint3 p3;
	uint num_plants = NumEntities();

	// Create cell subdivision
	osg::ref_ptr<PlantCell> cell = new PlantCell;
	cell->reserveTrees(num_plants);

	vtPlantInstanceShader pi;
	for (uint i = 0; i < num_plants; i++)
	{
		GetPlant(i, pi.m_size, pi.m_species_id);
		m_pHeightField->ConvertEarthToSurfacePoint(GetPoint(i), p3);
		pi.m_pos.set(p3.x, p3.y, p3.z);

		cell->addTree(pi);
	}
	cell->divide(kMaxPlantsPerCell);
#if VTDEBUG
	LogCellGraph(cell.get(), 0);
#endif

	CreateCellNodes(cell.get());

	// Add top node to scene graph
	m_group = new vtGroup;
	m_group->setName("VegGroup");
	m_group->addChild(cell->m_group);

	return NumEntities();
}

bool vtPlantInstanceArray3d::CreatePlantNode(uint i)
{
	// If it was already constructed, destruct so we can build again
	ReleasePlantGeometry(i);

	if (!m_pSpeciesList)
		return false;

	DPoint2 pos = GetPoint(i);
	float size;
	short species_id;
	GetPlant(i, size, species_id);

	if (!m_pHeightField->ContainsEarthPoint(pos))
	{
		m_iOffTerrain++;
		return false;
	}

	vtPlantInstance3d *inst3d = GetInstance3d(i);
	if (!inst3d)
	{
		inst3d = new vtPlantInstance3d;
		m_Instances3d.SetAt(i, inst3d);
	}

	vtPlantSpecies3d *ps = GetSpeciesList()->GetSpecies(species_id);
	if (!ps)
		return false;

	vtPlantAppearance3d *pApp = ps->GetAppearanceByHeight(size);
	if (!pApp)
		return false;

	// confirm that it is loaded and ready to use
	pApp->LoadAndCreate();

	if (!inst3d->m_pContainer)
		inst3d->m_pContainer = new vtTransform;

	pApp->GenerateGeom(inst3d->m_pContainer);

	UpdateTransform(i);

	// We need to scale the model to produce the desired size, not the
	//  size of the appearance but the size of the instance.
	float scale = size / pApp->m_height;
	inst3d->m_pContainer->Scale(scale);

	if (pApp->m_eType == AT_BILLBOARD)
	{
		// Since the billboard are symmetric, a small rotation helps provide
		//  a more natural look.
		float random_rotation = random(PI2f);
		inst3d->m_pContainer->RotateLocal(FPoint3(0,1,0), random_rotation);
	}
	return true;
}

void vtPlantInstanceArray3d::SetEnabled(bool en)
{
	// Currently, the shader plants are all under a single group, so they are
	// easy to control in one place.
	if (m_group.valid())
		m_group->SetEnabled(en);

	// whereas the conventional plants are distributed in an LOD Grid, so we
	// need to control each one independently
	for (uint i = 0; i < m_Instances3d.GetSize(); i++)
	{
		vtPlantInstance3d *inst3d = GetInstance3d(i);
		if (inst3d && inst3d->m_pContainer)
			inst3d->m_pContainer->SetEnabled(en);
	}
}

bool vtPlantInstanceArray3d::GetEnabled() const
{
	if (m_group.valid())
		return m_group->GetEnabled();

	if (m_Instances3d.GetSize() > 0)
	{
		vtPlantInstance3d *inst3d = GetInstance3d(0);
		if (inst3d && inst3d->m_pContainer)
			return inst3d->m_pContainer->GetEnabled();
	}
	return false;
}

void vtPlantInstanceArray3d::ReleasePlantGeometry(uint i)
{
	vtPlantInstance3d *inst3d = GetInstance3d(i);
	if (inst3d)
		inst3d->ReleaseContents();
}

vtTransform *vtPlantInstanceArray3d::GetPlantNode(uint i) const
{
	if (i >= m_Instances3d.GetSize())
		return NULL;

	vtPlantInstance3d *inst3d = GetInstance3d(i);
	if (inst3d)
		return inst3d->m_pContainer;
	return NULL;
}

void vtPlantInstanceArray3d::VisualDeselectAll()
{
	uint size = NumEntities();

	for (uint i = 0; i < size; i++)
	{
		vtPlantInstance3d *inst3d = GetInstance3d(i);
		if (inst3d)
		{
			Select(i, false);
			inst3d->ShowBounds(false);
		}
	}
}

void vtPlantInstanceArray3d::VisualSelect(uint i)
{
	vtPlantInstance3d *inst3d = GetInstance3d(i);
	if (inst3d && inst3d->m_pContainer != NULL)
	{
		Select(i, true);
		inst3d->ShowBounds(true);
	}
}

void vtPlantInstanceArray3d::OffsetSelectedPlants(const DPoint2 &offset)
{
	uint size = NumEntities();
	for (uint i = 0; i < size; i++)
	{
		if (!IsSelected(i))
			continue;

		SetPoint(i, GetPoint(i) + offset);
		UpdateTransform(i);
	}
}

void vtPlantInstanceArray3d::UpdateTransform(uint i)
{
	vtPlantInstance3d *inst3d = GetInstance3d(i);

	FPoint3 p3;
	m_pHeightField->ConvertEarthToSurfacePoint(GetPoint(i), p3);

	// We should really move the plant to a new cell in the LOD
	// Grid, but unless it's moving really far we don't need to
	// worry about this.

	inst3d->m_pContainer->SetTrans(p3);
}

//
// Note you must remove the plant from the scene graph before deleting it!
//
void vtPlantInstanceArray3d::DeletePlant(uint i)
{
	vtPlantInstance3d *inst3d = GetInstance3d(i);

	// Get rid of the feature from the FeatureSet
	SetToDelete(i);
	ApplyDeletion();

	// and its 3D component
	m_Instances3d.RemoveAt(i);
	delete inst3d;
}

bool vtPlantInstanceArray3d::FindPlantFromNode(osg::Node *pNode, int &iOffset)
{
	bool bFound = false;
	uint i, j;

	for (i = 0; (i < m_Instances3d.GetSize()) & !bFound; i++)
	{
		vtTransform *pTransform = GetPlantNode(i);
		if (!pTransform)	// safety check
			continue;
		for (j = 0; (j < pTransform->getNumChildren()) && !bFound; j++)
		{
			osg::Node *pPlantNode = pTransform->getChild(j);
			if (pPlantNode != GetInstance3d(i)->m_pHighlight)
			{
				if (pPlantNode == pNode)
				{
					bFound = true;
					iOffset = i;
				}
			}
		}
	}
	return bFound;
}

