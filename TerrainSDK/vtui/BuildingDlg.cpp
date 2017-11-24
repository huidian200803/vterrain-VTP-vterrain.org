//
// Name: BuildingDlg.cpp
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/colordlg.h>
#include <wx/numdlg.h>

#include "BuildingDlg.h"
#include "vtui/Helper.h"
#include "vtdata/DataPath.h"
#include "vtdata/MaterialDescriptor.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// BuildingDlg
//----------------------------------------------------------------------------

// Statics
wxColourData BuildingDlg::s_ColorData;


// WDR: event table for BuildingDlg

BEGIN_EVENT_TABLE(BuildingDlg, BuildingDlgBase)
	EVT_INIT_DIALOG (BuildingDlg::OnInitDialog)
	EVT_BUTTON( wxID_OK, BuildingDlg::OnOK )
	EVT_BUTTON( ID_SET_COLOR, BuildingDlg::OnColor1 )
	EVT_LISTBOX( ID_LEVEL, BuildingDlg::OnLevel )
	EVT_LISTBOX( ID_EDGE, BuildingDlg::OnEdge )
	EVT_BUTTON( ID_SET_ROOF_TYPE, BuildingDlg::OnSetRoofType )
	EVT_BUTTON( ID_SET_EDGE_SLOPES, BuildingDlg::OnSetEdgeSlopes )
	EVT_BUTTON( ID_SET_MATERIAL, BuildingDlg::OnSetMaterial )
	EVT_SPINCTRL( ID_STORIES, BuildingDlg::OnSpinStories )
	EVT_TEXT( ID_STORY_HEIGHT, BuildingDlg::OnStoryHeight )
	EVT_BUTTON( ID_LEVEL_COPY, BuildingDlg::OnLevelCopy )
	EVT_BUTTON( ID_LEVEL_UP, BuildingDlg::OnLevelUp )
	EVT_BUTTON( ID_LEVEL_DEL, BuildingDlg::OnLevelDelete )
	EVT_BUTTON( ID_LEVEL_DOWN, BuildingDlg::OnLevelDown )
	EVT_BUTTON( ID_EDGES, BuildingDlg::OnEdges )
	EVT_TEXT( ID_EDGE_SLOPE, BuildingDlg::OnEdgeSlope )
	EVT_BUTTON( ID_FEAT_CLEAR, BuildingDlg::OnFeatClear )
	EVT_BUTTON( ID_FEAT_WALL, BuildingDlg::OnFeatWall )
	EVT_BUTTON( ID_FEAT_WINDOW, BuildingDlg::OnFeatWindow )
	EVT_BUTTON( ID_FEAT_DOOR, BuildingDlg::OnFeatDoor )
	EVT_CLOSE(BuildingDlg::OnCloseWindow)
	EVT_CHAR_HOOK(BuildingDlg::OnCharHook)
	EVT_CHOICE( ID_FACADE, BuildingDlg::OnChoiceFacade )
END_EVENT_TABLE()

BuildingDlg::BuildingDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	BuildingDlgBase( parent, id, title, position, size, style )
{
	m_pSA = NULL;

	m_bSetting = false;
	m_bEdges = false;

	// we only need to do this once
	SetupValidators();

	AdjustDialogForEdges();
}

void BuildingDlg::Setup(vtStructureArray *pSA, vtBuilding *bld)
{
	m_pSA = pSA;
	m_pBuilding = bld;
}

void BuildingDlg::SetupValidators()
{
	// In panel 1
	AddValidator(this, ID_STORIES, &m_iStories);
	AddNumValidator(this, ID_STORY_HEIGHT, &m_fStoryHeight);
	AddValidator(this, ID_MATERIAL1, &m_strMaterial1);
	AddValidator(this, ID_EDGE_SLOPES, &m_strEdgeSlopes);

	// In panel 3
	AddValidator(this, ID_MATERIAL2, &m_strMaterial2);
	AddNumValidator(this, ID_EDGE_SLOPE, &m_iEdgeSlope);
	AddValidator(this, ID_FEATURES, &m_strFeatures);
}

