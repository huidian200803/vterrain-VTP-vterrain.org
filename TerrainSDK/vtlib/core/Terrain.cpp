//
// Terrain.cpp
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/vtosg/GroupLOD.h"
#include "vtlib/vtosg/MultiTexture.h"

#include "vtdata/vtLog.h"
#include "vtdata/CubicSpline.h"
#include "vtdata/DataPath.h"

#include "Terrain.h"

#include "Building3d.h"
#include "Fence3d.h"
#include "ImageSprite.h"
#include "Light.h"
#include "PagedLodGrid.h"
#include "vtTin3d.h"

#include "SMTerrain.h"
#include "BruteTerrain.h"
#include "SRTerrain.h"
#include "TiledGeom.h"
#include "vtlib/vtosg/ExternalHeightField3d.h"
// add your own terrain method header here!

// The Terrain uses two LOD grids (class vtLodGrid, a sparse grid of LOD cells)
//  of size LOD_GRIDSIZE x LOD_GRIDSIZE to group structures and vegetation.
//  This allows them to be culled more efficiently.
#define LOD_GRIDSIZE		128


//////////////////////////////////////////////////////////////////////

vtTerrain::vtTerrain()
{
	m_bIsCreated = false;

	m_ocean_color.Set(40.0f/255, 75.0f/255, 124.0f/255);	// unshaded color
	m_fog_color.Set(1.0f, 1.0f, 1.0f);

	m_pContainerGroup = NULL;
	m_pTerrainGroup = NULL;
	m_pUnshadowedGroup = NULL;
	m_iShadowTextureUnit = -1;
	m_pFog = NULL;
	m_pShadow = NULL;
	m_bFog = false;
	m_bShadows = false;

	m_pEphemMats = new vtMaterialArray;
	m_idx_water = -1;

	m_pHeightField = NULL;
	m_bPreserveInputGrid = false;
	m_pScaledFeatures = NULL;
	m_pFeatureLoader = NULL;

	m_pOceanGeom = NULL;
	m_pRoadGroup = NULL;

	// vegetation
	m_pVegGroup = NULL;
	m_pVegGrid = NULL;
	m_pSpeciesList = NULL;

	m_pDynGeom = NULL;
	m_pDynGeomScale = NULL;
	m_pTiledGeom = NULL;
	m_pPagedStructGrid = NULL;

	m_pActiveLayer = NULL;

	// structures
	m_pStructGrid = NULL;

	m_CamLocation.Identity();
	m_bVisited = false;

	m_CenterGeoLocation.Set(-999, -999);	// initially unknown

	m_pOverlay = NULL;
	m_progress_callback = NULL;

	m_pExternalHeightField = NULL;
	m_bTextureCompression = false;
}

vtTerrain::~vtTerrain()
{
	VTLOG("Terrain destructing: '%s' ..", (const char *) GetName());

	// Remove/release the things this terrain has added to the scene.
	m_Content.ReleaseContents();
	m_Content.Clear();

	m_AnimContainer.clear();

	m_Layers.clear();

	// Do not delete the SpeciesList, the application may be sharing the same
	// list with several different terrains.

	if (m_bPreserveInputGrid)
		m_pElevGrid.release();

	if (m_pRoadGroup)
		m_pTerrainGroup->removeChild(m_pRoadGroup);

	if (m_pOceanGeom)
		m_pTerrainGroup->removeChild(m_pOceanGeom);

	if (m_pStructGrid)
		m_pTerrainGroup->removeChild(m_pStructGrid);

	if (m_pVegGroup)
		m_pTerrainGroup->removeChild(m_pVegGroup);

	if (m_pVegGrid)
		m_pTerrainGroup->removeChild(m_pVegGrid);

	if (m_pDynGeom)
		m_pDynGeomScale->removeChild(m_pDynGeom);

	if (m_pDynGeomScale)
		m_pTerrainGroup->removeChild(m_pDynGeomScale);

	if (m_pTiledGeom)
		m_pTerrainGroup->removeChild(m_pTiledGeom);

	// Release anything remaining in the terrain's scenegraph
	m_pContainerGroup = NULL;

	VTLOG1(" done.\n");
}


///////////////////////////////////////////////////////////////////////

/**
 * Tells the terrain what file contains the parameters to use.
 *
 * \param fname The name of a terrain parameters file, e.g. "Simple.xml".
 */
void vtTerrain::SetParamFile(const char *fname)
{
	m_strParamFile = fname;
}

bool vtTerrain::LoadParams()
{
	TParams params;
	bool success = params.LoadFrom(m_strParamFile);
	if (success)
		SetParams(params);
	return success;
}

/**
 * Set all of the parameters for this terrain.
 *
 * \param Params An object which contains all the parameters for the terrain.
 *
 * \par Note that you can set individual parameters like this:
\code
	TParams &par = pTerrain->GetParams();
	par.SetBoolValue(STR_SKY, false);
\endcode
 */
void vtTerrain::SetParams(const TParams &Params)
{
	m_Params = Params;

	RGBi color;
	if (m_Params.GetValueRGBi(STR_FOGCOLOR, color))
	{
		if (color.r != -1)
			m_fog_color = color;
	}
	if (m_Params.GetValueRGBi(STR_BGCOLOR, color))
	{
		if (color.r != -1)
			m_background_color = color;
	}
}

/**
 * Returns a direct reference to the parameters object for this terrain, so
 * that you can get and set the parameters.
 *
 * \par Example:
\code
	TParams &par = pTerrain->GetParams();
	par.SetBoolValue(STR_SKY, false);
\endcode
 */
TParams &vtTerrain::GetParams()
{
	return m_Params;
}

/**
 * This method allows you to give the terrain a grid to use directly
 * instead of loading the BT file specified in the TParams.
 *
 * You must allocate this grid dynamically with 'new', since vtTerrain
 * will 'delete' it after using it during initialization.  If you don't
 * want the memory to be deleted, pass 'true' for bPreserve.
 *
 * \param pGrid The grid object which the terrain should use.
 * \param bPreserve True if the terrain should not delete the grid object,
 * otherwise false.
 *
 */
void vtTerrain::SetLocalGrid(vtElevationGrid *pGrid, bool bPreserve)
{
	m_pElevGrid.reset(pGrid);
	m_bPreserveInputGrid = bPreserve;
}

/**
 * This method allows you to give the terrain a TIN to use directly
 * instead of loading a .tin file as specified in the TParams.
 */
void vtTerrain::SetTin(vtTin3d *pTin)
{
	m_pTin = pTin;
}


///////////////////////////////////////////////////////////////////////

void vtTerrain::_CreateRoads()
{
	// for GetValueFloat below
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	vtString road_fname = "RoadData/";
	road_fname += m_Params.GetValueString(STR_ROADFILE);
	vtString road_path = FindFileOnPaths(vtGetDataPath(), road_fname);
	if (road_path == "")
		return;

	VTLOG("Creating Roads: ");
	m_pRoadMap = new vtRoadMap3d;

	VTLOG("  Reading from file '%s'\n", (const char *) road_path);
	bool success = m_pRoadMap->ReadRMF(road_path);
	if (!success)
	{
		VTLOG("	read failed.\n");
		m_pRoadMap = NULL;
		return;
	}

	// Sanity checks: The roads might be off our terrain completely, either
	//  from mismatched data or perhaps a wrong CRS.
	const DRECT &terrain_extents = GetHeightField()->GetEarthExtents();
	const DPoint2 roads_center = m_pRoadMap->GetMapExtent().GetCenter();
	if (!terrain_extents.ContainsPoint(roads_center))
	{
		VTLOG("Roadmap extent center (%lf, %lf) is not on the terrain.\n",
			roads_center.x, roads_center.y);
		return;
	}

	//some nodes may not have any roads attached to them.  delete them.
	m_pRoadMap->RemoveUnusedNodes();

	m_pRoadMap->DetermineSurfaceAppearance();

	m_pRoadMap->SetHeightOffGround(m_Params.GetValueFloat(STR_ROADHEIGHT));
	m_pRoadMap->DrapeOnTerrain(m_pHeightField);
	m_pRoadMap->ComputeIntersectionVertices();

	m_pRoadMap->SetLodDistance(m_Params.GetValueFloat(STR_ROADDISTANCE) * 1000);	// convert km to m

	bool bDoTexture = m_Params.GetValueBool(STR_TEXROADS);
	m_pRoadGroup = m_pRoadMap->GenerateGeometry(bDoTexture,
		m_Params.GetValueBool(STR_HWY),
		m_Params.GetValueBool(STR_PAVED),
		m_Params.GetValueBool(STR_DIRT),
		m_progress_callback);
	m_pRoadGroup->SetCastShadow(false);
	m_pTerrainGroup->addChild(m_pRoadGroup);

	if (m_Params.GetValueBool(STR_ROADCULTURE))
		m_pRoadMap->GenerateSigns(m_pStructGrid);
}


/**
 * Set the array of colors to be used when automatically generating the
 * terrain texture from the elevation values.  This is the color map which
 * is used for automatic generation of texture from elevation, when the
 * terrain is built normally with the "Derived" texture option.
 * The colors brackets go from the lowest elevation value to the highest.
 *
 * \param colors A pointer to a colormap.  The terrain takes ownership of
 *		the ColorMap object so it will be deleted when the terrain is deleted.
 *
 * \par Example:
	\code
	ColorMap *colors = new ColorMap;
	colors->m_bRelative = false;
	colors->Add(100, RGBi(0,255,0));
	colors->Add(200, RGBi(255,200,150));
	pTerr->SetTextureColorMap(colors);
	\endcode
 */
void vtTerrain::SetTextureColorMap(ColorMap *colors)
{
	m_Texture.m_pColorMap = colors;
}

/**
 * This method sets the terrain's color map to a series of white and black
 * bands which indicate elevation contour lines.  This is the color map
 * which is used for automatic generation of texture from elevation, when
 * the terrain is built normally with the "Derived" texture option.
 *
 * You can use this function either before the terrain is built, or
 * afterwards if you intend to re-build the textures.
 *
 * \par Example:
	\code
	vtTerrain *pTerr = new vtTerrain;
	pTerr->SetTextureContours(100, 4);
	\endcode
 *
 * \param fInterval  The vertical spacing between the contours.  For example,
 *		if the elevation range of your data is from 50 to 350 meters, then
 *		an fIterval of 100 will place contour bands at 100,200,300 meters.
 * \param fSize  The vertical thickness of each contour band, generally a
 *		few meters.  A band of this thickness will be centered on each contour
 *		line of the desired elevation.
 */
void vtTerrain::SetTextureContours(float fInterval, float fSize)
{
	// Create a color map and fill it with contour strip bands
	ColorMap *cmap = new ColorMap;
	cmap->m_bBlend = true;
	cmap->m_bRelative = false;

	RGBi white(255,255,255);
	RGBi black(0,0,0);

	float fMin, fMax;
	m_pHeightField->GetHeightExtents(fMin, fMax);
	int start = (int) (fMin / fInterval);
	int stop = (int) (fMax / fInterval);

	cmap->Add(fMin, white);
	for (int i = start; i < stop; i++)
	{
		// create a black stripe of the desired vertical thickness
		cmap->Add(i * fInterval - fSize*0.8f, white);
		cmap->Add(i * fInterval - fSize*0.5f, black);
		cmap->Add(i * fInterval + fSize*0.5f, black);
		cmap->Add(i * fInterval + fSize*0.8f, white);
	}

	// Set these as the desired color bands for the next PainDib
	m_Texture.m_pColorMap = cmap;
}

/**
	Re-shade the ground texture.  This is useful if you have changed the
	time of day, and want to see the lighting/shading of the terrain updated.
 */
void vtTerrain::ReshadeTexture(vtTransform *pSunLight, bool progress_callback(int))
{
	m_Texture.CopyFromUnshaded(m_Params);
	m_Texture.ShadeTexture(m_Params, GetHeightFieldGrid3d(), pSunLight->GetDirection(), progress_callback);

	// Make sure OSG knows that the texture has changed
	vtMaterial *mat = m_Texture.m_pMaterials->at(0);
	mat->SetTexture2D(m_Texture.m_pTextureImage);
	mat->ModifiedTexture();
}

/**
	Re-create the ground texture.  It is completely made again (reloaded from disk,
	or regenerated from a colormap).
 */
void vtTerrain::RecreateTexture(vtTransform *pSunLight, bool progress_callback(int))
{
	m_Texture.MakeTexture(m_Params, GetHeightFieldGrid3d(),
		m_bTextureCompression, m_progress_callback);

	m_Texture.ShadeTexture(m_Params, GetHeightFieldGrid3d(), pSunLight->GetDirection(),
		m_progress_callback);

	// Make sure OSG knows that the texture has changed
	vtMaterial *mat = m_Texture.m_pMaterials->at(0);
	mat->SetTexture2D(m_Texture.m_pTextureImage);
	mat->ModifiedTexture();
}

/**
 * Get the texture image of the ground texture, if there is one.  If the
 * texture is more complicated (e.g. tileset) then NULL is returned.
 */
