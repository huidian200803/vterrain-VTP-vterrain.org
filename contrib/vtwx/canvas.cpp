//
// Name:	 canvas.cpp
// Purpose: Implements the canvas class for wxWidgets application.
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "canvas.h"

#include "vtlib/vtlib.h"
#include "vtlib/core/Event.h"
#include "vtlib/core/NavEngines.h"
#include "vtdata/vtLog.h"


// Support for the SpaceNavigator
#include "vtlib/core/SpaceNav.h"
vtSpaceNav g_SpaceNav;

#define LOG_MOUSE_CAPTURE	0
#if LOG_MOUSE_CAPTURE
#define VTLOGCAP VTLOG
#else
#define VTLOGCAP
#endif

/*
 * vtGLCanvas implementation
 */
BEGIN_EVENT_TABLE(vtGLCanvas, wxGLCanvas)
EVT_CLOSE(vtGLCanvas::OnClose)
EVT_SIZE(vtGLCanvas::OnSize)
#ifndef __WXMAC__
EVT_PAINT(vtGLCanvas::OnPaint)
#endif
EVT_CHAR(vtGLCanvas::OnChar)
EVT_KEY_DOWN(vtGLCanvas::OnKeyDown)
EVT_KEY_UP(vtGLCanvas::OnKeyUp)
EVT_MOUSE_EVENTS(vtGLCanvas::OnMouseEvent)
EVT_ERASE_BACKGROUND(vtGLCanvas::OnEraseBackground)
EVT_MOUSE_CAPTURE_LOST(vtGLCanvas::OnMouseCaptureLost)
EVT_IDLE(vtGLCanvas::OnIdle)
END_EVENT_TABLE()

static vtGLCanvas *s_canvas = NULL;

vtGLCanvas::vtGLCanvas(wxWindow *parent, wxWindowID id, const wxPoint &pos,
		       const wxSize &size, long style, const wxString &name, int *gl_attrib,
		       vtGLCanvasListener* listener):
#ifdef __WXMAC__
  wxGLCanvas(parent, id, pos, size, style, name, gl_attrib)
#else
  wxGLCanvas(parent, id, gl_attrib, pos, size, style, name)
#endif
{
  VTLOG1("vtGLCanvas constructor\n");

#ifdef __WXMAC__
  const GLint Value = 1;
  aglSetInteger(GetContext()->m_glContext, AGL_SWAP_INTERVAL, &Value); // Force VSYNC on
#else
  m_bFirstPaint = true;
  m_bPainting = false;
#endif
  m_bRunning = true;
  m_bShowFrameRateChart = false;
  m_bCapture = false;

  m_Listener = listener;

  VTLOG1("vtGLCanvas: calling Show on parent\n");
  parent->Show();

#ifndef __WXMAC__
  m_glContext = new wxGLContext(this);
#endif

#if __WXMSW__
  HGLRC hContext = m_glContext->GetGLRC();
  if (NULL == hContext)
    {
      wxMessageBox(_("No OpenGL support found") , _("Error"), wxICON_ERROR | wxOK);
      exit(-1);
    }
  else
    VTLOG("OpenGL context: %lx\n", hContext);
#endif

  // Documentation says about SetCurrent:
  // "Note that this function may only be called after the window has been shown."
  VTLOG1("vtGLCanvas: calling SetCurrent\n");
  SetCurrent();

  VTLOG1("OpenGL version: ");
  VTLOG1((const char *) glGetString(GL_VERSION));
  VTLOG1("\n");
  VTLOG1("OpenGL vendor: ");
  VTLOG1((const char *) glGetString(GL_VENDOR));
  VTLOG1("\n");
  VTLOG1("OpenGL renderer: ");
  VTLOG1((const char *) glGetString(GL_RENDERER));
  VTLOG1("\n");

  for (int i = 0; i < 512; i++)
    m_pbKeyState[i] = false;
  vtGetScene()->SetKeyStates(m_pbKeyState);
  m_iConsecutiveMousemoves = 0;

  // On RTL (right-to-left) system, the canvas should still be always LTR
  SetLayoutDirection(wxLayout_LeftToRight);

  // Initialize spacenavigator, if there is one present
  g_SpaceNav.Init();

  s_canvas = this;
  VTLOG1("vtGLCanvas, leaving constructor\n");
}

vtGLCanvas::~vtGLCanvas(void)
{
  VTLOG1("Deleting Canvas\n");
}

void vtGLCanvas::SetSpaceNavTarget(vtTransform *t)
{
  // BUG: Not compiling: why?
  //g_SpaceNav.SetTarget(t);
}

void vtGLCanvas::SetSpaceNavSpeed(float f)
{
  g_SpaceNav.SetSpeed(f);
}

void vtGLCanvas::SetSpaceNavAllowRoll(bool b)
{
  g_SpaceNav.SetAllowRoll(b);
}

#if WIN32
WXLRESULT vtGLCanvas::MSWDefWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
{
  // Catch SpaceNavigator messages; all others pass through
  if (nMsg == WM_INPUT)
    g_SpaceNav.ProcessWM_INPUTEvent(lParam);
  return wxWindowMSW::MSWDefWindowProc(nMsg, wParam, lParam);
}
#endif

