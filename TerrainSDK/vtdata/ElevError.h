//
// ElevError.h
//

#pragma once

#include "vtString.h"

/**
 Contains a description an error, for any operation on elevation.
 */
struct vtElevError {
	enum ErrorType {
		FILE_OPEN,			/// Could not open file.
		ALLOCATE,			/// Couldn't allocate that much memory.
		UNKNOWN_FORMAT,		/// Couldn't determine the file format.
		NOT_FORMAT,			/// Not the correct file format.
		UNSUPPORTED_VERSION,/// File is an unsupported version.
		READ_CRS,			/// Could not read the CRS.
		READ_DATA,			/// Could not read the file data.
		CONVERT_CRS,		/// Could not convert CRS
		GRID_SIZE,			/// Unsupported grid size
		CANCELLED			/// User cancelled the file read.
	} type;
	vtString message;
};

