//
// Name: TParamsDlg.h
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __TParamsDlg_H__
#define __TParamsDlg_H__

#include "EnviroUI.h"
#include "vtui/AutoDialog.h"
#include "vtlib/core/TParams.h"
#include "vtdata/vtTime.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// TParamsDlg
//----------------------------------------------------------------------------

class TParamsDlg: public TParamsDlgBase
{
public:
	// constructors and destructors
	TParamsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	~TParamsDlg();

	void OnInitDialog(wxInitDialogEvent& event);

	bool TransferDataToWindow();
	bool TransferDataFromWindow();

	void SetParams(const TParams &Params);
	void GetParams(TParams &Params);
	void UpdateEnableState();
	void RefreshLocationFields();
	void UpdateTimeString();
	void UpdateColorControl();
	void DeleteItem(wxListBox *pBox);
	int FindLayerByFilename(const vtString &fname);
	void UpdateScenarioChoices();

	// main copy of the parameters
	TParams		m_Params;

	// overall name
	wxString	m_strTerrainName;

	// elevation
	bool		m_bGrid;
	int			m_iLodMethod;
	int			m_iTriCount;
	bool		m_bTin;
	bool		m_bTileset;
	bool		m_bExternal;
	wxString	m_strExternalData;
	wxString	m_strFilename;
	wxString	m_strFilenameTin;
	wxString	m_strFilenameTileset;
	float		m_fTextureLODFactor;
	wxString	m_strTextureTileset;
	bool		m_bTextureGradual;
	float		m_fVerticalExag;

	// navigation
	float		m_fMinHeight;
	int			m_iNavStyle;
	float		m_fNavSpeed;
	float		m_fDamping;
	wxString	m_strLocFile;
	int			m_iInitLocation;
	wxString	m_strInitLocation;
	float		m_fHither;
	bool		m_bAccel;
	bool		m_bAllowRoll;

	// LOD
	int		m_iVertCount;
	int		m_iTileCacheSize;
	bool	m_bTileThreading;

	// time
	bool	m_bTimeOn;
	vtTime  m_InitTime;
	float   m_fTimeSpeed;

	// culture
	bool		m_bRoads;
	wxString	m_strRoadFile;
	bool		m_bHwy;
	bool		m_bPaved;
	bool		m_bDirt;
	float		m_fRoadHeight;
	float		m_fRoadDistance;
	bool		m_bTexRoads;
	bool		m_bRoadCulture;

	// vegetation
	int		m_iVegDistance;
	bool    m_bTreesUseShaders;

	bool	m_bFog;
	float   m_fFogDistance;

	wxString m_strContent;

	std::vector<vtTagArray> m_Layers;
	vtStringArray m_AnimPaths;
	int		m_iStructDistance;
	bool	m_bStructureShadows;
	int		m_iShadowRez;
	float	m_fDarkness;
	bool	m_bShadowsDefaultOn;
	bool	m_bShadowsEveryFrame;
	bool	m_bLimitShadowArea;
	float	m_fShadowRadius;
	bool	m_bPagingStructures;
	int		m_iPagingStructureMax;
	float	m_fPagingStructureDist;

	// ephemeris
	bool		m_bSky;
	wxString	m_strSkyTexture;
	bool		m_bOceanPlane;
	float		m_fOceanPlaneLevel;
	bool		m_bWater;
	wxString	m_strFilenameWater;
	bool		m_bDepressOcean;
	float		m_fDepressOceanLevel;
	wxColor		m_BgColor;

//	bool	m_bVehicles;
//  float   m_fVehicleSize;
//  float   m_fVehicleSpeed;

	wxString m_strUtilFile;

	// HUD
	wxString m_strOverlayFile;
	int		m_iOverlayX;
	int		m_iOverlayY;
	bool	m_bOverview;
	bool	m_bCompass;

	wxString m_strInitScenario;

	wxString m_strInitTime;
//  wxString   m_strMemRequired;

// Scenarios
	std::vector<ScenarioParams> m_Scenarios;

