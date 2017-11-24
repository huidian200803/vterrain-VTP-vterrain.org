//
// vtElevationGridBT.cpp
//
// This modules contains the implementations of the BT file I/O methods of
// the class vtElevationGrid.
//
// Copyright (c) 2001-2013 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#include "ElevationGrid.h"
#include "ByteOrder.h"
#include "vtdata/vtLog.h"
#include "vtdata/FilePath.h"


/** Loads just the header information from a BT (Binary Terrain) file.
 * \par
 * This can be useful if you want to check the information such as the
 * size of the data, without reading the entire file.
 * This method works whether it is given a normal BT file, or one which
 * has been compressed with gzip.
 * \returns \c true if the header was successfully parsed.
 */
bool vtElevationGrid::LoadBTHeader(const char *szFileName, vtElevError *err)
{
	// The gz functions (gzopen etc.) behave exactly like the stdlib
	//  functions (fopen etc.) in the case where the input file is not in
	//  gzip format, so we can simply use them without worry.
	gzFile fp = vtGZOpen(szFileName, "rb");
	if (!fp)
	{
		SetError(err, vtElevError::FILE_OPEN, "Couldn't open file '%s'", szFileName);
		return false;
	}

	char buf[11];
	gzread(fp, buf, 10);
	buf[10] = '\0';

	if (strncmp(buf, "binterr", 7))
	{
		gzclose(fp);
		SetError(err, vtElevError::NOT_FORMAT, "Not a BT file");
		return false;		// Not a current BT file
	}

	// Get version
	int iMajor = (int) buf[7]-'0';
	int iMinor = (int) buf[9]-'0';

	VTLOG("BT header: version %d.%d", iMajor, iMinor);

	// NOTE:  BT format is little-endian
	GZFRead(&m_iSize.y, DT_INT, 1, fp, BO_LITTLE_ENDIAN);
	GZFRead(&m_iSize.x,	 DT_INT, 1, fp, BO_LITTLE_ENDIAN);

	// Default to internal projection
	short external = 0;

	DRECT extents;
	short svalue, proj_type, zone, datum;
	int ivalue;
	float ftmp;
	if (iMajor == 1 && iMinor == 0)		// Version 1.0
	{
		// data size
		GZFRead(&ivalue, DT_INT, 1, fp, BO_LITTLE_ENDIAN);

		// UTM flag
		GZFRead(&svalue, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);
		proj_type = (svalue == 1);

		// UTM zone
		GZFRead(&zone, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);

		// 1.0 didn't support Datum, so assume WGS84
		datum = EPSG_DATUM_WGS84;

		// coordinate extents left-right
		GZFRead(&ftmp, DT_FLOAT, 1, fp, BO_LITTLE_ENDIAN);
		extents.left = ftmp;
		GZFRead(&ftmp, DT_FLOAT, 1, fp, BO_LITTLE_ENDIAN);
		extents.right = ftmp;

		// coordinate extents bottom-top
		GZFRead(&ftmp, DT_FLOAT, 1, fp, BO_LITTLE_ENDIAN);
		extents.bottom = ftmp;
		GZFRead(&ftmp, DT_FLOAT, 1, fp, BO_LITTLE_ENDIAN);
		extents.top = ftmp;

		// is the data floating point or integers?
		GZFRead(&m_bFloatMode, DT_INT, 1, fp, BO_LITTLE_ENDIAN);
		if (m_bFloatMode != true)
			m_bFloatMode = false;
	}
	else if (iMajor == 1 && (iMinor == 1 || iMinor == 2 || iMinor == 3))
	{
		// Version 1.1, 1.2, or 1.3

		// data size
		GZFRead(&svalue, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);

		// Is floating point data?
		GZFRead(&svalue, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);
		m_bFloatMode = (svalue == 1);

		// Projection (0 = geo, 1 = utm, 2 = feet, 3 = u.s. feet)
		GZFRead(&proj_type, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);

		// UTM zone (ignore unless projection == 1)
		GZFRead(&zone, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);

		// Datum (ignore unless projection == 0 or 1)
		GZFRead(&datum, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);

		// coordinate extents
		GZFRead(&extents.left, DT_DOUBLE, 1, fp, BO_LITTLE_ENDIAN);
		GZFRead(&extents.right, DT_DOUBLE, 1, fp, BO_LITTLE_ENDIAN);
		GZFRead(&extents.bottom, DT_DOUBLE, 1, fp, BO_LITTLE_ENDIAN);
		GZFRead(&extents.top, DT_DOUBLE, 1, fp, BO_LITTLE_ENDIAN);

		// External projection flag
		GZFRead(&external, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);

		// 1.3 adds the vertical scale field
		if (iMajor == 1 && iMinor == 3)
			GZFRead(&m_fVMeters, DT_FLOAT, 1, fp, BO_LITTLE_ENDIAN);
	}
	else
	{
		SetError(err, vtElevError::UNSUPPORTED_VERSION, "Unsupported BT version %d.%d",
			iMajor, iMinor);
		return false;
	}

	gzclose(fp);

	SetEarthExtents(extents);

	VTLOG(", float %d, CRS external %d", m_bFloatMode, external);

	// Set up CRS.
	if (external == 1)
	{
		// Read external projection (.prj) file.
		if (!m_crs.ReadProjFile(szFileName))
		{
			SetError(err, vtElevError::READ_CRS, "Couldn't read CRS from file '%s'", szFileName);
			return false;
		}
	}
	else
	{
		// Internal specification: proj_type 0 = Geo, 1 = UTM
		VTLOG(" (type %d, zone %d, datum %d)", proj_type, zone, datum);
		m_crs.SetSimple(proj_type == 1, zone, datum);
	}

	ComputeCornersFromExtents();
	VTLOG("\n");

	return true;
}


