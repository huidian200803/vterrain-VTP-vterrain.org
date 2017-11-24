//
// LODDlg.cpp
//
// Copyright (c) 2005-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/spinctrl.h"

#include "vtlib/vtlib.h"
#include "vtlib/core/TiledGeom.h"
#include "vtlib/core/PagedLodGrid.h"
#include "vtlib/core/Terrain.h"

#include "vtui/Helper.h"	// for MakeColorBitmap

#include "../Enviro.h"

#include "LODDlg.h"
#include "EnviroFrame.h"

#include <mini/mini.h>
#include <mini/miniload.h>
#include <mini/datacloud.h>

// WDR: class implementations

//----------------------------------------------------------------------------
// LODDlg
//----------------------------------------------------------------------------

// WDR: event table for LODDlg

BEGIN_EVENT_TABLE(LODDlg, PagingDlgBase)
	EVT_INIT_DIALOG (LODDlg::OnInitDialog)
	EVT_SPIN_UP( ID_SPIN, LODDlg::OnSpinTargetUp )
	EVT_SPIN_DOWN( ID_SPIN, LODDlg::OnSpinTargetDown )
	EVT_TEXT( ID_TARGET, LODDlg::OnTarget )
	EVT_TEXT( ID_TEXT_PRANGE, LODDlg::OnText )
	EVT_SLIDER( ID_SLIDER_PRANGE, LODDlg::OnRangeSlider )
	EVT_TEXT( ID_TEXT_PAGEOUT, LODDlg::OnText )
	EVT_SLIDER( ID_SLIDER_PAGEOUT, LODDlg::OnRangeSlider )
END_EVENT_TABLE()

LODDlg::LODDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	PagingDlgBase( parent, id, title, position, size, style )
{
	m_bSet = false;
	m_bHaveRange = false;
	m_bHaveRangeVal = false;
	m_iCountCur = 0;
	m_iCountMax = 0;
	m_fPageout = 0.0f;
	m_fRange = 0.0f;
	m_bShowTilesetStatus = false;

	m_pTerrain = NULL;

	// Work around wxFormDesigner's lack of support for limiting to smallest size
	GetSizer()->SetSizeHints(this);

	// make sure that validation gets down to the child windows
	// including the children of the notebook
	GetNotebook()->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);

	GetTileStatus()->SetValue(_T("No paging threads"));

	m_iTarget = 0;
	AddValidator(this, ID_TARGET, &m_iTarget);

	AddNumValidator(this, ID_TEXT_PRANGE, &m_fRange);
	AddValidator(this, ID_SLIDER_PRANGE, &m_iRange);

	AddNumValidator(this, ID_TEXT_PAGEOUT, &m_fPageout);
	AddValidator(this, ID_SLIDER_PAGEOUT, &m_iPageout);

	AddNumValidator(this, ID_COUNT_CURRENT, &m_iCountCur);
	AddNumValidator(this, ID_COUNT_MAXIMUM, &m_iCountMax);
}

float PRANGE_MIN = 0.0f;
float PRANGE_RANGE = 100.0f;

#define DIST_MIN 2.0f	// 100 m
#define DIST_MAX 4.0f	// 10 km
#define DIST_RANGE (DIST_MAX-DIST_MIN)

void LODDlg::SetPagingRange(float fmin, float fmax)
{
	PRANGE_MIN = fmin;
	PRANGE_RANGE = fmax - fmin;
	m_bHaveRange = true;
}

