//
// Name:		StyleDlg.cpp
//
// Copyright (c) 2004-2011 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "StyleDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include <wx/colordlg.h>
#include <wx/fontdlg.h>
#include <wx/fontenum.h>

#include "StyleDlg.h"
#include "vtlib/vtlib.h"
#include "vtdata/DataPath.h"
#include "vtdata/Features.h"
#include "vtdata/FilePath.h"
#include "vtui/Helper.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// StyleDlg
//----------------------------------------------------------------------------

// WDR: event table for StyleDlg

BEGIN_EVENT_TABLE(StyleDlg,StyleDlgBase)
	EVT_INIT_DIALOG (StyleDlg::OnInitDialog)

	// Object Geometry
	EVT_CHECKBOX( ID_ENABLE_OBJECT_GEOM, StyleDlg::OnCheck )
	EVT_BUTTON( ID_OBJECT_GEOM_COLOR, StyleDlg::OnObjectGeomColor )

	// Line Geometry
	EVT_CHECKBOX( ID_ENABLE_LINE_GEOM, StyleDlg::OnCheck )
	EVT_BUTTON( ID_LINE_GEOM_COLOR, StyleDlg::OnLineGeomColor )

	// Text Labels
	EVT_CHECKBOX( ID_ENABLE_TEXT_LABELS, StyleDlg::OnCheck )
	EVT_BUTTON( ID_TEXT_COLOR, StyleDlg::OnLabelColor )

	// Texture Overlay
	EVT_RADIOBUTTON( ID_RADIO1, StyleDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO_USE_OBJECT_COLOR_FIELD, StyleDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO2, StyleDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO_USE_LINE_COLOR_FIELD, StyleDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO3, StyleDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO_USE_TEXT_COLOR_FIELD, StyleDlg::OnRadio )
END_EVENT_TABLE()

StyleDlg::StyleDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	StyleDlgBase( parent, id, title, position, size, style )
{
	// including the children of the notebook
	wxNotebook *notebook = (wxNotebook*) FindWindow( ID_NOTEBOOK );
	notebook->SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);

	m_pFeatureSet = NULL;
	AddValidator(this, ID_FEATURE_TYPE, &m_strFeatureType);

	// Work around wxFormDesigner's lack of support for limiting to smallest size
	GetSizer()->SetSizeHints(this);

	// Work around wxFormDesigner setting radio buttons to true when they aren't
	GetRadio2()->SetValue(true);

	// Object Geometry
	m_bObjectGeometry = false;
	m_ObjectGeomColor.Set(255, 255, 255);
	m_bRadioUseObjectColorField = false;
	m_iObjectColorField = 0;
	m_fObjectGeomHeight = 1.0f;
	m_fObjectGeomSize = 1.0f;

	AddValidator(this, ID_ENABLE_OBJECT_GEOM, &m_bObjectGeometry);
	AddValidator(this, ID_RADIO_USE_OBJECT_COLOR_FIELD, &m_bRadioUseObjectColorField);
	AddValidator(this, ID_OBJECT_COLOR_FIELD, &m_iObjectColorField);
	AddNumValidator(this, ID_OBJECT_GEOM_HEIGHT, &m_fObjectGeomHeight);
	AddNumValidator(this, ID_OBJECT_GEOM_SIZE, &m_fObjectGeomSize);

	// Line Geometry
	m_bLineGeometry = false;
	m_LineGeomColor.Set(255, 255, 255);
	m_bRadioUseLineColorField = false;
	m_iLineColorField = 0;
	m_fLineGeomHeight = 1.0f;
	m_fLineWidth = 1.0f;
	m_bTessellate = false;

	AddValidator(this, ID_ENABLE_LINE_GEOM, &m_bLineGeometry);
	AddValidator(this, ID_RADIO_USE_LINE_COLOR_FIELD, &m_bRadioUseLineColorField);
	AddValidator(this, ID_LINE_COLOR_FIELD, &m_iLineColorField);
	AddNumValidator(this, ID_LINE_GEOM_HEIGHT, &m_fLineGeomHeight);
	AddNumValidator(this, ID_LINE_WIDTH, &m_fLineWidth);
	AddValidator(this, ID_TESSELLATE, &m_bTessellate);

	// Text Labels
	m_bTextLabels = false;
	m_LabelColor.Set(255,255,255);
	m_bRadioUseTextColorField = false;
	m_iTextColorField = 0;
	m_iTextField = 0;
	m_fLabelHeight = 0.0f;
	m_fLabelSize = 0.0f;
	m_bLabelOutline = true;

	AddValidator(this, ID_ENABLE_TEXT_LABELS, &m_bTextLabels);
	AddValidator(this, ID_RADIO_USE_TEXT_COLOR_FIELD, &m_bRadioUseTextColorField);
	AddValidator(this, ID_TEXT_COLOR_FIELD, &m_iTextColorField);
	AddValidator(this, ID_TEXT_FIELD, &m_iTextField);
	AddNumValidator(this, ID_LABEL_HEIGHT, &m_fLabelHeight);
	AddNumValidator(this, ID_LABEL_SIZE, &m_fLabelSize);
	AddValidator(this, ID_FONT, &m_strFont);
	AddValidator(this, ID_OUTLINE, &m_bLabelOutline);
}

