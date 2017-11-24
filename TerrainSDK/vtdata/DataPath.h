//
// DataPath.h
//
// Copyright (c) 2007-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __DataPath_h__
#define __DataPath_h__

#include "vtdata/vtString.h"

void vtSetDataPath(const vtStringArray &paths);
vtStringArray &vtGetDataPath();
bool vtLoadDataPath(const char *user_config_dir = NULL,
					const char *config_dir = NULL);
bool vtSaveDataPath(const char *fname = NULL);
bool MakeRelativeToDataPath(vtString &fname, const vtString &folder);

#endif // __DataPath_h__
