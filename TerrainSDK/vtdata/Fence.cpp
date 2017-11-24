//
// Fence.cpp
//
// Implemented vtFence;
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "Fence.h"
#include "shapelib/shapefil.h"

#define LONGEST_FENCE 2000   // in meters

///////////////////

void vtLinearParams::Defaults()
{
	ApplyStyle(FS_WOOD_POSTS_WIRE);
}

void vtLinearParams::Blank()
{
	m_PostType = "none";
	m_fPostSpacing = 1.0f;
	m_fPostHeight = 1.0f;
	m_fPostWidth = 0.0f;
	m_fPostDepth = 0.0f;
	m_PostExtension = "none";
	//
	m_iConnectType = 0;
	m_ConnectMaterial = "none";
	m_fConnectTop = 1.0f;
	m_fConnectBottom = 0.0f;
	m_fConnectWidth = 0.0f;
	m_iConnectSlope = 90;
	m_bConstantTop = false;
}

void vtLinearParams::ApplyStyle(vtLinearStyle style)
{
	Blank();
	switch (style)
	{
	case FS_WOOD_POSTS_WIRE:
		//
		m_PostType = "wood";
		m_fPostSpacing = 2.5f;
		m_fPostHeight = 1.2f;
		m_fPostWidth = 0.13f;
		m_fPostDepth = 0.13f;
		//
		m_iConnectType = 1;
		m_fConnectTop = 1.1f;
		m_fConnectBottom = 0.5f;
		m_fConnectWidth = 0.0f;
		break;
	case FS_METAL_POSTS_WIRE:
		//
		m_PostType = "steel";
		m_fPostSpacing = 2.5f;
		m_fPostHeight = 1.2f;
		m_fPostWidth = 0.05f;
		m_fPostDepth = 0.05f;
		//
		m_iConnectType = 1;
		m_fConnectTop = 1.1f;
		m_fConnectBottom = 0.5f;
		m_fConnectWidth = 0.0f;
		break;
	case FS_METAL_POSTS_HOGWIRE:
		//
		m_PostType = "steel";
		m_fPostSpacing = 2.5f;
		m_fPostHeight = 1.2f;
		m_fPostWidth = 0.05f;
		m_fPostDepth = 0.05f;
		//
		m_iConnectType = 2;
		m_ConnectMaterial = "Woven wire";
		m_fConnectTop = 1.2f;
		m_fConnectBottom = 0.0f;
		m_fConnectWidth = 0.0f;
		break;
	case FS_CHAINLINK:
		//
		m_PostType = "steel";
		m_fPostSpacing = 3.0f;
		m_fPostHeight = 2.0f;
		m_fPostWidth = 0.05f;
		m_fPostDepth = 0.05f;
		//
		m_iConnectType = 2;
		m_ConnectMaterial = "chain-link";
		m_fConnectTop = m_fPostHeight;
		m_fConnectBottom = 0.0f;
		m_fConnectWidth = 0.0f;
		break;
	case FS_SECURITY:
		//
		m_PostType = "steel";
		m_fPostSpacing = 3.0f;
		m_fPostHeight = 2.0f;
		m_fPostWidth = 0.05f;
		m_fPostDepth = 0.05f;
		m_PostExtension = "double";
		//
		m_iConnectType = 2;
		m_ConnectMaterial = "chain-link";
		m_fConnectTop = m_fPostHeight;
		m_fConnectBottom = 0.0f;
		m_fConnectWidth = 0.0f;
		break;
	case FS_DRYSTONE:
		//
		m_PostType = "none";
		//
		m_iConnectType = 2;
		m_ConnectMaterial = "drystone";
		m_fConnectTop = 1.5f;
		m_fConnectBottom = 0.0f;
		m_fConnectWidth = 0.3f;
		break;
	case FS_STONE:
		//
		m_PostType = "none";
		//
		m_iConnectType = 2;
		m_ConnectMaterial = "stone";
		m_fConnectTop = 1.5f;
		m_fConnectBottom = 0.0f;
		m_fConnectWidth = 0.3f;
		break;
	case FS_PRIVET:
		//
		m_PostType = "none";
		//
		m_iConnectType = 2;
		m_ConnectMaterial = "privet";
		m_fConnectTop = 1.5f;
		m_fConnectBottom = 0.0f;
		m_fConnectWidth = 0.5f;
		break;
	case FS_BERM:
		//
		m_PostType = "none";
		//
		m_iConnectType = 2;
		m_ConnectMaterial = "grass";
		m_fConnectTop = 1.5f;
		m_fConnectBottom = -1.0f;
		m_fConnectWidth = 1.0f;
		m_iConnectSlope = 60;
		break;
	case FS_RAILING_ROW:
		//
		m_PostType = "none";
		//
		m_iConnectType = 2;
		m_ConnectMaterial = "railing_pipe";
		m_fConnectTop = 0.70f;
		m_fConnectBottom = 0.0f;
		m_fConnectWidth = 0.0f;
		break;
	case FS_RAILING_CABLE:
		//
		m_PostType = "none";
		//
		m_iConnectType = 2;
		m_ConnectMaterial = "railing_wire";
		m_fConnectTop = 0.70f;
		m_fConnectBottom = 0.0f;
		m_fConnectWidth = 0.0f;
		break;
	case FS_RAILING_EU:
		//
		m_PostType = "none";
		//
		m_iConnectType = 2;
		m_ConnectMaterial = "railing_eu";
		m_fConnectTop = 0.70f;
		m_fConnectBottom = 0.0f;
		m_fConnectWidth = 0.0f;
		break;
	case FS_TOTAL:
		// keep picky compilers quiet.
		break;
	}
}

