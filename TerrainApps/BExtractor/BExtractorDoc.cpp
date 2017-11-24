//
// BExtractorDoc.cpp : implementation of the BExtractorDoc class
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "StdAfx.h"
#include "BExtractor.h"
#include "BExtractorDoc.h"
#include "BExtractorView.h"
#include "ProjectionDlg.h"
#include "gdal_priv.h"
#include "BImage.h"
#include "ProgDlg.h"
#include "Dib.h"
#include "xmlhelper/easyxml.hpp"
#include "vtdata/vtLog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////
// Helper

void ShowErrorMessage(int error)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				  FORMAT_MESSAGE_FROM_SYSTEM |
				  FORMAT_MESSAGE_IGNORE_INSERTS,
				  NULL,
				  error,
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				  (LPTSTR) &lpMsgBuf,	0,	NULL );
	MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
	LocalFree( lpMsgBuf );
}

/////////////////////////////////////////////////////////////////////////////
// BExtractorDoc

IMPLEMENT_DYNCREATE(BExtractorDoc, CDocument)

BEGIN_MESSAGE_MAP(BExtractorDoc, CDocument)
	//{{AFX_MSG_MAP(BExtractorDoc)
	ON_COMMAND(ID_IMPORTIMAGE, OnImportimage)
	ON_COMMAND(ID_FUNCTIONS_CLEARBUILDINGS, OnFunctionsClearbuildings)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, OnUpdateFileOpen)
	ON_UPDATE_COMMAND_UI(ID_FULLRES, OnUpdateFullres)
	ON_COMMAND(ID_RMF_OPEN, OnRmfOpen)
	ON_COMMAND(ID_RMF_SAVE, OnRmfSave)
	ON_COMMAND(ID_RMF_SAVE_AS, OnRmfSaveAs)
	ON_UPDATE_COMMAND_UI(ID_RMF_OPEN, OnUpdateRmfOpen)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// BExtractorDoc construction/destruction

BExtractorDoc::BExtractorDoc()
{
	m_pImage = NULL;
	m_picLoaded = false;

	m_hdd = DrawDibOpen();

	SetupEPSGDatums();

	m_proj.SetWellKnownGeogCS( "WGS84" );
	m_proj.SetUTMZone(1);
}

BExtractorDoc::~BExtractorDoc()
{
	if (m_hdd)
	{
		DrawDibClose(m_hdd);
		m_hdd = NULL;
	}
	DeleteContents();
}

BOOL BExtractorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

void BExtractorDoc::DeleteContents()
{
	if (m_pImage)
	{
		delete m_pImage;
		m_pImage = NULL;
	}
	m_Buildings.Empty();
	m_Links.DeleteElements();
}

/////////////////////////////////////////////////////////////////////////////
// BExtractorDoc serialization

void BExtractorDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{

	}
}

/////////////////////////////////////////////////////////////////////////////
// BExtractorDoc diagnostics

#ifdef _DEBUG
void BExtractorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void BExtractorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

///////////////////////////////////////////////////////////////
// BExtractorDoc commands
void BExtractorDoc::PreFloodFillDIB(CDib *bm)
{
	int width = bm->GetWidth();
	int height = bm->GetHeight();
	byte target = 0x00;
	int result = 0;
	int result2 = 0;

	CProgressDlg prog(CGS_KERNEL);
	prog.Create(NULL);	// top level
	prog.SetPos(0);
	for (int i = 0; i < width; i++)
	{
		prog.SetPos(i*200/width);
		for (int j = 0; j < height; j++)
		{
			if (bm->GetPixel8(i,j) == target)
			{
				//0x22=arbitrary value
				result = Fill(bm, i, j, 0x00, 0x22, width, height, 0);

				// We may have something too big to be a building.
				// The reason this # had to be 80 and not something
				// slightly larger than building size (~30) is because sometimes
				// the building pixels bleed into some thin lines made up
				// of black pixels that get added to the overall count.
				// So to avoid losing these buildings (or buildings whose
				// black pixels bleed into each other), the number is
				// bigger.

				// This is also why the Fill method takes the diag
				// parameter, it tells it whether include pixels diagonal
				// to each other in the flood fill count (lines tend to
				// be diagonal)
				if (result > 80)
				{
					result2 = Fill(bm, i, j, 0x22, 0xff, width, height, 0); //get rid of it
				}
				else
				{
					//put the pixels back to what they were
					Fill(bm, i, j, 0x22, 0x00, width, height, 0);
				}
			}
		}
	}
}

