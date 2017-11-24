//
// MainFrm.cpp : implementation of the CMainFrame class
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "stdafx.h"
#include "BExtractor.h"

#include "MainFrm.h"
#include "BExtractorView.h"
#include "BExtractorDoc.h"
#include "vtdata/vtLog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,	// status line indicator

	ID_COORDSYS,	// 1
	ID_ZONE,		// 2
	ID_DATUM,		// 3
	ID_UNITS,		// 4
	ID_CURSOR,		// 5

	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// helper
vtString FormatCoord(bool bGeo, double val, bool minsec = false)
{
	vtString str;
	if (bGeo)
	{
		if (minsec)
		{
			// show minutes and seconds
			double degree = val;
			double min = (degree - (int)degree) * 60.0f;
			double sec = (min - (int)min) * 60.0f;

			str.Format("%d° %d' %.1f\"", (int)degree, (int)min, sec);
		}
		else
			str.Format("%3.6lf", val);	// decimal degrees
	}
	else
		str.Format("%.2lf", val);	// meters-based
	return str;
}


/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	VTLOG("constructing CMainFrame\n");
}

CMainFrame::~CMainFrame()
{
	VTLOG("destructing CMainFrame\n");
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		VTLOG("Failed to create toolbar\n");
		return -1;	// fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		VTLOG("Failed to create status bar\n");
		return -1;	// fail to create
	}

	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFrameWnd::PreCreateWindow(cs);
}

void CMainFrame::RefreshStatusBar(BExtractorView *pView)
{
	BExtractorDoc *pDoc = pView->GetDocument();
	const vtProjection &proj = pDoc->GetProjection();
	bool bGeo = (proj.IsGeographic() != 0);
	vtString str;

	// Coordinate system
	str = proj.GetProjectionNameShort();
	m_wndStatusBar.SetPaneText(1, str);

	// Zone
	int zone = proj.GetUTMZone();
	if (zone != 0)
		str.Format("Zone %d", zone);
	else
		str = "";
	m_wndStatusBar.SetPaneText(2, str);

	// Datum
	str = DatumToStringShort(proj.GetDatum());
	m_wndStatusBar.SetPaneText(3, str);

	// Units
	LinearUnits lu = proj.GetUnits();
	str = GetLinearUnitName(lu);
	m_wndStatusBar.SetPaneText(4, str);

	// Mouse location
	DPoint2 p = pView->GetCurLocation();
	str = "Mouse: ";
	str += FormatCoord(bGeo, p.x);
	str += ", ";
	str += FormatCoord(bGeo, p.y);

	m_wndStatusBar.SetPaneText(5, str);
}


/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers



