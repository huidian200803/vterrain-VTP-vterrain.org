//
// Name: ImportStructOGRDlg.cpp
//
// Copyright (c) 2003-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "ImportStructDlgOGR.h"
#include "vtui/AutoDialog.h"
#include "StructLayer.h"

#include "ogrsf_frmts.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ImportStructOGRDlg
//----------------------------------------------------------------------------

// WDR: event table for ImportStructOGRDlg

BEGIN_EVENT_TABLE(ImportStructOGRDlg, ImportStructOGRDlgBase)
	EVT_INIT_DIALOG (ImportStructOGRDlg::OnInitDialog)
	EVT_RADIOBUTTON( ID_TYPE_BUILDING, ImportStructOGRDlg::OnRadio )
	EVT_RADIOBUTTON( ID_TYPE_LINEAR, ImportStructOGRDlg::OnRadio )
	EVT_RADIOBUTTON( ID_TYPE_INSTANCE, ImportStructOGRDlg::OnRadio )
	EVT_CHOICE( ID_CHOICE_HEIGHT_FIELD, ImportStructOGRDlg::OnChoiceHeightField )
	EVT_CHOICE( ID_CHOICE_FILE_FIELD, ImportStructOGRDlg::OnChoiceFileField )
	EVT_CHOICE( ID_LAYERNAME, ImportStructOGRDlg::OnChoiceLayerName )
END_EVENT_TABLE()

ImportStructOGRDlg::ImportStructOGRDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	ImportStructOGRDlgBase( parent, id, title, position, size, style )
{
	m_opt.m_HeightType = StructImportOptions::METERS;
	m_opt.bInsideOnly = false;
	m_opt.bBuildFoundations = false;;
	m_opt.bUse25DForElevation = false;

	AddValidator(this, ID_INSIDE_AREA, &m_opt.bInsideOnly);
	AddValidator(this, ID_BUILD_FOUNDATIONS, &m_opt.bBuildFoundations);
	AddValidator(this, ID_USE_25D, &m_opt.bUse25DForElevation);
	AddValidator(this, ID_CHOICE_HEIGHT_TYPE, (int *)&m_opt.m_HeightType);

	GetSizer()->SetSizeHints(this);
}

bool ImportStructOGRDlg::GetRadio(int id)
{
	wxRadioButton *button = (wxRadioButton*) FindWindow(id);
	if (!button)
		return false;
	return button->GetValue();
}


// WDR: handler implementations for ImportStructOGRDlg

void ImportStructOGRDlg::OnChoiceLayerName( wxCommandEvent &event )
{
	wxString str = GetLayername()->GetStringSelection();
	m_opt.m_strLayerName = str.mb_str(wxConvUTF8);
	UpdateFieldNames();
	UpdateEnables();
}

void ImportStructOGRDlg::OnChoiceFileField( wxCommandEvent &event )
{
	wxString str = GetChoiceFileField()->GetStringSelection();
	m_opt.m_strFieldNameFile = str.mb_str(wxConvUTF8);
}

void ImportStructOGRDlg::OnChoiceHeightField( wxCommandEvent &event )
{
	wxString str = GetChoiceHeightField()->GetStringSelection();
	m_opt.m_strFieldNameHeight = str.mb_str(wxConvUTF8);
}

void ImportStructOGRDlg::OnRadio( wxCommandEvent &event )
{
	if (GetRadio(ID_TYPE_BUILDING))
		m_iType = 1;
	if (GetRadio(ID_TYPE_LINEAR))
		m_iType = 2;
	if (GetRadio(ID_TYPE_INSTANCE))
		m_iType = 3;
	UpdateEnables();
}

void ImportStructOGRDlg::OnInitDialog(wxInitDialogEvent& event)
{
	// Select one of the radio buttons, whichever is enabled
	m_iType = 1;
	GetTypeBuilding()->SetValue(true);

	UpdateEnables();

	int iNumLayers = m_pDatasource->GetLayerCount();
	for (int i = 0 ; i < iNumLayers; i++)
	{
		wxString str(m_pDatasource->GetLayer(i)->GetLayerDefn()->GetName(), wxConvUTF8);
		GetLayername()->Append(str);
	}
	GetLayername()->Enable(iNumLayers > 1);
	GetLayername()->SetSelection(0);

	// Pete Willemsen - I'm not sure, but the gcc 3.2.X compilers
	// don't like the form of this that creates a temporary???
	// Replaced: OnChoiceFileField(wxCommandEvent());
	wxCommandEvent wce = wxCommandEvent();
	OnChoiceLayerName( wce );

	UpdateFieldNames();
	UpdateEnables();

	TransferDataToWindow();
}

void ImportStructOGRDlg::UpdateFieldNames()
{
	OGRLayer *pLayer;
	OGRFeatureDefn *pFeatureDefn;
	OGRFieldDefn *pFieldDefn;
	OGRFieldType FieldType;
	int iNumFields;
	int i;
	int iCount;
	wxString str;

	pLayer = m_pDatasource->GetLayer(GetLayername()->GetSelection());

	if (NULL != pLayer)
	{
		pFeatureDefn = pLayer->GetLayerDefn();

		if (NULL != pFeatureDefn)
		{
			iNumFields = pFeatureDefn->GetFieldCount();

			GetUse25d()->Enable((pFeatureDefn->GetGeomType() & wkb25DBit) > 0);

			iCount = GetChoiceHeightField()->GetCount();
			for (i = 0; i < iCount; i++)
				GetChoiceHeightField()->Delete(0);
			iCount = GetChoiceFileField()->GetCount();
			for (i = 0; i < iCount; i++)
				GetChoiceFileField()->Delete(0);

			GetChoiceHeightField()->Append(_("(none)"));
			GetChoiceFileField()->Append(_("(none)"));

			for (i = 0; i < iNumFields; i++)
			{
				pFieldDefn = pFeatureDefn->GetFieldDefn(i);
				if (NULL != pFieldDefn)
				{
					FieldType = pFieldDefn->GetType();
					if ((FieldType == OFTInteger) || (FieldType == OFTReal))
					{
						str = wxString(pFieldDefn->GetNameRef(), wxConvUTF8);
						GetChoiceHeightField()->Append(str);
					}
					else if ((FieldType == OFTString) || (FieldType == OFTWideString))
					{
						str = wxString(pFieldDefn->GetNameRef(), wxConvUTF8);
						GetChoiceFileField()->Append(str);
					}
				}
			}
		}
	}

	// TODO: get rid of this yucky hack of passing bogus command events!
	wxCommandEvent wce = wxCommandEvent();

	GetChoiceFileField()->SetSelection(0);
	OnChoiceFileField(wce);
	GetChoiceHeightField()->SetSelection(0);
	OnChoiceHeightField(wce);
}

void ImportStructOGRDlg::UpdateEnables()
{
	GetBuildFoundations()->Enable(m_iType == 1);
	GetChoiceHeightField()->Enable(GetChoiceHeightField()->GetCount() > 1);
	GetChoiceFileField()->Enable((GetChoiceFileField()->GetCount() > 1) && (m_iType == 3));
}

