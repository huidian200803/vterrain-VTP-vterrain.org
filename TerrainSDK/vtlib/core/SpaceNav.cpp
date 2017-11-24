//
// Name: SpaceNav.cpp
// Purpose: Implements Win32-specific support for the SpaceNavigator 6DOF device.
//
// Copyright (c) 2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
// Based on: WM_INPUTExample.cpp, 3Dconnexion Inc. May 2007
//
// Uses WM_INPUT/RawInput to get data from 3Dx devices.
// You can disconnect and reconnect devices at will while the program is running.
// You can connect more than one device at a time, and distinguish the arriving
// data between the different devices.
//

#ifdef _MSC_VER
#define _WIN32_WINNT 0x0501
#include <windows.h>	// unfortunately
#include <tchar.h>
#endif

// Headers for the VTP libraries
#include "vtlib/vtlib.h"
#include "vtlib/core/Engine.h"
#include "vtdata/vtLog.h"

#include "SpaceNav.h"

vtSpaceNav::vtSpaceNav()
{
#if WIN32
	g_pRawInputDeviceList = NULL;
	g_pRawInputDevices = NULL;
	m_hUser32Dll = NULL;
#endif
	m_fSpeed = 100.0f;
	m_bAllowRoll = false;
}

vtSpaceNav::~vtSpaceNav()
{
#if WIN32
	if (g_pRawInputDeviceList)
		free(g_pRawInputDeviceList);
	if (g_pRawInputDevices)
		free(g_pRawInputDevices);
	if (NULL != m_hUser32Dll)
		FreeLibrary(m_hUser32Dll);
#endif
}

void vtSpaceNav::Eval()
{
	// Nothing here; the action is event-driven, not per-frame
}

