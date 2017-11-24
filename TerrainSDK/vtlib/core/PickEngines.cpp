//
// PickEngines.cpp
//
// Engines used by Enviro
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "PickEngines.h"
#include "vtdata/HeightField.h"
#include "Globe.h"

//////////////////////////////////////////////////////////////////////

//
// Terrain picking ability
//
TerrainPicker::TerrainPicker() : vtMouseStateEngine()
{
	m_pHeightField = NULL;
	m_bOnTerrain = false;
}

void TerrainPicker::OnMouse(vtMouseEvent &event)
{
	vtMouseStateEngine::OnMouse(event);

	FindGroundPoint();
}

void TerrainPicker::FindGroundPoint()
{
	if (!m_pHeightField)
		return;

	FPoint3 pos, dir, result;

	vtGetScene()->CameraRay(m_pos, pos, dir);

	// test whether we hit the heightfield
	m_bOnTerrain = m_pHeightField->CastRayToSurface(pos, dir, result);
	if (!m_bOnTerrain)
		return;

	for (uint i = 0; i < NumTargets(); i++)
	{
		vtTransform *pTarget = (vtTransform *) GetTarget(i);
		pTarget->SetTrans(result);
	}

	// save result
	m_GroundPoint = result;

	// Find corresponding earth coordinates
	m_pHeightField->m_LocalCS.LocalToEarth(m_GroundPoint, m_EarthPos);
}

void TerrainPicker::Eval()
{
	// Don't calculate here, since we can get OnMouse events out of synch
	//  with the Paint events that trigger the scene update and engine Eval().
}

bool TerrainPicker::GetCurrentPoint(FPoint3 &p)
{
	if (m_bOnTerrain)
		p = m_GroundPoint;
	return m_bOnTerrain;
}

bool TerrainPicker::GetCurrentEarthPos(DPoint3 &p)
{
	if (m_bOnTerrain)
		p = m_EarthPos;
	return m_bOnTerrain;
}


//
// Globe picking ability
//
GlobePicker::GlobePicker() : vtMouseStateEngine()
{
	m_fRadius = 1.0;
	m_bOnTerrain = false;
	m_pGlobe = NULL;
	m_fTargetScale = 0.01f;
}

void GlobePicker::Eval()
{
	FPoint3 pos, dir;

	vtGetScene()->CameraRay(m_pos, pos, dir);

	// test whether we hit the globe
	FSphere sphere(FPoint3(0.0f, 0.0f, 0.0f), (float)m_fRadius);
	FPoint3 akPoint[2];
	int riQuantity;

	m_bOnTerrain = RaySphereIntersection(pos, dir, sphere, riQuantity, akPoint);
	if (m_bOnTerrain)
	{
		// save result
		m_GroundPoint = akPoint[0];

		// apply global position to target (which is not a child of the globe)
		vtTransform *pTarget = (vtTransform *) GetTarget();
		if (pTarget)
		{
			pTarget->Identity();
			pTarget->SetTrans(m_GroundPoint);
			pTarget->PointTowards(m_GroundPoint * 2.0f);
			pTarget->Scale(m_fTargetScale);
		}

		if (m_pGlobe)
		{
			// rotate to find position relative to globe's rotation
			vtTransform *xform = m_pGlobe->GetTop();
			FMatrix4 rot;
			xform->GetTransform(rot);
			FMatrix4 inverse;
			inverse.Invert(rot);
			FPoint3 newpoint;

			// work around SML bug: matrices flagged as identity but
			// will still transform by their components
			if (! inverse.IsIdentity())
			{
				inverse.Transform(m_GroundPoint, newpoint);
				m_GroundPoint = newpoint;
			}
		}

		// Find corresponding geographic coordinates
		xyz_to_geo(m_fRadius, m_GroundPoint, m_EarthPos);
	}
}

bool GlobePicker::GetCurrentPoint(FPoint3 &p)
{
	if (m_bOnTerrain)
		p = m_GroundPoint;
	return m_bOnTerrain;
}

bool GlobePicker::GetCurrentEarthPos(DPoint3 &p)
{
	if (m_bOnTerrain)
		p = m_EarthPos;
	return m_bOnTerrain;
}

