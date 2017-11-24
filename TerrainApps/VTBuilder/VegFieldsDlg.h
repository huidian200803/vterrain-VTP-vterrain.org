//
// Name:		VegFieldsDlg.h
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __VegFieldsDlg_H__
#define __VegFieldsDlg_H__

#include "VTBuilder_UI.h"
#include "vtdata/shapelib/shapefil.h"
#include "vtdata/MathTypes.h"
#include "VegPointOptions.h"

class vtVegLayer;

// WDR: class declarations

//----------------------------------------------------------------------------
// VegFieldsDlg
//----------------------------------------------------------------------------

class VegFieldsDlg: public VegFieldsDlgBase
{
public:
	// constructors and destructors
	VegFieldsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void SetShapefileName(const wxString &filename);
	void SetVegLayer(vtVegLayer *pLayer) { m_pLayer = pLayer; }

	// WDR: method declarations for VegFieldsDlg
	wxRadioButton* GetHeightRandom()  { return (wxRadioButton*) FindWindow( ID_HEIGHT_RANDOM ); }
	wxChoice* GetSpeciesChoice()  { return (wxChoice*) FindWindow( ID_SPECIES_CHOICE ); }
	wxRadioButton* GetUseSpecies()  { return (wxRadioButton*) FindWindow( ID_USE_SPECIES ); }
	wxRadioButton* GetBiotypeString()  { return (wxRadioButton*) FindWindow( ID_BIOTYPE_STRING ); }
	wxRadioButton* GetBiotypeInt()  { return (wxRadioButton*) FindWindow( ID_BIOTYPE_INT ); }
	wxRadioButton* GetCommonName()  { return (wxRadioButton*) FindWindow( ID_COMMON_NAME ); }
	wxRadioButton* GetSpeciesName()  { return (wxRadioButton*) FindWindow( ID_SPECIES_NAME ); }
	wxRadioButton* GetSpeciesId()  { return (wxRadioButton*) FindWindow( ID_SPECIES_ID ); }
	wxChoice* GetHeightField()  { return (wxChoice*) FindWindow( ID_HEIGHT_FIELD ); }
	wxChoice* GetSpeciesField()  { return (wxChoice*) FindWindow( ID_SPECIES_FIELD ); }

public:
	VegPointOptions m_options;

private:
	// WDR: member variable declarations for VegFieldsDlg
	vtVegLayer *m_pLayer;

	SHPHandle m_hSHP;
	int	   m_nElem, m_nShapeType;
	wxString  m_filename;
	uint	   m_iFields;

	bool	m_bUseSpecies;
	bool	m_bSpeciesUseField;
	bool	m_bHeightRandomize;
	bool	m_bHeightFixed;
	bool	m_bHeightUseField;
	float	m_fFixedHeight;

	void RefreshEnabled();

private:
	// WDR: handler declarations for VegFieldsDlg
	void OnRadio( wxCommandEvent &event );
	void OnOK( wxCommandEvent &event );
	void OnChoice2( wxCommandEvent &event );
	void OnChoice1( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __VegFieldsDlg_H__

