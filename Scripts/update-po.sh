#!/bin/sh

if [ $# -lt 2 ] ; then
  echo "Usage: update-po AppName Language"
  echo "Example: update-po VTBuilder it"
  exit
fi

APPNAME=$1
LANGUAGE=$2

echo "Merging the .pot into the existing .po file."

msgmerge i18n/${LANGUAGE}/${APPNAME}.po i18n/${APPNAME}.pot --output-file=i18n/${LANGUAGE}/${APPNAME}.po
