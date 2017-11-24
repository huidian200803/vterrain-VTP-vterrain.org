//
// Name: RoadDlg.cpp
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtui/AutoDialog.h"

#include "RoadDlg.h"
#include "RoadLayer.h"

#define MULTIPLE	5000	// special value meaning "more than one value"

// WDR: class implementations

//----------------------------------------------------------------------------
// RoadDlg
//----------------------------------------------------------------------------

// WDR: event table for RoadDlg

BEGIN_EVENT_TABLE(RoadDlg, RoadDlgBase)
	EVT_INIT_DIALOG (RoadDlg::OnInitDialog)
	EVT_BUTTON( wxID_OK, RoadDlg::OnOK )
END_EVENT_TABLE()

RoadDlg::RoadDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	RoadDlgBase( parent, id, title, position, size, style )
{
	GetSurfType()->Append(_("None"));
	GetSurfType()->Append(_("Gravel"));
	GetSurfType()->Append(_("Trail"));
	GetSurfType()->Append(_("2 Track"));
	GetSurfType()->Append(_("Dirt"));
	GetSurfType()->Append(_("Paved"));
	GetSurfType()->Append(_("Railroad"));
	GetSurfType()->Append(_("Stone"));
	GetSurfType()->Append(_("(multiple types)"));
	GetSurfType()->SetSelection(0);

	GetSidewalk()->Append(_("None"));
	GetSidewalk()->Append(_("Left"));
	GetSidewalk()->Append(_("Right"));
	GetSidewalk()->Append(_("Both"));
	GetSidewalk()->Append(_("(multiple)"));
	GetSidewalk()->SetSelection(0);

	GetParking()->Append(_("None"));
	GetParking()->Append(_("Left"));
	GetParking()->Append(_("Right"));
	GetParking()->Append(_("Both"));
	GetParking()->Append(_("(multiple)"));
	GetParking()->SetSelection(0);

	GetMargin()->Append(_("No"));
	GetMargin()->Append(_("Yes"));
	GetMargin()->Append(_("(multiple)"));
	GetMargin()->SetSelection(0);

	GetSizer()->SetSizeHints(this);
}

void RoadDlg::SetRoad(LinkEdit *pSingleRoad, vtRoadLayer *pLayer)
{
	m_pRoad = pSingleRoad;
	m_pLayer = pLayer;
}

void RoadDlg::ClearState()
{
	m_iLanes = -1;
	m_iHwy = -1;
	m_iSidewalk = -1;
	m_iParking = -1;
	m_iMargin = -1;
	m_iSurf = -1;
	m_fSidewalkWidth = -1.0f;
	m_fCurbHeight = -1.0f;
	m_fMarginWidth = -1.0f;
	m_fLaneWidth = -1.0f;
	m_fParkingWidth = -1.0f;
}

void RoadDlg::AccumulateState(LinkEdit *pRoad)
{
	if (m_iLanes == -1)
		m_iLanes = pRoad->m_iLanes;
	if (pRoad->m_iLanes != m_iLanes)
		m_iLanes = MULTIPLE;

	if (m_iHwy == -1)
		m_iHwy = pRoad->m_iHwy;
	if (pRoad->m_iHwy != m_iHwy)
		m_iHwy = MULTIPLE;

	if (m_iSidewalk == -1)
		m_iSidewalk = pRoad->GetSidewalk();
	if (pRoad->GetSidewalk() != m_iSidewalk)
		m_iSidewalk = MULTIPLE;

	if (m_iParking == -1)
		m_iParking = pRoad->GetParking();
	if (pRoad->GetParking() != m_iParking)
		m_iParking = MULTIPLE;

	if (m_iMargin == -1)
		m_iMargin = pRoad->GetFlag(RF_MARGIN);
	if (pRoad->GetFlag(RF_MARGIN) != m_iMargin)
		m_iMargin = MULTIPLE;

	if (m_iSurf == -1)
		m_iSurf = pRoad->m_Surface;
	if (pRoad->m_Surface != m_iSurf)
		m_iSurf = MULTIPLE;

	if (m_fSidewalkWidth == -1.0f)
		m_fSidewalkWidth = pRoad->m_fSidewalkWidth;
	else if (pRoad->m_fSidewalkWidth != m_fSidewalkWidth)
		m_fSidewalkWidth = MULTIPLE;

	if (m_fCurbHeight == -1.0f)
		m_fCurbHeight = pRoad->m_fCurbHeight;
	else if (pRoad->m_fCurbHeight != m_fCurbHeight)
		m_fCurbHeight = MULTIPLE;

	if (m_fMarginWidth == -1.0f)
		m_fMarginWidth = pRoad->m_fMarginWidth;
	else if (pRoad->m_fMarginWidth != m_fMarginWidth)
		m_fMarginWidth = MULTIPLE;

	if (m_fLaneWidth == -1.0f)
		m_fLaneWidth = pRoad->m_fLaneWidth;
	else if (pRoad->m_fLaneWidth != m_fLaneWidth)
		m_fLaneWidth = MULTIPLE;

	if (m_fParkingWidth == -1.0f)
		m_fParkingWidth = pRoad->m_fParkingWidth;
	else if (pRoad->m_fParkingWidth != m_fParkingWidth)
		m_fParkingWidth = MULTIPLE;
}

