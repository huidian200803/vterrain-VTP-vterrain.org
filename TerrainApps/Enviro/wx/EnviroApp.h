//
// Name: EnviroApp.h
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

class EnviroFrame;
class vtTerrain;

#include "vtui/LocaleApp.h"

/** The wxWidgets application for Enviro. */
class EnviroApp: public LocaleApp
{
public:
	EnviroApp();

	bool OnInit();
	int OnExit();
	void Args(int argc, wxChar **argv);

	void RefreshTerrainList();
	bool AskForTerrainName(wxWindow *pParent, wxString &strTerrainName);
	vtString GetIniFileForTerrain(const vtString &name);
	wxString GetLanguageCode() { return m_locale.GetCanonicalName(); }
	wxString MakeFrameTitle(vtTerrain *terrain = NULL);

	// Override to help debugging
    void OnAssertFailure(const wxChar *file, int line, const wxChar *func,
						 const wxChar *cond, const wxChar *msg);

	bool m_bShowStartupDialog;

	vtStringArray terrain_files;
	vtStringArray terrain_paths;
	vtStringArray terrain_names;

protected:
	void StartLog();
	void LoadOptions();
	EnviroFrame *CreateMainFrame();
};

// helpers
int EditTerrainParameters(wxWindow *parent, const char *filename);

#ifndef STRING_ORGNAME
#define STRING_ORGNAME "VTP"
#endif

#ifndef STRING_APPNAME
#define STRING_APPNAME "Enviro"
#define ENVIRO_NATIVE
#else
void SetupCustomOptions();
#endif

#define STRING_APPORG STRING_ORGNAME " " STRING_APPNAME

