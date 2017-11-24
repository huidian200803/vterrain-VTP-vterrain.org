//
// Name: TagDlg.cpp
//
// Copyright (c) 2007-2011 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "TagDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// TagDlg
//----------------------------------------------------------------------------

// WDR: event table for TagDlg

BEGIN_EVENT_TABLE(TagDlg,TagDlgBase)
END_EVENT_TABLE()

TagDlg::TagDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	TagDlgBase( parent, id, title, position, size, style )
{
	m_pList = GetList();

	m_pList->ClearAll();
	m_pList->SetSingleStyle(wxLC_REPORT);
	m_pList->InsertColumn(0, _T("Tag"));
	m_pList->SetColumnWidth(0, 60);
	m_pList->InsertColumn(1, _T("Value"));
	m_pList->SetColumnWidth(1, 320);

	GetSizer()->SetSizeHints(this);
}

void TagDlg::SetTags(vtTagArray *tags)
{
	m_pList->DeleteAllItems();
	vtTag *tag;
	for (uint i = 0; i < tags->NumTags(); i++)
	{
		tag = tags->GetTag(i);
		int item = m_pList->InsertItem(i, wxString(tag->name, wxConvUTF8));
		m_pList->SetItem(item, 1, wxString(tag->value, wxConvUTF8));
	}
}

// WDR: handler implementations for TagDlg

