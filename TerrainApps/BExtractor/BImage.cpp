//
// BImage.cpp : implementation of the CBImage class
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "StdAfx.h"
#include "ogr_spatialref.h"
#include "gdal_priv.h"
#include "BImage.h"
#include "Dib.h"
#include "BExtractor.h"
#include "BExtractorDoc.h"
#include "BExtractorView.h"
#include "ProgDlg.h"
#include "vtdata/vtLog.h"

/////////////////////////////////////////////////////////////////////////////
// CBImage commands

CBImage::CBImage()
{
	m_pSpatialReference = NULL;

	m_pSourceDIB = NULL;
	m_pMonoDIB = NULL;
	m_pCurrentDIB = NULL;
}

CBImage::~CBImage()
{
	if (m_pSourceDIB)
	{
		delete m_pSourceDIB;
		m_pSourceDIB = NULL;
	}
	if (m_pMonoDIB)
	{
		delete m_pMonoDIB;
		m_pMonoDIB = NULL;
	}

	if (m_pSpatialReference)
		delete m_pSpatialReference;
	m_pSpatialReference = NULL;
}

CProgressDlg *g_dlg = NULL;
bool progress_callback(int pos)
{
	if (g_dlg)
		g_dlg->SetPos(pos);
	return false;
}

bool CBImage::LoadGDAL(const char *szPathName, CDC *pDC, HDRAWDIB hdd)
{
	VTLOG("  LoadGDAL\n");
	GDALDataset *pDataset = NULL;
	OGRErr err;
	const char *pProjectionString;
	double affineTransform[6];
	OGRSpatialReference SpatialReference;
	double linearConversionFactor;
	GDALRasterBand *pBand;
	int i;
	int iRasterCount;
	bool bRet = true;

	CProgressDlg progImageLoad(CG_IDS_PROGRESS_CAPTION3);

	progImageLoad.Create(NULL);
	progImageLoad.SetStep(1);
	progImageLoad.SetRange(0, 100);
	g_dlg = &progImageLoad;

	g_GDALWrapper.RequestGDALFormats();

	pDataset = (GDALDataset *) GDALOpen( szPathName, GA_ReadOnly );
	if(pDataset == NULL )
	{
		// failed.
		VTLOG("  No Dataset\n");
		bRet = false;
		goto Exit;
	}

	m_PixelSize.x = pDataset->GetRasterXSize();
	m_PixelSize.y = pDataset->GetRasterYSize();
	VTLOG("  Size: %d x %d pixels\n", m_PixelSize.x, m_PixelSize.y);

	// compute size of image in meters
	// try for an affine transform
	// (Xp,Yp) from (P, L);
	// Xp = padfTransform[0] + P*padfTransform[1] + L*padfTransform[2];
	// Yp = padfTransform[3] + P*padfTransform[4] + L*padfTransform[5];
	// In a north up image, padfTransform[1] is the pixel width,
	// and padfTransform[5] is the pixel height.
	// The upper left corner of the upper left pixel is at position (padfTransform[0],padfTransform[3]).
	// Hope for a linear co-ordinate space
	//
	if (NULL == (pProjectionString = pDataset->GetProjectionRef()))
	{
		VTLOG("  No Projection at all.\n");
		bRet = false;
		goto Exit;
	}
	VTLOG("  Got Projection: '%s'\n", pProjectionString);

	err = SpatialReference.importFromWkt((char**)&pProjectionString);
	if (err != OGRERR_NONE)
	{
		VTLOG("  No valid projection, but proceeding anyway..\n");
		SpatialReference.Clear();
		/* bRet = false;
		goto Exit; */
		// allow images without projection?
	}

	CPLErr cerr;
	cerr = pDataset->GetGeoTransform(affineTransform);
	if (cerr != CE_None)
	{
		VTLOG("  No GeoTransform: error %d\n", cerr);
		bRet = false;
		goto Exit;
	}

	if (SpatialReference.IsGeographic())
	{
		// More work needed...
		// Try to convert to projected
		OGRCoordinateTransformation *pCoordTransform;
		OGRSpatialReference NewSpatialReference;
		double Lat, Long;
		double TLGeoX, TLGeoY, BRGeoX, BRGeoY;

		NewSpatialReference.SetWellKnownGeogCS("WGS84");

		// Get GEO coords of centre of image
		Long = affineTransform[0] + affineTransform[1] * m_PixelSize.x / 2 + affineTransform[2] * m_PixelSize.y / 2;
		Lat = affineTransform[3] + affineTransform[4] * m_PixelSize.x / 2 + affineTransform[5] * m_PixelSize.y / 2;

		// Set UTM projection
		NewSpatialReference.SetUTM((int)(Long + 180)/6 + 1, Lat > 0 ? 1 : 0);

		// Convert top left and bottom right to UTM
		if (NULL == (pCoordTransform = OGRCreateCoordinateTransformation(&SpatialReference, &NewSpatialReference)))
		{
			VTLOG("  Can't create coordinate transform\n");
			bRet = false;
			goto Exit;
		}
		// Calculate image size in metres
		TLGeoX = affineTransform[0];
		TLGeoY = affineTransform[3];
		BRGeoX = affineTransform[0] + affineTransform[1] * m_PixelSize.x + affineTransform[2] * m_PixelSize.y;
		BRGeoY = affineTransform[3] + affineTransform[4] * m_PixelSize.x + affineTransform[5] * m_PixelSize.y;
		if (!pCoordTransform->Transform(1, &TLGeoX, &TLGeoY))
		{
			bRet = false;
			goto Exit;
		}
		if (!pCoordTransform->Transform(1, &BRGeoX, &BRGeoY))
		{
			bRet = false;
			goto Exit;
		}
		m_fImageWidth = BRGeoX - TLGeoX;
		m_fImageHeight = TLGeoY - BRGeoY;
		m_xUTMoffset = TLGeoX;
		m_yUTMoffset = TLGeoY;
		if (NULL == (m_pSpatialReference = NewSpatialReference.Clone()))
		{
			bRet = false;
			goto Exit;
		}
	}
	else
	{
		double dBottom, dTop, dLeft, dRight;
		// I have not fully checked if there is any dependency on these units
		// being metres elsewhere
		linearConversionFactor = SpatialReference.GetLinearUnits();

		// Compute sizes in metres along NW/SE axis for compatibility with
		// world files, i.e. xright - xleft and ytop - ybottom
		dLeft = affineTransform[0];
		dTop = affineTransform[3];
		dRight = affineTransform[0] + affineTransform[1] * m_PixelSize.x + affineTransform[2] * m_PixelSize.y;
		dBottom = affineTransform[3] + affineTransform[4] * m_PixelSize.x + affineTransform[5] * m_PixelSize.y;
		m_fImageWidth = (dRight - dLeft) * linearConversionFactor;
		m_fImageHeight = (dTop - dBottom) * linearConversionFactor;

		// This is the upper left corner
		m_xUTMoffset = affineTransform[0] * linearConversionFactor;
		m_yUTMoffset = affineTransform[3] * linearConversionFactor;

		if (NULL == (m_pSpatialReference = SpatialReference.Clone()))
		{
			bRet = false;
			goto Exit;
		}
	}

	m_xMetersPerPixel = m_fImageWidth / m_PixelSize.x;
	m_yMetersPerPixel = - m_fImageHeight / m_PixelSize.y;

	// Raster count should be 3 for colour images (assume RGB)
	iRasterCount = pDataset->GetRasterCount();
	VTLOG("  Rasters: %d\n", iRasterCount);
	if (iRasterCount == 1)
	{
		pBand = pDataset->GetRasterBand(1);
		// Check data type - it's either integer or float
		if (GDT_Byte != pBand->GetRasterDataType())
		{
			VTLOG("  RasterDataType is not Byte\n");
			bRet = false;
			goto Exit;
		}
		if (GCI_PaletteIndex != pBand->GetColorInterpretation())
		{
			VTLOG("  ColorInterpretation is not PaletteIndex\n");
			bRet = false;
			goto Exit;
		}
	}
	else if (iRasterCount == 3)
	{
		bool bSupported = true;
		for (i = 1; i <= 3; i++)
		{
			pBand = pDataset->GetRasterBand(i);
			GDALDataType type = pBand->GetRasterDataType();
			GDALColorInterp interp = pBand->GetColorInterpretation();

			VTLOG(" Band %d: type %d, interpretation %d\n",
				i, type, interp);

			// Check data type - it's either integer or float
			if (GDT_Byte != type)
			{
				VTLOG("  RasterDataType is not Byte\n");
				bRet = false;
				goto Exit;
			}
			// I assume that the bands are in order RGB
			// I know "could do better"... but
			switch(i)
			{
			case 1:
				if (GCI_RedBand != interp)
					bSupported = false;
				break;
			case 2:
				if (GCI_GreenBand != interp)
					bSupported = false;
				break;
			case 3:
				if (GCI_BlueBand != interp)
					bSupported = false;
				break;
			}
		}
		if (!bSupported)
		{
			VTLOG(" Don't support bands of those types in that order.\n");
			bRet = false;
			goto Exit;
		}
	}
	else
	{
		VTLOG("  Don't know what to do with that number of Rasters\n");
		bRet = false;
		goto Exit;
	}

	VTLOG("  Creating CDib\n");
	m_pSourceDIB = new CDib;
	m_pSourceDIB->Setup(pDC, pDataset, hdd, progress_callback);

	// create monochrome version
	VTLOG("  Creating MonoDib\n");
	m_pMonoDIB = CreateMonoDib(pDC, m_pSourceDIB, hdd, progress_callback);
	m_pCurrentDIB = m_pMonoDIB;
	m_initialized = true;

	progImageLoad.DestroyWindow();

Exit:
	if (pDataset != NULL)
	{
		GDALClose(pDataset);
	}
	if (bRet == false)
	{
		// Tidy up
		if (NULL != m_pSpatialReference)
		{
			delete m_pSpatialReference;
			m_pSpatialReference = NULL;
		}
	}
	return bRet;
}

