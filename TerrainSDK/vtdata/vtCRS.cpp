//
// vtCRS.cpp
//
// Copyright (c) 2001-2015 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtCRS.h"
#include "StatePlane.h"
#include "MathTypes.h"
#include "vtString.h"	// for stricmp
#include "vtLog.h"
#include "Icosa.h"
#include "FilePath.h"

// GDAL
#include "cpl_csv.h"

/**
 * Enumeration of the Datum types
 *
 * This list of Datums was originally from the USGS "ProjectionLib" library
 * and provides a more concise way to represent a Datum than the string
 * values used by OGRSpatialReference.
 */
enum DATUM { ADINDAN = 0, ARC1950, ARC1960, AUSTRALIAN_GEODETIC_1966,
			 AUSTRALIAN_GEODETIC_1984, CAMP_AREA_ASTRO, CAPE,
			 EUROPEAN_DATUM_1950, EUROPEAN_DATUM_1979, GEODETIC_DATUM_1949,
			 HONG_KONG_1963, HU_TZU_SHAN, INDIAN, NAD27, NAD83,
			 OLD_HAWAIIAN_MEAN, OMAN, ORDNANCE_SURVEY_1936, PUERTO_RICO,
			 PULKOVO_1942, PROVISIONAL_S_AMERICAN_1956, TOKYO, WGS_72, WGS_84,
			 UNKNOWN_DATUM = -1, NO_DATUM = -2, DEFAULT_DATUM = -3 };

const char *datumToString(DATUM d);	// old function
const char *datumToStringShort(DATUM d); // old function
static void MassageDatumFromWKT(vtString &strDatum );

/////////////////////////////////////////////////////////////////////////////
// Implementation of class vtCRS
//

vtCRS::vtCRS() : OGRSpatialReference()
{
	m_bDymaxion = false;
}

vtCRS::~vtCRS()
{
}

/**
 * Assignment operator.
 */
vtCRS &vtCRS::operator=(const vtCRS &ref)
{
	// copy CRS
	if (ref.GetRoot() != NULL)
	{
		const OGRSpatialReference &ref_as_osr = ref;
		(*(OGRSpatialReference *)this) = ref_as_osr;
	}
	m_bDymaxion = ref.m_bDymaxion;
	return *this;
}

/**
 * Equality operator.
 */
bool vtCRS::operator==(const vtCRS &ref) const
{
	if (m_bDymaxion != ref.m_bDymaxion)
		return false;

	// Work around problem in IsSame, by detecting this type of difference
	if( IsProjected() != ref.IsProjected() )
		return false;
	bool same = IsSame( (OGRSpatialReference *) &ref ) != 0;
	if (!same)
		return false;

	return true;
}

/**
 * Inequality operator.
 */
bool vtCRS::operator!=(const vtCRS &ref) const
{
	if (m_bDymaxion != ref.m_bDymaxion)
		return true;

	// Work around problem in IsSame, by detecting this type of difference
	if( IsProjected() != ref.IsProjected() )
		return true;
	bool same = IsSame( (OGRSpatialReference *) &ref ) != 0;
	if (!same)
		return true;

	return false;
}

/**
 * Get the UTM zone of the projection.
 *
 * \param iZone
 * Should be one of the following values:
	- 1 through 60 for the northern hemisphere
	- -1 through -60 for the southern hemisphere
 */
void vtCRS::SetUTMZone(int iZone)
{
	// It appears that even lightweight tasks like getting UTM zone
	//  runs into trouble with the Locale ./, issue.
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	// reset the name of the projection so that SetUTM() will set it
	SetProjCS("unnamed");

	// Northern Hemisphere for positive zone numbers
	if (iZone > 0)
		SetUTM( iZone, 1);
	else
		SetUTM( -iZone, 0);
}

/**
 * Get the UTM zone of the projection.
 *
 * \return
 *	- 1 through 60 in the northern hemisphere
 *  - -1 through -60 for the southern hemisphere
 *  - 0 if the projection is not UTM
 */
int	vtCRS::GetUTMZone() const
{
	// It appears that even lightweight tasks like getting UTM zone
	//  runs into trouble with the Locale ./, issue.
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	int north;
	int zone = OGRSpatialReference::GetUTMZone(&north);
	if (north)
		return zone;
	else
		return -zone;
}


/**
 * Set the datum to an EPSG Datum code, a value in the range (6120 - 6904).
 *
 * Here are the codes for some of the better-known datums:
	- NAD27		6267
	- NAD83		6269
	- WGS84		6326
 */
