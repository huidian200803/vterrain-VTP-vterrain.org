#!/bin/sh

if [ $# -lt 1 ] ; then
  echo "Usage: mk_lib_chart.sh date"
  exit
fi

DATE=$1

echo "Making library chart libchart_${DATE}.png"

dot -Tpng -olibchart_${DATE}.png vtp_libs.dot