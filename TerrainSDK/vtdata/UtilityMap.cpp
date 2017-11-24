//
// UtilityMap.cpp
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdlib.h>
#include "UtilityMap.h"
#include "vtString.h"
#include "vtLog.h"
#include "Version.h"
#include "shapelib/shapefil.h"
#include "xmlhelper/easyxml.hpp"


//
// Make a polyline for this vtLine by using the points from each node.
//
void vtLine::MakePolyline(DLine2 &polyline)
{
	const uint num = m_poles.size();
	polyline.SetSize(num);
	for (uint i = 0; i < num; i++)
		polyline[i] = m_poles[i]->m_p;
}

///////////////////////////////////////////////////////////////////////////////

vtUtilityMap::vtUtilityMap()
{
	m_iNextAvailableID = 0;
}

vtUtilityMap::~vtUtilityMap()
{
	for (uint i = 0; i < m_Poles.size(); i++)
		delete m_Poles[i];
	m_Poles.clear();

	for (uint i = 0; i < m_Lines.size(); i++)
		delete m_Lines[i];
	m_Lines.clear();
}

vtPole *vtUtilityMap::ClosestPole(const DPoint2 &p)
{
	uint npoles = m_Poles.size();
	if (npoles == 0)
		return NULL;

	uint k, ret=0;
	double	dist, closest = 1E9;

	for (k = 0; k < npoles; k++)
	{
		dist = (p - m_Poles[k]->m_p).Length();
		if (dist < closest)
		{
			closest = dist;
			ret = k;
		}
	}
	return m_Poles[ret];
}

void vtUtilityMap::GetPoleExtents(DRECT &rect)
{
	if (m_Poles.empty())
		return;

	rect.SetInsideOut();

	const int size = m_Poles.size();
	for (int i = 0; i < size; i++)
	{
		vtPole *pole = m_Poles[i];
		rect.GrowToContainPoint(pole->m_p);
	}
}

bool vtUtilityMap::WriteOSM(const char *pathname)
{
	FILE *fp = fopen(pathname, "wb");
	if (!fp)
		return false;

	// OSM only understands Geographic WGS84, so convert to that.
	vtCRS wgs84_geo;
	wgs84_geo.SetGeogCSFromDatum(EPSG_DATUM_WGS84);
	ScopedOCTransform trans(CreateCoordTransform(&m_crs, &wgs84_geo));
	if (!trans)
	{
		VTLOG1(" Couldn't transform coordinates\n");
		return false;
	}

	fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(fp, "<osm version=\"0.6\" generator=\"VTP %s\">\n", VTP_VERSION);

	for (uint i = 0; i < m_Poles.size(); i++)
	{
		const vtPole *pole = m_Poles[i];
		DPoint2 p = pole->m_p;
		trans->Transform(1, &p.x, &p.y);

		fprintf(fp, " <node id=\"%d\" lat=\"%.8lf\" lon=\"%.8lf\" version=\"1\">\n",
			pole->m_id, p.y, p.x);

		fprintf(fp, "  <tag k=\"power\" v=\"tower\"/>\n");

		const int num_tags = pole->NumTags();
		for (uint j = 0; j < pole->NumTags(); j++)
		{
			fprintf(fp, "  tag k=\"%s\" v=\"%s\"/>\n",
				(const char *) pole->GetTag(j)->name,
				(const char *) pole->GetTag(j)->value);
		}
		fprintf(fp, " </node>\n");
	}
	for (uint i = 0; i < m_Lines.size(); i++)
	{
		const vtLine *line = m_Lines[i];
		const uint num_poles = line->m_poles.size();
		const uint num_tags = line->NumTags();

		fprintf(fp, " <way id=\"%d\" version=\"1\">\n", line->m_id);

		for (uint j = 0; j < num_poles; j++)
			fprintf(fp, "  <nd ref=\"%d\"/>\n", line->m_poles[j]->m_id);

		fprintf(fp, "  <tag k=\"power\" v=\"line\"/>\n");

		for (uint j = 0; j < line->NumTags(); j++)
		{
			fprintf(fp, "  tag k=\"%s\" v=\"%s\"/>\n",
				(const char *) line->GetTag(j)->name,
				(const char *) line->GetTag(j)->value);
		}
		fprintf(fp, " </way>\n");
	}
	fprintf(fp, "</osm>\n");
	fclose(fp);

	return true;
}

////////////////////////////////////////////////////////////////////////
// Visitor class, for XML parsing of an OpenStreetMap file.
//

#include <map>

struct OSMNode {
	DPoint2 p;
	bool signal_lights;
	vtPole *pole;
};

class UtilOSMVisitor : public XMLVisitor
{
public:
	UtilOSMVisitor(vtUtilityMap *util);
	void startElement(const char *name, const XMLAttributes &atts);
	void endElement(const char *name);
	void data(const char *s, int length) {}	// OSM doesn't use actual XML data
	void SetSignalLights();

	vtUtilityMap *m_util_layer;

	vtPole *m_pole;
	vtLine *m_line;

private:
	void StartPowerPole();
	void MakePowerLine();
	void ParseOSMTag(const vtString &key, const vtString &value);

	enum ParseState {
		PS_NONE,
		PS_NODE,
		PS_WAY
	} m_state;

	typedef std::map<int, OSMNode> NodeMap;
	NodeMap m_nodes;
	std::vector<int> m_refs;
	int			m_id;
};