OGRErr vtCRS::SetDatum(int iDatum)
{
	OGRErr err;

	// OGR does not have functionality to change the Datum of an
	// existing coordinate system.
	//
	if (IsGeographic() || GetRoot() == NULL)
	{
		// simple case: re-create the object with the new datum
		err = SetGeogCSFromDatum(iDatum);
	}
	else
	{
		// For a projected CS (PROJCS), we only want to change the geographic
		// part (GEOGCS).  Step 1: create the desired GEOCS
		vtCRS geo;
		err = geo.SetGeogCSFromDatum(iDatum);
		if (err != OGRERR_NONE)
			return err;
		OGR_SRSNode *clone = geo.GetRoot()->Clone();

		// Step 2, find the GEOGCS part of the PROJCS
		OGR_SRSNode *root = GetRoot();

		// Step 3, replace it
		root->DestroyChild(1);
		root->InsertChild(clone, 1);
	}
	return err;
}

/**
 * Return the datum as an EPSG code (an integer in the range of 6120 - 6904),
 * or -1 if the datum could not be determined.
 */
int vtCRS::GetDatum() const
{
	// Convert new DATUM string to old Datum enum
	const char *datum_string = GetAttrValue("DATUM");
	if (!datum_string)
		return -1;

	vtString strDatum = datum_string;
	MassageDatumFromWKT(strDatum);	// Convert WKT name to EPSG name

	for (uint i = 0; i < g_EPSGDatums.size(); i++)
	{
		if (!strcmp((const char *)strDatum, g_EPSGDatums[i].szName))
			return g_EPSGDatums[i].iCode;
	}
	return -1;
}

/**
 * Return the kind of horizontal units used by the CRS.  This is also called
 * "linear units."
 *
 * \return
	- LU_DEGREES - Arc Degrees
	- LU_METERS - Meters
	- LU_FEET_INT - Feet (International Foot)
	- LU_FEET_US - Feet (U.S. Survey Foot)
 */
LinearUnits vtCRS::GetUnits() const
{
	if (IsDymaxion())
		return LU_UNITEDGE;

	if (IsGeographic())
		return LU_DEGREES;  // degrees

	// Get horizontal units ("linear units")
	char *pszLinearUnits;
	double dfLinearConv = GetLinearUnits(&pszLinearUnits);
	double diff;

	diff = dfLinearConv - 0.3048;
	if( EQUAL(pszLinearUnits,SRS_UL_FOOT) || fabs(diff) < 0.000000001)
		return LU_FEET_INT;  // international feet

	diff = dfLinearConv - (1200.0/3937.0);
	if( EQUAL(pszLinearUnits,SRS_UL_US_FOOT) || fabs(diff) < 0.000000001)
		return LU_FEET_US;  // u.s. survey feet

	if( dfLinearConv == 1.0 )
		return LU_METERS;  // meters

	return LU_METERS;	// can't guess; assume meters
}


/**
 * Set the CRS by copying from a OGRSpatialReference.
 */
void vtCRS::SetSpatialReference(OGRSpatialReference *pRef)
{
	*((OGRSpatialReference *)this) = *pRef;
}

/**
 * Return a string describing the type of coordinate system.
 *
 * \par Example:
 *	"Geographic", "Transverse_Mercator", "Albers_Conic_Equal_Area"
 */
const char *vtCRS::GetName() const
{
	if (IsDymaxion())
		return "Dymax";

	const char *proj_string = GetAttrValue("PROJECTION");
	if (!proj_string)
		return "Geographic";
	else
		return proj_string;
}

/**
 * Return a very short string describing the type of coordinate system.
 * \par
 * Example values are "Geo", "UTM", "TM", "Albers", "LCC", etc.
 * or "Unknown" if it is unknown.
 */
const char *vtCRS::GetNameShort() const
{
	if (IsDymaxion())
		return "Dymax";
	if (IsGeographic())
		return "Geo";
	const char *proj_string = GetAttrValue("PROJECTION");
	if (!proj_string)
		return "Unknown";
	if (!strcmp(proj_string, SRS_PT_MERCATOR_1SP))
		return "Mercator";
	if (!strcmp(proj_string, SRS_PT_TRANSVERSE_MERCATOR))
	{
		if (GetUTMZone() != 0)
			return "UTM";
		else
			return "TM";
	}
	// Although supposedly Gauss-Kruger is just a form of Transverse Mercator.
	//  give it its own name which the user will probably expect to see.
	if (!strcmp(proj_string, "Gauss_Kruger"))
		return "GK";
	if (!strcmp(proj_string, SRS_PT_ALBERS_CONIC_EQUAL_AREA))
		return "Albers";
	if (!strcmp(proj_string, SRS_PT_LAMBERT_CONFORMAL_CONIC_1SP))
		return "LCC1SP";
	if (!strncmp(proj_string, "Lambert_Conformal_Conic", 23))
		return "LCC";
	if (!strcmp(proj_string, SRS_PT_HOTINE_OBLIQUE_MERCATOR))
		return "HOM";
	if (!strcmp(proj_string, SRS_PT_SINUSOIDAL))
		return "Sinus";
	if (!strcmp(proj_string, SRS_PT_STEREOGRAPHIC))
		return "Stereo";
	if (!strcmp(proj_string, SRS_PT_NEW_ZEALAND_MAP_GRID))
		return "NZMG";
	if (!strcmp(proj_string, SRS_PT_OBLIQUE_STEREOGRAPHIC))
		return "OS";
	if (!strcmp(proj_string, SRS_PT_POLAR_STEREOGRAPHIC))
		return "PS";
	if (!strcmp(proj_string, SRS_PT_KROVAK))
		return "Krovak";
	return "Other";
}


