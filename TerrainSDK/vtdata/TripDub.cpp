//
// TripDub.cpp
//
// Module for Double-You-Double-You-Double-You support (WWW)
// Ref: http://web.yost.com/Misc/nix-on-www.html
//
// Copyright (c) 2002-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "config_vtdata.h"

// The dependency on LibCURL is optional.  If not desired, skip this file.
#if SUPPORT_CURL

#include "TripDub.h"
#include "vtdata/vtLog.h"

#if 0

// Old cookie stuff, here just fstruct MyCookie
{
	vtString name;
	vtString value;
};

std::vector<MyCookie> g_cookies;
static int s_last_status = 0;

void AddCookie(const char *name, const char *value)
{
	uint i, size = g_cookies.size();
	for (i = 0; i < size; i++)
	{
		if (g_cookies[i].name == name)
		{
			g_cookies[i].value = value;
			return;
		}
	}
	// otherwise we need to create a new one
	MyCookie mc;
	mc.name = name;
	mc.value = value;
	g_cookies.push_back(mc);
}


void ReqContext::AddHeader(const char *token, const char *value)
{
}

bool ReqContext::DoQuery(vtBytes &data, int redirects)
{
}

#endif	// Old cookie stuff

/////////////////////////////////////////////////////////////////////////////
// LibCurl begins here

#include "curl/curl.h"

size_t write_as_string( void *ptr, size_t size, size_t nmemb, void *stream)
{
	ReqContext *context = (ReqContext *)stream;
	size_t length = size * nmemb;

	context->m_pDataString->Concat((pcchar) ptr, length);
	return length;
}

size_t write_as_bytes( void *ptr, size_t size, size_t nmemb, void *stream)
{
	ReqContext *context = (ReqContext *)stream;
	size_t length = size * nmemb;

	context->m_pDataBytes->Append((uchar *) ptr, length);
	return length;
}

int tripdub_progress(void *clientp,
					double dltotal, double dlnow,
					double ultotal, double ulnow)
{
	ReqContext *context = (ReqContext *)clientp;

	VTLOG("%lf %lf %lf %lf\n", dltotal, dlnow, ultotal, ulnow);

	if (context->m_progress_callback != NULL)
	{
		// If dltotal was valid, we could to this:
//		context->m_progress_callback((int) (dlnow * 100 / dltotal));

		// However, dltotal is always 0.0, so instead we can do only
		//  relative progress:
		static int prog = 0;
		if (dlnow == 0.0)
			prog = 0;
		prog += 10;
		if (prog == 100)
			prog = 10;
		context->m_progress_callback(prog);
	}

	// return a non-zero value from this callback to cancel
	return 0;
}


bool ReqContext::s_bFirst = true;

ReqContext::ReqContext()
{
	if (s_bFirst)
		InitializeLibrary();

	m_curl = curl_easy_init();

	CURLcode result;
	result = curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, this);

	// This tells CURL to give us an error if we get an e.g. 404, instead
	//  of giving us the HTML page from the server, e.g. "The page cannot
	//  be found..."
	result = curl_easy_setopt(m_curl, CURLOPT_FAILONERROR, 1);

	m_progress_callback = NULL;
}

ReqContext::~ReqContext()
{
	curl_easy_cleanup(m_curl);
}

void ReqContext::SetProgressCallback(bool progress_callback(int) = NULL)
{
	m_progress_callback = progress_callback;
}

bool ReqContext::Fetch(const char *url)
{
	CURLcode result;
	char errorbuf[CURL_ERROR_SIZE];
	result = curl_easy_setopt(m_curl, CURLOPT_URL, url);
	result = curl_easy_setopt(m_curl, CURLOPT_ERRORBUFFER, errorbuf);

	// setup progress indication
	result = curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, false);
	result = curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, tripdub_progress);
	result = curl_easy_setopt(m_curl, CURLOPT_PROGRESSDATA, this);

	result = curl_easy_perform(m_curl);
	if (result == 0)	// 0 means everything was ok
	{
		m_strErrorMsg = "";
		return true;
	}
	else
	{
		VTLOG("ReqContext::Fetch curl error: %d\n", result);
		m_strErrorMsg = errorbuf;
		return false;
	}
}

bool ReqContext::GetURL(const char *url, vtString &str)
{
	m_pDataString = &str;
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, write_as_string);
	return Fetch(url);
}

bool ReqContext::GetURL(const char *url, vtBytes &data)
{
	m_pDataBytes = &data;
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, write_as_bytes);
	return Fetch(url);
}

bool ReqContext::DoQuery(vtBytes &data, int redirects)
{
	return false;
}

void ReqContext::InitializeLibrary()
{
	s_bFirst = false;
	// Initialize everything possible. This sets all known bits.
	long flags = CURL_GLOBAL_ALL;
	CURLcode result = curl_global_init(flags);
	if (result != 0)
	{
		// error
	}
}

#endif	// SUPPORT_CURL
