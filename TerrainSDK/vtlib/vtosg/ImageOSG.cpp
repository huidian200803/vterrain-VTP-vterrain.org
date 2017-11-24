//
// ImageOSG.cpp
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/vtString.h"
#include "vtdata/vtLog.h"
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include "gdal_priv.h"
#include "vtdata/vtCRS.h"
#include "vtdata/GDALWrapper.h"

#ifndef LOG_IMAGE_LOAD
#define LOG_IMAGE_LOAD 1
#endif

//
// Set any of these definitions to use OSG's own support for the various
// image file formats instead of our own.
//
#define USE_OSG_FOR_PNG		1
#define USE_OSG_FOR_BMP		1
#define USE_OSG_FOR_JPG		1


////////////////////////////////////////////////////////////////////////
// vtImage class

vtImage::vtImage() : osg::Image()
{
}

vtImage::vtImage(vtDIB *pDIB) : osg::Image()
{
	_CreateFromDIB(pDIB);
}

vtImage::vtImage(vtImage *copyfrom) :
	osg::Image(*(copyfrom), osg::CopyOp::DEEP_COPY_ALL)
{
}

bool vtImage::Allocate(const IPoint2 &size, int bitdepth)
{
	GLenum pixelFormat;
	GLenum dataType = GL_UNSIGNED_BYTE;

	if (bitdepth == 24)
	{
		pixelFormat = GL_RGB;
	}
	else if (bitdepth == 32)
	{
		pixelFormat = GL_RGBA;
	}
	else if (bitdepth == 8)
	{
		pixelFormat = GL_LUMINANCE;
	}
	else
		return false;

	allocateImage(size.x, size.y, 1, pixelFormat, dataType);

	return true;
}

bool vtImage::WritePNG(const char *fname, bool progress_callback(int))
{
#if USE_OSG_FOR_PNG
	// fname is a UTF-8 string, but OSG only understands local charset
	vtString fname_local = UTF8ToLocal(fname);

	return osgDB::writeImageFile(*this, (const char *) fname_local);
#else
	// TODO: native libpng code here
	return false;
#endif
}

/**
 * Write this image to a JPEG file.  Quality setting is the same as libjpeg,
 * in the range of 0..100.  99 is a typically useful quality setting.
 */
bool vtImage::WriteJPEG(const char *fname, int quality, bool progress_callback(int))
{
#if USE_OSG_FOR_JPG
	osgDB::Registry *reg = osgDB::Registry::instance();
	osgDB::ReaderWriter::Options *opts = reg->getOptions();
	if (!opts)
	{
		opts = new osgDB::ReaderWriter::Options;
		opts->ref();	// workaround!  otherwise OSG might crash when
			// closing its DLL, as the options get deleted twice (?) or
			// perhaps it doesn't like deleting the object WE allocated.
	}
	reg->setOptions(opts);

	vtString str;
	str.Format("JPEG_QUALITY %d", quality);
	opts->setOptionString((const char *)str);

	// fname is a UTF-8 string, but OSG only understands local charset
	vtString fname_local = UTF8ToLocal(fname);

	return osgDB::writeImageFile(*this, (const char *) fname_local);
#else
	// TODO: native libjpeg code here
	return false;
#endif
}

void vtImage::_CreateFromDIB(const vtDIB *pDIB, bool b16bit)
{
	const int w   = pDIB->GetWidth();
	const int h   = pDIB->GetHeight();
	const int bpp = pDIB->GetDepth();
	const char *data = (const char *) pDIB->GetData();

	int SizeImage = w * h * (bpp / 8);

	GLubyte *image = new GLubyte[SizeImage];
	memcpy(image, data, SizeImage);

	int pixelFormat = GL_RGB;
	if ( bpp == 24 )
	{
		pixelFormat = GL_RGB;
	}
	else if ( bpp == 32 )
	{
		pixelFormat = GL_RGBA;
	}
	else if ( bpp == 8 )
	{
		pixelFormat = GL_LUMINANCE;
	}

	int internalFormat;
	if (b16bit)
		internalFormat = GL_RGB5;		// use specific
	else
		internalFormat = pixelFormat;	// use default

	setImage(w, h, 1,		// s, t, r
	   internalFormat,		// int internalFormat,
	   pixelFormat,			// uint pixelFormat,
	   GL_UNSIGNED_BYTE,	// uint dataType,
	   image,
	   osg::Image::USE_NEW_DELETE);
}

