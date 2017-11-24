//
// Name: LinearStructDlg.cpp
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "LinearStructDlg.h"
#include "Helper.h"		// for AddFilenamesToChoice
#include "vtdata/DataPath.h"

#define VALUE_MIN  0.2f
#define VALUE_MAX  10.2f
#define BOTTOM_MIN  -4.0f
#define BOTTOM_MAX  9.0f
#define WIDTH_MIN   0.0f
#define WIDTH_MAX   10.0f
#define SLOPE_MIN   15
#define SLOPE_MAX   90

// WDR: class implementations

//----------------------------------------------------------------------------
// LinearStructureDlg
//----------------------------------------------------------------------------

// WDR: event table for LinearStructureDlg

BEGIN_EVENT_TABLE(LinearStructureDlg, LinearStructDlgBase)
	EVT_INIT_DIALOG (LinearStructureDlg::OnInitDialog)
	EVT_CHOICE( ID_LINEAR_STRUCTURE_STYLE, LinearStructureDlg::OnStyle )
	EVT_TEXT( ID_POST_HEIGHT_EDIT, LinearStructureDlg::OnTextEdit )
	EVT_TEXT( ID_POST_SPACING_EDIT, LinearStructureDlg::OnTextEdit )
	EVT_TEXT( ID_POST_SIZE_EDIT, LinearStructureDlg::OnTextEdit )
	EVT_TEXT( ID_CONN_WIDTH_EDIT, LinearStructureDlg::OnTextEdit )
	EVT_TEXT( ID_CONN_TOP_EDIT, LinearStructureDlg::OnTextEdit )
	EVT_TEXT( ID_CONN_BOTTOM_EDIT, LinearStructureDlg::OnTextEdit )
	EVT_SLIDER( ID_POST_HEIGHT_SLIDER, LinearStructureDlg::OnSlider )
	EVT_SLIDER( ID_POST_SPACING_SLIDER, LinearStructureDlg::OnSlider )
	EVT_SLIDER( ID_POST_SIZE_SLIDER, LinearStructureDlg::OnSlider )
	EVT_SLIDER( ID_CONN_WIDTH_SLIDER, LinearStructureDlg::OnSlider )
	EVT_SLIDER( ID_CONN_TOP_SLIDER, LinearStructureDlg::OnSlider )
	EVT_SLIDER( ID_CONN_BOTTOM_SLIDER, LinearStructureDlg::OnSlider )
	EVT_CHOICE( ID_POST_TYPE, LinearStructureDlg::OnPostType )
	EVT_CHOICE( ID_CONN_TYPE, LinearStructureDlg::OnConnType )
	EVT_CHOICE( ID_CONN_MATERIAL, LinearStructureDlg::OnConnMaterial )
	EVT_CHOICE( ID_CHOICE_EXTENSION, LinearStructureDlg::OnExtension )
	EVT_TEXT( ID_SLOPE, LinearStructureDlg::OnTextEdit )
	EVT_SLIDER( ID_SLOPE_SLIDER, LinearStructureDlg::OnSlider )
	EVT_CHECKBOX( ID_CONSTANT_TOP, LinearStructureDlg::OnConstantTop )
	EVT_CHOICE( ID_CHOICE_PROFILE, LinearStructureDlg::OnChoiceProfile )
	EVT_BUTTON( ID_PROFILE_EDIT, LinearStructureDlg::OnProfileEdit )
END_EVENT_TABLE()

