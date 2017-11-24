//
// DLG.c
// read a DLG-O file
//
// Copyright (c) 2001-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <string.h>
#include <stdlib.h>

#include "DLG.h"
#include "vtCRS.h"
#include "FilePath.h"

// helper functions
int geti6(const char *buf)
{
	char buf2[7];
	strncpy(buf2, buf, 6);
	buf2[6] = 0;
	return atoi(buf2);
}

int geti3(const char *buf)
{
	char buf2[4];
	strncpy(buf2, buf, 3);
	buf2[3] = 0;
	return atoi(buf2);
}

double getd12(const char *buf)
{
	char buf2[13];
	strncpy(buf2, buf, 12);
	buf2[12] = 0;
	return atof(buf2);
}


//
// Return the highway/route number of a DLG
// line segment, or -1 if it doesn't have one.
//
int DLGLine::HighwayNumber()
{
	for (int j = 0; j < m_iAttribs; j++)
	{
		if (m_attr[j].m_iMajorAttr >= 172 && m_attr[j].m_iMajorAttr <= 176)
			return m_attr[j].m_iMinorAttr;
	}
	return -1;
}


/////////////////////////////////////////////


vtDLGFile::vtDLGFile()
{
}

bool vtDLGFile::Read(const char *fname, bool progress_callback(int))
{
	char buf[80];
	int i, j, iUTMZone;

	// basic initialization
	m_iError = 0;

	m_fname = fname;
	m_fp = vtFileOpen(fname, "rb");
	if (!m_fp)
	{
		m_iError = DLG_ERR_FILE;
		return false;
	}

	// check to see if this is a LF-delimited file
	m_bLFdelimited = false;
	fseek(m_fp, 56, SEEK_SET);
	for (i = 0; i < 24; i++)
	{
		if (fgetc(m_fp) == 10)
			m_bLFdelimited = true;
	}

	// rewind to beginning
	fseek(m_fp, 0, SEEK_SET);

	// record 1 - banner
	if (!GetRecord(buf)) return false;
	strncpy(m_header, buf, 72);

	// record 2 - cell name, date, qualifier, scale, sectional indicator
	if (!GetRecord(buf)) return false;

	// record 3 - contour interval info, status flags
	if (!GetRecord(buf)) return false;

	// record 4 - codes, resolution, transformation info
	if (!GetRecord(buf)) return false;
	int reference_system = geti6(buf + 6);	// 1 = UTM, 3 = Albers
	if (reference_system == 3)	// We don't support Albers
		return false;
	iUTMZone = geti6(buf + 12);

	// Datum.  Undocumented field!  Had to look at the government's
	// own "dlgv32" source to figure out how to find this value.
	int iDLGDatum = geti3(buf + 66);

	// safety check.. because they do
	if ((iDLGDatum < 0) || (iDLGDatum > 4))
		iDLGDatum = 0;

	// this is how they interpret the value
	int iDatum;
	switch (iDLGDatum)
	{
		case 0:
			iDatum = EPSG_DATUM_NAD27;
			break;
		case 1:
			iDatum = EPSG_DATUM_NAD83;
			break;
		default:
			iDatum = -1;
			break;
	}

	// record 5-9 - Projection parameters for map transformation
	for (i = 5; i < 10; i++)
		if (!GetRecord(buf)) return false;

	// record 10 - Internal file-to-map projection transformation parameters
	if (!GetRecord(buf)) return false;

	// record 11 - SW quadrangle corner
	if (!GetRecord(buf)) return false;
	m_SW_lat.y = getd12(buf+6);
	m_SW_lat.x = getd12(buf+18);
	m_SW_utm.x = getd12(buf+36);
	m_SW_utm.y = getd12(buf+48);

	// record 12 - NW quadrangle corner
	if (!GetRecord(buf)) return false;
	m_NW_lat.y = getd12(buf+6);
	m_NW_lat.x = getd12(buf+18);
	m_NW_utm.x = getd12(buf+36);
	m_NW_utm.y = getd12(buf+48);

	// record 13 - NE quadrangle corner
	if (!GetRecord(buf)) return false;
	m_NE_lat.y = getd12(buf+6);
	m_NE_lat.x = getd12(buf+18);
	m_NE_utm.x = getd12(buf+36);
	m_NE_utm.y = getd12(buf+48);

	// record 14 - SE quadrangle corner
	if (!GetRecord(buf)) return false;
	m_SE_lat.y = getd12(buf+6);
	m_SE_lat.x = getd12(buf+18);
	m_SE_utm.x = getd12(buf+36);
	m_SE_utm.y = getd12(buf+48);

	// Special exception: DLG for Hawai`i that says "NAD27" is actually in
	// Old Hawaiian Datum (OHD) - so check for it.
	if ((iUTMZone == 4 || iUTMZone == 5) && m_SW_utm.y < 3500000)
	{
		if (iDatum == EPSG_DATUM_NAD27)
			iDatum = EPSG_DATUM_OLD_HAWAIIAN;
	}

	// We now know enough to set the CRS.
	m_crs.SetSimple(true, iUTMZone, iDatum);

	// record 15 - category name, attribute format code, number of nodes...
	if (!GetRecord(buf)) return false;
	m_iNodes = geti6(buf + 24);
	m_iAreas = geti6(buf + 40);
	m_iLines = geti6(buf + 56);

	// allocate storage space
	m_nodes.resize(m_iNodes);
	m_areas.resize(m_iAreas);
	m_lines.resize(m_iLines);

	int total = m_iNodes + m_iAreas + m_iLines, elem = 0;

	// now read the nodes
	for (i = 0; i < m_iNodes; i++)
	{
		if (!GetRecord(buf)) return false;

		// do some safety checking
		if (buf[0] != 'N') break;		// make sure node starts with a N
		int id = geti6(buf + 1);
		if (id != i+1) break;		// got the right node number?

		m_nodes[i].m_p.x = getd12(buf+6);
		m_nodes[i].m_p.y = getd12(buf+18);
		m_nodes[i].m_iAttribs = geti6(buf + 48);
		int elements = geti6(buf + 36);
		int extra_records = ((elements*6) + 71) / 72 + (m_nodes[i].m_iAttribs>0);

		// linkage records
		for (int e = 0; e < extra_records; e++)
			if (!GetRecord(buf)) return false;

		if (progress_callback && (++elem % 20) == 0) progress_callback(elem * 100 / total);
	}

	// now read the areas
	for (i = 0; i < m_iAreas; i++)
	{
		if (!GetRecord(buf)) return false;

		// do some safety checking
		if (buf[0] != 'A') break;		// make sure area starts with a A
		int id = geti6(buf + 1);
		if (id != i+1) break;		// got the right area number?

		m_areas[i].m_p.x = getd12(buf+6);
		m_areas[i].m_p.y = getd12(buf+18);
		m_areas[i].m_iAttribs = geti6(buf + 48);
		int elements = geti6(buf + 36);
		int extra_records = ((elements*6) + 71) / 72 + (m_areas[i].m_iAttribs>0);

		// linkage records
		for (int e = 0; e < extra_records; e++)
			if (!GetRecord(buf)) return false;

		if (progress_callback && (++elem % 20) == 0) progress_callback(elem * 100 / total);
	}

	// now read the lines
	for (i = 0; i < m_iLines; i++)
	{
		if (!GetRecord(buf))
			return false;

		// do some safety checking
		if (buf[0] != 'L')
			break;		// make sure line starts with a L
		int id = geti6(buf + 1);
		if (id != i+1)
			break;		// got the right area number?

		m_lines[i].m_iNode1 = geti6(buf+6);
		m_lines[i].m_iNode2 = geti6(buf+12);
		m_lines[i].m_iLeftArea = geti6(buf+18);
		m_lines[i].m_iRightArea = geti6(buf+24);
		m_lines[i].m_iCoords = geti6(buf + 42);
		m_lines[i].m_iAttribs = geti6(buf + 48);

		// coordinate records
		m_lines[i].m_p.SetSize(m_lines[i].m_iCoords);
		int offset = 0;
		double x, y;
		for (int c = 0; c < m_lines[i].m_iCoords; c++)
		{
			if (c%3 == 0)
			{
				if (!GetRecord(buf))
					return false;
				offset = 0;
			}
			x = getd12(buf+offset);
			offset += 12;
			y = getd12(buf+offset);
			offset += 12;

			m_lines[i].m_p[c].x = x;
			m_lines[i].m_p[c].y = y;
		}

		// attribute records
		if (m_lines[i].m_iAttribs)
		{
			m_lines[i].m_attr.resize(m_lines[i].m_iAttribs);
			for (j = 0; j < m_lines[i].m_iAttribs; j++)
			{
				if (j%6 == 0)
				{
					if (!GetRecord(buf))
						return false;
					offset = 0;
				}
				m_lines[i].m_attr[j].m_iMajorAttr = geti6(buf+offset);
				offset += 6;
				m_lines[i].m_attr[j].m_iMinorAttr = geti6(buf+offset);
				offset += 6;
			}
		}

		if (progress_callback && (++elem % 20) == 0) progress_callback(elem * 100 / total);
	}

	// all done, close up
	fclose(m_fp);
	m_fp = NULL;
	return true;
}


