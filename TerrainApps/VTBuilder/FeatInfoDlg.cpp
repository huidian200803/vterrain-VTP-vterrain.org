//
// Name:		FeatInfoDlg.cpp
//
// Copyright (c) 2002-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "FeatInfoDlg.h"
#include "BuilderView.h"


FeatInfoDlg::FeatInfoDlg( wxWindow *parent, wxWindowID id,
	const wxString &title, const wxPoint &position, const wxSize& size, long style ) :
	FeatureTableDlg(parent, id, title, position, size, style)
{
	m_pLayer = NULL;
	m_pView = NULL;

	GetSizer()->SetSizeHints(this);
}

void FeatInfoDlg::OnModified()
{
	m_pLayer->SetModified(true);
}

void FeatInfoDlg::RefreshViz()
{
	m_pView->Refresh();
}

