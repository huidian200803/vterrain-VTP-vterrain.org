# Add a library target called vtdata
add_library(vtdata
		Building.cpp ByteOrder.cpp ChunkLOD.cpp ChunkUtil.cpp ColorMap.cpp Content.cpp
		CubicSpline.cpp DataPath.cpp DLG.cpp
		DxfParser.cpp ElevationGrid.cpp ElevationGridBT.cpp ElevationGridDEM.cpp ElevationGridIO.cpp FeatureGeom.cpp
		Features.cpp Fence.cpp FilePath.cpp GDALWrapper.cpp Geodesic.cpp GEOnet.cpp HeightField.cpp Icosa.cpp LevellerTag.cpp
		LocalCS.cpp LULC.cpp MaterialDescriptor.cpp MathTypes.cpp Matrix.cpp Plants.cpp
		PolyChecker.cpp vtCRS.cpp QuikGrid.cpp RoadMap.cpp SPA.cpp StructArray.cpp
		StructImport.cpp Structure.cpp TagArray.cpp Triangulate.cpp TripDub.cpp Unarchive.cpp UtilityMap.cpp
		Vocab.cpp vtDIB.cpp vtLog.cpp vtString.cpp vtTime.cpp vtTin.cpp vtUnzip.cpp WFSClient.cpp

		Array.h Building.h ByteOrder.h ChunkLOD.h ChunkUtil.h ColorMap.h
		config_vtdata.h Content.h CubicSpline.h DataPath.h DLG.h DxfParser.h ElevationGrid.h ElevError.h
		Features.h Fence.h FileFilters.h FilePath.h GDALWrapper.h GEOnet.h HeightField.h Icosa.h LayerBase.h
		LevellerTag.h LocalCS.h LULC.h Mainpage.h MaterialDescriptor.h MathTypes.h
		Plants.h PolyChecker.h vtCRS.h QuikGrid.h RoadMap.h Selectable.h SPA.h StatePlane.h
		StructArray.h Structure.h TagArray.h Triangulate.h TripDub.h Unarchive.h UtilityMap.h Version.h
		Vocab.h vtDIB.h vtLog.h vtString.h vtTime.h vtTin.h vtUnzip.h WFSClient.h

		triangle/triangle.c triangle/triangle.h)

if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIR})
	message(ZLIB_INCLUDE_DIR)
	message(${ZLIB_INCLUDE_DIR})
endif(ZLIB_FOUND)

if(JPEG_FOUND)
	include_directories(${JPEG_INCLUDE_DIR})
	message(JPEG_INCLUDE_DIR)
	message(${JPEG_INCLUDE_DIR})
endif(JPEG_FOUND)

if(PNG_FOUND)
	include_directories(${PNG_INCLUDE_DIR})
	message(PNG_INCLUDE_DIR)
	message(${PNG_INCLUDE_DIR})
endif(PNG_FOUND)

if(GDAL_FOUND)
	include_directories(${GDAL_INCLUDE_DIR})
	message(GDAL_INCLUDE_DIR)
	message(${GDAL_INCLUDE_DIR})
endif (GDAL_FOUND)

if(BZIP2_FOUND)
	include_directories(${BZIP2_INCLUDE_DIR})
	message(BZIP2_INCLUDE_DIR)
	message(${BZIP2_INCLUDE_DIR})
endif(BZIP2_FOUND)

if(CURL_FOUND)
	include_directories(${CURL_INCLUDE_DIR})
	message(CURL_INCLUDE_DIR)
	message(${CURL_INCLUDE_DIR})
endif (CURL_FOUND)

if(QUIKGRID_FOUND)
	include_directories(${QUIKGRID_INCLUDE_DIR})
	message(QUIKGRID_INCLUDE_DIR)
	message(${QUIKGRID_INCLUDE_DIR})
endif(QUIKGRID_FOUND)