void BExtractorDoc::FloodFillDIB(CDib *bm, CProgressDlg *prog)
{
	int width = bm->GetWidth();
	int height = bm->GetHeight();
	byte target = 0x00;
	int result = 0;

	for (int i = 0; i < width; i++)
	{
		if ((i % 10) == 0)
			prog->SetPos(i * 200 / width);
		for (int j = 0; j < height; j++)
		{
			if (bm->GetPixel8(i,j) == target)
			{
				result = Fill(bm, i, j, 0x00, 0xff, width, height, 1);
				DPoint2 point;
				point.x = i_UTMx(i);
				point.y = i_UTMy(j);

				// rough heuristic: adjusts the coordinate more towards the
				//center of the original building
				point.x += 5;

				vtBuilding *bld = new vtBuilding;
				bld->SetRectangle(point, 10, 10);
				bld->SetStories(1);
				bld->GetLevel(1)->SetEdgeColor(RGBi(255,255,255));	// white roof
				m_Buildings.Append(bld);
			}
		}
	}
}


//
// old_v = initial pixel value that you are looking to fill region of
// new_v = new value to fill area with
//
int BExtractorDoc::Fill(CDib *bm, int x, int y, byte old_v, byte new_v,
						  int w, int h, int diag, int count)
{
	byte val = bm->GetPixel8(x, y);
	int size = 0;

	// beware stack overflow - 1000 is a typically safe amount of recursion
	if (count > 1000)
		return 1;

	if (val == old_v)
	{
		bm->SetPixel8(x, y, new_v);
		size++;
		if (x+1 != w) size += Fill(bm, x+1, y, old_v, new_v, w, h, diag, count+1);
		if (x-1 >= 0) size += Fill(bm, x-1, y, old_v, new_v, w, h, diag, count+1);
		if (y+1 != h) size += Fill(bm, x, y+1, old_v, new_v, w, h, diag, count+1);
		if (y-1 >= 0)size += Fill(bm, x, y-1, old_v, new_v, w, h, diag, count+1);
		if ( (x+1 != w)&&(y+1 != h)&&(diag == 1) )
			size += Fill(bm, x+1, y+1, old_v, new_v, w, h, diag, count+1);
		if ( (x+1 != w)&&(y-1 >= 0)&&(diag == 1) )
			size += Fill(bm, x+1, y-1, old_v, new_v, w, h, diag, count+1);
		if ( (x-1 >= 0)&&(y+1 != h)&&(diag == 1) )
			size += Fill(bm, x-1, y+1, old_v, new_v, w, h, diag, count+1);
		if ( (x-1 >= 0)&&(y-1 >= 0)&&(diag == 1) )
			size += Fill(bm, x-1, y-1, old_v, new_v, w, h, diag, count+1);
	}
	return size;
}

double BExtractorDoc::i_UTMx(int ix)
{
	return (ix * m_pImage->m_xMetersPerPixel + m_pImage->m_xUTMoffset);
}

double BExtractorDoc::i_UTMy(int iy)
{
	return ((m_pImage->m_PixelSize.y - 1 - iy) * m_pImage->m_yMetersPerPixel
		+ m_pImage->m_yUTMoffset);
}

