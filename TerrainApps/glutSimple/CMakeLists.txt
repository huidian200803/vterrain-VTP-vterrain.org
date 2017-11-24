find_package(GLUT)

if(GLUT_FOUND)
	add_executable(glutSimple app.cpp)

	install(TARGETS glutSimple RUNTIME DESTINATION bin)

	# Internal library dependencies for this target
	target_link_libraries(glutSimple vtlib minidata vtdata  xmlhelper)

	# Specify common preprocessor definitions for this target
	set_property(TARGET glutSimple APPEND PROPERTY COMPILE_DEFINITIONS FREEGLUT_LIB_PRAGMAS=0)

	# Windows specific stuff
	if (WIN32)
		set_property(TARGET glutSimple APPEND PROPERTY COMPILE_DEFINITIONS _CRT_SECURE_NO_DEPRECATE)
		set_property(TARGET glutSimple APPEND PROPERTY LINK_FLAGS_DEBUG /NODEFAULTLIB:msvcrt)
	endif (WIN32)

	# External libraries for this target
	if(MINI_FOUND)
		target_link_libraries(glutSimple ${MINI_LIBRARIES})
	endif(MINI_FOUND)

	if(GLUT_FOUND)
		target_link_libraries(glutSimple ${GLUT_LIBRARIES})
	endif(GLUT_FOUND)

	if(OPENGL_FOUND)
		target_link_libraries(glutSimple ${OPENGL_LIBRARIES})
	endif(OPENGL_FOUND)

	if(OSG_FOUND)
		target_link_libraries(glutSimple ${OSG_ALL_LIBRARIES})
	endif (OSG_FOUND)

	if(BZIP2_FOUND)
		target_link_libraries(glutSimple ${BZIP2_LIBRARIES})
	endif(BZIP2_FOUND)

	if(CURL_FOUND)
		target_link_libraries(glutSimple ${CURL_LIBRARIES})
	endif(CURL_FOUND)

	if(GDAL_FOUND)
		target_link_libraries(glutSimple ${GDAL_LIBRARIES})
	endif (GDAL_FOUND)

	if(ZLIB_FOUND)
		target_link_libraries(glutSimple ${ZLIB_LIBRARIES})
	endif(ZLIB_FOUND)

	if(JPEG_FOUND)
		target_link_libraries(glutSimple ${JPEG_LIBRARY})
	endif(JPEG_FOUND)

	if(PNG_FOUND)
		target_link_libraries(glutSimple ${PNG_LIBRARIES})
	endif(PNG_FOUND)


	# Set up include directories for all targets at this level
	include_directories(${GLUT_INCLUDE_DIR})
	if(OSG_FOUND)
		include_directories(${OSG_INCLUDE_DIR})
		include_directories(${OSG_INSTALL_DIR}/include)
	endif(OSG_FOUND)

	if (OSGEARTH_FOUND)
		target_link_libraries(glutSimple ${OSGEARTH_ALL_LIBRARIES})
	endif(OSGEARTH_FOUND)

	if(GDAL_FOUND)
		include_directories(${GDAL_INCLUDE_DIR})
	endif(GDAL_FOUND)

	if(ZLIB_FOUND)
		include_directories(${ZLIB_INCLUDE_DIR})
	endif(ZLIB_FOUND)

endif(GLUT_FOUND)