//
// Set this dialog's controls from a tagarray.
//
void StyleDlg::SetOptions(const vtTagArray &layer_tags)
{
	if (m_pFeatureSet)
	{
		m_type = m_pFeatureSet->GetGeomType();
	}
	else
	{
		// without a featureset, we need the actual file location
		vtString strFilename = layer_tags.GetValueString("Filename");
		m_strResolved = FindFileOnPaths(vtGetDataPath(), strFilename);

		if (m_strResolved == "")
		{
			// for historical reasons, also look in a "PointData" folder on the path
			vtString path = "PointData/";
			m_strResolved = FindFileOnPaths(vtGetDataPath(), path + strFilename);
		}

		m_type = GetFeatureGeomType(m_strResolved);

		// try to load field list (there may or may not be a DBF)
		m_DummyFeatures.LoadFieldInfoFromDBF(m_strResolved);

		m_pFeatureSet = &m_DummyFeatures;
	}

	// Object Geometry
	m_bObjectGeometry = layer_tags.GetValueBool("ObjectGeometry");
	if (!layer_tags.GetValueRGBi("ObjectGeomColor", m_ObjectGeomColor))
		m_ObjectGeomColor.Set(255,255,255);
	m_bRadioUseObjectColorField = layer_tags.GetValueInt("ObjectColorFieldIndex", m_iObjectColorField);
	if (!m_bRadioUseObjectColorField)
		m_iObjectColorField = -1;
	if (!layer_tags.GetValueFloat("ObjectGeomHeight", m_fObjectGeomHeight))
		m_fObjectGeomHeight = 1;
	if (!layer_tags.GetValueFloat("ObjectGeomSize", m_fObjectGeomSize))
		m_fObjectGeomSize = 1;

	// Line Geometry
	m_bLineGeometry = layer_tags.GetValueBool("LineGeometry");
	if (!layer_tags.GetValueRGBi("LineGeomColor", m_LineGeomColor))
		m_LineGeomColor.Set(255,255,255);
	m_bRadioUseLineColorField = layer_tags.GetValueInt("LineColorFieldIndex", m_iLineColorField);
	if (!m_bRadioUseLineColorField)
		m_iLineColorField = -1;
	if (!layer_tags.GetValueFloat("LineGeomHeight", m_fLineGeomHeight))
		m_fLineGeomHeight = 1;
	if (!layer_tags.GetValueFloat("LineWidth", m_fLineWidth))
		m_fLineWidth = 1;
	m_bTessellate = layer_tags.GetValueBool("Tessellate");

	// Text Labels
	m_bTextLabels = layer_tags.GetValueBool("Labels");
	if (!layer_tags.GetValueRGBi("LabelColor", m_LabelColor))
		m_LabelColor.Set(255,255,255);
	m_bRadioUseTextColorField = layer_tags.GetValueInt("TextColorFieldIndex", m_iTextColorField);
	if (!m_bRadioUseTextColorField)
		m_iTextColorField = -1;
	if (!layer_tags.GetValueInt("TextFieldIndex", m_iTextField))
		m_iTextField = -1;

	if (!layer_tags.GetValueFloat("LabelHeight", m_fLabelHeight))
		m_fLabelHeight = 0;
	if (!layer_tags.GetValueFloat("LabelSize", m_fLabelSize))
		m_fLabelSize = 20;

	vtString font;
	if (layer_tags.GetValueString("Font", font))
		m_strFont = wxString(font, wxConvUTF8);
	else
		m_strFont = _T("Arial.ttf");
	m_bLabelOutline = layer_tags.GetValueBool("LabelOutline");
}

