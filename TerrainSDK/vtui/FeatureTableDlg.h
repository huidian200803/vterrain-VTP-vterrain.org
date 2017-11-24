//
// Name: FeatureTableDlg.h
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __FeatureTableDlg_H__
#define __FeatureTableDlg_H__

#include "vtui_UI.h"
#include "AutoDialog.h"
#include "vtdata/Features.h"

class vtFeatures;

// WDR: class declarations

//----------------------------------------------------------------------------
// FeatInfoDlg
//----------------------------------------------------------------------------

class FeatureTableDlg: public FeatureTableDlgBase
{
public:
	// constructors and destructors
	FeatureTableDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void SetFeatureSet(vtFeatureSet *pFeatures);
	vtFeatureSet *GetFeatureSet() { return m_pFeatures; }
	void Clear();
	void ShowSelected();
	void ShowPicked();
	void ShowAll();
	void ShowFeature(int iFeat);
	void UpdateFeatureText(int iItem, int iFeat);
	void RefreshItems();
	bool EditValue(int iFeature, int iColumn);

	virtual void OnModified() {}
	virtual void RefreshViz() {}
	virtual void OnFeatureDelete(vtFeature *f) {}
	virtual void OnFieldEdited(uint iIndex) {}
	virtual void OnEditEnd() {}

	// WDR: method declarations for FeatInfoDlg
	wxStaticText* GetTextVertical()  { return (wxStaticText*) FindWindow( ID_TEXT_VERTICAL ); }
	wxButton* GetDelHigh()  { return (wxButton*) FindWindow( ID_DEL_HIGH ); }
	wxChoice* GetChoiceVertical()  { return (wxChoice*) FindWindow( ID_CHOICE_VERTICAL ); }
	wxChoice* GetChoiceShow()  { return (wxChoice*) FindWindow( ID_CHOICE_SHOW ); }
	wxListCtrl* GetList()  { return (wxListCtrl*) FindWindow( ID_LIST ); }

protected:
	void UpdateTitle();

	// WDR: member variable declarations for FeatInfoDlg
	vtFeatureSet *m_pFeatures;
	int m_iShow;
	int m_iVUnits;
	int	m_iCoordColumns;

	bool m_bGeo;	// true if coordinates are geographic (lon-lat)

private:
	// WDR: handler declarations for FeatInfoDlg
	void OnItemRightClick( wxListEvent &event );
	void OnLeftDClick( wxMouseEvent &event );
	void OnDeleteHighlighted( wxCommandEvent &event );
	void OnChoiceVertical( wxCommandEvent &event );
	void OnChoiceShow( wxCommandEvent &event );
	void OnListRightClick( wxListEvent &event );
	void OnItemSelected( wxListEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __FeatureTableDlg_H__

