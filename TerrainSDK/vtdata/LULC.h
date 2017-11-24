//
// Classes/structures to represent the data in a LULC file
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef LULCH
#define LULCH

#include <stdio.h>
#include "MathTypes.h"

#define LULC_ERR_FILE	1
#define LULC_ERR_READ	2
#define LULC_ERR_HEADER	3

typedef struct
{
	short x;
	short y;
} Coord;

typedef struct
{
	short first_coord;
	short last_coord;
	short PL;
	short PR;
	int PAL;
	int PAR;
	int Length;			// in coordinate units
	short StartNode;
	short FinishNode;
} LULCArc;

class LULCPoly
{
public:
	LULCPoly();
	~LULCPoly();

	short first_arc;
	short last_arc;
	int Attribute;
	int PerimeterLength;
	short NumIslands;
	short SurroundingPoly;

	// Vertices extracted by following and decoding the Arc data
	int m_iCoords;
	DPoint2 *m_p;	// in lat-lon

	// store extents, in local coordinates, for speed in testing
	double xmin, zmin, xmax, zmax;

private:
	// Don't let unsuspecting users stumble into assuming that object
	// copy semantics will work.  Declare them private and never
	// define them.
	LULCPoly( const LULCPoly & );
	LULCPoly &operator=( const LULCPoly & );

};

class LULCSection
{
public:
	LULCSection() {};
	~LULCSection();

	uint m_iNumArcs;
	uint m_iNumCoords;
	uint m_iNumPolys;
	uint m_iLengthOfFAP;
	uint m_iNumNodes;

	LULCArc *m_pArc;
	Coord *m_pCoord;
	LULCPoly *m_pPoly;
	short *m_pFAP;

private:
	// Don't let unsuspecting users stumble into assuming that object
	// copy semantics will work.  Declare them private and never
	// define them.
	LULCSection( const LULCSection & );
	LULCSection &operator=( const LULCSection & );
};

/**
 * Implements reading data from a USGS LULC file.
 */
class vtLULCFile
{
public:
	/// constructor - pass it the name of a LULC (GIRAS) file
	vtLULCFile(const char *fname);

	/// destructor
	~vtLULCFile();

	/// attempt to determine the mapping from local to latlon
	void SetupMapping();

	// read a section (there are usually 4 sections)
	void ReadSection(LULCSection *pSection, FILE *fp);

	// read a single record from the file
	int GetRecord(FILE *fp, char *buf);

	// convert from local coordinates to latlon
	void LocalToLatlon(Coord &local, DPoint2 &latlon);

	// processing LULC
	void ProcessLULCPolys();
	void ProcessLULCPoly(LULCSection *pSection, LULCPoly *pPoly);
	int FindAttribute(double utm_x, double utm_y);

	/// return error type if it didn't load successfully
	int m_iError;
	/// if an error occured, return it as an English message
	const char *GetErrorMessage();

	// extent of control points, in local coordinates
	Coord m_cMin, m_cMax;

	// local coordinates of control points
	Coord m_cCorners[6];

	// latitude and longitude of control points
	DPoint2 m_Corners[6];

	/// access sections
	uint NumSections() { return m_iNumSections; };
	/// access sections
	LULCSection *GetSection(int i) { return m_pSection+i; };

	// used for making a list of these files
	vtLULCFile *m_pNext;

private:
	// Don't let unsuspecting users stumble into assuming that object
	// copy semantics will work.  Declare them private and never
	// define them.
	vtLULCFile( const vtLULCFile & );
	vtLULCFile &operator=( const vtLULCFile & );

	uint m_iNumSections;
	LULCSection *m_pSection;
	DMatrix3 m_transform;
//	DMatrix3 m_forwards;
};

#endif // LULCH
