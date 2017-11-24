//
// Name:		LinearStructureDlg3d.h
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __LinearStructureDlg3d_H__
#define __LinearStructureDlg3d_H__

#include "vtui/LinearStructDlg.h"

//----------------------------------------------------------------------------
// LinearStructureDlg3d
//----------------------------------------------------------------------------

class LinearStructureDlg3d: public LinearStructureDlg
{
public:
	// constructors and destructors
	LinearStructureDlg3d( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	virtual void OnSetOptions(const vtLinearParams &param, bool bProfileChanged = false);
};

#endif	// __LinearStructureDlg3d_H__