void vtImage::Scale(int w, int h)
{
	scaleImage(w, h, 1);
}


//////////////////////////

#if USE_OSG_FOR_PNG

bool vtImage::_ReadPNG(const char *filename)
{
	return false;
}

#else

#include "png.h"

bool vtImage::_ReadPNG(const char *filename)
{
	FILE *fp = NULL;

	uchar header[8];
	png_structp png;
	png_infop   info;
	png_infop   endinfo;
	png_bytep  *row_p;

	png_uint_32 width, height;
	int depth, color;

	png_uint_32 i;
	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png)
	{
		// We compiled against the headers of one version of libpng, but
		// linked against the libraries from another version.  If you get
		// this, fix the paths in your development environment.
		return false;
	}
	info = png_create_info_struct(png);
	endinfo = png_create_info_struct(png);

	fp = vtFileOpen(filename, "rb");
	if (fp && fread(header, 1, 8, fp) && png_check_sig(header, 8))
		png_init_io(png, fp);
	else
	{
		png_destroy_read_struct(&png, &info, &endinfo);
		return false;
	}
	png_set_sig_bytes(png, 8);

	png_read_info(png, info);
	png_get_IHDR(png, info, &width, &height, &depth, &color, NULL, NULL, NULL);

	if (color == PNG_COLOR_TYPE_GRAY || color == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);

	// never strip alpha
//	{
//		png_set_strip_alpha(png);
//		color &= ~PNG_COLOR_MASK_ALPHA;
//	}

	// Always expand paletted images
	if (color == PNG_COLOR_TYPE_PALETTE)
		png_set_expand(png);

	/*--GAMMA--*/
//	checkForGammaEnv();
	double screenGamma = 2.2 / 1.0;
#if 0
	// Getting the gamma from the PNG file is disabled here, since
	// PhotoShop writes bizarre gamma values like .227 (PhotoShop 5.0)
	// or .45 (newer versions)
	double	fileGamma;
	if (png_get_gAMA(png, info, &fileGamma))
		png_set_gamma(png, screenGamma, fileGamma);
	else
#endif
		png_set_gamma(png, screenGamma, 1.0/2.2);

	png_read_update_info(png, info);

	m_pPngData = (png_bytep) malloc(png_get_rowbytes(png, info)*height);
	row_p = (png_bytep *) malloc(sizeof(png_bytep)*height);

	bool StandardOrientation = true;
	for (i = 0; i < height; i++) {
		if (StandardOrientation)
			row_p[height - 1 - i] = &m_pPngData[png_get_rowbytes(png, info)*i];
		else
			row_p[i] = &m_pPngData[png_get_rowbytes(png, info)*i];
	}

	png_read_image(png, row_p);
	free(row_p);

	int iBitCount;

	switch (color)
	{
		case PNG_COLOR_TYPE_GRAY:
		case PNG_COLOR_TYPE_RGB:
		case PNG_COLOR_TYPE_PALETTE:
			iBitCount = 24;
			break;

		case PNG_COLOR_TYPE_GRAY_ALPHA:
		case PNG_COLOR_TYPE_RGB_ALPHA:
			iBitCount = 32;
			break;

		default:
			return false;
	}

	png_read_end(png, endinfo);
	png_destroy_read_struct(&png, &info, &endinfo);

	// Don't free the data, we're going to pass it to OSG
//	free(m_pPngData);

	if (fp)
		fclose(fp);

	int pixelFormat;
	uint internalFormat;

	if (iBitCount == 24)
		pixelFormat = GL_RGB;
	else if (iBitCount == 32)
		pixelFormat = GL_RGBA;

	if (m_internalformat == -1)
		internalFormat = pixelFormat;	// use default
	else
		internalFormat = m_internalformat;	// use specific

	setImage(width, height, 1,
	   internalFormat,		// int internalFormat,
	   pixelFormat,			// uint pixelFormat
	   GL_UNSIGNED_BYTE,	// uint dataType
	   m_pPngData,
	   osg::Image::USE_MALLOC_FREE);

	return true;
}

