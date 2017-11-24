//
// Name: ProfileEditDlg.cpp
//
// Copyright (c) 2006-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/Fence.h"
#include "vtdata/FileFilters.h"
#include "vtdata/PolyChecker.h"
#include "ProfileEditDlg.h"

// WDR: class implementations

BEGIN_EVENT_TABLE(ProfDlgView, wxScrolledWindow)
	EVT_MOUSE_EVENTS(ProfDlgView::OnMouseEvent)
	EVT_SIZE(ProfDlgView::OnSize)
END_EVENT_TABLE()

ProfDlgView::ProfDlgView(wxWindow *parent, wxWindowID id, const wxPoint& pos,
	const wxSize& size, long style, const wxString& name) :
		wxScrolledWindow(parent, id, pos, size, style, name)
{
	m_scale = 30.0f;
	m_bDragging = false;
	m_mode = 0;
}

void ProfDlgView::OnSize(wxSizeEvent& event)
{
	NewSize(event.GetSize());
	Refresh();
}

void ProfDlgView::NewSize(const wxSize &s)
{
	m_org.x = s.x / 2;
	m_org.y = s.y / 3 * 2;
}

void ProfDlgView::OnDraw(wxDC &dc)
{
	// Draw grid
	wxPen pen;
	pen.SetColour(200,200,200);
	dc.SetPen(pen);

	for (int x = -20; x <= 20; x++)
		dc.DrawLine((int)(m_org.x + x * m_scale), (int)m_org.y + -500,
					(int)(m_org.x + x * m_scale), m_org.y + 500);
	for (int y = -20; y <= 20; y++)
		dc.DrawLine(m_org.x + -500, (int)(m_org.y + y * m_scale),
					m_org.x + 500, (int)(m_org.y + y * m_scale));

	// Draw axes
	pen.SetColour(0,0,0);
	dc.SetPen(pen);

	dc.DrawLine(m_org.x + -500, m_org.y,
				m_org.x + 500, m_org.y);
	dc.DrawLine(m_org.x, m_org.y + -500,
				m_org.x, m_org.y + 500);

	// Draw profile
	int size = m_profile.GetSize();
	m_screen.SetSize(size);
	for (int i = 0; i < size; i++)
	{
		m_screen[i].x = (int) (m_org.x + m_profile[i].x * m_scale);
		m_screen[i].y = (int) (m_org.y - m_profile[i].y * m_scale);
	}

	// line itself
	pen.SetColour(0,0,255);
	dc.SetPen(pen);
	for (int i = 0; i < size-1; i++)
	{
		dc.DrawLine(m_screen[i].x, m_screen[i].y,
			m_screen[i+1].x, m_screen[i+1].y);
	}
	// and crossed point at each vertex
	pen.SetColour(0,100,0);
	pen.SetWidth(2);
	dc.SetPen(pen);
	for (int i = 0; i < size; i++)
	{
		dc.DrawLine(m_screen[i].x-3, m_screen[i].y-3,
			m_screen[i].x+3, m_screen[i].y+3);
		dc.DrawLine(m_screen[i].x+3, m_screen[i].y-3,
			m_screen[i].x-3, m_screen[i].y+3);
	}
}

void ProfDlgView::OnMouseEvent(wxMouseEvent &event)
{
	wxCoord x, y;
	event.GetPosition(&x, &y);
	FPoint2 fp((x-m_org.x)/m_scale, -(y-m_org.y)/m_scale);

	wxEventType  ev = event.GetEventType();
	if (ev == wxEVT_LEFT_DOWN)
	{
		// Add
		if (m_mode == 0)
		{
			int size = m_profile.GetSize();
			float dist1, dist2;
			int index1 = -1, index2 = -1;
			FPoint2 intersection;
			if (size < 2)
			{
				// simple case
				m_profile.Append(fp);
			}
			else
			{
				m_profile.NearestPoint(fp, index1, dist1);
				bool seg = m_profile.NearestSegment(fp, index2, dist2, intersection);
				if (!seg || dist1 < dist2)
				{
					// not near any segment
					if (index1 == 0)
						m_profile.InsertPointAfter(-1, fp);
					else
						m_profile.Append(fp);
				}
				else if (seg)
				{
					// insert along the line
					m_profile.InsertPointAfter(index2, fp);
				}
				else
					m_profile.Append(fp);
			}
			Refresh();
		}
		// Move
		if (m_mode == 1)
		{
			int index;
			m_profile.NearestPoint(fp, index);
			if (index != -1)
			{
				m_bDragging = true;
				m_iDragging = index;
			}
		}
		// Remove
		if (m_mode == 2)
		{
			int index;
			m_profile.NearestPoint(fp, index);
			if (index != -1)
			{
				m_profile.RemoveAt(index);
				Refresh();
			}
		}
	}
	else if (ev == wxEVT_LEFT_UP) {
		m_bDragging = false;
	} else if (ev == wxEVT_MIDDLE_DOWN) {
	} else if (ev == wxEVT_MIDDLE_UP) {
	} else if (ev == wxEVT_RIGHT_DOWN) {
	} else if (ev == wxEVT_RIGHT_UP) {
	} else if (ev == wxEVT_MOTION) {
		if (m_bDragging)
		{
			m_profile[m_iDragging] = fp;
			Refresh();
		}
	} else {
		// ignored mouse events, such as wxEVT_LEAVE_WINDOW
		return;
	}

	//if (event1.ControlDown())
	//if (event1.ShiftDown())
	//if (event1.AltDown())
}


