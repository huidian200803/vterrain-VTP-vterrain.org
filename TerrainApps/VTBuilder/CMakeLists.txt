# Add a library target called BuilderLib
set(BUILDERLIB_SOURCE_FILES
	Builder.cpp BuilderView.cpp ChunkDlg.cpp ElevLayer.cpp Export.cpp
	ExtentDlg.cpp GLTexture.cpp ImageGLCanvas.cpp ImageLayer.cpp Import.cpp
	ImportPointDlg.cpp ImportOSM.cpp ImportStructDlg.cpp ImportStructDlgOGR.cpp
	ImportVegDlg.cpp
	Layer.cpp NodeDlg.cpp RawDlg.cpp RawLayer.cpp RenderOptionsDlg.cpp
	RoadDlg.cpp RoadLayer.cpp RoadMapEdit.cpp RoadMapIO.cpp
	SampleElevationDlg.cpp SampleImageDlg.cpp ScaledView.cpp StructLayer.cpp
	TileDlg.cpp Tin2d.cpp
	UtilityLayer.cpp VegFieldsDlg.cpp VegLayer.cpp vtBitmap.cpp VTBuilder_UI.cpp
	vtImage.cpp WaterLayer.cpp)

set(BUILDERLIB_SOURCE_FILES_NO_PCH
	ElevDrawOptions.cpp)

set(BUILDERLIB_HEADER_FILES
	Builder.h BuilderView.h ChunkDlg.h ElevDrawOptions.h ElevLayer.h ExtentDlg.h
	GLTexture.h ImageGLCanvas.h ImageLayer.h ImportPointDlg.h ImportStructDlg.h
	ImportStructDlgOGR.h ImportVegDlg.h Layer.h NodeDlg.h Options.h RawDlg.h
	RawLayer.h RenderOptionsDlg.h RoadDlg.h RoadLayer.h
	RoadMapEdit.h SampleElevationDlg.h SampleImageDlg.h ScaledView.h StructLayer.h TileDlg.h Tin2d.h
	UIContext.h UtilityLayer.h VegFieldsDlg.h VegLayer.h vtBitmap.h VTBuilder_UI.h vtImage.h
	WaterLayer.h)

if(MSVC)
	add_library(BuilderLib ${BUILDERLIB_SOURCE_FILES} ${BUILDERLIB_SOURCE_FILES_NO_PCH} ${BUILDERLIB_HEADER_FILES} pre.cpp)
	set_source_files_properties(${BUILDERLIB_SOURCE_FILES} PROPERTIES COMPILE_FLAGS /Yuwx/wxprec.h)
	set_source_files_properties(pre.cpp PROPERTIES COMPILE_FLAGS /Ycwx/wxprec.h)
else(MSVC)
	add_library(BuilderLib ${BUILDERLIB_SOURCE_FILES} ${BUILDERLIB_SOURCE_FILES_NO_PCH} ${BUILDERLIB_HEADER_FILES})
endif(MSVC)

# Specify common preprocessor definitions for this target
set_property(TARGET BuilderLib APPEND PROPERTY COMPILE_DEFINITIONS USE_OPENGL=1)

set(VTBUILDER_SOURCE_FILES
	App.cpp Frame.cpp FrameMenus.cpp Geocode.cpp RoadFix.cpp Special.cpp
	StatusBar.cpp TreeView.cpp)

set(VTBUILDER_DIALOG_SOURCE_FILES
	DistanceDlg2d.cpp DistribVegDlg.cpp ElevMathDlg.cpp FeatInfoDlg.cpp GenGridDlg.cpp
	GeocodeDlg.cpp ImageMapDlg.cpp LayerPropDlg.cpp MapServerDlg.cpp
	MatchDlg.cpp NodeDlg.cpp OptionsDlg.cpp PrefDlg.cpp RenderDlg.cpp
	SelectDlg.cpp VegDlg.cpp)
set(VTBUILDER_HEADER_FILES
	App.h DistanceDlg2d.h DistribVegDlg.h ElevMathDlg.h FeatInfoDlg.h Frame.h
	GenGridDlg.h GeocodeDlg.h ImageMapDlg.h LayerPropDlg.h MapServerDlg.h
	MatchDlg.h MenuEnum.h OptionsDlg.h PrefDlg.h RenderDlg.h RoadMapEdit.h
	SelectDlg.h StatusBar.h TreeView.h VegDlg.h VegPointOptions.h)
set(VTBUILDER_RC_FILES
	VTBuilder.rc)
if(MSVC)
	add_executable(VTBuilder WIN32 ${VTBUILDER_SOURCE_FILES} ${VTBUILDER_DIALOG_SOURCE_FILES} ${VTBUILDER_HEADER_FILES} pre.cpp
		${VTBUILDER_RC_FILES})
	set_source_files_properties(${VTBUILDER_SOURCE_FILES} PROPERTIES COMPILE_FLAGS /Yuwx/wxprec.h)
	set_source_files_properties(${VTBUILDER_DIALOG_SOURCE_FILES} PROPERTIES COMPILE_FLAGS /Yuwx/wxprec.h)
	# already handled pre.cpp earlier on
