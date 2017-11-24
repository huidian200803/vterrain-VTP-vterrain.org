//
// Name:	 sdlSimple/app.cpp
// Purpose:  Example SDL/vtlib application.
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "SDL.h"
#ifdef __FreeBSD__
#  include <ieeefp.h>
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/TerrainScene.h"
#include "vtlib/core/NavEngines.h"
#include "vtdata/DataPath.h"
#include "vtdata/vtLog.h"

class App
{
public:
	App() { m_ts = NULL; m_pCamera = NULL; }
	bool CreateScene();

	void videosettings(bool same_video_mode, bool fullscreen);
	void display();
	void run();
	int main(int argc, char **argv);
	int  process_modifiers();
	void process_mouse_button(const SDL_Event &event);
	void process_mouse_motion(const SDL_Event &event);
	bool process_event(const SDL_Event &event);
	bool process_events();

public:
	vtTerrainScene *m_ts;
	vtCamera *m_pCamera;
};

//--------------------------------------------------------------------------

//
// Create the 3d scene: call vtlib to load the terrain and prepare for
// user interaction.
//
bool App::CreateScene()
{
	// Get a handle to the vtScene - one is already created for you
	vtScene *pScene = vtGetScene();

	// Look up the camera
	m_pCamera = pScene->GetCamera();
	m_pCamera->SetHither(10);
	m_pCamera->SetYon(100000);

	// The  terrain scene will contain all the terrains that are created.
	m_ts = new vtTerrainScene;

	// Set the global data path
	vtStringArray paths;
	paths.push_back(vtString("../Data/"));
	paths.push_back(vtString("Data/"));
	vtSetDataPath(paths);

	// Begin creating the scene, including the sun and sky
	vtGroup *pTopGroup = m_ts->BeginTerrainScene();

	// Tell the scene graph to point to this terrain scene
	pScene->SetRoot(pTopGroup);

	// Create a new vtTerrain, read its paramters from a file
	vtTerrain *pTerr = new vtTerrain;
	pTerr->SetParamFile("Data/Simple.xml");
	pTerr->LoadParams();

	// Add the terrain to the scene, and contruct it
	m_ts->AppendTerrain(pTerr);
	if (!m_ts->BuildTerrain(pTerr))
	{
		printf("Terrain creation failed.\n");
		return false;
	}
	m_ts->SetCurrentTerrain(pTerr);

	// Create a navigation engine to move around on the terrain
	// Get flight speed from terrain parameters
	float fSpeed = pTerr->GetParams().GetValueFloat(STR_NAVSPEED);

	vtTerrainFlyer *pFlyer = new vtTerrainFlyer(fSpeed);
	pFlyer->SetTarget(m_pCamera);
	pFlyer->SetHeightField(pTerr->GetHeightField());
	pScene->AddEngine(pFlyer);

	// Minimum height over terrain is 100 m
	vtHeightConstrain *pConstrain = new vtHeightConstrain(100);
	pConstrain->SetTarget(m_pCamera);
	pConstrain->SetHeightField(pTerr->GetHeightField());
	pScene->AddEngine(pConstrain);

#if 0
#include "C:/Dev/vtlibTests.cpp"
#endif

	return true;
}

void App::display()
{
	static int frame = 0;
	if (frame < 10)
	{
		frame++;
		VTLOG("Frame %d: ", frame);
		const GLubyte *ver = glGetString(GL_VERSION);
		if (ver != NULL)
			VTLOG1("Has context\n");
		else
		{
			VTLOG1("No context\n");
			return;
		}
	}

	vtGetScene()->DoUpdate();	// calls viewer->frame, etc.

	SDL_GL_SwapBuffers();
}

int App::process_modifiers()
{
	int flags = 0;
	SDLMod modifiers = SDL_GetModState();
	if (modifiers & (KMOD_LSHIFT | KMOD_RSHIFT)) flags |= VT_SHIFT;
	if (modifiers & (KMOD_LCTRL | KMOD_RCTRL))	 flags |= VT_CONTROL;
	if (modifiers & (KMOD_LALT | KMOD_RALT))	 flags |= VT_ALT;
	return flags;
}

void App::process_mouse_button(const SDL_Event &sdle)
{
	// turn SDL mouse button event into a VT mouse event
	vtMouseEvent event;
	event.type = (sdle.button.type == SDL_MOUSEBUTTONDOWN) ? VT_DOWN : VT_UP;

	if (sdle.button.button == 1)
		event.button = VT_LEFT;
	else if (sdle.button.button == 2)
		event.button = VT_MIDDLE;
	else if (sdle.button.button == 3)
		event.button = VT_RIGHT;

	event.flags = process_modifiers();
	event.pos.Set(sdle.button.x, sdle.button.y);

	vtGetScene()->OnMouse(event);
}

