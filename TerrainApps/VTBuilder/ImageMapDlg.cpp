//
// Name: ImageMapDlg.cpp
//
// Copyright (c) 2004-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "ImageMapDlg.h"
#include "vtdata/Features.h"
#include "vtui/AutoDialog.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ImageMapDlg
//----------------------------------------------------------------------------

// WDR: event table for ImageMapDlg

BEGIN_EVENT_TABLE(ImageMapDlg,ImageMapDlgBase)
END_EVENT_TABLE()

ImageMapDlg::ImageMapDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	ImageMapDlgBase( parent, id, title, position, size, style )
{
	m_iField = 0;

	AddValidator(this, ID_LINK_FIELD, &m_iField);

	GetSizer()->SetSizeHints(this);
}

void ImageMapDlg::SetFields(vtFeatureSet *pSet)
{
	for (uint i = 0; i < pSet->NumFields(); i++)
	{
		wxString str(pSet->GetField(i)->m_name, wxConvUTF8);
		GetLinkField()->Append(str);
	}
}

// WDR: handler implementations for ImageMapDlg

