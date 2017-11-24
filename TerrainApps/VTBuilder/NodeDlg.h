//
// Name: NodeDlg.h
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __NodeDlg_H__
#define __NodeDlg_H__

#include "VTBuilder_UI.h"

#include "RoadMapEdit.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// NodeDlg
//----------------------------------------------------------------------------

class NodeDlg: public NodeDlgBase
{
public:
	// constructors and destructors
	NodeDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void SetNode(NodeEdit *pSingleLink, vtRoadLayer *pLayer);
	void ApplyVisualToNode(NodeEdit *pNode, VisualIntersectionType vitype);

	// WDR: method declarations for NodeDlg
	wxListBox* GetBehavior()  { return (wxListBox*) FindWindow( ID_BEHAVIOR ); }
	wxListBox* GetLinkNum()  { return (wxListBox*) FindWindow( ID_ROADNUM ); }
	wxListBox* GetIntType()  { return (wxListBox*) FindWindow( ID_INTTYPE ); }

private:
	// WDR: member variable declarations for NodeDlg
	NodeEdit *m_pNode;
	vtRoadLayer *m_pLayer;

private:
	// WDR: handler declarations for NodeDlg
	void OnBehavior( wxCommandEvent &event );
	void OnLinkNum( wxCommandEvent &event );
	void OnIntType( wxCommandEvent &event );
	void OnOK( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);
	void OnDraw(wxDC &dc);

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __NodeDlg_H__

