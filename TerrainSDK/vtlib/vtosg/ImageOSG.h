//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_IMAGEH
#define VTOSG_IMAGEH

#include "vtdata/vtDIB.h"
#include "vtdata/vtCRS.h"

#include <osg/Image>
#include <osgDB/ReadFile>

/**
 This class extends osg::Image with an interface to support vtdata's Bitmap
 operations and use vtdata's vtDIB. It is useful for creating images in memory.

 If you are going to use OSG to load an image from disk, you must use
 osg::Image instead, which can be loaded with osgDB::readImageFile.

 A common use for an image is as a texture map for a textured material, by
 passing it to vtMaterial::SetTexture().
 */
class vtImage: public vtBitmapBase, public osg::Image
{
public:
	vtImage();
	vtImage(class vtDIB *pDIB);
	vtImage(vtImage *copyfrom);

	bool Allocate(const IPoint2 &size, int bitdepth);
	bool WritePNG(const char *fname, bool progress_callback(int) = NULL);
	bool WriteJPEG(const char *fname, int quality = 99, bool progress_callback(int) = NULL);
	bool IsAllocated() const { return valid() && data() != NULL; }
	void Scale(int w, int h);

	/// Return the name of the file, if any, from which the image was loaded.
	std::string GetFilename() const { return getFileName(); }

	// Provide vtBitmapBase methods
	uchar GetPixel8(int x, int y) const;
	void GetPixel24(int x, int y, RGBi &rgb) const;
	void GetPixel32(int x, int y, RGBAi &rgba) const;

	void SetPixel8(int x, int y, uchar color);
	void SetPixel24(int x, int y, const RGBi &rgb);
	void SetPixel32(int x, int y, const RGBAi &rgba);

	IPoint2 GetSize() const;
	uint GetDepth() const;

	uchar *GetData() { return data(); }
	uchar *GetRowData(int row) { return data(0, row); }

protected:
//	bool _Read(const char *fname, bool bAllowCache = true, bool progress_callback(int) = NULL);
	void _BasicInit();
	void _CreateFromDIB(const vtDIB *pDIB, bool b16bit = false);
	bool _ReadPNG(const char *filename);
};
typedef osg::ref_ptr<vtImage> vtImagePtr;
typedef osg::ref_ptr<osg::Image> ImagePtr;


/*
 When you want to operate on an osg::Image with vtdata's Bitmap routines, it
 can be wrapped in this class.
 */
class vtImageWrapper: public vtBitmapBase
{
public:
	vtImageWrapper(osg::Image *image) { m_image = image; }

	// Provide vtBitmapBase methods
	bool Allocate(const IPoint2 &size, int bitdepth) { /* no-op */ return true; }
	bool IsAllocated() const { return true;  }

	uchar GetPixel8(int x, int y) const;
	void GetPixel24(int x, int y, RGBi &rgb) const;
	void GetPixel32(int x, int y, RGBAi &rgba) const;

	void SetPixel8(int x, int y, uchar color);
	void SetPixel24(int x, int y, const RGBi &rgb);
	void SetPixel32(int x, int y, const RGBAi &rgba);

	IPoint2 GetSize() const { return IPoint2(m_image->s(), m_image->t()); }
	uint GetDepth() const { return m_image->getPixelSizeInBits(); }

	uchar *GetData() { return m_image->data(); }
	uchar *GetRowData(int row) { return m_image->data(0, row); }

	osg::Image *m_image;
};

// To ease the transition from vtImage to osg::Image, some helpers:
uchar GetPixel8(const osg::Image *image, int x, int y);
void GetPixel24(const osg::Image *image, int x, int y, RGBi &rgb);
void GetPixel32(const osg::Image *image, int x, int y, RGBAi &rgba);
void SetPixel8(osg::Image *image, int x, int y, uchar color);
void SetPixel24(osg::Image *image, int x, int y, const RGBi &rgb);
void SetPixel32(osg::Image *image, int x, int y, const RGBAi &rgba);
uint GetWidth(const osg::Image *image);
uint GetHeight(const osg::Image *image);
uint GetDepth(const osg::Image *image);
void Set16BitInternal(osg::Image *image, bool bFlag);


class vtImageGeo : public vtImage
{
public:
	vtImageGeo();
	vtImageGeo(const vtImageGeo *copyfrom);

	bool ReadTIF(const char *filename, bool progress_callback(int) = NULL);
	void ReadExtents(const char *filename);

	// In case the image was loaded from a georeferenced format (such as
	//  GeoTIFF), provide access to the georef
	vtCRS &GetCRS() { return m_crs; }
	DRECT &GetExtents() { return m_extents; }

protected:
	// These two fields are rarely used
	vtCRS m_crs;
	DRECT m_extents;
};
typedef osg::ref_ptr<vtImageGeo> vtImageGeoPtr;

bool vtImageInfo(const char *filename, int &width, int &height, int &depth);

#endif	// VTOSG_IMAGEH

