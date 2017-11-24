//
// vtElevationGridIO.cpp
//
// This modules contains the implementations of the file I/O methods of
// the class vtElevationGrid.
//
// Copyright (c) 2001-2009 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <fstream>
#include <memory>	// for auto_ptr
using namespace std;

#include "config_vtdata.h"
#include "ByteOrder.h"
#include "ElevationGrid.h"
#include "FilePath.h"
#include "GDALWrapper.h"
#include "vtDIB.h"
#include "vtLog.h"
#include "vtString.h"

// Headers for PNG support, which uses the library "libpng"
#include "png.h"

// compatibility with libpng 1.4
#if ((PNG_LIBPNG_VER_MAJOR==1 && PNG_LIBPNG_VER_MINOR>=4) || PNG_LIBPNG_VER_MAJOR>1)
#define png_infopp_NULL NULL
#endif

// GDAL
#include "gdal_priv.h"

// OGR
#include <ogrsf_frmts.h>

//	Header structure for a GTOPO30 DEM header file
typedef struct
{
	char			ByteOrder[30];		//	Byte order in which image pixel values are stored
										//		M = Motorola byte order (MSB first)
										//		I = Intel byte order (LSB first)
	char			Layout[30];			//	Organization of the bands in the file
										//		BIL = Band interleaved by line (DEM = single band image)
	unsigned long	NumRows;			//	Number of rows in the image
	unsigned long	NumCols;			//	Number of columns in the image
	char			Bands[30];			//	Number of spectral bands in the image (1 for DEM)
	char			Bits[30];			//	Number of bits per pixel (16 for DEM)
	char			BandRowBytes[30];	//	Number of bytes per band per row
										//		(twice the number of columns for 16 bit DEM)
	char			TotalRowBytes[30];	//	Total number of bytes of data per row
										//		(twice the number of columns for 16 bit DEM)
	char			BandGapBytes[30];	//	Number of bytes between bands in a BSQ format
										//		(0 for DEM)
	short			NoData;				//	Value used for no data or masking purposes
	double			ULXMap;				//	Longitude of the center of the upper left pixel in decimal degrees.
	double			ULYMap;				//	Latitude of the center of the upper left pixel in decimal degrees.
	double			XDim;				//	x dimension of a pixel in geographic units (decimal degrees).
	double			YDim;				//	y dimension of a pixel in geographic units (decimal degrees).
} GTOPOHeader;


/**
 * Load from a file whose type is not known a priori.  This will end up
 * calling one of the Load* member functions.
 *
 * You should call SetupLocalCS() after loading if you will be doing
 * heightfield operations on this grid.
 *
 * \returns \c true if successful.
 */
bool vtElevationGrid::LoadFromFile(const char *szFileName,
								   bool progress_callback(int), vtElevError *err)
{
	vtString FileExt = GetExtension(szFileName);

	if (FileExt == "")
	{
		SetError(err, vtElevError::UNKNOWN_FORMAT, "Couldn't determine the file format of '%s'",
			szFileName);
		return false;
	}

	// The first character in the file is useful for telling which format
	// the file really is.
	FILE *fp = vtFileOpen(szFileName, "rb");
	if (!fp)
	{
		SetError(err, vtElevError::FILE_OPEN, "Couldn't open file '%s'", szFileName);
		return false;
	}
	int FirstChar = fgetc(fp);
	fclose(fp);

	bool Success = false;

	if (!FileExt.CompareNoCase(".asc"))
	{
		// GDAL's ASC reader has trouble on some machines
//		Success = LoadWithGDAL(szFileName, progress_callback, err);

		Success = LoadFromASC(szFileName, progress_callback);
	}
	else if (!FileExt.CompareNoCase(".bil"))
	{
		Success = LoadWithGDAL(szFileName, progress_callback, err);
	}
	else if ((!FileExt.CompareNoCase(".bt")) || (!FileExt.CompareNoCase(".bt.gz")))
	{
		Success = LoadFromBT(szFileName, progress_callback, err);
	}
	else if (!FileExt.CompareNoCase(".cdf"))
	{
		Success = LoadWithGDAL(szFileName, progress_callback);
	}
	else if (!FileExt.CompareNoCase(".dem"))
	{
		// If there is a .hdr file in the same place, it is most likely
		//  a GTOPO30/SRTM30 file
		vtString hdr_fname = ChangeFileExtension(szFileName, ".hdr");
		if (vtFileExists(hdr_fname))
			Success = LoadFromGTOPO30(hdr_fname, progress_callback);
		else
		{
			if (FirstChar == '*')
				Success = LoadFromMicroDEM(szFileName, progress_callback);
			else
				Success = LoadFromDEM(szFileName, progress_callback, err);
		}
	}
	else if (!FileExt.CompareNoCase(".dte") ||
			 !FileExt.CompareNoCase(".dt0") ||
			 !FileExt.CompareNoCase(".dt1") ||
			 !FileExt.CompareNoCase(".dt2"))
	{
		Success = LoadFromDTED(szFileName, progress_callback);
	}
	else if (!FileExt.CompareNoCase(".grd"))
	{
		// might be CDF, might be GRD
		if (FirstChar == 'D')
			Success = LoadFromGRD(szFileName, progress_callback);
		else
			Success = LoadWithGDAL(szFileName, progress_callback);

		if (!Success)
		{
			// Might be 'Arc Binary Grid', try GDAL
			Success = LoadWithGDAL(szFileName, progress_callback, err);
		}
	}
	else if (!FileExt.CompareNoCase(".hdr"))
	{
		Success = LoadFromGTOPO30(szFileName, progress_callback);
		if (!Success)
		{
			// might be NOAA GLOBE header
			Success = LoadFromGLOBE(szFileName, progress_callback);
		}
	}
	else if (!FileExt.CompareNoCase(".hgt"))
	{
		Success = LoadFromHGT(szFileName, progress_callback);
	}
	else if (!FileExt.CompareNoCase(".ddf") ||
			 !FileExt.CompareNoCase(".ecw") ||
			 !FileExt.CompareNoCase(".tif") ||
			 !FileExt.CompareNoCase(".tiff") ||
			 !FileExt.CompareNoCase(".png") ||
			 !FileExt.CompareNoCase(".adf"))
	{
		Success = LoadWithGDAL(szFileName, progress_callback, err);
	}
	else if (!FileExt.CompareNoCase(".pgm"))
	{
		Success = LoadFromPGM(szFileName, progress_callback);
	}
	else if (!FileExt.CompareNoCase(".ter"))
	{
		Success = LoadFromTerragen(szFileName, progress_callback);
	}
	return Success;
}


/**
 * Loads from a 3TX ascii grid file.
 * Projection is Geo WGS84, extents are always 1 degree in size.
 *
 * You should call SetupLocalCS() after loading if you will be doing
 * heightfield operations on this grid.
 *
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFrom3TX(const char *szFileName,
								  bool progress_callback(int))
{
	// Free buffers to prepare to receive new data
	FreeData();

	FILE *fp = vtFileOpen(szFileName, "rb");
	if (!fp)
		return false;

	// SW origin in integer degrees
	int lon, lat;
	if (fscanf(fp, "%d %d\n", &lat, &lon) != 2)
		return false;
	DRECT ext(lon, lat+1, lon+1, lat);
	SetEarthExtents(ext);

	m_crs.SetSimple(false, 0, EPSG_DATUM_WGS84);

	m_iSize.Set(1201, 1201);
	m_bFloatMode = false;
	if (!AllocateGrid())
		return false;

	// elevationdata one per line in column-first order from the SW
	short val;
	for (int i = 0; i < 1201; i++)
	{
		if (progress_callback != NULL)
			progress_callback(i * 100 / 1201);
		for (int j = 0; j < 1201; j++)
		{
			if (fscanf(fp, "%hd\n", &val) != 1)
				return false;
			SetValue(i, j, val);
		}
	}
	return true;
}


/** Loads from a Arc/Info compatible ASCII grid file.
 * Projection is read from a corresponding .prj file.
 *
 * You should call SetupLocalCS() after loading if you will be doing
 * heightfield operations on this grid.
 *
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromASC(const char *szFileName,
								  bool progress_callback(int))
{
	// Free buffers to prepare to receive new data
	FreeData();

	FILE *fp = vtFileOpen(szFileName, "rb");
	if (!fp)
		return false;

	if (progress_callback != NULL) progress_callback(0);
	char buf[4000];
	char token[40];
	int ncols, nrows;
	double xllcorner, yllcorner, dCellsize, nodata;
	DPoint2 cellsize;

	// get dimension IDs
	bool bGo = true;
	while (bGo)
	{
		if (fgets(buf, 4000, fp) == NULL)
			return false;
		vtString four(buf, 4);

		if (four.CompareNoCase("ncol") == 0)
		{
			if (sscanf(buf, "%s %d", token, &ncols) != 2)
				return false;
		}
		else if (four.CompareNoCase("nrow") == 0)
		{
			if (sscanf(buf, "%s %d", token, &nrows) != 2)
				return false;
		}
		else if (four.CompareNoCase("xllc") == 0)
		{
			if (sscanf(buf, "%s %lf", token, &xllcorner) != 2)
				return false;
		}
		else if (four.CompareNoCase("yllc") == 0)
		{
			if (sscanf(buf, "%s %lf", token, &yllcorner) != 2)
				return false;
		}
		else if (four.CompareNoCase("cell") == 0)
		{
			if (sscanf(buf, "%s %lf", token, &dCellsize) != 2)
				return false;
			cellsize.x = dCellsize;
			cellsize.y = dCellsize;
		}
		else if (four.CompareNoCase("xdim") == 0)
		{
			if (sscanf(buf, "%s %lf", token, &cellsize.x) != 2)
				return false;
		}
		else if (four.CompareNoCase("ydim") == 0)
		{
			if (sscanf(buf, "%s %lf", token, &cellsize.y) != 2)
				return false;
		}
		else if (four.CompareNoCase("noda") == 0)
		{
			if (sscanf(buf, "%s %lf", token, &nodata) != 2) // nodata_value
				return false;
			bGo = false;
		}
		else
		{
			VTLOG1("Couldn't parse line in ASC file: ");
			VTLOG1(buf);
			VTLOG1("\n");
			return false;
		}
	}

	m_iSize.Set(ncols, nrows);

	// There is no projection info in a ASC file, but there might be
	//  an accompanying .prj file, usually in the old ESRI .prj format.
	if (!m_crs.ReadProjFile(szFileName))
		m_crs.Clear();

	// Some ASC files contain integer data, some contain floating point.
	// There's no way for us to know in advance, so just assume float.
	m_bFloatMode = true;

	m_EarthExtents.left = xllcorner;
	m_EarthExtents.right = xllcorner + (ncols - 1) * cellsize.x;
	m_EarthExtents.top = yllcorner + (nrows - 1) * cellsize.y;
	m_EarthExtents.bottom = yllcorner;

	ComputeCornersFromExtents();
	if (!AllocateGrid())
		return false;

	int i, j;
	float z;
	for (i = 0; i < nrows; i++)
	{
		if (progress_callback != NULL)
		{
			if (progress_callback(i*100/nrows))
			{
				// Cancel
				fclose(fp);
				return false;
			}
		}
		for (j = 0; j < ncols; j++)
		{
			if (fscanf(fp, "%f", &z) != 1)
				return false;
			if (i == 0)
				VTLOG("(%f)", z);
			if (z == nodata || z > 1E6 || z < -1E6)
				SetFValue(j, nrows-1-i, INVALID_ELEVATION);
			else
				SetFValue(j, nrows-1-i, z);
		}
	}
	fclose(fp);
	return true;
}


/** Loads from a Terragen Terrain file.
 *
 * You should call SetupLocalCS() after loading if you will be doing
 * heightfield operations on this grid.
 *
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromTerragen(const char *szFileName,
								bool progress_callback(int))
{
	// Free buffers to prepare to receive new data
	FreeData();

	char buf[8];
	FILE *fp = vtFileOpen(szFileName, "rb");
	if (!fp)
		return false;

	// get file identifier
	if (fread(buf, 8, 1, fp) != 1)
		return false;
	if (strncmp(buf, "TERRAGEN", 8))
	{
		fclose(fp);
		return false;
	}
	if (fread(buf, 8, 1, fp) != 1)
		return false;
	if (strncmp(buf, "TERRAIN ", 8))
	{
		fclose(fp);
		return false;
	}

	if (progress_callback != NULL) progress_callback(0);

	m_crs.SetSimple(true, 1, EPSG_DATUM_WGS84);

	m_bFloatMode = false;

	short svalue, dummy;
	FPoint3 scale(30.0f, 30.0f, 30.0f);
	float fvalue;
	int i, j;

	while (fread(buf, 4, 1, fp) == 1)
	{
		if (!strncmp(buf, "SIZE", 4))
		{
			if (fread(&svalue, 2, 1, fp) != 1) return false;
			if (fread(&dummy, 2, 1, fp) != 1) return false;
			m_iSize.y = m_iSize.x = svalue + 1;
		}
		else if (!strncmp(buf, "XPTS", 4))
		{
			if (fread(&svalue, 2, 1, fp) != 1) return false;
			if (fread(&dummy, 2, 1, fp) != 1) return false;
			m_iSize.x = svalue;
		}
		else if (!strncmp(buf, "YPTS", 4))
		{
			if (fread(&svalue, 2, 1, fp) != 1) return false;
			if (fread(&dummy, 2, 1, fp) != 1) return false;
			m_iSize.y = svalue;
		}
		else if (!strncmp(buf, "SCAL", 4))
		{
			if (fread(&scale.x, 4, 1, fp) != 1) return false;
			if (fread(&scale.y, 4, 1, fp) != 1) return false;
			if (fread(&scale.z, 4, 1, fp) != 1) return false;
		}
		else if (!strncmp(buf, "CRAD", 4))
		{
			// radius of planet
			if (fread(&fvalue, 4, 1, fp) != 1) return false;
		}
		else if (!strncmp(buf, "CRVM", 4))
		{
			// "curve mode"
			if (fread(&svalue, 2, 1, fp) != 1) return false;
			if (fread(&dummy, 2, 1, fp) != 1) return false;
		}
		else if (!strncmp(buf, "ALTW", 4))
		{
			short HeightScale;
			short BaseHeight;
			if (fread(&HeightScale, 2, 1, fp) != 1) return false;
			if (fread(&BaseHeight, 2, 1, fp) != 1) return false;

			if (!AllocateGrid())
				return false;
			for (j = 0; j < m_iSize.y; j++)
			{
				if (progress_callback != NULL) progress_callback(j*100/m_iSize.y);
				for (i = 0; i < m_iSize.x; i++)
				{
					if (fread(&svalue, 2, 1, fp) != 1) return false;
					SetFValue(i, j, scale.z * (BaseHeight + ((float)svalue * HeightScale / 65536.0f)));
				}
			}
		}
		else if (!strncmp(buf, "EOF ", 4))
		{
			// do nothing
		}
	}
	fclose(fp);

	// make up some extents, based on the scaling
	m_EarthExtents.left = 0;
	m_EarthExtents.right = (m_iSize.x - 1) * scale.x;
	m_EarthExtents.top = (m_iSize.y - 1) * scale.y;
	m_EarthExtents.bottom = 0;
	ComputeCornersFromExtents();

	return true;
}


//
// Helper for DTED reader: read an 8-byte number in the
// form DDDMMSSH, degrees-minute-seconds-hemisphere
//
bool get_dms8(FILE *fp, double &value)
{
	char buf[8];
	if (fread(buf, 8, 1, fp) != 1)
		return false;

	char hem = buf[7];
	buf[7] = '\0';
	double seconds = (double)atof(buf + 5);
	buf[5] = '\0';
	double minutes = (double)atof(buf + 3);
	buf[3] = '\0';
	double degrees = (double)atof(buf);

	value = degrees + minutes/60 + seconds/3600;
	if (hem == 'W' || hem == 'S')
		value = -value;

	return true;
}

//
// Helper for DTED reader: read an 4-byte number in the
// return as integer
//
bool get_dddd(FILE *fp, int &value)
{
	char buf[5];
	buf[4] = '\0';
	if (fread(buf, 4, 1, fp) != 1)
		return false;
	value = atoi(buf);
	return true;
}

//
// Helper for DTED reader: read an 4-byte number in the
// form SSSS, decimal seconds
// return as decimal degree
//
bool get_ssss(FILE *fp, double &value)
{
	int i;
	if (!get_dddd(fp, i)) return false;
	value = (double)i / 36000;
	return true;
}


// Macros for DTED reader
#define SIGN_BIT 0x8000		// Position of 15th bit (high bit of 2 byte field)
#define IS_NEGATIVE(x) (x & SIGN_BIT)	// Check if sign bit is turned on
#define UNSET_SIGN_BIT(x) (x &= (~SIGN_BIT))	// Turn off sign bit

/** Loads from a DTED file.
 * \par
 * Should support DTED0, DTED1 and DTED2 files, although it has only been
 * tested on DTED0.  Projection is assumed to be geographic and elevation
 * is integer meters.
 *
 * You should call SetupLocalCS() after loading if you will be doing
 * heightfield operations on this grid.
 *
 * \returns \c true if the file was successfully opened and read.
 */
