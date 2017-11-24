//
// Name: ImageMapDlg.h
//
// Copyright (c) 2004-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __ImageMapDlg_H__
#define __ImageMapDlg_H__

#include "VTBuilder_UI.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// ImageMapDlg
//----------------------------------------------------------------------------

class ImageMapDlg: public ImageMapDlgBase
{
public:
	// constructors and destructors
	ImageMapDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for ImageMapDlg
	wxChoice* GetLinkField()  { return (wxChoice*) FindWindow( ID_LINK_FIELD ); }
	void SetFields(class vtFeatureSet *pSet);

public:
	// WDR: member variable declarations for ImageMapDlg
	int m_iField;

private:
	// WDR: handler declarations for ImageMapDlg

private:
	DECLARE_EVENT_TABLE()
};

#endif
