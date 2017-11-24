#!/bin/sh
# This script should be run from the VTP root directory (e.g. C:/VTP on Windows)
# It should work with either Cygwin on Windows, or the regular shell on Linux.

echo "Making a fresh i18n/VTBuilder.pot file."

xgettext --output=i18n/VTBuilder.pot --c++ --keyword=_ \
	TerrainApps/VTBuilder/*.cpp \
	TerrainApps/VTBuilder/*.h \
	TerrainSDK/vtui/*.cpp \
	TerrainSDK/vtui/*.h
