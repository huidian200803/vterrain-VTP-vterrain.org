//
// BExtractorView.h : interface of the BExtractorView class
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#if !defined(AFX_B_EXTRACTORVIEW_H)
#define AFX_B_EXTRACTORVIEW_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "vtdata/Building.h"

#define BLENGTH 6

#define NODE_CAPTURE_THRESHOLD 10.0  // Metres

enum LBMode {
	LB_AddRemove,	// click to add, drag to remove
	LB_Hand,		// drag to pan
	LB_Footprint,	// click to add footprints
	LB_Rectangle,	// click to add rectangular footprints
	LB_Circle,
	LB_EditShape,
	LB_EditRoadNodes,
	LB_EditRoad
};

class BExtractorDoc;
class CBImage;
class TNode;
class TLink;

class BExtractorView : public CView
{
protected: // create from serialization only
	BExtractorView();
	DECLARE_DYNCREATE(BExtractorView)

// Attributes
public:
	BExtractorDoc* GetDocument();
	DPoint2 GetCurLocation() { return m_curLocation; }

// Operations
public:
	void ReadDataPath();

	void ZoomToImage(CBImage *pImage);
	void DrawBuildings(CDC *pDC);
	void DrawBuilding(CDC *pDC, vtBuilding *bld);
	void DrawRoadNodes(CDC *pDC);
	void DrawRoadNode(CDC *pDC, TNode *pNode);
	void DrawRoads(CDC *pDC);
	void DrawRoad(CDC *pDC, TLink *pLink);
	bool FindNearestRoadNode(CPoint &point, TNode **pNearestNode);
	bool ReadINIFile();
	bool WriteINIFile();
	bool SelectionOnPicture(DPoint2 point);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(BExtractorView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~BExtractorView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	// For scaling and panning the view
	CPoint m_offset;		// pixel offset between UTM and screen origins
	CPoint m_offset_base;	// minimum value of m_offset
	CPoint m_offset_size;	// range m_offset
	double m_fScale;		// meters/screen_pixel
	int m_scrollposH;
	int m_scrollposV;

	// Left Mouse Button Mode
	LBMode m_mode;

	CPoint m_old_offset;	// saved to calculate new position for hand mode
	CPoint m_oldPoint;
	CPoint m_lastMousePoint;

	// Used for the space-bar zoom effect
	bool m_zoomed;
	CPoint m_SavedOffset;	// used to save offset when we ZoomToBuilding
	double m_fSavedScale;	// used to save Scale when we ZoomToBuilding

	// Color used to draw the buildings
	COLORREF m_buildingColor;

	// Color used to draw the roads
	COLORREF m_roadColor;

	// Current road when plotting roads
	TLink *m_pCurrentRoad;

	// Directory path at startup, used to locate the .ini file
	char m_directory[MAX_PATH];

	// Used while defining a new building polygon
	DLine2	m_poly;

	// Used while dragging the mouse
	bool	m_maybeRect;
	bool	m_bPanning;		// true while panning
	bool	m_bRubber;
	float	m_fPixelRadius;
	int		m_iStep;			// steps to a rectangle
	CPoint	m_p0, m_p1, m_p2, m_p3;
	CPoint	m_downPoint;		// point at which mouse was clicked down (used in hand mode and mop-up mode
	DPoint2	m_downLocation;
	DPoint2	m_curLocation;
	DPoint2 m_curBuildingCenter;

	// Used while editing buildings
	vtBuilding	*m_pCurBuilding, m_EditBuilding;
	int			m_iCurCorner;
	bool	m_bDragCenter;
	bool	m_bRotate;
	bool	m_bControl;
	bool	m_bShift;
	bool	m_bConstrain;

	// transform screen space -> UTM space
	double s_UTMx(int sx) { return ((sx - m_offset.x) * m_fScale); }
	double s_UTMy(int sy) { return -((sy - m_offset.y) * m_fScale); }
	void s_UTM(CPoint &p, DPoint2 &utm)
	{
		utm.x = s_UTMx(p.x);
		utm.y = s_UTMy(p.y);
	}

	// transform UTM space -> screen space
	int UTM_sx(double utm_x) { return (int)((utm_x / m_fScale) + m_offset.x); }
	int UTM_sy(double utm_y) { return (int)((-utm_y / m_fScale) + m_offset.y); }
	void UTM_s(const DPoint2 &utm, CPoint &p)
	{
		p.x = UTM_sx((float)utm.x);
		p.y = UTM_sy((float)utm.y);
	}

	// transform UTM space -> screen space
	int UTM_sdx(double utm_x) { return (int)(utm_x / m_fScale); }
	int UTM_sdy(double utm_y) { return (int)(-utm_y / m_fScale); }
	CPoint UTM_sd(const DPoint2 &utm)
	{
		return CPoint((int)(utm.x / m_fScale),
					  (int)(-utm.y / m_fScale));
	}
	double s_UTMdx(double sx) { return (sx * m_fScale); }
	double s_UTMdy(double sy) { return (-sy * m_fScale); }
	CRect screen(const DRECT &r)
	{
		CRect r2;
		r2.left = UTM_sx(r.left);
		r2.top = UTM_sy(r.top);
		r2.right = UTM_sx(r.right);
		r2.bottom = UTM_sy(r.bottom);
		return r2;
	}

	void ChangeScale(double fFactor);
	void MopRemove(const DPoint2 &start, const DPoint2 &end);
	void MopRemoveRoadNodes(const DPoint2 &start, const DPoint2 &end);
	void DrawRect(CDC *pDC, CPoint one, CPoint two);
	void ZoomToBuilding();
	void UpdateRanges();
	void ClipOffset();
	void UpdateScrollPos();
	void OnLButtonDownEditShape(UINT nFlags, CPoint point);
	void OnLButtonDownEditRoad(UINT nFlags, CPoint point);
	void OnLButtonUpAddRemove(CPoint point);
	void OnLButtonUpFootprint(CPoint point);
	void OnLButtonUpRectangle(CPoint point);
	void OnLButtonUpCircle(CPoint point);
	void OnLButtonUpEditRoadNodes(CPoint point);
	CDC *GetInvertDC();
	void DoPan(CPoint point);
	void DrawPoly(CDC *pDC);
	void UpdateRectangle(CPoint point);
	void DrawRectangle(CDC *pDC);
	void InvalidatePolyExtent();
	void UpdateCircle(CPoint point);
	void DrawCircle(CDC *pDC);
	void DrawCircle(CDC *pDC, CPoint &center, int iRadius);

	void UpdateMove();
	void UpdateResizeScale();
	void UpdateRotate();
	void DrawCurrentBuilding(CDC *pDC);
	void ContrainLocationForPoly();

	void SetMode(LBMode mode);
	void CancelFootprint();
	void CancelShape();

// Generated message map functions
protected:
	//{{AFX_MSG(BExtractorView)
	afx_msg void OnColorChange();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnFunctionsConvolve();
	afx_msg void OnAddRemove();
	afx_msg void OnUpdateAddRemove(CCmdUI* pCmdUI);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnFullres();
	afx_msg void OnHand();
	afx_msg void OnUpdateHand(CCmdUI* pCmdUI);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnZoomIn();
	afx_msg void OnZoomOut();
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnClearscreenofBuildings();
	afx_msg void OnUndo();
	afx_msg void OnViewViewfullcolorimage();
	afx_msg void OnUpdateViewViewfullcolorimage(CCmdUI* pCmdUI);
	afx_msg void OnModesFootprintMode();
	afx_msg void OnUpdateModesFootprintmode(CCmdUI* pCmdUI);
	afx_msg void OnModesRectangle();
	afx_msg void OnUpdateModesRectangle(CCmdUI* pCmdUI);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnModesCircle();
	afx_msg void OnUpdateModesCircle(CCmdUI* pCmdUI);
	afx_msg void OnUpdateUndo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFunctionsConvolve(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnModesMoveresize();
	afx_msg void OnUpdateModesMoveresize(CCmdUI* pCmdUI);
	afx_msg void OnModesRoadnode();
	afx_msg void OnUpdateModesRoadnode(CCmdUI* pCmdUI);
	afx_msg void OnChangeRoadColor();
	afx_msg void OnModesRoadEdit();
	afx_msg void OnUpdateModesRoadEdit(CCmdUI* pCmdUI);
	afx_msg void OnConstrain();
	afx_msg void OnUpdateConstrain(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in BExtractorView.cpp
inline BExtractorDoc* BExtractorView::GetDocument()
   { return (BExtractorDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_B_EXTRACTORVIEW_H)