osg::Image *vtTerrain::GetTextureImage()
{
	return m_Texture.m_pTextureImage.get();
}


/////////////////////

bool vtTerrain::_CreateDynamicTerrain()
{
	LodMethodEnum method = m_Params.GetLodMethod();
	VTLOG(" LOD method %d\n", method);

	if (method == LM_TOPOVISTA)
	{
		// Obsolete, removed.
	}
	else if (method == LM_MCNALLY)
	{
		m_pDynGeom = new SMTerrain;
		m_pDynGeom->setName("Seumas Geom");
	}
	else if (method == LM_DEMETER)
	{
		// Obsolete, removed.
	}
	else if (method == LM_BRUTE)
	{
		m_pDynGeom = new BruteTerrain;
		m_pDynGeom->setName("Brute-force Terrain Geom");
	}
	else if (method == LM_ROETTGER)
	{
		m_pDynGeom = new SRTerrain;
		m_pDynGeom->setName("Roettger Geom");
	}
	// else if (method == LM_YOURMETHOD)
	// {
	//	add your own LOD method here!
	// }
	if (!m_pDynGeom)
	{
		_SetErrorMessage("Unknown LOD method.");
		return false;
	}

	DTErr result = m_pDynGeom->Init(m_pElevGrid.get(), m_fVerticalExag);
	if (result != DTErr_OK)
	{
		m_pDynGeom = NULL;

		_CreateErrorMessage(result, m_pElevGrid.get());
		VTLOG(" Could not initialize CLOD: %s\n", (const char *) m_strErrorMsg);
		return false;
	}

	//
	// This is a hack to allow a transparent terrain surface.
	//  In order for OSG to draw the transparent surface correctly, it needs
	//  to know it's in the the "TRANSPARENT" render bin.  But because
	//  our surface draws itself, OSG doesn't know at binning time that
	//  it's transparent.  So, we have to tell it ahead of time.
	//
	if (m_Params.GetTextureEnum() == TE_SINGLE)
	{
		vtMaterial *mat = m_Texture.m_pMaterials->at(0);
		if (mat->GetTransparent())
		{
			osg::StateSet *sset = m_pDynGeom->getOrCreateStateSet();
			sset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		}
	}

	m_pDynGeom->SetPolygonTarget(m_Params.GetValueInt(STR_TRICOUNT));
	m_pDynGeom->SetMaterials(m_Texture.m_pMaterials);

	// build heirarchy (add terrain to scene graph)
	m_pDynGeomScale = new vtTransform;
	m_pDynGeomScale->SetCastShadow(false);
	m_pDynGeomScale->setName("Dynamic Geometry Scaling Container");

	FPoint2 spacing = m_pElevGrid->GetWorldSpacing();
	m_pDynGeomScale->Scale(spacing.x, m_fVerticalExag, -spacing.y);

	m_pDynGeomScale->addChild(m_pDynGeom);
	m_pTerrainGroup->addChild(m_pDynGeomScale);

	// the Dynamic terrain will be the heightfield used at runtime, so extend
	//  it with the terrain's culture
	m_pDynGeom->SetCulture(this);

	return true;
}

void vtTerrain::SetVerticalExag(float fExag)
{
	m_fVerticalExag = fExag;

	if (m_pDynGeom != NULL)
	{
		FPoint2 spacing = m_pDynGeom->GetWorldSpacing();
		m_pDynGeomScale->Identity();
		m_pDynGeomScale->Scale(spacing.x, m_fVerticalExag, -spacing.y);

		m_pDynGeom->SetVerticalExag(m_fVerticalExag);
	}
	else if (m_pTiledGeom != NULL)
	{
		m_pTiledGeom->SetVerticalExag(m_fVerticalExag);
	}
	if (m_pScaledFeatures != NULL)
	{
		m_pScaledFeatures->Identity();
		m_pScaledFeatures->Scale(1.0f, m_fVerticalExag, 1.0f);
	}
}

void vtTerrain::_CreateErrorMessage(DTErr error, vtElevationGrid *pGrid)
{
	const IPoint2 &size = pGrid->GetDimensions();
	switch (error)
	{
	case DTErr_OK:
		m_strErrorMsg = "No Error";
		break;
	case DTErr_EMPTY_EXTENTS:
		m_strErrorMsg.Format("The elevation has empty extents.");
		break;
	case DTErr_NOTSQUARE:
		m_strErrorMsg.Format("The elevation grid (%d x %d) is not square.", size.x, size.y);
		break;
	case DTErr_NOTPOWER2:
		m_strErrorMsg.Format("The elevation grid (%d x %d) is of an unsupported size.",
			size.x, size.y);
		break;
	case DTErr_NOMEM:
		m_strErrorMsg = "Not enough memory for CLOD.";
		break;
	default:
		m_strErrorMsg = "Unknown error.";
	}
}

void vtTerrain::_SetErrorMessage(const vtString &msg)
{
	m_strErrorMsg = msg;
	VTLOG("\t%s.\n", (const char *) msg);
}


/////////////////////////////////////////////////////////////////////////////
// Utilities
//
vtPole3d *vtTerrain::NewPole()
{
	return (vtPole3d *) m_UtilityMap.AddNewPole();
}

vtLine3d *vtTerrain::NewLine()
{
	return m_UtilityMap.AddLine();
}

void vtTerrain::AddPoleToLine(vtLine3d *line, const DPoint2 &epos,
	const char *structname)
{
	VTLOG("AddPoleToLine %.1lf %.1lf\n", epos.x, epos.y);

	vtPole3d *pole = m_UtilityMap.AddPole(epos, structname);
	line->AddPole(pole);

	line->ComputePoleRotations();
	m_UtilityMap.BuildGeometry(m_pStructGrid, m_pHeightField);
}

void vtTerrain::RebuildUtilityGeometry()
{
	m_UtilityMap.ComputePoleRotations();
	m_UtilityMap.BuildGeometry(m_pStructGrid, m_pHeightField);
}

/**
 * Create a horizontal water plane at sea level.  It can be moved up and down
 * with a transform.
 */
void vtTerrain::CreateWaterPlane()
{
	// Unless it's already made.
	if (m_pOceanGeom)
		return;

	MakeWaterMaterial();

	FRECT world_extents = m_pHeightField->m_WorldExtents;
	FPoint2 world_size(world_extents.Width(), world_extents.Height());

	// You can adjust these factors:
	const int STEPS = 5;
	const int TILING = 1;
	const float fUVTiling = 5.0f;

	vtGeode *geode = new vtGeode;
	geode->SetMaterials(m_pEphemMats);

	FPoint2 tile_size = world_size;
	for (int i = -STEPS; i < (STEPS+1); i++)
	{
		for (int j = -(STEPS); j < (STEPS+1); j++)
		{
			FPoint2 base;
			base.x = world_extents.left + (i * tile_size.x);
			base.y = world_extents.top - ((j+1) * tile_size.y);

			vtMesh *mesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_STRIP,
				VT_Normals | VT_TexCoords, 4);
			mesh->CreateRectangle(1, 1, 0, 2, 1, base, base+tile_size,
				0.0, fUVTiling);

			geode->AddMesh(mesh, m_idx_water);
		}
	}
	m_pOceanGeom = new vtMovGeode(geode);
	m_pOceanGeom->setName("Ocean plane");
	m_pOceanGeom->SetCastShadow(false);
	m_pTerrainGroup->addChild(m_pOceanGeom);
}

void vtTerrain::SetWaterLevel(float fElev)
{
	m_Params.SetValueFloat(STR_OCEANPLANELEVEL, fElev);
	if (m_pOceanGeom)
		m_pOceanGeom->SetTrans(FPoint3(0, fElev, 0));
}

void vtTerrain::MakeWaterMaterial()
{
	if (m_idx_water == -1)
	{
		// Water material: texture waves
		vtString fname = FindFileOnPaths(vtGetDataPath(), "GeoTypical/ocean1_256.jpg");
		osg::Image *image = LoadOsgImage(fname);
		m_idx_water = m_pEphemMats->AddTextureMaterial(image,
			false, false,		// culling, lighting
			false,				// the texture itself has no alpha
			false,				// additive
			TERRAIN_AMBIENT,	// ambient
			1.0f,				// diffuse
			0.5,				// alpha
			TERRAIN_EMISSIVE);	// emissive
		// Don't mipmap: allowing texture aliasing to occur, it actually looks
		// more water-like.
		if (m_idx_water != -1)
			m_pEphemMats->at(m_idx_water)->SetClamp(false);
	}
}

void vtTerrain::CreateWaterHeightfield(const vtString &fname)
{
	// add water surface to scene graph
	m_pWaterTin3d = new vtTin3d;
	bool success = m_pWaterTin3d->Read(fname);
	if (!success)
	{
		VTLOG("Couldn't read  water file: '%s'\n", (const char *) fname);
		return;
	}

	MakeWaterMaterial();
	m_pWaterTin3d->SetMaterial(m_pEphemMats, m_idx_water);
	vtGeode *wsgeom = m_pWaterTin3d->CreateGeometry(false);	// No shadow mesh.
	wsgeom->setName("Water surface");
	wsgeom->SetCastShadow(false);

	// We require that the TIN has a compatible CRS with the base
	//  terrain, but the extents may be different.  If they are,
	//  we may need to offset the TIN to be in the correct place.
	DRECT ext1 = m_pHeightField->GetEarthExtents();
	DRECT ext2 = m_pWaterTin3d->GetEarthExtents();
	DPoint2 offset = ext2.LowerLeft() - ext1.LowerLeft();
	float x, z;
	GetLocalCS().VectorEarthToLocal(offset, x, z);
	vtTransform *xform = new vtTransform;
	xform->Translate(FPoint3(x, 0, z));

	xform->addChild(wsgeom);
	m_pTerrainGroup->addChild(xform);
}

/**
 * For this terrain, look at its elevation source, and determine the extents
 * of that data, in geographic coords.  This is done without actually doing
 * a full load of the data, to quickly describe the terrain's location on
 * the earth.
 */
bool vtTerrain::GetGeoExtentsFromMetadata()
{
	vtString name = "Elevation/";
	name += m_Params.GetValueString(STR_ELEVFILE);
	vtString fname = FindFileOnPaths(vtGetDataPath(), name);
	if (fname == "")
	{
		VTLOG("\t'%s' not found on data paths.\n", (const char *)name);
		return false;
	}
	bool success;
	int type = m_Params.GetValueInt(STR_SURFACE_TYPE);
	if (type == 0)	// grid
	{
		vtElevationGrid grid;
		success = grid.LoadBTHeader(fname);
		if (!success)
		{
			VTLOG("\tCouldn't load BT header.\n");
			return false;
		}
		success = grid.GetCorners(m_Corners_geo, true);	// true=Geo
		if (!success)
		{
			VTLOG("\tCouldn't get terrain corners.\n");
			return false;
		}
	}
	else if (type == 1)	// tin
	{
		return false;	// TODO
	}
	else if (type == 2)	// tileset
	{
		TiledDatasetDescription set;
		success = set.Read(fname);
		if (!success)
		{
			VTLOG("\tCouldn't load Tileset description.\n");
			return false;
		}
		success = set.GetCorners(m_Corners_geo, true);	// true=Geo
		if (!success)
		{
			VTLOG("\tCouldn't get terrain corners.\n");
			return false;
		}
	}
	else if (type == 3)	// External
	{
		return false;	// TODO
	}
	return true;
}


///////////////////////////////////////////////
// Built Structures

void vtTerrain::CreateStructures(vtStructureArray3d *structures)
{
	int num_structs = structures->size();
	VTLOG("CreateStructures, %d structs\n", num_structs);

	bool bPaging = m_Params.GetValueBool(STR_STRUCTURE_PAGING);
	if (bPaging)
	{
		// Don't construct geometry, just add to the paged structure grid
		VTLOG("\tAppending %d structures to the paged grid.\n", num_structs);
		for (int i = 0; i < num_structs; i++)
		{
			m_pPagedStructGrid->AppendToGrid(structures, i);
		}
	}
	else
	{
		int suceeded = 0;
		for (int i = 0; i < num_structs; i++)
		{
			bool bSuccess = CreateStructure(structures, i);
			if (bSuccess)
				suceeded++;
			if (m_progress_callback != NULL)
				m_progress_callback(i * 100 / num_structs);
		}
		VTLOG("\tSuccessfully created and added %d of %d structures.\n",
			suceeded, num_structs);
	}
}

bool vtTerrain::CreateStructure(vtStructureArray3d *structures, int index)
{
	vtStructure *str = structures->at(index);
	vtStructure3d *str3d = structures->GetStructure3d(index);

	// Construct
	bool bSuccess = structures->ConstructStructure(str3d);
	if (!bSuccess)
	{
		VTLOG("\tFailed to create stucture %d\n", index);
		return false;
	}

	bSuccess = false;
	vtTransform *pTrans = str3d->GetContainer();
	if (pTrans)
		bSuccess = AddNodeToStructGrid(pTrans);
	else
	{
		vtGeode *pGeode = str3d->GetGeom();
		if (pGeode)
			bSuccess = AddNodeToStructGrid(pGeode);
	}
	if (!bSuccess)
	{
		VTLOG("\tWarning: Structure %d apparently not within bounds of terrain grid.\n", index);
		structures->DestroyStructure(index);
	}
	else
	{
		bool bShow;
		// If the structure has shadow state stored, use it
		if (!str->GetValueBool("shadow", bShow))
			// otherwise use the app default
			bShow = m_Params.GetValueBool(STR_SHADOWS_DEFAULT_ON);
		str3d->SetCastShadow(bShow);
	}

	OnCreateBehavior(str);

	return bSuccess;
}

