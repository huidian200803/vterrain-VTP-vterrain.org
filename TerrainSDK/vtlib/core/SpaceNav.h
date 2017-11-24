//
// Name: SpaceNav.h
// Purpose: Implements Win32-specific support for the SpaceNavigator 6DOF device.
//
// Copyright (c) 2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#if WIN32
#ifndef _WINDEF_
#error Sorry, this class is only usable from code which includes windows.h
#endif
#endif

#include "vtlib/core/Engine.h"

class vtSpaceNav : public vtEngine
{
public:
	vtSpaceNav();
	~vtSpaceNav();

	bool Init() { return InitRawDevices(); }
	void SetSpeed(float s) { m_fSpeed = s; }
	float GetSpeed() { return m_fSpeed; }
	void SetAllowRoll(bool b) { m_bAllowRoll = b; }
	bool GetAllowRoll() { return m_bAllowRoll; }
	virtual void OnButtons(uchar b1, uchar b2, uchar b3) {}
	virtual void Eval();

#if WIN32
	// The implementation of this class is WIN32-specfic
	void ProcessWM_INPUTEvent(LPARAM lParam);

protected:
	// ----------------  RawInput ------------------
	HMODULE				m_hUser32Dll;
	typedef UINT (WINAPI *GetRawInputDeviceList_t)(PRAWINPUTDEVICELIST, PUINT, UINT);
	typedef UINT (WINAPI *GetRawInputDeviceInfo_t)(HANDLE, UINT, LPVOID, PUINT);
	typedef BOOL (WINAPI *RegisterRawInputDevices_t)(PCRAWINPUTDEVICE, UINT, UINT);
	typedef UINT (WINAPI *GetRawInputData_t)(HRAWINPUT, UINT, LPVOID, PUINT, UINT);
	GetRawInputData_t	m_pfnGetRawInputData;

	PRAWINPUTDEVICELIST g_pRawInputDeviceList;
	PRAWINPUTDEVICE     g_pRawInputDevices;
	int                 g_nUsagePage1Usage8Devices;
#endif

	bool InitRawDevices();
	float m_fSpeed;
	bool m_bAllowRoll;
};

