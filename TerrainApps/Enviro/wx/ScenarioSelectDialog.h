//
// Name: ScenarioSelectDialog.h
//
// Copyright (c) 2005-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __ScenarioSelectDialog_H__
#define __ScenarioSelectDialog_H__

#include "EnviroUI.h"
#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// CScenarioListValidator
//----------------------------------------------------------------------------

class CScenarioListValidator: public wxValidator
{
public:
	// constructors and destructors
	CScenarioListValidator(std::vector<ScenarioParams> *pScenarios) : m_pScenarios(pScenarios) {}

	CScenarioListValidator(const CScenarioListValidator& Rhs)
	{
		m_pScenarios = Rhs.m_pScenarios;
	}
	virtual wxObject *Clone() const { return new CScenarioListValidator(*this); }
	virtual bool Validate(wxWindow *pParent) { return true; }
	virtual bool TransferToWindow();
	virtual bool TransferFromWindow();

protected:
	std::vector<ScenarioParams> *m_pScenarios;
};

//----------------------------------------------------------------------------
// ScenarioSelectDialog
//----------------------------------------------------------------------------

class ScenarioSelectDialog: public ScenarioSelectDlgBase
{
public:
	// constructors and destructors
	ScenarioSelectDialog( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void SetTerrain(vtTerrain *pTerrain);
	virtual bool TransferDataToWindow();
	void ActivateCurrent();

	// WDR: method declarations for ScenarioSelectDialog
	wxButton* GetApply()  { return (wxButton*) FindWindow( wxID_APPLY ); }
	wxButton* GetScenarioNext()  { return (wxButton*) FindWindow( ID_SCENARIO_NEXT ); }
	wxButton* GetScenarioPrevious()  { return (wxButton*) FindWindow( ID_SCENARIO_PREVIOUS ); }
	wxButton* GetMovedownScenario()  { return (wxButton*) FindWindow( ID_MOVEDOWN_SCENARIO ); }
	wxButton* GetMoveupScenario()  { return (wxButton*) FindWindow( ID_MOVEUP_SCENARIO ); }
	wxButton* GetEditScenario()  { return (wxButton*) FindWindow( ID_EDIT_SCENARIO ); }
	wxButton* GetDeleteScenario()  { return (wxButton*) FindWindow( ID_DELETE_SCENARIO ); }
	wxButton* GetNewScenario()  { return (wxButton*) FindWindow( ID_NEW_SCENARIO ); }
	wxListBox* GetScenarioList()  { return (wxListBox*) FindWindow( ID_SCENARIO_LIST ); }

private:
	// WDR: member variable declarations for ScenarioSelectDialog

private:
	// WDR: handler declarations for ScenarioSelectDialog
	void OnScenarioNext( wxCommandEvent &event );
	void OnScenarioPrevious( wxCommandEvent &event );
	void OnMoveDownScenario( wxCommandEvent &event );
	void OnMoveUpScenario( wxCommandEvent &event );
	void OnEditScenario( wxCommandEvent &event );
	void OnDeleteScenario( wxCommandEvent &event );
	void OnNewScenario( wxCommandEvent &event );
	void OnScenarioList( wxCommandEvent &event );
	void OnApply(wxCommandEvent& event);
	void OnOK(wxCommandEvent& event);
	void OnCancel(wxCommandEvent& event);

private:
	DECLARE_EVENT_TABLE()
	bool m_bModified;

protected:
	void UpdateEnableState();
	vtTerrain *m_pTerrain;
	std::vector<ScenarioParams> m_Scenarios;
};

#endif