/**
 * Set the CRS to a fresh, new geographical coordinate system based on the
 * indicated Datum.
 */
OGRErr vtCRS::SetGeogCSFromDatum(int iDatum)
{
	// It appears that even lightweight tasks like setting up a geographic
	//  CRS somehow runs into trouble with the Locale ./, issue.
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	OGRErr err;
	Clear();

	// support old USGS datums for backward compatibility
	if (iDatum < 24)
	{
		DATUM eDatum = (DATUM) iDatum;
		switch (eDatum)
		{
		case ADINDAN:
			err = SetWellKnownGeogCS( "EPSG:4201" ); break;
		case ARC1950:
			err = SetWellKnownGeogCS( "EPSG:4209" ); break;
		case ARC1960:
			err = SetWellKnownGeogCS( "EPSG:4210" ); break;
		case AUSTRALIAN_GEODETIC_1966:
			err = SetWellKnownGeogCS( "EPSG:4202" ); break;
		case AUSTRALIAN_GEODETIC_1984:
			err = SetWellKnownGeogCS( "EPSG:4203" ); break;
		case CAPE:
			err = SetWellKnownGeogCS( "EPSG:4222" ); break;
		case EUROPEAN_DATUM_1950:
			err = SetWellKnownGeogCS( "EPSG:4230" ); break;
		case GEODETIC_DATUM_1949:
			err = SetWellKnownGeogCS( "EPSG:4272" ); break;
		case HU_TZU_SHAN:
			err = SetWellKnownGeogCS( "EPSG:4236" ); break;
	//	case INDIAN:
	//		err = SetWellKnownGeogCS( "EPSG:" ); break;	// there are 3 Indian Datum
		case NAD27:
			err = SetWellKnownGeogCS( "NAD27" ); break;
		case NAD83:
			err = SetWellKnownGeogCS( "NAD83" ); break;

		case OLD_HAWAIIAN_MEAN:
			err = SetWellKnownGeogCS( "EPSG:4135" ); break;
		case OMAN:
			err = SetWellKnownGeogCS( "EPSG:4232" ); break;	// Fahud
		case ORDNANCE_SURVEY_1936:
			err = SetWellKnownGeogCS( "EPSG:4277" ); break;
		case PUERTO_RICO:
			err = SetWellKnownGeogCS( "EPSG:4139" ); break;
		case PULKOVO_1942:
			err = SetWellKnownGeogCS( "EPSG:4284" ); break;
		case PROVISIONAL_S_AMERICAN_1956:
			err = SetWellKnownGeogCS( "EPSG:4248" ); break;
		case TOKYO:
			err = SetWellKnownGeogCS( "EPSG:4301" ); break;

		case WGS_72:
			err = SetWellKnownGeogCS( "WGS72" ); break;
		case WGS_84:
			err = SetWellKnownGeogCS( "WGS84" ); break;
		default:
			err = SetWellKnownGeogCS( "WGS84" );
		}
	}
	else
	{
		// Turn the datum into a coord system
		int iCoordSystem = iDatum - 2000;
		char name[12];
		sprintf(name, "EPSG:%d", iCoordSystem);
		err = SetWellKnownGeogCS(name);
	}
	return err;
}


/**
 * Convenient way to set a simple coordinate system.
 *
 * \param bUTM true for UTM, false for Geographic.
 * \param iUTMZone If UTM, this is the zone: 1 through 60 in the northern
 *		hemisphere, -1 through -60 for the southern hemisphere.
 * \param iDatum The Datum as either an old USGS code or an EPSG code
 */
bool vtCRS::SetSimple(bool bUTM, int iUTMZone, int iDatum)
{
	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	OGRErr err = SetGeogCSFromDatum(iDatum);
	if (err != OGRERR_NONE)
		return false;
	if (bUTM)
		SetUTMZone(iUTMZone);
	return true;
}

/**
 * Get the coordinate system as a text description.  If the CRS is Geographic
 * or UTM, then a "simple" type string will be returned.  For all other
 * CRS types, a WKT string is returned.
 *
 * \param type A string buffer to contain the type of description.
 * This buffer should be at least 7 characters long to contain either the
 * word "simple" or "wkt".
 *
 * \param value A string buffer to contain the full description.
 * This buffer should be at least 2048 characters long to contain either
 * a simple or WKT description.
 */
