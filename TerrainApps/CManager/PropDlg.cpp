//
// Name:	PropDlg.cpp
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"

#include "PropDlg.h"
#include "vtdata/Content.h"
#include "vtui/AutoDialog.h"
#include "frame.h"
#include "TagDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// PropPanel
//----------------------------------------------------------------------------

// WDR: event table for PropPanel

BEGIN_EVENT_TABLE(PropPanel, PropPanelBase)
	EVT_INIT_DIALOG(PropPanel::OnInitDialog)
	EVT_TEXT( ID_ITEM, PropPanel::OnTextItem )
	EVT_CHOICE( ID_TYPECHOICE, PropPanel::OnChoiceType )
	EVT_CHOICE( ID_SUBTYPECHOICE, PropPanel::OnChoiceSubtype )
	EVT_BUTTON( ID_ADDTAG, PropPanel::OnAddTag )
	EVT_BUTTON( ID_REMOVETAG, PropPanel::OnRemoveTag )
	EVT_BUTTON( ID_EDITTAG, PropPanel::OnTagEdit )
END_EVENT_TABLE()

PropPanel::PropPanel( wxWindow *parent, wxWindowID id,
	const wxPoint &position, const wxSize& size, long style ) :
	PropPanelBase( parent, id, position, size, style )
{
	m_bUpdating = false;
	m_pCurrentItem = NULL;

	m_pTypeChoice = GetTypeChoice();
	m_pSubtypeChoice = GetSubtypeChoice();
	m_pTagList = GetTaglist();

	AddValidator(this, ID_ITEM, &m_strItem);
	AddValidator(this, ID_TYPECHOICE, &m_strType);
	AddValidator(this, ID_SUBTYPECHOICE, &m_strSubtype);
}

// WDR: handler implementations for PropPanel

void PropPanel::OnAddTag( wxCommandEvent &event )
{
	GetMainFrame()->RenderingPause();
	TagDlg dlg(GetMainFrame(), -1, _T("Add New Tag"));
	if (dlg.ShowModal() == wxID_OK)
	{
		vtTag tag;
		tag.name = dlg.m_strName.mb_str();
		tag.value = dlg.m_strValue.mb_str();
		m_pCurrentItem->AddTag(tag);
		UpdateTagList();
	}
	GetMainFrame()->RenderingResume();
}

