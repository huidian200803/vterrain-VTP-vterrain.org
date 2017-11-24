//
// Name: GeocodeDlg.cpp
//
// Copyright (c) 2005-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "GeocodeDlg.h"
#include "vtdata/config_vtdata.h"
#include "vtdata/FileFilters.h"
#include "vtui/AutoDialog.h"
#include "vtui/Helper.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// GeocodeDlg
//----------------------------------------------------------------------------

// WDR: event table for GeocodeDlg

BEGIN_EVENT_TABLE(GeocodeDlg, GeocodeDlgBase)
	EVT_BUTTON( ID_GET_FILE_DATA, GeocodeDlg::OnGetFileData )
	EVT_BUTTON( ID_GET_FILE_GAZ, GeocodeDlg::OnGetFileGaz )
	EVT_BUTTON( ID_GET_FILE_GNS, GeocodeDlg::OnGetFileGNS )
	EVT_BUTTON( ID_GET_FILE_ZIP, GeocodeDlg::OnGetFileZip )
END_EVENT_TABLE()

GeocodeDlg::GeocodeDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	GeocodeDlgBase( parent, id, title, position, size, style )
{
	AddValidator(this, ID_CHECK_USE1, &m_bGeocodeUS);
	AddValidator(this, ID_CHECK_USE2, &m_bGazetteer);
	AddValidator(this, ID_CHECK_USE3, &m_bGNS);
	AddValidator(this, ID_FILE_DATA, &m_strData);
	AddValidator(this, ID_FILE_GAZ, &m_strGaz);
	AddValidator(this, ID_FILE_ZIP, &m_strZip);
	AddValidator(this, ID_FILE_GNS, &m_strGNS);

	// If no web, then no Geocode.us
	GetCheckUse1()->Enable(SUPPORT_CURL);

	GetSizer()->SetSizeHints(this);
}

// WDR: handler implementations for GeocodeDlg

void GeocodeDlg::OnGetFileData( wxCommandEvent &event )
{
	wxString filter = _T("SHP and DBF files|");
	AddType(filter, FSTRING_SHP);
	AddType(filter, FSTRING_DBF);
	AddType(filter, FSTRING_CSV);

	wxFileDialog dlg(this, _T(""), _T(""), _T(""), filter, wxFD_OPEN);
	if (dlg.ShowModal() == wxID_OK)
	{
		TransferDataFromWindow();
		m_strData = dlg.GetPath();
		TransferDataToWindow();
	}
}

void GeocodeDlg::OnGetFileGaz( wxCommandEvent &event )
{
	wxFileDialog dlg(this, _T(""), _T(""), _T(""),
		_T("Gazetteer Files (*.txt)|*.txt"), wxFD_OPEN);
	if (dlg.ShowModal() == wxID_OK)
	{
		TransferDataFromWindow();
		m_strGaz = dlg.GetPath();
		TransferDataToWindow();
	}
}

void GeocodeDlg::OnGetFileZip( wxCommandEvent &event )
{
	wxFileDialog dlg(this, _T(""), _T(""), _T(""),
		_T("Gazetteer Zipcode Files (*.txt)|*.txt"), wxFD_OPEN);
	if (dlg.ShowModal() == wxID_OK)
	{
		TransferDataFromWindow();
		m_strZip = dlg.GetPath();
		TransferDataToWindow();
	}
}

void GeocodeDlg::OnGetFileGNS( wxCommandEvent &event )
{
	wxFileDialog dlg(this, _T(""), _T(""), _T(""),
		_T("VTP GNS Files (*.gcf)|*.gcf"), wxFD_OPEN);
	if (dlg.ShowModal() == wxID_OK)
	{
		TransferDataFromWindow();
		m_strGNS = dlg.GetPath();
		TransferDataToWindow();
	}
}


