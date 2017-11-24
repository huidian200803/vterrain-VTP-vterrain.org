//
// BExtractorView.cpp : implementation of the BExtractorView class
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "StdAfx.h"
#include "BExtractor.h"

#include "MainFrm.h"
#include "BExtractorDoc.h"
#include "BExtractorView.h"

#include "FloodDialog.h"
#include "ConvolveDialog.h"
#include "KernelDialog.h"
#include "gdal_priv.h"
#include "BImage.h"
#include "Dib.h"
#include "ProgDlg.h"
#include <fstream>
#include "vtdata/DataPath.h"
#include "vtdata/FilePath.h"
#include "vtdata/vtLog.h"

#if USE_IPL
#include "ipl.h"			// Intel's old Image Processing Library 2.1
#pragma comment(lib, "ipl.lib")
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//NOTE:
// The values below need to be selected very carefully (especially the ones
// expected to be the black pixels of the building) so that the total value
// obtained by the convolve2D function is approximately 255. That way, after
// the 8 bit shift is done, the pixel representing the center of a building
// will be black, all others (words, building edge pixels, etc) will have had
// their values reduced by not matching with n3, n1. Notice that if two
// buildings have less than two white pixels between them, they will also
// have their values reduced and will probably not show up in the final
// building count.
#define n3 -5
#define n1 -1
#define n2 10	//12 black pixels of this type in a perfect building
#define n4 15	// 9 black pixels of this type in a perfect building

#define SCROLL_RANGE	720
#define SCROLL_CENTER	360
#define SCROLL_MULT		50
/////////////////////////////////////////////////////////////////////////////
// BExtractorView

IMPLEMENT_DYNCREATE(BExtractorView, CView)

BEGIN_MESSAGE_MAP(BExtractorView, CView)
	//{{AFX_MSG_MAP(BExtractorView)
	ON_COMMAND(ID_COLORCHANGE, OnColorChange)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_COMMAND(ID_FUNCTIONS_CONVOLVE, OnFunctionsConvolve)
	ON_COMMAND(ID_MODES_ADDREMOVE, OnAddRemove)
	ON_UPDATE_COMMAND_UI(ID_MODES_ADDREMOVE, OnUpdateAddRemove)
	ON_WM_LBUTTONUP()
	ON_COMMAND(ID_FULLRES, OnFullres)
	ON_COMMAND(ID_HAND, OnHand)
	ON_UPDATE_COMMAND_UI(ID_HAND, OnUpdateHand)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_ZOOM_IN, OnZoomIn)
	ON_COMMAND(ID_ZOOM_OUT, OnZoomOut)
	ON_WM_MBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_CLEARSCREEN, OnClearscreenofBuildings)
	ON_COMMAND(ID_UNDO, OnUndo)
	ON_COMMAND(ID_VIEW_VIEWFULLCOLORIMAGE, OnViewViewfullcolorimage)
	ON_UPDATE_COMMAND_UI(ID_VIEW_VIEWFULLCOLORIMAGE, OnUpdateViewViewfullcolorimage)
	ON_COMMAND(ID_MODES_FOOTPRINT, OnModesFootprintMode)
	ON_UPDATE_COMMAND_UI(ID_MODES_FOOTPRINT, OnUpdateModesFootprintmode)
	ON_COMMAND(ID_MODES_RECTANGLE, OnModesRectangle)
	ON_UPDATE_COMMAND_UI(ID_MODES_RECTANGLE, OnUpdateModesRectangle)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_MODES_CIRCLE, OnModesCircle)
	ON_UPDATE_COMMAND_UI(ID_MODES_CIRCLE, OnUpdateModesCircle)
	ON_UPDATE_COMMAND_UI(ID_UNDO, OnUpdateUndo)
	ON_UPDATE_COMMAND_UI(ID_FUNCTIONS_CONVOLVE, OnUpdateFunctionsConvolve)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_COMMAND(ID_MODES_MOVERESIZE, OnModesMoveresize)
	ON_UPDATE_COMMAND_UI(ID_MODES_MOVERESIZE, OnUpdateModesMoveresize)
	ON_COMMAND(ID_MODES_ROADNODE, OnModesRoadnode)
	ON_UPDATE_COMMAND_UI(ID_MODES_ROADNODE, OnUpdateModesRoadnode)
	ON_COMMAND(ID_CHANGE_ROAD_COLOR, OnChangeRoadColor)
	ON_COMMAND(ID_MODES_ROAD_EDIT, OnModesRoadEdit)
	ON_UPDATE_COMMAND_UI(ID_MODES_ROAD_EDIT, OnUpdateModesRoadEdit)
	ON_COMMAND(ID_CONSTRAIN, OnConstrain)
	ON_UPDATE_COMMAND_UI(ID_CONSTRAIN, OnUpdateConstrain)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// BExtractorView construction/destruction
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BExtractorView::BExtractorView()
{
	m_offset.x = m_offset.y = 250;

	// initially 8 meters/pixel
	m_fScale = 8.0f;

	m_mode = LB_AddRemove;	//initially building select mode, not hand mode
	m_bPanning = false;
	m_iStep = 0;

	m_bRubber = false;
	m_maybeRect = false;

	m_zoomed = false;

	m_scrollposH = SCROLL_MULT;
	m_scrollposV = SCROLL_MULT;

	m_bConstrain = true;

	DWORD blah = MAX_PATH;
	GetCurrentDirectory(blah, m_directory);

	if (!ReadINIFile())
	{
		m_buildingColor = 0x0000ffff;
		m_roadColor = RGB(0xff, 0x00, 0x00);
	}

	// Load structure defaults
	vtStringArray paths;
	ReadDataPath();
	LoadGlobalMaterials();
	SetupDefaultStructures("");
}

BExtractorView::~BExtractorView()
{
	FreeGlobalMaterials();
}

BOOL BExtractorView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

//////////////////////////////

