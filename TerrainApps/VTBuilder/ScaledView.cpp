//
// ScaledView.cpp
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <algorithm>

#include "vtdata/vtLog.h"
#include "ScaledView.h"
#include "Builder.h"
#include "vtdata/Triangulate.h"

///////////////////////////////////////////////////////////////////////

vtScaledView::vtScaledView(wxWindow *parent, wxWindowID id, const wxPoint& pos,
						 const wxSize& size, long style, const wxString& name) :
	wxGLCanvas(parent, id, NULL, pos, size, style, name )
{
	m_offset.Set(0, 0);
	m_origin.Set(0, 0);
	m_dScale = 1.0f;
}

void vtScaledView::ZoomToPoint(const DPoint2 &p)
{
	wxSize clientSize = GetClientSize();
	wxSize halfSize = clientSize / 2;
	m_offset.Set(halfSize.x / m_dScale, halfSize.y / m_dScale);
	m_offset -= p;
	m_offset += m_origin;
}

void vtScaledView::ZoomToRect(const DRECT &geo_rect, float margin)
{
	VTLOG(" ZoomToRect LRTB(%lg, %lg, %lg, %lg)\n", geo_rect.left, geo_rect.right, geo_rect.top, geo_rect.bottom);

	// Use center as our local OpenGL origin
	SetLocalOrigin(geo_rect.GetCenter());

	wxRect client;
	GetClientSize(&client.width, &client.height);

	// safety check: if the data is a single point, back out slightly
	DRECT rect = geo_rect;
	if (rect.Width() == 0)
		rect.Grow(0.25, 0);
	if (rect.Height() == 0)
		rect.Grow(0, 0.25);

	// scale is pixels/coordinate unit
	DPoint2 scale;
	scale.x = (float) client.GetWidth() / rect.Width();
	scale.y = (float) client.GetHeight() / rect.Height();
	double smaller = std::min(scale.x, scale.y);
	smaller *= (1.0f - margin);
	SetScale(smaller);

	DPoint2 center;
	rect.GetCenter(center);
	ZoomToPoint(center);
}

// From pixels to world: subtract offset
// From world to pixels: add offset

void vtScaledView::ClientToWorld(const wxPoint &sp, DPoint2 &p) const
{
	//VTLOG("ClientToWorld(%d %d, scale %lf, offset %lf %lf)\n",
	//	sp.x, sp.y, m_dScale, m_offset.x, m_offset.y);
	p.x = sp.x / m_dScale - m_offset.x + m_origin.x;
	p.y = (m_clientSize.y - 1 - sp.y) / m_dScale - m_offset.y + m_origin.y;
}

void vtScaledView::SetScale(double scale)
{
	m_dScale = scale;
	Refresh();
}

double vtScaledView::GetScale() const
{
	return m_dScale;
}

void vtScaledView::ScaleAroundPoint(const DPoint2 &p, double scale)
{
	m_offset += p;
	m_offset -= m_origin;
	m_offset *= (m_dScale / scale);
	m_offset += m_origin;
	m_offset -= p;
	SetScale(scale);
}

void vtScaledView::SetColor(const RGBi &color)
{
	glColor3f(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f);
}

void vtScaledView::SetLocalOrigin(const DPoint2 &p)
{
	m_origin = p;
}

void vtScaledView::SendVertex(const DPoint2 &p)
{
	glVertex2d(p.x - m_origin.x, p.y - m_origin.y);
}

void vtScaledView::SendVertex(const double &x, const double &y)
{
	glVertex2d(x - m_origin.x, y - m_origin.y);
}

void vtScaledView::DrawLine(const DPoint2 &p0, const DPoint2 &p1)
{
	glBegin(GL_LINES);
	glVertex2d(p0.x - m_origin.x, p0.y - m_origin.y);
	glVertex2d(p1.x - m_origin.x, p1.y - m_origin.y);
	glEnd();
}

void vtScaledView::DrawLine(double p1x, double p1y, double p2x, double p2y)
{
	glBegin(GL_LINES);
	glVertex2d(p1x - m_origin.x, p1y - m_origin.y);
	glVertex2d(p2x - m_origin.x, p2y - m_origin.y);
	glEnd();
}

