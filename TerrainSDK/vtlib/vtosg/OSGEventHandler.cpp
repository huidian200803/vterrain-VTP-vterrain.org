//
// OSGEventHandler.cpp
//
// Handle OSG events, turn them into VTP events.
//
// Copyright (c) 2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "OSGEventHandler.h"
#include "vtdata/vtLog.h"

#define GEA		osgGA::GUIEventAdapter				// shorthand
#define GEType	osgGA::GUIEventAdapter::EventType	// shorthand

bool vtOSGEventHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	GEType etype = ea.getEventType();

	//if (etype != GEA::FRAME)
	//	VTLOG("getEventType %d button %d\n", etype, ea.getButton());

	switch (etype)
	{
	case GEA::KEYDOWN:
	//case GEA::KEYUP:
		handleKeyEvent(ea);
		break;
	case GEA::PUSH:
	case GEA::RELEASE:
	case GEA::DOUBLECLICK:
	case GEA::DRAG:
	case GEA::MOVE:
		handleMouseEvent(ea);
		break;
	case GEA::FRAME:
		break;
	case GEA::RESIZE:
		handleResize(ea);
		break;
	case GEA::SCROLL:
		VTLOG("SCROLL\n");
		break;
	case GEA::CLOSE_WINDOW:
		VTLOG("CLOSE_WINDOW\n");
		break;
	case GEA::QUIT_APPLICATION:
		VTLOG("QUIT_APPLICATION\n");
		break;
	}
	return true;	// We "handled" the event.
}

void vtOSGEventHandler::handleResize(const osgGA::GUIEventAdapter& ea)
{
	// With OSG 3.0.1, it seems in some cases we can get a continuous stream
	// of RESIZE events even though the size is not changing.  Check for that.
	int x = ea.getWindowWidth(), y = ea.getWindowHeight();
	if (x != last_x || y != last_y)
	{
		last_x = x;
		last_y = y;
		VTLOG("RESIZE %d %d\n", x,y );
		vtGetScene()->SetWindowSize(x, y);
	}
}

void vtOSGEventHandler::handleKeyEvent(const osgGA::GUIEventAdapter& ea)
{
	int vtkey = ea.getKey();

	int flags = 0;
	int mkm = ea.getModKeyMask();
	if (mkm & GEA::MODKEY_SHIFT) flags |= VT_SHIFT;
	if (mkm & GEA::MODKEY_CTRL) flags |= VT_CONTROL;
	if (mkm & GEA::MODKEY_ALT) flags |= VT_ALT;

	switch (ea.getKey())
	{
	case GEA::KEY_Home: vtkey = VTK_HOME; break;
	case GEA::KEY_Left: vtkey = VTK_LEFT; break;
	case GEA::KEY_Up: vtkey = VTK_UP; break;
	case GEA::KEY_Right: vtkey = VTK_RIGHT; break;
	case GEA::KEY_Down: vtkey = VTK_DOWN; break;

	case GEA::KEY_Page_Up: vtkey = VTK_PAGEUP; break;
	case GEA::KEY_Page_Down: vtkey = VTK_PAGEDOWN; break;

	case GEA::KEY_F1 : vtkey = VTK_F1; break;
	case GEA::KEY_F2 : vtkey = VTK_F2; break;
	case GEA::KEY_F3 : vtkey = VTK_F3; break;
	case GEA::KEY_F4 : vtkey = VTK_F4; break;
	case GEA::KEY_F5 : vtkey = VTK_F5; break;
	case GEA::KEY_F6 : vtkey = VTK_F6; break;
	case GEA::KEY_F7 : vtkey = VTK_F7; break;
	case GEA::KEY_F8 : vtkey = VTK_F8; break;
	case GEA::KEY_F9 : vtkey = VTK_F9; break;
	case GEA::KEY_F10: vtkey = VTK_F10; break;
	case GEA::KEY_F11: vtkey = VTK_F11; break;
	case GEA::KEY_F12: vtkey = VTK_F12; break;

	case GEA::KEY_Shift_L: vtkey = VTK_SHIFT; break;
	case GEA::KEY_Shift_R: vtkey = VTK_SHIFT; break;
	case GEA::KEY_Control_L: vtkey = VTK_CONTROL; break;
	case GEA::KEY_Control_R: vtkey = VTK_CONTROL; break;
	case GEA::KEY_Alt_L: vtkey = VTK_ALT; break;
	case GEA::KEY_Alt_R: vtkey = VTK_ALT; break;
	}
	vtGetScene()->OnKey(vtkey, flags);
}

void vtOSGEventHandler::handleMouseEvent(const osgGA::GUIEventAdapter& ea)
{
	// Turn OSG mouse event into a VT mouse event
	vtMouseEvent event;
	event.flags = 0;
	event.pos.Set(ea.getX(), ea.getWindowHeight()-1-ea.getY());		// Flip Y from OSG to everyone else

	int mkm = ea.getModKeyMask();
	if (mkm & GEA::MODKEY_SHIFT) event.flags |= VT_SHIFT;
	if (mkm & GEA::MODKEY_CTRL) event.flags |= VT_CONTROL;
	if (mkm & GEA::MODKEY_ALT) event.flags |= VT_ALT;

	GEType etype = ea.getEventType();
	switch (etype)
	{
	case GEA::PUSH:
		event.type = VT_DOWN;
		if (ea.getButton() == GEA::LEFT_MOUSE_BUTTON)
			event.button = VT_LEFT;
		else if (ea.getButton() == GEA::MIDDLE_MOUSE_BUTTON)
			event.button = VT_MIDDLE;
		else if (ea.getButton() == GEA::RIGHT_MOUSE_BUTTON)
			event.button = VT_RIGHT;
		vtGetScene()->OnMouse(event);
		break;
	case GEA::RELEASE:
		event.type = VT_UP;
		if (ea.getButton() == GEA::LEFT_MOUSE_BUTTON)
			event.button = VT_LEFT;
		else if (ea.getButton() == GEA::MIDDLE_MOUSE_BUTTON)
			event.button = VT_MIDDLE;
		else if (ea.getButton() == GEA::RIGHT_MOUSE_BUTTON)
			event.button = VT_RIGHT;
		vtGetScene()->OnMouse(event);
		break;
	case GEA::DOUBLECLICK:
		break;
	case GEA::DRAG:
	case GEA::MOVE:
		event.type = VT_MOVE;
		if (ea.getButton() == 0)
			event.button = VT_LEFT;
		else if (ea.getButton() == 1)
			event.button = VT_MIDDLE;
		else if (ea.getButton() == 2)
			event.button = VT_RIGHT;
		vtGetScene()->OnMouse(event);
		break;
	}
}