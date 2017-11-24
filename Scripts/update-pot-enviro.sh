#!/bin/sh
# This script should be run from the VTP root directory (e.g. C:/VTP on Windows)
# It should work with either Cygwin on Windows, or the regular shell on Linux.

echo "Making a fresh i18n/Enviro.pot file."

xgettext --output=i18n/Enviro.pot --c++ --keyword=_ \
	TerrainApps/Enviro/*.cpp \
	TerrainApps/Enviro/*.h \
	TerrainApps/Enviro/wx/*.cpp \
	TerrainApps/Enviro/wx/*.h \
	TerrainSDK/wxosg/*.cpp \
	TerrainSDK/wxosg/*.h \
	TerrainSDK/vtui/*.cpp \
	TerrainSDK/vtui/*.h