UtilOSMVisitor::UtilOSMVisitor(vtUtilityMap *util) : m_state(PS_NONE)
{
	m_util_layer = util;

	// OSM is always in Geo WGS84
	vtCRS crs;
	crs.SetWellKnownGeogCS("WGS84");
	m_util_layer->SetCRS(crs);
}

void UtilOSMVisitor::startElement(const char *name, const XMLAttributes &atts)
{
	const char *val;

	if (m_state == 0)
	{
		if (!strcmp(name, "node"))
		{
			DPoint2 p;

			val = atts.getValue("id");
			if (val)
				m_id = atoi(val);
			else
				m_id = -1;	// Shouldn't happen.

			val = atts.getValue("lon");
			if (val)
				p.x = atof(val);

			val = atts.getValue("lat");
			if (val)
				p.y = atof(val);

			OSMNode node;
			node.p = p;
			m_nodes[m_id] = node;

			m_state = PS_NODE;

			m_pole = NULL;
		}
		else if (!strcmp(name, "way"))
		{
			m_refs.clear();
			m_state = PS_WAY;
			val = atts.getValue("id");
			if (val)
				m_id = atoi(val);
			else
				m_id = -1;	// Shouldn't happen.

			// Defaults
			m_line = NULL;
		}
	}
	else if (m_state == PS_NODE && !strcmp(name, "tag"))
	{
		vtString key, value;

		val = atts.getValue("k");
		if (val)
			key = val;

		val = atts.getValue("v");
		if (val)
			value = val;

		if (key == "power" && value == "tower")
			StartPowerPole();

		// Node key/value
		else if (key == "highway")
		{
			if (value == "traffic_signals")
			{
				m_nodes[m_nodes.size()-1].signal_lights = true;
			}
		}
		else if (m_pole)
		{
			// Add all node tags for power towers
			m_pole->AddTag(key, value);
		}
	}
	else if (m_state == PS_WAY)
	{
		if (!strcmp(name, "nd"))
		{
			val = atts.getValue("ref");
			if (val)
			{
				int ref = atoi(val);
				m_refs.push_back(ref);
			}
		}
		else if (!strcmp(name, "tag"))
		{
			vtString key, value;

			val = atts.getValue("k");
			if (val)
				key = val;

			val = atts.getValue("v");
			if (val)
				value = val;

			if (m_line)
				m_line->AddTag(key, value);
			else
				ParseOSMTag(key, value);
		}
	}
}

void UtilOSMVisitor::endElement(const char *name)
{
	if (m_state == PS_NODE && !strcmp(name, "node"))
		m_state = PS_NONE;
	else if (m_state == PS_WAY && !strcmp(name, "way"))
		m_state = PS_NONE;
}

void UtilOSMVisitor::ParseOSMTag(const vtString &key, const vtString &value)
{
	if (key == "height")
	{
		// TODO: m_fHeight = atof((const char *)value);
	}
	if (key == "power" && value == "line")
		MakePowerLine();
}

void UtilOSMVisitor::StartPowerPole()
{
	OSMNode &node = m_nodes[m_id];

	m_pole = m_util_layer->AddNewPole();
	m_pole->m_id = m_id;
	m_pole->m_p = node.p;

	node.pole = m_pole;
}

void UtilOSMVisitor::MakePowerLine()
{
	m_line = m_util_layer->AddNewLine();

	m_line->m_poles.resize(m_refs.size());
	for (uint r = 0; r < m_refs.size(); r++)
	{
		int idx = m_refs[r];

		// Look for that node by id; if we don't find it, then it wasn't a tower;
		// it was probably a start or end point at a non-tower feature.
		NodeMap::iterator it = m_nodes.find(m_id);
		if (it != m_nodes.end())
		{
			// Connect to a known pole.
			m_line->m_poles[r] = m_nodes[idx].pole;
		}
		else
		{
			// We need to make a new pole node.
			OSMNode &node = m_nodes[idx];

			m_pole = m_util_layer->AddNewPole();
			m_pole->m_id = idx;
			m_pole->m_p = node.p;

			node.pole = m_pole;

			// Then we can connect it
			m_line->m_poles[r] = m_pole;
		}
	}
}

bool vtUtilityMap::ReadOSM(const char *pathname, bool progress_callback(int))
{
	// Avoid trouble with '.' and ',' in Europe
	//  OSM always has English punctuation
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	UtilOSMVisitor visitor(this);
	try
	{
		readXML(pathname, visitor, progress_callback);
	}
	catch (xh_exception &ex)
	{
		VTLOG1(ex.getFormattedMessage().c_str());
		return false;
	}
	return true;
}

void vtUtilityMap::SetCRS(const vtCRS &crs)
{
	m_crs = crs;
}

bool vtUtilityMap::TransformTo(vtCRS &crs)
{
	// Convert from (usually, Wgs84 Geographic) to what we need.
	ScopedOCTransform trans(CreateCoordTransform(&m_crs, &crs));
	if (!trans)
	{
		VTLOG1(" Couldn't transform coordinates\n");
		return false;
	}
	for (uint i = 0; i < m_Poles.size(); i++)
	{
		vtPole *pole = m_Poles[i];
		trans->Transform(1, &pole->m_p.x, &pole->m_p.y);
	}

	// Adopt new CRS
	m_crs = crs;

	return true;
}

void vtUtilityMap::Offset(const DPoint2 &delta)
{
	for (uint i = 0; i < m_Poles.size(); i++)
	{
		vtPole *pole = m_Poles[i];
		pole->m_p += delta;
	}
}