bool vtCRS::GetTextDescription(char *type, char *value) const
{
	int datum = GetDatum();
	const char *datum_string = DatumToStringShort(datum);
	int zone = GetUTMZone();

	if (IsGeographic())
	{
		strcpy(type, "simple");
		sprintf(value, "geo, datum %s", datum_string);
	}
	else if (zone != 0)
	{
		strcpy(type, "simple");
		sprintf(value, "utm, datum %s, zone %d", datum_string, zone);
	}
	else
	{
		// Something less common, must use full WTK representation
		strcpy(type, "wkt");

		char *wkt;
		OGRErr err = exportToWkt(&wkt);
		if (err != OGRERR_NONE)
			return false;
		strcpy(value, wkt);
		OGRFree(wkt);
	}
	return true;
}

/**
 * Set the coordinate system using a text description.
 *
 * \param type The type of description, either "simple" for short simple
 * string, or "wkt" for a full-length WKT (Well-Known Text) description.
 *
 * \param value The description itself.  A WKT description should be a
 * single string, with no extra whitespace.  A simple string can have the
 * following forms:
 *		- geo, datum D
 *		- utm, datum D, zone Z
 *
 * \par Example:
	\code
	crs.SetTextDescription("simple", "utm, datum WGS_84, zone 11");
	\endcode
 */
bool vtCRS::SetTextDescription(const char *type, const char *value)
{
	if (!strcmp(type, "simple"))
	{
		char datum[60];
		int iUTMZone;

		if (!strncmp(value, "geo", 3))
		{
			sscanf(value, "geo, datum %s", datum);
			SetWellKnownGeogCS(datum);
			return true;
		}
		else if (!strncmp(value, "utm", 3))
		{
			sscanf(value, "utm, datum %s zone %d", datum, &iUTMZone);
			if (datum[strlen(datum)-1] == ',')
				datum[strlen(datum)-1] = 0;
			SetWellKnownGeogCS(datum);
			SetUTMZone(iUTMZone);
			return true;
		}
	}
	else if (!strcmp(type, "wkt"))
	{
		char wkt_buf[2000], *wkt = wkt_buf;
		strcpy(wkt, value);
		OGRErr err = importFromWkt((char **) &wkt);
		return (err == OGRERR_NONE);
	}
	return false;
}

//
// Helper: open the .prj file associated with a filename
//
FILE *OpenCorrespondingPrjFile(vtString &filename, const char *mode)
{
	// check file extension
	if (filename.Right(4).CompareNoCase(".prj") == 0)
		return vtFileOpen(filename, mode);

	// doesn't already ends in .prj
	vtString base = filename;
	RemoveFileExtensions(base, false);
	filename = base + ".prj";
	FILE *fp = vtFileOpen(filename, mode);

	if (!fp)
	{
		// look backward one more extension, e.g. for .bt.gz
		RemoveFileExtensions(base, false);
		filename = base + ".prj";
		fp = vtFileOpen(filename, mode);
	}
	return fp;
}

/**
 * Read the CRS from a .prj file.
 *
 * If the filename does not have the file extension ".prj", this
 * method will look for a file which has the same name with a
 * ".prj" extension.
 *
 * \return true if successful.
 */
bool vtCRS::ReadProjFile(const char *filename)
{
	vtString prj_name = filename;
	FILE *fp = OpenCorrespondingPrjFile(prj_name, "rb");
	if (!fp)
		return false;
	fclose(fp);

	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	// GDAL doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = prj_name.UTF8ToLocal();

	// Now read and parse the file
	OGRErr eErr;
	char **papszPrj = CSLLoad( fname_local );

	// Must clear any old info before attempting to import the new info.
	Clear();

	// Actually, importFromESRI() does the whole job for us, including
	//  handling both normal .prj files, and weird ESRI variations.
	eErr = importFromESRI(papszPrj);

	CSLDestroy( papszPrj );

	return (eErr == OGRERR_NONE);
}


/**
 * Write the CRS to a .prj file.
 *
 * \return true if successful.
 */
bool vtCRS::WriteProjFile(const char *filename) const
{
	vtString fname = filename;
	FILE *fp2 = OpenCorrespondingPrjFile(fname, "wb");
	if (!fp2)
		return false;

	char *wkt;
	OGRErr err = exportToWkt(&wkt);
	if (err != OGRERR_NONE)
		return false;
	fprintf(fp2, "%s\n", wkt);
	fclose(fp2);
	OGRFree(wkt);
	return true;
}

/**
 * Given two geographic coordinates (longitude/latitude in degrees),
 * return the geodesic arc distance in meters.  The WGS84 spheroid
 * is used.
 */
