# The Virtual Terrain Project Software #

The [VTP](http://vterrain.org/) has a suite of cross-platform applications and libraries for geospatial processing and 3D visualization of terrain. The main applications are:

* VTBuilder: Loads all your raw data from GIS formats, allows you to process them as needed to prepare them for visualization: resample, reproject, add higher-level descriptions, set attributes, then export to efficient formats.

* Enviro: 3D runtime environment, which allows you to not only visualize but also dynamically interact in many ways (move buildings, plant trees, change time of day, and much more.)

If you only want to run the software, there are [installers for Windows](http://vterrain.org/Notify/)

### How do I get set up? ###

* For build instructions, see [Directions for Windows](http://vterrain.org/Distrib/win.html) or [Directions for Unix](http://vterrain.org/Distrib/unix.html)

### Dependencies ###

* Required: GDAL, libMini, wxWidgets, OpenSceneGraph, libjpeg, libpng.

* Optional: libbzip2, libcurl, QuikGrid, and a few more.

### New Dependencies ###

* Required: GDAL 2, OpenSceneGraph (>= 3.2), libMini (>= 11.5.2), wxWidgets (>= 2.8), libjpeg, libpng.

### Current Quirks on Ubuntu 15.04 ###

* remove OsgEarth_include dir in ccmake
* GDAL2 has no package yet, I suggest building it with ./configure --prefix=/usr/local/gdal2 so it doesnt interfere with your system gdal, then just set GDAL_INCLUDE and GDAL_LIBRARY in cmake

### Contribution guidelines ###

* Please send feedback to the [mailing list](https://groups.yahoo.com/neo/groups/vtp/info)