// DTED chunks/bytes:
//  UHL 80
//  DSI 648
//  ACC 2700
//  Data (variable)
//
bool vtElevationGrid::LoadFromDTED(const char *szFileName,
								 bool progress_callback(int))
{
	// Free buffers to prepare to receive new data
	FreeData();

	FILE *fp = vtFileOpen(szFileName, "rb");
	if (!fp)	// Cannot Open File
		return false;

	// all DTEDs are geographic and in integer meters
	// datum is always WGS84
	m_crs.SetSimple(false, 0, EPSG_DATUM_WGS84);
	m_bFloatMode = false;

	// check for correct format
	char buf[80];
	int header = 0;
	if (fread(buf, 4, 1, fp) != 1)
		return false;
	buf[4] = 0;
	if (!strncmp(buf, "HDR1", 4))
		header = 1;
	if (!strncmp(buf, "UHL1", 4))
		header = 2;

	if (header == 0)
	{
		// Not a DTED file
		fclose(fp);
		return false;
	}

	fseek(fp, 0, SEEK_SET);
	if (header == 1)
	{
		// 4 bytes for 'HDR1'
		if (fread(buf, 4, 1, fp) != 1)
			return false;

		// 17 bytes for 'Filename'
		if (fread(buf, 17, 1, fp) != 1)
			return false;
		buf[17] = 0;

		// 6 bytes always 'UNIVAC'
		// 4 bytes 'Reel Sequence Number'
		// 4 bytes 'File Sequence Number'
		// 6 bytes version numbers
		// 6 bytes Creation Date
		// 6 bytes Expiration Date
		// 1 byte 'Accessibility'
		// 6 bytes 'Block Count'
		// 13 bytes 'Qualifier'
		// 7 bytes blank
		fseek(fp, 80, SEEK_SET);
		header = 2;
	}

	if (header == 2)
	{
		// 4 bytes for 'UHL1'
		if (fread(buf, 4, 1, fp) != 1) return false;

		get_dms8(fp, m_Corners[0].x);
		get_dms8(fp, m_Corners[0].y);

		double xInterval, yInterval;
		get_ssss(fp, xInterval);
		get_ssss(fp, yInterval);

		// Skip over:
		// 4 bytes Absolute Vertical Accuracy
		// 3 bytes Security Code
		// 12 bytes Unique Reference
		fseek(fp, 4+3+12, SEEK_CUR);

		// get dimensions
		get_dddd(fp, m_iSize.x);
		get_dddd(fp, m_iSize.y);

		// imply other corners
		m_Corners[1].x = m_Corners[0].x;
		m_Corners[1].y = m_Corners[0].y + yInterval * (m_iSize.y - 1);
		m_Corners[2].x = m_Corners[0].x + xInterval * (m_iSize.x - 1);
		m_Corners[2].y = m_Corners[0].y + yInterval * (m_iSize.y - 1);
		m_Corners[3].x = m_Corners[0].x + xInterval * (m_iSize.x - 1);
		m_Corners[3].y = m_Corners[0].y;
		ComputeExtentsFromCorners();

		// Skip over:
		// 1 byte Multiple Accuracy
		// 24 bytes Reserved
		fseek(fp, 1+24, SEEK_CUR);
	}

	if (!AllocateGrid())
		return false;

	// REF: record header length + (2 * number of rows) + checksum length
	int line_length = 8 + (2 * m_iSize.y) + 4;
	uchar *linebuf = new uchar[line_length];

	// Skip DSI and ACC headers
	fseek(fp, 648 + 2700, SEEK_CUR);

	// each elevation, z, is stored in 2 bytes
	const int z_len = 2;
	uchar swap[z_len];

	int i, j, offset;
	for (i = 0; i < m_iSize.x; i++)
	{
		if (progress_callback != NULL)
			progress_callback(i * 100 / m_iSize.x);

		if (fread(linebuf, line_length, 1, fp) != 1)
			return false;
		if (*linebuf != 0252)	// sentinel = 252 base 8
			break;

		offset = 8;	// record header length

		for (j = 0; j < m_iSize.y; j++)
		{
			// memcpy(z, linebuf + offset, z_len);
			swap[1] = *(linebuf + offset);
			swap[0] = *(linebuf + offset+1);

			// z = ntohs(z);	// swap bytes
			short z = *((short *)swap);

			// DTED values are signed magnitude, so convert to complement
			if (IS_NEGATIVE(z))
			{
				UNSET_SIGN_BIT(z);
				z *= -1;
			}

			if (-32767 == z)
				SetValue(i, j, INVALID_ELEVATION);
			else
				SetValue(i, j, z);

			offset += z_len;
		}
	}
	delete [] linebuf;
	fclose(fp);
	ComputeHeightExtents();
	return true;
}


/**
 * Loads from a GTOPO30 (or SRTM30) file.
 *
 * GTOPO30 files are actually composed of at least 2 files, a header with a
 * .hdr extension and data with a .dem extension.  Pass the filename of
 * the .hdr file to this function, and it will automatically look for
 * a corresponding .dem file in the same location.
 *
 * Projection is always geographic and elevation is integer meters.
 *
 * You should call SetupLocalCS() after loading if you will be doing
 * heightfield operations on this grid.
 *
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromGTOPO30(const char *szFileName,
									bool progress_callback(int))
{
	// Free buffers to prepare to receive new data
	FreeData();

	if (progress_callback != NULL) progress_callback(1);

	// Open the header file
	// Charset Beware!  Filename may be utf-8, but how can we open an ifstream with utf-8 in
	//  a portable way?
	ifstream hdrFile(szFileName);
	if (!hdrFile.is_open())
	  return false;

	// Parse the file
	char strName[30];
	char strValue[30];

	GTOPOHeader gh;

	// Read the byte order
	hdrFile >> strName >> gh.ByteOrder;

	// Read the layout
	hdrFile >> strName >> gh.Layout;
	// Read the number of rows
	hdrFile >> strName >> strValue;
	gh.NumRows = atol(strValue);

	// Read the number of columns
	hdrFile >> strName >> strValue;
	gh.NumCols = atol(strValue);

	// Read the number of bands
	hdrFile >> strName >> gh.Bands;

	// Read the number of bits per pixel
	hdrFile >> strName >> gh.Bits;

	// Read the band row bytes
	hdrFile >> strName >> gh.BandRowBytes;

	// Read the total row bytes
	hdrFile >> strName >> gh.TotalRowBytes;

	// Read the band gap bytes
	hdrFile >> strName >> gh.BandGapBytes;

	// Read the no data value
	hdrFile >> strName >> strValue;
	gh.NoData = (short) atoi(strValue);

	// Read the upper left x coordinate
	hdrFile >> strName >> strValue;
	gh.ULXMap = atof(strValue);

	// Read the upper left y coordinate
	hdrFile >> strName >> strValue;
	gh.ULYMap = atof(strValue);

	// Read the x pixel spacing
	hdrFile >> strName >> strValue;
	gh.XDim = atof(strValue);

	// Read the y pixel spacing
	hdrFile >> strName >> strValue;
	gh.YDim = atof(strValue);

	// Close the file
	hdrFile.close();

	// make the corresponding filename for the DEM
	vtString dem_fname = ChangeFileExtension(szFileName, ".dem");
	FILE *fp = vtFileOpen(dem_fname, "rb");
	if (!fp)
		return false;

	if (progress_callback != NULL) progress_callback(5);

	// Projection is always geographic, integer
	m_crs.SetSimple(false, 0, EPSG_DATUM_WGS84);
	m_bFloatMode = false;

	m_EarthExtents.left = gh.ULXMap;
	m_EarthExtents.top = gh.ULYMap;
	m_EarthExtents.right = m_EarthExtents.left + (gh.XDim * (gh.NumCols-1));
	m_EarthExtents.bottom = m_EarthExtents.top - (gh.YDim * (gh.NumRows-1));
	ComputeCornersFromExtents();

	// set up for an array of the indicated size
	m_iSize.Set(gh.NumCols, gh.NumRows);

	if (!AllocateGrid())
		return false;

	// read the file
	short z;
	char *cp = (char *)&z, temp;
	for (int j = 0; j < m_iSize.y; j++)
	{
		if (progress_callback != NULL) progress_callback(10 + j * 90 / m_iSize.y);
		for (int i = 0; i < m_iSize.x; i++)
		{
			/*  FIXME:  there be byte order issues here.  See below in this routine.  */
			if (fread(&z, sizeof(short), 1, fp) != 1)
				return false;
			if (gh.ByteOrder[0] == 'M')
			{
				// must swap byte order
				temp = cp[0];
				cp[0] = cp[1];
				cp[1] = temp;
			}
			SetValue(i, m_iSize.y-1-j, (z == gh.NoData) ? INVALID_ELEVATION : z);
		}
	}
	return true;
}


