//
// Name:	frame.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef FRAMEH
#define FRAMEH

class vtFrame: public wxFrame
{
public:
	vtFrame(wxFrame *frame, const wxString& title, const wxPoint& pos,
		const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);
	~vtFrame();

	// command handlers
	void OnClose(wxCloseEvent &event);
	void OnIdle(wxIdleEvent& event);

public:
	class vtGLCanvas	*m_canvas;

protected:
	bool	m_bCloseOnIdle;

DECLARE_EVENT_TABLE()
};

#endif

