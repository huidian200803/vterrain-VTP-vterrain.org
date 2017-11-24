Instructions for CMake
======================

N.B. These instructions are for the windows GUI. On Linux you should follow a
similar process using ccmake or one of the gui versions.

1. You need at least CMake vesion 3.0.

2. Launch the cmake-gui program and set the "Where to build the binaries" 
directory to the root of your VTP source tree. If the "Where is the source code"
directory is not automatically set to the same directory then set it manually.
Once you have run cmake at least once you should be able to drag the 
CMakeCache.txt file from the root of your VTP source tree and drop it on the 
cmake program and it will start with the correct directories. Drag and drop of 
the top level CMakeLists.txt DOES NOT WORK! Also drag and drop is broken in a 
number of versions before 2.8.4. You can fix this on Linux desktops by editing 
the CMake.desktop file and changing the %f in the Exec key to %d, but beware the 
%d key is deprecated and will not work forever.

3. The CMake process is interative. You need to keep pressing configure until 
all the displayed cache values go grey. Each time you press configure you may be 
presented with dialog boxes asking for some input, when you have finished giving 
input the list of cache values will be updated. Some of these values will be in 
red. You need to check you are happy with these values or manually enter the 
value you want. In general the ones you need to manually enter will be either 
the full path to a library file or the full path to a directory containing 
include files, and will be displayed as XXXXXXXXXXXXXXXX-NOTFOUND. When you have 
finished entering values press configure again. This may result in all the 
values going grey or some new red values being displayed. Keep on fixing the red 
values and pressing configure until all grey values are displayed. The red 
values will always be displayed at the top of the list.

4. These are the values you may need to specify as of the time of writing of 
this note.
BZIP2_INCLUDE_DIR
BZIP2_LIBRARY
BZIP2_LIBRARY_DEBUG
CURL_INCLUDE_DIR
CURL_LIBRARY
GDAL_INCLUDE_DIR
GDAL_LIBRARY
GDAL_LIBRARY_DEBUG
GLEXT_INCLUDE_DIR
GLUT_INCLUDE_DIR
JPEG_INCLUDE_DIR
JPEG_LIBRARY
MINI_INCLUDE_DIR
MINI_LIBRARY
MINI_LIBRARY_DEBUG
OSG_INSTALL_DIR - If you set this correctly this all the osg stuff will be found automatically
PNG_INCLUDE_DIR
PNG_LIBRARY
QUIKGRID_INCLUDE_DIR
QUIKGRID_LIBRARY
QUIKGRID_LIBRARY_DEBUG
SQUISH_INCLUDE_DIR
SQUISH_LIBRARY
ZLIB_INCLUDE_DIR
ZLIB_LIBRARY
ZLIB_LIBRARY_DEBUG
wxWidgets_ROOT_DIR - If you set this all the wxWidgets stuff will be found automatically.
                     It should point at the root of your wxWidgets install tree.
wxWidgets_LIB_DIR - This should point at the directory containing the wxWidgets DLLs you want to use. Normally xxxxxx/vc_dll

CMAKE_INSTALL_PREFIX, wxWidgets_wxrc_EXECUTABLE - You can safely ignore these.

5. Once all the values are grey. Press OK. If all is well CMake should quietly write the Visual studio (or other build system) solution and
project files and exit.

6. For Visual Studio you should find a sln file called VTP.sln in the root of your VTP source tree.

