//
// GEOnet.cpp
//
// Copyright (c) 2002-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "GEOnet.h"
#include "shapelib/shapefil.h"
#include "vtLog.h"
#include "FilePath.h"

#include <stdio.h>

#if SUPPORT_WSTRING

//
// Helper Function: Convert all accented characters to unaccented.
//
vtString RemoveAccents(const vtString &strInput)
{
	vtString strOutput = "";

	int len = strInput.GetLength();
	for (int i = 0; i < len; i++)
	{
		uchar ch = (uchar) strInput[i];
		switch (ch)
		{
			// do conversion from "ANSI western europe" character set
		case 0xc0:	// 'À'
		case 0xc1:	// 'Á':
		case 0xc2:	// 'Â':
		case 0xc3:	// 'Ã':
		case 0xc4:	// 'Ä':
		case 0xc5:	// 'Å':
			strOutput += 'A';
			break;
		case 0xc7:	// 'Ç':
			strOutput += 'C';
			break;
		case 0xc8:	// 'È':
		case 0xc9:	// 'É':
		case 0xca:	// 'Ê':
		case 0xcb:	// 'Ë':
			strOutput += 'E';
			break;
		case 0xcc:	// 'Ì':
		case 0xcd:	// 'Í':
		case 0xce:	// 'Î':
		case 0xcf:	// 'Ï':
			strOutput += 'I';
			break;
		case 0xd0:	// 'Ð':
			strOutput += 'D';
			break;
		case 0xd1:	// 'Ñ':
			strOutput += 'N';
			break;
		case 0xd2:	// 'Ò':
		case 0xd3:	// 'Ó':
		case 0xd4:	// 'Ô':
		case 0xd5:	// 'Õ':
		case 0xd6:	// 'Ö':
		case 0xd8:	// 'Ø':
			strOutput += 'O';
			break;
		case 0xd9:	// 'Ù':
		case 0xda:	// 'Ú':
		case 0xdb:	// 'Û':
		case 0xdc:	// 'Ü':
			strOutput += 'U';
			break;
		case 0xdd:	// 'Ý':
			strOutput += 'Y';
			break;
		case 0xdf:	// 'ß':
			strOutput += 's';	// NOTE: converts to 'ss'
			strOutput += 's';
			break;
		case 0xe0:	// 'à':
		case 0xe1:	// 'á':
		case 0xe2:	// 'â':
		case 0xe3:	// 'ã':
		case 0xe4:	// 'ä':
		case 0xe5:	// 'å':
			strOutput += 'a';
			break;
		case 0xe7:	// 'ç':
			strOutput += 'c';
			break;
		case 0xe8:	// 'è':
		case 0xe9:	// 'é':
		case 0xea:	// 'ê':
		case 0xeb:	// 'ë':
			strOutput += 'e';
			break;
		case 0xec:	// 'ì':
		case 0xed:	// 'í':
		case 0xee:	// 'î':
		case 0xef:	// 'ï':
			strOutput += 'i';
			break;
		case 0xf1:	// 'ñ':
			strOutput += 'n';
			break;
		case 0xf2:	// 'ò':
		case 0xf3:	// 'ó':
		case 0xf4:	// 'ô':
		case 0xf5:	// 'õ':
		case 0xf6:	// 'ö':
		case 0xf8:	// 'ø':
			strOutput += 'o';
			break;
		case 0xf9:	// 'ù':
		case 0xfa:	// 'ú':
		case 0xfb:	// 'û':
		case 0xfc:	// 'ü':
			strOutput += 'u';
			break;
		case 0xfd:	// 'ý':
		case 0xff:	// 'ÿ':
			strOutput += 'y';
			break;

		// Also do conversion from old DOS character set - fortunately, they
		// (surprisingly) don't overlap with ANSI accented characters.
		case 0x83:	// 'ƒ' (ANSI 'â')
		case 0x84:	// '„' (ANSI 'ä')
		case 0x85:	// '…' (ANSI 'à')
		case 0x86:	// '†' (ANSI 'å')
		case 0xa0:	// ' ' (ANSI 'á')
			strOutput += 'a';
			break;
		case 0x8b:	// '‹' (ANSI 'ï')
		case 0x8c:	// 'Œ' (ANSI 'î')
		case 0x8d:	// '' (ANSI 'ì')
		case 0xa1:	// '¡' (ANSI 'í')
			strOutput += 'i';
			break;
		case 0x81:	// '' (ANSI 'ü')
		case 0x96:	// '–' (ANSI 'û')
		case 0x97:	// '—' (ANSI 'ù')
		case 0xa3:	// '£' (ANSI 'ú')
			strOutput += 'u';
			break;
		case 0x82:	// '‚' (ANSI 'é')
		case 0x88:	// 'ˆ' (ANSI 'ê')
		case 0x89:	// '‰' (ANSI 'ë')
		case 0x8a:	// 'Š' (ANSI 'è')
			strOutput += 'e';
			break;
		case 0x93:	// '“' (ANSI 'ô')
		case 0x94:	// '”' (ANSI 'ö')
		case 0x95:	// '•' (ANSI 'ò')
		case 0xa2:	// '¢' (ANSI 'ó')
			strOutput += 'o';
			break;
		case 0x8e:	// 'Ž' (ANSI 'Ä')
		case 0x8f:	// '' (ANSI 'Å')
			strOutput += 'A';
			break;
		case 0x9a:	// 'š' (ANSI 'Ü')
			strOutput += 'U';
			break;
		case 0x90:	// '' (ANSI 'É')
			strOutput += 'E';
			break;
		case 0x99:	// '™' (ANSI 'Ö')
			strOutput += 'O';
			break;
		case 0x87:	// '‡' (ANSI 'ç')
			strOutput += 'c';
			break;
		case 0x80:	// '€' (ANSI 'Ç')
			strOutput += 'C';
			break;
		case 0x98:	// '˜' (ANSI 'ÿ')
			strOutput += 'y';
			break;
		case 0xa4:	// '¤' (ANSI 'ñ')
			strOutput += 'n';
			break;
		case 0xa5:	// '¥' (ANSI 'Ñ')
			strOutput += 'N';
			break;
		default:
			strOutput += ch;
		}
	}
	return strOutput;
}

