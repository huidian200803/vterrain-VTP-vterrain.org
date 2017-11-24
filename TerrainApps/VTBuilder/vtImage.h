//
// vtImage.h
//
// Copyright (c) 2002-2015 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

#include "wx/image.h"
#include "gdal.h"

#include "TilingOptions.h"
#include "vtdata/vtCRS.h"

class vtDIB;
class GDALDataset;
class GDALRasterBand;
class GDALColorTable;
class BuilderView;
class ImageGLCanvas;

// The following mechanism is for a small buffer, consisting of a small
//  number of scanlines, to cache the results of accessing large image
//  files out of memory (direct from disk).
struct Scanline
{
	RGBAi *m_data;
	int m_y;
};
#define BUF_SCANLINES	4

class LineBufferGDAL
{
public:
	LineBufferGDAL();
	~LineBufferGDAL() { Cleanup(); }

	void Setup(GDALDataset *pDataset, int iOverview);
	void Cleanup();

	void ReadScanline(int y, int bufrow);
	RGBAi *GetScanlineFromBuffer(int y);
	void FindMaxBlockSize(GDALDataset *pDataset);

	int		m_iXSize;
	int		m_iYSize;

	int m_iRasterCount;
	uchar *m_pBlock;
	uchar *m_pRedBlock, *m_pGreenBlock, *m_pBlueBlock, *m_pAlphaBlock;
	GDALRasterBand *m_pBand;
	GDALRasterBand *m_pRed, *m_pGreen, *m_pBlue, *m_pAlpha;
	GDALColorTable *m_pTable;
	int m_MaxBlockSize;

	// Total views, including Overviews
	//int m_iViewCount;
	int m_iOverview;	// 0 is the base image, 1 2 3.. are the overviews

	Scanline m_row[BUF_SCANLINES];
	int m_use_next;
	int m_found_last;

	// Statistics
	int m_linereads;
	int m_blockreads;

	vtString m_error_message;
};

class BitmapInfo
{
public:
	BitmapInfo() { m_pBitmap = NULL; m_bOnDisk = false; }

	int number;				// 0, 1, 2..
	vtDIB *m_pBitmap;	// non-NULL if in memory
	bool m_bOnDisk;		// true if GDAL overview exists on disk
	IPoint2 m_Size;			// size in pixels
	DPoint2 m_Spacing;		// spatial resolution in earth units/pixel
	LineBufferGDAL m_linebuf;
};

//////////////////////////////////////////////////////////

class vtImage
{
public:
	vtImage();
	vtImage(const DRECT &area, const IPoint2 &pixelSize,
		const vtCRS &crs, int bitDepth);
	virtual ~vtImage();

	bool GetExtent(DRECT &rect) const;
	void SetExtent(const DRECT &rect);
	vtDIB *GetBitmapToDraw(vtScaledView *pView);
	bool ConvertCRS(vtImage *input, vtCRS &crs_new,
					int iSampleN, bool progress_callback(int) = NULL);

	DPoint2 GetSpacing(int bitmap = 0) const;
	vtDIB *GetBitmap() {
		if (m_Bitmaps.size() != 0)
			return m_Bitmaps[0].m_pBitmap;
		return NULL;
	}

	void GetCRS(vtCRS &crs) const;
	vtCRS &GetAtCRS() { return m_crs; }
	void SetCRS(const vtCRS &crs);
	bool ReprojectExtents(const vtCRS &crs_new);

	void GetDimensions(int &xsize, int &ysize) const
	{
		xsize = m_Bitmaps[0].m_Size.x;
		ysize = m_Bitmaps[0].m_Size.y;
	}
	IPoint2 GetDimensions() const
	{
		return m_Bitmaps[0].m_Size;
	}
	bool GetColorSolid(const DPoint2 &p, RGBAi &rgb, double dRes = 0.0);
	bool GetMultiSample(const DPoint2 &p, const DLine2 &offsets, RGBAi &rgb, double dRes = 0.0);
	void GetRGBA(int x, int y, RGBAi &rgb, double dRes = 0.0);
	void SetRGBA(int x, int y, const RGBAi &rgb);
	void ReplaceColor(const RGBi &rgb1, const RGBi &rgb2);
	void SetupBitmapInfo(const IPoint2 &size);

	// File IO
	bool ReadPPM(const char *fname, bool progress_callback(int) = NULL);
	bool WritePPM(const char *fname) const;
	bool SaveToFile(const char *fname) const;
	bool ReadPNGFromMemory(uchar *buf, int len);
	bool LoadFromGDAL(const char *fname);
	bool CreateOverviews();
	bool LoadOverviews();

	bool WriteTileset(TilingOptions &opts, BuilderView *pView);
	bool WriteTile(const TilingOptions &opts, BuilderView *pView, vtString &dirname,
		DRECT &tile_area, DPoint2 &tile_dim, int col, int row, int lod);

	// global switch
	static bool bTreatBlackAsTransparent;

	// used when reading from a file with GDAL
	GDALDataset *m_pDataset;

	size_t NumBitmaps() const { return m_Bitmaps.size(); }
	BitmapInfo &GetBitmapInfo(size_t i) { return m_Bitmaps[i]; }
	int NumBitmapsInMemory();
	int NumBitmapsOnDisk();
	int GetBitDepth() const { return m_BitDepth; }

	void AllocMipMaps();
	void DrawMipMaps();
	void FreeMipMaps();

protected:
	void SetDefaults();

	vtCRS	m_crs;
	DRECT   m_Extents;
	std::vector<BitmapInfo> m_Bitmaps;

	// The bitmaps each know their own depth, but in case none are in memory, we
	// still need to know their depth in bits per pixel.
	int		m_BitDepth;

	// Used during writing of tilesets
	int m_iTotal, m_iCompleted;
	ImageGLCanvas *m_pCanvas;
};

// Helpers
bool GetBitDepthUsingGDAL(const char *fname, int &depth_in_bits, GDALDataType &eType);
void MakeSampleOffsets(const DPoint2 cellsize, uint N, DLine2 &offsets);
void SampleMipLevel(vtDIB *bigger, vtDIB *smaller);

