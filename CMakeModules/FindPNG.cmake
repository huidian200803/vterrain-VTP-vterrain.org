# Defines
# PNG_FOUND
# PNG_INCLUDE_DIR
# PNG_LIBRARIES

set(PNG_FIND_DEBUG TRUE CACHE BOOL "Also search for the debug version of the PNG library")

if(PNG_FIND_DEBUG)
	if(PNG_INCLUDE_DIR AND PNG_LIBRARY AND PNG_LIBRARY_DEBUG)
		set(PNG_FIND_QUIETLY TRUE)
	endif(PNG_INCLUDE_DIR AND PNG_LIBRARY AND PNG_LIBRARY_DEBUG)
else(PNG_FIND_DEBUG)
	if(PNG_INCLUDE_DIR AND PNG_LIBRARY)
		set(PNG_FIND_QUIETLY TRUE)
	endif(PNG_INCLUDE_DIR AND PNG_LIBRARY)
endif(PNG_FIND_DEBUG)

find_path(PNG_INCLUDE_DIR png.h DOC "Directory containing png.h")

find_library(PNG_LIBRARY NAMES png libpng png12 libpng12 DOC "Path to PNG library")
if(PNG_FIND_DEBUG)
	find_library(PNG_LIBRARY_DEBUG NAMES pngd libpngd png12d libpng12d DOC "Path to PNG debug library")
endif(PNG_FIND_DEBUG)

# handle the QUIETLY and REQUIRED arguments and set PNG_FOUND to TRUE if 
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PNG DEFAULT_MSG PNG_LIBRARY PNG_INCLUDE_DIR)

if(PNG_FOUND)
	if(PNG_FIND_DEBUG)
		if(NOT PNG_LIBRARY_DEBUG)
			set(PNG_LIBRARY_DEBUG ${PNG_LIBRARY})
		endif(NOT PNG_LIBRARY_DEBUG)
		set(PNG_LIBRARIES debug ${PNG_LIBRARY_DEBUG} optimized ${PNG_LIBRARY})
	else(PNG_FIND_DEBUG)
		set(PNG_LIBRARIES ${PNG_LIBRARY})
	endif(PNG_FIND_DEBUG)
else(PNG_FOUND)
   SET(PNG_LIBRARIES )
endif(PNG_FOUND)

