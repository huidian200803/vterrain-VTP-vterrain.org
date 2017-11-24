# Add a library target called envdlg_xxx
set(ENVDLG_SOURCE_FILES
	EnviroUI.cpp CameraDlg.cpp DistanceDlg3d.cpp DriveDlg.cpp EphemDlg.cpp
	FeatureTableDlg3d.cpp LayerDlg.cpp LinearStructDlg3d.cpp LocationDlg.cpp
	LODDlg.cpp OptionsDlg.cpp PlantDlg.cpp ScenarioParamsDialog.cpp
	ScenarioSelectDialog.cpp StartupDlg.cpp	StyleDlg.cpp TerrManDlg.cpp TextureDlg.cpp
	TinTextureDlg.cpp TParamsDlg.cpp UtilDlg.cpp VehicleDlg.cpp VIADlg.cpp VIAGDALOptionsDlg.cpp)

if(NVPERFSDK_FOUND AND VTP_NVIDIA_PERFORMANCE_MONITORING)
	set(ENVDLG_SOURCE_FILES ${ENVDLG_SOURCE_FILES} PerformanceMonitor.cpp)
endif(NVPERFSDK_FOUND AND VTP_NVIDIA_PERFORMANCE_MONITORING)

set(ENVDLG_HEADER_FILES
	EnviroUI.h CameraDlg.h DistanceDlg3d.h DriveDlg.h EphemDlg.h
	FeatureTableDlg3d.h LayerDlg.h LinearStructDlg3d.h LocationDlg.h LODDlg.h OptionsDlg.h
	PlantDlg.h ScenarioParamsDialog.h ScenarioSelectDialog.h StartupDlg.h
	StyleDlg.h TerrManDlg.h TinTextureDlg.h TParamsDlg.h TextureDlg.h UtilDlg.h
	VehicleDlg.h VIADlg.h VIAGDALOptionsDlg.h)

if(NVPERFSDK_FOUND AND VTP_NVIDIA_PERFORMANCE_MONITORING)
	set(ENVDLG_HEADER_FILES ${ENVDLG_HEADER_FILES} PerformanceMonitor.h)
endif(NVPERFSDK_FOUND AND VTP_NVIDIA_PERFORMANCE_MONITORING)

if(MSVC)
	add_library(envdlg ${ENVDLG_SOURCE_FILES} ${ENVDLG_HEADER_FILES} wx_headers.cpp)
	set_source_files_properties(${ENVDLG_SOURCE_FILES} PROPERTIES COMPILE_FLAGS /Yuwx/wxprec.h)
	set_source_files_properties(wx_headers.cpp PROPERTIES COMPILE_FLAGS /Ycwx/wxprec.h)
else(MSVC)
	add_library(envdlg ${ENVDLG_SOURCE_FILES} ${ENVDLG_HEADER_FILES})
endif(MSVC)

# Performance monitoring
if(NVPERFSDK_FOUND AND VTP_NVIDIA_PERFORMANCE_MONITORING)
	set_property(TARGET envdlg APPEND PROPERTY COMPILE_DEFINITIONS VTP_NVIDIA_PERFORMANCE_MONITORING=1)
endif(NVPERFSDK_FOUND AND VTP_NVIDIA_PERFORMANCE_MONITORING)

# Set up include directories for all targets at this level
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/icons)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/bitmap)

if(wxWidgets_FOUND)
	include(${wxWidgets_USE_FILE})
endif(wxWidgets_FOUND)

if(GDAL_FOUND)
	include_directories(${GDAL_INCLUDE_DIR})
endif(GDAL_FOUND)

if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIR})
endif(ZLIB_FOUND)

if(OSG_FOUND)
	include_directories(${OSG_INCLUDE_DIR})
	include_directories(${OSG_INSTALL_DIR}/include)
endif(OSG_FOUND)

if(MINI_FOUND)
	include_directories(${MINI_INCLUDE_DIR})
endif(MINI_FOUND)

if(NVPERFSDK_FOUND AND VTP_NVIDIA_PERFORMANCE_MONITORING)
	include_directories(${NVPERFSDK_INCLUDE_DIR})
endif(NVPERFSDK_FOUND AND VTP_NVIDIA_PERFORMANCE_MONITORING)

