include_directories(${TERRAIN_SDK_ROOT})

add_subdirectory(xmlhelper)
add_subdirectory(unzip)
add_subdirectory(minidata)
add_subdirectory(vtdata)

include_directories(${TERRAIN_SDK_ROOT}/vtlib)

add_subdirectory(vtlib)
add_subdirectory(vtui)
add_subdirectory(wxosg)
