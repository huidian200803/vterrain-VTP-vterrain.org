//
// LULC.cpp
// read a LULC GIRAS file
//
// Copyright (c) 2001-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <string.h>
#include <stdlib.h>

#include "LULC.h"
#include "FilePath.h"

///////////////////////////////////
//
// helpers
//
int geti5(const char *buf)
{
	char buf2[6];
	strncpy(buf2, buf, 5);
	buf2[5] = 0;
	return atoi(buf2);
}

int geti10(const char *buf)
{
	char buf2[11];
	strncpy(buf2, buf, 10);
	buf2[10] = 0;
	return atoi(buf2);
}

double getdegree(const char *buf)
{
	char buf2[11];
	strncpy(buf2, buf, 10);
	buf2[10] = 0;
	double seconds = (double)atoi(buf2+8);
	buf2[8] = 0;
	double minutes = (double)atoi(buf2+6);
	buf2[6] = 0;
	double degrees = (double)atoi(buf2+3);

	double fDegrees = degrees + (minutes / 60.0) + (seconds / 3600.0);

	// "West longitude values are given as positive numbers,
	//		increasing in value from east to west."
	if (fDegrees > 50.0) fDegrees = -fDegrees;

	return fDegrees;
}


// A LULC file has:
//
// 80-character records
// 3 types of data-element field:
//	  a 16-bit integer, coded as 5 ASCII characters
//	  a 32-bit integer, coded as 10 ASCII characters
//	  a string
// Contains 6 requried subfiles:
//
// Frame of reference given by 6 control point in lat-lon fomat,
//  all other coordinates are 16-bit integers


vtLULCFile::vtLULCFile(const char *fname)
{
	char buf[80];
	uint i;

	m_pNext = NULL;
	m_iError = 0;

	FILE *fp = vtFileOpen(fname, "rb");
	if (!fp)
	{
		m_iError = LULC_ERR_FILE;
		return;
	}

	// subfile A - header
	if (!GetRecord(fp, buf)) return;
	geti10(buf+0);		// iNumArcs
	geti10(buf+10);		// iNumCoords
	geti10(buf+20);		// iNumPolys
	m_iNumSections = geti5(buf+40);
	geti5(buf+45);		// iMapType, 1 = LULC
	geti5(buf+50);		// iSubfileLength
	geti5(buf+55);		// iProjectionCode
	geti10(buf+60);		// iScaleDenominator
	geti10(buf+70);		// iMapDate

	// extent of control points, in local coordinates
	if (!GetRecord(fp, buf)) return;
	m_cMin.x = (short) geti5(buf+0);
	m_cMin.y = (short) geti5(buf+5);
	m_cMax.x = (short) geti5(buf+10);
	m_cMax.y = (short) geti5(buf+15);

	// local coordinates of control points
	for (i = 0; i < 6; i++)
	{
		m_cCorners[i].x = (short) geti5(buf+20+i*10);
		m_cCorners[i].y = (short) geti5(buf+20+i*10+5);
	}

	// latitude and longitude of control points
	if (!GetRecord(fp, buf)) return;
	m_Corners[0].y = getdegree(buf+0);
	m_Corners[0].x = getdegree(buf+10);
	m_Corners[1].y = getdegree(buf+20);
	m_Corners[1].x = getdegree(buf+30);
	m_Corners[2].y = getdegree(buf+40);
	m_Corners[2].x = getdegree(buf+50);
	m_Corners[3].y = getdegree(buf+60);
	m_Corners[3].x = getdegree(buf+70);
	if (!GetRecord(fp, buf)) return;
	m_Corners[4].y = getdegree(buf+0);
	m_Corners[4].x = getdegree(buf+10);
	m_Corners[5].y = getdegree(buf+20);
	m_Corners[5].x = getdegree(buf+30);

	// try to determine the mapping from local to latlon
	SetupMapping();

	geti5(buf+40);		// iNad
	int iNumCharTitle  = geti5(buf+45);
	if (iNumCharTitle > 64) iNumCharTitle = 64;
	geti5(buf+50);		// iLengthOfFAP
	geti10(buf+60);	// CreationDate

	if (!GetRecord(fp, buf)) return;
	char title[65];
	strncpy(title, buf, iNumCharTitle);

	// allocate sections
	m_pSection = new LULCSection[m_iNumSections];

	for (i = 0; i < m_iNumSections; i++)
		ReadSection(m_pSection + i, fp);

	fclose(fp);
}

