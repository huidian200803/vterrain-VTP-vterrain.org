//
// Name: ImportStructDlg.cpp
//
// Copyright (c) 2003-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/colordlg.h>
#include "vtui/AutoDialog.h"
#include "vtui/Helper.h"

#include "ImportStructDlg.h"
#include "StructLayer.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ImportStructDlg
//----------------------------------------------------------------------------

// WDR: event table for ImportStructDlg

BEGIN_EVENT_TABLE(ImportStructDlg, ImportStructDlgBase)
	EVT_INIT_DIALOG (ImportStructDlg::OnInitDialog)
	EVT_RADIOBUTTON( ID_TYPE_LINEAR, ImportStructDlg::OnRadio )
	EVT_RADIOBUTTON( ID_TYPE_INSTANCE, ImportStructDlg::OnRadio )
	EVT_RADIOBUTTON( ID_TYPE_CENTER, ImportStructDlg::OnRadio )
	EVT_RADIOBUTTON( ID_TYPE_FOOTPRINT, ImportStructDlg::OnRadio )

	EVT_RADIOBUTTON( ID_RADIO_COLOR_DEFAULT, ImportStructDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO_COLOR_FIXED, ImportStructDlg::OnRadio )

	EVT_RADIOBUTTON( ID_RADIO_ROOF_DEFAULT, ImportStructDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO_ROOF_SINGLE, ImportStructDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO_ROOF_FIELD, ImportStructDlg::OnRadio )

	EVT_CHOICE( ID_CHOICE_FILE_FIELD, ImportStructDlg::OnChoiceFileField )
	EVT_CHOICE( ID_CHOICE_HEIGHT_FIELD, ImportStructDlg::OnChoiceHeightField )
	EVT_CHOICE( ID_CHOICE_HEIGHT_TYPE, ImportStructDlg::OnChoiceHeightType )
	EVT_CHOICE( ID_CHOICE_ROOF_TYPE, ImportStructDlg::OnChoiceRoofType )
	EVT_CHOICE( ID_CHOICE_ROOF_FIELD, ImportStructDlg::OnChoiceRoofField )

	EVT_BUTTON( ID_SET_COLOR1, ImportStructDlg::OnColorBuilding )
	EVT_BUTTON( ID_SET_COLOR2, ImportStructDlg::OnColorRoof )

	EVT_SPINCTRL( ID_SLOPECNTR, ImportStructDlg::OnSpinRoofDegrees )
	EVT_TEXT( ID_SLOPECNTR, ImportStructDlg::OnTextRoofDegrees )
END_EVENT_TABLE()

ImportStructDlg::ImportStructDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	ImportStructDlgBase( parent, id, title, position, size, style )
{
	m_iType = 0;
	m_iHeightType = 0;
	m_iRoofType = 0;
	m_opt.bInsideOnly = false;
	m_opt.m_bFixedColor = false;

	m_defaults_filename = wxString(g_DefaultStructures.GetFilename(), wxConvUTF8);

	AddValidator(this, ID_INSIDE_AREA, &m_opt.bInsideOnly);
	AddValidator(this, ID_CHOICE_HEIGHT_TYPE, &m_iHeightType);
	AddValidator(this, ID_CHOICE_ROOF_TYPE, &m_iRoofType);
	AddValidator(this, ID_RADIO_COLOR_FIXED, &m_opt.m_bFixedColor);
	AddValidator(this, ID_DEFAULTS_FILE, &m_defaults_filename);

	GetSizer()->SetSizeHints(this);
}

bool ImportStructDlg::GetRadio(int id)
{
	wxRadioButton *button = (wxRadioButton*) FindWindow(id);
	if (!button)
		return false;
	return button->GetValue();
}


// WDR: handler implementations for ImportStructDlg

void ImportStructDlg::OnChoiceFileField( wxCommandEvent &event )
{
	TransferDataFromWindow();
	wxString str = GetChoiceFileField()->GetStringSelection();
	m_opt.m_strFieldNameFile = str.mb_str(wxConvUTF8);
}

void ImportStructDlg::OnChoiceHeightField( wxCommandEvent &event )
{
	TransferDataFromWindow();
	wxString str = GetChoiceHeightField()->GetStringSelection();
	m_opt.m_strFieldNameHeight = str.mb_str(wxConvUTF8);
}

void ImportStructDlg::OnChoiceHeightType( wxCommandEvent &event )
{
	TransferDataFromWindow();
	m_opt.m_HeightType = (StructImportOptions::HeightType) m_iHeightType;
}