void EnableContinuousRendering(bool bTrue)
{
  VTLOG("EnableContinuousRendering %d\n", bTrue);
  if (!s_canvas)
    return;

#ifdef __WXMAC__
  s_canvas->m_bRunning = bTrue;
#else
  bool bNeedRefresh = (s_canvas->m_bRunning == false && bTrue == true);
  s_canvas->m_bRunning = bTrue;
  if (bNeedRefresh)
    s_canvas->Refresh(FALSE);
#endif

  vtGetScene()->TimerRunning(bTrue);
}

#ifndef __WXMAC__
void vtGLCanvas::OnPaint( wxPaintEvent& event )
{
  if (m_bFirstPaint) VTLOG1("vtGLCanvas: first OnPaint\n");

  // place the dc inside a scope, to delete it before the end of function
  if (1)
    {
      // This is a dummy, to avoid an endless succession of paint messages.
      // OnPaint handlers must always create a wxPaintDC.
      if (m_bFirstPaint) VTLOG1("vtGLCanvas: creating a wxPaintDC on the stack\n");
      wxPaintDC dc(this);
    }

  // Safety checks
  if (!s_canvas)
    {
      VTLOG1("OnPaint: Canvas not yet constructed, returning\n");
      return;
    }

  // Avoid reentrance
  if (m_bPainting) return;

  m_bPainting = true;

  // Make sure the Graphics context of this thread is this window
  SetCurrent();

  // Render the Scene Graph
  if (m_bFirstPaint) VTLOG1("vtGLCanvas: DoUpdate\n");
  vtGetScene()->DoUpdate();

  if (m_bShowFrameRateChart)
    vtGetScene()->DrawFrameRateChart();

  if (m_bFirstPaint) VTLOG1("vtGLCanvas: SwapBuffers\n");
  SwapBuffers();

  if (m_Listener) {
    m_Listener->AfterUpdate(this);
  }
  /** This was the enviro code:

      if (m_bFirstPaint) VTLOG1("vtGLCanvas: update status bar\n");
      EnviroFrame *frame = (EnviroFrame*) GetParent();

      // update the status bar every 1/10 of a second
      static float last_stat = 0.0f;
      static vtString last_msg;
      float cur = vtGetTime();
      if (cur - last_stat > 0.1f || g_App.GetMessage() != last_msg)
      {
      last_msg = g_App.GetMessage();
      last_stat = cur;
      frame->UpdateStatus();
      }

      frame->UpdateLODInfo();
	
      g_App.UpdateCompass();
  */
  m_bPainting = false;

  // Reset the number of mousemoves we've gotten since last redraw
  m_iConsecutiveMousemoves = 0;

  if (m_bFirstPaint)
    m_bFirstPaint = false;
}
#endif

void vtGLCanvas::OnClose(wxCloseEvent& event)
{
  m_bRunning = false;
}

void vtGLCanvas::OnSize(wxSizeEvent& event)
{
  static int count = 0;
  if (count < 3)
    {
      VTLOG("Canvas  OnSize: %d %d\n", event.GetSize().x, event.GetSize().y);
      count++;
    }
  SetCurrent();
  int width, height;
  GetClientSize(&width, &height);

  vtGetScene()->SetWindowSize(width, height);

  wxGLCanvas::OnSize(event);
}

void vtGLCanvas::OnChar(wxKeyEvent& event)
{
  long key = event.GetKeyCode();

  // pass the char to the frame for it to do "accelerator" shortcuts
  if (m_Listener && m_Listener->OnChar(this, &event) == false) {
    return;
  }
  /** for enviro:
  EnviroFrame *frame = (EnviroFrame*) GetParent();
  frame->OnChar(event);
  */
  int flags = 0;

  if (event.ControlDown())
    flags |= VT_CONTROL;

  if (event.ShiftDown())
    flags |= VT_SHIFT;

  if (event.AltDown())
    flags |= VT_ALT;

  // pass the char to the vtlib Scene
  vtGetScene()->OnKey(key, flags);

  // Allow wxWindows to pass the event along to other code
  event.Skip();
}

void vtGLCanvas::OnKeyDown(wxKeyEvent& event)
{
  m_pbKeyState[event.m_keyCode] = true;
  event.Skip();
}

void vtGLCanvas::OnKeyUp(wxKeyEvent& event)
{
  m_pbKeyState[event.m_keyCode] = false;
  event.Skip();
}