bool vtSpaceNav::InitRawDevices()
{
#if WIN32
	VTLOG1("Looking for a SpaceNav: ");

	GetRawInputDeviceList_t pfnGetRawInputDeviceList;
	GetRawInputDeviceInfo_t pfnGetRawInputDeviceInfo;
	RegisterRawInputDevices_t pfnRegisterRawInputDevices;

	// Don't assume that the version of Windows has the method 'getRawInputData'
	//  in User32.dll.  Older OS (like Win2000) does not have it.  We use
	//  LoadLibrary and GetProcAddress to test.
	if (NULL == (m_hUser32Dll = LoadLibrary(_T("user32.dll"))))
		return false;
	if (NULL == (pfnGetRawInputDeviceList = (GetRawInputDeviceList_t)GetProcAddress(m_hUser32Dll, _T("GetRawInputDeviceList"))))
		return false;
	if (NULL == (pfnRegisterRawInputDevices = (RegisterRawInputDevices_t)GetProcAddress(m_hUser32Dll, _T("RegisterRawInputDevices"))))
		return false;
	if (NULL == (m_pfnGetRawInputData = (GetRawInputData_t)GetProcAddress(m_hUser32Dll, _T("GetRawInputData"))))
		return false;
#ifdef UNICODE
	if (NULL == (pfnGetRawInputDeviceInfo = (GetRawInputDeviceInfo_t)GetProcAddress(m_hUser32Dll, _T("GetRawInputDeviceInfoW"))))
		return false;
#else
	if (NULL == (pfnGetRawInputDeviceInfo = (GetRawInputDeviceInfo_t)GetProcAddress(m_hUser32Dll, _T("GetRawInputDeviceInfoA"))))
		return false;
#endif
	// Find the Raw Devices
	UINT nDevices;
	// Get Number of devices attached
	if (pfnGetRawInputDeviceList(NULL, &nDevices, sizeof(RAWINPUTDEVICELIST)) != 0)
	{
		VTLOG("No RawInput devices attached\n");
		return false;
	}
	// Create list large enough to hold all RAWINPUTDEVICE structs
	if ((g_pRawInputDeviceList = (PRAWINPUTDEVICELIST)malloc(sizeof(RAWINPUTDEVICELIST) * nDevices)) == NULL)
	{
		VTLOG("Error mallocing RAWINPUTDEVICELIST\n");
		return false;
	}
	// Now get the data on the attached devices
	if (pfnGetRawInputDeviceList(g_pRawInputDeviceList, &nDevices, sizeof(RAWINPUTDEVICELIST)) == -1)
	{
		VTLOG("Error from GetRawInputDeviceList\n");
		return false;
	}

	g_pRawInputDevices = (PRAWINPUTDEVICE)malloc( nDevices * sizeof(RAWINPUTDEVICE) );
	g_nUsagePage1Usage8Devices = 0;

	// Look through device list for RIM_TYPEHID devices with UsagePage == 1, Usage == 8
	for(UINT i=0; i<nDevices; i++)
	{
		if (g_pRawInputDeviceList[i].dwType == RIM_TYPEHID)
		{
			UINT nchars = 300;
			TCHAR deviceName[300];
			if (pfnGetRawInputDeviceInfo(g_pRawInputDeviceList[i].hDevice,
									  RIDI_DEVICENAME, deviceName, &nchars) >= 0)
#ifdef UNICODE
				VTLOG("Device[%d]: handle=0x%x name = %S\n", i,
#else
				VTLOG("Device[%d]: handle=0x%x name = %s\n", i,
#endif
					g_pRawInputDeviceList[i].hDevice, deviceName);
			RID_DEVICE_INFO dinfo;
			UINT sizeofdinfo = sizeof(dinfo);
			dinfo.cbSize = sizeofdinfo;
			if (pfnGetRawInputDeviceInfo(g_pRawInputDeviceList[i].hDevice,
									  RIDI_DEVICEINFO, &dinfo, &sizeofdinfo ) >= 0)
			{
				if (dinfo.dwType == RIM_TYPEHID)
				{
					RID_DEVICE_INFO_HID *phidInfo = &dinfo.hid;
					VTLOG("  VID = 0x%x, ", phidInfo->dwVendorId);
					VTLOG("PID = 0x%x, ", phidInfo->dwProductId);
					VTLOG("Version = 0x%x, ", phidInfo->dwVersionNumber);
					VTLOG("UsagePage = 0x%x, ", phidInfo->usUsagePage);
					VTLOG("Usage = 0x%x\n", phidInfo->usUsage);

					// Add this one to the list of interesting devices?
					// Actually only have to do this once to get input from all usage 1, usagePage 8 devices
					// This just keeps out the other usages.
					// You might want to put up a list for users to select amongst the different devices.
					// In particular, to assign separate functionality to the different devices.
					if (phidInfo->usUsagePage == 1 && phidInfo->usUsage == 8)
					{
						g_pRawInputDevices[g_nUsagePage1Usage8Devices].usUsagePage = phidInfo->usUsagePage;
						g_pRawInputDevices[g_nUsagePage1Usage8Devices].usUsage     = phidInfo->usUsage;
						g_pRawInputDevices[g_nUsagePage1Usage8Devices].dwFlags     = 0;
						g_pRawInputDevices[g_nUsagePage1Usage8Devices].hwndTarget  = NULL;
						g_nUsagePage1Usage8Devices++;
					}
				}
			}
		}
	}
	if (g_nUsagePage1Usage8Devices > 0)
	{
		VTLOG("  Found %d matching devices, registering.\n", g_nUsagePage1Usage8Devices);
		// Register for input from the devices in the list
		if (pfnRegisterRawInputDevices( g_pRawInputDevices, g_nUsagePage1Usage8Devices, sizeof(RAWINPUTDEVICE) ) == FALSE )
		{
			VTLOG("Error calling RegisterRawInputDevices\n");
			return false;
		}
	}
	else
		VTLOG1("  No matching devices found.\n");
	return true;
#else
	// not implemented
	return false;
#endif
}

#if WIN32
void vtSpaceNav::ProcessWM_INPUTEvent(LPARAM lParam)
{
	#ifdef SHOW_DETAILS
	VTLOG("WM_INPUT lParam=0x%x\n", lParam );
	#endif

	RAWINPUTHEADER header;
	UINT size_rawinputheader = sizeof(RAWINPUTHEADER);

	UINT size = sizeof(header);
	if (m_pfnGetRawInputData( (HRAWINPUT)lParam, RID_HEADER, &header, &size, size_rawinputheader ) == -1)
	{
		VTLOG("Error from GetRawInputData(RID_HEADER)\n");
		return;
	}
	// Ask Windows for the size of the event, because it is different on 32-bit
	//  vs. 64-bit versions of the OS
	UINT required_size;
	if (m_pfnGetRawInputData( (HRAWINPUT)lParam, RID_INPUT, NULL, &required_size, size_rawinputheader ) == -1)
	{
		VTLOG("Error from GetRawInputData(RID_INPUT)\n");
		return;
	}
	size = required_size;

	// Set aside enough memory for the full event
	LPRAWINPUT evt = (LPRAWINPUT)malloc(required_size);

	if (m_pfnGetRawInputData( (HRAWINPUT)lParam, RID_INPUT, evt, &size, size_rawinputheader ) == -1)
	{
		VTLOG("Error from GetRawInputData(RID_INPUT)\n");
		free(evt);
		return;
	}
	else
	{
		if (evt->header.dwType == RIM_TYPEHID)
		{
			static BOOL bGotTranslation = FALSE,
				        bGotRotation    = FALSE;
			static int all6DOFs[6] = {0};
			LPRAWHID pRawHid = &evt->data.hid;

			#ifdef SHOW_DETAILS
			VTLOG("rawInput count: %d\n", pRawHid->dwCount);
			VTLOG("          size: %d\n", pRawHid->dwSizeHid);
			for(UINT i=0; i<pRawHid->dwSizeHid; i++)
			{
				VTLOG("%d ", pRawHid->bRawData[i] );
			}
			_RPT0( _CRT_WARN, "\n" );
			#endif

			// Translation or Rotation packet?  They come in two different packets.
			if (pRawHid->bRawData[0] == 1) // Translation vector
			{
				all6DOFs[0] = (pRawHid->bRawData[1] & 0x000000ff) | ((signed short)(pRawHid->bRawData[2]<<8) & 0xffffff00);
				all6DOFs[1] = (pRawHid->bRawData[3] & 0x000000ff) | ((signed short)(pRawHid->bRawData[4]<<8) & 0xffffff00);
				all6DOFs[2] = (pRawHid->bRawData[5] & 0x000000ff) | ((signed short)(pRawHid->bRawData[6]<<8) & 0xffffff00);
				bGotTranslation = TRUE;
			}
			else if (pRawHid->bRawData[0] == 2) // Rotation vector
			{
				all6DOFs[3] = (pRawHid->bRawData[1] & 0x000000ff) | ((signed short)(pRawHid->bRawData[2]<<8) & 0xffffff00);
				all6DOFs[4] = (pRawHid->bRawData[3] & 0x000000ff) | ((signed short)(pRawHid->bRawData[4]<<8) & 0xffffff00);
				all6DOFs[5] = (pRawHid->bRawData[5] & 0x000000ff) | ((signed short)(pRawHid->bRawData[6]<<8) & 0xffffff00);
				bGotRotation = TRUE;
			}
			else if (pRawHid->bRawData[0] == 3) // Buttons (display most significant byte to least)
			{
				//VTLOG("Button mask: %.2x %.2x %.2x\n",(uchar)pRawHid->bRawData[3],(uchar)pRawHid->bRawData[2],(uchar)pRawHid->bRawData[1]);
				OnButtons((uchar)pRawHid->bRawData[3],
						  (uchar)pRawHid->bRawData[2],
						  (uchar)pRawHid->bRawData[1]);
			}

			if (bGotTranslation && bGotRotation)
			{
				bGotTranslation = bGotRotation = FALSE;
				//VTLOG("all6DOFs: %d %d %d ", all6DOFs[0], all6DOFs[1], all6DOFs[2]);
				//VTLOG(          "%d %d %d\n", all6DOFs[3], all6DOFs[4], all6DOFs[5]);
				FPoint3 trans((float) all6DOFs[0]/256*m_fSpeed,
							  (float)-all6DOFs[2]/256*m_fSpeed,
							  (float) all6DOFs[1]/256*m_fSpeed);

				for (uint t = 0; t < NumTargets(); t++)
				{
					vtTransform *target = dynamic_cast<vtTransform *>(GetTarget(t));

					target->TranslateLocal(trans);

					// Pitch is always around local X
					target->RotateLocal(FPoint3(1,0,0), (float) all6DOFs[3]/25600);

					if (m_bAllowRoll)
					{
						// If we allow roll, then the user can be allowed to
						//  rotate around local Y, and use also roll directly
						target->RotateLocal(FPoint3(0,1,0), (float)-all6DOFs[5]/25600);
						target->RotateLocal(FPoint3(0,0,1), (float) all6DOFs[4]/25600);
					}
					else
					{
						// Otherwise, use parent Y to avoid 'creeping roll'
						target->RotateParent(FPoint3(0,1,0), (float)-all6DOFs[5]/25600);
					}
				}
			}
		}
	}
	free(evt);
}

#endif	// WIN32