void vtScaledView::DrawXHair(const DPoint2 &p, int pixelSize)
{
	DPoint2 hairsize = PixelsToWorld(pixelSize);
	glBegin(GL_LINES);
	glVertex2d(p.x - hairsize.x - m_origin.x, p.y - m_origin.y);
	glVertex2d(p.x + hairsize.x - m_origin.x, p.y - m_origin.y);
	glVertex2d(p.x - m_origin.x, p.y - hairsize.y - m_origin.y);
	glVertex2d(p.x - m_origin.x, p.y + hairsize.y - m_origin.y);
	glEnd();
}

void vtScaledView::DrawRectangle(const DRECT &rect)
{
	glBegin(GL_LINE_STRIP);
	glVertex2d(rect.left - m_origin.x, rect.top - m_origin.y);
	glVertex2d(rect.right - m_origin.x, rect.top - m_origin.y);
	glVertex2d(rect.right - m_origin.x, rect.bottom - m_origin.y);
	glVertex2d(rect.left - m_origin.x, rect.bottom - m_origin.y);
	glVertex2d(rect.left - m_origin.x, rect.top - m_origin.y);
	glEnd();
}

void vtScaledView::DrawRectangle(const DPoint2 &p0, const DPoint2 &p1)
{
	glBegin(GL_LINE_STRIP);
	glVertex2d(p0.x - m_origin.x, p0.y - m_origin.y);
	glVertex2d(p1.x - m_origin.x, p0.y - m_origin.y);
	glVertex2d(p1.x - m_origin.x, p1.y - m_origin.y);
	glVertex2d(p0.x - m_origin.x, p1.y - m_origin.y);
	glVertex2d(p0.x - m_origin.x, p0.y - m_origin.y);
	glEnd();
}

void vtScaledView::DrawPolyLine(const DLine2 &dline, bool bClose)
{
	glBegin(GL_LINE_STRIP);
	for (uint i = 0; i < dline.GetSize(); i++)
		glVertex2d(dline[i].x - m_origin.x, dline[i].y - m_origin.y);
	if (bClose)
		glVertex2d(dline[0].x - m_origin.x, dline[0].y - m_origin.y);
	glEnd();
}

void vtScaledView::DrawPolygon(const DPolygon2 &poly, bool bFill)
{
	// just draw each ring
	for (uint ring = 0; ring < poly.size(); ring++)
		DrawPolyLine(poly[ring], true);

	// TODO: support filled polygons; could use CallTriangle as below
}

void vtScaledView::DrawOGRLinearRing(const OGRLinearRing *line, bool bCircles)
{
	int i, size = line->getNumPoints();
	if (size < 2)
		return;

	glBegin(GL_LINE_STRIP);
	OGRPoint op;
	for (i = 0; i < size; i++)
	{
		line->getPoint(i, &op);
		glVertex2d(op.getX() - m_origin.x, op.getY() - m_origin.y);
	}
	line->getPoint(0, &op);
	glVertex2d(op.getX() - m_origin.x, op.getY() - m_origin.y);
	glEnd();

	// TODO? "bCircles"
}

void vtScaledView::DrawOGRPolygon(const OGRPolygon &poly, bool bFill, bool bCircles)
{
	if (bFill)
	{
		// TODO
	}
	else
	{
		// just draw each ring
		DrawOGRLinearRing(poly.getExteriorRing(), bCircles);

		for (int ring = 0; ring < poly.getNumInteriorRings(); ring++)
			DrawOGRLinearRing(poly.getInteriorRing(ring), bCircles);
	}
}

void vtScaledView::DrawDPolygon2(const DPolygon2 &poly, bool bFill,
								  bool bCircles)
{
	if (bFill)
	{
		// tessellate.  we could also draw these as solid triangles.
		DLine2 result;
		CallTriangle(poly, result);
		int tcount = result.GetSize()/3;
		for (int j = 0; j < tcount; j++)
		{
			const DPoint2 &p1 = result[j*3+0];
			const DPoint2 &p2 = result[j*3+1];
			const DPoint2 &p3 = result[j*3+2];
			DrawLine(p1, p2);
			DrawLine(p2, p3);
			DrawLine(p3, p1);
		}
	}
	else
	{
		// just draw each ring
		for (uint ring = 0; ring < poly.size(); ring++)
		{
			DrawPolyLine(poly[ring], true);
		}
		// TODO? "bCircles"
	}
}

///////////////////////////////////////////////////////////////////////////////

PushLogicOp::PushLogicOp(GLenum opcode)
{
	glGetIntegerv(GL_LOGIC_OP_MODE, &stored);
	glLogicOp(opcode);
}
PushLogicOp::~PushLogicOp()
{
	glLogicOp(stored);
}

