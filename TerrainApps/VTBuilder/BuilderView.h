//
// BuilderView.h
//
// Copyright (c) 2001-2015 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

#include "ScaledView.h"
#include "UIContext.h"
#include "vtdata/vtCRS.h"
#include "vtdata/Icosa.h"

class vtLayer;
class vtRoadLayer;
class vtElevLayer;
class vtImageLayer;
class vtStructureLayer;
class vtUtilityLayer;
class LinkEdit;

class BuilderView : public vtScaledView
{
	friend class vtRoadLayer;

public:
	BuilderView(wxWindow *parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize, const wxString& name = _T(""));
	virtual ~BuilderView();

	virtual void OnPaint(wxPaintEvent& event); // overridden to draw this view

	// Cursor
	void SetCorrectCursor();

	// Mouse
	void SetMode(LBMode m);
	LBMode GetMode() { return m_ui.mode; }
	void GetMouseLocation(DPoint2 &p);

	// UTM zone boundary display
	void DrawUTMBounds();
	void DrawDymaxionOutline();

	// World Map
	void SetWMProj(const vtCRS &p);

	// Key handler
	void OnChar(wxKeyEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnMouseWheel(wxMouseEvent& event);

	// More public methods
	void DeselectAll();
	void DeleteSelected(vtRoadLayer *pRL);
	void SetActiveLayer(vtLayer *lp);
	void MatchZoomToElev(vtElevLayer *pEL);
	void MatchZoomToImage(vtImageLayer *pEL);
	void SetShowMap(bool bShow);
	bool GetShowMap() { return m_bShowMap; }
	void SetShowScaleBar(bool bShow);
	bool GetShowScaleBar() { return m_bScaleBar; }
	void InvertAreaTool(const DRECT &rect);
	void SetGridMarks(const DRECT &area, int cols, int rows, int active_col, int active_row);
	void HideGridMarks();
	void SetDistanceToolMode(bool bPath);
	bool GetDistanceToolMode();
	void SetDistancePoints(const DPoint2 &p1, const DPoint2 &p2)
	{ m_distance_p1 = p1; m_distance_p2 = p2; }
	void SetDistancePath(const DLine2 &path)
	{ m_distance_path = path; }
	void ClearDistanceTool();
	void UpdateDistance();

	void RunTest();

	bool	m_bCrossSelect;
	bool	m_bShowUTMBounds;
	bool	m_bConstrain;

protected:
	// Edit
	void UpdateResizeScale();
	void UpdateRotate();
	void OnDragDistance();

	// Elevation
	void CheckForTerrainSelect(const DPoint2 &loc);
	void HighlightArea(const DRECT &rect);

	// Pan handlers
	void BeginPan();
	void EndPan();

	// Box handlers
	void BeginBox();
	void EndBox(const wxMouseEvent& event);
	void EndBoxFeatureSelect(const wxMouseEvent& event, const DRECT &worldRect);
	void BeginArea();
	void UpdateAreaTool(const DPoint2 &delta);

	// Mouse handlers
	void OnLeftDown(wxMouseEvent& event);
	void OnLeftUp(wxMouseEvent& event);
	void OnLeftDoubleClick(wxMouseEvent& event);
	void OnMiddleDown(wxMouseEvent& event);
	void OnMiddleUp(wxMouseEvent& event);
	void OnRightDown(wxMouseEvent& event);
	void OnRightUp(wxMouseEvent& event);

	void OnLButtonClick(wxMouseEvent& event);
	void OnLButtonDragRelease(wxMouseEvent& event);
	void OnLButtonClickElement(vtRoadLayer *pRL);
	void OnLButtonClickLinkEdit(vtRoadLayer *pRL);
	void OnLButtonClickFeature(vtLayer *pL);
	void OnRightUpStructure(vtStructureLayer *pSL);

	void OnMouseMove(wxMouseEvent& event);
	void OnMouseMoveLButton(const wxPoint &point);

	void OnIdle(wxIdleEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnEraseBackground(wxEraseEvent& event);

	void DrawInverseRect(const DRECT &r, bool bDashed = false);
	void DrawInverseRect(const DPoint2 &one, const DPoint2 &two, bool bDashed = false);
	void DrawAreaTool(const DRECT &area);
	void DrawDistanceTool();
	void BeginDistance();

	bool m_bGotFirstIdle;
	bool m_bSkipNextDraw;
	bool m_bSkipNextRefresh;
	wxSize m_previous_size;
	DPoint2 m_distance_p1, m_distance_p2;
	DLine2 m_distance_path;

	// Used while mouse button is down
	bool	m_bMouseMoved;
	bool	m_bPanning;		// currently panning
	bool	m_bBoxing;		// currently drawing a rubber box
	int		m_iDragSide;	// which side of the area box being dragged

	wxCursor	*m_pCursorPan;
	bool		m_bMouseCaptured;

	// World Map
	bool			m_bShowMap;
	bool			m_bScaleBar;
	DLine2Array		WMPoly;		// Original data from SHP file
	DLine2Array		WMPolyDraw; // This is the WM that is drawn
	std::vector<DRECT>	WMPolyExtents;
	uint			m_iEntities;
	bool			m_bAttemptedLoad;
	DymaxIcosa		m_icosa;
	OCTransform		*m_pMapToCurrent, *m_pCurrentToMap;

	bool ImportWorldMap();
	void DrawWorldMap();

	// Grid marks
	bool m_bShowGridMarks;
	DRECT m_GridArea;
	int m_iGridCols, m_iGridRows, m_iActiveCol, m_iActiveRow;
	void DrawGridMarks();

	wxRect m_ScaleBarArea;

	UIContext m_ui;
	wxGLContext *m_context;

	DECLARE_EVENT_TABLE()
};