LinearStructureDlg::LinearStructureDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	LinearStructDlgBase( parent, id, title, position, size, style )
{
	m_bSetting = false;

	m_pStructureMaterials = NULL;
	m_iStyle = 0;
	m_param.Defaults();
	m_pProfileEditDlg = NULL;

	AddValidator(this, ID_LINEAR_STRUCTURE_STYLE, &m_iStyle);

	AddValidator(this, ID_POST_HEIGHT_SLIDER, &m_iPostHeight);
	AddValidator(this, ID_POST_SPACING_SLIDER, &m_iPostSpacing);
	AddValidator(this, ID_POST_SIZE_SLIDER, &m_iPostSize);

	AddValidator(this, ID_CONN_TYPE, &m_param.m_iConnectType);
	AddValidator(this, ID_CONN_WIDTH_SLIDER, &m_iConnWidth);
	AddValidator(this, ID_CONN_TOP_SLIDER, &m_iConnTop);
	AddValidator(this, ID_CONN_BOTTOM_SLIDER, &m_iConnBottom);
	AddValidator(this, ID_SLOPE_SLIDER, &m_iSlope);
	AddValidator(this, ID_CONSTANT_TOP, &m_param.m_bConstantTop);

	AddNumValidator(this, ID_POST_HEIGHT_EDIT, &m_param.m_fPostHeight, 2);
	AddNumValidator(this, ID_POST_SPACING_EDIT, &m_param.m_fPostSpacing, 2);
	AddNumValidator(this, ID_POST_SIZE_EDIT, &m_param.m_fPostWidth, 2);

	AddNumValidator(this, ID_CONN_WIDTH_EDIT, &m_param.m_fConnectWidth, 2);
	AddNumValidator(this, ID_CONN_TOP_EDIT, &m_param.m_fConnectTop, 2);
	AddNumValidator(this, ID_CONN_BOTTOM_EDIT, &m_param.m_fConnectBottom, 2);
	AddNumValidator(this, ID_SLOPE, &m_param.m_iConnectSlope);

	UpdateChoices();

	GetSizer()->SetSizeHints(this);
}

void LinearStructureDlg::SetStructureMaterials(const vtMaterialDescriptorArray *desc)
{
	m_pStructureMaterials = desc;
	UpdateConnectChoices();
	UpdateTypes();
}

void LinearStructureDlg::UpdateChoices()
{
	// NB -- these must match the FS_ enum in order
	GetStyle()->Clear();
	GetStyle()->Append(_("Wooden posts, wire"));
	GetStyle()->Append(_("Metal posts, wire"));
	GetStyle()->Append(_("Metal posts, hog wire"));
	GetStyle()->Append(_("Metal poles, chain-link"));
	GetStyle()->Append(_("Security fence"));
	GetStyle()->Append(_("Dry-stone wall"));
	GetStyle()->Append(_("Stone wall"));
	GetStyle()->Append(_("Privet hedge"));
	GetStyle()->Append(_("Berm"));
	GetStyle()->Append(_("Railing (Pipe)"));
	GetStyle()->Append(_("Railing (Wire)"));
	GetStyle()->Append(_("Railing (EU)"));
	GetStyle()->Append(_("(custom)"));

	GetConnType()->Clear();
	GetConnType()->Append(_T("none"));
	GetConnType()->Append(_T("wire"));
	GetConnType()->Append(_T("simple"));
	GetConnType()->Append(_T("profile"));

	UpdateConnectChoices();
}

void LinearStructureDlg::AddConnectStringsFromDescriptors(const vtMaterialDescriptorArray *mats)
{
	for (uint i = 0; i < mats->size(); i++)
	{
		vtMaterialDescriptor *desc = mats->at(i);

		// Type 0 means a surface type. This avoids fence posts, windows,
		//  and other things inappropriate for a linear connector.
		if (desc->GetMatType() == 0)
		{
			wxString str(desc->GetName(), wxConvUTF8);
			GetConnMat()->Append(str);
		}
	}
}