/**
 * Get the currently active structure layer for this terrain.
 */
vtStructureLayer *vtTerrain::GetStructureLayer() const
{
	return dynamic_cast<vtStructureLayer *>(m_pActiveLayer);
}

/**
 * Create a new structure array for this terrain, and return it.
 */
vtStructureLayer *vtTerrain::NewStructureLayer()
{
	vtStructureLayer *slay = new vtStructureLayer;

	// These structures will use the heightfield and CRS of this terrain
	slay->SetTerrain(this);
	slay->m_crs = m_crs;

	m_Layers.push_back(slay);
	return slay;
}

/**
 * Delete all the selected structures in the terrain's active structure array.
 *
 * \return the number of structures deleted.
 */
int vtTerrain::DeleteSelectedStructures(vtStructureLayer *st_layer)
{
	// first remove them from the terrain
	for (uint i = 0; i < st_layer->size(); i++)
	{
		if (st_layer->at(i)->IsSelected())
			DeleteStructureFromTerrain(st_layer, i);
	}
	// then do a normal delete-selected
	return st_layer->DeleteSelected();
}

void vtTerrain::DeleteStructureFromTerrain(vtStructureLayer *st_layer, int index)
{
	// notify any structure-handling extension
	vtStructure *str = st_layer->at(index);
	OnDeleteBehavior(str);

	// Remove it from the paging grid
	if (m_pPagedStructGrid)
		m_pPagedStructGrid->RemoveFromGrid(st_layer, index);

	vtStructure3d *str3d = st_layer->GetStructure3d(index);
	osg::Node *node = str3d->GetContainer();
	if (!node)
		node = str3d->GetGeom();

	RemoveNodeFromStructGrid(node);
	str3d->DeleteNode();

	// if there are any engines pointing to this node, inform them
	vtGetScene()->TargetRemoved(node);
}

bool vtTerrain::FindClosestStructure(const DPoint2 &point, double epsilon,
	int &structure, vtStructureLayer **st_layer, double &closest,
	float fMaxInstRadius, float fLinearWidthBuffer)
{
	structure = -1;
	closest = 1E8;

	// if all structures are hidden, don't find them
	if (!GetFeatureVisible(TFT_STRUCTURES))
		return false;

	double dist;
	int index;
	for (uint i = 0; i < m_Layers.size(); i++)
	{
		vtStructureLayer *slay = dynamic_cast<vtStructureLayer*>(m_Layers[i].get());
		if (!slay)
			continue;
		if (!slay->GetEnabled())
			continue;
		if (slay->FindClosestStructure(point, epsilon, index, dist,
			fMaxInstRadius, fLinearWidthBuffer))
		{
			if (dist < closest)
			{
				*st_layer = slay;
				structure = index;
				closest = dist;
			}
		}
	}
	return (structure != -1);
}

void vtTerrain::DeselectAllStructures()
{
	for (size_t i = 0; i < m_Layers.size(); i++)
	{
		vtStructureArray3d *st_layer = dynamic_cast<vtStructureArray3d *>(m_Layers[i].get());
		if (st_layer)
			st_layer->VisualDeselectAll();
	}
}

void vtTerrain::WriteStructuresToOBJ(const char *filename)
{
	vtStructureLayer *slay = vtTerrain::GetStructureLayer();
	if (!slay)
		return;
	slay->WriteStructuresToOBJ(filename);
}

/**
 * Loads an external 3D model as a movable node.  The file will be looked for
 * on the Terrain's data path, and wrapped with a vtTransform so that it can
 * be moved.
 *
 * To add the model to the Terrain's scene graph, use <b>AddModel</b> or
 * <b>AddModelToLodGrid</b>.  To plant the model on the terrain, use
 * <b>PlantModel</b> or <b>PlantModelAtPoint</b>.
 *
 * You should also make sure that your model is displayed at the correct
 * scale.  If the units are of the model are not meters, you should scale
 * the correct factor so that it matches the units of the Terrain:
 *
 * \par Example:
	\code
MyTerrain::CreateCustomCulture()
{
	// model is in centimeters (cm)
	vtTransform *pFountain = LoadModel("Culture/fountain.3ds");

	pFountain->Scale(.01f);

	PlantModelAtPoint(pFountain, DPoint2(217690, 4123475));
	AddModel(pFountain);
}
	\endcode
 */
vtTransform *vtTerrain::LoadModel(const char *filename, bool bAllowCache)
{
	vtString path = FindFileOnPaths(vtGetDataPath(), filename);
	if (path == "")
	{
		VTLOG("Couldn't locate file '%s'\n", filename);
		return NULL;
	}
	NodePtr node = vtLoadModel(path, bAllowCache);
	if (node.valid())
	{
		vtTransform *trans = new vtTransform;
		trans->addChild(node);
		return trans;
	}
	return NULL;
}


/**
 * "Plants" a model on the ground.  This is done by moving the model directly
 * up or down such that its local origin is at the height of the terrain.
 *
 * Note: this function does not add the model to the terrain's scene
 * graph.  Use <b>AddNode</b> for that operation.
 */
void vtTerrain::PlantModel(vtTransform *model)
{
	FPoint3 pos = model->GetTrans();
	m_pHeightField->FindAltitudeAtPoint(pos, pos.y);
	model->SetTrans(pos);
}


/**
 * "Plants" a model on the ground.  This is done by moving the node to the
 * indicated earth coordinate, then moving it directly up or down such that
 * its local origin is at the height of the terrain.
 *
 * \param model The model to be placed on the terrain.
 * \param pos The position (in earth coordinates) at which to place it.
 * This position is assumed to be in the same coordinate system that
 * the Terrain is using.
 *
 * Note: this function does not add the model to the terrain's scene
 * graph.  Use <b>AddNode</b> for that operation.
 */
void vtTerrain::PlantModelAtPoint(vtTransform *model, const DPoint2 &pos)
{
	FPoint3 wpos;

	m_pHeightField->m_LocalCS.EarthToLocal(pos.x, pos.y, wpos.x, wpos.z);
	m_pHeightField->FindAltitudeAtPoint(wpos, wpos.y);
	model->SetTrans(wpos);
}

void vtTerrain::_CreateOtherCulture()
{
	m_pTerrainGroup->addChild(m_UtilityMap.Setup());

	// create utility structures (routes = towers and wires)
	vtString util_file = m_Params.GetValueString(STR_UTILITY_FILE);
	if (util_file != "")
	{
		if (m_UtilityMap.ReadOSM(util_file))
		{
			m_UtilityMap.TransformTo(m_crs);
		}
	}

	// create any utility geometry
	m_UtilityMap.ComputePoleStructures();
	m_UtilityMap.ComputePoleRotations();
	m_UtilityMap.BuildGeometry(m_pStructGrid, m_pHeightField);

	// create HUD overlay geometry
	vtString fname;
	int x, y;
	if (m_Params.GetOverlay(fname, x, y))
	{
		vtImageSprite *pSprite = new vtImageSprite;
		vtString path = FindFileOnPaths(vtGetDataPath(), fname);
		if (path != "")
		{
			if (pSprite->Create(fname, true))	// blending true
			{
				m_pOverlay = new vtGroup;
				m_pOverlay->setName("Overlay");
				IPoint2 size = pSprite->GetSize();
				pSprite->SetPosition((float) x, (float) y+size.y, (float) x+size.x, (float) y);
				m_pOverlay->addChild(pSprite->GetGeode());
			}
		}
	}

	// Let any terrain subclasses provide their own culture
	CreateCustomCulture();
}


//
// Create an LOD grid to contain and efficiently hide stuff that's far away
//
void vtTerrain::_SetupVegGrid(float fLODDistance)
{
	// must have a terrain with some size
	if (!m_pHeightField)
		return;

	FRECT world_extents;
	world_extents = m_pHeightField->m_WorldExtents;

	FPoint3 org(world_extents.left, 0.0f, world_extents.bottom);
	FPoint3 size(world_extents.right, 0.0f, world_extents.top);

	m_pVegGrid = new vtSimpleLodGrid;
	m_pVegGrid->Setup(org, size, LOD_GRIDSIZE, fLODDistance, m_pHeightField);
	m_pVegGrid->setName("Vegetation LOD Grid");
	m_pVegGrid->SetCastShadow(false);
	m_pTerrainGroup->addChild(m_pVegGrid);
}

// create vegetation
void vtTerrain::_CreateVegetation()
{
	// The vegetation nodes will be contained in an LOD Grid
	float fVegDistance = m_Params.GetValueInt(STR_VEGDISTANCE);
	_SetupVegGrid(fVegDistance);

	clock_t r1 = clock();	// start timing

	for (uint i = 0; i < m_Params.NumLayers(); i++)
	{
		// Look for veg layers
		if (m_Params.GetLayerType(i) != LT_VEG)
			continue;

		const vtTagArray &tags = m_Params.m_Layers[i];

		VTLOG(" Layer %d: Vegetation\n", i);
		vtString fname = tags.GetValueString("Filename");

		// Read the VF file
		vtString plants_fname = "PlantData/";
		plants_fname += fname;

		VTLOG("\tLooking for plants file: %s\n", (const char *) plants_fname);

		vtString plants_path = FindFileOnPaths(vtGetDataPath(), plants_fname);
		if (plants_path == "")
		{
			VTLOG1("\tNot found.\n");
			continue;
		}
		VTLOG("\tFound: %s\n", (const char *) plants_path);

		vtVegLayer *v_layer = LoadVegetation(plants_path);
		if (v_layer)
		{
			// If the user wants it to start hidden, hide it
			bool bVisible;
			if (tags.GetValueBool("visible", bVisible))
				v_layer->SetEnabled(bVisible);
		}
	}
	VTLOG(" Vegetation: %.3f seconds.\n", (float)(clock() - r1) / CLOCKS_PER_SEC);
}

//
// Create an LOD grid to contain and efficiently hide stuff that's far away
//
void vtTerrain::_SetupStructGrid(float fLODDistance)
{
	// must have a terrain with some size
	if (!m_pHeightField)
		return;

	FRECT world_extents;
	world_extents = m_pHeightField->m_WorldExtents;

	FPoint3 org(world_extents.left, 0.0f, world_extents.bottom);
	FPoint3 size(world_extents.right, 0.0f, world_extents.top);

	if (m_Params.GetValueBool(STR_STRUCTURE_PAGING))
	{
		m_pPagedStructGrid = new vtPagedStructureLodGrid;
		m_pStructGrid = m_pPagedStructGrid;

		m_iPagingStructureMax = m_Params.GetValueInt(STR_STRUCTURE_PAGING_MAX);
		m_fPagingStructureDist = m_Params.GetValueFloat(STR_STRUCTURE_PAGING_DIST);

		VTLOG("Created paged structure LOD grid, max %d, distance %f\n",
			m_iPagingStructureMax, m_fPagingStructureDist);
	}
	else
		m_pStructGrid = new vtSimpleLodGrid;
	m_pStructGrid->SetCastShadow(false);

	m_pStructGrid->Setup(org, size, LOD_GRIDSIZE, fLODDistance, m_pHeightField);
	m_pStructGrid->setName("Structures LOD Grid");
	m_pTerrainGroup->addChild(m_pStructGrid);
}

void vtTerrain::_CreateStructures()
{
	// Read terrain-specific content file
	vtString con_file = m_Params.GetValueString(STR_CONTENT_FILE);
	if (con_file != "")
	{
		VTLOG(" Looking for terrain-specific content file: '%s'\n", (const char *) con_file);
		vtString fname = FindFileOnPaths(vtGetDataPath(), con_file);
		if (fname != "")
		{
			VTLOG("  Found.\n");
			try
			{
				m_Content.ReadXML(fname);
			}
			catch (xh_io_exception &ex)
			{
				// display (or a least log) error message here
				VTLOG("  XML error:");
				VTLOG(ex.getFormattedMessage().c_str());
				return;
			}
		}
		else
			VTLOG("  Not found.\n");
	}

	// Always create a LOD grid for structures, as the user might create some
	// The LOD distances are in meters
	_SetupStructGrid((float) m_Params.GetValueInt(STR_STRUCTDIST));

	// Make sure we have our global material descriptors.
	vtStructure3d::InitializeMaterialArrays();

	// Create built structures
	for (uint i = 0; i < m_Params.NumLayers(); i++)
	{
		// Look for structure layers
		if (m_Params.GetLayerType(i) != LT_STRUCTURE)
			continue;

		VTLOG(" Layer %d: Structure\n", i);

		vtStructureLayer *st_layer = NewStructureLayer();
		st_layer->SetProps(m_Params.m_Layers[i]);
		if (!st_layer->Load(m_progress_callback))
		{
			VTLOG("\tCouldn't load structures.\n");
			// Removing the layer deletes it, by dereference.
			m_Layers.Remove(st_layer);
		}
	}
	for (uint i = 0; i < m_Layers.size(); i++)
	{
		vtStructureLayer *slay = dynamic_cast<vtStructureLayer*>(m_Layers[i].get());
		if (slay)
			CreateStructures(slay);
	}
}