// helper
void ParseExtent(DRECT &extents, const char *name, const char *value)
{
	if (!strcmp(name, "upper_map_y"))
		extents.top = atof(value);
	if (!strcmp(name, "lower_map_y"))
		extents.bottom = atof(value);
	if (!strcmp(name, "left_map_x"))
		extents.left = atof(value);
	if (!strcmp(name, "right_map_x"))
		extents.right = atof(value);
}

/** Loads from a NOAA GlOBE file.
 *
 * In fact, there is no "GLOBE format", GLOBE files are delivered as raw
 * data, which can be intepreted using a variety of separate header files.
 * Using the GLOBE server "Select Your Own Area" feature results in
 * 2 files, a header with a .hdr extension and data with a .bin extension.
 * This method reads those file.  Pass the filename of the .hdr file to this
 * function, and it will automatically look for a corresponding .bin file in
 * the same location.
 *
 * Projection is always geographic and elevation is integer meters.
 *
 * You should call SetupLocalCS() after loading if you will be doing
 * heightfield operations on this grid.
 *
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromGLOBE(const char *szFileName,
									bool progress_callback(int))
{
	// Free buffers to prepare to receive new data
	FreeData();

	if (progress_callback != NULL)
		progress_callback(1);

	// Open the header file
	// Charset Beware!  Filename may be utf-8, but how can we open an ifstream with utf-8 in
	//  a portable way?
	ifstream hdrFile(szFileName);
	if (!hdrFile.is_open())
	  return false;

	// Parse the file
	char strName[30];
	char szEqual[30];
	char strValue[30];
	vtString file_title;
	vtString str;

	// Read file_title
	hdrFile >> strName >> szEqual;
	file_title = get_line_from_stream(hdrFile);	// file title

	if (strcmp(strName, "file_title"))
		return false;	// not a GLOBE header

	// skip a few
	hdrFile >> strName >> szEqual;
	str = get_line_from_stream(hdrFile);	// data_type
	hdrFile >> strName >> szEqual;
	str = get_line_from_stream(hdrFile);	// grid_cell_registration
	hdrFile >> strName >> szEqual;
	str = get_line_from_stream(hdrFile);	// map_projection

	// Read the left, right, upper, lower coordinates
	hdrFile >> strName >> szEqual >> strValue;
	ParseExtent(m_EarthExtents, strName, strValue);
	hdrFile >> strName >> szEqual >> strValue;
	ParseExtent(m_EarthExtents, strName, strValue);
	hdrFile >> strName >> szEqual >> strValue;
	ParseExtent(m_EarthExtents, strName, strValue);
	hdrFile >> strName >> szEqual >> strValue;
	ParseExtent(m_EarthExtents, strName, strValue);

	// Read the number of rows
	hdrFile >> strName >> szEqual >> strValue;
	m_iSize.y = atol(strValue);

	// Read the number of columns
	hdrFile >> strName >> szEqual >> strValue;
	m_iSize.x = atol(strValue);

	// OK to skip the rest of the file, parameters which we will assume

	// Close the header file
	hdrFile.close();

	// make the corresponding filename for the DEM
	char dem_fname[200];
	strcpy(dem_fname, szFileName);
	char *ext = strrchr(dem_fname, '.');
	if (!ext)
		return false;
	strcpy(ext, ".bin");
	FILE *fp = vtFileOpen(dem_fname, "rb");
	if (!fp)
		return false;

	if (progress_callback != NULL) progress_callback(5);

	// Projection is always geographic, integer
	m_crs.SetSimple(false, 0, EPSG_DATUM_WGS84);
	m_bFloatMode = false;

	ComputeCornersFromExtents();

	if (!AllocateGrid())
		return false;

	// read the file
	int i, j;
	short z;
	for (j = 0; j < m_iSize.y; j++)
	{
		if (progress_callback != NULL) progress_callback(10 + j * 90 / m_iSize.y);
		for (i = 0; i < m_iSize.x; i++)
		{
			if (fread(&z, sizeof(short), 1, fp) != 1)
				return false;
			if (z == -500)	// 'unknown' generally used for ocean surface
				z = 0;
			SetValue(i, m_iSize.y-1-j, z);
		}
	}
	return true;
}


/**
 * Helper function for LoadFromGRD, loads from a Surfer ascii grid file (GRD)
 *
 * Projection is always geographic and elevation is floating-point.
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromDSAA(const char* szFileName, bool progress_callback(int))
{
	// Free buffers to prepare to receive new data
	FreeData();

	FILE *fp = vtFileOpen(szFileName, "rt");
	if (!fp)
		return false;

	int nx, ny;
	// read GRD header data
	double xlo, xhi, ylo, yhi, zlo, zhi;

	char ch[100];
	if (fscanf(fp, "%s\n", ch) != 1) return false; // DSAA
	if (fscanf(fp, "%d%d\n", &nx, &ny) != 2) return false;
	if (fscanf(fp, "%lf%lf\n", &xlo, &xhi) != 2) return false;
	if (fscanf(fp, "%lf%lf\n", &ylo, &yhi) != 2) return false;
	if (fscanf(fp, "%lf%lf\n", &zlo, &zhi) != 2) return false;

	// Set the CRS (actually we don't know it)
	m_crs.SetSimple(true, 1, EPSG_DATUM_WGS84);

	// set the corresponding vtElevationGrid info
	m_bFloatMode = true;
	m_EarthExtents.left = xlo;
	m_EarthExtents.top = yhi;
	m_EarthExtents.right = xhi;
	m_EarthExtents.bottom = ylo;
	ComputeCornersFromExtents();

	m_iSize.Set(nx, ny);

	if (!AllocateGrid())
		return false;

	float z;
	for (int y = 0; y < ny; y++)
	{
		if (progress_callback != NULL && (y%20) == 0)
		{
			if (progress_callback(y * 100 / ny))
			{
				// Cancel
				fclose(fp);
				return false;
			}
		}
		for (int x = 0; x < nx; x++)
		{
			if (fscanf(fp, "%f", &z) != 1)
				return false;
			if(z < zlo || z > zhi)
				SetFValue(x,y,INVALID_ELEVATION);
			else
				SetFValue(x, y, z);
		}
	}

	fclose(fp);
	return true;
}


/**
 * Loads from a Surfer binary grid file (GRD)
 *
 * GRD does not contain any information about CRS, so the following assumption
 * is made: UTM zone 1.
 *
 * You should call SetupLocalCS() after loading if you will be doing
 * heightfield operations on this grid.
 *
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromGRD(const char *szFileName,
								  bool progress_callback(int))
{
	// Free buffers to prepare to receive new data
	FreeData();

	if (progress_callback != NULL)
		progress_callback(1);

	// Open the header file
	FILE *fp = vtFileOpen(szFileName, "rb");
	if (!fp)
	  return false;

	// read GRD header data
	int nx, ny;
	double xlo, xhi, ylo, yhi, zlo, zhi;

	// Parse the file, determine what kind of GRD it is
	int quiet;
	char szHeader[5];
	quiet = fread(szHeader, 4, 1, fp);
	bool bDSBB = !strncmp(szHeader, "DSBB", 4);
	bool bDSRB = !strncmp(szHeader, "DSRB", 4);
	bool bDSAA = !strncmp(szHeader, "DSAA", 4);

	if (bDSBB) //DSBB format
	{
		// read GRD header data
		short nx2, ny2;

		/*  FIXME:  there be byte order issues here.  See below in this routine.  */
		quiet = fread(&nx2, 2, 1, fp);
		quiet = fread(&ny2, 2, 1, fp);
		nx = nx2;
		ny = ny2;

		quiet = fread(&xlo, 8, 1, fp);
		quiet = fread(&xhi, 8, 1, fp);
		quiet = fread(&ylo, 8, 1, fp);
		quiet = fread(&yhi, 8, 1, fp);
		quiet = fread(&zlo, 8, 1, fp);
		quiet = fread(&zhi, 8, 1, fp);
	}
	else if (bDSRB) //DSRB format, Surfer 7
	{
		int size = 0L; // size of header
		quiet = fread(&size, 4, 1, fp); //needs to be 4
		long id = 0L;
		quiet = fread(&id, 4, 1, fp); //needs to be 1
		char tag[5];
		quiet = fread(&tag, 4, 1, fp); //nedds to be "GRID"
		quiet = fread(&size, 4, 1, fp); // size in bytes of grid section

		quiet = fread(&ny, 4, 1, fp);	// Yes, the Y coordinate *does* come first!
		quiet = fread(&nx, 4, 1, fp);
		quiet = fread(&xlo, 8, 1, fp);
		quiet = fread(&ylo, 8, 1, fp);

		double xsize;
		quiet = fread(&xsize, 8, 1, fp);
		xhi = xlo + xsize*nx;
		double ysize;
		quiet = fread(&ysize, 8, 1, fp);
		yhi = ylo + ysize*ny;

		quiet = fread(&zlo, 8, 1, fp);
		quiet = fread(&zhi, 8, 1, fp);

		double rotation;
		quiet = fread(&rotation, 8, 1, fp);
		double blankValue;
		quiet = fread(&blankValue, 8, 1, fp);
		quiet = fread(&tag, 4, 1, fp); // should be "DATA"
		quiet = fread(&size, 4, 1, fp);
		assert(size/sizeof(double) == nx*ny);
	}
	else if (bDSAA) //DSAA format
	{
		fclose(fp);
		return LoadFromDSAA(szFileName,progress_callback);
	}
	else
	{
		// not the right kind of file
		fclose(fp);
		return false;
	}
	// Set the CRS.  GRD doesn't tell us projection, so we simply default
	//  to UTM zone 1, so that we have something meters-based.
	m_crs.SetSimple(true, 1, EPSG_DATUM_WGS84);

	// set the corresponding vtElevationGrid info
	m_bFloatMode = true;
	m_EarthExtents.left = xlo;
	m_EarthExtents.top = yhi;
	m_EarthExtents.right = xhi;
	m_EarthExtents.bottom = ylo;
	ComputeCornersFromExtents();

	m_iSize.Set(nx, ny);

	if (!AllocateGrid())
		return false;

	float z;
	double dz;
	for (int y = 0; y < ny; y++)
	{
		if (progress_callback != NULL)
		{
			if (progress_callback(y * 100 / ny))
			{
				// Cancel
				fclose(fp);
				return false;
			}
		}
		for (int x = 0; x < nx; x++)
		{
			if (bDSRB) //DSRB format
			{
				quiet = fread(&dz, 8, 1, fp);
				z = (float) dz;

				if (z < zlo || z > zhi)
					z = INVALID_ELEVATION;

				SetFValue(x, y, z);
			}
			else
			{
				quiet = fread(&z, 4, 1, fp);

				if (z < zlo || z > zhi)
					z = INVALID_ELEVATION;

				SetFValue(x, y, z);
			}
		}
	}
	fclose(fp);
	return true;
}


