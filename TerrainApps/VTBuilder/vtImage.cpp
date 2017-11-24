//
// vtImage.cpp
//
// Copyright (c) 2002-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/vtLog.h"
#include "vtdata/GDALWrapper.h"
#include "vtui/Helper.h"
#include "vtui/ProjectionDlg.h"
#include "minidata/LocalDatabuf.h"

#include "ogr_spatialref.h"
#include "gdal_priv.h"

#include "Builder.h"
#include "BuilderView.h"	// For grid marks
#include "ExtentDlg.h"
#include "ImageGLCanvas.h"
#include "ImageLayer.h"
#include "Options.h"
#include "vtBitmap.h"
#include "vtImage.h"

// static global
bool vtImage::bTreatBlackAsTransparent = false;


///////////////////////////////////////////////////////////////////////

LineBufferGDAL::LineBufferGDAL()
{
	m_pBlock = NULL;
	m_pRedBlock = NULL;
	m_pGreenBlock = NULL;
	m_pBlueBlock = NULL;
	m_pAlphaBlock = NULL;
	m_pBand = NULL;
	m_pRed = NULL;
	m_pGreen = NULL;
	m_pBlue = NULL;
	m_pTable = NULL;

	for (int i = 0; i < BUF_SCANLINES; i++)
		m_row[i].m_data = NULL;
}

void LineBufferGDAL::Setup(GDALDataset *pDataset, int iOverview)
{
	m_iOverview = iOverview;

	GDALRasterBand *pFirstBand = pDataset->GetRasterBand(1);
	if (iOverview != 0)
		pFirstBand = pFirstBand->GetOverview(iOverview - 1);	// GetOverview is 0-based

	m_iXSize = pFirstBand->GetXSize();
	m_iYSize = pFirstBand->GetYSize();

	// Prepare scanline buffers
	for (int i = 0; i < BUF_SCANLINES; i++)
	{
		m_row[i].m_data = new RGBAi[m_iXSize];
		m_row[i].m_y = -1;
	}
	m_use_next = 0;
	m_found_last = -1;
	m_linereads = 0;
	m_blockreads = 0;

	// Raster count should be 3 for colour images (assume RGB)
	m_iRasterCount = pDataset->GetRasterCount();

	if (m_iRasterCount != 1 && m_iRasterCount != 3 && m_iRasterCount != 4)
	{
		m_error_message.Format("Image has %d bands (not 1, 3, or 4).", m_iRasterCount);
		throw (const char *) m_error_message;
	}

	FindMaxBlockSize(pDataset);

	if (m_iRasterCount == 1)
	{
		m_pBand = pDataset->GetRasterBand(1);

		// Check data type - it's either integer or float
		if (GDT_Byte != m_pBand->GetRasterDataType())
			throw "Raster is not of type byte.";
		GDALColorInterp ci = m_pBand->GetColorInterpretation();
		if (ci == GCI_PaletteIndex)
		{
			if (NULL == (m_pTable = m_pBand->GetColorTable()))
				throw "Couldn't get color table.";
		}
		else if (ci == GCI_GrayIndex)
		{
			// we will assume 0-255 is black to white
		}
		else
			throw "Unsupported color interpretation.";

		if (NULL == (m_pBlock = new uchar[m_MaxBlockSize]))
			throw "Couldnt allocate scan line.";
	}

	if (m_iRasterCount == 3 || m_iRasterCount == 4)
	{
		bool bAlpha = (m_iRasterCount == 4);

		int num_undefined = 0;
		for (int i = 1; i <= m_iRasterCount; i++)
		{
			GDALRasterBand *pBand = pDataset->GetRasterBand(i);

			// Jump down to the requested overview.
			if (iOverview != 0)
				pBand = pBand->GetOverview(iOverview - 1);	// GetOverview is 0-based

			// Check data type - it's either integer or float
			if (GDT_Byte != pBand->GetRasterDataType())
				throw "Three rasters, but not of type byte.";
			switch(pBand->GetColorInterpretation())
			{
			case GCI_RedBand:
				m_pRed = pBand;
				break;
			case GCI_GreenBand:
				m_pGreen = pBand;
				break;
			case GCI_BlueBand:
				m_pBlue = pBand;
				break;
			case GCI_AlphaBand:
				m_pAlpha = pBand;
				break;
			case GCI_Undefined:
				num_undefined++;
				break;
			}
		}
		if (num_undefined == m_iRasterCount)
		{
			// All three are undefined, assume they are R,G,B (,A)
			m_pRed = pDataset->GetRasterBand(1);
			m_pGreen = pDataset->GetRasterBand(2);
			m_pBlue = pDataset->GetRasterBand(3);
			if (bAlpha)
				m_pAlpha = pDataset->GetRasterBand(4);
		}
		if ((NULL == m_pRed) || (NULL == m_pGreen) || (NULL == m_pBlue))
			throw "Couldn't find bands for Red, Green, Blue.";
		if (bAlpha && (NULL == m_pAlpha))
			throw "Couldn't find band for Alpha.";

		//m_pRed->GetBlockSize(&m_xBlockSize, &m_yBlockSize);
		//m_nxBlocks = (m_iXSize + m_xBlockSize - 1) / m_xBlockSize;
		//m_nyBlocks = (m_iYSize + m_yBlockSize - 1) / m_yBlockSize;

		m_pRedBlock = new uchar[m_MaxBlockSize];
		m_pGreenBlock = new uchar[m_MaxBlockSize];
		m_pBlueBlock = new uchar[m_MaxBlockSize];
		if (bAlpha)
			m_pAlphaBlock = new uchar[m_MaxBlockSize];
	}
}

void LineBufferGDAL::FindMaxBlockSize(GDALDataset *pDataset)
{
	m_MaxBlockSize = -1;

	int rasters = pDataset->GetRasterCount();
	for (int i = 1; i <= rasters; i++)
	{
		GDALRasterBand *band = pDataset->GetRasterBand(i);
		if (m_iOverview != 0)
			band = band->GetOverview(m_iOverview - 1);	// GetOverview is 0-based

		int xblocksize, yblocksize;
		band->GetBlockSize(&xblocksize, &yblocksize);
		if (xblocksize * yblocksize > m_MaxBlockSize)
			m_MaxBlockSize = xblocksize * yblocksize;
	}
}

void LineBufferGDAL::Cleanup()
{
	delete m_pBlock;
	delete m_pRedBlock;
	delete m_pGreenBlock;
	delete m_pBlueBlock;
	delete m_pAlphaBlock;

	m_pBlock = NULL;
	m_pRedBlock = NULL;
	m_pGreenBlock = NULL;
	m_pBlueBlock = NULL;
	m_pAlphaBlock = NULL;
	m_pBand = NULL;
	m_pRed = NULL;
	m_pGreen = NULL;
	m_pBlue = NULL;

	for (int i = 0; i < BUF_SCANLINES; i++)
	{
		delete m_row[i].m_data;
		m_row[i].m_data = NULL;
	}
}

