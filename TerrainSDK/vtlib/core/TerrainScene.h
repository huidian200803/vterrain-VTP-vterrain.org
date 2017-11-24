//
// TerrainScene.h
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TERRAINSCENEH
#define TERRAINSCENEH

#include "vtdata/FilePath.h"
#include "TimeEngines.h"
#include "Content3d.h"

// Forward references
class vtSkyDome;
class vtTerrain;
class vtTimeEngine;
class vtSkyTrackEngine;

/**
 * This class represents a single type of utility structure, such as a
 * telephone pole, power or transmission tower.
 */
class vtUtilStruct
{
public:
	vtUtilStruct()
	{
		m_pTower = NULL;
		m_iNumWires = 0;
	}

	osg::Node	*m_pTower;	// The station may have a tower placed on it
	vtString m_sStructName;

	// The points at which the wires attach
	FLine3 m_fpWireAtt1;
	FLine3 m_fpWireAtt2;

	int m_iNumWires;
};

/** \defgroup terrain Terrain
 * These classes are the terrain objects, which act as containers for
 * elevation, imagery, culture, and all other visual components of
 * a part of the earth.
 */
/*@{*/

/**
 * vtTerrainScene provides a container for all of your vtTerrain objects.
 *
 * It keeps track of which terrain is active, and allows you to switch
 * between terrains if you have more than one.  It also creates some of the
 * surrounding environment (sunlight, skydome) which is common to all
 * of the terrains.
 */
class vtTerrainScene : public vtTimeTarget
{
public:
	vtTerrainScene();
	virtual ~vtTerrainScene();

	// scene setup
	vtGroup *BeginTerrainScene();
	void AppendTerrain(vtTerrain *pTerrain);
	void Finish(const vtStringArray &datapath) {}	// dummy; obsolete method
	void SetCurrentTerrain(vtTerrain *pTerrain);
	vtGroup *BuildTerrain(vtTerrain *pTerrain);
	void RemoveTerrain(vtTerrain *pTerrain);
	void CleanupScene();

	vtGroup *GetTop() { return m_pTop; }
	vtSkyDome *GetSkyDome() { return m_pSkyDome; }
	void UpdateSkydomeForTerrain(vtTerrain *pTerrain);

	/// Access the terrains in the list.
	uint NumTerrains() { return m_Terrains.size(); }
	vtTerrain *GetTerrain(uint i) const { return m_Terrains[i]; }

	/// Get the terrain currently being shown.
	vtTerrain *GetCurrentTerrain() const { return m_pCurrentTerrain; }

	/// Find a terrain by its name.
	vtTerrain *FindTerrainByName(const char *name);
	vtTimeEngine *GetTimeEngine() { return m_pTimeEngine; }

	/// The Sun
	vtTransform *GetSunLightTransform() { return m_pSunLight; }
	vtLightSource *GetSunLightSource() { return m_pLightSource; }

	void SetTime(const vtTime &time);

	// utility structures, shared between all terrains
	vtUtilStruct *LoadUtilStructure(const vtString &name);
	float		m_fCatenaryFactor;

	// Statics
	// handle to the singleton
	static vtTerrainScene *s_pTerrainScene;

	// Global content manager for content shared between all terrains
	vtContentManager3d m_Content;

	// Overrides should return true if event processing should continue, false if
	// the event was taken.
	virtual bool OnMouse(vtMouseEvent &event) { return true; }

protected:
	// main scene graph outline
	vtGroupPtr	m_pTop;
	vtSkyDome	*m_pSkyDome;

	void _CreateSky();
	void _CreateEngines();

	vtGroup		*m_pAtmosphereGroup;

	// array of terrain objects
	std::vector<vtTerrain*> m_Terrains;

	// the terrain that is currently active
	vtTerrain	*m_pCurrentTerrain;

	// engines
	vtEngine			*m_pTerrainEngines;
	vtTimeEngine		*m_pTimeEngine;
	vtSkyTrackEngine	*m_pSkyTrack;

	// Utility structures, created on demand from global content file
	vtArray<vtUtilStruct*>	m_StructObjs;

	vtLightSource	*m_pLightSource;
	vtTransform		*m_pSunLight;
};

// global helper function
vtTerrainScene *vtGetTS();

/*@}*/	// Group terrain

#endif	// TERRAINSCENEH