//
// Helper Function: Convert all accented characters to unaccented.
//
vtString RemoveAccentsUTF8(const vtString &strInput)
{
	wstring2 str;
	str.from_utf8(strInput);
	vtString vs(str.mb_str());
	return RemoveAccents(vs);
}


///////////////////////////////////////////////////////////////////////
// Country class

Country::Country()
{
}

Country::~Country()
{
	// manually free memory
	int j, num_places = m_places.GetSize();
	for (j = 0; j < num_places; j++)
		delete m_places[j];
	m_places.Clear();
}

bool Country::FindPlace(const char *name_nd, DPoint2 &point, bool bFullLength)
{
	int j, num_places = m_places.GetSize();

	bool match = false;
	for (j = 0; j < num_places; j++)
	{
		Place *place = m_places[j];
		if (bFullLength)
			match = (place->m_fullname_nd.CompareNoCase(name_nd) == 0);
		else
		{
			int len = strlen(name_nd);
			if (len > 2)
				match = (place->m_fullname_nd.Left(len).CompareNoCase(name_nd) == 0);
		}
		if (match)
		{
			point = place->m_pos;
			return true;
		}
	}
	return false;
}

bool Country::FindPlace(const std::wstring &name, DPoint2 &point, bool bFullLength)
{
	int j, num_places = m_places.GetSize();

	bool match;
	for (j = 0; j < num_places; j++)
	{
		match = false;
		Place *place = m_places[j];

		if (bFullLength)
			match = (place->m_fullname.compare(name) == 0);
		else
		{
			int len = name.length();
			if (len > 2)
				match = (place->m_fullname.compare(0, len, name) == 0);
		}
		if (match)
		{
			point = place->m_pos;
			return true;
		}
	}
	return false;
}

bool Country::FindAllMatchingPlaces(const char *name_nd, bool bFullLength, PlaceArray &places)
{
	int j, num_places = m_places.GetSize();

	bool match = false;
	places.Clear();

	for (j = 0; j < num_places; j++)
	{
		Place *place = m_places[j];
		if (bFullLength)
			match = (place->m_fullname_nd.CompareNoCase(name_nd) == 0);
		else
		{
			int len = strlen(name_nd);
			if (len > 2)
				match = (place->m_fullname_nd.Left(len).CompareNoCase(name_nd) == 0);
		}
		if (match)
		{
			places.Append(place);
		}
	}
	if (places.GetSize() > 0)
		return true;
	else
		return false;
}

bool Country::WriteSHP(const char *fname)
{
	SHPHandle hSHP = SHPCreate(fname, SHPT_POINT);
	if (!hSHP)
		return false;

	SHPObject *obj;

	int j, num_places;
	int longest_place_name = 0;

	num_places = m_places.GetSize();

	for (j = 0; j < num_places; j++)
	{
		Place *place = m_places[j];

		obj = SHPCreateSimpleObject(SHPT_POINT, 1,
			&place->m_pos.x, &place->m_pos.y, NULL);

		SHPWriteObject(hSHP, -1, obj);
		SHPDestroyObject(obj);

		const char *utf8 = place->m_fullname.to_utf8();
		int len = strlen(utf8);
		if (len > longest_place_name)
			longest_place_name = len;
	}
	SHPClose(hSHP);

	// Save DBF File also
	vtString dbfname = fname;
	dbfname = dbfname.Left(dbfname.GetLength() - 4);
	dbfname += ".dbf";
	DBFHandle db = DBFCreate(dbfname);
	if (db == NULL)
		return false;

	DBFAddField(db, "Country", FTString, 2, 0);
	DBFAddField(db, "PPC", FTInteger, 2, 0);
	DBFAddField(db, "Name", FTString, longest_place_name, 0);

	int entity = 0;

	for (j = 0; j < num_places; j++)
	{
		Place *place = m_places[j];

		DBFWriteStringAttribute(db, entity, 0, (const char *) m_abb);

		DBFWriteIntegerAttribute(db, entity, 1, place->m_ppc);

		const char *utf8 = place->m_fullname.to_utf8();
		DBFWriteStringAttribute(db, entity, 2, utf8);

		entity++;
	}
	DBFClose(db);

	return true;
}


/////////////////////////////////////////////////////////////////////////////
//

Countries::~Countries()
{
	// manually free memory
	Free();
}

bool Countries::ReadCountryList(const char *fname)
{
	FILE *fp = vtFileOpen(fname, "rb");
	if (!fp)
		return false;

	int off;
	vtString str, name, abb;
	char buf[400];
	while (fgets(buf, 400, fp) != NULL)
	{
		str = buf;
		off = str.Find(':');
		Country *country = new Country;
		country->m_full = str.Left(off);
		country->m_abb = str.Mid(off+1, 2);
		m_countries.Append(country);
	}
	fclose(fp);
	return true;
}

void Countries::ParseRawCountryFiles(const char *path_prefix, bool bNativeNames)
{
	m_path = path_prefix;

	uint num = m_countries.GetSize();
	for (int unsigned i = 0; i < num; i++)
		ParseRawCountry(i, bNativeNames);
}