double vtCRS::GeodesicDistance(const DPoint2 &geo1, const DPoint2 &geo2,
	bool bQuick)
{
	if (bQuick)
	{
		// when the user cares more about speed than accuracy, just do
		// the quick calculation assuming the earth is a sphere
		// (TODO if performance of full calculation ever proves a bottleneck)
	}

	// We don't have direct access to the PROJ.4 library from this module,
	// so we can't set the exact coordinate system (in particular, the
	// spheroid) using exportToProj4() and pj_init().
	//
	// Instead, fill in the values directly for WGS84, which in practice
	// shouldn't give distance values significantly different from other
	// spheroids.

	Geodesic gd;
	gd.a = 6378137.0000000;
	gd.onef = 0.99664718933525;
	gd.f = 1.0 - gd.onef;
	gd.f2 = gd.f / 2.0;
	gd.f4 = gd.f / 4.0;
	gd.f64 = gd.f * gd.f / 64.0;

	// Now fill in the start and end points, convert to lon/lat in radians
	gd.lam1 = geo1.x / 180.0 * PId;
	gd.phi1 = geo1.y / 180.0 * PId;
	gd.lam2 = geo2.x / 180.0 * PId;
	gd.phi2 = geo2.y / 180.0 * PId;

	gd.CalculateInverse();
	return gd.S;
}

void vtCRS::LogDescription() const
{
	LinearUnits lu = GetUnits();

	VTLOG("Units: %s\n", GetLinearUnitName(lu));
	VTLOG1("WTK: ");

	char *wkt;
	OGRErr err = exportToWkt(&wkt);
	if (err != OGRERR_NONE)
		VTLOG1("Error\n");
	else
	{
		VTLOG1(wkt);
		VTLOG1("\n");
		OGRFree(wkt);
	}
}

/**
 * Try to determine the EPSG code that corresponds to the current CRS.
 * This important capability is mysteriously absent from the underlying
 * libraries (OGR, PROJ.4) so it is implemented here.
 *
 * \return An EPSG coordinate system code, or -1 if it couldn't be guessed.
 */
int vtCRS::GuessEPSGCode() const
{
	int iCoordSystem = -1;
	int iDatum = GetDatum();
	if (IsGeographic())
		return iDatum - 2000;

	const char *attr = GetAttrValue("PROJCS");
	if (attr != NULL && strcmp(attr, "OSGB 1936 / British National Grid") == 0)
		return 27700;

	int zone = GetUTMZone();
	if (zone > 0)
	{
		// It's UTM North
		if (iDatum == 6326)
			iCoordSystem = 32600 + zone;
	}
	if (zone < 0)
	{
		// It's UTM South
		if (iDatum == 6326)
			iCoordSystem = 32700 + (-zone);
	}
	return iCoordSystem;
}

/////////////////////////////////////////////////////////////////////////////
// Helper functions


/**
 * Given a non-geographic CRS, produce a geographic CRS which has the same
 * datum/ellipsoid values.
 */
void CreateSimilarGeographicCRS(const vtCRS &source,
	vtCRS &geo)
{
	geo.SetWellKnownGeogCS("WGS84");

	// We can't convert datum yet.  Force assumption that source datum
	// is the same as the destination.
	const char *datum_string = source.GetAttrValue("DATUM");
	const char *ellipsoid_string = source.GetAttrValue("SPHEROID");
	geo.SetGeogCS("WGS84", datum_string, ellipsoid_string,
		source.GetSemiMajor(), source.GetInvFlattening());
}


/**
 * Provides access to the State Plane Table
 */
StatePlaneInfo *GetStatePlaneTable()
{
	return g_StatePlaneInfo;
}

int NumStatePlanes()
{
	return sizeof(g_StatePlaneInfo) /  sizeof(StatePlaneInfo);
}


const char *datumToString(DATUM d)
{
	switch ( d )
	{
		case ADINDAN:
			return "ADINDAN";
		case ARC1950:
			return "ARC1950";
		case ARC1960:
			return "ARC1960";
		case AUSTRALIAN_GEODETIC_1966:
			return "AUSTRALIAN GEODETIC 1966";
		case AUSTRALIAN_GEODETIC_1984:
			return "AUSTRALIAN GEODETIC 1984";
		case CAMP_AREA_ASTRO:
			return "CAMP AREA ASTRO";
		case CAPE:
			return "CAPE";
		case EUROPEAN_DATUM_1950:
			return "EUROPEAN DATUM 1950";
		case EUROPEAN_DATUM_1979:
			return "EUROPEAN DATUM 1979";
		case GEODETIC_DATUM_1949:
			return "GEODETIC DATUM 1949";
		case HONG_KONG_1963:
			return "HONG KONG 1963";
		case HU_TZU_SHAN:
			return "HU TZU SHAN";
		case INDIAN:
			return "INDIAN";
		case NAD27:
			return "NAD27";
		case NAD83:
			return "NAD83";
		case OLD_HAWAIIAN_MEAN:
			return "OLD HAWAIIAN MEAN";
		case OMAN:
			return "OMAN";
		case ORDNANCE_SURVEY_1936:
			return "ORDNANCE SURVEY 1936";
		case PUERTO_RICO:
			return "PUERTO RICO";
		case PULKOVO_1942:
			return "PULKOVO 1942";
		case PROVISIONAL_S_AMERICAN_1956:
			return "PROVISIONAL SOUTH AMERICAN 1956";
		case TOKYO:
			return "TOKYO";
		case WGS_72:
			return "WGS72";
		case WGS_84:
			return "WGS84";
		case UNKNOWN_DATUM:
			return "Unknown";
		case NO_DATUM:
			return "None";
		case DEFAULT_DATUM:
			return "Default";
		default:
			return "Bad";
	}
}
/**
 * Convert an a Datum Code to a string of the Datum Name.
 */
