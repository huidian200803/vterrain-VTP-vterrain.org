//
// InstanceDlg.cpp
//
// Copyright (c) 2003-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/filename.h>

#include "InstanceDlg.h"

#include "vtdata/Content.h"
#include "vtdata/DataPath.h"
#include "vtdata/FileFilters.h"
#include "vtdata/FilePath.h"
#include "vtdata/vtLog.h"
#include "vtui/Helper.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// InstanceDlg
//----------------------------------------------------------------------------

// WDR: event table for InstanceDlg

BEGIN_EVENT_TABLE(InstanceDlg, InstanceDlgBase)
	EVT_INIT_DIALOG (InstanceDlg::OnInitDialog)
	EVT_RADIOBUTTON( ID_RADIO_CONTENT, InstanceDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO_MODEL, InstanceDlg::OnRadio )
	EVT_CHOICE( ID_CHOICE_FILE, InstanceDlg::OnChoice )
	EVT_CHOICE( ID_CHOICE_TYPE, InstanceDlg::OnChoice )
	EVT_CHOICE( ID_CHOICE_ITEM, InstanceDlg::OnChoiceItem )
	EVT_BUTTON( ID_BROWSE_MODEL_FILE, InstanceDlg::OnBrowseModelFile )
	EVT_TEXT( ID_LOCATION, InstanceDlg::OnLocationText )
	EVT_BUTTON( ID_CREATE, InstanceDlg::OnButtonCreate )
END_EVENT_TABLE()

InstanceDlg::InstanceDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	InstanceDlgBase( parent, id, title, position, size, style )
{
	m_bContent = true;
	m_iManager = 0;
	m_iItem = 0;
	m_bSetting = false;

	// Work around wxFormDesigner's lack of support for limiting to smallest size
	GetSizer()->SetSizeHints(this);

	AddValidator(this, ID_RADIO_CONTENT, &m_bContent);
	AddValidator(this, ID_CHOICE_FILE, &m_iManager);
	AddValidator(this, ID_CHOICE_ITEM, &m_iItem);
}

void InstanceDlg::UpdateLoc()
{
	LinearUnits lu = m_crs.GetUnits();

	wxString str;
	if (lu == LU_DEGREES)
		str.Printf(_T("%lf, %lf"), m_pos.x, m_pos.y);
	else
		str.Printf(_T("%.2lf, %.2lf"), m_pos.x, m_pos.y);
	m_bSetting = true;
	GetLocation()->SetValue(str);
	m_bSetting = false;
}

void InstanceDlg::SetLocation(const DPoint2 &pos)
{
	m_pos = pos;
	UpdateLoc();
}

vtTagArray *InstanceDlg::GetTagArray()
{
	m_dummy.Clear();

	// Return a description of the current content item
	if (m_bContent)
	{
		if (m_iItem == -1)
			return NULL;

		vtContentManager *cman = Current();
		if (!cman)
			return NULL;
		if (m_iItem >= (int) cman->NumItems())
			return NULL;
		vtItem *item = cman->GetItem(m_iItem);
		if (!item)
			return NULL;
		m_dummy.SetValueString("itemname", item->m_name, true);
	}
	else
	{
		wxString str = GetModelFile()->GetValue();
		m_dummy.SetValueString("filename", (const char *) str.mb_str(wxConvUTF8), true);
	}
	return &m_dummy;
}

void InstanceDlg::UpdateEnabling()
{
	GetChoiceFile()->Enable(m_bContent);
	GetChoiceType()->Enable(m_bContent);
	GetChoiceItem()->Enable(m_bContent);

	GetModelFile()->Enable(!m_bContent);
	GetBrowseModelFile()->Enable(!m_bContent);
}

