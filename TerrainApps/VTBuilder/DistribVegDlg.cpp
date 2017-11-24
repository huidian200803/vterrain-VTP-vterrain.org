//
// Name: DistribVegDlg.cpp
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "DistribVegDlg.h"
#include "Frame.h"
#include "VegLayer.h"
#include "vtui/AutoDialog.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// DistribVegDlg
//----------------------------------------------------------------------------

// WDR: event table for DistribVegDlg

BEGIN_EVENT_TABLE(DistribVegDlg, DistribVegDlgBase)
	EVT_INIT_DIALOG (DistribVegDlg::OnInitDialog)
	EVT_BUTTON( wxID_OK, DistribVegDlg::OnOK )
	EVT_RADIOBUTTON( ID_SPECIES1, DistribVegDlg::OnRadio )
	EVT_RADIOBUTTON( ID_SPECIES2, DistribVegDlg::OnRadio )
	EVT_RADIOBUTTON( ID_SPECIES3, DistribVegDlg::OnRadio )
	EVT_RADIOBUTTON( ID_DENSITY1, DistribVegDlg::OnRadio )
	EVT_RADIOBUTTON( ID_DENSITY2, DistribVegDlg::OnRadio )
	EVT_RADIOBUTTON( ID_DENSITY3, DistribVegDlg::OnRadio )
	EVT_RADIOBUTTON( ID_SIZE1, DistribVegDlg::OnRadio )
	EVT_RADIOBUTTON( ID_SIZE2, DistribVegDlg::OnRadio )
END_EVENT_TABLE()

DistribVegDlg::DistribVegDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	DistribVegDlgBase( parent, id, title, position, size, style )
{
	m_iChoiceSpecies = 0;
	m_iChoiceBiotype = 0;
	m_iChoiceBiotypeLayer = 0;
	m_iChoiceDensityLayer = 0;

	m_iRandomFrom = 1;
	m_iRandomTo = 100;

	AddNumValidator(this, ID_SAMPLING, &m_opt.m_fSampling);
	AddNumValidator(this, ID_SCARCITY, &m_opt.m_fScarcity);

	// species
	AddValidator(this, ID_SPECIES1, &m_bSpecies1);
	AddValidator(this, ID_SPECIES2, &m_bSpecies2);
	AddValidator(this, ID_SPECIES3, &m_bSpecies3);
	AddValidator(this, ID_CHOICE_SPECIES, &m_iChoiceSpecies);
	AddValidator(this, ID_CHOICE_BIOTYPE, &m_iChoiceBiotype);
	AddValidator(this, ID_CHOICE_BIOTYPE_LAYER, &m_iChoiceBiotypeLayer);

	// density
	AddValidator(this, ID_DENSITY1, &m_bDensity1);
	AddNumValidator(this, ID_TEXT_FIXED_DENSITY, &m_opt.m_fFixedDensity);
	AddValidator(this, ID_DENSITY2, &m_bDensity2);
	AddValidator(this, ID_DENSITY3, &m_bDensity3);
	AddValidator(this, ID_CHOICE_DENSITY_LAYER, &m_iChoiceDensityLayer);

	// size
	AddValidator(this, ID_SIZE1, &m_bSize1);
	AddValidator(this, ID_SIZE2, &m_bSize2);
	AddNumValidator(this, ID_TEXT_FIXED_SIZE, &m_opt.m_fFixedSize);
	AddValidator(this, ID_SPIN_RANDOM_FROM, &m_iRandomFrom);
	AddValidator(this, ID_SPIN_RANDOM_TO, &m_iRandomTo);

	GetSizer()->SetSizeHints(this);
}


// WDR: handler implementations for DistribVegDlg

