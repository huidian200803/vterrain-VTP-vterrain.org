//
// TerrainLayers.cpp
//
// Copyright (c) 2006-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/vtosg/MultiTexture.h"
#include "vtlib/core/Light.h"

#include "vtdata/DataPath.h"
#include "vtdata/vtLog.h"

#include "TParams.h"
#include "TerrainLayers.h"
#include "vtTin3d.h"
#include "SurfaceTexture.h"


////////////////////////////////////////////////////////////////////////////
// Structures

vtStructureLayer::vtStructureLayer() : vtLayer(LT_STRUCTURE)
{
	m_Props.SetValueString("Type", TERR_LTYPE_STRUCTURE, true);
}

/**
 * Attempt to load structures from its VTST file.
 */
bool vtStructureLayer::Load(bool progress_callback(int))
{
	vtString fname = m_Props.GetValueString("Filename");

	VTLOG("\tLooking for structures file: %s\n", (const char *) fname);
	vtString building_path = FindFileOnPaths(vtGetDataPath(), fname);
	if (building_path == "")
	{
		VTLOG("\tNot found.\n");
		vtString fname2 = "BuildingData/";
		fname2 += fname;

		VTLOG("\tLooking for structures file: %s\n", (const char *) fname2);
		building_path = FindFileOnPaths(vtGetDataPath(), fname2);
	}
	if (building_path == "")
	{
		VTLOG("\tNot found.\n");
		return false;
	}
	else
		VTLOG("\tFound: %s\n", (const char *) building_path);

	if (!ReadXML(building_path, progress_callback))
		return false;

	// If the user wants it to start hidden, hide it
	bool bVisible;
	if (m_Props.GetValueBool("visible", bVisible))
		SetEnabled(bVisible);

	return true;
}

void vtStructureLayer::SetLayerName(const vtString &fname)
{
	SetFilename(fname);
	m_Props.SetValueString("Filename", fname);
}


////////////////////////////////////////////////////////////////////////////
// Vegetation

vtVegLayer::vtVegLayer() : vtLayer(LT_VEG)
{
	m_Props.SetValueString("Type", TERR_LTYPE_VEGETATION, true);
}


////////////////////////////////////////////////////////////////////////////
// Imagery

vtImageLayer::vtImageLayer() : vtLayer(LT_IMAGE)
{
	m_Props.SetValueString("Type", TERR_LTYPE_IMAGE, true);

	m_pImage = new vtImageGeo;
	m_pMultiTexture = NULL;
}

vtImageLayer::~vtImageLayer()
{
	m_pMultiTexture = NULL;
}

void vtImageLayer::SetVisible(bool vis)
{
	if (m_pMultiTexture)
		m_pMultiTexture->Enable(vis);
	vtLayerBase::SetVisible(vis);
}

bool vtImageLayer::Load(bool progress_callback(int))
{
	vtString fname = m_Props.GetValueString("Filename");
	vtString path = FindFileOnPaths(vtGetDataPath(), fname);
	if (path == "")
	{
		vtString prefix = "GeoSpecific/";
		path = FindFileOnPaths(vtGetDataPath(), prefix+fname);
	}
	if (path == "")
	{
		VTLOG("Couldn't find image layer file '%s'\n", (const char *) fname);
		return false;
	}

	if (!m_pImage->ReadTIF(path, progress_callback))
	{
		VTLOG("Couldn't read image from file '%s'\n", (const char *) path);
		return false;
	}
	VTLOG("Read image from file '%s'\n", (const char *) path);

	DRECT extents = m_pImage->GetExtents();
	if (extents.IsEmpty())
	{
		VTLOG("Couldn't get extents from image, so we can't use it as an image overlay.\n");
		return false;
	}
	return true;
}


////////////////////////////////////////////////////////////////////////////
// Elevation Surfaces

vtElevLayer::vtElevLayer() : vtLayer(LT_ELEVATION)
{
	VTLOG1("vtElevLayer constructor\n");
	m_Props.SetValueString("Type", TERR_LTYPE_ELEVATION, true);
}

vtElevLayer::~vtElevLayer()
{
	VTLOG1("vtElevLayer destructor\n");
}

void vtElevLayer::SetLayerName(const vtString &fname)
{
	m_Props.SetValueString("Filename", fname);
}

vtString vtElevLayer::GetLayerName()
{
	return m_Props.GetValueString("Filename");
}

void vtElevLayer::SetVisible(bool vis)
{
	if (m_pTin.get() && m_pTin->GetGeometry())
	{
		m_pTin->GetGeometry()->SetEnabled(vis);
	}
	vtLayerBase::SetVisible(vis);
}

bool vtElevLayer::Load(bool progress_callback(int))
{
	vtString fname = m_Props.GetValueString("Filename");
	vtString path = FindFileOnPaths(vtGetDataPath(), fname);
	if (path == "")
	{
		vtString prefix = "Elevation/";
		path = FindFileOnPaths(vtGetDataPath(), prefix+fname);
	}
	if (path == "")
	{
		VTLOG("Couldn't find elevation layer file '%s'\n", (const char *) fname);
		return false;
	}

	m_pTin = new vtTin3d;
	if (!m_pTin->Read(path))
		return false;
	VTLOG("Read elevation layer from file '%s'\n", (const char *) path);

	return true;
}

void vtElevLayer::MakeMaterials(bool bTextureCompression)
{
	m_pTin->MakeMaterialsFromOptions(m_Props, bTextureCompression);
}

vtTransform *vtElevLayer::CreateGeometry()
{
	bool drop_shadow = false;
	m_pGeode = m_pTin->CreateGeometry(drop_shadow);
	m_pGeode->SetCastShadow(false);

	m_pTransform = new vtTransform;
	m_pTransform->addChild(m_pGeode);

	return m_pTransform;
}


////////////////////////////////////////////////////////////////////////////
// LayerSet

void LayerSet::Remove(vtLayer *lay)
{
	for (size_t i = 0; i < size(); i++)
	{
		if (lay == at(i))
		{
			erase(begin()+i);
			return;
		}
	}
}

vtLayer *LayerSet::FindByName(const vtString &name)
{
	for (size_t i = 0; i < size(); i++)
	{
		if (at(i)->GetLayerName() == name)
			return at(i);
	}
	return NULL;
}

vtStructureLayer *LayerSet::FindStructureFromNode(osg::Node *pNode, int &iOffset)
{
	iOffset = -1;
	size_t iNumLayers = size();
	bool bFound = false;

	for (size_t i = 0; i < iNumLayers && !bFound; i++)
	{
		vtStructureLayer *slay = dynamic_cast<vtStructureLayer *>(at(i).get());
		if (!slay)
			continue;
		int iNumStructures = slay->size();
		for (int j = 0; (j < iNumStructures) && !bFound; j++)
		{
			vtStructure3d *pStructure3d = slay->GetStructure3d(j);
			if (FindAncestor(pNode, pStructure3d->GetContainer()) ||
				FindAncestor(pNode, pStructure3d->GetContained()) ||
				FindAncestor(pNode, pStructure3d->GetGeom()))
			{
				iOffset = j;
				return slay;
			}
		}
	}
	return NULL;
}

