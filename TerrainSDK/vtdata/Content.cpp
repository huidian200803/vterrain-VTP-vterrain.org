//
// Content.cpp
//
// Implements the ContentManager class, including the ability to read
// and write the contents to an XML file.
//
// Copyright (c) 2001-2006 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#include <stdio.h>
#include "Content.h"
#include "vtLog.h"
#include "FilePath.h"	// for vtFileOpen

////////////////////////////////////////////////////////////////////////
// Implementation of class vtItem
//

vtItem::vtItem()
{
	m_extents.SetToZero();
}

vtItem::~vtItem()
{
	// clean up
	DeleteModels();
}

void vtItem::DeleteModels()
{
	for (uint i = 0; i < m_models.GetSize(); i++)
		delete m_models[i];

	m_models.Clear();
}

void vtItem::RemoveModel(vtModel *model)
{
	int i = m_models.Find(model);
	m_models.RemoveAt(i);
	delete model;
}

////////////////////////////////////////////////////////////////////////
// Visitor class, for XML parsing of Content files.
////////////////////////////////////////////////////////////////////////

class ContentVisitor : public XMLVisitor
{
public:
	ContentVisitor(vtContentManager *man)
	: _level(0), m_pMan(man) {}

	virtual ~ContentVisitor() {}

	void startXML();
	void endXML();
	void startElement(const char * name, const XMLAttributes &atts);
	void endElement(const char * name);
	void data(const char * s, int length);

private:
	struct State
	{
		State () : item(0), type("") {}
		State (vtItem * _item, const char * _type)
			: item(_item), type(_type) {}
		vtItem * item;
		string type;
	};

	State &state () { return _state_stack[_state_stack.size() - 1]; }

	void push_state(vtItem * _item, const char * type)
	{
		if (type == 0)
			_state_stack.push_back(State(_item, "unspecified"));
		else
			_state_stack.push_back(State(_item, type));
		_level++;
		_data = "";
	}

	void pop_state()
	{
		_state_stack.pop_back();
		_level--;
	}

	string _data;
	int _level;
	vector<State> _state_stack;

	vtContentManager *m_pMan;
};

void ContentVisitor::startXML ()
{
  _level = 0;
  _state_stack.resize(0);
}

void ContentVisitor::endXML ()
{
  _level = 0;
  _state_stack.resize(0);
}


void ContentVisitor::startElement(const char *name, const XMLAttributes &atts)
{
	if (_level == 0)
	{
		if (string(name) != (string)"vtp-content") {
			string message = "Root element name is ";
			message += name;
			message += "; expected vtp-content";
			throw xh_io_exception(message, "XML Reader");
		}
		push_state(NULL, "top");
		return;
	}

	const char * attval;

	if (_level == 1)
	{
		if (string(name) == (string)"item")
		{
			vtItem *pItem = m_pMan->NewItem();
			// Get the name.
			attval = atts.getValue("name");
			if (attval != NULL) {
				pItem->m_name = attval;
			}
			push_state(pItem, "item");
		}
		else
		{
			// Unknown field, ignore.
			push_state(NULL, "dummy");
		}
		return;
	}

	if (_level == 2)
	{
		State &st = state();
		vtItem *pItem = st.item;

		if (string(name) == (string)"model")
		{
			vtModel *pModel = new vtModel;
			pModel->m_filename = atts.getValue("filename");

			attval = atts.getValue("distance");
			if (attval && *attval)
				pModel->m_distance = (float)atof(attval);
			else
				pModel->m_distance = 0.0f;

			attval = atts.getValue("scale");
			if (attval && *attval)
				pModel->m_scale = (float)atof(attval);

			pItem->AddModel(pModel);
		}
		else
			_data = "";
	}
}

void ContentVisitor::endElement(const char * name)
{
	State &st = state();

	if (string(name) == (string)"item")
	{
		if (st.item != NULL)
		{
			m_pMan->AddItem(st.item);
		}
		pop_state();
	}
	if (_level == 2)
	{
		vtItem *pItem = st.item;
		if (string(name) == (string)"model")
		{
		}
		else if (string(name) == (string)"extents")
		{
			FRECT ext;
			int result = sscanf(_data.c_str(), "%f, %f, %f, %f", &ext.left,
				&ext.top, &ext.right, &ext.bottom);
			if (result == 4)
				pItem->m_extents = ext;
		}
		else
		{
			// save all other tags as literal strings
			vtTag tag;
			tag.name = name;
			tag.value = _data.c_str();
			pItem->AddTag(tag);
		}
	}
}