#endif	// USE_OSG_FOR_PNG

uchar vtImage::GetPixel8(int x, int y) const
{
	// OSG appears to reference y=0 as the bottom of the image
	const uchar *buf = data(x, _t-1-y);
	return *buf;
}

void vtImage::SetPixel8(int x, int y, uchar color)
{
	// OSG appears to reference y=0 as the bottom of the image
	uchar *buf = data(x, _t-1-y);
	*buf = color;
}

void vtImage::GetPixel24(int x, int y, RGBi &rgb) const
{
	// OSG appears to reference y=0 as the bottom of the image
	const uchar *buf = data(x, _t-1-y);
	rgb.r = buf[0];
	rgb.g = buf[1];
	rgb.b = buf[2];
}

void vtImage::SetPixel24(int x, int y, const RGBi &rgb)
{
	// OSG appears to reference y=0 as the bottom of the image
	uchar *buf = data(x, _t-1-y);
	buf[0] = rgb.r;
	buf[1] = rgb.g;
	buf[2] = rgb.b;
}

void vtImage::GetPixel32(int x, int y, RGBAi &rgba) const
{
	// OSG appears to reference y=0 as the bottom of the image
	const uchar *buf = data(x, _t-1-y);
	rgba.r = buf[0];
	rgba.g = buf[1];
	rgba.b = buf[2];
	rgba.a = buf[3];
}

void vtImage::SetPixel32(int x, int y, const RGBAi &rgba)
{
	// OSG appears to reference y=0 as the bottom of the image
	uchar *buf = data(x, _t-1-y);
	buf[0] = rgba.r;
	buf[1] = rgba.g;
	buf[2] = rgba.b;
	buf[3] = rgba.a;
}

IPoint2 vtImage::GetSize() const
{
	return IPoint2(s(), t());
}

uint vtImage::GetDepth() const
{
	return getPixelSizeInBits();
}


//////////////////////////////////////////////////////////////////////////
// vtImageWrapper

uchar vtImageWrapper::GetPixel8(int x, int y) const
{
	// OSG appears to reference y=0 as the bottom of the image
	const uchar *buf = m_image->data(x, m_image->t()-1-y);
	return *buf;
}

void vtImageWrapper::SetPixel8(int x, int y, uchar color)
{
	// OSG appears to reference y=0 as the bottom of the image
	uchar *buf = m_image->data(x, m_image->t()-1-y);
	*buf = color;
}

void vtImageWrapper::GetPixel24(int x, int y, RGBi &rgb) const
{
	// OSG appears to reference y=0 as the bottom of the image
	const uchar *buf = m_image->data(x, m_image->t()-1-y);
	rgb.r = buf[0];
	rgb.g = buf[1];
	rgb.b = buf[2];
}

void vtImageWrapper::SetPixel24(int x, int y, const RGBi &rgb)
{
	// OSG appears to reference y=0 as the bottom of the image
	uchar *buf = m_image->data(x, m_image->t()-1-y);
	buf[0] = rgb.r;
	buf[1] = rgb.g;
	buf[2] = rgb.b;
}

void vtImageWrapper::GetPixel32(int x, int y, RGBAi &rgba) const
{
	// OSG appears to reference y=0 as the bottom of the image
	const uchar *buf = m_image->data(x, m_image->t()-1-y);
	rgba.r = buf[0];
	rgba.g = buf[1];
	rgba.b = buf[2];
	rgba.a = buf[3];
}

void vtImageWrapper::SetPixel32(int x, int y, const RGBAi &rgba)
{
	// OSG appears to reference y=0 as the bottom of the image
	uchar *buf = m_image->data(x, m_image->t()-1-y);
	buf[0] = rgba.r;
	buf[1] = rgba.g;
	buf[2] = rgba.b;
	buf[3] = rgba.a;
}


