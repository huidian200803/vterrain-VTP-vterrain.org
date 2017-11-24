//
// FrameTimer.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef FRAMETIMER_H
#define FRAMETIMER_H

#ifdef WIN32
#define TLONG __int64
#else
#define TLONG long
#endif

#define AVGLEN	8

class FrameTimer
{
public:
	// initialize the clock.
	TLONG Init();
	// time since Init() in seconds.
	float clockSeconds();

	// update the number of ticks since the last frame update.
	void updateFrameTick();
	// time from the current frame update and the previous one in seconds.
	float frameSeconds() { return (float)frameTick()/(float)_tickRatePerSecond; }
	float frameRate();
	float frameRateAverge() { return m_avg; }

	TLONG _tickRatePerSecond;
	TLONG _initialTick;
	TLONG _lastFrameTick;
	TLONG _frameTick;

	// system tick.
	TLONG clockTick();
	TLONG frameTick();
	TLONG _tick();

	// for keeping an average framerate
	TLONG m_previous[AVGLEN];
	int m_pos;
	float m_avg;
};

#endif
