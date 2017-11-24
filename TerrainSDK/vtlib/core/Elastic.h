//
// Elastic.h
//
// Copyright (c) 2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

/** \addtogroup utility */
/*@{*/

/**
 A polyline geometry that is easy to use, easy to move around on a terrain
 surface in an elastic way.
 */
class ElasticPolyline
{
public:
	ElasticPolyline();

	void SetTerrain(vtTerrain *pTerr);
	void SetPostHeight(float fMeters) { m_fPostHeight = fMeters; }
	void SetLineHeight(float fMeters) { m_fLineHeight = fMeters; }
	void SetLineColor(const RGBAf &color);

	void SetPolyline(const DLine2 &line);
	void AddPoint(const DPoint2 &point, bool bEnforceRightAngles = false);
	void RemovePoint(int iPoint);
	void SetPoint(int iPoint, const DPoint2 &point, bool bEnforceRightAngles = false);
	void EnforceRightAngles();
	void Clear();

	DLine2 &GetPolyline() { return m_Line; }
	uint NumPoints() { return m_Line.GetSize(); }

protected:
	void Realize();

	vtMaterialArrayPtr m_Materials;
	vtGroupPtr	m_Container;
	DLine2		m_Line;
	vtGeodePtr m_Marker;
	vtTerrain *m_pTerr;
	float		m_fPostHeight;
	float		m_fLineHeight;
};

/*@}*/  // utility