/** Loads from a BT (Binary Terrain) file.
 * \par
 * Both the current version (1.1) and older BT versions are supported.
 * This method works whether it is given a normal BT file, or one which
 * has been compressed with gzip.
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromBT(const char *szFileName, bool progress_callback(int),
								 vtElevError *err)
{
	// Free buffers to prepare to receive new data
	FreeData();

	// First load the header
	if (!LoadBTHeader(szFileName, err))
		return false;

	if (!LoadBTData(szFileName, progress_callback, err))
		return false;

	return true;
}

bool vtElevationGrid::LoadBTData(const char *szFileName, bool progress_callback(int),
								 vtElevError *err)
{
	gzFile fp = vtGZOpen(szFileName, "rb");
	if (!fp)
	{
		SetError(err, vtElevError::FILE_OPEN, "Couldn't open file '%s'", szFileName);
		return false;
	}

	// elevation data always starts at offset 256
	gzseek(fp, 256, SEEK_SET);

	if (!AllocateGrid(err))
	{
		gzclose(fp);
		return false;
	}

	int i;
#if 0
	// slow way
	int value;
	float fvalue;
	for (i = 0; i < m_iSize.y; i++)
	{
		if (progress_callback != NULL) progress_callback(i * 100 / m_iSize.y);
		for (j = 0; j < m_iSize.x; j++)
		{
			if (m_bFloatMode)
			{
				GZFRead(&fvalue, DT_FLOAT, 1, fp, BO_LITTLE_ENDIAN);
				SetFValue(i, j, fvalue);
			}
			else
			{
				GZFRead(&svalue, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);
				SetValue(i, j, svalue);
			}
		}
	}
#else
	// fast way
	if (m_bFloatMode)
	{
		for (i = 0; i < m_iSize.y; i++)
		{
			if (progress_callback != NULL && ((i%40) == 0))
			{
				if (progress_callback(i * 100 / m_iSize.y))
				{
					// Cancel
					SetError(err, vtElevError::CANCELLED, "Cancelled loading '%s'", szFileName);
					gzclose(fp);
					return false;
				}
			}
			int nitems = GZFRead(m_pFData + (i*m_iSize.x), DT_FLOAT, m_iSize.x,
				fp, BO_LITTLE_ENDIAN);
			if (nitems != m_iSize.x)
			{
				SetError(err, vtElevError::READ_DATA, "Error reading data from file '%s'", szFileName);
				gzclose(fp);
				return false;
			}
		}
	}
	else
	{
		for (i = 0; i < m_iSize.y; i++)
		{
			if (progress_callback != NULL && ((i%40) == 0))
			{
				if (progress_callback(i * 100 / m_iSize.y))
				{
					// Cancel
					SetError(err, vtElevError::CANCELLED, "Cancelled loading '%s'", szFileName);
					gzclose(fp);
					return false;
				}
			}
			int nitems = GZFRead(m_pData + (i*m_iSize.x), DT_SHORT, m_iSize.x,
				fp, BO_LITTLE_ENDIAN);
			if (nitems != m_iSize.x)
			{
				SetError(err, vtElevError::READ_DATA, "Error reading data from file '%s'", szFileName);
				gzclose(fp);
				return false;
			}
		}
	}
#endif

	// be sure to close the file!
	gzclose(fp);

	ComputeHeightExtents();
	return true;
}


/**
 * Writes the grid to a BT (Binary Terrain) file.
 * The current BT format version (1.3) is written.
 *
 * \param szFileName		The file name to write to.
 * \param progress_callback If supplied, this function will be called back
 *				with a value of 0 to 100 as the operation progresses.
 * \param bGZip			If true, the data will be compressed with gzip.
 *				If true, you should Use a filename ending with ".gz".
 */
