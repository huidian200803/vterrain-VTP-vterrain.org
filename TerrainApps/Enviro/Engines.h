//
// Engines.h
//
// Engines used by Enviro
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ENVIRO_ENGINESH
#define ENVIRO_ENGINESH

#include "vtlib/core/NavEngines.h"

class TerrainPicker;

///////////////////////////////////////////////////

class FlatFlyer : public vtMouseStateEngine
{
public:
	FlatFlyer();

	void Eval();
	void OnMouse(vtMouseEvent &event);

protected:
	bool m_bDrag;
	bool m_bZoom;
	FPoint3 m_start_wp;
	IPoint2 m_startpos;
};

///////////////////////////////////////////////////

/**
 Grab-pivot navigation.  Grab the ground to move horizontally, orbit (pivot)
 the camera around a point on the ground to turn.
 */
class GrabFlyer : public vtTerrainFlyer
{
public:
	GrabFlyer(float fSpeed);
	void Eval();
	void OnMouse(vtMouseEvent &event);

	void SetTerrainPicker(TerrainPicker *pTP) { m_pTP = pTP; }

protected:
	bool m_bDrag;
	bool m_bPivot;
	bool m_bZoom;
	IPoint2 m_startpos;

	TerrainPicker *m_pTP;
	FPoint3 m_start_wp;
	FMatrix4 m_start_eye;

	// for dragging
	float m_fHeight;
};

#endif	// ENVIRO_ENGINESH

