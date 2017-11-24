//
// Name: ProfileDlg.cpp
//
// Copyright (c) 2005-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <algorithm>	// For std::min/max

#include "ProfileDlg.h"
#include "vtdata/FileFilters.h"
#include "vtdata/vtString.h"

#define MARGIN_LEFT   60
#define MARGIN_BOTTOM 185

// WDR: class implementations

//----------------------------------------------------------------------------
// ProfileDlg
//----------------------------------------------------------------------------

// WDR: event table for ProfileDlg

BEGIN_EVENT_TABLE(ProfileDlg, ProfileDlgBase)
	EVT_PAINT(ProfileDlg::OnPaint)
	EVT_SIZE(ProfileDlg::OnSize)
	EVT_LEFT_DOWN(ProfileDlg::OnLeftDown)
	EVT_LEFT_UP(ProfileDlg::OnLeftUp)
	EVT_MOTION(ProfileDlg::OnMouseMove)
	EVT_BUTTON( ID_SHOW_CULTURE, ProfileDlg::OnShowCulture )
	EVT_CHECKBOX( ID_LINE_OF_SIGHT, ProfileDlg::OnLineOfSight )
	EVT_CHECKBOX( ID_VISIBILITY, ProfileDlg::OnVisibility )
	EVT_TEXT( ID_HEIGHT1, ProfileDlg::OnHeight1 )
	EVT_TEXT( ID_HEIGHT2, ProfileDlg::OnHeight2 )
	EVT_CHECKBOX( ID_FRESNEL, ProfileDlg::OnFresnel )
	EVT_CHECKBOX( ID_USE_EFFECTIVE, ProfileDlg::OnUseEffective )
	EVT_TEXT( ID_RF, ProfileDlg::OnRF )
	EVT_CHOICE( ID_CURVATURE, ProfileDlg::OnCurvature )
	EVT_BUTTON( ID_EXPORT_DXF, ProfileDlg::OnExportDXF )
	EVT_BUTTON( ID_EXPORT_TRACE, ProfileDlg::OnExportTrace )
	EVT_BUTTON( ID_EXPORT_CSV, ProfileDlg::OnExportCSV )
END_EVENT_TABLE()


ProfileDlg::ProfileDlg( wxWindow *parent, wxWindowID id,
	const wxString& title, const wxPoint &position, const wxSize& size, long style ) :
	ProfileDlgBase( parent, id, title, position, size, style | wxRESIZE_BORDER | wxMAXIMIZE)
{
	m_callback = NULL;
	m_clientsize.Set(0, 0);
	m_bHavePoints = false;
	m_bHaveValues = false;
	m_bMouseOnLine = false;
	m_bLeftButton = false;
	m_fGeodesicDistance=0;
	m_bHaveLOS = false;
	m_bHaveFresnel = false;
	m_bHaveGeoidSurface=false;
	m_bLineOfSight = false;
	m_bVisibility = false;
	m_bUseFresnel = false;
	m_fHeight1 = 1;
	m_fHeight2 = 1;
	m_xrange = 0;
	m_fRadioFrequency = 2400.0;
	m_iCurvature = 0;
	m_bGetCulture = false;
	m_bHaveCulture = false;

	// Work around wxFormDesigner's lack of support for limiting to smallest size
	GetSizer()->SetSizeHints(this);

	AddNumValidator(this, ID_HEIGHT1, &m_fHeight1);
	AddNumValidator(this, ID_HEIGHT2, &m_fHeight2);
	AddNumValidator(this, ID_RF, &m_fRadioFrequency);
	AddValidator(this, ID_CURVATURE, &m_iCurvature);

	GetHeight1()->Enable(false);
	GetHeight2()->Enable(false);

	SetBackgroundColour(wxColour(255,255,255));

	GetCurvature()->SetSelection(m_iCurvature);
	UpdateEnabling();
}

void ProfileDlg::SetCRS(const vtCRS &crs)
{
	m_crs = crs;
}

void ProfileDlg::SetPoints(const DPoint2 &p1, const DPoint2 &p2)
{
	m_p1 = p1;
	m_p2 = p2;
	m_bHavePoints = true;
	m_bHavePath = false;
	m_bHaveValues = false;
	m_bHaveLOS = false;
	m_bHaveFresnel = false;
	m_bHaveGeoidSurface = false;

	DPoint2 geo1, geo2;		// points 1 and 2 in geographic CS
	geo1 = m_p1;
	geo2 = m_p2;
	if (!m_crs.IsGeographic())
	{
		// convert points to geographic CS
		vtCRS geo;
		CreateSimilarGeographicCRS(m_crs, geo);
		ScopedOCTransform trans(CreateCoordTransform(&m_crs, &geo));
		trans->Transform(1, &geo1.x, &geo1.y);
		trans->Transform(1, &geo2.x, &geo2.y);
	}
	m_fGeodesicDistance = vtCRS::GeodesicDistance(geo1, geo2);
	m_fGeoDistAtPoint.resize(2);
	m_fGeoDistAtPoint[0] = 0.0f;
	m_fGeoDistAtPoint[1] = m_fGeodesicDistance;

	Refresh();
	UpdateEnabling();
}

void ProfileDlg::SetPath(const DLine2 &path)
{
	m_path = path;
	m_bHavePoints = false;
	m_bHavePath = true;
	m_bHaveValues = false;
	m_bHaveLOS = false;
	m_bHaveFresnel = false;
	m_bHaveGeoidSurface = false;

	m_fGeodesicDistance = 0.0f;
	uint i, len = m_path.GetSize();
	if (len > 1)
	{
		DLine2 m_path_geo = m_path;		// path in geographic CS
		if (!m_crs.IsGeographic())
		{
			// convert points to geographic CS
			vtCRS geo;
			CreateSimilarGeographicCRS(m_crs, geo);
			ScopedOCTransform trans(CreateCoordTransform(&m_crs, &geo));
			for (i = 0; i < len; i++)
				trans->Transform(1, &(m_path_geo[i].x), &(m_path_geo[i].y));
		}
		m_fGeoDistAtPoint.resize(len);
		m_fGeoDistAtPoint[0] = 0.0;
		for (i = 0; i < len-1; i++)
		{
			m_fGeodesicDistance += vtCRS::GeodesicDistance(m_path_geo[i],
				m_path_geo[i+1]);
			m_fGeoDistAtPoint[i+1] = m_fGeodesicDistance;
		}
	}
	if (len > 2)
	{
		// Several of the options only make sense for a single segment
		m_bLineOfSight = false;
		m_bVisibility = false;
		m_bUseFresnel = false;
		m_bUseEffectiveRadius = false;
	}

	Refresh();
	UpdateEnabling();
}

void ProfileDlg::SetCallback(ProfileCallback *callback)
{
	m_callback = callback;
}

