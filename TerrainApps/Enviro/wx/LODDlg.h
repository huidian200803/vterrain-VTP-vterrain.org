//
// LODDlg.h
//
// Copyright (c) 2005-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __LODDlg_H__
#define __LODDlg_H__

#include "EnviroUI.h"
#include "vtui/AutoDialog.h"

class EnviroFrame;
class vtTiledGeom;
class vtCamera;
class vtPagedStructureLodGrid;

// WDR: class declarations

//----------------------------------------------------------------------------
// LODDlg
//----------------------------------------------------------------------------

class LODDlg: public PagingDlgBase
{
public:
	// constructors and destructors
	LODDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for LODDlg
	wxTextCtrl* GetTileStatus()  { return (wxTextCtrl*) FindWindow( ID_TILE_STATUS ); }
	wxTextCtrl* GetTarget()  { return (wxTextCtrl*) FindWindow( ID_TARGET ); }
	wxTextCtrl* GetCurrent()  { return (wxTextCtrl*) FindWindow( ID_CURRENT ); }
	wxPanel* GetPanel1()  { return (wxPanel*) FindWindow( ID_PANEL1 ); }
	wxPanel* GetPanel2()  { return (wxPanel*) FindWindow( ID_PANEL2 ); }
	wxPanel* GetPanel3()  { return (wxPanel*) FindWindow( ID_PANEL3 ); }
	wxStaticBox* GetTilesetBox() { return (wxStaticBox*) FindWindow(ID_TILESET_STATUS); }
	wxNotebook *GetNotebook() { return (wxNotebook*) FindWindow( ID_NOTEBOOK ); }

	void Refresh(float res0, float res, float res1, int target, int count,
		float prange);
	void DrawLODChart(float res0, float res, float res1, int target, int count);
	void DrawTilesetState(vtTiledGeom *tg, vtCamera *cam);
	void DrawStructureState(vtPagedStructureLodGrid *grid, float fPageOutDist);
	void SetFrame(EnviroFrame *pFrame) { m_pFrame = pFrame; }
	void SetPagingRange(float fmin, float fmax);
	void SetTerrain(class vtTerrain *pTerr) { m_pTerrain = pTerr; }

private:
	// WDR: member variable declarations for LODDlg
	bool m_bHaveRange;
	bool m_bHaveRangeVal;
	bool m_bSet;
	bool m_bShowTilesetStatus;
	int m_iTarget;
	EnviroFrame *m_pFrame;
	int m_iRange;
	float m_fRange;
	int m_iPageout;
	float m_fPageout;
	int m_iCountCur;
	int m_iCountMax;

	class vtTerrain *m_pTerrain;

	void SlidersToValues();
	void ValuesToSliders();

private:
	// WDR: handler declarations for LODDlg
	void OnSpinTargetUp( wxSpinEvent &event );
	void OnSpinTargetDown( wxSpinEvent &event );
	void OnText( wxCommandEvent &event );
	void OnTarget( wxCommandEvent &event );
	void OnRangeSlider( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __LODDlg_H__