bool Countries::WriteSHPPerCountry(const char *prefix)
{
	int unsigned num = m_countries.GetSize();
	for (int unsigned i = 0; i < num; i++)
	{
		Country *country = m_countries[i];

		if (country->m_places.GetSize() > 0)
		{
			vtString out_path = prefix;
			out_path += country->m_abb;
			out_path += ".shp";
			if (!country->WriteSHP(out_path))
				return false;
		}
	}
	return true;
}

void Countries::Free(bool progress_callback(int))
{
	// manually free memory
	uint size = m_countries.GetSize();
	for (int unsigned i = 0; i < size; i++)
	{
		if (progress_callback != NULL)
			progress_callback(i * 100 / size);
		Country *country = m_countries[i];
		delete country;
	}
	m_countries.SetSize(0);
}


#include "GnsCodes.h"

void GNS_to_wstring(int region, const char *word, std::wstring &output)
{
	int len = strlen(word);
	uchar ch;
	unsigned short unicode;

	output.clear();

	for (int i = 0; i < len; i++)
	{
		ch = (uchar) word[i];
		if (ch < 128)
			unicode = ch;
		else
		{
			if (region == 1)
				unicode = region1[ch-128];
			else if (region == 2)
				unicode = region2[ch-128];
			else if (region == 3)
				unicode = region3[ch-128];
			else // TODO: other 3 regions
				unicode = region3[ch-128];
			if (unicode == 0x0000)
				unicode = '?';
		}
		output += unicode;
	}
}

/*
	Fields:
	 0 RC Region Code
		1 = Western Europe/Americas;
		2 = Eastern Europe;
		3 = Africa/Middle East;
		4 = Central Asia;
		5 = Asia/Pacific;
		6 = Vietnam
	 1 UFI Unique Feature Identifier
	 2 UNI Unique Name Identifier
	 3 DD_LAT Latitude
	 4 DD_LONG Longitude
	 5 DMS_LAT Latitude
	 6 DMS_LONG Longitude
	 7 UTM Universal Transverse Mercator
	 8 JOG Joint Operations Graphic
	 9 FC Feature Classification
		A = Administrative region;
		P = Populated place;
		V = Vegetation;
		L = Locality or area;
		U = Undersea;
		R = Streets, highways, roads, or railroad;
		T = Hypsographic;
		H = Hydrographic;
		S = Spot feature.
	10 DSG Feature Designation Code
	11 PC Populated Place Classification (1 high, 5 low)
	12 CC1 Primary Country Code
	13 ADM1 First-order administrative division
	14 ADM2 Second-order administrative division
	15 DIM Dimension
	16 CC2 Secondary Country Code
	17 NT Name Type
		C = Conventional;
		D = Not verified;
		N = Native;
		V = Variant or alternate.
	18 LC Language Code
	19 SHORT_FORM
	20 GENERIC (does not apply to populated place names)
	21 SORT_NAME Upper-case form of the full name for easy sorting of the name
	22 FULL_NAME specific name, generic name, and any articles or prepositions
	23 FULL_NAME_ND Diacritics/special characters substituted with Roman characters
	24 MODIFY_DATE
*/

void Countries::ParseRawCountry(int which, bool bNativeNames)
{
	Country *country = m_countries[which];

	printf("Parsing %s...", (const char *) country->m_full);

	int i, num_important[7];
	for (i = 0; i < 7; i++)
		num_important[i] = 0;

	FILE *fp = vtFileOpen(m_path + country->m_abb + ".txt", "rb");
	if (!fp)
	{
		printf("couldn't open.\n");
		return;
	}

	DPoint2 point;
	char fc=0;	// feature classification, P = populated palce
	char pc=0;	// Populated Place Classification
	char nt=0;	// Name Type
	char *p, *w;
	vtString fullname, fullname_nd;
	char buf[4000];
	char word[200];	// some place names in Russia are more than 100 chars!
	int line_number = 0;
	Place *place;
	int region=0;
	bool bSkip;

	while (fgets(buf, 4000, fp) != NULL)
	{
		bSkip = false;
		line_number++;
		i = 0;
		w = word;
		for (p = buf; *p != '\n'; p++)
		{
			if (*p == '\t')
			{
				*w = '\0';
				// handle word
				if (i == 0)
					region = atoi(word); // RC region code
				if (i == 3)
					point.y = atof(word); // lat
				if (i == 4)
					point.x = atof(word); // lon
				if (i == 9)
					fc = word[0];
				if (i == 11)
					pc = word[0] ? word[0] - '0' : 6;	// "importance" 1-5, 1 most
				if (i == 17)	// Name Type
					nt = word[0];
				if (i == 22)	// FULL_NAME
					fullname = word;
				if (i == 23)	// FULL_NAME_ND
					fullname_nd = word;

				i++;
				w = word;
			}
			else
			{
				*w = *p;
				w++;
			}
		}
		if (fc != 'P')	// populated place
			bSkip = true;
		if (nt != 'N' && bNativeNames == true)
			bSkip = true;

		if (bSkip)
			continue;

		// count how many of each importance
		if (pc >= 1 && pc <= 6) num_important[pc] ++;

		place = new Place;
		place->m_pos = point;
		place->m_ppc = pc;
		country->m_places.Append(place);

		GNS_to_wstring(region, fullname, place->m_fullname);
		place->m_fullname_nd = fullname_nd;
	}
	fclose(fp);
	int size = country->m_places.GetSize();
	printf("\n  %d places. Importances: %d %d %d %d %d (%d)\n",
		size, num_important[1], num_important[2], num_important[3],
		num_important[4], num_important[5], num_important[6]);
}

void WriteString(FILE *fp, const vtString &str)
{
	short len = (short) str.GetLength();
	fwrite(&len, 2, 1, fp);
	const char *buf = (const char *)str;
	fwrite(buf, len, 1, fp);
}

