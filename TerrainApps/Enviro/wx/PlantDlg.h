//
// Name:		PlantDlg.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __PlantDlg_H__
#define __PlantDlg_H__

#include "EnviroUI.h"
#include "vtui/AutoDialog.h"
#include "../PlantingOptions.h"
#include "vtdata/Array.h"

class vtSpeciesList3d;
class vtPlantInstance3d;

// WDR: class declarations

//---------------------------------------------------------------------------
// PlantDlg
//---------------------------------------------------------------------------

class PlantDlg: public PlantDlgBase
{
public:
	// constructors and destructors
	PlantDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void SetSpeciesList(vtSpeciesList3d *plants);
	void SetDlgPlantOptions(PlantingOptions &opt);
	void ShowOnlyAvailableSpecies(bool bFlag) { m_bOnlyAvailableSpecies = bFlag; }
	void SetLang(const wxString &strTwoLetterLangCode);

	// WDR: method declarations for PlantDlg
	wxChoice* GetLanguage()  { return (wxChoice*) FindWindow( ID_LANGUAGE ); }
	wxCheckBox* GetCommonNames()  { return (wxCheckBox*) FindWindow( ID_COMMON_NAMES ); }
	wxSlider* GetPlantVarianceSlider()  { return (wxSlider*) FindWindow( ID_PLANT_VARIANCE_SLIDER ); }
	wxRadioButton* GetPlantContinuous()  { return (wxRadioButton*) FindWindow( ID_PLANT_CONTINUOUS ); }
	wxRadioButton* GetPlantLinear()  { return (wxRadioButton*) FindWindow( ID_PLANT_LINEAR ); }
	wxRadioButton* GetPlantIndividual()  { return (wxRadioButton*) FindWindow( ID_PLANT_INDIVIDUAL ); }
	wxSlider* GetHeightSlider()  { return (wxSlider*) FindWindow( ID_HEIGHT_SLIDER ); }
	wxChoice* GetSpecies()  { return (wxChoice*) FindWindow( ID_SPECIES ); }

protected:
	void UpdateEnabling();
	void UpdateAvailableLanguages();
	void UpdatePlantSizes();
	void UpdatePlantNames();
	void HeightToSlider();
	void ModeToRadio();
	void SpeciesIdToSpeciesIndex();
	void SpeciesIndexToSpeciesId();
	void UpdateHeightFromSpecies();

private:
	// WDR: member variable declarations for PlantDlg
	bool m_bSetting;
	bool m_bOnlyAvailableSpecies;

	// planting options
	PlantingOptions m_opt;

	bool m_bCommonNames;
	int  m_iLanguage;
	int  m_iHeightSlider;
	int  m_iVarianceSlider;
	int  m_iSpeciesChoice;
	wxString m_strLang;

	wxSlider		*m_pHeightSlider;
	wxChoice		*m_pSpecies;
	vtSpeciesList3d *m_pSpeciesList;

	// Each species has a size (height) which the user has indicated
	std::vector<float> m_PreferredSizes;

private:
	// WDR: handler declarations for PlantDlg
	void OnInitDialog(wxInitDialogEvent& event);
	void OnLanguage( wxCommandEvent &event );
	void OnCommonNames( wxCommandEvent &event );
	void OnVarianceSlider( wxCommandEvent &event );
	void OnVariance( wxCommandEvent &event );
	void OnRadio( wxCommandEvent &event );
	void OnSpacingEdit( wxCommandEvent &event );
	void OnSelChangeSpecies( wxCommandEvent &event );
	void OnHeightSlider( wxCommandEvent &event );
	void OnHeightEdit( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __PlantDlg_H__