//
// destructor
vtLULCFile::~vtLULCFile()
{
	delete [] m_pSection;
}

LULCSection::~LULCSection()
{
	delete [] m_pArc;
	delete [] m_pCoord;
	delete [] m_pPoly;
	delete m_pFAP;
}

LULCPoly::LULCPoly()
{
	m_iCoords = 0;
	m_p = NULL;
}

LULCPoly::~LULCPoly()
{
	delete [] m_p;
}

//
// try to determine the mapping from local to latlon
//
void vtLULCFile::SetupMapping()
{
	// this algorithm for doing a simple warp transformation is adapted
	// from the book 'Digital Image Warping', pages 52-56
	double x0 = m_cCorners[0].x;
	double y0 = m_cCorners[0].y;

	double x1 = m_cCorners[4].x;
	double y1 = m_cCorners[4].y;

	double x2 = m_cCorners[3].x;
	double y2 = m_cCorners[3].y;

	double x3 = m_cCorners[1].x;
	double y3 = m_cCorners[1].y;

	double dx1 = x1 - x2;
	double dx2 = x3 - x2;
	double dx3 = x0 - x1 + x2 - x3;
	double dy1 = y1 - y2;
	double dy2 = y3 - y2;
	double dy3 = y0 - y1 + y2 - y3;

	double a13 = (dx3 * dy2 - dy3 * dx2) / (dx1 * dy2 - dy1 * dx2);
	double a23 = (dx1 * dy3 - dy1 * dx3) / (dx1 * dy2 - dy1 * dx2);
	double a11 = x1 - x0 + a13 * x1;
	double a21 = x3 - x0 + a23 * x3;
	double a31 = x0;
	double a12 = y1 - y0 + a13 * y1;
	double a22 = y3 - y0 + a23 * y3;
	double a32 = y0;
	double a33 = 1.0f;

	DMatrix4 forwards4, inverse4;
	forwards4.Identity();
	forwards4.Set(0, 0, a11);
	forwards4.Set(0, 1, a21);
	forwards4.Set(0, 2, a31);
	forwards4.Set(1, 0, a12);
	forwards4.Set(1, 1, a22);
	forwards4.Set(1, 2, a32);
	forwards4.Set(2, 0, a13);
	forwards4.Set(2, 1, a23);
	forwards4.Set(2, 2, a33);

	inverse4.Invert(forwards4);
	m_transform.SetByMatrix4(inverse4);
}