void ProfileDlg::GetValues()
{
	if (!m_callback)
		return;

	m_clientsize = GetClientSize();

	// compute how large an area we have to draw the chart
	m_xrange = m_clientsize.x - MARGIN_LEFT - 10;
	if (m_xrange < 2) m_xrange = 2;
	m_yrange = m_clientsize.y - MARGIN_BOTTOM - 10;
	if (m_yrange < 2) m_yrange = 2;

	// fill array with elevation values, collect extents
	m_fMin = 1E9;
	m_fMax = -1E9;
	m_fDrawMin = 1E9;
	m_fDrawMax = -1E9;
	m_values.resize(m_xrange);
	m_values_culture.resize(m_xrange);
	m_callback->Begin();

	// We can use the same logic for two points or a polyline
	if (m_bHavePoints)
	{
		if (m_p1 == m_p2)
			return;
		m_path.Clear();
		m_path.Append(m_p1);
		m_path.Append(m_p2);
	}

	m_bHaveValidData = false;
	m_bHaveInvalid = false;
	m_bHaveValues = false;
	m_bMouseOnLine = false;

	uint len = m_path.GetSize();
	if (len < 2)
	{
		// Need more points
		return;
	}

	// Iterate over the polyline (linearly in map coordinates)
	double fTotalDistance = 0.0;
	uint seg = 0, total_segments = len-1;
	for (seg = 0; seg < total_segments; seg++)
		fTotalDistance += (m_path[seg+1] - m_path[seg]).Length();
	double fStep = fTotalDistance / (m_xrange - 1);
	double fSegmentDist = 0.0;
	double fDist = 0.0;
	seg = 0;
	DPoint2 segvector(m_path[seg+1] - m_path[seg]);
	double fSegmentLength = segvector.Length();
	DPoint2 p = m_path[0];

	for (int i = 0; i < m_xrange; i++)
	{
		float f = m_callback->GetElevation(p);
		m_values[i] = f;

		if (f == INVALID_ELEVATION)
			m_bHaveInvalid = true;
		else
		{
			m_bHaveValidData  = true;
			if (f < m_fMin)
			{
				m_fMin = f;
				m_iMin = i;
				m_fMinDist = (float)i / (m_xrange-1) * m_fGeodesicDistance;
			}
			if (f > m_fMax)
			{
				m_fMax = f;
				m_iMax = i;
				m_fMaxDist = (float)i / (m_xrange-1) * m_fGeodesicDistance;
			}
			if (f < m_fDrawMin) m_fDrawMin = f;
			if (f > m_fDrawMax) m_fDrawMax = f;
		}

		if (m_bGetCulture)
		{
			float f2 = m_callback->GetCultureHeight(p);
			m_values_culture[i] = f2;
			if (f2 != INVALID_ELEVATION)
			{
				if (f2 < m_fDrawMin) m_fDrawMin = f2;
				if (f2 > m_fDrawMax) m_fDrawMax = f2;
			}
		}

		if (i == m_xrange-1)	// finished
			break;

		// Advance one step
		double fRemain = fSegmentLength - fSegmentDist;
		double fAdvance = fStep;
		while (fAdvance != 0.0)
		{
			if (fAdvance <= fRemain)
			{
				// same segment
				fDist += fAdvance;
				fSegmentDist += fAdvance;

				p = m_path[seg] + (segvector / fSegmentLength * fSegmentDist);
				fAdvance = 0.0f;
			}
			else
			{
				// advance to next segment
				fDist += fRemain;
				fAdvance -= fRemain;

				seg++;
				segvector = m_path[seg+1] - m_path[seg];
				fSegmentLength = segvector.Length();
				fSegmentDist = 0.0;
				fRemain = fSegmentLength;
			}
		}
	}
	if (m_bGetCulture)
	{
		m_bGetCulture = false;
		m_bHaveCulture = true;
	}
	else
		m_bHaveCulture = false;
	m_fDrawRange = m_fDrawMax - m_fDrawMin;
	m_bHaveValues = true;

	Analyze();
}

void ProfileDlg::Analyze()
{
	m_bValidStart = false;
	m_bValidLine = false;
	m_bIntersectsGround = false;
	m_bIntersectsCulture = false;

	if (!m_bHaveValues)
		return;

	// Some analysis is shared between the line of sight and the visibility
	m_fHeightAtStart = m_values[0];
	if (m_fHeightAtStart != INVALID_ELEVATION)
	{
		m_fHeightAtStart += m_fHeight1;
		m_bValidStart = true;
	}

	if (m_bHavePoints || m_bHavePath)
		ComputeGeoidSurface();

	if (m_bLineOfSight)
		ComputeLineOfSight();

	if (m_bVisibility && m_bValidStart)
		ComputeVisibility();

	if (m_bValidLine && m_bLineOfSight && m_bUseFresnel && m_fGeodesicDistance>0)
		ComputeFirstFresnel(); // depends on ComputeLineOfSight()
	else
		m_bHaveFresnel=false;
}

void ProfileDlg::ComputeLineOfSight()
{
	// compute height at end of line-of-sight, and the height
	//  range to draw
	if (m_bValidStart)
	{
		m_fHeightAtEnd = m_values[m_xrange - 1];
		if (m_fHeightAtEnd != INVALID_ELEVATION)
		{
			m_bValidLine = true;
			m_fHeightAtEnd += m_fHeight2;
		}
	}
	if (!m_bValidLine)
		return;

	m_LineOfSight.resize(m_xrange);

	// Cast line from beginning to end
	float diff = m_fHeightAtEnd - m_fHeightAtStart;
	for (int i = 0; i < m_xrange; i++)
	{
		float fLineHeight = m_fHeightAtStart + diff * i / m_xrange;
		float fGroundHeight = m_values[i];

		// adjust for curvature
		fLineHeight = ApplyGeoid(fLineHeight, i, 2);
		fGroundHeight = ApplyGeoid(fGroundHeight, i, 1);

		m_LineOfSight[i] = fLineHeight;

		if (!m_bIntersectsCulture && !m_bIntersectsGround)
		{
			if (m_bHaveCulture)
			{
				float fCultureHeight = m_values_culture[i];
				if (fLineHeight < fCultureHeight)
				{
					// line of sight intersects the culture
					m_bIntersectsCulture = true;
					m_fIntersectHeight = fLineHeight;
					m_fIntersectDistance = (float)i / (m_xrange-1) * m_fGeodesicDistance;
					m_iIntersectIndex = i;
				}
			}
			if (fLineHeight < fGroundHeight)
			{
				// line of sight intersects the ground
				m_bIntersectsGround = true;
				m_fIntersectHeight = fGroundHeight;
				m_fIntersectDistance = (float)i / (m_xrange-1) * m_fGeodesicDistance;
				m_iIntersectIndex = i;
			}
		}
	}
	m_bHaveLOS = true;
}

float ProfileDlg::TotalHeightAt(int i)
{
	float val = m_values[i];
	if (val == INVALID_ELEVATION)
		return val;

	if (m_bHaveGeoidSurface && m_iCurvature == 1)
		val += m_GeoidSurface[i];

	if (m_bHaveCulture)
	{
		float fCultureHeight = m_values_culture[i];
		if (fCultureHeight != INVALID_ELEVATION)
		{
			float diff = m_values_culture[i] - m_values[i];
			val += diff;
		}
	}
	return val;
}

void ProfileDlg::ComputeVisibility()
{
	// prepare visibility array
	int i, j;
	float diff;
	int apply_geoid = (m_bHaveGeoidSurface ? m_iCurvature : 0);
	m_visible.resize(m_xrange);
	m_rvisible.resize(m_xrange);

	// compute visibility at each point on the line
	m_visible[0] = 1;
	for (j = 1; j < m_xrange; j++)
	{
		// forward visibility
		bool vis = true;
		diff = m_values[j] + (apply_geoid==1 ? m_GeoidSurface[j] : 0) - m_fHeightAtStart;
		for (i = 0; i < j; i++)
		{
			float fLineHeight = m_fHeightAtStart + diff * i / j;
			if (fLineHeight <= TotalHeightAt(i))
			{
				vis = false;
				break;
			}
		}
		m_visible[j] = vis;

		// reverse visibility
		vis = true;
		diff = m_values[j] + (apply_geoid==1 ? m_GeoidSurface[j] : 0) - m_fHeightAtEnd;
		for (i = m_xrange-1; i > j; i--)
		{
			float fLineHeight = m_fHeightAtEnd + diff * (m_xrange-i) / (m_xrange-j);
			if (fLineHeight <= TotalHeightAt(i))
			{
				vis = 0;
				break;
			}
		}
		m_rvisible[j] = vis;
	}
}