void BuildingDlg::AdjustDialogForEdges()
{
	if (m_bEdges)
	{
		m_panel1->Show(false);
		m_panel2->Show(true);
		m_panel3->Show(true);

		m_pColorBitmapControl = GetColorBitmap2();

		RefreshEdgesBox();
		m_pEdgeListBox->SetSelection(0);
		SetEdge(0);
	}
	else
	{
		m_panel1->Show(true);
		m_panel2->Show(false);
		m_panel3->Show(false);

		m_pColorBitmapControl = GetColorBitmap1();

		if (m_pSA)
			m_pSA->SetEditedEdge(NULL, 0, 0);
	}
	Layout();
	GetSizer()->Fit( this );
}

void BuildingDlg::EditColor()
{
	s_ColorData.SetChooseFull(true);

	// Set the existing color to the dialog
	RGBi rgb;
	if (m_bEdges)
		rgb = m_pEdge->m_Color;
	else
	{
		if (!m_pLevel->GetOverallEdgeColor(rgb))
			rgb = m_pLevel->GetEdge(0)->m_Color;
	}
	m_Color.Set(rgb.r, rgb.g, rgb.b);
	s_ColorData.SetColour(m_Color);

	EnableRendering(false);
	wxColourDialog dlg(this, &s_ColorData);
	if (dlg.ShowModal() == wxID_OK)
	{
		// Get the color from the dialog
		s_ColorData = dlg.GetColourData();
		m_Color = s_ColorData.GetColour();
		RGBi result(m_Color.Red(), m_Color.Green(), m_Color.Blue());

		// and apply it to the appropriate feature
		if (m_bEdges)
			m_pEdge->m_Color = result;
		else
		{
			// Handle roofs specially.
			int num_levels = m_pBuilding->NumLevels();
			if (m_iLevel == num_levels - 1)
			{
				// It's a roof.  Only set non-vertical edges.
				for (int i = 0; i < m_pLevel->NumEdges(); i++)
				{
					if (m_pLevel->GetEdge(i)->m_iSlope != 90)
						m_pLevel->GetEdge(i)->m_Color = result;
				}
			}
			else
			{
				m_pLevel->SetEdgeColor(result);

				if (m_iLevel == num_levels - 2)
				{
					// It's the level below the roof. Extend the color up to the
					// vertical edges of the roof above it.
					vtLevel *roof = m_pBuilding->GetLevel(num_levels - 1);
					for (int i = 0; i < m_pLevel->NumEdges() && i < roof->NumEdges(); i++)
					{
						if (roof->GetEdge(i)->m_iSlope == 90)
							roof->GetEdge(i)->m_Color = result;
					}
				}
			}
		}

		UpdateColorControl();
	}
	EnableRendering(true);
}

