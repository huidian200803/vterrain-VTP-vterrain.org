#!/bin/sh

TERRAINSDK=/cygdrive/c/VTP/TerrainSDK

echo "Making vtdata docs with doxygen.."
cd $TERRAINSDK/vtdata
doxygen

echo "Making vtlib docs with doxygen.."
cd $TERRAINSDK/vtlib
doxygen

echo "Making vtui docs with doxygen.."
cd $TERRAINSDK/vtui
doxygen

