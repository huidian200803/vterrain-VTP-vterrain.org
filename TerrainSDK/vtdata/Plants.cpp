//
// Plants.cpp
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vtLog.h"
#include "Plants.h"
#include "MathTypes.h"
#include "FilePath.h"
#include "xmlhelper/easyxml.hpp"
#include "shapelib/shapefil.h"

extern int FindDBField(DBFHandle db, const char *field_name);

/////////////////////////

vtPlantAppearance::vtPlantAppearance(AppearType type, const char *filename,
								 float width, float height,
								 float shadow_radius, float shadow_darkness)
{
	m_eType = type;
	m_filename = filename;
	m_width = width;
	m_height = height;
	m_shadow_radius = shadow_radius;
	m_shadow_darkness = shadow_darkness;
}

vtPlantAppearance::vtPlantAppearance()
{
}

vtPlantAppearance::~vtPlantAppearance()
{
}

////////////////////////////////////////////////////////////////////////

vtPlantSpecies::vtPlantSpecies()
{
}


vtPlantSpecies::~vtPlantSpecies()
{
	for (uint i = 0; i < m_Apps.GetSize(); i++)
		delete m_Apps[i];
}


/**
 * Set the common name for this species.  Language is options.
 *
 \param Name The common name in UTF-8.  Example: "Colorado Blue Spruce".
 \param Lang The language, as a lower-case two-character ISO 639 standard language code.
 *	Examples: en for English, de for German, zh for Chinese.
 *  The default is English.
 */
void vtPlantSpecies::AddCommonName(const char *Name, const char *Lang)
{
	CommonName name;
	name.m_strName = Name;

	if (Lang != NULL)
		name.m_strLang = Lang;
	else
		name.m_strLang = "en";	// default language is English
	m_CommonNames.push_back(name);
}

/**
 * Return the number of common names of this species in a given language.
 * Language is a two-letter ISO 639 code, e.g. "fr" for French.
 */
uint vtPlantSpecies::CommonNamesOfLanguage(const char *lang)
{
	uint count = 0;
	for (uint j = 0; j < NumCommonNames(); j++)
	{
		if (m_CommonNames[j].m_strLang == lang)
			count++;
	}
	return count;
}

/**
 * Set the scientific name for this species, as a plain ASCII string.
 * It should have the standard form with the genus capitolized,
 * e.g. "Cocos nucifera"
 */
void vtPlantSpecies::SetSciName(const char *SciName)
{
	m_szSciName = SciName;
}


/////////////////////////////////////////////////////////////////////////

vtSpeciesList::vtSpeciesList()
{
}

vtSpeciesList::~vtSpeciesList()
{
	for (uint i = 0; i < m_Species.GetSize(); i++)
		delete m_Species[i];
}

