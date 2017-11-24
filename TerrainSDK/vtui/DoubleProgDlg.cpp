//
// DoubleProgressDialog class
//
// Copyright (c) 2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
	#include "wx/utils.h"
	#include "wx/frame.h"
	#include "wx/button.h"
	#include "wx/stattext.h"
	#include "wx/sizer.h"
	#include "wx/event.h"
	#include "wx/gauge.h"
	#include "wx/intl.h"
	#include "wx/dcclient.h"
	#include "wx/timer.h"
	#include "wx/settings.h"
#endif

#include "DoubleProgDlg.h"

// ---------------------------------------------------------------------------
// macros
// ---------------------------------------------------------------------------

// Macro for avoiding #ifdefs when value have to be different.
#define wxLARGESMALL(large,small) large

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

#define LAYOUT_MARGIN 4

// ----------------------------------------------------------------------------
// private functions
// ----------------------------------------------------------------------------

// update the label to show the given time (in seconds)
static void SetTimeLabel(unsigned long val, wxStaticText *label);

// ----------------------------------------------------------------------------
// event tables
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(DoubleProgressDialog, wxDialog)
	EVT_BUTTON(wxID_CANCEL, DoubleProgressDialog::OnCancel)
	EVT_CLOSE(DoubleProgressDialog::OnClose)
END_EVENT_TABLE()

IMPLEMENT_CLASS(DoubleProgressDialog, wxDialog)

// ============================================================================
// DoubleProgressDialog implementation
// ============================================================================

// ----------------------------------------------------------------------------
// DoubleProgressDialog creation
// ----------------------------------------------------------------------------

DoubleProgressDialog::DoubleProgressDialog(wxString const &title,
								   wxString const &message,
								   wxWindow *parent,
								   int style) :
	wxDialog(parent, wxID_ANY, title), m_delay(3), m_hasAbortButton(false)
{
	// we may disappear at any moment, let the others know about it
	SetExtraStyle(GetExtraStyle() | wxWS_EX_TRANSIENT);
	m_windowStyle |= style;

	m_hasAbortButton = (style & wxPD_CAN_ABORT) != 0;

	bool isPda = (wxSystemSettings::GetScreenType() <= wxSYS_SCREEN_PDA);

#if defined(__WXMSW__) && !defined(__WXUNIVERSAL__)
	// we have to remove the "Close" button from the title bar then as it is
	// confusing to have it - it doesn't work anyhow
	//
	// FIXME: should probably have a (extended?) window style for this
	if ( !m_hasAbortButton )
	{
		EnableCloseButton(false);
	}
#endif // wxMSW

	m_state = m_hasAbortButton ? Continue : Uncancelable;
	m_maximum = 100;

	m_parentTop = wxGetTopLevelParent(parent);

	wxClientDC dc(this);
	dc.SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	wxCoord widthText = 0;
	dc.GetTextExtent(message, &widthText, NULL, NULL, NULL, NULL);

	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

	m_msg = new wxStaticText(this, wxID_ANY, message);
	sizer->Add(m_msg, 0, wxLEFT | wxTOP, 2*LAYOUT_MARGIN);

	wxSize sizeDlg,
		   sizeLabel = m_msg->GetSize();
	sizeDlg.y = 2*LAYOUT_MARGIN + sizeLabel.y;

	if ( m_maximum > 0 )
	{
		int gauge_style = wxGA_HORIZONTAL;
		if ( ( style & wxPD_SMOOTH ) == wxPD_SMOOTH )
			gauge_style |= wxGA_SMOOTH;

		m_gauge1 = new wxGauge(this, wxID_ANY, m_maximum,
							  wxDefaultPosition, wxSize(-1, 20),
							  gauge_style );
		sizer->Add(m_gauge1, 0, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, 2*LAYOUT_MARGIN);
		m_gauge1->SetValue(0);

		m_gauge2 = new wxGauge(this, wxID_ANY, m_maximum,
							  wxDefaultPosition, wxSize(-1, 20),
							  gauge_style );
		sizer->Add(m_gauge2, 0, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, 2*LAYOUT_MARGIN);
		m_gauge2->SetValue(0);

		wxSize sizeGauge = m_gauge1->GetSize();
		sizeDlg.y += 2*LAYOUT_MARGIN + sizeGauge.y;
		sizeDlg.y += 2*LAYOUT_MARGIN + sizeGauge.y;
	}
	else
	{
		m_gauge1 = (wxGauge *)NULL;
		m_gauge2 = (wxGauge *)NULL;
	}

	// if we are going to have at least one label, remmeber it in this var
	wxStaticText *label = NULL;

	// also count how many labels we really have
	size_t nTimeLabels = 0;

	m_btnAbort = (wxButton *)NULL;
	bool sizeDlgModified = false;
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);

	const int sizerFlags =