/////////////////////////

void vtTerrain::_CreateAbstractLayersFromParams()
{
	// Go through the layers in the terrain parameters, and try to load them
	for (uint i = 0; i < m_Params.NumLayers(); i++)
	{
		if (m_Params.GetLayerType(i) == LT_RAW)
		{
			VTLOG(" Layer %d: Abstract\n", i);
			_CreateAbstractLayerFromParams(i);
		}
	}
}

bool vtTerrain::_CreateAbstractLayerFromParams(int index)
{
	const vtTagArray &lay = m_Params.m_Layers[index];

	// Show the tags
	for (uint j = 0; j < lay.NumTags(); j++)
	{
		const vtTag *tag = lay.GetTag(j);
		VTLOG("   Tag '%s': '%s'\n", (const char *)tag->name, (const char *)tag->value);
	}

	vtAbstractLayer *ab_layer = new vtAbstractLayer;

	// Copy all the properties from params to the new layer
	VTLOG1("  Setting layer properties.\n");
	ab_layer->SetProps(lay);
	m_Layers.push_back(ab_layer);

	// Abstract geometry goes into the scale features group, so it will be
	//  scaled up/down with the vertical exaggeration.
	bool success = ab_layer->Load(GetCRS(), NULL, m_progress_callback);
	if (!success)
	{
		m_Layers.Remove(ab_layer);
		return false;
	}

	CreateAbstractLayerVisuals(ab_layer);
	return true;
}

///////////////////////////////////////////////////////////////////////////////

void vtTerrain::_CreateElevLayers()
{
	// Go through the layers in the terrain parameters, and try to load them
	for (uint i = 0; i < m_Params.NumLayers(); i++)
	{
		// Look for elevation layers
		if (m_Params.GetLayerType(i) != LT_ELEVATION)
			continue;

		VTLOG(" Layer %d: Elevation\n", i);
		CreateElevLayerFromTags(m_Params.m_Layers[i]);
	}
}

bool vtTerrain::CreateElevLayerFromTags(const vtTagArray &layer_tags)
{
	for (uint j = 0; j < layer_tags.NumTags(); j++)
	{
		const vtTag *tag = layer_tags.GetTag(j);
		VTLOG("   Tag '%s': '%s'\n", (const char *)tag->name, (const char *)tag->value);
	}

	vtElevLayer *elayer = new vtElevLayer;
	elayer->SetProps(layer_tags);

	if (!elayer->Load(m_progress_callback))
		return false;

	elayer->MakeMaterials(m_bTextureCompression);

	vtTransform *top_node = elayer->CreateGeometry();
	top_node->setName("Elevation layer");

	// We require that the TIN has a compatible CRS with the base
	//  terrain, but the extents may be different.  If they are,
	//  we may need to offset the TIN to be in the correct place.
	DRECT ext1 = m_pHeightField->GetEarthExtents();
	DRECT ext2 = elayer->GetTin()->GetEarthExtents();
	DPoint2 offset = ext2.LowerLeft() - ext1.LowerLeft();
	float x, z;
	GetLocalCS().VectorEarthToLocal(offset, x, z);
		
	top_node->Translate(FPoint3(x, 0, z));
	m_pUnshadowedGroup->addChild(top_node);

	m_Layers.push_back(elayer);

	return true;
}

/**
 Turn on fog for this terrain.  Mutually exclusive with shadow.
 */
void vtTerrain::SetFog(bool fog)
{
	m_bFog = fog;
	if (m_bFog)
	{
		if (!m_pFog)
			m_pFog = new vtFog;

		ConnectFogShadow(true, false);	// true for Fog

		float dist = m_Params.GetValueFloat(STR_FOGDISTANCE) * 1000;
		if (m_fog_color.r != -1)
			m_pFog->SetFog(true, 0, dist, m_fog_color);
		else
			m_pFog->SetFog(true, 0, dist);
	}
	else
	{
		if (m_pFog)
			m_pFog->SetFog(false);
	}
}

/**
 Set the color of the fog.
 */
void vtTerrain::SetFogColor(const RGBf &color)
{
	m_fog_color = color;
	if (m_bFog)
		SetFog(true);
}

/**
 Set the distance at which the fog is solid.
 */
void vtTerrain::SetFogDistance(float fMeters)
{
	m_Params.SetValueFloat(STR_FOGDISTANCE, fMeters / 1000);
	if (m_bFog)
		SetFog(true);
}

/**
 Turn on shadows for this terrain.  Mutually exclusive with fog.
 */
void vtTerrain::SetShadows(bool shadows)
{
	m_bShadows = shadows;
	if (shadows)
	{
		if (!m_pShadow)
		{
			m_pShadow = new vtShadow(GetShadowTextureUnit(), m_pLightSource->getLight()->getLightNum());
			m_pShadow->SetHeightField3d(GetHeightField());
			m_pShadow->SetDarkness(m_Params.GetValueFloat(STR_SHADOW_DARKNESS));
			m_pShadow->SetShadowTextureResolution(m_Params.GetValueInt(STR_SHADOW_REZ));
			if (m_Params.GetValueBool(STR_LIMIT_SHADOW_AREA))
				m_pShadow->SetShadowSphereRadius(m_Params.GetValueFloat(STR_SHADOW_RADIUS));
			else
				m_pShadow->SetShadowSphereRadius(GetLODDistance(TFT_STRUCTURES)/2);
			m_pShadow->SetRecalculateEveryFrame(m_Params.GetValueBool(STR_SHADOWS_EVERY_FRAME));
			m_pShadow->AddLodGridToIgnore(GetStructureGrid());
			m_pShadow->setName("Shadow Group");
#if !VTLISPSM
			// When we connect up multitexturing it should probably be set up
			// here intitially. But at the moment we have not stored the texture
			// units used anywhere. So we cannot do it yet. Therefore any
			// multitexturing will be ignored when shadows are turned on and a
			// fragment shader is used. A fragment shader will be used if the
			// shadow darkness is less than one and the ARB_shadow_ambient
			// extension cannot be found.
#endif
		}
		ConnectFogShadow(false, true);
#if defined (VTDEBUG) && defined (VTDEBUGSHADOWS)
		m_pShadow->SetDebugHUD(m_pContainerGroup);
#endif
#if VTLISPSM
		m_pShadow->RemoveAllAdditionalTerrainTextureUnits();
		uint NumLayers = m_Layers.GetSize();
		for (uint i = 0; i < NumLayers; i++)
		{
			vtAbstractLayer *pAbstractLayer = dynamic_cast<vtAbstractLayer*>(m_Layers[i]);
			vtImageLayer *pImageLayer = dynamic_cast<vtImageLayer*>(m_Layers[i]);
			if (NULL != pAbstractLayer)
			{
				vtMultiTexture *pMultiTexture = pAbstractLayer->GetMultiTexture();
				if (NULL != pMultiTexture)
					m_pShadow->AddAdditionalTerrainTextureUnit(pMultiTexture->m_iTextureUnit, pMultiTexture->m_iMode);
			}
			else if (NULL != pImageLayer)
			{
				vtMultiTexture *pMultiTexture = pImageLayer->m_pMultiTexture;
				if (NULL != pMultiTexture)
					m_pShadow->AddAdditionalTerrainTextureUnit(pMultiTexture->m_iTextureUnit, pMultiTexture->m_iMode);
			}
		}
#endif
		m_pStructGrid->SetCastShadow(true);
	}
	else
	{
		ConnectFogShadow(m_bFog, false);
		m_pStructGrid->SetCastShadow(false);
	}
}

/**
 Set shadow options (darkness, radius, etc.)
 */
void vtTerrain::SetShadowOptions(const vtShadowOptions &opt)
{
	//param.SetValueBool(STR_STRUCT_SHADOWS);
	m_Params.SetValueFloat(STR_SHADOW_DARKNESS, opt.fDarkness);
	m_Params.SetValueBool(STR_SHADOWS_EVERY_FRAME, opt.bShadowsEveryFrame);
	m_Params.SetValueBool(STR_LIMIT_SHADOW_AREA, opt.bShadowLimit);
	m_Params.SetValueFloat(STR_SHADOW_RADIUS, opt.fShadowRadius);

	if (m_pShadow)
	{
		m_pShadow->SetDarkness(opt.fDarkness);
		m_pShadow->SetRecalculateEveryFrame(opt.bShadowsEveryFrame);
		if (opt.bShadowLimit)
			m_pShadow->SetShadowSphereRadius(opt.fShadowRadius);
		else
			m_pShadow->SetShadowSphereRadius(GetLODDistance(TFT_STRUCTURES)/2);
	}
}

void vtTerrain::GetShadowOptions(vtShadowOptions &opt)
{
	m_Params.GetValueBool(STR_LIMIT_SHADOW_AREA, opt.bShadowLimit);
	m_Params.GetValueFloat(STR_SHADOW_RADIUS, opt.fShadowRadius);

	if (m_pShadow)
	{
		opt.fDarkness = m_pShadow->GetDarkness();
		opt.bShadowsEveryFrame = m_pShadow->GetRecalculateEveryFrame();
	}
	else
	{
		m_Params.GetValueFloat(STR_SHADOW_DARKNESS, opt.fDarkness);
		m_Params.GetValueBool(STR_SHADOWS_EVERY_FRAME, opt.bShadowsEveryFrame);
	}
}

void vtTerrain::ForceShadowUpdate()
{
	if (m_pShadow)
		m_pShadow->ForceShadowUpdate();
}

void vtTerrain::SetBgColor(const RGBf &color)
{
	m_background_color = color;
}

void vtTerrain::ConnectFogShadow(bool bFog, bool bShadow)
{
	// Be careful - we are switching around some nodes, don't lose them to
	//  dereferencing deletion.
	m_pTerrainGroup->ref();

	// Add the fog, or shadow, into the scene graph between container and terrain
	while (m_pContainerGroup->getNumChildren() > 0)
		m_pContainerGroup->removeChild(m_pContainerGroup->getChild(0));
	if (m_pShadow)
		m_pShadow->removeChild(m_pTerrainGroup);
	if (m_pFog)
		m_pFog->removeChild(m_pTerrainGroup);

	if (bFog && m_pFog)
	{
		m_pContainerGroup->addChild(m_pFog);
		m_pFog->addChild(m_pTerrainGroup);
	}
	else if (bShadow && m_pShadow)
	{
		m_pContainerGroup->addChild(m_pShadow);
		m_pShadow->addChild(m_pTerrainGroup);
	}
	else
		m_pContainerGroup->addChild(m_pTerrainGroup);

	// re-attach
	m_pContainerGroup->addChild(m_pUnshadowedGroup);

	// Now safe to reset
	m_pTerrainGroup->unref();
}

///////////////////////////////////////////////////////////////////////
//////////////////////////// Time Methods /////////////////////////////

/**
 * Get the time at which a terrain is set to begin.
 */
vtTime vtTerrain::GetInitialTime()
{
	const char *str = m_Params.GetValueString(STR_INITTIME);

	VTLOG("Initial time: %s\n", str);

	vtTime localtime;
	localtime.SetFromString(str);
	return localtime;
}

/**
 * Given a time value, convert it from the LT (local time) of the center of
 * this terrain to GMT.  Local time is defined precisely by longitude,
 * e.g. at noon local time, the sun is exactly halfway across the sky.
 *
 * Note that this is different that the "standard time" of a given place,
 * which involves finding out what time zone is in effect (complicated!)
 */
void vtTerrain::TranslateToGMT(vtTime &time)
{
	if (m_CenterGeoLocation.x == -999)
		_ComputeCenterLocation();

	time.Increment(-m_iDifferenceFromGMT);
}

/**
 * Given a time value, convert it to the LT (local time) of the center of
 * this terrain from GMT.  Local time is defined precisely by longitude,
 * e.g. at noon local time, the sun is exactly halfway across the sky.
 *
 * Note that this is different that the "standard time" of a given place,
 * which involves finding out what time zone is in effect (complicated!)
 */
void vtTerrain::TranslateFromGMT(vtTime &time)
{
	if (m_CenterGeoLocation.x == -999)
		_ComputeCenterLocation();

	time.Increment(m_iDifferenceFromGMT);
}

DPoint2 vtTerrain::GetCenterGeoLocation()
{
	if (m_CenterGeoLocation.x == -999)
		_ComputeCenterLocation();

	return m_CenterGeoLocation;
}

