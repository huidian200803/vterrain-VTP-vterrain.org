//
// LocaleApp.h
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtdata/vtString.h"

class LocaleApp: public wxApp
{
public:
	wxString GetLanguageCode();
	void SetupLocale(const char *appname);

	wxLocale m_locale; // locale we'll be using
	vtString m_locale_name;
};

