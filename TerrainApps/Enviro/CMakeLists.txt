add_subdirectory(wx)

set(ENVIRO_SOURCE_FILES
	EarthView.cpp Engines.cpp Enviro.cpp Hawaii.cpp Options.cpp)

set(ENVIRO_SOURCE_FILES_WX
	wx/EnviroCanvas.cpp wx/EnviroApp.cpp wx/EnviroFrame.cpp wx/EnviroGUI.cpp wx/EnviroMenus.cpp wx/StatusBar.cpp)

set(ENVIRO_HEADER_FILES
	Engines.h Enviro.h EnviroEnum.h Hawaii.h Options.h PlantingOptions.h)

set(ENVIRO_HEADER_FILES_WX
	wx/EnviroCanvas.h wx/EnviroApp.h wx/EnviroFrame.h wx/EnviroGUI.h wx/menu_id.h wx/StatusBar.h)

set(ENVIRO_RC_FILES
	wx/enviro-wx.rc)

if(MSVC)
	add_executable(Enviro WIN32 ${ENVIRO_SOURCE_FILES} ${ENVIRO_SOURCE_FILES_WX} ${ENVIRO_HEADER_FILES}
		${ENVIRO_HEADER_FILES_WX} ${ENVIRO_RC_FILES} vtlib_headers.cpp wx/wx_headers.cpp)
	# When I can find a way of having telling CMake to pch files in the build directories (Debug Release etc.)
	# then I can have more than pch file. One possible way round thnis would be to force vtlib_headers.cpp
	# to be recompiled on every build
	#set_source_files_properties(${ENVIRO_SOURCE_FILES} PROPERTIES COMPILE_FLAGS "/Yuvtlib/vtlib.h /Fpvtlib.pch")
	#set_source_files_properties(vtlib_headers.cpp PROPERTIES COMPILE_FLAGS "/Ycvtlib/vtlib.h /Fpvtlib.pch")
	set_source_files_properties(${ENVIRO_SOURCE_FILES_WX} PROPERTIES COMPILE_FLAGS /Yuwx/wxprec.h)
	set_source_files_properties(wx/wx_headers.cpp PROPERTIES COMPILE_FLAGS /Ycwx/wxprec.h)
else(MSVC)
	add_executable(Enviro WIN32 MACOSX_BUNDLE ${ENVIRO_SOURCE_FILES} ${ENVIRO_SOURCE_FILES_WX} ${ENVIRO_HEADER_FILES}
		${ENVIRO_HEADER_FILES_WX} ${ENVIRO_RC_FILES})
endif(MSVC)

install(TARGETS Enviro RUNTIME DESTINATION bin
                       BUNDLE DESTINATION bin)

# Special versions of Enviro can be created by using the same files with a different frame
# include("Pathfinder/CMakeLists.txt")

# Internal dependencies for this target
target_link_libraries(Enviro envdlg wxosg vtlib vtui minidata vtdata xmlhelper)

if (UNIX)
    target_link_libraries(Enviro X11)
endif(UNIX)

# Performance monitoring
if(NVPERFSDK_FOUND AND VTP_NVIDIA_PERFORMANCE_MONITORING)
	set_property(TARGET Enviro APPEND PROPERTY COMPILE_DEFINITIONS VTP_NVIDIA_PERFORMANCE_MONITORING=1)
	target_link_libraries(Enviro ${NVPERFSDK_LIBRARY})
endif(NVPERFSDK_FOUND AND VTP_NVIDIA_PERFORMANCE_MONITORING)

# Windows specific stuff
if (WIN32)
	set_property(TARGET Enviro APPEND PROPERTY LINK_FLAGS_DEBUG /NODEFAULTLIB:msvcrt;wxpngd;wxtiffd;wxjpegd;wxzlibd)
	set_property(TARGET Enviro APPEND PROPERTY LINK_FLAGS /NODEFAULTLIB:wxpng;wxtiff;wxjpeg;wxzlib)
endif (WIN32)

# Specify debug preprocessor definitions for this target
#set_property(TARGET Enviro APPEND PROPERTY COMPILE_DEFINITIONS_DEBUG VTDEBUG)

# External libraries for this target
if(wxWidgets_FOUND)
	target_link_libraries(Enviro ${wxWidgets_LIBRARIES})
endif(wxWidgets_FOUND)

if(GDAL_FOUND)
	target_link_libraries(Enviro ${GDAL_LIBRARIES})
endif (GDAL_FOUND)

if(OSG_FOUND)
	target_link_libraries(Enviro ${OSG_ALL_LIBRARIES})
endif (OSG_FOUND)

if (OSGEARTH_FOUND)
	target_link_libraries(Enviro ${OSGEARTH_ALL_LIBRARIES})
endif(OSGEARTH_FOUND)

if(BZIP2_FOUND)
	target_link_libraries(Enviro ${BZIP2_LIBRARIES})
endif(BZIP2_FOUND)

if(CURL_FOUND)
	target_link_libraries(Enviro ${CURL_LIBRARIES})
endif(CURL_FOUND)

if(ZLIB_FOUND)
	target_link_libraries(Enviro ${ZLIB_LIBRARIES})
endif(ZLIB_FOUND)

if(JPEG_FOUND)
	target_link_libraries(Enviro ${JPEG_LIBRARY})
endif(JPEG_FOUND)

if(PNG_FOUND)
	target_link_libraries(Enviro ${PNG_LIBRARIES})
endif(PNG_FOUND)

if(QUIKGRID_FOUND)
	target_link_libraries(Enviro ${QUIKGRID_LIBRARIES})
endif(QUIKGRID_FOUND)

if(MINI_FOUND)
	target_link_libraries(Enviro ${MINI_LIBRARIES})
endif(MINI_FOUND)

if(OPENGL_gl_LIBRARY)
	target_link_libraries(Enviro ${OPENGL_gl_LIBRARY})
endif(OPENGL_gl_LIBRARY)

if(OPENGL_glu_LIBRARY)
	target_link_libraries(Enviro ${OPENGL_glu_LIBRARY})
endif(OPENGL_glu_LIBRARY)

# Set up include directories for all targets at this level
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/wx/icons)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/wx/bitmap)

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

if(wxWidgets_FOUND)
	include(${wxWidgets_USE_FILE})
endif(wxWidgets_FOUND)