void vtTerrain::_ComputeCenterLocation()
{
	vtHeightField3d *pHF = GetHeightField();
	DRECT drect = pHF->GetEarthExtents();
	drect.GetCenter(m_CenterGeoLocation);

	// must convert from whatever we CRS are, to Geographic
	vtCRS Dest;
	Dest.SetWellKnownGeogCS("WGS84");

	// We won't fail on tricky Datum conversions, but we still might
	//  conceivably fail if the GDAL/PROJ files aren't found.
	ScopedOCTransform trans(CreateTransformIgnoringDatum(&m_crs, &Dest));
	if (trans)
		trans->Transform(1, &m_CenterGeoLocation.x, &m_CenterGeoLocation.y);

	// calculate offset FROM GMT
	// longitude of 180 deg = 12 hours = 720 min = 43200 sec
	m_iDifferenceFromGMT = (int) (m_CenterGeoLocation.x / 180 * 43200);
}


void vtTerrain::CreateStep1()
{
	// Only do this method once
	if (m_pTerrainGroup)
		return;

	// create terrain group - this holds all surfaces for the terrain
	m_pContainerGroup = new vtGroup;
	m_pContainerGroup->setName("Terrain Container");
	m_pTerrainGroup = new vtGroup;
	m_pTerrainGroup->setName("Terrain Group");
	m_pUnshadowedGroup = new vtGroup;
	m_pUnshadowedGroup->setName("Unshadowed Group");

#if 0
	// TEST new shadow functionality
	m_pShadow = new vtShadow;
	m_pShadow->setName("Shadow Group");
	ConnectFogShadow(false, true);
#else
	// Initially, there is no fog or shadow
	ConnectFogShadow(false, false);
#endif

	// create engine group, the parent of all engines for this terrain
	m_pEngineGroup = new vtEngine;
	vtString name = "Engines for ";
	name += GetName();
	m_pEngineGroup->setName(name);
	m_AnimContainer.SetEngineContainer(m_pEngineGroup);
}

/**
 * First step in terrain creation: load elevation.
 * You can use these methods to build a terrain step by step, or simply
 * use the method vtTerrainScene::BuildTerrain, which calls them all.
 */
bool vtTerrain::CreateStep2()
{
	VTLOG1("Step2\n");

	// for GetValueFloat below
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	m_fVerticalExag = m_Params.GetValueFloat(STR_VERTICALEXAG);

	// User may have have supplied a grid directly, via SetLocalGrid
	if (m_pElevGrid.get())
	{
		VTLOG1("Using supplied elevation grid.\n");
		m_pElevGrid->SetupLocalCS(m_Params.GetValueFloat(STR_VERTICALEXAG));
		m_pHeightField = m_pElevGrid.get();
		m_crs = m_pElevGrid->GetCRS();
		m_bIsCreated = true;
		return true;
	}
	// User may have supplied a TIN directly, via SetTin
	if (m_pTin)
	{
		VTLOG1("Using supplied TIN.\n");
		m_pHeightField = m_pTin;
		m_crs = m_pTin->m_crs;
		m_bIsCreated = true;
		return true;
	}

	vtString fname;
	vtString elev_file = m_Params.GetValueString(STR_ELEVFILE);
	int surface_type = m_Params.GetValueInt(STR_SURFACE_TYPE);

	// Look for the elevation data source
	vtString elev_path;

	// The path might be a URL (for 'external terrain')
	if (elev_file.Left(5).CompareNoCase("http:") == 0)
	{
		// It's a URL, so we can't check for its validity yet; just pass it on
		elev_path = elev_file;
	}
	else if (vtFileExists(elev_file))		// Might be an absolute path
		elev_path = elev_file;
	else
	{
		// filename may be relative to any Elevation folder on the data path
		fname = "Elevation/";
		fname += elev_file;
		VTLOG("\tLooking for elevation file: %s\n", (const char *) fname);

		elev_path = FindFileOnPaths(vtGetDataPath(), fname);
		if (elev_path == "")
		{
			VTLOG("\t\tNot found.\n");
			if (surface_type != 3)
			{
				vtString msg;
				msg.Format("Couldn't find elevation '%s'", (const char *) elev_file);
				_SetErrorMessage(msg);
				return false;
			}
		}
	}

	VTLOG("\tFound elevation at: %s\n", (const char *) elev_path);

	if (surface_type == 0)
	{
		// Elevation input is a single grid; load it
		m_pElevGrid.reset(new vtElevationGrid);

		vtElevError err;
		bool status = m_pElevGrid->LoadFromBT(elev_path, m_progress_callback, &err);
		if (status == false)
		{
			_SetErrorMessage(err.message);
			return false;
		}
		VTLOG("\tGrid load succeeded.\n");

		// Set global CRS based on this terrain.
		m_crs = m_pElevGrid->GetCRS();

		const IPoint2 &gridsize = m_pElevGrid->GetDimensions();
		VTLOG("\t\tSize: %d x %d.\n", gridsize.x, gridsize.y);
		DRECT rect = m_pElevGrid->GetEarthExtents();
		VTLOG("\t\tEarth Extents LRTB: %lg %lg %lg %lg\n",
			rect.left, rect.right, rect.top, rect.bottom);

		float exag = m_Params.GetValueFloat(STR_VERTICALEXAG);
		VTLOG("\t\tVertical exaggeration: %g\n", exag);
		m_pElevGrid->SetupLocalCS(exag);

		FRECT frect = m_pElevGrid->m_WorldExtents;
		VTLOG("\t\tWorld Extents LRTB: %g %g %g %g\n",
			frect.left, frect.right, frect.top, frect.bottom);

		m_pHeightField = m_pElevGrid.get();

		// Apply ocean depth
		if (m_Params.GetValueBool(STR_DEPRESSOCEAN))
		{
			float fOceanDepth = m_Params.GetValueFloat(STR_DEPRESSOCEANLEVEL);
			m_pElevGrid->ReplaceValue(0, fOceanDepth);
		}
	}
	else if (surface_type == 1)
	{
		// Elevation input is a single TIN
		if (!m_pTin.valid())
		{
			// if they did not provide us with a TIN, try to load it
			m_pTin = new vtTin3d;
			bool status = m_pTin->Read(elev_path);

			if (status == false)
			{
				_SetErrorMessage("TIN load failed.");
				return false;
			}
			VTLOG("\tTIN load succeeded.\n");

			m_crs = m_pTin->m_crs;
			m_pHeightField = m_pTin;
		}
	}
	else if (surface_type == 2)
	{
		// Elevation input is a tileset.
		vtString tex_file = m_Params.GetValueString(STR_TEXTUREFILE);
		fname = "GeoSpecific/";
		fname += tex_file;
		VTLOG("\tLooking for texture file: %s\n", (const char *) fname);

		vtString tex_path = FindFileOnPaths(vtGetDataPath(), fname);
		if (tex_path == "")
		{
			VTLOG("\t\tNot found.\n");

			vtString msg;
			msg.Format("Couldn't find texture '%s'", (const char *) tex_path);
			_SetErrorMessage(msg);
			return false;
		}
		VTLOG("\tFound texture at: %s\n", (const char *) tex_path);

		// Elevation input is a set of tiles, which will be loaded later as needed
		m_pTiledGeom = new vtTiledGeom;
		m_pTiledGeom->setName("Tiled Geometry Container");
		m_pTiledGeom->SetVerticalExag(m_fVerticalExag);
		m_pTiledGeom->SetVertexTarget(m_Params.GetValueInt(STR_VERTCOUNT));
		if (vtGetScene()->getViewer()->getThreadingModel() == osgViewer::Viewer::SingleThreaded)
			m_pTiledGeom->SetProgressCallback(m_progress_callback);
		m_pTiledGeom->SetCastShadow(false);

		// tile cache size is in MB for the user, but bytes for the class
		int tile_cache_mb = m_Params.GetValueInt(STR_TILE_CACHE_SIZE);
		//m_pTiledGeom->SetTileCacheSize(tile_cache_mb * 1024 * 1024);

		bool bThread = m_Params.GetValueBool(STR_TILE_THREADING);
		bool bGradual = m_Params.GetValueBool(STR_TEXTURE_GRADUAL);
		bool status = m_pTiledGeom->ReadTileList(elev_path, tex_path,
			bThread, bGradual);

		if (status == false)
		{
			_SetErrorMessage("Tile list load failed.");
			return false;
		}
		m_pTiledGeom->SetTexLODFactor(m_Params.GetValueFloat(STR_TEXURE_LOD_FACTOR));
		m_pHeightField = m_pTiledGeom;
		m_crs = m_pTiledGeom->m_crs;

		// The tiled geometry base texture will always use texture unit 0
		m_TextureUnits.ReserveTextureUnit();
	}
	else if (surface_type == 3)
	{
		// Elevation input is some external thing like osgTerrain or osgEarth
		m_pExternalHeightField = new vtExternalHeightField3d;
		if (!m_pExternalHeightField->Initialize(elev_path))
		{
			_SetErrorMessage("Failed to initialise external heightfield.");
			return false;
		}
		m_pHeightField = m_pExternalHeightField;
		m_crs = m_pExternalHeightField->GetCRS();
	}
	char type[10], value[2048];
	m_crs.GetTextDescription(type, value);
	VTLOG(" Projection of the terrain: %s, '%s'\n", type, value);
	DRECT ext = m_pHeightField->GetEarthExtents();
	VTLOG(" Earth extents LRTB: %lf %lf %lf %lf\n", ext.left, ext.right, ext.top, ext.bottom);

	m_bIsCreated = true;
	return true;
}

/**
 * Next step in terrain creation: create textures.
 */
bool vtTerrain::CreateStep3(vtTransform *pSunLight, vtLightSource *pLightSource)
{
	VTLOG1("Step3\n");

	// Remember the lightsource in case we need it later for shadows
	m_pLightSource = pLightSource;

	// if we aren't going to produce the terrain surface, nothing to do
	if (m_Params.GetValueBool(STR_SUPPRESS))
		return true;

	int type = m_Params.GetValueInt(STR_SURFACE_TYPE);
	if (type == 0)		// Single grid
	{
		// measure total texture processing time
		clock_t c1 = clock();

		bool success = m_Texture.MakeTexture(m_Params, GetHeightFieldGrid3d(),
			m_bTextureCompression, m_progress_callback);

		if (success)
		{
			m_Texture.ShadeTexture(m_Params, GetHeightFieldGrid3d(), pSunLight->GetDirection(),
				m_progress_callback);

			// The terrain's base texture will always use unit 0
			m_TextureUnits.ReserveTextureUnit();
		}
	}
	if (type == 1)	// TIN
	{
		m_pTin->MakeMaterialsFromOptions(m_Params, m_bTextureCompression);

		vtMaterial *surface_material = m_pTin->GetSurfaceMaterial();
		if (surface_material)
		{
			for (uint i = 0; i < surface_material->NextAvailableTextureUnit(); i++)
				m_TextureUnits.ReserveTextureUnit();
		}
	}
	return true;
}

/**
 * Next step in terrain creation: create 3D geometry for the terrain.
 */
bool vtTerrain::CreateStep4()
{
	VTLOG1("Step4\n");

	// if we aren't going to produce the terrain surface, nothing to do
	if (m_Params.GetValueBool(STR_SUPPRESS))
		return true;

	if (m_Params.GetValueInt(STR_SURFACE_TYPE) == 0)	// single grid
		return CreateFromGrid();
	else if (m_Params.GetValueInt(STR_SURFACE_TYPE) == 1)	// TIN
		return CreateFromTIN();
	else if (m_Params.GetValueInt(STR_SURFACE_TYPE) == 2)	// tiles
		return CreateFromTiles();
	else if (m_Params.GetValueInt(STR_SURFACE_TYPE) == 3)	// external
		return CreateFromExternal();
	return true;
}

bool vtTerrain::CreateFromTIN()
{
	bool bDropShadow = false;

	// Make the TIN's geometry.
	vtGeode *geode = m_pTin->CreateGeometry(bDropShadow);
	geode->SetCastShadow(false);
	m_pTerrainGroup->addChild(geode);

	return true;
}

// for timing how long the CLOD takes to initialize
clock_t tm1;

bool vtTerrain::CreateFromGrid()
{
	VTLOG(" CreateFromGrid\n");
	tm1 = clock();

	// create elegant dynamic LOD terrain
	if (!_CreateDynamicTerrain())
	{
		return false;
	}
	else
	{
		m_pDynGeom->SetEnabled(true);
		m_pHeightField = m_pDynGeom;
	}

	if (!m_bPreserveInputGrid && !m_Params.GetValueBool(STR_ALLOW_GRID_SCULPTING))
	{
		// we don't need the original grid any more
		m_pElevGrid.reset();
	}
	return true;
}

bool vtTerrain::CreateFromTiles()
{
	// m_pTiledGeom already exists (although probably should be unbundled)
	m_pTerrainGroup->addChild(m_pTiledGeom);

	// the tileset will be the heightfield used at runtime, so extend
	//  it with the terrain's culture
	m_pTiledGeom->SetCulture(this);

	return true;
}

bool vtTerrain::CreateFromExternal()
{
	m_pTerrainGroup->addChild(m_pExternalHeightField->CreateGeometry());
	return true;
}

/**
 * Next step in terrain creation: additional CLOD construction.
 */
