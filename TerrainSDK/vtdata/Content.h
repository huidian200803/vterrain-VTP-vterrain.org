//
// Content.h
//
// Header for the Content Management classes.
//
// Copyright (c) 2001-2006 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#pragma once

#include "vtString.h"
#include "MathTypes.h"
#include "TagArray.h"

/**
 * vtModel contains a reference to a 3d polygonal model: a filename, the
 * scale of the model, and the distance at which this LOD should be used.
 */
class vtModel
{
public:
	vtModel()
	{
		m_distance = 0.0f;
		m_scale = 1.0f;
		m_attempted_load = false;
	}

	vtString	m_filename;
	float		m_distance;
	float		m_scale;	// meters per unit (e.g. cm = .01)
	bool		m_attempted_load;
};

/**
 * Represents a "culture" item.  A vtItem has a name and any number of tags
 * which provide description.  It also contains a set of models (vtModel)
 * which are polygonal models of the item at various LOD.
 */
class vtItem : public vtTagArray
{
public:
	vtItem();
	virtual ~vtItem();

	void DeleteModels();
	void AddModel(vtModel *item) { m_models.Append(item); }
	void RemoveModel(vtModel *model);
	uint NumModels() { return m_models.GetSize(); }
	vtModel *GetModel(int i) { return m_models[i]; }

	vtString	m_name;
	FRECT		m_extents;

protected:
	vtArray<vtModel*>	m_models;
};

/**
 * The vtContentManager class keeps a list of 3d models, along with
 * information about what they are and how they should be loaded.
 * It consists of a set of Content Items (vtItem) which each represent a
 * particular object, which in turn consist of Models (vtModel) which are a
 * particular 3D geometry for that Item.  An Item can have several Models
 * which represent different levels of detail (LOD).
 * \par
 * To load a set of content  from a file, first create a
 * vtContentManager object, then call ReadXML() with the name of name of a
 * VT Content file (.vtco).
 */
class vtContentManager
{
public:
	virtual ~vtContentManager();

	void ReadXML(const char *filename);
	void WriteXML(const char *filename) const;

	void Clear();
	void AddItem(vtItem *item) { m_items.Append(item); }
	void RemoveItem(vtItem *item);
	uint NumItems() { return m_items.GetSize(); }
	virtual vtItem *NewItem() { return new vtItem; }
	vtItem *GetItem(int i) { return m_items[i]; }
	vtItem *FindItemByName(const char *name);
	vtItem *FindItemByType(const char *type, const char *subtype);
	vtString GetFilename() { return m_strFilename; }

protected:
	vtArray<vtItem*>	m_items;
	vtString m_strFilename;
};

