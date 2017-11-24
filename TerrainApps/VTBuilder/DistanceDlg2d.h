//
// Name: DistanceDlg2d.h
//
// Copyright (c) 2006-2009 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __DistanceDlg2d_H__
#define __DistanceDlg2d_H__

#include "vtui/DistanceDlg.h"

class MainFrame;

// WDR: class declarations

//----------------------------------------------------------------------------
// DistanceDlg
//----------------------------------------------------------------------------

class DistanceDlg2d: public DistanceDlg
{
public:
	// constructors and destructors
	DistanceDlg2d( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void SetFrame(MainFrame *frame) { m_pFrame = frame; }

	virtual void OnMode(bool bPath);
	virtual void SetPathToBase(const DLine2 &path);
	virtual void Reset();

	MainFrame *m_pFrame;
};

#endif	// __DistanceDlg_H__

