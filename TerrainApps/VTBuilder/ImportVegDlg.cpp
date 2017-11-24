//
// Name:		ImportVegDlg.cpp
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "ImportVegDlg.h"
#include "vtdata/shapelib/shapefil.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ImportVegDlg
//----------------------------------------------------------------------------

// WDR: event table for ImportVegDlg

BEGIN_EVENT_TABLE(ImportVegDlg,ImportVegDlgBase)
	EVT_BUTTON( wxID_OK, ImportVegDlg::OnOK )
	EVT_INIT_DIALOG (ImportVegDlg::OnInitDialog)
END_EVENT_TABLE()

ImportVegDlg::ImportVegDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	ImportVegDlgBase( parent, id, title, position, size, style )
{
	m_pcbField = GetField();
	m_pDensity = GetDensity();
	m_pBiotype1 = GetBiotype1();
	m_pBiotype2 = GetBiotype2();

	GetSizer()->SetSizeHints(this);
}

void ImportVegDlg::SetShapefileName(const wxString &filename)
{
	m_filename = filename;
}

// WDR: handler implementations for ImportVegDlg

void ImportVegDlg::OnInitDialog(wxInitDialogEvent& event)
{
	// SHPOpen doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(m_filename.mb_str(wxConvUTF8));

	// Open the SHP File
	SHPHandle hSHP = SHPOpen(fname_local, "rb");
	if (hSHP == NULL)
	{
		wxMessageBox(_("Couldn't open shapefile."));
		return;
	}

	// Get number of polys and type of data
	int	 nElem, nShapeType;
	SHPGetInfo(hSHP, &nElem, &nShapeType, NULL, NULL);

	// Check Shape Type, Veg Layer should be Poly data
	if (nShapeType != SHPT_POLYGON && nShapeType != SHPT_POINT)
	{
		wxMessageBox(_("Shapefile must have either point features (for individual\n plants) or polygon features (for plant distribution areas)."));
		return;
	}

	// Open DBF File
	DBFHandle db = DBFOpen(fname_local, "rb");
	if (db == NULL)
	{
		wxMessageBox(_("Couldn't open DBF file."));
		return;
	}
	wxString str;
	int fields, i, iField, *pnWidth = 0, *pnDecimals = 0;
	DBFFieldType fieldtype;
	char pszFieldName[80];
	fields = DBFGetFieldCount(db);
	for (i = 0; i < fields; i++)
	{
		iField = i;
		fieldtype = DBFGetFieldInfo(db, iField,
			pszFieldName, pnWidth, pnDecimals );
		str.Printf(_T("%d: "), i);
		wxString fieldname(pszFieldName, wxConvUTF8);
		str += fieldname;

		if (fieldtype == FTString)
			str += _(" (String)");
		if (fieldtype == FTInteger)
			str += _(" (Integer)");
		if (fieldtype == FTDouble)
			str += _(" (Double)");
		m_pcbField->Append(str);
	}
	m_pcbField->SetSelection(0);

	DBFClose(db);
	SHPClose(hSHP);

	m_pDensity->SetValue(1);

	wxDialog::OnInitDialog(event);  // calls TransferDataToWindow()
}

void ImportVegDlg::OnOK( wxCommandEvent &event )
{
	m_fieldindex = m_pcbField->GetSelection();

	if (m_pDensity->GetValue())
		m_datatype = VIFT_Density;

	if (m_pBiotype1->GetValue())
		m_datatype = VIFT_BiotypeName;

	if (m_pBiotype2->GetValue())
		m_datatype = VIFT_BiotypeID;

	event.Skip();
}

