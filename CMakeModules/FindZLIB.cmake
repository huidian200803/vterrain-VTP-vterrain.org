# Defines
# ZLIB_FOUND
# ZLIB_INCLUDE_DIR
# ZLIB_LIBRARIES

set(ZLIB_FIND_DEBUG TRUE CACHE BOOL "Also search for the debug version of the ZLIB library")

if(ZLIB_FIND_DEBUG)
	if(ZLIB_INCLUDE_DIR AND ZLIB_LIBRARY AND ZLIB_LIBRARY_DEBUG)
		set(ZLIB_FIND_QUIETLY TRUE)
	endif(ZLIB_INCLUDE_DIR AND ZLIB_LIBRARY AND ZLIB_LIBRARY_DEBUG)
else(ZLIB_FIND_DEBUG)
	if(ZLIB_INCLUDE_DIR AND ZLIB_LIBRARY)
		set(ZLIB_FIND_QUIETLY TRUE)
	endif(ZLIB_INCLUDE_DIR AND ZLIB_LIBRARY)
endif(ZLIB_FIND_DEBUG)

find_path(ZLIB_INCLUDE_DIR NAMES zlib.h DOC "Directory containing zlib.h")

find_library(ZLIB_LIBRARY NAMES z zlib zdll zlib1 DOC "Path to zlib library")
if(ZLIB_FIND_DEBUG)
	find_library(ZLIB_LIBRARY_DEBUG NAMES zd zlibd zdlld zlib1d DOC "Path to zlib debug library")
endif(ZLIB_FIND_DEBUG)

# handle the QUIETLY and REQUIRED arguments and set ZLIB_FOUND to TRUE if 
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ZLIB DEFAULT_MSG ZLIB_LIBRARY ZLIB_INCLUDE_DIR)

if(ZLIB_FOUND)
	if(ZLIB_FIND_DEBUG)
		if(NOT ZLIB_LIBRARY_DEBUG)
			set(ZLIB_LIBRARY_DEBUG ${ZLIB_LIBRARY})
		endif(NOT ZLIB_LIBRARY_DEBUG)
		set(ZLIB_LIBRARIES debug ${ZLIB_LIBRARY_DEBUG} optimized ${ZLIB_LIBRARY})
	else(ZLIB_FIND_DEBUG)
		set(ZLIB_LIBRARIES ${ZLIB_LIBRARY})
	endif(ZLIB_FIND_DEBUG)
else(ZLIB_FOUND)
	SET(ZLIB_LIBRARIES )
endif(ZLIB_FOUND)
