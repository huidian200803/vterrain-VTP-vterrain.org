//
// ScaledView.h
//
// Copyright (c) 2001-2015 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

#include "vtdata/MathTypes.h"
#include "ogr_geometry.h"
#include "wx/glcanvas.h"

class vtScaledView : public wxGLCanvas
{
public:
	vtScaledView(wxWindow *parent, wxWindowID id = -1,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize, long style = 0,
		const wxString& name = _T(""));

	void SetScale(double scale);
	double GetScale() const;
	void ScaleAroundPoint(const DPoint2 &p, double scale);

	void ZoomToPoint(const DPoint2 &p);
	void ZoomToRect(const DRECT &geo_rect, float margin);

	// transform integer mouse x, y to world coordinates
	void ClientToWorld(const wxPoint &sp, DPoint2 &p) const;
	DPoint2 PixelsToWorld(int pixels) const
	{
		return DPoint2(pixels / m_dScale, pixels / m_dScale);
	}

	void SetColor(const RGBi &color);
	void DrawLine(const DPoint2 &p0, const DPoint2 &p1);
	void DrawLine(double p1x, double p1y, double p2x, double p2y);
	void DrawXHair(const DPoint2 &p, int pixelSize);
	void DrawRectangle(const DRECT &rect);
	void DrawRectangle(const DPoint2 &p0, const DPoint2 &p1);
	void DrawPolyLine(const DLine2 &line, bool bClose);
	void DrawPolygon(const DPolygon2 &poly, bool bFill);

	void DrawOGRLinearRing(const OGRLinearRing *line, bool bCircles);
	void DrawOGRPolygon(const OGRPolygon &poly, bool bFill, bool bCircles);
	void DrawDPolygon2(const DPolygon2 &poly, bool bFill, bool bCircles);

	void GetViewParams(double &scale, DPoint2 &offset)
	{
		scale = m_dScale;
		offset = m_offset;
	}
	void SetLocalOrigin(const DPoint2 &p);
	void SendVertex(const DPoint2 &p);
	void SendVertex(const double &x, const double &y);

protected:
	double	m_dScale;	// Pixels per geographic unit.
	DPoint2 m_offset;	// View offset.
	DPoint2 m_origin;	// Local origin to work around OpenGL's float precision.
	wxSize  m_clientSize;
};

// Helpers
struct PushLogicOp
{
	PushLogicOp(GLenum opcode);
	~PushLogicOp();
	GLint stored;
};