const char *DatumToString(int d)
{
	if (d < 24)
		return datumToString((DATUM)d);	// allow backward compatibility

	for (uint i = 0; i < g_EPSGDatums.size(); i++)
	{
		if (g_EPSGDatums[i].iCode == d)
			return g_EPSGDatums[i].szName;
	}
	return "Bad";
}


const char *datumToStringShort(DATUM d)
{
	switch (d)
	{
	case ADINDAN:
		return "ADINDAN";
	case ARC1950:
		return "ARC1950";
	case ARC1960:
		return "ARC1960";
	case AUSTRALIAN_GEODETIC_1966:
		return "AGD66";
	case AUSTRALIAN_GEODETIC_1984:
		return "AGD84";
	case CAMP_AREA_ASTRO:
		return "CAMP AREA ASTRO";
	case CAPE:
		return "CAPE";
	case EUROPEAN_DATUM_1950:
		return "ED50";
	case EUROPEAN_DATUM_1979:
		return "ED79";
	case GEODETIC_DATUM_1949:
		return "GD49";
	case HONG_KONG_1963:
		return "HONG KONG 1963";
	case HU_TZU_SHAN:
		return "HU TZU SHAN";
	case INDIAN:
		return "INDIAN";
	case NAD27:
		return "NAD27";
	case NAD83:
		return "NAD83";
	case OLD_HAWAIIAN_MEAN:
		return "OLD HAWAII";
	case OMAN:
		return "OMAN";
	case ORDNANCE_SURVEY_1936:
		return "OSGB 1936";
	case PUERTO_RICO:
		return "PUERTO RICO";
	case PULKOVO_1942:
		return "PULKOVO 1942";
	case PROVISIONAL_S_AMERICAN_1956:
		return "PSAD 1956";
	case TOKYO:
		return "TOKYO";
	case WGS_72:
		return "WGS72";
	case WGS_84:
		return "WGS84";
	case UNKNOWN_DATUM:
		return "Unknown";
	case NO_DATUM:
		return "None";
	case DEFAULT_DATUM:
		return "Default";
	default:
		return "Bad";
	}
}
/**
 * Convert a Datum Code to a (short) string of the Datum Name.
 */
const char *DatumToStringShort(int d)
{
	if (d < 24)
		return datumToStringShort((DATUM)d); // allow backward compatibility

	for (uint i = 0; i < g_EPSGDatums.size(); i++)
	{
		if (g_EPSGDatums[i].iCode == d)
		{
			const char *name = g_EPSGDatums[i].szShortName;
			if (*name == '\0')
				name = g_EPSGDatums[i].szName;
			return name;
		}
	}
	return "Bad";
}

#include "EPSG_Datums.h"

std::vector<EPSGDatum> g_EPSGDatums;

int compare_datum(const void *aa, const void *bb)
{
	EPSGDatum *a = (EPSGDatum *)aa;
	EPSGDatum *b = (EPSGDatum *)bb;
	return strcmp(a->szName, b->szName);
}

/**
 * Setup the global array of EPSG Datums, accessible as g_EPSGDatums.
 */
void SetupEPSGDatums()
{
	// Copy from the statically initialized C array into our more
	//  robust C++ array.
	int count = sizeof(epsg_datums) / sizeof(epsg_datum);
	EPSGDatum dat;

	g_EPSGDatums.reserve(count);
	for (int i = 0; i < count; i++)
	{
		dat.bCommon = epsg_datums[i].common != 0;
		dat.iCode = epsg_datums[i].code;
		dat.szName = epsg_datums[i].name;
		dat.szShortName = epsg_datums[i].shortname;
		g_EPSGDatums.push_back(dat);
	}
	// sort them
	qsort(&g_EPSGDatums.front(), count, sizeof(EPSGDatum), compare_datum);
}

void CleanupEPSGDatums()
{
	g_EPSGDatums.clear();
}

/**
 * Determine an approximate conversion from degrees of longitude to meters,
 * given a latitude in degrees.
 */
double EstimateDegreesToMeters(double latitude)
{
	// estimate meters per degree of longitude, using the terrain origin
	double r0 = EARTH_RADIUS * cos(latitude / 180.0 * PId);
	double circ = 2.0 * r0 * PId;
	return circ / 360.0f;
}

