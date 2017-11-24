//
// vtBitmap.cpp
//
// Copyright (c) 2003-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include "wx/image.h"
#include "wx/mstream.h"

#include "vtdata/vtLog.h"
#include "vtdata/FilePath.h"
#include "vtBitmap.h"

// Headers for PNG support, which uses the library "libpng"
#include "png.h"

// Headers for JPEG support, which uses the library "libjpeg"
extern "C" {
#ifdef WIN32
#define XMD_H	// hack to workaround conflict between windows.h and jpeglib.h
#undef FAR	// hack to prevent a warning due to cluttered windows.h
#endif
#include "jpeglib.h"
}

vtBitmap::vtBitmap()
{
	m_pBitmap = NULL;
#if !USE_DIBSECTIONS
	m_pImage = NULL;
#endif
}

vtBitmap::~vtBitmap()
{
	delete m_pBitmap;
#if !USE_DIBSECTIONS
	delete m_pImage;
#endif
}

bool vtBitmap::Allocate(const IPoint2 &size, int iDepth)
{
	if (iDepth == 8)
		return Allocate8(size);
	else if (iDepth == 24)
		return Allocate24(size);
	else
		return false;
}

bool vtBitmap::IsAllocated() const
{
	return (m_pBitmap != NULL && m_pBitmap->Ok());
}

bool vtBitmap::Allocate8(const IPoint2 &size)
{
	// TODO: difficult, as wxImage can only be 24-bit.
	return false;
}

bool vtBitmap::Allocate24(const IPoint2 &size)
{
#if USE_DIBSECTIONS
	BITMAPINFO ScanlineFormat =
	{
		{
			sizeof(BITMAPINFOHEADER), // Sizeof structure
			0, 0,	// width, height
			1,		// number of bit planes (always one)
			24,		// number of bits per pixel
			BI_RGB,	// compression
			0,		// size of image data (if BI_RGB then only for Dib Sections)
			1, 1,	// pixels per meter X  and Y
			0, 0	// colours used & important (0 for 24 bits per pixel)
		}, 0
	};
	ScanlineFormat.bmiHeader.biWidth = size.x;
	ScanlineFormat.bmiHeader.biHeight = -size.y;

	// Reportedly, biSizeImage need not be specified.
	//  In fact, setting this field to other than 0 can produce crashes.
	ScanlineFormat.bmiHeader.biSizeImage = 0;

	m_iScanlineWidth = (((size.x)*(24) + 31) / 32 * 4);

	// The following sets the m_pScanline pointer, which points to a giant
	// block of directly accesible image data!
	HBITMAP hSection;
	hSection = CreateDIBSection(NULL, &ScanlineFormat, DIB_RGB_COLORS,
		(void**)&m_pScanline, NULL, 0);
	if (hSection == NULL)
		return false;

	m_pBitmap = new wxBitmap;
	m_pBitmap->SetHBITMAP((WXHBITMAP)hSection);
	m_pBitmap->SetWidth(ScanlineFormat.bmiHeader.biWidth);
//	m_pBitmap->SetHeight(ScanlineFormat.bmiHeader.biHeight);
	// A little hack to make the call to StretchBlt work: negative height
	//  matches the behavior of a normal BITMAP?
	m_pBitmap->SetHeight(-ScanlineFormat.bmiHeader.biHeight);
	m_pBitmap->SetDepth(24);
#else
	// yes, we could use some error-checking here
	m_pImage = new wxImage(size.x, size.y);
	if (!m_pImage->Ok())
	{
		delete m_pImage;
		m_pImage = NULL;
		return false;
	}
	m_pBitmap = new wxBitmap(*m_pImage);
	if (!m_pBitmap->Ok())
	{
		delete m_pBitmap;
		delete m_pImage;
		m_pBitmap = NULL;
		m_pImage = NULL;
		return false;
	}
#endif
	return true;
}

