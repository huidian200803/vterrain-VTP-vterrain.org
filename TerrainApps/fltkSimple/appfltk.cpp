//
// Name:     appfltk.cpp
// Purpose:  Example FLTK/vtlib application
//   (tested only on Linux using FLTK 1.1.5)
//   (tested only on Windows using FLTK 1.1.6)
//
// Copyright (c) 2004-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
// Provided by Clark Borst (November 12, 2004)

#ifdef __FreeBSD__
#  include <ieeefp.h>
#endif

#include <iostream>

// the required FLTK headers for this example
// note that fltk seems to use capital H for cpp headers!
#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>
#include <GL/glu.h>

// the required VTP headers
#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/TerrainScene.h"
#include "vtlib/core/NavEngines.h"
#include "vtdata/DataPath.h"
#include "vtdata/vtLog.h"

// handy definitions for FLTK mouse buttons
#define FL_LEFT 1
#define FL_MIDDLE 2
#define FL_RIGHT 3

using namespace std;

// prototype of global idle callback function
void IdleCallback(void* pData);

// create a new FLTK window with OpenGL context by making
// a subclass of the Fl_Gl_Window class
class VTPWindow : public Fl_Gl_Window
{
	public:
		VTPWindow(int width, int height, char* title);
		virtual ~VTPWindow();
		void InitializeGL();
		virtual void draw();
		bool CreateScene();
		virtual int handle(int event);
	private:
		int screenwidth, screenheight;
};

// the idle function will continuously update (redraw) the window,
// even when there are no events
void IdleCallback(void* pData)
{
	if (pData != NULL)
	{
		VTPWindow* pWindow = reinterpret_cast<VTPWindow*>(pData);
		// execute the draw() function of the window by invoking
		// method redraw()
		pWindow->redraw();
	}
}

// constructor
VTPWindow::VTPWindow(int width, int height, char* title) : Fl_Gl_Window(width, height, title)
{
	mode(FL_RGB | FL_ALPHA | FL_DEPTH | FL_DOUBLE);
	// add the idle function to this window
	Fl::add_idle(IdleCallback, this);
}

// destructor
VTPWindow::~VTPWindow()
{
	//
}

// the draw() function must be overridden by your own
// drawing code. By invoking method redraw(), FLTK will
// call this draw() method
void VTPWindow::draw()
{
	static bool firstTime = true;
	if (firstTime)
	{
		// get the window size with Fl::Widget (parent class of Fl_Gl_Window) methods w() and h()
		screenwidth  = w();
		screenheight = h();
		// print the window size to the terminal
		VTLOG("screenwidth %d, screenheight %d\n", screenwidth, screenheight);
		// adjust the scene's viewport to the FL window size
		vtGetScene()->SetWindowSize(screenwidth, screenheight);
		// prevent this sequence from being executed over and over again
		firstTime = false;
	};

	// update the terrain scene
	vtGetScene()->DoUpdate();
}

// the event handler of the window, which takes mouse and keyboard events
// and converts them into VTerrain events
int VTPWindow::handle(int event)
{
	vtMouseEvent vtp_event;
	int x, y;
	uchar key;

	switch (event)
	{
		case FL_FOCUS:
		case FL_UNFOCUS:
		return 1;

		// process keyboard events for this window
		case FL_KEYBOARD:
			key = Fl::event_key();
			switch (key)
			{
				// ESC means quit the program
				case 27 :
					exit(0);
					return 1;
				// q means quit the program
				case 'q':
					exit(0);
					return 1;

				default:
					vtGetScene()->OnKey(key, 0);
					return 1;
			}
		// process mouse events for this window
		// First: handle a pushed button by checking
		//        which button has been pushed and
		//        convert it into VTerrain mouse buttons.
		case FL_PUSH:
			if(Fl::event_button() == FL_LEFT){
				vtp_event.button = VT_LEFT;
			}
			else if (Fl::event_button() == FL_MIDDLE){
				vtp_event.button = VT_MIDDLE;
			}
			else if (Fl::event_button() == FL_RIGHT){
				vtp_event.button = VT_RIGHT;
			}
			x = Fl::event_x();
			y = Fl::event_y();
			vtp_event.pos.Set(x, y);
			vtp_event.type = VT_DOWN;
			vtp_event.flags = 0;
			vtGetScene()->OnMouse(vtp_event);
			return 1;
		// Second: if a button has been pushed (i.e. FL_PUSH returned 1),
		//         and the mouse position changed while the button is still
		//         pushed, set the VTerrain tupe to MOVE and force a redraw()
		//	   of the window
		case FL_DRAG:
			x = Fl::event_x();
			y = Fl::event_y();
			vtp_event.type = VT_MOVE;
			vtp_event.button = VT_NONE;
			vtp_event.flags = 0;
			vtp_event.pos.Set(x, y);
			vtGetScene()->OnMouse(vtp_event);
			redraw();
			return 1;
		// Third: handle the mouse button being released
		case FL_RELEASE:
			if(Fl::event_button() == FL_LEFT){
				vtp_event.button = VT_LEFT;
			}
			else if (Fl::event_button() == FL_MIDDLE){
				vtp_event.button = VT_MIDDLE;
			}
			else if (Fl::event_button() == FL_RIGHT){
				vtp_event.button = VT_RIGHT;
			}
			vtp_event.type = VT_UP;
			x = Fl::event_x();
			y = Fl::event_y();
			vtp_event.flags = 0;
			vtp_event.pos.Set(x, y);
			vtGetScene()->OnMouse(vtp_event);
			return 1;
	}

	return Fl_Gl_Window::handle(event);
}