void App::process_mouse_motion(const SDL_Event &sdle)
{
	// turn SDL mouse move event into a VT mouse event
	vtMouseEvent event;
	event.type = VT_MOVE;
	event.button = VT_NONE;
	event.flags = process_modifiers();
	event.pos.Set(sdle.motion.x, sdle.motion.y);

	vtGetScene()->OnMouse(event);
}

bool App::process_event(const SDL_Event &event)
{
	int key;

	switch( event.type )
	{
		case SDL_QUIT:
			return true;;
		case SDL_KEYDOWN:
			break;
		case SDL_KEYUP:
			// turn SDL key event into a VT mouse event
			key = event.key.keysym.sym;
			if ( key == 27 /* ESC */ || key == 'q' || key == 'Q' )
				return true;
			vtGetScene()->OnKey(key, 0);
			break;
		case SDL_MOUSEMOTION:
			process_mouse_motion(event);
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			process_mouse_button(event);
			break;
		case SDL_VIDEORESIZE:
			// Tell vtlib
			vtGetScene()->SetWindowSize(event.resize.w, event.resize.h);
			break;
	}
	return false;
}

bool App::process_events()
{
	SDL_Event event;

	while ( SDL_PollEvent( &event ) )
	{
		if (process_event(event))
			return true;
	}
	return false;
}

void App::run()
{
	while ( true )
	{
		display();			// draw scene
		if (process_events())	// handle user events
			return;
	}
}

/*
  The works.
*/
int App::main(int argc, char **argv)
{
	// Log messages to make troubleshooting easier
	VTSTARTLOG("debug.txt");
	VTLOG("sdlSimple\n");

#ifdef __FreeBSD__
	/*  FreeBSD is more stringent with FP ops by default, and OSG is	*/
	/*	doing silly things sqrt(Inf) (computing lengths of MAXFLOAT		*/
	/*	and NaN Vec3's).   This turns off FP bug core dumps, ignoring	*/
	/*	the error like most platforms do by default.					*/
	fpsetmask(0);
#endif

    // init SDL
 	VTLOG("Initializing SDL..\n");
   if ( SDL_Init(SDL_INIT_VIDEO) < 0 )
    {
        VTLOG("Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);

    // Starting with SDL 1.2.10, passing in 0 will use the system's current resolution.
    unsigned int windowWidth = 0;
    unsigned int windowHeight = 0;

    // Passing in 0 for bitdepth also uses the system's current bitdepth. This works before 1.2.10 too.
    unsigned int bitDepth = 0;

	//videosettings(true, true);
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    // set up the surface to render to
    SDL_Surface* screen = SDL_SetVideoMode(windowWidth, windowHeight, bitDepth, SDL_OPENGL | SDL_FULLSCREEN | SDL_RESIZABLE);
    if ( screen == NULL )
    {
        std::cerr<<"Unable to set "<<windowWidth<<"x"<<windowHeight<<" video: %s\n"<< SDL_GetError()<<std::endl;
        exit(1);
    }

    SDL_EnableUNICODE(1);

    // If we used 0 to set the fields, query the values so we can pass it to osgViewer
    windowWidth = screen->w;
    windowHeight = screen->h;

	VTLOG("Initializing vtlib/OSG..\n");
	vtGetScene()->Init(argc, argv);
    vtGetScene()->getViewer()->setThreadingModel(osgViewer::Viewer::SingleThreaded);
    vtGetScene()->getViewer()->setUpViewerAsEmbeddedInWindow(0,0,windowWidth,windowHeight);
//    vtGetScene()->SetGraphicsContext(new osgViewer::GraphicsWindowEmbedded(0, 0, width, height));

	// Tell window size to vtlib
	vtGetScene()->SetWindowSize(windowWidth, windowHeight);

	VTLOG("Creating the terrain..\n");
	if (!CreateScene())
		return 0;

	VTLOG("Running..\n");
	run();

	VTLOG("Cleaning up..\n");
	vtGetScene()->SetRoot(NULL);
	if (m_ts)
		m_ts->CleanupScene();
	delete m_ts;
	vtGetScene()->Shutdown();

	return 0;
}

int main(int argc, char **argv)
{
#if WIN32 && defined(_MSC_VER) && DEBUG
	// sometimes, MSVC seems to need to be told to show unfreed memory on exit
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	App app;
	return app.main(argc, argv);
}