void DistribVegDlg::OnInitDialog(wxInitDialogEvent& event)
{
	// Populate the Choice controls with necessary values
	wxString str;
	uint i;

	// Single species
	GetChoiceSpecies()->Clear();
	vtSpeciesList *pl = g_bld->GetSpeciesList();
	if (pl != NULL)
	{
		for (i = 0; i < pl->NumSpecies(); i++)
		{
			vtPlantSpecies *spe = pl->GetSpecies(i);
			str = wxString(spe->GetSciName(), wxConvUTF8);
			GetChoiceSpecies()->Append(str);
		}
	}
	GetChoiceSpecies()->SetSelection(0);

	// Single biotype
	GetChoiceBiotype()->Clear();
	vtBioRegion *br = g_bld->GetBioRegion();
	if (br != NULL)
	{
		for (i = 0; i < br->m_Types.GetSize(); i++)
		{
			vtBioType *bt = br->m_Types[i];
			str.Printf(_T("(%d) "), i);
			wxString str2(bt->m_name, wxConvUTF8);
			str += str2;
			GetChoiceBiotype()->Append(str);
		}
	}
	GetChoiceBiotype()->SetSelection(0);

	// Biotype layers
	GetChoiceBiotypeLayer()->Clear();
	for (uint i = 0; i < g_bld->NumLayers(); i++)
	{
		vtLayer *lp = g_bld->GetLayer(i);
		if (lp->GetType() == LT_VEG)
		{
			vtVegLayer *vl = (vtVegLayer *) lp;
			if (vl->GetVegType() == VLT_BioMap)
				GetChoiceBiotypeLayer()->Append(vl->GetLayerFilename(), vl);
		}
	}
	GetChoiceBiotypeLayer()->SetSelection(0);

	// Density layers
	GetChoiceDensityLayer()->Clear();
	for (uint i = 0; i < g_bld->NumLayers(); i++)
	{
		vtLayer *lp = g_bld->GetLayer(i);
		if (lp->GetType() == LT_VEG)
		{
			vtVegLayer *vl = (vtVegLayer *) lp;
			if (vl->GetVegType() == VLT_Density)
			{
				GetChoiceDensityLayer()->Append(vl->GetLayerFilename(), vl);
			}
		}
	}
	GetChoiceDensityLayer()->SetSelection(0);

	// Count layers of each type
	int iDensityLayers = 0, iBiotypeLayers = 0;
	for (uint i = 0; i < g_bld->NumLayers(); i++)
	{
		vtLayer *lp = g_bld->GetLayer(i);
		if (lp->GetType() == LT_VEG)
		{
			vtVegLayer *vl = (vtVegLayer *) lp;
			if (vl->GetVegType() == VLT_Density)
				iDensityLayers ++;
			else
			if (vl->GetVegType() == VLT_BioMap)
				iBiotypeLayers ++;
		}
	}
	bool bHaveDensityLayer = (iDensityLayers != 0);
	bool bHaveBiotypeLayer = (iBiotypeLayers != 0);
	bool bHaveBiotypes = (g_bld->GetBioRegion()->NumTypes() != 0);

	// Species enabling
	GetSpecies2()->Enable(bHaveBiotypes);
	GetSpecies3()->Enable(bHaveBiotypeLayer);

	// Species default values
	m_bSpecies1 = false;
	m_bSpecies2 = false;
	m_bSpecies3 = false;

	if (bHaveBiotypeLayer)
		m_bSpecies3 = true;
	else if (bHaveBiotypes)
		m_bSpecies2 = true;
	else
		m_bSpecies1 = true;

	// Density enabling
	GetDensity3()->Enable(bHaveDensityLayer);

	// Density default values
	m_bDensity1 = false;
	m_bDensity2 = false;
	m_bDensity3 = false;

	if (bHaveDensityLayer)
		m_bDensity3 = true;
	else if (bHaveBiotypes)
		m_bDensity2 = true;
	else
		m_bDensity1 = true;

	// Size
	m_bSize1 = false;
	m_bSize2 = true;

	UpdateEnabling();

	wxDialog::OnInitDialog(event);  // transfers data to window
}

void DistribVegDlg::UpdateEnabling()
{
	GetChoiceSpecies()->Enable(m_bSpecies1);
	GetChoiceBiotype()->Enable(m_bSpecies2);
	GetChoiceBiotypeLayer()->Enable(m_bSpecies3);

	// they can inherit density from biotype only if they are using a biotype
	GetTextFixedDensity()->Enable(m_bDensity1);
	GetDensity2()->Enable(m_bSpecies2 || m_bSpecies3);
	GetChoiceDensityLayer()->Enable(m_bDensity3);

	GetTextFixedSize()->Enable(m_bSize1);
	GetSpinRandomFrom()->Enable(m_bSize2);
	GetSpinRandomTo()->Enable(m_bSize2);
}

void DistribVegDlg::OnRadio( wxCommandEvent &event )
{
	TransferDataFromWindow();
	UpdateEnabling();
}

void DistribVegDlg::OnOK( wxCommandEvent &event )
{
	// pass through
	event.Skip();
}

bool DistribVegDlg::TransferDataFromWindow()
{
	bool value = wxDialog::TransferDataFromWindow();

	// species
	if (m_bSpecies1)
		m_opt.m_iSingleSpecies = m_iChoiceSpecies;
	else if (m_bSpecies2)
		m_opt.m_iSingleBiotype = m_iChoiceBiotype;
	else if (m_bSpecies3)
	{
		void *ptr = GetChoiceBiotypeLayer()->GetClientData(m_iChoiceBiotypeLayer);
		m_opt.m_pBiotypeLayer = (vtVegLayer *) ptr;
	}

	// density
	if (m_bDensity1)
	{
		// allow m_opt.m_fFixedDensity to keep its value
	}
	else if (m_bDensity2)
	{
		m_opt.m_fFixedDensity = -1.0f;
	}
	if (m_bDensity3)
	{
		void *ptr = GetChoiceDensityLayer()->GetClientData(m_iChoiceDensityLayer);
		m_opt.m_pDensityLayer = (vtVegLayer *) ptr;
	}

	// size
	if (m_bSize1)
	{
		// allow m_opt.m_fFixedSize to keep its value
	}
	else
	{
		m_opt.m_fFixedSize = -1.0f;
		m_opt.m_fRandomFrom = m_iRandomFrom / 100.0f;
		m_opt.m_fRandomTo = m_iRandomTo / 100.0f;
	}

	return value;
}

