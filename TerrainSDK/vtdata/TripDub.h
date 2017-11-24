//
// TripDub.h
//
// Module for Double-You-Double-You-Double-You support (WWW)
//
// Copyright (c) 2003-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTDATA_TRIPDUB_H
#define VTDATA_TRIPDUB_H

#include "vtString.h"

#if __GNUC__ == 4 && __GNUC_MINOR__ >= 3
  #include <cstdlib>
#endif

// The dependency on LibCURL is optional.  If not desired, skip this file.
#if SUPPORT_CURL

class vtBytes
{
public:
	vtBytes() { m_data = NULL; m_len = 0; }
	~vtBytes() { if (m_data) free(m_data); }

	void Set(uchar *data, size_t len)
	{
		m_data = (uchar *) malloc(len);
		memcpy(m_data, data, len);
		m_len = len;
	}
	void Append(uchar *data, size_t len)
	{
		if (m_data)
			m_data = (uchar *) realloc(m_data, m_len + len);
		else
			m_data = (uchar *) malloc(len);
		memcpy(m_data + m_len, data, len);
		m_len += len;
	}
	uchar *Get() { return m_data; }
	size_t Len() { return m_len; }

protected:
	uchar *m_data;
	size_t m_len;
};


typedef void CURL;

class ReqContext
{
public:
	ReqContext();
	~ReqContext();

	bool GetURL(const char *url, vtString &str);
	bool GetURL(const char *url, vtBytes &data);
	bool DoQuery(vtBytes &data, int redirects = 0);
	void SetProgressCallback(bool progress_callback(int));
	vtString &GetErrorMsg() { return m_strErrorMsg; }

	/// Set level of logging output: 0 (none) 1 (some) 2 (lots)
	void SetVerbosity(int i) { m_iVerbosity = i; }

	vtString *m_pDataString;
	vtBytes *m_pDataBytes;
	bool (*m_progress_callback)(int);

protected:
	bool Fetch(const char *url);
	void InitializeLibrary();

	CURL *m_curl;

	int m_iVerbosity;
	vtString m_strErrorMsg;
	static bool s_bFirst;
};

void AddCookie(const char *name, const char *value);

#else
// provide dummies
class ReqContext
{
public:
	bool GetURL(const char *url, vtString &str) { return false; }
};

#endif // SUPPORT_CURL

#endif // VTDATA_TRIPDUB_H

