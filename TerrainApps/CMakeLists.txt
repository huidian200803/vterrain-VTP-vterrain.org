include_directories(${TERRAIN_SDK_ROOT})
# include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(VTBuilder)

include_directories(${TERRAIN_SDK_ROOT}/vtlib)

add_subdirectory(Enviro)
add_subdirectory(CManager)
if(WITH_GLUT)
  add_subdirectory(glutSimple)
endif()
add_subdirectory(VTConvert)
add_subdirectory(wxSimple)
add_subdirectory(Simple)
add_subdirectory(vtTest)