void BuildingDlg::OnSetMaterial( wxCommandEvent &event )
{
	int i, j;
	int iInitialSelection = -1;
	int iNumberofMaterials = GetGlobalMaterials()->size();

	wxString *matstring;
	if (m_bEdges)
		matstring = &m_strMaterial2;
	else
		matstring = &m_strMaterial1;

	*matstring = wxString(m_pLevel->GetOverallEdgeMaterial(), wxConvUTF8);

	wxString *pChoices = new wxString[iNumberofMaterials];

	int iShown = 0;
	for (i = 0; i < iNumberofMaterials; i++)
	{
		vtMaterialDescriptor *mat = GetGlobalMaterials()->at(i);

		// only show surface materials, not typed feature materials
		if (mat->GetMatType() > 0)
			continue;

		const vtString& MaterialName = mat->GetName();
		wxString matname(MaterialName, wxConvUTF8);

		// for multiple materials with the same name, only show them once
		bool bFound = false;
		for (j = 0; j < iShown; j++)
		{
			if (pChoices[j] == matname) bFound = true;
		}
		if (bFound)
			continue;

		pChoices[iShown] = matname;
		if (pChoices[iShown] == *matstring)
			iInitialSelection = iShown;
		iShown++;
	}

	wxSingleChoiceDialog dialog(this, _T("Choice"),
		_("Set Building Material for All Edges"), iShown, pChoices);

	if (iInitialSelection != -1)
		dialog.SetSelection(iInitialSelection);

	if (dialog.ShowModal() != wxID_OK)
		return;

	*matstring = pChoices[dialog.GetSelection()];

	delete[] pChoices;

	vtString matname = (const char *) matstring->mb_str(wxConvUTF8);
	const vtString *matname2 = GetGlobalMaterials()->FindName(matname);
	if (m_bEdges)
		m_pEdge->m_pMaterial = matname2;
	else
	{
		// Handle roofs specially.
		int num_levels = m_pBuilding->NumLevels();
		if (m_iLevel == num_levels - 1)
		{
			// It's a roof.  Only set non-vertical edges.
			for (int i = 0; i < m_pLevel->NumEdges(); i++)
			{
				if (m_pLevel->GetEdge(i)->m_iSlope != 90)
					m_pLevel->GetEdge(i)->m_pMaterial = matname2;
			}
		}
		else
		{
			m_pLevel->SetEdgeMaterial(matname2);

			if (m_iLevel == num_levels - 2)
			{
				// It's the level below the roof. Extend the color up to the
				// vertical edges of the roof above it.
				vtLevel *roof = m_pBuilding->GetLevel(num_levels - 1);
				for (int i = 0; i < m_pLevel->NumEdges() && i < roof->NumEdges(); i++)
				{
					if (roof->GetEdge(i)->m_iSlope == 90)
						roof->GetEdge(i)->m_pMaterial = matname2;
				}
			}
		}
	}

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
	Modified();
}

void BuildingDlg::HighlightSelectedLevel()
{
	m_pLevelListBox->SetSelection(m_iLevel);
}

void BuildingDlg::HighlightSelectedEdge()
{
	m_pEdgeListBox->SetSelection(m_iEdge);
}

// WDR: handler implementations for BuildingDlg

void BuildingDlg::OnChoiceFacade( wxCommandEvent &event )
{
	wxString Facade = GetFacadeChoice()->GetStringSelection();

	if (Facade == _("(None)"))
		Facade = wxT("");

	if (0 != m_pEdge->m_Facade.Compare(Facade.mb_str(wxConvUTF8)))
	{
		m_pEdge->m_Facade = Facade.mb_str(wxConvUTF8);
		Modified();
	}
}

void BuildingDlg::OnFeatDoor( wxCommandEvent &event )
{
	vtEdgeFeature f;
	f.m_code = WFC_DOOR;
	f.m_width = -1.0f;
	f.m_vf1 = 0.0f;
	f.m_vf2 = 0.8f;
	m_pEdge->m_Features.push_back(f);
	UpdateFeatures();
	Modified();
}

void BuildingDlg::OnFeatWindow( wxCommandEvent &event )
{
	vtEdgeFeature f;
	f.m_code = WFC_WINDOW;
	f.m_width = -1.0f;
	f.m_vf1 = 0.3f;
	f.m_vf2 = 0.8f;
	m_pEdge->m_Features.push_back(f);
	UpdateFeatures();
	Modified();
}

void BuildingDlg::OnFeatWall( wxCommandEvent &event )
{
	vtEdgeFeature f;
	f.m_code = WFC_WALL;
	f.m_width = -1.0f;
	f.m_vf1 = 0;
	f.m_vf2 = 1;
	m_pEdge->m_Features.push_back(f);
	UpdateFeatures();
	Modified();
}

void BuildingDlg::OnFeatClear( wxCommandEvent &event )
{
	m_pEdge->m_Features.clear();
	UpdateFeatures();
	Modified();
}