void BExtractorView::ReadDataPath()
{
	char buf1[MAX_PATH];
	SHGetFolderPathA(
		NULL,               // parent window, not used
		CSIDL_APPDATA,
		NULL,               // access token (current user)
		SHGFP_TYPE_CURRENT, // current path, not just default value
		buf1
		);
	vtString AppDataUser = buf1;
	//vtString AppDataCommon;

	// Read the vt datapaths
	vtLoadDataPath(AppDataUser, NULL);

	vtStringArray &dp = vtGetDataPath();
	// Supply the special symbols {appdata} and {appdatacommon}
	for (unsigned int i = 0; i < dp.size(); i++)
	{
		dp[i].Replace("{appdata}", AppDataUser);
		//dp[i].Replace("{appdatacommon}", AppDataCommon);
	}
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// BExtractorView drawing
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void BExtractorView::OnDraw(CDC* pDC)
{
	BExtractorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (!pDoc->m_picLoaded)
		return;

	CBImage *pImage = pDoc->m_pImage;

	// now, draw

	// try to do clipping in doubles, not ints, to minimize roundoff errors

	// how big is the bitmap going to be on screen
	CPoint dest_size;
	dest_size.x = (long)(pImage->m_fImageWidth / m_fScale);
	dest_size.y = (long)(pImage->m_fImageHeight / m_fScale);

	CPoint dest_offset;
	dest_offset.x = UTM_sx(pImage->m_xUTMoffset);
	dest_offset.y = UTM_sy(pImage->m_yUTMoffset);

	//get client window size
	CRect r;
	GetClientRect(r);

	//clip stuff, so we only blit what we need
	if ((dest_offset.x + dest_size.x < r.left) ||
		(dest_offset.y + dest_size.y < r.top) ||
		(dest_offset.x > r.right) ||
		(dest_offset.y > r.bottom))
		//image completely off screen, return
		return;

	CRect srcRect(0, 0, pImage->m_PixelSize.x, pImage->m_PixelSize.y);
	double ratio_x, ratio_y;
	ratio_x = (double)pImage->m_PixelSize.x / dest_size.x;
	ratio_y = (double)pImage->m_PixelSize.y / dest_size.y;
	int diff;
	int diff_source;

	// clip left
	diff = r.left - dest_offset.x;
	diff_source = (long)(diff * ratio_x); // round to number of whole pixels
	diff = (int) (diff_source / ratio_x);
	if (diff > 0)
	{
		dest_offset.x += diff;
		dest_size.x -= diff;
		srcRect.left += diff_source;
	}

	// clip top
	diff = r.top - dest_offset.y;
	diff_source = (long)(diff * ratio_y); // round to number of whole pixels
	diff = (int) (diff_source / ratio_y);
	if (diff > 0)
	{
		dest_offset.y += diff;
		dest_size.y -= diff;
		srcRect.top += (long)(diff * ratio_y);
	}

	// clip right
	diff = (dest_offset.x + dest_size.x) - r.right;
	diff_source = (long)(diff * ratio_x); // round to number of whole pixels
	diff = (int) (diff_source / ratio_x);
	if (diff > 0)
	{
		dest_size.x -= diff;
		srcRect.right -= (long)(diff * ratio_x);
	}

	// clip bottom
	diff = (dest_offset.y + dest_size.y) - r.bottom;
	diff_source = (long)(diff * ratio_y); // round to number of whole pixels
	diff = (int) (diff_source / ratio_y);
	if (diff > 0)
	{
		dest_size.y -= diff;
		srcRect.bottom -= (long)(diff * ratio_y);
	}

	CRect destRect(dest_offset.x, dest_offset.y, dest_offset.x + dest_size.x,
					 dest_offset.y + dest_size.y);
	pImage->m_pCurrentDIB->Draw(*pDC, &destRect, &srcRect, TRUE, NULL, FALSE);

	// now buildings
	DrawBuildings(pDC);

	// now roads
	DrawRoadNodes(pDC);
	DrawRoads(pDC);

	// and any building polygons which may be in process
	if (m_poly.GetSize() > 0)
	{
		CPen bgPen( PS_SOLID, 1, RGB(0xff, 0xff, 0xff) ); //pen is black
		pDC->SetROP2(R2_NOT);

		DrawPoly(pDC);
		pDC->LineTo(m_oldPoint.x, m_oldPoint.y);
	}
}

/////////////////////////////////////////////////////////////////////////////
// BExtractorView : Draw Building Marks

void BExtractorView::DrawBuildings(CDC *pDC)
{
	BExtractorDoc* pDoc = GetDocument();

	COLORREF color;
	color = m_buildingColor;
	CPen bgPen( PS_SOLID, 1, color);
	pDC->SelectObject(bgPen);

	DPoint2 temp;
	for (unsigned int i = 0; i < pDoc->m_Buildings.GetSize(); i++)
	{
		// draw each "building"
		// TODO: draw other structure types as well?  That depends on how
		// full-featured we intend BExtractor to be.  If it becomes much
		// more than a simple "building extractor", we'll want the other
		// structure types.
		vtStructure *str = pDoc->m_Buildings.GetAt(i);
		vtBuilding *bld = str->GetBuilding();
		if (bld)
			DrawBuilding(pDC, bld);
	}
}

void BExtractorView::DrawBuilding(CDC *pDC, vtBuilding *bld)
{
	CPoint origin, array[500];
	DPoint2 center;
	bld->GetBaseLevelCenter(center);
	UTM_s(center, origin);

	int size = UTM_sdx(20);
	if (size > BLENGTH) size = BLENGTH;
	if (size < 1) size = 1;

	// always draw crosshair at center of extents
	pDC->MoveTo(origin.x-size, origin.y);
	pDC->LineTo(origin.x+size+1, origin.y);
	pDC->MoveTo(origin.x, origin.y-size);
	pDC->LineTo(origin.x, origin.y+size+1);

	const DPolygon2 &foot = bld->GetFootprint(0);
	for (unsigned int r = 0; r < foot.size(); r++)
	{
		const DLine2 &line = foot[r];

		unsigned int j;
		for (j = 0; j < line.GetSize() && j < 500-1; j++)
			UTM_s(line[j], array[j]);
		array[j] = array[0];

		pDC->Polyline(array, j+1);
	}
}

/////////////////////////////////////////////////////////////////////////////
// BExtractorView : Draw Road Marks

void BExtractorView::DrawRoadNodes(CDC *pDC)
{

	BExtractorDoc* pDoc = GetDocument();
	TNode *pNode;

	COLORREF color;
	color = m_roadColor;
	CPen bgPen( PS_SOLID, 1, color);
	pDC->SelectObject(bgPen);

	for (pNode = pDoc->m_Links.GetFirstNode(); NULL != pNode; pNode = pNode->m_pNext)
	{
		DrawRoadNode(pDC, pNode);
	}
}

void BExtractorView::DrawRoadNode(CDC *pDC, TNode *pNode)
{
	CPoint origin;

	UTM_s(pNode->m_p, origin);
	pDC->MoveTo(origin.x-BLENGTH, origin.y);
	pDC->LineTo(origin.x+BLENGTH+1, origin.y);
	pDC->MoveTo(origin.x, origin.y-BLENGTH);
	pDC->LineTo(origin.x, origin.y+BLENGTH+1);
}

void BExtractorView::DrawRoads(CDC *pDC)
{
	BExtractorDoc* pDoc = GetDocument();
	TLink *pLink;

	COLORREF color;
	color = m_roadColor;
	CPen bgPen( PS_SOLID, 1, color);
	pDC->SelectObject(bgPen);

	for (pLink = pDoc->m_Links.GetFirstLink(); NULL != pLink; pLink = pLink->m_pNext)
	{
		DrawRoad(pDC, pLink);
	}
}

void BExtractorView::DrawRoad(CDC *pDC, TLink *pLink)
{
	CPoint point;
	int i, size;
	TNode *pNode;

	if (NULL != (pNode = pLink->GetNode(0)))
	{
		UTM_s(pNode->m_p, point);
		pDC->MoveTo(point);
		size = pLink->GetSize();
		for (i = 0; i < size; i++)
		{
			UTM_s(pLink->GetAt(i), point);
			pDC->LineTo(point);
		}
		if (NULL != (pNode = pLink->GetNode(1)))
		{
			UTM_s(pNode->m_p, point);
			pDC->LineTo(point);
		}
	}
}

bool BExtractorView::FindNearestRoadNode(CPoint &point, TNode **pNearestNode)
{
	BExtractorDoc* pDoc = GetDocument();
	DPoint2 geoPoint;

	s_UTM(point, geoPoint);

	*pNearestNode = pDoc->m_Links.FindNodeAtPoint(geoPoint, NODE_CAPTURE_THRESHOLD);
	return (*pNearestNode != NULL);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// BExtractorView diagnostics
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void BExtractorView::AssertValid() const
{
	CView::AssertValid();
}

void BExtractorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

BExtractorDoc* BExtractorView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(BExtractorDoc)));
	return (BExtractorDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// BExtractorView scroll, mouse handlers
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void BExtractorView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
}

void BExtractorView::ContrainLocationForPoly()
{
	int points = m_poly.GetSize();
	DPoint2 p1 = m_poly[points-1];
	DPoint2 p0 = m_poly[points-2];
	DPoint2 vec = p1 - p0;	// Vector along last edge

	vec.Normalize();
	DPoint2 vec2 = m_curLocation - p0;	// Vector to new point

	double a = vec2.Dot(vec);	// Distance along edge
	DPoint2 result = p0 + (vec * a);
	m_poly[points-1] = result;

	vec.Rotate(PID2d);
	double b = vec2.Dot(vec);	// Distance perpendicular to edge
	result += (vec * b);

	m_curLocation = result;
}

// keep the view offset within bounds
void BExtractorView::ClipOffset()
{
	if (m_offset.x < m_offset_base.x)
		m_offset.x = m_offset_base.x;
	if (m_offset.x > (m_offset_base.x + m_offset_size.x))
		m_offset.x = (m_offset_base.x + m_offset_size.x);

	if (m_offset.y < m_offset_base.y)
		m_offset.y = m_offset_base.y;
	if (m_offset.y > (m_offset_base.y + m_offset_size.y))
		m_offset.y = (m_offset_base.y + m_offset_size.y);
}

//
// determine the scrollbar positions from the display offset
//
void BExtractorView::UpdateScrollPos()
{
	m_scrollposH = m_offset_size.x - (m_offset.x - m_offset_base.x);
	SetScrollPos(SB_HORZ, m_scrollposH);

	m_scrollposV = m_offset_size.y - (m_offset.y - m_offset_base.y);
	SetScrollPos(SB_VERT, m_scrollposV);
}