/**
 * Loads from a PGM (Portable Gray Map) file.
 * Both PGM Binary and ASCII varieties are supported.
 *
 * PGM does not contain any information about geographic location, so
 * the following assumptions are made: UTM coordinates, 1-meter spacing,
 * origin at (0,0).
 *
 * You should call SetupLocalCS() after loading if you will be doing
 * heightfield operations on this grid.
 *
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromPGM(const char *szFileName, bool progress_callback(int))
{
	// Free buffers to prepare to receive new data
	FreeData();

	// open input file
	FILE *fp = vtFileOpen(szFileName, "rb");
	if (!fp)		// Could not open input file
		return false;

	char sbuf[512],			// buffer for file input
		 dummy[2];			// used for \n and \0
	if (fread(sbuf, sizeof(char), 2, fp) != 2)
	{
		// "Could not read file"
		return false;
	}

	bool bBinary = false;	// PGM binary format flag: assume PGM ascii
	if (strncmp(sbuf,"P5",2) == 0)
		bBinary = 1;		// PGM binary format

	if (strncmp(sbuf,"GI",2) == 0) {
		fclose(fp);
		return false;
	}
	else if ((strncmp(sbuf,"P5",2) != 0) && (strncmp(sbuf,"P2",2) != 0))
	{
		/* not PGM Ascii */
		fclose(fp);
		return false;
	}

	// read PGM ASCII or binary file
	bool have_header = false;
	int coord_sys = 0;
	int coord_zone = 0;
	int coord_datum = 0;
	int missing_value = INVALID_ELEVATION;
	DRECT ext;
	double x, y;
	bool bArcSeconds = false;
	int quiet;
	while ((fscanf(fp, "%s", sbuf) != EOF) && sbuf[0] == '#')
	{
		// comment
		quiet = fscanf(fp,"%[^\n]", sbuf);  // read comment beyond '#'
		quiet = fscanf(fp,"%[\n]", dummy);  // read newline
		char *buf = sbuf+1;	// skip leading space after '#'
		if (!strncmp(buf, "DEM", 3) || !strncmp(buf, "BOX", 3))
		{
			have_header= true;
		}
		else if (!strncmp(buf, "description", 11))
		{
			//do nothing
		}
		else if (!strncmp(buf, "coordinate system", 17))
		{
			if (!strcmp(buf+18, "LL")) coord_sys=0;
			else if (!strcmp(buf+18,"UTM")) coord_sys=1;
		}
		else if (!strncmp(buf, "coordinate zone", 15))
		{
			coord_zone = atoi(buf+16);
		}
		else if (!strncmp(buf, "coordinate datum", 16))
		{
			coord_datum = atoi(buf+17);
		}
		else if (!strncmp(buf, "SW corner", 9))
		{
			char units[99];
			quiet = sscanf(buf+10, "%lf/%lf %s", &x, &y, units);
			ext.left = x;
			ext.bottom = y;
			// check units, might be arcseconds or degrees
			if (!strcmp(units, "arc-seconds"))
				bArcSeconds = true;
		}
		else if (!strncmp(buf, "NE corner", 9))
		{
			quiet = sscanf(buf+10, "%lf/%lf", &x, &y);
			ext.right = x;
			ext.top = y;
		}
		else if (!strncmp(buf, "missing value", 13))
		{
			missing_value = atoi(buf+14);
		}
	}

	int xsize, ysize, maxval;
	xsize = atoi(sbuf);		// store xsize of array
	quiet = fscanf(fp,"%d\n",&ysize);		// read ysize of array
	quiet = fscanf(fp,"%d",&maxval);		// read maxval of array
	// Be careful here with the last LF!
	// Simply calling fscanf(fp,"\n") sometimes eats 2 characters.
	int cr = fgetc(fp);

	// set the corresponding vtElevationGrid info
	m_bFloatMode = false;
	if (have_header)
	{
		int datum = EPSG_DATUM_WGS84;
		switch (coord_datum)
		{
		case 1: datum = EPSG_DATUM_NAD27; break;
		case 2: datum = EPSG_DATUM_WGS72; break;
		case 3: datum = EPSG_DATUM_WGS84; break;
		case 4: datum = EPSG_DATUM_NAD83; break;

		case 5: /*Sphere (with radius 6370997 meters)*/ break;
		case 6: /*Clarke1880 (Clarke spheroid of 1880)*/ break;
		case 7: /*International1909 (Geodetic Reference System of 1909)*/ break;
		case 8: /*International1967 (Geodetic Reference System of 1967)*/; break;

		case 9: /*WGS60*/ break;
		case 10: /*WGS66*/; break;
		case 11: /*Bessel1841*/ break;
		case 12: /*Krassovsky*/ break;
		}
		if (coord_sys == 0)	// LL
		{
			m_crs.SetSimple(false, 0, datum);
			if (bArcSeconds)
			{
				ext.left /= 3600;	// arc-seconds to degrees
				ext.right /= 3600;
				ext.top /= 3600;
				ext.bottom /= 3600;
			}
		}
		else if (coord_sys == 1)	// UTM
			m_crs.SetSimple(true, coord_zone, datum);
	}
	else
	{
		// Set the CRS (actually we don't know it)
		m_crs.SetSimple(true, 1, EPSG_DATUM_WGS84);

		ext.left = 0;
		ext.top = ysize-1;
		ext.right = xsize-1;
		ext.bottom = 0;
	}
	SetEarthExtents(ext);
	ComputeCornersFromExtents();

	m_iSize.Set(xsize, ysize);

	if (!AllocateGrid())
		return false;

	if (bBinary)
	{
		// read PGM binary
		int offset_start = ftell(fp);
		fseek(fp, 0, SEEK_END);
		int offset_end = ftell(fp);
		fseek(fp, offset_start, SEEK_SET);	// go back again

		int data_length = offset_end - offset_start;
		int data_size = data_length / (xsize*ysize);

		uchar oneb;		// one byte from file
		short twob;				// two bytes from file
		for (int j = 0; j < ysize; j++)
		{
			if (progress_callback != NULL) progress_callback(j * 100 / ysize);
			for (int i = 0; i < xsize; i++)
			{
				if (maxval == 32767 || maxval == 65535)
				{
					quiet = fread(&twob, sizeof(short), 1, fp);
					twob = SwapShort(twob);
					if (twob == missing_value)
						twob = INVALID_ELEVATION;
					SetFValue(i, ysize-1-j, twob);
				}
				else
				{
					quiet = fread(&oneb, sizeof(uchar), 1, fp);
					SetFValue(i, ysize-1-j, oneb);
				}
			}
		}
	}
	else
	{
		// read PGM ASCII
		double a;
		char *junk;			// unconverted part of a number
		for (int j = 0; j < ysize; j++)
		{
			if (progress_callback != NULL) progress_callback(j * 100 / ysize);
			for (int i = 0; i < xsize; i++)
			{
				quiet = fscanf(fp, "%s", sbuf);
				a = strtod(sbuf, &junk);
				SetFValue(i, ysize-1-j, (float)a);
			}
		}
	}
	fclose(fp);
	return true;
}


/**
 * Writes the grid to a TerraGen Terrain file.
 * \param szFileName The filename to write to.
 */
bool vtElevationGrid::SaveToTerragen(const char *szFileName) const
{
	FILE *fp = vtFileOpen(szFileName, "wb");
	if (!fp)
		return false;

	short w = (short) m_iSize.x;
	short h = (short) m_iSize.y;
	short dummy = 0;

	// write identifying header
	fwrite("TERRAGEN", 8, 1, fp);
	fwrite("TERRAIN ", 8, 1, fp);

	// write dimension
	fwrite("SIZE", 4, 1, fp);
	fwrite(&w, 2, 1, fp);
	fwrite(&dummy, 2, 1, fp);

	fwrite("XPTS", 4, 1, fp);
	fwrite(&w, 2, 1, fp);
	fwrite(&dummy, 2, 1, fp);

	fwrite("YPTS", 4, 1, fp);
	fwrite(&h, 2, 1, fp);
	fwrite(&dummy, 2, 1, fp);

	// write "scale"
	// this is actually the grid spacing (x, y) and elevation precision (z)
	fwrite("SCAL", 4, 1, fp);
	FPoint2 spacing = GetSpacing();
	float zscale = 1.0f;
	fwrite(&spacing.x, 4, 1, fp);
	fwrite(&spacing.y, 4, 1, fp);
	fwrite(&zscale, 4, 1, fp);

	// don't write "CRAD" and "CRVM"
	// they are optional and would not have meaningful values

	// write altitude (elevation data)
	fwrite("ALTW", 4, 1, fp);
	short HeightScale = 16384;		// divided by 65526, means units of 1/4 meter
	short BaseHeight = 0;
	fwrite(&HeightScale, 2, 1, fp);
	fwrite(&BaseHeight, 2, 1, fp);

	int i, j;
	float fvalue;
	short svalue;
	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			fvalue = GetFValue(i, j);
			svalue = (short) (fvalue * 4.0f);
			fwrite(&svalue, 2, 1, fp);
		}
	}
	fclose(fp);
	return true;
}


bool vtElevationGrid::SaveTo3TX(const char *szFileName, bool progress_callback(int)) const
{
	if (m_iSize.x != 1201 || m_iSize.y != 1201)
		return false;

	FILE *fp = vtFileOpen(szFileName, "wb");
	if (!fp)
		return false;

	// SW origin in integer degrees
	DRECT ext = GetEarthExtents();
	fprintf(fp, "%d %d\r\n", (int) ext.bottom, (int) ext.left);

	// elevationdata one per line in column-first order from the SW
	short val;
	for (int i = 0; i < 1201; i++)
	{
		if (progress_callback != NULL)
			progress_callback(i * 100 / 1201);
		for (int j = 0; j < 1201; j++)
		{
			val = (short) GetFValue(i, j);
			fprintf(fp, "%d\r\n", val);
		}
	}
	return true;
}


/**
 * Write the elevation grid to a 16-bit greyscale GeoTIFF.
 */
bool vtElevationGrid::SaveToGeoTIFF(const char *szFileName) const
{
	g_GDALWrapper.RequestGDALFormats();

	// Save with GDAL to GeoTIFF
	GDALDriverManager *pManager = GetGDALDriverManager();
	if (!pManager)
		return false;

	GDALDriver *pDriver = pManager->GetDriverByName("GTiff");
	if (!pDriver)
		return false;

	char **papszParmList = NULL;

	// GDAL doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(szFileName);

	GDALDataset *pDataset;
	if (m_bFloatMode)
		pDataset = pDriver->Create(fname_local, m_iSize.x, m_iSize.y,
			1, GDT_Float32, papszParmList );
	else
		pDataset = pDriver->Create(fname_local, m_iSize.x, m_iSize.y,
			1, GDT_Int16, papszParmList );
	if (!pDataset)
		return false;

	const DPoint2 &spacing = GetSpacing();
	double adfGeoTransform[6] = { m_EarthExtents.left, spacing.x, 0,
								  m_EarthExtents.top, 0, -spacing.y };
	pDataset->SetGeoTransform(adfGeoTransform);

	char *pszSRS_WKT = NULL;
	m_crs.exportToWkt( &pszSRS_WKT );
	pDataset->SetProjection(pszSRS_WKT);
	CPLFree( pszSRS_WKT );

	GDALRasterBand *pBand = pDataset->GetRasterBand(1);

	if (m_bFloatMode)
	{
		float *raster = new float[m_iSize.x*m_iSize.y];
		for (int x = 0; x < m_iSize.x; x++)
		{
			for (int y = 0; y < m_iSize.y; y++)
			{
				// flip as we copy
				const float value = GetFValue(x, m_iSize.y-1-y);
				raster[y*m_iSize.x + x] = value;
			}
		}
		pBand->RasterIO(GF_Write, 0, 0, m_iSize.x, m_iSize.y,
			raster, m_iSize.x, m_iSize.y, GDT_Float32, 0, 0);
		delete raster;
	}
	else
	{
		GInt16 *raster = new GInt16[m_iSize.x*m_iSize.y];
		for (int x = 0; x < m_iSize.x; x++)
		{
			for (int y = 0; y < m_iSize.y; y++)
			{
				// flip as we copy
				const float value = GetFValue(x, m_iSize.y-1-y);
				raster[y*m_iSize.x + x] = (short) value;
			}
		}
		pBand->RasterIO(GF_Write, 0, 0, m_iSize.x, m_iSize.y,
			raster, m_iSize.x, m_iSize.y, GDT_Int16, 0, 0);
		delete raster;
	}
	GDALClose(pDataset);

	return true;
}