void LODDlg::Refresh(float res0, float res, float res1, int target,
					 int count, float prange)
{
	bool bNeedRefresh = false;
	if (target != m_iTarget)
	{
		m_iTarget = target;
		bNeedRefresh = true;
	}

	// don't bother updating if window isn't shown
	if (!IsShown())
		return;

	// Only on the first notebook page
	if (GetNotebook()->GetSelection() != 0)
		return;

	// Hide the tileset status if our current terrain has no tileset.
	bool bshow = (prange != -1);
	GetTilesetBox()->GetContainingSizer()->Show(bshow);
	if (bshow != m_bShowTilesetStatus)
	{
		Layout();
		GetSizer()->Fit( this );
		m_bShowTilesetStatus = bshow;
	}

	wxString str;
	if (bNeedRefresh)
	{
		str.Printf(_T("%d"), m_iTarget);
		m_bSet = true;
		GetTarget()->SetValue(str);
		m_bSet = false;
	}

	str.Printf(_T("%d"), count);
	GetCurrent()->SetValue(str);

	if (m_bHaveRange && prange != m_fRange)
	{
		m_fRange = prange;
		ValuesToSliders();
		m_bSet = true;
		TransferDataToWindow();
		m_bSet = false;
		m_bHaveRangeVal = true;
	}

	// Now draw the chart
	DrawLODChart(res0, res, res1, target, count);
}

void LODDlg::SlidersToValues()
{
	m_fRange = PRANGE_MIN + (m_iRange * PRANGE_RANGE / 100);
	m_fPageout = powf(10, (DIST_MIN + m_iPageout * DIST_RANGE / 200));
}

void LODDlg::ValuesToSliders()
{
	m_iRange = (int) ((m_fRange - PRANGE_MIN) / PRANGE_RANGE * 100);
	m_iPageout = (int) ((log10f(m_fPageout) - DIST_MIN) / DIST_RANGE * 200);
}

void LODDlg::OnText( wxCommandEvent &event )
{
	if (m_bSet)
		return;
	TransferDataFromWindow();
	ValuesToSliders();

	if (m_bHaveRangeVal)
		m_pFrame->ChangePagingRange(m_fRange);
	if (m_pTerrain)
		m_pTerrain->SetStructurePageOutDistance(m_fPageout);

	m_bSet = true;
	TransferDataToWindow();
	m_bSet = false;
}

void LODDlg::OnRangeSlider( wxCommandEvent &event )
{
	if (m_bSet)
		return;
	TransferDataFromWindow();
	SlidersToValues();

	if (m_bHaveRangeVal)
		m_pFrame->ChangePagingRange(m_fRange);
	if (m_pTerrain)
		m_pTerrain->SetStructurePageOutDistance(m_fPageout);

	m_bSet = true;
	TransferDataToWindow();
	m_bSet = false;
}

void LODDlg::DrawLODChart(float res0, float res, float res1, int target, int count)
{
	int ires = (int) res;
	int imax = target * 2;
	int itarget = target * 300 / imax;
	int icount = count * 300 / imax;
	int range = (target / 10);
	int irange1 = (target-range) * 300 / imax;
	int irange2 = (target+range) * 300 / imax;

	wxPanel *panel = GetPanel1();
	wxClientDC dc(panel);
	PrepareDC(dc);
	dc.Clear();

	if (res0 != -1)
	{
		int ires0 = (int) res0;
		int ires1 = (int) res1;

		wxPen pen1(wxColour(255,0,0), 1, wxSOLID);	// red
		dc.SetPen(pen1);

		dc.DrawLine(ires0-10, 0, ires0, 10);
		dc.DrawLine(ires0, 10, ires0-10, 20);

		dc.DrawLine(ires1+10, 0, ires1, 10);
		dc.DrawLine(ires1, 10, ires1+10, 20);
	}

	wxPen pen2(wxColour(0,0,255), 3, wxSOLID);	// blue
	dc.SetPen(pen2);

	dc.DrawLine(ires, 0, ires, 20);

	wxPen pen3(wxColour(0,128,0), 1, wxSOLID);	// green
	dc.SetPen(pen3);

	dc.DrawLine(irange1-10, 20, irange1, 30);
	dc.DrawLine(irange1, 30, irange1-10, 40);

	dc.DrawLine(itarget, 20, itarget, 40);

	dc.DrawLine(irange2+10, 20, irange2, 30);
	dc.DrawLine(irange2, 30, irange2+10, 40);

	dc.DrawLine(irange1, 30, irange2, 30);

	wxPen pen4(wxColour(255,0,255), 3, wxSOLID);	// purple
	dc.SetPen(pen4);

	dc.DrawLine(icount, 20, icount, 40);
}

