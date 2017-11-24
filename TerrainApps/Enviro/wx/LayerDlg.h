//
// Name: LayerDlg.h
//
// Copyright (c) 2003-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __LayerDlg_H__
#define __LayerDlg_H__

#include "wx/imaglist.h"
#include "wx/aui/aui.h"

#include "EnviroUI.h"
#include "vtlib/core/AbstractLayer.h"
#include "vtlib/core/TerrainLayers.h"
#include "vtlib/core/Globe.h"

// WDR: class declarations
class LayerItemData : public wxTreeItemData
{
public:
	LayerItemData(vtElevLayer *elay)
	{
		Defaults();
		m_type = LT_ELEVATION;
		m_layer = m_elay = elay;
	}
	LayerItemData(vtVegLayer *vlay)
	{
		Defaults();
		m_type = LT_VEG;
		m_layer = m_vlay = vlay;
	}
	LayerItemData(vtStructureLayer *slay, int index, int item)
	{
		Defaults();
		m_type = LT_STRUCTURE;
		m_layer = m_slay = slay;
		m_index = index;
		m_item = item;
	}
	LayerItemData(vtAbstractLayer *alay, vtFeatureSet *set)
	{
		Defaults();
		m_type = LT_ABSTRACT;
		m_layer = m_alay = alay;
		m_fset = set;
	}
	LayerItemData(vtImageLayer *ilay)
	{
		Defaults();
		m_type = LT_IMAGE;
		m_layer = m_ilay = ilay;
	}
	LayerItemData(GlobeLayer *glay)
	{
		Defaults();
		m_type = LT_ABSTRACT;
		m_glay = glay;
		m_fset = glay->m_pSet;
	}
	LayerItemData(LayerType type)
	{
		Defaults();
		m_type = type;
	}
	void Defaults()
	{
		m_layer = NULL;
		m_alay = NULL;
		m_vlay = NULL;
		m_slay = NULL;
		m_glay = NULL;
		m_fset = NULL;
		m_index = -1;
		m_item = -1;
	}
	LayerType m_type;
	vtLayer *m_layer;
	vtAbstractLayer *m_alay;
	vtImageLayer *m_ilay;
	vtVegLayer *m_vlay;
	vtElevLayer *m_elay;
	vtStructureLayer *m_slay;
	vtFeatureSet *m_fset;
	GlobeLayer *m_glay;
	int m_index;
	int m_item;
	wxString m_text;	// We cache the text label to detect when it changes.
	int m_icon;			// We cache the icon to detect when it changes.
};


//----------------------------------------------------------------------------
// LayerDlg
//----------------------------------------------------------------------------

class LayerDlg: public wxPanel
{
public:
	// constructors and destructors
	LayerDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	~LayerDlg();

	// WDR: method declarations for LayerDlg
	void RefreshTreeContents();
	void RefreshTreeTerrain();
	void UpdateTreeTerrain();
	void RefreshTreeSpace();

	void SetShowAll(bool bTrue);
	void UpdateEnabling();
	void SetTerrain(class vtTerrain *pTerr) { m_pTerrain = pTerr; }

	// Public handler declarations for LayerDlg
	void OnLayerCreate( wxCommandEvent &event );
	void OnLayerLoad( wxCommandEvent &event );

private:
	// WDR: member variable declarations for LayerDlg
	wxTreeCtrl *m_pTree;
	wxTreeItemId m_root;
	wxTreeItemId m_item;
	wxImageList *m_imageListNormal;
	bool	m_bShowAll;

private:
	wxAuiManager m_mgr;
	wxPanel *m_main;
	wxToolBar *m_pToolbar;
	class vtTerrain *m_pTerrain;

	osg::Node *GetNodeFromItem(wxTreeItemId item, bool bContainer = false);
	vtStructureLayer *GetStructureLayerFromItem(wxTreeItemId item);
	vtLayer *GetLayerFromItem(wxTreeItemId item);
	LayerItemData *GetLayerDataFromItem(wxTreeItemId item);
	void ToggleVisible(bool bVis, wxTreeItemId id);
	void CreateImageList(int size = 16);

	// WDR: handler declarations for LayerDlg
	void OnLayerRemove( wxCommandEvent &event );
	void OnLayerSave( wxCommandEvent &event );
	void OnLayerSaveAs( wxCommandEvent &event );
	void OnZoomTo( wxCommandEvent &event );
	void OnVisible( wxCommandEvent &event );
	void OnTable( wxCommandEvent &event );
	void OnRefresh( wxCommandEvent &event );
	void OnShadowVisible( wxCommandEvent &event );
	void OnShowAll( wxCommandEvent &event );
	void OnSelChanged( wxTreeEvent &event );
	void OnItemActived( wxTreeEvent &event );

	void OnUpdateCreate(wxUpdateUIEvent& event);
	void OnUpdateVisible(wxUpdateUIEvent& event);
	void OnUpdateRefresh(wxUpdateUIEvent& event);
	void OnUpdateShadow(wxUpdateUIEvent& event);
	void OnUpdateShowAll(wxUpdateUIEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __LayerDlg_H__