void ReadString(FILE *fp, vtString &str)
{
	char buf[100];
	short len;
	fread(&len, 2, 1, fp);
	fread(buf, len, 1, fp);
	buf[len] = '\0';
	str = buf;
}

bool Countries::WriteSingleSHP(const char *fname)
{
	SHPHandle hSHP = SHPCreate(fname, SHPT_POINT);
	if (!hSHP)
		return false;

	SHPObject *obj;

	int i, j, num_places;
	int longest_place_name = 0;
	int num_countries = m_countries.GetSize();

	for (i = 0; i < num_countries; i++)
	{
		Country *country = m_countries[i];
		num_places = country->m_places.GetSize();

		for (j = 0; j < num_places; j++)
		{
			Place *place = country->m_places[j];

			obj = SHPCreateSimpleObject(SHPT_POINT, 1,
				&place->m_pos.x, &place->m_pos.y, NULL);

			SHPWriteObject(hSHP, -1, obj);
			SHPDestroyObject(obj);

			const char *utf8 = place->m_fullname.to_utf8();
			int len = strlen(utf8);
			if (len > longest_place_name)
				longest_place_name = len;
		}
	}
	SHPClose(hSHP);

	// Save DBF File also
	vtString dbfname = fname;
	dbfname = dbfname.Left(dbfname.GetLength() - 4);
	dbfname += ".dbf";
	DBFHandle db = DBFCreate(dbfname);
	if (db == NULL)
		return false;

	DBFAddField(db, "Country", FTString, 2, 0);
	DBFAddField(db, "PPC", FTInteger, 2, 0);
	DBFAddField(db, "Name", FTString, longest_place_name, 0);

	int entity = 0;
	for (i = 0; i < num_countries; i++)
	{
		Country *country = m_countries[i];
		num_places = country->m_places.GetSize();
		for (j = 0; j < num_places; j++)
		{
			Place *place = country->m_places[j];

			DBFWriteStringAttribute(db, entity, 0, (const char *) country->m_abb);

			DBFWriteIntegerAttribute(db, entity, 1, place->m_ppc);

			const char *utf8 = place->m_fullname.to_utf8();
			DBFWriteStringAttribute(db, entity, 2, utf8);

			entity++;
		}
	}
	DBFClose(db);

	return true;
}

void Countries::WriteGCF(const char *fname)
{
	FILE *fp = vtFileOpen(fname, "wb");

	int num = m_countries.GetSize();
	fwrite(&num, sizeof(int), 1, fp);

	int i, j, num_places;
	for (i = 0; i < num; i++)
	{
		Country *country = m_countries[i];
		num_places = country->m_places.GetSize();

		WriteString(fp, country->m_full);
		fwrite(&num_places, sizeof(int), 1, fp);

		for (j = 0; j < num_places; j++)
		{
			Place *place = country->m_places[j];
			fwrite(&place->m_pos.x, sizeof(double), 2, fp);
			WriteString(fp, place->m_fullname_nd);
		}
	}
	fclose(fp);
}

bool Countries::ReadGCF(const char *fname, bool progress_callback(int))
{
	FILE *fp = vtFileOpen(fname, "rb");
	if (!fp)
		return false;

	int num;
	fread(&num, sizeof(int), 1, fp);

	m_countries.SetMaxSize(num);

	int i, j, num_places;
	for (i = 0; i < num; i++)
	{
		if (progress_callback != NULL)
			progress_callback(i * 100 / num);

		Country *country = new Country;
		m_countries.Append(country);

		ReadString(fp, country->m_full);
		printf("Reading %s...\n", (const char *) country->m_full);

		fread(&num_places, sizeof(int), 1, fp);
		country->m_places.SetMaxSize(num_places);

		for (j = 0; j < num_places; j++)
		{
			Place *place = new Place;
			fread(&place->m_pos.x, sizeof(double), 2, fp);
			ReadString(fp, place->m_fullname_nd);
			country->m_places.Append(place);
		}
	}
	fclose(fp);
	return true;
}

bool Countries::FindPlace(const char *country_val, const char *place_val,
						  DPoint2 &point)
{
	Country *country;

	int num = m_countries.GetSize();

	int i;
	for (i = 0; i < num; i++)
	{
		country = m_countries[i];

		if (country->m_full.CompareNoCase(country_val) != 0)
			continue;

		bool success = country->FindPlace(place_val, point, true);

		// try again with just the length of the initial sea
		if (!success)
			success = country->FindPlace(place_val, point, false);

		if (success)
			return true;
	}
	return false;
}

/**
 * Find a location given its country and placename.
 * If successful, it returns true and places the coordinate in 'geo'.
 *
 * \param country Country input
 * \param place Place intput - this is generally a town or city
 * \param geo If successful, receives the resulting point
 * \param bUTF8 Important!  If your country and place names are UTF8 strings,
 *		pass true.  Otherwise they are assumed to be ANSI (western europe).
 */