void vtGLCanvas::OnMouseEvent(wxMouseEvent& event1)
{
  // turn WX mouse event into a VT mouse event
  vtMouseEvent event;
  wxEventType  ev = event1.GetEventType();
  if (ev == wxEVT_LEFT_DOWN) {
    event.type = VT_DOWN;
    event.button = VT_LEFT;
  } else if (ev == wxEVT_LEFT_UP) {
    event.type = VT_UP;
    event.button = VT_LEFT;
  } else if (ev == wxEVT_MIDDLE_DOWN) {
    event.type = VT_DOWN;
    event.button = VT_MIDDLE;
  } else if (ev == wxEVT_MIDDLE_UP) {
    event.type = VT_UP;
    event.button = VT_MIDDLE;
  } else if (ev == wxEVT_RIGHT_DOWN) {
    event.type = VT_DOWN;
    event.button = VT_RIGHT;
  } else if (ev == wxEVT_RIGHT_UP) {
    event.type = VT_UP;
    event.button = VT_RIGHT;
  } else if (ev == wxEVT_MOTION) {
    event.type = VT_MOVE;
    event.button = VT_NONE;
    m_iConsecutiveMousemoves++;		// Increment
  } else if (ev == wxEVT_MOUSEWHEEL) {
    event.type = VT_WHEEL;
    event.button = event1.GetWheelRotation() / event1.GetWheelDelta();
  } else {
    // ignored mouse events, such as wxEVT_LEAVE_WINDOW
    return;
  }

  if (ev == wxEVT_LEFT_DOWN || ev == wxEVT_MIDDLE_DOWN || ev == wxEVT_RIGHT_DOWN)
    {
      VTLOGCAP("DOWN: capture %d", m_bCapture);
      if (!m_bCapture)
		{
		  CaptureMouse();
		  m_bCapture = true;
		  VTLOGCAP(" -> true");
		}
      VTLOGCAP("\n");
    }
  if (ev == wxEVT_LEFT_UP || ev == wxEVT_MIDDLE_UP || ev == wxEVT_RIGHT_UP)
    {
      VTLOGCAP("  UP: capture %d, isdown %d %d %d", m_bCapture,
			   event1.LeftIsDown(), event1.MiddleIsDown(), event1.RightIsDown());

      if (m_bCapture && !event1.LeftIsDown() && !event1.MiddleIsDown() && !event1.RightIsDown())
		{
		  ReleaseMouse();
		  m_bCapture = false;
		  VTLOGCAP(" -> false");
		}
      VTLOGCAP("\n");
    }

  // Because of the way the event pump works, if it takes too long to
  //  handle a MouseMove event, then we might get the next MouseMove
  //  event without ever seeing a Redraw or Idle.  That's because the
  //  MouseMove events are considered higher priority in the queue.
  // So, to keep Enviro response smooth, we effectively ignore all but
  //  one MouseMove event per Draw event.
  if (ev == wxEVT_MOTION && m_iConsecutiveMousemoves > 1)
    return;

  event.flags = 0;
  wxCoord xpos, ypos;
  event1.GetPosition(&xpos, &ypos);
  event.pos.Set(xpos, ypos);

  if (event1.ControlDown())
    event.flags |= VT_CONTROL;

  if (event1.ShiftDown())
    event.flags |= VT_SHIFT;

  if (event1.AltDown())
    event.flags |= VT_ALT;

  // inform Enviro app, it will return false if it takes over the event
  if (m_Listener && m_Listener->OnMouse(this, &event) == false) {
	return;
  }
  /* enviro:
  if (g_App.OnMouse(event) == false)
    return;
  */
  // inform vtlib scene, which informs the engines
  vtGetScene()->OnMouse(event);
}

void vtGLCanvas::OnEraseBackground(wxEraseEvent& event)
{
  // Do nothing, to avoid flashing.
}

void vtGLCanvas::OnMouseCaptureLost(wxMouseCaptureLostEvent& event1)
{
  VTLOGCAP("MouseCaptureLost, capture -> false\n");
  m_bCapture = false;

  // When capture is lost, we won't get mouse events anymore.
  // That means we won't know when a mouse button goes up or down, so
  //  Enviro might think that a button is still down even when the focus
  //  comes back, when the button isn't down.
  // If we get this event, should we let the app know that any mouse
  //  buttons which were down are no longer down?  Not necessarily.
  //  They might in fact still be down. There is not a clear good solution.
}

void vtGLCanvas::OnIdle(wxIdleEvent &event)
{
  // We use the "Refresh on Idle" approach to continuous rendering.
  if (m_bRunning)
#ifdef __WXMAC__
    {
      // Make sure the Graphics context of this thread is this window
      SetCurrent();

      // Render the Scene Graph
      vtGetScene()->DoUpdate();

      if (m_bShowFrameRateChart)
		vtGetScene()->DrawFrameRateChart();

      SwapBuffers();
      if (m_Listener) {
	m_Listener->AfterUpdate(this);
	return;
      }
	  /** for enviro after paint:
      EnviroFrame *frame = (EnviroFrame*) GetParent();

      // update the status bar every 1/10 of a second
      static float last_stat = 0.0f;
      static vtString last_msg;
      float cur = vtGetTime();
      if (cur - last_stat > 0.1f || g_App.GetMessage() != last_msg)
		{
		  last_msg = g_App.GetMessage();
		  last_stat = cur;
		  frame->UpdateStatus();
		}

      frame->UpdateLODInfo();

      g_App.UpdateCompass();
	  */

      // Reset the number of mousemoves we've gotten since last redraw
      m_iConsecutiveMousemoves = 0;
	
      event.RequestMore();
    }
#else
  Refresh(FALSE);
#endif
}


vtGLCanvasListener::vtGLCanvasListener() {}
