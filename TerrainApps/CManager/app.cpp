//
// Name:     app.cpp
// Purpose:  The application class the CManager application.
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// Header for the vtlib librarys
#include "vtlib/vtlib.h"
#include "vtlib/core/NavEngines.h"
#include "vtui/Helper.h"	// for ConvertArgcArgv
#include "vtui/LogCatcher.h"

#include "app.h"
#include "frame.h"
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
    m_pTrackball = NULL;

	// Redirect the wxWidgets log messages to our own logging stream
	wxLog *logger = new LogCatcher;
	wxLog::SetActiveTarget(logger);

	VTSTARTLOG("debug.txt");
	VTLOG("CManager\n");

	VTLOG("Setup scene\n");
	vtScene *pScene = vtGetScene();

	//
	// Create the main frame window
	//
	VTLOG("Creating frame\n");
	vtFrame *frame = new vtFrame(NULL, _T("Content Manager"),
		wxPoint(50, 50), wxSize(800, 600));

	int MyArgc;
	char** MyArgv;
	ConvertArgcArgv(wxApp::argc, wxApp::argv, &MyArgc, &MyArgv);
	pScene->Init(MyArgc, MyArgv);

	frame->m_canvas->InitGraphicsWindowWX();

	pScene->SetBgColor(RGBf(0.5f, 0.5f, 0.5f));		// grey

	// Make sure the scene knows the size of the canvas
	//  (on wxGTK, the first size events arrive too early before the Scene exists)
	wxSize canvas_size = frame->m_canvas->GetClientSize();
	pScene->SetWindowSize(canvas_size.x, canvas_size.y);

	VTLOG(" getting camera\n");
	vtCamera *pCamera = pScene->GetCamera();
	pCamera->setName("Default Camera");

	m_pRoot = new vtGroup;
	m_pRoot->setName("Root");
	pScene->SetRoot(m_pRoot);

	// make a simple directional light
	VTLOG(" creating light\n");
	vtLightSource *pLight = new vtLightSource(0);
	pLight->setName("Light");
	vtTransform *pMovLight = new vtTransform;
	pMovLight->addChild(pLight);
	pMovLight->setName("Movable Light");
	pLight->SetAmbient(RGBf(1, 1, 1));
	pLight->SetDiffuse(RGBf(1, 1, 1));
	pLight->SetSpecular(RGBf(1, 1, 1));
	pMovLight->SetDirection(FPoint3(-0.2, -0.4, -0.9));
	m_pRoot->addChild(pMovLight);

	frame->UseLight(pMovLight);

	// make a trackball controller for the camera
	VTLOG(" creating trackball\n");
	m_pTrackball = new vtTrackball(3.0f);
	m_pTrackball->AddTarget(pScene->GetCamera());
	m_pTrackball->setName("Trackball");
	m_pTrackball->SetRotateButton(VT_LEFT, 0);
	m_pTrackball->SetZoomButton(VT_LEFT|VT_RIGHT, 0);
	m_pTrackball->SetZoomScale(3000.0f);
	m_pTrackball->SetTranslateButton(VT_RIGHT, 0);
	pScene->AddEngine(m_pTrackball);

	// Memleak Testing
//	GetMainFrame()->AddModelFromFile("E:/3D/Sample FLT files/spitfire.flt");
//	NodePtr pNode = vtLoadModel("E:/3D/Sample FLT files/spitfire.flt");
//	if (pNode.valid())
//		m_pRoot->addChild(pNode);
//	GetMainFrame()->AddNewItem();

//	osg::ref_ptr<osg::Node> node = osgDB::readNodeFile("E:/3D/Sample FLT files/spitfire.flt");
//	node = NULL;

	VTLOG(" end of OnInit\n");
	return true;
}


int vtApp::OnExit(void)
{
	VTLOG("App OnExit\n");

	vtGetScene()->SetRoot(NULL);
	m_pRoot = NULL;

	vtGetScene()->Shutdown();
	return 0;
}

