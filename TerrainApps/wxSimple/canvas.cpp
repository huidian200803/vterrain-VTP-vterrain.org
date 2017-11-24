//
// Name:	canvas.cpp
// Purpose: Implements the canvas class for the wxSimple application.
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtdata/vtLog.h"

#include "canvas.h"
#include "app.h"

DECLARE_APP(vtApp)

SimpleCanvas::SimpleCanvas(wxWindow *parent, wxWindowID id, const wxPoint &pos,
	const wxSize &size, long style, const wxString &name, int *gl_attrib):
		vtGLCanvas(parent, id, pos, size, style, name, gl_attrib)
{
	VTLOG("SimpleCanvas constructor\n");
}

void SimpleCanvas::OnCanvasChar(wxKeyEvent& event)
{
	long key = event.GetKeyCode();
	if ( key == WXK_ESCAPE || key == 'q' || key == 'Q' )
		wxGetApp().GetTopWindow()->Close();
}