	// WDR: method declarations for TParamsDlg
	wxRadioButton* GetUseExternal()  { return (wxRadioButton*) FindWindow( ID_USE_EXTERNAL ); }
	wxButton* GetMovedownScenario()  { return (wxButton*) FindWindow( ID_MOVEDOWN_SCENARIO ); }
	wxButton* GetMoveupScenario()  { return (wxButton*) FindWindow( ID_MOVEUP_SCENARIO ); }
	wxButton* GetEditScenario()  { return (wxButton*) FindWindow( ID_EDIT_SCENARIO ); }
	wxButton* GetDeleteScenario()  { return (wxButton*) FindWindow( ID_DELETE_SCENARIO ); }
	wxStaticBitmap* GetColorBitmap()  { return (wxStaticBitmap*) FindWindow( ID_COLOR3 ); }
	wxCheckBox* GetCheckStructureShadows()  { return (wxCheckBox*) FindWindow( ID_CHECK_STRUCTURE_SHADOWS ); }
	wxRadioButton* GetUseGrid()  { return (wxRadioButton*) FindWindow( ID_USE_GRID ); }
	wxRadioButton* GetUseTin()  { return (wxRadioButton*) FindWindow( ID_USE_TIN ); }
	wxRadioButton* GetUseTileset()  { return (wxRadioButton*) FindWindow( ID_USE_TILESET ); }
	wxRadioButton* GetDerived()  { return (wxRadioButton*) FindWindow( ID_DERIVED ); }
	wxRadioButton* GetSingle()  { return (wxRadioButton*) FindWindow( ID_SINGLE ); }
	wxTextCtrl* GetLightFactor()  { return (wxTextCtrl*) FindWindow( ID_LIGHT_FACTOR ); }
	wxTextCtrl* GetFogDistance()  { return (wxTextCtrl*) FindWindow( ID_FOG_DISTANCE ); }

	bool	m_bReady;
	bool	m_bSetting;
	vtStringArray m_TextureFiles;

private:
	// WDR: member variable declarations for TParamsDlg
	wxTextCtrl* m_pPreLightFactor;
	wxListBox* m_pImageFiles;
	wxRadioButton* m_pSingle;
	wxRadioButton* m_pDerived;
	wxRadioButton* m_pTiled;
	wxRadioButton* m_pTileset;
	wxListBox* m_pScenarioList;

private:
	// WDR: handler declarations for TParamsDlg
	void OnComboTFileSingle( wxCommandEvent &event );
	void OnOverlay( wxCommandEvent &event );
	void OnBgColor( wxCommandEvent &event );
	void OnCheckBoxElevType( wxCommandEvent &event );
	void OnCheckBox( wxCommandEvent &event );
	void OnListDblClickElev( wxCommandEvent &event );
	void OnListDblClickPlants( wxCommandEvent &event );
	void OnListDblClickStructure( wxCommandEvent &event );
	void OnListDblClickRaw( wxCommandEvent &event );
	void OnListDblClickAnimPaths( wxCommandEvent &event );
	void OnListDblClickImage( wxCommandEvent &event );
	void OnChoiceLocFile( wxCommandEvent &event );
	void OnChoiceInitLocation( wxCommandEvent &event );
	void OnSetInitTime( wxCommandEvent &event );
	void OnStyle( wxCommandEvent &event );
	void OnScenarioListEvent( wxCommandEvent &event );
	void OnNewScenario( wxCommandEvent &event );
	void OnDeleteScenario( wxCommandEvent &event );
	void OnEditScenario( wxCommandEvent &event );
	void OnMoveUpScenario( wxCommandEvent &event );
	void OnMoveDownSceanario( wxCommandEvent &event );
	void OnChoiceScenario( wxCommandEvent &event );
	void OnPrimaryTexture( wxCommandEvent &event );
	void OnSetTinTexture( wxCommandEvent &event );
	void OnSetTexture( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __TParamsDlg_H__

