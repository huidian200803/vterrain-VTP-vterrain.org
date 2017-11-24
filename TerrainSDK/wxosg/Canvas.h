//
// Name: canvas.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef CANVASH
#define CANVASH

#if !wxUSE_GLCANVAS
#error Please set wxUSE_GLCANVAS to 1 in setup.h.
#endif
#include "wx/glcanvas.h"

//
// Extent wxWidget's canvas to integrate with vtlib and OSG
//
class vtGLCanvas: public wxGLCanvas
{
public:
	vtGLCanvas(wxWindow *parent, const wxWindowID id, const wxPoint &pos,
	  const wxSize &size, long style, const wxString &name, int *gl_attrib);
	~vtGLCanvas(void);

	// Initialization
	void InitGraphicsWindowWX();

	// SpaceNavigator methods
	void EnableSpaceNav();
	void SetSpaceNavSpeed(float f);
	void SetSpaceNavAllowRoll(bool b);

	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnEraseBackground(wxEraseEvent& event);
	void OnMouseCaptureLost(wxMouseCaptureLostEvent & event);
	void OnChar(wxKeyEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	void OnMouseEvent(wxMouseEvent& event);
	void OnClose(wxCloseEvent& event);
	void OnIdle(wxIdleEvent &event);

	bool m_bPainting;
	bool m_bFirstPaint;
	bool m_bRunning;
	bool m_bCapture;

#if WIN32
    // Hook into the default window procedure
    virtual WXLRESULT MSWDefWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);
#endif

	// Override to be notified
	virtual void OnAfterUpdate() {}
	virtual void OnCanvasChar(wxKeyEvent& event) {}

protected:
	bool m_pbKeyState[512];

	// The number of mousemoves we've gotten since last redraw
	int m_iConsecutiveMousemoves;

	DECLARE_EVENT_TABLE()
};

void EnableContinuousRendering(bool bTrue);

#endif	// CANVASH

