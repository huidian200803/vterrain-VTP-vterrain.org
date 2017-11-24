//
// Content.h
//
// Header for the vtTagArray.
//
// Copyright (c) 2001-2015 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#pragma once

#include "vtString.h"
#include "MathTypes.h"
#include "xmlhelper/easyxml.hpp"

/**
 * Each tag has two strings: a Name and a Value.
 * This is similar to the concept of a tag in XML.
 */
class vtTag
{
public:
	vtString name;
	vtString value;

	bool operator==(const vtTag &v) const
	{
		return (name == v.name && value == v.value);
	}
	bool operator!=(const vtTag &v) const
	{
		return (name != v.name || value != v.value);
	}
};

/**
 * A simple set of tags.  Each tag (vtTag) has two strings: a Name and a Value.
 * This is similar to the concept of a tag in XML.
 * \par
 * If this gets used for something more performance-sensitive, we could replace
 * the linear lookup with a hash map.
 */
class vtTagArray
{
public:
	virtual ~vtTagArray() {}

	void AddTag(const vtTag &pTag);
	void AddTag(const char *name, const char *value);

	vtTag *FindTag(const char *szTagName);
	const vtTag *FindTag(const char *szTagName) const;

	vtTag *GetTag(int index);
	const vtTag *GetTag(int index) const;

	uint NumTags() const;
	void RemoveTag(int index);
	void RemoveTag(const char *szTagName);
	void Clear();

	// Set value
	void SetValueString(const char *szTagName, const vtString &string, bool bCreating = false);
	void SetValueBool(const char *szTagName, bool value, bool bCreating = false);
	void SetValueInt(const char *szTagName, int value, bool bCreating = false);
	void SetValueFloat(const char *szTagName, float value, bool bCreating = false);
	void SetValueDouble(const char *szTagName, double value, bool bCreating = false);
	void SetValueRGBi(const char *szTagName, const RGBi &value, bool bCreating = false);

	// Get value directly
	const char *GetValueString(const char *szTagName, bool bSuppressWarning = false) const;
	bool GetValueBool(const char *szTagName) const;
	int GetValueInt(const char *szTagName) const;
	float GetValueFloat(const char *szTagName) const;
	double GetValueDouble(const char *szTagName) const;
	RGBi GetValueRGBi(const char *szTagName) const;

	// Get by reference
	bool GetValueString(const char *szTagName, vtString &string) const;
	bool GetValueBool(const char *szTagName, bool &bValue) const;
	bool GetValueInt(const char *szTagName, int &iValue) const;
	bool GetValueFloat(const char *szTagName, float &fValue) const;
	bool GetValueDouble(const char *szTagName, double &dValue) const;
	bool GetValueRGBi(const char *szTagName, RGBi &color) const;

	// Operators
	vtTagArray &operator=(const vtTagArray &v);
	bool operator==(const vtTagArray &v) const;
	bool operator!=(const vtTagArray &v) const;

	// Copy each tag from one array to another
	void CopyTagsFrom(const vtTagArray &v);

	// File IO
	bool WriteToXML(const char *fname, const char *title) const;
	void WriteToXMLBody(FILE *fp, int iIndent) const;
	bool LoadFromXML(const char *fname);

	// Allow overriding values by subclasses
	virtual bool OverrideValue(const char *szTagName, const vtString &string)
	{
		return false;
	}
	virtual void WriteOverridesToXML(FILE *fp) const {}

	// Debugging info
	static void SetVerbose(bool value);
	void LogTags() const;

protected:
	std::vector<vtTag>	m_tags;
	static bool s_bVerbose;		// display debugging info
};


////////////////////////////////////////////////////////////////////////
// Visitor class for XML parsing of TagArray files.
// This is further subclasses by specialized TagArray parsers.

class TagVisitor : public XMLVisitor
{
public:
	TagVisitor(vtTagArray *pArray) : m_level(0), m_pArray(pArray) {}
	void startElement(const char *name, const XMLAttributes &atts);
	void endElement(const char *name);
	void data(const char *s, int length);

protected:
	int m_level;
	string m_data;
	vtTagArray *m_pArray;
};