void vtLinearParams::WriteXML(GZOutput &out) const
{
	if (m_PostType != "none")
	{
		gfprintf(out, "\t\t<Posts Type=\"%s\" Spacing=\"%.2f\" Height=\"%.2f\" Size=\"%.2f,%.2f\"",
			(const char *)m_PostType, m_fPostSpacing, m_fPostHeight, m_fPostWidth, m_fPostDepth);
		if (m_PostExtension != "none")
		{
			gfprintf(out, " Extension=\"");
			gfprintf(out, (const char *) m_PostExtension);
			gfprintf(out, "\"");
		}
		gfprintf(out, " />\n");
	}
	if (m_iConnectType != 0)
	{
		gfprintf(out, "\t\t<Connect Type=\"%d\"", m_iConnectType);
		if (m_iConnectType == 1)	// wire
		{
			// no wire parameters (yet)
		}
		if (m_iConnectType == 2)	// simple
		{
			if (m_ConnectMaterial != "none")
				gfprintf(out, " Material=\"%s\"", (const char *) m_ConnectMaterial);
			gfprintf(out, " Top=\"%.2f\"", m_fConnectTop);
			if (m_fConnectBottom != 0.0f)
				gfprintf(out, " Bottom=\"%.2f\"", m_fConnectBottom);
			if (m_fConnectWidth != 0.0f)
				gfprintf(out, " Width=\"%.2f\"", m_fConnectWidth);
			if (m_iConnectSlope != 90)
				gfprintf(out, " Slope=\"%d\"", m_iConnectSlope);
		}
		if (m_iConnectType == 3)	// profile
		{
			if (m_ConnectMaterial != "none")
				gfprintf(out, " Profile=\"%s\"", (const char *) m_ConnectProfile);
		}
		if (m_bConstantTop)
			gfprintf(out, " ConstantTop=\"true\"", m_iConnectSlope);
		gfprintf(out, " />\n");
	}
}

bool vtLinearParams::operator==(const vtLinearParams &rhs) const
{
	if (m_PostType != "none" || rhs.m_PostType != "none")
	{
		// compare posts
		if (m_PostType != rhs.m_PostType ||
			m_fPostWidth != rhs.m_fPostWidth ||
			m_fPostDepth != rhs.m_fPostDepth ||
//			m_fPostHeight != rhs.m_fPostHeight ||
//			m_fPostSpacing != rhs.m_fPostSpacing ||
			m_PostExtension != rhs.m_PostExtension)
			return false;
	}
	if (m_iConnectType != 0 || rhs.m_iConnectType != 0)
	{
		// compare connection
		if (m_iConnectType != rhs.m_iConnectType)
			return false;
		if (m_iConnectType == 2)	// simple
		{
			if (m_fConnectTop != rhs.m_fConnectTop ||
				m_fConnectBottom != rhs.m_fConnectBottom ||
				m_fConnectWidth != rhs.m_fConnectWidth)
				return false;
		}
		if (m_iConnectType == 3)	// profile
		{
			if (m_ConnectProfile != rhs.m_ConnectProfile)
				return false;
		}
	}
	return true;
}

///////////////////

vtFence::vtFence() : vtStructure()
{
	SetType(ST_LINEAR);	// structure type
	m_Params.Defaults();
}

vtFence &vtFence::operator=(const vtFence &v)
{
	// copy parent data
	vtStructure::CopyFrom(v);

	// copy class data
	m_Params = v.m_Params;
	m_pFencePts = v.m_pFencePts;
	return *this;
}


void vtFence::AddPoint(const DPoint2 &epos)
{
	uint numfencepts = m_pFencePts.GetSize();

	if (numfencepts == 0)
		m_pFencePts.Append(epos);
	else
	{
		// check distance
		const DPoint2 &LastPt = m_pFencePts[numfencepts - 1];
		double distance = (LastPt - epos).Length();
		if (distance <= LONGEST_FENCE)
			m_pFencePts.Append(epos);
	}
}

