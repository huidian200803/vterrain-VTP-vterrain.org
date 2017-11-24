//
// This file configures the dependencies for the vtdata library.
//
// Set each option to either 0 or 1 as desired, or you can set them beforehand
//  in the build process.
//

#ifndef CONFIG_VTDATA_H
#define CONFIG_VTDATA_H

// Use the bzip2 library, which adds support for reading from .bz2 files
//
// The library is free (BSD-style license) and available from:
// http://www.bzip.org/
//
#ifndef SUPPORT_BZIP2
#define SUPPORT_BZIP2	0
#endif

// Use the Curl library, which adds cross-platform support for HTTP
// operations such as reading from WFS (Web Feature Servers).
//
// The Curl library is available from:
//		http://curl.haxx.se/libcurl/
//
#ifndef SUPPORT_CURL
#define SUPPORT_CURL	0
#endif

// Use the QuikGrid library, which adds support for operations such as
// generating contour lines.
//
// The QuikGrid library is LGPL and available from:
//		http://www.perspectiveedge.com/
//
#ifndef SUPPORT_QUIKGRID
#define SUPPORT_QUIKGRID	0
#endif

// Use the 'squish' texture compression library,
//
// The library is free (MIT license) and available from:
// http://sjbrown.co.uk/?code=squish
//
#ifndef SUPPORT_SQUISH
#define SUPPORT_SQUISH	0
#endif

// Set to 1 if your C++ compiler supports wide strings (std::wstring)
//
// Apparently, there is some environment on the Macintosh without this.
// If 0, then international characters in strings may not (will not)
// be handled correctly.
//
#ifndef SUPPORT_WSTRING
#define SUPPORT_WSTRING	1
#endif

// Put these useful typedefs here so that they can be used throughout the
// VTP codebase.
typedef unsigned int uint;
typedef unsigned char uchar;

// Because vtdata (and the apps which use it) tend to make heavy use of GDAL,
// this seems an appropriate place to disable one of the most irritating
// MSVC/GDAL warnings.
#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#endif

#endif // CONFIG_VTDATA_H