void vtBitmap::SetPixel24(int x, int y, uchar r, uchar g, uchar b)
{
#if USE_DIBSECTIONS
	*(m_pScanline + (y * m_iScanlineWidth) + (x * 3)) = b;
	*(m_pScanline + (y * m_iScanlineWidth) + (x * 3) + 1) = g;
	*(m_pScanline + (y * m_iScanlineWidth) + (x * 3) + 2) = r;
#else
	if (x == 0 && y == 0)
		VTLOG(" Setting pixel 0 0 to %d %d %d\n", r, g, b);
	m_pImage->SetRGB(x, y, r, g, b);
#endif
}

void vtBitmap::GetPixel24(int x, int y, RGBi &rgb) const
{
#if USE_DIBSECTIONS
	rgb.b = *(m_pScanline + (y * m_iScanlineWidth) + (x * 3));
	rgb.g = *(m_pScanline + (y * m_iScanlineWidth) + (x * 3) + 1);
	rgb.r = *(m_pScanline + (y * m_iScanlineWidth) + (x * 3) + 2);
#else
	rgb.r = m_pImage->GetRed(x, y);
	rgb.g = m_pImage->GetGreen(x, y);
	rgb.b = m_pImage->GetBlue(x, y);
#endif
}

void vtBitmap::SetPixel32(int x, int y, const RGBAi &rgba)
{
	// unsupported
}

void vtBitmap::GetPixel32(int x, int y, RGBAi &rgba) const
{
	// unsupported
}

uchar vtBitmap::GetPixel8(int x, int y) const
{
	// unimplemented
	return 0;
}

void vtBitmap::SetPixel8(int x, int y, uchar color)
{
	// unimplemented
}

IPoint2 vtBitmap::GetSize() const
{
	return IPoint2(m_pBitmap->GetWidth(), m_pBitmap->GetHeight());
}

uint vtBitmap::GetDepth() const
{
	// not fully implemented
	return 24;
}

//
// If we aren't using DIBSections, then we don't have direct access to the
// image data, so we must copy from the image to the bitmap when we want
// changes to affect the drawn image.
//
void vtBitmap::ContentsChanged()
{
#if !USE_DIBSECTIONS
	VTLOG("Updating Bitmap Contents from Image Data\n");
	if (!m_pImage)
		return;
	delete m_pBitmap;
	m_pBitmap = new wxBitmap(*m_pImage);
#endif
}

// The following small class and function provide the ability for libpng to
//  read a PNG file from memory.
class membuf
{
public:
	membuf(uint8_t *data) { m_data = data; m_offset = 0; }
	uint8_t *m_data;
	int m_offset;
};

void user_read_data(png_structp png_ptr,
					png_bytep data, png_size_t length)
{
	membuf *buf = (membuf *) png_get_io_ptr(png_ptr);
	memcpy(data, buf->m_data+buf->m_offset, length);
	buf->m_offset += (uint)length;
}

bool vtBitmap::ReadPNGFromMemory(uchar *buf, int len, bool progress_callback(int))
{
#if USE_DIBSECTIONS
	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png)
	{
		// Possibly, we compiled against the headers of one version of libpng,
		// but linked against the libraries from another version.
		return false;
	}
	png_infop info = png_create_info_struct(png);
	png_infop endinfo = png_create_info_struct(png);

	if (!png_check_sig(buf, 8))
	{
		png_destroy_read_struct(&png, &info, &endinfo);
		return false;
	}
	png_set_sig_bytes(png, 8);

	// Tell libpng we want to use our own read function (to read from a memory
	//  buffer instead of FILE io)
	membuf buffer(buf+8);
	png_set_read_fn(png, &buffer, user_read_data);

	png_read_info(png, info);

	png_uint_32 width, height;
	int depth, color;
	png_get_IHDR(png, info, &width, &height, &depth, &color, NULL, NULL, NULL);

	if (color == PNG_COLOR_TYPE_GRAY || color == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);

	// Don't strip alpha
	//if (color&PNG_COLOR_MASK_ALPHA)
	//{
	//	png_set_strip_alpha(png);
	//	color &= ~PNG_COLOR_MASK_ALPHA;
	//}

	// Always expand paletted images
