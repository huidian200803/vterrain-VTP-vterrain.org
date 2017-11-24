//
// Name: ScenarioParamsDialog.cpp
//
// Copyright (c) 2005-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include "wx/filename.h"

#include "ScenarioParamsDialog.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ScenarioParamsDialog
//----------------------------------------------------------------------------

// WDR: event table for ScenarioParamsDialog

BEGIN_EVENT_TABLE(ScenarioParamsDialog,ScenarioParamsDlgBase)
	EVT_INIT_DIALOG (ScenarioParamsDialog::OnInitDialog)
	EVT_BUTTON( ID_SCENARIO_ADD_VISIBLE_LAYER, ScenarioParamsDialog::OnScenarioAddVisibleLayer )
	EVT_BUTTON( ID_SCENARIO_REMOVE_VISIBLE_LAYER, ScenarioParamsDialog::OnScenarioRemoveVisibleLayer )
	EVT_LISTBOX( ID_SCENARIO_VISIBLE_LAYERS, ScenarioParamsDialog::OnScenarioVisibleLayers )
	EVT_LISTBOX( ID_SCENARIO_AVAILABLE_LAYERS, ScenarioParamsDialog::OnScenarioAvailableLayers )
	EVT_TEXT( ID_SCENARIO_NAME, ScenarioParamsDialog::OnScenarioNameText )
END_EVENT_TABLE()

ScenarioParamsDialog::ScenarioParamsDialog( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	ScenarioParamsDlgBase( parent, id, title, position, size, style )
{
	m_bModified = false;

	GetSizer()->SetSizeHints(this);
}

// WDR: handler implementations for ScenarioParamsDialog

void ScenarioParamsDialog::OnScenarioNameText( wxCommandEvent &event )
{
	wxString str = event.GetString();
	m_Params.SetValueString(STR_SCENARIO_NAME, (const char *) str.mb_str(wxConvUTF8));
	m_bModified = true;
}

void ScenarioParamsDialog::OnScenarioAvailableLayers( wxCommandEvent &event )
{
	UpdateEnableState();
}

void ScenarioParamsDialog::OnScenarioVisibleLayers( wxCommandEvent &event )
{
	UpdateEnableState();
}

void ScenarioParamsDialog::OnScenarioRemoveVisibleLayer( wxCommandEvent &event )
{
	wxListBox *pAvailableLayers = GetScenarioAvailableLayers();
	wxListBox *pVisibleLayers = GetScenarioVisibleLayers();
	int iOffset;

	if (wxNOT_FOUND != (iOffset = pVisibleLayers->GetSelection()))
	{
		wxString Layer = pVisibleLayers->GetStringSelection();
		pVisibleLayers->Delete(iOffset);
		m_Params.GetActiveLayers().erase(m_Params.GetActiveLayers().begin() + iOffset);
		pAvailableLayers->Append(Layer);
		m_bModified = true;
	}
	UpdateEnableState();
}

void ScenarioParamsDialog::OnScenarioAddVisibleLayer( wxCommandEvent &event )
{
	wxListBox *pAvailableLayers = GetScenarioAvailableLayers();
	wxListBox *pVisibleLayers = GetScenarioVisibleLayers();
	int iOffset;

	if (wxNOT_FOUND != (iOffset = pAvailableLayers->GetSelection()))
	{
		wxString Layer = pAvailableLayers->GetStringSelection();
		pAvailableLayers->Delete(iOffset);
		pVisibleLayers->Append(Layer);
		m_Params.GetActiveLayers().push_back((const char *) Layer.mb_str(wxConvUTF8));
		m_bModified = true;
	}
	UpdateEnableState();
}

void ScenarioParamsDialog::OnInitDialog(wxInitDialogEvent& event)
{
	wxTextCtrl *pScenarioName = GetScenarioName();
	wxListBox *pAvailableLayers = GetScenarioAvailableLayers();
	wxListBox *pVisibleLayers = GetScenarioVisibleLayers();
	vtStringArray TempVisibleLayers = m_Params.GetActiveLayers();

	wxString str(m_Params.GetValueString(STR_SCENARIO_NAME), wxConvUTF8);
	pScenarioName->SetValue(str);

	for (uint i = 0; i < m_Layers.size(); i++)
	{
		wxFileName FileName;
		wxString fname(m_Layers[i], wxConvUTF8);

		FileName.Assign(fname);
		pAvailableLayers->Append(FileName.GetName());
	}

	// Validate the passed in visible layers and reject any that are not available
	vtStringArray::iterator iTr = TempVisibleLayers.begin();
	while (iTr != TempVisibleLayers.end())
	{
		int iOffset;
		str = wxString((pcchar)(*iTr), wxConvUTF8);
		if (wxNOT_FOUND != (iOffset = pAvailableLayers->FindString(str)))
		{
			pVisibleLayers->Append(str);
			pAvailableLayers->Delete(iOffset);
			iTr++;
		}
		else
		{
			iTr = TempVisibleLayers.erase(iTr);
			m_bModified = true;
		}
	}

	m_Params.GetActiveLayers() = TempVisibleLayers;

	UpdateEnableState();
}

void ScenarioParamsDialog::UpdateEnableState()
{
	if (wxNOT_FOUND != GetScenarioAvailableLayers()->GetSelection())
		GetScenarioAddVisibleLayer()->Enable(true);
	else
		GetScenarioAddVisibleLayer()->Enable(false);
	if (wxNOT_FOUND != GetScenarioVisibleLayers()->GetSelection())
		GetScenarioRemoveVisibleLayer()->Enable(true);
	else
		GetScenarioRemoveVisibleLayer()->Enable(false);
}




