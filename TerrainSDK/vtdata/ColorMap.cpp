//
// HeightField.cpp
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "ColorMap.h"
#include "FilePath.h"

//
// Class implementation: ColorMap
//
ColorMap::ColorMap()
{
	m_bBlend = true;
	m_bRelative = true;
}

ColorMap::~ColorMap()
{
}

bool ColorMap::Save(const char *fname) const
{
	// watch out for %f
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	FILE *fp = vtFileOpen(fname, "wb");
	if (!fp)
		return false;
	fprintf(fp, "colormap1\n");
	fprintf(fp, "blend: %d\n", m_bBlend);
	fprintf(fp, "relative: %d\n", m_bRelative);
	int size = m_elev.size();
	fprintf(fp, "size %d\n", size);
	for (int i = 0; i < size; i++)
	{
		fprintf(fp, "\telev %f color %d %d %d\n", m_elev[i],
			m_color[i].r, m_color[i].g, m_color[i].b);
	}
	fclose(fp);
	return true;
}

bool ColorMap::Load(const char *fname)
{
	// watch out for %f
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	FILE *fp = vtFileOpen(fname, "rb");
	if (!fp)
		return false;

	char buf[80];
	fgets(buf, 80, fp);
	if (strncmp(buf, "colormap1", 9))
		return false;

	while (fgets(buf, 80, fp) != NULL)
	{
		int ival;
		if (!strncmp(buf, "blend", 5))
		{
			sscanf(buf, "blend: %d\n", &ival);
			m_bBlend = (ival != 0);
		}

		else if (!strncmp(buf, "relative", 8))
		{
			sscanf(buf, "relative: %d\n", &ival);
			m_bRelative = (ival != 0);
		}

		else if (!strncmp(buf, "size", 4))
		{
			int size;
			sscanf(buf, "size %d\n", &size);

			m_elev.resize(size);
			m_color.resize(size);
			for (int i = 0; i < size; i++)
			{
				float f;
				short r, g, b;
				fscanf(fp, "\telev %f color %hd %hd %hd\n", &f, &r, &g, &b);
				m_elev[i] = f;
				m_color[i].Set(r, g, b);
			}
		}
	}
	fclose(fp);
	return true;
}

/**
 * Add a color entry, keeping the elevation values sorted.
 */
void ColorMap::Add(float elev, const RGBi &color)
{
	float next, previous = -1E9;
	int size = m_elev.size();
	for (int i = 0; i < size+1; i++)
	{
		if (i < size)
			next = m_elev[i];
		else
			next = 1E9;
		if (previous <= elev && elev <= next)
		{
			m_elev.insert(m_elev.begin() + i, elev);
			m_color.insert(m_color.begin() + i, color);
			return;
		}
	}
}

void ColorMap::RemoveAt(int num)
{
	m_elev.erase(m_elev.begin()+num);
	m_color.erase(m_color.begin()+num);
}

void ColorMap::Clear()
{
	m_elev.clear();
	m_color.clear();
}

int ColorMap::Num() const
{
	return m_elev.size();
}

/**
 * Tell this ColorMap to generate an internal table of interpolated colors.
 * This sets it up to use ColorFromTable().
 *
 * \param iTableSize The desired number of elements in the table.
 * \param fMin, fMax The elevation range to interpolate over.
 */
void ColorMap::GenerateColorTable(int iTableSize, float fMin, float fMax)
{
	if (m_color.size() < 2)
		return;

	m_iTableSize = iTableSize;
	m_fMin = fMin;
	m_fMax = fMax;
	m_fRange = fMax - fMin;
	float step = m_fRange/iTableSize;

	int current = 0;
	int num = Num();
	RGBi c1, c2;
	float base = 0, next, bracket_size = 0, fraction;

	// In case this method is called more than once, be sure to empty the array.
	m_table.clear();

	if (m_bRelative == true)
	{
		bracket_size = m_fRange / (num - 1);
		current = -1;
	}

	RGBi c3;
	for (int i = 0; i < iTableSize; i++)
	{
		float elev = fMin + (step * i);
		if (m_bRelative)
		{
			// use regular divisions
			int bracket = (int) ((elev-fMin) / m_fRange * (num-1));
			if (bracket != current)
			{
				current = bracket;
				base = fMin + bracket * bracket_size;
				c1 = m_color[current];
				c2 = m_color[current+1];
			}
		}
		else
		{
			// use absolute elevations
			while (current < num-1 && elev >= m_elev[current])
			{
				c1 = m_color[current];
				c2 = m_color[current+1];
				base = m_elev[current];
				next = m_elev[current+1];
				bracket_size = next - base;
				current++;
			}
		}
		if (m_bBlend)
		{
			fraction = (elev - base) / bracket_size;
			c3 = c1 * (1-fraction) + c2 * fraction;
		}
		else
			c3 = c1;
		m_table.push_back(c3);
	}

	// Add one to catch top data
	c3 = m_table[iTableSize-1];
	m_table.push_back(c3);
}

