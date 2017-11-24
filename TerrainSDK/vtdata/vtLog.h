//
// Log - simple message logging to a file.
//
// Copyright (c) 2002-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
/** \file vtLog.h */

#ifndef VTLOG_H
#define VTLOG_H

#include "config_vtdata.h"
#include <stdio.h>

/**
 * This class provide a convenient way to log all the messages that your
 * application generates.  Everything logged will be saved to a file,
 * and also be sent to the console.  Under MSVC, the log output will
 * go to the Debug Output window.
 *
 * There is always and only a single vtLog instance (singleton), called
 * g_Log.
 *
 * To use:
 * - 1. Call VTSTARTLOG with the name of the log file you want.
 * - 2. Use VTLOG just like you would use printf, to log messages.
 *    Alternately you can use VTLOG1 which takes a single string.
 *
 * \par Example:
	\code
	VTSTARTLOG("debug.txt");
	VTLOG("This code was compiled on %s\n", __DATE__);
	VTLOG1("Program starting.\n");
	\endcode
 *
 * That's it.  The file will be flushed immediately after every write, so
 * that log output will not be lost even if your application crashes.
 */
class vtLog
{
public:
	vtLog();
	~vtLog();

	void StartLog(const char *fname);
	void Log(const char *str);
	void Log(char ch);
	void Printf(const char *pFormat, ...);

#if SUPPORT_WSTRING
	void Log(const wchar_t *str);
	void Printf(const wchar_t *pFormat, ...);
#endif

private:
	FILE *m_log;
};

extern vtLog g_Log;
#define VTSTARTLOG	g_Log.StartLog
#define VTLOG		g_Log.Printf
#define VTLOG1		g_Log.Log		// for simple strings, takes 1 argument

#endif // VTLOG_H