/**
 * Create a conversion between coordinate systems, making the assumption that
 * the Datum of the target is the same as the Datum of the source.
 */
OCTransform *CreateTransformIgnoringDatum(const vtCRS *pSource, vtCRS *pTarget)
{
	vtCRS DummyTarget = *pTarget;

	const char *datum_string = pSource->GetAttrValue("DATUM");

	OGR_SRSNode *dnode = DummyTarget.GetAttrNode("DATUM");
	if (dnode)
		dnode->GetChild(0)->SetValue(datum_string);

	const OGR_SRSNode *enode1 = pSource->GetAttrNode("SPHEROID");
	OGR_SRSNode *enode2 = DummyTarget.GetAttrNode("SPHEROID");
	if (enode1 && enode2)
	{
		enode2->GetChild(0)->SetValue(enode1->GetChild(0)->GetValue());
		enode2->GetChild(1)->SetValue(enode1->GetChild(1)->GetValue());
		enode2->GetChild(2)->SetValue(enode1->GetChild(2)->GetValue());
	}

#if VTDEBUG
	bool bLog = true;
#else
	bool bLog = false;
#endif
	return CreateCoordTransform(pSource, &DummyTarget, bLog);
}

/**
 * Support for converting to and from the Dymaxion projection is implemented
 * by overriding _and_ containing an OCT object.
 *
 * The contained OCT transforms to/from WGS84 Geographic coords.
 * The subclassing is used to implement the Transform() method which does
 * the additional Dymaxion conversion.
 */
class DymaxOCT : public OCTransform
{
public:
	DymaxOCT()
	{
		m_pStandardTransform = NULL;
	}
	DymaxOCT(OCTransform *pStandard, bool bDirection)
	{
		m_pStandardTransform = pStandard;
		m_bDirection = bDirection;
	}
	~DymaxOCT()
	{
		delete m_pStandardTransform;
	}

	OGRSpatialReference *GetSourceCS() { return m_pStandardTransform->GetSourceCS(); }
	OGRSpatialReference *GetTargetCS() { return m_pStandardTransform->GetTargetCS(); }

	int Transform(int nCount, double *x, double *y, double *z = NULL);
	int TransformEx(int nCount, double *x, double *y, double *z = NULL, int *pabSuccess = NULL );

	OCTransform *m_pStandardTransform;
	bool m_bDirection;		// true: to dymax, false: from dymax
	DymaxIcosa m_ico;
};

int DymaxOCT::Transform(int nCount, double *x, double *y, double *z)
{
	int iConverted = 0;
	for (int i = 0; i < nCount; i++)
	{
		DPoint2 pin(x[i], y[i]), pout;

		bool success = m_ico.GeoToDymax(pin, pout);
		if (!success)
			return i;	// stop right here

		x[i] = pout.x;
		y[i] = pout.y;
		iConverted++;
	}
	return iConverted != 0;
}

int DymaxOCT::TransformEx(int nCount, double *x, double *y, double *z, int *pabSuccess)
{
	int iConverted = 0;
	for (int i = 0; i < nCount; i++)
	{
		DPoint2 pin(x[i], y[i]), pout;

		bool success = m_ico.GeoToDymax(pin, pout);
		if (pabSuccess != NULL)
			pabSuccess[i] = (int) success;

		x[i] = pout.x;
		y[i] = pout.y;
		if (success)
			iConverted++;
	}
	return iConverted != 0;
}

// display debugging information to the log
void LogConvertingCRSs(const vtCRS *pSource, const vtCRS *pTarget)
{
#if 0
	char *wkt1, *wkt2;
	OGRErr err = pSource->exportToWkt(&wkt1);
	err = pTarget->exportToWkt(&wkt2);
	VTLOG(" Converting from: %s\n", wkt1);
	VTLOG("   Converting to: %s\n", wkt2);
	OGRFree(wkt1);
	OGRFree(wkt2);
#else
	char *crs1, *crs2;
	OGRErr err = pSource->exportToProj4(&crs1);
	err = pTarget->exportToProj4(&crs2);
	VTLOG(" Converting from: %s\n", crs1);
	VTLOG("   Converting to: %s\n", crs2);
	OGRFree(crs1);
	OGRFree(crs2);
#endif
}

/**
 * Use this function instead of OGRCreateCoordinateTransformation to create
 * a transformation between two vtCRSs.  Not only does it get around
 * the 'const' issue with the arguments to the OGR function, but it also
 * has a handy logging option, and can deal with any additional projections
 * that vtCRS adds to OGRSpatialReference.
 */
