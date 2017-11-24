//
// AttribMap.cpp
//
// This class gives you the ability to load in a colored bitmap representating
// a geographical coverage, and query it for attribute given a world coordinate.
//
// Copyright (c) 2001-2010 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// To construct, you must provide both the coverage bitmap and
// the filename of a file which describes the coverage, in the format
// of the following example:
//
/*
west: 176764
east: 310954
south: 2091656
north: 2244376
types: 3
type 1 ff00ff <optional description, ignored>
... for each type
*/

#include <vtlib/vtlib.h>
#include "vtdata/FilePath.h"
#include <stdio.h>
#include "AttribMap.h"

AttribMap::AttribMap()
{
}

bool AttribMap::Load(const char *filename_attrib, const char *filename_png)
{
	bool result = ReadPNG(filename_png);
	if (!result)
		return false;

	m_AttribTable = NULL;

	// also read attribute description file
	FILE *fp = vtFileOpen(filename_attrib, "r");
	if (!fp)
		return false;

	if (fscanf(fp, "west: %d\n", &m_xmin) != 1) return false;
	if (fscanf(fp, "east: %d\n", &m_xmax) != 1) return false;
	if (fscanf(fp, "south: %d\n", &m_ymin) != 1) return false;
	if (fscanf(fp, "north: %d\n", &m_ymax) != 1) return false;

	char buf[80];
	int att;
	uint rgb;

	if (fscanf(fp, "types: %d\n", &m_iNumAttribs) != 1) return false;
	m_AttribTable = new AttribTableEntry[m_iNumAttribs];
	for (int i = 0; i < m_iNumAttribs; i++)
	{
		if (fgets(buf, 80, fp) == NULL) return false;
		if (sscanf(buf, "type %d %x ", &att, &rgb) != 2) return false;
		m_AttribTable[i].attrib = att-1;
		m_AttribTable[i].rgb = rgb;
	}
	fclose(fp);
	return true;
}

AttribMap::~AttribMap()
{
	delete m_AttribTable;
}

int AttribMap::GetAttribute(float utm_x, float utm_y)
{
	if (!m_AttribTable) return -1;

	int x_offset = (int) (((float)(utm_x - m_xmin) / (m_xmax - m_xmin)) * GetWidth());
	int y_offset = (int) (((float)(m_ymax - utm_y) / (m_ymax - m_ymin)) * GetHeight());

	uint rgb = GetPixel24(x_offset, y_offset);
	for (int i = 0; i < m_iNumAttribs; i++)
	{
		if (rgb == m_AttribTable[i].rgb)
			return m_AttribTable[i].attrib;
	}
	return -1;
}


