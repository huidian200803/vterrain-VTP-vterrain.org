//
// Copyright (c) 2007-2013 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#include "vtdata/Content.h"

// option tag names
#define TAG_SHOW_MAP "ShowMap"
#define TAG_SHOW_UTM "ShowUTMBounds"
#define TAG_SHOW_PATHS "ShowFullPaths"
#define TAG_ROAD_DRAW_WIDTH "RoadDrawWidth"

#define TAG_USE_CURRENT_CRS	"UseCurrentCRS"
#define TAG_LOAD_IMAGES_ALWAYS "LoadImagesAlways"
#define TAG_LOAD_IMAGES_NEVER "LoadImagesNever"
#define TAG_REPRO_TO_FLOAT_ALWAYS "ReproToFloatAlways"
#define TAG_REPRO_TO_FLOAT_NEVER "ReproToFloatNever"
#define TAG_SAMPLING_N "MultiSampleN"
#define TAG_ELEV_MAX_SIZE "ElevMaxRenderSize"
#define TAG_MAX_MEGAPIXELS "MaxMegapixels"
#define TAG_BLACK_TRANSP "BlackAsTransparent"
#define TAG_TIFF_COMPRESS "TiffCompressDeflate"
#define TAG_DEFAULT_GZIP_BT "DefaultGzipBT"
#define TAG_DELAY_LOAD_GRID "ElevDelayLoadGrid"
#define TAG_MAX_MEM_GRID "ElevMaxMemGrid"
#define TAG_DRAW_RAW_SIMPLE "DrawSimpleRawLayers"
#define TAG_DRAW_TIN_SIMPLE "DrawSimpleTinLayers"

#define TAG_SLOW_FILL_GAPS "SlowFillGaps"	// deprecated
#define TAG_GAP_FILL_METHOD "GapFillMethod"		// 1 fast, 2 slow, 3 region-growing

// status bar options
#define TAG_SHOW_MINUTES "ShowMinutes"
#define TAG_SHOW_VERT_UNITS "ShowVerticalUnits"	// enum LinearUnits

// User Preferences
extern vtTagArray g_Options;

