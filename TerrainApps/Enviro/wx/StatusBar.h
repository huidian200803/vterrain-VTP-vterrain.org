//
// A custom status bar for Enviro
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef STATUSBARH
#define STATUSBARH

#include <wx/statusbr.h>

class MyStatusBar : public wxStatusBar
{
public:
	MyStatusBar(wxWindow *parent);

	// event handlers
	void UpdateText();

private:
	enum
	{
		Field_Text,
		Field_Fps,
		Field_Cursor,
		Field_CursorVal,	// value of thing under cursor
		Field_Max
	};

	DECLARE_EVENT_TABLE()
};

#endif	// STATUSBARH