void BuildingDlg::OnEdgeSlope( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	m_pEdge->m_iSlope = m_iEdgeSlope;
	Modified();
}

void BuildingDlg::OnLevelUp( wxCommandEvent &event )
{
	if (m_iLevel > 0)
	{
		m_pBuilding->SwapLevels(m_iLevel-1, m_iLevel);
		RefreshLevelsBox();
		SetLevel(m_iLevel-1);
		HighlightSelectedLevel();
		Modified();
	}
}

void BuildingDlg::OnLevelDown( wxCommandEvent &event )
{
	if (m_iLevel < (int) m_pBuilding->NumLevels() - 1)
	{
		m_pBuilding->SwapLevels(m_iLevel, m_iLevel+1);
		RefreshLevelsBox();
		SetLevel(m_iLevel+1);
		HighlightSelectedLevel();
		Modified();
	}
}

void BuildingDlg::CopyCurrentLevel()
{
	vtLevel *pNewLevel = new vtLevel(*m_pLevel);
	m_pBuilding->InsertLevel(m_iLevel, pNewLevel);
	RefreshLevelsBox();
	SetLevel(m_iLevel);
	HighlightSelectedLevel();
	Modified();
}

void BuildingDlg::OnLevelCopy( wxCommandEvent &event )
{
	CopyCurrentLevel();
}

void BuildingDlg::DeleteCurrentLevel()
{
	m_pBuilding->DeleteLevel(m_iLevel);
	if (m_iLevel == m_pBuilding->NumLevels())
		m_iLevel--;
	RefreshLevelsBox();
	SetLevel(m_iLevel);
	HighlightSelectedLevel();
	Modified();
}

void BuildingDlg::OnLevelDelete( wxCommandEvent &event )
{
	DeleteCurrentLevel();
}

void BuildingDlg::OnStoryHeight( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	m_pLevel->m_fStoryHeight = m_fStoryHeight;
	m_pBuilding->DetermineLocalFootprints();
	Modified();
}

void BuildingDlg::OnSpinStories( wxSpinEvent &event )
{
	if (m_bSetting)
		return;
	TransferDataFromWindow();
	m_pLevel->m_iStories = m_iStories;
	m_pBuilding->DetermineLocalFootprints();
	RefreshLevelsBox();
	HighlightSelectedLevel();
	Modified();
}

void BuildingDlg::OnColor1( wxCommandEvent &event )
{
	EditColor();
	Modified();
}

void BuildingDlg::OnOK( wxCommandEvent &event )
{
	m_pSA->SetEditedEdge(NULL, 0, 0);
	event.Skip();
}

void BuildingDlg::OnCloseWindow(wxCloseEvent& event)
{
	m_pSA->SetEditedEdge(NULL, 0, 0);
	event.Skip();
}

void BuildingDlg::SetupControls()
{
	m_pLevelListBox = GetLevelCtrl();
	m_pEdgeListBox = GetEdgeCtrl();

	RefreshLevelsBox();

	if (NULL != GetFacadeChoice())
	{
		GetFacadeChoice()->Clear();
		for (uint i = 0; i < vtGetDataPath().size(); i++)
		{
			vtString Directory = vtGetDataPath()[i] + "Facade";
			for (dir_iter it((const char *)Directory); it != dir_iter(); ++it)
			{
				if (it.is_hidden() || it.is_directory())
					continue;

				std::string name1 = it.filename();
				vtString name = name1.c_str();

				// Only look for ".jpg|.png|.bmp|.dds" image files
				vtString ext = GetExtension(name, false);
				if (!ext.CompareNoCase(".jpg") ||
					!ext.CompareNoCase(".png") ||
					!ext.CompareNoCase(".bmp") ||
					!ext.CompareNoCase(".dds"))
				{
					GetFacadeChoice()->Append(wxString(name, wxConvUTF8));
				}
			}
		}
	}

	SetLevel(m_iLevel);
	HighlightSelectedLevel();
}

