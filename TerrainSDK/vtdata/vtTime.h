//
// vtTime.h
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTTIMEH
#define VTTIMEH

#include <time.h>
#include "vtString.h"

///////////////////////////////////////////////////

/**
 * This class encapsulates time (in increments of whole seconds.)
 *
 * Time is stored both as a time_t value (integer seconds) and
 * as a tm structure (year, month, day, hour, minute, second).
 * The two values are kept in synch.
 *
 * Implementation notes: The functions mktime() and localtime()
 *  are used to convert between time_t and tm.  Although it seems
 *  as if this operating on "local" time, it isn't really; it is
 *  just a time value.  These functions are used because they are
 *  the only bidirectional conversion functions exposed by the ANSI
 *  C library - the gmtime() function converts time_t to tm more
 *  directly, but there is no corresponding method to convert
 *  directly from tm to time_t.
 */
class vtTime
{
public:
	vtTime();

	void SetDate(int year, int month, int day);
	void GetDate(int &year, int &month, int &day) const;

	void SetTimeOfDay(int hr, int min, int sec);

	void GetTimeOfDay(int &hr, int &min, int &sec) const;
	int GetSecondOfDay() const;

	time_t GetTime() const;
	const tm &GetTM() const { return m_tm; }
	void Increment(int secs);

	bool SetFromString(const vtString &str);
	vtString GetAsString();

	void GetSystemTime();

protected:
	void _UpdateTM();

	tm m_tm;
	time_t m_time;

//	static time_t s_DifferenceFromGMT;
};

#endif // VTTIMEH

