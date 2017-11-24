//
// Log - simple message logging to a file.
//
// Copyright (c) 2002-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "cpl_error.h"	// From GDAL/include
#include "vtString.h"
#include "vtLog.h"
#include "FilePath.h"
#include <stdarg.h>

#ifndef __DARWIN_OSX__
#include <wchar.h>		// for fputws()
#endif

#ifdef _MSC_VER
#include <windows.h>	// for OutputDebugString, unfortunately
#endif

vtLog g_Log;

vtLog::vtLog()
{
	m_log = NULL;
}

vtLog::~vtLog()
{
	if (m_log)
	{
		fclose(m_log);
		m_log = NULL;
	}
}

void vtLog::StartLog(const char *fname)
{
	m_log = vtFileOpen(fname, "wb");
}

void vtLog::Log(const char *msg)
{
	if (m_log)
	{
		fputs(msg, m_log);
		fflush(m_log);
	}
#ifdef _MSC_VER
	OutputDebugStringA(msg);
#endif
	// also send to the console, for those console-mode developers!
	fputs(msg, stdout);
}

void vtLog::Log(char ch)
{
	if (m_log)
	{
		fputc(ch, m_log);
		fflush(m_log);
	}
#ifdef _MSC_VER
	char str[2];
	str[0] = ch;
	str[1] = 0;
	OutputDebugStringA(str);
#endif
	// also send to the console, for those console-mode developers!
	fputc(ch, stdout);
}

void vtLog::Printf(const char *pFormat, ...)
{
	va_list va;
	va_start(va, pFormat);

	char ach[2048];
	vsprintf(ach, pFormat, va);

	Log(ach);
}


#if SUPPORT_WSTRING

void vtLog::Log(const wchar_t *msg)
{
	wstring2 str = msg;
	const char *mbstr = str.mb_str();
	if (m_log)
	{
		// it is not so useful to write wide characters to the file, which
		// otherwise contains 8-bit text, so convert back first
//		fputws(msg, m_log);
		fputs(mbstr, m_log);
		fflush(m_log);
	}
	// also send to the console, for those console-mode developers!
	fputs(mbstr, stdout);

#ifdef _MSC_VER
	// We can pass a wide string, but unfortunately it may not display right.
	// MSDN says: "OutputDebugStringW converts the specified string based on
	//  the current system locale information and passes it to
	//  OutputDebugStringA to be displayed. As a result, some Unicode
	//  characters may not be displayed correctly."
	OutputDebugStringW(msg);
#endif
}

void vtLog::Printf(const wchar_t *pFormat, ...)
{
	va_list va;
	va_start(va, pFormat);

	// Use wide characters
	wchar_t ach[2048];

#if defined(_MSC_VER) && _MSC_VER < 1300
	vswprintf(ach, pFormat, va);
#else
	// on MSVC7.x and non-MSVC platforms this takes 4 arguments (safer)
	vswprintf(ach, 2048, pFormat, va);
#endif

	Log(ach);
}

#endif // SUPPORT_WSTRING