bool Countries::FindPlaceWithGuess(const char *country, const char *place,
									 DPoint2 &geo, bool bUTF8)
{
	// country should never have accents in it
	vtString strCountry, strCity;
	if (bUTF8)
	{
		strCountry = RemoveAccentsUTF8(country);
		strCity = RemoveAccentsUTF8(place);
	}
	else
	{
		strCountry = RemoveAccents(country);
		strCity = RemoveAccents(place);
	}

	// Australia
	if (!strCountry.CompareNoCase("Australia"))
	{
		if (!strCity.CompareNoCase("Flinders View"))	// Brisbane suburb
			strCity = "Brisbane";

		if (!strCity.CompareNoCase("Daw Park"))	// Adelaide suburb
			strCity = "Mitcham";

		if (!strCity.CompareNoCase("Booragoon"))	// Perth suburb
		{
			geo.Set(115.834, -32.039);
			return true;
		}
	}

	// Barbados
	if (!strCountry.CompareNoCase("Barbados"))
	{
		if (!strCity.CompareNoCase("St. Michael"))	// Bridgetown suburb
			strCity = "Bridgetown";
	}

	// Bosnia
	if (!strCountry.CompareNoCase("Bosnia-Herzegovina"))
		strCountry = "BOSNIA AND HERZEGOVINA";

	// Brazil
	if (!strCountry.CompareNoCase("Brazil"))
	{
		if (!strCity.CompareNoCase("Lucas do Rio Verde"))	// tiny speck?
		{
			geo.Set(-55.951, -13.135);
			return true;
		}
	}

	// Brunei
	if (!strCity.CompareNoCase("BSB"))	// you can see why they abbreviate it
		strCity = "Bandar Seri Begawan";

	// Canada
	if (!strCountry.CompareNoCase("Canada"))
	{
		if (!strCity.CompareNoCase("100 Mile House"))
			strCity = "One Hundred Mile House";
		if (!strCity.CompareNoCase("150 Mile House"))
			strCity = "One Hundred Fifty Mile House";
		if (!strCity.CompareNoCase("Hanwell"))	// tiny town in NB
			strCity = "Fredericton";		// nearest known town?
		if (!strCity.CompareNoCase("Huberdeau"))	// in Quebec, pop. 942
			strCity = "Saint-Remi-d'Amherst";	// nearby small town
		if (!strCity.CompareNoCase("Quebec City"))
			strCity = "Quebec";
		if (!strCity.CompareNoCase("St. Albert"))
			strCity = "Saint Albert";
		if (!strCity.CompareNoCase("St. John's"))
			strCity = "Saint John's";
		if (!strCity.CompareNoCase("Sault Ste. Marie"))
			strCity = "Sault Sainte Marie";
		if (!strCity.CompareNoCase("Robson"))	// tiny speck?
		{
			geo.Set(-117.666, 49.333);
			return true;
		}
	}

	// China
	if (!strCity.CompareNoCase("Dujiangyan"))
	{
		// mysteriously missing from atlases, even though it's UNESCO site
		//  (http://whc.unesco.org/sites/1001.htm)
		geo.Set(103.428, 30.877);
		return true;
	}
	if (!strCity.CompareNoCase("Maanshan"))
		strCity = "Ma'anshan";	// 54km SW of Nanjing

	// Denmark
	if (!strCity.CompareNoCase("Solroed Strand"))
		strCity = "Solrod Strand";
	if (!strCity.CompareNoCase("Bornholm"))
		// it's an island; major town is romanized as Ronne
		strCity = "Ronne";

	// Colombia
	if (!strCountry.CompareNoCase("Colombia"))
	{
		if (!strCity.Left(12).CompareNoCase("Cartagena de"))
			strCity = "Cartagena";
	}

	// France
	if (!strCountry.CompareNoCase("France"))
	{
		if (!strCity.CompareNoCase("Aix en Provence"))
			strCity = "Aix-en-Provence";
		if (!strCity.CompareNoCase("Les Sables d'Olonne"))
			strCity = "Les Sables-d'Olonne";
		if (!strCity.CompareNoCase("Marly le Roi"))
			strCity = "Marly-le-Roi";
		if (!strCity.CompareNoCase("Maisons Laffitte"))
			strCity = "Maisons-Laffitte";
		if (!strCity.CompareNoCase("Sophia Antipolis"))
			strCity = "Nice";
		if (!strCity.CompareNoCase("Sophia-Antipolis"))
			strCity = "Nice";
		if (!strCity.CompareNoCase("Salon de Provence"))
			strCity = "Salon-de-Provence";
		if (!strCity.CompareNoCase("St. Cloud"))
			strCity = "Saint Cloud";
		if (!strCity.CompareNoCase("St. Georges de Reintembault"))
			strCity = "Saint Georges de Reintembault";
	}
	if (!strCountry.CompareNoCase("France") || !strCountry.CompareNoCase("Reunion") ||
		!strCountry.CompareNoCase("New Caledonia"))
	{
		// French addresses often have "Cedex" following the city name;
		// this is a required part of the address but should be ignored for
		// purposes of matching the city name.
		int offset = strCity.Find(" cedex");
		if (offset == -1) offset = strCity.Find(" CEDEX");
		if (offset == -1) offset = strCity.Find(" Cedex");
		if (offset != -1)
		{
			strCity = strCity.Left(offset);
			VTLOG("  Removing French Cedex: %s\n", (const char *) strCity);
		}
	}

	// Germany
	if (!strCountry.CompareNoCase("Germany"))
	{
		if (!strCity.CompareNoCase("Bad Arolsen"))
			strCity = "Arolsen";	// apparently, English name has no "bad"
		if (!strCity.CompareNoCase("Bad Duerrenberg"))
			strCity = "Bad Durrenberg";
		if (!strCity.CompareNoCase("Bedburg - Rath"))
			strCity = "Bedburg";
		if (!strCity.CompareNoCase("Goettingen"))
			strCity = "Gottingen";
		if (!strCity.CompareNoCase("Grafenwoehr"))
			strCity = "Grafenwohr";
		if (!strCity.Left(10).CompareNoCase("Oberthal-G"))
			strCity = "Gudesweiler";
		if (!strCity.CompareNoCase("Loehne"))
			strCity = "Lohne";
		if (!strCity.CompareNoCase("Osnabrueck"))
			strCity = "Osnabruck";
		if (!strCity.CompareNoCase("Saarbruecken"))
			strCity = "Saarbrucken";
		if (!strCity.CompareNoCase("St. Ingbert"))
			strCity = "Saint Ingbert";
		if (!strCity.CompareNoCase("Tuebingen"))
			strCity = "Tubingen";
	}

	// Georgia
	if (!strCity.CompareNoCase("Tbilisi"))
		strCity = "T'bilisi";

	// Greece
	if (!strCity.CompareNoCase("Archanes"))	// on Crete
		strCity = "Heraklion";

	// Hong Kong
	if (!strCity.CompareNoCase("Shatin"))
		strCity = "Sha Tin";

	// India
	if (!strCity.Left(7).CompareNoCase("Ambala "))	// "Ambala Cantonment" etc.
		strCity = "Ambala";

	// Israel
	if (!strCountry.CompareNoCase("Israel"))
	{
		if (!strCity.CompareNoCase("Herzlia") ||
			!strCity.CompareNoCase("Hertzlia") ||
			!strCity.CompareNoCase("Herzlia Pituach"))
			strCity = "Herzliya";
		if (!strCity.CompareNoCase("Bat-Yam"))
			strCity = "Bat Yam";
		if (!strCity.CompareNoCase("Tel-Aviv"))
			strCity = "Tel Aviv";
		if (!strCity.CompareNoCase("Tel-Adashim"))
			strCity = "Tel Adashim";
		if (!strCity.CompareNoCase("Raanana"))
			strCity = "Ra'ananna";
		// GEOnet doesn't know "Jerusalem" as a city!?
	}

	// Italy
	if (!strCity.CompareNoCase("Giardini Naxos"))
		strCity = "Giardini";
	if (!strCity.CompareNoCase("Pomarolo"))		// tiny town in Trentino province
		strCity = "Nomi";		// nearest uniquely-named known town
	if (!strCity.CompareNoCase("Majano"))	// town missing from atlases
		strCity = "Udine";		// nearest major city, probably 25km away

	// Hong Kong - GEOnet/FIPS says it's a country
	if (!strCity.CompareNoCase("Hong Kong"))
		strCountry = "Hong Kong";

	// Lebanon
	if (!strCity.CompareNoCase("Jounieh"))	// "Jounieh" is the westernized name
		strCity = "Juniyah";

	// Mexico
	if (!strCity.CompareNoCase("Mexico City"))
		strCity = "Ciudad de Mexico";

	// New Caledonia
	if (!strCity.CompareNoCase("Mont Dore"))
		strCity = "Mont-Dore";

	// Palestine
	if (!strCountry.CompareNoCase("Palestine"))
		strCountry = "West Bank";

	// Phillipines
	if (!strCity.CompareNoCase("Metro Manila"))
		strCity = "Manila";

	// Reunion
	if (!strCity.CompareNoCase("Sainte Clotilde"))
		strCity = "Sainte-Clotilde";

	// Romania
	if (!strCity.CompareNoCase("Miercurea Ciuc"))
		strCity = "Miercurea-Ciuc";
	if (!strCity.CompareNoCase("Camplung"))
		strCity = "Campulungul Moldovenesc";

	// Russia
	if (!strCountry.CompareNoCase("Russia"))
	{
		if (!strCity.CompareNoCase("Astrakhan"))
			strCity = "Astrakhan'";
		if (!strCity.CompareNoCase("Friazino"))
			strCity = "Fryazino";
		if (!strCity.CompareNoCase("Nalchik"))
			strCity = "Nal'chik";
		if (!strCity.CompareNoCase("Nizhny Novgorod"))
			strCity = "Nizhni Novgorod";
		if (!strCity.CompareNoCase("St.Petersburg"))
			strCity = "Saint Petersburg";
		if (!strCity.CompareNoCase("St. Petersburg"))
			strCity = "Saint Petersburg";
		if (!strCity.CompareNoCase("Saint-Petersburg"))
			strCity = "Saint Petersburg";
		if (!strCity.CompareNoCase("Tyumen"))
			strCity = "Tyumen'";
	}

	// Serbia - GEOnet doesn't know "Serbia"
	if (!strCountry.Left(6).CompareNoCase("Serbia"))
		strCountry = "Yugoslavia";

	// Spain
	// "Bellaterra (municipality of Cerdanyola del Vallès), between the towns of
	// Sabadell and Sant Cugat del Vallès."
	if (!strCity.CompareNoCase("Bellaterra"))
		strCity = "Sabadell";
	if (!strCity.Left(13).CompareNoCase("La Bisbal d'E"))
		strCity = "La Bisbal";
	if (!strCity.CompareNoCase("Playa del Ingles"))	// resort on gran canaria
		strCity = "Maspalomas";		// nearest town

	// Switzerland
	if (!strCountry.CompareNoCase("Switzerland"))
	{
		if (!strCity.CompareNoCase("St-Aubin"))
			strCity = "Saint-Aubin";
	}

	// Taiwan
	if (!strCountry.CompareNoCase("Taiwan"))
	{
		if (!strCity.CompareNoCase("Taipei"))
			strCity = "T'ai-pei";
		if (!strCity.CompareNoCase("Hsinchu"))
			strCity = "Hsin-chu";
		if (!strCity.CompareNoCase("Tanshui"))
			strCity = "Tan-shui";
		if (!strCity.CompareNoCase("Taichung"))
			strCity = "T'ai-chung";
	}

	// Tanzania
	if (!strCity.CompareNoCase("Dar-es-Salaam"))
		strCity = "Dar es Salaam";

	// Turkey
	if (!strCity.CompareNoCase("Beysukent"))
		strCity = "Ankara";

	// Ukraine
	if (!strCity.CompareNoCase("Donetsk"))
		strCity = "Donets'k";
	if (!strCity.CompareNoCase("Lviv"))
		strCity = "L'viv";
	if (!strCity.CompareNoCase("Kramatorsk"))
		strCity = "Kramators'k";
	if (!strCity.CompareNoCase("Kharkov"))
		strCity = "Khar'kov";

	// UK
	if (!strCountry.CompareNoCase("United Kingdom"))
	{
		if (!strCity.CompareNoCase("St Leonards on Sea"))
			strCity = "Saint Leonards";
		if (!strCity.CompareNoCase("Walton-On-Thames"))
			strCity = "Walton upon Thames";
		if (!strCity.CompareNoCase("Prenton"))
			strCity = "Birkenhead";
	}

	// (South) Korea
	if (!strCountry.CompareNoCase("Korea (Republic of)"))
		strCountry = "South Korea";
	if (!strCity.CompareNoCase("In'choen"))
		strCity = "Incheon";
	if (!strCity.CompareNoCase("Inchon"))
		strCity = "Inch'on";
	if (!strCity.CompareNoCase("Seocho-dong"))
		strCity = "Seocho";
	if (!strCountry.CompareNoCase("South Korea"))
	{
		// Apparently "-gu" is a generic ending for Korean place names, and
		// the suffix is not stored in the GEOnet database
		int offset = strCity.Find("-gu");
		if (offset != -1)
		{
			VTLOG("  Removing Korean -gu\n");
			strCity.Delete(offset, 3);
		}
	}

	// Yemen
	if (!strCity.CompareNoCase("Sana'a") || !strCity.CompareNoCase("Sana`a"))
		strCity = "Sanaa";

	// If it has no city info, but the country is _very_ small, it's fair to assume
	//  that the location is in the population center of the country.
	if (strCity == "")
	{
		if (!strCountry.CompareNoCase("Bahrain"))
			strCity = "Manama";
		if (!strCountry.CompareNoCase("Singapore"))
			strCity = "Singapore";
		if (!strCountry.CompareNoCase("San Marino"))
			strCity = "San Marino";
		if (!strCountry.CompareNoCase("Reunion"))
			strCity = "Saint-Denis";
		if (!strCountry.CompareNoCase("Bahrain"))
			strCity = "Manama";
	}

	// By this point, we need a city or it's not going to work
	if (strCity == "")
	{
		VTLOG("  Failed: country (%s) and no city\n", (const char *)strCountry);
		return false;
	}

	VTLOG("Trying: (%s), (%s)\n",
		(const char *) strCity, (const char *) strCountry);

	bool bFound = FindPlace(strCountry, strCity, geo);

	// some addresses have a compound name for city, separated by commas,
	// e.g. "Arbroath, Angus"
	// if so, try just first part before the comma
	if (!bFound && strCity.Find(",") != -1)
	{
		int comma_loc = strCity.Find(',');

		vtString newCity = strCity.Left(comma_loc);
		VTLOG("  Trying before comma: (%s)\n", (const char *) newCity);
		bFound = FindPlace(strCountry, newCity, geo);
		if (!bFound)
		{
			// try the part after the comma
			newCity = strCity.Right(strCity.GetLength() - comma_loc - 1);
			newCity.TrimLeft();
			newCity.TrimRight();
			VTLOG("  Trying after comma: (%s)\n", (const char *) newCity);
			bFound = FindPlace(strCountry, newCity, geo);
			if (bFound)
				strCity = newCity;
		}
	}

	// some addresses have a compound name for city, separated by a slash
	if (!bFound && strCity.Find("/") != -1)
	{
		int slash_loc = strCity.Find('/');

		vtString newCity = strCity.Left(slash_loc);
		newCity.TrimRight();
		VTLOG("  Trying before slash: (%s)\n", (const char *) newCity);
		bFound = FindPlace(strCountry, newCity, geo);

		if (!bFound)
		{
			// try the part after the comma
			newCity = strCity.Right(strCity.GetLength() - slash_loc - 1);
			newCity.TrimLeft();
			newCity.TrimRight();
			VTLOG("  Trying after comma: (%s)\n", (const char *) newCity);
			bFound = FindPlace(strCountry, newCity, geo);
			if (bFound)
				strCity = newCity;
		}
	}

	// many addresses (especially in Japan / Korea / China) uses "Foo City"
	//  in place of simply "Foo"
	if (!bFound && !strCity.Right(5).CompareNoCase(" City"))
	{
		strCity = strCity.Left(strCity.GetLength() - 5);
		VTLOG("  Trying without 'city': (%s)\n", (const char *) strCity);
		bFound = FindPlace(strCountry, strCity, geo);
	}

	// some addresses have a small obscure town name, followed by a more easily
	//  found town in parentheses, e.g. Bucasia (Mackay), Australia
	if (!bFound && strCity.Find("(") != -1)
	{
		int paren_loc = strCity.Find('(');

		vtString strCity1 = strCity.Left(paren_loc-1);
		vtString strCity2 = strCity.Mid(paren_loc+1, strCity.GetLength() - paren_loc - 2);

		// remove "near"
		if (strCity2.Left(5) == "near ")
		{
			strCity2 = strCity2.Right(strCity2.GetLength()-5);
		}

		bFound = FindPlace(strCountry, strCity1, geo);
		if (!bFound)
		{
			bFound = FindPlace(strCountry, strCity2, geo);
		}
	}

	if (bFound)
		VTLOG("  Found: %3.2lf, %3.2lf\n", geo.x, geo.y);
	else
		VTLOG("  Failed.\n");

	return bFound;
}

