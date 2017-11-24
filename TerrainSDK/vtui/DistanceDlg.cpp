//
// Name: DistanceDlg.cpp
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "DistanceDlg.h"
#include "Helper.h" // for FormatCoord
#include "vtdata/Features.h"
#include "vtdata/FilePath.h"
#include <float.h>  // for FLT_MIN
#include <wx/tokenzr.h>

// WDR: class implementations

//----------------------------------------------------------------------------
// DistanceDlg
//----------------------------------------------------------------------------

// WDR: event table for DistanceDlg

BEGIN_EVENT_TABLE(DistanceDlg, DistanceDlgBase)
	EVT_INIT_DIALOG (DistanceDlg::OnInitDialog)
	EVT_CHOICE( ID_UNITS1, DistanceDlg::OnUnits )
	EVT_CHOICE( ID_UNITS2, DistanceDlg::OnUnits )
	EVT_CHOICE( ID_UNITS3, DistanceDlg::OnUnits )
	EVT_CHOICE( ID_UNITS4, DistanceDlg::OnUnits )
	EVT_CHOICE( ID_UNITS5, DistanceDlg::OnUnits )
	EVT_RADIOBUTTON( ID_RADIO_LINE, DistanceDlg::OnRadioLine )
	EVT_RADIOBUTTON( ID_RADIO_PATH, DistanceDlg::OnRadioPath )
	EVT_BUTTON( ID_DIST_TOOL_CLEAR, DistanceDlg::OnClear )
	EVT_BUTTON( ID_DIST_LOAD_PATH, DistanceDlg::OnLoadPath )
END_EVENT_TABLE()

DistanceDlg::DistanceDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	DistanceDlgBase( parent, id, title, position, size, style )
{
	GetUnits2()->Append(_("Meters"));
	GetUnits2()->Append(_("Feet"));
	GetUnits2()->Append(_("US Survey Feet"));
	GetUnits2()->Append(_("Kilometers"));
	GetUnits2()->Append(_("Miles"));

	GetUnits3()->Append(_("Meters"));
	GetUnits3()->Append(_("Feet"));
	GetUnits3()->Append(_("US Survey Feet"));
	GetUnits3()->Append(_("Kilometers"));
	GetUnits3()->Append(_("Miles"));

	GetUnits4()->Append(_("Meters"));
	GetUnits4()->Append(_("Feet"));
	GetUnits4()->Append(_("US Survey Feet"));
	GetUnits4()->Append(_("Kilometers"));
	GetUnits4()->Append(_("Miles"));

	GetUnits5()->Append(_("Meters"));
	GetUnits5()->Append(_("Feet"));
	GetUnits5()->Append(_("US Survey Feet"));

	AddValidator(this, ID_UNITS1, &m_iUnits1);
	AddValidator(this, ID_UNITS2, &m_iUnits2);
	AddValidator(this, ID_UNITS3, &m_iUnits3);
	AddValidator(this, ID_UNITS4, &m_iUnits4);
	AddValidator(this, ID_UNITS5, &m_iUnits5);

	m_pTransformToGeo = NULL;
	m_bPathMode = false;
	Zero();

	GetSizer()->SetSizeHints(this);
}

DistanceDlg::~DistanceDlg()
{
	delete m_pTransformToGeo;
}

void DistanceDlg::Zero()
{
	m_p1.Set(0,0);
	m_p2.Set(0,0);
	m_path.Clear();
	m_fGround = FLT_MIN;
	m_fVertical = FLT_MIN;
}

void DistanceDlg::SetCRS(const vtCRS &crs)
{
	if (m_crs != crs)
	{
		// Changing CRS
		Reset();
	}
	m_crs = crs;

	GetMapOffset()->SetValue(_T(""));
	GetMapDist()->SetValue(_T(""));
	GetGeodDist()->SetValue(_T(""));
	UpdateAvailableUnits();

	bool bIsGeo = (m_crs.IsGeographic() != FALSE);
	if (!bIsGeo)
	{
		// Free previous object
		delete m_pTransformToGeo;

		// We may need degrees later, but don't have them, prepare to compute them.
		vtCRS geo;
		CreateSimilarGeographicCRS(m_crs, geo);
		m_pTransformToGeo = CreateCoordTransform(&m_crs, &geo);
	}
	ShowValues();
}

void DistanceDlg::SetPoints(const DPoint2 &p1, const DPoint2 &p2, bool bUpdate)
{
	m_p1 = p1;
	m_p2 = p2;
	if (bUpdate)
		ShowValues();
}

