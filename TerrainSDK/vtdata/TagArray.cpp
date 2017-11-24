//
// TagArray.cpp
//
// Copyright (c) 2001-2015 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#include "TagArray.h"
#include "vtLog.h"
#include "FilePath.h"	// for vtFileOpen

////////////////////////////////////////////////////////////////////////
// Implementation of class vtTagArray
//

bool vtTagArray::s_bVerbose = false;

void vtTagArray::AddTag(const vtTag &pTag)
{
	m_tags.push_back(pTag);
}

void vtTagArray::AddTag(const char *name, const char *value)
{
	vtTag tag;
	tag.name = name;
	tag.value = value;
	m_tags.push_back(tag);
}

vtTag *vtTagArray::FindTag(const char *name)
{
	int size = m_tags.size();
	if (s_bVerbose)
		VTLOG(" FindTag('%s'), %d tags\n", name, size);

	for (int i = 0; i < size; i++)
	{
		vtTag *tag = &m_tags[i];

		if (s_bVerbose)
			VTLOG("  tag %d: '%s'\n", i, (const char *)tag->name);

		if (!tag->name.CompareNoCase(name))
		{
			if (s_bVerbose)
				VTLOG("  Found tag %d (value '%s')\n", i, (const char*)tag->value);
			return tag;
		}
	}
	if (s_bVerbose)
		VTLOG("  Tag not found.\n");
	return NULL;
}

const vtTag *vtTagArray::FindTag(const char *name) const
{
	int size = m_tags.size();

	if (s_bVerbose)
		VTLOG(" FindTag('%s'), %d tags\n", name, size);

	for (int i = 0; i < size; i++)
	{
		const vtTag *tag = &m_tags[i];

		if (s_bVerbose)
			VTLOG("  tag %d: '%s'\n", i, (const char *)tag->name);

		if (!tag->name.CompareNoCase(name))
		{
			if (s_bVerbose)
				VTLOG("  Found tag %d (value '%s')\n", i, (const char*)tag->value);
			return tag;
		}
	}
	if (s_bVerbose)
		VTLOG("  Tag not found.\n");
	return NULL;
}

vtTag *vtTagArray::GetTag(int index)
{
	return &m_tags[index];
}

const vtTag *vtTagArray::GetTag(int index) const
{
	return &m_tags[index];
}

uint vtTagArray::NumTags() const
{
	return m_tags.size();
}

void vtTagArray::RemoveTag(int index)
{
	m_tags.erase(m_tags.begin() + index);
}

void vtTagArray::RemoveTag(const char *szTagName)
{
	int size = m_tags.size();
	vtTag *tag;
	for (int i = 0; i < size; i++)
	{
		tag = &m_tags[i];
		if (!tag->name.CompareNoCase(szTagName))
		{
			m_tags.erase(m_tags.begin() + i);
			break;
		}
	}
}

void vtTagArray::Clear()
{
	m_tags.clear();
}

//
// Set
//

void vtTagArray::SetValueString(const char *szTagName, const vtString &string,
	bool bCreating)
{
	vtTag *tag = FindTag(szTagName);
	if (tag)
		tag->value = string;
	else
	{
		// might be overridden by a derived class
		if (OverrideValue(szTagName, string))
			return;

		// if not, add it as a new tag
		if (!bCreating)
			VTLOG("\tWarning: tag %s was not found, creating.\n", szTagName);
		AddTag(szTagName, string);
	}
}

void vtTagArray::SetValueBool(const char *szTagName, bool bValue, bool bCreating)
{
	if (bValue)
		SetValueString(szTagName, "true", bCreating);
	else
		SetValueString(szTagName, "false", bCreating);
}

void vtTagArray::SetValueInt(const char *szTagName, int iValue, bool bCreating)
{
	vtString str;
	str.Format("%d", iValue);
	SetValueString(szTagName, str, bCreating);
}

void vtTagArray::SetValueFloat(const char *szTagName, float fValue, bool bCreating)
{
	vtString str;
	str.Format("%f", fValue);
	SetValueString(szTagName, str, bCreating);
}

