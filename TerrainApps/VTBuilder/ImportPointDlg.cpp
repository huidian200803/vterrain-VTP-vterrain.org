//
// Name: ImportPointDlg.cpp
//
// Copyright (c) 2004-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "ImportPointDlg.h"
#include "vtui/AutoDialog.h"
#include "vtui/ProjectionDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ImportPointDlg
//----------------------------------------------------------------------------

// WDR: event table for ImportPointDlg

BEGIN_EVENT_TABLE(ImportPointDlg,ImportPointDlgBase)
	EVT_BUTTON( ID_SET_CRS, ImportPointDlg::OnSetCRS )
	EVT_CHECKBOX( ID_CHECK_ELEVATION, ImportPointDlg::OnCheck )
END_EVENT_TABLE()

ImportPointDlg::ImportPointDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	ImportPointDlgBase( parent, id, title, position, size, style )
{
	m_iEasting = 0;
	m_iNorthing = 0;
	m_bElevation = false;
	m_iElevation = 0;
	m_bImportField = false;
	m_iImportField = 0;
	m_crs.SetSimple(false, 0, EPSG_DATUM_WGS84);
	m_bFormat1 = true;
	m_bFormat2 = false;
	m_bFormat3 = false;
	m_bLongitudeWest = false;

	AddValidator(this, ID_EASTING, &m_iEasting);
	AddValidator(this, ID_NORTHING, &m_iNorthing);

	AddValidator(this, ID_CHECK_ELEVATION, &m_bElevation);
	AddValidator(this, ID_ELEVATION, &m_iElevation);

	AddValidator(this, ID_CHECK_IMPORT_FIELD, &m_bImportField);
	AddValidator(this, ID_IMPORT_FIELD, &m_iImportField);

	AddValidator(this, ID_CRS, &m_strCRS);
	AddValidator(this, ID_FORMAT_DECIMAL, &m_bFormat1);
	AddValidator(this, ID_FORMAT_DMS, &m_bFormat2);
	AddValidator(this, ID_FORMAT_HDM, &m_bFormat3);
	AddValidator(this, ID_LONGITUDE_WEST, &m_bLongitudeWest);

	RefreshProjString();
	UpdateEnabling();

	GetSizer()->SetSizeHints(this);
}

void ImportPointDlg::SetCRS(const vtCRS &crs)
{
	m_crs = crs;
	RefreshProjString();
}

void ImportPointDlg::RefreshProjString()
{
	char *str1;
	m_crs.exportToProj4(&str1);
	m_strCRS = wxString(str1, wxConvUTF8);
	OGRFree(str1);

	TransferDataToWindow();
}

void ImportPointDlg::UpdateEnabling()
{
	FindWindow(ID_ELEVATION)->Enable(m_bElevation);
}

// WDR: handler implementations for ImportPointDlg

void ImportPointDlg::OnCheck(wxCommandEvent &event)
{
	TransferDataFromWindow();
	UpdateEnabling();
}

void ImportPointDlg::OnSetCRS(wxCommandEvent &event)
{
	ProjectionDlg dlg(this, -1, _("Please indicate CRS"));
	dlg.SetCRS(m_crs);

	if (dlg.ShowModal() == wxID_OK)
	{
		dlg.GetCRS(m_crs);
		RefreshProjString();
	}
}

