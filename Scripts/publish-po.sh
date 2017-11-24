#!/bin/sh
# This script should be run from the VTP root directory (e.g. C:/VTP on Windows)

if [ $# -lt 2 ] ; then
  echo "Usage: publish-po AppName Language"
  echo "Example: publish-po VTBuilder it"
  exit
fi

APPNAME=$1
LANGUAGE=$2

echo "Converting the .po to .mo"

msgfmt i18n/${LANGUAGE}/${APPNAME}.po --output-file=i18n/${LANGUAGE}/${APPNAME}.mo
