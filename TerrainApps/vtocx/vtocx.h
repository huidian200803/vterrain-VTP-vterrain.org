#pragma once

// vtocx.h : main header file for vtocx.DLL

#if !defined( __AFXCTL_H__ )
#error include 'afxctl.h' before including this file
#endif

#include "resource.h"       // main symbols


// CvtocxApp : See vtocx.cpp for implementation.

class CvtocxApp : public COleControlModule
{
public:
	BOOL InitInstance();
	int ExitInstance();
	BOOL OnIdle(LONG lCount);
	virtual int Run();

	void Update();
};

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;

