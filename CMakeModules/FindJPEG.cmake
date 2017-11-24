# - Find JPEG
# Find the native JPEG includes and library
# This module defines
#  JPEG_INCLUDE_DIR, where to find jpeglib.h, etc.
#  JPEG_LIBRARY, the libraries needed to use JPEG.
#  JPEG_FOUND, If false, do not try to use JPEG.

FIND_PATH(JPEG_INCLUDE_DIR jpeglib.h)

FIND_LIBRARY(JPEG_LIBRARY NAMES jpeg )

# handle the QUIETLY and REQUIRED arguments and set JPEG_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(JPEG DEFAULT_MSG JPEG_LIBRARY JPEG_INCLUDE_DIR)
