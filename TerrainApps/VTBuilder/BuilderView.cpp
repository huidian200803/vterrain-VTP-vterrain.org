//
// BuilderView.cpp
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/shapelib/shapefil.h"
#include "vtdata/DataPath.h"
#include "vtdata/ElevationGrid.h"
#include "vtdata/vtLog.h"

#include "Options.h"
#include "BuilderView.h"
#include "Builder.h"
#include "MenuEnum.h"
#include "VTBuilder_UI.h"
// Layers
#include "ElevLayer.h"
#include "ImageLayer.h"
#include "RawLayer.h"
#include "RoadLayer.h"
#include "StructLayer.h"
#include "UtilityLayer.h"
// Dialogs
#include "vtui/DistanceDlg.h"
#include "vtui/Helper.h"	// for GuessZoneFromGeo
#include "vtui/InstanceDlg.h"

#include "cpl_error.h"
#include <float.h>

#define BOUNDADJUST 5

#include "App.h"
DECLARE_APP(BuilderApp)


////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(BuilderView, vtScaledView)
EVT_PAINT(BuilderView::OnPaint)
EVT_LEFT_DOWN(BuilderView::OnLeftDown)
EVT_LEFT_UP(BuilderView::OnLeftUp)
EVT_LEFT_DCLICK(BuilderView::OnLeftDoubleClick)
EVT_MIDDLE_DOWN(BuilderView::OnMiddleDown)
EVT_MIDDLE_UP(BuilderView::OnMiddleUp)
EVT_RIGHT_DOWN(BuilderView::OnRightDown)
EVT_RIGHT_UP(BuilderView::OnRightUp)
EVT_MOTION(BuilderView::OnMouseMove)
EVT_MOUSEWHEEL(BuilderView::OnMouseWheel)

EVT_KEY_DOWN(BuilderView::OnKeyDown)
EVT_CHAR(BuilderView::OnChar)
EVT_IDLE(BuilderView::OnIdle)
EVT_SIZE(BuilderView::OnSize)
EVT_ERASE_BACKGROUND(BuilderView::OnEraseBackground)
END_EVENT_TABLE()

/////////////////////////////////////////////////////////////////

BuilderView::BuilderView(wxWindow *parent, wxWindowID id, const wxPoint& pos,
	const wxSize& size, const wxString& name) :
		vtScaledView(parent, id, pos, size, 0, name )
{
	VTLOG(" Constructing BuilderView\n");

	m_bSkipNextRefresh = false;
	m_bGotFirstIdle = false;

	m_bCrossSelect = false;
	m_bShowMap = true;
	m_bScaleBar = false;
	m_bShowUTMBounds = false;

	m_bMouseMoved = false;
	m_bPanning = false;
	m_bBoxing = false;
	m_iDragSide = 0;
	m_bMouseCaptured = false;
	m_bShowGridMarks = false;
	m_pMapToCurrent = NULL;
	m_pCurrentToMap = NULL;

	m_ui.m_bRubber = false;
	m_ui.mode = LB_None;
	m_ui.m_bLMouseButton = m_ui.m_bMMouseButton = m_ui.m_bRMouseButton = false;
	m_ui.m_pEditingRoad = NULL;
	m_ui.m_iEditingPoint = -1;
	m_ui.m_pCurBuilding = NULL;
	m_ui.m_pCurLinear = NULL;
	m_ui.m_bDistanceToolMode = false;

	m_distance_p1.Set(0,0);
	m_distance_p2.Set(0,0);

	// Cursors are a little messy, since support is not even across platforms
#if defined(__WXMSW__)
	m_pCursorPan = new wxCursor(_T("cursors/panhand.cur"), wxBITMAP_TYPE_CUR);
#else
	// the predefined "hand" cursor isn't quite correct, since it is a image
	// of a hand with a pointing finger, not a closed, grasping hand.
	m_pCursorPan = new wxCursor(wxCURSOR_HAND);
#endif

	// world map SHP file
	m_iEntities = 0;
	m_bAttemptedLoad = false;

	m_context = new wxGLContext(this);

	SetLocalOrigin(DPoint2(40, 40));
}

BuilderView::~BuilderView()
{
	delete m_context;
	delete m_pCursorPan;
	delete m_pMapToCurrent;
	delete m_pCurrentToMap;
}

////////////////////////////////////////////////////////////
// Operations

