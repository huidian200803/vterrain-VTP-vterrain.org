//
// Structure3d.cpp
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

#include "vtdata/vtLog.h"
#include "vtdata/DataPath.h"

#include "Structure3d.h"
#include "Building3d.h"
#include "Fence3d.h"
#include "Terrain.h"
#include "PagedLodGrid.h"

// Static members
vtMaterialDescriptorArray3d vtStructure3d::s_MaterialDescriptors;


///////////////////////////////////////////////////////////////////////

vtStructInstance3d::vtStructInstance3d() : vtStructInstance()
{
	m_pContainer = NULL;
	m_pHighlight = NULL;
	m_pModel = NULL;
}

void vtStructInstance3d::UpdateTransform(vtHeightField3d *pHeightField)
{
	if (!m_pContainer)	// safety check
		return;

	m_pContainer->Identity();

	if (m_fScale != 1.0f)
		m_pContainer->Scale(m_fScale);

	if (m_fRotation != 0.0f)
		m_pContainer->Rotate(FPoint3(0,1,0), m_fRotation);

	// convert earth -> XZ
	FPoint3 point;
	pHeightField->m_LocalCS.EarthToLocal(m_p, point.x, point.z);

	if (m_bAbsolute)
	{
		// Absolute means elevation is relative to sealevel
		point.y = m_fElevationOffset;
	}
	else
	{
		// Should we drape structure instances on all culture, including roads
		//  and other structures?  There are some cases where this is not
		//  desirable, such as buildings composed of multiple footprints which
		//  should intersect, not stack.  Should it be a user option, global,
		//  per-layer, or per-structure?

		//int iIncludeCulture = CE_ALL;
		int iIncludeCulture = CE_ROADS;

		// look up altitude
		pHeightField->FindAltitudeAtPoint(point, point.y, false, iIncludeCulture);

		// Elevation Offset is relative to the terrain surface
		point.y += m_fElevationOffset;
	}

	m_pContainer->SetTrans(point);
}

void vtStructInstance3d::Reload()
{
	// Is this method necessary?  Or can the object simply be reconstructed
	//  to produce the same effect?
}

/**
 * Display some bounding wires around the object to highlight it.
 */
void vtStructInstance3d::ShowBounds(bool bShow)
{
	if (bShow)
	{
		if (m_pModel)
		{
			if (!m_pHighlight)
			{
				// the highlight geometry doesn't exist, so create it
				// get bounding sphere
				FSphere sphere;
				s2v(m_pModel->getBound(), sphere);

				m_pHighlight = CreateBoundSphereGeode(sphere);
				m_pHighlight->SetCastShadow(false);		// no shadow
				m_pContainer->addChild(m_pHighlight);
			}
			m_pHighlight->SetEnabled(true);
		}
	}
	else
	{
		if (m_pHighlight)
			m_pHighlight->SetEnabled(false);
	}
}

// implement vtStructure3d methods
bool vtStructInstance3d::CreateNode(vtTerrain *pTerr)
{
	// if previously created, destroy to re-create
	bool bRecreating = false;
	if (m_pModel)
	{
		bRecreating = true;
		m_pContainer->removeChild(m_pModel);
		m_pModel = NULL;
	}

	const char *filename = GetValueString("filename", true);
	if (filename)
	{
		// relative path: look on the standard data paths
		vtString fullpath = FindFileOnPaths(vtGetDataPath(), filename);
		if (fullpath == "")
		{
			// also look in BuildingModels on data path
			vtString prefix = "BuildingModels/";
			fullpath = FindFileOnPaths(vtGetDataPath(), prefix+filename);
		}
		if (fullpath == "")
		{
			// still can't find it - give up.
			VTLOG("Couldn't find file '%s'\n", filename);
			return false;
		}
		// If they are re-creating this object, it's probably because
		//  the object has changed on disk, so force a reload
		bool bForce = bRecreating;

#if VTDEBUG
		VTLOG("Loading Model from '%s'\n", (const char *)fullpath);
#endif
		m_pModel = vtLoadModel(fullpath, !bForce);
		if (!m_pModel)
		{
			VTLOG("Couldn't load model from file '%s'\n", filename);
			return false;
		}
	}
	const char *itemname = GetValueString("itemname", true);
	if (itemname)
	{
		// Use ContentManager to create the structure, using the
		//  terrain's specific content manager
		vtContentManager3d &con = pTerr->m_Content;
		VTLOG("Looking in terrain's content (%x, %d items)\n", &con, con.NumItems());
		m_pModel = con.CreateNodeFromItemname(itemname);

		// If that didn't work, also try the global content manager
		if (!m_pModel)
			m_pModel = vtGetContent().CreateNodeFromItemname(itemname);

		if (!m_pModel)
			return false;
	}
	if (!m_pContainer)
	{
		// constructing for the first time
		m_pContainer = new vtTransform;
		m_pContainer->setName("instance container");
	}
	m_pContainer->addChild(m_pModel);

	float sc;
	if (GetValueFloat("scale", sc))
		m_fScale = sc;

	UpdateTransform(pTerr->GetHeightField());

	// Remember the radius for later
	FSphere sphere;
	s2v(m_pModel->getBound(), sphere);
	DPoint2 evector;
	pTerr->GetLocalCS().VectorLocalToEarth(sphere.radius, 0, evector);
	m_RadiusInEarthCoords = evector.x;

	return true;
}

