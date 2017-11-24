#!/bin/sh

if [ $# -lt 1 ] ; then
  echo "Usage: mk_plant_zip.sh date"
  echo
  echo "Example: mk_plant_zip.sh 041029"
  exit
fi

TARGETDIR=/cygdrive/c/Distrib
DATE=$1
DIST_FILE1=${TARGETDIR}/vtp-plantlib-${DATE}.zip

# Create the archive containing the Plant Library
rm -f $DIST_FILE1

cd /cygdrive/g/Data-Distro
zip $DIST_FILE1 PlantData/species.xml

cd /cygdrive/g/Data-Common
zip $DIST_FILE1 PlantModels/*

cd /cygdrive/g/Data-ECL
zip $DIST_FILE1 PlantModels/*

echo $DIST_FILE1 ready.