/**
 * Loads an elevation grid using the GDAL library.  The GDAL library
 * supports a very large number of formats, including SDTS-DEM.  See
 * http://www.remotesensing.org/gdal/formats_list.html for the complete list
 * of GDAL-supported formats.
 *
 * \param szFileName The file name to read from.
 * \param progress_callback If supplied, this function will be called back
 *		with a value of 0 to 100 as the operation progresses.
 * \param err If supplied, will be set to a description of any error that occurs.
 *
 * You should call SetupLocalCS() after loading if you will be doing
 * heightfield operations on this grid.
 *
 * \returns True if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadWithGDAL(const char *szFileName,
								   bool progress_callback(int), vtElevError *err)
{
	g_GDALWrapper.RequestGDALFormats();

	// GDAL doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(szFileName);

	GDALDataset  *poDataset = (GDALDataset *) GDALOpen(fname_local, GA_ReadOnly);
	if (poDataset == NULL)
	{
		SetError(err, vtElevError::FILE_OPEN, "Couldn't open GDAL dataset from '%s'", szFileName);
		return false;
	}
	m_iSize.x = poDataset->GetRasterXSize();
	m_iSize.y = poDataset->GetRasterYSize();

	// Get the projection information.
	const char *str1 = poDataset->GetProjectionRef();
	char *str2 = (char *) str1;
	OGRErr ogr_err = m_crs.importFromWkt(&str2);
	if (ogr_err != OGRERR_NONE)
	{
		// No projection info.
		m_crs.Clear();
	}

	// Get spacing and extents
	double		adfGeoTransform[6];
	if (poDataset->GetGeoTransform(adfGeoTransform) == CE_None)
	{
		// Upper left corner is adfGeoTransform[0], adfGeoTransform[3]
		m_EarthExtents.left = adfGeoTransform[0];
		m_EarthExtents.top = adfGeoTransform[3];
		m_EarthExtents.right = m_EarthExtents.left + (adfGeoTransform[1] * m_iSize.x);
		m_EarthExtents.bottom = m_EarthExtents.top + (adfGeoTransform[5] * m_iSize.y);
	}
	else
	{
		// No extents
		m_EarthExtents.SetToZero();
	}

	ComputeCornersFromExtents();

	// Raster count should be 1 for elevation datasets
//	int rc = poDataset->GetRasterCount();

	GDALRasterBand *poBand = poDataset->GetRasterBand(1);

	// Assume block size will be one raster line
//	int			 nBlockXSize, nBlockYSize;
//	poBand->GetBlockSize(&nBlockXSize, &nBlockYSize);

	// Check data type - it's either integer or float
	GDALDataType rtype = poBand->GetRasterDataType();
	VTLOG("vtElevationGrid::LoadWithGDAL: Raster size %d x %d, type '%s'\n",
		m_iSize.x, m_iSize.y, GDALGetDataTypeName(rtype));
	if (rtype == GDT_Int16 || rtype == GDT_Byte)
		m_bFloatMode = false;
	else
		m_bFloatMode = true;

	// Check vertical units - if they're feet, scale to meters
	float fScale = 1.0f;
	const char *runits = poBand->GetUnitType();
	if (runits && !strcmp(runits, "ft"))
	{
		// feet to meter conversion
		if (m_bFloatMode)
			fScale = 0.3048f;	// already floats, just multiply
		else
			SetScale(0.3048f);	// stay with shorts, use scaling
	}

	// We don't check for memory allocation many places in the VTP software,
	//  because most allocations are fine-grained and will degrade gracefully
	//  on a modern OS.  However, some singular large allocation, such as
	//  huge elevation grids, should be checked.
	bool success = AllocateGrid(err);
	if (!success)
		return false;

	short *pasScanline;
	short elev;
	float *pafScanline;
	float fElev;
	int   nXSize = poBand->GetXSize();

	pasScanline = (short *) CPLMalloc(sizeof(short)*nXSize);
	pafScanline = (float *) CPLMalloc(sizeof(float)*nXSize);
	int i, j;
	for (j = 0; j < m_iSize.y; j++)
	{
		if (m_bFloatMode)
		{
			poBand->RasterIO(GF_Read, 0, j, nXSize, 1,
							  pafScanline, nXSize, 1, GDT_Float32,
							  0, 0);
			for (i = 0; i < nXSize; i++)
			{
				fElev = pafScanline[i];

				// check for several different commonly used values meaning
				// "no data at this location"
				if (fElev == -9999 || fElev == -32766 || fElev == 32767 || fElev == -32767 || fElev < -100000)
					SetValue(i, m_iSize.y-1-j, INVALID_ELEVATION);
				else
					SetFValue(i, m_iSize.y-1-j, fElev * fScale);
			}
		}
		else
		{
			poBand->RasterIO(GF_Read, 0, j, nXSize, 1,
							  pasScanline, nXSize, 1, GDT_Int16,
							  0, 0);
			for (i = 0; i < nXSize; i++)
			{
				elev = pasScanline[i];

				// check for several different commonly used values meaning
				// "no data at this location"
				if (elev == -9999 || elev == -32766 || elev == 32767)
					SetValue(i, m_iSize.y-1-j, INVALID_ELEVATION);
				else
					SetFValue(i, m_iSize.y-1-j, elev * fScale);
			}
		}
		if (progress_callback != NULL)
		{
			if (progress_callback(100*j/m_iSize.y))
			{
				// Cancel
				delete poDataset;
				SetError(err, vtElevError::CANCELLED, "Cancelled");
				return false;
			}
		}
	}

	// Clean up
	CPLFree(pasScanline);
	CPLFree(pafScanline);
	delete poDataset;

	// Return success
	return true;
}

bool vtElevationGrid::ParseNTF5(GDALDataset *pDatasource, vtString &msg,
								bool progress_callback(int))
{
	// Time Test
	clock_t tm1 = clock();

	OGREnvelope Extent;
	OGRFeature *pFeature = NULL;

	int lays = pDatasource->GetLayerCount();
	if (1 != lays)
	{
		msg.Format("Layer count is %d, expected 1", lays);
		return false;
	}
	OGRLayer *pLayer = pDatasource->GetLayer(0);
	if (NULL == pLayer)
	{
		msg = "Couldn't get layer";
		return false;
	}
	OGRFeatureDefn *pFeatureDefn = pLayer->GetLayerDefn();
	if (NULL == pFeatureDefn)
	{
		msg = "Couldn't get feature definition";
		return false;
	}
	if (0 != strncmp(pFeatureDefn->GetName(), "DTM_", 4))
	{
		msg = "Feature definition doesn't start with 'DTM_'";
		return false;
	}
	if (wkbPoint25D != pFeatureDefn->GetGeomType())
	{
		msg = "Feature type isn't Point25D";
		return false;
	}
	if (1 != pFeatureDefn->GetFieldCount())
	{
		msg = "Field count isn't 1";
		return false;
	}
	OGRFieldDefn *pFieldDefn = pFeatureDefn->GetFieldDefn(0);
	if (NULL == pFieldDefn)
	{
		msg = "Couldn't get field definition";
		return false;
	}
	if (0 != strcmp(pFieldDefn->GetNameRef(), "HEIGHT"))
	{
		msg = "Couldn't get HEIGHT field";
		return false;
	}
	OGRSpatialReference *pSpatialRef = pLayer->GetSpatialRef();
	if (NULL == pSpatialRef)
	{
		msg = "Couldn't get spatial reference";
		return false;
	}

	// Time Test
	clock_t tm2 = clock();

	pLayer->GetExtent(&Extent);

	float time_ext = ((float)clock() - tm2)/CLOCKS_PER_SEC;
	VTLOG("ParseNTF5 GetExtent: %.2f seconds.\n", time_ext);

	// Get number of features. In this case the total number of cells
	// in the elevation matrix
	int iTotalCells = (int) pLayer->GetFeatureCount();

	pLayer->ResetReading();

	// Prescan the features to calculate the x and y intervals
	// This is awkward, it would be better to do everything in one pass.
	int iRowCount = 0;
	OGRPoint *pPoint;
	double dX = 0;	// set to avoid compiler warning
	while ( (pFeature = pLayer->GetNextFeature()) != NULL )
	{
		// make sure we delete the feature no matter how the loop exits
		std::auto_ptr<OGRFeature> ensure_deletion(pFeature);

		if (NULL == (pPoint = (OGRPoint*)pFeature->GetGeometryRef()))
		{
			msg = "Couldn't get point feature";
			return false;
		}

		// Is there any need to cal wkbFlatten or check the geom type?
		//if (wkbPoint != wkbFlatten(pPoint->getGeometryType()))	// RJ fix 03.11.21
		//{
		//	msg = "Couldn't flatten point feature";
		//	return false;
		//}

		if (0 == iRowCount)
			dX = pPoint->getX();
		else if (pPoint->getX() != dX)
			break;

		iRowCount++;
	}

	int iColCount = iTotalCells / iRowCount;

	m_iSize.x = iColCount;
	m_iSize.y = iRowCount;
	m_crs.SetSpatialReference(pSpatialRef);

	// One online reference says of NTF elevation:
	// "Height values are rounded to the nearest metre", hence integers.
	// However, NTF dems have been seen that very definitely have good smooth
	//  float values.  So, stay on the safe side and expect floats.
	m_bFloatMode = true;
	m_EarthExtents.left = Extent.MinX;
	m_EarthExtents.top = Extent.MaxY;
	m_EarthExtents.right = Extent.MaxX;
	m_EarthExtents.bottom = Extent.MinY;
	ComputeCornersFromExtents();

	if (!AllocateGrid())
		return false;

	// Time Test
	clock_t tm3 = clock();

	pLayer->ResetReading();

	int i;
	for (i = 0; i < iTotalCells; i++)
	{
		// Don't call progress too much, 1 time every 1000 is fine.
		if ((i%1000)==0 && progress_callback != NULL)
		{
			if (progress_callback(i * 100 / iTotalCells))
			{
				// Cancel
				return false;
			}
		}
		if (NULL == (pFeature = pLayer->GetNextFeature()))
		{
			msg = "Couldn't get next feature";
			return false;
		}
		if (NULL == (pPoint = (OGRPoint*)pFeature->GetGeometryRef()))
		{
			msg = "Couldn't get point feature";
			return false;
		}
		SetFValue(i / iRowCount, i % iRowCount, (float) pPoint->getZ());
		delete pFeature;
		pFeature = NULL;
	}

	// Time Test
	float time_read = ((float)clock() - tm3)/CLOCKS_PER_SEC;
	VTLOG("ParseNTF5 read: %.2f seconds.\n", time_read);

	ComputeHeightExtents();
	msg.Format("Read %d cells (%d x %d)", iTotalCells, m_iSize.x, m_iSize.y);

	// Time Test
	float time = ((float)clock() - tm1)/CLOCKS_PER_SEC;
	VTLOG("ParseNTF5 total: %.2f seconds.\n", time);

	return true;
}

/**
 * Loads an elevation grid from an UK Ordnance Survey NTF level 5 file using
 * the OGR library.
 *
 * \param szFileName The file name to read from.
 * \param progress_callback If supplied, this function will be called back
 *		with a value of 0 to 100 as the operation progresses.
 *
 * You should call SetupLocalCS() after loading if you will be doing
 * heightfield operations on this grid.
 *
 * \returns True if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromNTF5(const char *szFileName,
								   bool progress_callback(int))
{
	// Free buffers to prepare to receive new data
	FreeData();

	// let GDAL know we're going to use its OGR format drivers
	g_GDALWrapper.RequestOGRFormats();

	// OGR doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(szFileName);

	vtString msg;
	bool bSuccess = false;
	GDALDataset *pDatasource = (GDALDataset*) GDALOpen(fname_local, GA_ReadOnly);
	if (pDatasource)
		bSuccess = ParseNTF5(pDatasource, msg, progress_callback);

	delete pDatasource;
	return bSuccess;
}

/**
 * Loads from a RAW file (a naked array of elevation values).
 * The file will not contain any information about at all about data size,
 * data type, or CRS, so this information must be passed in as arguments.
 *
 * \param szFileName	The file name to read from.
 * \param width			The width of the expected array.
 * \param height		The height of the expected array.
 * \param bytes_per_element The number of bytes for each elevation value.  If
 *		this is 1 or 2, the data is assumed to be integer.  If 4, then the
 *		data is assumed to be floating-point values.
 * \param vertical_units Indicates what scale factor to apply to the
 *		elevation values to convert them to meters.  E.g. if the file is in
 *		meters, pass 1.0, if in feet, pass 0.3048.
 * \param bBigEndian	True for Big-endian byte order, false for
 *		Little-endian (Intel byte order).
 * \param progress_callback If supplied, this function will be called back
 *		with a value of 0 to 100 as the operation progresses.
 *
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromRAW(const char *szFileName, int width, int height,
								  int bytes_per_element, float vertical_units,
								  bool bBigEndian, bool progress_callback(int))
{
	// Free buffers to prepare to receive new data
	FreeData();

	FILE *fp = vtFileOpen(szFileName, "rb");
	if (!fp)
		return false;

	m_iSize.Set(width, height);

	// set extents arbitrarily for now; if the user knows them, they can set
	// them after loading
	m_EarthExtents.left = 0;
	m_EarthExtents.top = height;
	m_EarthExtents.right = width;
	m_EarthExtents.bottom = 0;
	ComputeCornersFromExtents();

	m_crs.SetSimple(true, 1, EPSG_DATUM_WGS84);
	if (bytes_per_element == 4)
		m_bFloatMode = true;
	else
		m_bFloatMode = false;

	if (!AllocateGrid())
		return false;

	ByteOrder order;
	if (bBigEndian)
		order = BO_BIG_ENDIAN;
	else
		order = BO_LITTLE_ENDIAN;

	int i, j, z;
	void *data = &z;
	int quiet;
	for (j = 0; j < m_iSize.y; j++)
	{
		if (progress_callback != NULL)
		{
			if (progress_callback(100*j/m_iSize.y))
			{
				// Cancel
				fclose(fp);
				return false;
			}
		}
		for (i = 0; i < m_iSize.x; i++)
		{
			if (bytes_per_element == 1)
			{
				quiet = fread(data, 1, 1, fp);
				SetValue(i, m_iSize.y-1-j, *((uchar *)data));
			}
			if (bytes_per_element == 2)
			{
				FRead(data, DT_SHORT, 1, fp, order);
				SetFValue(i, m_iSize.y-1-j, *((short *)data) * vertical_units);
			}
			if (bytes_per_element == 4)
			{
				FRead(data, DT_FLOAT, 1, fp, order);
				SetFValue(i, m_iSize.y-1-j, *((float *)data) * vertical_units);
			}
		}
	}

	// Clean up
	fclose(fp);

	SetScale(vertical_units);

	// Return success
	return true;
}


/**
 * Loads from a MicroDEM format file.
 *
 * You should call SetupLocalCS() after loading if you will be doing
 * heightfield operations on this grid.
 *
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromMicroDEM(const char *szFileName, bool progress_callback(int))
{
	// Free buffers to prepare to receive new data
	FreeData();

	/* open input file */
	FILE *fp = vtFileOpen(szFileName, "rb");
	if (!fp)	// Could not open input file
		return false;

	char buf[40];
	if (fread(buf, 9, 1, fp) != 1)
		return false;
	if (strncmp(buf, "*MICRODEM", 9))
	{
		// not MicroDEM format
		fclose(fp);
		return false;
	}

	// Find offsets to header and data
	int quiet;
	fseek(fp, 14, SEEK_SET);
	quiet = fread(buf, 5, 1, fp);
	buf[5] = '\0';
	int offset_to_header = atoi(buf);
	fseek(fp, 36, SEEK_SET);
	quiet = fread(buf, 5, 1, fp);
	buf[5] = '\0';
	int offset_to_data = atoi(buf);

	// Read header
	short xsize, ysize;
	short max_elev, min_elev;
	char xspacing, yspacing;
	int utm_x_lowerleft;
	int utm_y_lowerleft;
	char elev_unit_type;
	char utm_zone;
	char dem_type;
	char spacing_unit;
	char hemi;
	short elev;
	char digitize_datum;
	double scalefactor;
	int i, j;

	fseek(fp, offset_to_header, SEEK_SET);
	quiet = fread(&xsize, 2, 1, fp);
	quiet = fread(&ysize, 2, 1, fp);
	quiet = fread(buf, 3, 1, fp);			// "OSquareID"
	quiet = fread(&max_elev, 2, 1, fp);
	quiet = fread(&min_elev, 2, 1, fp);
	quiet = fread(&yspacing, 1, 1, fp);		// N-S data point spacing, in sec, min, or m
	quiet = fread(&xspacing, 1, 1, fp);		// E-W data point spacing, in sec, min, or m
	quiet = fread(buf, 20, 1, fp);			// unused
	quiet = fread(&utm_x_lowerleft, 4, 1, fp);
	quiet = fread(&utm_y_lowerleft, 4, 1, fp);
	quiet = fread(&elev_unit_type, 1, 1, fp);	// (Meters, Feet, TenthMgal,
		// Milligal, TenthGamma, Decimeters, Gammas, HundredthMGal, DeciFeet,
		// Centimeters, OtherElev, HundredthMa, HundredthPercentSlope,
		// Undefined, zDegrees, UndefinedHundredth
	quiet = fread(buf, 8, 1, fp);			// unused
	quiet = fread(&utm_zone, 1, 1, fp);
	quiet = fread(&dem_type, 1, 1, fp);		// 0 = UTM DEM, 1 = ArcSecond DEM
	quiet = fread(&spacing_unit, 1, 1, fp);	// Meters,Seconds,Minutes,KM,100m,Feet
		// KFeet,Degrees,HundredthSecond,MercProj100m,PolarStereo100m,100
	quiet = fread(&digitize_datum, 1, 1, fp);	// (WGS72, WGS84, NAD27, NAD83, Spherical, Local)
	quiet = fread(buf, 12, 1, fp);			// unused
	quiet = fread(&hemi, 1, 1, fp);			// hemisphere 'N' or 'S'
	// rest of header unused

	// Read data
	fseek(fp, offset_to_data, SEEK_SET);

	// Set the CRS
	switch (digitize_datum)
	{
	case 0: //WGS72
		m_crs.SetSimple(dem_type==0, utm_zone, EPSG_DATUM_WGS72);
		break;
	case 1: //WGS84
		m_crs.SetSimple(dem_type==0, utm_zone, EPSG_DATUM_WGS84);
		break;
	case 2: //NAD27
		m_crs.SetSimple(dem_type==0, utm_zone, EPSG_DATUM_NAD27);
		break;
	case 3: //NAD83
		m_crs.SetSimple(dem_type==0, utm_zone, EPSG_DATUM_NAD83);
		break;
	case 4: //Spherical
	case 5: //Local
	default:
		m_crs.SetSimple(dem_type==0, utm_zone, EPSG_DATUM_WGS84);
		break;
	}

	// convert extents to degrees or metres
	switch (spacing_unit) //Meters,Seconds,Minutes,KM,100m, Feet,KFeet, Degrees,HundredthSecond,
	  // MercProj100m,  PolarStereo100m,10m,TenthSecond
	{
	case 0: // Metres
		scalefactor = 1.0;
		break;
	case 1: // Arc Seconds
		scalefactor = 3600.0;
		break;
	case 2: // Arc Minutes
		scalefactor = 60.0;
		break;
	case 3: // Kilometres
		scalefactor = 0.001;
		break;
	case 4: // 100 Metres
		scalefactor = 0.01;
		break;
	case 5: // Feet
		scalefactor = 0.3048;
		break;
	case 6: // 1000 Feet
		scalefactor = 0.0003048;
		break;
	case 7: // Degrees
		scalefactor = 1;
		break;
	case 8: // .01 Arc Second
		scalefactor = 360000.0;
		break;
	case 9: // Mercator projection 100M
		scalefactor = 0.01;
		break;
	case 10: // PolarStereo 100M
		scalefactor = 0.01;
		break;
	case 11: // .1 Arc Second
		scalefactor = 36000.0;
		break;
	default:
		scalefactor = 1;
		break;
	}

	m_EarthExtents.left = utm_x_lowerleft / scalefactor;
	m_EarthExtents.top = (utm_y_lowerleft + (ysize - 1) * yspacing) / scalefactor;
	m_EarthExtents.right = (utm_x_lowerleft + (xsize - 1) * xspacing) / scalefactor;
	m_EarthExtents.bottom = utm_y_lowerleft / scalefactor;

	ComputeCornersFromExtents();

	// set the corresponding vtElevationGrid info
	m_bFloatMode = false;

	m_iSize.Set(xsize, ysize);

	if (!AllocateGrid())
		return false;

	for (i = 0; i < xsize; i++)
	{
		if (progress_callback != NULL) progress_callback(i * 100 / xsize);
		for (j = 0; j < ysize; j++)
		{
			if (fread(&elev, 2, 1, fp) != 1)
				return false;
			if (elev == 32767)
				SetValue(i, j, INVALID_ELEVATION);
			else
				SetValue(i, j, elev);
		}
	}
	fclose(fp);

	// Apply the vertical scaling factor *after* we have copied the raw
	// elevation values, since they are not meter values.
	switch (elev_unit_type)
	{
	case 0: // Meters
		break;
	case 1:	// Feet
		SetScale(0.3084f);
		break;
	case 5:	// Decimeters
		SetScale(0.1f);
		break;
	case 8:	// DeciFeet
		SetScale(0.3084f / 10);
		break;
	case 9:	// Centimeters
		SetScale(0.01f);
		break;
	}

	return true;
}


