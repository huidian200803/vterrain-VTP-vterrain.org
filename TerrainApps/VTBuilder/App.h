//
// App.h
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtui/LocaleApp.h"

class BuilderApp: public LocaleApp
{
public:
	void Args(int argc, wxChar **argv);
	bool OnInit();
	int OnExit();
};

