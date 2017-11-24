//
// LayerBase.h
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef LAYER_BASE_H
#define LAYER_BASE_H

enum LayerType
{
	LT_UNKNOWN = -1,
	LT_RAW,
	LT_ELEVATION,
	LT_IMAGE,
	LT_ROAD,
	LT_STRUCTURE,
	LT_WATER,
	LT_VEG,
	LT_UTILITY,
	LAYER_TYPES,

	LT_ABSTRACT = LT_RAW	// A synonym.
};

/**
 * The basic functionality of a layer, to be shared by higher-level applications.
 */
class vtLayerBase
{
public:
	vtLayerBase(LayerType type)
	{
		m_type = type;
		m_bVisible = true;
		m_bModified = false;
	}

	// attributes
	LayerType GetType() { return m_type; }
	virtual void SetVisible(bool bVisible) { m_bVisible = bVisible; }
	bool GetVisible() { return m_bVisible; }
	void SetModified(bool bModified = true)
	{
		bool bChanged = (m_bModified != bModified);
		m_bModified = bModified;
		if (bChanged)
			OnModifiedChange();
	}
	bool GetModified() { return m_bModified; }

	virtual void OnModifiedChange() {}

protected:
	LayerType	m_type;
	bool		m_bVisible;
	bool		m_bModified;
};

#endif // LAYER_BASE_H
