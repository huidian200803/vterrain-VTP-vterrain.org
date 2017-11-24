#ifndef CAPTURE_H
#define CAPTURE_H

#include <map>

/**	Capture is a class to capture events sent to an HWND.
  */
class Capture
{
public:
	Capture(void* winId);
	~Capture();

	void init();
	bool ok() {return _ok;}

	LRESULT CALLBACK wndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK s_wndProc( HWND hWnd, UINT, WPARAM, LPARAM );
	static std::map <void*, Capture*> registry;

	virtual void OnLButtonDown(UINT flags, int x, int y) {}
	virtual void OnMButtonDown(UINT flags, int x, int y) {}
	virtual void OnRButtonDown(UINT flags, int x, int y) {}
	virtual void OnLButtonUp(UINT flags, int x, int y) {}
	virtual void OnMButtonUp(UINT flags, int x, int y) {}
	virtual void OnRButtonUp(UINT flags, int x, int y) {}
	virtual void OnMouseMove(UINT flags, int x, int y) {}

protected:
	bool _ok;
	void* _winId; // often a winId is needed when accessing an input device

private:
	WNDPROC _oldWndProc;
};

#endif	// CAPTURE_H
