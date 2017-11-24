//
// PickEngines.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

/**
 Used to cast a ray from the camera to the ground, and make that point
 available to code which needs to know it.
 */
class TerrainPicker : public vtMouseStateEngine
{
public:
	TerrainPicker();
	void Eval();
	void SetHeightField(const vtHeightField3d *pHeight) { m_pHeightField = pHeight; }

	bool GetCurrentPoint(FPoint3 &p);
	bool GetCurrentEarthPos(DPoint3 &p);

	void OnMouse(vtMouseEvent &event);

protected:
	void FindGroundPoint();

	FPoint3			m_GroundPoint;
	const vtHeightField3d *m_pHeightField;
	DPoint3			m_EarthPos;
	bool			m_bOnTerrain;
};

///////////////////////////////////////////////////

class vtIcoGlobe;

/**
 Used to cast a ray from the camera to the globe, and make that point
 available to code which needs to know it.
 */
class GlobePicker : public vtMouseStateEngine
{
public:
	GlobePicker();
	void Eval();
	void SetRadius(double fRadius) { m_fRadius = fRadius; }
	void SetGlobe(vtIcoGlobe *pGlobe) { m_pGlobe = pGlobe; }
	void SetTargetScale(float s) { m_fTargetScale = s; }

	bool GetCurrentPoint(FPoint3 &p);
	bool GetCurrentEarthPos(DPoint3 &p);

protected:
	FPoint3		m_GroundPoint;
	DPoint3		m_EarthPos;
	bool		m_bOnTerrain;
	double		m_fRadius;
	vtIcoGlobe	*m_pGlobe;
	float		m_fTargetScale;
};

