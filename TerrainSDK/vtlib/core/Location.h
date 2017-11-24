//
// Location classes
//
// Copyright (c) 2002-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef LOCATION_H
#define LOCATION_H

#include "vtdata/vtString.h"
#include "vtdata/vtCRS.h"
#include "vtdata/LocalCS.h"

#if SUPPORT_WSTRING
	#define LocNameString wstring2
#else
	#define LocNameString std::string
#endif

/** \addtogroup nav */
/*@{*/

/**
 * A location is represented as a earth-coordinate point, an elevation,
 * and an orientation.  The point is always in Geographic coordinates
 * (Latitude/Longitude, WGS84).  It should be converted to and from
 * the actual, desired CRS as appropriate.
 *
 * Location and view direction are encoded as two 3D points.  The first
 * is the location, the second is the "Look At" point.  Heading and
 * pitch are implicit in the second point.
 *
 * The distance between the two points is arbitrary, but it should be
 * neither very small nor very large, to avoid numerical problems.
 */
class vtLocation
{
public:
	vtLocation() {}
	vtLocation(const vtLocation &v) { *this = v; }
	vtLocation &operator=(const vtLocation &v)
	{
		m_strName = v.m_strName;
		m_pos1 = v.m_pos1;
		m_fElevation1 = v.m_fElevation1;
		m_pos2 = v.m_pos2;
		m_fElevation2 = v.m_fElevation2;
		return *this;
	}

	LocNameString m_strName;

	// The two points: "look from" and "look at"
	DPoint2	m_pos1;
	float	m_fElevation1;
	DPoint2 m_pos2;
	float	m_fElevation2;
};

/**
 * This class manages a set of named locations (position and orientation).
 * It can save and load the locations to a file, and jump back to
 * any stored location.
 *
 * \sa vtLocation
 */
class vtLocationSaver
{
	friend class LocationVisitor;
public:
	vtLocationSaver();
	vtLocationSaver(const vtString &fname);
	~vtLocationSaver();

	/// Read from a .loc file
	bool Read(const vtString &fname);

	/// Write to a .loc file
	bool Write(const vtString &fname = "");

	const vtString &GetFilename() { return m_strFilename; }

	/// Remove a location by index
	void Remove(int num);

	/// Return number of locations
	int NumLocations() { return m_loc.GetSize(); }

	/// Get a location by index.
	vtLocation *GetLocation(int num) const { return m_loc[num]; }

	/// Find a location by name.
	int FindLocation(const char *locname);

	void Clear();

	// You must call these methods before this class is useful for vtTerrain:
	/// Attach this LocationSaver to an object in the scenegraph.
	void SetTransform(vtTransform *trans) { m_pTransform = trans; }
	/// Return the object attached to this LocationSaver
	vtTransform *GetTransform() { return m_pTransform; }

	/// Define the conversion between this saver and terrain coordinates.
	void SetLocalCS(const LocalCS &conv) { m_LocalCS = conv; }
	const LocalCS &GetLocalCS() { return m_LocalCS; }

	/// Define the CRS of the object attached to this LocationSaver.
	void SetCRS(const vtCRS &crs);
	const vtCRS &GetAtCRS() const { return m_crs; }

	/// Get the location of the attached object and store it.
	bool StoreTo(uint num, const LocNameString &name = "");

	/// Recall a location by index, and set it to the attached object.
	bool RecallFrom(int num);
	bool RecallFrom(const char *name);

protected:
	// Implementation
	// Store information necessary to convert from global earth CS
	// to the local CS
	LocalCS		m_LocalCS;
	vtCRS		m_crs;
	vtTransform	*m_pTransform;

	vtString	m_strFilename;
	OCTransform	*m_pConvertToWGS;
	OCTransform	*m_pConvertFromWGS;

	vtArray<vtLocation*> m_loc;
};

/*@}*/  // nav

#endif	// LOCATION_H
