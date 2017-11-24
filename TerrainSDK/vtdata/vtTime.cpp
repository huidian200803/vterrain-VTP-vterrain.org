//
// vtTime.cpp
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtTime.h"
#include "vtLog.h"

#define _BASE_YEAR			70		/* 1970 is the base year */
#define _MAX_YEAR			138		/* 2038 is the max year */
#define _LEAP_YEAR_ADJUST	17		/* Leap years 1900 - 1970 */
static int _days[] = {
	-1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364
};
#define ChkAdd(dest, src1, src2)   ( ((src1 >= 0L) && (src2 >= 0L) \
	&& (dest < 0L)) || ((src1 < 0L) && (src2 < 0L) && (dest >= 0L)) )
#define ChkMul(dest, src1, src2)   ( src1 ? (dest/src1 != src2) : 0 )

/*
 * Supply a function that is missing from the ANSI C standard library:
 *  convert a structure tm in GMT directly into a time_t.  This does the
 *  reverse operation of gmtime().
 */
static time_t make_time_t(struct tm *tb)
{
	time_t tmptm1, tmptm2, tmptm3;
	struct tm *tbtemp;

	/*
	* First, make sure tm_year is reasonably close to being in range.
	*/
	if ( ((tmptm1 = tb->tm_year) < _BASE_YEAR - 1) || (tmptm1 > _MAX_YEAR + 1) )
		goto err_mktime;

	/*
	 * Adjust month value so it is in the range 0 - 11.  This is because
	 * we don't know how many days are in months 12, 13, 14, etc.
	 */

	if ( (tb->tm_mon < 0) || (tb->tm_mon > 11) )
	{
		/*
		 * no danger of overflow because the range check above.
		 */
		tmptm1 += (tb->tm_mon / 12);

		if ( (tb->tm_mon %= 12) < 0 ) {
			tb->tm_mon += 12;
			tmptm1--;
		}

		/*
		 * Make sure year count is still in range.
		 */
		if ( (tmptm1 < _BASE_YEAR - 1) || (tmptm1 > _MAX_YEAR + 1) )
			goto err_mktime;
	}

	/***** HERE: tmptm1 holds number of elapsed years *****/

	/*
	 * Calculate days elapsed minus one, in the given year, to the given
	 * month. Check for leap year and adjust if necessary.
	 */
	tmptm2 = _days[tb->tm_mon];
	if ( !(tmptm1 & 3) && (tb->tm_mon > 1) )
		tmptm2++;

	/*
	 * Calculate elapsed days since base date (midnight, 1/1/70, UTC)
	 *
	 * 365 days for each elapsed year since 1970, plus one more day for
	 * each elapsed leap year. no danger of overflow because of the range
	 * check (above) on tmptm1.
	 */
	tmptm3 = (tmptm1 - _BASE_YEAR) * 365L + ((tmptm1 - 1L) >> 2)
	- _LEAP_YEAR_ADJUST;

	/*
	 * elapsed days to current month (still no possible overflow)
	 */
	tmptm3 += tmptm2;

	/*
	 * elapsed days to current date. overflow is now possible.
	 */
	tmptm1 = tmptm3 + (tmptm2 = (time_t)(tb->tm_mday));
	if ( ChkAdd(tmptm1, tmptm3, tmptm2) )
		goto err_mktime;

	/***** HERE: tmptm1 holds number of elapsed days *****/

	/*
	 * Calculate elapsed hours since base date
	 */
	tmptm2 = tmptm1 * 24L;
	if ( ChkMul(tmptm2, tmptm1, 24L) )
		goto err_mktime;

	tmptm1 = tmptm2 + (tmptm3 = (time_t)tb->tm_hour);
	if ( ChkAdd(tmptm1, tmptm2, tmptm3) )
		goto err_mktime;

	/***** HERE: tmptm1 holds number of elapsed hours *****/

	/*
	 * Calculate elapsed minutes since base date
	 */

	tmptm2 = tmptm1 * 60L;
	if ( ChkMul(tmptm2, tmptm1, 60L) )
		goto err_mktime;

	tmptm1 = tmptm2 + (tmptm3 = (time_t)tb->tm_min);
	if ( ChkAdd(tmptm1, tmptm2, tmptm3) )
		goto err_mktime;

	/***** HERE: tmptm1 holds number of elapsed minutes *****/

	/*
	 * Calculate elapsed seconds since base date
	 */

	tmptm2 = tmptm1 * 60L;
	if ( ChkMul(tmptm2, tmptm1, 60L) )
		goto err_mktime;

	tmptm1 = tmptm2 + (tmptm3 = (time_t)tb->tm_sec);
	if ( ChkAdd(tmptm1, tmptm2, tmptm3) )
		goto err_mktime;

	/***** HERE: tmptm1 holds number of elapsed seconds *****/

	if ( (tbtemp = gmtime(&tmptm1)) == NULL )
		goto err_mktime;

	/***** HERE: tmptm1 holds number of elapsed seconds, adjusted *****/
	/*****       for local time if requested                      *****/

	*tb = *tbtemp;
	return (time_t)tmptm1;

err_mktime:
	/*
	* All errors come to here
	*/
	return (time_t)(-1);
}

