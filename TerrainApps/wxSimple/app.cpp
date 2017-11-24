//
// Name:     app.cpp
// Purpose:  The application class for a wxWidgets application.
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// Header for the vtlib library
#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/TerrainScene.h"
#include "vtlib/core/NavEngines.h"
#include "vtdata/DataPath.h"
#include "vtdata/vtLog.h"

#include "app.h"
#include "frame.h"

#include "vtui/Helper.h"
#include "canvas.h"

/* wxGTK and X11 multihtreading issues
   ===================================
   Although they have probably always been present, I have
   recently (08/2011) come across a number of X11 related multithreading
   issues when running on more recent versions of GTK+ (wxGTK)
   and X11 (XOrg) on multiprocessor systems. These all seem to
   relate to the use of modeless top level windows such as
   wxProgressDialog and multithreading OpenGL rendering, either
   individually or together. These issues can all be resolved by
   calling the X11 function XinitThreads before any other X
   related calls have been made. The following code is conditional
   on the use of wxGTK, but be aware these issues can occur
   whenever the X windowing system is used. In my view the making
   of such a low level windowing system call should be the responsibility
   of wxWidgets (and GTK+ if that is used) but that is not happening with
   current releases (08/2011). */
#if defined(__WXGTK__) && !defined(NO_XINITTHREADS)
IMPLEMENT_APP_NO_MAIN(vtApp)

int main(int argc, char *argv[])
{
    // I have decided to only call XInitThreads on multi processor systems.
    // However I believe that the same multithreading issues can arise on single
    // processor systems due to pre-emptive multi-tasking, albeit much more
    // rarely. The classic symptom of a X multithreading problem is the assert
    // xcb_io.c:140: dequeue_pending_request: Assertion `req == dpy->xcb->pending_requests' failed
    // or xcb_io.c .... Unknown request in queue while dequeuing
    // If you see anyhting like this on a single processor system then try commenting out this test.
    if (sysconf (_SC_NPROCESSORS_ONLN) > 1)
        XInitThreads();
    return wxEntry(argc, argv);

}
#else
IMPLEMENT_APP(vtApp)
#endif

//
// Initialize the app object
//
bool vtApp::OnInit(void)
{
	VTSTARTLOG("debug.txt");

	m_pTerrainScene = NULL;

	// Create the main frame window
	m_pFrame = new vtFrame(NULL, _T("Simple vtlib example"), wxPoint(50, 50), wxSize(800, 600));

	int MyArgc;
	char** MyArgv;
	ConvertArgcArgv(wxApp::argc, wxApp::argv, &MyArgc, &MyArgv);
	vtGetScene()->Init(MyArgc, MyArgv);

	m_pFrame->m_canvas->InitGraphicsWindowWX();

	// Make sure the scene knows the size of the canvas
	//  (on wxGTK, the first size events arrive too early before the Scene exists)
	wxSize canvas_size = m_pFrame->m_canvas->GetClientSize();
	vtGetScene()->SetWindowSize(canvas_size.x, canvas_size.y);

	return CreateScene();
}

//
// Create the 3d scene
//
bool vtApp::CreateScene()
{
	// Get a handle to the vtScene - one is already created for you
	vtScene *pScene = vtGetScene();

	// Look up the camera
	vtCamera *pCamera = pScene->GetCamera();
	pCamera->SetHither(10);
	pCamera->SetYon(100000);

	// The  terrain scene will contain all the terrains that are created.
	m_pTerrainScene = new vtTerrainScene;

	// Set the global data path to look in the many place the sample data might be
	vtStringArray paths;
	paths.push_back(vtString("G:/Data-Distro/"));
	paths.push_back(vtString("../../../Data/"));
	paths.push_back(vtString("../../Data/"));
	paths.push_back(vtString("../Data/"));
	paths.push_back(vtString("Data/"));
	vtSetDataPath(paths);

	// Begin creating the scene, including the sun and sky
	vtGroup *pTopGroup = m_pTerrainScene->BeginTerrainScene();

	// Tell the scene graph to point to this terrain scene
	pScene->SetRoot(pTopGroup);

	vtString pfile = FindFileOnPaths(vtGetDataPath(), "Terrains/Simple.xml");
	if (pfile == "")
	{
		VTLOG("Couldn't find terrain parameters Simple.xml\n");
		return false;
	}

	// Create a new vtTerrain, read its paramters from a file
	vtTerrain *pTerr = new vtTerrain;
	pTerr->SetParamFile(pfile);
	pTerr->LoadParams();

	// Add the terrain to the scene, and contruct it
	m_pTerrainScene->AppendTerrain(pTerr);
	if (!m_pTerrainScene->BuildTerrain(pTerr))
	{
		wxMessageBox(_T("Couldn't create the terrain.  Perhaps the elevation data file isn't in the expected location?"));
		return false;
	}
	m_pTerrainScene->SetCurrentTerrain(pTerr);

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

int vtApp::OnExit()
{
	if (m_pTerrainScene)
	{
		// Clean up the scene
		vtGetScene()->SetRoot(NULL);
		m_pTerrainScene->CleanupScene();
		delete m_pTerrainScene;
		vtGetScene()->Shutdown();
	}
	return 0;
}