void vtTagArray::SetValueDouble(const char *szTagName, double dValue, bool bCreating)
{
	vtString str;
	str.Format("%lf", dValue);
	SetValueString(szTagName, str, bCreating);
}

void vtTagArray::SetValueRGBi(const char *szTagName, const RGBi &color, bool bCreating)
{
	vtString str;
	str.Format("%d %d %d", color.r, color.g, color.b);
	SetValueString(szTagName, str, bCreating);
}


//
// Get values directly.  Convenient syntax, but can't report failure
//  if the tag doesn't exist.
//

const char *vtTagArray::GetValueString(const char *szTagName, bool bSuppressWarning) const
{
	const vtTag *tag = FindTag(szTagName);
	if (tag)
		return tag->value;
	else
	{
		if (!bSuppressWarning)
			VTLOG("\tWarning: could not get tag '%s', not found.\n", szTagName);
		return NULL;
	}
}

bool vtTagArray::GetValueBool(const char *szTagName) const
{
	if (s_bVerbose)
		VTLOG("GetValueBool('%s')\n", szTagName);
	const vtTag *tag = FindTag(szTagName);
	if (tag)
		return (tag->value[0] == 't' || tag->value[0] == '1');
	return false;
}

int vtTagArray::GetValueInt(const char *szTagName) const
{
	if (s_bVerbose)
		VTLOG("GetValueInt('%s')\n", szTagName);
	const vtTag *tag = FindTag(szTagName);
	if (tag)
		return atoi((const char *)tag->value);
	return 0;
}

float vtTagArray::GetValueFloat(const char *szTagName) const
{
	if (s_bVerbose)
		VTLOG("GetValueFloat('%s')\n", szTagName);
	const vtTag *tag = FindTag(szTagName);
	if (tag)
		return (float)atof((const char *)tag->value);
	return 0.0f;
}

double vtTagArray::GetValueDouble(const char *szTagName) const
{
	if (s_bVerbose)
		VTLOG("GetValueDouble('%s')\n", szTagName);
	const vtTag *tag = FindTag(szTagName);
	if (tag)
		return atof((const char *)tag->value);
	return 0.0;
}

RGBi vtTagArray::GetValueRGBi(const char *szTagName) const
{
	if (s_bVerbose)
		VTLOG("GetValueRGBi('%s')\n", szTagName);
	const vtTag *tag = FindTag(szTagName);
	if (tag)
	{
		RGBi color;
		if (sscanf(tag->value, "%hd %hd %hd", &color.r, &color.g, &color.b) == 3)
			return color;
	}
	return RGBi(-1, -1, -1);
}

//
// Get by reference
//
bool vtTagArray::GetValueString(const char *szTagName, vtString &string) const
{
	const vtTag *tag = FindTag(szTagName);
	if (tag)
		string = tag->value;
	return (tag != NULL);
}

bool vtTagArray::GetValueBool(const char *szTagName, bool &bValue) const
{
	if (s_bVerbose)
		VTLOG("GetValueBool('%s')\n", szTagName);
	const vtTag *tag = FindTag(szTagName);
	if (tag)
		bValue = (tag->value[0] == 't' || tag->value[0] == '1');
	return (tag != NULL);
}

bool vtTagArray::GetValueInt(const char *szTagName, int &iValue) const
{
	if (s_bVerbose)
		VTLOG("GetValueInt('%s')\n", szTagName);
	const vtTag *tag = FindTag(szTagName);
	if (tag)
		iValue = atoi((const char *)tag->value);
	return (tag != NULL);
}

bool vtTagArray::GetValueFloat(const char *szTagName, float &fValue) const
{
	if (s_bVerbose)
		VTLOG("GetValueFloat('%s')\n", szTagName);
	const vtTag *tag = FindTag(szTagName);
	if (tag)
		fValue = (float)atof((const char *)tag->value);
	return (tag != NULL);
}

