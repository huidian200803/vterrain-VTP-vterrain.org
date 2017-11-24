
add_executable(VTConvert VTConvert.cpp)

install(TARGETS VTConvert RUNTIME DESTINATION bin)

# Internal library dependencies for this target
target_link_libraries(VTConvert vtdata)

# Specify debug preprocessor definitions for this target
#set_property(TARGET VTConvert APPEND PROPERTY COMPILE_DEFINITIONS_DEBUG VTDEBUG)

# Windows specific stuff
if (WIN32)
	set_property(TARGET VTConvert APPEND PROPERTY LINK_FLAGS_DEBUG /NODEFAULTLIB:msvcrt)
endif (WIN32)

# External libraries for this target
if(BZIP2_FOUND)
	target_link_libraries(VTConvert ${BZIP2_LIBRARIES})
endif(BZIP2_FOUND)

if(GDAL_FOUND)
	target_link_libraries(VTConvert ${GDAL_LIBRARIES})
endif (GDAL_FOUND)

if(ZLIB_FOUND)
	target_link_libraries(VTConvert ${ZLIB_LIBRARIES})
endif(ZLIB_FOUND)

if(JPEG_FOUND)
	target_link_libraries(VTConvert ${JPEG_LIBRARY})
endif(JPEG_FOUND)

if(PNG_FOUND)
	target_link_libraries(VTConvert ${PNG_LIBRARIES})
endif(PNG_FOUND)

# Set up include directories for all targets at this level
include_directories(${TERRAIN_SDK_ROOT})

if(GDAL_FOUND)
	include_directories(${GDAL_INCLUDE_DIR})
endif(GDAL_FOUND)

if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIR})
endif(ZLIB_FOUND)

