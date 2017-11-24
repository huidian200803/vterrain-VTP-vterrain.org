//
// Name:     app.cpp
// Purpose:  Example OSG-VTP application.
//
// Copyright (c) 2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

#include <osgViewer/Viewer>

#include "vtlib/core/Terrain.h"
#include "vtlib/core/TerrainScene.h"
#include "vtlib/core/NavEngines.h"
#include "vtlib/vtosg/OSGEventHandler.h"
#include "vtdata/DataPath.h"
#include "vtdata/vtLog.h"

vtTerrainScene *g_terrscene;

//
// Create the 3d scene
//
bool CreateScene()
{
	// Get a handle to the vtScene - one is already created for you
	vtScene *pScene = vtGetScene();

	// Look up the camera
	vtCamera *pCamera = pScene->GetCamera();
	pCamera->SetHither(10);
	pCamera->SetYon(100000);

	// The  terrain scene will contain all the terrains that are created.
	g_terrscene = new vtTerrainScene;

	// Set the global data path to look in the many places the sample data might be
	vtStringArray paths;
	paths.push_back(vtString("G:/Data-Distro/"));
	paths.push_back(vtString("../../../Data/"));
	paths.push_back(vtString("../../Data/"));
	paths.push_back(vtString("../Data/"));
	paths.push_back(vtString("Data/"));
	vtSetDataPath(paths);

	// Begin creating the scene, including the sun and sky
	vtGroup *pTopGroup = g_terrscene->BeginTerrainScene();

	// Tell the scene graph to point to this terrain scene
	pScene->SetRoot(pTopGroup);

	vtString pfile = FindFileOnPaths(vtGetDataPath(), "Terrains/Simple.xml");
	if (pfile == "")
	{
		printf("Couldn't find terrain parameters Simple.xml\n");
		return false;
	}

	// Create a new vtTerrain, read its parameters from a file
	vtTerrain *pTerr = new vtTerrain;
	pTerr->SetParamFile(pfile);
	pTerr->LoadParams();

	// Add the terrain to the scene, and contruct it
	g_terrscene->AppendTerrain(pTerr);
	if (!g_terrscene->BuildTerrain(pTerr))
	{
		printf("Terrain creation failed: %s\n", (const char *)pTerr->GetLastError());
		return false;
	}
	g_terrscene->SetCurrentTerrain(pTerr);

	// Create a navigation engine to move around on the terrain
	// Get flight speed from terrain parameters
	float fSpeed = pTerr->GetParams().GetValueFloat(STR_NAVSPEED);

	vtTerrainFlyer *pFlyer = new vtTerrainFlyer(fSpeed);
	pFlyer->AddTarget(pCamera);
	pFlyer->SetHeightField(pTerr->GetHeightField());
	pScene->AddEngine(pFlyer);

	// Minimum height over terrain is 100 m
	vtHeightConstrain *pConstrain = new vtHeightConstrain(100);
	pConstrain->AddTarget(pCamera);
	pConstrain->SetHeightField(pTerr->GetHeightField());
	pScene->AddEngine(pConstrain);

	VTLOG("Done creating scene.\n");
	return true;
}

/*
  The works.
  */
int main(int argc, char ** argv)
{
#if WIN32 && defined(_MSC_VER) && VTDEBUG
	// sometimes, MSVC seems to need to be told to show unfreed memory on exit
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// Log messages to make troubleshooting easier
	VTSTARTLOG("debug.txt");
	VTLOG("osgViewerSimple\n");

	// Make a scene and a viewer:
	vtGetScene()->Init(argc, argv);
	osgViewer::Viewer *viewer = vtGetScene()->getViewer();

	// Add a handler for GUI events.
	osg::ref_ptr<vtOSGEventHandler> pHandler = new vtOSGEventHandler;
	viewer->addEventHandler(pHandler);

	// We must call realize to be certain that a display/context is set up.
	viewer->realize();

	// Tell our scene about OSG's context.
	vtGetScene()->SetGraphicsContext(viewer->getCamera()->getGraphicsContext());

	// Only then can we safely get window size.
	vtGetScene()->GetWindowSizeFromOSG();

	printf("Creating the terrain..\n");
	CreateScene();

	printf("Running..\n");
	while (!viewer->done())
		vtGetScene()->DoUpdate();		// calls viewer::frame

	g_terrscene->CleanupScene();
	delete g_terrscene;

	vtGetScene()->Shutdown();

	return 0;
}

