# Locate OSG
# This module defines
# xxx_LIBRARY xxx_LIBRARY_DEBUG xxx_LIBRARIES for each of the OpenSceneGraph libraries in OSG_LIBRARY_NAMES below
# xxx_LIBRARIES contains both debug and release libraries with relevant target_link_libraries flags
# OSG_ALL_LIBRARIES contains all the xxx_LIBRARIES
# OSG_FOUND, if false, do not try to link to osg 
# OSG_INCLUDE_DIR, where to find the headers
#
# OSG_INSTALL_DIR where the OpenSceneGraph files are installed (used internally)
#
#
# Created by Robert Osfield. Modified by Roger James 05/03/09

IF (WIN32)
    SET(OSG_INSTALL_DIR "c:/Program Files/OpenSceneGraph" CACHE PATH "OpenSceneGraph WIN32 install directory")
ELSE (WIN32)
    SET(OSG_INSTALL_DIR "/usr/local/openscenegraph" CACHE PATH "OpenSceneGraph UNIX install directory")
ENDIF (WIN32)

SET(OSG_LIBRARY_NAMES
    osg
    osgGA
    osgUtil
    osgDB
    osgText
    osgWidget
    osgTerrain
    osgSim
    osgFX
    osgViewer
    osgVolume
    osgShadow
    osgParticle
    OpenThreads)

FIND_PATH(OSG_INCLUDE_DIR osg/Node
    ${OSG_INSTALL_DIR}/include
    $ENV{OSG_DIR}/include
    $ENV{OSG_DIR}
    $ENV{OSGDIR}/include
    $ENV{OSGDIR}
    $ENV{OSG_ROOT}/include
    NO_DEFAULT_PATH
)

FIND_PATH(OSG_INCLUDE_DIR osg/Node)

MACRO(FIND_OSG_LIBRARY MYLIBRARYNAME)

    FIND_LIBRARY(${MYLIBRARYNAME}_LIBRARY_DEBUG
        NAMES ${MYLIBRARYNAME}d
        PATHS
        ${OSG_INSTALL_DIR}/lib/Debug
        ${OSG_INSTALL_DIR}/lib
        $ENV{OSG_DIR}/lib/debug
        $ENV{OSG_DIR}/lib
        $ENV{OSG_DIR}
        $ENV{OSGDIR}/lib
        $ENV{OSGDIR}
        $ENV{OSG_ROOT}/lib
        NO_DEFAULT_PATH
    )

    FIND_LIBRARY(${MYLIBRARYNAME}_LIBRARY_DEBUG
        NAMES ${MYLIBRARYNAME}d
        PATHS
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/local/lib64
        /usr/lib
        /usr/lib64
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
        /usr/freeware/lib64
    )
    
    FIND_LIBRARY(${MYLIBRARYNAME}_LIBRARY
        NAMES ${MYLIBRARYNAME}
        PATHS
        ${OSG_INSTALL_DIR}/lib/Release
        ${OSG_INSTALL_DIR}/lib
        $ENV{OSG_DIR}/lib/Release
        $ENV{OSG_DIR}/lib
        $ENV{OSG_DIR}
        $ENV{OSGDIR}/lib
        $ENV{OSGDIR}
        $ENV{OSG_ROOT}/lib
        NO_DEFAULT_PATH
    )

    FIND_LIBRARY(${MYLIBRARYNAME}_LIBRARY
        NAMES ${MYLIBRARYNAME}
        PATHS
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/local/lib64
        /usr/lib
        /usr/lib64
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
        /usr/freeware/lib64
    )
    
    IF( NOT ${MYLIBRARYNAME}_LIBRARY_DEBUG)
        IF(${MYLIBRARYNAME}_LIBRARY)
            SET(${MYLIBRARYNAME}_LIBRARY_DEBUG ${${MYLIBRARYNAME}_LIBRARY})
         ENDIF(${MYLIBRARYNAME}_LIBRARY)
    ENDIF( NOT ${MYLIBRARYNAME}_LIBRARY_DEBUG)

    IF(${MYLIBRARYNAME}_LIBRARY)
        SET(${MYLIBRARYNAME}_LIBRARIES debug ${${MYLIBRARYNAME}_LIBRARY_DEBUG} optimized ${${MYLIBRARYNAME}_LIBRARY})
    ENDIF(${MYLIBRARYNAME}_LIBRARY)

           
ENDMACRO(FIND_OSG_LIBRARY MYLIBRARYNAME)

FOREACH(LIB_NAME ${OSG_LIBRARY_NAMES})
    FIND_OSG_LIBRARY(${LIB_NAME})
    IF (${LIB_NAME}_LIBRARIES)
        LIST(APPEND OSG_ALL_LIBRARIES ${${LIB_NAME}_LIBRARIES})
    ENDIF (${LIB_NAME}_LIBRARIES)
ENDFOREACH(LIB_NAME)

SET(OSG_FOUND "NO")
IF(osg_LIBRARY AND OSG_INCLUDE_DIR)
    SET(OSG_FOUND "YES")
ENDIF(osg_LIBRARY AND OSG_INCLUDE_DIR)