void BExtractorView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CRect r;
	GetClientRect(r);
	int window_width = r.right - r.left;

	int delta = 0;
	switch (nSBCode)
	{
		case SB_LINELEFT:
			delta = -SCROLL_MULT;
			break;
		case SB_LINERIGHT:
			delta = SCROLL_MULT;
			break;
		case SB_PAGELEFT:
			delta = -window_width;
			break;
		case SB_PAGERIGHT:
			delta = window_width;
			break;
		case SB_THUMBPOSITION:
			break;
		case SB_THUMBTRACK:
			delta = nPos - m_scrollposH;
			break;
		case SB_LEFT:
			break;
		case SB_RIGHT:
			break;
		case SB_ENDSCROLL:
			break;
	}
	int last_offset = m_offset.x;
	m_offset.x -= delta;

	ClipOffset();	// keep within bounds
	ScrollWindow(m_offset.x - last_offset, 0);
	UpdateScrollPos();

	// Update any stored screen positions
	if (LB_EditRoad == m_mode)
	{
		m_p0.x -= delta;
		m_p1.x -= delta;
	}
}

void BExtractorView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CRect r;
	GetClientRect(r);
	int screen_height = r.bottom - r.top;

	int delta = 0;
	switch (nSBCode)
	{
		case SB_LINEUP:
			delta = -SCROLL_MULT;
			break;
		case SB_LINEDOWN:
			delta = SCROLL_MULT;
			break;
		case SB_PAGEUP:
			delta = -screen_height;
			break;
		case SB_PAGEDOWN:
			delta = screen_height;
			break;
		case SB_THUMBPOSITION:
			break;
		case SB_THUMBTRACK:
			delta = nPos - m_scrollposV;
			break;
		case SB_TOP:
			break;
		case SB_BOTTOM:
			break;
		case SB_ENDSCROLL:
			break;
	}
	int last_offset = m_offset.y;
	m_offset.y -= delta;
	ClipOffset();	// keep within bounds
	ScrollWindow(0, m_offset.y - last_offset);
	UpdateScrollPos();

	// Update any stored screen positions
	if (LB_EditRoad == m_mode)
	{
		m_p0.y -= delta;
		m_p1.y -= delta;
	}
}

void BExtractorView::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_downPoint = point;	//save the point where they started
	s_UTM(m_downPoint, m_downLocation);

	switch (m_mode)
	{
	case LB_Hand:
		SetCapture();			//capture mouse
		m_bPanning = true;
		m_old_offset = m_offset;	//save the old offset
		break;

	case LB_AddRemove:
		m_maybeRect = true;			//maybe they are mopping?
		SetCapture();
		break;

	case LB_Rectangle:
		if (m_iStep == 0)
		{
			m_bRubber = true;
			m_p0 = m_p1 = point;
			m_iStep = 1;
		}
		else if (m_iStep == 1)
		{
			CDC *pDC = GetInvertDC();

			DrawRectangle(pDC);
			m_p1 = point;
			m_iStep = 2;
			UpdateRectangle(point);
			DrawRectangle(pDC);

			ReleaseDC(pDC);
		}
		break;

	case LB_Circle:
		m_bRubber = true;
		m_fPixelRadius = 0.0f;
		break;

	case LB_EditShape:
		OnLButtonDownEditShape(nFlags, point);
		break;

	case LB_EditRoadNodes:
		m_maybeRect = true;			//maybe they are mopping?
		SetCapture();
		break;

	case LB_EditRoad:
		OnLButtonDownEditRoad(nFlags, point);
		break;
	}
}

void BExtractorView::OnLButtonDownEditShape(UINT nFlags, CPoint point)
{
	//calculate what UTM_ERROR pixels are as scaled UTM coord
	double error = s_UTMdx(UTM_ERROR);

	BExtractorDoc* pDoc = GetDocument();
	int building1, building2, corner;
	double dist1, dist2;

	bool found1 = pDoc->m_Buildings.FindClosestBuildingCenter(m_downLocation, error, building1, dist1);
	bool found2 = pDoc->m_Buildings.FindClosestBuildingCorner(m_downLocation, error, building2, corner, dist2);

	if (found1 && found2)
	{
		// which was closer?
		if (dist1 < dist2)
			found2 = false;
		else
			found1 = false;
	}
	if (found1)
	{
		// closest point is a building center
		m_pCurBuilding = pDoc->m_Buildings.GetAt(building1)->GetBuilding();
		m_bDragCenter = true;
	}
	if (found2)
	{
		// closest point is a building corner
		m_pCurBuilding = pDoc->m_Buildings.GetAt(building2)->GetBuilding();
		m_bDragCenter = false;

		m_bShift = ((nFlags & MK_SHIFT) != 0);
		m_bControl = ((nFlags & MK_CONTROL) != 0);

		m_bRotate = m_bControl;
	}
	if (found1 || found2)
	{
		m_bRubber = true;
		m_iCurCorner = corner;

		// make a copy of the building, to edit and diplay while dragging
		m_EditBuilding = *m_pCurBuilding;
	}
}

void BExtractorView::OnLButtonDownEditRoad(UINT nFlags, CPoint point)
{
	BExtractorDoc* pDoc = GetDocument();
	TNode *pNode;
	DPoint2 geoPoint;
	CPoint nodePoint = point;
	CRect myRect;
	CPoint rectPoint1, rectPoint2;
	CRect roadSectionRect;

	GetClientRect(myRect);

	if (!myRect.PtInRect(point))
		// Cull out scroll presses etc.
		return;

	if (!m_bRubber)
	{
		// Starting a road
		if (FindNearestRoadNode(nodePoint, &pNode))
		{
			m_pCurrentRoad = pDoc->m_Links.NewLink();
			m_pCurrentRoad->m_iLanes = 2;
			m_pCurrentRoad->SetSize(1);
			(*m_pCurrentRoad)[0] = pNode->m_p;
			pDoc->m_Links.AddLink(m_pCurrentRoad);
			m_pCurrentRoad->SetNode(0, pNode);
			pNode->AddLink(m_pCurrentRoad, true);	// road starts at this node
			m_bRubber = true;
			m_p1 = m_p0 = nodePoint;
			OnMouseMove(nFlags, point);
		}
	}
	else
	{
		// If near a node then finish the road
		// But make sure loops have at least 2 intermediate points
		if (FindNearestRoadNode(nodePoint, &pNode) &&
			((pNode != m_pCurrentRoad->GetNode(0)) || (m_pCurrentRoad->GetSize() > 3)))
		{
			m_pCurrentRoad->SetNode(1, pNode);
			if (pNode != m_pCurrentRoad->GetNode(0))
			{
				pNode->AddLink(m_pCurrentRoad, false);	// road ends at this node
				m_pCurrentRoad->SetSize(m_pCurrentRoad->GetSize() + 1);
				(*m_pCurrentRoad)[m_pCurrentRoad->GetSize() - 1] = pNode->m_p;
			}
			OnMouseMove(nFlags, nodePoint);
			m_bRubber = false;
			UTM_s((*m_pCurrentRoad)[m_pCurrentRoad->GetSize() - 1], rectPoint1);
			UTM_s((*m_pCurrentRoad)[m_pCurrentRoad->GetSize() - 2], rectPoint2);
			roadSectionRect.SetRect(rectPoint1.x, rectPoint1.y,
				rectPoint2.x, rectPoint2.y);
			roadSectionRect.NormalizeRect();
			InvalidateRect(roadSectionRect);
			m_pCurrentRoad = NULL;
		}
		else
		{
			s_UTM(point, geoPoint);
			m_pCurrentRoad->SetSize(m_pCurrentRoad->GetSize() + 1);
			(*m_pCurrentRoad)[m_pCurrentRoad->GetSize() - 1] = geoPoint;
			m_p0 = m_p1 = point;
			UTM_s((*m_pCurrentRoad)[m_pCurrentRoad->GetSize() - 1], rectPoint1);
			UTM_s((*m_pCurrentRoad)[m_pCurrentRoad->GetSize() - 2], rectPoint2);
			roadSectionRect.SetRect(rectPoint1.x, rectPoint1.y,
				rectPoint2.x, rectPoint2.y);
			roadSectionRect.NormalizeRect();
			InvalidateRect(roadSectionRect);
		}
	}
}


void BExtractorView::OnMButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();				//capture mouse
	m_downPoint = point;
	m_old_offset = m_offset;
	m_bPanning = true;
}

void BExtractorView::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_bPanning = false;
	if (m_mode == LB_Footprint)
		CancelFootprint();
	if (m_mode == LB_Rectangle || m_mode == LB_Circle)
		CancelShape();
}

