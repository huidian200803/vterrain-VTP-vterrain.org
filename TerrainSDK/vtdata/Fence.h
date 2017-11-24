//
// Fences.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef FENCESH
#define FENCESH

#include <stdio.h>
#include "MathTypes.h"
#include "Structure.h"

/**
 * These various style of linear structure are just for convenience; they can
 * be passed to vtFence::ApplyStyle() for a number of commonly-encountered
 * styles of structure.
 */
enum vtLinearStyle
{
	FS_WOOD_POSTS_WIRE,
	FS_METAL_POSTS_WIRE,
	FS_METAL_POSTS_HOGWIRE,
	FS_CHAINLINK,
	FS_SECURITY,
	FS_DRYSTONE,
	FS_STONE,
	FS_PRIVET,
	FS_BERM,
	FS_RAILING_ROW,
	FS_RAILING_CABLE,
	FS_RAILING_EU,
	FS_TOTAL
};

#define FENCE_DEFAULT_HEIGHT	1.2f
#define FENCE_DEFAULT_SPACING	2.5f

class vtLinearParams
{
public:
	void Defaults();
	void Blank();
	void ApplyStyle(vtLinearStyle style);
	void WriteXML(GZOutput &out) const;

	bool operator==(const vtLinearParams &rhs) const;

	// Posts
	vtString	m_PostType;			// wood, steel, none
	float		m_fPostHeight;
	float		m_fPostSpacing;
	float		m_fPostWidth;
	float		m_fPostDepth;
	vtString	m_PostExtension;	// left, right, double, none

	// Connect
	int			m_iConnectType;		// 0=none, 1=wire, 2=simple, 3=profile
	vtString	m_ConnectMaterial;	// chain-link, privet, drystone, etc.
	float		m_fConnectTop;
	float		m_fConnectBottom;
	float		m_fConnectWidth;
	short		m_iConnectSlope;	// slope on the sides of the connector
	bool		m_bConstantTop;		// top of connector is constant height
	vtString	m_ConnectProfile;	// filename of profile
};

/**
 * The vtFence class represents any linear structure.  This can be a fence,
 * a wall, a railing, or any other kind of build structure which follows
 * a line on the ground.
 */
class vtFence : public vtStructure
{
public:
	vtFence();

	// copy operator
	vtFence &operator=(const vtFence &v);

	// geometry methods
	void AddPoint(const DPoint2 &epos);
	DLine2 &GetFencePoints() { return m_pFencePts; }
	void SetFencePoints(const DLine2 &pts) { m_pFencePts = pts; }
	void GetClosestPoint(const DPoint2 &point, DPoint2 &closest);
	double GetDistanceToLine(const DPoint2 &point);
	int GetNearestPointIndex(const DPoint2 &point, double &fDist);
	bool IsContainedBy(const DRECT &rect) const;
	bool GetExtents(DRECT &rect) const;

	// IO
	void WriteXML(GZOutput &out, bool bDegrees) const;

	// style
	void ApplyStyle(vtLinearStyle style);
	vtLinearParams &GetParams() { return m_Params; }
	virtual void SetParams(const vtLinearParams &params) { m_Params = params; }
	virtual void ProfileChanged() {}

protected:
	DLine2		m_pFencePts;	// in earth coordinates

	vtLinearParams	m_Params;
};

// Helpers
bool LoadFLine2FromSHP(const char *fname, FLine2 &prof);
bool SaveFLine2ToSHP(const char *fname, const FLine2 &prof);

#endif	// FENCESH

