//
// Name: DistanceDlg2d.cpp
//
// Copyright (c) 2006-2009 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "Frame.h"
#include "BuilderView.h"
#include "DistanceDlg2d.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// DistanceDlg2d
//----------------------------------------------------------------------------

// WDR: event table for DistanceDlg2d

DistanceDlg2d::DistanceDlg2d( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	DistanceDlg( parent, id, title, position, size, style )
{
	m_pFrame = NULL;

	GetSizer()->SetSizeHints(this);
}

void DistanceDlg2d::OnMode(bool bPath)
{
	if (!m_pFrame)
		return;

	// If we are switching mode, erase previous distance
	bool previous = m_pFrame->GetView()->GetDistanceToolMode();
	if (bPath != previous)
		m_pFrame->ClearDistance();

	m_pFrame->GetView()->SetDistanceToolMode(bPath);
}

void DistanceDlg2d::SetPathToBase(const DLine2 &path)
{
	m_pFrame->GetView()->SetDistancePath(path);
	m_pFrame->GetView()->UpdateDistance();
}

void DistanceDlg2d::Reset()
{
	if (!m_pFrame)
		return;

	m_pFrame->ClearDistance();
}