bool vtSpeciesList::WriteXML(const char *fname) const
{
	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	FILE *fp = vtFileOpen(fname, "wb");
	if (!fp)
	{
		throw xh_io_exception("Failed to open file", xh_location(fname),
				"XML Writer");
	}

	fprintf(fp, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\n");
	fprintf(fp, "<species-file file-format-version=\"1.1\">\n");

	for (uint i = 0; i < NumSpecies(); i++)
	{
		vtPlantSpecies *spe = GetSpecies(i);
		fprintf(fp, "\t<species name=\"%s\" max_height=\"%.2f\">\n",
			spe->GetSciName(),
			spe->GetMaxHeight());
		for (uint j = 0; j < spe->NumCommonNames(); j++)
		{
			vtPlantSpecies::CommonName cname = spe->GetCommonName(j);
			fprintf(fp, "\t\t<common name=\"%s\" lang=\"%s\" />\n",
				(const char *) cname.m_strName,
				(const char *) cname.m_strLang);
		}
		for (uint j = 0; j < GetSpecies(i)->NumAppearances(); j++)
		{
			vtPlantAppearance* app = GetSpecies(i)->GetAppearance(j);
			fprintf(fp, "\t\t<appearance type=\"%d\" filename=\"%s\" "
				"width=\"%.2f\" height=\"%.2f\" shadow_radius=\"%.1f\" shadow_darkness=\"%.1f\" />\n",
				app->m_eType, (const char *)app->m_filename, app->m_width,
				app->m_height, app->m_shadow_radius, app->m_shadow_darkness);
		}
		fprintf(fp, "\t</species>\n");
	}
	fprintf(fp, "</species-file>\n");
	fclose(fp);
	return true;
}

bool vtSpeciesList::WriteHTML(const char *fname) const
{
	FILE *fp = fopen(fname, "wb");
	if (!fp)
		return false;
	fprintf(fp, "<html>\n\
<head>\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n\
<title>VTP Plant Library: List of species</title>\n\
<link rel=\"stylesheet\" href=\"../../vtp.css\">\n\
</head>\n\
\n\
<body>\n\
<script language=\"JavaScript\" type=\"text/javascript\" src=\"../../header3.js\"></script>\n\
<h2>VTP Plant Library: List of species</h2>\n\
\n");
	fprintf(fp, "<blockquote>\n\
<p><a href=\"index.html\">About the VTP Plant Library</a></p>\n\
");
	time_t t = time(NULL);
	fprintf(fp, "<p>Generated on %s GMT</p>\n", asctime(gmtime(&t)));

	int total = 0;
	for (uint i = 0; i < NumSpecies(); i++)
		total += GetSpecies(i)->NumAppearances();

	fprintf(fp, "<p>Total: %d species with %d appearances</p>\n", NumSpecies(), total);
	fprintf(fp, "</blockquote>\n\n");

	fprintf(fp, "<ul>\n");
	for (uint i = 0; i < NumSpecies(); i++)
	{
		vtPlantSpecies *sp = GetSpecies(i);
		fprintf(fp, "  <li><i><font size=\"4\">%s</font></i>\n    <ul>\n", sp->GetSciName());
		fprintf(fp, "      <li>Common names: ");
		for (uint j = 0; j < sp->NumCommonNames(); j++)
		{
			if (j > 0)
				fprintf(fp, ", ");
			vtPlantSpecies::CommonName cn = sp->GetCommonName(j);
			fprintf(fp, "%s", (const char *) cn.m_strName);
			if (cn.m_strLang != "" && cn.m_strLang != "en")
				fprintf(fp, " (%s)", (const char *) cn.m_strLang);
		}
		fprintf(fp, "</li>\n");
		fprintf(fp, "      <li>Appearances: %d images</li>\n", sp->NumAppearances());
		fprintf(fp, "    </ul>\n");
	}
	fprintf(fp, "  </ul>\n  </li>\n");
	fprintf(fp, "</ul>\n</body>\n</html>\n");
	fclose(fp);
	return true;
}

short vtSpeciesList::FindSpeciesId(vtPlantSpecies *ps)
{
	for (uint i = 0; i < m_Species.GetSize(); i++)
	{
		if (m_Species[i] == ps)
			return (short) i;
	}
	return -1;
}

vtString RemSpaces(const vtString &str)
{
	vtString out;
	for (int i = 0; i < str.GetLength(); i++)
	{
		if (str[i] != ' ')
			out += str[i];
	}
	return out;
}

/*void vtSpeciesList::LookupPlantIndices(vtBioType *bt)
{
	for (uint i = 0; i < bt->m_Densities.GetSize(); i++)
	{
		vtString common_name = RemSpaces(bt->m_Densities[i]->m_common_name);

		bt->m_Densities[i]->m_list_index = -1;
		for (uint j = 0; j < NumSpecies(); j++)
		{
			vtPlantSpecies *ps = GetSpecies(j);
			if (common_name == RemSpaces(ps->GetCommonName()))
			{
				bt->m_Densities[i]->m_list_index = j;
				break;
			}
		}
	}
}*/

/**
 * Find the index of a species by scientific name.
 * Example:
	\code
	short index = pSpeciesList->GetSpeciesIdByName("Cocos nucifera");
	\endcode
 * \return The 0-based index, or -1 if not found.
 */
short vtSpeciesList::GetSpeciesIdByName(const char *name) const
{
	for (uint j = 0; j < NumSpecies(); j++)
	{
		if (!strcmp(name, m_Species[j]->GetSciName()))
			return (short) j;
	}
	return -1;
}

/**
 * Find the index of a species by common name.
 * Example:
	\code
	short index = pSpeciesList->GetSpeciesIdByName("Coconut Palm");
	\endcode
 * \return The 0-based index, or -1 if not found.
 */
short vtSpeciesList::GetSpeciesIdByCommonName(const char *name) const
{
	uint i, j;
	for (i = 0; i < NumSpecies(); i++)
	{
		vtPlantSpecies *spe = GetSpecies(i);
		for (j = 0; j < spe->NumCommonNames(); j++)
		{
			vtPlantSpecies::CommonName cname = spe->GetCommonName(j);
			if (!strcmp(name, cname.m_strName))
				return (short) i;

			// also, for backward compatibility, look for a match without spaces
			vtString nospace = RemSpaces(cname.m_strName);
			if (!strcmp(name, nospace))
				return (short) i;
		}
	}
	return -1;
}

////////////////////////////////////////////////////////////////////////
// Visitor class, for XML parsing of SpeciesList files.

class SpeciesListVisitor : public XMLVisitor
{
public:
	SpeciesListVisitor(vtSpeciesList *pl) :
		m_state(0), m_pPL(pl) { m_pSpecies = NULL; }

	virtual ~SpeciesListVisitor () { delete m_pSpecies; }

	void startXML() { m_state = 0; }
	void startElement(const char *name, const XMLAttributes &atts);
	void endElement(const char *name);

private:
	vtSpeciesList *m_pPL;
	vtPlantSpecies *m_pSpecies;
	int m_state;
};

void SpeciesListVisitor::startElement(const char *name, const XMLAttributes &atts)
{
	const char *attval;

	m_state++;

	if (m_state == 1)
	{
		if (string(name) != (string)"species-file")
		{
			string message = "Root element name is ";
			message += name;
			message += "; expected species-file";
			throw xh_io_exception(message, "XML Reader");
		}
	}

	if (m_state == 2)
	{
		if (string(name) == (string)"species")
		{
			m_pSpecies = new vtPlantSpecies;

			// Get name and max_height
			attval = atts.getValue("name");
			if (attval != NULL)
				m_pSpecies->SetSciName(attval);
			attval = atts.getValue("max_height");
			if (attval != NULL)
				m_pSpecies->SetMaxHeight((float)atof(attval));
		}
	}

	if (m_pSpecies && m_state == 3)
	{
		if (string(name) == (string)"common")
		{
			// Get the common name
			vtString name, lang;
			attval = atts.getValue("name");
			if (attval != NULL)
			{
				name = attval;
				attval = atts.getValue("lang");
				m_pSpecies->AddCommonName(name, attval);
			}
		}
		else if (string(name) == (string)"appearance")
		{
			AppearType type = AT_BILLBOARD;
			vtString filename;
			float width=1, height=1, shadow_radius = 1.0f, shadow_darkness = 0.0f;
			attval = atts.getValue("type");
			if (attval != NULL)
				type = (AppearType) atoi(attval);
			attval = atts.getValue("filename");
			if (attval != NULL)
				filename = attval;
			attval = atts.getValue("width");
			if (attval != NULL)
				width = (float)atof(attval);
			attval = atts.getValue("height");
			if (attval != NULL)
				height = (float)atof(attval);
			attval = atts.getValue("shadow_radius");
			if (attval != NULL)
				shadow_radius = (float)atof(attval);
			attval = atts.getValue("shadow_darkness");
			if (attval != NULL)
				shadow_darkness = (float)atof(attval);
			m_pSpecies->AddAppearance(type, filename, width, height,
				shadow_radius, shadow_darkness);
		}
	}
}

void SpeciesListVisitor::endElement(const char *name)
{
	if (m_pSpecies != NULL && m_state == 2)
	{
		m_pPL->Append(m_pSpecies);
		m_pSpecies = NULL;
	}
	m_state--;
}

bool vtSpeciesList::ReadXML(const char *pathname, vtString *msg)
{
	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	SpeciesListVisitor visitor(this);
	try
	{
		readXML(pathname, visitor);
	}
	catch (xh_exception &ex)
	{
		// pass back the error message, if they want it
		if (msg)
			*msg = ex.getFormattedMessage().c_str();
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////

vtBioRegion::vtBioRegion()
{
}

vtBioRegion::~vtBioRegion()
{
	for (uint i = 0; i < m_Types.GetSize(); i++)
		delete m_Types[i];
}

bool vtBioRegion::Read(const char *fname, const vtSpeciesList &species)
{
	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	FILE *fp = vtFileOpen(fname, "r");
	if (!fp) return false;

	int num = 0;
	char buf[80];

	if (fread(buf, 13, 1, fp) != 1)
		return false;
	if (strncmp(buf, "bioregion1.0\n", 12))
		return false;

	if (fscanf(fp, "types: %d\n", &num) != 1)
		return false;

	for (int i = 0; i < num; i++)
	{
		vtBioType *bt = new vtBioType;
		int num2;
		if (fscanf(fp, "species: %d\n", &num2) != 1)
			return false;
		for (int j = 0; j < num2; j++)
		{
			char common_name[80];
			float plant_per_m2;
			if (fscanf(fp, "\t%s %f\n", common_name, &plant_per_m2) != 2)
				return false;
			int snum = species.GetSpeciesIdByCommonName(common_name);
			if (snum != -1)
				bt->AddPlant(species.GetSpecies(snum), plant_per_m2);
		}
		m_Types.Append(bt);
	}
	fclose(fp);
	return true;
}

bool vtBioRegion::WriteXML(const char *fname) const
{
	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	FILE *fp = vtFileOpen(fname, "wb");
	if (!fp)
	{
		throw xh_io_exception("Failed to open file", xh_location(fname),
				"XML Writer");
	}

	fprintf(fp, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\n");
	fprintf(fp, "<bioregion-file file-format-version=\"1.1\">\n");

	int num = m_Types.GetSize();
	for (int i = 0; i < num; i++)
	{
		vtBioType *bt = m_Types[i];
		fprintf(fp, "\t<biotype name=\"%s\" id=\"%d\">\n", (const char *) bt->m_name, i);

		int num2 = bt->m_Densities.GetSize();
		for (int j = 0; j < num2; j++)
		{
			vtPlantDensity *pd = bt->m_Densities[j];
			fprintf(fp, "\t\t<species name=\"%s\" per_hectare=\"%.2f\"",
				pd->m_pSpecies->GetSciName(), pd->m_plant_per_m2 * 10000);
			if (pd->m_typical_size != -1.0f)
				fprintf(fp, " typical_size=\"%.2f\"", pd->m_typical_size);
			fprintf(fp, " />\n");
		}
		fprintf(fp, "\t</biotype>\n");
	}
	fprintf(fp, "</bioregion-file>\n");
	fclose(fp);
	return true;
}

////////////////////////////////////////////////////////////////////////
// Visitor class, for XML parsing of Bioregion files.

class BioRegionVisitor : public XMLVisitor
{
public:
	BioRegionVisitor(vtBioRegion *br, const vtSpeciesList &sl) :
	  m_pBR(br), m_Species(sl) {}

	void startXML() { m_state = 0; }
	void startElement(const char *name, const XMLAttributes &atts);
	void endElement(const char *name);

private:
	int m_state;
	vtBioRegion *m_pBR;
	vtBioType *m_pBiotype;
	const vtSpeciesList &m_Species;
};

void BioRegionVisitor::startElement(const char *name, const XMLAttributes &atts)
{
	const char *attval;

	m_state++;

	if (m_state == 1)
	{
		if (string(name) != (string)"bioregion-file")
		{
			string message = "Root element name is ";
			message += name;
			message += "; expected bioregion-file";
			throw xh_io_exception(message, "XML Reader");
		}
	}

	if (m_state == 2)
	{
		if (string(name) == (string)"biotype")
		{
			m_pBiotype = new vtBioType;
			// Get name and max_height
			attval = atts.getValue("name");
			if (attval != NULL)
				m_pBiotype->m_name = attval;
		}
	}

	if (m_state == 3)
	{
		if (string(name) == (string)"species")
		{
			vtString sciname;
			float per_m2;
			float typical_size = -1.0f;
			attval = atts.getValue("name");
			if (attval != NULL)
				sciname = attval;
			attval = atts.getValue("per_hectare");
			if (attval != NULL)
				per_m2 = (float) atof(attval) / 10000;	// hectare is 100m*100m
			attval = atts.getValue("typical_size");
			if (attval != NULL)
				typical_size = (float) atof(attval);

			int index = m_Species.GetSpeciesIdByName(sciname);
			if (index != -1)
			{
				vtPlantSpecies *ps = m_Species.GetSpecies(index);
				m_pBiotype->AddPlant(ps, per_m2, typical_size);
			}
		}
	}
}

void BioRegionVisitor::endElement(const char *name)
{
	if (m_state == 2)
		m_pBR->AddType(m_pBiotype);

	m_state--;
}

bool vtBioRegion::ReadXML(const char *fname, const vtSpeciesList &species,
						  vtString *msg)
{
	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	BioRegionVisitor visitor(this, species);
	try
	{
		readXML(fname, visitor);
	}
	catch (xh_exception &ex)
	{
		// pass back the error message, if they want it
		if (msg)
			*msg = ex.getFormattedMessage().c_str();
		return false;
	}
	return true;
}

int vtBioRegion::FindBiotypeIdByName(const char *name) const
{
	int num = m_Types.GetSize();
	for (int i = 0; i < num; i++)
	{
		vtBioType *bt = m_Types[i];
		if (bt->m_name == name)
			return i;
	}
	return -1;
}

void vtBioRegion::ResetAmounts()
{
	int num = m_Types.GetSize();
	for (int i = 0; i < num; i++)
	{
		m_Types[i]->ResetAmounts();
	}
}


///////////////////////////////////////////////////////////////////////

vtBioType::vtBioType()
{
}

vtBioType::~vtBioType()
{
	for (uint i = 0; i < m_Densities.GetSize(); i++)
		delete m_Densities[i];
}

void vtBioType::AddPlant(vtPlantSpecies *pSpecies, float plant_per_m2, float typical_size)
{
	vtPlantDensity *pd = new vtPlantDensity;
	pd->m_pSpecies = pSpecies;
	pd->m_plant_per_m2 = plant_per_m2;
	pd->m_typical_size = typical_size;
	pd->m_amount = 0.0f;
	pd->m_iNumPlanted = 0;
	m_Densities.Append(pd);
}

void vtBioType::ResetAmounts()
{
	for (uint i = 0; i < m_Densities.GetSize(); i++)
		m_Densities[i]->ResetAmounts();
}

/**
 * Request a plant (species ID) from this biotype.  Returns a random plant,
 *  weighted by the relative densities of each species in this biotype.
 */
int vtBioType::GetWeightedRandomPlant()
{
	// The species are weighted by multiplying a random number against each
	//  density.  The species with the highest resulting value is chosen.
	float highest = 0;
	int picked = -1;
	float val;

	int densities = m_Densities.GetSize();
	for (int i = 0; i < densities; i++)
	{
		val = random(m_Densities[i]->m_plant_per_m2);
		if (val > highest)
		{
			highest = val;
			picked = i;
		}
	}
	return picked;
}


///////////////////////////////////////////////////////////////////////


vtPlantInstanceArray::vtPlantInstanceArray()
{
	m_SizeField = AddField("Size", FT_Float);
	m_SpeciesField = AddField("Species", FT_Short);
}

int vtPlantInstanceArray::AddPlant(const DPoint2 &pos, float size,
									   short species_id)
{
	if (size < 0.0001f || size > 100.0f)
		VTLOG(" Warning: Plant with unusual height of %f\n", size);
	int index = AddPoint(pos);
	SetValue(index, m_SizeField, size);
	SetValue(index, m_SpeciesField, species_id);
	return index;
}

int vtPlantInstanceArray::AddPlant(const DPoint2 &pos, float size,
									vtPlantSpecies *ps)
{
	short species_id = m_pSpeciesList->FindSpeciesId(ps);
	if (species_id == -1)
		return -1;
	return AddPlant(pos, size, species_id);
}

void vtPlantInstanceArray::SetPlant(int iNum, float size, short species_id)
{
	SetValue(iNum, m_SizeField, size);
	SetValue(iNum, m_SpeciesField, species_id);
}

void vtPlantInstanceArray::GetPlant(int iNum, float &size, short &species_id) const
{
	size = GetFloatValue(iNum, m_SizeField);
	species_id = GetShortValue(iNum, m_SpeciesField);
}

/**
 * Given a species index, return the number of instances which are of that species.
 */
uint vtPlantInstanceArray::InstancesOfSpecies(short species_id)
{
	uint i, count = 0, numinstances = NumEntities();
	float size;
	short species;
	for (i = 0; i < numinstances; i++)
	{
		GetPlant(i, size, species);
		if (species == species_id)
			count++;
	}
	return count;
}

/*void vtPlantInstanceArray::AppendFrom(const vtPlantInstanceArray &from)
{
	// TODO: match actual species
	for (uint i = 0; i < from.GetSize(); i++)
	{
		Append(from[i]);
	}
}*/

#ifndef DOXYGEN_SHOULD_SKIP_THIS
struct vtPlantInstance10 {
	float x, y;
	float size;
	short species_id;
};

struct vtPlantInstance11 {
	DPoint2 m_p;
	float size;
	short species_id;
};

struct vtPlantInstance20 {
	DPoint2 m_p;
	float size;
	short species_id;
};
#endif

bool vtPlantInstanceArray::ReadVF_version11(const char *fname)
{
	FILE *fp = vtFileOpen(fname, "rb");
	if (!fp)
		return false;

	char buf[6];
	if (fread(buf, 6, 1, fp) != 1)
		return false;
	if (strncmp(buf, "vf", 2))
	{
		fclose(fp);
		return false;
	}
	float version = (float) atof(buf+2);
	bool utm;
	int zone, datum;
	int quiet;
	quiet = fread(&utm, 1, 1, fp);
	/* FIXME:  Ahoy, there be byte order issues here. See below in this routine. */
	quiet = fread(&zone, 4, 1, fp);
	quiet = fread(&datum, 4, 1, fp);
	if (utm)
		m_crs.SetUTMZone(zone);
	m_crs.SetDatum(datum);

	int i, size;
	quiet = fread(&size, 4, 1, fp);
	Reserve(size);

	if (version == 1.0f)
	{
		vtPlantInstance10 *pOld = new vtPlantInstance10[size];
		quiet = fread(pOld, sizeof(vtPlantInstance10), size, fp);
		vtPlantInstance20 pi;
		for (i = 0; i < size; i++)
			AddPlant(DPoint2(pOld[i].x, pOld[i].y), pOld[i].size, pOld[i].species_id);

		delete [] pOld;
	}
	else if (version == 1.1f)
	{
		vtPlantInstance11 *pTemp = new vtPlantInstance11[size];
		quiet = fread(pTemp, sizeof(vtPlantInstance11), size, fp);

		for (i = 0; i < size; i++)
			AddPlant(pTemp[i].m_p, pTemp[i].size, pTemp[i].species_id);

		delete [] pTemp;
	}
	else
		return false;
	fclose(fp);
	return true;
}

bool vtPlantInstanceArray::ReadVF(const char *fname)
{
	VTLOG("Reading VF file '%s'\n", fname);

	if (m_pSpeciesList == NULL)
	{
		VTLOG(" can't read, because there are no known species.\n");
		return false;
	}

	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	FILE *fp = vtFileOpen(fname, "rb");
	if (!fp)
		return false;

	char buf[6];
	if (fread(buf, 6, 1, fp) != 1)
		return false;
	if (strncmp(buf, "vf", 2))
	{
		fclose(fp);
		return false;
	}
	float version = (float) atof(buf+2);
	if (version < 2.0f)
	{
		fclose(fp);
		return ReadVF_version11(fname);
	}

	int i, numinstances, numspecies, quiet;

	// read WKT SRS
	short len;
	quiet = fread(&len, sizeof(short), 1, fp);

	char wkt_buf[2000], *wkt = wkt_buf;
	quiet = fread(wkt, len, 1, fp);
	OGRErr err = m_crs.importFromWkt(&wkt);
	if (err != OGRERR_NONE)
	{
		// It shouldn't be fatal to encounter a missing or unparsable projection
		// return false;
	}

	// read number of species
	quiet = fread(&numspecies, sizeof(int), 1, fp);

	// read species binomial strings, creating lookup table of new IDs
	int unknown = 0;
	short species_id;
	vector<short> local_ids;
	char name[200];
	for (i = 0; i < numspecies; i++)
	{
		quiet = fread(&len, sizeof(short), 1, fp);
		quiet = fread(name, len, 1, fp);
		name[len] = 0;
		species_id = m_pSpeciesList->GetSpeciesIdByName(name);
		if (species_id == -1)
		{
			VTLOG("  Unknown species: %s\n", name);
			unknown++;
		}
		local_ids.push_back(species_id);
	}
	if (unknown > 0)
		VTLOG("Warning: %d unknown species encountered in VF table\n", unknown);

	// read number of instances
	quiet = fread(&numinstances, sizeof(int), 1, fp);
	Reserve(numinstances);

	// read local origin (center of exents) as double-precision coordinates
	DPoint2 origin;
	quiet = fread(&origin, sizeof(double), 2, fp);

	// read instances
	short height;
	FPoint2 local_offset;
	short local_species_id;
	unknown = 0;
	for (i = 0; i < numinstances; i++)
	{
		// location
		quiet = fread(&local_offset, sizeof(float), 2, fp);
		DPoint2 pos = origin + DPoint2(local_offset);

		// height in centimeters
		quiet = fread(&height, sizeof(short), 1, fp);
		float size = (float) height / 100.0f;

		// species id
		quiet = fread(&local_species_id, sizeof(short), 1, fp);

		// convert from file-local id to new id
		if (local_species_id < 0 || local_species_id > numspecies-1)
		{
			VTLOG(" Warning: species index %d out of range [0..%d]\n", local_species_id, numspecies-1);
		}
		else
		{
			species_id = local_ids[local_species_id];
			if (species_id == -1)
				unknown++;
			else
				AddPlant(pos, size, species_id);
		}
	}
	if (unknown > 0)
		VTLOG("Warning: %d/%d instances were ignored because of unknown species.\n", unknown, numinstances);

	fclose(fp);
	return true;
}

bool vtPlantInstanceArray::WriteVF(const char *fname) const
{
	int i, numinstances = NumEntities();
	if (numinstances == 0)
		return false;	// empty files not allowed
	if (!m_pSpeciesList)
		return false;
	int numspecies = m_pSpeciesList->NumSpecies();
	short len;	// for string lengths
	short species_id;
	float size;

	FILE *fp = vtFileOpen(fname, "wb");
	if (!fp)
		return false;

	fwrite("vf2.0", 6, 1, fp);

	// write SRS as WKT
	char *wkt;
	OGRErr err = m_crs.exportToWkt(&wkt);
	if (err != OGRERR_NONE)
		return false;
	len = (short) strlen(wkt);
	fwrite(&len, sizeof(short), 1, fp);
	fwrite(wkt, len, 1, fp);
	OGRFree(wkt);

	// filter out ununsed species, create table of used species
	vector<int> index_count;
	for (i = 0; i < numspecies; i++)
		index_count.push_back(0);
	for (i = 0; i < numinstances; i++)
	{
		GetPlant(i, size, species_id);
		index_count[species_id]++;
	}
	vector<int> index_table;
	for (i = 0; i < numspecies; i++)
	{
		if (index_count[i] > 0)
			index_table.push_back(i);
	}
	int used = index_table.size();

	// write number of species
	fwrite(&used, sizeof(int), 1, fp);

	// write species binomial strings
	for (i = 0; i < used; i++)
	{
		species_id = index_table[i];
		const char *name = m_pSpeciesList->GetSpecies(species_id)->GetSciName();
		len = (short) strlen(name);
		fwrite(&len, sizeof(short), 1, fp);
		fwrite(name, len, 1, fp);
	}

	// reverse table for lookup
	vector<short> reverse_table;
	reverse_table.resize(numspecies);
	for (i = 0; i < used; i++)
		reverse_table[index_table[i]] = i;

	// write number of instances
	fwrite(&numinstances, sizeof(int), 1, fp);

	// write local origin (center of exents) as double-precision coordinates
	DRECT rect;
	ComputeExtent(rect);
	DPoint2 origin, diff;
	rect.GetCenter(origin);
	fwrite(&origin, sizeof(double), 2, fp);

	// write instances
	FPoint2 offset;
	for (i = 0; i < numinstances; i++)
	{
		// location
		diff = GetPoint(i) - origin;
		offset = diff;	// acceptable to use single precision for local offset
		fwrite(&offset, sizeof(float), 2, fp);

		GetPlant(i, size, species_id);

		// height in centimeters
		short height = (short) (size * 100.0f);
		fwrite(&height, sizeof(short), 1, fp);

		// species id
		short local_id = reverse_table[species_id];
		fwrite(&local_id, sizeof(short), 1, fp);
	}

	fclose(fp);
	return true;
}

bool vtPlantInstanceArray::ReadSHP(const char *fname)
{
	// SHPOpen doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(fname);

	// Open the SHP File & Get Info from SHP:
	SHPHandle hSHP = SHPOpen(fname_local, "rb");
	if (hSHP == NULL)
		return false;

	LoadGeomFromSHP(hSHP);
	SHPClose(hSHP);

	if (!LoadDataFromDBF(fname))
		return false;

	m_SizeField = GetFieldIndex("Size");
	m_SpeciesField = GetFieldIndex("Species");

	if (m_SizeField == -1 || m_SpeciesField == -1)
		return false;

	return true;
}