// dist = metres from source
// freq = frequency in Hz
// zone = fresnel zone number (0 = object free zone)
float ProfileDlg::ComputeFresnelRadius(float dist, float freq, int zone)
{
	float radius;
	if (zone==1 || zone==0)
	{
		float total_dist=m_fGeodesicDistance/1000;
		dist/=1000;		// need this in Km
		freq/=1000;		// need this in GHz
		radius=17.3 * sqrt( (dist * (total_dist-dist)) / (freq * m_fGeodesicDistance) );
		if (zone==0) radius *= 0.60f;
	}
	else if (zone>1)
	{
		float wavelength=299792458.0/(freq*1000000);
		radius=sqrt(( zone * wavelength * dist * (m_fGeodesicDistance - dist))/m_fGeodesicDistance);
	}
	else radius=0;
	return radius;
}

void ProfileDlg::ComputeFirstFresnel()
{
	float total_dist=m_fGeodesicDistance/1000; // Km
	float freq=m_fRadioFrequency/1000; // in GHz
	m_FirstFresnel.resize(m_xrange);

	for (int i=0; i<m_xrange; i++)
	{
		float range=total_dist * i/m_xrange;
		m_FirstFresnel[i]=17.3 * sqrt( (range * (total_dist-range)) / (freq * total_dist) );
	}
	m_bHaveFresnel=true;
}


void ProfileDlg::ComputeGeoidSurface()
{
  /*
  neato trig lies beneath...

				  S  (arc P1 -> P2)
			  *****
		  ****     **** P3
	   ***             ***   dist (arc P1 -> P3)
	 **             H/    **
 P2 *               /       * P1
   *---------------X---------*
  *               /       .   *
  *              /   .  R     *
  *             C             *

  Problem:  Find H (height of chord at angle) given length of arc S and radius R,
		and the partial arc distance between P1 and P3

  Formulas from: http://mathforum.org/dr.math/faq/faq.circle.segment.html

  The angle at centre C between P1 and P2 is given by  S/R
  The partial arc angle at C between P1 and P3 is given by  dist/R
  We have an iscoceles triangle - P1, P2, C
  The angle at P1 must equal the angle at P2, and is given by (PI-angle_at_C)/2

  We now have a triangle X, C, P1 in which we know the angles at C and P1, plus
  the length of R.

  The angle at X is PI - angle_at_P1 - angle_at_C.

  The length CX is (R*sin(P1)) / sin(X)

  H is fairly obviously R - CX

  In one step, this is written:

  H = R - ( (R * sin( (PI-(S/R))/2 )) / sin(PI - angle_at_P1 - (dist/R)) )

  NOTE: It may be more appropriate to compute the length of the normal from
    the chord through P3 (ie a line perpendicular to the chord P1, P2, passing
	through P3 but as the heights are drawn as perpendicular to the surface,
	I think the surface should be drawn in the same manner.

  The code optimises by calculating some values that do not change once only.
*/

#define ER_EQUATORIAL  6378.137		// Equatorial Radius
#define ER_POLAR       6356.752		// Polar Radius
#define ER             6366.000		// accepted average

#define EER (ER * 4.0/3.0)		// Effective earth radius (radio signals bend)

	// TODO: Obtain R from the projection, if possible

	double S, dist, h, P1angle, RsinP1angle, PIminusP1angle;
	double R = (m_bUseEffectiveRadius ? EER : ER) * 1000; // everything in meters.
	m_GeoidSurface.resize(m_xrange);

	S = m_fGeodesicDistance;

	P1angle			= (PIf-(S/R))/2;
	RsinP1angle		= R*sin( P1angle );
	PIminusP1angle	= PIf - P1angle;

	if (S<10 || S >= (PIf*R))
	{
		// more than half way around the world, this will break, so don't bother...
		// less than 10m or so, and it breaks due to rounding error.
		m_bHaveGeoidSurface=false;
		return;
	}

	for (int i = 0; i < m_xrange; i++)
	{
		dist = m_fGeodesicDistance * i/m_xrange;
		h = (dist<10 ? 0 : R-(RsinP1angle/sin(PIminusP1angle - (dist/R))) );
		m_GeoidSurface[i]=h;
	}

	// maximum - We assume it's in the middle, like all good chords should be.
	m_fGeoidCurvature = m_GeoidSurface[m_xrange/2];
	m_bHaveGeoidSurface = true;

	// It might affect the range of values we'll draw
	for (int i = 0; i < m_xrange; i++)
	{
		float f = m_values[i];
		if (f != INVALID_ELEVATION && m_iCurvature == 1)
		{
			f += m_GeoidSurface[i];
			if (f > m_fDrawMax) m_fDrawMax = f;
		}
	}
}


float ProfileDlg::ApplyGeoid(float h, int i, char t)
{
	if (m_bHaveGeoidSurface && m_iCurvature == t)
	{
		switch (t)
		{
		case 0: break;			// don't apply
		case 1: h+=m_GeoidSurface[i]; break;	// apply to heights relative to surface
		case 2: h-=m_GeoidSurface[i]; break;	// apply to heights relative to line of sight
		}
	}
	return h;
}


// Free space loss only (we aren't that fancy yet)
void ProfileDlg::ComputeSignalLoss(float dist, float freq)
{
	// #define log10(x) (log(x)/log(10))
	dist *= 0.621f;		// need it in miles
	freq /= 1000000.0;	// and in MHz
	float free_space_loss = 36.56 + (20*log10(freq)) + (20*log10(dist));
}


void ProfileDlg::MakePoint(wxPoint &p, int i, float value)
{
	p.x = m_base.x + i;
	p.y = (int)(m_base.y - (value - m_fDrawMin) / m_fDrawRange * m_yrange);
}