void ContentVisitor::data(const char * s, int length)
{
	if (state().item != NULL)
		_data.append(string(s, length));
}


////////////////////////////////////////////////////////////////////////
// vtContentManager class implementation.
////////////////////////////////////////////////////////////////////////


vtContentManager::~vtContentManager()
{
	// clean up
	Clear();
}

void vtContentManager::Clear()
{
	uint items = m_items.GetSize();
	if (items)
		VTLOG("vtContentManager::Clear, %d items to delete\n", items);
	for (uint i = 0; i < items; i++)
		delete m_items[i];

	m_items.Clear();
}

void vtContentManager::RemoveItem(vtItem *item)
{
	int i = m_items.Find(item);
	if (i != -1)
	{
		m_items.RemoveAt(i);
		delete item;
	}
}

vtItem *vtContentManager::FindItemByName(const char *name)
{
	for (uint i = 0; i < m_items.GetSize(); i++)
	{
		vtItem *pItem = m_items[i];
		if (!pItem->m_name.CompareNoCase(name))
			return pItem;
	}
	return NULL;
}

vtItem *vtContentManager::FindItemByType(const char *type, const char *subtype)
{
	for (uint i = 0; i < m_items.GetSize(); i++)
	{
		vtItem *pItem = m_items[i];
		vtTag *tag1 = pItem->FindTag("type");
		if (tag1 && !tag1->value.CompareNoCase(type))
		{
			if (subtype)
			{
				vtTag *tag2 = pItem->FindTag("subtype");
				if (tag2 && !tag2->value.CompareNoCase(subtype))
					return pItem;
			}
			else
				return pItem;
		}
	}
	return NULL;
}

/**
 * Read content items from an XML file.
 *
 * There is no return value because if there is an error, an
 * xh_io_exception will be thrown.
 *
 * \param filename A string containing the file path.
 */
void vtContentManager::ReadXML(const char *filename)
{
	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	ContentVisitor visitor(this);
	readXML(filename, visitor);

	// If no exception was thrown, store the name of the file we loaded from
	m_strFilename = filename;
}


/**
 * Write content items to an XML file.
 *
 * There is no return value because if there is an error, an
 * xh_io_exception will be thrown.
 *
 * \param filename A string containing the file path.
 */
void vtContentManager::WriteXML(const char *filename) const
{
	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	uint i, j;
	FILE *fp = vtFileOpen(filename, "wb");
	if (!fp)
	{
		throw xh_io_exception("Failed to open file", xh_location(filename),
				"XML Parser");
	}

	fprintf(fp, "<?xml version=\"1.0\"?>\n\n");

	fprintf(fp, "<vtp-content file-format-version=\"1.1\">\n");
	for (i = 0; i < m_items.GetSize(); i++)
	{
		vtItem *pItem = m_items[i];

		// Write name
		const char *name = pItem->m_name;
		fprintf(fp, "\t<item name=\"%s\">\n", name);

		// Write tags
		for (j = 0; j < pItem->NumTags(); j++)
		{
			vtTag *tag = pItem->GetTag(j);
			fprintf(fp, "\t\t<%s>%s</%s>\n", (const char *)tag->name,
				(const char *)tag->value, (const char *)tag->name);
		}

		// Write extents
		FRECT ext = pItem->m_extents;
		if (!ext.IsEmpty())
		{
			fprintf(fp, "\t\t<extents>%.2f, %.2f, %.2f, %.2f</extents>\n",
				ext.left, ext.top, ext.right, ext.bottom);
		}

		// Write models
		for (j = 0; j < pItem->NumModels(); j++)
		{
			vtModel *pMod = pItem->GetModel(j);
			const char *filename = pMod->m_filename;
			fprintf(fp, "\t\t<model filename=\"%s\"", filename);
			if (pMod->m_distance != 0.0f)
				fprintf(fp, " distance=\"%g\"", pMod->m_distance);
			if (pMod->m_scale != 1.0f)
				fprintf(fp, " scale=\"%g\"", pMod->m_scale);
			fprintf(fp, " />\n");
		}
		fprintf(fp, "\t</item>\n");
	}
	fprintf(fp, "</vtp-content>\n");
	fclose(fp);
}

