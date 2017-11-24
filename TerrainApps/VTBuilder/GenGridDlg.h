//
// Name: GenGridDlg.h
//
// Copyright (c) 2007-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __GenGridDlg_H__
#define __GenGridDlg_H__

#include "VTBuilder_UI.h"
#include "vtdata/MathTypes.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// GenGridDlg
//----------------------------------------------------------------------------

class GenGridDlg: public GenGridDlgBase
{
public:
	// constructors and destructors
	GenGridDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		bool bIsGeo, const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for GenGridDlg
	wxTextCtrl* GetSizeY()  { return (wxTextCtrl*) FindWindow( ID_SIZEY ); }
	wxTextCtrl* GetSizeX()  { return (wxTextCtrl*) FindWindow( ID_SIZEX ); }
	wxTextCtrl* GetSpacingY()  { return (wxTextCtrl*) FindWindow( ID_SPACINGY ); }
	wxTextCtrl* GetSpacingX()  { return (wxTextCtrl*) FindWindow( ID_SPACINGX ); }

	void RecomputeSize();

	double  m_fSpacingX;
	double  m_fSpacingY;
	IPoint2 m_Size;
	double  m_fAreaX;
	double  m_fAreaY;
	float m_fDistanceCutoff;

private:
	// WDR: member variable declarations for GenGridDlg
	bool m_bSetting;

private:
	// WDR: handler declarations for GenGridDlg
	void OnSizeXY( wxCommandEvent &event );
	void OnSpacingXY( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};




#endif
