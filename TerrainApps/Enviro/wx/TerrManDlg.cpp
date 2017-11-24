//
// Name: TerrManDlg.cpp
//
// Copyright (c) 2003-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "TerrManDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/core/TParams.h"
#include "vtdata/FilePath.h"
#include "vtdata/DataPath.h"
#include "TerrManDlg.h"
#include "../Options.h"
#include "EnviroApp.h"

/////////////////////////

class TMTreeItemData : public wxTreeItemData
{
public:
	vtString m_strDir;
	vtString m_strXmlFile;
	vtString m_strName;
};

// WDR: class implementations

//---------------------------------------------------------------------------
// TerrainManagerDlg
//---------------------------------------------------------------------------

// WDR: event table for TerrainManagerDlg

BEGIN_EVENT_TABLE(TerrainManagerDlg,TerrManDlgBase)
	EVT_INIT_DIALOG (TerrainManagerDlg::OnInitDialog)
	EVT_TREE_SEL_CHANGED( ID_TREECTRL, TerrainManagerDlg::OnSelChanged )
	EVT_TREE_DELETE_ITEM( ID_TREECTRL, TerrainManagerDlg::OnDeleteItem )
	EVT_BUTTON( ID_ADD_PATH, TerrainManagerDlg::OnAddPath )
	EVT_BUTTON( ID_ADD_TERRAIN, TerrainManagerDlg::OnAddTerrain )
	EVT_BUTTON( ID_DELETE, TerrainManagerDlg::OnDelete )
	EVT_BUTTON( ID_EDIT_PARAMS, TerrainManagerDlg::OnEditParams )
	EVT_BUTTON( ID_COPY, TerrainManagerDlg::OnCopy )
END_EVENT_TABLE()

TerrainManagerDlg::TerrainManagerDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	TerrManDlgBase( parent, id, title, position, size, style | wxRESIZE_BORDER )
{
	m_pTree = GetTree();
	m_iSelect = 0;

	GetSizer()->SetSizeHints(this);
}

void TerrainManagerDlg::RefreshTreeContents()
{
	m_pTree->DeleteAllItems();

	vtStringArray &paths = vtGetDataPath();
	size_t i, num = paths.size();
	wxString wstr;

	m_Root = m_pTree->AddRoot(_("Terrain Data Paths"));

	for (i = 0; i < num; i++)
	{
		vtString str = paths[i];

		wstr = wxString(str, wxConvUTF8);
		wxTreeItemId hPath = m_pTree->AppendItem(m_Root, wstr);

		vtString directory = str + "Terrains";
		for (dir_iter it((const char *)directory); it != dir_iter(); ++it)
		{
			if (it.is_hidden() || it.is_directory())
				continue;

			std::string name1 = it.filename();
			vtString name = name1.c_str();

			// only look terrain parameters files
			vtString ext = GetExtension(name, false);
			if (ext.CompareNoCase(".xml") != 0)
				continue;

			TParams params;
			bool success = params.LoadFrom(directory + "/" + name);

			wstr = wxString(name, wxConvUTF8);
			if (success)
			{
				wstr += _T(" (");
				wxString wstr2(params.GetValueString(STR_NAME), wxConvUTF8);
				wstr += wstr2;
				wstr += _T(")");
			}

			wxTreeItemId hItem = m_pTree->AppendItem(hPath, wstr);
			TMTreeItemData *data = new TMTreeItemData;
			data->m_strDir = directory;
			data->m_strXmlFile = name;
			data->m_strName = params.GetValueString(STR_NAME);
			m_pTree->SetItemData(hItem, data);
		}
		// m_pTree->Expand(hPath);
	}
	m_pTree->Expand(m_Root);
}