void BExtractorView::OnLButtonUp(UINT nFlags, CPoint point)
{
	s_UTM(point, m_curLocation);

	switch (m_mode)
	{
	case LB_AddRemove: //we're selecting buildings
		OnLButtonUpAddRemove(point);
		break;
	case LB_Hand:
		m_bPanning = false;
		break;
	case LB_Footprint:
		OnLButtonUpFootprint(point);
		break;
	case LB_Rectangle:
		OnLButtonUpRectangle(point);
		break;
	case LB_Circle:
		OnLButtonUpCircle(point);
		break;
	case LB_EditShape:
		if (m_bRubber)
		{
			DRECT extent_old, extent_new;
			m_pCurBuilding->GetExtents(extent_old);
			m_EditBuilding.GetExtents(extent_new);
			CRect screen_old = screen(extent_old);
			CRect screen_new = screen(extent_new);
			screen_old.InflateRect(1, 1);
			screen_new.InflateRect(1, 1);

			InvalidateRect(screen_old);
			InvalidateRect(screen_new);

			// copy back from temp building to real building
			*m_pCurBuilding = m_EditBuilding;
			m_bRubber = false;
		}
		break;
	case LB_EditRoadNodes:
		OnLButtonUpEditRoadNodes(point);
		break;
	}
	ReleaseCapture();		//release the mouse
}

void BExtractorView::OnLButtonUpFootprint(CPoint point)
{
	BExtractorDoc* pDoc = GetDocument();

	int points = m_poly.GetSize();
	if (points > 2)
	{
		// continuing poly
		// did they click back near the first point?
		CPoint screenpoint;
		UTM_s(m_poly[0], screenpoint);
		CPoint diff = screenpoint - point;
		if (abs(diff.x) + abs(diff.y) < 6)
		{
			// yes, done
			vtBuilding *bld = new vtBuilding;
			bld->SetFootprint(0, m_poly);
			bld->SetStories(1);
			bld->GetLevel(1)->SetEdgeColor(RGBi(255,255,255));	// white roof

			pDoc->m_Buildings.Append(bld);

			InvalidatePolyExtent();
			m_poly.Empty();
			return;
		}
	}
	if (points > 1 && m_bConstrain)
		ContrainLocationForPoly();

	// add points to poly
	m_poly.Append(m_curLocation);
}

void BExtractorView::OnLButtonUpRectangle(CPoint point)
{
	if (!m_bRubber)
		return;

	BExtractorDoc* pDoc = GetDocument();

	if (m_iStep == 2)
	{
		DPoint2 p0, p1, p2, p3;

		vtBuilding *bld = new vtBuilding;

		s_UTM(m_p0, p0);
		s_UTM(m_p1, p1);
		s_UTM(m_p2, p2);
		s_UTM(m_p3, p3);

		DPoint2 edge1 = p1 - p0, edge2 = p2 - p1;
		DPoint2 center = (p0 + p2) / 2.0f;

		float fWidth = (float) edge1.Length();
		float fDepth = (float) edge2.Length();

		// get rotation from the slope of the first edge
		float fRotation = (float) atan2(edge1.y, edge1.x);

		bld->SetRectangle(center, fWidth, fDepth, fRotation);
		bld->SetStories(1);
		bld->GetLevel(1)->SetEdgeColor(RGBi(255,255,255));	// white roof

		pDoc->m_Buildings.Append(bld);

		DRECT extent_new;
		bld->GetExtents(extent_new);
		CRect screen_new = screen(extent_new);
		InvalidateRect(screen_new);

		m_iStep = 0;
		m_bRubber = false;
	}
}

void BExtractorView::OnLButtonUpCircle(CPoint point)
{
	if (!m_bRubber)
		return;

	BExtractorDoc* pDoc = GetDocument();

	vtBuilding *bld = new vtBuilding;

	DPoint2 p;
	s_UTM(m_p0, p);
	float fCoordRadius = (float) s_UTMdx(m_fPixelRadius);

	bld->SetCircle(p, fCoordRadius);
	bld->SetStories(1);
	bld->GetLevel(1)->SetEdgeColor(RGBi(255,255,255));	// white roof

	pDoc->m_Buildings.Append(bld);

	DRECT extent_new;
	bld->GetExtents(extent_new);
	CRect screen_new = screen(extent_new);
	InvalidateRect(screen_new);

	m_bRubber = false;
}

void BExtractorView::OnLButtonUpAddRemove(CPoint point)
{
	DPoint2 imagepoint;
	s_UTM(point, imagepoint);

	//check to see whether they dragged or not
	if (!m_bRubber) // they clicked without dragging
	{
		m_maybeRect = false; //reset this guy

		BExtractorDoc* pDoc = GetDocument();

		// put one there if it is on the TIF
		if (!SelectionOnPicture(imagepoint))
			return;

		// create and add building
		vtBuilding *bld = new vtBuilding;
		bld->SetRectangle(imagepoint, 10, 10);
		bld->SetStories(1);
		bld->GetLevel(1)->SetEdgeColor(RGBi(255,255,255));	// white roof
		pDoc->m_Buildings.Append(bld);

		// redraw area of window around new building
		int size = UTM_sdx(15.0f);
		RECT r;
		r.top = point.y-size-1;
		r.bottom = point.y+size+2;
		r.left = point.x-size-1;
		r.right = point.x+size+2;
		InvalidateRect(&r);
	}
	else  //they're mopping
	{
		m_bRubber = false; //done drawing

		//put starting and ending points in UTM coords
		DPoint2 UTM_start, UTM_end;
		UTM_start = m_downLocation;

		// cycle through the current buildings, removing any that are in the
		// selected area
		MopRemove(UTM_start, imagepoint);
	}
}

void BExtractorView::OnLButtonUpEditRoadNodes(CPoint point)
{
	DPoint2 imagepoint;
	s_UTM(point, imagepoint);

	//check to see whether they dragged or not
	if (!m_bRubber) // they clicked without dragging
	{
		m_maybeRect = false; //reset this guy

		BExtractorDoc* pDoc = GetDocument();

		// put one there if it is on the TIF
		if (!SelectionOnPicture(imagepoint))
			return;

		// create and new road node
		TNode *pNode = pDoc->m_Links.NewNode();
		pDoc->m_Links.AddNode(pNode);
		pNode->m_p = imagepoint;

		int size = UTM_sdx(15.0f);
		RECT r;
		r.top = point.y-size-1;
		r.bottom = point.y+size+2;
		r.left = point.x-size-1;
		r.right = point.x+size+2;
		InvalidateRect(&r);
	}
	else  //they're mopping
	{
		m_bRubber = false; //done drawing

		//put starting and ending points in UTM coords
		DPoint2 UTM_start, UTM_end;
		UTM_start = m_downLocation;

		//cycle through the current nodes/roads, removing any that are in
		// the selected area
		MopRemoveRoadNodes(UTM_start, imagepoint);
	}
}

void BExtractorView::OnMButtonUp(UINT nFlags, CPoint point)
{
	m_bPanning = false;
	ReleaseCapture();			//release the mouse
}

