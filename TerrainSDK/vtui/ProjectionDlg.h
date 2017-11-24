//
// Name:		ProjectionDlg.h
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __ProjectionDlg_H__
#define __ProjectionDlg_H__

#include "vtui_UI.h"
#include "vtui/AutoDialog.h"
#include "vtdata/vtCRS.h"

enum ProjType
{
	PT_ALBERS,
	PT_GEO,
	PT_HOM,		// Hotine Oblique Mercator
	PT_KROVAK,
	PT_LAEA,	// Lambert Azimuthal Equal-Area
	PT_LCC,		// Lambert Conformal Conic
	PT_LCC1SP,	// Lambert Conformal Conic with 1 Standard Parallel
	PT_NZMG,
	PT_OS,
	PT_PS,
	PT_SINUS,
	PT_STEREO,
	PT_MERC,
	PT_TM,
	PT_UTM,
	PT_DYMAX,
	PT_HOMAC		// Hotine_Oblique_Mercator_Azimuth_Center
};

// WDR: class declarations

//----------------------------------------------------------------------------
// ProjectionDlg
//----------------------------------------------------------------------------

class ProjectionDlg: public ProjectionDlgBase
{
public:
	// constructors and destructors
	ProjectionDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void SetCRS(const vtCRS &crs);
	void GetCRS(vtCRS &crs);
	void SetGeoRefPoint(const DPoint2 &p) { m_GeoRefPoint = p; }

protected:
	// WDR: method declarations for ProjectionDlg
	wxListCtrl* GetProjparam()  { return (wxListCtrl*) FindWindow( ID_PROJPARAM ); }
	wxChoice* GetHorizchoice()  { return (wxChoice*) FindWindow( ID_HORUNITS ); }
	wxChoice* GetZonechoice()  { return (wxChoice*) FindWindow( ID_ZONE ); }
	wxChoice* GetDatumchoice()  { return (wxChoice*) FindWindow( ID_DATUM ); }
	wxChoice* GetProjchoice()  { return (wxChoice*) FindWindow( ID_PROJ ); }
	wxTextCtrl* GetEllipsoid()  { return (wxTextCtrl*) FindWindow( ID_ELLIPSOID ); }

	void SetUIFromProjection();
	void SetProjectionUI(ProjType type);
	void UpdateControlStatus();
	void DisplayProjectionSpecificParams();
	void AskStatePlane();
	void RefreshDatums();
	void UpdateDatumStatus();
	void UpdateEllipsoid();

private:
	// WDR: member variable declarations for ProjectionDlg
	wxListCtrl  *m_pParamCtrl;
	wxChoice	*m_pZoneCtrl;
	wxChoice	*m_pHorizCtrl;
	wxChoice	*m_pDatumCtrl;
	wxChoice	*m_pProjCtrl;

	vtCRS	m_crs;
	ProjType		m_eProj;
	int		m_iDatum;
	int		m_iZone;
	int		m_iProj;
	int		m_iUnits;
	bool	m_bShowAllDatums;
	bool	m_bInitializedUI;
	DPoint2 m_GeoRefPoint;

private:
	// WDR: handler declarations for ProjectionDlg
	void OnProjSave( wxCommandEvent &event );
	void OnProjLoad( wxCommandEvent &event );
	void OnDatum( wxCommandEvent &event );
	void OnItemRightClick( wxListEvent &event );
	void OnHorizUnits( wxCommandEvent &event );
	void OnZone( wxCommandEvent &event );
	void OnSetStatePlane( wxCommandEvent &event );
	void OnSetEPSG( wxCommandEvent &event );
	void OnProjChoice( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);
	void OnShowAllDatums( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __ProjectionDlg_H__