#if defined(__WXMSW__) || defined(__WXPM__)
						   wxALIGN_RIGHT | wxALL
#else // !MSW
						   wxALIGN_CENTER_HORIZONTAL | wxBOTTOM | wxTOP
#endif // MSW/!MSW
						   ;

	if ( m_hasAbortButton )
	{
		m_btnAbort = new wxButton(this, wxID_CANCEL);

		// Windows dialogs usually have buttons in the lower right corner
		buttonSizer->Add(m_btnAbort, 0, sizerFlags, LAYOUT_MARGIN);
		if(!sizeDlgModified)
			sizeDlg.y += 2*LAYOUT_MARGIN + wxButton::GetDefaultSize().y;
	}

	sizer->Add(buttonSizer, 0, sizerFlags, LAYOUT_MARGIN );

	SetSizerAndFit(sizer);

	if (!isPda)
	{
		sizeDlg.y += 2*LAYOUT_MARGIN;

		// try to make the dialog not square but rectangular of reasonable width
		sizeDlg.x = (wxCoord)wxMax(widthText, 4*sizeDlg.y/3);
		sizeDlg.x *= 3;
		sizeDlg.x /= 2;
		SetClientSize(sizeDlg);
	}

	Centre(wxCENTER_FRAME | wxBOTH);

	if ( style & wxPD_APP_MODAL )
	{
		m_winDisabler = new wxWindowDisabler(this);
	}
	else
	{
		if ( m_parentTop )
			m_parentTop->Disable();
		m_winDisabler = NULL;
	}

	Show();
	Enable();
	Update();
}

wxStaticText *DoubleProgressDialog::CreateLabel(const wxString& text,
											wxSizer *sizer)
{
	wxBoxSizer *locsizer = new wxBoxSizer(wxLARGESMALL(wxHORIZONTAL,wxVERTICAL));

	wxStaticText *dummy = new wxStaticText(this, wxID_ANY, text);
	wxStaticText *label = new wxStaticText(this, wxID_ANY, _("unknown"));

	// select placement most native or nice on target GUI
#if defined(__WXMSW__) || defined(__WXPM__) || defined(__WXMAC__) || defined(__WXGTK20__)
	// label and time centered in one row
	locsizer->Add(dummy, 1, wxLARGESMALL(wxALIGN_RIGHT,wxALIGN_LEFT));
	locsizer->Add(label, 1, wxALIGN_LEFT | wxLEFT, LAYOUT_MARGIN);
	sizer->Add(locsizer, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP, LAYOUT_MARGIN);
#else
	// label and time to the right in one row
	sizer->Add(locsizer, 0, wxALIGN_RIGHT | wxRIGHT | wxTOP, LAYOUT_MARGIN);
	locsizer->Add(dummy);
	locsizer->Add(label, 0, wxLEFT, LAYOUT_MARGIN);
#endif

	return label;
}

// ----------------------------------------------------------------------------
// DoubleProgressDialog operations
// ----------------------------------------------------------------------------

bool
DoubleProgressDialog::Update(int value1, int value2, const wxString& newmsg)
{
	wxASSERT_MSG( value1 <= m_maximum, wxT("invalid progress value") );
	wxASSERT_MSG( value2 <= m_maximum, wxT("invalid progress value") );

	// fill up the gauge if value == maximum because this means that the dialog
	// is going to close and the gauge shouldn't be partly empty in this case
	if ( m_gauge1 && value1 != -1 )
		m_gauge1->SetValue(value1 == m_maximum ? value1 : value1 + 1);

	if ( m_gauge2 && value2 != -1 )
		m_gauge2->SetValue(value2);

	UpdateMessage(newmsg);

	if ( value1 == m_maximum )
	{
		if ( m_state == Finished )
		{
			// ignore multiple calls to Update(m_maximum): it may sometimes be
			// troublesome to ensure that Update() is not called twice with the
			// same value (e.g. because of the rounding errors) and if we don't
			// return now we're going to generate asserts below
			return true;
		}

		// so that we return true below and that out [Cancel] handler knew what
		// to do
		m_state = Finished;
		if( !(GetWindowStyle() & wxPD_AUTO_HIDE) )
		{
			EnableClose();
#if defined(__WXMSW__) && !defined(__WXUNIVERSAL__)
			EnableCloseButton();
#endif // __WXMSW__

			if ( newmsg.empty() )
			{
				// also provide the finishing message if the application didn't
				m_msg->SetLabel(_("Done."));
			}

			DoYield();

			(void)ShowModal();
		}
		else // auto hide
		{
			// reenable other windows before hiding this one because otherwise
			// Windows wouldn't give the focus back to the window which had
			// been previously focused because it would still be disabled
			ReenableOtherWindows();

			Hide();
		}
	}
	else // not at maximum yet
	{
		return DoAfterUpdate();
	}

	// update the display in case yielding above didn't do it
	Update();

	return m_state != Canceled;
}