void LineBufferGDAL::ReadScanline(int iYRequest, int bufrow)
{
	m_linereads++;	// statistics

	CPLErr Err;
	GDALColorEntry Ent;
	int nxValid;

	if (m_iRasterCount == 1)
	{
		int xBlockSize, yBlockSize;
		m_pBand->GetBlockSize(&xBlockSize, &yBlockSize);
		int nxBlocks = (m_pBand->GetXSize() + xBlockSize - 1) / xBlockSize;
		int nyBlocks = (m_pBand->GetYSize() + yBlockSize - 1) / yBlockSize;

		int iyBlock = iYRequest / yBlockSize;
		int iY = iYRequest - (iyBlock * yBlockSize);

		// Convert to rgb and write to image
		for(int ixBlock = 0; ixBlock < nxBlocks; ixBlock++)
		{
			m_blockreads++;	// statistics

			Err = m_pBand->ReadBlock(ixBlock, iyBlock, m_pBlock);
			if (Err != CE_None)
				throw "Readblock failed.";

			// Compute the portion of the block that is valid
			// for partial edge blocks.
			if ((ixBlock+1) * xBlockSize > m_iXSize)
				nxValid = m_iXSize - ixBlock * xBlockSize;
			else
				nxValid = xBlockSize;

			for( int iX = 0; iX < nxValid; iX++ )
			{
				int val = m_pBlock[iY * xBlockSize + iX];
				if (m_pTable)
				{
					m_pTable->GetColorEntryAsRGB(val, &Ent);
					m_row[bufrow].m_data[iX].Set(Ent.c1, Ent.c2, Ent.c3);
				}
				else
				{
					// greyscale
					m_row[bufrow].m_data[iX].Set(val, val, val);
				}
			}
		}
	}
	else if (m_iRasterCount == 3 || m_iRasterCount == 4)
	{
		bool bAlpha = (m_iRasterCount == 4);

		int xBlockSize, yBlockSize;
		m_pRed->GetBlockSize(&xBlockSize, &yBlockSize);
		int nxBlocks = (m_pRed->GetXSize() + xBlockSize - 1) / xBlockSize;
		int nyBlocks = (m_pRed->GetYSize() + yBlockSize - 1) / yBlockSize;

		int iyBlock = iYRequest / yBlockSize;
		int iY = iYRequest - (iyBlock * yBlockSize);

		RGBAi rgba;
		for(int ixBlock = 0; ixBlock < nxBlocks; ixBlock++)
		{
			try {
				Err = m_pRed->ReadBlock(ixBlock, iyBlock, m_pRedBlock);
				if (Err != CE_None)
					throw "Readblock (red) failed";
				Err = m_pGreen->ReadBlock(ixBlock, iyBlock, m_pGreenBlock);
				if (Err != CE_None)
					throw "Readblock (green) failed";
				Err = m_pBlue->ReadBlock(ixBlock, iyBlock, m_pBlueBlock);
				if (Err != CE_None)
					throw "Readblock (blue) failed";
				if (bAlpha)
				{
					Err = m_pAlpha->ReadBlock(ixBlock, iyBlock, m_pAlphaBlock);
					if (Err != CE_None)
						throw "Readblock (alpha) failed";
				}
			}
			catch (const char *msg)
			{
				VTLOG1(msg);
				VTLOG(" in ReadScanline(y=%d) (XSize %d, YSize %d, yBlock %d, yBlockSize %d, yBlocks %d)\n",
					iYRequest, m_iXSize, m_iYSize, iyBlock, yBlockSize, nyBlocks);

				// Posible TODO: Fill the buffer with some fixed value, like
				//  black, before returning
				return;
			}

			m_blockreads += 3;	// statistics

			// Compute the portion of the block that is valid
			// for partial edge blocks.
			if ((ixBlock+1) * xBlockSize > m_iXSize)
				nxValid = m_iXSize - ixBlock * xBlockSize;
			else
				nxValid = xBlockSize;

			for( int iX = 0; iX < nxValid; iX++ )
			{
				int alpha;
				if (bAlpha)
					alpha = m_pAlphaBlock[iY * xBlockSize + iX];
				else
					alpha = 255;
				rgba.Set(m_pRedBlock[iY * xBlockSize + iX],
						 m_pGreenBlock[iY * xBlockSize + iX],
						 m_pBlueBlock[iY * xBlockSize + iX],
						 alpha);
				m_row[bufrow].m_data[ixBlock*xBlockSize + iX] = rgba;
			}
		}
	}
}

RGBAi *LineBufferGDAL::GetScanlineFromBuffer(int y)
{
	// first check if the row is already in memory
	int i;
	for (i = 0; i < BUF_SCANLINES; i++)
	{
		if (m_row[i].m_y == y)
		{
			// yes it is
			return m_row[i].m_data;
		}
	}

	// ok, it isn't. load it into the next appropriate slot.
	ReadScanline(y, m_use_next);
	m_row[m_use_next].m_y = y;
	m_row[m_use_next].m_data;	// WTF?
	RGBAi *data = m_row[m_use_next].m_data;

	// increment which buffer row we'll use next
	m_use_next++;
	if (m_use_next == BUF_SCANLINES)
		m_use_next = 0;

	return data;
}


///////////////////////////////////////////////////////////////////////

vtImage::vtImage()
{
	SetDefaults();
}

vtImage::vtImage(const DRECT &area, const IPoint2 &pixelSize,
				 const vtCRS &crs, int bitDepth)
{
	SetDefaults();
	m_Extents = area;
	m_crs = crs;

	// yes, we could use some error-checking here
	vtDIB *pBitmap = new vtDIB;
	pBitmap->Allocate(pixelSize, bitDepth);
	SetupBitmapInfo(pixelSize);
	m_Bitmaps[0].m_pBitmap = pBitmap;
	m_BitDepth = bitDepth;
}

vtImage::~vtImage()
{
	for (uint i = 0; i < m_Bitmaps.size(); i++)
		delete m_Bitmaps[i].m_pBitmap;

	if (NULL != m_pDataset)
		GDALClose(m_pDataset);
}

void vtImage::SetDefaults()
{
	m_pDataset = NULL;
	m_pCanvas = NULL;
	m_BitDepth = 24;	// Unless we know otherwise.
	m_iTotal = 0;
	m_iCompleted = 0;
}

bool vtImage::GetExtent(DRECT &rect) const
{
	rect = m_Extents;
	return true;
}

vtDIB *vtImage::GetBitmapToDraw(vtScaledView *pView)
{
	// If there are mipmaps in memory, choose the appropriate one to draw
	vtDIB *pBitmapToDraw = NULL;

	// Determine which overview resolution (with an in-memory bitmap) is
	//  most appropriate to draw
	double dRes = 1.0 / pView->GetScale();
	const DPoint2 spacing = GetSpacing();
	double spacing_diff = 1E9;
	for (uint i = 0; i < m_Bitmaps.size(); i++)
	{
		const double d2 = fabs(dRes - m_Bitmaps[i].m_Spacing.x);
		if (d2 < spacing_diff && m_Bitmaps[i].m_pBitmap)
		{
			spacing_diff = d2;
			pBitmapToDraw = m_Bitmaps[i].m_pBitmap;
		}
	}
	return pBitmapToDraw;

	// TODO
	//pDC2->StretchBlit(*pBitmap->m_pBitmap, destRect.x, destRect.y,
	//	destRect.width, destRect.height, srcRect.x, srcRect.y,
	//	srcRect.width, srcRect.height);
}

