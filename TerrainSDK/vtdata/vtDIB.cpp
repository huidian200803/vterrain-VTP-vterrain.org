//
// vtDIB.cpp
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdio.h>
#include <stdlib.h>

// Headers for PNG support, which uses the library "libpng"
#include "png.h"

// compatibility with libpng 1.4
#if ((PNG_LIBPNG_VER_MAJOR==1 && PNG_LIBPNG_VER_MINOR>=4) || PNG_LIBPNG_VER_MAJOR>1)
#define png_infopp_NULL NULL
#endif

// Headers for JPEG support, which uses the library "libjpeg"
extern "C" {
#include "jpeglib.h"
}

#include "vtDIB.h"
#include "vtLog.h"
#include "ByteOrder.h"
#include "FilePath.h"
#include "GDALWrapper.h"

// GDAL for TIFF support
#include "gdal_priv.h"
#include "vtCRS.h"

///////////////////////////////////////////////////////////////////////
// Base class vtBitmapBase
//

void vtBitmapBase::ScalePixel8(int x, int y, float fScale)
{
	uint texel = (int) (GetPixel8(x, y) * fScale);
	if (texel > 255)
		texel = 255;
	SetPixel8(x, y, (uchar) texel);
}

void vtBitmapBase::ScalePixel24(int x, int y, float fScale)
{
	RGBi rgb;
	GetPixel24(x, y, rgb);
	rgb *= fScale;
	if (rgb.r > 255) rgb.r = 255;
	if (rgb.g > 255) rgb.g = 255;
	if (rgb.b > 255) rgb.b = 255;
	SetPixel24(x, y, rgb);
}

void vtBitmapBase::ScalePixel32(int x, int y, float fScale)
{
	RGBAi rgba;
	GetPixel32(x, y, rgba);
	rgba.MultRGB(fScale);
	if (rgba.r > 255) rgba.r = 255;
	if (rgba.g > 255) rgba.g = 255;
	if (rgba.b > 255) rgba.b = 255;
	SetPixel32(x, y, rgba);
}