bool vtTerrain::CreateStep5()
{
	VTLOG1("Step5\n");

	// some algorithms need an additional stage of initialization
	if (m_pDynGeom != NULL)
	{
		m_pDynGeom->Init2();

		clock_t tm2 = clock();
		float time = ((float)tm2 - tm1)/CLOCKS_PER_SEC;
		VTLOG("CLOD construction: %.3f seconds.\n", time);
	}

	// We should also speed up our TIN if we have one.
	if (m_pTin != NULL)
	{
		m_pTin->SetupTriangleBins(25);	// TODO: make this number adaptive
	}

	return true;
}

/**
 * Next step in terrain creation: create the culture and labels.
 */
void vtTerrain::CreateStep6()
{
	VTLOG1("Step6\n");

	// must have a heightfield by this point
	if (!m_pHeightField)
		return;

	// Node to put all the scaled features under
	m_pScaledFeatures = new vtTransform;
	m_pScaledFeatures->setName("Scaled Features");
	m_pScaledFeatures->Scale(1.0f, m_fVerticalExag, 1.0f);
	m_pScaledFeatures->SetCastShadow(false);
	m_pUnshadowedGroup->addChild(m_pScaledFeatures);

	_CreateStructures();
}

void vtTerrain::CreateStep7()
{
	VTLOG1("Step7\n");

	// create roads
	if (m_Params.GetValueBool(STR_ROADS))
		_CreateRoads();
}

void vtTerrain::CreateStep8()
{
	VTLOG1("Step8\n");

	_CreateVegetation();
}

void vtTerrain::CreateStep9()
{
	VTLOG1("Step9\n");

	_CreateOtherCulture();

	if (m_Params.GetValueBool(STR_OCEANPLANE))
	{
		CreateWaterPlane();
		SetWaterLevel(m_Params.GetValueFloat(STR_OCEANPLANELEVEL));
	}

	if (m_Params.GetValueBool(STR_WATER))
	{
		vtString prefix = "Elevation/";
		vtString wfile = m_Params.GetValueString(STR_WATERFILE);
		vtString wpath = FindFileOnPaths(vtGetDataPath(), prefix + wfile);
		if (wpath == "")
			VTLOG("Couldn't find  water elevation file: %s\n", (const char *) wfile);
		else
			CreateWaterHeightfield(wpath);
	}
}

void vtTerrain::CreateStep10()
{
	VTLOG1("Step10\n");

	_CreateAbstractLayersFromParams();
}

void vtTerrain::CreateStep11()
{
	VTLOG1("Step11\n");

	CreateImageLayers();
}

void vtTerrain::CreateStep12()
{
	VTLOG1("Step12\n");

	_CreateElevLayers();

	// Engines will be activated later in vtTerrainScene::SetTerrain
	ActivateEngines(false);

	// Read stored locations
	vtString locfile = m_Params.GetValueString(STR_LOCFILE);
	if (locfile != "")
	{
		vtString loc = "Locations/";
		loc += locfile;
		vtString path = FindFileOnPaths(vtGetDataPath(), loc);
		if (path != "")
		{
			VTLOG("Reading locations file: %s.\n", (const char *) path);
			m_LocSaver.Read(path);
		}
	}
	VTLOG1("Setup location saver coordinate conversion.\n");
	m_LocSaver.SetLocalCS(m_pHeightField->m_LocalCS);
	m_LocSaver.SetCRS(m_crs);

	// Read stored animpaths
	for (uint i = 0; i < m_Params.m_AnimPaths.size(); i++)
	{
		vtString fname1 = m_Params.m_AnimPaths[i];
		vtString fname2 = "Locations/" + fname1;
		vtString path = FindFileOnPaths(vtGetDataPath(), fname2);
		if (path == "")
			continue;

		VTLOG("Reading animpath: %s.\n", (const char *) path);
		vtAnimPath *anim = new vtAnimPath;

		// Ensure that anim knows the CRS
		if (!anim->SetCRS(GetCRS(), GetLocalCS()))
		{
			// no CRS, no functionality
			delete anim;
			continue;
		}
		if (!anim->Read(path))
		{
			delete anim;
			continue;
		}
		vtAnimPathEngine *engine = new vtAnimPathEngine(anim);
		engine->setName("AnimPathEngine");
		engine->AddTarget(vtGetScene()->GetCamera());
		engine->SetEnabled(false);
		AddEngine(engine);

		vtAnimEntry entry;
		entry.m_pAnim = anim;
		entry.m_pEngine = engine;
		entry.m_Name = fname1;
		m_AnimContainer.AppendEntry(entry);
	}
}

void vtTerrain::SetProgressCallback(ProgFuncPtrType progress_callback)
{
	m_progress_callback = progress_callback;
}

bool vtTerrain::ProgressCallback(int i)
{
	if (m_progress_callback != NULL)
		return m_progress_callback(i);
	else
		return false;
}

bool vtTerrain::IsCreated()
{
	return m_bIsCreated;
}

void vtTerrain::Enable(bool bVisible)
{
	m_pContainerGroup->SetEnabled(bVisible);
}

/**
 * Return the center of the bounding sphere that encloses the terrain's
 * dynamic geometry.
 */
FPoint3 vtTerrain::GetCenter()
{
	GetTerrainBounds();
	return m_bound_sphere.center;
}

/**
 * Return the radius of the bounding sphere that encloses the terrain's
 * dynamic geometry.
 */
float vtTerrain::GetRadius()
{
	GetTerrainBounds();
	return m_bound_sphere.radius;
}

void vtTerrain::GetTerrainBounds()
{
	if (m_pDynGeomScale != NULL)
		m_pDynGeomScale->GetBoundSphere(m_bound_sphere);
	else
		m_bound_sphere.SetToZero();
}

/**
 * Tests whether a given point (in Earth coordinates) is within the current
 * extents of the terrain.
 */
bool vtTerrain::PointIsInTerrain(const DPoint2 &p)
{
	float x, z;
	m_pHeightField->m_LocalCS.EarthToLocal(p, x,  z);	// convert earth -> XZ
	return m_pHeightField->ContainsWorldPoint(x, z);
}

/**
 * The base CreateCustomCulture does nothing; this virtual method is meant to
 * be overridden by your terrain subclass to add its own culture.
 */
void vtTerrain::CreateCustomCulture()
{
}

/**
 * Add an engine to this terrain's group of engines.
 * The engine will be enabled only when the terrain is visible.
 */
void vtTerrain::AddEngine(vtEngine *pE)
{
	// add to this Terrain's engine list
	m_pEngineGroup->AddChild(pE);
}

void vtTerrain::ActivateEngines(bool bActive)
{
	m_pEngineGroup->SetEnabled(bActive);
}

//////////////////////////

void vtTerrain::SetFeatureVisible(TFType ftype, bool bOn)
{
	switch (ftype)
	{
	case TFT_TERRAINSURFACE:
		if (m_pDynGeom)
			m_pDynGeom->SetEnabled(bOn);
		else if (m_pTiledGeom)
			m_pTiledGeom->SetEnabled(bOn);
		else if (m_pTin)
			m_pTin->GetGeometry()->SetEnabled(bOn);
		break;
	case TFT_OCEAN:
		CreateWaterPlane();
		m_pOceanGeom->SetEnabled(bOn);
		break;
	case TFT_VEGETATION:
		if (m_pVegGroup)
			m_pVegGroup->SetEnabled(bOn);
		if (m_pVegGrid)
			m_pVegGrid->SetEnabled(bOn);
		break;
	case TFT_STRUCTURES:
		if (m_pStructGrid)
			m_pStructGrid->SetEnabled(bOn);
		break;
	case TFT_ROADS:
		if (m_pRoadGroup)
			m_pRoadGroup->SetEnabled(bOn);
		break;
	}
}

bool vtTerrain::GetFeatureVisible(TFType ftype)
{
	switch (ftype)
	{
	case TFT_TERRAINSURFACE:
		if (m_pDynGeom)
			return m_pDynGeom->GetEnabled();
		else if (m_pTiledGeom)
			return m_pTiledGeom->GetEnabled();
		else if (m_pTin)
			return m_pTin->GetGeometry()->GetEnabled();
		break;
	case TFT_OCEAN:
		if (m_pOceanGeom)
			return m_pOceanGeom->GetEnabled();
		break;
	case TFT_VEGETATION:
		if (m_pVegGrid)
			return m_pVegGrid->GetEnabled();
		else if (m_pVegGroup)
			return m_pVegGroup->GetEnabled();
		break;
	case TFT_STRUCTURES:
		if (m_pStructGrid)
			return m_pStructGrid->GetEnabled();
		break;
	case TFT_ROADS:
		if (m_pRoadGroup)
			return m_pRoadGroup->GetEnabled();
		break;
	}
	return false;
}

void vtTerrain::SetLODDistance(TFType ftype, float fDistance)
{
	switch (ftype)
	{
	case TFT_TERRAINSURFACE:
	case TFT_OCEAN:
		break;
	case TFT_VEGETATION:
		if (m_pVegGrid)
			m_pVegGrid->SetDistance(fDistance);
		osg::GroupLOD::setGroupDistance(fDistance);
		break;
	case TFT_STRUCTURES:
		if (m_pStructGrid)
		{
			if (fDistance != m_pStructGrid->GetDistance())
				VTLOG("Structure LOD dist = %.1f\n", fDistance);
			m_pStructGrid->SetDistance(fDistance);
			if (m_pShadow && !m_Params.GetValueBool(STR_LIMIT_SHADOW_AREA))
				m_pShadow->SetShadowSphereRadius(fDistance);
			EnforcePageOut();
		}
		break;
	case TFT_ROADS:
		if (m_pRoadMap.get())
			m_pRoadMap->SetLodDistance(fDistance);
		break;
	}
}

float vtTerrain::GetLODDistance(TFType ftype)
{
	switch (ftype)
	{
	case TFT_TERRAINSURFACE:
	case TFT_OCEAN:
		break;
	case TFT_VEGETATION:
		if (m_pVegGrid)
			return m_pVegGrid->GetDistance();
		else
			return osg::GroupLOD::getGroupDistance();
		break;
	case TFT_STRUCTURES:
		if (m_pStructGrid)
			return m_pStructGrid->GetDistance();
		break;
	case TFT_ROADS:
		if (m_pRoadMap.get())
			return m_pRoadMap->GetLodDistance();
		break;
	}
	return 0.0f;
}

/**
 * Return the heightfield for this terrain.  It may be a grid, or a TIN.
 * If you know that your data is a grid, you can use GetHeightFieldGrid3d()
 * to get that specifically.
 */
vtHeightField3d *vtTerrain::GetHeightField() const
{
	return m_pHeightField;
}

/**
 * Return the heightfield grid for this terrain.
 * During the construction of the terain, this may be a source vtElevationGrid.
 * Later, at runtime, it is likely to be one of the dynamic geometry (CLOD)
 * grids.
 */
vtHeightFieldGrid3d *vtTerrain::GetHeightFieldGrid3d()
{
	// if we still have the source elevation, use it
	if (m_pElevGrid.get())
		return m_pElevGrid.get();

	// otherwise, later on, we might only have the runtime (CLOD) grid
	else if (m_pDynGeom)
		return m_pDynGeom;

	return NULL;	// no grid to return, possible because it's a TIN
}

bool vtTerrain::FindAltitudeOnCulture(const FPoint3 &p3, float &fAltitude,
									  bool bTrue, int iCultureFlags) const
{
	// beware - OSG can be picky about the length of this segment.  It
	//  is a numerical precision issue.  If we use 1E9,-1E9 then it fails
	//  to find some objects.  Instead, search just in the range of elevation
	//  for this terrain, plus a margin to include potential culture.
	float minh, maxh;
	m_pHeightField->GetHeightExtents(minh, maxh);
	FPoint3 start(p3.x, maxh + 1000, p3.z);
	FPoint3 end(p3.x, minh - 1000, p3.z);

	vtHitList hlist;

	if (iCultureFlags & CE_STRUCTURES)
		vtIntersect(m_pStructGrid, start, end, hlist);

	if ((iCultureFlags & CE_ROADS) && m_pRoadGroup)
		vtIntersect(m_pRoadGroup, start, end, hlist);

	if (hlist.size() > 0)
	{
		// take first match encountered
		fAltitude =  hlist[0].point.y;

		if (bTrue)
		{
			const vtDynTerrainGeom *dg = GetDynTerrain();
			if (dg)
				fAltitude /= dg->GetVerticalExag();
		}
		return true;
	}
	return false;
}

int vtTerrain::GetShadowTextureUnit()
{
	if (m_iShadowTextureUnit == -1)
		m_iShadowTextureUnit = m_TextureUnits.ReserveTextureUnit(true);
	return m_iShadowTextureUnit;
}


///////////////////////////////////////////////////////////////////////
// Camera viewpoints

void vtTerrain::SetCamLocation(FMatrix4 &mat)
{
	FPoint3 trans = mat.GetTrans();
	VTLOG("Setting stored viewpoint for terrain '%s' to position %.1f, %.1f, %.1f\n",
		(const char *) GetName(), trans.x, trans.y, trans.z);

	m_CamLocation = mat;
}


