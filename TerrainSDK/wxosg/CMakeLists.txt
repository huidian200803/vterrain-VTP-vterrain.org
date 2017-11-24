# Add a library target called wxosg
set(WXOSG_SOURCE_FILES 
		wxosg_UI.cpp BuildingDlg3d.cpp Canvas.cpp SceneGraphDlg.cpp TimeDlg.cpp
		GraphicsWindowWX.cpp)
set(WXOSG_HEADER_FILES 
		wxosg_UI.h BuildingDlg3d.h Canvas.h SceneGraphDlg.h TimeDlg.h
		GraphicsWindowWX.h)

if(MSVC)
	add_library(wxosg ${WXOSG_SOURCE_FILES} ${WXOSG_HEADER_FILES} pre.cpp)
	set_source_files_properties(${WXOSG_SOURCE_FILES} PROPERTIES COMPILE_FLAGS /Yuwx/wxprec.h)
	set_source_files_properties(pre.cpp PROPERTIES COMPILE_FLAGS /Ycwx/wxprec.h)
else(MSVC)
	add_library(wxosg ${WXOSG_SOURCE_FILES} ${WXOSG_HEADER_FILES})
endif(MSVC)

include_directories(${TERRAIN_SDK_ROOT})

if(wxWidgets_FOUND)
	include(${wxWidgets_USE_FILE})
endif(wxWidgets_FOUND)

if(GDAL_FOUND)
	include_directories(${GDAL_INCLUDE_DIR})
endif (GDAL_FOUND)

if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIR})
endif (ZLIB_FOUND)

if(OSG_FOUND)
	include_directories(${OSG_INCLUDE_DIR})
	include_directories(${OSG_INSTALL_DIR}/include)
endif(OSG_FOUND)