void RoadDlg::TransferStateToControls()
{
	wxString str;

	if (m_iLanes == MULTIPLE)
		str = _("(multiple)");
	else
		str.Printf(_T("%d"), m_iLanes);
	GetNumLanes()->SetValue(str);

	if (m_iHwy == MULTIPLE)
		str = _("(multiple)");
	else
		str.Printf(_T("%d"), m_iHwy);
	GetHwyName()->SetValue(str);

	if (m_iSidewalk == MULTIPLE)
		GetSidewalk()->SetSelection(4);
	else
		GetSidewalk()->SetSelection(m_iSidewalk);

	if (m_iParking == MULTIPLE)
		GetParking()->SetSelection(4);
	else
		GetParking()->SetSelection(m_iParking);

	if (m_iMargin == MULTIPLE)
		GetMargin()->SetSelection(2);
	else
		GetMargin()->SetSelection(m_iMargin);

	if (m_iSurf == MULTIPLE)
		GetSurfType()->SetSelection(8);
	else
		GetSurfType()->SetSelection(m_iSurf);

	if (m_fSidewalkWidth == MULTIPLE)
		str = _("(multiple)");
	else
		str.Printf(_T("%.02f"), m_fSidewalkWidth);
	GetSidewalkWidth()->SetValue(str);

	if (m_fCurbHeight == MULTIPLE)
		str = _("(multiple)");
	else
		str.Printf(_T("%.02f"), m_fCurbHeight);
	GetCurbHeight()->SetValue(str);

	if (m_fMarginWidth == MULTIPLE)
		str = _("(multiple)");
	else
		str.Printf(_T("%.02f"), m_fMarginWidth);
	GetMarginWidth()->SetValue(str);

	if (m_fLaneWidth == MULTIPLE)
		str = _("(multiple)");
	else
		str.Printf(_T("%.02f"), m_fLaneWidth);
	GetLaneWidth()->SetValue(str);

	if (m_fParkingWidth == MULTIPLE)
		str = _("(multiple)");
	else
		str.Printf(_T("%.02f"), m_fParkingWidth);
	GetParkingWidth()->SetValue(str);
}

void RoadDlg::OnInitDialog(wxInitDialogEvent& event)
{
	ClearState();
	if (m_pRoad)
		AccumulateState(m_pRoad);
	else
	{
		LinkEdit *pRoad;
		for (pRoad = m_pLayer->GetFirstLink(); pRoad; pRoad=pRoad->GetNext())
		{
			if (pRoad->IsSelected())
				AccumulateState(pRoad);
		}
	}
	TransferStateToControls();
}

// WDR: handler implementations for RoadDlg

void RoadDlg::OnOK( wxCommandEvent &event )
{
	if (m_pRoad)
		ApplyState(m_pRoad);
	else
	{
		LinkEdit *pRoad;
		for (pRoad = m_pLayer->GetFirstLink(); pRoad; pRoad=pRoad->GetNext())
		{
			if (pRoad->IsSelected())
				ApplyState(pRoad);
		}
	}
	event.Skip();
}

//
// Apply the state directly from the controls to a given road.
//
void RoadDlg::ApplyState(LinkEdit *pRoad)
{
	wxString str;
	int val;

	str = GetNumLanes()->GetValue();
	val = atoi(str.mb_str(wxConvUTF8));
	if (val != 0)
		pRoad->m_iLanes = val;

	str = GetHwyName()->GetValue();
	val = atoi(str.mb_str(wxConvUTF8));
		pRoad->m_iHwy = val;

	val = GetSidewalk()->GetSelection();
	if (val != 4)
	{
		pRoad->SetFlag(RF_SIDEWALK_LEFT, (val & 1) != 0);
		pRoad->SetFlag(RF_SIDEWALK_RIGHT, (val & 2) != 0);
	}

	val = GetParking()->GetSelection();
	if (val != 4)
	{
		pRoad->SetFlag(RF_PARKING_LEFT, (val & 1) != 0);
		pRoad->SetFlag(RF_PARKING_RIGHT, (val & 2) != 0);
	}

	val = GetMargin()->GetSelection();
	if (val != 2)
		pRoad->SetFlag(RF_MARGIN, (val != 0));

	val = GetSurfType()->GetSelection();
	if (val != 8)
		pRoad->m_Surface = (SurfaceType) val;

	float fval;

	str = GetMarginWidth()->GetValue();
	fval = atof(str.mb_str(wxConvUTF8));
	if (fval != 0.0f)
	{
		pRoad->m_fMarginWidth = fval;
		// Force recompute of m_fWidth
		pRoad->Dirtied();
	}

	str = GetParkingWidth()->GetValue();
	fval = atof(str.mb_str(wxConvUTF8));
	if (fval != 0.0f)
	{
		pRoad->m_fParkingWidth = fval;
		pRoad->Dirtied();
	}

	str = GetCurbHeight()->GetValue();
	fval = atof(str.mb_str(wxConvUTF8));
	if (fval != 0.0f)
	{
		pRoad->m_fCurbHeight = fval;
		pRoad->Dirtied();
	}

	str = GetSidewalkWidth()->GetValue();
	fval = atof(str.mb_str(wxConvUTF8));
	if (fval != 0.0f)
	{
		pRoad->m_fSidewalkWidth = fval;
		pRoad->Dirtied();
	}

	str = GetLaneWidth()->GetValue();
	fval = atof(str.mb_str(wxConvUTF8));
	if (fval != 0.0f)
	{
		pRoad->m_fLaneWidth = fval;
		pRoad->Dirtied();
	}
}

