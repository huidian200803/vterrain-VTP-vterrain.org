//
// LocaleApp.cpp - Extend wxApp with convenience method for locale.
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/vtLog.h"
#include "LocaleApp.h"
#include "Helper.h"	


wxString LocaleApp::GetLanguageCode()
{
	return m_locale.GetCanonicalName();
}

void LocaleApp::SetupLocale(const char *appname)
{
	VTLOG1("SetupLocale:\n");

	wxLog::SetVerbose(true);

	// Enable this for very detailed locale troubleshooting (in debugger only)
//	wxLog::AddTraceMask(_T("i18n"));

	// Locale stuff
	int lang = wxLANGUAGE_DEFAULT;
	int default_lang = m_locale.GetSystemLanguage();

	const wxLanguageInfo *info = wxLocale::GetLanguageInfo(default_lang);
	VTLOG("Default language: %d (%s)\n", default_lang,
		(const char *) info->Description.mb_str(wxConvUTF8));

	// After wx2.4.2, wxWidgets looks in the application's directory for
	//  locale catalogs, not the current directory.  Here we force it to
	//  look in the current directory as well.
	wxString cwd = wxGetCwd();
	m_locale.AddCatalogLookupPathPrefix(cwd);

#if VTDEBUG
	m_locale.AddCatalogLookupPathPrefix(_T("../../../i18n"));
#endif

	bool bSuccess=false;
	if (m_locale_name != "")
	{
		VTLOG("Looking up language: %s\n", (const char *) m_locale_name);
		lang = GetLangFromName(wxString(m_locale_name, *wxConvCurrent));
		if (lang == wxLANGUAGE_UNKNOWN)
		{
			VTLOG(" Unknown, falling back on default language.\n");
			lang = wxLANGUAGE_DEFAULT;
		}
		else
		{
			info = m_locale.GetLanguageInfo(lang);
			VTLOG("Initializing locale to language %d, Canonical name '%s', Description: '%s':\n",
				lang,
				(const char *) info->CanonicalName.mb_str(wxConvUTF8),
				(const char *) info->Description.mb_str(wxConvUTF8));
			bSuccess = m_locale.Init(lang, wxLOCALE_LOAD_DEFAULT);
		}
	}
	if (lang == wxLANGUAGE_DEFAULT)
	{
		VTLOG("Initializing locale to default language:\n");
		bSuccess = m_locale.Init(wxLANGUAGE_DEFAULT, wxLOCALE_LOAD_DEFAULT);
		if (bSuccess)
			lang = default_lang;
	}
	if (bSuccess)
		VTLOG(" succeeded.\n");
	else
		VTLOG(" failed.\n");

	VTLOG("Attempting to load the '%s.mo' catalog for the current locale.\n", appname);
	wxString ws_appname = wxString::FromUTF8(appname);
	bSuccess = m_locale.AddCatalog(ws_appname);
	if (bSuccess)
		VTLOG(" succeeded.\n");
	else
		VTLOG(" not found.\n");
	VTLOG1("\n");

	wxLog::SetVerbose(false);
}

