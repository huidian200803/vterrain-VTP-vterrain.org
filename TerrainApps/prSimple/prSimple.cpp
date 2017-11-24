//
// prSimple.cpp : A simple console application which demonstrates how to use
//		vtlib with the OpenProducer library.
//
// Copyright (c) 2004-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <Producer/KeyboardMouse>

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/TerrainScene.h"
#include "vtlib/core/NavEngines.h"
#include "vtdata/vtLog.h"


bool CreateScene()
{
	// Get a handle to the vtScene - one is already created for you
	vtScene *pScene = vtGetScene();
	pScene->Init();

	// Log messages to make troubleshooting easier
	VTSTARTLOG("debug.txt");
	VTLOG("prSimple\n");
	VTLOG("Creating the terrain..\n");

	// Look up the camera
	vtCamera *pCamera = pScene->GetCamera();
	pCamera->SetHither(10);
	pCamera->SetYon(100000);

	// The  terrain scene will contain all the terrains that are created.
	vtTerrainScene *ts = new vtTerrainScene;

	// Set the global data path
	vtStringArray paths;
	paths.push_back(vtString("Data/"));
	pScene->SetDataPath(paths);

	// Begin creating the scene, including the sun and sky
	vtGroup *pTopGroup = ts->BeginTerrainScene();

	// Tell the scene graph to point to this terrain scene
	pScene->SetRoot(pTopGroup);

	// Create a new vtTerrain, read its paramters from a file
	vtTerrain *pTerr = new vtTerrain;
	pTerr->SetParamFile("Data/Simple.xml");
	pTerr->LoadParams();

	// Add the terrain to the scene, and contruct it
	ts->AppendTerrain(pTerr);
	if (!ts->BuildTerrain(pTerr))
	{
		printf("Terrain creation failed.");
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

vtKeyCode ProducerToVT(Producer::KeyCharacter key)
{
	switch (key)
	{
	case Producer::KeyChar_BackSpace: return (vtKeyCode) 8;
	case Producer::KeyChar_Tab: return (vtKeyCode) 9;
	case Producer::KeyChar_Linefeed: return (vtKeyCode) 10;
	case Producer::KeyChar_Return: return (vtKeyCode) 13;
	case Producer::KeyChar_Escape: return (vtKeyCode) 27;

	case Producer::KeyChar_Home: return VTK_HOME;
	case Producer::KeyChar_Left: return VTK_LEFT;
	case Producer::KeyChar_Up: return VTK_UP;
	case Producer::KeyChar_Right: return VTK_RIGHT;
	case Producer::KeyChar_Down: return VTK_DOWN;
	case Producer::KeyChar_Page_Up: return VTK_PAGEUP;
	case Producer::KeyChar_Page_Down: return VTK_PAGEDOWN;
	case Producer::KeyChar_End: return VTK_END;

	case Producer::KeyChar_KP_0: return VTK_NUMPAD0;
	case Producer::KeyChar_KP_1: return VTK_NUMPAD1;
	case Producer::KeyChar_KP_2: return VTK_NUMPAD2;
	case Producer::KeyChar_KP_3: return VTK_NUMPAD3;
	case Producer::KeyChar_KP_4: return VTK_NUMPAD4;
	case Producer::KeyChar_KP_5: return VTK_NUMPAD5;
	case Producer::KeyChar_KP_6: return VTK_NUMPAD6;
	case Producer::KeyChar_KP_7: return VTK_NUMPAD7;
	case Producer::KeyChar_KP_8: return VTK_NUMPAD8;
	case Producer::KeyChar_KP_9: return VTK_NUMPAD9;

	case Producer::KeyChar_F1: return VTK_F1;
	case Producer::KeyChar_F2: return VTK_F2;
	case Producer::KeyChar_F3: return VTK_F3;
	case Producer::KeyChar_F4: return VTK_F4;
	case Producer::KeyChar_F5: return VTK_F5;
	case Producer::KeyChar_F6: return VTK_F6;
	case Producer::KeyChar_F7: return VTK_F7;
	case Producer::KeyChar_F8: return VTK_F8;
	case Producer::KeyChar_F9: return VTK_F9;
	case Producer::KeyChar_F10: return VTK_F10;
	case Producer::KeyChar_F11: return VTK_F11;
	case Producer::KeyChar_F12: return VTK_F12;

	case Producer::KeyChar_Shift_L: return VTK_SHIFT;
	case Producer::KeyChar_Shift_R: return VTK_SHIFT;
	case Producer::KeyChar_Control_L: return VTK_CONTROL;
	case Producer::KeyChar_Control_R: return VTK_CONTROL;
	case Producer::KeyChar_Alt_L: return VTK_ALT;
	case Producer::KeyChar_Alt_R: return VTK_ALT;
	}
	return (vtKeyCode) (int) key;
}

class MyKeyboardMouseCallback : public Producer::KeyboardMouseCallback
{
public:
	MyKeyboardMouseCallback() :	Producer::KeyboardMouseCallback()
	{
		m_done = false;
	}
	virtual void specialKeyPress( Producer::KeyCharacter key )
	{
		VTLOG("specialKeyPress %d (%c)\n", key, key);
		if (key==Producer::KeyChar_Escape)
			m_done = true;
		vtGetScene()->OnKey(ProducerToVT(key), 0);
	}
	virtual void keyPress( Producer::KeyCharacter key)
	{
		VTLOG("keyPress %d (%c)\n", key, key);
		vtGetScene()->OnKey(key, 0);
	}
	virtual void buttonPress( float mx, float my, unsigned int mbutton )
	{
		VTLOG("buttonPress %d (%f, %f)\n", mbutton, mx, my);

		IPoint2 winsize = vtGetScene()->GetWindowSize();
		m_event.pos.Set((mx+1)*winsize.x/2, (1-my)*winsize.y/2);
		m_event.type = VT_DOWN;
		if (mbutton == 1) m_event.button = VT_LEFT;
		if (mbutton == 2) m_event.button = VT_MIDDLE;
		if (mbutton == 3) m_event.button = VT_RIGHT;
		m_event.flags = 0;
		vtGetScene()->OnMouse(m_event);
	}
	virtual void buttonRelease( float mx, float my, unsigned int mbutton )
	{
		VTLOG("buttonRelease %d\n", mbutton);

		IPoint2 winsize = vtGetScene()->GetWindowSize();
		m_event.pos.Set((mx+1)*winsize.x/2, (1-my)*winsize.y/2);
		m_event.type = VT_UP;
		if (mbutton == 1) m_event.button = VT_LEFT;
		if (mbutton == 2) m_event.button = VT_MIDDLE;
		if (mbutton == 3) m_event.button = VT_RIGHT;
		m_event.flags = 0;
		vtGetScene()->OnMouse(m_event);
	}
	virtual void mouseMotion( float mx, float my )
	{
		IPoint2 winsize = vtGetScene()->GetWindowSize();
		m_event.pos.Set((mx+1)*winsize.x/2, (1-my)*winsize.y/2);
		m_event.type = VT_MOVE;
		m_event.button = 0;
		m_event.flags = 0;
		vtGetScene()->OnMouse(m_event);
	}
	virtual void windowConfig( int x, int y, unsigned int width, unsigned int height )
	{
		VTLOG("windowConfig(%d,%d,%d,%d\n", x, y, width, height);
	}
	bool done() { return m_done; }

private:
	vtMouseEvent	m_event;
	bool			m_done;
};


int main(int argc, char *argv[])
{
	// First, create the VTP scene
	CreateScene();

	// Window dimensions
	int x, y;
	unsigned int width, height;
	x = 225;
	y = 200;
	width = 800;
	height = 600;

	vtScene *pScene = vtGetScene();
	pScene->SetWindowSize(width, height);
	pScene->SetBgColor(RGBf(0.5f,0,0));

	// create the window to draw to.
	osg::ref_ptr<Producer::RenderSurface> renderSurface = new Producer::RenderSurface;
	renderSurface->setWindowName("prSimple");
	renderSurface->setWindowRectangle(x, y, width, height);
	renderSurface->useBorder(true);
	renderSurface->realize();

	// set up a KeyboardMouse to manage the events comming in from the RenderSurface
	osg::ref_ptr<Producer::KeyboardMouse>  kbm = new Producer::KeyboardMouse(renderSurface.get());

	// create a KeyboardMouseCallback to handle the mouse events within this applications
	osg::ref_ptr<MyKeyboardMouseCallback> kbmcb = new MyKeyboardMouseCallback;

	// main loop (note, window toolkits which take control over the main loop will require a window redraw callback containing the code below.)
	while( renderSurface->isRealized() && !kbmcb->done())
	{
		// the RenderSurface may have been resized this frame
		renderSurface->getWindowRectangle( x, y, width, height );
		pScene->SetWindowSize(width, height);

		// pass any keyboard mouse events onto the local keyboard mouse callback.
		kbm->update( *kbmcb );

		// do the usual culling and drawing of the scene
		pScene->DoUpdate();

		// Swap Buffers
		renderSurface->swapBuffers();
	}
	return 0;
}