void InstanceDlg::UpdateContentItems()
{
	GetChoiceItem()->Clear();

//	for (int i = 0; i < m_contents.size(); i++)
//	{
		vtContentManager *mng = Current();
		if (!mng)
			return;

		wxString str;
		for (uint j = 0; j < mng->NumItems(); j++)
		{
			vtItem *item = mng->GetItem(j);
			str = wxString(item->m_name, wxConvUTF8);
//			str += _T(" (");
//			str += item->GetValue("filename");
//			str += _T(")");
			GetChoiceItem()->Append(str);
		}
//	}
	GetChoiceItem()->SetSelection(0);
}


void InstanceDlg::ClearContent()
{
	m_contents.clear();
}

void InstanceDlg::AddContent(vtContentManager *mng)
{
	m_contents.push_back(mng);
}

// WDR: handler implementations for InstanceDlg

void InstanceDlg::OnInitDialog(wxInitDialogEvent& event)
{
	GetChoiceFile()->Clear();
	for (uint i = 0; i < m_contents.size(); i++)
	{
		vtContentManager *mng = m_contents[i];
		vtString str = mng->GetFilename();
		wxString ws(str, wxConvUTF8);
		GetChoiceFile()->Append(ws);
	}
	GetChoiceFile()->Select(0);

	GetChoiceType()->Clear();
	GetChoiceType()->Append(_("(All)"));
	GetChoiceType()->Select(0);

	UpdateLoc();
	UpdateEnabling();
	UpdateContentItems();

	wxDialog::OnInitDialog(event);
}

void InstanceDlg::OnLocationText( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	wxString str = GetLocation()->GetValue();

	double x, y;
	int num = sscanf((const char *) str.ToAscii(), "%lf, %lf", &x, &y);
	if (num == 2)
		m_pos.Set(x, y);
}

void InstanceDlg::OnBrowseModelFile( wxCommandEvent &event )
{
	wxString filter = _("3D Model files");
	filter += _T("|");
	AddType(filter, FSTRING_3DS);
	AddType(filter, FSTRING_DAE);
	AddType(filter, FSTRING_OBJ);
	AddType(filter, FSTRING_LWO);
	AddType(filter, FSTRING_FLT);
	AddType(filter, FSTRING_OSG);
	AddType(filter, FSTRING_IVE);
	AddType(filter, FSTRING_PLY);
	filter += _T("|");
	filter += FSTRING_ALL;
	wxFileDialog SelectFile(this, _("Choose model file"),
							_T(""), _T(""), filter, wxFD_OPEN);
	if (SelectFile.ShowModal() != wxID_OK)
		return;

	// If model file can be found by mimicing the search strategy implemented by
	// vtStructInstance3d::CreateNode then remove the directory part of the path
	// so that paths are not stored unnecessarily
	wxFileName TargetModel(SelectFile.GetPath());
	wxString filepart = TargetModel.GetFullName();
	vtString FoundModel = FindFileOnPaths(vtGetDataPath(), filepart.mb_str(wxConvUTF8));
	if ("" != FoundModel)
	{
		GetModelFile()->SetValue(filepart);
		return;
	}
	wxString test = wxT("BuildingModels/") + filepart;
	FoundModel = FindFileOnPaths(vtGetDataPath(), test.mb_str(wxConvUTF8));
	if ("" != FoundModel)
	{
		GetModelFile()->SetValue(filepart);
		return;
	}
	// Otherwise, use the full absolute path
	GetModelFile()->SetValue(SelectFile.GetPath());
}

void InstanceDlg::OnButtonCreate( wxCommandEvent &event )
{
	OnCreate();
}

void InstanceDlg::OnChoice( wxCommandEvent &event )
{
	TransferDataFromWindow();
	UpdateContentItems();
}

void InstanceDlg::OnChoiceItem( wxCommandEvent &event )
{
	TransferDataFromWindow();
}

void InstanceDlg::OnRadio( wxCommandEvent &event )
{
	TransferDataFromWindow();
	UpdateEnabling();
}

vtContentManager *InstanceDlg::Current()
{
	if (m_iManager < (int) m_contents.size())
		return m_contents[m_iManager];
	return NULL;
}

