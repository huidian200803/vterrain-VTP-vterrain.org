//
// Joystick.cpp - provide access to wxWidget's joystick capabilities for Win32
//  if it was not compiled into the main library.
//
// Copyright (c) 2006-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "Joystick.h"

///////////////////////////////////////////////////////////////////////

#if WIN32

#if !wxUSE_JOYSTICK

///////////////////////////////////////////////////////////////////////

wxJoystick::wxJoystick(int joystick)
{
	JOYINFO joyInfo;
	int i, maxsticks;

	maxsticks = joyGetNumDevs();
	for( i=0; i<maxsticks; i++ )
	{
		if( joyGetPos(i, & joyInfo) == JOYERR_NOERROR )
		{
			if( !joystick )
			{
				/* Found the one we want, store actual OS id and return */
				m_joystick = i;
				return;
			}
			joystick --;
		}
	}

	/* No such joystick, return ID 0 */
	m_joystick = 0;
	return;
};

wxPoint wxJoystick::GetPosition() const
{
	JOYINFO joyInfo;
	MMRESULT res = joyGetPos(m_joystick, & joyInfo);
	if (res == JOYERR_NOERROR )
		return wxPoint(joyInfo.wXpos, joyInfo.wYpos);
	else
		return wxPoint(0,0);
}

int wxJoystick::GetZPosition() const
{
	JOYINFO joyInfo;
	MMRESULT res = joyGetPos(m_joystick, & joyInfo);
	if (res == JOYERR_NOERROR )
		return joyInfo.wZpos;
	else
		return 0;
}

int wxJoystick::GetButtonState() const
{
	JOYINFO joyInfo;
	MMRESULT res = joyGetPos(m_joystick, & joyInfo);
	if (res == JOYERR_NOERROR )
		return joyInfo.wButtons;
	else
		return 0;
}

/**
	Returns -1 to signify error.
*/
int wxJoystick::GetPOVPosition() const
{
	JOYINFOEX joyInfo;
	joyInfo.dwFlags = JOY_RETURNPOV;
	joyInfo.dwSize = sizeof(joyInfo);
	MMRESULT res = joyGetPosEx(m_joystick, & joyInfo);
	if (res == JOYERR_NOERROR )
		return joyInfo.dwPOV;
	else
		return -1;
}

/**
	Returns -1 to signify error.
*/
int wxJoystick::GetPOVCTSPosition() const
{
	JOYINFOEX joyInfo;
	joyInfo.dwFlags = JOY_RETURNPOVCTS;
	joyInfo.dwSize = sizeof(joyInfo);
	MMRESULT res = joyGetPosEx(m_joystick, & joyInfo);
	if (res == JOYERR_NOERROR )
		return joyInfo.dwPOV;
	else
		return -1;
}

int wxJoystick::GetRudderPosition() const
{
	JOYINFOEX joyInfo;
	joyInfo.dwFlags = JOY_RETURNR;
	joyInfo.dwSize = sizeof(joyInfo);
	MMRESULT res = joyGetPosEx(m_joystick, & joyInfo);
	if (res == JOYERR_NOERROR )
	{
		return joyInfo.dwRpos;
	}
	else
		return 0;
}

int wxJoystick::GetUPosition() const
{
	JOYINFOEX joyInfo;
	joyInfo.dwFlags = JOY_RETURNU;
	joyInfo.dwSize = sizeof(joyInfo);
	MMRESULT res = joyGetPosEx(m_joystick, & joyInfo);
	if (res == JOYERR_NOERROR )
	{
		return joyInfo.dwUpos;
	}
	else
		return 0;
}

int wxJoystick::GetVPosition() const
{
	JOYINFOEX joyInfo;
	joyInfo.dwFlags = JOY_RETURNV;
	joyInfo.dwSize = sizeof(joyInfo);
	MMRESULT res = joyGetPosEx(m_joystick, & joyInfo);
	if (res == JOYERR_NOERROR )
	{
		return joyInfo.dwVpos;
	}
	else
		return 0;
}

int wxJoystick::GetMovementThreshold() const
{
	UINT thresh = 0;
	MMRESULT res = joyGetThreshold(m_joystick, & thresh);
	if (res == JOYERR_NOERROR )
	{
		return thresh;
	}
	else
		return 0;
}

void wxJoystick::SetMovementThreshold(int threshold)
{
	UINT thresh = threshold;
	joySetThreshold(m_joystick, thresh);
}

// Capabilities
////////////////////////////////////////////////////////////////////////////

int wxJoystick::GetNumberJoysticks()
{
	JOYINFO joyInfo;
	int i, maxsticks, actualsticks;
	maxsticks = joyGetNumDevs();
	actualsticks = 0;
	for( i=0; i<maxsticks; i++ )
	{
		if( joyGetPos( i, & joyInfo ) == JOYERR_NOERROR )
		{
			actualsticks ++;
		}
	}
	return actualsticks;
}

bool wxJoystick::IsOk() const
{
	JOYINFO joyInfo;
	return (joyGetPos(m_joystick, & joyInfo) == JOYERR_NOERROR);
}