else(MSVC)
	add_executable(VTBuilder WIN32 MACOSX_BUNDLE ${VTBUILDER_SOURCE_FILES} ${VTBUILDER_DIALOG_SOURCE_FILES} ${VTBUILDER_HEADER_FILES}
		${VTBUILDER_RC_FILES})
endif(MSVC)

install(TARGETS VTBuilder RUNTIME DESTINATION bin
                          BUNDLE DESTINATION bin)

# Internal dependencies for this target
target_link_libraries(VTBuilder BuilderLib vtui vtdata xmlhelper minidata unzip)

# Specify common preprocessor definitions for this target
set_property(TARGET VTBuilder APPEND PROPERTY COMPILE_DEFINITIONS USE_OPENGL=1)

if (UNIX)
    target_link_libraries(VTBuilder dl)
endif(UNIX)

# Windows specific stuff
if (WIN32)
	set_property(TARGET VTBuilder APPEND PROPERTY LINK_FLAGS_DEBUG "/NODEFAULTLIB:msvcrt")
endif (WIN32)

# External libraries for this target
if(wxWidgets_FOUND)
	include(${wxWidgets_USE_FILE})
	target_link_libraries(VTBuilder ${wxWidgets_LIBRARIES})
endif(wxWidgets_FOUND)

if(GDAL_FOUND)
	target_link_libraries(VTBuilder ${GDAL_LIBRARIES})
endif (GDAL_FOUND)

if(BZIP2_FOUND)
	target_link_libraries(VTBuilder ${BZIP2_LIBRARIES})
endif(BZIP2_FOUND)

if(CURL_FOUND)
	target_link_libraries(VTBuilder ${CURL_LIBRARIES})
endif(CURL_FOUND)

if(ZLIB_FOUND)
	target_link_libraries(VTBuilder ${ZLIB_LIBRARIES})
endif(ZLIB_FOUND)

if(JPEG_FOUND)
	target_link_libraries(VTBuilder ${JPEG_LIBRARY})
endif(JPEG_FOUND)

if(PNG_FOUND)
	target_link_libraries(VTBuilder ${PNG_LIBRARIES})
endif(PNG_FOUND)

find_library(SQUISH_LIBRARY squish DOC "Path to squish library")
if(SQUISH_LIBRARY)
	target_link_libraries(VTBuilder ${SQUISH_LIBRARY})
	set_property(TARGET VTBuilder APPEND PROPERTY COMPILE_DEFINITIONS SUPPORT_SQUISH)
	set_property(TARGET BuilderLib APPEND PROPERTY COMPILE_DEFINITIONS SUPPORT_SQUISH)
endif(SQUISH_LIBRARY)

if(QUIKGRID_FOUND)
	target_link_libraries(VTBuilder ${QUIKGRID_LIBRARIES})
endif(QUIKGRID_FOUND)

if(MINI_FOUND)
	target_link_libraries(VTBuilder ${MINI_LIBRARIES})
endif(MINI_FOUND)

if(OPENGL_FOUND)
	target_link_libraries(VTBuilder ${OPENGL_gl_LIBRARY})
endif(OPENGL_FOUND)

if(OPENGL_GLU_FOUND)
	target_link_libraries(VTBuilder ${OPENGL_glu_LIBRARY})
endif(OPENGL_GLU_FOUND)

# Set up include directories for all targets at this level
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/bitmaps)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/icons)

if(GDAL_FOUND)
	include_directories(${GDAL_INCLUDE_DIR})
endif(GDAL_FOUND)

if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIR})
endif(ZLIB_FOUND)

if(MINI_FOUND)
	include_directories(${MINI_INCLUDE_DIR})
endif(MINI_FOUND)

if(PNG_FOUND)
	# Add the png include directory to the compilers search path 
	include_directories(${PNG_INCLUDE_DIR})
endif(PNG_FOUND)

if(QUIKGRID_FOUND)
	include_directories(${QUIKGRID_INCLUDE_DIR})
endif(QUIKGRID_FOUND)

if(JPEG_FOUND)
	include_directories(${JPEG_INCLUDE_DIR})
endif(JPEG_FOUND)

# Find the directory containing squish.h
find_path(SQUISH_INCLUDE_DIR squish.h DOC "Directory containing squish.h")
if (SQUISH_INCLUDE_DIR)
	# Add the squish include directory to the compilers search path 
	include_directories(${SQUISH_INCLUDE_DIR})
endif (SQUISH_INCLUDE_DIR)

# Find the directory containing GL/glext.h
find_path(GLEXT_INCLUDE_DIR GL/glext.h DOC "Directory containing GL/glext.h")
if (GLEXT_INCLUDE_DIR)
	# Add the glext include directory to the compilers search path 
	include_directories(${GLEXT_INCLUDE_DIR})
endif (GLEXT_INCLUDE_DIR)