///////////////////////////////////////////////////////

//time_t vtTime::s_DifferenceFromGMT = (time_t) -1;

vtTime::vtTime()
{
	// default to "current" time
	GetSystemTime();

#if 0
	// This code attempts to work around the trouble with mktime(), but
	//  it isn't used, as we avoid the issue by using localtime() instead.
	if (s_DifferenceFromGMT == (time_t) -1)
	{
		VTLOG("vtTime: Calculating offset from local timezone to GMT.\n");
		// Determine the offset between local (meaning the timezome of the
		//  computer which is running this software) and Greenwich Mean time,
		//  and use it to compensate for the fact mktime always tries to
		//  factor in the "local time zone".
		struct tm tm_gm, tm_local;
		tm_gm = *gmtime(&m_time);
		tm_local = *localtime(&m_time);
		VTLOG(" Local: %s", asctime(&tm_local));
		VTLOG("   GMT: %s", asctime(&tm_gm));

		s_DifferenceFromGMT = mktime(&tm_local) - mktime(&tm_gm);
		VTLOG(" Diff: %d seconds\n", s_DifferenceFromGMT);
	}
#endif
}

void vtTime::_UpdateTM()
{
	struct tm *t = gmtime(&m_time);
	if (t != NULL)	// safety check: it can fail for <1970,>2038
		m_tm = *t;
}

void vtTime::Increment(int secs)
{
	m_time += secs;
	_UpdateTM();
}

bool vtTime::SetFromString(const vtString &str)
{
	const char *sz = str;
	int num = sscanf(sz, "%d %d %d %d %d %d", &m_tm.tm_year, &m_tm.tm_mon,
		&m_tm.tm_mday, &m_tm.tm_hour, &m_tm.tm_min, &m_tm.tm_sec);
	if (num != 6)
		return false;

	m_time = make_time_t(&m_tm);
	if (m_time == -1)
		return false;

//	m_time += s_DifferenceFromGMT;
	return true;
}

vtString vtTime::GetAsString()
{
	vtString str;
	str.Format("%d %d %d %d %d %d", m_tm.tm_year, m_tm.tm_mon,
		m_tm.tm_mday, m_tm.tm_hour, m_tm.tm_min, m_tm.tm_sec);
	return str;
}

void vtTime::GetSystemTime()
{
	time(&m_time);
	_UpdateTM();
}

/**
 * Set the date components of a vtTime value.
 *
 * \param year E.g. 2001.
 * \param month in the range of 1 to 12.
 * \param day in the range of 1 to 31.
 */
void vtTime::SetDate(int year, int month, int day)
{
	// safety checks
	if (year > 2038)
		year = 2038;
	if (year < 1970)
		year = 1970;

	m_tm.tm_year = year - 1900;

	// note that the tm structure has a 0-based month
	m_tm.tm_mon = month-1;

	m_tm.tm_mday = day;
	m_time = make_time_t(&m_tm);
//	m_time += s_DifferenceFromGMT;
	_UpdateTM();
}

void vtTime::GetDate(int &year, int &month, int &day) const
{
	year = 1900 + m_tm.tm_year;

	// note that the tm structure has a 0-based month
	month = m_tm.tm_mon + 1;

	day = m_tm.tm_mday;
}

void vtTime::SetTimeOfDay(int hr, int min, int sec)
{
	m_tm.tm_hour = hr;
	m_tm.tm_min = min;
	m_tm.tm_sec = sec;
	m_time = make_time_t(&m_tm);

//	m_time += s_DifferenceFromGMT;
	_UpdateTM();
}

void vtTime::GetTimeOfDay(int &hr, int &min, int &sec) const
{
	hr = m_tm.tm_hour;
	min = m_tm.tm_min;
	sec = m_tm.tm_sec;
}

int vtTime::GetSecondOfDay() const
{
	return (m_tm.tm_hour * 60 + m_tm.tm_min) * 60 + m_tm.tm_sec;
}

time_t vtTime::GetTime() const
{
	return m_time;
}

