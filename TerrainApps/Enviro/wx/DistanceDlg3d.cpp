//
// Name:		DistanceDlg3d.cpp
//
// Copyright (c) 2006-2009 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "DistanceDlg3d.h"
#include "EnviroGUI.h"	// for g_App

// WDR: class implementations

//----------------------------------------------------------------------------
// DistanceDlg3d
//----------------------------------------------------------------------------

DistanceDlg3d::DistanceDlg3d( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	DistanceDlg( parent, id, title, position, size, style )
{
}

void DistanceDlg3d::OnMode(bool bPath)
{
	g_App.SetDistanceToolMode(bPath);
}

void DistanceDlg3d::Reset()
{
	g_App.ResetDistanceTool();
}

void DistanceDlg3d::SetPathToBase(const DLine2 &path)
{
	g_App.SetDistanceTool(path);
}
