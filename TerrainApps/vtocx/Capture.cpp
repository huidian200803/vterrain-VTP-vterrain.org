//
// Capture.cpp : Capture events to a window.
//

#include "stdafx.h"
#include "Capture.h"
#include "vtdata/vtLog.h"

// Class statics
std::map <void*, Capture*> Capture::registry;

Capture::Capture(void* winId)
{
	_ok = false;
	_winId = winId;
	registry.insert(std::pair<void*, Capture*>(_winId, this));
}

void Capture::init()
{
	// install an event filter on the widget
	_oldWndProc = (WNDPROC)SetWindowLong((HWND)_winId, GWL_WNDPROC, (LONG) s_wndProc);
	_ok = true;
}

Capture::~Capture()
{
	if(_ok)
		SetWindowLong((HWND)_winId, GWL_WNDPROC, (LONG) _oldWndProc);
}

LRESULT CALLBACK Capture::s_wndProc(HWND hWnd, UINT uMsg,
									WPARAM wParam, LPARAM lParam)
{
	std::map <void*, Capture*>::iterator p;
	p = registry.find( (void*)hWnd );
	if( p == registry.end() )
		return DefWindowProc( hWnd, uMsg, wParam, lParam );
	else
		return p->second->wndProc( hWnd, uMsg, wParam, lParam );
}

LRESULT CALLBACK Capture::wndProc(HWND hWnd, UINT iMsg,
								  WPARAM wParam, LPARAM lParam)
{
#if _DEBUG
	VTLOG("Capture sees %3d (0x%04x)", iMsg, iMsg);
	if (iMsg == WM_MOVE) VTLOG1(" (WM_MOVE)");
	if (iMsg == WM_SETFOCUS) VTLOG1(" (WM_SETFOCUS)");
	if (iMsg == WM_SETCURSOR) VTLOG1(" (WM_SETCURSOR)");
	if (iMsg == WM_MOUSEMOVE) VTLOG1(" (WM_MOUSEMOVE)");
	if (iMsg == WM_NCHITTEST) VTLOG1(" (WM_NCHITTEST)");
	if (iMsg == WM_SHOWWINDOW) VTLOG1(" (WM_SHOWWINDOW)");
	if (iMsg == WM_NCPAINT) VTLOG1(" (WM_NCPAINT)");
	if (iMsg == WM_SIZE) VTLOG1(" (WM_SIZE)");
	if (iMsg == WM_TIMER) VTLOG1(" (WM_TIMER)");
	if (iMsg == WM_PAINT) VTLOG1(" (WM_PAINT)");
	if (iMsg == WM_ERASEBKGND) VTLOG1(" (WM_ERASEBKGND)");
	if (iMsg == WM_WINDOWPOSCHANGING) VTLOG1(" (WM_WINDOWPOSCHANGING)");
	if (iMsg == WM_WINDOWPOSCHANGED) VTLOG1(" (WM_WINDOWPOSCHANGED)");
	if (iMsg == WM_IME_SETCONTEXT) VTLOG1(" (WM_IME_SETCONTEXT)");
	if (iMsg == WM_LBUTTONDOWN) VTLOG1(" (WM_LBUTTONDOWN)");
	if (iMsg == WM_LBUTTONUP) VTLOG1(" (WM_LBUTTONUP)");
	VTLOG1("\n");
#endif

	int x = LOWORD( lParam );
	int y = HIWORD( lParam );
	UINT flags = static_cast<UINT>(wParam);
	switch ( iMsg )
	{
	case WM_PAINT:
		//VTLOG1("Capture sees WM_PAINT\n");

		// try to validate the whole window so we don't keep getting PAINT msgs
		//ValidateRect(hWnd, NULL);

		//return 0;
		//		return DefWindowProc( hWnd, iMsg, wParam, lParam );
		break;
	//case WM_DRAW:
	//	VTLOG1("Capture sees WM_DRAW\n");
	//	break;
	//case WM_IDLE:
	//	VTLOG1("Capture sees WM_IDLE\n");
	//	break;
	case WM_TIMER:
		//VTLOG1("Capture sees WM_TIMER\n");
		break;
	case WM_LBUTTONDOWN:
		//VTLOG1("Capture sees WM_LBUTTONDOWN\n");
		OnLButtonDown(flags, x, y);
		break;
	case WM_MBUTTONDOWN:
		OnMButtonDown(flags, x, y);
		break;
	case WM_RBUTTONDOWN:
		OnRButtonDown(flags, x, y);
		break;
	case WM_LBUTTONUP:
		//VTLOG1("Capture sees WM_LBUTTONUP\n");
		OnLButtonUp(flags, x, y);
		break;
	case WM_MBUTTONUP:
		OnMButtonUp(flags, x, y);
		break;
	case WM_RBUTTONUP:
		OnRButtonUp(flags, x, y);
		break;

	case WM_MOUSEMOVE:
		if(x > 32768) x -= 65536;
		if(y > 32768) y -= 65536;
		OnMouseMove(flags, x, y);
		break;
	case WM_MOUSEWHEEL:
		{
			// VTLOG1("WM_MOUSEWHEEL\n");
			short zDelta  = HIWORD(wParam);
		}
		break;

		// Many other potential events could be caught here.
	case WM_NCLBUTTONUP:
	case WM_NCMBUTTONUP:
	case WM_NCRBUTTONUP:
		//::ReleaseCapture();
		break;
	//case WM_LBUTTONDBLCLK:
	//case WM_CONTEXTMENU:
	//{
	//	// prevent the context menu from showing up, except when right clicking
	//	int x = LOWORD( lParam );
	//	int y = HIWORD( lParam );
	//	if( ( fabs(x() - (1.0f/w->width())*m->x()) < 1.e-3) && ( fabs(y() - (1.0f/w->height())*m->y()) < 1.e-3) )
	//		return false;
	//	else
	//		return true;
	//}
	//break;

	default:
		break;
	}
	return CallWindowProc(_oldWndProc, hWnd, iMsg, wParam, lParam);
}


