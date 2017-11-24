//
// Status bar implementation
//
// Copyright (c) 2001-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/ElevationGrid.h"
#include "vtdata/vtLog.h"

#include "Frame.h"
#include "StatusBar.h"
#include "BuilderView.h"
#include "vtui/Helper.h"	// for FormatCoord


MyStatusBar::MyStatusBar(wxWindow *parent) : wxStatusBar(parent, -1)
{
	VTLOG(" Creating Status Bar.\n");
	static const int widths[Field_Max] =
	{
		-1,		// main message area
		46,		// Geo, or short CRS identifier
		65,		// Zone
		65,		// Datum
		200,	// Units
		208,	// Coordinates of cursor
		86		// Elevation under cursor
	};

	SetFieldsCount(Field_Max);
	SetStatusWidths(Field_Max, widths);

	m_bShowMinutes = false;
	m_ShowVertUnits = LU_METERS;
}

MyStatusBar::~MyStatusBar()
{
}

void MyStatusBar::OnSize(wxSizeEvent& event)
{
	// could do resizing of status bar elements here if needed
//	m_checkbox->SetSize(rect.x + 2, rect.y + 2, rect.width - 4, rect.height - 4);
//	event.Skip();
}


wxString MyStatusBar::FormatCoord(bool bGeo, double coord)
{
	vtString str;
	if (bGeo)
	{
		str = ::FormatCoord(bGeo, coord, m_bShowMinutes);
	}
	else	// something meters-based
	{
		str.Format("%.1f", coord);
	}
	return wxString(str, wxConvUTF8);
}

void MyStatusBar::SetTexts(MainFrame *frame)
{
//	VTLOG(" StatusBar SetTexts: ");

	vtCRS &crs = frame->GetAtCRS();
	bool bGeo = (crs.IsGeographic() != 0);

	wxString str(crs.GetNameShort(), wxConvUTF8);
	SetStatusText(str, Field_Coord);

	int zone = crs.GetUTMZone();
	if (zone != 0)
		str.Printf(_("Zone %d"), zone);
	else
		str = _T("");
	SetStatusText(str, Field_Zone);

	str = wxString(DatumToStringShort(crs.GetDatum()), wxConvUTF8);
	SetStatusText(str, Field_Datum);

	DPoint2 p;
	BuilderView *pView = frame->GetView();
	if (pView)
	{
		// Scale and units
		double scale = pView->GetScale();
		LinearUnits lu = crs.GetUnits();
		if (lu == LU_DEGREES)
			str.Printf(_("1 Pixel = %.6lg "), 1.0/scale);
		else
			str.Printf(_("1 Pixel = %.2lf "), 1.0/scale);
		str += wxString(GetLinearUnitName(lu), wxConvUTF8);
		SetStatusText(str, Field_HUnits);

		pView->GetMouseLocation(p);
		str = _("Mouse");
		str += _T(": ");
		str += FormatCoord(bGeo, p.x);
		str += _T(", ");
		str += FormatCoord(bGeo, p.y);

		SetStatusText(str, Field_Mouse);
//		VTLOG(" '%s' ", (const char *)str);

		float height = frame->GetHeightFromTerrain(p);
		if (height == INVALID_ELEVATION)
			str = _T("");
		else
		{
			if (m_ShowVertUnits == LU_METERS)
				str.Printf(_T("%.2f m"), height);
			else
				str.Printf(_T("%.2f ft"), height / GetMetersPerUnit(m_ShowVertUnits));
		}

		RGBi rgb;
		if (frame->GetRGBUnderCursor(p, rgb))
		{
			if (str != _T(""))
				str += _T(", ");
			str += wxString::Format(_T("%d %d %d"), rgb.r, rgb.g, rgb.b);
		}

		SetStatusText(str, Field_Height);
	}
	else
	{
		SetStatusText(_("Mouse"), Field_Mouse);
		SetStatusText(_T(""), Field_Height);
	}
//	VTLOG(" Done.\n");
}

//
// Not sure why the event table must be down here, but it does - for WinZip
//
BEGIN_EVENT_TABLE(MyStatusBar, wxStatusBar)
EVT_SIZE(MyStatusBar::OnSize)
END_EVENT_TABLE()