///////////////////////////////////////////////////////////////////////
// Layers

void vtTerrain::RemoveLayer(vtLayer *lay, bool progress_callback(int))
{
	vtStructureLayer *slay = dynamic_cast<vtStructureLayer*>(lay);
	vtAbstractLayer *alay = dynamic_cast<vtAbstractLayer*>(lay);
	vtVegLayer *vlay = dynamic_cast<vtVegLayer*>(lay);
	vtElevLayer *elay = dynamic_cast<vtElevLayer*>(lay);
	if (slay)
	{
		// Remove each structure from the terrain
		for (uint i = 0; i < slay->size(); i++)
		{
			if (progress_callback != NULL)
				progress_callback(i * 99 / slay->size());

			DeleteStructureFromTerrain(slay, i);
		}
		// Be certain we're not still trying to page it in
		if (m_pPagedStructGrid)
			m_pPagedStructGrid->ClearQueue(slay);
	}
	else if (alay)
	{
		// first remove them from the terrain
		RemoveFeatureGeometries(alay);
	}
	else if (vlay)
	{
		for (uint i = 0; i < vlay->NumEntities(); i++)
			vlay->Select(i, true);
		DeleteSelectedPlants(vlay);
	}
	else if (elay)
	{
		vtTransform *xform = elay->GetTopNode();
		if (xform)
			m_pUnshadowedGroup->removeChild(xform);
	}

	// If that was the active layer, deal with it
	if (lay == GetActiveLayer())
		SetActiveLayer(NULL);

	// Removing the reference-counted layer causes it to be deleted
	m_Layers.Remove(lay);
}

vtLayer *vtTerrain::GetOrCreateLayerOfType(LayerType type)
{
	vtLayer *layer = GetActiveLayer();
	if (layer && layer->GetType() == type)
		return layer;		// We already have one active.

	// Look for one
	for (uint i = 0; i < m_Layers.size(); i++)
	{
		if (m_Layers[i]->GetType() == type)
			return m_Layers[i];
	}
	// else, create one.
	switch (type)
	{
	case LT_IMAGE:
		// TODO maybe
		break;
	case LT_STRUCTURE:
		layer = NewStructureLayer();
		layer->SetLayerName("Untitled.vtst");
		break;
	case LT_VEG:
		layer = NewVegLayer();
		layer->SetLayerName("Untitled.vf");
		break;
	default:
		// We don't support the rest (..yet). Keep picky compilers quiet.
		break;
	}
	return layer;
}

uint vtTerrain::NumLayersOfType(LayerType type)
{
	uint count = 0;
	for (uint i = 0; i < m_Layers.size(); i++)
	{
		if (m_Layers[i]->GetType() == type)
			count++;
	}
	return count;
}


///////////////////////////////////////////////////////////////////////
// Plants

vtVegLayer *vtTerrain::GetVegLayer()
{
	return dynamic_cast<vtVegLayer *>(m_pActiveLayer);
}

/**
 * Create a new veg array for this terrain, and returns it.
 */
vtVegLayer *vtTerrain::NewVegLayer()
{
	vtVegLayer *vlay = new vtVegLayer;

	// Apply properties from the terrain
	vlay->SetSpeciesList(m_pSpeciesList);
	vlay->SetCRS(m_crs);
	vlay->SetHeightField(m_pHeightField);

	m_Layers.push_back(vlay);
	return vlay;
}

vtVegLayer *vtTerrain::LoadVegetation(const vtString &fname)
{
	vtVegLayer *v_layer = NewVegLayer();
	bool success;
	if (!fname.Right(3).CompareNoCase("shp"))
		success = v_layer->ReadSHP(fname);
	else
		success = v_layer->ReadVF(fname);
	if (success)
	{
		VTLOG("\tLoaded plants file, %d plants.\n", v_layer->NumEntities());
		v_layer->SetFilename(fname);
	}
	else
	{
		VTLOG1("\tCouldn't load plants file.\n");
		return NULL;
	}

	// Create the 3d plants
	VTLOG1(" Creating Plant geometry..\n");
	if (m_Params.GetValueBool(STR_TREES_USE_SHADERS))
	{
		float fVegDistance = m_Params.GetValueInt(STR_VEGDISTANCE);
		osg::GroupLOD::setGroupDistance(fVegDistance);
		int created = v_layer->CreatePlantShaderNodes(m_progress_callback);
		m_pVegGroup = v_layer->m_group;
		m_pTerrainGroup->addChild(m_pVegGroup);
	}
	else
	{
		int created = v_layer->CreatePlantNodes(m_progress_callback);
		VTLOG("\tCreated: %d of %d plants\n", created, v_layer->NumEntities());
		if (v_layer->NumOffTerrain())
			VTLOG("\t%d were off the terrain.\n", v_layer->NumOffTerrain());

		for (uint i = 0; i < v_layer->NumEntities(); i++)
		{
			vtTransform *pTrans = v_layer->GetPlantNode(i);

			// add tree to scene graph
			if (pTrans)
				AddNodeToVegGrid(pTrans);
		}
	}
	return v_layer;
}

/**
 * Create a new plant instance at a given location and add it to the terrain.
 *
 * \param v_layer The vegetation layer to add to.
 * \param pos The 2D earth position of the new plant.
 * \param iSpecies Index of the species in the terrain's plant list.
 *		If you don't know the index, you can find it with
 *		 vtSpeciesList::GetSpeciesIdByName or vtSpeciesList::GetSpeciesIdByCommonName.
 * \param fSize Height of the new plant (meters).
 */
bool vtTerrain::AddPlant(vtVegLayer *v_layer, const DPoint2 &pos, int iSpecies, float fSize)
{
	int num = v_layer->AddPlant(pos, fSize, iSpecies);
	if (num == -1)
		return false;

	if (!v_layer->CreatePlantNode(num))
		return false;

	// add tree to scene graph
	AddNodeToVegGrid(v_layer->GetPlantNode(num));
	return true;
}

/**
 * Delete all the selected plants in the terrain's plant array.
 */
int vtTerrain::DeleteSelectedPlants(vtVegLayer *v_layer)
{
	int num_deleted = 0;
	for (int i = v_layer->NumEntities() - 1; i >= 0; i--)
	{
		if (v_layer->IsSelected(i))
		{
			RemoveAndDeletePlant(v_layer, i);
			num_deleted++;
		}
	}
	return num_deleted;
}

void vtTerrain::RemoveAndDeletePlant(vtVegLayer *v_layer, int index)
{
	vtTransform *pTrans = v_layer->GetPlantNode(index);
	if (pTrans != NULL)
	{
		osg::Group *pParent = pTrans->getParent(0);
		if (pParent)
		{
			pParent->removeChild(pTrans);
			v_layer->DeletePlant(index);
		}
	}
}

/**
 * Set the list of plant species that this terrain should use.  Using this
 * method allows a set of species to be shared between many terrains.
 */
void vtTerrain::SetSpeciesList(vtSpeciesList3d *pSpeciesList)
{
	m_pSpeciesList = pSpeciesList;
}

/**
 * Adds a node to the terrain.
 * The node will be added directly to this specific terrain, so it will be
 * hidden whenever the terrain is not active.
 *
 * \sa AddNodeToVegGrid, AddNodeToStructGrid
 */
void vtTerrain::addNode(osg::Node *pNode)
{
	m_pTerrainGroup->addChild(pNode);
}

/**
 * Adds a node to the terrain.
 * The node will be added to the Vegetation LOD Grid of the terrain, so it
 * will be culled when it is far from the viewer.  This method is used by
 * the terrain vegetation methods, and you can also use it directly if you
 * have your own vegetation nodes to add.
 *
 * \sa AddNode
 */
bool vtTerrain::AddNodeToVegGrid(osg::Node *pNode)
{
	if (!m_pVegGrid)
		return false;
	return m_pVegGrid->AddToGrid(pNode);
}

int vtTerrain::NumVegLayers() const
{
	int count = 0;
	for (size_t i = 0; i < m_Layers.size(); i++)
	{
		if (dynamic_cast<vtVegLayer*>(m_Layers[i].get()))
			count++;
	}
	return count;
}

bool vtTerrain::FindClosestPlant(const DPoint2 &point, double epsilon,
	int &plant_index, vtVegLayer **v_layer)
{
	plant_index = -1;
	double closest = 1E8;

	// if all plants are hidden, don't find them
	if (!GetFeatureVisible(TFT_VEGETATION))
		return false;

	double dist;
	int layers = m_Layers.size();
	for (int i = 0; i < layers; i++)
	{
		vtVegLayer *vlay = dynamic_cast<vtVegLayer*>(m_Layers[i].get());
		if (!vlay)
			continue;
		if (!vlay->GetEnabled())
			continue;

		// find index of closest plant
		int plant = vlay->FindClosestPoint(point, epsilon, &dist);
		if (plant != -1 && dist < closest)
		{
			*v_layer = vlay;
			plant_index = plant;
			closest = dist;
		}
	}
	return (plant_index != -1);
}

void vtTerrain::DeselectAllPlants()
{
	for (size_t i = 0; i < m_Layers.size(); i++)
	{
		vtVegLayer *v_layer = dynamic_cast<vtVegLayer *>(m_Layers[i].get());
		if (v_layer)
			v_layer->VisualDeselectAll();
	}
}


/**
 * Adds a node to the terrain.
 * The node will be added to the Structure LOD Grid of the terrain, so it will be
 * culled when it is far from the viewer.  This is usually desirable when
 * the models are complicated or there are lot of them.
 *
 * There is another form of this method which takes a vtGeode node instead.
 *
 * \sa AddNode
 */
bool vtTerrain::AddNodeToStructGrid(osg::Node *pNode)
{
	if (!m_pStructGrid)
		return false;
	return m_pStructGrid->AddToGrid(pNode);
}

/**
 * Removes a node from the terrain.
 *
 * \sa RemoveNodeFromStructGrid
 */
void vtTerrain::removeNode(osg::Node *pNode)
{
	m_pTerrainGroup->removeChild(pNode);
}

/**
 * Removes a node from the terrain's structure LOD grid.
 *
 * \sa RemoveNode
 */
void vtTerrain::RemoveNodeFromStructGrid(osg::Node *pNode)
{
	if (m_pStructGrid)
		m_pStructGrid->RemoveFromGrid(pNode);
}

int vtTerrain::DoStructurePaging()
{
	if (!m_pPagedStructGrid)
		return 0;

	vtCamera *cam = vtGetScene()->GetCamera();
	FPoint3 CamPos = cam->GetTrans();

	m_pPagedStructGrid->DoPaging(CamPos, m_iPagingStructureMax,
		m_fPagingStructureDist);
	return m_pPagedStructGrid->GetQueueSize();
}

void vtTerrain::SetStructurePageOutDistance(float f)
{
	if (m_pPagedStructGrid)
	{
		m_fPagingStructureDist = f;
		EnforcePageOut();
	}
}

void vtTerrain::EnforcePageOut()
{
	// The page-out distance should never be less than the structure LOD
	//  distance, otherwise we might be trying to load and unload the same
	//  structures.
	float fStructLodDist = GetLODDistance(TFT_STRUCTURES);
	if (m_fPagingStructureDist < fStructLodDist + 50)	// 50m arbitrary spacing
		m_fPagingStructureDist = fStructLodDist + 50;
}

///////////////////////////////////////////////////////////////////////

class TurbineEngine : public vtEngine
{
public:
	TurbineEngine()
	{
		m_fLastTime = FLT_MAX;
		m_fDir = PIf * 0.5f;
		m_fRot = 0.0f;
		m_fRotSpeed = 0.0f;
	}
	void Eval();

	float m_fLastTime;
	float m_fDir;		// radians
	float m_fRot;		// radians
	float m_fRotSpeed;	// radians per second
	FPoint3 m_rotor_pivot;
	vtStructure *m_pStructure;
	vtTerrain *m_pTerrain;
};

void TurbineEngine::Eval()
{
	float fNow = vtGetTime();
	if (m_fLastTime==FLT_MAX)
		m_fLastTime = fNow;
	float fElapsed = fNow - m_fLastTime;

	float fWindDir = 0.0f;
	float fWindSpeed = 0.0f;

	// Get these properties from the terrain
	TParams &params = m_pTerrain->GetParams();
	params.GetValueFloat("WindDirection", fWindDir);
	params.GetValueFloat("WindSpeed", fWindSpeed);

	// Convert degrees to radians
	float fTargetDir = ((180-fWindDir) / 180.0f * PIf);

	// Face wind gradually
	m_fDir += ((fTargetDir - m_fDir) * 0.05f);

	// "The maximum rotor speed is about 35 rpm for the smallest windmills.
	//  The larger windmills never go above 15-18 rpm."
	// 35 rpm = 3.66 radians/sec

	// Windspeed goes from 0-40, mapping 40->3.66 is 0.0915

	// And match wind speed gradually
	// Convert m/s to approximate radians/s
	float fTargetSpeed = fWindSpeed * 0.0915;	// ad hoc
	m_fRotSpeed += ((fTargetSpeed - m_fRotSpeed) * 0.05f);

	for (unsigned int t = 0; t < NumTargets(); t++)
	{
		osg::Referenced *tar = GetTarget(t);
		vtTransform *x1 = dynamic_cast<vtTransform*>(tar);
		if (!x1) return;

		//x1->Identity();
		//x1->Rotate2(FPoint3(0,1,0), m_fDir);

		m_fRot += (m_fRotSpeed * fElapsed);
		if (m_fRot >= PI2f)
			m_fRot -= PI2f;
		x1->Identity();
		x1->Rotate(FPoint3(0,0,1), m_fRot);
		x1->Translate(m_rotor_pivot);
	}
	m_fLastTime = fNow;
}