void ImportStructDlg::OnChoiceRoofType( wxCommandEvent &event )
{
	TransferDataFromWindow();
	CopyToOptions();
	UpdateEnables();
}

void ImportStructDlg::OnChoiceRoofField( wxCommandEvent &event )
{
	TransferDataFromWindow();
	wxString str = GetChoiceRoofField()->GetStringSelection();
	m_opt.m_strFieldNameRoof = str.mb_str(wxConvUTF8);
}

void ImportStructDlg::OnRadio( wxCommandEvent &event )
{
	TransferDataFromWindow();

	if (GetRadio(ID_TYPE_LINEAR)) m_iType = 0;
	if (GetRadio(ID_TYPE_INSTANCE)) m_iType = 1;
	if (GetRadio(ID_TYPE_CENTER)) m_iType = 2;
	if (GetRadio(ID_TYPE_FOOTPRINT)) m_iType = 3;

	if (GetRadio(ID_RADIO_ROOF_DEFAULT)) m_iRoofMode = 0;
	if (GetRadio(ID_RADIO_ROOF_SINGLE)) m_iRoofMode = 1;
	if (GetRadio(ID_RADIO_ROOF_FIELD)) m_iRoofMode = 2;

	CopyToOptions();
	UpdateEnables();
}

void ImportStructDlg::OnInitDialog(wxInitDialogEvent& event)
{
	m_nShapeType = GetSHPType(m_filename.mb_str(wxConvUTF8));
	UpdateEnables();

	// Select one of the radio buttons, whichever is enabled
	if (GetTypeCenter()->IsEnabled())
	{
		GetTypeCenter()->SetValue(true);
		m_iType = 2;
	}
	else
	if (GetTypeFootprint()->IsEnabled())
	{
		GetTypeFootprint()->SetValue(true);
		m_iType = 3;
	}
	else
	if (GetTypeLinear()->IsEnabled())
	{
		GetTypeLinear()->SetValue(true);
		m_iType = 0;
	}

	m_opt.m_RoofColor = RGBi( 188, 135, 121 );
	m_opt.m_BuildingColor = RGBi( 255, 224, 178 );

	m_pColorBitmapRoof = GetColorBitmapRoof();
	m_pColorBitmapBuilding = GetColorBitmapBuilding();

	FillWithColorSize(m_pColorBitmapRoof, 32, 18, m_opt.m_RoofColor);
	FillWithColorSize(m_pColorBitmapBuilding, 32, 18, m_opt.m_BuildingColor);

	UpdateEnables();

	GetChoiceHeightField()->Append(_("(none)"));
	GetChoiceFileField()->Append(_("(none)"));
	GetChoiceRoofField()->Append(_("(none)"));

	// DBFOpen doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(m_filename.mb_str(wxConvUTF8));

	// Open DBF File
	DBFHandle db = DBFOpen(fname_local, "rb");
	if (db != NULL)
	{
		// Fill the DBF field names into the "Use Field" controls
		int *pnWidth = 0, *pnDecimals = 0;
		char pszFieldName[20];
		int iFields = DBFGetFieldCount(db);
		int i;
		for (i = 0; i < iFields; i++)
		{
			DBFFieldType fieldtype = DBFGetFieldInfo(db, i,
				pszFieldName, pnWidth, pnDecimals );
			wxString str(pszFieldName, wxConvUTF8);

			if (fieldtype == FTString)
				GetChoiceFileField()->Append(str);
			if (fieldtype == FTInteger || fieldtype == FTDouble)
				GetChoiceHeightField()->Append(str);
			if (fieldtype == FTString)
				GetChoiceRoofField()->Append(str);
		}
	}
	GetChoiceFileField()->SetSelection(0);
	GetChoiceHeightField()->SetSelection(0);
	GetChoiceRoofField()->SetSelection(0);

	TransferDataToWindow();

	CopyToOptions();
}

