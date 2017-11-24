# Add a library target called vtui
set(VTUI_SOURCE_FILES 
		vtui_UI.cpp AutoDialog.cpp BuildingDlg.cpp ColorMapDlg.cpp ContourDlg.cpp DistanceDlg.cpp
		DoubleProgDlg.cpp FeatureTableDlg.cpp Helper.cpp InstanceDlg.cpp Joystick.cpp
		LinearStructDlg.cpp LocaleApp.cpp ProfileDlg.cpp ProfileEditDlg.cpp ProjectionDlg.cpp
		SizeDlg.cpp StatePlaneDlg.cpp TagDlg.cpp)

set(VTUI_HEADER_FILES 
		vtui_UI.h AutoDialog.h BuildingDlg.h ColorMapDlg.h ContourDlg.h DistanceDlg.h
		DoubleProgDlg.h FeatureTableDlg.h Helper.h InstanceDlg.h Joystick.h
		LinearStructDlg.h LocaleApp.h LogCatcher.h ProfileDlg.h ProfileEditDlg.h ProjectionDlg.h
		SizeDlg.h StatePlaneDlg.h TagDlg.h)

if(MSVC)
	add_library(vtui ${VTUI_SOURCE_FILES} ${VTUI_HEADER_FILES} pre.cpp)
	set_source_files_properties(${VTUI_SOURCE_FILES} PROPERTIES COMPILE_FLAGS /Yuwx/wxprec.h)
	set_source_files_properties(pre.cpp PROPERTIES COMPILE_FLAGS /Ycwx/wxprec.h)
else(MSVC)
	add_library(vtui ${VTUI_SOURCE_FILES} ${VTUI_HEADER_FILES})
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

if(QUIKGRID_FOUND)
	include_directories(${QUIKGRID_INCLUDE_DIR})
endif(QUIKGRID_FOUND)