//
// Copy this dialog's controls to a tagarray.
//
void StyleDlg::GetOptions(vtTagArray &layer_tags)
{
	// Object Geometry
	layer_tags.SetValueBool("ObjectGeometry", m_bObjectGeometry, true);
	if (m_bObjectGeometry)
	{
		layer_tags.SetValueRGBi("ObjectGeomColor", m_ObjectGeomColor, true);
		if (m_bRadioUseObjectColorField)
			layer_tags.SetValueInt("ObjectColorFieldIndex", m_iObjectColorField, true);
		else
			layer_tags.RemoveTag("ObjectColorFieldIndex");
		if (!GeometryTypeIs3D(m_type))
			layer_tags.SetValueFloat("ObjectGeomHeight", m_fObjectGeomHeight);
		else
			layer_tags.RemoveTag("ObjectGeomHeight");
		layer_tags.SetValueFloat("ObjectGeomSize", m_fObjectGeomSize);
	}
	else
	{
		layer_tags.RemoveTag("ObjectGeomColor");
		layer_tags.RemoveTag("ObjectColorFieldIndex");
		layer_tags.RemoveTag("ObjectGeomHeight");
		layer_tags.RemoveTag("ObjectGeomSize");
	}

	// Line Geometry
	layer_tags.SetValueBool("LineGeometry", m_bLineGeometry, true);
	if (m_bLineGeometry)
	{
		layer_tags.SetValueRGBi("LineGeomColor", m_LineGeomColor, true);
		if (m_bRadioUseLineColorField)
			layer_tags.SetValueInt("LineColorFieldIndex", m_iLineColorField, true);
		else
			layer_tags.RemoveTag("LineColorFieldIndex");
		if (!GeometryTypeIs3D(m_type))
			layer_tags.SetValueFloat("LineGeomHeight", m_fLineGeomHeight);
		layer_tags.SetValueFloat("LineWidth", m_fLineWidth);
		layer_tags.SetValueBool("Tessellate", m_bTessellate);
	}
	else
	{
		layer_tags.RemoveTag("LineGeomColor");
		layer_tags.RemoveTag("LineColorFieldIndex");
		layer_tags.RemoveTag("LineGeomHeight");
		layer_tags.RemoveTag("LineWidth");
		layer_tags.RemoveTag("Tessellate");
	}

	// Text Labels
	layer_tags.SetValueBool("Labels", m_bTextLabels, true);
	if (m_bTextLabels)
	{
		layer_tags.SetValueRGBi("LabelColor", m_LabelColor, true);
		if (m_bRadioUseTextColorField)
			layer_tags.SetValueInt("TextColorFieldIndex", m_iTextColorField, true);
		else
			layer_tags.RemoveTag("TextColorFieldIndex");
		layer_tags.SetValueInt("TextFieldIndex", m_iTextField, true);
		layer_tags.SetValueFloat("LabelHeight", m_fLabelHeight, true);
		layer_tags.SetValueFloat("LabelSize", m_fLabelSize, true);
		layer_tags.SetValueString("Font", (const char *) m_strFont.mb_str(wxConvUTF8), true);
		layer_tags.SetValueBool("LabelOutline", m_bLabelOutline, true);
	}
	else
	{
		layer_tags.RemoveTag("LabelColor");
		layer_tags.RemoveTag("TextColorFieldIndex");
		layer_tags.RemoveTag("TextFieldIndex");
		layer_tags.RemoveTag("LabelHeight");
		layer_tags.RemoveTag("LabelSize");
		layer_tags.RemoveTag("Font");
		layer_tags.RemoveTag("LabelOutline");
	}
}

void Bound(int &val, int num)
{
	if (val < 0) val = 0;
	if (val > num-1) val = num-1;
}

void StyleDlg::RefreshFields()
{
	GetObjectColorField()->Clear();
	GetLineColorField()->Clear();
	GetTextColorField()->Clear();
	GetTextField()->Clear();

	if (!m_pFeatureSet)
		return;

	m_strFeatureType = wxString(OGRGeometryTypeToName(m_type), wxConvUTF8);
	int i, num = m_pFeatureSet->NumFields();
	for (i = 0; i < num; i++)
	{
		const Field *field = m_pFeatureSet->GetField(i);
		wxString field_name(field->m_name, wxConvUTF8);

		GetObjectColorField()->Append(field_name);
		GetLineColorField()->Append(field_name);
		GetTextColorField()->Append(field_name);
		GetTextField()->Append(field_name);
	}
	if (num)
	{
		Bound(m_iObjectColorField, num);
		Bound(m_iLineColorField, num);
		Bound(m_iTextColorField, num);
		Bound(m_iTextField, num);
	}
}

