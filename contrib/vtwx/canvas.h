//
// Name: Canvas.h
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef CANVASH
#define CANVASH

//#include "vtui_UI.h"
//#include "vtlib/vtlib.h"

#if !wxUSE_GLCANVAS
#error Please set wxUSE_GLCANVAS to 1 in setup.h.
#endif
#include "wx/glcanvas.h"

class vtGLCanvas;
class vtTransform;
class vtScene;
class vtMouseEvent;

class vtGLCanvasListener {
 public:
  vtGLCanvasListener();
  ~vtGLCanvasListener() {}
  virtual void AfterUpdate(vtGLCanvas*) = 0;
  virtual bool OnChar(vtGLCanvas*, wxKeyEvent*) = 0;
  virtual bool OnMouse(vtGLCanvas*, vtMouseEvent*) = 0;
};
//
// A Canvas for the main view area.
//
class vtGLCanvas: public wxGLCanvas
{
 public:
  vtGLCanvas(wxWindow *parent, const wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition,
	     const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = _T("vtGLCanvas"),
	     int* gl_attrib = NULL, vtGLCanvasListener *listener=NULL);
  ~vtGLCanvas(void);

  // SpaceNavigator methods
  void SetSpaceNavTarget(vtTransform *t);
  void SetSpaceNavSpeed(float f);
  void SetSpaceNavAllowRoll(bool b);
	
#ifndef __WXMAC__
  void OnPaint(wxPaintEvent& event);
#endif
  void OnSize(wxSizeEvent& event);
  void OnEraseBackground(wxEraseEvent& event);
  void OnMouseCaptureLost(wxMouseCaptureLostEvent & event);
  void OnChar(wxKeyEvent& event);
  void OnKeyDown(wxKeyEvent& event);
  void OnKeyUp(wxKeyEvent& event);
  void OnMouseEvent(wxMouseEvent& event);
  void OnClose(wxCloseEvent& event);
  void QueueRefresh(bool eraseBackground);
  void OnIdle(wxIdleEvent &event);

#ifndef __WXMAC__
  bool m_bPainting;
  bool m_bFirstPaint;
#endif
  bool m_bRunning;
  bool m_bShowFrameRateChart;
  bool m_bCapture;

  vtGLCanvasListener* m_Listener;
#if WIN32
  // Hook into the default window procedure
  virtual WXLRESULT MSWDefWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);
#endif
  friend class vtGLCanvasListener;
 protected:
  bool m_pbKeyState[512];

  // The number of mousemoves we've gotten since last redraw
  int m_iConsecutiveMousemoves;
  DECLARE_EVENT_TABLE()
    };

void EnableContinuousRendering(bool bTrue);

#endif	// CANVASH