void BuilderView::OnPaint(wxPaintEvent& event)  // overridden to draw this view
{
	if (!IsShown())
		return;

	SetCurrent(*m_context);
	wxPaintDC dc(this);

	static bool bFirstDraw = true;
	if (bFirstDraw)
	{
		bFirstDraw = false;
		VTLOG("First View OnDraw\n");
		glEnable(GL_COLOR_LOGIC_OP);
	}

	wxSize clientSize = GetClientSize();

	const float greyLevel = 0.7f;
	glClearColor(greyLevel, greyLevel, greyLevel, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);	// | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, clientSize.x, clientSize.y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	VTLOG("client size %d, %d\n", clientSize.x, clientSize.y);
	glOrtho(0.0f, clientSize.x, 0.0f, clientSize.y, 0.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glScaled(m_dScale, m_dScale, 1.0);
	glTranslated(m_offset.x, m_offset.y, 0.0);

	// Test axes
	//glColor3f(1, 0, 0);
	//DrawLine(DPoint2(0, 0), DPoint2(10, 0));
	//glColor3f(0, 1, 0);
	//DrawLine(DPoint2(0, 0), DPoint2(0, 10));
	//glColor3f(0, 0, 1);
	//DrawLine(DPoint2(0, 0), DPoint2(100, 100));

	// no point in drawing until the Idle events have made it to the splitter
	if (!m_bGotFirstIdle)
		return;

	if (g_bld->DrawDisabled())
		return;

	vtLayerPtr lp;
	const int iLayers = g_bld->NumLayers();

	// Draw 'interrupted projection outline' for current projection
	if (g_bld->GetAtCRS().IsDymaxion())
		DrawDymaxionOutline();

	// Draw the world map SHP file of country outline polys in latlon
	if (m_bShowMap)
		DrawWorldMap();

	// Draw the solid layers first
	for (int i = 0; i < iLayers; i++)
	{
		lp = g_bld->GetLayer(i);
		if (lp->GetType() != LT_IMAGE && lp->GetType() != LT_ELEVATION)
			continue;
		if (lp->GetVisible())
			lp->DrawLayer(this, m_ui);
	}
	// Then the poly/vector/point layers
	for (int i = 0; i < iLayers; i++)
	{
		lp = g_bld->GetLayer(i);
		if (lp->GetType() == LT_IMAGE || lp->GetType() == LT_ELEVATION)
			continue;
		if (lp->GetVisible())
			lp->DrawLayer(this, m_ui);
	}
	vtLayer *curr = g_bld->GetActiveLayer();
	if (curr && (curr->GetType() == LT_ELEVATION || curr->GetType() == LT_IMAGE))
	{
		DRECT rect;
		curr->GetAreaExtent(rect);
		HighlightArea(rect);
	}

	if (m_bShowUTMBounds)
		DrawUTMBounds();

	DrawAreaTool(g_bld->GetAtArea());

	if (m_bShowGridMarks)
		DrawGridMarks();

	DrawDistanceTool();

	if (m_bBoxing)
	{
		glColor3f(1, 1, 1);
		DrawInverseRect(m_ui.m_DownLocation, m_ui.m_CurLocation);
	}

	SwapBuffers();
}

void BuilderView::DrawDymaxionOutline()
{
	DLine2Array polys;

	m_icosa.GetDymaxEdges(polys);

	for (uint i = 0; i < polys.size(); i++)
	{
		DrawPolyLine(polys[i], true);
	}
}

void BuilderView::GetMouseLocation(DPoint2 &p)
{
	p = m_ui.m_CurLocation;
}

void BuilderView::SetMode(LBMode m)
{
	if (m_ui.mode == LB_Dir && m != LB_Dir)
	{
		vtRoadLayer::SetShowDirection(false);
		Refresh();
	}

	m_ui.mode = m;
	SetCorrectCursor();

	switch (m_ui.mode)
	{
	case LB_Dir:
		vtRoadLayer::SetShowDirection(true);
		Refresh();
		break;
	case LB_Node:
		if (!vtRoadLayer::GetDrawNodes()) {
			vtRoadLayer::SetDrawNodes(true);
			Refresh();
		}
		break;
	default:	// Keep picky compilers quiet.
		break;
	}

	g_bld->OnSetMode(m);

	if (m_ui.mode != LB_LinkEdit)
	{
		if (m_ui.m_pEditingRoad)
		{
			m_ui.m_pEditingRoad->m_bDrawPoints = false;
		}
		m_ui.m_pEditingRoad = NULL;
	}
}

void BuilderView::DrawUTMBounds()
{
	glColor3f(1, 0.5f, 0);		// Orange

	const vtCRS &crs = g_bld->GetAtCRS();

	int width, height;
	GetClientSize(&width, &height);

	DPoint2 ll, utm;
	wxPoint sp, array[4000];
	int zone;

	if (crs.IsGeographic())
	{
		for (zone = 0; zone < 60; zone++)
		{
			double lon = -180 + zone * 6.0;
			DrawLine(DPoint2(lon, -70.0), DPoint2(lon, 70.0));
		}
	}
	else
	{
		int zone_start = 0;
		int zone_end = 60;
		DPoint2 projectedPoint;

		vtCRS geo;
		CreateSimilarGeographicCRS(crs, geo);

		ScopedOCTransform trans1(CreateCoordTransform(&crs, &geo));

		// Avoid zones that are too far from our location.
		ClientToWorld(wxPoint(0, height/2), projectedPoint);
		trans1->Transform(1, &projectedPoint.x, &projectedPoint.y);
		zone = GuessZoneFromGeo(projectedPoint);
		if (zone-1 > zone_start) zone_start = zone-1;

		ClientToWorld(wxPoint(width, height/2), projectedPoint);
		trans1->Transform(1, &projectedPoint.x, &projectedPoint.y);
		zone = GuessZoneFromGeo(projectedPoint);
		if (zone+1 < zone_end) zone_end = zone+1;

		// Now convert the longitude lines (boundaries between the UTM zones)
		// to the current projection
		ScopedOCTransform trans2(CreateCoordTransform(&geo, &crs));

		for (int zone = zone_start; zone < zone_end; zone++)
		{
			glBegin(GL_LINE_STRIP);
			ll.x = -180.0f + zone * 6.0;
			for (ll.y = -70.0; ll.y <= 70.0; ll.y += 0.1)
			{
				projectedPoint = ll;
				trans2->Transform(1, &projectedPoint.x, &projectedPoint.y);
				SendVertex(projectedPoint);
			}
			glEnd();
		}
	}
}

//////////////////////////////////////////////////////////
// Added capability to read & display world map, poly SHP file.

// Get data out of SHP into WMPolys
bool BuilderView::ImportWorldMap()
{
	SHPHandle	hSHP;
	int			nShapeType, nShapeCount;
	uint i;
	int j, k;

	vtString fname = FindFileOnPaths(vtGetDataPath(), "WorldMap/gnv19.shp");
	if (fname == "")
		return false;

	// SHPOpen doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(fname);

	// Open SHP file
	hSHP = SHPOpen(fname_local, "rb");
	if (hSHP == NULL)
		return false;

	VTLOG(" Opened, reading worldmap.\n");
	SHPGetInfo(hSHP, &nShapeCount, &nShapeType, NULL, NULL);
	if (nShapeType != SHPT_POLYGON)
		return false;
	uint iShapeCount = nShapeCount;

	// Copy SHP data into World Map Poly data
	WMPoly.reserve(iShapeCount * 11 / 10);

	int points, start, stop;

	for (i = 0; i < iShapeCount; i++)
	{
		DPoint2 p;

		SHPObject	*psShape;
		psShape = SHPReadObject(hSHP, i);

		if (psShape->nParts > 1)
			p.Set(0,0);

		for (j = 0; j < psShape->nParts; j++)
		{
			start = psShape->panPartStart[j];
			if (j < psShape->nParts - 1)
				stop = psShape->panPartStart[j+1];
			else
				stop = psShape->nVertices;
			points = stop - start;

			DLine2 current;
			current.SetMaxSize(points);

			for (k = start; k < stop; k++)
			{
				p.x = psShape->padfX[k];
				p.y = psShape->padfY[k];
				current.Append(p);
			}
			WMPoly.push_back(current);
		}
		SHPDestroyObject(psShape);
	}

	// Close SHP file
	SHPClose(hSHP);

	// Initialize the drawn World Map WMPolyDraw to original (latlon)
	m_iEntities = (uint)WMPoly.size();
	WMPolyDraw.resize(m_iEntities);
	for (i = 0; i < m_iEntities; i++)
		WMPolyDraw[i] = WMPoly[i];

	return true;
}

void BuilderView::SetWMProj(const vtCRS &crs)
{
	uint i, j;

	if (WMPoly.size() == 0)
		return;

	const char *crs_name = crs.GetNameShort();
	if (!strcmp(crs_name, "Geo") || !strcmp(crs_name, "Unknown"))
	{
		// the data is already in latlon so just use WMPoly
		for (i = 0; i < m_iEntities; i++)
			WMPolyDraw[i] = WMPoly[i];
		return;
	}

	// Otherwise, must convert from Geo to whatever project is desired
	vtCRS Source;
	CreateSimilarGeographicCRS(crs, Source);

#if VTDEBUG
	// Check CRS text
	char *str1, *str2;
	Source.exportToWkt(&str1);
	crs.exportToWkt(&str2);
	VTLOG("World Map projection, converting:\n");
	VTLOG(" From: ");
	VTLOG(str1);
	VTLOG("\n   To: ");
	VTLOG(str2);
	OGRFree(str1);
	OGRFree(str2);

	// Check texts in PROJ4
	char *str3, *str4;
	Source.exportToProj4(&str3);
	VTLOG("\n From: ");
	VTLOG(str3);
	crs.exportToProj4(&str4);
	VTLOG("\n   To: ");
	VTLOG(str4);
	VTLOG("\n");
	OGRFree(str3);
	OGRFree(str4);
#endif

	// (Re-)create conversion object
	delete m_pMapToCurrent;
	delete m_pCurrentToMap;

	m_pMapToCurrent = CreateCoordTransform(&Source, &crs);
	m_pCurrentToMap = CreateCoordTransform(&crs, &Source);

	if (!m_pMapToCurrent)
	{
		m_bShowMap = false;
		return;
	}

	DPoint2 point;
	WMPolyExtents.resize(m_iEntities);
	for (i = 0; i < m_iEntities; i++)
	{
		// gather an extent bound for each original feature
		WMPolyExtents[i].SetInsideOut();
		WMPolyExtents[i].GrowToContainLine(WMPoly[i]);

		// and project into current CRS
		WMPolyDraw[i].Clear();
		for (j = 0; j < WMPoly[i].GetSize(); j++)
		{
			point = WMPoly[i].GetAt(j);

			int converted = m_pMapToCurrent->Transform(1, &point.x, &point.y);

			if (converted == 1)
				WMPolyDraw[i].Append(point);
		}
	}
}

void BuilderView::DrawWorldMap()
{
	if (m_iEntities == 0 && !m_bAttemptedLoad)
	{
		m_bAttemptedLoad = true;
		if (ImportWorldMap())
		{
			vtCRS crs;
			g_bld->GetCRS(crs);
			SetWMProj(crs);
		}
		else
		{
			m_bShowMap = false;
			return;
		}
	}

	glColor3f(0, 0, 0);  //solid black pen

	// Draw each poly in WMPolyDraw
	for (uint i = 0; i < m_iEntities; i++)
	{
		DrawPolyLine(WMPolyDraw[i], true);
	}
}


//////////////////////////////////////////////////////////
// Pan handlers

void BuilderView::BeginPan()
{
	VTLOG1("BeginPan\n");

	m_bPanning = true;
	SetCursor(*m_pCursorPan);

	// hide scale bar while panning
	if (m_bScaleBar)
		RefreshRect(m_ScaleBarArea);
}

void BuilderView::EndPan()
{
	VTLOG1("EndPan\n");

	m_bPanning = false;
	SetCorrectCursor();
	Refresh();
}


//////////////////////////////////////////////////////////
// Box handlers

void BuilderView::DrawInverseRect(const DRECT &r, bool bDashed)
{
	DrawInverseRect(DPoint2(r.left, r.top),
					DPoint2(r.right, r.bottom), bDashed);
}

void BuilderView::DrawInverseRect(const DPoint2 &one,
	const DPoint2 &two, bool bDashed)
{
	PushLogicOp(GL_XOR);
	if (bDashed)
	{
		glPushAttrib(GL_ENABLE_BIT);
		glLineStipple(1, 0xAAAA);
		glEnable(GL_LINE_STIPPLE);
	}
	DrawRectangle(one, two);
	if (bDashed)
	{
		glPopAttrib();
	}
}

void BuilderView::BeginBox()
{
	m_bBoxing = true;
}

void BuilderView::EndBox(const wxMouseEvent& event)
{
	m_bBoxing = false;

	if (!m_bMouseMoved)
		return;

	DRECT worldRect(m_ui.m_DownLocation, m_ui.m_LastLocation);
	switch (m_ui.mode)
	{
	case LB_Mag:
		ZoomToRect(worldRect, 0.0f);
		break;
	case LB_Box:
		g_bld->SetArea(worldRect);
		Refresh();
		break;
	case LB_Node:
	case LB_Link:
		{
			// select everything in the highlighted box.
			vtRoadLayer *pRL = g_bld->GetActiveRoadLayer();
			if (pRL->SelectArea(worldRect, (m_ui.mode == LB_Node),
				m_bCrossSelect))
			{
				Refresh();
			}
			else
				DeselectAll();
		}
		break;
	case LB_Move:
		Refresh();
		break;
	case LB_FSelect:
		EndBoxFeatureSelect(event, worldRect);
	default:	// Keep picky compilers quiet.
		break;
	}
}

void BuilderView::EndBoxFeatureSelect(const wxMouseEvent& event, const DRECT &worldRect)
{
	VTLOG1("EndBoxFeatureSelect:");

	vtLayer *pL = g_bld->GetActiveLayer();
	if (!pL) return;

	wxString verb;
	SelectionType st;
	// operation may be select, add (shift), subtract (alt), toggle (ctrl)
	if (event.ShiftDown())
	{
		st = ST_ADD;
		verb = _("Added");
	}
	else if (event.AltDown())
	{
		st = ST_SUBTRACT;
		verb = _("Subtracted");
	}
	else if (event.ControlDown())
	{
		st = ST_TOGGLE;
		verb = _("Toggled");
	}
	else
	{
		st = ST_NORMAL;
		verb = _("Selected");
	}

	int changed=0, selected=0;
	if (pL->GetType() == LT_STRUCTURE)
	{
		VTLOG1(" Doing structure box select\n");
		vtStructureLayer *pSL = (vtStructureLayer *)pL;
		changed = pSL->DoBoxSelect(worldRect, st);
		selected = pSL->NumSelected();
	}
	if (pL->GetType() == LT_RAW)
	{
		VTLOG1(" Doing raw box select\n");
		vtRawLayer *pRL = (vtRawLayer *)pL;
		changed = pRL->GetFeatureSet()->DoBoxSelect(worldRect, st);
		selected = pRL->GetFeatureSet()->NumSelected();
	}
	wxString msg;
	if (changed == 1)
		msg.Printf(_("1 entity, %d total selected"), selected);
	else
		msg.Printf(_("%d entities, %d total selected"), changed, selected);
	verb += _T(" ");
	verb += msg;
	if (g_bld->m_pParentWindow)
		g_bld->m_pParentWindow->SetStatusText(verb);

	// Log it also
	VTLOG((const char *)verb.mb_str(wxConvUTF8));

	g_bld->OnSelectionChanged();
	Refresh(false);
}

void BuilderView::DrawAreaTool(const DRECT &area)
{
	if (area.IsEmpty())
		return;

	VTLOG("DrawAreaTool %lf %lf %lf %lf\n", area.left, area.top, area.right, area.bottom);

	// dashed-line rectangle
	glColor3f(1, 1, 1);
	DrawInverseRect(area, true);	// true = Dashed.

	DPoint2 d = PixelsToWorld(2);

	// four small rectangles, for the handles at each corner
	DPoint2 bottom(area.left, area.bottom);
	DPoint2 width(area.Width(), 0.0), height(0.0, area.Height());

	DrawInverseRect(bottom - d, bottom + d);
	DrawInverseRect(bottom + width - d, bottom + width + d);
	DrawInverseRect(bottom + height - d, bottom + height + d);
	DrawInverseRect(bottom + width + height - d, bottom + width + height + d);
}

////////////////////////////////////////////////////////////

void BuilderView::DrawDistanceTool()
{
	glLogicOp(GL_INVERT);

	if (m_ui.m_bDistanceToolMode)
	{
		// Path mode
		// draw the polyline
		DrawPolyLine(m_distance_path, false);

		// draw small crosshairs
		for (uint i = 0; i < m_distance_path.GetSize(); i++)
		{
			DrawXHair(m_distance_path[i], 4);
		}
	}
	else
	{
		// Line mode
		DrawLine(m_distance_p1, m_distance_p2);

		// Draw small crosshairs
		DrawXHair(m_distance_p1, 4);
		DrawXHair(m_distance_p2, 4);
	}
	glLogicOp(GL_COPY);
}

void BuilderView::ClearDistanceTool()
{
	SetDistancePoints(DPoint2(0,0), DPoint2(0,0));
	SetDistancePath(DLine2());
	Refresh();
}

void BuilderView::UpdateDistance()
{
	if (m_ui.m_bDistanceToolMode)
		g_bld->UpdateDistance(m_distance_path);
	else
		g_bld->UpdateDistance(m_distance_p1, m_distance_p2);
}


////////////////////////////////////////////////////////////
// Elevation

void BuilderView::CheckForTerrainSelect(const DPoint2 &loc)
{
	// perhaps the user clicked on a terrain
	bool bFound = false;
	DRECT rect;
	for (uint l = 0; l < g_bld->NumLayers(); l++)
	{
		vtLayerPtr lp = g_bld->GetLayer(l);
		if (lp->GetType() != LT_ELEVATION) continue;
		vtElevLayer *t = (vtElevLayer *)lp;

		t->GetExtent(rect);
		if (rect.ContainsPoint(loc))
		{
			SetActiveLayer(t);
			bFound = true;
			break;
		}
	}
}

//
// The view needs to be notified of the new active layer to update
// the selection marks drawn around the active elevation layer.
//
void BuilderView::SetActiveLayer(vtLayerPtr lp)
{
	vtLayer *last = g_bld->GetActiveLayer();

	if (lp != last)
	{
		// Change the current layer
		g_bld->SetActiveLayer(lp, false);

		LayerType prev_type = last ? last->GetType() : LT_UNKNOWN;
		LayerType curr_type = lp ? lp->GetType() : LT_UNKNOWN;
		if (prev_type == LT_ELEVATION || prev_type == LT_IMAGE ||
			curr_type == LT_ELEVATION || curr_type == LT_IMAGE)
			Refresh();
	}
}

void BuilderView::HighlightArea(const DRECT &rect)
{
	glColor3f(1, 1, 1);	// White
	PushLogicOp(GL_XOR);
	glLineWidth(3);

	const double sx = rect.Width() / 3;
	const double sy = rect.Height() / 3;
	const DPoint2 d = PixelsToWorld(2), e = PixelsToWorld(4);

	DrawLine(rect.left - e.x, rect.top + d.y, rect.left - e.x, rect.top - sy);
	DrawLine(rect.left - d.x, rect.top + e.y, rect.left + sx, rect.top + e.y);

	DrawLine(rect.right - sx, rect.top + e.y, rect.right + e.x, rect.top + e.y);
	DrawLine(rect.right + e.x, rect.top + d.y, rect.right + e.x, rect.top - sy);
	
	DrawLine(rect.right + e.x, rect.bottom + sy, rect.right + e.x, rect.bottom - d.y);
	DrawLine(rect.right - sx, rect.bottom - e.y,  rect.right + e.x, rect.bottom - e.y);
	
	DrawLine(rect.left - e.x, rect.bottom + sy, rect.left - e.x, rect.bottom - d.y);
	DrawLine(rect.left + sx, rect.bottom - e.y,  rect.left - e.x, rect.bottom - e.y);
	glLineWidth(1);
}

////////////////////////////////////////////////////////////
// Cursor

void BuilderView::SetCorrectCursor()
{
	switch (m_ui.mode)
	{
	case LB_None:	// none
	case LB_Link:	// select/edit links
	case LB_Node:	// select/edit nodes
	case LB_Move:	// move selected nodes
		SetCursor(wxCURSOR_ARROW); break;
	case LB_Pan:	// pan the view
		SetCursor(*m_pCursorPan); break;
	case LB_Dist:	// measure distance
		SetCursor(wxCURSOR_CROSS); break;
	case LB_Mag:	// zoom into rectangle
		SetCursor(wxCURSOR_MAGNIFIER); break;
	case LB_TowerAdd: // add a tower to the location
		SetCursor(wxCURSOR_CROSS);break;
	case LB_TrimTIN:
		SetCursor(wxCURSOR_CROSS); break;
	case LB_Dir:		// show/change road direction
	case LB_LinkEdit:	// edit road points
	case LB_LinkExtend: //extend a road selection
	case LB_TSelect:
	case LB_Box:
	default:
		SetCursor(wxCURSOR_ARROW); break;
	}
}

///////////////////////////////////////////////////////////////////////
// Distance tool

void BuilderView::BeginDistance()
{
	if (m_ui.m_bDistanceToolMode)
	{
		// Path mode - set initial segment
		int len = m_distance_path.GetSize();
		if (len == 0)
		{
			// begin new path
			m_distance_path.Append(m_ui.m_DownLocation);
		}
		// default: add point to the path
		m_distance_path.Append(m_ui.m_DownLocation);

		g_bld->UpdateDistance(m_distance_path);
	}
	else
	{
		// Line mode - set initial points
		m_distance_p1 = m_ui.m_DownLocation;
		m_distance_p2 = m_ui.m_DownLocation;
	}
	Refresh();
}

/////////////////////////////////////////////////////////////

void BuilderView::BeginArea()	// in canvas coordinates
{
	DRECT area = g_bld->GetAtArea();
	if (area.IsEmpty())
	{
		BeginBox();
		return;
	}

	// check to see if they've clicked near one of the sides of the area
	m_iDragSide = 0;
	DPoint2 epsilon = PixelsToWorld(10);	// epsilon in pixels

	double d0 = abs(m_ui.m_CurLocation.x - area.left);
	double d1 = abs(m_ui.m_CurLocation.x - area.right);
	double d2 = abs(m_ui.m_CurLocation.y - area.top);
	double d3 = abs(m_ui.m_CurLocation.y - area.bottom);

	if (d0 < epsilon.x) m_iDragSide |= 1;
	if (d1 < epsilon.x) m_iDragSide |= 2;
	if (d2 < epsilon.y) m_iDragSide |= 4;
	if (d3 < epsilon.y) m_iDragSide |= 8;

	if (!m_iDragSide)
	{
		// if they click inside the box, drag it
		if (area.ContainsPoint(m_ui.m_CurLocation))
			m_iDragSide = 15;

		// if they didn't click near the box, start a new one
		else
			BeginBox();
	}
}

void BuilderView::UpdateAreaTool(const DPoint2 &delta)
{
	if (m_iDragSide & 1)
		g_bld->GetAtArea().left += delta.x;
	if (m_iDragSide & 2)
		g_bld->GetAtArea().right += delta.x;
	if (m_iDragSide & 4)
		g_bld->GetAtArea().top += delta.y;
	if (m_iDragSide & 8)
		g_bld->GetAtArea().bottom += delta.y;
}

void BuilderView::InvertAreaTool(const DRECT &rect)
{
	DrawAreaTool(rect);
}

void BuilderView::SetGridMarks(const DRECT &area, int cols, int rows,
								int active_col, int active_row)
{
	wxClientDC dc(this);
	PrepareDC(dc);

	if (cols < 1 || rows < 1)
		return;

	m_GridArea = area;
	m_iGridCols = cols;
	m_iGridRows = rows;
	m_iActiveCol = active_col;
	m_iActiveRow = active_row;
	m_bShowGridMarks = true;
}

void BuilderView::HideGridMarks()
{
	m_bShowGridMarks = false;
}

void BuilderView::DrawGridMarks()
{
#if 0 // TODO
	dc.SetPen(wxPen(*wxBLACK_PEN));
	dc.SetBrush(wxBrush(*wxBLACK_BRUSH));
	dc.SetLogicalFunction(wxINVERT);

	DPoint2 p;
	int *wx = new int[m_iGridCols+1];
	int *wy = new int[m_iGridRows+1];

	for (int x = 0; x <= m_iGridCols; x++)
		wx[x] = sx(m_GridArea.left + (m_GridArea.Width()/m_iGridCols) * x);
	for (int y = 0; y <= m_iGridRows; y++)
		wy[y] = sy(m_GridArea.bottom + (m_GridArea.Height()/m_iGridRows) * y);

	for (int x = 0; x <= m_iGridCols; x++)
		dc.DrawLine(wx[x], wy[0], wx[x], wy[m_iGridRows]);

	for (int y = 0; y <= m_iGridRows; y++)
		dc.DrawLine(wx[0], wy[y], wx[m_iGridCols], wy[y]);

	if (m_iActiveCol != -1)
	{
		dc.DrawRectangle(wx[m_iActiveCol], wy[m_iActiveRow+1],
			wx[1]-wx[0], wy[0]-wy[1]);
	}
	delete [] wx;
	delete [] wy;
#endif
}

void BuilderView::DeselectAll()
{
	vtRoadLayer *pRL = g_bld->GetActiveRoadLayer();
	if (pRL)
	{
		if (pRL->DeSelectAll())
			Refresh();
	}
	vtStructureLayer *pSL = g_bld->GetActiveStructureLayer();
	if (pSL)
	{
		pSL->DeselectAll();
		Refresh(TRUE);
	}
	vtRawLayer *pRawL = g_bld->GetActiveRawLayer();
	if (pRawL)
	{
		pRawL->GetFeatureSet()->DeselectAll();
		Refresh(TRUE);
		g_bld->OnSelectionChanged();
	}
}

void BuilderView::DeleteSelected(vtRoadLayer *pRL)
{
	// Delete the items.
	if (pRL->DeleteSelected())
		pRL->SetModified(true);

	if (pRL->RemoveUnusedNodes() != 0)
		pRL->ComputeExtents();

	Refresh();
}

void BuilderView::MatchZoomToElev(vtElevLayer *pEL)
{
	if (!pEL || !pEL->GetGrid())
		return;

	const DPoint2 &spacing = pEL->GetGrid()->GetSpacing();
	SetScale(1.0f / spacing.x);

	DPoint2 center;
	DRECT area;
	pEL->GetExtent(area);
	area.GetCenter(center);
	ZoomToPoint(center);

	Refresh();
}

void BuilderView::MatchZoomToImage(vtImageLayer *pIL)
{
	if (!pIL)
		return;

	const DPoint2 &spacing = pIL->GetSpacing();
	SetScale(1.0f / spacing.x);

	DPoint2 center;
	DRECT area;
	pIL->GetExtent(area);
	area.GetCenter(center);
	ZoomToPoint(center);

	Refresh();
}

void BuilderView::SetShowMap(bool bShow)
{
	if (bShow)
		m_bAttemptedLoad = false;
	m_bShowMap = bShow;
}

void BuilderView::SetShowScaleBar(bool bShow)
{
	m_bScaleBar = bShow;
	Refresh();
}

void BuilderView::SetDistanceToolMode(bool bPath)
{
	m_ui.m_bDistanceToolMode = bPath;
}

bool BuilderView::GetDistanceToolMode()
{
	return m_ui.m_bDistanceToolMode;
}


/////////////////////////////////////////////////////////////
// Mouse handlers

void BuilderView::OnLeftDown(wxMouseEvent& event)
{
	m_ui.m_bLMouseButton = true;
	m_bMouseMoved = false;

	// save the point where the user clicked
	m_ui.m_DownPoint = event.GetPosition();
	m_ui.m_CurPoint = m_ui.m_DownPoint;
	m_ui.m_LastPoint = m_ui.m_DownPoint;

	// "points" are in window pixels, "locations" are in the current CRS
	ClientToWorld(m_ui.m_DownPoint, m_ui.m_DownLocation);

	// Remember modifier key state
	m_ui.m_bShift = event.ShiftDown();
	m_ui.m_bControl = event.ControlDown();
	m_ui.m_bAlt = event.AltDown();

	// We must 'capture' the mouse in order to receive button-up events
	// in the case where the cursor leaves the window.
	if (!m_bMouseCaptured)
	{
		CaptureMouse();
		m_bMouseCaptured = true;
	}

	vtLayerPtr pL = g_bld->GetActiveLayer();
	switch (m_ui.mode)
	{
	case LB_TSelect:
		CheckForTerrainSelect(m_ui.m_DownLocation);
		break;

	case LB_Pan:
		BeginPan();
		break;

	case LB_Mag:
	case LB_Node:
	case LB_Link:
	case LB_FSelect:
		BeginBox();
		break;

	case LB_Box:
		BeginArea();
		break;

	case LB_Dist:
		BeginDistance();
		break;
	default:	// Keep picky compilers quiet.
		break;
	}
	// Dispatch for layer-specific handling
	if (pL)
		pL->OnLeftDown(this, m_ui);

	// Allow wxWidgets to pass the event along.  This is important because
	//  otherwise (with wx>2.4) we may not receive keyboard focus.
	event.Skip();
}

void BuilderView::OnLeftUp(wxMouseEvent& event)
{
	if (m_bMouseCaptured)
	{
		ReleaseMouse();
		m_bMouseCaptured = false;
	}

	if (!m_bMouseMoved)
		OnLButtonClick(event);

	OnLButtonDragRelease(event);

	// Dispatch for layer-specific handling
	vtLayerPtr pL = g_bld->GetActiveLayer();
	if (pL)
		pL->OnLeftUp(this, m_ui);

	m_ui.m_bLMouseButton = false;	// left mouse button no longer down
}

void BuilderView::OnLeftDoubleClick(wxMouseEvent& event)
{
	m_ui.m_DownPoint = event.GetPosition();
	m_ui.m_CurPoint = m_ui.m_LastPoint = m_ui.m_DownPoint;
	ClientToWorld(m_ui.m_DownPoint, m_ui.m_DownLocation);

	vtLayer *pL = g_bld->GetActiveLayer();
	if (pL)
		pL->OnLeftDoubleClick(this, m_ui);
}

void BuilderView::OnLButtonClick(wxMouseEvent& event)
{
	vtLayerPtr pL = g_bld->GetActiveLayer();
	if (!pL) return;

	m_ui.m_DownPoint = event.GetPosition();
	m_ui.m_CurPoint = m_ui.m_LastPoint = m_ui.m_DownPoint;
	ClientToWorld(m_ui.m_DownPoint, m_ui.m_DownLocation);

	if (pL->GetType() == LT_ROAD)
	{
		switch (m_ui.mode)
		{
			case LB_Link:
			case LB_Node:
				OnLButtonClickElement((vtRoadLayer *)pL);
				break;
		}
	}
	if (m_ui.mode == LB_FSelect)
		OnLButtonClickFeature(pL);
}

void BuilderView::OnLButtonDragRelease(wxMouseEvent& event)
{
	if (m_bPanning)
		EndPan();
	if (m_bBoxing)
		EndBox(event);

	if (m_iDragSide)
	{
		g_bld->GetAtArea().Sort();
		m_iDragSide = 0;
	}
}

void BuilderView::OnDragDistance()
{
	if (m_ui.m_bDistanceToolMode)
	{
		// Path mode
		int len = m_distance_path.GetSize();
		m_distance_path[len-1] = m_ui.m_CurLocation;
	}
	else
	{
		// Line mode
		m_distance_p2 = m_ui.m_CurLocation;
	}
	UpdateDistance();
}

void BuilderView::OnLButtonClickElement(vtRoadLayer *pRL)
{
	DRECT world_bound;

	// error is how close to the road/node can we be off by?
	DPoint2 epsilon = PixelsToWorld(5);

	bool returnVal = false;
	if (m_ui.mode == LB_Node)
		returnVal = pRL->SelectNode(m_ui.m_DownLocation, epsilon.x, world_bound);
	else if (m_ui.mode == LB_Link)
		returnVal = pRL->SelectLink(m_ui.m_DownLocation, epsilon.x, world_bound);
	else if (m_ui.mode == LB_LinkExtend)
		returnVal = pRL->SelectAndExtendLink(m_ui.m_DownLocation, epsilon.x, world_bound);

	wxString str;
	if (returnVal)
	{
		Refresh();
		if (m_ui.mode == LB_Node)
			str.Printf(_("Selected 1 Node (%d total)"), pRL->GetSelectedNodes());
		else
			str.Printf(_("Selected 1 Road (%d total)"), pRL->GetSelectedLinks());
	}
	else
	{
		DeselectAll();
		str = _("Deselected all");
	}
	if (g_bld->m_pParentWindow)
		g_bld->m_pParentWindow->SetStatusText(str);
}


void BuilderView::OnLButtonClickLinkEdit(vtRoadLayer *pRL)
{
}

void BuilderView::OnLButtonClickFeature(vtLayerPtr pL)
{
	if (pL->GetType() == LT_STRUCTURE)
	{
		vtStructureLayer *pSL = (vtStructureLayer *)pL;

		// first do a deselect-all
		pSL->DeselectAll();

		DPoint2 epsilon = PixelsToWorld(5);

		// see if there is a building at m_ui.m_DownPoint
		int building;
		double distance;
		bool found = pSL->FindClosestStructure(m_ui.m_DownLocation, epsilon.x,
				building, distance);
		if (found)
		{
			vtStructure *str = pSL->at(building);
			str->Select(!str->IsSelected());
		}
		Refresh(false);
	}
	else if (pL->GetType() == LT_UTILITY)
	{
		// TODO? single click selection of utility features
		//vtUtilityLayer *pTL = (vtUtilityLayer *)pL;
	}
	else if (pL->GetType() == LT_RAW)
	{
		// TODO? single click selection of raw features
		//vtRawLayer *pRL = (vtRawLayer *)pL;
	}
}

////////////////

void BuilderView::OnMiddleDown(wxMouseEvent& event)
{
	m_ui.m_bMMouseButton = true;
	m_bMouseMoved = false;

	// save the point where the user clicked
	m_ui.m_DownPoint = event.GetPosition();
	m_ui.m_CurPoint = m_ui.m_DownPoint;
	ClientToWorld(m_ui.m_DownPoint, m_ui.m_DownLocation);

	if (!m_bMouseCaptured)
	{
		CaptureMouse();
		m_bMouseCaptured = true;
	}

	BeginPan();
}

void BuilderView::OnMiddleUp(wxMouseEvent& event)
{
	if (m_bPanning)
		EndPan();

	if (m_bMouseCaptured)
	{
		ReleaseMouse();
		m_bMouseCaptured = false;
	}
}

void BuilderView::OnRightDown(wxMouseEvent& event)
{
	m_ui.m_bRMouseButton = true;
	if (!m_bMouseCaptured)
	{
		CaptureMouse();
		m_bMouseCaptured = true;
	}

	// Dispatch to the layer
	vtLayer *pL = g_bld->GetActiveLayer();
	if (pL)
		pL->OnRightDown(this, m_ui);
}

void BuilderView::OnRightUp(wxMouseEvent& event)
{
	m_ui.m_bRMouseButton = false;	//right mouse button no longer down
	if (m_bMouseCaptured)
	{
		ReleaseMouse();
		m_bMouseCaptured = false;
	}

	if (m_ui.mode == LB_Dist)
	{
		wxMenu *popmenu = new wxMenu;
		wxMenuItem *item;
		item = popmenu->Append(ID_DISTANCE_CLEAR, _("Clear Distance Tool"));
		wxPoint point = event.GetPosition();
		PopupMenu(popmenu, point);
		delete popmenu;
		return;
	}

	vtLayer *pL = g_bld->GetActiveLayer();
	if (!pL)
		return;

	// Dispatch to the layer
	pL->OnRightUp(this, m_ui);

	if (pL->GetType() == LT_STRUCTURE)
		OnRightUpStructure((vtStructureLayer *)pL);
}

void BuilderView::OnRightUpStructure(vtStructureLayer *pSL)
{
	pSL->EditBuildingProperties();
}

void BuilderView::OnMouseMove(wxMouseEvent& event)
{
	wxPoint pointp = event.GetPosition();

	m_ui.m_CurPoint = event.GetPosition();
	ClientToWorld(m_ui.m_CurPoint, m_ui.m_CurLocation);

	if (m_ui.m_bLMouseButton || m_ui.m_bMMouseButton || m_ui.m_bRMouseButton)
	{
		wxPoint diff = m_ui.m_CurPoint - m_ui.m_DownPoint;
		int mag = abs(diff.x) + abs(diff.y);
		if (mag > 2 && !m_bMouseMoved)
			m_bMouseMoved = true;
	}

	if (m_bPanning)
	{
		wxPoint diff = m_ui.m_CurPoint - m_ui.m_LastPoint;
		m_offset += DPoint2(diff.x / m_dScale, -diff.y / m_dScale);
		Refresh();
	}

	// left button click and drag
	if (m_ui.m_bLMouseButton)
	{
		if (m_bBoxing)
		{
			Refresh();
		}
		if (m_iDragSide)
		{
			UpdateAreaTool(m_ui.m_CurLocation - m_ui.m_LastLocation);
			Refresh();
		}
		if (m_ui.mode == LB_Dist)
		{
			OnDragDistance();		// update
			Refresh();
		}
		else if (m_ui.mode == LB_BldEdit && m_ui.m_bRubber)
		{
		}
	}

	// Dispatch for layer-specific handling
	vtLayerPtr pL = g_bld->GetActiveLayer();
	if (pL)
		pL->OnMouseMove(this, m_ui);

	// update new mouse coordinates, etc. in status bar
	g_bld->RefreshStatusBar();
	if (g_bld->m_pInstanceDlg)
		g_bld->m_pInstanceDlg->SetLocation(m_ui.m_CurLocation);

	m_ui.m_LastPoint = m_ui.m_CurPoint;
	m_ui.m_LastLocation = m_ui.m_CurLocation;
}

void BuilderView::OnMouseWheel(wxMouseEvent& event)
{
	VTLOG("OnMouseWheel %d\n", event.m_wheelRotation);
	if (event.m_wheelRotation > 0)
		ScaleAroundPoint(m_ui.m_CurLocation, GetScale() * sqrt(2.0));
	else
		ScaleAroundPoint(m_ui.m_CurLocation, GetScale() / sqrt(2.0));

	// update scale in status bar
	g_bld->RefreshStatusBar();
}

void BuilderView::OnIdle(wxIdleEvent& event)
{
	// Prevent re-entrance, which may happen on Idle -> ProgressDialog -> Idle
	static bool s_in = false;
	if (s_in) return;
	s_in = true;

	if (!m_bGotFirstIdle)
	{
		m_bGotFirstIdle = true;
		VTLOG("First View Idle\n");
		g_bld->ZoomAll();
		Refresh();
		// wxGetApp().Exit();	// handy for testing memleaks
	}

	int i, iLayers = g_bld->NumLayers();

	// Check to see if any elevation layers needs drawing
	bool bNeedDraw = false;
	bool bDrew = false;
	for (i = 0; i < iLayers; i++)
	{
		vtLayer *lp = g_bld->GetLayer(i);
		if (lp->GetType() == LT_ELEVATION)
		{
			vtElevLayer *pEL = (vtElevLayer *)lp;
			if (pEL->m_draw.m_bShowElevation && pEL->NeedsDraw())
				bNeedDraw = true;
		}
	}
	if (bNeedDraw)
	{
		OpenProgressDialog2(_("Rendering elevation layers"), false);
		for (i = 0; i < iLayers; i++)
		{
			vtLayer *lp = g_bld->GetLayer(i);
			UpdateProgressDialog2(i * 99 / iLayers, 0, lp->GetLayerFilename());
			if (lp->GetType() == LT_ELEVATION)
			{
				vtElevLayer *pEL = (vtElevLayer *)lp;
				if (pEL->m_draw.m_bShowElevation && pEL->NeedsDraw())
				{
					pEL->RenderBitmap();
				}
			}
		}
		CloseProgressDialog2();
		Refresh(true);
	}
	s_in = false;
}

void BuilderView::OnSize(wxSizeEvent& event)
{
	// Remember it.
	m_clientSize = event.GetSize();

	// Attempt to avoid unnecessary redraws on shrinking the window.
	// Unfortunately using Skip() alone appears to have no effect,
	//  we still get the Refresh-Draw event.
	wxSize size = GetSize();
	//VTLOG("View OnSize %d, %d\n", size.x, size.y);
	if (size == m_previous_size)
		event.Skip(true);	// allow event to be handled normally
	else
	{
		if (m_bScaleBar)
			RefreshRect(m_ScaleBarArea);
		if (size.x <= m_previous_size.x && size.y <= m_previous_size.y && m_bGotFirstIdle)
		{
			// "prevent additional event handlers from being called and control
			// will be returned to the sender of the event immediately after the
			// current handler has finished."
			event.Skip(false);

			// Since that doesn't work, we use our own logic
			m_bSkipNextRefresh = true;
		}
		else
			event.Skip(true);	// allow event to be handled normally
	}
	m_previous_size = size;
}


//////////////////
// Keyboard shortcuts

#include <map>

void BuilderView::OnChar(wxKeyEvent& event)
{
#if VTDEBUG
	VTLOG("Char %d (%c) ctrl:%d\n", event.GetKeyCode(), event.GetKeyCode(), event.ControlDown());
#endif

	bool ctrl = event.ControlDown();
	int code = event.GetKeyCode();

	if (code == ' ')
	{
		SetMode(LB_Pan);
		SetCorrectCursor();
	}
	else if (code == WXK_ADD && ctrl)
	{
		SetScale(GetScale() * sqrt(2.0));
	}
	else if (code == WXK_SUBTRACT && ctrl)
	{
		SetScale(GetScale() / sqrt(2.0));
	}
	else
		event.Skip();
}

void BuilderView::RunTest()
{
	// a place to put quick hacks and tests
#if 0
	vtRoadLayer *pR = (vtRoadLayer *)g_bld->FindLayerOfType(LT_ROAD);
	vtElevLayer *pE = (vtElevLayer *)g_bld->FindLayerOfType(LT_ELEVATION);
	pR->CarveRoadway(pE, 2.0);
#endif
#if 0
	vtElevLayer *pE = (vtElevLayer *)g_bld->FindLayerOfType(LT_ELEVATION);
	if (pE)
	{
		vtElevationGrid *g = pE->m_pGrid;
		int xs, zs;
		g->GetDimensions(xs, zs);
		for (int i = 0; i < xs; i++)
			for (int j = 0; j < zs; j++)
			{
				float val = g->GetFValue(i, j);
				val += (cos(j*0.02) + cos(i*0.02))*5;
				g->SetFValue(i, j, val);
			}
			g->ComputeHeightExtents();
			pE->SetModified(true);
			pE->ReRender();
	}
#endif
#if 0
	vtString dir = "E:/Data-Distro/Culture/UtilityStructures";
	for (dir_iter it((const char *)dir); it != dir_iter(); ++it)
	{
		if (it.is_directory())
			continue;
		vtString name = it.filename().c_str();
		if (name.Find(".obj") == -1)
			continue;
		FILE *in = vtFileOpen(dir + "/" + name, "rb");
		FILE *out = vtFileOpen(dir + "/" + name+"2", "wb");
		if (!in || !out)
			continue;
		char buf[99];
		double x, y, z;
		while (fgets(buf, 99, in))
		{
			if (buf[0] == 'v')
			{
				sscanf(buf, "v %lf %lf %lf", &x, &y, &z);
				fprintf(out, "v %lf %lf %lf\n", x, z, -y);
			}
			else
				fputs(buf, out);
		}
		fclose(out);
		fclose(in);
	}
#endif
#if 0
	{
		// create grid of polygons
		vtFeatureSetPolygon set;
		vtCRS crs;
		crs.SetWellKnownGeogCS("NAD83");
		crs.SetUTMZone(5);
		set.SetCRS(crs);
		DPoint2 base(215500, 2213000), spacing(1000,1000);
		for (int i = 0; i < 12; i++)
		{
			for (int j = 0; j < 6; j++)
			{
				DLine2 dline;
				dline.Append(base + DPoint2(i*spacing.x, j*spacing.y));
				dline.Append(base + DPoint2((i+1)*spacing.x, j*spacing.y));
				dline.Append(base + DPoint2((i+1)*spacing.x, (j+1)*spacing.y));
				dline.Append(base + DPoint2(i*spacing.x, (j+1)*spacing.y));
				DPolygon2 poly;
				poly.push_back(dline);
				set.AddPolygon(poly);
			}
		}
		set.SaveToSHP("C:/Temp/waimea_quads.shp");
	}
#endif
#if 0
	DLine2 dline;
	dline.Append(DPoint2(0,0));
	dline.Append(DPoint2(1,0));
	dline.Append(DPoint2(1,1));
	dline.Append(DPoint2(0,1));
	vtStructureArray str;
	vtBuilding *bld = str.NewBuilding();
	bld->SetFootprint(0, dline);
	bld->SetNumStories(2);
	bld->SetRoofType(RT_HIP);
	bld->SetColor(BLD_BASIC, RGBi(255,0,0))
		bld->SetColor(BLD_ROOF, RGBi(255,255,255))
#endif
#if 0
		double left=0.00000000000000000;
	double top=0.0052590002305805683;
	double right=0.0070670000277459621;
	double bottom=0.00000000000000000;

	double ScaleX = vtCRS::GeodesicDistance(DPoint2(left,bottom),DPoint2(right,bottom));
	double foo = ScaleX;
#endif
#if 0
	wxString pname = _T("G:/Data-Charsettest/Temp");
	wxString filename;
	wxDir dir(pname);
	dir.GetFirst(&filename);
	bool result = wxFile::Access(pname + _T("/") + filename, wxFile::read);
	if (result)
		VTLOG("success\n");
#endif
#if 0
	ReqContext con;

	IPoint2 base(8838, 7430);
	IPoint2 size(50, 30);
	vtDIB output;
	output.Create(size.x*258, size.y*258, 24);
	for (int x = 0; x < size.x; x++)
	{
		for (int y = 0; y < size.y; y++)
		{
			int xx = base.x + x;
			int yy = base.y + y;
			vtBytes data;
			vtString url;
			url.Format("http://us.maps3.yimg.com/aerial.maps.yimg.com/tile?v=1.4&t=a&x=%d&y=%d&z=1",
				xx, yy);
			VTLOG1(url + "\n");
			bool result = con.GetURL(url, data);
			if (!result)
				continue;

			vtString fname;
			fname.Format("c:/temp/tile_%04d_%04d.jpg", xx, yy);
			FILE *fp = fopen(fname, "wb");
			fwrite(data.Get(), data.Len(), 1, fp);
			fclose(fp);

			vtDIB tile;
			if (tile.ReadJPEG(fname))
				tile.BlitTo(output, x * 258, (size.y - 1 - y) * 258);
		}
	}
	VTLOG1("Writing output\n");
	output.WriteBMP("c:/temp/output.bmp");
#endif
#if 0
	#include "C:/Dev/TMK-Process/TMK.cpp"
#endif
#if 0
	{
		// create grid of points over current layer
		vtFeatureSetPoint2D set;
		vtCRS crs;
		set.SetCRS(g_bld->GetAtCRS());
		DRECT area = g_bld->m_area;
		set.AddField("filename", FT_String, 30);
		set.AddField("rotation", FT_Float);

		DPoint2 spacing(area.Width()/21, area.Height()/21);
		for (int i = 0; i < 22; i++)
		{
			for (int j = 0; j < 22; j++)
			{
				DPoint2 p;
				p.x = area.left + i*spacing.x;
				p.y = area.bottom + j*spacing.y;
				int rec = set.AddPoint(p);
				set.SetValue(rec, 0, "C:/temp/triangle.osg");
				set.SetValue(rec, 1, 110 + (i * 5) - (j * 2));
			}
		}
		set.SaveToSHP("C:/Temp/PearlRiverPoints.shp");
	}
#endif
#if 0
	{
		vtStructureArray *sa = new vtStructureArray;
		sa->m_crs.SetGeogCSFromDatum(EPSG_DATUM_WGS84);
		// 1557 buildings
		sa->ReadXML("G:/Data-USA/Data-Hawaii/BuildingData/stage5.vtst");
		//sa->ReadXML("G:/Data-USA/Data-Hawaii/BuildingData/one_building.vtst");
	}
#endif
#if 0
	{
		vtRawLayer *ab = g_bld->GetActiveRawLayer();
		vtFeatureSetLineString3D *fe3;
		fe3 = dynamic_cast<vtFeatureSetLineString3D*>(ab->GetFeatureSet());
		if (fe3)
		{
			DLine3 &line = fe3->GetPolyLine(0);
			for (int i = 0; i < line.GetSize(); i++)
			{
				DPoint3 p = line[i];
				p.z -= 2000;
				line[i] = p;
			}
		}
	}
#endif
#if 0
	{
		vtStructureLayer *pL = (vtStructureLayer *)g_bld->FindLayerOfType(LT_STRUCTURE);
		if (pL)
		{
			pL->DeselectAll();
			vtStructure *str = pL->GetAt(868);
			if (str)
			{
				str->Select(true);
				DRECT r;
				str->GetExtents(r);
				ZoomToRect(r, 0.1f);
			}
		}
	}
#endif
#if 0
	vtRawLayer *pRaw = g_bld->GetActiveRawLayer();
	if (!pRaw) return;
	vtFeatureSetPolygon *fsp = (vtFeatureSetPolygon*) pRaw->GetFeatureSet();
	int fixed = fsp->SelectBadFeatures(0.05);	// 5 cm
	Refresh();
#endif
}

void BuilderView::OnKeyDown(wxKeyEvent& event)
{
	int code = event.GetKeyCode();
	bool ctrl = event.ControlDown();
#if VTDEBUG
//	VTLOG("View: KeyDown %d (%c) ctrl:%d\n", code, event.GetKeyCode(), ctrl);
#endif

#if 0
	wxCommandEvent dummy;

	// Some accelerators aren't caught properly (at least on Windows)
	//  So, explicitly check for them here.
	if (code == 43 && ctrl)
		g_bld->OnViewZoomIn(dummy);
	else if (code == 45 && ctrl)
		g_bld->OnViewZoomOut(dummy);

	else
		event.Skip();
#endif
}

void BuilderView::OnEraseBackground( wxEraseEvent& event )
{
	// there are some erase events we don't need, such as when sizing the
	//  window smaller
	if (m_bSkipNextRefresh)
		event.Skip(false);
	else
		event.Skip(true);
	m_bSkipNextRefresh = false;
}
