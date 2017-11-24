//
// Name: EnviroCanvas.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ENVIRO_CANVASH
#define ENVIRO_CANVASH

#include "wxosg/Canvas.h"

/**
 A Canvas for the main view area.
*/
class EnviroCanvas: public vtGLCanvas
{
public:
	EnviroCanvas(wxWindow *parent, const wxWindowID, const wxPoint &pos,
	  const wxSize &size, long style, const wxString &name, int *gl_attrib);

	virtual void OnAfterUpdate();
	virtual void OnCanvasChar(wxKeyEvent& event);
};

#endif	// ENVIRO_CANVASH