void BuildingDlg::OnInitDialog(wxInitDialogEvent& event)
{
	m_iLevel = 0;
	m_pLevel = NULL;
	m_pEdge = NULL;
	m_fStoryHeight = 0.0f;
	m_strMaterial1 = _T("");
	m_strMaterial2 = _T("");

	SetupControls();

	m_bSetting = true;
	wxDialog::OnInitDialog(event);  // calls TransferDataToWindow()
	m_bSetting = false;
}

void BuildingDlg::RefreshLevelsBox()
{
	m_pLevelListBox->Clear();
	wxString str;
	int i, levels = m_pBuilding->NumLevels();
	for (i = 0; i < levels; i++)
	{
		vtLevel *pLev = m_pBuilding->GetLevel(i);
		str.Printf(_T("%d"), i);
		RoofType rt = pLev->GuessRoofType();
		if (rt == ROOF_FLAT)
			str += _(" (flat roof)");
		else if (rt == ROOF_SHED)
			str += _(" (shed roof)");
		else if (rt == ROOF_GABLE)
			str += _(" (gable roof)");
		else if (rt == ROOF_HIP)
			str += _(" (hip roof)");
		else
		{
			wxString str2;
			str2.Printf(_(" (stories : %d)"), pLev->m_iStories);
			str += str2;
		}
		m_pLevelListBox->Append(str);
	}
	GetSizer()->Fit(this);
}

void BuildingDlg::RefreshEdgesBox()
{
	m_pEdgeListBox->Clear();
	wxString str;
	int i, edges = m_pLevel->NumEdges();
	for (i = 0; i < edges; i++)
	{
		str.Printf(_T("%d"), i);
		m_pEdgeListBox->Append(str);
	}
	GetSizer()->Fit(this);
}


/////////////////////////////////////////////////////////////

void BuildingDlg::OnEdge( wxCommandEvent &event )
{
	int sel = m_pEdgeListBox->GetSelection();

	SetEdge(sel);
}

void BuildingDlg::SetEdge(int iEdge)
{
	TransferDataFromWindow();
	m_iEdge = iEdge;
	m_pEdge = m_pLevel->GetEdge(iEdge);
	m_iEdgeSlope = m_pEdge->m_iSlope;

	UpdateMaterialControl();	// material
	UpdateColorControl();		// color
	UpdateSlopes();				// slopes
	UpdateFeatures();			// features
	UpdateFacade();

	m_pSA->SetEditedEdge(m_pBuilding, m_iLevel, m_iEdge);
}

void BuildingDlg::OnLevel( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	int sel = m_pLevelListBox->GetSelection();
	if (sel != -1)
		SetLevel(sel);
}

void BuildingDlg::SetLevel(int iLev)
{
	m_iLevel = iLev;
	m_pLevel = m_pBuilding->GetLevel(iLev);

	m_iStories = m_pLevel->m_iStories;
	m_fStoryHeight = m_pLevel->m_fStoryHeight;

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;

	if (m_bEdges)
	{
		RefreshEdgesBox();
		SetEdge(0);
		HighlightSelectedEdge();
	}
	else
	{
		UpdateMaterialControl();	// material
		UpdateColorControl();		// color
		UpdateSlopes();				// slopes
	}

	// enable up/down
	GetLevelUp()->Enable(m_iLevel > 0);
	GetLevelDown()->Enable(m_iLevel < (int) m_pBuilding->NumLevels()-1);
	GetLevelDel()->Enable(m_pBuilding->NumLevels() > 1);
	GetLevelCopy()->Enable(true);
}

