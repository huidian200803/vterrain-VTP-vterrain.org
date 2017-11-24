//
// MapOverviewEngine.h
//
// Copyright (c) 2004-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

#include "Terrain.h"
#include "ImageSprite.h"

/**
 * Provide a view of the terrain from the top
 * and display a cursor to show current canera position
 * User can move the camera by clicking on the map
 * with the mouse middle button.
 */
class MapOverviewEngine : public vtEngine
{
public:
	MapOverviewEngine(vtTerrain *pTerr);
	~MapOverviewEngine();

	void OnMouse(vtMouseEvent &event);
	void Eval();

	void ShowMapOverview(bool bShow) { m_pMapGroup->SetEnabled(bShow); }
	bool GetShowMapOverview() {return m_pMapGroup->GetEnabled();}
	void SetTerrain(vtTerrain *pTerr);

protected:
	void CreateMapView(vtTerrain *pTerr);
	void RefreshMapView();
	void CreateArrow();

	vtGroup			*m_pMapGroup;
	osg::ref_ptr<vtImageSprite>	m_pMapView;
	vtTransform		*m_pArrow;
	vtImagePtr		 m_pOwnedImage;

	float	anglePrec;
	int		MapWidth;
	int		MapMargin;
	float	MapRatio;
	float	ratioMapTerrain;
	bool	m_bDown;
};

