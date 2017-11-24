//
// GEOnet.h
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef GEONET_H
#define GEONET_H

#include "config_vtdata.h"
#include "MathTypes.h"
#include "vtString.h"

#if SUPPORT_WSTRING

class Place
{
public:
	DPoint2 m_pos;
	wstring2 m_fullname;
	vtString m_fullname_nd;
	short m_ppc; // Populated Place Classification (1 high to 5 low, 6=unknown)
};

class PlaceArray : public vtArray<Place *>
{
public:
	// this class is used for reference purposes only, it does not own the
	// objects it contains so it does not and should not delete them
};

class Country
{
public:
	Country();
	~Country();

	bool FindPlace(const char *name_nd, DPoint2 &point, bool bFullLength);
	bool FindPlace(const std::wstring &name, DPoint2 &point, bool bFullLength);
	bool FindAllMatchingPlaces(const char *name_nd, bool bFullLength, PlaceArray &places);

	bool WriteSHP(const char *fname);

	vtString m_abb;
	vtString m_full;
	vtArray<Place*> m_places;
};

/**
 * This class parses and utilizes a subset of the information from the
 * GEOnet Names Server.
 *
 * See http://earth-info.nga.mil/gns/html/ for the description of the data and
 * the raw data files.
 *
 * From the 700 MB of raw source files, a compact 'GCF' file containing only
 * the geographic coordinate and a single name string are extracted for
 * each populated place.
 *
 * The GCF (75 MB) can be then loaded and used for rough geocoding of
 * international addresses
 */
class Countries
{
public:
	~Countries();

	// create GCF from raw GEOnet Names Server (GNS) files
	bool ReadCountryList(const char *fname);
	void ParseRawCountryFiles(const char *path_prefix, bool bNativeNames);
	void ParseRawCountry(int i, bool bNativeNames);
	void WriteGCF(const char *fname);
	bool WriteSingleSHP(const char *fname);
	bool WriteSHPPerCountry(const char *prefix);
	void Free(bool progress_callback(int)=0);

	// load and use GCF
	bool ReadGCF(const char *fname, bool progress_callback(int) = NULL);
	bool FindPlace(const char *country, const char *place, DPoint2 &point);
	bool FindPlaceWithGuess(const char *country, const char *place, DPoint2 &point, bool bUTF8=false);

protected:
	vtArray<Country*> m_countries;
	vtString m_path;
};

#endif // SUPPORT_WSTRING

/**
 * Reads the U.S. Census Gazetteer data files, and allows querying the data.
 */
class Gazetteer
{
public:
	bool ReadPlaces(const char *fname);
	bool ReadZips(const char *fname);

	bool FindPlace(const vtString &state, const vtString &place, DPoint2 &geo);
	bool FindZip(int zip, DPoint2 &geo);

	class Place {
	public:
		vtString m_state;
		vtString m_name;
		DPoint2 geo;
	};
	std::vector<Place> m_places;

	class Zip {
	public:
		int m_zip;
		DPoint2 geo;
	};
	std::vector<Zip> m_zips;
};


#endif // GEONET_H