void LinearStructureDlg::UpdateConnectChoices()
{
	GetConnMat()->Clear();
	GetConnMat()->Append(_T("none"));

	if (m_pStructureMaterials)
		AddConnectStringsFromDescriptors(m_pStructureMaterials);
	else
	{
		// just show some well-known materials
		GetConnMat()->Append(_T("chain-link"));
		GetConnMat()->Append(_T("drystone"));
		GetConnMat()->Append(_T("stone"));
		GetConnMat()->Append(_T("privet"));
		GetConnMat()->Append(_T("grass"));
		GetConnMat()->Append(_T("railing_wire"));
		GetConnMat()->Append(_T("railing_eu"));
		GetConnMat()->Append(_T("railing_pipe"));
	}

	// Also update post materials available
	GetPostType()->Clear();
	GetPostType()->Append(_T("none"));
	if (m_pStructureMaterials)
	{
		for (uint i = 0; i < m_pStructureMaterials->size(); i++)
		{
			vtMaterialDescriptor *desc = m_pStructureMaterials->at(i);

			// Type 3 means a post material.
			if (desc->GetMatType() == 3)
				GetPostType()->Append(wxString(desc->GetName(), wxConvUTF8));
		}
	}
}

void LinearStructureDlg::UpdateProfiles()
{
	wxChoice *cc = GetChoiceProfile();
	cc->Clear();
	for (uint i = 0; i < vtGetDataPath().size(); i++)
	{
		// fill the "profiles" control with available profile files
		AddFilenamesToChoice(cc, vtGetDataPath()[i] + "BuildingData", "*.shp");

		wxString ws(m_param.m_ConnectProfile, wxConvUTF8);
		int sel = cc->FindString(ws);
		if (sel != -1)
			cc->SetSelection(sel);
	}
}

void LinearStructureDlg::SetOptions(const vtLinearParams &param)
{
	m_param = param;

	ValuesToSliders();
	UpdateTypes();
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
	UpdateEnabling();

	OnSetOptions(m_param);
}

void LinearStructureDlg::UpdateTypes()
{
	wxString ws(m_param.m_PostType, wxConvUTF8);
	GetPostType()->SetStringSelection(ws);

	ws = wxString(m_param.m_ConnectMaterial, wxConvUTF8);
	GetConnMat()->SetStringSelection(ws);

	vtString str = m_param.m_PostExtension;
	if (str == "none")
		GetExtension()->SetSelection(0);
	if (str == "left")
		GetExtension()->SetSelection(1);
	if (str == "right")
		GetExtension()->SetSelection(2);
	if (str == "double")
		GetExtension()->SetSelection(3);
}

void LinearStructureDlg::UpdateEnabling()
{
	bool bHasPosts = (m_param.m_PostType != "none");
	bool bHasConn = (m_param.m_iConnectType != 0);
	bool bSimpleConn = (m_param.m_iConnectType == 2);
	bool bCustomProf = (m_param.m_iConnectType == 3);

	GetPostSpacingEdit()->Enable(bHasPosts);
	GetPostSpacingSlider()->Enable(bHasPosts);
	GetPostHeightEdit()->Enable(bHasPosts);
	GetPostHeightSlider()->Enable(bHasPosts);
	GetPostSizeEdit()->Enable(bHasPosts);
	GetPostSizeSlider()->Enable(bHasPosts);

	GetConnMat()->Enable(bSimpleConn || bCustomProf);
	GetConnWidthEdit()->Enable(bSimpleConn);
	GetConnWidthSlider()->Enable(bSimpleConn);
	GetConnTopEdit()->Enable(bSimpleConn);
	GetConnTopSlider()->Enable(bSimpleConn);
	GetConnBottomEdit()->Enable(bSimpleConn);
	GetConnBottomSlider()->Enable(bSimpleConn);
	GetSlope()->Enable(bSimpleConn);
	GetSlopeSlider()->Enable(bSimpleConn);
	GetConstantTop()->Enable(bHasConn);

	GetChoiceProfile()->Enable(bCustomProf);
	GetProfileEdit()->Enable(bCustomProf);
}

void LinearStructureDlg::GuessStyle()
{
	// Go through the known styles and see if any match the current params
	vtLinearParams p;
	for (int i = 0; i < FS_TOTAL; i++)
	{
		p.ApplyStyle((vtLinearStyle) i);
		if (m_param == p)
		{
			m_iStyle = i;
			return;
		}
	}
	m_iStyle = FS_TOTAL;	// Custom
}