bool CBImage::LoadTFW(const char *szPathName)
{
	char worldfile[100];
	strcpy(worldfile, szPathName);
	char *pPtr = strrchr(worldfile, '.');

	if (!pPtr)
		return false;


	// RFJ !!!!!!!
	if (0 == stricmp(pPtr, ".tif"))
		strcpy(pPtr, ".tfw");
	else if (0 == stricmp(pPtr, ".bmp"))
		strcpy(pPtr, ".bpw");
	else
		return false;

	FILE* tfwstream;
	float dummy;
	if ( (tfwstream = fopen(worldfile, "r")) != NULL)
	{
		fscanf(tfwstream, "%f\n", &m_xMetersPerPixel);
		fscanf(tfwstream, "%f\n", &dummy); //don't want these two
		fscanf(tfwstream, "%f\n", &dummy);
		fscanf(tfwstream, "%f\n", &m_yMetersPerPixel);
		fscanf(tfwstream, "%f\n", &m_xUTMoffset);
		fscanf(tfwstream, "%f\n", &m_yUTMoffset);
		fclose(tfwstream);
	}
	else
	{
		CString msg;
		msg.Format("Couldn't find corresponding world file '%s'", worldfile);
		AfxMessageBox(msg);
		return false;
	}
	return true;
}

bool CBImage::LoadFromFile(const char *szPathName, CDC *pDC, HDRAWDIB hdd)
{
	// Try to load via the GDAL library first
	if (!LoadGDAL(szPathName, pDC, hdd))
		return false;

//	if (!LoadTFW(szPathName))
//		return false;

	return true;
}

