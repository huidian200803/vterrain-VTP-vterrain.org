//
// Name: ScenarioSelectDialog.cpp
//
// Copyright (c) 2005-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "ScenarioSelectDialog.h"
#include "ScenarioParamsDialog.h"
#include "EnviroGUI.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// CScenarioListValidator
//----------------------------------------------------------------------------
bool CScenarioListValidator::TransferToWindow()
{
	wxListBox* pListBox = wxDynamicCast(GetWindow(), wxListBox);
	if (NULL != pListBox)
	{
		pListBox->Clear();
		for (uint i = 0; i < m_pScenarios->size(); i++)
		{
			wxString str((*m_pScenarios)[i].GetValueString(STR_SCENARIO_NAME), wxConvUTF8);
			pListBox->Append(str);
		}
		return true;
	}
	else
		return false;
}

bool CScenarioListValidator::TransferFromWindow()
{
	// Just do a sanity check
	// the control should update its data real time
	wxListBox* pListBox = wxDynamicCast(GetWindow(), wxListBox);
	if (NULL != pListBox)
		return true;
	else
		return false;
}

//----------------------------------------------------------------------------
// ScenarioSelectDialog
//----------------------------------------------------------------------------

// WDR: event table for ScenarioSelectDialog

BEGIN_EVENT_TABLE(ScenarioSelectDialog,ScenarioSelectDlgBase)
	EVT_LISTBOX( ID_SCENARIO_LIST, ScenarioSelectDialog::OnScenarioList )
	EVT_BUTTON( ID_NEW_SCENARIO, ScenarioSelectDialog::OnNewScenario )
	EVT_BUTTON( ID_DELETE_SCENARIO, ScenarioSelectDialog::OnDeleteScenario )
	EVT_BUTTON( ID_EDIT_SCENARIO, ScenarioSelectDialog::OnEditScenario )
	EVT_BUTTON( ID_MOVEUP_SCENARIO, ScenarioSelectDialog::OnMoveUpScenario )
	EVT_BUTTON( ID_MOVEDOWN_SCENARIO, ScenarioSelectDialog::OnMoveDownScenario )
	EVT_BUTTON( ID_SCENARIO_PREVIOUS, ScenarioSelectDialog::OnScenarioPrevious )
	EVT_BUTTON( ID_SCENARIO_NEXT, ScenarioSelectDialog::OnScenarioNext )
	EVT_BUTTON(wxID_OK, ScenarioSelectDialog::OnOK)
	EVT_BUTTON(wxID_APPLY, ScenarioSelectDialog::OnApply)
	EVT_BUTTON(wxID_CANCEL, ScenarioSelectDialog::OnCancel)
END_EVENT_TABLE()