//----------------------------------------------------------------------------
// ProfileEditDlg
//----------------------------------------------------------------------------

// WDR: event table for ProfileEditDlg

BEGIN_EVENT_TABLE(ProfileEditDlg,ProfileEditDlgBase)
	EVT_INIT_DIALOG (ProfileEditDlg::OnInitDialog)
	EVT_RADIOBUTTON( ID_ADD_POINT, ProfileEditDlg::OnAdd )
	EVT_RADIOBUTTON( ID_MOVE_POINT, ProfileEditDlg::OnMove )
	EVT_RADIOBUTTON( ID_REMOVE_POINT, ProfileEditDlg::OnRemove )
	EVT_BUTTON( ID_SAVE_PROF, ProfileEditDlg::OnSave )
	EVT_BUTTON( ID_SAVE_AS_PROF, ProfileEditDlg::OnSaveAs )
	EVT_BUTTON( ID_LOAD_PROF, ProfileEditDlg::OnLoad )
	EVT_BUTTON( wxID_OK, ProfileEditDlg::OnOK )
END_EVENT_TABLE()

ProfileEditDlg::ProfileEditDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	ProfileEditDlgBase( parent, id, title, position, size, style )
{
	m_pView = new ProfDlgView(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	m_pView->SetMinSize( wxSize( 240,240 ) );

	viewsizer->Add( m_pView, 1, wxEXPAND | wxALL, 5 );

	GetSizer()->SetSizeHints(this);
}

void ProfileEditDlg::UpdateEnabling()
{
	GetSave()->Enable(m_strFilename != _T(""));
}

void ProfileEditDlg::SetFilename(const char *fname)
{
	if (LoadFLine2FromSHP(fname, m_pView->m_profile))
	{
		m_strFilename = wxString(fname, wxConvUTF8);
		UpdateEnabling();
	}
}

void ProfileEditDlg::CheckClockwisdom()
{
	// We want to stick to a counter-clockwise convention for closed shapes.
	//  Check if they have specified some clockwise points, and flip if so.
	PolyChecker pc;
	if (pc.IsClockwisePolygon(m_pView->m_profile))
	{
		m_pView->m_profile.ReverseOrder();
	}
}

// WDR: handler implementations for ProfileEditDlg

void ProfileEditDlg::OnInitDialog(wxInitDialogEvent& event)
{
	UpdateEnabling();
}

void ProfileEditDlg::OnOK( wxCommandEvent &event )
{
	event.Skip();
}

void ProfileEditDlg::OnLoad( wxCommandEvent &event )
{
	wxFileDialog loadFile(NULL, _("Load Profile"), _T(""), _T(""),
		FSTRING_SHP, wxFD_OPEN);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	wxString str = loadFile.GetPath();
	vtString fname = (const char *) str.mb_str(wxConvUTF8);
	if (LoadFLine2FromSHP(fname, m_pView->m_profile))
	{
		m_strFilename = str;
		Refresh();
		UpdateEnabling();
	}
}

void ProfileEditDlg::OnSaveAs( wxCommandEvent &event )
{
	CheckClockwisdom();

	wxFileDialog saveFile(NULL, _("Save Profile"), _T(""), _T(""),
		FSTRING_SHP, wxFD_SAVE);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	wxString str = saveFile.GetPath();
	vtString fname = (const char *) str.mb_str(wxConvUTF8);
	if (SaveFLine2ToSHP(fname, m_pView->m_profile))
	{
		m_strFilename = str;
		TransferDataToWindow();
		UpdateEnabling();
	}
}

void ProfileEditDlg::OnSave( wxCommandEvent &event )
{
	CheckClockwisdom();

	vtString fname = (const char *) m_strFilename.mb_str(wxConvUTF8);
	SaveFLine2ToSHP(fname, m_pView->m_profile);
}

void ProfileEditDlg::OnRemove( wxCommandEvent &event )
{
	m_pView->m_mode = 2;
}

void ProfileEditDlg::OnMove( wxCommandEvent &event )
{
	m_pView->m_mode = 1;
}

void ProfileEditDlg::OnAdd( wxCommandEvent &event )
{
	m_pView->m_mode = 0;
}