#endif // SUPPORT_WSTRING


///////////////////////////////////////////////////////////////////////
// Gazetteer class: a table of US cities.

vtString Grab(const char *str, int start, int len)
{
	char *buf = new char[len+1];
	strncpy(buf, str+start, len);
	buf[len] = 0;
	vtString vstr = buf;
	delete [] buf;
	return vstr;
}

bool Gazetteer::ReadPlaces(const char *fname)
{
	// safety checks
	if (!fname)
		return false;
	if (*fname == 0)
		return false;

	FILE *fp = vtFileOpen(fname, "rb");
	if (!fp)
		return false;

	char buf[999];
	while (fgets(buf, 999, fp) != NULL)
	{
		Place p;
		p.m_state = Grab(buf, 0, 2);
		p.m_name = Grab(buf, 9, 60);
		p.m_name.TrimRight();

		if (p.m_name.Right(10) == " (balance)")
			p.m_name = p.m_name.Left(p.m_name.GetLength() - 10);

		if (p.m_name.Right(5) == " city")
			p.m_name = p.m_name.Left(p.m_name.GetLength() - 5);
		else if (p.m_name.Right(5) == " town")
			p.m_name = p.m_name.Left(p.m_name.GetLength() - 5);
		else if (p.m_name.Right(4) == " CDP")
			p.m_name = p.m_name.Left(p.m_name.GetLength() - 4);
		else if (p.m_name.Right(13) == " municipality")
			p.m_name = p.m_name.Left(p.m_name.GetLength() - 13);
		else if (p.m_name.Right(17) == " city and borough")
			p.m_name = p.m_name.Left(p.m_name.GetLength() - 17);
		else if (p.m_name.Right(8) == " borough")
			p.m_name = p.m_name.Left(p.m_name.GetLength() - 8);
		else if (p.m_name.Right(8) == " village")
			p.m_name = p.m_name.Left(p.m_name.GetLength() - 8);
		else if (p.m_name.Right(10) == " comunidad")
			p.m_name = p.m_name.Left(p.m_name.GetLength() - 10);
		else if (p.m_name.Right(12) == " zona urbana")
			p.m_name = p.m_name.Left(p.m_name.GetLength() - 12);
		else if (p.m_name.Right(7) == " urbana")
			p.m_name = p.m_name.Left(p.m_name.GetLength() - 7);
		else
		{
			int foo = 1;
		}
		double x = atof(buf+153);
		double y = atof(buf+143);
		p.geo.Set(x, y);

		m_places.push_back(p);
	}
	fclose(fp);
	return true;
}