void StyleDlg::UpdateEnabling()
{
	bool bIs3D = GeometryTypeIs3D(m_type);

	// Object Geometry
	GetRadio1()->Enable(m_bObjectGeometry);
	GetObjectGeomColor()->Enable(m_bObjectGeometry && !m_bRadioUseObjectColorField);
	GetRadioUseObjectColorField()->Enable(m_bObjectGeometry);
	GetObjectColorField()->Enable(m_bRadioUseObjectColorField);
	GetObjectGeomSize()->Enable(m_bObjectGeometry);
	GetObjectGeomHeight()->Enable(m_bObjectGeometry && !bIs3D);

	// Line Geometry
	GetRadio2()->Enable(m_bLineGeometry);
	GetLineGeomColor()->Enable(m_bLineGeometry && !m_bRadioUseLineColorField);
	GetRadioUseLineColorField()->Enable(m_bLineGeometry);
	GetLineColorField()->Enable(m_bRadioUseLineColorField);
	GetLineGeomHeight()->Enable(m_bLineGeometry && !bIs3D);
	GetLineWidth()->Enable(m_bLineGeometry);
	GetTessellate()->Enable(m_bLineGeometry && !bIs3D);

	// Text Labels
	GetRadio3()->Enable(m_bTextLabels);
	GetTextColor()->Enable(m_bTextLabels && !m_bRadioUseTextColorField);
	GetRadioUseTextColorField()->Enable(m_bTextLabels);
	GetTextColorField()->Enable(m_bRadioUseTextColorField);
	GetTextField()->Enable(m_bTextLabels);
	GetLabelHeight()->Enable(m_bTextLabels && !bIs3D);
	GetLabelSize()->Enable(m_bTextLabels);
	GetFont()->Enable(m_bTextLabels);
}

void StyleDlg::UpdateColorButtons()
{
	FillWithColor(GetObjectGeomColor(), m_ObjectGeomColor);
	FillWithColor(GetLineGeomColor(), m_LineGeomColor);
	FillWithColor(GetTextColor(), m_LabelColor);
}

RGBi StyleDlg::AskColor(const RGBi &input)
{
	m_Colour.Set(input.r, input.g, input.b);
	m_ColourData.SetChooseFull(true);
	m_ColourData.SetColour(m_Colour);

	wxColourDialog dlg(this, &m_ColourData);
	if (dlg.ShowModal() == wxID_OK)
	{
		m_ColourData = dlg.GetColourData();
		m_Colour = m_ColourData.GetColour();
		return RGBi(m_Colour.Red(), m_Colour.Green(), m_Colour.Blue());
	}
	else
		return RGBi(-1,-1,-1);
}

// WDR: handler implementations for StyleDlg

void StyleDlg::OnInitDialog(wxInitDialogEvent& event)
{
	RefreshFields();
	UpdateEnabling();
	UpdateColorButtons();
	wxDialog::OnInitDialog(event);
}

void StyleDlg::OnCheck( wxCommandEvent &event )
{
	TransferDataFromWindow();
	UpdateEnabling();
}

void StyleDlg::OnRadio( wxCommandEvent &event )
{
	TransferDataFromWindow();
	UpdateEnabling();
}

void StyleDlg::OnObjectGeomColor( wxCommandEvent &event )
{
	RGBi result = AskColor(m_ObjectGeomColor);
	if (result.r == -1)
		return;
	m_ObjectGeomColor = result;
	UpdateColorButtons();
}

void StyleDlg::OnLineGeomColor( wxCommandEvent &event )
{
	RGBi result = AskColor(m_LineGeomColor);
	if (result.r == -1)
		return;
	m_LineGeomColor = result;
	UpdateColorButtons();
}

void StyleDlg::OnLabelColor( wxCommandEvent &event )
{
	RGBi result = AskColor(m_LabelColor);
	if (result.r == -1)
		return;
	m_LabelColor = result;
	UpdateColorButtons();
}