ScenarioSelectDialog::ScenarioSelectDialog( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	ScenarioSelectDlgBase( parent, id, title, position, size, style )
{
	GetScenarioList()->SetValidator(CScenarioListValidator(&m_Scenarios));

	m_bModified = false;

	GetSizer()->SetSizeHints(this);
}

void ScenarioSelectDialog::SetTerrain(vtTerrain *pTerrain)
{
	m_pTerrain = pTerrain;
	if (pTerrain)
		m_Scenarios = pTerrain->GetParams().m_Scenarios;
	else
		m_Scenarios.clear();
	m_bModified = false;
	UpdateEnableState();
}

bool ScenarioSelectDialog::TransferDataToWindow()
{
	bool bRet = wxDialog::TransferDataToWindow();

	UpdateEnableState();

	return bRet;
}

void ScenarioSelectDialog::ActivateCurrent()
{
	int sel = GetScenarioList()->GetSelection();
	if (sel < 0)
		return;
	if (m_pTerrain)
		m_pTerrain->ActivateScenario(sel);
	g_App.SetScenario(sel);
}

void ScenarioSelectDialog::UpdateEnableState()
{
	int iSelected = GetScenarioList()->GetSelection();
	if (iSelected != wxNOT_FOUND)
	{
		GetEditScenario()->Enable(true);
		GetDeleteScenario()->Enable(true);
		if (iSelected != (GetScenarioList()->GetCount() - 1))
			GetMovedownScenario()->Enable(true);
		else
			GetMovedownScenario()->Enable(false);
		if (iSelected != 0)
			GetMoveupScenario()->Enable(true);
		else
			GetMoveupScenario()->Enable(false);
		if (m_bModified)
		{
			GetScenarioNext()->Enable(false);
			GetScenarioPrevious()->Enable(false);
		}
		else
		{
			GetScenarioNext()->Enable(true);
			GetScenarioPrevious()->Enable(true);
		}
	}
	else
	{
		GetEditScenario()->Enable(false);
		GetDeleteScenario()->Enable(false);
		GetMoveupScenario()->Enable(false);
		GetMovedownScenario()->Enable(false);
		GetScenarioNext()->Enable(false);
		GetScenarioPrevious()->Enable(false);
	}
	if (m_bModified)
		GetApply()->Enable(true);
	else
		GetApply()->Enable(false);
}


// WDR: handler implementations for ScenarioSelectDialog

void ScenarioSelectDialog::OnScenarioNext( wxCommandEvent &event )
{
	wxListBox *pScenarioList = GetScenarioList();
	int iCount = pScenarioList->GetCount();
	if (!m_bModified)
	{
		pScenarioList->SetSelection((pScenarioList->GetSelection() + 1) % iCount);
		ActivateCurrent();
		UpdateEnableState();
	}
}

void ScenarioSelectDialog::OnScenarioPrevious( wxCommandEvent &event )
{
	wxListBox *pScenarioList = GetScenarioList();
	int iCount = pScenarioList->GetCount();
	if (!m_bModified)
	{
		pScenarioList->SetSelection((pScenarioList->GetSelection() + iCount - 1) % iCount);
		ActivateCurrent();
		UpdateEnableState();
	}
}

void ScenarioSelectDialog::OnMoveDownScenario( wxCommandEvent &event )
{
	wxListBox *pScenarioList = GetScenarioList();
	int iSelected = pScenarioList->GetSelection();

	if ((iSelected != wxNOT_FOUND) && (iSelected != (pScenarioList->GetCount() - 1)))
	{
		ScenarioParams TempParams = m_Scenarios[iSelected];
		wxString TempString = pScenarioList->GetString(iSelected);
		pScenarioList->Delete(iSelected);
// Bug in wxWidgets
//	  pScenarioList->SetSelection(pScenarioList->Insert(TempString, iSelected + 1));
		pScenarioList->Insert(TempString, iSelected + 1);
		pScenarioList->SetSelection(iSelected + 1);
		m_Scenarios.erase(m_Scenarios.begin() + iSelected);
		m_Scenarios.insert(m_Scenarios.begin() + iSelected + 1,TempParams);
		m_bModified = true;
		UpdateEnableState();
	}
}

void ScenarioSelectDialog::OnMoveUpScenario( wxCommandEvent &event )
{
	wxListBox *pScenarioList = GetScenarioList();
	int iSelected = pScenarioList->GetSelection();

	if ((iSelected != wxNOT_FOUND) && (iSelected != 0))
	{
		ScenarioParams TempParams = m_Scenarios[iSelected];
		wxString TempString = pScenarioList->GetString(iSelected);
		pScenarioList->Delete(iSelected);
// Bug in wxWidgets
//	  pScenarioList->SetSelection(pScenarioList->Insert(TempString, iSelected - 1));
		pScenarioList->Insert(TempString, iSelected - 1);
		pScenarioList->SetSelection(iSelected - 1);
		m_Scenarios.erase(m_Scenarios.begin() + iSelected);
		m_Scenarios.insert(m_Scenarios.begin() + iSelected - 1,TempParams);
		m_bModified = true;
		UpdateEnableState();
	}
}

void ScenarioSelectDialog::OnEditScenario( wxCommandEvent &event )
{
	wxListBox *pScenarioList = GetScenarioList();
	ScenarioParamsDialog ScenarioParamsDialog(this, -1, _("Scenario Parameters"));
	int iSelected = pScenarioList->GetSelection();

	if (iSelected != wxNOT_FOUND)
	{
		vtStringArray lnames;
		if (m_pTerrain->IsCreated())
		{
			LayerSet &layers = m_pTerrain->GetLayers();
			for (uint i = 0; i < layers.size(); i++)
				lnames.push_back(layers[i]->GetLayerName());
		}
		else
		{
			std::vector<vtTagArray> &layers = m_pTerrain->GetParams().m_Layers;
			for (uint i = 0; i < layers.size(); i++)
				lnames.push_back(layers[i].GetValueString("Filename"));
		}

		ScenarioParamsDialog.SetAvailableLayers(lnames);
		ScenarioParamsDialog.SetParams(m_Scenarios[iSelected]);

		if (wxID_OK == ScenarioParamsDialog.ShowModal())
		{
			if (ScenarioParamsDialog.IsModified())
			{
				wxString str(m_Scenarios[iSelected].GetValueString(STR_SCENARIO_NAME), wxConvUTF8);
				m_Scenarios[iSelected] = ScenarioParamsDialog.GetParams();
				pScenarioList->SetString(iSelected, str);
				m_bModified = true;
			}
		}
	}
}

void ScenarioSelectDialog::OnDeleteScenario( wxCommandEvent &event )
{
	wxListBox *pScenarioList = GetScenarioList();
	int iSelected = pScenarioList->GetSelection();

	if (iSelected != wxNOT_FOUND)
	{
		pScenarioList->Delete(iSelected);
		m_Scenarios.erase(m_Scenarios.begin() + iSelected);
		m_bModified = true;
		UpdateEnableState();
	}
}

void ScenarioSelectDialog::OnNewScenario( wxCommandEvent &event )
{
	wxString ScenarioName = wxGetTextFromUser(_("Enter Scenario Name"), _("New Scenario"));
	wxListBox *pScenarioList = GetScenarioList();

	if (!ScenarioName.IsEmpty())
	{
		ScenarioParams Scenario;

		Scenario.SetValueString(STR_SCENARIO_NAME,
			(const char *) ScenarioName.mb_str(wxConvUTF8), true);
		m_Scenarios.push_back(Scenario);
		pScenarioList->SetSelection(pScenarioList->Append(ScenarioName));
		m_bModified = true;
		UpdateEnableState();
	}
}

void ScenarioSelectDialog::OnScenarioList( wxCommandEvent &event )
{
	UpdateEnableState();
	if (!m_bModified)
	{
		// Activate the selected scenario
		ActivateCurrent();
	}
}

void ScenarioSelectDialog::OnApply(wxCommandEvent& event)
{
	std::vector<ScenarioParams> TempParams = m_pTerrain->GetParams().m_Scenarios;

	m_pTerrain->GetParams().m_Scenarios = m_Scenarios;
	if (m_pTerrain->GetParams().WriteToXML(m_pTerrain->GetParamFile(), STR_TPARAMS_FORMAT_NAME))
	{
		m_bModified = false;
		UpdateEnableState();
		event.Skip();
	}
	else
	{
		wxString str;
		str.Printf(_("Couldn't save to file %hs.\n"), (const char *)m_pTerrain->GetParamFile());
		str += _("Please make sure the file is not read-only.");
		wxMessageBox(str);
		m_pTerrain->GetParams().m_Scenarios = TempParams;
	}
}

void ScenarioSelectDialog::OnOK(wxCommandEvent& event)
{
	if (m_bModified)
	{
		std::vector<ScenarioParams> TempParams = m_pTerrain->GetParams().m_Scenarios;

		m_pTerrain->GetParams().m_Scenarios = m_Scenarios;
		if (m_pTerrain->GetParams().WriteToXML(m_pTerrain->GetParamFile(), STR_TPARAMS_FORMAT_NAME))
		{
			m_bModified = false;
			UpdateEnableState();
			event.Skip();
		}
		else
		{
			wxString str;
			str.Printf(_("Couldn't save to file %hs.\n"), (const char *)m_pTerrain->GetParamFile());
			str += _("Please make sure the file is not read-only.");
			wxMessageBox(str);
			m_pTerrain->GetParams().m_Scenarios = TempParams;
		}
	}
	else
		event.Skip();
}

void ScenarioSelectDialog::OnCancel(wxCommandEvent& event)
{
	if (m_bModified)
	{
		// Reset the data
		SetTerrain(m_pTerrain);
		TransferDataToWindow();
	}
	event.Skip();
}




