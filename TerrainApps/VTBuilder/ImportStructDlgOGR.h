//
// Name: ImportStructOGRDlg.h
//
// Copyright (c) 2003-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __ImportStructOGRDlg_H__
#define __ImportStructOGRDlg_H__

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "VTBuilder_UI.h"
#include "vtdata/StructArray.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// ImportStructOGRDlg
//----------------------------------------------------------------------------

class ImportStructOGRDlg: public ImportStructOGRDlgBase
{
public:
	ImportStructOGRDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for ImportStructOGRDlg
	wxChoice* GetLayername()  { return (wxChoice*) FindWindow( ID_LAYERNAME ); }
	wxCheckBox* GetUse25d()  { return (wxCheckBox*) FindWindow( ID_USE_25D ); }
	wxCheckBox* GetBuildFoundations()  { return (wxCheckBox*) FindWindow( ID_BUILD_FOUNDATIONS ); }
	wxChoice* GetChoiceHeightType()  { return (wxChoice*) FindWindow( ID_CHOICE_HEIGHT_TYPE ); }
	wxChoice* GetChoiceHeightField()  { return (wxChoice*) FindWindow( ID_CHOICE_HEIGHT_FIELD ); }
	wxChoice* GetChoiceFileField()  { return (wxChoice*) FindWindow( ID_CHOICE_FILE_FIELD ); }
	wxRadioButton* GetTypeInstance()  { return (wxRadioButton*) FindWindow( ID_TYPE_INSTANCE ); }
	wxRadioButton* GetTypeLinear()  { return (wxRadioButton*) FindWindow( ID_TYPE_LINEAR ); }
	wxRadioButton* GetTypeBuilding()  { return (wxRadioButton*) FindWindow( ID_TYPE_BUILDING ); }
	void SetDatasource(GDALDataset *pDatasource) { m_pDatasource = pDatasource; }
	void UpdateFieldNames();
	void UpdateEnables();
	bool GetRadio(int id);

	int	 m_iType;
//	int	 m_iHeightType; // 0 = stories, 1 = meters, 2 = feet
//	int	 m_iElevationUnits; // 0 = meters, 1 = feet

	StructImportOptions m_opt;

private:
	// WDR: member variable declarations for ImportStructOGRDlg
	GDALDataset *m_pDatasource;

private:
	// WDR: handler declarations for ImportStructOGRDlg
	void OnChoiceLayerName( wxCommandEvent &event );
	void OnChoiceFileField( wxCommandEvent &event );
	void OnChoiceHeightField( wxCommandEvent &event );
	void OnRadio( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif	// __ImportStructOGRDlg_H__

