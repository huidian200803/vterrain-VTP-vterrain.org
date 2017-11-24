//
// Name: TagDlg.h
//
// Copyright (c) 2007-2011 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#ifndef __TagDlg_H__
#define __TagDlg_H__

#include "vtui_UI.h"
#include "vtdata/Content.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// TagDlg
//----------------------------------------------------------------------------

class TagDlg: public TagDlgBase
{
public:
	// constructors and destructors
	TagDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for TagDlg
	wxListCtrl* GetList()  { return (wxListCtrl*) FindWindow( ID_TAGLIST ); }

	void SetTags(vtTagArray *tags);

private:
	// WDR: member variable declarations for TagDlg
	wxListCtrl *m_pList;

private:
	// WDR: handler declarations for TagDlg

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __TagDlg_H__