bool DoubleProgressDialog::DoAfterUpdate()
{
	if (m_hasAbortButton)
	{
		// we have to yield because not only we want to update the display but
		// also to process the clicks on the cancel button
		DoYield();
	}

	Update();

	return m_state != Canceled;
}

void DoubleProgressDialog::Resume()
{
	m_state = Continue;

	EnableAbort();
}

bool DoubleProgressDialog::Show( bool show )
{
	// reenable other windows before hiding this one because otherwise
	// Windows wouldn't give the focus back to the window which had
	// been previously focused because it would still be disabled
	if(!show)
		ReenableOtherWindows();

	return wxDialog::Show(show);
}

// ----------------------------------------------------------------------------
// event handlers
// ----------------------------------------------------------------------------

void DoubleProgressDialog::OnCancel(wxCommandEvent& event)
{
	if ( m_state == Finished )
	{
		// this means that the count down is already finished and we're being
		// shown as a modal dialog - so just let the default handler do the job
		event.Skip();
	}
	else
	{
		// request to cancel was received, the next time Update() is called we
		// will handle it
		m_state = Canceled;

		// update the buttons state immediately so that the user knows that the
		// request has been noticed
		DisableAbort();
	}
}

void DoubleProgressDialog::OnClose(wxCloseEvent& event)
{
	if ( m_state == Uncancelable )
	{
		// can't close this dialog
		event.Veto();
	}
	else if ( m_state == Finished )
	{
		// let the default handler close the window as we already terminated
		event.Skip();
	}
	else
	{
		// next Update() will notice it
		m_state = Canceled;
		DisableAbort();
	}
}

// ----------------------------------------------------------------------------
// destruction
// ----------------------------------------------------------------------------

DoubleProgressDialog::~DoubleProgressDialog()
{
	// normally this should have been already done, but just in case
	ReenableOtherWindows();
}

void DoubleProgressDialog::ReenableOtherWindows()
{
	if ( GetWindowStyle() & wxPD_APP_MODAL )
	{
		delete m_winDisabler;
		m_winDisabler = NULL;
	}
	else
	{
		if ( m_parentTop )
			m_parentTop->Enable();
	}
}

// ----------------------------------------------------------------------------
// private functions
// ----------------------------------------------------------------------------

static void SetTimeLabel(unsigned long val, wxStaticText *label)
{
	if ( label )
	{
		wxString s;

		if (val != (unsigned long)-1)
		{
		unsigned long hours = val / 3600;
		unsigned long minutes = (val % 3600) / 60;
		unsigned long seconds = val % 60;
		s.Printf(wxT("%lu:%02lu:%02lu"), hours, minutes, seconds);
		}
		else
		{
			s = _("Unknown");
		}

		if ( s != label->GetLabel() )
			label->SetLabel(s);
	}
}

void DoubleProgressDialog::EnableAbort(bool enable)
{
	if(m_hasAbortButton)
	{
		if(m_btnAbort)
			m_btnAbort->Enable(enable);
	}
}

void DoubleProgressDialog::EnableClose()
{
	if(m_hasAbortButton)
	{
		if(m_btnAbort)
		{
			m_btnAbort->Enable();
			m_btnAbort->SetLabel(_("Close"));
		}
	}
}

void DoubleProgressDialog::UpdateMessage(const wxString &newmsg)
{
	if ( !newmsg.empty() && newmsg != m_msg->GetLabel() )
	{
		m_msg->SetLabel(newmsg);
		DoYield();
	}
}

void DoubleProgressDialog::DoYield()
{
#ifndef __WXGTK__
	// For some reason, calling wxYieldIfNeeded on Linux seems to cause the
	//  application to lock up quite often.  And it seems to work OK on that
	//  platform if we don't even call Yield at all.
	wxYieldIfNeeded() ;
#endif
}

