//
// FrameTimer.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#ifndef WIN32
#  include <sys/time.h>
#endif
#include "FrameTimer.h"

#ifdef WIN32
//extern "C" { WINBASEAPI VOID WINAPI Sleep(DWORD dwMilliseconds); }
extern "C" { __declspec(dllimport) void __stdcall Sleep(unsigned long dwMilliseconds); }
#endif

TLONG FrameTimer::Init()
{
	_initialTick = _tick();
	m_pos = 0;

#ifdef WIN32
	TLONG noSleepIntervalsPerSecond = 10;
	Sleep (1000/noSleepIntervalsPerSecond);
	_tickRatePerSecond = clockTick()*noSleepIntervalsPerSecond;
#else
	_tickRatePerSecond = 1000000;
#endif

	return _initialTick;
}


float FrameTimer::clockSeconds()
{
	TLONG ct = clockTick();
	float cs = (float)ct/(float)_tickRatePerSecond;
	return cs;
}

float FrameTimer::frameRate()
{
	float fps = (float)_tickRatePerSecond/_frameTick;
	return fps;
}


// in micro seconds (1/1000000th).
TLONG FrameTimer::clockTick()
{
	return _tick()-_initialTick;
}

// time from the current frame update and the previous one in microseconds.
TLONG FrameTimer::frameTick()
{
	return _frameTick;
}


// update time from the current frame update and the previous one in microseconds.
void FrameTimer::updateFrameTick()
{
	TLONG currFrameTick = _tick();
	_frameTick = currFrameTick - _lastFrameTick;
	_lastFrameTick = currFrameTick;

	// keep average of last AVGLEN frames
	m_avg = (float)_tickRatePerSecond / (currFrameTick - m_previous[m_pos]) * (float) AVGLEN;
	m_previous[m_pos] = currFrameTick;
	m_pos = (m_pos + 1) % AVGLEN;
}


#ifdef WIN32

TLONG FrameTimer::_tick()
{
	volatile unsigned __int64 ts;

	volatile unsigned int HighPart;
	volatile unsigned int LowPart;
	_asm
	{
		xor eax, eax	//  Used when QueryPerformanceCounter()
		xor edx, edx	//  not supported or minimal overhead
		_emit 0x0f		//  desired
		_emit 0x31		//
		mov HighPart,edx
		mov LowPart,eax
	}
	//ts = LowPart | HighPart >> 32;
	*((unsigned int*)&ts) = LowPart;
	*((unsigned int*)&ts+1) = HighPart;

	return ts;
}

#else

TLONG FrameTimer::_tick()
{
	static struct timeval tv;
	gettimeofday(&tv, NULL);
	return (long)(tv.tv_sec * 1000000 + tv.tv_usec);
}

#endif