void ImportStructDlg::CopyToOptions()
{
	if (m_iType == 3)		// is a footprint
	{
		if (m_iRoofMode == 1)	// type: single
		{
			m_opt.m_eRoofType = (RoofType) m_iRoofType;
			m_opt.m_strFieldNameRoof = "";
		}
		else if (m_iRoofMode == 0)	// type: default
		{
			m_opt.m_eRoofType = ROOF_UNKNOWN;
			m_opt.m_strFieldNameRoof = "";
		}
		else if (m_iRoofMode == 2)	// type: from field
		{
			m_opt.m_eRoofType = ROOF_UNKNOWN;
			wxString str = GetChoiceRoofField()->GetStringSelection();
			m_opt.m_strFieldNameRoof = str.mb_str(wxConvUTF8);
		}
		m_opt.m_iSlope = GetEdgeDeg()->GetValue();
	}
	else	// not a footprint
	{
		m_opt.m_eRoofType = ROOF_UNKNOWN;
		m_opt.m_strFieldNameRoof = "";
	}

	if (m_iType == 0) m_opt.type = ST_LINEAR;
	if (m_iType == 1) m_opt.type = ST_INSTANCE;
	if (m_iType == 2) m_opt.type = ST_BUILDING;
	if (m_iType == 3) m_opt.type = ST_BUILDING;
}

void ImportStructDlg::UpdateEnables()
{
	GetTypeCenter()->Enable(m_nShapeType == SHPT_POINT);

	GetTypeFootprint()->Enable(m_nShapeType == SHPT_POLYGON ||
		m_nShapeType == SHPT_POLYGONZ || m_nShapeType == SHPT_ARC);

	GetTypeLinear()->Enable(m_nShapeType == SHPT_ARC ||
		m_nShapeType == SHPT_POLYGON);

	GetTypeInstance()->Enable(m_nShapeType == SHPT_POINT ||
		m_nShapeType == SHPT_POINTZ);

	GetChoiceHeightField()->Enable(m_iType == 3);
	GetChoiceHeightType()->Enable(m_iType == 3);

	GetChoiceFileField()->Enable(m_iType == 1);

	GetRadioRoofDefault()->Enable(m_iType == 3);
	GetRadioRoofSingle()->Enable(m_iType == 3);
	GetRadioRoofField()->Enable(m_iType == 3);

	GetChoiceRoofType()->Enable(m_iType == 3 && m_iRoofMode == 1);
	GetChoiceRoofField()->Enable(m_iType == 3 && m_iRoofMode == 2);

	GetRadioColorDefault()->Enable(m_opt.type == ST_BUILDING);
	GetRadioColorFixed()->Enable(m_opt.type == ST_BUILDING);

	GetColorRoof()->Enable(m_opt.type == ST_BUILDING && m_opt.m_bFixedColor);
	GetColorBuilding()->Enable(m_opt.type == ST_BUILDING && m_opt.m_bFixedColor);
	GetEdgeDeg()->Enable(m_opt.type == ST_BUILDING && m_iRoofMode == 1 &&
		m_iRoofType != 0);	// any sloped (non-flat) roof type
}

void ImportStructDlg::OnTextRoofDegrees( wxCommandEvent &event )
{
	TransferDataFromWindow();
	CopyToOptions();
}

void ImportStructDlg::OnSpinRoofDegrees( wxSpinEvent &event )
{
	TransferDataFromWindow();
	CopyToOptions();
}

void ImportStructDlg::UpdateColorControl(bool select)
{
	if( select )
		FillWithColorSize(m_pColorBitmapBuilding, 32, 18, m_opt.m_BuildingColor); //Building Bitmap
	else
		FillWithColorSize(m_pColorBitmapRoof, 32, 18, m_opt.m_RoofColor); //Roof Bitmap
}

void ImportStructDlg::OnColorBuilding( wxCommandEvent &event )
{
	wxColourData data;
	data.SetChooseFull(true);
	data.SetColour(m_ColorB);

	wxColourDialog dlg(this, &data);
	if (dlg.ShowModal() == wxID_OK)
	{
		wxColourData data2 = dlg.GetColourData();
		m_ColorB = data2.GetColour();

		RGBi result(m_ColorB.Red(), m_ColorB.Green(), m_ColorB.Blue());
		m_opt.m_BuildingColor = result;

		UpdateColorControl(true);
	}
}

void ImportStructDlg::OnColorRoof( wxCommandEvent &event )
{
	wxColourData data;
	data.SetChooseFull(true);
	data.SetColour(m_ColorR);

	wxColourDialog dlg(this, &data);
	if (dlg.ShowModal() == wxID_OK)
	{
		wxColourData data2 = dlg.GetColourData();
		m_ColorR = data2.GetColour();

		RGBi result(m_ColorR.Red(), m_ColorR.Green(), m_ColorR.Blue());
		m_opt.m_RoofColor = result;

		UpdateColorControl(false);
	}
}

