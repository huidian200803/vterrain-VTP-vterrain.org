# Add a library target called minidata
add_library(minidata jpegbase.cpp MiniDatabuf.cpp LocalDatabuf.cpp minidata.cpp pngbase.cpp jpegbase.h LocalDatabuf.h MiniDatabuf.h pngbase.h zlibbase.h zlibbase.cpp)

if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIR})
endif (ZLIB_FOUND)

if(MINI_FOUND)
	include_directories(${MINI_INCLUDE_DIR})
endif(MINI_FOUND)

if(JPEG_FOUND)
	include_directories(${JPEG_INCLUDE_DIR})
endif(JPEG_FOUND)

if(PNG_FOUND)
	include_directories(${PNG_INCLUDE_DIR})
endif(PNG_FOUND)

if(GDAL_FOUND)
	include_directories(${GDAL_INCLUDE_DIR})
endif (GDAL_FOUND)

