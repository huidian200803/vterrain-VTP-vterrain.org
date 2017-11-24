//
// Name: DistanceDlg.h
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __DistanceDlg_H__
#define __DistanceDlg_H__

#include "vtui_UI.h"
#include "vtui/AutoDialog.h"
#include "vtdata/vtCRS.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// DistanceDlg
//----------------------------------------------------------------------------

class DistanceDlg: public DistanceDlgBase
{
public:
	// constructors and destructors
	DistanceDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	~DistanceDlg();

	// WDR: method declarations for DistanceDlg
	wxButton* GetDistToolClear()  { return (wxButton*) FindWindow( ID_DIST_TOOL_CLEAR ); }
	wxRadioButton* GetRadioPath()  { return (wxRadioButton*) FindWindow( ID_RADIO_PATH ); }
	wxRadioButton* GetRadioLine()  { return (wxRadioButton*) FindWindow( ID_RADIO_LINE ); }
	wxTextCtrl* GetVertical()  { return (wxTextCtrl*) FindWindow( ID_VERTICAL ); }
	wxTextCtrl* GetGroundDist()  { return (wxTextCtrl*) FindWindow( ID_GROUND_DIST ); }
	wxChoice* GetUnits1()  { return (wxChoice*) FindWindow( ID_UNITS1 ); }
	wxChoice* GetUnits2()  { return (wxChoice*) FindWindow( ID_UNITS2 ); }
	wxChoice* GetUnits3()  { return (wxChoice*) FindWindow( ID_UNITS3 ); }
	wxChoice* GetUnits4()  { return (wxChoice*) FindWindow( ID_UNITS4 ); }
	wxChoice* GetUnits5()  { return (wxChoice*) FindWindow( ID_UNITS5 ); }
	wxTextCtrl* GetMapOffset()  { return (wxTextCtrl*) FindWindow( ID_MAP_OFFSET ); }
	wxTextCtrl* GetMapDist()  { return (wxTextCtrl*) FindWindow( ID_MAP_DIST ); }
	wxTextCtrl* GetGeodDist()  { return (wxTextCtrl*) FindWindow( ID_GEOD_DIST ); }

	void SetCRS(const vtCRS &crs);
	void SetPoints(const DPoint2 &p1, const DPoint2 &p2, bool bUpdate);
	void SetPath(const DLine2 &path, bool bUpdate);
	void GetPoints(DPoint2 &p1, DPoint2 &p2);
	void SetGroundAndVertical(float fGround, float fVertical, bool bUpdate);
	void UpdateAvailableUnits();
	void ShowValues();
	void Zero();

	virtual void OnMode(bool bPath) = 0;
	virtual void SetPathToBase(const DLine2 &path) = 0;
	virtual void Reset() = 0;

private:
	// WDR: member variable declarations for DistanceDlg
	vtCRS	m_crs;
	int	 m_iUnits1;
	int	 m_iUnits2;
	int	 m_iUnits3;
	int	 m_iUnits4;
	int	 m_iUnits5;
	DPoint2 m_p1, m_p2;
	DLine2 m_path;
	float   m_fGround, m_fVertical;
	bool m_bPathMode;
	OCTransform *m_pTransformToGeo;

private:
	// WDR: handler declarations for DistanceDlg
	void OnLoadPath( wxCommandEvent &event );
	void OnClear( wxCommandEvent &event );
	void OnRadioPath( wxCommandEvent &event );
	void OnRadioLine( wxCommandEvent &event );
	void OnUnits( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __DistanceDlg_H__