void BuildingDlg::UpdateMaterialControl()
{
	// In the case of a whole level, attempt to show the most
	//  commonly occuring material.
	if (m_bEdges == false)
		m_strMaterial1 = wxString(m_pLevel->GetOverallEdgeMaterial(), wxConvUTF8);
	else
	{
		if (m_pEdge->m_pMaterial == NULL)
			m_strMaterial2 = _("Unknown");
		else
			m_strMaterial2 = wxString(*m_pEdge->m_pMaterial, wxConvUTF8);
	}
}

void BuildingDlg::UpdateColorControl()
{
	RGBi color;

	// In the case of a whole level, attempt to show the most
	//  commonly occuring material.
	if (m_bEdges == false)
	{
		int edges = m_pLevel->NumEdges();
		if (edges == 0)
		{
			// badly formed building; don't crash
			FillWithColor(m_pColorBitmapControl, RGBi(0,0,0));
			return;
		}
		// color
		bool uniform = m_pLevel->GetOverallEdgeColor(color);
		if (uniform)
			m_Color.Set(color.r, color.g, color.b);
		else
			m_Color.Set(0, 0, 0);

		// Draw the image with vertical bands corresponding to all the edges
		int xsize = 32, ysize = 18;
		float factor = (float) edges / (float) xsize * .9999f;
		wxImage pImage(xsize, ysize);
		int i, j;
		for (i = 0; i < xsize; i++)
		{
			vtEdge *pEdge = m_pLevel->GetEdge((int) (i * factor));
			RGBi col = pEdge->m_Color;
			for (j = 0; j < ysize; j++)
			{
				pImage.SetRGB(i, j, col.r, col.g, col.b);
			}
		}
		wxBitmap *pBitmap = new wxBitmap(pImage);
		m_pColorBitmapControl->SetBitmap(*pBitmap);
		delete pBitmap;
	}
	else
	{
		// Case of a single edge, much simpler.
		FillWithColorSize(m_pColorBitmapControl, 32, 18, m_pEdge->m_Color);
	}
}

bool BuildingDlg::AskForTypeAndSlope(bool bAll, RoofType &eType, int &iSlope)
{
	wxString choices[5];
	choices[0] = _("Flat (all edges 0(degree sign))");
	choices[1] = _("Shed");
	choices[2] = _("Gable");
	choices[3] = _("Hip");
	choices[4] = _("Vertical (all edges 90(degree sign))");
	int num = 5;
	if (!bAll)
		num = 4;

	wxSingleChoiceDialog dialog(this, _T("Choice"),
		_("Please indicate edge slopes"), num, (const wxString *)choices);

	dialog.SetSelection(0);

	if (dialog.ShowModal() != wxID_OK)
		return false;

	int sel = dialog.GetSelection();
	if (sel == 1 || sel == 2 || sel == 3)
	{
		// need slope
		if (sel == 1) iSlope = 4;
		if (sel == 2) iSlope = 15;
		if (sel == 3) iSlope = 15;
		iSlope = wxGetNumberFromUser(_("Sloped edges"), _("Degrees"),
			_("Slope"), iSlope, 0, 90);
		if (iSlope == -1)
			return false;
	}
	eType = (enum RoofType) sel;
	return true;
}

void BuildingDlg::OnSetRoofType( wxCommandEvent &event )
{
	RoofType type;
	int slope;
	if (!AskForTypeAndSlope(false, type, slope))
		return;

	m_pBuilding->SetRoofType(type, slope);
	UpdateSlopes();
	RefreshLevelsBox();
	HighlightSelectedLevel();
	Modified();
}

void BuildingDlg::OnSetEdgeSlopes( wxCommandEvent &event )
{
	RoofType type;
	int slope;
	if (!AskForTypeAndSlope(true, type, slope))
		return;

	if (type != ROOF_UNKNOWN)
		m_pLevel->SetRoofType(type, slope);
	else
	{
		int i, edges = m_pLevel->NumEdges();
		for (i = 0; i < edges; i++)
			m_pLevel->GetEdge(i)->m_iSlope = 90;
	}
	m_fStoryHeight = m_pLevel->m_fStoryHeight;
	UpdateSlopes();
	RefreshLevelsBox();
	HighlightSelectedLevel();
	Modified();
}

