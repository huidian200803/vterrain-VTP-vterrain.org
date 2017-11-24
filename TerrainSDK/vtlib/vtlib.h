//
// Main header for VT library, for all platforms
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// frequently used standard headers
#include <stdio.h>

// vtdata headers
#include "vtdata/config_vtdata.h"
#include "vtdata/MathTypes.h"

// OpenGL headers
#if WIN32
	// these definitions let us include gl.h without the entire Windows headers
	#include "core/winglue.h"
#endif

#ifdef __DARWIN_OSX__
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

///////////////////////////////// OSG //////////////////////////////

#include "vtosg/Mesh.h"
#include "vtosg/Material.h"
#include "vtosg/ImageOSG.h"
#include "vtosg/MathOSG.h"
#include "vtosg/NodeOSG.h"
#include "vtosg/SceneOSG.h"

#if defined(_MSC_VER) && _DEBUG && !defined(_STDAFX_H_INCLUDED_) && 0
#pragma message("(Including MemTracker)")
#include "vtlib/core/MemoryTracker.h"
#endif

