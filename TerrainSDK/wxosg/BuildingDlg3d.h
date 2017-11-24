//
// Name: BuildingDlg3d.h
//
// Copyright (c) 2002-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __BuildingDlg3d_H__
#define __BuildingDlg3d_H__

#include "vtui/BuildingDlg.h"

//----------------------------------------------------------------------------
// BuildingDlg3d
//----------------------------------------------------------------------------

class BuildingDlg3d: public BuildingDlg
{
public:
	// constructors and destructors
	BuildingDlg3d( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for BuildingDlg
	void Setup(vtBuilding3d *bld3d);

	// WDR: member variable declarations for BuildingDlg
	vtBuilding3d	*m_pBuilding3d;

	// allow the dialog to control rendering
	virtual void EnableRendering(bool bEnable);

	// recevied notification when the building is modified
	virtual void Modified();

protected:
	// WDR: handler declarations for BuildingDlg
	virtual void OnOK( wxCommandEvent &event );
};

#endif