/**
 * Loads from an "XYZ file", which is a simple text file containing
 * a set of grid points in the form X,Y,Z - where X and Y are ground
 * coordinates, and Z is elevation.  The values may be separated either
 * by commas (X, Y, Z) or by whitespace (X Y Z).
 *
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromXYZ(const char *szFileName, bool progress_callback(int))
{
	// Free buffers to prepare to receive new data
	FreeData();

	FILE *fp = vtFileOpen(szFileName, "rb");
	if (!fp)
		return false;

	VTLOG("XYZ file: %s\n", szFileName);

	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	// first, test if we are comma or space delimited
	char firstline[256];
	if (fgets(firstline, 256, fp) == NULL)
		return false;
	float a, b, c, d;
	int count1 = sscanf(firstline, "%f,%f,%f,%f\n", &a, &b, &c, &d);
	int count2 = sscanf(firstline, "%f %f %f %f\n", &a, &b, &c, &d);

	const char *pattern;
	if (count1 > 1)
	{
		if (count1 == 3)
			pattern = "x,y,z";
		else if (count1 == 4)
			pattern = "n,x,y,z";
	}
	else if (count2 > 1)
	{
		if (count2 == 3)
			pattern = "x y z";
		else if (count2 == 4)
			pattern = "n x y z";
	}
	else
	{
		fclose(fp);
		return false;
	}
	bool success = LoadFromXYZ(fp, pattern, progress_callback);
	fclose(fp);
	return success;
}

bool vtElevationGrid::GetXYZLine(const char *buf, const char *pattern, const char *format,
								 int components, double *x, double *y, double *z)
{
	double val[4];
	if (sscanf(buf, format, &val[0], &val[1], &val[2], &val[3]) != components)
		return false;
	int v = 0;
	for (uint i = 0; i < strlen(pattern); i++)
	{
		char ch = pattern[i];
		switch (ch)
		{
		case 'n':
			v++;
			break;
		case 'x':
			*x = val[v++];
			break;
		case 'y':
			*y = val[v++];
			break;
		case 'z':
			*z = val[v++];
			break;
		}
	}
	return true;
}

bool vtElevationGrid::LoadFromXYZ(FILE *fp, const char *pattern, bool progress_callback(int))
{
	char buf[80];
	DRECT extents;
	extents.SetInsideOut();
	int iNum = 0;

	// Convert pattern ("n x y z") to fscanf format ("%lf %lf %lf %lf")
	char format[40];
	uint i, j = 0;
	int components = 0;
	for (i = 0; i < strlen(pattern); i++)
	{
		char ch = pattern[i];
		switch (ch)
		{
		case 'n':
		case 'x':
		case 'y':
		case 'z':
			format[j++] = '%';
			format[j++] = 'l';
			format[j++] = 'f';
			components++;
			break;
		default:
			format[j++] = pattern[i];
		}
	}
	format[j] = 0;	// terminate string

	// Look at the first two points
	rewind(fp);
	DPoint2 testp[2];
	bool bInteger = true;
	double x, y, z;
	for (i = 0; fgets(buf, 80, fp) != NULL && i < 2; i++)
	{
		VTLOG("Line %d: %s", i, buf);
		GetXYZLine(buf, pattern, format, components, &x, &y, &z);
		testp[i].Set(x, y);

		// Try to guess if the data is integer or floating point
		if ((int)z != z)
			bInteger = false;
	}

	// Compare them and decide whether we are row or column order
	bool bColumnOrder;
	DPoint2 diff = testp[1] - testp[0];
	if (diff.x == 0)
		bColumnOrder = true;
	else if (diff.y == 0)
		bColumnOrder = false;
	else
	{
		VTLOG("Can't determine if file is row-first or column-first.\n");
		fclose(fp);
		return false;
	}

	// Now make two passes over the file; the first time, we collect extents
	rewind(fp);
	int prog = 0;
	while (fgets(buf, 80, fp) != NULL)
	{
		if ((iNum%1000) == 0 && progress_callback != NULL)
		{
			prog++;
			if (progress_callback(prog % 100))
			{
				fclose(fp);		// Cancel
				return false;
			}
		}

		GetXYZLine(buf, pattern, format, components, &x, &y, &z);
		extents.GrowToContainPoint(DPoint2(x, y));
		iNum++;
	}

	// Depending on order, convert extents to grid dimensions
	int iColumns, iRows;
	if (bColumnOrder)
	{
		// column-first ordering
		double est = extents.Height() / fabs(diff.y);
		int rounded = (int) (est + 0.5);	// round to nearest
		iRows = rounded + 1;
		iColumns = iNum / iRows;
	}
	else
	{
		// row-first ordering
		double est = extents.Width() / fabs(diff.x);
		int rounded = (int) (est + 0.5);	// round to nearest
		iColumns = rounded + 1;
		iRows = iNum / iColumns;
	}

	// Create the grid, then go back and read all the points
	vtCRS unknown;
	Create(extents, IPoint2(iColumns, iRows), !bInteger, unknown);

	DPoint2 base(extents.left, extents.bottom);
	DPoint2 spacing = GetSpacing();

	rewind(fp);
	DPoint2 p;
	int xpos, ypos;
	i = 0;
	while (fgets(buf, 80, fp) != NULL)
	{
		if (progress_callback != NULL && ((i%100)==0))
		{
			if (progress_callback(i * 100 / iNum))
			{
				fclose(fp);		// Cancel
				return false;
			}
		}

		GetXYZLine(buf, pattern, format, components, &x, &y, &z);
		p.Set(x, y);
		xpos = (int) ((x - base.x) / spacing.x + 0.5);	// round to nearest
		ypos = (int) ((y - base.y) / spacing.y + 0.5);
		SetFValue(xpos, ypos, (float) z);
		i++;
	}
	fclose(fp);
	return true;
}


/**
 * Loads from an "HGT" file, which is the format used by the USGS SRTM
 * FTP site for their 1-degree blocks of SRTM data.
 * It is simply a raw block of signed 2-byte data, in WGS84 geographic coords.
 *
 * The file will either be:
 *	 2,884,802 bytes (for 3 arcsec, 1201*1201) or
 *	25,934,402 bytes (for 1 arcsec, 3601*3601)
 *
 * You should call SetupLocalCS() after loading if you will be doing
 * heightfield operations on this grid.
 *
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromHGT(const char *szFileName, bool progress_callback(int))
{
	// Free buffers to prepare to receive new data
	FreeData();

	// extract extents from the filename
	const char *fname = StartOfFilename(szFileName);
	if (!fname)
		return false;

	char ns, ew;
	int lat, lon;
	sscanf(fname, "%c%02d%c%03d", &ns, &lat, &ew, &lon);
	if (ns == 'S')
		lat = -lat;
	else if (ns != 'N')
		return false;
	if (ew == 'W')
		lon = -lon;
	else if (ew != 'E')
		return false;

	// Check file size to guess if it is 1 arcsec
	FILE *ffp = vtFileOpen(szFileName, "rb");
	if (!ffp)
		return false;
	fseek(ffp, 0, SEEK_END);
	bool b1arcsec = (ftell(ffp) > 3000000);
	fclose(ffp);

	vtCompressionReader reader;

	if (!reader.open(szFileName))
		return false;
	m_EarthExtents.SetRect(lon, lat+1, lon+1, lat);
	ComputeCornersFromExtents();

	// Projection is always geographic, integer
	m_crs.SetSimple(false, 0, EPSG_DATUM_WGS84);
	m_bFloatMode = false;

	if (b1arcsec)
		m_iSize.x = m_iSize.y = 3601;
	else
		m_iSize.x = m_iSize.y = 1201;
	if (!AllocateGrid())
		return false;

#define SWAP_2(x) ( (((x) & 0xff) << 8) | ((unsigned short)(x) >> 8) )

	short *buf = new short[m_iSize.x];
	short value;
	int i, j;
	for (j = 0; j < m_iSize.y; j++)
	{
		if (progress_callback != NULL)
			progress_callback(j * 100 / m_iSize.y);

		reader.read(buf, m_iSize.x * sizeof(short));
		for (i = 0; i < m_iSize.x; i++)
		{
			value = SWAP_2(buf[i]);
			SetValue(i, m_iSize.y-1-j, value);
		}
	}
	delete [] buf;
	reader.close();
	return true;
}


////////////////////////////////////////////////////
// Defines for STM writer

#define MAGIC_VALUE 0x04030201
typedef union {
	uint val;
	uchar bytes[4];
} bitTest;
#define SWAP(a,b)	{ a^=b; b^=a; a^=b; }
#define BYTE		uchar
#define BSWAP_W(w)	SWAP( (((BYTE *)&w)[0]), (((BYTE *)&w)[1]) )

/**
 * Write elevation to the STM (Simple Terrain Model) format created by Michael
 * Garland for his 'Scape' Terrain Simplification software.
 *
 * \returns \c true if the file was successfully opened and written.
 */
