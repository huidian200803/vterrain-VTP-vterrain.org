//
// Structure.cpp
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdio.h>
#include "Building.h"
#include "DataPath.h"
#include "Fence.h"
#include "FilePath.h"
#include "Structure.h"
#include "StructArray.h"
#include "vtLog.h"

vtStructInstance::vtStructInstance() : vtStructure()
{
	SetType(ST_INSTANCE);

	m_p.Set(0, 0);
	m_fRotation = 0.0f;
	m_fScale = 1.0f;

	m_pItem = NULL;
}

/**
 * Asignment operator, which makes an explicit copy of the structure's
 *  description tags.
 */
vtStructInstance &vtStructInstance::operator=(const vtStructInstance &v)
{
	// copy parent data
	vtStructure::CopyFrom(v);

	// copy class data
	m_p = v.m_p;
	m_fRotation = v.m_fRotation;
	m_fScale = v.m_fScale;
	m_pItem = v.m_pItem;
	return *this;
}

void vtStructInstance::WriteXML(GZOutput &out, bool bDegrees) const
{
	const char *coord_format = "%.9lg";	// up to 9 significant digits

	gfprintf(out, "\t<Imported");
	if (m_fElevationOffset != 0.0f)
		gfprintf(out, " ElevationOffset=\"%.2f\"", m_fElevationOffset);
	if (m_bAbsolute)
		gfprintf(out, " Absolute=\"true\"");
	gfprintf(out, ">\n");

	// first write the placement
	gfprintf(out, "\t\t<Location>\n");
	gfprintf(out, "\t\t\t<gml:coordinates>");
	gfprintf(out, coord_format, m_p.x);
	gfprintf(out, ",");
	gfprintf(out, coord_format, m_p.y);
	gfprintf(out, "</gml:coordinates>\n");
	gfprintf(out, "\t\t</Location>\n");

	if (m_fRotation != 0.0f)
	{
		gfprintf(out, "\t\t<Rotation>%g</Rotation>\n", m_fRotation);
	}
	if (m_fScale != 1.0f)
	{
		gfprintf(out, "\t\t<Scale>%g</Scale>\n", m_fScale);
	}
	WriteTags(out);
	gfprintf(out, "\t</Imported>\n");
}

bool vtStructInstance::GetExtents(DRECT &rect) const
{
	// we have no way (yet) of knowing the extents of an external
	// reference, so just give a placeholder of a single point.
	rect.SetRect(m_p.x, m_p.y, m_p.x, m_p.y);
	return true;
}

bool vtStructInstance::IsContainedBy(const DRECT &rect) const
{
	return rect.ContainsPoint(m_p);
}

double vtStructInstance::DistanceToPoint(const DPoint2 &p, float fMaxRadius) const
{
	// simple distance from the origin of this instance to the given point
	return (m_p - p).Length();
}


///////////////////////////////////////////////////////////////////////

vtStructure::vtStructure()
{
	m_type = ST_NONE;
	m_fElevationOffset = 0.0f;
	m_bAbsolute = false;
#ifdef VIAVTDATA
	m_bIsVIAContributor = false;
	m_bIsVIATarget = false;
#endif
}

vtStructure::~vtStructure()
{
	m_type = ST_NONE;
}

void vtStructure::CopyFrom(const vtStructure &v)
{
	// copy parent members
	CopyTagsFrom(v);

	// copy structure members
	m_type = v.m_type;
	m_fElevationOffset = v.m_fElevationOffset;
	m_bAbsolute = v.m_bAbsolute;
}

void vtStructure::WriteTags(GZOutput &out) const
{
	// now write all extra tags (attributes) for this structure
	for (uint i = 0; i < NumTags(); i++)
	{
		const vtTag *tag = GetTag(i);
		gfprintf(out, "\t\t<%s>%s</%s>\n", (const char *)tag->name,
			(const char *) EscapeStringForXML(tag->value), (const char *)tag->name);
	}
}