//
// Create the 3d terrain scene
//
bool VTPWindow::CreateScene()
{
	vtScene *pScene = vtGetScene();

	// Look up the camera
	vtCamera *pCamera = pScene->GetCamera();
	pCamera->SetHither(10);
	pCamera->SetYon(1000000);

	// The  terrain scene will contain all the terrains that are created.
	vtTerrainScene *ts = new vtTerrainScene;

	// Set the global data path
	vtStringArray paths;
	paths.push_back(vtString("G:/Data-Distro/"));
	paths.push_back(vtString("../../../Data/"));
	paths.push_back(vtString("../../Data/"));
	paths.push_back(vtString("../Data/"));
	paths.push_back(vtString("Data/"));
	vtSetDataPath(paths);

	// Begin creating the scene, including the sun and sky
	vtGroup *pTopGroup = ts->BeginTerrainScene();

	// Tell the scene graph to point to this terrain scene
	pScene->SetRoot(pTopGroup);

	// Create a new vtTerrain, read its parameters from a file
	vtTerrain *pTerr = new vtTerrain;
	pTerr->SetParamFile("Data/Simple.xml");
	pTerr->LoadParams();

	// Add the terrain to the scene, and contruct it
	ts->AppendTerrain(pTerr);
	if (!ts->BuildTerrain(pTerr))
	{
		printf("Terrain creation failed: %s\n",
			(const char *)pTerr->GetLastError());
		return false;
	}
	ts->SetCurrentTerrain(pTerr);

	// Create a navigation engine to move around on the terrain
	// Get flight speed from terrain parameters
	float fSpeed = pTerr->GetParams().GetValueFloat(STR_NAVSPEED);

	vtTerrainFlyer *pFlyer = new vtTerrainFlyer(fSpeed);
	pFlyer->SetTarget(pCamera);
	pFlyer->SetHeightField(pTerr->GetHeightField());
	pScene->AddEngine(pFlyer);

	// Minimum height over terrain is 100 m
	vtHeightConstrain *pConstrain = new vtHeightConstrain(100);
	pConstrain->SetTarget(pCamera);
	pConstrain->SetHeightField(pTerr->GetHeightField());
	pScene->AddEngine(pConstrain);

	return true;
}


//
//  The works.
//
int main(int argc, char **argv)
{
#ifdef __FreeBSD__
	/*  FreeBSD is more stringent with FP ops by default, and OSG is doing  */
	/*    silly things sqrt(Inf) (computing lengths of MAXFLOAT and NaN     */
	/*    Vec3's).   This turns off FP bug core dumps, ignoring the error   */
	/*    like most platforms do by default.                                */
	fpsetmask(0);
#endif

	// Log messages to make troubleshooting easier
	VTSTARTLOG("debug.txt");
	VTLOG("glutSimple\n");

	// Create an instance of the VTPWindow class, show it
	VTPWindow MyWindow(800,600,"FLTK and VTP example");
	MyWindow.show();

	// Get a handle to the vtScene - one is already created for you
	vtScene *pScene = vtGetScene();
	pScene->Init(argc, argv);

	pScene->getViewer()->setThreadingModel(osgViewer::Viewer::SingleThreaded);
    pScene->SetGraphicsContext(new osgViewer::GraphicsWindowEmbedded(0, 0, 800, 600));

	// create the VTP scene
	MyWindow.CreateScene();

	// Note: with FLTK you can create more instances of the same
	// VTPWindow class. SDL and GLUT allow to have only one active
	// window with OpenGL context, while FLTK can have more in which
	// each window will have its own callbacks.

	// go into the FLTK mainloop
	return(Fl::run());
}

