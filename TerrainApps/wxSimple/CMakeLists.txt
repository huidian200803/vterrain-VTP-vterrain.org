set(WXSIMPLE_SOURCE_FILES app.cpp canvas.cpp frame.cpp)
set(WXSIMPLE_HEADER_FILES app.h canvas.h frame.h)

if(MSVC)
	add_executable(wxSimple WIN32 ${WXSIMPLE_SOURCE_FILES} ${WXSIMPLE_HEADER_FILES} wx_headers.cpp)
	set_source_files_properties(${WXSIMPLE_SOURCE_FILES} PROPERTIES COMPILE_FLAGS "/Yuwx/wxprec.h")
	set_source_files_properties(wx_headers.cpp PROPERTIES COMPILE_FLAGS "/Ycwx/wxprec.h")
else(MSVC)
	add_executable(wxSimple WIN32 MACOSX_BUNDLE ${WXSIMPLE_SOURCE_FILES} ${WXSIMPLE_HEADER_FILES})
endif(MSVC)

install(TARGETS wxSimple RUNTIME DESTINATION bin
                         BUNDLE DESTINATION bin)

# Internal library dependencies for this target
target_link_libraries(wxSimple wxosg vtlib minidata vtdata xmlhelper vtui)

# Windows specific stuff
if (WIN32)
	set_property(TARGET wxSimple APPEND PROPERTY LINK_FLAGS_DEBUG /NODEFAULTLIB:msvcrt)
endif (WIN32)

if (UNIX)
    target_link_libraries(wxSimple X11)
endif(UNIX)

# External libraries for this target
if(wxWidgets_FOUND)
	include(${wxWidgets_USE_FILE})
	target_link_libraries(wxSimple ${wxWidgets_LIBRARIES})
endif(wxWidgets_FOUND)

if(GDAL_FOUND)
	target_link_libraries(wxSimple ${GDAL_LIBRARIES})
endif (GDAL_FOUND)

if(OSG_FOUND)
	target_link_libraries(wxSimple ${OSG_ALL_LIBRARIES})
endif (OSG_FOUND)

if (OSGEARTH_FOUND)
	target_link_libraries(wxSimple ${OSGEARTH_ALL_LIBRARIES})
endif(OSGEARTH_FOUND)

if(BZIP2_FOUND)
	target_link_libraries(wxSimple ${BZIP2_LIBRARIES})
endif(BZIP2_FOUND)

if(CURL_FOUND)
	target_link_libraries(wxSimple ${CURL_LIBRARIES})
endif(CURL_FOUND)

if(ZLIB_FOUND)
	target_link_libraries(wxSimple ${ZLIB_LIBRARIES})
endif(ZLIB_FOUND)

if(JPEG_FOUND)
	target_link_libraries(wxSimple ${JPEG_LIBRARY})
endif(JPEG_FOUND)

if(PNG_FOUND)
	target_link_libraries(wxSimple ${PNG_LIBRARIES})
endif(PNG_FOUND)

if(QUIKGRID_FOUND)
	target_link_libraries(wxSimple ${QUIKGRID_LIBRARIES})
endif(QUIKGRID_FOUND)

if(MINI_FOUND)
	target_link_libraries(wxSimple ${MINI_LIBRARIES})
endif(MINI_FOUND)

if(OPENGL_gl_LIBRARY)
	target_link_libraries(wxSimple ${OPENGL_gl_LIBRARY})
endif(OPENGL_gl_LIBRARY)

if(OPENGL_glu_LIBRARY)
	target_link_libraries(wxSimple ${OPENGL_glu_LIBRARY})
endif(OPENGL_glu_LIBRARY)

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

find_path(GLEXT_INCLUDE_DIR GL/glext.h DOC "Directory containing GL/glext.h")
if (GLEXT_INCLUDE_DIR)
	include_directories(${GLEXT_INCLUDE_DIR})
endif (GLEXT_INCLUDE_DIR)