BOOL BExtractorDoc::OnOpenDocument(LPCTSTR szPathName)
{
	VTLOG("OnOpenDocument %s\n", szPathName);
	CString name = szPathName;
	CString ext = name.Right(4);

	bool success = false;
	if (!ext.CompareNoCase(".tif"))
	{
		OnImportimage2(szPathName);
		SetPathName(szPathName, true);
		SetPathName("Untitled", false);
		return false;
	}
	else if (!ext.CompareNoCase(".bmp") || !ext.CompareNoCase(".jpg"))
	{
		OnImportimage2(szPathName);
		SetPathName(szPathName, true);
		SetPathName("Untitled", false);
		return false;
	}
	else if (!ext.CompareNoCase(".bcf"))
	{
		if (m_picLoaded == false)
		{
			AfxMessageBox("Sorry, you cannot load a .BCF file "
				"without having an image loaded!");
			return false;
		}

		// read in the bcf.
		success = m_Buildings.ReadBCF(szPathName);
		if (success)
		{
			if (m_Buildings.m_proj.GetUTMZone() == -1)
			{
				// loading file with unknown zone, assume the current
				m_Buildings.m_proj.SetUTMZone(m_proj.GetUTMZone());
			}
		}
	}
	else if (!ext.CompareNoCase("vtst"))
	{
		try
		{
			success = m_Buildings.ReadXML(szPathName);
		}
		catch (xh_io_exception &e)
		{
			string str = "Problem opening file: ";
			str += e.getFormattedMessage();
			AfxMessageBox(str.c_str());
		}
	}
	else
	{
		CString str;
		str.Format("Sorry, don't know how to load the file:\n%s", szPathName);
		AfxMessageBox(str);
	}
	if (success)
	{
		GetView()->Invalidate();
	}
	return success;
}


BOOL BExtractorDoc::OnSaveDocument(LPCTSTR szPathName)
{
	VTLOG("OnSaveDocument(%s)\n", szPathName);
	return m_Buildings.WriteXML(szPathName);
}


