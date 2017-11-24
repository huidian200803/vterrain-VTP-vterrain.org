//
// Event.h
//
// Copyright (c) 2001-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTLIB_EVENTH
#define VTLIB_EVENTH

#include "vtdata/MathTypes.h"

/** \addtogroup eng */
/*@{*/

enum vtMouseEventType
{
	VT_MOVE,
	VT_DOWN,
	VT_UP,
	VT_WHEEL
};

#define VT_NONE		0
#define VT_LEFT		1
#define VT_RIGHT	2
#define VT_MIDDLE	4

// event flags: modifier keys for mouse and keyboard events
#define VT_SHIFT	1
#define VT_CONTROL	2
#define VT_ALT		4

/**
 * A Mouse Event is used by the VTP event handling system to inform
 * each Engine of mouse input.
 */
struct vtMouseEvent
{
	vtMouseEventType type;
	int button;
	IPoint2 pos;
	int flags;
};

enum vtKeyCode
{
	VTK_START   = 300,
	VTK_LBUTTON,
	VTK_RBUTTON,
	VTK_CANCEL,
	VTK_MBUTTON,
	VTK_CLEAR,
	VTK_SHIFT,
	VTK_ALT,
	VTK_CONTROL,
	VTK_MENU,
	VTK_PAUSE,
	VTK_CAPITAL,
	VTK_END,
	VTK_HOME,
	VTK_LEFT,
	VTK_UP,
	VTK_RIGHT,
	VTK_DOWN,
	VTK_SELECT,
	VTK_PRINT,
	VTK_EXECUTE,
	VTK_SNAPSHOT,
	VTK_INSERT,
	VTK_HELP,
	VTK_NUMPAD0,
	VTK_NUMPAD1,
	VTK_NUMPAD2,
	VTK_NUMPAD3,
	VTK_NUMPAD4,
	VTK_NUMPAD5,
	VTK_NUMPAD6,
	VTK_NUMPAD7,
	VTK_NUMPAD8,
	VTK_NUMPAD9,
	VTK_MULTIPLY,
	VTK_ADD,
	VTK_SEPARATOR,
	VTK_SUBTRACT,
	VTK_DECIMAL,
	VTK_DIVIDE,
	VTK_F1,
	VTK_F2,
	VTK_F3,
	VTK_F4,
	VTK_F5,
	VTK_F6,
	VTK_F7,
	VTK_F8,
	VTK_F9,
	VTK_F10,
	VTK_F11,
	VTK_F12,
	VTK_F13,
	VTK_F14,
	VTK_F15,
	VTK_F16,
	VTK_F17,
	VTK_F18,
	VTK_F19,
	VTK_F20,
	VTK_F21,
	VTK_F22,
	VTK_F23,
	VTK_F24,
	VTK_NUMLOCK,
	VTK_SCROLL,
	VTK_PAGEUP,		// 'Prior'
	VTK_PAGEDOWN	// 'Next'
};

/*@}*/	// Group eng

#endif // VTLIB_EVENTH