//////////////////////////////////////////////////////////////////////////
// Helpers

uchar GetPixel8(const osg::Image *image, int x, int y)
{
	// OSG appears to reference y=0 as the bottom of the image
	const uchar *buf = image->data(x, image->t()-1-y);
	return *buf;
}

void SetPixel8(osg::Image *image, int x, int y, uchar color)
{
	// OSG appears to reference y=0 as the bottom of the image
	uchar *buf = image->data(x, image->t()-1-y);
	*buf = color;
}

void GetPixel24(const osg::Image *image, int x, int y, RGBi &rgb)
{
	// OSG appears to reference y=0 as the bottom of the image
	const uchar *buf = image->data(x, image->t()-1-y);
	rgb.r = buf[0];
	rgb.g = buf[1];
	rgb.b = buf[2];
}

void SetPixel24(osg::Image *image, int x, int y, const RGBi &rgb)
{
	// OSG appears to reference y=0 as the bottom of the image
	uchar *buf = image->data(x, image->t()-1-y);
	buf[0] = rgb.r;
	buf[1] = rgb.g;
	buf[2] = rgb.b;
}

void GetPixel32(const osg::Image *image, int x, int y, RGBAi &rgba)
{
	// OSG appears to reference y=0 as the bottom of the image
	const uchar *buf = image->data(x, image->t()-1-y);
	rgba.r = buf[0];
	rgba.g = buf[1];
	rgba.b = buf[2];
	rgba.a = buf[3];
}

void SetPixel32(osg::Image *image, int x, int y, const RGBAi &rgba)
{
	// OSG appears to reference y=0 as the bottom of the image
	uchar *buf = image->data(x, image->t()-1-y);
	buf[0] = rgba.r;
	buf[1] = rgba.g;
	buf[2] = rgba.b;
	buf[3] = rgba.a;
}

uint GetWidth(const osg::Image *image)
{
	return image->s();
}

uint GetHeight(const osg::Image *image)
{
	return image->t();
}

uint GetDepth(const osg::Image *image)
{
	return image->getPixelSizeInBits();
}

/**
 * Call this method to tell vtlib that you want it to use a 16-bit texture
 * (internal memory format) to be sent to the graphics card.
 */
void Set16BitInternal(osg::Image *image, bool bFlag)
{
	GLenum pixf = image->getPixelFormat();
	if (bFlag)
	{
		// use a 16-bit internal format
		if (pixf == GL_RGB)
			image->setInternalTextureFormat(GL_RGB5);
		if (pixf == GL_RGBA)
			image->setInternalTextureFormat(GL_RGB5_A1);
	}
	else
		image->setInternalTextureFormat(pixf);
}


///////////////////////////////////////////////////////////////////////////////
// vtImageGeo class

