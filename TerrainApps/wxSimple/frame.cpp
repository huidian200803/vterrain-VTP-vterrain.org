//
// Name:	frame.cpp
// Purpose: The frame class for a wxWidgets application.
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// Header for the vtlib library
#include "vtlib/vtlib.h"

#include "vtdata/vtLog.h"
#include "app.h"
#include "frame.h"
#include "canvas.h"

DECLARE_APP(vtApp)

BEGIN_EVENT_TABLE(vtFrame, wxFrame)
	EVT_CLOSE(vtFrame::OnClose)
	EVT_IDLE(vtFrame::OnIdle)
END_EVENT_TABLE()

// Frame constructor
vtFrame::vtFrame(wxFrame *parent, const wxString& title, const wxPoint& pos,
	const wxSize& size, long style):
	wxFrame(parent, -1, title, pos, size, style)
{
	VTLOG(" constructing Frame (%x, title, pos, size, %x)\n", parent, style);
	m_bCloseOnIdle = false;

	// We definitely want full color and a 24-bit Z-buffer!
	int gl_attrib[7] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER,
		WX_GL_BUFFER_SIZE, 24, WX_GL_DEPTH_SIZE, 24, 0	};

	// Make a vtGLCanvas
	VTLOG1(" creating canvas\n");
	m_canvas = new SimpleCanvas(this, -1, wxPoint(0, 0), wxSize(-1, -1), 0,
		_T("SimpleCanvas"), gl_attrib);

	// Show the frame
	Show(true);
}

vtFrame::~vtFrame()
{
	delete m_canvas;
	m_canvas = NULL;
}

//
// Intercept close command
//
void vtFrame::OnClose(wxCloseEvent &event)
{
	if (m_canvas)
	{
		m_canvas->m_bRunning = false;
		m_bCloseOnIdle = true;
	}
	event.Skip();
}

void vtFrame::OnIdle(wxIdleEvent& event)
{
	// Check if we were requested to close on the next Idle event.
	if (m_bCloseOnIdle)
		Close();
	else
		event.Skip();
}