void DistanceDlg::GetPoints(DPoint2 &p1, DPoint2 &p2)
{
	p1 = m_p1;
	p2 = m_p2;
}

void DistanceDlg::SetPath(const DLine2 &path, bool bUpdate)
{
	m_path = path;
	if (bUpdate)
		ShowValues();
}

void DistanceDlg::SetGroundAndVertical(float fGround, float fVertical, bool bUpdate)
{
	m_fGround = fGround;
	m_fVertical = fVertical;
	if (bUpdate)
		ShowValues();
}

double GetScaleFromUnits(int units)
{
	switch (units)
	{
	case 3:
		return 1.0 / 1000;  // km
	case 4:
		return 1.0 / (5280 * GetMetersPerUnit(LU_FEET_INT)); // miles
	}
	// otherwise, normal linear units
	return 1.0 / GetMetersPerUnit((LinearUnits)(units+1));
}

void DistanceDlg::ShowValues()
{
	wxString str;
	bool bIsGeo = (m_crs.IsGeographic() != FALSE);
	double scale;
	double map_distance, geodesic_meters;

	LinearUnits lu = m_crs.GetUnits();

	if (!bIsGeo && !m_pTransformToGeo)
	{
		// This should never happen, unless something is majorly wrong,
		//  like proj.dll is not found.
		GetMapOffset()->SetValue(_("<Projection failure>"));
		return;
	}

	if (m_bPathMode)
	{
		DLine2 geo_line;

		uint i, len = m_path.GetSize();
		if (bIsGeo)
			geo_line = m_path;
		else
		{
			// We need degrees, but don't have them, so compute them.
			geo_line.SetSize(len);
			for (i = 0; i < len; i++)
			{
				DPoint2 p = m_path[i];
				m_pTransformToGeo->Transform(1, &p.x, &p.y);
				geo_line[i] = p;
			}
		}
		map_distance = 0.0f;
		geodesic_meters = 0.0f;
		if (len > 1)
		{
			for (i = 0; i < len-1; i++)
			{
				// find map distance
				map_distance += (m_path[i+1] - m_path[i]).Length();

				// find geodesic distance
				geodesic_meters += vtCRS::GeodesicDistance(geo_line[i], geo_line[i+1]);
			}
		}

		// Map Offset
		str = _T("N/A");
		GetMapOffset()->SetValue(str);
	}
	else
	{
		// The "simple" case: two points
		// Find map offset and map distance
		DPoint2 diff_map = m_p2 - m_p1;
		map_distance = diff_map.Length();

		DPoint2 diff_degrees;
		DPoint2 geo1, geo2;
		if (bIsGeo)
		{
			diff_degrees = diff_map;
			geo1 = m_p1;
			geo2 = m_p2;
		}
		else
		{
			// We need degrees, but don't have them, so compute them.
			geo1 = m_p1;
			geo2 = m_p2;
			m_pTransformToGeo->Transform(1, &geo1.x, &geo1.y);
			m_pTransformToGeo->Transform(1, &geo2.x, &geo2.y);
			diff_degrees = geo2 - geo1;
		}
		// find geodesic distance
		geodesic_meters = vtCRS::GeodesicDistance(geo1, geo2);

		// Map Offset
		if (m_iUnits1 == 0) // degrees
		{
			str = wxString(FormatCoord(true, diff_degrees.x), wxConvUTF8);
			str += _T(", ");
			str += wxString(FormatCoord(true, diff_degrees.y), wxConvUTF8);
		}
		else
		{
			scale = GetMetersPerUnit(lu) /
				GetMetersPerUnit((LinearUnits)m_iUnits1);
			DPoint2 diff_show = diff_map * scale;
			str = wxString(FormatCoord(bIsGeo, diff_show.x), wxConvUTF8);
			str += _T(", ");
			str += wxString(FormatCoord(bIsGeo, diff_show.y), wxConvUTF8);
		}
		GetMapOffset()->SetValue(str);
	}

	// Map Distance
	if (bIsGeo)
		str = _T("N/A");
	else
	{
		// convert map units to meters, then meters to the desired unit
		scale = GetMetersPerUnit(lu) *
			GetScaleFromUnits(m_iUnits2);
		str = wxString(FormatCoord(false, map_distance * scale), wxConvUTF8);
	}
	GetMapDist()->SetValue(str);

	// Geodesic Distance
	scale = GetScaleFromUnits(m_iUnits3);
	str = wxString(FormatCoord(false, geodesic_meters * scale), wxConvUTF8);
	GetGeodDist()->SetValue(str);

	// Approximate Ground Distance
	bool bShowGround = (m_fGround != FLT_MIN);
	GetGroundDist()->Enable(bShowGround);
	GetUnits4()->Enable(bShowGround);
	if (bShowGround)
	{
		scale = GetScaleFromUnits(m_iUnits4);
		str = wxString(FormatCoord(false, m_fGround * scale), wxConvUTF8);
		GetGroundDist()->SetValue(str);
	}
	else
		GetGroundDist()->SetValue(_T("N/A"));

	// Vertical Difference
	bool bShowVertical = (m_fVertical != FLT_MIN);
	GetVertical()->Enable(bShowVertical);
	GetUnits5()->Enable(bShowVertical);
	if (bShowVertical)
	{
		scale = GetScaleFromUnits(m_iUnits5);
		str = wxString(FormatCoord(false, m_fVertical * scale), wxConvUTF8);
		GetVertical()->SetValue(str);
	}
	else
		GetVertical()->SetValue(_T("N/A"));
}