void TerrainManagerDlg::RefreshTreeText()
{
	wxTreeItemId i1, i2;
	TParams params;

	wxTreeItemIdValue cookie1, cookie2;
	for (i1 = m_pTree->GetFirstChild(m_Root, cookie1); i1.IsOk(); i1 = m_pTree->GetNextChild(i1, cookie1))
	{
		for (i2 = m_pTree->GetFirstChild(i1, cookie2); i2.IsOk(); i2 = m_pTree->GetNextChild(i2, cookie2))
		{
			TMTreeItemData *data = (TMTreeItemData *) m_pTree->GetItemData(i2);
			vtString path = data->m_strDir + "/" + data->m_strXmlFile;
			if (params.LoadFrom(path))
			{
				data->m_strName = params.GetValueString(STR_NAME);
				wxString wstr(data->m_strXmlFile, wxConvUTF8);
				wstr += _T(" (");
				wstr += wxString(params.GetValueString(STR_NAME), wxConvUTF8);
				wstr += _T(")");
				m_pTree->SetItemText(i2, wstr);
			}
		}
	}
}

wxString TerrainManagerDlg::GetCurrentPath()
{
	wxTreeItemId parent = m_pTree->GetItemParent(m_Selected);
	return m_pTree->GetItemText(parent);
}

wxString TerrainManagerDlg::GetCurrentTerrainPath()
{
	wxString path = GetCurrentPath();
	TMTreeItemData *data = (TMTreeItemData *) m_pTree->GetItemData(m_Selected);
	path += _T("Terrains/");
	path += wxString(data->m_strXmlFile, wxConvUTF8);
	return path;
}

// WDR: handler implementations for TerrainManagerDlg

void TerrainManagerDlg::OnCopy( wxCommandEvent &event )
{
	if (m_iSelect != 2)
		return;

	TMTreeItemData *data = (TMTreeItemData *) m_pTree->GetItemData(m_Selected);
	wxString file(data->m_strXmlFile, wxConvUTF8);

	wxString msg = _("Please enter the name for the terrain copy.");
	wxString str = wxGetTextFromUser(msg, _("Add Copy of Terrain"), file);
	if (str == _T(""))
		return;

	TParams params;
	params.LoadFrom(GetCurrentTerrainPath().mb_str(wxConvUTF8));

	wxString newpath = GetCurrentPath();
	newpath += _T("Terrains/");
	newpath += str;
	params.WriteToXML(newpath.mb_str(wxConvUTF8), STR_TPARAMS_FORMAT_NAME);
	RefreshTreeContents();
}

void TerrainManagerDlg::OnEditParams( wxCommandEvent &event )
{
	if (m_iSelect != 2)
		return;

	wxString curpath = GetCurrentTerrainPath();
	int res = EditTerrainParameters(this, curpath.mb_str(wxConvUTF8));
	if (res == wxID_OK)
	{
		// They might have changed an .ini to .xml
		TMTreeItemData *data = (TMTreeItemData *) m_pTree->GetItemData(m_Selected);
		vtString str = data->m_strXmlFile;
		if (GetExtension(str, false) == ".ini")
			data->m_strXmlFile = str.Left(str.GetLength()-4)+".xml";

		// They might have changed the terrain name
		RefreshTreeText();
	}
}

void TerrainManagerDlg::OnDelete( wxCommandEvent &event )
{
	if (m_iSelect == 1)
	{
		// remove path
		vtStringArray &paths = vtGetDataPath();
		wxString path = m_pTree->GetItemText(m_Selected);
		vtString vpath = (const char *) path.mb_str(wxConvUTF8);
		for (vtStringArray::iterator it = paths.begin(); it != paths.end(); it++)
		{
			if (*it == vpath)
			{
				paths.erase(it);
				break;
			}
		}
		vtSaveDataPath();
		m_pTree->Delete(m_Selected);
	}
	if (m_iSelect == 2)
	{
		// delete terrain .ini file
		wxTreeItemId parent = m_pTree->GetItemParent(m_Selected);
		vtString path = (const char *) m_pTree->GetItemText(parent).mb_str(wxConvUTF8);
		path +="Terrains/";

		TMTreeItemData *data = (TMTreeItemData *) m_pTree->GetItemData(m_Selected);
		path += data->m_strXmlFile;
		vtDeleteFile(path);

		m_pTree->Delete(m_Selected);
	}
}