void LODDlg::DrawTilesetState(vtTiledGeom *tg, vtCamera *cam)
{
	// don't bother updating if window isn't shown
	if (!IsShown())
		return;

	// Only on the first notebook page
	if (GetNotebook()->GetSelection() != 0)
		return;

	wxPanel *panel = GetPanel2();
	wxClientDC dc(panel);
	PrepareDC(dc);

	//wxPen pen3(wxColour(0,128,0), 1, wxSOLID);	// green
	//dc.SetPen(pen3);
	wxBrush b1(wxColour(180,180,180), wxSOLID);
	wxBrush b2(wxColour(255,100,255), wxSOLID);
	wxBrush b3(wxColour(255,0,0), wxSOLID);
	wxBrush b4(wxColour(255,128,0), wxSOLID);
	wxBrush b5(wxColour(255,255,0), wxSOLID);
	wxBrush b6(wxColour(0,255,0), wxSOLID);
	wxBrush b7(wxColour(0,255,255), wxSOLID);
	wxBrush b8(wxColour(0,0,255), wxSOLID);
	wxBrush bwhite(wxColour(255,255,255), wxSOLID);

	//wxPen p1(wxColour(255,0,0), 2, wxSOLID);
	//wxPen p2(wxColour(255,128,0), 2, wxSOLID);
	//wxPen p3(wxColour(255,255,0), 2, wxSOLID);
	//wxPen p4(wxColour(0,255,0), 2, wxSOLID);
	//wxPen p5(wxColour(0,255,255), 2, wxSOLID);
	//wxPen p6(wxColour(0,0,255), 2, wxSOLID);
	//wxPen p7(wxColour(255,0,255), 2, wxSOLID);
	//wxPen pwhite(wxColour(255,255,255), 2, wxSOLID);
	wxPen pblack(wxColour(0,0,0), 1, wxSOLID);

	// draw rectangles for texture state
	minitile *mt = tg->GetMiniTile();
	wxSize size = panel->GetSize();
	int border = 20;
	int sx = (size.x - border*2) / tg->cols;
	int sy = (size.y - border*2) / tg->rows;

	dc.Clear();
	for (int i = 0; i < tg->cols; i++)
	{
		for (int j = 0; j < tg->rows; j++)
		{
			int t = mt->gettexw(i,j);
			switch (t)
			{
			case 16: dc.SetBrush(b1); break;
			case 32: dc.SetBrush(b2); break;
			case 64: dc.SetBrush(b3); break;
			case 128: dc.SetBrush(b4); break;
			case 256: dc.SetBrush(b5); break;
			case 512: dc.SetBrush(b6); break;
			case 1024: dc.SetBrush(b7); break;
			case 2048: dc.SetBrush(b8); break;
			default: dc.SetBrush(bwhite); break;
			}
			/*
			int s = mt->getsize(i,j);
			switch (s)
			{
			case 65: dc.SetPen(p1); break;
			case 129: dc.SetPen(p2); break;
			case 257: dc.SetPen(p3); break;
			case 513: dc.SetPen(p4); break;
			case 1025: dc.SetPen(p5); break;
			case 2049: dc.SetPen(p6); break;
			default: dc.SetPen(pwhite); break;
			}
			*/
			dc.DrawRectangle(border + i*sx, border + j*sy, sx-1, sy-1);
		}
	}
	// draw camera FOV
	FPoint3 p = cam->GetTrans();
	float fx = p.x / tg->coldim;
	float fy = tg->rows + (p.z / tg->rowdim);
	int csx = border + (int)(fx * sx);
	int csy = border + (int)(fy * sy);
	dc.SetPen(pblack);
	dc.DrawLine(csx - 10, csy, csx + 10, csy);
	dc.DrawLine(csx, csy - 10, csx, csy + 10);

	datacloud *cloud = tg->GetDataCloud();
	if (cloud)
	{
		double mem = cloud->getmem();
		wxString str;
		str.Printf(_T("Mem %.1f MB, Total %d, Pending %d"),
			mem, cloud->gettotal(), cloud->getpending());
		GetTileStatus()->SetValue(str);
	}

	// These always return the whole extents, for some reason
	int left = mt->getvisibleleft();
	int right = mt->getvisibleright();
	int bottom = mt->getvisiblebottom();
	int top = mt->getvisibletop();
}

