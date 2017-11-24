//
// Projections.h
//
// Copyright (c) 2001-2015 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
/** \file Projections.h */

#pragma once

//
// Earth's diameter: 12756 km
// approximate circumference: 40074 km
// each degree of latitude: 111.3 km
//
#define EARTH_RADIUS		6378000.0f	// in meters
#define METERS_PER_LATITUDE	111317.1f

// Some class names are just too long!
#define OCTransform OGRCoordinateTransformation

// Define a few common datums for convenience
#define EPSG_DATUM_OLD_HAWAIIAN	6135
#define EPSG_DATUM_PUERTO_RICO	6139
#define EPSG_DATUM_NAD27		6267
#define EPSG_DATUM_NAD83		6269
#define EPSG_DATUM_WGS72		6322
#define EPSG_DATUM_WGS84		6326

#include "ogr_spatialref.h"
#include "MathTypes.h"
#include "Units.h"
#include "ScopedPtr.h"

// A common use is to allocate a transform, then you want it to free itself.
typedef scoped_ptr<OGRCoordinateTransformation> ScopedOCTransform;


///////////////////////////

/**
 * The vtCRS class represents an earth coordinate reference system (CRS), which
 * is generally a projected coordinate system (PCS) or a geographic coordinate
 * system (GCS).  It is based on the OGR class
 * <a href="http://www.gdal.org/ogr/classOGRSpatialReference.html">OGRSpatialReference</a>
 * which represents a full OpenGIS Spatial Reference System.  The vtCRS
 * class extends OGRSpatialReference with several useful methods.
 */
class vtCRS : public OGRSpatialReference
{
public:
	vtCRS();
	~vtCRS();

	// Assignment
	vtCRS &operator=(const vtCRS &ref);

	// Equality
	bool operator==(const vtCRS &ref) const;
	bool operator!=(const vtCRS &ref) const;

	void	SetUTMZone(int iZone);
	int		GetUTMZone() const;
	OGRErr	SetDatum(int iDatum);
	int		GetDatum() const;
	LinearUnits	GetUnits() const;
	int		GuessEPSGCode() const;

	OGRErr	SetGeogCSFromDatum(int iDatum);

	bool	SetSimple(bool bUTM, int iUTMZone, int iDatum);
	void	SetSpatialReference(OGRSpatialReference *pRef);

	const char *GetName() const;
	const char *GetNameShort() const;

	bool GetTextDescription(char *type, char *value) const;
	bool SetTextDescription(const char *type, const char *value);

	bool ReadProjFile(const char *filename);
	bool WriteProjFile(const char *filename) const;

	static double GeodesicDistance(const DPoint2 &in, const DPoint2 &out, bool bQuick = false);

	void SetDymaxion(bool bTrue) { m_bDymaxion = bTrue; }
	bool IsDymaxion() const { return m_bDymaxion; }

protected:
	bool	m_bDymaxion;

	// Useful for debugging
	void LogDescription() const;
};

struct StatePlaneInfo
{
	const char *name;
	bool bNAD27;
	bool bNAD83;
	int usgs_code;
};

class Geodesic
{
public:
	void CalculateInverse();

	double	a;
	double	lam1, phi1;
	double	lam2, phi2;
	double	S;
	double	onef, f, f2, f4, f64;
};


//////////////////////////////
// Helper functions

const char *DatumToString(int d);
const char *DatumToStringShort(int d);
struct EPSGDatum
{
	bool bCommon;
	int iCode;
	const char *szName;
	const char *szShortName;
};
extern std::vector<EPSGDatum> g_EPSGDatums;
void SetupEPSGDatums();

StatePlaneInfo *GetStatePlaneTable();
int NumStatePlanes();
void CreateSimilarGeographicCRS(const vtCRS &source, vtCRS &geo);
OCTransform *CreateTransformIgnoringDatum(const vtCRS *pSource, vtCRS *pTarget);
OCTransform *CreateCoordTransform(const vtCRS *pSource,
						  const vtCRS *pTarget, bool bLog = false);
void TransformInPlace(OCTransform *transform, DPolygon2 &poly);
void TransformInPlace(OCTransform *transform, DLine2 &line);

/**
 * Determine an approximate conversion from degrees of longitude to meters,
 * given a latitude in degrees.
 */
double EstimateDegreesToMeters(double latitude);

/**
 * Return the number of meters for a given type of linear units
 */
double GetMetersPerUnit(LinearUnits lu);

/**
 * Return a string describing the units.
 */
const char *GetLinearUnitName(LinearUnits lu);

/**
 * Return number of meters per degree of longitude, at a given latitude.
 */
double MetersPerLongitude(double latitude);

/**
 * Read the contents of a world file.  You can pass any filename, and it will
 * look for the corresponding world file.
 */
bool ReadAssociatedWorldFile(const char *filename_base, double params[6]);