bool vtImage::ConvertCRS(vtImage *pOld, vtCRS &NewCRS,
						 int iSampleN, bool progress_callback(int))
{
	// Create conversion object
	const vtCRS *pSource, *pDest;
	pSource = &pOld->GetAtCRS();
	pDest = &NewCRS;

	ScopedOCTransform trans(CreateCoordTransform(pSource, pDest));
	if (!trans)
	{
		// Inconvertible coordinate systems.
		return false;
	}

	// find where the extent corners are going to be in the new terrain
	DRECT OldExtents;
	pOld->GetExtent(OldExtents);
	DLine2 OldCorners;
	OldCorners.Append(DPoint2(OldExtents.left, OldExtents. bottom));
	OldCorners.Append(DPoint2(OldExtents.right, OldExtents. bottom));
	OldCorners.Append(DPoint2(OldExtents.right, OldExtents. top));
	OldCorners.Append(DPoint2(OldExtents.left, OldExtents. top));

	DLine2 Corners = OldCorners;
	m_Extents.SetInsideOut();
	int success;
	for (int i = 0; i < 4; i++)
	{
		success = trans->Transform(1, &Corners[i].x, &Corners[i].y);
		if (success == 0)
		{
			// Inconvertible coordinate systems.
			delete trans;
			return false;
		}
		m_Extents.GrowToContainPoint(Corners[i]);
	}

	// now, how large an array will we need for the new terrain?
	// try to preserve the sampling rate approximately
	//
	bool bOldGeo = (pSource->IsGeographic() != 0);
	bool bNewGeo = (pDest->IsGeographic() != 0);

	const DPoint2 old_step = pOld->GetSpacing();
	DPoint2 new_step;
	double meters_per_longitude;

	if (bOldGeo && !bNewGeo)
	{
		// convert degrees to meters (approximately)
		meters_per_longitude = MetersPerLongitude(OldCorners[0].y);
		new_step.x = old_step.x * meters_per_longitude;
		new_step.y = old_step.y * METERS_PER_LATITUDE;
	}
	else if (!bOldGeo && bNewGeo)
	{
		// convert meters to degrees (approximately)
		meters_per_longitude = MetersPerLongitude(Corners[0].y);
		new_step.x = old_step.x / meters_per_longitude;
		new_step.y = old_step.y / METERS_PER_LATITUDE;	// convert degrees to meters (approximately)
	}
	else
	{
		// check horizontal units or old and new terrain
		const double units_old = pSource->GetLinearUnits(NULL);
		const double units_new = pDest->GetLinearUnits(NULL);
		new_step = old_step * (units_old / units_new);
	}
	const double fColumns = m_Extents.Width() / new_step.x;
	const double fRows = m_Extents.Height() / new_step.y;

	// round up to the nearest integer
	const IPoint2 pixelSize(fColumns + 0.999, fRows + 0.999);

	// do safety checks
	if (pixelSize.x < 1 || pixelSize.y < 1)
		return false;
	if (pixelSize.x > 40000 || pixelSize.y > 40000)
		return false;

	// Now we're ready to fill in the new image.
	m_crs = NewCRS;
	vtDIB *pBitmap = new vtDIB;
	if (!pBitmap->Allocate(pixelSize, pOld->GetBitDepth()))
		return false;
	SetupBitmapInfo(pixelSize);
	m_Bitmaps[0].m_pBitmap = pBitmap;

	// Convert each bit of data from the old array to the new
	// Transformation points backwards, from the target to the source
	ScopedOCTransform trans_back(CreateCoordTransform(pDest, pSource));
	if (!trans_back)
	{
		// Inconvertible coordinate systems.
		// "Couldn't convert between coordinate systems.";
		return false;
	}

	// Prepare to multisample
	const DPoint2 step = GetSpacing();
	DLine2 offsets;
	MakeSampleOffsets(step, iSampleN, offsets);

	RGBAi value, sum;
	int count;
	DPoint2 mp;
	for (int i = 0; i < pixelSize.x; i++)
	{
		if (progress_callback != NULL)
			progress_callback(i * 99 / pixelSize.x);

		for (int j = 0; j < pixelSize.y; j++)
		{
			// Sample at pixel centers
			const DPoint2 p(m_Extents.left + (step.x/2) + (i * step.x),
							m_Extents.bottom + (step.y/2)+ (j * step.y));
			count = 0;
			sum.Set(0,0,0);
			for (uint k = 0; k < offsets.GetSize(); k++)
			{
				mp = p + offsets[k];

				// Since transforming the extents succeeded, it's safe to assume
				// that the points will also transform without errors.
				trans_back->Transform(1, &mp.x, &mp.y);

				if (pOld->GetColorSolid(mp, value))
				{
					count++;
					sum += value;
				}
			}
			if (count > 0)
				SetRGBA(i, pixelSize.y-1-j, sum / count);
			else
				SetRGBA(i, pixelSize.y-1-j, RGBAi(0,0,0,0));	// nodata
		}
	}
	return true;
}

void vtImage::GetCRS(vtCRS &crs) const
{
	crs = m_crs;
}

void vtImage::SetCRS(const vtCRS &crs)
{
	m_crs = crs;
}

void vtImage::SetExtent(const DRECT &rect)
{
	m_Extents = rect;
}

DPoint2 vtImage::GetSpacing(int bitmap) const
{
	// safety check
	if (bitmap >= (int) m_Bitmaps.size())
		return DPoint2(1, 1);

	return DPoint2(m_Extents.Width() / m_Bitmaps[bitmap].m_Size.x,
		m_Extents.Height() / m_Bitmaps[bitmap].m_Size.y);
}

/**
 * Reprojects an image by converting just the extents to a new CRS.
 *
 * This is much faster than creating a new grid and reprojecting every heixel,
 * but it only produces correct results when the difference between the CRSes
 * is only a horizontal shift.  For example, this occurs when the only
 * difference between the old and new CRS is choice of Datum.
 *
 * \param crs_new	The new CRS to convert to.
 *
 * \return True if successful.
 */
bool vtImage::ReprojectExtents(const vtCRS &crs_new)
{
	// Create conversion object
	ScopedOCTransform trans(CreateCoordTransform(&m_crs, &crs_new));
	if (!trans)
		return false;	// Inconvertible coordinate systems.

	int success = 0;
	success += trans->Transform(1, &m_Extents.left, &m_Extents.bottom);
	success += trans->Transform(1, &m_Extents.right, &m_Extents.top);

	if (success != 2)
		return false;	// Inconvertible coordinate systems.

	m_crs = crs_new;
	return true;
}

/**
 * Sample image color at a given point, assuming that the pixels are solid.
 *
 * \return true if a value was found, false if the point is outside the
 *		extent or (if the option is enabled) the value was 'nodata'.
 */
bool vtImage::GetColorSolid(const DPoint2 &p, RGBAi &rgba, double dRes)
{
	// Safety check
	if (m_Bitmaps.size() == 0)
		return false;

	// could speed this up by keeping these values around
	const DPoint2 &spacing = m_Bitmaps[0].m_Spacing;
	const IPoint2 &size = m_Bitmaps[0].m_Size;

	double u = (p.x - m_Extents.left) / spacing.x;
	if (u < -0.5 || u > size.x+0.5) return false; // check extents
	if (u < 0.5) u = 0.5; // adjust left edge
	if (u > size.x-0.5) u = size.x-0.5; // adjust right edge
	int ix = (int) u; // round to closest pixel

	double v = (m_Extents.top - p.y) / spacing.y;
	if (v < -0.5 || v > size.y+0.5) return false; // check extents
	if (v < 0.5) v = 0.5; // adjust top edge
	if (v > size.y-0.5) v = size.y-0.5; // adjust bottom edge
	int iy = (int) v; // round to closest pixel

	GetRGBA(ix, iy, rgba, dRes);
	if (bTreatBlackAsTransparent && rgba == RGBAi(0,0,0,255))
		return false;

	return true;
}

