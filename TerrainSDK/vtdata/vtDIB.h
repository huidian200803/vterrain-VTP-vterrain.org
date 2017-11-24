//
// vtDIB.h
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

#include "MathTypes.h"

class vtCRS;

/**
 * An abstract class which defines the basic functionality that any bitmap must expose.
 */
class vtBitmapBase
{
public:
	virtual ~vtBitmapBase() {}

	virtual bool Allocate(const IPoint2 &size, int bitdepth) = 0;
	virtual bool IsAllocated() const = 0;

	virtual uchar GetPixel8(int x, int y) const = 0;
	virtual void GetPixel24(int x, int y, RGBi &rgb) const = 0;
	virtual void GetPixel32(int x, int y, RGBAi &rgba) const = 0;

	virtual void SetPixel8(int x, int y, uchar color) = 0;
	virtual void SetPixel24(int x, int y, const RGBi &rgb) = 0;
	virtual void SetPixel32(int x, int y, const RGBAi &rgba) = 0;

	virtual IPoint2 GetSize() const = 0;
	virtual uint GetDepth() const = 0;

	void ScalePixel8(int x, int y, float fScale);
	void ScalePixel24(int x, int y, float fScale);
	void ScalePixel32(int x, int y, float fScale);
	void BlitTo(vtBitmapBase &target, int x, int y);
};

/**
 * A DIB is a Device-Independent Bitmap.  It is a way of representing a
 * bitmap in memory which has its origins in early MS Windows usage, but
 * is entirely applicable to normal bitmap operations.
 */
class vtDIB : public vtBitmapBase
{
public:
	vtDIB();
	virtual ~vtDIB();

	bool Allocate(const IPoint2 &size, int bitdepth);
	bool IsAllocated() const;

	bool Read(const char *fname, bool progress_callback(int) = NULL);
	bool ReadJPEG(const char *fname, bool progress_callback(int) = NULL);
	bool ReadPNG(const char *fname, bool progress_callback(int) = NULL);
	bool ReadPNGFromMemory(uchar *buf, int len, bool progress_callback(int) = NULL);

	bool WriteJPEG(const char *fname, int quality, bool progress_callback(int) = NULL);
	bool WritePNG(const char *fname);
	bool WriteTIF(const char *fname, const DRECT *area = NULL,
		const vtCRS *crs = NULL, bool progress_callback(int) = NULL);

	uint GetPixel24(int x, int y) const;
	void GetPixel24(int x, int y, RGBi &rgb) const;
	void SetPixel24(int x, int y, uint color);
	void SetPixel24(int x, int y, const RGBi &rgb);

	void GetPixel32(int x, int y, RGBAi &rgba) const;
	void SetPixel32(int x, int y, const RGBAi &rgba);

	uchar GetPixel8(int x, int y) const;
	void SetPixel8(int x, int y, uchar color);

	void SetColor(const RGBi &rgb);
	void Invert();
	void Blit(vtDIB &target, int x, int y);

	IPoint2 GetSize() const { return IPoint2(m_iWidth, m_iHeight); }
	uint GetWidth() const { return m_iWidth; }
	uint GetHeight() const { return m_iHeight; }
	uint GetDepth() const { return m_iByteCount * 8; }

	void *GetData() const { return m_Data; }

private:
	// The DIB's header and data
	void	*m_Data;

	uint	m_iWidth, m_iHeight, m_iByteCount;
	uint	m_iByteWidth;
};

