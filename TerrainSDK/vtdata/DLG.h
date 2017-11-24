//
// Classes/structures to represent the data in a DLG file
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTDATA_DLGH
#define VTDATA_DLGH

#include <stdio.h>
#include <vector>
#include "MathTypes.h"
#include "vtCRS.h"

#define DLG_ERR_FILE	1
#define DLG_ERR_READ	2
#define DLG_ERR_HEADER	3
#define DLG_ERR_NODE	4
#define DLG_ERR_AREA	5
#define DLG_ERR_LINE	6

enum DLGType
{
	DLG_HYPSO,
	DLG_HYDRO,
	DLG_VEG,
	DLG_NONVEG,
	DLG_BOUNDARIES,
	DLG_MARKERS,
	DLG_ROAD,
	DLG_RAIL,
	DLG_MTF,
	DLG_MANMADE,
	DLG_UNKNOWN
};

struct DLGAttribute
{
	int m_iMajorAttr, m_iMinorAttr;
};

class DLGNode
{
public:
	DPoint2 m_p;
	int m_iAttribs;
};

class DLGArea
{
public:
	DPoint2 m_p;
	int m_iAttribs;
};

class DLGLine
{
public:
	int	HighwayNumber();

	int m_iNode1, m_iNode2;
	int m_iLeftArea, m_iRightArea;
	int m_iCoords;
	int m_iAttribs;
	std::vector<DLGAttribute> m_attr;
	DLine2 m_p;
};

/**
 * Implements reading data from a USGS DLG file.
 */
class vtDLGFile
{
public:
	vtDLGFile();	// constructor

	/// Reads a file.  Pass it the name of a DLG-O (.opt) file.  Returns true if successful.
	bool Read(const char *fname, bool progress_callback(int) = NULL);

	// read a single record from the file
	bool GetRecord(char *buf);

	/// after loading, guess file type from the element attributes
	DLGType GuessFileType();

	/// if an error occured, return it as an English message
	const char *GetErrorMessage();

	//for m_fp
	void OpenFile();
	void CloseFile();

	// return error if it didn't load successfully
	int m_iError;

	char	m_header[80];
	int		m_iNodes;
	int		m_iAreas;
	int		m_iLines;
	bool	m_bLFdelimited;

	// used while reading
	const char* m_fname;

	// quad corners (in the ground planimetric coordinate system)
	DPoint2 m_SW_utm, m_NW_utm, m_NE_utm, m_SE_utm;
	DPoint2 m_SW_lat, m_NW_lat, m_NE_lat, m_SE_lat;

	std::vector<DLGNode> m_nodes;
	std::vector<DLGArea> m_areas;
	std::vector<DLGLine> m_lines;

	vtCRS &GetCRS() { return m_crs; }

protected:
	FILE *m_fp;

	vtCRS	m_crs;

private:
	// Don't let unsuspecting users stumble into assuming that object
	// copy semantics will work.  Declare them private and never
	// define them,

	vtDLGFile( const vtDLGFile & );
	vtDLGFile &operator=( const vtDLGFile & );
};

// helper
bool ConvertDLG_from_LFDelim(const char *fname_from, const char *fname_to);

#endif