void vtFence::ApplyStyle(vtLinearStyle style)
{
	m_Params.ApplyStyle(style);
}

bool vtFence::GetExtents(DRECT &rect) const
{
	uint size = m_pFencePts.GetSize();

	if (size == 0)
		return false;

	rect.SetInsideOut();
	for (uint i = 0; i < size; i++)
		rect.GrowToContainPoint(m_pFencePts[i]);
	return true;
}

/**
 * Given a 2d point, return the fence point closest to it.
 */
void vtFence::GetClosestPoint(const DPoint2 &point, DPoint2 &closest_point)
{
	double dist, closest = 1E8;

	int size = m_pFencePts.GetSize();
	for (int i = 0; i < size; i++)
	{
		dist = (m_pFencePts[i] - point).Length();
		if (dist < closest)
		{
			closest = dist;
			closest_point = m_pFencePts[i];
		}
	}
}

/**
 * Given a 2d point, return the distance to the closest point on the centerline.
 */
double vtFence::GetDistanceToLine(const DPoint2 &point)
{
	uint size = m_pFencePts.GetSize();

	// simple case, one point, just get distance to it
	if (size == 1)
		return (point-m_pFencePts[0]).Length();

	double dist, closest = 1E8;
	for (uint i = 0; i < size-1; i++)
	{
		dist = DistancePointToLine(m_pFencePts[i], m_pFencePts[i+1], point);
		if (dist < closest)
			closest = dist;
	}
	return closest;
}

int vtFence::GetNearestPointIndex(const DPoint2 &point, double &fDist)
{
	uint size = m_pFencePts.GetSize();
	int closest = -1;
	fDist = 1E8;

	for (uint i = 0; i < size; i++)
	{
		double dist = (point - m_pFencePts[i]).Length();
		if (dist < fDist)
		{
			fDist = dist;
			closest = i;
		}
	}
	return closest;
}

void vtFence::WriteXML(GZOutput &out, bool bDegrees) const
{
	int i;
	const char *coord_format = "%.9lg";	// up to 9 significant digits

	// Write the XML to describe this fence to a built-structure XML file.
	gfprintf(out, "\t<Linear>\n");

	gfprintf(out, "\t\t<Path>\n");
	gfprintf(out, "\t\t\t<gml:coordinates>");
	int points = m_pFencePts.GetSize();
	for (i = 0; i < points; i++)
	{
		DPoint2 p = m_pFencePts[i];
		gfprintf(out, coord_format, p.x);
		gfprintf(out, ",");
		gfprintf(out, coord_format, p.y);
		if (i != points-1)
			gfprintf(out, " ");
	}
	gfprintf(out, "</gml:coordinates>\n");
	gfprintf(out, "\t\t</Path>\n");

	m_Params.WriteXML(out);

	WriteTags(out);
	gfprintf(out, "\t</Linear>\n");
}


//////////////////////////////////////////////////////////////////

bool vtFence::IsContainedBy(const DRECT &rect) const
{
	return rect.ContainsLine(m_pFencePts);
}


//----------------------------------------------------------------------------
// Helpers

bool LoadFLine2FromSHP(const char *fname, FLine2 &prof)
{
	SHPHandle hSHP = SHPOpen(fname, "rb");
	if (hSHP == NULL)
		return false;

	int nElems, nShapeType;
	SHPGetInfo(hSHP, &nElems, &nShapeType, NULL, NULL);
	if (!nElems || nShapeType != SHPT_ARC)
		return false;

	SHPObject *psShape = SHPReadObject(hSHP, 0);
	int verts = psShape->nVertices;
	prof.SetSize(verts);
	for (int j = 0; j < verts; j++)
	{
		prof[j].x = (float) psShape->padfX[j];
		prof[j].y = (float) psShape->padfY[j];
	}
	SHPDestroyObject(psShape);
	SHPClose(hSHP);
	return true;
}

bool SaveFLine2ToSHP(const char *fname, const FLine2 &prof)
{
	SHPHandle hSHP = SHPCreate(fname, SHPT_ARC);
	if (!hSHP)
		return false;

	uint size = prof.GetSize();
	double* dX = new double[size];
	double* dY = new double[size];

	for (uint j = 0; j < size; j++) //for each vertex
	{
		dX[j] = prof[j].x;
		dY[j] = prof[j].y;
	}
	SHPObject *obj = SHPCreateSimpleObject(SHPT_ARC, size, dX, dY, NULL);

	delete [] dX;
	delete [] dY;

	SHPWriteObject(hSHP, -1, obj);
	SHPDestroyObject(obj);

	SHPClose(hSHP);
	return true;
}

