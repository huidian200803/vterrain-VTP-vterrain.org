
Readme for "Shapelib"
---------------------

The VTP libraries and applications depend on the "Shapelib" library for
shapefile support.

We could require that Shapelib is installed and used, but there is a simpler
way.  We already have GDAL as a dependency, and GDAL includes the entire
functionality of Shapelib.  All that is necessary to use the Shapelib
inside GDAL is the header file, "shapefil.h", which is not included in the
GDAL distribution.

Therefore, the file "shapefil.h" is included here.