bool vtElevationGrid::SaveToSTM(const char *szFileName, bool progress_callback(int))
{
	FILE *outf = vtFileOpen(szFileName, "wb");
	if (!outf)
		return false;

	short *data = new short[m_iSize.x];

	bitTest test;
	test.val = MAGIC_VALUE;
	fprintf(outf, "STM %d %d %c%c%c%c", m_iSize.x, m_iSize.y,
		test.bytes[0], test.bytes[1], test.bytes[2], test.bytes[3]);
	fputc(0x0A, outf);

	for (int j = 0; j < m_iSize.y; j++)
	{
		if (progress_callback != NULL)
			progress_callback(j * 100 / m_iSize.y);
		for (int i = 0; i < m_iSize.x; i++)
		{
			/* This byte swap is only necessary for PC and Alpha machines */
			short val = (short) GetFValue(i, j);
			BSWAP_W(val);
			data[i] = val;
			data[i] += 32767;
		}
		fwrite(data, sizeof(short), m_iSize.x, outf);
	}
	delete [] data;
	fclose(outf);
	return true;
}

/**
 * Write elevation to the MSI Planet (Marconi) format.
 *
 * \returns \c true if the file was successfully opened and written.
 */
bool vtElevationGrid::SaveToPlanet(const char *szDirName, bool progress_callback(int))
{
	DRECT area;

	// create index file
	vtString fname_txt = szDirName;
	fname_txt += "/index.txt";
	FILE *indexfile = vtFileOpen(fname_txt, "wb");
	if (!indexfile)
		return false;

	const int TILE_SIZE = 2048;
	int xtiles = (m_iSize.x + TILE_SIZE - 1) / TILE_SIZE;
	int ytiles = (m_iSize.y + TILE_SIZE - 1) / TILE_SIZE;

	for (int xtile = 0; xtile < xtiles; xtile++)
	for (int ytile = 0; ytile < ytiles; ytile++)
	{
		// create name of binary file and write it to disk
		vtString fname_dat;
		fname_dat.Format("%s/binarydem%02d%02d.dat", szDirName, xtile, ytile);
		FILE *outfile = vtFileOpen(fname_dat, "wb");
		if (!outfile)
			return false;

		int xbase = xtile * TILE_SIZE;
		int ybase = ytile * TILE_SIZE;
		int xsize = (xtile < xtiles-1) ? TILE_SIZE : m_iSize.x-xbase;
		int ysize = (ytile < ytiles-1) ? TILE_SIZE : m_iSize.y-ybase;

		short* pixels = new short[xsize * ysize];
		int idx = 0;
		for (int j = 0; j < ysize; j++)
		{
			if ((j % 5) == 0 && progress_callback != NULL)
				progress_callback(j * 100 / m_iSize.y);

			int y = ybase + ysize - 1 - j;	// flip order within block
			for (int i = 0; i < xsize; i++)
			{
				int x = xbase + i;
				short val = (short) GetFValue(x, y);
				BSWAP_W(val);
				pixels[idx++] = val;
			}
		}
		fwrite(pixels, sizeof(short) * xsize, ysize, outfile);
		fclose(outfile);
		delete [] pixels;

		// Planet uses cell-edge, not cell-center for its extents.  That is,
		//  the area affected by the elevation grid points.
		area.left = m_EarthExtents.left + (xbase * m_dStep.x) - (m_dStep.x / 2.0f);
		area.right = area.left + (xsize * m_dStep.x);

		area.bottom = m_EarthExtents.bottom + (ybase * m_dStep.y) - (m_dStep.y / 2.0f);
		area.top = area.bottom + (ysize * m_dStep.y);

		vtString fname2 = StartOfFilename(fname_dat);

		fprintf(indexfile, "%s %g %g %g %g %g\n",
			(const char *) fname2,
			area.left,   area.right,
			area.bottom, area.top,
			m_dStep.x);		// apparently spacing must be even in both directions
	}
	fclose(indexfile);

	return true;
}


/**
 * Writes an Arc/Info compatible ASCII grid file.
 * Projection is written to a corresponding .prj file.
 *
 * \returns \c true if the file was successfully opened and written.
 */
bool vtElevationGrid::SaveToASC(const char *szFileName,
								bool progress_callback(int)) const
{
	FILE *fp = vtFileOpen(szFileName, "wb");
	if (!fp)
		return false;

	if (progress_callback != NULL) progress_callback(0);

	// write dimension IDs
	fprintf(fp, "ncols %d\n", m_iSize.x);
	fprintf(fp, "nrows %d\n", m_iSize.y);
	fprintf(fp, "xllcorner %lf\n", m_EarthExtents.left);
	fprintf(fp, "yllcorner %lf\n", m_EarthExtents.bottom);
	fprintf(fp, "cellsize %lf\n", m_dStep.x);

	// Try to decrease filesize by choosing a nodata value appropriately
	int nodata = INVALID_ELEVATION;
	if (m_fMinHeight > -9999)
		nodata = -9999;
	if (m_fMinHeight > -999)
		nodata = -999;
	if (m_fMinHeight > -99)
		nodata = -99;
	if (m_fMinHeight > 1)
		nodata = 0;
	fprintf(fp, "nodata_value %d\n", nodata);

	int i, j;
	float z;
	bool bWriteIntegers = (!m_bFloatMode && m_fVerticalScale == 1.0f);
	if (!bWriteIntegers)
	{
		// We might be able to write integers anyway, as long as our
		//  existing values are all actually integral.  It's worth taking
		//  a second to scan, as it makes a much smaller output file.
		bWriteIntegers = true;
		for (i = 0; i < m_iSize.y; i++)
		{
			for (j = 0; j < m_iSize.x; j++)
			{
				z = GetFValue(j, i);
				if (z == INVALID_ELEVATION)
					continue;
				if ((int)z*100 != (int)(z*100))
					bWriteIntegers= false;
			}
			if (!bWriteIntegers)
				break;
		}
	}
	// Now we can write the actual data
	for (i = 0; i < m_iSize.y; i++)
	{
		if (progress_callback != NULL)
			progress_callback(i*100/m_iSize.y);

		for (j = 0; j < m_iSize.x; j++)
		{
			z = GetFValue(j, m_iSize.y-1-i);
			if (z == INVALID_ELEVATION)
				fprintf(fp, " %d", nodata);
			else
			{
				if (bWriteIntegers)
					fprintf(fp, " %d", (int) z);
				else
					fprintf(fp, " %f", z);
			}
		}
		fprintf(fp, "\n");
	}
	fclose(fp);

	// There is no projection info in a ASC file, but we can create
	//  an accompanying .prj file, in the WKT-style .prj format.
	m_crs.WriteProjFile(szFileName);

	return true;
}