void TerrainManagerDlg::OnAddTerrain( wxCommandEvent &event )
{
	wxString msg = _("Please enter the name for a new terrain .xml file.");
	wxString str = wxGetTextFromUser(msg, _("Add Terrain"));
	if (str == _T(""))
		return;

	if (str.Right(4).CmpNoCase(_T(".xml")))
		str += _T(".xml");

	vtString path = (const char *) m_pTree->GetItemText(m_Selected).mb_str(wxConvUTF8);
	path += "Terrains";

	// Make sure Terrains directory exists
	vtCreateDir(path);

	path += "/";
	path += str.mb_str(wxConvUTF8);

	TParams params;
	params.SetValueString(STR_NAME, "Untitled");
	params.WriteToXML(path, STR_TPARAMS_FORMAT_NAME);

	RefreshTreeContents();
}

void TerrainManagerDlg::OnAddPath( wxCommandEvent &event )
{
#if 0
	// This is nice in that it allows you to type a relative path
	wxString msg = _T("Please enter an absolute or relative path.";
	wxString str = wxGetTextFromUser(msg, _T("Add Path"));
	if (str == _T(""))
		return;
#endif
	// Ask the user for a directory (can only be absolute)
	wxDirDialog getDir(this, _("Specify Data Directory"));
	bool bResult = (getDir.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	wxString str = getDir.GetPath();

	// Make sure there is a trailing slash
	if (str.Length() > 1)
	{
		char ch = str.GetChar(str.Length()-1);
		if (ch != '/' && ch != '\\')
			str += _T("/");
	}

	vtString path(str.mb_str(wxConvUTF8));
	vtGetDataPath().push_back(path);
	vtSaveDataPath();

	// To be helpful, also create most of the standard sub-directories
	vtString SubDirectory;
	SubDirectory = path + vtString("BuildingData");
	vtCreateDir(SubDirectory);
	SubDirectory = path + vtString("BuildingModels");
	vtCreateDir(SubDirectory);
	SubDirectory = path + vtString("Elevation");
	vtCreateDir(SubDirectory);
	SubDirectory = path + vtString("GeoSpecific");
	vtCreateDir(SubDirectory);
	SubDirectory = path + vtString("Locations");
	vtCreateDir(SubDirectory);
	SubDirectory = path + vtString("PlantData");
	vtCreateDir(SubDirectory);
	SubDirectory = path + vtString("RoadData");
	vtCreateDir(SubDirectory);
	SubDirectory = path + vtString("Terrains");
	vtCreateDir(SubDirectory);

	RefreshTreeContents();
}

void TerrainManagerDlg::OnInitDialog(wxInitDialogEvent& event)
{
	RefreshTreeContents();
	UpdateEnabling();
	wxWindow::OnInitDialog(event);
}


//
// Tree events
//
void TerrainManagerDlg::OnDeleteItem( wxTreeEvent &event )
{
}

void TerrainManagerDlg::OnSelChanged( wxTreeEvent &event )
{
	wxTreeItemId item = event.GetItem();
	m_Selected = item;

//  MyTreeItemData *data = (MyTreeItemData *)m_pTree->GetItemData(item);

	if (item == m_Root)
		m_iSelect = 0;
	else
	{
		item = m_pTree->GetItemParent(item);
		if (item == m_Root)
			m_iSelect = 1;
		else
		{
			item = m_pTree->GetItemParent(item);
			if (item == m_Root)
				m_iSelect = 2;
		}
	}
	UpdateEnabling();
}

void TerrainManagerDlg::UpdateEnabling()
{
	GetAddTerrain()->Enable(m_iSelect == 1);
	GetCopy()->Enable(m_iSelect == 2);
	GetDelete()->Enable(m_iSelect == 1 || m_iSelect == 2);
	if (m_iSelect == 1)
		GetDelete()->SetLabel(_("Remove"));
	else if (m_iSelect == 2)
		GetDelete()->SetLabel(_("Delete"));
	GetEditParams()->Enable(m_iSelect == 2);
}