bool Gazetteer::ReadZips(const char *fname)
{
	// safety checks
	if (!fname)
		return false;
	if (*fname == 0)
		return false;

	FILE *fp = vtFileOpen(fname, "rb");
	if (!fp)
		return false;

	vtString str;
	char buf[999];
	while (fgets(buf, 999, fp) != NULL)
	{
		Zip p;

		str = Grab(buf, 2, 5);
		if (str[4] == 'H' || str[4] == 'X')
			continue;

		p.m_zip = atoi(str);

		double x = atof(buf+146);
		double y = atof(buf+136);
		p.geo.Set(x, y);

		m_zips.push_back(p);
	}

	fclose(fp);
	return true;
}

bool Gazetteer::FindPlace(const vtString &state, const vtString &place_org, DPoint2 &geo)
{
	vtString place = place_org;
	if (!place.CompareNoCase("Saint Paul"))
		place = "St. Paul";
	if (!place.CompareNoCase("Essex Jct."))
		place = "Essex Junction";
	if (!place.CompareNoCase("Boise"))
		place = "Boise City";
	if (!place.CompareNoCase("Moon Township"))
		place = "Carnot-Moon";
	if (!place.CompareNoCase("Brick"))	// New Jersey
		place = "Laurelton";
	if (!place.CompareNoCase("Point Mugu"))	// California
		place = "Port Hueneme";		// nearest town
	if (!place.CompareNoCase("N. Massapequa"))
		place = "North Massapequa";
	if (!place.CompareNoCase("Clinton Township"))
		place = "Clinton";
	if (!place.CompareNoCase("Ft. Rucker"))
		place = "Fort Rucker";
	if (!place.CompareNoCase("Ft. Knox"))
		place = "Fort Know";
	if (!place.CompareNoCase("Tampa Bay"))
		place = "Tampa";
	if (!place.CompareNoCase("Ft. Lauderdale"))
		place = "Fort Lauderdale";
	if (!state.Compare("GA") && !place.CompareNoCase("Augusta"))
		place = "Augusta-Richmond County";
	if (!state.Compare("CA") && !place.CompareNoCase("Paso Robles"))
		place = "El Paso de Robles (Paso Robles)";
	if (!place.CompareNoCase("Urbana/Champaign"))
		place = "Urbana";
	if (!state.Compare("ME") && !place.CompareNoCase("Rockport"))
		place = "Camden";	// closest known town
	if (!state.Compare("CA") && !place.CompareNoCase("Cardiff-by-the-sea"))
		place = "Encinitas";	// closest known town
	if (!state.Compare("UT") && !place.CompareNoCase("Salt Lake"))
		place = "Salt Lake City";
	if (!state.Compare("MA") && !place.CompareNoCase("Littleton"))
		place = "Littleton Common";

	// Some missing places:
	// Valley, WA
	// Brooklyn, NY
	// Long Island City, NY
	// Redstone, CO
	// La Jolla, CA		(neighboorhood of San Diego)
	// Littleton, MA
	// Saunderstown, RI
	// Ashburn, VA		(speck outside DC)

	for (uint i = 0; i < m_places.size(); i++)
	{
		const Place &p = m_places[i];
		if (p.m_state.Compare(state) == 0 && p.m_name.CompareNoCase(place) == 0)
		{
			geo = p.geo;
			return true;
		}
	}
	return false;
}

bool Gazetteer::FindZip(int zip, DPoint2 &geo)
{
	for (uint i = 0; i < m_zips.size(); i++)
	{
		const Zip &p = m_zips[i];
		if (p.m_zip == zip)
		{
			geo = p.geo;
			return true;
		}
	}
	return false;
}


