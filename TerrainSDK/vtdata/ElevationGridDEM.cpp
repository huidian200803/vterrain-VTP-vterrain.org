//
// vtElevationGridDEM.cpp
//
// This modules contains the implementations of the DEM file I/O methods of
// the class vtElevationGrid.
//
// Copyright (c) 2001-2013 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#include "ElevationGrid.h"
#include "vtLog.h"
#include "FilePath.h"
#include <algorithm>	// for std::min/max

#if VTDEBUG
#define DEBUG_DEM true
#else
#define DEBUG_DEM false
#endif

// ************** DConvert - DEM Helper function ****************

bool DConvert(FILE *fp, int length, double &value, bool bDebug=false)
{
	char szCharString[64];

	if (fread(szCharString, length, 1, fp) != 1)
		return false;
	szCharString[length] = 0;

	// Microsoft's C standard library documentation says that the exponent in
	//  an atof() call can be {d,D,e,E}.   But Unix and MacOSX don't like {d,D}.
	// And as of version 2015, MSVC wants "E" also. So, we change it.
	for (int i=0; i < length; i++)
		if (szCharString[i] == 'D' || szCharString[i] == 'd')
			szCharString[i] = 'E';

	value = atof(szCharString);

	if (bDebug)
		VTLOG(" DConvert '%s' -> %lf\n", szCharString, value);

	return true;
}

bool IConvert(FILE *fp, int length, int &value)
{
	char szCharString[64];

	if (fread(szCharString, length, 1, fp) != 1)
		return false;
	szCharString[length] = 0;

	value = atoi(szCharString);
	return true;
}