void PropPanel::OnRemoveTag( wxCommandEvent &event )
{
	int sel = m_pTagList->GetNextItem(-1, wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	if (sel != -1)
	{
		int tagnum = m_pTagList->GetItemData(sel);
		m_pCurrentItem->RemoveTag(tagnum);
		UpdateTagList();
	}
}

void PropPanel::OnTagEdit( wxCommandEvent &event )
{
	GetMainFrame()->RenderingPause();

	int sel = m_pTagList->GetNextItem(-1, wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	if (sel != -1)
	{
		int tagnum = m_pTagList->GetItemData(sel);
		vtTag *tag = m_pCurrentItem->GetTag(tagnum);

		TagDlg dlg(GetMainFrame(), -1, _T("Edit Tag"));
		dlg.m_strName = wxString(tag->name, wxConvUTF8);
		dlg.m_strValue = wxString(tag->value, wxConvUTF8);
		if (dlg.ShowModal() == wxID_OK)
		{
			tag->name = dlg.m_strName.mb_str(wxConvUTF8);
			tag->value = dlg.m_strValue.mb_str(wxConvUTF8);
			UpdateTagList();
		}
	}
	GetMainFrame()->RenderingResume();
}

void PropPanel::OnInitDialog(wxInitDialogEvent& event)
{
	char buf[80];
	FILE *fp = fopen("itemtypes.txt", "rb");
	if (fp)
	{
		Pair p;
		while (fgets(buf, 80, fp))
		{
			// string EOL
			if (buf[strlen(buf)-1] == 10) buf[strlen(buf)-1] = 0;
			if (buf[strlen(buf)-1] == 13) buf[strlen(buf)-1] = 0;

			if (buf[0] == '\t')
				p.subtype = buf+1;
			else
			{
				p.type = buf;
				p.subtype = "";
			}
			m_types.push_back(p);
		}
		fclose(fp);
	}
	m_pTypeChoice->Clear();
	for (uint i = 0; i < m_types.size(); i++)
	{
		if (m_types[i].subtype == "")
			m_pTypeChoice->Append(wxString(m_types[i].type, wxConvUTF8));
	}
	m_pTypeChoice->SetSelection(0);
	UpdateSubtypes();

	m_pTagList->ClearAll();
	m_pTagList->SetSingleStyle(wxLC_REPORT);
	m_pTagList->InsertColumn(0, _T("Tag"));
	m_pTagList->SetColumnWidth(0, 100);
	m_pTagList->InsertColumn(1, _T("Value"));
	m_pTagList->SetColumnWidth(1, 120);
}

void PropPanel::OnChoiceType( wxCommandEvent &event )
{
	UpdateFromControls();
	UpdateSubtypes();
}

void PropPanel::OnChoiceSubtype( wxCommandEvent &event )
{
	UpdateFromControls();
}

void PropPanel::OnTextItem( wxCommandEvent &event )
{
	UpdateFromControls();
	if (!m_bUpdating)
		GetMainFrame()->RefreshTreeItems();
}


//////////////////////////////////////////////////////////////////////////

void PropPanel::UpdateSubtypes()
{
	vtString type = (const char *) m_pTypeChoice->GetStringSelection().mb_str();

	m_pSubtypeChoice->Clear();
	for (uint i = 0; i < m_types.size(); i++)
	{
		if (m_types[i].type == type)
			m_pSubtypeChoice->Append(wxString(m_types[i].subtype, wxConvUTF8));
	}
	if (m_pCurrentItem)
	{
		const char *subtype = m_pCurrentItem->GetValueString("subtype");
		if (subtype)
			m_pSubtypeChoice->SetStringSelection(wxString(subtype, wxConvUTF8));
	}
}

void PropPanel::UpdateTagList()
{
	m_pTagList->DeleteAllItems();
	if (!m_pCurrentItem)
		return;

	int item;
	vtTag *tag;
	for (uint i = 0; i < m_pCurrentItem->NumTags(); i++)
	{
		tag = m_pCurrentItem->GetTag(i);
		if (!tag->name.Compare("type"))
			continue;
		if (!tag->name.Compare("subtype"))
			continue;
		item = m_pTagList->InsertItem(i, wxString(tag->name, wxConvUTF8));
		m_pTagList->SetItem(item, 1, wxString(tag->value, wxConvUTF8));
		m_pTagList->SetItemData(item, i);
	}
}

void PropPanel::SetCurrentItem(vtItem *item)
{
	if (item)
	{
		m_strItem = wxString(item->m_name, wxConvUTF8);
		const char *type = item->GetValueString("type");
		if (type)
			m_strType = wxString(type, wxConvUTF8);
		else
			m_strType = _T("unknown");
		const char *subtype = item->GetValueString("subtype");
		if (subtype)
			m_strSubtype = wxString(subtype, wxConvUTF8);
		else
			m_strSubtype = _T("");
	}
	else
	{
		m_strItem = _T("");
		m_strType = _("unspecified");
	}
	m_pCurrentItem = item;

	m_bUpdating = true;
	TransferDataToWindow();
	int sel = m_pTypeChoice->FindString(m_strType);
	if (sel != -1)
		m_pTypeChoice->SetSelection(sel);
	else
		m_pTypeChoice->SetSelection(0);
	UpdateSubtypes();
	UpdateTagList();
	m_bUpdating = false;
}

void PropPanel::UpdateFromControls()
{
	if (m_bUpdating)
		return;

	TransferDataFromWindow();
	if (m_pCurrentItem)
	{
		m_pCurrentItem->m_name = m_strItem.mb_str();
		m_pCurrentItem->SetValueString("type", (const char *) m_strType.mb_str());
		//if (m_strSubtype != _T(""))
			m_pCurrentItem->SetValueString("subtype", (const char *) m_strSubtype.mb_str());
	}
}

