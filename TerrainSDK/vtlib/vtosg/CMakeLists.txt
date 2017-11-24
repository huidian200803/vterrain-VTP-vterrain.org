
set(VTLIB_OSG_SOURCE_FILES ExternalHeightField3d.cpp GeometryUtils.cpp GroupLOD.cpp
		ImageOSG.cpp LightSpacePerspectiveShadowTechnique.cpp Material.cpp Mesh.cpp
		NodeLog.cpp NodeOSG.cpp OSGEventHandler.cpp SaveImageOSG.cpp SceneOSG.cpp
		MultiTexture.cpp ScreenCaptureHandler.cpp SimpleInterimShadowTechnique.cpp
		VisualImpactCalculatorOSG.cpp)
set(VTLIB_OSG_HEADER_FILES ExternalHeightField3d.h GeometryUtils.h GroupLOD.h
		ImageOSG.h LightSpacePerspectiveShadowTechnique.h Material.h MathOSG.h
		Mesh.h MultiTexture.h NodeOSG.h OSGEventHandler.h SaveImageOSG.h SceneOSG.h
		ScreenCaptureHandler.h SimpleInterimShadowTechnique.h
		VisualImpactCalculatorOSG.h)

# Add a library target called vtlib

if(MSVC)
	add_library(vtlib ${VTLIB_CORE_SOURCE_FILES} ${VTLIB_CORE_SOURCE_FILES_NON_PCH} ${VTLIB_CORE_HEADER_FILES}
			${VTLIB_OSG_SOURCE_FILES} ${VTLIB_OSG_HEADER_FILES} ../pre.cpp)
	set_source_files_properties(${VTLIB_CORE_SOURCE_FILES} ${VTLIB_OSG_SOURCE_FILES} PROPERTIES COMPILE_FLAGS /Yuvtlib/vtlib.h)
	set_source_files_properties(../pre.cpp PROPERTIES COMPILE_FLAGS /Ycvtlib/vtlib.h)
else(MSVC)
	add_library(vtlib ${VTLIB_CORE_SOURCE_FILES} ${VTLIB_CORE_SOURCE_FILES_NON_PCH} ${VTLIB_CORE_HEADER_FILES}
	${VTLIB_OSG_SOURCE_FILES} ${VTLIB_OSG_HEADER_FILES})
endif(MSVC)


if(GDAL_FOUND)
	include_directories(${GDAL_INCLUDE_DIR})
endif (GDAL_FOUND)

if(QUIKGRID_FOUND)
	include_directories(${QUIKGRID_INCLUDE_DIR})
endif(QUIKGRID_FOUND)

if(CURL_FOUND)
	include_directories(${CURL_INCLUDE_DIR})
endif (CURL_FOUND)

if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIR})
endif(ZLIB_FOUND)

if(OSG_FOUND)
	include_directories(${OSG_INCLUDE_DIR})
	include_directories(${OSG_INSTALL_DIR}/include)
endif(OSG_FOUND)

if(OSGEARTH_FOUND)
	include_directories(${OSGEARTH_INCLUDE_DIR})
endif(OSGEARTH_FOUND)

if(MINI_FOUND)
	include_directories(${MINI_INCLUDE_DIR})
endif(MINI_FOUND)

# Find the directory containing GL/glext.h
find_path(GLEXT_INCLUDE_DIR GL/glext.h DOC "Directory containing GL/glext.h")
if (GLEXT_INCLUDE_DIR)
	# Add the glext include directory to the compilers search path
	include_directories(${GLEXT_INCLUDE_DIR})
endif (GLEXT_INCLUDE_DIR)

# Specify normal preprocessor definition for this target
if(OSGEARTH_FOUND)
	set_property(TARGET vtlib APPEND PROPERTY COMPILE_DEFINITIONS USE_OSGEARTH=1)
endif(OSGEARTH_FOUND)
