//
// AttribMap.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ATTRIBMAPH
#define ATTRIBMAPH

#include "vtdata/vtDIB.h"

struct AttribTableEntry {
	uint rgb;
	int attrib;
};

/**
 * This class implements a bitmap which has its colors mapped onto
 * specific integer attributes.  For example, Red pixels can mean
 * attribute value 1, Orange can mean 2, and so forth.
 */
class AttribMap : public vtDIB
{
public:
	AttribMap();
	~AttribMap();

	bool Load(const char *filename_attrib, const char *filename_png);
	int GetAttribute(float utm_x, float utm_y);

	int m_iNumAttribs;
	int m_xmin, m_xmax, m_ymin, m_ymax;
	AttribTableEntry *m_AttribTable;
};

#endif	// ATTRIBMAPH

