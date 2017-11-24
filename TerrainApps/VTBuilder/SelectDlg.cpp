//
// Name:		SelectDlg.cpp
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "SelectDlg.h"
#include "RawLayer.h"
#include "vtui/AutoDialog.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// SelectDlg
//----------------------------------------------------------------------------

// WDR: event table for SelectDlg

BEGIN_EVENT_TABLE(SelectDlg, SelectDlgBase)
	EVT_INIT_DIALOG (SelectDlg::OnInitDialog)
	EVT_BUTTON( wxID_OK, SelectDlg::OnOK )
	EVT_LISTBOX( ID_FIELD, SelectDlg::OnChoiceField )
END_EVENT_TABLE()

SelectDlg::SelectDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	SelectDlgBase( parent, id, title, position, size, style )
{
	m_pLayer = NULL;
	m_iField = 0;
	m_iCondition = 0;
	m_strValue = _T("");

	AddValidator(this, ID_CONDITION, &m_iCondition);

	// The order of these must not change
	GetCondition()->Append(_T(" = "));
	GetCondition()->Append(_T(" > "));
	GetCondition()->Append(_T(" < "));
	GetCondition()->Append(_T(" >= "));
	GetCondition()->Append(_T(" <= "));
	GetCondition()->Append(_T(" <> "));
	GetCondition()->SetSelection(0);

	GetSizer()->SetSizeHints(this);
}

void SelectDlg::SetRawLayer(vtRawLayer *pRL)
{
	m_pLayer = pRL;
}

void SelectDlg::OnInitDialog(wxInitDialogEvent& event)
{
	vtCRS crs;
	m_pLayer->GetCRS(crs);

	m_iFauxFields = 0;
	vtFeatureSet *pSet = m_pLayer->GetFeatureSet();

	OGRwkbGeometryType type = pSet->GetGeomType();
	if (type == wkbPoint || type == wkbPoint25D)
	{
		if (crs.IsGeographic())
		{
			GetField()->Append(_("X (longitude)"), (void *) 900);
			GetField()->Append(_("Y (latitude)"), (void *) 901);
		}
		else
		{
			GetField()->Append(_("X (easting)"), (void *) 900);
			GetField()->Append(_("Y (northing)"), (void *) 901);
		}
		m_iFauxFields = 2;
	}
	if (type == wkbPoint25D)
	{
		GetField()->Append(_("Z (meters)"), (void *) 902);
		m_iFauxFields = 3;
	}

	for (uint i = 0; i < pSet->NumFields(); i++)
	{
		Field *field = pSet->GetField(i);
		wxString str(field->m_name, wxConvUTF8);
		GetField()->Append(str, (void *) 0);
	}
	GetField()->SetSelection(0);

	FillValuesControl();

	m_bSetting = true;
	wxDialog::OnInitDialog(event);	// calls TransferValuesToWindow
	m_bSetting = false;
}

void SelectDlg::FillValuesControl()
{
	int values = 0;
	vtString str;
	wxString ws;

	GetComboValue()->Clear();

	if (m_iField < 0)
	{
		GetComboValue()->Append(_T("0.0"));
		GetComboValue()->SetSelection(0);
		return;
	}

	vtFeatureSet *pSet = m_pLayer->GetFeatureSet();
	for (uint i = 0; i < pSet->NumEntities(); i++)
	{
		pSet->GetValueAsString(i, m_iField, str);
		if (str == "")
			continue;

		ws = wxString(str, wxConvUTF8);
		if (GetComboValue()->FindString(ws) == -1)
		{
			GetComboValue()->Append(ws);
			values++;
		}
		if (values == 100)
			break;
	}
	GetComboValue()->SetSelection(0);
}


// WDR: handler implementations for SelectDlg

void SelectDlg::OnChoiceField( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	m_iField = GetField()->GetSelection();

	// work around the 2-3 "special" fields
	void *data = GetField()->GetClientData(m_iField);
	if (data == (void*) 900)
		m_iField = -1;
	else if (data == (void*) 901)
		m_iField = -2;
	else if (data == (void*) 902)
		m_iField = -3;
	else
		m_iField -= m_iFauxFields;

	TransferDataFromWindow();
	FillValuesControl();
}

void SelectDlg::OnOK( wxCommandEvent &event )
{
	// Using a validator for Combo Box doesn't work perfectly in last version
	// of wxWidgets.  Get the value directly instead.
	m_strValue = GetComboValue()->GetValue();

	event.Skip();
}


