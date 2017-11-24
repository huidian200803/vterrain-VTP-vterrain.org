//
// Name:	 EnviroCanvas.cpp
// Purpose: Implements the canvas class for the Enviro wxWidgets application.
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

#include "EnviroCanvas.h"
#include "EnviroGUI.h"		// for g_App
#include "EnviroFrame.h"	// for UpdateStatus and OnChar


EnviroCanvas::EnviroCanvas(wxWindow *parent, wxWindowID id, const wxPoint &pos,
					   const wxSize &size, long style, const wxString &name, int *gl_attrib):
	vtGLCanvas(parent, id, pos, size, style, name, gl_attrib)
{
	VTLOG1("EnviroCanvas constructor\n");
}

void EnviroCanvas::OnAfterUpdate()
{
#ifndef __WXMAC__
	if (m_bFirstPaint)
		VTLOG1("EnviroCanvas: update status bar\n");
#endif

	EnviroFrame *frame = (EnviroFrame*) GetParent();

	// update the status bar every 1/10 of a second
	static float last_stat = 0.0f;
	static vtString last_msg;
	vtString current_msg = g_App.GetMessage1() + g_App.GetMessage2();
	float cur = vtGetTime();
	if (cur - last_stat > 0.1f || current_msg != last_msg)
	{
		last_msg = current_msg;
		last_stat = cur;
		frame->UpdateStatus();
	}

	frame->UpdateLODInfo();

	g_App.UpdateCompass();
}

void EnviroCanvas::OnCanvasChar(wxKeyEvent& event)
{
	EnviroFrame *frame = (EnviroFrame*) GetParent();
	frame->OnChar(event);
}

