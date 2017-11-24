//
// MiniDatabuf.h
//
// Copyright (c) 2006-2008 Virtual Terrain Project and Stefan Roettger
// Free for all uses, see license.txt for details.
//

#ifndef MINIDATABUF_H
#define MINIDATABUF_H

#include "vtdata/HeightField.h"	// for INVALID_ELEVATION
#include "vtdata/vtCRS.h"

#if !USE_LIBMINI_DATABUF

/* This is a subset of a libMini class which represents a single 1D, 2D, 3D
	or 4D data buffer, with associated metadata such as dimensions and type. */

class MiniDatabuf
{
public:
	//! default constructor
	MiniDatabuf();
	~MiniDatabuf();

	// mandatory metadata
	unsigned int xsize;  // data size along the x-axis of 1D data
	unsigned int ysize;  // optional data size along the y-axis of 2D data
	unsigned int zsize;  // optional data size along the z-axis of 3D data
	unsigned int tsteps; // optional number of frames for time-dependent data
	unsigned int type;   // 0 = unsigned byte, 1 = signed short, 2 = float, 3 = RGB, 4 = RGBA, 5 = compressed RGB, 6 = compressed RGBA

	// optional metadata
	float swx,swy; // SW corner of data brick
	float nwx,nwy; // NW corner of data brick
	float nex,ney; // NE corner of data brick
	float sex,sey; // SE corner of data brick
	float h0,dh;   // base elevation and height of data brick
	float t0,dt;   // time frame start and exposure time

	// optional scaling
	float scaling; // scale factor of data values, default=1.0f
	float bias;    // bias of data values, default=0.0f

	//! optional minimum and maximum value
	float minvalue;
	float maxvalue;

	//! optional corner points in Lat/Lon (WGS84, degrees)
	float LLWGS84_swx,LLWGS84_swy; // SW corner of data brick
	float LLWGS84_nwx,LLWGS84_nwy; // NW corner of data brick
	float LLWGS84_nex,LLWGS84_ney; // NE corner of data brick
	float LLWGS84_sex,LLWGS84_sey; // SE corner of data brick

	// data chunk
	void *data;         // pointer to raw data, null pointer indicates missing data
	unsigned int bytes; // number of raw data bytes

	void set_extents(float left, float right, float bottom, float top);
	void set_LLWGS84corners(float sw_corner_x,float sw_corner_y,
							float se_corner_x,float se_corner_y,
							float nw_corner_x,float nw_corner_y,
							float ne_corner_x,float ne_corner_y);

	// A useful method to set the extents (in local CRS) and the corners
	//  (in Geo WGS84) at the same time.
	bool SetBounds(const vtCRS &crs, const DRECT &extents);

	// allocate a new memory chunk
	void alloc(unsigned int xs,unsigned int ys,unsigned int zs,unsigned int ts=1,unsigned int ty=0);

	// reset buffer
	void reset();

	// release buffer
	void release();

	// native input/output
	void savedata(const char *filename); // data is saved in MSB format

	// JPEG output
	bool savedataJPEG(const char *filename, int quality);

protected:
	static int MAGIC;

private:
	static unsigned short int INTEL_CHECK;
	void swapbytes();
};

#endif // !USE_LIBMINI_DATABUF

// Helper to write headers
class DRECT;
class vtCRS;
class LODMap	// A simple 2D array of min/max LOD values (log2)
{
public:
	LODMap()
	{
		m_min = m_max = NULL;
	}
	LODMap(int cols, int rows)
	{
		alloc(cols, rows);
	}
	~LODMap()
	{
		delete [] m_min;
		delete [] m_max;
	}
	bool exists() { return m_min != NULL; }
	void alloc(int cols, int rows)
	{
		m_cols = cols;
		m_rows = rows;
		m_min = new int [cols*rows];
		m_max = new int [cols*rows];
		for (int i = 0; i < cols*rows; i++)
			m_min[i] = m_max[i] = 0;
	}
	void set(int c, int r, int minlevel, int maxlevel)
	{
		m_min[c*m_rows+r] = minlevel;
		m_max[c*m_rows+r] = maxlevel;
	}
	void get(int c, int r, int &minlevel, int &maxlevel)
	{
		minlevel = m_min[c*m_rows+r];
		maxlevel = m_max[c*m_rows+r];
	}
	int m_cols, m_rows;
	int *m_min, *m_max;
};

bool WriteTilesetHeader(const char *filename, int cols, int rows, int lod0size,
						const DRECT &area, const vtCRS &crs,
						float minheight=INVALID_ELEVATION, float maxheight=INVALID_ELEVATION,
						LODMap *lodmap = NULL, bool bJPEG = false);

#endif