// WDR: handler implementations for LinearStructureDlg

void LinearStructureDlg::OnInitDialog(wxInitDialogEvent& event)
{
	ValuesToSliders();
	GuessStyle();
	UpdateTypes();
	UpdateProfiles();
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
	UpdateEnabling();
}

void LinearStructureDlg::OnProfileEdit( wxCommandEvent &event )
{
	if (!m_pProfileEditDlg)
		m_pProfileEditDlg = new ProfileEditDlg(this, -1,
		 _("Edit Linear Structure Profile"),	wxDefaultPosition, wxDefaultSize,
		 wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

	if (m_param.m_ConnectProfile != "")
	{
		vtString look_in = "BuildingData/";
		vtString path = FindFileOnPaths(vtGetDataPath(), look_in + m_param.m_ConnectProfile);
		if (path != "")
			m_pProfileEditDlg->SetFilename(path);
	}
	m_pProfileEditDlg->ShowModal();

	UpdateProfiles();

	// Profile may have changed, even if the parameters didn't, so pass true.
	OnSetOptions(m_param, true);
}

void LinearStructureDlg::OnChoiceProfile( wxCommandEvent &event )
{
	wxString ws = GetChoiceProfile()->GetStringSelection();
	m_param.m_ConnectProfile = ws.mb_str(wxConvUTF8);
	OnSetOptions(m_param);
}

void LinearStructureDlg::OnConstantTop( wxCommandEvent &event )
{
	TransferDataFromWindow();
	OnSetOptions(m_param);
}

void LinearStructureDlg::OnConnType( wxCommandEvent &event )
{
	if (m_bSetting) return;

	TransferDataFromWindow();
	UpdateEnabling();

	GuessStyle();
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
	OnSetOptions(m_param);
}

void LinearStructureDlg::OnConnMaterial( wxCommandEvent &event )
{
	if (m_bSetting) return;

	wxString ws = GetConnMat()->GetStringSelection();
	m_param.m_ConnectMaterial = ws.mb_str(wxConvUTF8);
	UpdateEnabling();

	GuessStyle();
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
	OnSetOptions(m_param);
}

void LinearStructureDlg::OnPostType( wxCommandEvent &event )
{
	if (m_bSetting) return;

	wxString ws = GetPostType()->GetStringSelection();
	m_param.m_PostType = ws.mb_str(wxConvUTF8);
	UpdateEnabling();

	GuessStyle();
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
	OnSetOptions(m_param);
}

void LinearStructureDlg::OnExtension( wxCommandEvent &event )
{
	if (m_bSetting) return;

	int val = GetExtension()->GetSelection();
	if (val == 0)
		m_param.m_PostExtension = "none";
	if (val == 1)
		m_param.m_PostExtension = "left";
	if (val == 2)
		m_param.m_PostExtension = "right";
	if (val == 3)
		m_param.m_PostExtension = "double";

	GuessStyle();
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
	OnSetOptions(m_param);
}

void LinearStructureDlg::OnStyle( wxCommandEvent &event )
{
	if (m_bSetting) return;

	TransferDataFromWindow();
	vtLinearStyle style = (vtLinearStyle) m_iStyle;
	if (style != FS_TOTAL)
	{
		m_param.ApplyStyle(style);
		UpdateTypes();
		ValuesToSliders();
		m_bSetting = true;
		TransferDataToWindow();
		m_bSetting = false;
		OnSetOptions(m_param);
		UpdateEnabling();
	}
}

void LinearStructureDlg::OnSlider( wxCommandEvent &event )
{
	if (m_bSetting) return;

	TransferDataFromWindow();
	int id = event.GetId();
	SlidersToValues(id);
	GuessStyle();

	// Update text controls
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
	OnSetOptions(m_param);
}

void LinearStructureDlg::OnTextEdit( wxCommandEvent &event )
{
	if (m_bSetting) return;

	TransferDataFromWindow();
	m_param.m_fPostDepth = m_param.m_fPostWidth;
	ValuesToSliders();
	GuessStyle();

	// Update sliders
	m_bSetting = true;
	GetConnBottomSlider()->GetValidator()->TransferToWindow();
	GetConnTopSlider()->GetValidator()->TransferToWindow();
	GetConnWidthSlider()->GetValidator()->TransferToWindow();
	GetPostSizeSlider()->GetValidator()->TransferToWindow();
	GetPostHeightSlider()->GetValidator()->TransferToWindow();
	GetPostSpacingSlider()->GetValidator()->TransferToWindow();
	m_bSetting = false;

	OnSetOptions(m_param);
}

void LinearStructureDlg::SlidersToValues(int which)
{
	switch (which)
	{
	case ID_POST_SPACING_SLIDER:
		m_param.m_fPostSpacing   = VALUE_MIN + m_iPostSpacing * (VALUE_MAX - VALUE_MIN) / 100.0f;
		break;
	case ID_POST_HEIGHT_SLIDER:
		m_param.m_fPostHeight   = VALUE_MIN + m_iPostHeight *  (VALUE_MAX - VALUE_MIN) / 100.0f;
		break;
	case ID_POST_SIZE_SLIDER:
		m_param.m_fPostWidth	 = VALUE_MIN + m_iPostSize *	(VALUE_MAX - VALUE_MIN) / 100.0f;
		m_param.m_fPostDepth	 = VALUE_MIN + m_iPostSize *	(VALUE_MAX - VALUE_MIN) / 100.0f;
		break;
	case ID_CONN_TOP_SLIDER:
		m_param.m_fConnectTop   = VALUE_MIN + m_iConnTop *   (VALUE_MAX - VALUE_MIN) / 100.0f;
		break;
	case ID_CONN_BOTTOM_SLIDER:
		m_param.m_fConnectBottom = BOTTOM_MIN + m_iConnBottom * (BOTTOM_MAX - BOTTOM_MIN) / 100.0f;
		break;
	case ID_CONN_WIDTH_SLIDER:
		m_param.m_fConnectWidth  = WIDTH_MIN + m_iConnWidth *   (WIDTH_MAX - WIDTH_MIN) / 100.0f;
		break;
	case ID_SLOPE_SLIDER:
		m_param.m_iConnectSlope  = SLOPE_MIN + (int) (m_iSlope * (SLOPE_MAX - SLOPE_MIN) / 100.0f);
		break;
	}
}

void LinearStructureDlg::ValuesToSliders()
{
	m_iPostHeight =  (int) ((m_param.m_fPostHeight - VALUE_MIN) /   (VALUE_MAX - VALUE_MIN) * 100.0f);
	m_iPostSpacing = (int) ((m_param.m_fPostSpacing - VALUE_MIN) /   (VALUE_MAX - VALUE_MIN) * 100.0f);
	m_iPostSize =	(int) ((m_param.m_fPostWidth - VALUE_MIN) /	 (VALUE_MAX - VALUE_MIN) * 100.0f);
	m_iConnTop =	 (int) ((m_param.m_fConnectTop - VALUE_MIN) /   (VALUE_MAX - VALUE_MIN) * 100.0f);
	m_iConnBottom =  (int) ((m_param.m_fConnectBottom - BOTTOM_MIN) / (BOTTOM_MAX - BOTTOM_MIN) * 100.0f);
	m_iConnWidth =   (int) ((m_param.m_fConnectWidth - WIDTH_MIN) /  (WIDTH_MAX - WIDTH_MIN) * 100.0f);
	m_iSlope =	   (int) ((m_param.m_iConnectSlope - SLOPE_MIN) /  (float)(SLOPE_MAX - SLOPE_MIN) * 100.0f);
}

