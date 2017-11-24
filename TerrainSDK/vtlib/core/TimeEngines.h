//
// TimeEngines.h
//
// Copyright (c) 2001-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TIMEENGINESH
#define TIMEENGINESH

#include "Engine.h"
#include "vtdata/vtTime.h"

/** \addtogroup eng */
/*@{*/

/**
 * A TimeTarget is a kind of vtTarget which expects to be told what time
 * it is.
 */
class vtTimeTarget : public osg::Referenced
{
public:
	virtual void SetTime(const vtTime &time) {}
};

/**
 * The TimeEngine class keeps track of time of day (hours, minutes, seconds
 * since midnight) and when evaluated, informs each of its target of the time.
 *
 * Time can advance at either real time (1 second simulated = 1 real second),
 * or at any faster or slower rate.
 */
class vtTimeEngine : public vtEngine
{
public:
	vtTimeEngine(int start_hour = -1);

	void Eval();

	void SetTime(const vtTime &time);
	void GetTime(vtTime &time) const;
	vtTime GetTime() const;

	void SetSpeed(float factor);
	float GetSpeed() { return m_fSpeed; }

protected:
	void _InformTarget();

	float m_fSeconds;
	float m_fSpeed;
	float m_last_time;

	vtTime m_time;
};

/*@}*/	// Group eng

#endif	// TIMEENGINESH

