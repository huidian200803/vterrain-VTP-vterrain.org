find_package(OpenGL)

add_executable(vtTest app.cpp)

install(TARGETS vtTest RUNTIME DESTINATION bin)

# Internal library dependencies for this target
target_link_libraries(vtTest vtlib minidata vtdata xmlhelper)

# Windows specific stuff
if (WIN32)
	set_property(TARGET vtTest APPEND PROPERTY LINK_FLAGS_DEBUG /NODEFAULTLIB:msvcrt)
endif (WIN32)

# External libraries for this target
if(OSG_FOUND)
	target_link_libraries(vtTest ${OSG_ALL_LIBRARIES})
endif (OSG_FOUND)

if (OSGEARTH_FOUND)
	target_link_libraries(vtTest ${OSGEARTH_ALL_LIBRARIES})
endif(OSGEARTH_FOUND)

if(GDAL_FOUND)
	target_link_libraries(vtTest ${GDAL_LIBRARIES})
endif (GDAL_FOUND)

if(OPENGL_FOUND)
	target_link_libraries(vtTest ${OPENGL_LIBRARIES})
endif(OPENGL_FOUND)

if(CURL_FOUND)
	target_link_libraries(vtTest ${CURL_LIBRARIES})
endif(CURL_FOUND)

if(PNG_FOUND)
	target_link_libraries(vtTest ${PNG_LIBRARIES})
endif(PNG_FOUND)

if(JPEG_FOUND)
	target_link_libraries(vtTest ${JPEG_LIBRARY})
endif(JPEG_FOUND)

if(MINI_FOUND)
	target_link_libraries(vtTest ${MINI_LIBRARIES})
endif(MINI_FOUND)

if(OPENGL_gl_LIBRARY)
	target_link_libraries(vtTest ${OPENGL_gl_LIBRARY})
endif(OPENGL_gl_LIBRARY)

if(OPENGL_glu_LIBRARY)
	target_link_libraries(vtTest ${OPENGL_glu_LIBRARY})
endif(OPENGL_glu_LIBRARY)

if(ZLIB_FOUND)
	target_link_libraries(vtTest ${ZLIB_LIBRARIES})
endif(ZLIB_FOUND)

# Set up include directories for all targets at this level
if(GDAL_FOUND)
	include_directories(${GDAL_INCLUDE_DIR})
endif(GDAL_FOUND)

if(OSG_FOUND)
	include_directories(${OSG_INCLUDE_DIR})
	include_directories(${OSG_INSTALL_DIR}/include)
endif(OSG_FOUND)

if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIR})
endif(ZLIB_FOUND)

if(BZIP2_FOUND)
	target_link_libraries(vtTest ${BZIP2_LIBRARIES})
endif(BZIP2_FOUND)