void MakeSampleOffsets(const DPoint2 cellsize, uint N, DLine2 &offsets)
{
	DPoint2 spacing = cellsize / N;
	DPoint2 base = spacing * -((float) (N-1) / 2);
	for (uint i = 0; i < N; i++)
		for (uint j = 0; j < N; j++)
			offsets.Append(DPoint2(base.x + (i*spacing.x), base.y + (j*spacing.y)));
}

/**
 * Get image color by sampling several points and averaging them.
 * The area to test is given by center and offsets, use MakeSampleOffsets()
 * to make a set of N x N offsets.
 */
bool vtImage::GetMultiSample(const DPoint2 &p, const DLine2 &offsets, RGBAi &rgba, double dRes)
{
	RGBAi color;
	rgba.Set(0,0,0,0);
	int count = 0;
	for (uint i = 0; i < offsets.GetSize(); i++)
	{
		if (GetColorSolid(p+offsets[i], color, dRes))
		{
			rgba += color;
			count++;
		}
	}
	if (count)
	{
		rgba /= count;
		return true;
	}
	return false;
}

int vtImage::NumBitmapsInMemory()
{
	int count = 0;
	for (uint i = 0; i < m_Bitmaps.size(); i++)
		if (m_Bitmaps[i].m_pBitmap != NULL)
			count++;
	return count;
}

int vtImage::NumBitmapsOnDisk()
{
	int count = 0;
	for (uint i = 0; i < m_Bitmaps.size(); i++)
		if (m_Bitmaps[i].m_bOnDisk)
			count++;
	return count;
}

void vtImage::GetRGBA(int x, int y, RGBAi &rgba, double dRes)
{
	int closest_bitmap = -1;
	double diff = 1E9;

	if (dRes == 0.0)
	{
		// no resolution to match, take first available in memory
		for (int i = 0; i < (int)m_Bitmaps.size(); i++)
		{
			if (m_Bitmaps[i].m_pBitmap)
			{
				closest_bitmap = i;
				break;
			}
		}
	}
	else
	{
		// What overview resolution is most appropriate
		for (int i = 0; i < (int)m_Bitmaps.size(); i++)
		{
			// if it is available
			if (m_Bitmaps[i].m_pBitmap || m_Bitmaps[i].m_bOnDisk)
			{
				double spc = (m_Bitmaps[i].m_Spacing.x + m_Bitmaps[i].m_Spacing.y)/2.0;
				double rel_spc = fabs(dRes - spc);

				if (rel_spc < diff)
				{
					diff = rel_spc;
					closest_bitmap = i;
				}
			}
		}
	}

	if (closest_bitmap < 0)
	{
		// safety measure for missing overviews
		rgba = RGBAi(255,0,0,0);
		return;
	}

	if (closest_bitmap != 0)
	{
		// get smaller coordinates from subsampled view
		x >>= closest_bitmap;
		y >>= closest_bitmap;
	}

	BitmapInfo &bm = m_Bitmaps[closest_bitmap];
	if (bm.m_pBitmap)
	{
		// get pixel from bitmap in memory
		if (bm.m_pBitmap->GetDepth() == 8)
		{
			uint8_t value = bm.m_pBitmap->GetPixel8(x, y);
			rgba.Set(value, 0, 0, 255);		// Use Red for a single value.
		}
		else if (bm.m_pBitmap->GetDepth() == 24)
		{
			RGBi rgb;
			bm.m_pBitmap->GetPixel24(x, y, rgb);
			rgba = rgb;
		}
		else if (bm.m_pBitmap->GetDepth() == 32)
		{
			bm.m_pBitmap->GetPixel32(x, y, rgba);
		}
	}
	else if (bm.m_bOnDisk)
	{
		// support for out-of-memory image
		RGBAi *data = bm.m_linebuf.GetScanlineFromBuffer(y);
		rgba = data[x];
	}
}

void vtImage::SetRGBA(int x, int y, const RGBAi &rgba)
{
	// this method clearly only works for in-memory images
	if (!m_Bitmaps[0].m_pBitmap)
		return;

	switch (m_Bitmaps[0].m_pBitmap->GetDepth())
	{
	case 8:
		m_Bitmaps[0].m_pBitmap->SetPixel24(x, y, rgba.r);
		break;
	case 24:
	{
		RGBi rgb;
		rgb = rgba;
		m_Bitmaps[0].m_pBitmap->SetPixel24(x, y, rgb);
		break;
	}
	case 32:
		m_Bitmaps[0].m_pBitmap->SetPixel32(x, y, rgba);
		break;
	}
}

void vtImage::ReplaceColor(const RGBi &rgb1, const RGBi &rgb2)
{
	// this method only works for in-memory images
	if (!m_Bitmaps[0].m_pBitmap)
		return;

	BitmapInfo &bmi = m_Bitmaps[0];
	if (!bmi.m_pBitmap)
		return;
	RGBi color;
	for (int i = 0; i < bmi.m_Size.x; i++)
		for (int j = 0; j < bmi.m_Size.y; j++)
		{
			bmi.m_pBitmap->GetPixel24(i, j, color);
			if (color == rgb1)
				bmi.m_pBitmap->SetPixel24(i, j, rgb2);
		}
}

void vtImage::SetupBitmapInfo(const IPoint2 &size)
{
	DPoint2 spacing(m_Extents.Width() / size.x, m_Extents.Height() / size.y);
	const int smaller = min(size.x, size.y);

	// How many mipmaps to consider?
	const int powers = vt_log2(smaller) - 3;

	m_Bitmaps.resize(powers);
	IPoint2 size2 = size;
	for (int m = 0; m < powers; m++)
	{
		m_Bitmaps[m].number = m;
		m_Bitmaps[m].m_pBitmap = NULL;
		m_Bitmaps[m].m_bOnDisk = false;
		m_Bitmaps[m].m_Size = size2;
		m_Bitmaps[m].m_Spacing = spacing;

		size2.x = (size2.x + 1) / 2;	// Round up
		size2.y = (size2.y + 1) / 2;
		spacing *= 2;
	}
}

void vtImage::AllocMipMaps()
{
	FreeMipMaps();

	if (m_Bitmaps[0].m_pBitmap == NULL)
		return;

	const IPoint2 &size = m_Bitmaps[0].m_Size;
	const int depth = m_Bitmaps[0].m_pBitmap->GetDepth();

	for (size_t m = 1; m < m_Bitmaps.size(); m++)
	{
		if (!m_Bitmaps[m].m_pBitmap)
		{
			vtDIB *bm = new vtDIB;
			bm->Allocate(IPoint2(size.x >> m, size.y >> m), depth);
			m_Bitmaps[m].m_pBitmap = bm;
		}
	}
}

void vtImage::DrawMipMaps()
{
	vtDIB *big = m_Bitmaps[0].m_pBitmap;
	for (size_t m = 1; m < m_Bitmaps.size(); m++)
	{
		vtDIB *smaller = m_Bitmaps[m].m_pBitmap;
		SampleMipLevel(big, smaller);
		big = smaller;
	}
}