void BExtractorView::OnMouseMove(UINT nFlags, CPoint point)
{
	m_lastMousePoint = point;
	s_UTM(point, m_curLocation);

	BExtractorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CDC *pDC = GetInvertDC();

	if (m_maybeRect)
	{
		//check to see if they are mopping
		if ((abs(point.x - m_downPoint.x) > 2) &&
			(abs(point.y - m_downPoint.y) > 2))
		{
			m_bRubber = true; //definitely mopping
			m_maybeRect = false;  //reset this
		}
	}
	if (m_bPanning == true)
		DoPan(point);
	else if ((m_mode == LB_AddRemove) && m_bRubber)
	{
		DrawRect(pDC, m_downPoint, m_oldPoint);
		//draw the new rectangle
		DrawRect(pDC, m_downPoint, point);
	}
	else if ((m_mode == LB_EditRoadNodes) && m_bRubber)
	{
		DrawRect(pDC, m_downPoint, m_oldPoint);
		//draw the new rectangle
		DrawRect(pDC, m_downPoint, point);
	}
	else if (m_mode == LB_Footprint)
	{
		int points = m_poly.GetSize();
		if (points > 0)
		{
			DrawPoly(pDC);
			pDC->LineTo(m_oldPoint.x, m_oldPoint.y);
		}
		if (points > 1 && m_bConstrain)
		{
			ContrainLocationForPoly();
			UTM_s(m_curLocation, point);
		}
		if (points > 0)
		{
			DrawPoly(pDC);
			pDC->LineTo(point.x, point.y);
		}
	}
	else if (m_mode == LB_Rectangle)
	{
		if (m_bRubber)
		{
			DrawRectangle(pDC);
			UpdateRectangle(point);
			DrawRectangle(pDC);
		}
	}
	else if (m_mode == LB_Circle)
	{
		if (m_bRubber)
		{
			DrawCircle(pDC);
			UpdateCircle(point);
			DrawCircle(pDC);
		}
	}
	else if (m_mode == LB_EditShape)
	{
		if (m_bRubber)
		{
			DrawCurrentBuilding(pDC);
			if (m_bDragCenter)
				UpdateMove();
			else if (m_bRotate)
				UpdateRotate();
			else
				UpdateResizeScale();
			DrawCurrentBuilding(pDC);
		}
	}
	else if (LB_EditRoad == m_mode)
	{
		if (m_bRubber)
		{
			pDC->MoveTo(m_p0);
			pDC->LineTo(m_p1);
			m_p1 = point;
			pDC->MoveTo(m_p0);
			pDC->LineTo(m_p1);
		}
	}

	ReleaseDC(pDC);

	//update old point
	m_oldPoint = point;

	// update status bar
	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	pFrame->RefreshStatusBar(this);
}

CDC *BExtractorView::GetInvertDC()
{
	CDC *pDC = GetDC();
	CPen bgPen( PS_SOLID, 1, RGB(0xff, 0xff, 0xff) ); //pen is black
	pDC->SetROP2(R2_NOT);
	return pDC;
}

void BExtractorView::DoPan(CPoint point)
{
	int oldx = m_offset.x;
	int oldy = m_offset.y;

	// calculate how far user has dragged since they began
	// update the offset to account for this movement
	m_offset.x = m_old_offset.x + (point.x - m_downPoint.x);
	m_offset.y = m_old_offset.y + (point.y - m_downPoint.y);

	// keep it in bounds
	ClipOffset();

	// update picture to reflect the changes
	ScrollWindow(m_offset.x - oldx, m_offset.y - oldy);

	// and the scroll bars
	UpdateScrollPos();
}

void BExtractorView::UpdateRectangle(CPoint point)
{
	if (m_iStep == 1)
		m_p1 = point;
	if (m_iStep == 2)
	{
		CPoint A = m_p0;
		CPoint B = m_p1;
		CPoint C = point;
		CPoint AB = B - A;
		float L2 = (float) (AB.x*AB.x + AB.y*AB.y);
		float L = sqrtf(L2);

		float s = ((A.y-C.y)*(B.x-A.x)-(A.x-C.x)*(B.y-A.y)) / L2;

		FPoint2 norm((float)-AB.y, (float)AB.x);
		norm.Normalize();
		m_p2.x = (long) (B.x - (norm.x * s * L));
		m_p2.y = (long) (B.y - (norm.y * s * L));
		m_p3.x = (long) (A.x - (norm.x * s * L));
		m_p3.y = (long) (A.y - (norm.y * s * L));
	}
}

void BExtractorView::DrawRectangle(CDC *pDC)
{
	if (m_iStep == 1)
	{
		pDC->MoveTo(m_p0);
		pDC->LineTo(m_p1);
	}
	if (m_iStep == 2)
	{
		pDC->MoveTo(m_p0);
		pDC->LineTo(m_p1);
		pDC->LineTo(m_p2);
		pDC->LineTo(m_p3);
		pDC->LineTo(m_p0);
	}
}

void BExtractorView::DrawPoly(CDC *pDC)
{
	CPoint screen;
	for (unsigned int i = 0; i < m_poly.GetSize(); i++)
	{
		UTM_s(m_poly[i], screen);
		if (i == 0)
			pDC->MoveTo(screen.x, screen.y);
		else
			pDC->LineTo(screen.x, screen.y);
	}
}

void BExtractorView::UpdateCircle(CPoint point)
{
	CPoint A = m_downPoint;
	CPoint B = point;
	CPoint AB = B - A;
	m_p0.x = (A.x + B.x) / 2;
	m_p0.y = (A.y + B.y) / 2;
	m_fPixelRadius = sqrtf((float)AB.x * AB.x + AB.y * AB.y) / 2.0f;
}

void BExtractorView::DrawCircle(CDC *pDC)
{
	int size = (int) m_fPixelRadius;
	DrawCircle(pDC, m_p0, size);
}

void BExtractorView::DrawCircle(CDC *pDC, CPoint &center, int iRadius)
{
	int sx, sy;
	for (int i = 0; i <= 16; i++)
	{
		double a = (double)i * PI2d / 16;
		sx = (int) (center.x + cos(a) * iRadius);
		sy = (int) (center.y + sin(a) * iRadius);
		if (i == 0)
			pDC->MoveTo(sx, sy);
		else
			pDC->LineTo(sx, sy);
	}
}

void BExtractorView::UpdateMove()
{
	DPoint2 p;
	DPoint2 moved_by = m_curLocation - m_downLocation;

	m_EditBuilding = *m_pCurBuilding;
	m_EditBuilding.Offset(moved_by);
}

void BExtractorView::UpdateResizeScale()
{
	DPoint2 moved_by = m_curLocation - m_downLocation;

	DPoint2 origin;
	m_pCurBuilding->GetBaseLevelCenter(origin);

	DPoint2 diff1 = m_downLocation - origin;
	DPoint2 diff2 = m_curLocation - origin;
	double fScale = diff2.Length() / diff1.Length();

	unsigned int i;
	DPoint2 p;
	DPolygon2 footprint = m_pCurBuilding->GetFootprint(0);
	DLine2 &foot = footprint[0];
	if (m_bShift)
	{
		// Scale evenly
		for (i = 0; i < foot.GetSize(); i++)
		{
			p = foot.GetAt(i);
			p -= origin;
			p *= fScale;
			p += origin;
			foot.SetAt(i, p);
		}
	}
	else
	{
		// drag individual corner points
		p = foot.GetAt(m_iCurCorner);

		int points = foot.GetSize();
		if (m_bConstrain && points > 3)
		{
			// Maintain angles
			DPoint2 p0 = foot.GetSafePoint(m_iCurCorner - 1);
			DPoint2 p1 = foot.GetSafePoint(m_iCurCorner + 1);
			DPoint2 vec0 = (p - p0).Normalize();
			DPoint2 vec1 = (p - p1).Normalize();
			DPoint2 vec2 = vec0;
			vec2.Rotate(PID2d);
			DPoint2 vec3 = vec1;
			vec3.Rotate(PID2d);

			p += moved_by;
			double a;

			a = (p - p0).Dot(vec2);
			foot.SetSafePoint(m_iCurCorner - 1, p0 + (vec2 * a));
			a = (p - p1).Dot(vec3);
			foot.SetSafePoint(m_iCurCorner + 1, p1 + (vec3 * a));
			foot.SetAt(m_iCurCorner, p);
		}
		else
		{
			p += moved_by;
			foot.SetAt(m_iCurCorner, p);
		}
	}
	// Changing only the lowest level is near useless.  For the great
	//  majority of cases, the user will want the footprints for all
	//  levels to remain in sync.
	for (i = 0; i < m_EditBuilding.GetNumLevels(); i++)
		m_EditBuilding.SetFootprint(i, foot);
}

void BExtractorView::UpdateRotate()
{
	DPoint2 origin;
	m_pCurBuilding->GetBaseLevelCenter(origin);

	DPoint2 original_vector = m_downLocation - origin;
	double length1 = original_vector.Length();
	double angle1 = atan2(original_vector.y, original_vector.x);

	DPoint2 cur_vector = m_curLocation - origin;
	double length2 = cur_vector.Length();
	double angle2 = atan2(cur_vector.y, cur_vector.x);

	double angle_diff = angle2 - angle1;

	DPoint2 p;
	DPolygon2 footprint = m_pCurBuilding->GetFootprint(0);
	DLine2 &foot = footprint[0];
	for (unsigned int i = 0; i < foot.GetSize(); i++)
	{
		p = foot.GetAt(i);
		p -= origin;
		p.Rotate(angle_diff);
		p += origin;
		foot.SetAt(i, p);
	}
	m_EditBuilding.SetFootprint(0, foot);
}