void vtLULCFile::ReadSection(LULCSection *pSection, FILE *fp)
{
	char buf[80];
	uint i;

	// subfile B - section header
	if (!GetRecord(fp, buf)) return;
	geti5(buf+0);		// iSection
	pSection->m_iNumArcs = geti5(buf+5);
	pSection->m_iNumCoords = geti5(buf+10) / 2;
	pSection->m_iNumPolys = geti5(buf+15);
	pSection->m_iLengthOfFAP = geti5(buf+20);
	pSection->m_iNumNodes = geti5(buf+50);

	// allocate arcs
	pSection->m_pArc = new LULCArc[pSection->m_iNumArcs];

	// subfile C - arc records
	for (i = 0; i < pSection->m_iNumArcs; i++)
	{
		if (!GetRecord(fp, buf)) return;
		geti5(buf+0);	// iArcNum

		if (i == 0)
			pSection->m_pArc[i].first_coord = 0;
		else
			pSection->m_pArc[i].first_coord =
			pSection->m_pArc[i-1].last_coord;
		// There are 2 coords per point
		pSection->m_pArc[i].last_coord = (short) geti5(buf+5) / 2;
		pSection->m_pArc[i].PL = (short) geti5(buf+10);
		pSection->m_pArc[i].PR = (short) geti5(buf+15);
		pSection->m_pArc[i].PAL = geti10(buf+20);
		pSection->m_pArc[i].PAR = geti10(buf+30);
		pSection->m_pArc[i].Length = geti10(buf+60);
		pSection->m_pArc[i].StartNode = (short) geti5(buf+70);
		pSection->m_pArc[i].FinishNode = (short) geti5(buf+75);
	}

	// allocate coords
	pSection->m_pCoord = new Coord[pSection->m_iNumCoords];

	// subfile D - coordinates
	for (i = 0; i < pSection->m_iNumCoords; i++)
	{
		int index = (i % 8);
		if (index == 0)
		{
			// read another record
			if (!GetRecord(fp, buf)) return;
		}
		pSection->m_pCoord[i].x = (short) geti5(buf+index*10);
		pSection->m_pCoord[i].y = (short) geti5(buf+index*10+5);
	}

	// allocate polys
	pSection->m_pPoly = new LULCPoly[pSection->m_iNumPolys];

	// subfile E - polygon records
	for (i = 0; i < pSection->m_iNumPolys; i++)
	{
		if (!GetRecord(fp, buf)) return;
		geti5(buf+0);		// iPolyNum

		if (i == 0)
			pSection->m_pPoly[i].first_arc = 0;
		else
			pSection->m_pPoly[i].first_arc =
			pSection->m_pPoly[i-1].last_arc;
		pSection->m_pPoly[i].last_arc = (short) geti5(buf+5);
		pSection->m_pPoly[i].Attribute = geti10(buf+20);
		pSection->m_pPoly[i].PerimeterLength = geti10(buf+60);
		pSection->m_pPoly[i].NumIslands = (short) geti5(buf+70);
		pSection->m_pPoly[i].SurroundingPoly = (short) geti5(buf+75);
	}

	// allocate FAP
	pSection->m_pFAP = new short[pSection->m_iLengthOfFAP];

	// subfile F - FAP (file of arcs by polygon)
	for (i = 0; i < pSection->m_iLengthOfFAP; i++)
	{
		int index = (i % 16);
		if (index == 0)
		{
			// read another record
			if (!GetRecord(fp, buf)) return;
		}
		pSection->m_pFAP[i] = (short) geti5(buf+index*5);
	}
}


//
// read a single record from the file
//
int vtLULCFile::GetRecord(FILE *fp, char *buf)
{
	int read = fread(buf, 80, 1, fp);
	if (read != 1)
	{
		m_iError = LULC_ERR_READ;
		return 0;
	}
	else
		return 1;
}


// convert from "local" coordinates to latlon
void vtLULCFile::LocalToLatlon(Coord &local, DPoint2 &latlon)
{
	/* I'm not certain how to do this correctly.
	A close approximation might be to treat each section as its own irregular
	quadrilateral, and transform to latlon based on knowing the latlon
	coordinates of the corners.  That might be around 98% accurate.  It must
	be possible, because Maptitude does it with apparently no error at all. */

	// Use the quadrilateral-to-rectangle mapping transform that we set up
	//  earlier with SetupMapping()
	DPoint3 src, dst;
	src.x = local.x;
	src.y = local.y;
	src.z = 1.0f;

	m_transform.Transform(src, dst);

	// normalize output
	dst /= dst.z;

	latlon.x = m_Corners[0].x +	(dst.x / dst.z) * (m_Corners[3].x - m_Corners[0].x);
	latlon.y = m_Corners[0].y +	(dst.y / dst.z) * (m_Corners[3].y - m_Corners[0].y);
}


/*
 *
 * Return the attribute of the polygon at a specified point, or
 * -1 if there is no polygon there.
 *
 */