//
// read a single record from the file
//
bool vtDLGFile::GetRecord(char *buf)
{
	if (m_bLFdelimited)
	{
		int c, count = 0;
		while ((c = fgetc(m_fp)) != 10)
		{
			if (c == EOF)
			{
				m_iError = DLG_ERR_READ;
				return false;
			}
			else
				buf[count++] = (char) c;
		}
		return true;
	}
	else
	{
		int read = fread(buf, 80, 1, m_fp);
		if (read != 1)
		{
			m_iError = DLG_ERR_READ;
			return false;
		}
		else
			return true;
	}
}


void vtDLGFile::OpenFile()
{
	m_fp = vtFileOpen(m_fname, "rb");
}

void vtDLGFile::CloseFile()
{
	fclose(m_fp);
}

DLGType vtDLGFile::GuessFileType()
{
	int i, j;

	int major = 0;
	for (i = 0; i < m_iLines; i++)
	{
		int attr = m_lines[i].m_iAttribs;
		for (j = 0; j < attr; j++)
		{
			major = m_lines[i].m_attr[j].m_iMajorAttr;
			if (major) break;
		}
		if (major) break;
	}

	switch (major / 10)
	{
	case 2:  return DLG_HYPSO;
	case 5:  return DLG_HYDRO;
	case 7:  return DLG_VEG;
	case 8:  return DLG_NONVEG;
	case 9:  return DLG_BOUNDARIES;
	case 15: return DLG_MARKERS;
	case 17: return DLG_ROAD;
	case 18: return DLG_RAIL;
	case 19: return DLG_MTF;
	case 20: return DLG_MANMADE;
	}
	return DLG_UNKNOWN;
}


const char *vtDLGFile::GetErrorMessage()
{
	switch (m_iError)
	{
	case DLG_ERR_FILE:
		return "Problem opening that file";
		break;
	case DLG_ERR_READ:
		return "Problem reading that file";
		break;
	case DLG_ERR_HEADER:
		return "Problem reading DLG-O header";
		break;
	case DLG_ERR_NODE:
		return "Problem reading a node entity from the DLG-O file";
		break;
	case DLG_ERR_AREA:
		return "Problem reading an area entity from the DLG-O file";
		break;
	case DLG_ERR_LINE:
		return "Problem reading a line entity from the DLG-O file";
		break;
	}
	return "Unknown error";
}


//
// helper - read a LF-delimited DEM and write it with fixed-length records
//
bool ConvertDLG_from_LFDelim(const char *fname_from, const char *fname_to)
{
	FILE *out = vtFileOpen(fname_to, "wb");
	if (!out) return false;

	char buf[160];

	vtDLGFile in;
	in.Read(fname_from);
	if (!in.m_iError) {
		in.OpenFile();
		while (in.GetRecord(buf))
		{
			fwrite(buf, 80, 1, out);
		}
		in.CloseFile();
	}
	fclose(out);
	return true;
}


