//
// Name:	 canvas.cpp
// Purpose:	 Implements the canvas class for the CManager application.
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
#include "frame.h"


CManagerCanvas::CManagerCanvas(wxWindow *parent, wxWindowID id, const wxPoint &pos,
	const wxSize &size, long style, const wxString &name, int *gl_attrib) :
		vtGLCanvas(parent, id, pos, size, style, name, gl_attrib)
{
	VTLOG1("CManagerCanvas constructor\n");
}

void CManagerCanvas::OnAfterUpdate()
{
	// update the status bar every 1/10 of a second
	static float last_stat = 0.0f;
	float cur = vtGetTime();
	if (cur - last_stat > 0.1f)
	{
		last_stat = cur;
		vtFrame *frame = GetMainFrame();
		frame->UpdateStatusText();
	}
}

