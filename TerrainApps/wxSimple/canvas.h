//
// Name: canvas.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef SIMPLECANVASH
#define SIMPLECANVASH

#include "wxosg/Canvas.h"

//
// A Canvas for the main view area.
//
class SimpleCanvas: public vtGLCanvas
{
public:
	SimpleCanvas(wxWindow *parent, const wxWindowID id, const wxPoint &pos,
	  const wxSize &size, long style, const wxString &name, int *gl_attrib);

	virtual void OnCanvasChar(wxKeyEvent& event);
};

#endif	// SIMPLECANVASH