int wxJoystick::GetXMin() const
{
	JOYCAPS joyCaps;
	if (joyGetDevCaps(m_joystick, & joyCaps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
		return 0;
	else
		return joyCaps.wXmin;
}

int wxJoystick::GetYMin() const
{
	JOYCAPS joyCaps;
	if (joyGetDevCaps(m_joystick, & joyCaps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
		return 0;
	else
		return joyCaps.wYmin;
}

int wxJoystick::GetZMin() const
{
	JOYCAPS joyCaps;
	if (joyGetDevCaps(m_joystick, & joyCaps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
		return 0;
	else
		return joyCaps.wZmin;
}

int wxJoystick::GetXMax() const
{
	JOYCAPS joyCaps;
	if (joyGetDevCaps(m_joystick, & joyCaps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
		return 0;
	else
		return joyCaps.wXmax;
}

int wxJoystick::GetYMax() const
{
	JOYCAPS joyCaps;
	if (joyGetDevCaps(m_joystick, & joyCaps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
		return 0;
	else
		return joyCaps.wYmax;
}

int wxJoystick::GetZMax() const
{
	JOYCAPS joyCaps;
	if (joyGetDevCaps(m_joystick, & joyCaps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
		return 0;
	else
		return joyCaps.wZmax;
}

int wxJoystick::GetNumberButtons() const
{
	JOYCAPS joyCaps;
	if (joyGetDevCaps(m_joystick, & joyCaps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
		return 0;
	else
		return joyCaps.wNumButtons;
}

int wxJoystick::GetNumberAxes() const
{
	JOYCAPS joyCaps;
	if (joyGetDevCaps(m_joystick, & joyCaps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
		return 0;
	else
		return joyCaps.wNumAxes;
}

int wxJoystick::GetMaxButtons() const
{
	JOYCAPS joyCaps;
	if (joyGetDevCaps(m_joystick, & joyCaps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
		return 0;
	else
		return joyCaps.wMaxButtons;
}

int wxJoystick::GetMaxAxes() const
{
	JOYCAPS joyCaps;
	if (joyGetDevCaps(m_joystick, & joyCaps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
		return 0;
	else
		return joyCaps.wMaxAxes;
}

int wxJoystick::GetPollingMin() const
{
	JOYCAPS joyCaps;
	if (joyGetDevCaps(m_joystick, & joyCaps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
		return 0;
	else
		return joyCaps.wPeriodMin;
}

int wxJoystick::GetPollingMax() const
{
	JOYCAPS joyCaps;
	if (joyGetDevCaps(m_joystick, & joyCaps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
		return 0;
	else
		return joyCaps.wPeriodMax;
}

int wxJoystick::GetRudderMin() const
{
	JOYCAPS joyCaps;
	if (joyGetDevCaps(m_joystick, & joyCaps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
		return 0;
	else
		return joyCaps.wRmin;
}

int wxJoystick::GetRudderMax() const
{
	JOYCAPS joyCaps;
	if (joyGetDevCaps(m_joystick, & joyCaps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
		return 0;
	else
		return joyCaps.wRmax;
}

bool wxJoystick::HasRudder() const
{
	JOYCAPS joyCaps;
	if (joyGetDevCaps(m_joystick, & joyCaps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
		return false;
	else
		return ((joyCaps.wCaps & JOYCAPS_HASR) == JOYCAPS_HASR);
}

bool wxJoystick::HasZ() const
{
	JOYCAPS joyCaps;
	if (joyGetDevCaps(m_joystick, & joyCaps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
		return false;
	else
		return ((joyCaps.wCaps & JOYCAPS_HASZ) == JOYCAPS_HASZ);
}

bool wxJoystick::HasPOV() const
{
	JOYCAPS joyCaps;
	if (joyGetDevCaps(m_joystick, & joyCaps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
		return false;
	else
		return ((joyCaps.wCaps & JOYCAPS_HASPOV) == JOYCAPS_HASPOV);
}

bool wxJoystick::HasPOV4Dir() const
{
	JOYCAPS joyCaps;
	if (joyGetDevCaps(m_joystick, & joyCaps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
		return false;
	else
		return ((joyCaps.wCaps & JOYCAPS_POV4DIR) == JOYCAPS_POV4DIR);
}

bool wxJoystick::HasPOVCTS() const
{
	JOYCAPS joyCaps;
	if (joyGetDevCaps(m_joystick, & joyCaps, sizeof(JOYCAPS)) != JOYERR_NOERROR)
		return false;
	else
		return ((joyCaps.wCaps & JOYCAPS_POVCTS) == JOYCAPS_POVCTS);
}

// Operations
////////////////////////////////////////////////////////////////////////////

bool wxJoystick::SetCapture(wxWindow *win, int pollingFreq)
{
	BOOL changed = (pollingFreq == 0);
	MMRESULT res = joySetCapture((HWND) win->GetHWND(), m_joystick, pollingFreq, changed);
	return (res == JOYERR_NOERROR);
}

bool wxJoystick::ReleaseCapture()
{
	MMRESULT res = joyReleaseCapture(m_joystick);
	return (res == JOYERR_NOERROR);
}

#endif // !wxUSE_JOYSTICK

#endif // WIN32