void vtBitmapBase::BlitTo(vtBitmapBase &target, int x, int y)
{
	const int depth = GetDepth();
	const int tdepth = target.GetDepth();

	const IPoint2 source_size = GetSize();
	const IPoint2 target_size = target.GetSize();

	RGBi rgb;
	RGBAi rgba;
	for (int i = 0; i < source_size.x; i++)
	{
		for (int j = 0; j < source_size.y; j++)
		{
			const int tx = i+x, ty = j+y;
			if (tx < 0 || tx > target_size.x-1 || ty < 0 || ty > target_size.y-1)
				continue;

			if (depth == 8 && tdepth == 8)
			{
				const uchar value = GetPixel8(i, j);
				target.SetPixel8(tx, ty, value);
			}
			else if (tdepth == 24)
			{
				GetPixel24(i, j, rgb);
				target.SetPixel24(tx, ty, rgb);
			}
			else if (tdepth == 32)
			{
				GetPixel32(i, j, rgba);
				target.SetPixel32(tx, ty, rgba);
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////
// Class vtDIB
//

/**
 * Create a new empty DIB wrapper.
 */
vtDIB::vtDIB()
{
	m_Data = NULL;
}

vtDIB::~vtDIB()
{
	if (m_Data != NULL)
		free(m_Data);
	m_Data = NULL;
}


/**
 * Create a new DIB in memory.
 */
bool vtDIB::Allocate(const IPoint2 &size, int bitdepth)
{
	m_iWidth = size.x;
	m_iHeight = size.y;
	m_iByteCount = bitdepth / 8;

	// compute the width in bytes of each scanline (with DIB padding)
	m_iByteWidth = ((m_iWidth * bitdepth + 31) / 32 * 4);

	const int ImageSize = m_iByteWidth * m_iHeight;
	m_Data = malloc(ImageSize);
	if (!m_Data)
	{
		VTLOG("Could not allocate %d bytes\n", ImageSize);
		return false;
	}
	return true;
}

bool vtDIB::IsAllocated() const
{
	return (m_Data != nullptr);
}

/**
 * Read a image file into the DIB.  This method will check to see if the
 * file is a BMP or JPEG and call the appropriate reader.
 */
bool vtDIB::Read(const char *fname, bool progress_callback(int))
{
	FILE *fp = vtFileOpen(fname, "rb");
	if (!fp)
		return false;
	uchar buf[2];
	if (fread(buf, 2, 1, fp) != 1)
		return false;
	fclose(fp);
	if (buf[0] == 0xFF && buf[1] == 0xD8)
		return ReadJPEG(fname, progress_callback);
	else if (buf[0] == 0x89 && buf[1] == 0x50)
		return ReadPNG(fname, progress_callback);
	return false;
}

/**
 * Read a JPEG file. A DIB of the necessary size and depth is allocated.
 */
bool vtDIB::ReadJPEG(const char *fname, bool progress_callback(int))
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE * input_file;
	JDIMENSION num_scanlines;

	/* Initialize the JPEG decompression object with default error handling. */
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	input_file = vtFileOpen(fname, "rb");
	if (input_file == NULL)
		return false;

	/* Specify data source for decompression */
	jpeg_stdio_src(&cinfo, input_file);

	/* Read file header, set default decompression parameters */
	jpeg_read_header(&cinfo, TRUE);

	int bitdepth;
	if (cinfo.num_components == 1)
		bitdepth = 8;
	else
		bitdepth = 24;
	if (!Allocate(IPoint2(cinfo.image_width, cinfo.image_height), bitdepth))
		return false;

	/* Start decompressor */
	jpeg_start_decompress(&cinfo);

	int buffer_height = 1;
	int row_stride = cinfo.output_width * cinfo.output_components;

	/* Make a one-row-high sample array that will go away when done with image */
	JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	uint col;

	/* Process data */
	while (cinfo.output_scanline < cinfo.output_height)
	{
		if (progress_callback != NULL)
			progress_callback(cinfo.output_scanline * 100 / cinfo.output_height);

		JSAMPROW inptr = buffer[0];
		uint8_t *adr = ((uint8_t *)m_Data) + (m_iHeight-1-cinfo.output_scanline)*m_iByteWidth;

		num_scanlines = jpeg_read_scanlines(&cinfo, buffer,
						buffer_height);

		/* Transfer data.  Note destination values must be in BGR order
		 * (even though Microsoft's own documents say the opposite).
		 */
		if (bitdepth == 8)
		{
			for (col = 0; col < cinfo.output_width; col++)
				*adr++ = *inptr++;
		}
		else
		{
			for (col = 0; col < cinfo.output_width; col++)
			{
				adr[2] = *inptr++;	/* can omit GETJSAMPLE() safely */
				adr[1] = *inptr++;
				adr[0] = *inptr++;
				adr += 3;
			}
		}
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	/* Close files, if we opened them */
	if (input_file != stdin)
		fclose(input_file);

	return true;
}

/**
 * Write a JPEG file.
 *
 * \param fname		The output filename.
 * \param quality	JPEG quality in the range of 0..100.
 * \param progress_callback	If supplied, this will be called back with progress
 *		indication in the range of 1 to 100.
 *
 * \return True if successful.
 */
bool vtDIB::WriteJPEG(const char *fname, int quality, bool progress_callback(int))
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE * outfile;

	/* Initialize the JPEG decompression object with default error handling. */
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	outfile = vtFileOpen(fname, "wb");
	if (outfile == NULL)
		return false;

	/* Specify data source for decompression */
	jpeg_stdio_dest(&cinfo, outfile);

	// set parameters for compression
	cinfo.image_width = m_iWidth;	/* image width and height, in pixels */
	cinfo.image_height = m_iHeight;
	cinfo.input_components = m_iByteCount;	/* # of color components per pixel */
	if (m_iByteCount == 1)
		cinfo.in_color_space = JCS_GRAYSCALE;
	else
		cinfo.in_color_space = JCS_RGB;	/* colorspace of input image */

	// Now use the library's routine to set default compression parameters.
	jpeg_set_defaults(&cinfo);

	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

	/* Start compressor */
	jpeg_start_compress(&cinfo, TRUE);

	int row_stride = cinfo.image_width * cinfo.input_components;

	uint col;
	JSAMPROW row_buffer = new JSAMPLE[row_stride];

	/* Process data */
	while (cinfo.next_scanline < cinfo.image_height)
	{
		if (progress_callback != NULL)
			progress_callback(cinfo.next_scanline * 100 / cinfo.image_height);

		// Transfer data.  Note source values are in BGR order.
		JSAMPROW outptr = row_buffer;
		JSAMPROW adr = ((JSAMPROW)m_Data) + (m_iHeight-1-cinfo.next_scanline)*m_iByteWidth;
		if (m_iByteCount == 1)
		{
			for (col = 0; col < m_iWidth; col++)
				*outptr++ = *adr++;
		}
		else
		{
			for (col = 0; col < m_iWidth; col++)
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

	/* After finish_compress, we can close the output file. */
	fclose(outfile);

	jpeg_destroy_compress(&cinfo);

	return true;
}

/**
 * Read a PNG file. A DIB of the necessary size and depth is allocated.
 *
 * \return True if successful.
 */
bool vtDIB::ReadPNG(const char *fname, bool progress_callback(int))
{
	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png)
	{
		// Possibly, we compiled against the headers of one version of libpng,
		// but linked against the libraries from another version.
		return false;
	}
	png_infop info = png_create_info_struct(png);
	png_infop endinfo = png_create_info_struct(png);

	FILE *fp = vtFileOpen(fname, "rb");
	uchar header[8];
	if (fp && fread(header, 1, 8, fp) && png_check_sig(header, 8))
		png_init_io(png, fp);
	else
	{
		png_destroy_read_struct(&png, &info, &endinfo);
		return false;
	}
	png_set_sig_bytes(png, 8);

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

#if 0
		/*--GAMMA--*/
//	checkForGammaEnv();
	double screenGamma = 2.2 / 1.0;

	// Getting the gamma from the PNG file is disabled here, since
	// PhotoShop writes bizarre gamma values like .227 (PhotoShop 5.0)
	// or .45 (newer versions)
	double	fileGamma;
	if (png_get_gAMA(png, info, &fileGamma))
		png_set_gamma(png, screenGamma, fileGamma);
	else
		png_set_gamma(png, screenGamma, 1.0/2.2);
#endif

	png_read_update_info(png, info);

	uchar *pPngData = (png_bytep) malloc(png_get_rowbytes(png, info)*height);
	png_bytep  *row_p = (png_bytep *) malloc(sizeof(png_bytep)*height);

	bool StandardOrientation = true;
	for (png_uint_32 i = 0; i < height; i++) {
		if (StandardOrientation)
			row_p[height - 1 - i] = &pPngData[png_get_rowbytes(png, info)*i];
		else
			row_p[i] = &pPngData[png_get_rowbytes(png, info)*i];
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
	for (uint row = 0; row < height; row++)
	{
		if (progress_callback != NULL)
			progress_callback(row * 100 / height);

		uint8_t *adr = ((uint8_t *)m_Data) + row*m_iByteWidth;
		const png_bytep inptr = pPngData + row*png_stride;
		if (iBitCount == 8)
		{
			memcpy(adr, inptr, width);
		}
		else if (iBitCount == 24)
		{
			memcpy(adr, inptr, width * 3);
		}
		else if (iBitCount == 32)
		{
			memcpy(adr, inptr, width * 4);
		}
	}

	png_read_end(png, endinfo);
	png_destroy_read_struct(&png, &info, &endinfo);
	free(pPngData);
	fclose(fp);
	return true;
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
	membuf *buf = (membuf *)png_get_io_ptr(png_ptr);
	memcpy(data, buf->m_data + buf->m_offset, length);
	buf->m_offset += (uint)length;
}

/**
* Read a PNG file. A DIB of the necessary size and depth is allocated.
*
* \return True if successful.
*/
bool vtDIB::ReadPNGFromMemory(uchar *buf, int len, bool progress_callback(int))
{
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
	membuf buffer(buf + 8);
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

#if 0
	/*--GAMMA--*/
	//	checkForGammaEnv();
	double screenGamma = 2.2 / 1.0;

	// Getting the gamma from the PNG file is disabled here, since
	// PhotoShop writes bizarre gamma values like .227 (PhotoShop 5.0)
	// or .45 (newer versions)
	double	fileGamma;
	if (png_get_gAMA(png, info, &fileGamma))
		png_set_gamma(png, screenGamma, fileGamma);
	else
		png_set_gamma(png, screenGamma, 1.0 / 2.2);
#endif

	png_read_update_info(png, info);

	uchar *pPngData = (png_bytep)malloc(png_get_rowbytes(png, info)*height);
	png_bytep  *row_p = (png_bytep *)malloc(sizeof(png_bytep)*height);

	const bool StandardOrientation = true;
	for (png_uint_32 i = 0; i < height; i++) {
		if (StandardOrientation)
			row_p[height - 1 - i] = &pPngData[png_get_rowbytes(png, info)*i];
		else
			row_p[i] = &pPngData[png_get_rowbytes(png, info)*i];
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
	for (uint row = 0; row < height; row++)
	{
		if (progress_callback != NULL)
			progress_callback(row * 100 / height);

		uint8_t *adr = ((uint8_t *)m_Data) + row*m_iByteWidth;
		const png_bytep inptr = pPngData + row*png_stride;
		if (iBitCount == 8)
		{
			memcpy(adr, inptr, width);
		}
		else if (iBitCount == 24)
		{
			memcpy(adr, inptr, width * 3);
		}
		else if (iBitCount == 32)
		{
			memcpy(adr, inptr, width * 4);
		}
	}

	png_read_end(png, endinfo);
	png_destroy_read_struct(&png, &info, &endinfo);
	free(pPngData);
	return true;
}

/**
 * Write a PNG file.
 *
 * \return True if successful.
 */
bool vtDIB::WritePNG(const char *fname)
{
	FILE *fp;
	png_infop info_ptr;
	png_colorp palette = NULL;

	/* open the file */
	fp = vtFileOpen(fname, "wb");
	if (fp == NULL)
		return false;

	/* Create and initialize the png_struct with the desired error handler
	* functions.  If you want to use the default stderr and longjump method,
	* you can supply NULL for the last three parameters.  We also check that
	* the library version is compatible with the one used at compile time,
	* in case we are using dynamically linked libraries.  REQUIRED.
	*/
	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL);

	if (png == NULL)
	{
		fclose(fp);
		return false;
	}

	/* Allocate/initialize the image information data.  REQUIRED */
	info_ptr = png_create_info_struct(png);
	if (info_ptr == NULL)
	{
		fclose(fp);
		png_destroy_write_struct(&png,  png_infopp_NULL);
		return false;
	}

	/* Set error handling.  REQUIRED if you aren't supplying your own
	* error handling functions in the png_create_write_struct() call.
	*/
	if (setjmp(png_jmpbuf(png)))
	{
		/* If we get here, we had a problem reading the file */
		fclose(fp);
		png_destroy_write_struct(&png, &info_ptr);
		return false;
	}

	/* set up the output control if you are using standard C streams */
	png_init_io(png, fp);

	/* This is the hard way */

	/* Set the image information here.  Width and height are up to 2^31,
	* bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
	* the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
	* PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
	* or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
	* PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
	* currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
	*/
	int color_type=0;	// set to 0 to avoid compiler warning
	int png_bit_depth;
	if (m_iByteCount == 3)
	{
		color_type = PNG_COLOR_TYPE_RGB;
		png_bit_depth = 8;
	}
	if (m_iByteCount == 4)
	{
		color_type = PNG_COLOR_TYPE_RGB_ALPHA;
		png_bit_depth = 8;
	}

	png_set_IHDR(png, info_ptr, m_iWidth, m_iHeight, png_bit_depth, color_type,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	/* You must not free palette here, because png_set_PLTE only makes a link to
	  the palette that you malloced.  Wait until you are about to destroy
	  the png structure. */

	/* Optional gamma chunk is strongly suggested if you have any guess
	* as to the correct gamma of the image.
	*/
//	png_set_gAMA(png, info_ptr, gamma);

	/* other optional chunks like cHRM, bKGD, tRNS, tIME, oFFs, pHYs, */
	/* note that if sRGB is present the gAMA and cHRM chunks must be ignored
	 * on read and must be written in accordance with the sRGB profile */

	/* Write the file header information.  REQUIRED */
	png_write_info(png, info_ptr);

	/* The easiest way to write the image (you may have a different memory
	 * layout, however, so choose what fits your needs best).  You need to
	 * use the first method if you aren't handling interlacing yourself.
	 */
	png_uint_32 k, height = m_iHeight, width = m_iWidth;
	png_byte *image = (png_byte *)malloc(height * width * m_iByteCount);
	png_bytep *row_pointers = (png_bytep *)malloc(height * sizeof(png_bytep *));
	for (k = 0; k < height; k++)
		row_pointers[k] = image + k*width*m_iByteCount;

	uint row, col;
	for (row = 0; row < height; row++)
	{
		uint8_t *adr = ((uint8_t *)m_Data) + row*m_iByteWidth;
		png_bytep pngptr = row_pointers[height-1-row];
		if (m_iByteCount == 1)
		{
			for (col = 0; col < width; col++)
				*pngptr++ = *adr++;
		}
		else if (m_iByteCount == 3)
		{
			for (col = 0; col < width; col++)
			{
				*pngptr++ = adr[2];
				*pngptr++ = adr[1];
				*pngptr++ = adr[0];
				adr += 3;
			}
		}
		else if (m_iByteCount == 4)
		{
			for (col = 0; col < width; col++)
			{
				*pngptr++ = adr[2];
				*pngptr++ = adr[1];
				*pngptr++ = adr[0];
				*pngptr++ = adr[3];
				adr += 4;
			}
		}
	}

	/* write out the entire image data in one call */
	png_write_image(png, row_pointers);

   /* You can write optional chunks like tEXt, zTXt, and tIME at the end
	* as well.  Shouldn't be necessary in 1.1.0 and up as all the public
	* chunks are supported and you can use png_set_unknown_chunks() to
	* register unknown chunks into the info structure to be written out.
	*/

	/* It is REQUIRED to call this to finish writing the rest of the file */
	png_write_end(png, info_ptr);

	/* If you png_malloced a palette, free it here (don't free info_ptr->palette,
	  as recommended in versions 1.0.5m and earlier of this example; if
	  libpng mallocs info_ptr->palette, libpng will free it).  If you
	  allocated it with malloc() instead of png_malloc(), use free() instead
	  of png_free(). */
	if (palette)
	{
		png_free(png, palette);
		palette=NULL;
	}

	free(image);
	free(row_pointers);

	/* clean up after the write, and free any memory allocated */
	png_destroy_write_struct(&png, &info_ptr);

	/* close the file */
	fclose(fp);

	/* that's it */
	return true;
}

/**
 * Write a TIFF file.  If CRS and projection are provided, a GeoTIFF file
 * will be written.
 *
 * \param fname		The output filename.
 * \param area		The extents if the image.  Can be NULL if the image is
 *		not georeferenced.
 * \param crs		The CRS if the image.  Can be NULL if the image is
 *		not georeferenced.
 * \param progress_callback	If supplied, this will be called back with progress
 *		indication in the range of 1 to 100.
 *
 * \return True if successful.
 */
bool vtDIB::WriteTIF(const char *fname, const DRECT *area,
					 const vtCRS *crs, bool progress_callback(int))
{
	g_GDALWrapper.RequestGDALFormats();

	// Save with GDAL to GeoTIFF
	GDALDriverManager *pManager = GetGDALDriverManager();
	if (!pManager)
		return false;

	GDALDriver *pDriver = pManager->GetDriverByName("GTiff");
	if (!pDriver)
		return false;

	// GDAL doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(fname);

	char **papszParmList = NULL;
	GDALDataset *pDataset;
	if (m_iByteCount == 1)
		pDataset = pDriver->Create(fname_local, m_iWidth, m_iHeight,
			1, GDT_Byte, papszParmList );
	else
		pDataset = pDriver->Create(fname_local, m_iWidth, m_iHeight,
			3, GDT_Byte, papszParmList );
	if (!pDataset)
		return false;

	if (area != NULL)
	{
		DPoint2 spacing(area->Width() / m_iWidth, area->Height() / m_iHeight);
		double adfGeoTransform[6] = { area->left, spacing.x, 0,
									area->top, 0, -spacing.y };
		pDataset->SetGeoTransform(adfGeoTransform);
	}
	if (crs != NULL)
	{
		char *pszSRS_WKT = NULL;
		crs->exportToWkt( &pszSRS_WKT );
		pDataset->SetProjection(pszSRS_WKT);
		CPLFree( pszSRS_WKT );
	}

	GByte *raster = new GByte[m_iWidth*m_iHeight];

	GDALRasterBand *pBand;

	if (m_iByteCount == 1)
	{
		pBand = pDataset->GetRasterBand(1);
		for (uint x = 0; x < m_iWidth; x++)
		{
			for (uint y = 0; y < m_iHeight; y++)
				raster[y*m_iWidth + x] = GetPixel8(x, y);
		}
		pBand->RasterIO( GF_Write, 0, 0, m_iWidth, m_iHeight,
			raster, m_iWidth, m_iHeight, GDT_Byte, 0, 0 );
	}
	else
	{
		RGBi rgb;
		for (int i = 1; i <= 3; i++)
		{
			pBand = pDataset->GetRasterBand(i);

			for (uint x = 0; x < m_iWidth; x++)
			{
				if (progress_callback != NULL)
					progress_callback((i-1)*33 + (x * 33 / m_iWidth));

				for (uint y = 0; y < m_iHeight; y++)
				{
					GetPixel24(x, y, rgb);
					if (i == 1) raster[y*m_iWidth + x] = (GByte) rgb.r;
					if (i == 2) raster[y*m_iWidth + x] = (GByte) rgb.g;
					if (i == 3) raster[y*m_iWidth + x] = (GByte) rgb.b;
				}
			}
			pBand->RasterIO( GF_Write, 0, 0, m_iWidth, m_iHeight,
				raster, m_iWidth, m_iHeight, GDT_Byte, 0, 0 );
		}
	}

	delete raster;
	GDALClose(pDataset);

	return true;
}

/**
 * Get a 24-bit RGB value from a 24-bit bitmap.
 *
 * \return R,G,B as the three lowest bytes in an uint.
 */
uint vtDIB::GetPixel24(int x, int y) const
{
	register uint8_t* adr;

	// note: Most processors don't support unaligned int/float reads, and on
	//	   those that do, it's slower than aligned reads.
	adr = ((uint8_t *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + (x*m_iByteCount);
	return (*(uint8_t *)(adr+2)) << 16 |
		   (*(uint8_t *)(adr+1)) <<  8 |
		   (*(uint8_t *)(adr+0));
}

/**
 * Get a 24-bit RGB value from a 24-bit bitmap, place it in the rgb parameter.
 */
void vtDIB::GetPixel24(int x, int y, RGBi &rgb) const
{
	register uint8_t* adr;

	// note: Most processors don't support unaligned int/float reads, and on
	//	   those that do, it's slower than aligned reads.
	adr = ((uint8_t *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + (x*m_iByteCount);
	rgb.r = *((uint8_t *)(adr+0));
	rgb.g = *((uint8_t *)(adr+1));
	rgb.b = *((uint8_t *)(adr+2));
}

void vtDIB::GetPixel32(int x, int y, RGBAi &rgba) const
{
	register uint8_t* adr;

	// note: Most processors don't support unaligned int/float reads, and on
	//	   those that do, it's slower than aligned reads.
	adr = ((uint8_t *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + (x*m_iByteCount);
	rgba.r = *((uint8_t *)(adr+0));
	rgba.g = *((uint8_t *)(adr+1));
	rgba.b = *((uint8_t *)(adr+2));
	rgba.a = *((uint8_t *)(adr+3));
}


/**
 * Set a 24-bit RGB value in a 24-bit bitmap.
 */
void vtDIB::SetPixel24(int x, int y, uint color)
{
	register uint8_t* adr;

	// note: Most processors don't support unaligned int/float writes, and on
	//	   those that do, it's slower than unaligned writes.
	adr = ((uint8_t *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + (x*m_iByteCount);
	*((uint8_t *)(adr + 0)) = (uchar)color;
	*((uint8_t *)(adr + 1)) = (uchar)(color >> 8);
	*((uint8_t *)(adr + 2)) = (uchar)(color >> 16);
}

void vtDIB::SetPixel24(int x, int y, const RGBi &rgb)
{
	register uint8_t* adr;

	// note: Most processors don't support unaligned int/float writes, and on
	//	   those that do, it's slower than unaligned writes.
	adr = ((uint8_t *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + (x*m_iByteCount);
	*((uint8_t *)(adr+0)) = (uchar) rgb.r;
	*((uint8_t *)(adr+1)) = (uchar) rgb.g;
	*((uint8_t *)(adr+2)) = (uchar) rgb.b;
}

void vtDIB::SetPixel32(int x, int y, const RGBAi &rgba)
{
	register uint8_t* adr;

	// note: Most processors don't support unaligned int/float writes, and on
	//	   those that do, it's slower than unaligned writes.
	adr = ((uint8_t *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + (x*m_iByteCount);
	*((uint8_t *)(adr+0)) = (uchar) rgba.r;
	*((uint8_t *)(adr+1)) = (uchar) rgba.g;
	*((uint8_t *)(adr+2)) = (uchar) rgba.b;
	if (m_iByteCount == 4)
		*((uint8_t *)(adr+3)) = (uchar) rgba.a;
}

/**
 * Get a single byte from an 8-bit bitmap.
 */
uchar vtDIB::GetPixel8(int x, int y) const
{
	register uint8_t* adr;

	adr = ((uint8_t *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + x;
	return *adr;
}


/**
 * Set a single byte in an 8-bit bitmap.
 */
void vtDIB::SetPixel8(int x, int y, uchar value)
{
	register uint8_t* adr;

	adr = ((uint8_t *)m_Data) + (m_iHeight-y-1)*m_iByteWidth + x;
	*adr = value;
}

/**
 * Sets the entire bitmap to a single color.
 */
void vtDIB::SetColor(const RGBi &rgb)
{
	uint i, j;
	for (i = 0; i < m_iWidth; i++)
		for (j = 0; j < m_iHeight; j++)
		{
			SetPixel24(i, j, rgb);
		}
}

/**
 * Invert the bitmap colors.
 */
void vtDIB::Invert()
{
	uint i, j;
	if (m_iByteCount == 1)
	{
		for (i = 0; i < m_iWidth; i++)
			for (j = 0; j < m_iHeight; j++)
			{
				SetPixel8(i, j, 8-GetPixel8(i, j));
			}
	}
	else if (m_iByteCount == 3)
	{
		RGBi rgb;
		for (i = 0; i < m_iWidth; i++)
			for (j = 0; j < m_iHeight; j++)
			{
				GetPixel24(i, j, rgb);
				rgb.r = 255 - rgb.r;
				rgb.g = 255 - rgb.g;
				rgb.b = 255 - rgb.b;
				SetPixel24(i, j, rgb);
			}
	}
}

/**
 * Copy from this bitmap to another.  Currently, this is implemented with
 * a slow, completely unoptimized approach of one pixel at a time.
 */
void vtDIB::Blit(vtDIB &target, int x, int y)
{
	uint depth = GetDepth();
	if (depth != target.GetDepth())
		return;
	int tw = target.GetWidth();
	int th = target.GetHeight();

	uint i, j;
	for (i = 0; i < m_iWidth; i++)
	{
		for (j = 0; j < m_iHeight; j++)
		{
			int tx = i+x, ty = j+y;
			if (tx < 0 || tx > tw-1 || ty < 0 || ty > th-1)
				continue;

			if (depth == 8)
			{
				uchar value = GetPixel8(i, j);
				target.SetPixel8(tx, ty, value);
			}
			else if (depth == 24)
			{
				uint value2 = GetPixel24(i, j);
				target.SetPixel24(tx, ty, value2);
			}
		}
	}
}