//	if (!(PalettedTextures && mipmap >= 0 && trans == PNG_SOLID))
	if (color == PNG_COLOR_TYPE_PALETTE)
		png_set_expand(png);

	png_read_update_info(png, info);

	uchar *m_pPngData = (png_bytep) malloc(png_get_rowbytes(png, info)*height);
	png_bytep *row_p = (png_bytep *) malloc(sizeof(png_bytep)*height);

	const bool StandardOrientation = false;
	for (png_uint_32 i = 0; i < height; i++) {
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

	Allocate(IPoint2(width, height), iBitCount);

	png_size_t png_stride = png_get_rowbytes(png, info);
	uint row, col;
	for (row = 0; row < height; row++)
	{
		if (progress_callback != NULL)
			progress_callback(row * 100 / height);

		byte *adr = m_pScanline + (row * m_iScanlineWidth);
		png_bytep inptr = m_pPngData + row*png_stride;

		// we can't just memcpy, because we must flip RGB -> BGR
		for (col = 0; col < width; col++)
		{
			adr[2] = *inptr++;
			adr[1] = *inptr++;
			adr[0] = *inptr++;
			adr += 3;
		}
	}

	png_read_end(png, endinfo);
	png_destroy_read_struct(&png, &info, &endinfo);
	free(m_pPngData);
	return true;
#else
	m_pImage = new wxImage;

	// This is necessary for wx to know how to load a PNG file
	wxInitAllImageHandlers();

	// Create a memory stream from our existing memory buffer
	wxMemoryInputStream stream((char *)buf, len);
	if (m_pImage->LoadFile(stream, wxBITMAP_TYPE_PNG))
	{
		ContentsChanged();	// copy image to bitmap
		return true;
	}
	else
		return false;
#endif
}

/**
 * Write a JPEG file.
 *
 * \param fname		The output filename.
 * \param quality	JPEG quality in the range of 0..100.
 */
bool vtBitmap::WriteJPEG(const char *fname, int quality)
{
#if USE_DIBSECTIONS
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE * outfile;

	// Initialize the JPEG decompression object with default error handling.
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	outfile = vtFileOpen(fname, "wb");
	if (outfile == NULL)
		return false;

	// Specify data source for decompression
	jpeg_stdio_dest(&cinfo, outfile);

	// set parameters for compression
	cinfo.image_width = m_pBitmap->GetWidth();	// image width and height, in pixels
	cinfo.image_height = m_pBitmap->GetHeight();
	cinfo.input_components = m_pBitmap->GetDepth()/8;	// # of color components per pixel
	if (m_pBitmap->GetDepth()/8 == 1)
		cinfo.in_color_space = JCS_GRAYSCALE;
	else
		cinfo.in_color_space = JCS_RGB;	// colorspace of input image

	// Now use the library's routine to set default compression parameters.
	jpeg_set_defaults(&cinfo);

	jpeg_set_quality(&cinfo, quality, TRUE); // limit to baseline-JPEG values

	// Start compressor
	jpeg_start_compress(&cinfo, TRUE);

	int row_stride = cinfo.image_width * cinfo.input_components;

	int col;
	JSAMPROW row_buffer = new JSAMPLE[row_stride];

	// Process data
	while (cinfo.next_scanline < cinfo.image_height)
	{
		// Transfer data.  Note source values are in BGR order.
		JSAMPROW outptr = row_buffer;
		JSAMPROW adr = ((JSAMPROW)m_pScanline) + cinfo.next_scanline*m_iScanlineWidth;
		if (m_pBitmap->GetDepth() == 8)
		{
			for (col = 0; col < m_pBitmap->GetWidth(); col++)
				*outptr++ = *adr++;
		}
		else
		{
			for (col = 0; col < m_pBitmap->GetWidth(); col++)
			{
				*outptr++ = adr[2];
				*outptr++ = adr[1];
				*outptr++ = adr[0];
				adr += 3;
			}
		}
		jpeg_write_scanlines(&cinfo, &row_buffer, 1);
	}

	delete row_buffer;

	jpeg_finish_compress(&cinfo);

	// After finish_compress, we can close the output file.
	fclose(outfile);

	jpeg_destroy_compress(&cinfo);

#else
	return m_pImage->SaveFile(wxString(fname, wxConvUTF8));
#endif

	return true;
}