bool vtStructInstance3d::IsCreated()
{
	return (m_pContainer != NULL);
}

void vtStructInstance3d::DeleteNode()
{
	if (m_pContainer)
	{
		if (m_pHighlight)
		{
			m_pContainer->removeChild(m_pHighlight);
			m_pHighlight = NULL;
		}
		if (m_pModel)
		{
			m_pContainer->removeChild(m_pModel);
			m_pModel = NULL;
		}
		m_pContainer = NULL;
	}
}

double vtStructInstance3d::DistanceToPoint(const DPoint2 &p, float fMaxRadius) const
{
	if (!m_pModel)
		return 1E9;	// Ignore instances that have no model

	if (m_pContainer && m_pModel)
	{
		// If we have the 3D model already loaded, we can return distance
		//  from the given point to the edge of the bounding sphere.  This
		//  makes objects easier to select, because their selectable zone
		//  is larger for larger objects.  This is a little messy, because
		//  it's a world-coord operation applied to a earth-coord value.
		if (m_RadiusInEarthCoords < fMaxRadius)
		{
			double dist_to_center = vtStructInstance::DistanceToPoint(p, fMaxRadius);
			return dist_to_center - m_RadiusInEarthCoords;
		}
		else
			return 1E9;	// Ignore instances with such a large radius
	}
	// otherwise, simple distance from the origin of this instance to the given point
	return vtStructInstance::DistanceToPoint(p, fMaxRadius);
}

///////////////////////////////////////////////////////////////////////
//
// vtStructureArray3d
//

vtStructureArray3d::vtStructureArray3d() : vtStructureArray()
{
	m_pTerrain = NULL;
}

vtBuilding *vtStructureArray3d::NewBuilding()
{
	return new vtBuilding3d;
}

vtFence *vtStructureArray3d::NewFence()
{
	return new vtFence3d;
}

vtStructInstance *vtStructureArray3d::NewInstance()
{
	return new vtStructInstance3d;
}

vtStructure3d *vtStructureArray3d::GetStructure3d(int i)
{
	// Safety check
	if (i < 0 || i >= (int) size())
		return NULL;

	vtStructure *str = at(i);

	// Due to the somewhat complicated structure of the multiple inheritance
	// here, we must do a double-cast: first cast down to the object's true
	// type, then back up to vtStructure3d.

	if (str->GetType() == ST_BUILDING)
		return (vtStructure3d *) (vtBuilding3d *) str;

	if (str->GetType() == ST_LINEAR)
		return (vtStructure3d *) (vtFence3d *) str;

	if (str->GetType() == ST_INSTANCE)
		return (vtStructure3d *) (vtStructInstance3d *) str;

	return NULL;
}

bool vtStructureArray3d::ConstructStructure(vtStructure3d *str)
{
	return str->CreateNode(m_pTerrain);
}

bool vtStructureArray3d::ConstructStructure(int index)
{
	vtStructure3d *str = GetStructure3d(index);
	if (str)
		return str->CreateNode(m_pTerrain);
	return false;
}

void vtStructureArray3d::OffsetSelectedStructures(const DPoint2 &offset)
{
	vtStructure *str;
	for (uint i = 0; i < size(); i++)
	{
		str = at(i);
		if (!str->IsSelected())
			continue;
		if (str->GetType() == ST_BUILDING)
		{
			vtBuilding3d *bld = GetBuilding(i);
			bld->Offset(offset);
			bld->AdjustHeight(m_pTerrain->GetHeightField());

			// Should really move the building to a new cell in the LOD
			// Grid, but unless it's moving really far we don't need to
			// worry about this.
		}
		if (str->GetType() == ST_LINEAR)
		{
			// vtFence3d *fen = GetFence(i);
			// TODO: implement moving of fences?
		}
		if (str->GetType() == ST_INSTANCE)
		{
			vtStructInstance3d *inst = GetInstance(i);
			inst->Offset(offset);
			inst->UpdateTransform(m_pTerrain->GetHeightField());
		}
	}
}

void vtStructureArray3d::OffsetSelectedStructuresVertical(float offset)
{
	vtStructure *str;
	for (uint i = 0; i < size(); i++)
	{
		str = at(i);
		if (!str->IsSelected())
			continue;
		if (str->GetType() == ST_BUILDING)
		{
			vtBuilding3d *bld = GetBuilding(i);
			bld->SetElevationOffset(bld->GetElevationOffset() + offset);
			bld->AdjustHeight(m_pTerrain->GetHeightField());
		}
		if (str->GetType() == ST_INSTANCE)
		{
			vtStructInstance3d *inst = GetInstance(i);
			inst->SetElevationOffset(inst->GetElevationOffset() + offset);
			inst->UpdateTransform(m_pTerrain->GetHeightField());
		}
	}
}

