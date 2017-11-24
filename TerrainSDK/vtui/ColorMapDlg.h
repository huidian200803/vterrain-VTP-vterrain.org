//
// Name: ColorMapDlg.h
//
// Copyright (c) 2004-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __ColorMapDlg_H__
#define __ColorMapDlg_H__

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "wx/imaglist.h"

#if wxCHECK_VERSION(2, 9, 0)
  #include <wx/colourdata.h>
#else
  // The older include
  #include <wx/cmndata.h>
#endif

#include "vtui_UI.h"
#include "AutoDialog.h"
#include "vtdata/HeightField.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// ColorMapDlg
//----------------------------------------------------------------------------

class ColorMapDlg: public ColorMapDlgBase
{
public:
	// constructors and destructors
	ColorMapDlg( wxWindow *parent, wxWindowID id,
		const wxString& title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for ColorMapDlg
	wxButton* GetSave()  { return (wxButton*) FindWindow( ID_SAVE_CMAP ); }
	wxTextCtrl* GetHeightToAdd()  { return (wxTextCtrl*) FindWindow( ID_HEIGHT_TO_ADD ); }
	wxButton* GetDeleteColor()  { return (wxButton*) FindWindow( ID_DELETE_ELEVATION ); }
	wxButton* GetChangeColor()  { return (wxButton*) FindWindow( ID_CHANGE_COLOR ); }
	wxListCtrl* GetList()  { return (wxListCtrl*) FindWindow( ID_COLORLIST ); }
	wxTextCtrl* GetCmapFile()  { return (wxTextCtrl*) FindWindow( ID_CMAP_FILE ); }

	void SetFile(const char *fname);
	void UpdateItems();

private:
	// WDR: member variable declarations for ColorMapDlg
	ColorMap	m_cmap;
	wxString	m_strFile;
	float		m_fHeight;
	bool		m_bRelative, m_bBlend;
	long		m_iItem;
	wxColourData m_ColorData;
	wxColour	m_Color;
	wxImageList m_imlist;

private:
	// WDR: handler declarations for ColorMapDlg
	void OnLoad( wxCommandEvent &event );
	void OnSave( wxCommandEvent &event );
	void OnSaveAs( wxCommandEvent &event );
	void OnAdd( wxCommandEvent &event );
	void OnDeleteColor( wxCommandEvent &event );
	void OnChangeColor( wxCommandEvent &event );
	void OnItemSelected( wxListEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

	void UpdateEnabling();
	bool AskColor(RGBi &rgb);

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __ColorMapDlg_H__

