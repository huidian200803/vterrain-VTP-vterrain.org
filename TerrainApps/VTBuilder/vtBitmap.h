//
// vtBitmap.h
//
// Copyright (c) 2003-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTBITMAP_H
#define VTBITMAP_H

#include "vtdata/vtDIB.h"

#if WIN32
#define USE_DIBSECTIONS 1
#endif

/**
 * This class provides an encapsulation of "bitmap" behavior, which can
 * either use the Win32 DIBSection methods, or the wxWidgets Bitmap methods.
 *
 * Set USE_DIBSECTIONS to 1 to get the DIBSection functionality.
 */
class vtBitmap : public vtBitmapBase
{
public:
	vtBitmap();
	virtual ~vtBitmap();

	bool Allocate(const IPoint2 &size, int iDepth = 24);
	bool IsAllocated() const;
	void SetPixel24(int x, int y, uchar r, uchar g, uchar b);
	void SetPixel24(int x, int y, const RGBi &rgb)
	{
		SetPixel24(x, y, rgb.r, rgb.g, rgb.b);
	}
	void GetPixel24(int x, int y, RGBi &rgb) const;

	void SetPixel32(int x, int y, const RGBAi &rgba);
	void GetPixel32(int x, int y, RGBAi &rgba) const;

	uchar GetPixel8(int x, int y) const;
	void SetPixel8(int x, int y, uchar color);

	IPoint2 GetSize() const;
	uint GetDepth() const;

	void ContentsChanged();

	bool ReadPNGFromMemory(uchar *buf, int len, bool progress_callback(int));
	bool WriteJPEG(const char *fname, int quality);

	wxBitmap	*m_pBitmap;

protected:
	bool Allocate8(const IPoint2 &size);
	bool Allocate24(const IPoint2 &size);

#if USE_DIBSECTIONS
	// A DIBSection is a special kind of bitmap, handled as a HBITMAP,
	//  created with special methods, and accessed as a giant raw
	//  memory array.
	uchar *m_pScanline;
	int m_iScanlineWidth;
#else
	// For non-Windows platforms, or Windows platforms if we're being more
	//  cautious, the Bitmap is device-dependent and therefore can't be
	//  relied upon to store data the way we expect.  Hence, we must have
	//  both a wxImage (portable and easy to use, but can't be directly
	//  rendered) and a wxBitmap (which can be drawn to the window).
	//
	// This is less memory efficient and slower.
	//
	wxImage		*m_pImage;
#endif
};

#endif // VTBITMAP_H

