//
// Parts of this module were derived from example code in the
// 1996 Microsoft Systems Journal, by Paul DiLascia.
//
// CDib - Device Independent Bitmap.
// This implementation draws bitmaps using normal Win32 API functions,
// not DrawDib. CDib is derived from CBitmap, so you can use it with
// any other MFC functions that use bitmaps.
//
// Changes 8/99: added constructor to convert from DIB to CDib
//

#include "StdAfx.h"
#include "BExtractor.h"
#include "Dib.h"

// GDAL
#include "ogr_spatialref.h"
#include "gdal_priv.h"
#include "bimage.h"
#include "BExtractor.h"
#include "BExtractorDoc.h"
#include "BExtractorView.h"
#include "ProgDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int MAXPALCOLORS = 256;

IMPLEMENT_DYNAMIC(CDib, CObject)

CDib::CDib()
{
	memset(&m_bm, 0, sizeof(m_bm));
	m_hdd = NULL;
	m_bmi = NULL;
	m_data = NULL;
	m_colors = NULL;
	m_hbm = NULL;
}

CDib::~CDib()
{
	DeleteObject();
	if (m_bmi)
		delete m_bmi;
	m_bmi = NULL;
}

bool CDib::Setup(CDC* pDC, int width, int height, int bits_per_pixel,
				 HDRAWDIB hdd, RGBQUAD *colors)
{
	m_hdd = hdd;

	int bpp = bits_per_pixel;
	m_stride = ((width * bpp + 31)/32) * 4;
	int datasize = m_stride * height;

	void *buf = malloc(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
	m_bmi = (BITMAPINFOHEADER *)buf;
	if (bpp == 8)
	{
		m_colors = (RGBQUAD *) (m_bmi + 1);
		for (int i = 0; i < 256; i++)
		{
			if (colors)
				m_colors[i] = colors[i];
			else
			{
				m_colors[i].rgbBlue = i;
				m_colors[i].rgbGreen = i;
				m_colors[i].rgbRed = i;
				m_colors[i].rgbReserved = 0;
			}
		}
	}

	m_bmi->biSize = sizeof(BITMAPINFOHEADER);
	m_bmi->biWidth = width;
	m_bmi->biHeight = height;
	m_bmi->biPlanes = 1;
	m_bmi->biBitCount = bpp;
	m_bmi->biCompression = BI_RGB;
	m_bmi->biSizeImage = datasize;
	m_bmi->biXPelsPerMeter = 1;
	m_bmi->biYPelsPerMeter = 1;
	m_bmi->biClrUsed = (bpp == 8) ? 256 : 0;
	m_bmi->biClrImportant = m_bmi->biClrUsed;

	m_hbm = CreateDIBSection(pDC->GetSafeHdc(),
							(BITMAPINFO *) m_bmi,
							DIB_RGB_COLORS,
							&m_data,
							NULL,
							0);
	BOOL result = Attach(m_hbm);
	return true;
}

bool CDib::Setup(CDC* pDC, GDALDataset *pDataset, HDRAWDIB hdd,
				 bool progress_callback(int))
{
	GDALColorEntry Ent;
	int x, y, i;
	int nxValid, nyValid;
	CPLErr Err;
	int iPixelWidth;
	int iPixelHeight;
	int xBlockSize, yBlockSize;
	int nxBlocks, nyBlocks;
	int ixBlock, iyBlock;
	int iRasterCount;
	GDALRasterBand *pBand;
	GDALColorTable *pTable;
	GDALRasterBand *pRed = NULL;
	GDALRasterBand *pGreen = NULL;
	GDALRasterBand *pBlue = NULL;
	char *pRedline = NULL;
	char *pGreenline = NULL;
	char *pBlueline = NULL;
	int iX, iY, flipY;
	uchar *pGdalBuffer = NULL;
	RGBQUAD q;

	m_hdd = hdd;

	BITMAPINFO BitmapInfo =
	{
		{
			sizeof(BITMAPINFOHEADER), // Sizeof structure
			0, // width
			0, // height
			1, // number of bit planes (always one)
			24, // number of bits per pixel
			BI_RGB, // compression
			0, // size of image data (if bI_RGB then only needs to be set for things like Dib Sections)
			1, // pixels per meter X
			1, // pixels per meter Y
			0, // colours used (0 for 24 bits per pixel)
			0 // colours important (0 for 24 bits per pixel)
		},
		0
	};

	// This all assumes MM_TEXT
	try
	{
		iPixelWidth = pDataset->GetRasterXSize();
		iPixelHeight = pDataset->GetRasterYSize();

		BitmapInfo.bmiHeader.biWidth = iPixelWidth;
		BitmapInfo.bmiHeader.biHeight = iPixelHeight;
		m_stride = ((iPixelWidth * BitmapInfo.bmiHeader.biBitCount + 31)/32) * 4;
		BitmapInfo.bmiHeader.biSizeImage = m_stride * iPixelHeight;

		m_hbm = CreateDIBSection(pDC->GetSafeHdc(), &BitmapInfo,
			DIB_RGB_COLORS, &m_data, NULL, 0);

		// temporarily point to the BMI header of the drawdib
		m_bmi = &BitmapInfo.bmiHeader;

		if (NULL == m_hbm)
			throw "Cannot create bitmap section";
		if (!Attach(m_hbm))
			throw "Cannot attach bitmap section";

		iRasterCount = pDataset->GetRasterCount();

		// Put the image data into the bitmap
		// I am not going to change this data so I only need to do this once
		// If the rest of the code did not assume a DIbSection I could
		// use the set SetDIBits function for each group of scanlines
		if (iRasterCount == 1)
		{
			pBand = pDataset->GetRasterBand(1);
			// Check data type - it's either integer or float
			if (GDT_Byte != pBand->GetRasterDataType())
				throw "Raster is not of type byte.";
			if (GCI_PaletteIndex != pBand->GetColorInterpretation())
				throw "Couldn't get palette.";
			if (NULL == (pTable = pBand->GetColorTable()))
				throw "Couldn't get color table.";

			pBand->GetBlockSize(&xBlockSize, &yBlockSize);

			if (NULL == (pGdalBuffer = new uchar[xBlockSize * yBlockSize]))
				throw "Couldnt allocate buffer for GDAL.";

			nxBlocks = (iPixelWidth + xBlockSize - 1) / xBlockSize;
			nyBlocks = (iPixelHeight + yBlockSize - 1) / yBlockSize;
			// Read the data
			// Convert to rgb and write to image
			pBand->FlushBlock(0, 0); // Bug in gdal
			for (iyBlock = 0; iyBlock < nyBlocks; iyBlock++)
			{
				if (progress_callback != NULL)
					progress_callback(iyBlock * 100 / nyBlocks);

				y = iyBlock * yBlockSize;
				for (ixBlock = 0; ixBlock < nxBlocks; ixBlock++)
				{
					x = ixBlock * xBlockSize;
					Err = pBand->ReadBlock(ixBlock, iyBlock, pGdalBuffer);
					if (Err != CE_None)
						throw "Problem reading the image data.";

					// Compute the portion of the block that is valid
					// for partial edge blocks.
					if ((ixBlock+1) * xBlockSize > iPixelWidth)
						nxValid = iPixelWidth - ixBlock * xBlockSize;
					else
						nxValid = xBlockSize;

					if( (iyBlock+1) * yBlockSize > iPixelHeight)
						nyValid = iPixelHeight - iyBlock * yBlockSize;
					else
						nyValid = yBlockSize;

					for( iY = 0; iY < nyValid; iY++ )
					{
						flipY = iPixelHeight - 1 - iY;
						for( iX = 0; iX < nxValid; iX++ )
						{
							pTable->GetColorEntryAsRGB(pGdalBuffer[iY * xBlockSize + iX], &Ent);
							// DIBs are bottom-up, not top-down
							q.rgbRed = (uchar) Ent.c1;
							q.rgbGreen = (uchar) Ent.c2;
							q.rgbBlue = (uchar) Ent.c3;
							SetPixel24(x + iX, flipY - y, q);
						}
					}
				}
#ifdef DONTUSESECTION
				// This function takes data from a buffer in which the data is
				// stored in DIB format as specified by the BITMAPINFO structure
				// and copies it into a device dependant bitmap (DDB) in
				// whatever internal format that is using.
				// The third parameter is the starting scanline in the TARGET
				// DDB!!! that the data is to be written to, this is always a
				// bottom up co-ordinate (0 = bottom). The fourth parameter is
				// the number of scanlines contained in the SOURCE buffer and
				// to be written to the target. The source buffer maybe organised
				// top down or bottom up depending on the setting in the
				// BITMAPINFO structure. This function will sort it out. This
				// means that you can select which whole scanlines to set in the target but not parts of scanlines
				if (!SetDIBits(m_MemoryDC, m_Bitmap, m_iPixelHeight - y - 1,
							   nyValid, m_data, &BitmapInfo, DIB_RGB_COLORS))
					throw "SetDIBits failed.";
#endif
			}
		}
		else if (iRasterCount == 3)
		{
			for (i = 1; i <= 3; i++)
			{
				pBand = pDataset->GetRasterBand(i);
				// Check data type - it's either integer or float
				if (GDT_Byte != pBand->GetRasterDataType())
					throw "Three rasters, but not of type byte.";
				switch(pBand->GetColorInterpretation())
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

			nxBlocks = (iPixelWidth + xBlockSize - 1) / xBlockSize;
			nyBlocks = (iPixelHeight + yBlockSize - 1) / yBlockSize;

			pRed->FlushBlock(0, 0);
			pGreen->FlushBlock(0, 0);
			pBlue->FlushBlock(0, 0);

			if (NULL == (pRedline = new char[xBlockSize * yBlockSize]))
				throw "Cannot allocate Red Scanline buffer.";
			if (NULL == (pGreenline = new char[xBlockSize * yBlockSize]))
				throw "Cannot allocate Green Scanline buffer.";
			if (NULL == (pBlueline = new char[xBlockSize * yBlockSize]))
				throw "Cannot allocate Blue Scanline buffer.";

			for (iyBlock = 0; iyBlock < nyBlocks; iyBlock++)
			{
				if (progress_callback != NULL)
					progress_callback(iyBlock * 100 / nyBlocks);

				y = iyBlock * yBlockSize;
				for (ixBlock = 0; ixBlock < nxBlocks; ixBlock++)
				{
					x = ixBlock * xBlockSize;
					Err = pRed->ReadBlock(ixBlock, iyBlock, pRedline);
					if (Err != CE_None)
						throw "Cannot read data.";
					Err = pGreen->ReadBlock(ixBlock, iyBlock, pGreenline);
					if (Err != CE_None)
						throw "Cannot read data.";
					Err = pBlue->ReadBlock(ixBlock, iyBlock, pBlueline);
					if (Err != CE_None)
						throw "Cannot read data.";

					// Compute the portion of the block that is valid
					// for partial edge blocks.
					if ((ixBlock+1) * xBlockSize > iPixelWidth)
						nxValid = iPixelWidth - ixBlock * xBlockSize;
					else
						nxValid = xBlockSize;

					if( (iyBlock+1) * yBlockSize > iPixelHeight)
						nyValid = iPixelHeight - iyBlock * yBlockSize;
					else
						nyValid = yBlockSize;

					for (int iY = 0; iY < nyValid; iY++)
					{
						for (int iX = 0; iX < nxValid; iX++)
						{
							// Reverse the order for a DIB
							q.rgbRed = pRedline[iY * xBlockSize + iX];
							q.rgbGreen = pGreenline[iY * xBlockSize + iX];
							q.rgbBlue = pBlueline[iY * xBlockSize + iX];
							SetPixel24(x + iX, iPixelHeight - 1 - iY - y, q);
						}
					}
				}
#ifdef DONTUSESECTION
				// This function takes data from a buffer in which the data is stored in DIB format
				// as specified by the BITMAPINFO structure and copies it into a device dependant bitmap (DDB)
				// in whatever internal format that is using.
				// The third parameter is the starting scanline in the TARGET DDB!!! that the data is to be written
				// to, this is always a bottom up co-ordinate (0 = bottom). The fourth parameter is the number of scanlines
				// contained in the SOURCE buffer and to be written to the target. The source buffer maybe organised top
				// down or bottom up depending on the setting in the BITMAPINFO structure. This function will sort it out.
				// This means that you can select which whole scanlines to set in the target but not parts of scanlines
				if (!SetDIBits(m_MemoryDC, m_Bitmap, iPixelHeight - y - 1, nyValid, m_data, &BitmapInfo, DIB_RGB_COLORS))
					throw "SetDIBits failed.";
#endif
			}
		}
		else
			throw "Image does not have 1 or 3 bands.";
	}

	catch (const char *msg)
	{
		HRESULT hResult = GetLastError();
		CString ErrorMessage;
		CString DisplayMessage;

		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, hResult, 0, ErrorMessage.GetBuffer(1024), 1023, NULL);

		DisplayMessage.Format("%s %08x\n%s\n",
							msg, hResult,
							(LPCSTR)ErrorMessage);

		AfxMessageBox(DisplayMessage);
		return false;
	}

	// Ugly !!
	void *buf = malloc(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
	m_bmi = (BITMAPINFOHEADER *)buf;
	*m_bmi = BitmapInfo.bmiHeader;

	if (NULL != pGdalBuffer)
		delete pGdalBuffer;
	if (NULL != pRedline)
		delete pRedline;
	if (NULL != pGreenline)
		delete pGreenline;
	if (NULL != pBlueline)
		delete pBlueline;
	return true;
}


//////////////////
// Delete Object. Delete DIB and palette.
//
BOOL CDib::DeleteObject()
{
	m_pal.DeleteObject();
	memset(&m_bm, 0, sizeof(m_bm));
	return CBitmap::DeleteObject();
}

//////////////////
// Read DIB from file.
//
BOOL CDib::Load(LPCTSTR szPathName)
{
	return Attach(::LoadImage(NULL, szPathName, IMAGE_BITMAP, 0, 0,
		LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_DEFAULTSIZE));
}

//////////////////
// Load bitmap resource. Never tested.
//
BOOL CDib::Load(HINSTANCE hInst, LPCTSTR lpResourceName)
{
	return Attach(::LoadImage(hInst, lpResourceName, IMAGE_BITMAP, 0, 0,
		LR_CREATEDIBSECTION | LR_DEFAULTSIZE));
}

//////////////////
// Attach is just like the CGdiObject version,
// except it also creates the palette
//
BOOL CDib::Attach(HGDIOBJ hbm)
{
	if (CBitmap::Attach(hbm)) {
		if (!GetBitmap(&m_bm))			// load BITMAP for speed
			return FALSE;
		m_pal.DeleteObject();			// in case one is already there
		return CreatePalette(m_pal);	// create palette
	}
	return FALSE;
}

//////////////////
// Get size (width, height) of bitmap.
// extern fn works for ordinary CBitmap objects.
//
CSize GetBitmapSize(CBitmap* pBitmap)
{
	BITMAP bm;
	return pBitmap->GetBitmap(&bm) ?
		CSize(bm.bmWidth, bm.bmHeight) : CSize(0,0);
}

//////////////////
// You can use this static function to draw ordinary
// CBitmaps as well as CDibs
//
BOOL DrawBitmap(CDC& dc, CBitmap* pBitmap,
	const CRect* rcDst, const CRect* rcSrc)
{
	// Compute rectangles where NULL specified
	CRect rc;
	if (!rcSrc) {
		// if no source rect, use whole bitmap
		rc = CRect(CPoint(0,0), GetBitmapSize(pBitmap));
		rcSrc = &rc;
	}
	if (!rcDst) {
		// if no destination rect, use source
		rcDst=rcSrc;
	}

	// Create memory DC
	CDC memdc;
	memdc.CreateCompatibleDC(&dc);
	CBitmap* pOldBm = memdc.SelectObject(pBitmap);

	// Blast bits from memory DC to target DC.
	// Use StretchBlt if size is different.
	//
	BOOL bRet = FALSE;
	if (rcDst->Size()==rcSrc->Size())
	{
		bRet = dc.BitBlt(rcDst->left, rcDst->top,
			rcDst->Width(), rcDst->Height(),
			&memdc, rcSrc->left, rcSrc->top, SRCCOPY);
	}
	else
	{
		dc.SetStretchBltMode(COLORONCOLOR);
		bRet = dc.StretchBlt(rcDst->left, rcDst->top, rcDst->Width(),
			rcDst->Height(), &memdc, rcSrc->left, rcSrc->top, rcSrc->Width(),
			rcSrc->Height(), SRCCOPY);
	}
	memdc.SelectObject(pOldBm);

	return bRet;
}

////////////////////////////////////////////////////////////////
// Draw DIB on caller's DC. Does stretching from source to destination
// rectangles. Generally, you can let the following default to zero/NULL:
//
//		bUseDrawDib = whether to use use DrawDib, default TRUE
//		pPal		  = palette, default=NULL, (use DIB's palette)
//		bForeground = realize in foreground (default FALSE)
//
// If you are handling palette messages, you should use bForeground=FALSE,
// since you will realize the foreground palette in WM_QUERYNEWPALETTE.
//
BOOL CDib::Draw(CDC& dc, const CRect* rcDst, const CRect* rcSrc,
	BOOL bUseDrawDib, CPalette* pPal, BOOL bForeground)
{
	if (!m_hObject)
		return FALSE;

	// Select, realize palette
	if (pPal==NULL)				// no palette specified:
		pPal = GetPalette();		// use default
	CPalette* pOldPal = dc.SelectPalette(pPal, !bForeground);
	dc.RealizePalette();

	BOOL bRet = FALSE;
	if (bUseDrawDib)
	{
		// Compute rectangles where NULL specified
		//
		CRect rc(0,0,-1,-1);	// default for DrawDibDraw
		if (!rcSrc)
			rcSrc = &rc;
		if (!rcDst)
			rcDst=rcSrc;
		if (!m_hdd)
			VERIFY(m_hdd = DrawDibOpen());	//should never get called here anymore

		GetDIBFromSection();

		// Let DrawDib do the work!
		bRet = DrawDibDraw(m_hdd, dc,
			rcDst->left, rcDst->top, rcDst->Width(), rcDst->Height(),
			m_bmi,			// ptr to BITMAPINFOHEADER + colors
			m_bm.bmBits,	// bits in memory
			rcSrc->left, rcSrc->top, rcSrc->Width(), rcSrc->Height(),
			bForeground ? 0 : DDF_BACKGROUNDPAL);
	}
	else
	{
		// use normal draw function
		bRet = DrawBitmap(dc, this, rcDst, rcSrc);
	}
	if (pOldPal)
		dc.SelectPalette(pOldPal, TRUE);
	return bRet;
}


void CDib::GetDIBFromSection()
{
	// Get BITMAPINFOHEADER/color table. I copy into stack object each time.
	// This doesn't seem to slow things down visibly.
	//
	DIBSECTION ds;

	VERIFY(GetObject(sizeof(ds), &ds)==sizeof(ds));
	char buf[sizeof(BITMAPINFOHEADER) + MAXPALCOLORS*sizeof(RGBQUAD)];
	BITMAPINFOHEADER& bmih = *(BITMAPINFOHEADER*)buf;
	RGBQUAD* colors = (RGBQUAD*)(m_bmi+1);
	// N.B. There is a known bug in GetObject it always returns a positive
	// bitmap height, even if the bitmap is top down see MSKB article Q186586
	// but because drawdib cannot handle negatives it doesn't matter
	memcpy(m_bmi, &ds.dsBmih, sizeof(BITMAPINFOHEADER));
	if (m_bmi->biBitCount == 8)
		GetColorTable(m_colors, MAXPALCOLORS);
}

#define PALVERSION 0x300	// magic number for LOGPALETTE

//////////////////
// Create the palette. Use halftone palette for hi-color bitmaps.
//
BOOL CDib::CreatePalette(CPalette& pal)
{
	// should not already have palette
	ASSERT(pal.m_hObject==NULL);

	BOOL bRet = FALSE;
	RGBQUAD* colors = new RGBQUAD[MAXPALCOLORS];
	UINT nColors = GetColorTable(colors, MAXPALCOLORS);
	if (nColors > 0)
	{
		// Allocate memory for logical palette
		int len = sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * nColors;
		LOGPALETTE* pLogPal = (LOGPALETTE*)new char[len];
		if (!pLogPal)
			return NULL;

		// set version and number of palette entries
		pLogPal->palVersion = PALVERSION;
		pLogPal->palNumEntries = nColors;

		// copy color entries
		for (UINT i = 0; i < nColors; i++)
		{
			pLogPal->palPalEntry[i].peRed   = colors[i].rgbRed;
			pLogPal->palPalEntry[i].peGreen = colors[i].rgbGreen;
			pLogPal->palPalEntry[i].peBlue  = colors[i].rgbBlue;
			pLogPal->palPalEntry[i].peFlags = 0;
		}

		// create the palette and destroy LOGPAL
		bRet = pal.CreatePalette(pLogPal);
		delete [] (char*)pLogPal;
	} else {
		CWindowDC dcScreen(NULL);
		bRet = pal.CreateHalftonePalette(&dcScreen);
	}
	delete colors;
	return bRet;
}

//////////////////
// Helper to get color table. Does all the mem DC voodoo.
//
UINT CDib::GetColorTable(RGBQUAD* colorTab, UINT nColors)
{
	CWindowDC dcScreen(NULL);
	CDC memdc;
	memdc.CreateCompatibleDC(&dcScreen);
	CBitmap* pOldBm = memdc.SelectObject(this);
	nColors = GetDIBColorTable(memdc, 0, nColors, colorTab);
	memdc.SelectObject(pOldBm);
	return nColors;
}

void CDib::SetPixel24(int x, int y, const RGBQUAD &rgb)
{
	if (m_bmi->biBitCount == 24)
	{
		((char *)m_data)[y * m_stride + (x * 3)] = rgb.rgbBlue;
		((char *)m_data)[y * m_stride + (x * 3) + 1] = rgb.rgbGreen;
		((char *)m_data)[y * m_stride + (x * 3) + 2] = rgb.rgbRed;
	}
	else if (m_bmi->biBitCount == 8)
	{
		// TODO if necessary: look up closest value in m_colors
	}
}

void CDib::GetPixel24(int x, int y, RGBQUAD &rgb)
{
	// This probably needs more work RFJ !!!!!
	if (m_bmi->biBitCount == 24)
	{
		rgb.rgbBlue = ((char *)m_data)[y * m_stride + (x * 3)];
		rgb.rgbGreen = ((char *)m_data)[y * m_stride + (x * 3) + 1];
		rgb.rgbRed = ((char *)m_data)[y * m_stride + (x * 3) + 2];
	}
	else if (m_bmi->biBitCount == 8)
	{
		byte i = ((char *)m_data)[y * m_stride + x];
		rgb = m_colors[i];
	}
}

void CDib::SetPixel8(int x, int y, byte val)
{
	((char *)m_data)[y * m_stride + x] = val;
}

byte CDib::GetPixel8(int x, int y)
{
	return ((char *)m_data)[y * m_stride + x];
}

///////////////////////////////////////////

CDib *CreateMonoDib(CDC *pDC, CDib *pDib, HDRAWDIB hdd, bool progress_callback(int))
{
	pDib->GetDIBFromSection();

	CSize size = pDib->GetSize();
	CDib *pNew = new CDib;
	pNew->Setup(pDC, size.cx, size.cy, 8, hdd);

	BITMAPINFOHEADER *header = pNew->GetDIBHeader();
	RGBQUAD* colors = (RGBQUAD*) (header + 1);
	int i;
	for (i = 0; i < 256; i++)
	{
		colors[i].rgbRed = i;
		colors[i].rgbGreen = i;
		colors[i].rgbBlue = i;
		colors[i].rgbReserved = 0;
	}

	int x, y;
	RGBQUAD rgb;
	byte color8;

	for (y = 0; y < size.cy; y++)
	{
		if (progress_callback != NULL)
			progress_callback(y * 100 / size.cy);
		for (x = 0; x < size.cx; x++)
		{
			pDib->GetPixel24(x, y, rgb);
			 //if color is black, white, or gray, don't change it
			if (rgb.rgbRed == rgb.rgbGreen && rgb.rgbGreen == rgb.rgbBlue)
				color8 = rgb.rgbBlue;
			else
				color8 = 0xff;
			pNew->SetPixel8(x, y, color8);
		}
	}
	return pNew;
}