//
// TIF stuff
//
void BExtractorDoc::OnImportimage()
{
	CFileDialog openDialog(TRUE, "tif", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Image Files (*.bmp, *.tif, *.jpg)|*.bmp;*.tif;*.jpg||");

	if (openDialog.DoModal() != IDOK)
		return;

	CString str = openDialog.GetPathName();
	const char *szPathName = str;

	if (m_pImage) delete m_pImage;

	OnImportimage2(szPathName);
}

void BExtractorDoc::OnImportimage2(LPCTSTR szPathName)
{
	VTLOG("OnImportimage2 %s\n", szPathName);
	m_picLoaded = false;
	m_pImage = new CBImage;

	if (1)
	{
		CProgressDlg prog(CG_IDS_PROGRESS_CAPTION3);
		prog.Create(NULL);	// top level
		CDC *pDC = GetView()->GetDC();
		bool success = m_pImage->LoadFromFile(szPathName, pDC, m_hdd);
		GetView()->ReleaseDC(pDC);
		if (!success)
		{
			VTLOG("LoadFromFile failed\n");
			return;
		}
	}

	// we don't know projection unless we have read a geotiff
	OGRSpatialReference *srs = m_pImage->m_pSpatialReference;
	if (srs == NULL || (srs != NULL && srs->GetRoot() == NULL))
	{
		VTLOG("no SRS, opening Projection dialog.\n");
		// Assume that I have loaded from a world file
		// and need some more info
		// get utm zone from a dialog
		CProjectionDlg dlg;
		dlg.m_iZone = 1;
		if (dlg.DoModal() != IDOK)
			return;

		m_proj.SetWellKnownGeogCS( "WGS84" );
		switch (dlg.m_iProjection)
		{
		case 0: // UTM
			m_proj.SetUTMZone(dlg.m_iZone);
			m_Buildings.m_proj.SetUTMZone(dlg.m_iZone);
			m_Links.GetProjection().SetUTMZone(dlg.m_iZone);
			break;
		case 1: // OSGB
			m_proj.SetProjectionSimple(false, -1, 6277);	// OSGB 1936
			m_Buildings.m_proj.SetProjectionSimple(false, -1, 6277);
			m_Links.GetProjection().SetProjectionSimple(false, -1, 6277);
			break;
		default: // default to UTM
			m_proj.SetUTMZone(dlg.m_iZone);
			m_Buildings.m_proj.SetUTMZone(dlg.m_iZone);
			m_Links.GetProjection().SetUTMZone(dlg.m_iZone);
			break;
		}
	}
	else
	{
		// Loaded from a GDAL dataset (Geotiff etc.)
		// I should have a valid projection
		m_proj.SetSpatialReference(m_pImage->m_pSpatialReference);
		m_Buildings.m_proj.SetSpatialReference(m_pImage->m_pSpatialReference);
		m_Links.GetProjection().SetSpatialReference(m_pImage->m_pSpatialReference);
	}

	// is image >50 million pixels?
	if (m_pImage->m_PixelSize.x * m_pImage->m_PixelSize.y > 45000000)
	{
		CString str;
		str.Format("Warning!  That image is really large (%d * %d)\n"
			"You can save memory by disabling color display.\n"
			"Do you want to do this?",
			m_pImage->m_PixelSize.x, m_pImage->m_PixelSize.y);

		int result = AfxMessageBox(str, MB_YESNO);
		if (result == IDYES)
		{
			if (NULL != m_pImage->m_pSourceDIB)
			{
				delete m_pImage->m_pSourceDIB;
				m_pImage->m_pSourceDIB = NULL;
			}
		}
	}
	m_picLoaded = true;

	// Tell the view to zoom to the freshly loaded bitmap
	m_Buildings.Empty(); //clear out any old buildings we have lying around
	m_Links.DeleteElements();
	GetView()->ZoomToImage(m_pImage);
	VTLOG("OnImportimage2 finished\n");
}


void BExtractorDoc::OnFunctionsClearbuildings()
{
	m_Buildings.Empty();
	GetView()->Invalidate();
}


void BExtractorDoc::OnUpdateFileOpen(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_picLoaded);
}

void BExtractorDoc::OnUpdateFullres(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_picLoaded);
}

void BExtractorDoc::OnRmfOpen()
{
	CFileDialog openDialog(TRUE, "rmf", NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "RMF Files (*.rmf)|*.rmf||");

	if (openDialog.DoModal() != IDOK)
		return;

	CString str = openDialog.GetPathName();
	const char *szPathName = str;

	if (!m_Links.ReadRMF(szPathName, true, true, true))
	{
		string str = "Problem reading file: ";
		AfxMessageBox(str.c_str());
	}
	else
	{
		m_roadFileName = szPathName;
		GetView()->Invalidate();
	}
}

void BExtractorDoc::OnRmfSave()
{
	if (m_roadFileName.IsEmpty())
		OnRmfSaveAs();
	else
	{
		m_Links.ComputeExtents();
		if (!m_Links.WriteRMF(m_roadFileName))
		{
			string str = "Problem writing file: ";
			AfxMessageBox(str.c_str());
		}
	}
}

void BExtractorDoc::OnRmfSaveAs()
{
	CFileDialog saveAsDialog(FALSE, "rmf", NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "RMF Files (*.rmf)|*.rmf||");

	if (saveAsDialog.DoModal() != IDOK)
		return;

	CString str = saveAsDialog.GetPathName();
	const char *szPathName = str;

	m_Links.ComputeExtents();
	if (!m_Links.WriteRMF(szPathName))
	{
		string str = "Problem writing file: ";
		AfxMessageBox(str.c_str());
	}
	else
		m_roadFileName = szPathName;
}

void BExtractorDoc::OnUpdateRmfOpen(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_picLoaded);
}

BExtractorView *BExtractorDoc::GetView()
{
	POSITION  pos = GetFirstViewPosition();
	return (BExtractorView*)GetNextView(pos);
}

