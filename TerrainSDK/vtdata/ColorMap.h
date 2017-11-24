//
// ColorMap.h
//
// Copyright (c) 2002-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTDATA_COLORMAP_H
#define VTDATA_COLORMAP_H

#include "MathTypes.h"

/**
 * This small class describes how to map elevation (as from a heightfield)
 * onto a set of colors.
 */
class ColorMap
{
public:
	ColorMap();
	~ColorMap();

	bool Save(const char *fname) const;
	bool Load(const char *fname);
	void Add(float elev, const RGBi &color);
	float Elev(int index) const { return m_elev[index]; }
	const RGBi &Color(int index) const { return m_color[index]; }
	void SetColor(int index, const RGBi &rgb) { m_color[index] = rgb; }
	void RemoveAt(int num);
	void Clear();
	int Num() const;
	void GenerateColorTable(int iTableSize, float fMin, float fMax);
	const RGBi &ColorFromTable(float fElev) const
	{
		return m_table[(uint)((fElev - m_fMin) / m_fRange * m_iTableSize)];
	}

	// Fast lookup
	std::vector<RGBi> m_table;	// A large table made to speed up color lookups
	float m_fMin, m_fMax, m_fRange;
	int m_iTableSize;

	bool m_bBlend;
	bool m_bRelative;

protected:
	std::vector<float> m_elev;	// The elevation of each reference color
	std::vector<RGBi> m_color;	// Each reference color
};

#endif  // VTDATA_COLORMAP_H