int vtLULCFile::FindAttribute(double world_x, double world_z)
{
	// for speed, remember the polygon that we found last time
	static LULCPoly *previous_poly = NULL;

	DPoint2 pos2;
	pos2.x = world_x;
	pos2.y = world_z;

	if (previous_poly)
	{
		if (CrossingsTest(previous_poly->m_p, previous_poly->m_iCoords, pos2))
			return previous_poly->Attribute;
	}
	for (uint s = 0; s < NumSections(); s++)
	{
		LULCSection *section = GetSection(s);

		for (uint p = 0; p < section->m_iNumPolys; p++)
		{
			LULCPoly *poly = section->m_pPoly+p;

			if (world_x < poly->xmin || world_x > poly->xmax ||
				world_z < poly->zmin || world_z > poly->zmax)
				continue;

			if (CrossingsTest(poly->m_p, poly->m_iCoords, pos2))
			{
				previous_poly = poly;
				return poly->Attribute;
			}
		}
	}
	previous_poly = NULL;
	return -1;
}


void vtLULCFile::ProcessLULCPolys()
{
	for (uint s = 0; s < NumSections(); s++)
	{
		LULCSection *section = GetSection(s);

		for (uint p = 0; p < section->m_iNumPolys; p++)
		{
			LULCPoly *poly = section->m_pPoly+p;

			ProcessLULCPoly(section, poly);
		}
	}
}

int most_count = 0;
int most_islands = 0;
#define MAX_ISLANDS		300

void vtLULCFile::ProcessLULCPoly(LULCSection *pSection, LULCPoly *pPoly)
{
	assert(pPoly->NumIslands < MAX_ISLANDS);

	DPoint2 coord[20000];
	int count = 0;
	DPoint2 island_stack[MAX_ISLANDS];
	int island_count = 0;

	for (int a = pPoly->first_arc; a < pPoly->last_arc; a++)
	{
		bool reverse_direction = false;

		int arc = pSection->m_pFAP[a];
		if (arc == 0)
		{
			// special: next set of arcs will describe an island
			island_stack[island_count] = coord[count-1];
			island_count++;
			continue;
		}
		if (arc < 0)
		{
			reverse_direction = true;
			arc = -arc;
		}
		// our arrays are 0-indexed, not 1-indexed
		LULCArc *pArc = pSection->m_pArc + (arc-1);

		// pArc->Length is in local geographical units, NOT number of coords
		if (reverse_direction)
		{
			for (int c = pArc->last_coord-1; c >= pArc->first_coord; c--)
				LocalToLatlon(pSection->m_pCoord[c], coord[count++]);
		}
		else
		{
			for (int c = pArc->first_coord; c < pArc->last_coord; c++)
				LocalToLatlon(pSection->m_pCoord[c], coord[count++]);
		}
	}
	if (count > most_count)
		most_count = count;

	// pop the island stack, retracing our route back to the polygon origin
	if (island_count > most_islands) most_islands = island_count;
	while (island_count)
	{
		coord[count] = island_stack[island_count-1];
		island_count--;
		count++;
	}

	pPoly->m_iCoords = count;
	pPoly->m_p = new DPoint2[count];
	pPoly->xmin = 1.0E9f;
	pPoly->zmin = 1.0E9f;
	pPoly->xmax = -1.0E9f;
	pPoly->zmax = -1.0E9f;
	for (int i = 0; i < count; i++)
	{
		pPoly->m_p[i].x = coord[i].x;
		if (coord[i].x < pPoly->xmin) pPoly->xmin = coord[i].x;
		if (coord[i].x > pPoly->xmax) pPoly->xmax = coord[i].x;
		pPoly->m_p[i].y = coord[i].y;
		if (coord[i].y < pPoly->zmin) pPoly->zmin = coord[i].y;
		if (coord[i].y > pPoly->zmax) pPoly->zmax = coord[i].y;
	}
}



const char *vtLULCFile::GetErrorMessage()
{
	switch (m_iError)
	{
	case LULC_ERR_FILE:
		return "Problem opening that file";
		break;
	case LULC_ERR_READ:
		return "Problem reading that file";
		break;
	case LULC_ERR_HEADER:
		return "Problem reading LULC header";
		break;
	}
	return "Unknown error";
}

