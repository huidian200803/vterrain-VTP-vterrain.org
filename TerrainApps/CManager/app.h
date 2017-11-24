//
// Name: app.h
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

class vtTrackball;
class vtGroup;

// Define a new application type
class vtApp: public wxApp
{
public:
	bool OnInit(void);
	int OnExit(void);

	vtTrackball	*m_pTrackball;
	vtGroupPtr m_pRoot;
};