/**
 * Write elevation to a VRML file which contains a single ElevationGrid node.
 *
 * \returns \c true if the file was successfully opened and written.
 */
bool vtElevationGrid::SaveToVRML(const char *szFileName, bool progress_callback(int)) const
{
	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	FILE *fp = vtFileOpen(szFileName, "wb");
	if (!fp)
		return false;

	// Plain VRML can't handle real geographic coordinates, so it's basically
	//  a local coordinate system with 0,0 at the south-west corner.

	fprintf(fp, "#VRML V2.0 utf8\n");
	fprintf(fp, "# Written by Virtual Terrain Project software, build date %s\n", __DATE__);
	fprintf(fp, "# Conforms to ISO VRML 97 specification.\n");
	fprintf(fp, "# Please report problems to formats@vterrain.org\n");
	fprintf(fp, "\n");
	fprintf(fp, "Group\n");
	fprintf(fp, "{\n");
	fprintf(fp, "\tchildren\n");
	fprintf(fp, "\t[\n");
	fprintf(fp, "\t\tShape\n");
	fprintf(fp, "\t\t{\n");
	fprintf(fp, "\t\t\tappearance Appearance\n");
	fprintf(fp, "\t\t\t{\n");
	fprintf(fp, "\t\t\t\tmaterial Material { diffuseColor 0.800 0.800 0.800 }\n");
	fprintf(fp, "\t\t\t}\n");
	fprintf(fp, "\t\t\tgeometry ElevationGrid\n");
	fprintf(fp, "\t\t\t{\n");
	fprintf(fp, "\t\t\t	xDimension %d\n", m_iSize.x);
	fprintf(fp, "\t\t\t\txSpacing %f\n", m_fStep.x);
	fprintf(fp, "\t\t\t\tzDimension %d\n", m_iSize.y);
	fprintf(fp, "\t\t\t\tzSpacing %f\n", m_fStep.y);
	fprintf(fp, "\t\t\t\tsolid FALSE\n");
	fprintf(fp, "\t\t\t\theight\n");
	fprintf(fp, "\t\t\t\t[\n");

	// what does VRML use for NODATA - presuming it even supports it?
	float nodata = INVALID_ELEVATION;

	int i, j;
	float z;
	bool bWriteIntegers = (!m_bFloatMode && m_fVerticalScale == 1.0f);
	if (!bWriteIntegers)
	{
		// We might be able to write integers anyway, as long as our
		//  existing values are all actually integral.  It's worth taking
		//  a second to scan, as it makes a much smaller output file.
		bWriteIntegers = true;
		for (i = 0; i < m_iSize.y; i++)
		{
			for (j = 0; j < m_iSize.x; j++)
			{
				z = GetFValue(j, i);
				if (z == INVALID_ELEVATION)
					continue;
				if ((int)z*100 != (int)(z*100))
					bWriteIntegers= false;
			}
			if (!bWriteIntegers)
				break;
		}
	}
	// Now we can write the actual data
	for (i = 0; i < m_iSize.y; i++)
	{
		if (progress_callback != NULL)
			progress_callback(i*100/m_iSize.y);

		for (j = 0; j < m_iSize.x; j++)
		{
			if (j > 0 && (j%20) == 0)
				fprintf(fp, "\n");

			z = GetFValue(j, m_iSize.y-1-i);
			if (z == INVALID_ELEVATION)
				fprintf(fp, " %d", (int) nodata);
			else
			{
				if (bWriteIntegers)
					fprintf(fp, " %d", (int) z);
				else
					fprintf(fp, " %.3f", z);
			}
		}
		fprintf(fp, "\n");
	}

	fprintf(fp, "\t\t\t\t] # height\n");
	fprintf(fp, "\t\t\t} # geometry ElevationGrid\n");
	fprintf(fp, "\t\t} # ElevationGrid Shape\n");
	fprintf(fp, "\t] # children\n");
	fprintf(fp, "} # Group\n");

	fclose(fp);

	return true;
}

/**
 * Write elevation to a text file which contains an XYZ for each point.
 *
 * \returns \c true if the file was successfully opened and written.
 */
bool vtElevationGrid::SaveToXYZ(const char *szFileName, bool progress_callback(int)) const
{
	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	FILE *fp = vtFileOpen(szFileName, "wb");
	if (!fp)
		return false;

	bool bGeo = (m_crs.IsGeographic() != 0);

	DPoint3 loc;
	for (int i = 0; i < m_iSize.x; i++)
	{
		for (int j = 0; j < m_iSize.y; j++)
		{
			GetEarthLocation(i, j, loc);
			if (bGeo)
				fprintf(fp, "%.9f %.9f %.3f\n", loc.x, loc.y, loc.z);
			else
				fprintf(fp, "%.6f %.6f %.6f\n", loc.x, loc.y, loc.z);
		}
	}
	fclose(fp);

	return true;
}

/**
 * Write elevation to a .raw file along with an .inf file which describes
 * the data for the purpose of exporting to MS Flight Simulator 2004 via
 * their MSFS SDK 'resample' utility.
 *
 * \param szFileName Should end in ".raw"
 * \param progress_callback If supplied, this function will be called back
 *		with a value of 0 to 100 as the operation progresses.
 * \returns \c true if the file was successfully written.
 */
bool vtElevationGrid::SaveToRAWINF(const char *szFileName, bool progress_callback(int)) const
{
	// First, write the raw data
	FILE *fp = vtFileOpen(szFileName, "wb");
	if (!fp)
		return false;
	int i, j;
	for (j = 0; j < m_iSize.y; j++)
	{
		if (progress_callback != NULL)
			progress_callback(j * 100 / m_iSize.y);
		for (i = 0; i < m_iSize.x; i++)
		{
			float val = GetFValue(i, m_iSize.y-1-j);
			short s = (short) val;
			fwrite(&s, 1, sizeof(short), fp);
		}
	}
	fclose(fp);

	vtString fname = szFileName;
	RemoveFileExtensions(fname);
	const char *fullname = fname;
	const char *name = StartOfFilename(fullname);
	int pathlen = (name - fullname);
	vtString path = fname.Left(pathlen);

	vtString inf_fname = ChangeFileExtension(szFileName, ".inf");
	fp = vtFileOpen(inf_fname, "wb");
	if (!fp)
		return false;

	fprintf(fp, "[Destination]\n");
	fprintf(fp, "  LOD = Auto\n");
	fprintf(fp, "  DestDir = \".\"\n");
	fprintf(fp, "  DestBaseFileName = \"%s\"\n", name);
	fprintf(fp, "  UseSourceDimensions = 1\n");
	fprintf(fp, "[Source]\n");
	fprintf(fp, "  Type              = ElevS16LSB\n");
	fprintf(fp, "  SourceDir         = \"%s\"\n", (const char *) path);
	fprintf(fp, "  SourceFile        = \"%s.raw\"\n", name);
	// The latitude of the northwest corner of the bounding area.
	fprintf(fp, "  Lat               = %lf\n", m_EarthExtents.top);
	// The longitude of the northwest corner of the bounding area.
	fprintf(fp, "  Lon               = %lf\n", m_EarthExtents.left);
	fprintf(fp, "  NumOfCellsPerLine = %d\n", m_iSize.x);
	fprintf(fp, "  NumOfLines        = %d\n", m_iSize.y);
	fprintf(fp, "  CellXdimensionDeg = %.16lf\n", m_EarthExtents.Width() / m_iSize.x);
	fprintf(fp, "  CellYdimensionDeg = %.16lf\n", m_EarthExtents.Height() / m_iSize.y);
	fprintf(fp, "  ScaleinMeters     = 1.0\n");
	fclose(fp);

	return true;
}

/**
 * Write elevation to a .raw file, the way Unity import expects it:
 * Scaled to 8 bits (0-255).
 */
bool vtElevationGrid::SaveToRAW_Unity(const char *szFileName, bool progress_callback(int)) const
{
	float fMin, fMax;
	GetHeightExtents(fMin, fMax);
	const float fRange = fMax - fMin;

	// First, write the raw data
	FILE *fp = vtFileOpen(szFileName, "wb");
	if (!fp)
		return false;
	int i, j;
	for (j = 0; j < m_iSize.y; j++)
	{
		if (progress_callback != NULL)
			progress_callback(j * 100 / m_iSize.y);
		for (i = 0; i < m_iSize.x; i++)
		{
			float val = GetFValue(i, j);
			uchar s = (uchar) ((val - fMin) / fRange * 255);
			fwrite(&s, 1, sizeof(uchar), fp);
		}
	}
	fclose(fp);
	return true;
}

/**
 * Write elevation a 16-bit greyscale PNG file.
 */
bool vtElevationGrid::SaveToPNG16(const char *fname)
{
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;

	/* open the file */
	fp = vtFileOpen(fname, "wb");
	if (fp == NULL)
		return false;

	/* Create and initialize the png_struct with the desired error handler
	 * functions.  We also check that the library version is compatible with
	 * the one used at compile time, in case we are using dynamically linked
	 * libraries.  REQUIRED.
	 */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL);

	if (png_ptr == NULL)
	{
		fclose(fp);
		return false;
	}

	/* Allocate/initialize the image information data.  REQUIRED */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		fclose(fp);
		png_destroy_write_struct(&png_ptr,  png_infopp_NULL);
		return false;
	}

	/* Set error handling.  REQUIRED if you aren't supplying your own
	 * error handling functions in the png_create_write_struct() call.
	 */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* If we get here, we had a problem writing the file */
		fclose(fp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return false;
	}

	/* set up the output control if you are using standard C streams */
	png_init_io(png_ptr, fp);

	/* Set the image information here. REQUIRED */
	int color_type = PNG_COLOR_TYPE_GRAY;
	int png_bit_depth = 16;

	png_set_IHDR(png_ptr, info_ptr, m_iSize.x, m_iSize.y, png_bit_depth, color_type,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	/* Write the file header information.  REQUIRED */
	png_write_info(png_ptr, info_ptr);

	/* Write the image. */
	png_uint_32 k, height = m_iSize.y, width = m_iSize.x;
	png_byte *image = (png_byte *)malloc(height * width * 2);
	png_bytep *row_pointers = (png_bytep *)malloc(height * sizeof(png_bytep *));
	for (k = 0; k < height; k++)
		row_pointers[k] = image + k*width*2;

	short val;
	uint8_t *adr = (uint8_t *) &val;
	uint row, col;
	for (row = 0; row < height; row++)
	{
		png_bytep pngptr = row_pointers[row];
		for (col = 0; col < width; col++)
		{
			val = (short) GetFValue(col, height-1-row);
			*pngptr++ = adr[0];
			*pngptr++ = adr[1];
		}
	}

	/* write out the entire image data in one call */
	png_write_image(png_ptr, row_pointers);

	/* It is REQUIRED to call this to finish writing the rest of the file */
	png_write_end(png_ptr, info_ptr);

	free(image);
	free(row_pointers);

	/* clean up after the write, and free any memory allocated */
	png_destroy_write_struct(&png_ptr, &info_ptr);

	/* close the file */
	fclose(fp);

	/* that's it */
	return true;
}

