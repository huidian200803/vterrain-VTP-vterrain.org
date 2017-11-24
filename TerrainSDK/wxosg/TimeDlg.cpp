//
// Name: TimeDlg.cpp
//
// Copyright (c) 2004-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "TimeDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtui/AutoDialog.h"
#include "TimeDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// TimeDlg
//----------------------------------------------------------------------------

// WDR: event table for TimeDlg

BEGIN_EVENT_TABLE(TimeDlg, TimeDlgBase)
	EVT_INIT_DIALOG (TimeDlg::OnInitDialog)
	EVT_BUTTON( ID_BUTTON_STOP, TimeDlg::OnStop )
	EVT_TEXT( ID_TEXT_SPEED, TimeDlg::OnTextSpeed )
	EVT_SLIDER( ID_SLIDER_SPEED, TimeDlg::OnSliderSpeed )
	EVT_SPINCTRL( ID_SPIN_YEAR, TimeDlg::OnSpinner )
	EVT_SPINCTRL( ID_SPIN_MONTH, TimeDlg::OnSpinner )
	EVT_SPINCTRL( ID_SPIN_DAY, TimeDlg::OnSpinner )
	EVT_SPINCTRL( ID_SPIN_HOUR, TimeDlg::OnSpinner )
	EVT_SPINCTRL( ID_SPIN_MINUTE, TimeDlg::OnSpinner )
	EVT_SPINCTRL( ID_SPIN_SECOND, TimeDlg::OnSpinner )
END_EVENT_TABLE()

TimeDlg::TimeDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	TimeDlgBase( parent, id, title, position, size, style )
{
	m_pTop = GetSizer();

	m_bGoing = false;
	m_iSpeed = 0;
	m_fSpeed = 0.0f;
	m_bSetting = false;
	m_pTimeEngine = NULL;

	year = 2000;
	month = 1;
	day = 1;
	hour = 12;
	minute = 0;
	second = 0;

	AddValidator(this, ID_SPIN_YEAR, &year);
	AddValidator(this, ID_SPIN_MONTH, &month);
	AddValidator(this, ID_SPIN_DAY, &day);

	AddValidator(this, ID_SPIN_HOUR, &hour);
	AddValidator(this, ID_SPIN_MINUTE, &minute);
	AddValidator(this, ID_SPIN_SECOND, &second);

	AddNumValidator(this, ID_TEXT_SPEED, &m_fSpeed);
	AddValidator(this, ID_SLIDER_SPEED, &m_iSpeed);

	GetSliderSpeed()->Enable(false);
	GetTextSpeed()->Enable(false);
	GetStop()->Enable(false);

	GetSizer()->SetSizeHints(this);
}

void TimeDlg::AddOkCancel()
{
	wxBoxSizer *item25 = new wxBoxSizer( wxHORIZONTAL );

	wxButton *item26 = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	item25->Add( item26, 0, wxALIGN_CENTER|wxALL, 5 );

	wxButton *item27 = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	item25->Add( item27, 0, wxALIGN_CENTER|wxALL, 5 );

	m_pTop->Add( item25, 0, wxALIGN_CENTER|wxALL, 5 );

	m_pTop->Fit( this );
	m_pTop->SetSizeHints( this );
}

void TimeDlg::PullTime()
{
	if (m_pTimeEngine)
	{
		vtTime time = m_pTimeEngine->GetTime();
		SetTimeControls(time);
	}
}

void TimeDlg::PushTime()
{
	TransferDataFromWindow();

	if (m_pTimeEngine)
	{
		vtTime time;
		time.SetDate(year, month, day);
		time.SetTimeOfDay(hour, minute, second);

		m_pTimeEngine->SetTime(time);

		time.GetDate(year, month, day);
		time.GetTimeOfDay(hour, minute, second);
	}
}

void TimeDlg::SetTime(const vtTime &time)
{
	if (m_pTimeEngine)
		m_fSpeed = m_pTimeEngine->GetSpeed();
	m_bGoing = (m_fSpeed != 0.0f);

	UpdateSlider();

	UpdateEnabling();

	SetTimeControls(time);
}

void TimeDlg::GetTime(vtTime &time)
{
	time.SetDate(year, month, day);
	time.SetTimeOfDay(hour, minute, second);
}

void TimeDlg::SetTimeEngine(vtTimeEngine *pEngine)
{
	m_pTimeEngine = pEngine;

	vtTimeTarget *self = dynamic_cast<vtTimeTarget *>(this);

	// If the engine is not yet connected to us, connect it.
	bool bFound = false;
	for (uint i = 0; i < pEngine->NumTargets(); i++)
	{
		if (pEngine->GetTarget(i) == self)
			bFound = true;
	}
	if (!bFound)
		pEngine->AddTarget(self);

	GetSliderSpeed()->Enable(true);
	GetTextSpeed()->Enable(true);
	GetStop()->Enable(true);
}

void TimeDlg::SetTimeControls(const vtTime &time)
{
	time.GetDate(year, month, day);
	time.GetTimeOfDay(hour, minute, second);

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void TimeDlg::OnInitDialog(wxInitDialogEvent& event)
{
	PullTime();
	wxDialog::OnInitDialog(event);
}

// WDR: handler implementations for TimeDlg

void TimeDlg::OnSpinner( wxSpinEvent &event )
{
	PushTime();
}

void TimeDlg::UpdateEnabling()
{
	GetStop()->Enable(m_bGoing);
}

void TimeDlg::UpdateSlider()
{
	if (m_fSpeed == 0.0f)
		m_iSpeed = 0;
	else
		m_iSpeed = (int) sqrt(m_fSpeed);
	m_bSetting = true;
	GetSliderSpeed()->SetValue(m_iSpeed);
	m_bSetting = false;
}

void TimeDlg::OnSliderSpeed( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	m_fSpeed = m_iSpeed * m_iSpeed;

	m_bSetting = true;
	m_pTimeEngine->SetSpeed(m_fSpeed);
	m_bSetting = false;
}

void TimeDlg::OnTextSpeed( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	UpdateSlider();

	if (m_pTimeEngine)
		m_pTimeEngine->SetSpeed(m_fSpeed);
}

void TimeDlg::OnStop( wxCommandEvent &event )
{
	if (m_pTimeEngine)
		m_pTimeEngine->SetSpeed(0.0f);
	UpdateEnabling();
}

