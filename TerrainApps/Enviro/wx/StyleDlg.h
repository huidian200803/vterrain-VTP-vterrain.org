//
// Name: StyleDlg.h
//
// Copyright (c) 2004-2011 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#ifndef __StyleDlg_H__
#define __StyleDlg_H__

#if wxCHECK_VERSION(2, 9, 0)
  #include <wx/colourdata.h>
#else
  // The older include
  #include <wx/cmndata.h>
#endif

#include "EnviroUI.h"
#include "vtui/AutoDialog.h"
#include "vtdata/Features.h"

class vtTagArray;

// WDR: class declarations

//----------------------------------------------------------------------------
// StyleDlg
//----------------------------------------------------------------------------

class StyleDlg: public StyleDlgBase
{
public:
	// constructors and destructors
	StyleDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// This dialog can operate on a featureset in memory.  If you do not give
	//  it a featureset, it will assume the layer's filename is a SHP on disk
	//  and use that instead.
	void SetFeatureSet(const vtFeatureSet *pSet)
	{
		m_pFeatureSet = pSet;
	}

	void SetOptions(const vtTagArray &layer_tags);
	void GetOptions(vtTagArray &layer_tags);
	wxNotebook *GetNotebook() { return m_notebook; }

private:
	// WDR: method declarations for StyleDlg
	wxTextCtrl* GetFeatureType()  { return (wxTextCtrl*) FindWindow( ID_FEATURE_TYPE ); }
	// Object Geometry
	wxRadioButton* GetRadio1()  { return (wxRadioButton*) FindWindow( ID_RADIO1 ); }
	wxBitmapButton* GetObjectGeomColor()  { return (wxBitmapButton*) FindWindow( ID_OBJECT_GEOM_COLOR ); }
	wxRadioButton* GetRadioUseObjectColorField()  { return (wxRadioButton*) FindWindow( ID_RADIO_USE_OBJECT_COLOR_FIELD ); }
	wxChoice* GetObjectColorField()  { return (wxChoice*) FindWindow( ID_OBJECT_COLOR_FIELD ); }
	wxTextCtrl* GetObjectGeomSize()  { return (wxTextCtrl*) FindWindow( ID_OBJECT_GEOM_SIZE ); }
	wxTextCtrl* GetObjectGeomHeight()  { return (wxTextCtrl*) FindWindow( ID_OBJECT_GEOM_HEIGHT ); }
	// Line Geometry
	wxRadioButton* GetRadio2()  { return (wxRadioButton*) FindWindow( ID_RADIO2 ); }
	wxBitmapButton* GetLineGeomColor()  { return (wxBitmapButton*) FindWindow( ID_LINE_GEOM_COLOR ); }
	wxRadioButton* GetRadioUseLineColorField()  { return (wxRadioButton*) FindWindow( ID_RADIO_USE_LINE_COLOR_FIELD ); }
	wxChoice* GetLineColorField()  { return (wxChoice*) FindWindow( ID_LINE_COLOR_FIELD ); }
	wxTextCtrl* GetLineGeomHeight()  { return (wxTextCtrl*) FindWindow( ID_LINE_GEOM_HEIGHT ); }
	wxTextCtrl* GetLineWidth()  { return (wxTextCtrl*) FindWindow( ID_LINE_WIDTH ); }
	wxCheckBox* GetTessellate()  { return (wxCheckBox*) FindWindow( ID_TESSELLATE ); }
	// Text Labels
	wxRadioButton* GetRadio3()  { return (wxRadioButton*) FindWindow( ID_RADIO3 ); }
	wxBitmapButton* GetTextColor()  { return (wxBitmapButton*) FindWindow( ID_TEXT_COLOR ); }
	wxRadioButton* GetRadioUseTextColorField()  { return (wxRadioButton*) FindWindow( ID_RADIO_USE_TEXT_COLOR_FIELD ); }
	wxChoice* GetTextColorField()  { return (wxChoice*) FindWindow( ID_TEXT_COLOR_FIELD ); }
	wxChoice* GetTextField()  { return (wxChoice*) FindWindow( ID_TEXT_FIELD ); }
	wxTextCtrl* GetLabelHeight()  { return (wxTextCtrl*) FindWindow( ID_LABEL_HEIGHT ); }
	wxTextCtrl* GetLabelSize()  { return (wxTextCtrl*) FindWindow( ID_LABEL_SIZE ); }
	wxTextCtrl* GetFont()  { return (wxTextCtrl*) FindWindow( ID_FONT ); }

private:
	// WDR: member variable declarations for StyleDlg
	const vtFeatureSet *m_pFeatureSet;
	vtString m_strResolved;

	OGRwkbGeometryType m_type;
	wxString m_strFeatureType;

	// Object Geometry
	bool m_bObjectGeometry;
	RGBi m_ObjectGeomColor;
	bool m_bRadioUseObjectColorField;
	int m_iObjectColorField;
	float m_fObjectGeomHeight;
	float m_fObjectGeomSize;

	// Line Geometry
	bool m_bLineGeometry;
	RGBi m_LineGeomColor;
	bool m_bRadioUseLineColorField;
	int m_iLineColorField;
	float m_fLineGeomHeight;
	float m_fLineWidth;
	bool m_bTessellate;

	// Text Labels
	bool m_bTextLabels;
	RGBi m_LabelColor;
	bool m_bRadioUseTextColorField;
	int m_iTextColorField;
	int m_iTextField;
	float m_fLabelHeight;
	float m_fLabelSize;
	wxString m_strFont;
	bool m_bLabelOutline;

	vtFeatureSetPoint2D m_DummyFeatures;
	wxColourData m_ColourData;
	wxColour m_Colour;

private:
	void RefreshFields();
	void UpdateEnabling();
	void UpdateColorButtons();
	RGBi AskColor(const RGBi &input);

	// WDR: handler declarations for StyleDlg
	void OnRadio( wxCommandEvent &event );
	void OnLabelColor( wxCommandEvent &event );
	void OnLineGeomColor( wxCommandEvent &event );
	void OnObjectGeomColor( wxCommandEvent &event );
	void OnCheck( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __StyleDlg_H__