bool vtImageGeo::ReadTIF(const char *filename, bool progress_callback(int))
{
	// Use GDAL to read a TIF file (or any other format that GDAL is
	//  configured to read) into this OSG image.
	bool bRet = true;
	vtString message;

	setFileName(filename);

	g_GDALWrapper.RequestGDALFormats();

	GDALDataset *pDataset = NULL;
	GDALRasterBand *pBand;
	GDALRasterBand *pRed = NULL;
	GDALRasterBand *pGreen = NULL;
	GDALRasterBand *pBlue = NULL;
	GDALRasterBand *pAlpha = NULL;
	GDALColorTable *pTable;
	uchar *pScanline = NULL;
	uchar *pRedline = NULL;
	uchar *pGreenline = NULL;
	uchar *pBlueline = NULL;
	uchar *pAlphaline = NULL;

	CPLErr Err;
	bool bColorPalette = false;
	IPoint2 imageSize;
	int nxBlocks, nyBlocks;
	int xBlockSize, yBlockSize;

	try
	{
		pDataset = (GDALDataset *) GDALOpen(filename, GA_ReadOnly);
		if(pDataset == NULL )
			throw "Couldn't open that file.";

		// Get size
		imageSize.x = pDataset->GetRasterXSize();
		imageSize.y = pDataset->GetRasterYSize();

		// Try getting CRS
		vtCRS temp;
		bool bHaveProj = false;
		const char *pProjectionString = pDataset->GetProjectionRef();
		if (pProjectionString)
		{
			OGRErr err = temp.importFromWkt((char**)&pProjectionString);
			if (err == OGRERR_NONE)
			{
				m_crs = temp;
				bHaveProj = true;
			}
		}
		if (!bHaveProj)
		{
			// check for existence of .prj file
			bool bSuccess = temp.ReadProjFile(filename);
			if (bSuccess)
			{
				m_crs = temp;
				bHaveProj = true;
			}
		}

		// Try getting extents
		double affineTransform[6];
		if (pDataset->GetGeoTransform(affineTransform) == CE_None)
		{
			m_extents.left = affineTransform[0];
			m_extents.right = m_extents.left + affineTransform[1] * imageSize.x;
			m_extents.top = affineTransform[3];
			m_extents.bottom = m_extents.top + affineTransform[5] * imageSize.y;
		}

		// Raster count should be 3 for colour images (assume RGB)
		int iRasterCount = pDataset->GetRasterCount();

		if (iRasterCount != 1 && iRasterCount != 3 && iRasterCount != 4)
		{
			message.Format("Image has %d bands (not 1, 3, or 4).", iRasterCount);
			throw (const char *)message;
		}

		if (iRasterCount == 1)
		{
			pBand = pDataset->GetRasterBand(1);

			// Check the band's data type
			GDALDataType dtype = pBand->GetRasterDataType();
			if (dtype != GDT_Byte)
			{
				message.Format("Band is of type %s, but we support type Byte.", GDALGetDataTypeName(dtype));
				throw (const char *)message;
			}
			GDALColorInterp ci = pBand->GetColorInterpretation();

			if (ci == GCI_PaletteIndex)
			{
				if (NULL == (pTable = pBand->GetColorTable()))
					throw "Couldn't get color table.";
				bColorPalette = true;
			}
			else if (ci == GCI_GrayIndex)
			{
				// we will assume 0-255 is black to white
			}
			else
				throw "Unsupported color interpretation.";

			pBand->GetBlockSize(&xBlockSize, &yBlockSize);
			nxBlocks = (imageSize.x + xBlockSize - 1) / xBlockSize;
			nyBlocks = (imageSize.y + yBlockSize - 1) / yBlockSize;
			if (NULL == (pScanline = new uchar[xBlockSize * yBlockSize]))
				throw "Couldnt allocate scan line.";
		}

		if (iRasterCount == 3)
		{
			for (int i = 1; i <= 3; i++)
			{
				pBand = pDataset->GetRasterBand(i);

				// Check the band's data type
				GDALDataType dtype = pBand->GetRasterDataType();
				if (dtype != GDT_Byte)
				{
					message.Format("Band is of type %s, but we support type Byte.", GDALGetDataTypeName(dtype));
					throw (const char *)message;
				}
				switch (pBand->GetColorInterpretation())
				{
				case GCI_RedBand:
					pRed = pBand;
					break;
				case GCI_GreenBand:
					pGreen = pBand;
					break;
				case GCI_BlueBand:
					pBlue = pBand;
					break;
				}
			}
			if ((NULL == pRed) || (NULL == pGreen) || (NULL == pBlue))
				throw "Couldn't find bands for Red, Green, Blue.";

			pRed->GetBlockSize(&xBlockSize, &yBlockSize);
			nxBlocks = (imageSize.x + xBlockSize - 1) / xBlockSize;
			nyBlocks = (imageSize.y + yBlockSize - 1) / yBlockSize;

			pRedline = new uchar[xBlockSize * yBlockSize];
			pGreenline = new uchar[xBlockSize * yBlockSize];
			pBlueline = new uchar[xBlockSize * yBlockSize];
		}

		if (iRasterCount == 4)
		{
#if VTDEBUG
			VTLOG1("Band interpretations:");
#endif
			for (int i = 1; i <= 4; i++)
			{
				pBand = pDataset->GetRasterBand(i);

				// Check the band's data type
				GDALDataType dtype = pBand->GetRasterDataType();
				if (dtype != GDT_Byte)
				{
					message.Format("Band is of type %s, but we support type Byte.", GDALGetDataTypeName(dtype));
					throw (const char *)message;
				}
				GDALColorInterp ci = pBand->GetColorInterpretation();
#if VTDEBUG
				VTLOG(" %d", ci);
#endif
				switch (ci)
				{
				case GCI_RedBand:
					pRed = pBand;
					break;
				case GCI_GreenBand:
					pGreen = pBand;
					break;
				case GCI_BlueBand:
					pBlue = pBand;
					break;
				case GCI_AlphaBand:
					pAlpha = pBand;
					break;
				case GCI_Undefined:
					// If we have four bands: R,G,B,undefined, then assume that
					//  the undefined one is actually alpha
					if (pRed && pGreen && pBlue && !pAlpha)
						pAlpha = pBand;
					break;
				}
			}
#if VTDEBUG
			VTLOG1("\n");
#endif
			if ((NULL == pRed) || (NULL == pGreen) || (NULL == pBlue) || (NULL == pAlpha))
				throw "Couldn't find bands for Red, Green, Blue, Alpha.";

			pRed->GetBlockSize(&xBlockSize, &yBlockSize);
			nxBlocks = (imageSize.x + xBlockSize - 1) / xBlockSize;
			nyBlocks = (imageSize.y + yBlockSize - 1) / yBlockSize;

			pRedline = new uchar[xBlockSize * yBlockSize];
			pGreenline = new uchar[xBlockSize * yBlockSize];
			pBlueline = new uchar[xBlockSize * yBlockSize];
			pAlphaline = new uchar[xBlockSize * yBlockSize];
		}

		// Allocate the image buffer
		if (iRasterCount == 4)
		{
			Allocate(imageSize, 32);
		}
		else if (iRasterCount == 3 || bColorPalette)
		{
			Allocate(imageSize, 24);
		}
		else if (iRasterCount == 1)
			Allocate(imageSize, 8);

		// Read the data
#if LOG_IMAGE_LOAD
		VTLOG("Reading the image data (%d x %d pixels)\n", imageSize.x, imageSize.y);
#endif

		int nxValid, nyValid;
		RGBi rgb;
		RGBAi rgba;
		if (iRasterCount == 1)
		{
			GDALColorEntry Ent;
			for (int iyBlock = 0; iyBlock < nyBlocks; iyBlock++)
			{
				if (progress_callback != NULL)
					progress_callback(iyBlock * 100 / nyBlocks);

				const int y = iyBlock * yBlockSize;
				for (int ixBlock = 0; ixBlock < nxBlocks; ixBlock++)
				{
					const int x = ixBlock * xBlockSize;
					Err = pBand->ReadBlock(ixBlock, iyBlock, pScanline);
					if (Err != CE_None)
						throw "Problem reading the image data.";

					// Compute the portion of the block that is valid
					// for partial edge blocks.
					if ((ixBlock+1) * xBlockSize > imageSize.x)
						nxValid = imageSize.x - ixBlock * xBlockSize;
					else
						nxValid = xBlockSize;

					if( (iyBlock+1) * yBlockSize > imageSize.y)
						nyValid = imageSize.y - iyBlock * yBlockSize;
					else
						nyValid = yBlockSize;

					for(int iY = 0; iY < nyValid; iY++ )
					{
						for(int iX = 0; iX < nxValid; iX++ )
						{
							if (bColorPalette)
							{
								pTable->GetColorEntryAsRGB(pScanline[iY * xBlockSize + iX], &Ent);
								rgb.r = (uchar) Ent.c1;
								rgb.g = (uchar) Ent.c2;
								rgb.b = (uchar) Ent.c3;
								SetPixel24(x + iX, y + iY, rgb);
							}
							else
								SetPixel8(x + iX, y + iY, pScanline[iY * xBlockSize + iX]);
						}
					}
				}
			}
		}
		if (iRasterCount >= 3)
		{
			for (int iyBlock = 0; iyBlock < nyBlocks; iyBlock++)
			{
				if (progress_callback != NULL)
					progress_callback(iyBlock * 100 / nyBlocks);

				const int y = iyBlock * yBlockSize;
				for (int ixBlock = 0; ixBlock < nxBlocks; ixBlock++)
				{
					const int x = ixBlock * xBlockSize;
					Err = pRed->ReadBlock(ixBlock, iyBlock, pRedline);
					if (Err != CE_None)
						throw "Cannot read data.";
					Err = pGreen->ReadBlock(ixBlock, iyBlock, pGreenline);
					if (Err != CE_None)
						throw "Cannot read data.";
					Err = pBlue->ReadBlock(ixBlock, iyBlock, pBlueline);
					if (Err != CE_None)
						throw "Cannot read data.";
					if (iRasterCount == 4)
					{
						Err = pAlpha->ReadBlock(ixBlock, iyBlock, pAlphaline);
						if (Err != CE_None)
							throw "Cannot read data.";
					}

					// Compute the portion of the block that is valid
					// for partial edge blocks.
					if ((ixBlock+1) * xBlockSize > imageSize.x)
						nxValid = imageSize.x - ixBlock * xBlockSize;
					else
						nxValid = xBlockSize;

					if( (iyBlock+1) * yBlockSize > imageSize.y)
						nyValid = imageSize.y - iyBlock * yBlockSize;
					else
						nyValid = yBlockSize;

					for (int iY = 0; iY < nyValid; iY++)
					{
						for (int iX = 0; iX < nxValid; iX++)
						{
							if (iRasterCount == 3)
							{
								rgb.r = pRedline[iY * xBlockSize + iX];
								rgb.g = pGreenline[iY * xBlockSize + iX];
								rgb.b = pBlueline[iY * xBlockSize + iX];
								SetPixel24(x + iX, y + iY, rgb);
							}
							else if (iRasterCount == 4)
							{
								rgba.r = pRedline[iY * xBlockSize + iX];
								rgba.g = pGreenline[iY * xBlockSize + iX];
								rgba.b = pBlueline[iY * xBlockSize + iX];
								rgba.a = pAlphaline[iY * xBlockSize + iX];
								SetPixel32(x + iX, y + iY, rgba);
							}
						}
					}
				}
			}
		}
	}
	catch (const char *msg)
	{
		VTLOG1("Problem: ");
		VTLOG1(msg);
		VTLOG1("\n");
		bRet = false;
	}

	if (NULL != pDataset)
		GDALClose(pDataset);
	delete pScanline;
	delete pRedline;
	delete pGreenline;
	delete pBlueline;
	delete pAlphaline;

	return bRet;
}


