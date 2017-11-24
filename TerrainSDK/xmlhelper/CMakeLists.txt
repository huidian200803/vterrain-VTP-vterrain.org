# Add a library target called xmlhelper
add_library(xmlhelper easyxml.cpp exception.cpp hashtable.c widestring.cpp xmlparse.c xmlrole.c xmltok.c easyxml.hpp exception.hpp widestring.h)

# Make the debug configuration have a postfix on the output target?
# This seems to cause trouble with the .pdb not getting the postfix
# set_target_properties(xmlhelper PROPERTIES DEBUG_POSTFIX "d")

if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIR})
endif(ZLIB_FOUND)