OCTransform *CreateCoordTransform(const vtCRS *pSource, const vtCRS *pTarget,
	bool bLog)
{
	// It appears that even lightweight tasks like setting up a CRS Transform
	//  somehow runs into trouble with the Locale ./, issue.
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	if (bLog)
		LogConvertingCRSs(pSource, pTarget);

	OCTransform *result = OGRCreateCoordinateTransformation((OGRSpatialReference *)pSource,
		(OGRSpatialReference *)pTarget);
	if (bLog)
		VTLOG(" Transform: %s\n", result ? "succeeded" : "failed");

	if (!result && !bLog)
	{
		// Even if the caller didn't ask for logging, log failures.
		VTLOG("Could not convert:\n");
		LogConvertingCRSs(pSource, pTarget);
	}

	if (!pSource->IsDymaxion() && pTarget->IsDymaxion())
	{
		return new DymaxOCT(result, true);
	}
	else if (!pSource->IsDymaxion() && pTarget->IsDymaxion())
	{
		return new DymaxOCT(result, false);
	}
	else
		return result;
}

void TransformInPlace(OCTransform *transform, DPolygon2 &poly)
{
	for (uint ring = 0; ring < poly.size(); ring++)
		TransformInPlace(transform, poly[ring]);
}

void TransformInPlace(OCTransform *transform, DLine2 &line)
{
	for (uint v = 0; v < line.GetSize(); v++)
		transform->Transform(1, &line[v].x, &line[v].y);
}

double GetMetersPerUnit(LinearUnits lu)
{
	switch (lu)
	{
	case LU_DEGREES:
	case LU_UNITEDGE:
		return 1.0;		// actually no definition for degrees -> meters
	case LU_METERS:
		return 1.0;		// meters per meter
	case LU_FEET_INT:
		return 0.3048;		// international foot
	case LU_FEET_US:
		return (1200.0/3937.0);	// U.S. survey foot
	case LU_UNKNOWN:
		// keep picky compilers quiet.
		break;
	}
	return 1.0;
};

const char *GetLinearUnitName(LinearUnits lu)
{
	switch (lu)
	{
	case LU_DEGREES:  return "Degrees";
	case LU_METERS:	  return "Meters";
	case LU_FEET_INT: return "Feet";
	case LU_FEET_US:  return "Feet (US)";
	case LU_UNITEDGE:  return "UnitEdge";
	case LU_UNKNOWN:
		// keep picky compilers quiet.
		break;
	}
	return "Unknown";
}

// helper
double MetersPerLongitude(double latitude)
{
	return METERS_PER_LATITUDE * cos(latitude / 180.0 * PId);
}

bool ReadAssociatedWorldFile(const char *filename_base, double params[6])
{
	// Look for the file with a variety of possible extensions
	FILE *fp = NULL;
	vtString ext = GetExtension(filename_base), fname;
	if (!ext.CompareNoCase(".jpg"))
		fname = ChangeFileExtension(filename_base, ".jgw");
	else if (!ext.CompareNoCase(".png"))
		fname = ChangeFileExtension(filename_base, ".pgw");
	else if (!ext.CompareNoCase(".tif"))
		fname = ChangeFileExtension(filename_base, ".tfw");
	else
		fname = ChangeFileExtension(filename_base, ".wld");
	fp = fopen(fname, "rb");
	if (!fp && GetExtension(fname) != ".wld")
	{
		fname = ChangeFileExtension(filename_base, ".wld");
		fp = fopen(fname, "rb");
	}
	if (!fp)
		return false;

	// read the parameters
	for (int i = 0; i < 6; i++)
		fscanf(fp, "%lf", &params[i]);
	fclose(fp);
	return true;
}

static const char *papszDatumEquiv[] =
{
	"Militar_Geographische_Institut",
	"Militar_Geographische_Institute",
	"World_Geodetic_System_1984",
	"WGS_1984",
	"WGS_72_Transit_Broadcast_Ephemeris",
	"WGS_1972_Transit_Broadcast_Ephemeris",
	"World_Geodetic_System_1972",
	"WGS_1972",
	"European_Terrestrial_Reference_System_89",
	"European_Reference_System_1989",
	"Nouvelle_Triangulation_Francaise_(Paris)",
	"Nouvelle_Triangulation_Francaise_Paris",
	"Rikets_koordinatsystem_1990",
	"RT90",
	NULL
};

//
// A limited implementation of reversing the effect of WKTMassageDatum
//
static void MassageDatumFromWKT(vtString &strDatum )
{
	// Sometimes, there is an extra "D_" in front of the datum name.
	// This appears to come from ESRI software.  If it's there, skip it.
	if (strDatum.Left(2) == "D_")
		strDatum = strDatum.Right(strDatum.GetLength()-2);

	for(int i = 0; papszDatumEquiv[i] != NULL; i += 2)
	{
		if( !strcmp((const char *)strDatum,papszDatumEquiv[i+1]) )
		{
			strDatum = papszDatumEquiv[i];
			break;
		}
	}
	strDatum.Replace('_', ' ');
}