void vtImage::FreeMipMaps()
{
	for (size_t m = 1; m < m_Bitmaps.size(); m++)
	{
		delete m_Bitmaps[m].m_pBitmap;
		m_Bitmaps[m].m_pBitmap = NULL;
	}
}

bool vtImage::ReadPPM(const char *fname, bool progress_callback(int))
{
	// open input file
	FILE *fp = vtFileOpen(fname, "rb");
	if (!fp)		// Could not open input file
		return false;

	char sbuf[512],			// buffer for file input
		 dummy[2];			// used for \n and \0
	if (fread(sbuf, sizeof(char), 2, fp) != 2)
	{
		// "Could not read file"
		return false;
	}

	if (strncmp(sbuf, "P6", 2))
	{
		// not the flavor of PNM we're expecting
		fclose(fp);
		return false;
	}

	// P5 and P2 are not the flavors of PNM we're expecting:
	// if (!strncmp(sbuf, "P5", 2) || !strncmp(sbuf, "P2", 2))

	// read PGM ASCII or binary file
	bool have_header = false;
	int coord_sys = 0;
	int coord_zone = 0;
	int coord_datum = 0;
	DRECT ext;
	double x, y;
	int quiet;
	while ((fscanf(fp, "%s", sbuf) != EOF) && sbuf[0] == '#')
	{
		// comment
		quiet = fscanf(fp,"%[^\n]", sbuf);  // read comment beyond '#'
		quiet = fscanf(fp,"%[\n]", dummy);  // read newline
		char *buf = sbuf+1;	// skip leading space after '#'
		if (!strncmp(buf, "DEM", 3))
		{
			have_header= true;
		}
		else if (!strncmp(buf, "description", 11))
		{
			// do nothing
		}
		else if (!strncmp(buf, "coordinate system", 17))
		{
			if (!strcmp(buf+18, "LL")) coord_sys=0;
			else if (!strcmp(buf+18,"UTM")) coord_sys=1;
		}
		else if (!strncmp(buf, "coordinate zone", 15))
		{
			coord_zone = atoi(buf+16);
		}
		else if (!strncmp(buf, "coordinate datum", 16))
		{
			coord_datum = atoi(buf+17);
		}
		else if (!strncmp(buf, "SW corner", 9))
		{
			sscanf(buf+10, "%lf/%lf", &x, &y);
			ext.left = x;
			ext.bottom = y;
		}
		else if (!strncmp(buf, "NE corner", 9))
		{
			sscanf(buf+10, "%lf/%lf", &x, &y);
			ext.right = x;
			ext.top = y;
		}
		else if (!strncmp(buf, "missing value", 13))
		{
			// do nothing
		}
	}

	IPoint2 size;
	size.x = atoi(sbuf);			// store xsize of array

	quiet = fscanf(fp,"%s",sbuf);	// read ysize of array
	size.y = atoi(sbuf);

	quiet = fscanf(fp,"%s\n",sbuf);	// read maxval of array
	int maxval = atoi(sbuf);

	// set the corresponding vtElevationGrid info
	if (have_header)
	{
		int datum = EPSG_DATUM_WGS84;
		switch (coord_datum)
		{
		case 1: datum = EPSG_DATUM_NAD27; break;
		case 2: datum = EPSG_DATUM_WGS72; break;
		case 3: datum = EPSG_DATUM_WGS84; break;
		case 4: datum = EPSG_DATUM_NAD83; break;

		case 5: /*Sphere (with radius 6370997 meters)*/ break;
		case 6: /*Clarke1880 (Clarke spheroid of 1880)*/ break;
		case 7: /*International1909 (Geodetic Reference System of 1909)*/ break;
		case 8: /*International1967 (Geodetic Reference System of 1967)*/; break;

		case 9: /*WGS60*/ break;
		case 10: /*WGS66*/; break;
		case 11: /*Bessel1841*/ break;
		case 12: /*Krassovsky*/ break;
		}
		if (coord_sys == 0)	// LL
		{
			m_crs.SetSimple(false, 0, datum);
			ext.left /= 3600;	// arc-seconds to degrees
			ext.right /= 3600;
			ext.top /= 3600;
			ext.bottom /= 3600;
		}
		else if (coord_sys == 1)	// UTM
			m_crs.SetSimple(true, coord_zone, datum);
	}
	else
	{
		// Set the CRS (actually we don't know it)
		m_crs.SetSimple(true, 1, EPSG_DATUM_WGS84);

		ext.left = 0;
		ext.top = size.y - 1;
		ext.right = size.x - 1;
		ext.bottom = 0;
	}
	m_Extents = ext;
	vtDIB *pBitmap = new vtDIB;
	pBitmap->Allocate(size, 24);
	SetupBitmapInfo(size);
	m_Bitmaps[0].m_pBitmap = pBitmap;
	m_BitDepth = pBitmap->GetDepth();

	// read PPM binary
	int offset_start = ftell(fp);
	fseek(fp, 0, SEEK_END);
	int offset_end = ftell(fp);
	fseek(fp, offset_start, SEEK_SET);	// go back again

	int data_length = offset_end - offset_start;
	int data_size = data_length / (size.x * size.y);

	int line_length = 3 * size.x;
	uchar *line = new uchar[line_length];

	for (int j = 0; j < size.y; j++)
	{
		if (progress_callback != NULL)
			progress_callback(j * 100 / size.y);

		quiet = (int) fread(line, line_length, 1, fp);

		for (int i = 0; i < size.x; i++)
			pBitmap->SetPixel24(i, j, RGBi(line[i*3+0], line[i*3+1], line[i*3+2]));
	}
	delete [] line;
	fclose(fp);
	return true;
}

bool vtImage::WritePPM(const char *fname) const
{
	vtDIB *bm = m_Bitmaps[0].m_pBitmap;
	if (!bm)
		return false;

	// open input file
	FILE *fp = vtFileOpen(fname, "wb");
	if (!fp)		// Could not open output file
		return false;

	const IPoint2 size = bm->GetSize();
	fprintf(fp, "P6\n");		// PGM binary format
	fprintf(fp, "%d %d\n", size.x, size.y);
	fprintf(fp, "255\n");		// PGM standard value

	int line_length = 3 * size.x;
	uchar *line = new uchar[line_length];

	RGBi rgb;
	for (int j = 0; j < size.y; j++)
	{
		for (int i = 0; i < size.x; i++)
		{
			bm->GetPixel24(i, j, rgb);
			line[i*3+0] = rgb.r;
			line[i*3+1] = rgb.g;
			line[i*3+2] = rgb.b;
		}
		fwrite(line, line_length, 1, fp);
	}
	delete [] line;
	fclose(fp);
	return true;
}

