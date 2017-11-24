set(CMANAGER_SOURCE_FILES
	app.cpp canvas.cpp CManager_UI.cpp frame.cpp ItemGroup.cpp LightDlg.cpp
	ModelDlg.cpp PropDlg.cpp TagDlg.cpp TreeView.cpp)

set(CMANAGER_HEADER_FILES
	app.h canvas.h CManager_UI.h frame.h ItemGroup.h LightDlg.h menu_id.h
	ModelDlg.h PropDlg.h TagDlg.h TreeView.h)

set(CMANAGER_RC_FILES
	CManager.rc)

if(MSVC)
	add_executable(CManager WIN32 ${CMANAGER_SOURCE_FILES} ${CMANAGER_HEADER_FILES} ${CMANAGER_RC_FILES} pre.cpp)
	set_source_files_properties(${CMANAGER_SOURCE_FILES} PROPERTIES COMPILE_FLAGS /Yuwx/wxprec.h)
	set_source_files_properties(pre.cpp PROPERTIES COMPILE_FLAGS /Ycwx/wxprec.h)
else(MSVC)
	add_executable(CManager WIN32 MACOSX_BUNDLE ${CMANAGER_SOURCE_FILES} ${CMANAGER_HEADER_FILES}
			${CMANAGER_RC_FILES})
endif(MSVC)

install(TARGETS CManager RUNTIME DESTINATION bin
                         BUNDLE DESTINATION bin)

# Internal library dependencies for this target
target_link_libraries(CManager wxosg vtlib vtdata vtui xmlhelper minidata)


if (UNIX)
    target_link_libraries(CManager X11)
endif(UNIX)

# Windows specific stuff
if (WIN32)
	set_property(TARGET CManager APPEND PROPERTY LINK_FLAGS_DEBUG /NODEFAULTLIB:msvcrt;wxpngd;wxtiffd;wxjpegd;wxzlibd)
	set_property(TARGET CManager APPEND PROPERTY LINK_FLAGS /NODEFAULTLIB:wxpng;wxtiff;wxjpeg;wxzlib)
endif (WIN32)


# External libraries for this target
if(wxWidgets_FOUND)
	include(${wxWidgets_USE_FILE})
	target_link_libraries(CManager ${wxWidgets_LIBRARIES})
endif(wxWidgets_FOUND)

if(GDAL_FOUND)
	target_link_libraries(CManager ${GDAL_LIBRARIES})
endif (GDAL_FOUND)

if(OSG_FOUND)
	target_link_libraries(CManager ${OSG_ALL_LIBRARIES})
endif (OSG_FOUND)

if (OSGEARTH_FOUND)
	target_link_libraries(CManager ${OSGEARTH_ALL_LIBRARIES})
endif(OSGEARTH_FOUND)

if(BZIP2_FOUND)
	target_link_libraries(CManager ${BZIP2_LIBRARIES})
endif(BZIP2_FOUND)

if(ZLIB_FOUND)
	target_link_libraries(CManager ${ZLIB_LIBRARIES})
endif(ZLIB_FOUND)

if(JPEG_FOUND)
	target_link_libraries(CManager ${JPEG_LIBRARY})
endif(JPEG_FOUND)

if(PNG_FOUND)
	target_link_libraries(CManager ${PNG_LIBRARIES})
endif(PNG_FOUND)

if(MINI_FOUND)
	target_link_libraries(CManager ${MINI_LIBRARIES})
endif(MINI_FOUND)

if(OPENGL_gl_LIBRARY)
	target_link_libraries(CManager ${OPENGL_gl_LIBRARY})
endif(OPENGL_gl_LIBRARY)

if(OPENGL_glu_LIBRARY)
	target_link_libraries(CManager ${OPENGL_glu_LIBRARY})
endif(OPENGL_glu_LIBRARY)

if(CURL_FOUND)
	target_link_libraries(CManager ${CURL_LIBRARIES})
endif(CURL_FOUND)

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