void BExtractorView::DrawCurrentBuilding(CDC *pDC)
{
	DrawBuilding(pDC, &m_EditBuilding);
}

void BExtractorView::InvalidatePolyExtent()
{
	CRect e(1000, 1000, -1000, -1000);
	CPoint screen;

	for (unsigned int i = 0; i < m_poly.GetSize(); i++)
	{
		UTM_s(m_poly[i], screen);
		if (screen.x < e.left) e.left = screen.x;
		if (screen.x > e.right) e.right = screen.x;
		if (screen.y < e.top) e.top = screen.y;
		if (screen.y > e.bottom) e.bottom = screen.y;
	}
	e.left--;
	e.right++;
	e.top--;
	e.bottom++;
	InvalidateRect(&e);
}

void BExtractorView::DrawRect(CDC *pDC, CPoint one, CPoint two)
{
	pDC->MoveTo(one.x, one.y);
	pDC->LineTo(one.x, two.y);
	pDC->LineTo(two.x, two.y);
	pDC->LineTo(two.x, one.y);
	pDC->LineTo(one.x, one.y);
}

bool BExtractorView::SelectionOnPicture(DPoint2 point)
{
	//if the user has selected a point off the TIF, returns false. On the
	// TIF, returns true.
	if (GetDocument()->m_picLoaded)
	{
		CBImage* pImage = GetDocument()->m_pImage;
		double width = pImage->m_fImageWidth;
		double height = pImage->m_fImageHeight;
		double x_base = pImage->m_xUTMoffset;
		double y_base = pImage->m_yUTMoffset;

		double x_min = x_base;
		double x_max = x_base + width;
		double y_min = y_base - height;
		double y_max = y_base;

		if ( (point.x >= x_min) && (point.x <= x_max) &&
			 (point.y >= y_min) && (point.y <= y_max) )
			return true;
		else return false;
	}
	return false;
}

void BExtractorView::MopRemove(const DPoint2 &start, const DPoint2 &end)
{
	BExtractorDoc* doc = GetDocument();

	DRECT drect(start.x, start.y, end.x, end.y);
	drect.Sort();

	int num_points = doc->m_Buildings.GetSize();
	DPoint2 point;
	DPoint2 unused;
	bool firstpt = true;
	int counter = 0;
	CRect screen_rect;

	for (int i = 0; i < num_points; i++)
	{
		vtBuilding *bld = doc->m_Buildings.GetAt(i)->GetBuilding();
		if (!bld)
			continue;
		DPoint2 point;
		bld->GetBaseLevelCenter(point);

		if (drect.ContainsPoint(point))
		{
			//make a rectangle (to invalidate the changed part of the picture)
			DRECT extent_new;
			bld->GetExtents(extent_new);
			screen_rect = screen(extent_new);
			InvalidateRect(screen_rect);

			//this point was in the user-selected area, remove it
			doc->m_Buildings.RemoveAt(i, 1);
			if (firstpt)
			{
				firstpt = false;
			}
			counter++;
			i = 0;
			num_points = doc->m_Buildings.GetSize();
			i--;
		}
	}
	screen_rect = screen(drect);
	screen_rect.InflateRect(1, 1);
	InvalidateRect(screen_rect);
}

void BExtractorView::MopRemoveRoadNodes(const DPoint2 &start, const DPoint2 &end)
{

	BExtractorDoc* doc = GetDocument();
	DRECT drect(start.x, start.y, end.x, end.y);
	TNode *pNode;
	TNode *pNext;
	TNode *pPreviousNode = NULL;
	CRect screen_rect;
	int size = UTM_sdx(15.0f);
	CPoint screenPoint;
	int numRoads;

	drect.Sort();

	pNode = doc->m_Links.GetFirstNode();

	while (NULL != pNode)
	{
		pNext = pNode->m_pNext;
		if (drect.ContainsPoint(pNode->m_p))
		{
			// Remove any roads starting or terminating at this node
			numRoads = pNode->m_iLinks;
			for (int i = 0; i < numRoads; i++)
			{
				TLink *pLink = pNode->GetLink(0);
				if (pLink->GetNode(0) == pNode)
				{
					pLink->GetNode(0)->DetachLink(pLink, true);
					pLink->GetNode(1)->DetachLink(pLink, false);
				}
				else
				{
					pLink->GetNode(0)->DetachLink(pLink, false);
					pLink->GetNode(1)->DetachLink(pLink, true);
				}
				doc->m_Links.RemoveLink(pLink);
			}
			doc->m_Links.RemoveNode(pNode);
		}
		else
			pPreviousNode = pNode;
		pNode = pNext;
	}

	Invalidate();
}


/////////////////////////////////////////////////////////////////////////////
// BExtractorView : Functions to do the REAL work
/////////////////////////////////////////////////////////////////////////////