bool vtImage::SaveToFile(const char *fname) const
{
	vtDIB *bm = m_Bitmaps[0].m_pBitmap;
	if (!bm)
		return false;
	IPoint2 size = m_Bitmaps[0].m_Size;

	DPoint2 spacing = GetSpacing();
	vtString sExt = GetExtension(fname, false); //get extension type
	if (sExt.CompareNoCase(".jpg") == 0)
	{
		if (bm->WriteJPEG(fname, 90))
		{
			// Also write JPEG world file
			vtString sJGWFile = ChangeFileExtension(fname, ".jgw");

			FILE *fout = vtFileOpen(sJGWFile, "w");
			if (fout)
			{
				// World file extents are always (strangely) pixel centers,
				//  not full image extents, so we have to scoot inwards by
				//  half a pixel.
				fprintf(fout, "%lf\n%lf\n%lf\n%lf\n%.2lf\n%.2lf\n",
					spacing.x, 0.0, 0.0, -1*spacing.y,
					m_Extents.left + (spacing.x/2),
					m_Extents.top - (spacing.y/2));
				fclose(fout);
			}
			return true;
		}
		else
			return false;
	}

	// Save with GDAL
	GDALDriverManager *pManager = GetGDALDriverManager();
	if (!pManager)
		return false;

	// JPEG: Error 6: GDALDriver::Create() ... no create method implemented for this format.
//	GDALDriver *pDriver = pManager->GetDriverByName("JPEG");
	GDALDriver *pDriver = pManager->GetDriverByName("GTiff");
	if (!pDriver)
		return false;

	char **papszOptions = NULL;

	// COMPRESS=[JPEG/LZW/PACKBITS/DEFLATE/CCITTRLE/CCITTFAX3/CCITTFAX4/NONE]
	if (g_Options.GetValueBool(TAG_TIFF_COMPRESS))
	    papszOptions = CSLSetNameValue( papszOptions, "COMPRESS", "DEFLATE" );

	GDALDataset *pDataset;
	const int bitDepth = bm->GetDepth();
	const int numBands = bitDepth / 8;
	pDataset = pDriver->Create(fname, size.x, size.y, numBands, GDT_Byte, papszOptions );
    CSLDestroy(papszOptions);

	if (!pDataset)
		return false;

	double adfGeoTransform[6] = { m_Extents.left, spacing.x, 0, m_Extents.top, 0, -spacing.y };
	pDataset->SetGeoTransform(adfGeoTransform);

	GByte *raster = new GByte[size.x*size.y];

	char *pszSRS_WKT = NULL;
	m_crs.exportToWkt( &pszSRS_WKT );
	pDataset->SetProjection(pszSRS_WKT);
	CPLFree( pszSRS_WKT );

	if (size.x * size.y > 512*512)
		OpenProgressDialog(_("Writing file"), wxString::FromUTF8((const char *) fname), false);

	// TODO: ask Frank if there is a way to gave GDAL write the file without
	// having to make another entire copy in memory, as it does now:
	RGBi rgb;
	RGBAi rgba;
	GDALRasterBand *pBand;
	for (int i = 1; i <= numBands; i++)
	{
		pBand = pDataset->GetRasterBand(i);

		for (int y = 0; y < size.y; y++)
		{
			progress_callback((i-1)*33 + (y * 33 / size.y));

			for (int x = 0; x < size.x; x++)
			{
				if (bitDepth == 8)
				{
					int value = bm->GetPixel8(x, y);
					raster[y*size.x + x] = value;
				}
				else if (bitDepth == 24)
				{
					bm->GetPixel24(x, y, rgb);
					if (i == 1) raster[y*size.x + x] = rgb.r;
					if (i == 2) raster[y*size.x + x] = rgb.g;
					if (i == 3) raster[y*size.x + x] = rgb.b;
				}
				else if (bitDepth == 32)
				{
					bm->GetPixel32(x, y, rgba);
					if (i == 1) raster[y*size.x + x] = rgb.r;
					if (i == 2) raster[y*size.x + x] = rgb.g;
					if (i == 3) raster[y*size.x + x] = rgb.b;
				}
			}
		}
		pBand->RasterIO( GF_Write, 0, 0, size.x, size.y,
			raster, size.x, size.y, GDT_Byte, 0, 0 );
	}
	delete [] raster;
	GDALClose(pDataset);

	CloseProgressDialog();

	return true;
}

bool vtImage::ReadPNGFromMemory(uchar *buf, int len)
{
	vtDIB *pBitmap = new vtDIB;
	if (pBitmap->ReadPNGFromMemory(buf, len))
	{
		SetupBitmapInfo(pBitmap->GetSize());
		m_Bitmaps[0].m_pBitmap = pBitmap;
		m_BitDepth = pBitmap->GetDepth();
		return true;
	}
	else
	{
		delete pBitmap;
		return false;
	}
}

