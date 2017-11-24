#!/bin/sh

if [ $# -lt 1 ] ; then
  echo "Usage: mk_vtp_bin_zip.sh date"
  echo
  echo "Example: mk_vtp_bin_zip.sh 151029"
  exit
fi

SOURCEDIR=/cygdrive/c/vtp
BUILDDIR=/cygdrive/c/vtp/vc14
TARGETDIR=/cygdrive/c/Distrib
DATE=$1
DIST_FILE3=${TARGETDIR}/vtp-apps-bin-${DATE}.zip

cd $SOURCEDIR

# Create the archive containing the App Binaries
rm -f $DIST_FILE3

#cp ${BUILDDIR}/TerrainApps/BExtractor/Release-vc9/BExtractor.exe TerrainApps/BExtractor
#zip $DIST_FILE3 ${BUILDDIR}/TerrainApps/BExtractor/BE.ini
#zip $DIST_FILE3 ${BUILDDIR}/TerrainApps/BExtractor/BExtractor.exe
#zip $DIST_FILE3 ${BUILDDIR}/TerrainApps/BExtractor/license.txt
#zip $DIST_FILE3 ${BUILDDIR}/TerrainApps/BExtractor/Docs/*

cp ${BUILDDIR}/TerrainApps/CManager/Release/CManager.exe TerrainApps/CManager
zip $DIST_FILE3 TerrainApps/CManager/CManager.exe
zip $DIST_FILE3 TerrainApps/CManager/itemtypes.txt
zip $DIST_FILE3 TerrainApps/CManager/tags.txt
zip $DIST_FILE3 TerrainApps/CManager/Docs/*

cp ${BUILDDIR}/TerrainApps/Enviro/Release/Enviro.exe TerrainApps/Enviro
zip $DIST_FILE3 TerrainApps/Enviro/Enviro.exe
zip -r $DIST_FILE3 TerrainApps/Enviro/Docs -x *.svn

# deal specially with Enviro.xml
# mv TerrainApps/Enviro/Enviro.xml TerrainApps/Enviro/Enviro_dev.xml
# cp TerrainApps/Enviro/Ship/Enviro.xml TerrainApps/Enviro
# zip $DIST_FILE3 TerrainApps/Enviro/Enviro.xml
# rm TerrainApps/Enviro/Enviro.xml
# mv TerrainApps/Enviro/Enviro_dev.xml TerrainApps/Enviro/Enviro.xml

# Enviro locale files
zip $DIST_FILE3 TerrainApps/Enviro/af/Enviro.mo
zip $DIST_FILE3 TerrainApps/Enviro/ar/Enviro.mo
zip $DIST_FILE3 TerrainApps/Enviro/de/Enviro.mo
zip $DIST_FILE3 TerrainApps/Enviro/en/Enviro.mo
zip $DIST_FILE3 TerrainApps/Enviro/en_GB/Enviro.mo
zip $DIST_FILE3 TerrainApps/Enviro/es/Enviro.mo
zip $DIST_FILE3 TerrainApps/Enviro/fr/Enviro.mo
zip $DIST_FILE3 TerrainApps/Enviro/hu/Enviro.mo
zip $DIST_FILE3 TerrainApps/Enviro/it/Enviro.mo
zip $DIST_FILE3 TerrainApps/Enviro/pt/Enviro.mo
zip $DIST_FILE3 TerrainApps/Enviro/ro/Enviro.mo
zip $DIST_FILE3 TerrainApps/Enviro/sv/Enviro.mo
zip $DIST_FILE3 TerrainApps/Enviro/tr/Enviro.mo
zip $DIST_FILE3 TerrainApps/Enviro/zh/Enviro.mo

cp ${BUILDDIR}/TerrainApps/VTBuilder/Release/VTBuilder.exe TerrainApps/VTBuilder
zip $DIST_FILE3 TerrainApps/VTBuilder/VTBuilder.exe
zip $DIST_FILE3 TerrainApps/VTBuilder/VTBuilder.xml
zip -r $DIST_FILE3 TerrainApps/VTBuilder/Docs -x *.svn

# VTBuilder locale files
zip $DIST_FILE3 TerrainApps/VTBuilder/ar/VTBuilder.mo
zip $DIST_FILE3 TerrainApps/VTBuilder/en/VTBuilder.mo
zip $DIST_FILE3 TerrainApps/VTBuilder/fr/VTBuilder.mo
zip $DIST_FILE3 TerrainApps/VTBuilder/ro/VTBuilder.mo
zip $DIST_FILE3 TerrainApps/VTBuilder/zh/VTBuilder.mo

# VTConvert
cp ${BUILDDIR}/TerrainApps/VTConvert/Release/VTConvert.exe TerrainApps/VTConvert
zip $DIST_FILE3 TerrainApps/VTConvert/VTConvert.exe

#deal specially with docs
mv Docs/*.html TerrainApps
zip $DIST_FILE3 TerrainApps/*.html
mv TerrainApps/*.html Docs

echo $DIST_FILE3 ready.