void BExtractorView::OnFunctionsConvolve()
{
#if USE_IPL
	BExtractorDoc* doc = GetDocument();
	if (!doc->m_picLoaded)
		return;

	CDib *bm = doc->m_pImage->m_pMonoDIB;

	//do the pre-processing flood fill to remove large things (like big letters)
	CFloodDialog fdlg;
	fdlg.m_iSelection = 0;
	if (fdlg.DoModal() == IDCANCEL)
		return;

	if (fdlg.m_iSelection == 0)
	{
		doc->PreFloodFillDIB(bm);
	}

	BITMAPINFOHEADER *Hdr = bm->GetDIBHeader();

	//convert DIB -> IPLImage
	IplImage* i1 = iplCreateImageHeader(
		1, // number of channels
		0, // no alpha channel
		IPL_DEPTH_16U, // data of byte type
		"GRAY", // color model
		"GRAY", // color order
		IPL_DATA_ORDER_PIXEL,
		IPL_ORIGIN_BL,
		IPL_ALIGN_QWORD, // 8 bytes align
		Hdr->biWidth, // image width
		Hdr->biHeight, // image height
		NULL, // no ROI
		NULL, // no mask ROI
		NULL, // no image ID
		NULL); // not tiled

	iplConvertFromDIBSep(Hdr, bm->GetData(), i1);
	IplImage* i2 = iplCloneImage(i1);

	//do the image manipulation
	iplNot(i1, i2); //makes black be 0xffff, white be 0x0000

	//have the user select the kernel size, and do the image convolution
	CKernelDialog kdlg;
	kdlg.m_iSelection = 1;
	if (kdlg.DoModal() == IDCANCEL)
		return;

	if (1)
	{
		CProgressDlg prog(CG_IDS_PROGRESS_CAPTION4);
		prog.Create(NULL);	// top level

		switch (kdlg.m_iSelection)
		{
		case 0:
			{	//prepare convolve kernel: 9x9
				int kernel_array[81] = {n3,n3,n3,n3,n3,n3,n3,n3,n3,
										n3,n1,n1,n1,n1,n1,n1,n1,n3,
										n3,n1,n1,n2,n2,n2,n1,n1,n3,
										n3,n1,n2,n4,n4,n4,n2,n1,n3,
										n3,n1,n2,n4,n4,n4,n2,n1,n3,
										n3,n1,n2,n4,n4,n4,n2,n1,n3,
										n3,n1,n1,n2,n2,n2,n1,n1,n3,
										n3,n1,n1,n1,n1,n1,n1,n1,n3,
										n3,n3,n3,n3,n3,n3,n3,n3,n3 };

				IplConvKernel* kernel = iplCreateConvKernel(9,9,4,4,kernel_array, 8);
				iplConvolve2D(i2, i1, &kernel, 1, IPL_SUM);
			}
			break;
		case 1:
		default:
			{	//prepare convolve kernel: 7x7
				int kernel_array[49] = {n3,n3,n3,n3,n3,n3,n3,
										n3,n1,n2,n2,n2,n1,n3,
										n3,n2,n4,n4,n4,n2,n3,
										n3,n2,n4,n4,n4,n2,n3,
										n3,n2,n4,n4,n4,n2,n3,
										n3,n1,n2,n2,n2,n1,n3,
										n3,n3,n3,n3,n3,n3,n3 };

				IplConvKernel* kernel = iplCreateConvKernel(7,7,4,4,kernel_array, 8);
				iplConvolve2D(i2, i1, &kernel, 1, IPL_SUM);
			}
			break;
		case 2:
			{	// 5x5
				int kernel_array[25] = {n3,n3,n3,n3,n3,
										n3,n2,n2,n2,n3,
										n3,n2,n4,n2,n3,
										n3,n2,n2,n2,n3,
										n3,n3,n3,n3,n3 };

				IplConvKernel* kernel = iplCreateConvKernel(5,5,4,4,kernel_array, 8);
				iplConvolve2D(i2, i1, &kernel, 1, IPL_SUM);
			}
			break;
		}
	}

	//have the user select the threshold level
	CConvolveDialog dlg;
	dlg.m_iSelection = 3;
	dlg.DoModal();
	switch (dlg.m_iSelection)
	{
		//	If you have a data file that has very closely-packed
		//	buildings, you should use a lower threshold (finds more
		//	buildings, but also more false hits)
		case 0:
			{
				iplThreshold(i1, i2, 46155);
				// (181/255 on an 8-bit scale)
				break;
			}
		case 1:
			{
				iplThreshold(i1, i2, 47175);
				// (185/255 on an 8-bit scale)
				break;
			}
		case 2:
			{
				iplThreshold(i1, i2, 48195);
				// (189/255 on an 8-bit scale)
				break;
			}
		case 3:
			{
				iplThreshold(i1, i2, 49215);
				// (193/255 on an 8-bit scale)
				break;
			}
		case 4:
			{
				iplThreshold(i1, i2, 50235);
				// (197/255 on an 8-bit scale)
				break;
			}
		case 5:
			{
				iplThreshold(i1, i2, 51255);
				// (201/255 on an 8-bit scale)
				break;
			}
		case 6:
			{
				iplThreshold(i1, i2, 52275);
				// (205/255 on an 8-bit scale)
				break;
			}
		case 7:
			{
				iplThreshold(i1, i2, 53295);
				// (209/255 on an 8-bit scale)
				break;
			}
		default:
			{
				iplThreshold(i1, i2, 50235);
				break;
			}
	}

	iplNot(i2, i1);

	CPaintDC cDC(this), *pDC = &cDC;
	//convert IPLImage -> DIB
	CDib ResultDib;
	ResultDib.Setup(pDC, bm->GetWidth(), bm->GetHeight(), 8,
		GetDocument()->m_hdd);

	iplConvertToDIBSep(i1, ResultDib.GetDIBHeader(),
		(char *) ResultDib.m_data, IPL_DITHER_NONE, IPL_PALCONV_NONE);

	// Distinguish individual buildings and label them
	if (1)
	{
		CProgressDlg prog(CG_IDS_PROGRESS_CAPTION5);
		prog.Create(NULL);	// top level

		doc->FloodFillDIB(&ResultDib, &prog);
	}

	// Loop through the building coordinates, remove any that are too close
	// to one another
	int l;
	int num = doc->m_Buildings.GetSize();
	bool match = false;

	if (1)
	{
		CProgressDlg prog(CG_IDS_PROGRESS_CAPTION6);
		prog.Create(NULL);	// top level

		for (int k = 0; k < num; )
		{
			if ((k % 10) == 0)
				prog.SetPos(k * 200 / num);

			vtBuilding *bld = doc->m_Buildings.GetAt(k)->GetBuilding();

			DPoint2 point, point2;

			bld->GetBaseLevelCenter(point);
			for (l = 0; (!match)&&(l < num); l++)
			{
				if (l!=k)
				{
					vtBuilding *bld2 = doc->m_Buildings.GetAt(l)->GetBuilding();
					bld2->GetBaseLevelCenter(point2);

					if ((fabs(point.x - point2.x) < 11) &&
						(fabs(point.y - point2.y) < 7))
						match = true;
				}
			}
			if (match)
			{
				doc->m_Buildings.RemoveAt(k, 1);
				num--;
				match = false;
			}
			else k++;
		}
	}
	CString NumberBuildings;
	NumberBuildings.Format("Building Extraction is finished. %d buildings were extracted", num);

	AfxMessageBox(NumberBuildings, MB_ICONINFORMATION);

	Invalidate(); //redraw picture
#endif // USE_IPL
}

void BExtractorView::OnUpdateFunctionsConvolve(CCmdUI* pCmdUI)
{
#if USE_IPL
	BExtractorDoc* pDoc = GetDocument();
	pCmdUI->Enable(pDoc->m_picLoaded);
#else
	pCmdUI->Enable(false);
#endif
}


/////////////////////////////////////////////////////////////////////////////
// BExtractorView : MODE handlers
/////////////////////////////////////////////////////////////////////////////

void BExtractorView::CancelFootprint()
{
	// Cancel a polygonal footprint in progress
	m_bRubber = false;
	m_poly.Empty();
	Invalidate();
}

void BExtractorView::CancelShape()
{
	m_bRubber = false;
	m_iStep = 0;
	m_fPixelRadius = 0.0f;
	Invalidate();
}

void BExtractorView::SetMode(LBMode mode)
{
	if (mode != m_mode)
	{
		// changing mode
		if (m_mode == LB_Footprint)
		{
			// cancel any pending footprint
			CancelFootprint();
		}
		if (m_mode == LB_Rectangle)
		{
			// cancel any pending rectangle
			m_bRubber = false;
			m_poly.SetSize(0);
		}
	}
	m_mode = mode;
}

void BExtractorView::OnAddRemove()
{
	SetMode(LB_AddRemove);
}

void BExtractorView::OnUpdateAddRemove(CCmdUI* pCmdUI)
{
	BExtractorDoc* pDoc = GetDocument();
	pCmdUI->Enable(pDoc->m_picLoaded);
	pCmdUI->SetCheck(m_mode == LB_AddRemove);
}

void BExtractorView::OnHand()
{
	SetMode(LB_Hand);
}

void BExtractorView::OnUpdateHand(CCmdUI* pCmdUI)
{
	BExtractorDoc* pDoc = GetDocument();
	pCmdUI->Enable(pDoc->m_picLoaded);
	pCmdUI->SetCheck(m_mode == LB_Hand);
}

void BExtractorView::UpdateRanges()
{
	CRect r;
	GetClientRect(&r);
	CBImage *pImage = GetDocument()->m_pImage;

	m_offset_size.x = (long)(pImage->m_fImageWidth / m_fScale);
	m_offset_size.y = (long)(pImage->m_fImageHeight / m_fScale);

	m_offset_base.x = - UTM_sdx(pImage->m_xUTMoffset)  //this is (0,0)
		+ r.right/2 - m_offset_size.x;
	m_offset_base.y = - UTM_sdy(pImage->m_yUTMoffset)
		+ r.bottom/2 - m_offset_size.y;

	SetScrollRange(SB_HORZ, 0, (int)m_offset_size.x, TRUE);
	SetScrollRange(SB_VERT, 0, (int)m_offset_size.y, TRUE);
	TRACE("New scroll range: maxH, maxV = %d,%d, current:%d,%d\n",
		(int)m_offset_size.x, (int)m_offset_size.y, m_scrollposH, m_scrollposV);

	UpdateScrollPos();
}

void BExtractorView::ZoomToImage(CBImage *pImage)
{
	UpdateRanges();

	m_offset.x = m_offset_base.x + m_offset_size.x/2;
	m_offset.y = m_offset_base.y + m_offset_size.y/2;

	UpdateScrollPos();
	Invalidate();
}

void BExtractorView::OnFullres()
{
	BExtractorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc->m_picLoaded)	return;

	double oldscale = m_fScale;
	double ratio = (double)(pDoc->m_pImage->m_xMetersPerPixel)/oldscale;
	ChangeScale(ratio);
}

void BExtractorView::ChangeScale(double fFactor)
{
	// Find geographical location of center of screen
	DPoint2 utm;
	CRect r;
	GetClientRect(r);
	CPoint screen;			// center of the screen
	screen.x = (r.right - r.left) / 2;
	screen.y = (r.bottom - r.top) / 2;
	utm.x = s_UTMx(screen.x);
	utm.y = s_UTMy(screen.y);

	// now change scale
	m_fScale *= fFactor;

	// find new location of that geographical point
	CPoint screen_new;
	screen_new.x = UTM_sx(utm.x);
	screen_new.y = UTM_sy(utm.y);

	// compensate by the difference
	CPoint diff;
	diff = screen_new - screen;
	m_offset -= diff;

	UpdateRanges();
	Invalidate();
}


void BExtractorView::OnZoomIn()
{
	BExtractorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (pDoc->m_picLoaded == 1)
		ChangeScale(0.7f);
}

void BExtractorView::OnZoomOut()
{
	BExtractorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (pDoc->m_picLoaded == 1)
		ChangeScale(1.0f/0.7f);
}

