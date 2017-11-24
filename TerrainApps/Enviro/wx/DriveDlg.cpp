//
// Name:		DriveDlg.cpp
//
// Copyright (c) 2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "DriveDlg.h"
#include "vtlib/core/CarEngine.h"
#include "vtdata/vtLog.h"
#include "vtui/AutoDialog.h"


DriveDlg::DriveDlg( wxWindow* parent ) : DriveDlgBase( parent )
{
	GetSizer()->SetSizeHints(this);

	AddNumValidator(this, ID_SPEED, &m_fSpeed, 2);
	AddNumValidator(this, ID_TURN, &m_fTurn, 3);

	m_bFollow = false;
	m_bMouseDown = false;
}

void DriveDlg::OnLeftDown( wxMouseEvent& event )
{
	VTLOG(" OnLeftDown %d, %d\n", event.GetX(), event.GetY());
	m_bMouseDown = true;
	MouseToMotion(event.GetX(), event.GetY());
}

void DriveDlg::OnLeftUp( wxMouseEvent& event )
{
	VTLOG1(" OnLeftUp\n");
	m_bMouseDown = false;
	if (GetCarEngine())
		GetCarEngine()->SetFriction(0.95f);
}

void DriveDlg::OnLeftDClick( wxMouseEvent& event )
{
	VTLOG1(" OnLeftDClick\n");
	MouseToMotion(event.GetX(), event.GetY());
}

void DriveDlg::OnMotion( wxMouseEvent& event )
{
	VTLOG1(" OnMotion\n");
	if (m_bMouseDown)
		MouseToMotion(event.GetX(), event.GetY());
}

void DriveDlg::OnAreaPaint( wxPaintEvent& event )
{
	VTLOG1(" OnAreaPaint\n");
	wxPaintDC dc(m_area);

	// Draw X and Y axes
	wxPen pen1(*wxBLACK_PEN);
	dc.SetPen(pen1);
	dc.DrawLine(0, 120, 320, 120);
	dc.DrawLine(160, 0, 160, 140);

	event.Skip();
}

void DriveDlg::OnFollow(wxCommandEvent& event)
{
	if (GetCarEngine())
		GetCarEngine()->SetCameraFollow(event.IsChecked());
}

void DriveDlg::MouseToMotion(int mx, int my)
{
	CarEngine *eng = GetCarEngine();
	if (!eng)
		return;

	eng->SetFriction(1.0f);

	int x = -(mx - 160);
	int y = (140-my) - 20;


	m_fSpeed = y / 5.0f;
	m_fTurn = x / 2500.0f;

	VTLOG(" move %f, turn %f\n", y / 10.0f, x / 100.0f);

	eng->SetSpeed(m_fSpeed);
	eng->SetSteeringAngle(m_fTurn);

	TransferDataToWindow();
}

void DriveDlg::OnScroll( wxScrollEvent& event )
{
	int val = event.GetInt();
	VTLOG(" OnScroll %d\n", val);

	if (GetCarEngine())
		GetCarEngine()->SetCameraDistance(3.0f + val);
}