bool vtElevationGrid::SaveToBT(const char *szFileName,
							   bool progress_callback(int), bool bGZip)
{
	int w = m_iSize.y;
	int h = m_iSize.x;
	short zone = (short) m_crs.GetUTMZone();
	short datum = (short) m_crs.GetDatum();
	short isfloat = (short) IsFloatMode();
	short external = 1;		// always true: we always write an external .prj file

	LinearUnits units = m_crs.GetUnits();
	int hunits = (int) units;

	// Latest header, version 1.2
	short datasize = m_bFloatMode ? 4 : 2;
	DataType datatype = m_bFloatMode ? DT_FLOAT : DT_SHORT;

	if (bGZip == false)
	{
		// Use conventional IO
		FILE *fp = vtFileOpen(szFileName, "wb");
		if (!fp)
			return false;

		fwrite("binterr1.3", 10, 1, fp);
		FWrite(&w, DT_INT, 1, fp, BO_LITTLE_ENDIAN);
		FWrite(&h, DT_INT, 1, fp, BO_LITTLE_ENDIAN);
		FWrite(&datasize, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);
		FWrite(&isfloat, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);
		FWrite(&hunits,	DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);	// Horizontal Units (0, 1, 2, 3)
		FWrite(&zone,	DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);		// UTM zone
		FWrite(&datum,	DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);	// Datum

		// coordinate extents
		FWrite(&m_EarthExtents.left,	DT_DOUBLE, 1, fp, BO_LITTLE_ENDIAN);
		FWrite(&m_EarthExtents.right,	DT_DOUBLE, 1, fp, BO_LITTLE_ENDIAN);
		FWrite(&m_EarthExtents.bottom,	DT_DOUBLE, 1, fp, BO_LITTLE_ENDIAN);
		FWrite(&m_EarthExtents.top,		DT_DOUBLE, 1, fp, BO_LITTLE_ENDIAN);

		FWrite(&external, DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);		// External projection specification
		FWrite(&m_fVMeters, DT_FLOAT, 1, fp, BO_LITTLE_ENDIAN);	// Vertical scale factor (meters/units)

		// now write the data: always starts at offset 256
		fseek(fp, 256, SEEK_SET);

#if 0
		// slow way, one heixel at a time
		for (int i = 0; i < w; i++)
		{
			if (progress_callback != NULL) progress_callback(i * 100 / w);
			for (j = 0; j < h; j++)
			{
				if (m_bFloatMode) {
					fvalue = GetFValue(i, j);
					FWrite(&fvalue, datatype, 1, fp, BO_LITTLE_ENDIAN);
				} else {
					svalue = GetValue(i, j);
					FWrite(&svalue, datatype, 1, fp, BO_LITTLE_ENDIAN);
				}
			}
		}
#else
		// fast way, with the assumption that the data is stored column-first in memory
		if (m_bFloatMode)
		{
			for (int i = 0; i < w; i++)
			{
				if (progress_callback != NULL)
				{
					if (progress_callback(i * 100 / w))
					{ fclose(fp); return false; }
				}
				FWrite(m_pFData + (i * m_iSize.x), DT_FLOAT, m_iSize.x, fp, BO_LITTLE_ENDIAN);
			}
		}
		else
		{
			for (int i = 0; i < w; i++)
			{
				if (progress_callback != NULL)
				{
					if (progress_callback(i * 100 / w))
					{ fclose(fp); return false; }
				}
				FWrite(m_pData + (i * m_iSize.x), DT_SHORT, m_iSize.x, fp, BO_LITTLE_ENDIAN);
			}
		}
#endif
		fclose(fp);
	}
	else
	{
		// Use GZip IO
		gzFile fp = vtGZOpen(szFileName, "wb");
		if (!fp)
			return false;

		gzwrite(fp, (void *)"binterr1.3", 10);
		GZFWrite(&w, DT_INT, 1, fp, BO_LITTLE_ENDIAN);
		GZFWrite(&h, DT_INT, 1, fp, BO_LITTLE_ENDIAN);
		GZFWrite(&datasize,	DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);
		GZFWrite(&isfloat,	DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);
		GZFWrite(&hunits,	DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);		// Horizontal Units (0, 1, 2, 3)
		GZFWrite(&zone,		DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);		// UTM zone
		GZFWrite(&datum,	DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);		// Datum

		// coordinate extents
		GZFWrite(&m_EarthExtents.left,		DT_DOUBLE, 1, fp, BO_LITTLE_ENDIAN);
		GZFWrite(&m_EarthExtents.right,		DT_DOUBLE, 1, fp, BO_LITTLE_ENDIAN);
		GZFWrite(&m_EarthExtents.bottom,	DT_DOUBLE, 1, fp, BO_LITTLE_ENDIAN);
		GZFWrite(&m_EarthExtents.top,		DT_DOUBLE, 1, fp, BO_LITTLE_ENDIAN);

		GZFWrite(&external,		DT_SHORT, 1, fp, BO_LITTLE_ENDIAN);	// External projection specification
		GZFWrite(&m_fVMeters,	DT_FLOAT, 1, fp, BO_LITTLE_ENDIAN);	// Vertical scale factor (meters/units)

		// now write the data: always starts at offset 256
		gzseek(fp, 256, SEEK_SET);

		// fast way, with the assumption that the data is stored column-first in memory
		if (m_bFloatMode)
		{
			for (int i = 0; i < w; i++)
			{
				if (progress_callback != NULL)
				{
					if (progress_callback(i * 100 / w))
					{ gzclose(fp); return false; }
				}
				GZFWrite(m_pFData + (i * m_iSize.x), DT_FLOAT, m_iSize.x, fp, BO_LITTLE_ENDIAN);
			}
		}
		else
		{
			for (int i = 0; i < w; i++)
			{
				if (progress_callback != NULL)
				{
					if (progress_callback(i * 100 / w))
					{ gzclose(fp); return false; }
				}
				GZFWrite(m_pData + (i * m_iSize.x), DT_SHORT, m_iSize.x, fp, BO_LITTLE_ENDIAN);
			}
		}
		gzclose(fp);
	}

	if (external)
	{
		// Write external projection file (.prj)
		char prj_name[256];
		strcpy(prj_name, szFileName);
		int len = strlen(prj_name);
		if (bGZip)
			strcpy(prj_name + len - 6, ".prj"); // overwrite the .bt.gz
		else
			strcpy(prj_name + len - 3, ".prj"); // overwrite the .bt
		m_crs.WriteProjFile(prj_name);
	}

	return true;
}