/**
 Extend some structures with behavior.  A developer can also subclass vtTerrain
 to implement their own behaviors.
 */
void vtTerrain::OnCreateBehavior(vtStructure *str)
{
	vtStructInstance3d *si = dynamic_cast<vtStructInstance3d *>(str);
	if (!si)
		return;

	vtString itemname = si->GetValueString("itemname", true);
	if (itemname == "")
		return;

	vtItem *item = m_Content.FindItemByName(itemname);
	// If that didn't work, also try the global content manager
	if (!item)
		item = vtGetContent().FindItemByName(itemname);
	if (!item)
		return;

	vtString behavior = item->GetValueString("behavior", true);
	if (behavior == "wind_turbine")
	{
		itemname += " rotor";

		// Look in terrain content, and global content manager
		osg::Node *n2 = m_Content.CreateNodeFromItemname(itemname);
		if (!n2)
			n2 = vtGetContent().CreateNodeFromItemname(itemname);

		if (!n2)
			return;

		vtTransform *x1 = new vtTransform;

		x1->setName("x1");

		si->GetContainer()->addChild(x1);
		x1->addChild(n2);

		TurbineEngine *te = new TurbineEngine;
		te->m_pStructure = str;
		te->setName("Turbine");
		te->m_rotor_pivot.Set(0,0,0);

		// Look in terrain content, and global content manager
		vtItem *rotor_item = m_Content.FindItemByName(itemname);
		if (!rotor_item)
			rotor_item = vtGetContent().FindItemByName(itemname);
		if (rotor_item)
		{
			const char *pivot = rotor_item->GetValueString("rotor_pivot");
			if (pivot)
			{
				// Avoid trouble with '.' and ',' in Europe
				ScopedLocale normal_numbers(LC_NUMERIC, "C");

				FPoint3 p;
				if (sscanf(pivot, "%f, %f, %f", &p.x, &p.y, &p.z) == 3)
					te->m_rotor_pivot = p;
			}
		}
		te->AddTarget(x1);
		te->m_pTerrain = this;
		AddEngine(te);
	}
}

void vtTerrain::OnDeleteBehavior(vtStructure *str)
{
	// Remove all engines which are associated with this structure
	vtEngine *top = GetEngineGroup();
	for (unsigned int i = 0; i < top->NumChildren(); i++)
	{
		vtEngine *eng = top->GetChild(i);
		TurbineEngine *te = dynamic_cast<TurbineEngine*>(eng);
		if (te && te->m_pStructure == str)
		{
			// remove it
			top->RemoveChild(eng);
			return;
		}
	}
}


////////////////////////////////////////////////////////////////////////////
// Abstracts

/**
 * Create a new abstract array for this terrain, and returns it.
 */
vtAbstractLayer *vtTerrain::NewAbstractLayer()
{
	vtAbstractLayer *alay = new vtAbstractLayer;
	m_Layers.push_back(alay);
	return alay;
}

/** Get the currently active abstract layer for this terrain. */
vtAbstractLayer *vtTerrain::GetAbstractLayer()
{
	return dynamic_cast<vtAbstractLayer *>(m_pActiveLayer);
}

bool vtTerrain::CreateAbstractLayerVisuals(vtAbstractLayer *ab_layer)
{
	// We must decide how to tesselate the features, in case of interpolation of edges.
	// That depends on the spacing of the underlying surface, which may vary from place
	// to place.  Look for where the center of the featureset is.
	bool have_center = false;
	DPoint2 center;
	DRECT ext;
	bool have_extents = ab_layer->EarthExtents(ext);
	if (have_extents)
	{
		center = ext.GetCenter();
		const vtCRS &source = ab_layer->GetFeatureSet()->GetAtCRS();

		// If we have two valid CRSs, and they are not the same, then we need a transform
		if (source.GetRoot() && m_crs.GetRoot() && !source.IsSame(&m_crs))
		{
			ScopedOCTransform trans(CreateTransformIgnoringDatum(&source, &m_crs));
			if (trans)
			{
				if (trans->Transform(1, &center.x, &center.y) == 1)
					have_center = true;
			}
		}
	}
	if (!have_center)
	{
		// Just use the terrain center.
		center = GetHeightField()->GetEarthExtents().GetCenter();
	}
	float fSpacing = EstimateGroundSpacingAtPoint(center);

	VTLOG1("  Constructing layer visuals.\n");
	ab_layer->CreateFeatureVisuals(GetScaledFeatures(), GetHeightField(),
		fSpacing, m_progress_callback);

	// Show only layers which should be visible
	bool bVis;
	bool has_value = ab_layer->Props().GetValueBool("Visible", bVis);
	if (has_value)
		ab_layer->SetVisible(bVis);

	return true;
}

void vtTerrain::RemoveFeatureGeometries(vtAbstractLayer *alay)
{
	alay->ReleaseGeometry();
}

int vtTerrain::DeleteSelectedFeatures(vtAbstractLayer *alay)
{
	int count = 0;

	int NumToDelete = 0;
	vtFeatureSet *fset = alay->GetFeatureSet();
	for (uint j = 0; j < fset->NumEntities(); j++)
	{
		if (fset->IsSelected(j))
		{
			fset->SetToDelete(j);
			NumToDelete++;
		}
	}
	if (NumToDelete > 0)
	{
		VTLOG("Set %d items to delete, removing visuals..\n", NumToDelete);

		// Delete high-level features first
		for (uint j = 0; j < fset->NumEntities(); j++)
		{
			if (fset->IsDeleted(j))
			{
				vtFeature *f = fset->GetFeature(j);
				alay->DeleteFeature(f);
			}
		}
		// Then low-level
		fset->ApplyDeletion();

		// Finish
		alay->EditEnd();

		count += NumToDelete;
	}
	return count;
}


////////////////////////////////////////////////////////////////////////////
// Image layers

/**
 * Create a new image array for this terrain, and returns it.
 */
vtImageLayer *vtTerrain::NewImageLayer()
{
	vtImageLayer *ilay = new vtImageLayer;
	m_Layers.push_back(ilay);
	return ilay;
}

void vtTerrain::CreateImageLayers()
{
	// Must have something to drape on
	if (!GetHeightField())
		return;

	// Go through the layers in the terrain parameters, and try to load them
	for (uint i = 0; i < m_Params.NumLayers(); i++)
	{
		if (m_Params.GetLayerType(i) != LT_IMAGE)
			continue;

		VTLOG(" Layer %d: Image\n", i);
		const vtTagArray &lay = m_Params.m_Layers[i];
		for (uint j = 0; j < lay.NumTags(); j++)
		{
			const vtTag *tag = lay.GetTag(j);
			VTLOG("   Tag '%s': '%s'\n", (const char *) tag->name, (const char *) tag->value);
		}

		vtImageLayer *im_layer = NewImageLayer();
		im_layer->SetProps(lay);
		if (im_layer->Load(m_progress_callback))
			AddMultiTextureOverlay(im_layer);
		else
			m_Layers.Remove(im_layer);
	}
}

//
// \param TextureMode One of GL_DECAL, GL_MODULATE, GL_BLEND, GL_REPLACE, GL_ADD
//
void vtTerrain::AddMultiTextureOverlay(vtImageLayer *im_layer)
{
	DRECT extents = im_layer->m_pImage->GetExtents();
	DRECT EarthExtents = GetHeightField()->GetEarthExtents();

	int iTextureUnit = m_TextureUnits.ReserveTextureUnit();

	VTLOG("vtTerrain::AddMultiTextureOverlay: using texture unit %d\n", iTextureUnit);

	if (iTextureUnit == -1)
		return;

	// Calculate the mapping of texture coordinates
	DPoint2 scale;
	FPoint2 offset;
	vtHeightFieldGrid3d *grid = GetDynTerrain();

	if (grid)
	{
		int iCols, iRows;
		grid->GetDimensions(iCols, iRows);

		// input values go from (0,0) to (Cols-1,Rows-1)
		// output values go from 0 to 1
		scale.Set(1.0/(iCols - 1), 1.0/(iRows - 1));

		// stretch the (0-1) over the data extents
		scale.x *= (EarthExtents.Width() / extents.Width());
		scale.y *= (EarthExtents.Height() / extents.Height());
	}
	else	// might be a TiledGeom, or a TIN
	{
		FRECT worldExtents;
		GetLocalCS().EarthToLocal(extents, worldExtents);

		// Map input values (0-terrain size in world coords) to 0-1
		scale.Set(1.0/worldExtents.Width(), 1.0/worldExtents.Height());
	}

	// and offset it to place it at the right place
	offset.x = (float) ((extents.left - EarthExtents.left) / extents.Width());
	offset.y = (float) ((extents.bottom - EarthExtents.bottom) / extents.Height());

	im_layer->m_pMultiTexture = new vtMultiTexture;
	im_layer->m_pMultiTexture->Create(GetTerrainSurfaceNode(), im_layer->m_pImage,
		scale, offset, iTextureUnit, GL_DECAL);
}

osg::Node *vtTerrain::GetTerrainSurfaceNode()
{
	if (GetDynTerrain())
		return GetDynTerrain();
	else if (GetTiledGeom())
		return GetTiledGeom();
	else if (GetTin())
		return GetTin()->GetGeometry();
	return NULL;
}


////////////////////////////////////////////////////////////////////////////
// Scenarios

void vtTerrain::ActivateScenario(int iScenario)
{
	ScenarioParams &ScenarioParams = m_Params.m_Scenarios[iScenario];
	vtStringArray &ActiveLayers = ScenarioParams.GetActiveLayers();

	uint iNumActiveLayers = ActiveLayers.size();

	for (uint i = 0; i < m_Layers.size(); i++)
	{
		vtLayer *lay = m_Layers[i];
		vtString Name = StartOfFilename(lay->GetLayerName());
		RemoveFileExtensions(Name);

		bool bFound = false;
		for (uint j = 0; j < iNumActiveLayers; j++)
			if (Name == ActiveLayers[j])
				bFound = true;
		lay->SetVisible(bFound);
	}
}


////////////////////////////////////////////////////////////////////////////
// Dynamic elevation

/**
 * If you have told vtTerrain to preserve a copy of the original elevation
 * grid in memory, you can modify that grid, then call this method to update
 * the CLOD surface.  This can take up a few seconds, depending on the size
 * of your grid.
 */
void vtTerrain::UpdateElevation()
{
	if (!m_pDynGeom)
		return;
	SRTerrain *sr = dynamic_cast<SRTerrain*>(m_pDynGeom.get());
	if (!sr)
		return;
	sr->ReInit(m_pElevGrid.get());
}

/**
 * Drape all the culture on the terrain again, to keep them on the surface
 * in the case when the elevation values have changed.
 *
 * \param area You can speed up this function by passing the area to re-drape
 *		in.  Otherwise, simply pass an empty area, and all culture will be
 *		re-draped.
 */
void vtTerrain::RedrapeCulture(const DRECT &area)
{
	// Tell the terrain to re-drape all its structure instances.
	for (uint i = 0; i < m_Layers.size(); i++)
	{
		vtStructureLayer *slay = dynamic_cast<vtStructureLayer *>(m_Layers[i].get());
		if (slay)
		{
			for (uint j = 0; j < slay->size(); j++)
			{
				vtStructure *st = slay->at(j);
				vtStructure3d *s3 = slay->GetStructure3d(j);

				// If we were given an area, omit structures outside it
				if (!area.IsEmpty() && !st->IsContainedBy(area))
					continue;

				// A fence might need re-draping, so we have to rebuild geometry
				vtFence3d *f3 = dynamic_cast<vtFence3d*>(s3);
				if (f3)
					f3->CreateNode(this);

				// A building's geometry will not change, only move up or down
				vtBuilding3d *b3 = dynamic_cast<vtBuilding3d*>(s3);
				if (b3)
					b3->AdjustHeight(m_pHeightField);

				// A instance's geometry will not change, only move up or down
				vtStructInstance3d *si = dynamic_cast<vtStructInstance3d*>(s3);
				if (si)
					si->UpdateTransform(m_pHeightField);

			}
		}
		vtVegLayer *vlay = dynamic_cast<vtVegLayer *>(m_Layers[i].get());
		if (vlay)
		{
			for (uint i = 0; i < vlay->NumEntities(); i++)
			{
				vlay->UpdateTransform(i);
			}
		}
	}
	// What else?  Abstract Layers. Roads, perhaps.
}

