//
// Name:		LinearStructDlg.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __LinearStruct_H__
#define __LinearStruct_H__

#include "vtui_UI.h"
#include "vtui/AutoDialog.h"
#include "vtdata/Fence.h"
#include "vtdata/MaterialDescriptor.h"
#include "ProfileEditDlg.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// LinearStructureDlg
//----------------------------------------------------------------------------

class LinearStructureDlg: public LinearStructDlgBase
{
public:
	// constructors and destructors
	LinearStructureDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void SetStructureMaterials(const vtMaterialDescriptorArray *desc);

	void GuessStyle();
	void OnInitDialog(wxInitDialogEvent& event);
	void SlidersToValues(int which);
	void ValuesToSliders();
	void UpdateTypes();
	void UpdateEnabling();
	void SetOptions(const vtLinearParams &param);
	void UpdateChoices();
	void UpdateConnectChoices();
	void UpdateProfiles();

	// Override this with desired result
	virtual void OnSetOptions(const vtLinearParams &param, bool bProfileChanged = false) {};

	// WDR: method declarations for LinearStructureDlg
	wxButton* GetProfileEdit()  { return (wxButton*) FindWindow( ID_PROFILE_EDIT ); }
	wxChoice* GetChoiceProfile()  { return (wxChoice*) FindWindow( ID_CHOICE_PROFILE ); }
	wxCheckBox* GetConstantTop()  { return (wxCheckBox*) FindWindow( ID_CONSTANT_TOP ); }
	wxSlider* GetSlopeSlider()  { return (wxSlider*) FindWindow( ID_SLOPE_SLIDER ); }
	wxTextCtrl* GetSlope()  { return (wxTextCtrl*) FindWindow( ID_SLOPE ); }
	wxTextCtrl* GetConnBottomEdit()  { return (wxTextCtrl*) FindWindow( ID_CONN_BOTTOM_EDIT ); }
	wxTextCtrl* GetConnTopEdit()  { return (wxTextCtrl*) FindWindow( ID_CONN_TOP_EDIT ); }
	wxTextCtrl* GetConnWidthEdit()  { return (wxTextCtrl*) FindWindow( ID_CONN_WIDTH_EDIT ); }
	wxTextCtrl* GetPostSizeEdit()  { return (wxTextCtrl*) FindWindow( ID_POST_SIZE_EDIT ); }
	wxTextCtrl* GetPostHeightEdit()  { return (wxTextCtrl*) FindWindow( ID_POST_HEIGHT_EDIT ); }
	wxTextCtrl* GetPostSpacingEdit()  { return (wxTextCtrl*) FindWindow( ID_POST_SPACING_EDIT ); }

	wxSlider* GetConnBottomSlider()  { return (wxSlider*) FindWindow( ID_CONN_BOTTOM_SLIDER ); }
	wxSlider* GetConnTopSlider()  { return (wxSlider*) FindWindow( ID_CONN_TOP_SLIDER ); }
	wxSlider* GetConnWidthSlider()  { return (wxSlider*) FindWindow( ID_CONN_WIDTH_SLIDER ); }
	wxSlider* GetPostSizeSlider()  { return (wxSlider*) FindWindow( ID_POST_SIZE_SLIDER ); }
	wxSlider* GetPostHeightSlider()  { return (wxSlider*) FindWindow( ID_POST_HEIGHT_SLIDER ); }
	wxSlider* GetPostSpacingSlider()  { return (wxSlider*) FindWindow( ID_POST_SPACING_SLIDER ); }

	wxChoice* GetExtension()  { return (wxChoice*) FindWindow( ID_CHOICE_EXTENSION ); }
	wxChoice* GetConnMat()  { return (wxChoice*) FindWindow( ID_CONN_MATERIAL ); }
	wxChoice* GetConnType()  { return (wxChoice*) FindWindow( ID_CONN_TYPE ); }
	wxChoice* GetPostType()  { return (wxChoice*) FindWindow( ID_POST_TYPE ); }
	wxChoice* GetStyle()  { return (wxChoice*) FindWindow( ID_LINEAR_STRUCTURE_STYLE ); }

protected:
	void AddConnectStringsFromDescriptors(const vtMaterialDescriptorArray *mats);

	// WDR: member variable declarations for LinearStructureDlg
	int  m_iStyle;
	int  m_iPostHeight;
	int  m_iPostSpacing;
	int  m_iPostSize;
	int  m_iConnWidth;
	int  m_iConnTop;
	int  m_iConnBottom;
	int  m_iSlope;

	bool m_bSetting;

	vtLinearParams m_param;
	const vtMaterialDescriptorArray *m_pStructureMaterials;
	ProfileEditDlg *m_pProfileEditDlg;

private:
	// WDR: handler declarations for LinearStructureDlg
	void OnProfileEdit( wxCommandEvent &event );
	void OnChoiceProfile( wxCommandEvent &event );
	void OnConstantTop( wxCommandEvent &event );
	void OnTextEdit( wxCommandEvent &event );
	void OnConnMaterial( wxCommandEvent &event );
	void OnConnType( wxCommandEvent &event );
	void OnPostType( wxCommandEvent &event );
	void OnExtension( wxCommandEvent &event );
	void OnSlider( wxCommandEvent &event );
	void OnStyle( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __LinearStruct_H__