void DistanceDlg::UpdateAvailableUnits()
{
	bool bIsGeo = (m_crs.IsGeographic() != FALSE);
	GetUnits2()->Enable(!bIsGeo);
	GetMapDist()->Enable(!bIsGeo);

	GetMapOffset()->Enable(!m_bPathMode);
	GetUnits1()->Enable(!m_bPathMode);

	GetUnits1()->Clear();
	GetUnits1()->Append(_("Degrees"));
	if (!bIsGeo)
	{
		GetUnits1()->Append(_("Meters"));
		GetUnits1()->Append(_("Feet"));
		GetUnits1()->Append(_("US Survey Feet"));
	}

	switch (m_crs.GetUnits())
	{
	case LU_DEGREES:
		m_iUnits1 = 0;
		m_iUnits3 = 0;
		break;
	case LU_METERS:
		m_iUnits1 = 1;
		m_iUnits2 = 0;
		m_iUnits3 = 0;
		break;
	case LU_FEET_INT:
		m_iUnits1 = 2;
		m_iUnits2 = 1;
		m_iUnits3 = 1;
		break;
	case LU_FEET_US:
		m_iUnits1 = 3;
		m_iUnits2 = 2;
		m_iUnits3 = 2;
		break;
	case LU_UNITEDGE:
		// TODO
	case LU_UNKNOWN:
		break;
	}
	m_iUnits4 = m_iUnits3;
	m_iUnits5 = 0;
	TransferDataToWindow();
}

// WDR: handler implementations for DistanceDlg

void DistanceDlg::OnClear( wxCommandEvent &event )
{
	Reset();
}

void DistanceDlg::OnLoadPath( wxCommandEvent &event )
{
	wxString str = wxGetTextFromUser(_("Enter the path to a SHP file containing a polyline, or a series of points in the form X Y X Y .."), _("Input Path"), _T(""), this);
	if (str != _T(""))
	{
		DPoint2 p;
		DLine2 line;

		vtString vs = (const char *) str.mb_str(wxConvUTF8);
		FILE *fp = vtFileOpen(vs, "rb");
		if (fp != NULL)
		{
			fclose(fp);
			vtFeatureSetLineString fs;
			if (fs.LoadFromSHP(vs))
				line = fs.GetPolyLine(0);
		}
		else
		{
			// try to parse as a series of X Y values
			wxStringTokenizer tkz(str);
			while ( tkz.HasMoreTokens() )
			{
				tkz.GetNextToken().ToDouble(&p.x);
				tkz.GetNextToken().ToDouble(&p.y);
				line.Append(p);
			}
		}
		if (line.GetSize() > 0)
		{
			m_bPathMode = true;
			GetRadioPath()->SetValue(true);
			OnMode(m_bPathMode);
			UpdateAvailableUnits();
			SetPathToBase(line);
		}
		else
			DisplayAndLog("Couldn't read as file or as a series of points.");
	}
}

void DistanceDlg::OnRadioPath( wxCommandEvent &event )
{
	m_bPathMode = true;
	OnMode(m_bPathMode);
	UpdateAvailableUnits();
}

void DistanceDlg::OnRadioLine( wxCommandEvent &event )
{
	m_bPathMode = false;
	OnMode(m_bPathMode);
	UpdateAvailableUnits();
}

void DistanceDlg::OnInitDialog(wxInitDialogEvent& event)
{
	UpdateAvailableUnits();
	wxDialog::OnInitDialog(event);
}

void DistanceDlg::OnUnits( wxCommandEvent &event )
{
	TransferDataFromWindow();
	ShowValues();
}

