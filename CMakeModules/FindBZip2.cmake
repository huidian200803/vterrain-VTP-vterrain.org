# - Try to find BZip2
# Once done this will define
#
#  BZIP2_FOUND - system has BZip2
#  BZIP2_INCLUDE_DIR - the BZip2 include directory
#  BZIP2_LIBRARIES - Link these to use BZip2
#  BZIP2_DEFINITIONS - Compiler switches required for using BZip2
#  BZIP2_NEED_PREFIX - this is set if the functions are prefixed with BZ2_

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
# Modified for debug libraries by Roger James 07/03/09

set(BZip2_FIND_DEBUG TRUE CACHE BOOL "Also search for the debug version of the BZip2 library")

if(BZip2_FIND_DEBUG)
	IF (BZIP2_INCLUDE_DIR AND BZIP2_LIBRARY AND BZIP2_LIBRARY_DEBUG)
		SET(BZip2_FIND_QUIETLY TRUE)
	ENDIF (BZIP2_INCLUDE_DIR AND BZIP2_LIBRARY AND BZIP2_LIBRARY_DEBUG)
else(BZip2_FIND_DEBUG)
	IF (BZIP2_INCLUDE_DIR AND BZIP2_LIBRARY)
		SET(BZip2_FIND_QUIETLY TRUE)
	ENDIF (BZIP2_INCLUDE_DIR AND BZIP2_LIBRARY)
endif(BZip2_FIND_DEBUG)

FIND_PATH(BZIP2_INCLUDE_DIR bzlib.h )

FIND_LIBRARY(BZIP2_LIBRARY NAMES bz2 bz2d bzip2 )
if(BZip2_FIND_DEBUG)
	FIND_LIBRARY(BZIP2_LIBRARY_DEBUG NAMES bz2d bzip2d )
endif(BZip2_FIND_DEBUG)

# handle the QUIETLY and REQUIRED arguments and set BZip2_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BZip2 DEFAULT_MSG BZIP2_LIBRARY BZIP2_INCLUDE_DIR)


IF (BZIP2_FOUND)
	INCLUDE(CheckLibraryExists)
	CHECK_LIBRARY_EXISTS(${BZIP2_LIBRARY} BZ2_bzCompressInit "" BZIP2_NEED_PREFIX)
	if(BZip2_FIND_DEBUG)
		IF(NOT BZIP2_LIBRARY_DEBUG)
			SET(BZIP2_LIBRARY_DEBUG ${BZIP2_LIBRARY})
		ENDIF(NOT BZIP2_LIBRARY_DEBUG)
   		SET(BZIP2_LIBRARIES debug ${BZIP2_LIBRARY_DEBUG} optimized ${BZIP2_LIBRARY} )
	else(BZip2_FIND_DEBUG)
   		SET(BZIP2_LIBRARIES ${BZIP2_LIBRARY} )
	endif(BZip2_FIND_DEBUG)
ELSE (BZIP2_FOUND)
	SET(BZIP2_LIBRARIES )
ENDIF (BZIP2_FOUND)

