//
// Name: ChunkDlg.h
//
// Copyright (c) 2005-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __ChunkDlg_H__
#define __ChunkDlg_H__

#include "VTBuilder_UI.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// ChunkDlg
//----------------------------------------------------------------------------

class ChunkDlg: public ChunkDlgBase
{
public:
	// constructors and destructors
	ChunkDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for ChunkDlg

public:
	// WDR: member variable declarations for ChunkDlg
	int m_iDepth;
	float m_fMaxError;

private:
	// WDR: handler declarations for ChunkDlg

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __ChunkDlg_H__