/**
 * Loads elevation from a USGS DEM file.
 *
 * Some non-standard variations of the DEM format are supported.
 *
 * You should call SetupLocalCS() after loading if you will be doing
 * heightfield operations on this grid.
 *
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevationGrid::LoadFromDEM(const char *szFileName,
								  bool progress_callback(int), vtElevError *err)
{
	// Free buffers to prepare to receive new data
	FreeData();

	if (progress_callback != NULL) progress_callback(0);

	FILE *fp = vtFileOpen(szFileName,"rb");
	if (!fp)		// Cannot Open File
	{
		SetError(err, vtElevError::FILE_OPEN, "Couldn't open file '%s'", szFileName);
		return false;
	}

	// check for version of DEM format
	int		iRow, iColumn;
	char buffer[158];

	fseek(fp, 864, 0);
	if (fread(buffer, 144, 1, fp) != 1)
	{
		SetError(err, vtElevError::READ_DATA, "Couldn't read DEM data from '%s'", szFileName);
		return false;
	}
	bool bOldFormat = (strncmp(buffer, "     1     1", 12) == 0);
	bool bNewFormat = false;
	bool bFixedLength = true;
	int  iDataStartOffset = 1024;	// set here to avoid compiler warning
	int  i, j;

	if (bOldFormat)
		iDataStartOffset = 1024;	// 1024 is record length
	else
	{
		fseek(fp, 1024, 0);		// Check for New Format
		IConvert(fp, 6, iRow);
		IConvert(fp, 6, iColumn);
		if (iRow==1 && iColumn==1)	// File OK?
		{
			bNewFormat = true;
			iDataStartOffset = 1024;
		}
		else
		{
			// might be the Non-fixed-length record format
			// Record B can start anywhere from 865 to 1023
			// Record B is identified by starting with the row/column
			//  of its first profile, "     1     1"
			fseek(fp, 865, 0);
			if (fread(buffer, 158, 1, fp) != 1)
			{
				SetError(err, vtElevError::READ_DATA, "Couldn't read DEM data from '%s'", szFileName);
				fclose(fp);
				return false;
			}
			for (i = 0; i < 158-12; i++)
			{
				if (!strncmp(buffer+i, "     1     1", 12))
				{
					// Found it
					bFixedLength = false;
					iDataStartOffset = 865+i;
					break;
				}
			}
			if (i == 158-12)
			{
				// Not a DEM file
				SetError(err, vtElevError::READ_DATA, "Couldn't read DEM data from '%s'", szFileName);
				fclose(fp);
				return false;
			}
		}
	}

	// Read the embedded DEM name
	char szName[41];
	fseek(fp, 0, 0);
	if (fgets(szName, 41, fp) == NULL)
		return false;
	int len = strlen(szName);	// trim trailing whitespace
	while (len > 0 && szName[len-1] == ' ')
	{
		szName[len-1] = 0;
		len--;
	}
	m_strOriginalDEMName = szName;

	fseek(fp, 156, 0);
	int iCoordSystem, iUTMZone;
	IConvert(fp, 6, iCoordSystem);
	IConvert(fp, 6, iUTMZone);

	fseek(fp, 168, 0);
	double dProjParams[15];
	for (i = 0; i < 15; i++)
	{
		if (!DConvert(fp, 24, dProjParams[i], DEBUG_DEM))
			return false;
	}

	int iDatum = EPSG_DATUM_NAD27;	// default

	// OLD format header ends at byte 864 (0x360); new format has Datum
	if (bNewFormat)
	{
		// year of data compilation
		char szDateBuffer[5];
		fseek(fp, 876, 0);		// 0x36C
		if (fread(szDateBuffer, 4, 1, fp) != 1)
			return false;
		szDateBuffer[4] = 0;

		// Horizontal datum
		// 1=North American Datum 1927 (NAD 27)
		// 2=World Geodetic System 1972 (WGS 72)
		// 3=WGS 84
		// 4=NAD 83
		// 5=Old Hawaii Datum
		// 6=Puerto Rico Datum
		fseek(fp, 890, 0);	// 0x37A
		int datum;
		IConvert(fp, 2, datum);
		VTLOG("DEM Reader: Read Datum Value %d\n", datum);
		switch (datum)
		{
			case 1: iDatum = EPSG_DATUM_NAD27; break;
			case 2: iDatum = EPSG_DATUM_WGS72; break;
			case 3: iDatum = EPSG_DATUM_WGS84; break;
			case 4:	iDatum = EPSG_DATUM_NAD83; break;
			case 5: iDatum = EPSG_DATUM_OLD_HAWAIIAN; break;
			case 6: iDatum = EPSG_DATUM_PUERTO_RICO; break;
		}
	}

	fseek(fp, 528, 0);
	int iGUnit, iVUnit;
	IConvert(fp, 6, iGUnit);
	IConvert(fp, 6, iVUnit);

	// Ground (Horizontal) Units in meters
	double	fGMeters;
	switch (iGUnit)
	{
	case 0: fGMeters = 1.0;		break;	// 0 = radians (never encountered)
	case 1: fGMeters = 0.3048;	break;	// 1 = feet
	case 2: fGMeters = 1.0;		break;	// 2 = meters
	case 3: fGMeters = 30.922;	break;	// 3 = arc-seconds
	}

	// Vertical Units in meters
	double	fVertUnits;
	switch (iVUnit)
	{
	case 1:  fVertUnits = 0.3048;  break;	// feet to meter conversion
	case 2:  fVertUnits = 1.0;	   break;	// meters == meters
	default: fVertUnits = 1.0;	   break;	// anything else, assume meters
	}

	fseek(fp, 816, 0);
	double dxdelta, dydelta, dzdelta;
	DConvert(fp, 12, dxdelta, DEBUG_DEM);	// dxdelta (unused)
	DConvert(fp, 12, dydelta, DEBUG_DEM);
	DConvert(fp, 12, dzdelta, DEBUG_DEM);

	m_bFloatMode = false;

	// Read the coordinates of the 4 corners
	VTLOG("DEM corners:\n");
	DPoint2	corners[4];			// SW, NW, NE, SE
	fseek(fp, 546, 0);
	for (i = 0; i < 4; i++)
	{
		DConvert(fp, 24, corners[i].x, DEBUG_DEM);
		DConvert(fp, 24, corners[i].y, DEBUG_DEM);
	}
	for (i = 0; i < 4; i++)
		VTLOG(" (%lf, %lf)", corners[i].x, corners[i].y);
	VTLOG("\n");

	// Set up the CRS and corners
	bool bGeographic = (iCoordSystem == 0);
	if (bGeographic)
	{
		for (i = 0; i < 4; i++)
		{
			// convert arcseconds to degrees
			m_Corners[i].x = corners[i].x / 3600.0;
			m_Corners[i].y = corners[i].y / 3600.0;
		}
	}
	else
	{
		// some linear coordinate system
		for (i = 0; i < 4; i++)
			m_Corners[i] = corners[i];
	}

	// Special case.  Some old DEMs claim to be NAD27, but they are of Hawai'i,
	//  and Hawai'i has no NAD27, it is actually OHD.
	if (iDatum == EPSG_DATUM_NAD27)
	{
		DRECT Hawaii(0,0,0,0);
		if (bGeographic)
			Hawaii.SetRect(-164, 24, -152, 17);
		else if (iCoordSystem == 1)	// UTM
		{
			if (iUTMZone == 4)
				Hawaii.SetRect(240000, 2600000, 1000000, 2000000);
			else if (iUTMZone == 5)
				Hawaii.SetRect(-400000, 2600000, 400000, 2000000);
		}
		for (i = 0; i < 4; i++)
		{
			if (Hawaii.ContainsPoint(m_Corners[i]))
				iDatum = EPSG_DATUM_OLD_HAWAIIAN;
		}
	}

	bool bSuccessfulCRS = true;
	switch (iCoordSystem)
	{
	case 0:		// geographic (lat-lon)
		iUTMZone = -1;
		bSuccessfulCRS = m_crs.SetSimple(false, iUTMZone, iDatum);
		break;
	case 1:		// utm
		m_crs.SetSimple(true, iUTMZone, iDatum);
		break;
	case 3:		// Albers Conical Equal Area
		{
			// The Official DEM documentation says:
			// "Note: All angles (latitudes, longitudes, or azimuth) are
			// required in degrees, minutes, and arc seconds in the packed
			// real number format +DDDOMMOSS.SSSSS."
			// However, what i've actually seen is values like:
			//  0.420000000000000D+06' -> 420000
			// for 42 degrees, which is off by a decimal point from the DEM docs.
			// So, intepret the values with factor of 10000 to convert to degrees:

			// double semi_major = dProjParams[0];		// unused
			// double eccentricity = dProjParams[1];	// unused
			double lat_1st_std_parallel = dProjParams[2] / 10000;
			double lat_2nd_std_parallel = dProjParams[3] / 10000;
			double lon_central_meridian = dProjParams[4] / 10000;
			double lat_origin = dProjParams[5] / 10000;
			double false_easting = dProjParams[6];
			double false_northing = dProjParams[7];

			m_crs.SetGeogCSFromDatum(iDatum);
			m_crs.SetACEA(lat_1st_std_parallel, lat_2nd_std_parallel, lat_origin,
				lon_central_meridian, false_easting, false_northing);
		}
		break;
	case 2:		// State Plane (!)
	case 4:		// Lambert Conformal
	case 5:		// Mercator
	case 6:		// Polar Stereographic
	case 7:		// Polyconic
	case 8:		// Equidistant Conic Type A / B
	case 9:		// Transverse Mercator
	case 10:	// Stereographic
	case 11:	// Lambert Azimuthal Equal-Area
	case 12:	// Azimuthal Equidistant
	case 13:	// Gnomonic
	case 14:	// Orthographic
	case 15:	// General Vertical Near-Side Perspective
	case 16:	// Sinusoidal (Plate Caree)
	case 17:	// Equirectangular
	case 18:	// Miller Cylindrical
	case 19:	// Van Der Grinten I
	case 20:	// Oblique Mercator
		VTLOG("Warning!  We don't yet support DEM coordinate system %d.\n", iCoordSystem);
		break;
	}

	// We must have a functional CRS, or it will sebsequently fail
	if (!bSuccessfulCRS)
	{
		SetError(err, vtElevError::READ_CRS, "Couldn't determine CRS of DEM file");
		return false;
	}

	double dElevMin, dElevMax;
	DConvert(fp, 24, dElevMin, DEBUG_DEM);
	DConvert(fp, 24, dElevMax, DEBUG_DEM);

	fseek(fp, 852, 0);
	int iRows, iProfiles;
	IConvert(fp, 6, iRows);	// This "Rows" value will always be 1
	IConvert(fp, 6, iProfiles);
	VTLOG("DEM profiles: %d\n", iProfiles);

	m_iSize.x = iProfiles;

	// values we'll need while scanning the elevation profiles
	int		iProfileRows, iProfileCols;
	int		iElev;
	double	dLocalDatumElev, dProfileMin, dProfileMax;
	int		ygap;
	double	dMinY;
	DPoint2 start;

	if (bGeographic)
	{
		// If it's in degrees, it's flush square, so we can simply
		// derive the extents (m_EarthExtents) from the quad corners (m_Corners)
		ComputeExtentsFromCorners();
		dMinY = std::min(corners[0].y, corners[3].y);
	}
	else
	{
		VTLOG("DEM scanning to compute extents\n");
		m_EarthExtents.SetInsideOut();

		if (!bFixedLength)
			fseek(fp, iDataStartOffset, 0);
		// Need to scan over all the profiles, accumulating the TRUE
		// extents of the actual data points.
		int record = 0;
		int data_len;
		for (i = 0; i < iProfiles; i++)
		{
			if (progress_callback != NULL)
				progress_callback(i*49/iProfiles);

			if (bFixedLength)
				fseek(fp, iDataStartOffset + (record * 1024), 0);

			// We cannot use IConvert here, because there *might* be a spurious LF
			// after the number - seen in some rare files.
			if (fscanf(fp, "%d", &iRow) != 1)
			{
				SetError(err, vtElevError::READ_DATA, "Error reading DEM at profile %d of %d",
					i, iProfiles);
				return false;
			}
			IConvert(fp, 6, iColumn);
			// assert(iColumn == i+1);
			IConvert(fp, 6, iProfileRows);
			IConvert(fp, 6, iProfileCols);

			DConvert(fp, 24, start.x);
			DConvert(fp, 24, start.y);
			m_EarthExtents.GrowToContainPoint(start);
			start.y += ((iProfileRows-1) * dydelta);
			m_EarthExtents.GrowToContainPoint(start);

			if (bFixedLength)
			{
				record++;
				data_len = 144 + (iProfileRows * 6);
				while (data_len > 1020)	// max bytes in a record
				{
					data_len -= 1020;
					record++;
				}
			}
			else
			{
				DConvert(fp, 24, dLocalDatumElev);
				DConvert(fp, 24, dProfileMin);
				DConvert(fp, 24, dProfileMax);
				for (j = 0; j < iProfileRows; j++)
				{
					// We cannot use IConvert here, because there *might* be a spurious LF
					// after the number - seen in some rare files.
					if (fscanf(fp, "%d", &iElev) != 1)
						return false;
				}
			}
		}
		dMinY = m_EarthExtents.bottom;
	}
	VTLOG("DEM extents LRTB: %lf, %lf, %lf, %lf\n", m_EarthExtents.left,
		m_EarthExtents.right, m_EarthExtents.top, m_EarthExtents.bottom);

	// Compute number of rows
	double	fRows;
	if (bGeographic)
	{
		// degrees
		fRows = m_EarthExtents.Height() / dydelta * 3600.0f;
		m_iSize.y = (int)fRows + 1;	// 1 more than quad spacing
	}
	else
	{
		// some linear coordinate system
		fRows = m_EarthExtents.Height() / dydelta;
		m_iSize.y = (int)(fRows + 0.5) + 1;	// round to the nearest integer
	}

	// safety check
	if (m_iSize.y > 20000)
		return false;

	if (!AllocateGrid(err))
		return false;

	// jump to start of actual data
	fseek(fp, iDataStartOffset, 0);

	for (i = 0; i < iProfiles; i++)
	{
		if (progress_callback != NULL)
			progress_callback(50+i*49/iProfiles);

		// We cannot use IConvert here, because there *might* be a spurious LF
		// after the number - seen in some rare files.
		if (fscanf(fp, "%d", &iRow) != 1)
			return false;
		IConvert(fp, 6, iColumn);
		//assert(iColumn == i+1);

		IConvert(fp, 6, iProfileRows);
		IConvert(fp, 6, iProfileCols);

		DConvert(fp, 24, start.x);
		DConvert(fp, 24, start.y);
		DConvert(fp, 24, dLocalDatumElev);
		DConvert(fp, 24, dProfileMin);
		DConvert(fp, 24, dProfileMax);

		ygap = (int)((start.y - dMinY)/dydelta);

		for (j = ygap; j < (ygap + iProfileRows); j++)
		{
			//assert(j >=0 && j < m_iSize.y);	// useful safety check

			// We cannot use IConvert here, because there *might* be a spurious LF
			// after the number - seen in some rare files.
			if (fscanf(fp, "%d", &iElev) != 1)
				return false;
			if (iElev == -32767 || iElev == -32768)
				SetValue(i, j, INVALID_ELEVATION);
			else
			{
				// The DEM spec says:
				// "A value in this array would be multiplied by the "z" spatial
				// resolution (data element 15, record type A) and added to the
				// "Elevation of local datum for the profile" (data element 4, record
				// type B) to obtain the elevation for the point."
				SetValue(i, j, (short) iElev + (short) dLocalDatumElev);
			}
		}
	}
	fclose(fp);

	m_fVMeters = (float) (fVertUnits * dzdelta);
	ComputeHeightExtents();
	if (m_fMinHeight != dElevMin || m_fMaxHeight != dElevMax)
		VTLOG("DEM Reader: elevation extents in .dem (%.1f, %.1f) don't match data (%.1f, %.1f).\n",
		dElevMin, dElevMax, m_fMinHeight, m_fMaxHeight);

	return true;
}