void vtStructureArray3d::VisualDeselectAll()
{
	for (uint i = 0; i < size(); i++)
	{
		vtStructure *str = (vtStructure *) at(i);
		vtStructure3d *str3d = GetStructure3d(i);

		str->Select(false);
		str3d->ShowBounds(false);
	}
}

void vtStructureArray3d::SetEnabled(bool bTrue)
{
	for (uint j = 0; j < size(); j++)
	{
		vtStructure3d *str3d = GetStructure3d(j);
		if (str3d)
		{
			// Hide the structure's whole container, which includes the node
			//  and highlight and any other nodes associated with it.
			vtTransform *pContainer = str3d->GetContainer();
			if (pContainer)
				pContainer->SetEnabled(bTrue);
			else
			{
				vtGeode *geode = str3d->GetGeom();
				if (geode)
					geode->SetEnabled(bTrue);
			}
		}
	}
	m_bEnabled = bTrue;
	if (m_pTerrain)
	{
		vtPagedStructureLodGrid *paged = m_pTerrain->GetStructureLodGrid();
		if (paged)
		{
			if (bTrue == false)
				// don't keep paging in structures for this array
				paged->ClearQueue(this);
			else
				// re-check paging for this array, now that it's visible
				paged->RefreshPaging(this);
		}
	}
}

void vtStructureArray3d::SetShadows(bool bTrue)
{
	for (uint j = 0; j < size(); j++)
	{
		vtStructure3d *str3d = GetStructure3d(j);
		if (str3d)
		{
			// Set shadow
			vtTransform *pContainer = str3d->GetContainer();
			if (pContainer)
				pContainer->SetCastShadow(bTrue);
		}
	}
}

//
// Be informed of edit hightlighting
//
void vtStructureArray3d::SetEditedEdge(vtBuilding *bld, int lev, int edge)
{
	vtStructure3d *str1, *str2;

	if (m_pEditBuilding && m_pEditBuilding != bld)
	{
		m_pEditBuilding->RemoveTag("level");
		m_pEditBuilding->RemoveTag("edge");
		str1 = (vtStructure3d *) (vtBuilding3d *) m_pEditBuilding;
		ConstructStructure(str1);
	}

	vtStructureArray::SetEditedEdge(bld, lev, edge);

	if (m_pEditBuilding)
	{
		m_pEditBuilding->SetValueInt("level", m_iEditLevel);
		m_pEditBuilding->SetValueInt("edge", m_iEditEdge);
		str2 = (vtStructure3d *) (vtBuilding3d *) m_pEditBuilding;
		ConstructStructure(str2);
	}
}


void vtStructureArray3d::DestroyStructure(int i)
{
	// Need to destroy the 3D geometry for this structure
	vtStructure3d *st3d = GetStructure3d(i);
	st3d->DeleteNode();
}

void vtStructureArray3d::WriteStructuresToOBJ(const char *filename)
{
	vtOBJFile file;
	bool initialized = false;
	FPoint3 origin;

	for (uint i = 0; i < size(); i++)
	{
		//vtBuilding3d *bld = GetBuilding(i);
		vtStructure3d *s3d = GetStructure3d(i);
		if (!s3d)
			continue;
		vtTransform *trans = s3d->GetContainer();
		if (!trans)
			continue;
		vtGeode *geode = s3d->GetGeom();
		if (!geode)
			continue;
		if (!initialized)
		{
			if (!file.Begin(geode->GetMaterials(), filename))
				return;
			origin = trans->GetTrans();
			initialized = true;
		}
		FPoint3 offset = trans->GetTrans() - origin;
		file.WriteGeode(geode, offset);
	}
	file.Close();
}

/////////////////////////////////////////////////////////////////////////////
// Methods for vtStructure3d
//

bool vtStructure3d::s_bMaterialsInitialized = false;

void vtStructure3d::InitializeMaterialArrays()
{
	if (!s_bMaterialsInitialized)
	{
		s_bMaterialsInitialized = true;

		s_MaterialDescriptors.InitializeMaterials();

		// Now load external materials (user-modifiable, user-extendable)
		s_MaterialDescriptors.LoadExternalMaterials();

		SetGlobalMaterials(&s_MaterialDescriptors);
	}
}

void vtStructure3d::SetCastShadow(bool b)
{
	if (m_pContainer.valid())
		m_pContainer->SetCastShadow(b);
}

bool vtStructure3d::GetCastShadow()
{
	if (m_pContainer.valid())
		return m_pContainer->GetCastShadow();
	else
		return false;
}