bool vtImage::LoadFromGDAL(const char *fname)
{
	bool bRet = true;

	double affineTransform[6];
	const char *pProjectionString;
	OGRErr err;
	vtString message;

	g_GDALWrapper.RequestGDALFormats();

	// GDAL doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(fname);

	try
	{
		m_pDataset = (GDALDataset *)GDALOpen(fname_local, GA_Update);
		if (m_pDataset == NULL)
		{
			m_pDataset = (GDALDataset *)GDALOpen(fname_local, GA_ReadOnly);
			if (m_pDataset == NULL)
				throw "Couldn't open that file.";
		}

		vtCRS temp;
		bool bHaveCRS = false;
		pProjectionString = m_pDataset->GetProjectionRef();
		if (pProjectionString)
		{
			err = temp.importFromWkt((char**)&pProjectionString);

			// we must have a valid CRS, and it must not be local
			if (err == OGRERR_NONE && !temp.IsLocal())
			{
				m_crs = temp;
				bHaveCRS = true;
			}
		}
		if (!bHaveCRS)
		{
			// check for existence of .prj file
			if (temp.ReadProjFile(fname_local))
			{
				m_crs = temp;
				bHaveCRS = true;
			}
		}
		// if we still don't have it
		if (!bHaveCRS)
		{
			if (!g_bld->ConfirmValidCRS(&m_crs))
				throw "Import Cancelled.";
		}

		const IPoint2 Size(m_pDataset->GetRasterXSize(),
						   m_pDataset->GetRasterYSize());

		if (m_pDataset->GetGeoTransform(affineTransform) == CE_None)
		{
			m_Extents.left = affineTransform[0];
			m_Extents.right = m_Extents.left + affineTransform[1] * Size.x;
			m_Extents.top = affineTransform[3];
			m_Extents.bottom = m_Extents.top + affineTransform[5] * Size.y;
		}
		else
		{
			// No extents.
			m_Extents.SetToZero();
			wxString msg = _("File lacks geographic location (extents).  ");
			msg += _("Would you like to specify extents?\n");
			VTLOG(msg.mb_str(wxConvUTF8));
			int res = wxMessageBox(msg, _("Image Import"), wxYES_NO | wxCANCEL);
			if (res == wxYES)
			{
				VTLOG("Yes.\n");
				DRECT ext;
				ext.SetToZero();
				ExtentDlg dlg(NULL, -1, _("Extents"));
				dlg.SetArea(ext, (m_crs.IsGeographic() != 0));
				if (dlg.ShowModal() == wxID_OK)
					m_Extents = dlg.m_area;
				else
					throw "Import Cancelled.";
			}
			if (res == wxNO)
			{
				throw "Sorry, we need extents in order to make use of an image.";
			}
			if (res == wxCANCEL)
				throw "Import Cancelled.";
		}

		SetupBitmapInfo(Size);

		// GetRasterBand is 1-based.
		int iNumOverviews = m_pDataset->GetRasterBand(1)->GetOverviewCount();
		m_BitDepth = m_pDataset->GetRasterCount() * 8;

		GDALRasterBand *b0 = m_pDataset->GetRasterBand(1);
		GDALRasterBand *b1 = b0->GetOverview(0);
		GDALRasterBand *b2 = b0->GetOverview(1);
		GDALRasterBand *b3 = b0->GetOverview(2);
		GDALRasterBand *b4 = b0->GetOverview(3);

		for (int i = 0; i < iNumOverviews + 1 && i < (int)m_Bitmaps.size(); i++)
		{
			m_Bitmaps[i].m_bOnDisk = true;
			m_Bitmaps[i].m_linebuf.Setup(m_pDataset, i);
			IPoint2 imageSize = m_Bitmaps[i].m_Size;

			int iBigImage = g_Options.GetValueInt(TAG_MAX_MEGAPIXELS) * 1024 * 1024;
			bool bDefer = false;

			// don't try to load giant image?
			if (imageSize.x * imageSize.y > iBigImage)
			{
				if (g_Options.GetValueBool(TAG_LOAD_IMAGES_ALWAYS))
					bDefer = false;
				else if (g_Options.GetValueBool(TAG_LOAD_IMAGES_NEVER))
					bDefer = true;
				else
				{
					// Ask
					wxString msg;
					msg.Printf(_("Image is very large (%d x %d).\n"), imageSize.x, imageSize.y);
					msg += _("Would you like to create the layer using out-of-memory access to the image?"),
						VTLOG(msg.mb_str(wxConvUTF8));
					int result = wxMessageBox(msg, _("Question"), wxYES_NO);
					if (result == wxYES)
						bDefer = true;
				}
			}

			if (!bDefer)
			{
				vtDIB *pBitmap = new vtDIB;
				if (!pBitmap->Allocate(imageSize, m_BitDepth))
				{
					delete pBitmap;
					wxString msg;
					msg.Printf(_("Couldn't allocate bitmap of size %d x %d.\n"),
						imageSize.x, imageSize.y);
					msg += _("Would you like to create the layer using out-of-memory access to the image?"),
						VTLOG(msg.mb_str(wxConvUTF8));
					int result = wxMessageBox(msg, _("Question"), wxYES_NO);
					if (result == wxYES)
						bDefer = true;
					else
						throw "Couldn't allocate bitmap";
				}

				if (!bDefer)
				{
					if (imageSize.x * imageSize.y > 512 * 512)
						OpenProgressDialog(_("Reading file"), wxString::FromUTF8((const char *)fname), false);

					// Read the data
					VTLOG("Reading the image data (%d x %d pixels)\n", imageSize.x, imageSize.y);
					RGBi rgb;
					for (int iY = 0; iY < imageSize.y; iY++)
					{
						if (UpdateProgressDialog(iY * 99 / imageSize.y))
						{
							// cancel
							throw "Cancelled";
						}
						const RGBAi *data = m_Bitmaps[i].m_linebuf.GetScanlineFromBuffer(iY);
						for (int iX = 0; iX < imageSize.x; iX++)
						{
							switch (pBitmap->GetDepth())
							{
							case 8:
								pBitmap->SetPixel8(iX, iY, data[iX].r);
								break;
							case 24:
								pBitmap->SetPixel24(iX, iY, RGBi(data[iX]));
								break;
							case 32:
								pBitmap->SetPixel32(iX, iY, data[iX]);
								break;
							}
						}
					}
					m_Bitmaps[i].m_pBitmap = pBitmap;
				}
			}
		}
	}

	catch (const char *msg)
	{
		/*if (!bDefer)
		{*/
			bRet = false;

			vtString str = "Couldn't load Image layer ";

			str += "\"";
			str += fname;
			str += "\"";

			str += ": ";
			str += msg;

			DisplayAndLog(str);
		/*}*/
	}

	CloseProgressDialog();

	// Don't close the GDAL Dataset; leave it open just in case we need it.
//	GDALClose(pDataset);

	return bRet;
}

int CPL_STDCALL GDALProgress(double amount, const char *message, void *context)
{
	static int lastamount = -1;
	int newamount = amount * 99;
	if (newamount != lastamount)
	{
		lastamount = newamount;
		progress_callback(newamount);
	}
	return 1;
}

bool vtImage::CreateOverviews()
{
	if (!m_pDataset)
		return false;

	int panOverviewList[6] = { 2, 4, 8, 16, 32, 64 };
	int nOverviews = 6;
	CPLErr err = m_pDataset->BuildOverviews("AVERAGE", nOverviews, panOverviewList,
		0, NULL, GDALProgress, this);

	IPoint2 rasterSize(m_pDataset->GetRasterXSize(), m_pDataset->GetRasterYSize());
	SetupBitmapInfo(rasterSize);

	int iNumOverviews = m_pDataset->GetRasterBand(1)->GetOverviewCount();

	for (int i = 0; i < iNumOverviews && i < nOverviews; i++)
	{
		m_Bitmaps[i].m_bOnDisk = true;
		m_Bitmaps[i].m_linebuf.Setup(m_pDataset, i);
	}

	return (err == CE_None);
}

bool vtImage::LoadOverviews()
{
	// TODO
	return true;
}

// Helper
bool GetBitDepthUsingGDAL(const char *fname, int &depth_in_bits, GDALDataType &eType)
{
	// GDAL doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(fname);

	GDALDataset *pDataset = (GDALDataset *) GDALOpen(fname_local, GA_ReadOnly);
	if (pDataset == NULL)
		return false;

	// Raster count should be 3 for colour images (assume RGB)
	int iRasterCount = pDataset->GetRasterCount();
	GDALRasterBand *pBand = pDataset->GetRasterBand(1);
	eType = pBand->GetRasterDataType();
	depth_in_bits = iRasterCount * GDALGetDataTypeSize(eType);
	GDALClose(pDataset);

	return true;
}