void BuildingDlg::UpdateSlopes()
{
	if (m_bEdges == false)
	{
		wxString str;
		m_strEdgeSlopes = _T("");
		int i, edges = m_pLevel->NumEdges();
		for (i = 0; i < edges; i++)
		{
			vtEdge *edge = m_pLevel->GetEdge(i);
			str.Printf(_T(" %d"), edge->m_iSlope);
			m_strEdgeSlopes += str;
		}
	}
	else
	{
		// nothing special, m_iEdgeSlope is passed with Transfer
	}
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void BuildingDlg::UpdateFeatures()
{
	m_strFeatures = _T("");
	int feats = m_pEdge->NumFeatures();
	for (int i = 0; i < feats; i++)
	{
		vtEdgeFeature &feat = m_pEdge->m_Features[i];
		if (feat.m_code == WFC_WALL)
			m_strFeatures += _T("[W] ");
		else if (feat.m_code == WFC_GAP)
			m_strFeatures += _T("[Gap] ");
		else if (feat.m_code == WFC_POST)
			m_strFeatures += _T("[Post] ");
		else if (feat.m_code == WFC_WINDOW)
			m_strFeatures += _T("[Win] ");
		else if (feat.m_code == WFC_DOOR)
			m_strFeatures += _T("[Door] ");
	}
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void BuildingDlg::UpdateFacade()
{
	wxChoice *pFacadeChoice = GetFacadeChoice();
	wxString Facade = wxString(m_pEdge->m_Facade, wxConvUTF8);

	if (wxT("") == Facade)
		Facade = _("(None)");

	if (pFacadeChoice->FindString(Facade) == wxNOT_FOUND)
		pFacadeChoice->Append(Facade);
	pFacadeChoice->SetStringSelection(Facade);
}

void BuildingDlg::OnEdges( wxCommandEvent &event )
{
	TransferDataFromWindow();
	m_bEdges = !m_bEdges;

	AdjustDialogForEdges();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}


#include <vtdata/Vocab.h>

void BuildingDlg::OnCharHook( wxKeyEvent &event )
{
	int foo = event.GetKeyCode();
	if (foo != WXK_F2)
	{
		event.Skip();
		return;
	}

	// Test text input
	wxString str = wxGetTextFromUser(_T("Test Message"), _T("Test Caption"), _T(""), this);
	TestParser par;
	par.ParseInput(str.mb_str(wxConvUTF8));

	MatchToken *tok;

	// Pattern sentence: "Select floor/level <number>"
	SentenceMatch sen1;
	sen1.AddLiteral(true, "select");
	sen1.AddLiteral(true, "floor", "level");
	tok = sen1.AddToken(true, NUMBER);

	if (par.Matches(sen1))
	{
		SetLevel( (int) tok->number);
		HighlightSelectedLevel();
	}

	// Pattern sentence: "Select [the] <counter> floor/level"
	SentenceMatch sen2;
	sen2.AddLiteral(true, "select");
	sen2.AddLiteral(false, "the");
	tok = sen2.AddToken(true, COUNTER);
	sen2.AddLiteral(true, "floor", "level");

	if (par.Matches(sen2))
	{
		SetLevel( tok->counter);
		HighlightSelectedLevel();
	}

	// Pattern sentence: "Delete/remove [it]"
	SentenceMatch sen3;
	sen3.AddLiteral(true, "delete", "remove");
	sen3.AddLiteral(false, "it");

	if (par.Matches(sen3))
		DeleteCurrentLevel();

	// Pattern sentence: "Copy/duplicate [it]"
	SentenceMatch sen4;
	sen4.AddLiteral(true, "copy", "duplicate");
	sen4.AddLiteral(false, "it");

	if (par.Matches(sen4))
		CopyCurrentLevel();
}

