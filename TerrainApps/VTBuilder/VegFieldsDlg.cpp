//
// Name:		VegFieldsDlg.cpp
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "VegFieldsDlg.h"
#include "Builder.h"		// for Plants List
#include "vtui/AutoDialog.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// VegFieldsDlg
//----------------------------------------------------------------------------

// WDR: event table for VegFieldsDlg

BEGIN_EVENT_TABLE(VegFieldsDlg, VegFieldsDlgBase)
	EVT_INIT_DIALOG (VegFieldsDlg::OnInitDialog)
	EVT_CHOICE( ID_SPECIES_FIELD, VegFieldsDlg::OnChoice1 )
	EVT_CHOICE( ID_HEIGHT_FIELD, VegFieldsDlg::OnChoice2 )
	EVT_BUTTON( wxID_OK, VegFieldsDlg::OnOK )
	EVT_RADIOBUTTON( ID_USE_SPECIES, VegFieldsDlg::OnRadio )
	EVT_RADIOBUTTON( ID_SPECIES_USE_FIELD, VegFieldsDlg::OnRadio )
	EVT_RADIOBUTTON( ID_HEIGHT_RANDOM, VegFieldsDlg::OnRadio )
	EVT_RADIOBUTTON( ID_HEIGHT_USE_FIELD, VegFieldsDlg::OnRadio )
END_EVENT_TABLE()

VegFieldsDlg::VegFieldsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	VegFieldsDlgBase( parent, id, title, position, size, style )
{
	m_bUseSpecies = true;
	m_bSpeciesUseField = false;

	AddValidator(this, ID_USE_SPECIES, &m_bUseSpecies);
	AddValidator(this, ID_SPECIES_USE_FIELD, &m_bSpeciesUseField);

	m_bHeightRandomize = true;
	m_bHeightFixed = false;
	m_bHeightUseField = false;
	m_fFixedHeight = 5.0f;

	AddValidator(this, ID_HEIGHT_RANDOM, &m_bHeightRandomize);
	AddValidator(this, ID_HEIGHT_FIXED, &m_bHeightFixed);
	AddValidator(this, ID_HEIGHT_USE_FIELD, &m_bHeightUseField);
	AddNumValidator(this, ID_HEIGHT_FIXED_VALUE, &m_fFixedHeight);

	GetSizer()->SetSizeHints(this);
}

void VegFieldsDlg::SetShapefileName(const wxString &filename)
{
	m_filename = filename;
}

// WDR: handler implementations for VegFieldsDlg

void VegFieldsDlg::OnRadio( wxCommandEvent &event )
{
	// When a radio button is pressed, we should adjust which controls are
	// enabled and disabled.
	TransferDataFromWindow();

	RefreshEnabled();
}

void VegFieldsDlg::RefreshEnabled()
{
	GetSpeciesChoice()->Enable(m_bUseSpecies);

	GetSpeciesField()->Enable(m_bSpeciesUseField);
	GetSpeciesId()->Enable(m_bSpeciesUseField);
	GetSpeciesName()->Enable(m_bSpeciesUseField);
	GetCommonName()->Enable(m_bSpeciesUseField);
	GetBiotypeInt()->Enable(m_bSpeciesUseField);
	GetBiotypeString()->Enable(m_bSpeciesUseField);

	GetHeightField()->Enable(m_bHeightUseField);
}

void VegFieldsDlg::OnInitDialog(wxInitDialogEvent& event)
{
	uint i;

	// SHPOpen doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(m_filename.mb_str(wxConvUTF8));

	// Open the SHP File
	m_hSHP = SHPOpen(fname_local, "rb");
	if (m_hSHP == NULL)
	{
		wxMessageBox(_("Couldn't open shapefile."));
		return;
	}

	// Get number of polys and type of data
	SHPGetInfo(m_hSHP, &m_nElem, &m_nShapeType, NULL, NULL);

	// Open DBF File
	DBFHandle m_db = DBFOpen(fname_local, "rb");
	if (m_db == NULL)
	{
		wxMessageBox(_("Couldn't open DBF file."));
		return;
	}

	// Fill species names into the SpeciesChoice control
	wxString str;
	GetSpeciesChoice()->Clear();
	vtSpeciesList* pl = g_bld->GetSpeciesList();
	for (i = 0; i < pl->NumSpecies(); i++)
	{
		vtPlantSpecies *spe = pl->GetSpecies(i);
		str = wxString(spe->GetSciName(), wxConvUTF8);
		GetSpeciesChoice()->Append(str);
	}

	// Fill the DBF field names into the "Use Field" controls
	int *pnWidth = 0, *pnDecimals = 0;
	DBFFieldType fieldtype;
	char pszFieldName[20];
	m_iFields = DBFGetFieldCount(m_db);
	for (i = 0; i < m_iFields; i++)
	{
		void *clientdata = (void *) (long long int)(10+i);
		fieldtype = DBFGetFieldInfo(m_db, i,
			pszFieldName, pnWidth, pnDecimals );
		str = wxString(pszFieldName, wxConvUTF8);

		if (fieldtype == FTString || fieldtype == FTInteger || fieldtype == FTDouble)
			GetSpeciesField()->Append(str, clientdata);

		if (fieldtype == FTInteger || fieldtype == FTDouble)
			GetHeightField()->Append(str, clientdata);
	}
	wxDialog::OnInitDialog(event);  // calls TransferDataToWindow()
	RefreshEnabled();
}

void VegFieldsDlg::OnChoice1( wxCommandEvent &event )
{
}

void VegFieldsDlg::OnChoice2( wxCommandEvent &event )
{
}

void VegFieldsDlg::OnOK( wxCommandEvent &event )
{
	TransferDataFromWindow();

	// Species
	m_options.bFixedSpecies = m_bUseSpecies;
	m_options.strFixedSpeciesName = GetSpeciesChoice()->GetStringSelection();
	int sel = GetSpeciesField()->GetSelection();
	m_options.iSpeciesFieldIndex = ((long long int) GetSpeciesField()->GetClientData(sel)) - 10;

	if (GetSpeciesId()->GetValue())
		m_options.iInterpretSpeciesField = 0;
	if (GetSpeciesName()->GetValue())
		m_options.iInterpretSpeciesField = 1;
	if (GetCommonName()->GetValue())
		m_options.iInterpretSpeciesField = 2;
	if (GetBiotypeInt()->GetValue())
		m_options.iInterpretSpeciesField = 3;
	if (GetBiotypeString()->GetValue())
		m_options.iInterpretSpeciesField = 4;

	// Height
	m_options.bHeightRandom = m_bHeightRandomize;
	if (m_bHeightFixed)
		m_options.fHeightFixed = m_fFixedHeight;
	else
		m_options.fHeightFixed = -1.0f;
	if (m_bHeightUseField)
	{
		sel = GetHeightField()->GetSelection();
		m_options.iHeightFieldIndex = ((long long int) GetHeightField()->GetClientData(sel)) - 10;
	}
	else
		m_options.iHeightFieldIndex = -1;

	event.Skip();
}