bool vtImage::WriteTileset(TilingOptions &opts, BuilderView *pView)
{
	VTLOG1("vtImage::WriteTileset:\n");

	wxFrame *frame = NULL;

	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	// Check that options are valid
	CheckCompressionMethod(opts);
	bool bJPEG = (opts.bUseTextureCompression && opts.eCompressionType == TC_JPEG);

#if USE_OPENGL
	if (opts.bUseTextureCompression && opts.eCompressionType == TC_OPENGL)
	{
		frame = new wxFrame;
		frame->Create(pView, -1, _T("Texture Compression OpenGL Context"),
			wxPoint(100,400), wxSize(280, 300), wxCAPTION | wxCLIP_CHILDREN);
		m_pCanvas = new ImageGLCanvas(frame);
	}
#endif

	// largest tile size
	int base_tilesize = opts.lod0size;

	DRECT area = m_Extents;
	DPoint2 tile_dim(area.Width()/opts.cols, area.Height()/opts.rows);
	DPoint2 cell_size = tile_dim / base_tilesize;

	vtString units = GetLinearUnitName(m_crs.GetUnits());
	units.MakeLower();
	int zone = m_crs.GetUTMZone();
	vtString crs;
	if (m_crs.IsGeographic())
		crs = "LL";
	else if (zone != 0)
		crs = "UTM";
	else
		crs = "Other";

	// Try to create directory to hold the tiles
	vtString dirname = opts.fname;
	RemoveFileExtensions(dirname);
	if (!vtCreateDir(dirname))
		return false;

	// make a note of which lods exist
	LODMap lod_existence_map(opts.cols, opts.rows);

	if (!m_Bitmaps[0].m_pBitmap)
	{
		IPoint2 size = m_Bitmaps[0].m_Size;

		// If we're dealing with an out-of-core image, consider how many rows
		//  we need to cache to avoid reading the file more than once during
		//  the generation of the tiles
		int need_cache_rows = (size.y + (opts.rows-1)) / opts.rows;
		int need_cache_bytes = need_cache_rows * size.x * 3;
		// add a little bit for rounding up
		need_cache_bytes += (need_cache_bytes / 10);

		// there's little point in shrinking the cache, so check existing size
		int existing = GDALGetCacheMax();
		if (need_cache_bytes > existing)
			GDALSetCacheMax(need_cache_bytes);
	}

	int i, j, lod;
	m_iTotal = opts.rows * opts.cols * opts.numlods;
	m_iCompleted = 0;

	clock_t tm1 = clock();
	bool bCancelled = false;
	for (j = opts.rows-1; j >= 0 && !bCancelled; j--)
	{
		for (i = 0; i < opts.cols && !bCancelled; i++)
		{
			// We might want to skip certain rows
			if (opts.iMinRow != -1 &&
				(i < opts.iMinCol || i > opts.iMaxCol ||
				 j < opts.iMinRow || j > opts.iMaxRow))
				continue;

			DRECT tile_area;
			tile_area.left = area.left + tile_dim.x * i;
			tile_area.right = area.left + tile_dim.x * (i+1);
			tile_area.bottom = area.bottom + tile_dim.y * j;
			tile_area.top = area.bottom + tile_dim.y * (j+1);

			int col = i;
			int row = opts.rows-1-j;

			// We know this tile will be included, so note the LODs present
			int base_tile_exponent = vt_log2(base_tilesize);
			lod_existence_map.set(i, j, base_tile_exponent, base_tile_exponent-(opts.numlods-1));

			for (lod = 0; lod < opts.numlods && !bCancelled; lod++)
			{
				if (!WriteTile(opts, pView, dirname, tile_area, tile_dim,
					col, row, lod))
					bCancelled = true;
			}
		}
	}
	if (bCancelled)
		wxMessageBox(_("Cancelled."));
	else
	{
		// Write .ini file
		WriteTilesetHeader(opts.fname, opts.cols, opts.rows,
			opts.lod0size, area, m_crs, INVALID_ELEVATION, INVALID_ELEVATION,
			&lod_existence_map, bJPEG);

		clock_t tm2 = clock();
		float elapsed = ((float)tm2 - tm1)/CLOCKS_PER_SEC;
		wxString str;
		str.Printf(_("Wrote %d tiles (%d cells) in %.1f seconds (%.2f seconds per cell)"),
			m_iTotal, (opts.rows * opts.cols), elapsed, elapsed/(opts.rows * opts.cols));
		wxMessageBox(str);
	}
	if (pView)
		pView->HideGridMarks();

#if USE_OPENGL
	if (frame)
		frame->Close();
	delete frame;
	m_pCanvas = NULL;
#endif

	return !bCancelled;
}

bool vtImage::WriteTile(const TilingOptions &opts, BuilderView *pView, vtString &dirname,
							 DRECT &tile_area, DPoint2 &tile_dim, int col, int row, int lod)
{
	int tilesize = opts.lod0size >> lod;

	// safety check: don't create LODs which are too small
	if (tilesize < 2)
		return true;

	// Images are written as PixelIsPoint, and each LOD has
	// its own sample spacing
	DPoint2 spacing = tile_dim / (tilesize-1);

	// Write DB file (libMini's databuf format)
	bool bJPEG = (opts.bUseTextureCompression && opts.eCompressionType == TC_JPEG);
	vtString fname = MakeFilenameDB(dirname, col, row, lod);

	// make a message for the progress dialog
	wxString msg;
	msg.Printf(_("Writing tile '%hs', size %dx%d"),
		(const char *)fname, tilesize, tilesize);
	if (UpdateProgressDialog(m_iCompleted*99/m_iTotal, msg))
	{
		// User cancelled.
		return false;
	}

	// also draw our progress in the main view
	if (pView)
		pView->SetGridMarks(m_Extents, opts.cols, opts.rows, col, opts.rows-1-row);

	// First, fill a buffer with the uncompressed texels
	uchar *rgb_bytes = (uchar *) malloc(tilesize * tilesize * 3);
	int cb = 0;	// count bytes

	DPoint2 p;
	RGBAi rgba;

	// Get ready to multisample
	DLine2 offsets;
	int iNSampling = g_Options.GetValueInt(TAG_SAMPLING_N);
	MakeSampleOffsets(spacing, iNSampling, offsets);
	double dRes = (spacing.x+spacing.y)/2;

	for (int y = tilesize-1; y >= 0; y--)
	{
		p.y = tile_area.bottom + y * spacing.y;
		for (int x = 0; x < tilesize; x++)
		{
			p.x = tile_area.left + x * spacing.x;

			GetMultiSample(p, offsets, rgba, dRes);
#if 0 // LOD Stripes
			// For testing, add stripes to indicate LOD
			if (lod == 3 && x == y) rgb.Set(255,0,0);

			if (lod == 2 && (
				x == tilesize-y ||
				x == y+tilesize/2 ||
				x == y-tilesize/2)) rgb.Set(0,255,0);

			if (lod == 1 && (x%16)==0) rgb.Set(0,0,90);

			if (lod == 0 && (y%8)==0) rgb.Set(90,0,90);
#endif
			rgb_bytes[cb++] = rgba.r;
			rgb_bytes[cb++] = rgba.g;
			rgb_bytes[cb++] = rgba.b;
		}
	}
	int iUncompressedSize = cb;

	vtMiniDatabuf output_buf;
	output_buf.xsize = tilesize;
	output_buf.ysize = tilesize;
	output_buf.zsize = 1;
	output_buf.tsteps = 1;
	output_buf.SetBounds(m_crs, tile_area);

	// Write and optionally compress the image
	WriteMiniImage(fname, opts, rgb_bytes, output_buf,
		iUncompressedSize, m_pCanvas);

	// Free the uncompressed image
	free(rgb_bytes);

	m_iCompleted++;

	return true;
}

void SampleMipLevel(vtDIB *bigger, vtDIB *smaller)
{
	const IPoint2 size = bigger->GetSize();

	const int xsmall = size.x / 2;
	const int ysmall = size.y / 2;

	for (int y = 0; y < ysmall; y++)
	{
		if ((y%32)==0)
			progress_callback(y * 99 / ysmall);

		if (bigger->GetDepth() == 8)
		{
			int sum;
			for (int x = 0; x < xsmall; x++)
			{
				sum = 0;

				sum += bigger->GetPixel8(x * 2, y * 2);
				sum += bigger->GetPixel8(x * 2 + 1, y * 2);
				sum += bigger->GetPixel8(x * 2, y * 2 + 1);
				sum += bigger->GetPixel8(x * 2, y * 2);

				smaller->SetPixel8(x, y, sum / 4);
			}
		}
		else if (bigger->GetDepth() == 24)
		{
			RGBi rgb, sum;
			for (int x = 0; x < xsmall; x++)
			{
				sum.Set(0, 0, 0);

				bigger->GetPixel24(x * 2, y * 2, rgb);
				sum += rgb;
				bigger->GetPixel24(x * 2 + 1, y * 2, rgb);
				sum += rgb;
				bigger->GetPixel24(x * 2, y * 2 + 1, rgb);
				sum += rgb;
				bigger->GetPixel24(x * 2, y * 2, rgb);
				sum += rgb;

				smaller->SetPixel24(x, y, sum / 4);
			}
		}
	}
}