void ProfileDlg::DrawChart(wxDC& dc)
{
	if (!m_bHavePoints && !m_bHavePath)
	{
		GetText()->SetValue(_("No Endpoints"));
		return;
	}

	if (!m_bHaveValues)
		GetValues();

	float DrawMax = m_fDrawMax;
	if (m_bValidStart) DrawMax = std::max(DrawMax, m_fHeightAtStart);
	if (m_bValidLine) DrawMax = std::max(DrawMax, m_fHeightAtEnd);
	m_fDrawRange = DrawMax - m_fDrawMin;

	wxPen pen1(*wxMEDIUM_GREY_PEN);
	pen1.SetWidth(2);
	wxPen pen2(*wxLIGHT_GREY_PEN);
	pen2.SetStyle(wxDOT);
	wxPen pen3(wxColour(0,128,0), 1, wxSOLID);  // dark green
	wxPen pen4(wxColour(128,0,0), 1, wxSOLID);  // dark red
	wxPen pen5(wxColour(0,160,0), 1, wxSOLID);  // light green
	wxPen pen6(wxColour(170,0,0), 1, wxSOLID);  // light red
	wxPen pen7(wxColour(0,0,170), 1, wxSOLID);  // medium blue

	wxFont font(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	dc.SetFont(font);

	m_base.x = MARGIN_LEFT;
	m_base.y = m_clientsize.y - MARGIN_BOTTOM;

	// Draw X and Y axes
	dc.SetPen(pen1);
	dc.DrawLine(m_base.x, m_base.y, m_base.x + m_xrange, m_base.y);
	dc.DrawLine(m_base.x, m_base.y, m_base.x, m_base.y - m_yrange);

	if (!m_bHaveValidData)
	{
		GetText()->SetValue(_("No Data"));
		return;
	}

	// Draw tick marks
	int numticks, tick_spacing = 32;
	wxString str;
	int w, h;

	// Vertical ticks
	numticks = (m_yrange / tick_spacing)+2;
	for (int i = 0; i < numticks; i++)
	{
		const int y = m_base.y - (i * m_yrange / (numticks-1));

		if (i > 0)
		{
			dc.SetPen(pen2);
			dc.DrawLine(m_base.x, y, m_base.x + m_xrange, y);
		}

		dc.SetPen(pen1);
		dc.DrawLine(m_base.x - 5, y, m_base.x + 5, y);

		str.Printf(_T("%5.1f"), m_fDrawMin + (m_fDrawRange / (numticks-1) * i));
		dc.GetTextExtent(str, &w, &h);
		dc.DrawText(str, MARGIN_LEFT - w - 8, y-(h/2));
	}
	// Horizontal ticks
	numticks = (m_xrange / tick_spacing)+2;
	for (int i = 0; i < numticks; i++)
	{
		const int x = m_base.x + (i * m_xrange / (numticks-1));

		dc.SetPen(pen1);
		dc.DrawLine(x, m_base.y - 5, x, m_base.y + 5);

		if (m_fGeodesicDistance >= 50000)
			str.Printf(_T("%5.0fkm"), m_fGeodesicDistance / (numticks-1) * i / 1000);
		else
			str.Printf(_T("%5.1f"), m_fGeodesicDistance / (numticks-1) * i);
		dc.GetTextExtent(str, &w, &h);
		dc.DrawRotatedText(str, x-(h/2), m_base.y + w + 8, 90);
	}

	// Draw surface line
	wxPoint p1, p2;
	int apply_geoid = (m_bHaveGeoidSurface ? m_iCurvature : 0);

	if (m_bVisibility && m_bValidStart)
	{
		bool vis = true;
		bool visr= true;
		dc.SetPen(pen3);
		for (int i = 0; i < m_xrange; i++)
		{
			if (m_visible[i] != vis || m_rvisible[i] != visr)
			{
				vis = m_visible[i];
				visr= m_rvisible[i];
				if (vis && visr) dc.SetPen(pen5);
				else if (visr) dc.SetPen(pen6);
				else if (vis) dc.SetPen(pen3);
				else dc.SetPen(pen4);
			}
			float v1 = m_values[i];
			if (v1 == INVALID_ELEVATION)
				continue;
			// adjust for curvature
			v1 = ApplyGeoid(v1, i, 1);

			MakePoint(p1, i, v1);
			p2 = p1;
			p2.y = m_base.y;
			dc.DrawLine(p1, p2);
		}
	}
	else
	{
		dc.SetPen(pen3);
		if (m_bHaveInvalid)
		{
			// slow way, one datapoint at a time
			for (int i = 0; i < m_xrange-1; i++)
			{
				float v1 = m_values[i];
				float v2 = m_values[i+1];
				if (v1 == INVALID_ELEVATION || v2 == INVALID_ELEVATION)
					continue;
				if (apply_geoid == 1)
				{
					v1+=m_GeoidSurface[i];
					v2+=m_GeoidSurface[i+1];
				}
				MakePoint(p1, i, v1);
				MakePoint(p2, i+1, v2);
				dc.DrawLine(p1, p2);
			}
		}
		else
		{
			// faster way, pass an array
			wxPoint *pts = new wxPoint[m_xrange];
			for (int i = 0; i < m_xrange; i++)
			{
				MakePoint(pts[i], i, m_values[i] + (apply_geoid==1 ? m_GeoidSurface[i] : 0));
			}
			dc.DrawLines(m_xrange, pts);
			delete [] pts;
		}
	}

	// Draw vertical lines for multi-point paths
	uint len = m_path.GetSize();
	if (len > 2)
	{
		dc.SetPen(pen7);

		// Iterate over the polyline (linearly in map coordinates)
		double fTotalDistance = 0.0;
		uint seg = 0, total_segments = len-1;
		for (seg = 0; seg < total_segments; seg++)
			fTotalDistance += (m_path[seg+1] - m_path[seg]).Length();
		double fStep = fTotalDistance / (m_xrange - 1);

		float dist = 0.0f;
		for (seg = 0; seg < total_segments; seg++)
		{
			dist += (m_path[seg+1] - m_path[seg]).Length();
			dc.DrawLine(m_base.x + dist/fStep, m_base.y,
						m_base.x + dist/fStep, m_base.y - m_yrange);
		}
	}

	// Draw the fresnel zones
	if (m_bUseFresnel && m_bHaveFresnel && m_bHaveLOS)
	{
		wxPoint *pts0 = new wxPoint[m_xrange];
		wxPoint *pts1 = new wxPoint[m_xrange];
		wxPoint *pts2 = new wxPoint[m_xrange];

		for (int i=0; i<m_xrange; i++)
		{
			float base=(apply_geoid==2 ? m_GeoidSurface[i] : 0);
			MakePoint(pts1[i], i, m_LineOfSight[i] - m_FirstFresnel[i] - base);
			MakePoint(pts0[i], i, m_LineOfSight[i] - (m_FirstFresnel[i] * 0.60) - base);

			float r=ComputeFresnelRadius(m_fGeodesicDistance * i/m_xrange,m_fRadioFrequency,2);
			MakePoint(pts2[i], i, m_LineOfSight[i] - r - base);
		}

		// object free zone (60% of first zone)
		wxPen fresnelColourOF(wxColour(255,200,180));
		dc.SetPen(fresnelColourOF);
		dc.DrawLines(m_xrange, pts0);

		// first zone
		wxPen fresnelColour1(wxColour(255,210,190));
		dc.SetPen(fresnelColour1);
		dc.DrawLines(m_xrange, pts1);

		// second zone
		wxPen fresnelColour2(wxColour(255,220,200));
		dc.SetPen(fresnelColour2);
		dc.DrawLines(m_xrange, pts2);

		delete [] pts0;
		delete [] pts1;
		delete [] pts2;
	}

	// Draw Line of Sight
	if (m_bValidLine)
	{
		wxPen orange(wxColour(255,128,0), 1, wxSOLID);
		dc.SetPen(orange);
		if (apply_geoid == 2)
		{
			wxPoint *pts = new wxPoint[m_xrange];
			for (int i=0; i<m_xrange; i++)
				MakePoint(pts[i], i, m_LineOfSight[i]);
			dc.DrawLines(m_xrange, pts);
			delete [] pts;
		}
		else
		{
			MakePoint(p1, 0, m_fHeightAtStart);
			MakePoint(p2, m_xrange - 1, m_fHeightAtEnd);
			dc.DrawLine(p1, p2);
		}
	}

	// Draw origin of line of sight
	if ((m_bLineOfSight || m_bVisibility) && m_bValidStart)
	{
		// it's hard to see a yellow dot without a bit of outline
		wxPen lightgrey(*wxLIGHT_GREY_PEN);
		dc.SetPen(lightgrey);

		wxBrush yellow(wxColour(255,255,0), wxSOLID);
		dc.SetBrush(yellow);
		MakePoint(p1, 0, m_fHeightAtStart);
		dc.DrawCircle(p1, 5);
	}

	// Draw min/max/mouse markers
	wxPen nopen;
	nopen.SetStyle(wxTRANSPARENT);
	dc.SetPen(nopen);

	wxBrush brush1(wxColour(0,0,255), wxSOLID); // blue: minimum
	dc.SetBrush(brush1);
	MakePoint(p1, m_iMin, m_fMin + (apply_geoid==1 ? m_GeoidSurface[m_iMin] : 0));
	dc.DrawCircle(p1, 5);

	wxBrush brush2(wxColour(255,0,0), wxSOLID); // red: maximum
	dc.SetBrush(brush2);
	MakePoint(p1, m_iMax, m_fMax + (apply_geoid==1 ? m_GeoidSurface[m_iMax] : 0));
	dc.DrawCircle(p1, 5);

	if (m_bMouseOnLine)
	{
		wxBrush brush3(wxColour(0,255,0), wxSOLID); // green: mouse
		dc.SetBrush(brush3);
		MakePoint(p1, m_iMouse, m_fMouse + (apply_geoid==1 ? m_GeoidSurface[m_iMouse] : 0));
		dc.DrawCircle(p1, 5);
	}

	if (m_bHaveCulture)
	{
		dc.SetPen(pen7);

		// slow way, one datapoint at a time
		for (int i = 0; i < m_xrange; i++)
		{
			float v1 = m_values[i];
			float v2 = m_values_culture[i];
			if (v1 == INVALID_ELEVATION || v2 == INVALID_ELEVATION)
				continue;
			if (apply_geoid == 1)
			{
				v1+=m_GeoidSurface[i];
				v2+=m_GeoidSurface[i];
			}
			MakePoint(p1, i, v1);
			MakePoint(p2, i, v2);
			dc.DrawLine(p1, p2);
		}
	}

	if (m_bIntersectsGround || m_bIntersectsCulture)
	{
		wxBrush brush3(wxColour(255,128,0), wxSOLID);	// orange: intersection
		dc.SetBrush(brush3);
		MakePoint(p1, m_iIntersectIndex, m_fIntersectHeight);
		dc.DrawCircle(p1, 5);
	}

	// Also update message text
	UpdateMessageText();
}

void ProfileDlg::UpdateMessageText()
{
	wxString str, str2;

	str2.Printf(_("Minimum: %.2f m at distance %.1f\n"),
		ApplyGeoid(m_fMin,m_iMin,1), m_fMinDist);
	str += str2;
	str2.Printf(_("Maximum: %.2f m at distance %.1f"),
		ApplyGeoid(m_fMax,m_iMax,1), m_fMaxDist);
	str += str2;

	if (m_bMouseOnLine)
	{
		str += _T("\n");
		str2.Printf(_("Mouse: %.2f m at distance %.1f"),
			ApplyGeoid(m_fMouse,m_iMouse,1), m_fMouseDist);
		str += str2;
		if (m_bHaveSlope)
		{
			str += _T(", ");
			str2.Printf(_("Slope %.3f (%.1f degrees)"), m_fSlope,
				atan(m_fSlope) * 180 / PId);
			str += str2;
		}

		if (m_bHaveFresnel && m_bHaveLOS)
		{
			str += _T(", ");
			str2.Printf(_("Fresnel Zone 1: Radius: %.1f, Clearance %.1f"),
				m_fMouseFresnel,
				(ApplyGeoid(m_fMouseLOS,m_iMouse,2)-m_fMouseFresnel) -
				 ApplyGeoid(m_fMouse,m_iMouse,1));
			str+=str2;
			str2.Printf(_("  OF Zone: Radius: %.1f, Clearance %.1f"),
				m_fMouseFresnel*0.6,
				(ApplyGeoid(m_fMouseLOS,m_iMouse,2)-(m_fMouseFresnel*0.6)) -
				 ApplyGeoid(m_fMouse,m_iMouse,1));
			str+=str2;
		}
	}
	else if (m_bHaveFresnel)
	{
		str += _T("\n");
		str2.Printf(_("Fresnel zone radius at midpoint %.2fm (Object free zone %.2fm) Geoid Height %.1fm"),
			m_FirstFresnel[m_xrange/2], m_FirstFresnel[m_xrange/2]*0.60,
			m_fGeoidCurvature);
		str += str2;
	}

	if (m_bValidLine && m_bIntersectsGround)
	{
		str += _T("\n");
		str2.Printf(_("Intersects ground at height %.2f, distance %.1f"),
			m_fIntersectHeight, m_fIntersectDistance);
		str += str2;
	}
	if (m_bValidLine && m_bIntersectsCulture)
	{
		str += _T("\n");
		str2.Printf(_("Intersects culture at height %.2f, distance %.1f"),
			m_fIntersectHeight, m_fIntersectDistance);
		str += str2;
	}
	GetText()->SetValue(str);
}

void ProfileDlg::UpdateEnabling()
{
	GetLineOfSight()->Enable(m_bHavePoints);
	GetVisibility()->Enable(m_bHavePoints);

	GetHeight1()->Enable(m_bLineOfSight || m_bVisibility);
	GetHeight2()->Enable(m_bLineOfSight);

	GetFresnel()->Enable(m_bLineOfSight);

	GetRF()->Enable(m_bUseFresnel);
	GetCurvature()->Enable(m_bHavePoints);
	GetEffective()->Enable(m_bHavePoints);

	if (m_callback)
		GetShowCulture()->Enable(m_callback->HasCulture());
}

// WDR: handler implementations for ProfileDlg

void ProfileDlg::OnCurvature( wxCommandEvent &event )
{
	TransferDataFromWindow();
	Analyze();
	Refresh();
	UpdateEnabling();
}

void ProfileDlg::OnRF( wxCommandEvent &event )
{
	TransferDataFromWindow();
	Analyze();
	Refresh();
}

void ProfileDlg::OnUseEffective( wxCommandEvent &event )
{
	m_bUseEffectiveRadius = event.IsChecked();
	Analyze();
	Refresh();
}

void ProfileDlg::OnFresnel( wxCommandEvent &event )
{
	m_bUseFresnel = event.IsChecked();
	Analyze();
	Refresh();
	UpdateEnabling();
}

void ProfileDlg::OnHeight2( wxCommandEvent &event )
{
	TransferDataFromWindow();
	Analyze();
	Refresh();
}

void ProfileDlg::OnHeight1( wxCommandEvent &event )
{
	TransferDataFromWindow();
	Analyze();
	Refresh();
}

// This calls OnDraw, having adjusted the origin according to the current
// scroll position
void ProfileDlg::OnPaint(wxPaintEvent &event)
{
	// don't use m_targetWindow here, this is always called for ourselves
	wxPaintDC dc(this);
	OnDraw(dc);
}

void ProfileDlg::OnDraw(wxDC& dc)  // overridden to draw this view
{
	DrawChart(dc);
}

void ProfileDlg::OnSize(wxSizeEvent& event)
{
	wxSize size = GetClientSize();
	if (size != m_clientsize)
	{
		m_clientsize = size;
		m_bHaveValues = false;
		Refresh();
	}
	event.Skip();
}

void ProfileDlg::OnLeftDown(wxMouseEvent& event)
{
	wxPoint point = event.GetPosition();
	if (point.x > m_base.x && point.x < m_base.x + m_xrange - 1 &&
		point.y < m_base.y && point.y > m_base.x - m_yrange)
	{
		m_bLeftButton = true;
		OnMouseMove(event);
	}
}
void ProfileDlg::OnLeftUp(wxMouseEvent& event)
{
	m_bLeftButton = false;
}

void ProfileDlg::OnMouseMove(wxMouseEvent& event)
{
	if (!m_bHaveValidData || !m_bHaveValues || !m_bLeftButton)
	{
		m_bMouseOnLine = false;
		return;
	}
	wxPoint point = event.GetPosition();
	if (point.x > m_base.x && point.x < m_base.x + m_xrange - 1 &&
		point.y < m_base.y && point.y > m_base.x - m_yrange)
	{
		int offset = point.x - m_base.x;
		m_fMouse = m_values[offset];
		if (m_bHaveFresnel) m_fMouseFresnel = m_FirstFresnel[offset];
			else    m_fMouseFresnel = 0;
		if (m_bHaveLOS) m_fMouseLOS = m_LineOfSight[offset];
			else    m_fMouseLOS = 0;

		if (m_fMouse != INVALID_ELEVATION)
		{
			m_fMouseDist = (float)offset / m_xrange * m_fGeodesicDistance;
			m_iMouse = offset;
			m_bMouseOnLine = true;
			Refresh();

			// calculate slope
			float v2 = m_values[offset+1];
			if (v2 != INVALID_ELEVATION)
			{
				m_bHaveSlope = true;
				m_fSlope = (v2 - m_fMouse) / (m_fGeodesicDistance / m_xrange);
			}
		}
	}
	else
	{
		// mouse out
		if (m_bMouseOnLine)
			Refresh();
		m_bMouseOnLine = false;
	}
}

void ProfileDlg::OnLineOfSight( wxCommandEvent &event )
{
	m_bLineOfSight = event.IsChecked();
	Analyze();
	Refresh();
	UpdateEnabling();
}

void ProfileDlg::OnVisibility( wxCommandEvent &event )
{
	m_bVisibility = event.IsChecked();
	Analyze();
	Refresh();
	UpdateEnabling();
}

void ProfileDlg::OnShowCulture( wxCommandEvent &event )
{
	m_bHaveValues = false;
	m_bGetCulture = true;
	Refresh();
}

void ProfileDlg::OnExportDXF( wxCommandEvent &event )
{
	wxFileDialog saveFile(this, _("Export Profile to DXF"),
		_T(""), _T(""), FSTRING_DXF,
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (bResult)
	{
		wxString str = saveFile.GetPath();
		vtString fname = (const char *) str.mb_str(wxConvUTF8);
		WriteProfileToDXF(fname);
	}
}

void ProfileDlg::OnExportTrace( wxCommandEvent &event )
{
	wxFileDialog saveFile(this, _("Export Trace to DXF"),
		_T(""), _T(""), FSTRING_DXF,
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (bResult)
	{
		wxString str = saveFile.GetPath();
		vtString fname = (const char *) str.mb_str(wxConvUTF8);
		WriteTraceToDXF(fname);
	}
}

void ProfileDlg::OnExportCSV( wxCommandEvent &event )
{
	wxFileDialog saveFile(this, _("Export Trace to CSV"),
		_T(""), _T(""), FSTRING_CSV,
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (bResult)
	{
		wxString str = saveFile.GetPath();
		vtString fname = (const char *) str.mb_str(wxConvUTF8);
		WriteProfileToCSV(fname);
	}
}

void WriteLine(FILE *fp, const char *layer, float x1, float y1, float x2, float y2)
{
	fprintf(fp, "  0\nLINE\n");
	fprintf(fp, "  8\n%s\n", layer);
	fprintf(fp, " 10\n%f\n", x1);
	fprintf(fp, " 20\n%f\n", y1);
	fprintf(fp, " 30\n0.0\n");
	fprintf(fp, " 11\n%f\n", x2);
	fprintf(fp, " 21\n%f\n", y2);
	fprintf(fp, " 31\n0.0\n");
}

void WriteText(FILE *fp, const char *layer, const FPoint2 &p1, const char *text,
			   float fSize)
{
	fprintf(fp, "  0\nTEXT\n");
	fprintf(fp, "  8\n%s\n", layer);
	fprintf(fp, " 10\n%f\n", p1.x);
	fprintf(fp, " 20\n%f\n", p1.y);
	fprintf(fp, " 30\n0.0\n");
	fprintf(fp, " 40\n%f\n", fSize);	// text size
	fprintf(fp, "  1\n%s\n", text);
}

void WriteLine(FILE *fp, const char *layer, const FPoint2 &p1, const FPoint2 &p2)
{
	WriteLine(fp, layer, p1.x, p1.y, p2.x, p2.y);
}

void WriteLines(FILE *fp, const char *layer, int n, const FLine2 &line)
{
	for (int i = 0; i < n-1; i++)
	{
		WriteLine(fp, layer, line[i].x, line[i].y, line[i+1].x, line[i+1].y);
	}
}

/*
Color numbers:
ACAD Color Name
1 RED
2 YELLOW
3 GREEN
4 CYAN
5 BLUE
6 MAGENTA
7 BLACK/WHITE
21 = another cyan?
131 = light red?
*/

void WriteHeader(FILE *fp)
{
	// Header
	fprintf(fp, "  0\nSECTION\n");
	fprintf(fp, "  2\nHEADER\n  9\n$ACADVER\n  1\nAC1009\n");
	fprintf(fp, "  0\nENDSEC\n");

	// Tables section
	fprintf(fp, "  0\nSECTION\n");
	fprintf(fp, "  2\nTABLES\n");

	// ------------------------------------
	// Table of Linetypes
	fprintf(fp, "  0\nTABLE\n");
	fprintf(fp, "  2\nLTYPE\n");
	fprintf(fp, " 70\n2\n");	// max number of linetypes which follow

	// A linetype
	fprintf(fp, "  0\nLTYPE\n");
	fprintf(fp, "  2\nCONTINUOUS\n");	// ltype name
	fprintf(fp, " 70\n0\n");		// ltype flags
	fprintf(fp, "  3\nSolid line\n");	// descriptive text for linetype
	fprintf(fp, " 72\n65\n");		// alignment code, always 65
	fprintf(fp, " 73\n0\n");		// number of dash length items
	fprintf(fp, " 40\n0.0\n");		// total pattern length

	// A linetype
	fprintf(fp, "  0\nLTYPE\n");
	fprintf(fp, "  2\nDASHED\n");	// ltype name
	fprintf(fp, " 70\n0\n");		// ltype flags
	fprintf(fp, "  3\nDashed line\n");	// descriptive text for linetype
	fprintf(fp, " 72\n65\n");		// alignment code, always 65
	fprintf(fp, " 73\n1\n");		// number of dash length items
	fprintf(fp, " 40\n1.0\n");		// total pattern length
	fprintf(fp, " 49\n0.5\n");		// dash length 1

	// end linetypes table
	fprintf(fp, "  0\nENDTAB\n");

	// ------------------------------------
	// Table of Layers
	fprintf(fp, "  0\nTABLE\n");
	fprintf(fp, "  2\nLAYER\n");
	fprintf(fp, " 70\n7\n");	// max number of layers which follow

	// A layer
	fprintf(fp, "  0\nLAYER\n");
	fprintf(fp, "  2\nPEN1\n");	// layer name
	fprintf(fp, " 70\n0\n");	// layer flags
	fprintf(fp, " 62\n7\n");	// color number 7 = black
	fprintf(fp, "  6\nCONTINUOUS\n");	// linetype name

	// A layer
	fprintf(fp, "  0\nLAYER\n");
	fprintf(fp, "  2\nPEN2\n");	// layer name
	fprintf(fp, " 70\n0\n");	// layer flags
	fprintf(fp, " 62\n1\n");	// color number 1 = red
	fprintf(fp, "  6\nDASHED\n");	// linetype name

	// A layer
	fprintf(fp, "  0\nLAYER\n");
	fprintf(fp, "  2\nPEN3\n");	// layer name
	fprintf(fp, " 70\n0\n");	// layer flags
	fprintf(fp, " 62\n3\n");	// color number 3 = green
	fprintf(fp, "  6\nCONTINUOUS\n");	// linetype name

	// A layer
	fprintf(fp, "  0\nLAYER\n");
	fprintf(fp, "  2\nPEN7\n");	// layer name
	fprintf(fp, " 70\n0\n");	// layer flags
	fprintf(fp, " 62\n5\n");	// color number 5 = blue
	fprintf(fp, "  6\nCONTINUOUS\n");	// linetype name

	// end tables layer
	fprintf(fp, "  0\nENDTAB\n");

	// ------------------------------------
	// end tables section
	fprintf(fp, "  0\nENDSEC\n");
}

void ProfileDlg::WriteProfileToDXF(const char *filename)
{
	FILE *fp = fopen(filename, "wb");
	if (!fp)
		return;

	WriteHeader(fp);

	// Entities
	fprintf(fp, "  0\nSECTION\n");
	fprintf(fp, "  2\nENTITIES\n");
	DrawProfileToDXF(fp);
	fprintf(fp, "  0\nENDSEC\n");

	fprintf(fp, "  0\nEOF\n");
	fclose(fp);
}

void ProfileDlg::WriteTraceToDXF(const char *filename)
{
	FILE *fp = fopen(filename, "wb");
	if (!fp)
		return;

	WriteHeader(fp);

	// Entities
	fprintf(fp, "  0\nSECTION\n");
	fprintf(fp, "  2\nENTITIES\n");
	DrawTraceToDXF(fp);
	fprintf(fp, "  0\nENDSEC\n");

	fprintf(fp, "  0\nEOF\n");
	fclose(fp);
}

void ProfileDlg::WriteProfileToCSV(const char *filename)
{
	FILE *fp = fopen(filename, "wb");
	if (!fp)
		return;

	fprintf(fp, "Index, Elevation\n");

	// one datapoint at a time
	for (int i = 0; i < m_xrange; i++)
	{
		float v1 = m_values[i];
		if (v1 == INVALID_ELEVATION)
			continue;

		fprintf(fp, "%d, %.2f\n", i+1, v1);
	}
	fclose(fp);
}

void ProfileDlg::MakePoint(FPoint2 &p, int i, float value)
{
	p.x = i;
	p.y = (value - m_fDrawMin) / m_fDrawRange * m_yrange;
}

void ProfileDlg::MakePoint(const DPoint2 &p_in, DPoint2 &p_out)
{
	p_out.x = (p_in.x - m_DrawOrg.x) * m_DrawScale.x;
	p_out.y = (p_in.y - m_DrawOrg.y) * m_DrawScale.y;
}

void ProfileDlg::DrawProfileToDXF(FILE *fp)
{
	const char *layer = "PEN1";
	WriteLine(fp, layer, 0, 0, m_xrange, 0);
	WriteLine(fp, layer, 0, 0, 0, m_yrange);

	if (!m_bHaveValidData)
		return;

	// Draw tick marks
	int numticks, x, y, tick_spacing = 32;
	vtString str;
	int i;

	// Vertical ticks
	numticks = (m_yrange / tick_spacing)+2;
	for (i = 0; i < numticks; i++)
	{
		y = (i * m_yrange / (numticks-1));

		if (i > 0)
		{
			layer = "PEN2";
			WriteLine(fp, layer, 0, y, 0 + m_xrange, y);
		}

		layer = "PEN1";
		WriteLine(fp, layer, 0 - 5, y, 0 + 5, y);

		str.Format("%5.1f", m_fDrawMin + (m_fDrawRange / (numticks-1) * i));
		//dc.GetTextExtent(str, &w, &h);
		//dc.DrawText(str, MARGIN_LEFT - w - 8, y-(h/2));
		WriteText(fp, layer, FPoint2(-30.0f, y-2.5f), str, 5.0f);
	}
	// Horizontal ticks
	numticks = (m_xrange / tick_spacing)+2;
	for (i = 0; i < numticks; i++)
	{
		x = 0 + (i * m_xrange / (numticks-1));

		layer = "PEN1";
		WriteLine(fp, layer, x, 0 - 5, x, 0 + 5);

		if (m_fGeodesicDistance >= 50000)
			str.Format("%5.0fkm", m_fGeodesicDistance / (numticks-1) * i / 1000);
		else
			str.Format("%5.1f", m_fGeodesicDistance / (numticks-1) * i);
		//dc.GetTextExtent(str, &w, &h);
		//dc.DrawRotatedText(str, x-(h/2), 0 + w + 8, 90);
		WriteText(fp, layer, FPoint2(x - 15.0f, -10.0f), str, 5.0f);
	}

	// Draw surface line
	FPoint2 p1, p2;
	int apply_geoid = (m_bHaveGeoidSurface ? m_iCurvature : 0);

#if 0
	if (m_bVisibility && m_bValidStart)
	{
		bool vis = true;
		bool visr= true;
		layer = "PEN3";
		for (i = 0; i < m_xrange; i++)
		{
			if (m_visible[i] != vis || m_rvisible[i] != visr)
			{
				vis = m_visible[i];
				visr= m_rvisible[i];
				if (vis && visr) layer = "PEN5";
				else if (visr) layer = "PEN6";
				else if (vis) layer = "PEN3";
				else layer = "PEN4";
			}
			float v1 = m_values[i];
			if (v1 == INVALID_ELEVATION)
				continue;

			if (apply_geoid==1)
				v1 += m_GeoidSurface[i];

			MakePoint(p1, i, v1);
			p2 = p1;
			p2.y = 0;
			WriteLine(fp, layer, p1, p2);
		}
	}
	else
	{
#endif
		layer = "PEN3";

		// slow way, one datapoint at a time
		for (i = 0; i < m_xrange-1; i++)
		{
			float v1 = m_values[i];
			float v2 = m_values[i+1];
			if (v1 == INVALID_ELEVATION || v2 == INVALID_ELEVATION)
				continue;
			if (apply_geoid == 1)
			{
				v1+=m_GeoidSurface[i];
				v2+=m_GeoidSurface[i+1];
			}
			MakePoint(p1, i, v1);
			MakePoint(p2, i+1, v2);
			WriteLine(fp, layer, p1, p2);
		}
	//}
#if 0
	// Draw the fresnel zones
	if (m_bUseFresnel && m_bHaveFresnel && m_bHaveLOS)
	{
		wxPoint *pts0 = new wxPoint[m_xrange];
		wxPoint *pts1 = new wxPoint[m_xrange];
		wxPoint *pts2 = new wxPoint[m_xrange];

		for (i=0; i<m_xrange; i++)
		{
			float base=(apply_geoid==2 ? m_GeoidSurface[i] : 0);
			MakePoint(pts1[i], i, m_LineOfSight[i] - m_FirstFresnel[i] - base);
			MakePoint(pts0[i], i, m_LineOfSight[i] - (m_FirstFresnel[i] * 0.60) - base);

			float r=ComputeFresnelRadius(m_fGeodesicDistance * i/m_xrange,m_fRadioFrequency,2);
			MakePoint(pts2[i], i, m_LineOfSight[i] - r - base);
		}

		// object free zone (60% of first zone)
		wxPen fresnelColourOF(wxColour(255,200,180));
		layer = "fresnelColourOF";
		WriteLines(fp, m_xrange, pts0);

		// first zone
		wxPen fresnelColour1(wxColour(255,210,190));
		layer = "fresnelColour1";
		WriteLines(fp, m_xrange, pts1);

		// second zone
		wxPen fresnelColour2(wxColour(255,220,200));
		layer = "fresnelColour2";
		WriteLines(fp, m_xrange, pts2);

		delete [] pts0;
		delete [] pts1;
		delete [] pts2;
	}
#endif
	// Draw Line of Sight
	if (m_bValidLine)
	{
		wxPen orange(wxColour(255,128,0), 1, wxSOLID);
		layer = "ORANGE";
		if (apply_geoid == 2)
		{
			FLine2 pts(m_xrange);
			for (i=0; i<m_xrange; i++)
				MakePoint(pts[i], i, m_LineOfSight[i]);
			WriteLines(fp, layer, m_xrange, pts);
		}
		else
		{
			MakePoint(p1, 0, m_fHeightAtStart);
			MakePoint(p2, m_xrange - 1, m_fHeightAtEnd);
			WriteLine(fp, layer, p1, p2);
		}
	}

#if 0
	// Draw origin of line of sight
	if ((m_bLineOfSight || m_bVisibility) && m_bValidStart)
	{
		// it's hard to see a yellow dot without a bit of outline
		wxPen lightgrey(*wxLIGHT_GREY_PEN);
		layer = "lightgrey";

		wxBrush yellow(wxColour(255,255,0), wxSOLID);
		dc.SetBrush(yellow);
		MakePoint(p1, 0, m_fHeightAtStart);
		dc.DrawCircle(p1, 5);
	}

	// Draw min/max/mouse markers
	wxBrush brush1(wxColour(0,0,255), wxSOLID); // blue: minimum
	dc.SetBrush(brush1);
	MakePoint(p1, m_iMin, m_fMin + (apply_geoid==1 ? m_GeoidSurface[m_iMin] : 0));
	dc.DrawCircle(p1, 5);

	wxBrush brush2(wxColour(255,0,0), wxSOLID); // red: maximum
	dc.SetBrush(brush2);
	MakePoint(p1, m_iMax, m_fMax + (apply_geoid==1 ? m_GeoidSurface[m_iMax] : 0));
	dc.DrawCircle(p1, 5);

	if (m_bMouseOnLine)
	{
		wxBrush brush3(wxColour(0,255,0), wxSOLID); // green: mouse
		dc.SetBrush(brush3);
		MakePoint(p1, m_iMouse, m_fMouse + (apply_geoid==1 ? m_GeoidSurface[m_iMouse] : 0));
		dc.DrawCircle(p1, 5);
	}

	if (m_bIntersectsGround)
	{
		wxBrush brush3(wxColour(255,128,0), wxSOLID);	// orange: intersection
		dc.SetBrush(brush3);
		MakePoint(p1, m_iIntersectIndex, m_fIntersectHeight);
		dc.DrawCircle(p1, 5);
	}
#endif

	if (m_bHaveCulture)
	{
		layer = "PEN7";

		// slow way, one datapoint at a time
		for (i = 0; i < m_xrange-1; i++)
		{
			float v1 = m_values_culture[i];
			float v2 = m_values_culture[i+1];
			float v3 = m_values[i];
			float v4 = m_values[i+1];
			if (v1 == INVALID_ELEVATION && v2 == INVALID_ELEVATION)
				continue;
			if (v1 == INVALID_ELEVATION && v2 != INVALID_ELEVATION)
			{
				if (apply_geoid == 1)
				{
					v4+=m_GeoidSurface[i];
					v2+=m_GeoidSurface[i];
				}
				MakePoint(p1, i+1, v4);
				MakePoint(p2, i+1, v2);
			}
			else if (v1 != INVALID_ELEVATION && v2 == INVALID_ELEVATION)
			{
				if (apply_geoid == 1)
				{
					v1+=m_GeoidSurface[i];
					v3+=m_GeoidSurface[i];
				}
				MakePoint(p1, i, v1);
				MakePoint(p2, i, v3);
			}
			else
			{
				if (apply_geoid == 1)
				{
					v1+=m_GeoidSurface[i];
					v2+=m_GeoidSurface[i];
				}
				MakePoint(p1, i, v1);
				MakePoint(p2, i+1, v2);
			}
			WriteLine(fp, layer, p1, p2);
		}
	}
}

#if SUPPORT_QUIKGRID
// We can use the QuikGrid library to derive and draw contours
#include "vtdata/QuikGrid.h"

void LineCallback(void *context, float x, float y, bool bStart)
{
	static FPoint2 oldp;

	if (bStart)
		oldp.Set(x, y);	// start a new line
	else
	{
		FILE *fp = (FILE *) context;
		FPoint2 newp(x, y);
		WriteLine(fp, "PEN7", oldp.x, oldp.y, newp.x, newp.y);
		oldp = newp;
	}
}
#endif	// SUPPORT_QUIKGRID

void ProfileDlg::DrawTraceToDXF(FILE *fp)
{
	if (!m_bHavePoints && !m_bHavePath)
		return;

	// Gather extents of the area we're going to write
	DRECT ext;
	ext.SetInsideOut();
	ext.GrowToContainLine(m_path);

	// Enforce minimum ratio of width to height
	DPoint2 center = ext.GetCenter();
	double w = ext.Width(), h = ext.Height();
	if (h < w/2)
	{
		h = w/2;
		ext.bottom = center.y - h/2;
		ext.top = center.y + h/2;
	}
	else if (w < h/2)
	{
		w = h/2;
		ext.left = center.x - w/2;
		ext.right = center.x + w/2;
	}

	// Consider larger dimension
	double larger = std::max(w, h);

	// Give it a little space around the sides
	ext.Grow(larger/10, larger/10);
	w = ext.Width();
	h = ext.Height();
	larger = std::max(w, h);

	DPoint2 ratio(w / larger, h / larger);
	DPoint2 drawing_size(w, h);

	m_DrawOrg.Set(ext.left, ext.bottom);
	m_DrawScale.Set(1, 1);

	// Draw rectangle around the drawing
	const char *layer = "PEN1";
	WriteLine(fp, layer, 0, 0, drawing_size.x, 0);
	WriteLine(fp, layer, drawing_size.x, 0, drawing_size.x, drawing_size.y);
	WriteLine(fp, layer, drawing_size.x, drawing_size.y, 0, drawing_size.y);
	WriteLine(fp, layer, 0, drawing_size.y, 0, 0);

	// Text cannot be fixed size, so scale it based on vertical extent
	float text_size = h/20;

#if SUPPORT_QUIKGRID
	// Draw contours
	if (m_bHaveValidData)
	{
		const int samp = 500;
		SurfaceGrid grid(samp,samp);
		DPoint2 p;
		float zmin = 1E9, zmax = -1E9;
		for (int a = 0; a < samp; a++)
		{
			grid.xset(a, a * (drawing_size.x / samp));
			grid.yset(a, a * (drawing_size.y / samp));
		}
		for (int x = 0; x < samp; x++)
		{
			p.x = ext.left + (double) x / samp * w;
			for (int y = 0; y < samp; y++)
			{
				p.y = ext.bottom + (double) y / samp * h;
				float z = m_callback->GetElevation(p);
				if (z != INVALID_ELEVATION)
				{
					grid.zset(x, y, z);
					if (z < zmin) zmin = z;
					if (z > zmax) zmax = z;
				}
			}
		}
		// Don't try to make contours if there is no elevation range
		if (zmin != INVALID_ELEVATION && zmax != zmin)
		{
			SetQuikGridCallbackFunction(LineCallback, fp);
			const int contours = 12;
			for (int c = 1; c < contours-1; c++)
			{
				float fContourValue = zmin + (zmax - zmin) / contours * c;
				Contour(grid, fContourValue);
			}
		}
	}
#endif

	// Draw crosshairs
	DPoint2 p1, p2;
	layer = "PEN3";
	double dist = 0.0f;
	for (uint i = 0; i < m_path.GetSize(); i++)
	{
		MakePoint(m_path[i], p1);
		WriteLine(fp, layer, p1.x - 5, p1.y, p1.x + 5, p1.y);
		WriteLine(fp, layer, p1.x, p1.y - 5, p1.x, p1.y + 5);

		// Label each crosshair with distance along line
		if (i > 0)
			dist += (m_path[i] - m_path[i-1]).Length();

		vtString str;
		if (m_fGeodesicDistance >= 50000)
			str.Format("%5.0fkm", m_fGeoDistAtPoint[i] / 1000);
		else
			str.Format("%5.1f", m_fGeoDistAtPoint[i]);
		WriteText(fp, "PEN1", p1, str, text_size);
	}

	// Draw path
	layer = "PEN1";
	for (uint i = 0; i < m_path.GetSize()-1; i++)
	{
		MakePoint(m_path[i], p1);
		MakePoint(m_path[i+1], p2);
		WriteLine(fp, layer, p1.x, p1.y, p2.x, p2.y);
	}

	// Label the local origin
	const char *format;
	if (m_crs.IsGeographic())
		format = "(%2.7f, %2.7f)";
	else
		format = "(%7.2f, %7.2f)";
	vtString str;
	str.Format(format, ext.left, ext.bottom);
	WriteText(fp, "PEN1", DPoint2(0,0), str, text_size);
}

