//
// UIContext.h
//
// Copyright (c) 2001-2015 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

#include "vtdata/Building.h"
#include "vtdata/Fence.h"
#include "vtdata/MathTypes.h"

class LinkEdit;

//
// Left-button modes
//
enum LBMode {
	LB_None,	// none
	LB_Link,	// select/edit links
	LB_Node,	// select/edit nodes
	LB_Move,	// move selected nodes
	LB_Pan,		// pan the view
	LB_Dist,	// measure distance
	LB_Mag,		// zoom into rectangle
	LB_Dir,		// show/change road direction
	LB_LinkEdit,	// edit the points of a road centerline
	LB_LinkExtend,  //extend a link selection,
	LB_TSelect,		// select elevation layer
	LB_Box,			// set area box
	LB_FSelect,		// select feature (raw or structure)
	LB_BldEdit,		// edit built structures
	LB_BldAddPoints,		// add footprint points
	LB_BldDeletePoints,		// delete footprint points
	LB_AddLinear,	// structures: add linear features
	LB_EditLinear,	// structures: add linear features
	LB_AddInstance,	// structures: add structure instances
	LB_AddPoints,	// add raw points
	LB_TowerSelect,	// selectTowers
	LB_TowerAdd,	// Add Tower to layer
	LB_FeatInfo,	// Get info about features by picking them
	LB_TrimTIN		// Trim TIN triangles by indicating them
};

struct UIContext
{
	bool	m_bLMouseButton;
	bool	m_bMMouseButton;
	bool	m_bRMouseButton;

	bool	m_bRubber;

	// Used while editing buildings
	vtBuilding	*m_pCurBuilding, m_EditBuilding;
	int		m_iCurCorner;
	bool	m_bDragCenter;
	bool	m_bRotate;
	bool	m_bControl;
	bool	m_bShift;
	bool	m_bAlt;

	// Used for linear structures
	vtFence *m_pCurLinear, m_EditLinear;

	// Used while editing roads
	LinkEdit *m_pEditingRoad;
	int		m_iEditingPoint;

	// Left Mouse Button Mode
	LBMode	mode;

	// Mouse in world coordinates
	DPoint2 m_DownLocation;
	DPoint2 m_CurLocation;
	DPoint2 m_LastLocation;

	// Mouse in canvas coords
	wxPoint	m_DownPoint;
	wxPoint m_CurPoint;		// current position of mouse
	wxPoint m_LastPoint;	// last position of mouse

	bool m_bDistanceToolMode;
};

#pragma once