///////////////////////////////////////////////////////////////////////

vtImageGeo::vtImageGeo()
{
	m_extents.SetToZero();
}

vtImageGeo::vtImageGeo(const vtImageGeo *copyfrom) : vtImage(*copyfrom)
{
	m_crs = copyfrom->m_crs;
	m_extents = copyfrom->m_extents;
}

void vtImageGeo::ReadExtents(const char *filename)
{
	// This might be a geospecific image.  If we did not get extents, look
	//  a world file.
	if (m_extents.IsEmpty())
	{
		double params[6];
		if (ReadAssociatedWorldFile(filename, params))
		{
			const IPoint2 size = GetSize();
			m_extents.left = params[4];
			m_extents.right = params[4] + params[0] * size.x;
			m_extents.top = params[5];
			m_extents.bottom = params[5] + params[3] * size.y;
		}
	}
}

bool vtImageInfo(const char *filename, int &width, int &height, int &depth)
{
	g_GDALWrapper.RequestGDALFormats();

	// open the input image and find out the image depth using gdal
	GDALDataset *poDataset;
	poDataset = (GDALDataset *) GDALOpen(filename, GA_ReadOnly);
	if (!poDataset)
		return false;
	width = poDataset->GetRasterXSize();
	height = poDataset->GetRasterYSize();
	depth = poDataset->GetRasterCount()*8;
	GDALClose(poDataset);
	return true;
}
