//
// LocalDatabuf.h: a wrapper for the Databuf class which is found either in
//  in vtdata or libMini.
//
// Copyright (c) 2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef LocalDatabuf_H
#define LocalDatabuf_H

#define USE_LIBMINI_DATABUF 1

#include "MiniDatabuf.h"

#if USE_LIBMINI_DATABUF

#include <mini/database.h> // part of libMini

// Directly subclass from libMini
class vtMiniDatabuf: public databuf
{
public:
	// A useful method to set the extents (in local CRS) and the corners
	//  (in Geo WGS84) at the same time.
	bool SetBounds(const vtCRS &crs, const DRECT &extents);
};

#else	// not USE_LIBMINI_DATABUF

// Simply use the implementation in vtdata
class vtMiniDatabuf: public MiniDatabuf {};

#endif	// USE_LIBMINI_DATABUF

#endif  // LocalDatabuf_H