void BExtractorView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_UP)
	{
		if (GetKeyState(VK_SHIFT)&0x8000)
			OnVScroll(SB_PAGEUP, 0, NULL);
		else
			OnVScroll(SB_LINEUP, 0, NULL);
	}
	else if (nChar == VK_DOWN)
	{
		if (GetKeyState(VK_SHIFT)&0x8000)
			OnVScroll(SB_PAGEDOWN, 0, NULL);
		else
			OnVScroll(SB_LINEDOWN, 0, NULL);
	}
	else if (nChar == VK_LEFT)
	{
		if (GetKeyState(VK_SHIFT)&0x8000)
			OnHScroll(SB_PAGELEFT, 0, NULL);
		else
			OnHScroll(SB_LINELEFT, 0, NULL);
	}
	else if (nChar == VK_RIGHT)
	{	if (GetKeyState(VK_SHIFT)&0x8000)
			OnHScroll(SB_PAGERIGHT, 0, NULL);
		else
			OnHScroll(SB_LINERIGHT, 0, NULL);
	}
	else if (nChar == VK_SPACE) ZoomToBuilding();
}

void BExtractorView::ZoomToBuilding()
{
	BExtractorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (pDoc->m_picLoaded == 1)	//make sure an image is loaded
	{
		if (m_zoomed) //user wants to zoom back out (restore original view)
		{
			m_zoomed = false;

			m_fScale = m_fSavedScale;
			m_offset = m_SavedOffset;

			UpdateRanges();
			Invalidate();
		}
		else //user wants to zoom in to mark a building
		{
			m_zoomed = true;

			m_fSavedScale = m_fScale;	//save scale
			m_SavedOffset = m_offset;		//save offset

			//find mouse location in utm coords
			DPoint2 utm_mouse;
			s_UTM(m_lastMousePoint, utm_mouse);

			// now change scale
			m_fScale *= 0.125;

			// find new location of that mouse point
			CPoint screen_new;
			UTM_s(utm_mouse, screen_new);

			// compensate by the difference (keep mouse point where it was,
			// just bigger image)
			CPoint diff;
			diff = screen_new - m_lastMousePoint;
			m_offset -= diff;

			UpdateRanges();
			Invalidate();
		}
	}
}

void BExtractorView::OnColorChange()
{
	CColorDialog dlgColor(m_buildingColor);
	if (dlgColor.DoModal() == IDOK)
	{
		m_buildingColor = dlgColor.GetColor();
		WriteINIFile();
		Invalidate();
	}
}

bool BExtractorView::ReadINIFile()
{
	CString front(m_directory);
	front += "\\BE.ini";
	unsigned int tmp;

	FILE *fp = fopen(front, "r");
	if (!fp) return false;
	fscanf(fp, "%d ", &tmp);
	m_buildingColor = tmp;
	fscanf(fp, "%d ", &tmp);
	m_roadColor = tmp;
	fclose(fp);
	return true;
}

bool BExtractorView::WriteINIFile()
{
	CString front(m_directory);
	front += "\\BE.ini";
	unsigned int tmp;
	tmp = m_buildingColor;

	FILE *fp = fopen(front, "w");
	if (!fp) return false;
	fprintf(fp, "%d ", tmp);
	tmp = m_roadColor;
	fprintf(fp, "%d ", tmp);
	fclose(fp);
	return true;
}

void BExtractorView::OnClearscreenofBuildings()
{
	CRect r;
	GetClientRect(r);
	DPoint2 UTM_start;
	UTM_start.x = s_UTMx(r.left);
	UTM_start.y = s_UTMy(r.top);
	DPoint2 UTM_end;
	UTM_end.x = s_UTMx(r.right);
	UTM_end.y = s_UTMy(r.bottom);

	DPoint2 topleft, botright;
	topleft.x = UTM_start.x; topleft.y = UTM_start.y;
	botright.x = UTM_end.x; botright.y = UTM_end.y;

	// cycle through the selected buildings, removing any that are in the
	// given area
	MopRemove(topleft, botright);
}

void BExtractorView::OnUndo()
{
	BExtractorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

//	pDoc->UndoChange();
	Invalidate();
}

void BExtractorView::OnUpdateUndo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(FALSE);
}

void BExtractorView::OnViewViewfullcolorimage()
{
	BExtractorDoc* pDoc = GetDocument();
	CBImage *pImage = pDoc->m_pImage;
	if (!pImage)
		return;

	if (pImage->m_pCurrentDIB == pImage->m_pMonoDIB && pImage->m_pSourceDIB != NULL)
		pImage->m_pCurrentDIB = pImage->m_pSourceDIB;
	else
		pImage->m_pCurrentDIB = pImage->m_pMonoDIB;
	Invalidate();
}

void BExtractorView::OnUpdateViewViewfullcolorimage(CCmdUI* pCmdUI)
{
	BExtractorDoc* pDoc = GetDocument();
	CBImage *pImage = pDoc->m_pImage;

	pCmdUI->Enable(pImage != NULL && pImage->m_pSourceDIB != NULL);
	pCmdUI->SetCheck(pImage != NULL && pImage->m_pCurrentDIB != NULL &&
		pImage->m_pCurrentDIB == pImage->m_pSourceDIB);
}

void BExtractorView::OnModesFootprintMode()
{
	SetMode(LB_Footprint);
}

void BExtractorView::OnUpdateModesFootprintmode(CCmdUI* pCmdUI)
{
	BExtractorDoc* pDoc = GetDocument();
	pCmdUI->Enable(pDoc->m_picLoaded);
	pCmdUI->SetCheck(m_mode == LB_Footprint);
}

void BExtractorView::OnModesRectangle()
{
	SetMode(LB_Rectangle);
}

void BExtractorView::OnUpdateModesRectangle(CCmdUI* pCmdUI)
{
	BExtractorDoc* pDoc = GetDocument();
	pCmdUI->Enable(pDoc->m_picLoaded);
	pCmdUI->SetCheck(m_mode == LB_Rectangle);
}

void BExtractorView::OnModesCircle()
{
	SetMode(LB_Circle);
}

void BExtractorView::OnUpdateModesCircle(CCmdUI* pCmdUI)
{
	BExtractorDoc* pDoc = GetDocument();
	pCmdUI->Enable(pDoc->m_picLoaded);
	pCmdUI->SetCheck(m_mode == LB_Circle);
}

void BExtractorView::OnUpdateFileSave(CCmdUI* pCmdUI)
{
	BExtractorDoc* pDoc = GetDocument();
	pCmdUI->Enable(pDoc->m_Buildings.GetSize() != 0);
}

void BExtractorView::OnModesMoveresize()
{
	SetMode(LB_EditShape);
}

void BExtractorView::OnUpdateModesMoveresize(CCmdUI* pCmdUI)
{
	BExtractorDoc* pDoc = GetDocument();
	pCmdUI->Enable(pDoc->m_picLoaded);
	pCmdUI->SetCheck(m_mode == LB_EditShape);
}


void BExtractorView::OnModesRoadnode()
{
	SetMode(LB_EditRoadNodes);
}

void BExtractorView::OnUpdateModesRoadnode(CCmdUI* pCmdUI)
{
	BExtractorDoc* pDoc = GetDocument();
	pCmdUI->Enable(pDoc->m_picLoaded);
	pCmdUI->SetCheck(m_mode == LB_EditRoadNodes);
}

void BExtractorView::OnChangeRoadColor()
{
	CColorDialog dlgColor(m_roadColor);
	if (dlgColor.DoModal() == IDOK)
	{
		m_roadColor = dlgColor.GetColor();
		WriteINIFile();
		Invalidate();
	}
}

void BExtractorView::OnModesRoadEdit()
{
	SetMode(LB_EditRoad);
}

void BExtractorView::OnUpdateModesRoadEdit(CCmdUI* pCmdUI)
{
	BExtractorDoc* pDoc = GetDocument();
	pCmdUI->Enable(pDoc->m_picLoaded);
	pCmdUI->SetCheck(m_mode == LB_EditRoad);
}

void BExtractorView::OnConstrain()
{
	m_bConstrain = !m_bConstrain;
}

void BExtractorView::OnUpdateConstrain(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bConstrain);
}


void BExtractorView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	// update status bar
	if (bActivate == TRUE)
	{
		CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
		pFrame->RefreshStatusBar(this);
	}

	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}