bool vtTagArray::GetValueDouble(const char *szTagName, double &dValue) const
{
	if (s_bVerbose)
		VTLOG("GetValueDouble('%s')\n", szTagName);
	const vtTag *tag = FindTag(szTagName);
	if (tag)
		dValue = atof((const char *)tag->value);
	return (tag != NULL);
}

bool vtTagArray::GetValueRGBi(const char *szTagName, RGBi &color) const
{
	const vtTag *tag = FindTag(szTagName);
	if (tag)
	{
		int r, g, b;
		if (sscanf(tag->value, "%d %d %d", &r, &g, &b) == 3)
			color.Set(r, g, b);
		else
			return false;
	}
	return (tag != NULL);
}

//
// Operators
//
vtTagArray &vtTagArray::operator=(const vtTagArray &v)
{
	m_tags = v.m_tags;
	return *this;
}

bool vtTagArray::operator==(const vtTagArray &v) const
{
	uint size = m_tags.size();
	if (size != v.m_tags.size())
		return false;

	for (uint i = 0; i < size; i++)
	{
		const vtTag *tag = GetTag(i);
		const vtTag *tag2 = v.FindTag(tag->name);
		if (!tag2)
			return false;
		if (*tag != *tag2)
			return false;
	}
	return true;
}

bool vtTagArray::operator!=(const vtTagArray &v) const
{
	//	return (m_tags != v.m_tags);
	return true;
}

void vtTagArray::CopyTagsFrom(const vtTagArray &v)
{
	for (uint i = 0; i < v.NumTags(); i++)
	{
		const vtTag *tag = v.GetTag(i);
		SetValueString(tag->name, tag->value, true);	// suppress warn
	}
}

// File IO
bool vtTagArray::WriteToXML(const char *fname, const char *title) const
{
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	FILE *fp = vtFileOpen(fname, "wb");
	if (!fp)
		return false;

	fprintf(fp, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	fprintf(fp, "<%s>\n", title);
	WriteToXMLBody(fp, 1);
	fprintf(fp, "</%s>\n", title);
	fclose(fp);
	return true;
}

void vtTagArray::WriteToXMLBody(FILE *fp, int iIndent) const
{
	for (uint i = 0; i < NumTags(); i++)
	{
		// indent
		for (int j = 0; j < iIndent; j++)
			fprintf(fp, "\t");

		// write field as an XML element
		const vtTag *tag = GetTag(i);
		vtString asXml = EscapeStringForXML(tag->value);
		fprintf(fp, "<%s>%s</%s>\n", (const char *)tag->name,
			(const char *)asXml, (const char *)tag->name);
	}
	WriteOverridesToXML(fp);
}

////////////////////////////////////////////////////////////////////////
// class TagVisitor, for XML parsing of Content files.

void TagVisitor::startElement(const char *name, const XMLAttributes &atts)
{
	m_level++;
	m_data = "";
}

void TagVisitor::endElement(const char *name)
{
	if (m_level == 2)
		m_pArray->SetValueString(name, m_data.c_str(), true);
	m_level--;
}

void TagVisitor::data(const char *s, int length)
{
	m_data.append(string(s, length));
}

bool vtTagArray::LoadFromXML(const char *fname)
{
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	TagVisitor visitor(this);
	try
	{
		readXML(fname, visitor);
	}
	catch (xh_io_exception &ex)
	{
		const string msg = ex.getFormattedMessage();
		VTLOG(" XML problem: %s\n", msg.c_str());
		return false;
	}
	return true;
}

void vtTagArray::SetVerbose(bool value)
{
	VTLOG("vtTagArray::SetVerbose %d\n", value);
	s_bVerbose = value;
}

void vtTagArray::LogTags() const
{
	uint size = NumTags();
	VTLOG(" LogTags: %d tags\n", size);
	for (uint i = 0; i < size; i++)
	{
		const vtTag *tag = &m_tags[i];
		VTLOG("  tag %d: '%s'\n", i, (const char *)tag->name);
	}
}