void LODDlg::DrawStructureState(vtPagedStructureLodGrid *grid, float fPageOutDist)
{
	// don't bother updating if window isn't shown
	if (!IsShown())
		return;

	// Only on the second notebook page
	if (GetNotebook()->GetSelection() != 1)
		return;

	//
	if (m_pTerrain)
	{
		float prev1 = m_fPageout;
		int prev2 = m_iCountCur;
		int prev3 = m_iCountMax;
		m_fPageout = m_pTerrain->GetStructurePageOutDistance();
		m_iCountCur = grid->GetTotalConstructed();
		m_iCountMax = m_pTerrain->GetStructurePageMax();
		if (prev1 != m_fPageout || prev2 != m_iCountCur || prev3 != m_iCountMax)
		{
			ValuesToSliders();
			m_bSet = true;
			TransferDataToWindow();
			m_bSet = false;
		}
	}

	static float last_draw = 0.0f;
	float curtime = vtGetTime();
	if (curtime - last_draw < 0.1f)	// 10 time a second
		return;
	last_draw = curtime;

	int dim = grid->GetDimension();

	wxPanel *panel = GetPanel3();
	wxClientDC dc(panel);
	PrepareDC(dc);
	dc.SetFont(*wxSWISS_FONT);
	dc.Clear();

	wxBrush bwhite(wxColour(255,255,255), wxSOLID);
	wxPen pwhite(wxColour(255,255,255), 2, wxSOLID);
	wxPen pblack(wxColour(0,0,0), 1, wxSOLID);
	wxPen pgreen(wxColour(0,128,0), 1, wxSOLID);
	wxPen pblue(wxColour(0,0,255), 1, wxSOLID);
	wxPen pred(wxColour(255,0,0), 1, wxSOLID);

	// Consider camera FOV
	vtCamera *cam = vtGetScene()->GetCamera();
	FPoint3 cam_pos = cam->GetTrans();
	FPoint3 cam_dir = cam->GetDirection();

	FPoint3 csize = grid->GetCellSize();
	float fx = cam_pos.x / csize.x;
	float fy = /*(dim-1) -*/ (cam_pos.z / csize.z);

	wxSize panelsize = panel->GetSize();
	int cellx = (panelsize.x / 56);
	int celly = (panelsize.y / 70);

	int startx = (int)(fx - cellx);
	int starty = (int)(fy - celly);
	if (startx < 0) startx = 0;
	if (starty < 0) starty = 0;

	// draw rectangles for Structure state
	dc.SetPen(pblack);
	dc.SetBrush(*wxTRANSPARENT_BRUSH);

	int border = 10;
	int sx = (panelsize.x - border*2) / (cellx*2);
	int sy = (panelsize.y - border*2) / (celly*2);

	wxString msg;
	dc.SetPen(pblack);
	for (int i = startx; i < (startx+cellx*2) && i < dim; i++)
	{
		int drawx = border + (i-startx)*sx;
		for (int j = starty; j < (starty+celly*2) && j < dim; j++)
		{
			int drawy = panelsize.y - (border + (j-starty)*sy);
			dc.DrawRectangle(drawx, drawy-sy, sx-1, sy-1);
		}
	}
	for (int i = startx; i < (startx+cellx*2) && i < dim; i++)
	{
		int drawx = border + (i-startx)*sx;
		for (int j = starty; j < (starty+celly*2) && j < dim; j++)
		{
			vtPagedStructureLOD *cell = grid->GetPagedCell(i, j);
			int drawy = panelsize.y - (border + (j-starty)*sy);
			if (cell)
			{
				int num_con = cell->m_iNumConstructed;
				int num_tot = cell->m_StructureRefs.size();
				if (num_con == num_tot)
				{
					// all built
					dc.SetTextForeground(wxColour(0,128,0));
					msg.Printf(_T("%d"), num_con);
					dc.DrawText(msg, drawx+4, drawy - 25);
				}
				else if (num_con > 0 && num_con < num_tot)
				{
					// partially built
					dc.SetTextForeground(wxColour(0,128,0));
					msg.Printf(_T("%d"), num_con);
					dc.DrawText(msg, drawx+4, drawy - 33);

					dc.SetTextForeground(wxColour(80,0,80));
					msg.Printf(_T("%d"), num_tot-num_con);
					dc.DrawText(msg, drawx+4, drawy - 19);
				}
				else
				{
					// not at all built
					dc.SetTextForeground(wxColour(80,0,80));
					msg.Printf(_T("%d"), num_tot);
					dc.DrawText(msg, drawx+4, drawy - 25);
				}
			}
		}
	}

	// Draw camera location as blue crosshair
	dc.SetPen(pblue);
	int csx = border + (int)((fx-startx) * sx);
	int csy = panelsize.y - (border + (int)((fy-starty) * sy));
	dc.DrawLine(csx - 10, csy, csx + 10, csy);
	dc.DrawLine(csx, csy - 10, csx, csy + 10);

	// Draw LOD range as blue circle
	float lod_dist = grid->GetDistance();
	int circle_xsize = (int) (lod_dist / csize.x * sx);
	int circle_ysize = (int) (lod_dist / (-csize.z) * sy);
	dc.DrawEllipse(csx - circle_xsize, csy - circle_ysize,
		circle_xsize*2, circle_ysize*2);

	// Draw frustum (FOV)
	float fov = cam->GetFOV();
	cam_dir.y = 0;
	cam_dir.Normalize();
	cam_dir *= (lod_dist * 0.9f);

	FPoint2 vec(cam_dir.x, cam_dir.z);
	FPoint2 vecrot = vec;
	vecrot.Rotate(fov/2);
	int vx, vy;

	vx = (int) (vecrot.x / csize.x * sx);
	vy = (int) (vecrot.y / (-csize.z) * sy);
	dc.DrawLine(csx, csy, csx+vx, csy+vy);

	vecrot = vec;
	vecrot.Rotate(-fov/2);

	vx = (int) (vecrot.x / csize.x * sx);
	vy = (int) (vecrot.y / (-csize.z) * sy);
	dc.DrawLine(csx, csy, csx+vx, csy+vy);

	// Draw page-out range as red circle
	dc.SetPen(pred);
	circle_xsize = (int) (fPageOutDist / csize.x * sx);
	circle_ysize = (int) (fPageOutDist / (-csize.z) * sy);
	dc.DrawEllipse(csx - circle_xsize, csy - circle_ysize,
		circle_xsize*2, circle_ysize*2);
}

// WDR: handler implementations for LODDlg

void LODDlg::OnInitDialog(wxInitDialogEvent& event)
{
	m_bSet = true;
	wxDialog::OnInitDialog(event);  // calls TransferDataToWindow
	m_bSet = false;
}

void LODDlg::OnSpinTargetUp( wxSpinEvent &event )
{
	m_pFrame->SetTerrainDetail(m_pFrame->GetTerrainDetail()+1000);
	event.Veto();
}

void LODDlg::OnSpinTargetDown( wxSpinEvent &event )
{
	m_pFrame->SetTerrainDetail(m_pFrame->GetTerrainDetail()-1000);
	event.Veto();
}

void LODDlg::OnTarget( wxCommandEvent &event )
{
	if (m_bSet)
		return;

	// User typed something into the Target control
	TransferDataFromWindow();
	m_pFrame->SetTerrainDetail(m_iTarget);
}

