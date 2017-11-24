//
// Location classes
//
// Copyright (c) 2001-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "Location.h"
#include "xmlhelper/easyxml.hpp"
#include "vtdata/vtLog.h"
#include "vtdata/FilePath.h"

///////////////////////////////

vtLocationSaver::vtLocationSaver()
{
	m_pTransform = NULL;
	m_pConvertToWGS = NULL;
	m_pConvertFromWGS = NULL;
}

vtLocationSaver::vtLocationSaver(const vtString &fname)
{
	Read(fname);
}

vtLocationSaver::~vtLocationSaver()
{
	delete m_pConvertToWGS;
	delete m_pConvertFromWGS;
	Clear();
}

void vtLocationSaver::Clear()
{
	int i, num = m_loc.GetSize();
	for (i = 0; i < num; i++)
		delete m_loc[i];
	m_loc.Clear();
}

bool vtLocationSaver::Write(const vtString &fname_in)
{
	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	vtString fname;
	if (fname_in != "")
	{
		m_strFilename = fname_in;
		fname = fname_in;
	}
	else
		fname = m_strFilename;

	FILE *fp = vtFileOpen(fname, "wb");
	if (!fp) return false;

	int i, num = m_loc.GetSize();
	fprintf(fp, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	fprintf(fp, "<locations-file file-format-version=\"1.0\">\n");
	for (i = 0; i < num; i++)
	{
		vtLocation *loc = m_loc[i];
		fprintf(fp, "  <location>\n");

		// first, we must convert characters like <, >, and &
#if SUPPORT_WSTRING
		wstring2 xml;
		EscapeStringForXML(loc->m_strName, xml);

		// then, deal with 16-bit characters by using UTF-8
		const char *utf8 = xml.to_utf8();
#else
		std::string xml;
		EscapeStringForXML(loc->m_strName, xml);
		// no wide string support
		const char *utf8 = xml.c_str();
#endif

		fprintf(fp, "   <name>%s</name>\n", utf8);
		fprintf(fp, "   <point1>%.12lf,%.12lf,%.2f</point1>\n",
			loc->m_pos1.x, loc->m_pos1.y, loc->m_fElevation1);
		fprintf(fp, "   <point2>%.12lf,%.12lf,%.2f</point2>\n",
			loc->m_pos2.x, loc->m_pos2.y, loc->m_fElevation2);

		// Elements that may be added later include:
		// 1. roll
		// 2. camera parameters (fov, orthographic, etc.) although those
		// don't apply to the locations of non-camera objects.

		fprintf(fp, "  </location>\n");
	}
	fprintf(fp, "</locations-file>\n");
	fclose(fp);
	return true;
}

/////////////////////////////////////////////////////////////////////////////

/**
 * Internal class used by the vtLocationSaver class to read .loc XML files.
 */
class LocationVisitor : public XMLVisitor
{
public:
	LocationVisitor(vtLocationSaver *saver) { m_saver = saver; m_level = 0; }
	~LocationVisitor();
	void startXML() { m_level = 0; }
	void endXML() { m_level = 0; }
	void startElement(const char *name, const XMLAttributes &atts);
	void endElement(const char *name);
	void data(const char *s, int length);

private:
	string m_data;
	int m_level;
	vtLocation *m_loc;
	vtLocationSaver *m_saver;
};

LocationVisitor::~LocationVisitor()
{
}

void LocationVisitor::startElement(const char *name, const XMLAttributes &atts)
{
	if (m_level == 1 && !strcmp(name, "location"))
	{
		m_loc = new vtLocation;
		m_level = 2;
	}
	if (m_level == 0 && !strcmp(name, "locations-file"))
		m_level = 1;
	m_data = "";
}

void LocationVisitor::endElement(const char *name)
{
	if (m_level == 2)
	{
		if (!strcmp(name, "point1"))
			sscanf(m_data.c_str(), "%lf,%lf,%f",
			&m_loc->m_pos1.x, &m_loc->m_pos1.y, &m_loc->m_fElevation1);
		if (!strcmp(name, "point2"))
			sscanf(m_data.c_str(), "%lf,%lf,%f",
			&m_loc->m_pos2.x, &m_loc->m_pos2.y, &m_loc->m_fElevation2);
		if (!strcmp(name, "name"))
		{
			// the special "&" characters have already been un-escaped for
			// us, but we still need to convert from UTF-8 to wide string
#if SUPPORT_WSTRING
			m_loc->m_strName.from_utf8(m_data.c_str());
#else
			m_loc->m_strName = m_data;
#endif
		}
	}
	if (m_level == 1 && !strcmp(name, "locations-file"))
		m_level = 0;
	if (m_level == 2 && !strcmp(name, "location"))
	{
		m_saver->m_loc.Append(m_loc);
		m_level = 1;
	}
}

void LocationVisitor::data(const char *s, int length)
{
	m_data.append(string(s, length));
}

/////////////////////////////////////////////

bool vtLocationSaver::Read(const vtString &fname)
{
	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	LocationVisitor visitor(this);
	try
	{
		readXML((const char *)fname, visitor);
	}
	catch (xh_io_exception &exp)
	{
		// TODO: would be good to pass back the error message.
		VTLOG("XML parsing error: %s\n", exp.getFormattedMessage().c_str());
		return false;
	}
	m_strFilename = fname;
	return true;
}

void vtLocationSaver::SetCRS(const vtCRS &crs)
{
	m_crs = crs;

	// Prepare to convert from local to global CRS.
	vtCRS global_crs;
	OGRErr err = global_crs.SetGeogCSFromDatum(EPSG_DATUM_WGS84);
	if (err != OGRERR_NONE)
	{
		VTLOG("Can't set location saver's projection, error %d\n", err);
		return;
	}

	delete m_pConvertToWGS;
	delete m_pConvertFromWGS;

	m_pConvertToWGS = CreateCoordTransform(&m_crs, &global_crs, true);
	m_pConvertFromWGS = CreateCoordTransform(&global_crs, &m_crs, true);
}

bool vtLocationSaver::StoreTo(uint num, const LocNameString &name)
{
	if (!m_pTransform)
		return false;

	vtLocation *loc;
	if (num < m_loc.GetSize())
		loc = m_loc[num];
	else
		loc = new vtLocation;

	// Get terrain coordinates for position and direction from the vtTransform
	FMatrix4 mat;
	m_pTransform->GetTransform(mat);

	FPoint3 pos1 = m_pTransform->GetTrans();
	FPoint3 pos2, dir;

	// get direction
	FPoint3 forward(0, 0, -10);		// 10-meter vector
	mat.TransformVector(forward, dir);
	pos2 = pos1 + dir;

	// convert to earth coords
	DPoint3 epos1, epos2;
	m_LocalCS.LocalToEarth(pos1, epos1);
	m_LocalCS.LocalToEarth(pos2, epos2);

	if (!m_pConvertToWGS)
	{
		// fatal: can't convert between CS
		return false;
	}
	int result = 0;
	result += m_pConvertToWGS->Transform(1, &epos1.x, &epos1.y);
	result += m_pConvertToWGS->Transform(1, &epos2.x, &epos2.y);

	if (result != 2)
	{
		// fatal: can't convert these points between CS
		return false;
	}

	loc->m_pos1.Set(epos1.x, epos1.y);
	loc->m_fElevation1 = (float) epos1.z;

	loc->m_pos2.Set(epos2.x, epos2.y);
	loc->m_fElevation2 = (float) epos2.z;

	if (!name.empty())
		loc->m_strName = name;

	m_loc.SetAt(num, loc);
	return true;
}

bool vtLocationSaver::RecallFrom(int num)
{
	if (!m_pTransform)
		return false;

	vtLocation *loc = m_loc[num];

	DPoint3 epos1, epos2;
	epos1.Set(loc->m_pos1.x, loc->m_pos1.y, loc->m_fElevation1);
	epos2.Set(loc->m_pos2.x, loc->m_pos2.y, loc->m_fElevation2);

	if (!m_pConvertFromWGS)
	{
		// fatal: can't convert between CRS
		return false;
	}
	int result = 0;
	result += m_pConvertFromWGS->Transform(1, &epos1.x, &epos1.y);
	result += m_pConvertFromWGS->Transform(1, &epos2.x, &epos2.y);

	if (result != 2)
	{
		// fatal: can't convert these points between CRS
		return false;
	}

	// convert to terrain coords
	FPoint3 pos1, pos2;
	m_LocalCS.EarthToLocal(epos1, pos1);
	m_LocalCS.EarthToLocal(epos2, pos2);

	m_pTransform->SetTrans(pos1);
	m_pTransform->PointTowards(pos2);
	return true;
}

bool vtLocationSaver::RecallFrom(const char *name)
{
	int num = FindLocation(name);
	if (num != -1)
		return RecallFrom(num);
	return false;
}

int vtLocationSaver::FindLocation(const char *locname)
{
	LocNameString name;
#if SUPPORT_WSTRING
	name.from_utf8(locname);
#else
	name = locname;
#endif

	int i, num = m_loc.GetSize();
	for (i = 0; i < num; i++)
	{
		vtLocation *loc = m_loc[i];
		if (name == loc->m_strName)
			return i;
	}
	return -1;
}

void vtLocationSaver::Remove(int num)
{
	delete m_loc[num];
	m_loc.RemoveAt(num);
}

