//
// Name: ChunkDlg.cpp
//
// Copyright (c) 2005-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "ChunkDlg.h"
#include "vtui/AutoDialog.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ChunkDlg
//----------------------------------------------------------------------------

// WDR: event table for ChunkDlg

BEGIN_EVENT_TABLE(ChunkDlg,ChunkDlgBase)
END_EVENT_TABLE()

ChunkDlg::ChunkDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	ChunkDlgBase( parent, id, title, position, size, style )
{
	AddNumValidator(this, ID_DEPTH, &m_iDepth);
	AddNumValidator(this, ID_MAXERROR, &m_fMaxError);

	GetSizer()->SetSizeHints(this);
}

// WDR: handler implementations for ChunkDlg

