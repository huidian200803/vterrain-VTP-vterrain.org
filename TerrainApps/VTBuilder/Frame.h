//
// Frame.h
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

#include "Builder.h"
#include "wx/aui/aui.h"
#include "wx/dnd.h"
#include "vtdata/WFSClient.h"	// for OGCServerArray
#include "StatusBar.h"

// some shortcuts
#define ADD_TOOL(bar, id, bmp, label) \
	bar->AddTool(id, label, bmp, wxNullBitmap, wxITEM_NORMAL, label, label)
#define ADD_TOOL2(bar, id, bmp, label, type) \
	bar->AddTool(id, label, bmp, wxNullBitmap, type, label, label)

class MyTreeCtrl;
class MyStatusBar;

// dialogs
class SpeciesListDlg;
class FeatInfoDlg;
class DistanceDlg2d;
class LinearStructureDlg;
class LinearStructureDlg2d;
class InstanceDlg;
class MapServerDlg;
class ProfileDlg;
class BioRegionDlg;
class vtScaleBar;

/**
* The main frame is the central class of the whole terrain builder.
* Not only does it represent the top window of the application, but it also
* contains many of the powerful methods for working with data.
*/
class MainFrame: public wxFrame, public Builder
{
public:
	MainFrame(wxFrame *frame, const wxString& title,
		const wxPoint& pos, const wxSize& size);
	virtual ~MainFrame();

	// view
	virtual void CreateView();
	void ZoomAll();

	void SetupUI();
	virtual void CreateMenus();
	void UpdateMRU(wxMenu *menu, const vtStringArray &files, int first_id);
	void ManageToolbar(const wxString &name, wxToolBar *bar, bool show);
	wxToolBar *NewToolbar();
	void CreateToolbar();
	void RefreshToolbars();
	virtual void AddMainToolbars();
	virtual void SetCRS(const vtCRS &p);

protected:
	void OnClose(wxCloseEvent &event);
	void OnFillIn(int method);

	// Menu commands
	void OnProjectNew(wxCommandEvent& event);
	void OnProjectOpen(wxCommandEvent& event);
	void OnProjectSave(wxCommandEvent& event);
	void OnProjectPrefs(wxCommandEvent& event);
	void OnElevFlip(wxCommandEvent &event);
	void OnBatchConvert(wxCommandEvent &event);
	void OnDymaxTexture(wxCommandEvent &event);
	void OnDymaxMap(wxCommandEvent &event);
	void OnProcessBillboard(wxCommandEvent &event);
	void OnElevCopy(wxCommandEvent& event);
	void OnElevPasteNew(wxCommandEvent& event);
	void OnGeocode(wxCommandEvent &event);
	void OnRunTest(wxCommandEvent &event);
	void OnQuit(wxCommandEvent& event);

	void OnUpdateFileMRU(wxUpdateUIEvent& event);

	void OnEditDelete(wxCommandEvent& event);
	void OnUpdateEditDelete(wxUpdateUIEvent& event);
	void OnEditDeselectAll(wxCommandEvent& event);
	void OnEditInvertSelection(wxCommandEvent& event);
	void OnEditCrossingSelection(wxCommandEvent& event);
	void OnUpdateCrossingSelection(wxUpdateUIEvent& event);
	void OnEditOffset(wxCommandEvent& event);

	void OnUpdateEditOffset(wxUpdateUIEvent& event);

	void OnLayerNew(wxCommandEvent& event);
	void OnLayerOpen(wxCommandEvent& event);
	void OnLayerSave(wxCommandEvent& event);
	void OnLayerSaveAs(wxCommandEvent& event);
	void OnLayerImport(wxCommandEvent& event);
	void OnLayerImportTIGER(wxCommandEvent& event);
	void OnLayerImportOSM(wxCommandEvent& event);
	void OnLayerImportNTF(wxCommandEvent& event);
	void OnLayerImportMapSource(wxCommandEvent& event);
	void OnLayerImportPoint(wxCommandEvent& event);
	void OnLayerImportXML(wxCommandEvent& event);
	void OnLayerImportDXF(wxCommandEvent& event);
	void OnLayerProperties(wxCommandEvent& event);
	void OnLayerConvertCRS(wxCommandEvent& event);
	void OnLayerSetCRS(wxCommandEvent& event);
	void OnLayerCombine(wxCommandEvent& event);

	void OnUpdateLayerSave(wxUpdateUIEvent& event);
	void OnUpdateLayerSaveAs(wxUpdateUIEvent& event);
	void OnUpdateMRULayer(wxUpdateUIEvent& event);
	void OnUpdateMRUImport(wxUpdateUIEvent& event);
	void OnUpdateLayerProperties(wxUpdateUIEvent& event);
	void OnUpdateLayerConvert(wxUpdateUIEvent& event);
	void OnUpdateLayerCombine(wxUpdateUIEvent& event);

	void OnLayerShow(wxCommandEvent& event);
	void OnLayerUp(wxCommandEvent& event);
	void OnLayerDown(wxCommandEvent& event);
	void OnViewMagnifier(wxCommandEvent& event);
	void OnViewPan(wxCommandEvent& event);
	void OnViewDistance(wxCommandEvent& event);
	void OnViewSetArea(wxCommandEvent& event);
public:
	void OnViewZoomIn(wxCommandEvent& event);
	void OnViewZoomOut(wxCommandEvent& event);
protected:
	void OnViewZoomAll(wxCommandEvent& event);
	void OnViewZoomToLayer(wxCommandEvent& event);
	void OnViewFull(wxCommandEvent& event);
	void OnViewZoomArea(wxCommandEvent& event);
	void OnViewToolbar(wxCommandEvent& event);
	void OnViewLayers(wxCommandEvent& event);
	void OnViewWorldMap(wxCommandEvent& event);
	void OnViewUTMBounds(wxCommandEvent& event);
	void OnViewProfile(wxCommandEvent& event);
	void OnViewScaleBar(wxCommandEvent& event);
	void OnViewOptions(wxCommandEvent& event);

	void OnUpdateLayerShow(wxUpdateUIEvent& event);
	void OnUpdateLayerUp(wxUpdateUIEvent& event);
	void OnUpdateLayerDown(wxUpdateUIEvent& event);
	void OnUpdateMagnifier(wxUpdateUIEvent& event);
	void OnUpdatePan(wxUpdateUIEvent& event);
	void OnUpdateDistance(wxUpdateUIEvent& event);
	void OnUpdateViewFull(wxUpdateUIEvent& event);
	void OnUpdateViewZoomArea(wxUpdateUIEvent& event);
	void OnUpdateViewToolbar(wxUpdateUIEvent& event);
	void OnUpdateViewLayers(wxUpdateUIEvent& event);
	void OnUpdateViewZoomToLayer(wxUpdateUIEvent& event);
	void OnUpdateWorldMap(wxUpdateUIEvent& event);
	void OnUpdateUTMBounds(wxUpdateUIEvent& event);
	void OnUpdateViewProfile(wxUpdateUIEvent& event);
	void OnUpdateViewScaleBar(wxUpdateUIEvent& event);

	void OnSelectLink(wxCommandEvent& event);
	void OnSelectNode(wxCommandEvent& event);
	void OnSelectWhole(wxCommandEvent& event);
	void OnDirection(wxCommandEvent& event);
	void OnRoadEdit(wxCommandEvent& event);
	void OnRoadShowNodes(wxCommandEvent& event);
	void OnSelectHwy(wxCommandEvent& event);
	void OnRoadClean(wxCommandEvent& event);
	void OnRoadGuess(wxCommandEvent& event);
	void OnRoadFlatten(wxCommandEvent& event);

	void OnUpdateSelectLink(wxUpdateUIEvent& event);
	void OnUpdateSelectNode(wxUpdateUIEvent& event);
	void OnUpdateSelectWhole(wxUpdateUIEvent& event);
	void OnUpdateDirection(wxUpdateUIEvent& event);
	void OnUpdateRoadEdit(wxUpdateUIEvent& event);
	void OnUpdateRoadShowNodes(wxUpdateUIEvent& event);
	void OnUpdateRoadFlatten(wxUpdateUIEvent& event);

	void OnElevSelect(wxCommandEvent& event);
	void OnElevRemoveRange(wxCommandEvent& event);
	void OnElevArithmetic(wxCommandEvent& event);
	void OnElevSetUnknown(wxCommandEvent& event);
	void OnFillFast(wxCommandEvent& event);
	void OnFillSlow(wxCommandEvent& event);
	void OnFillRegions(wxCommandEvent& event);
	void OnElevScale(wxCommandEvent& event);
	void OnElevVertOffset(wxCommandEvent& event);
	void OnElevExport(wxCommandEvent& event);
	void OnElevExportBitmap(wxCommandEvent& event);
	void OnElevToTin(wxCommandEvent& event);
	void OnElevContours(wxCommandEvent& event);
	void OnElevCarve(wxCommandEvent& event);
	void OnElevExportTiles(wxCommandEvent& event);
	void OnElevMergeTin(wxCommandEvent& event);
	void OnElevTrimTin(wxCommandEvent& event);

	void OnUpdateElevSelect(wxUpdateUIEvent& event);
	void OnUpdateViewSetArea(wxUpdateUIEvent& event);
	void OnUpdateElevSetUnknown(wxUpdateUIEvent& event);
	void OnUpdateElevMergeTin(wxUpdateUIEvent& event);
	void OnUpdateElevTrimTin(wxUpdateUIEvent& event);
	void OnUpdateIsElevation(wxUpdateUIEvent& event);
	void OnUpdateIsGrid(wxUpdateUIEvent& event);
	void OnUpdateArithmetic(wxUpdateUIEvent& event);

	void OnImageReplaceRGB(wxCommandEvent& event);
	void OnImageCreateOverviews(wxCommandEvent& event);
	void OnImageCreateOverviewsAll(wxCommandEvent& event);
	void OnImageCreateMipMaps(wxCommandEvent& event);
	void OnImageLoadMipMaps(wxCommandEvent& event);
	void OnImageExportTiles(wxCommandEvent& event);
	void OnImageExportPPM(wxCommandEvent& event);
	void OnUpdateHaveImageLayer(wxUpdateUIEvent& event);
	void OnUpdateHaveImageLayerInMem(wxUpdateUIEvent& event);

	void OnTowerSelect(wxCommandEvent& event);
	void OnTowerEdit(wxCommandEvent& event);
	void OnTowerAdd(wxCommandEvent& event);

	void OnUpdateTowerSelect(wxUpdateUIEvent& event);
	void OnUpdateTowerEdit(wxUpdateUIEvent& event);
	void OnUpdateTowerAdd(wxUpdateUIEvent& event);

	void OnVegPlants(wxCommandEvent& event);
	void OnVegBioregions(wxCommandEvent& event);
	void OnVegRemap(wxCommandEvent& event);
	void OnVegExportSHP(wxCommandEvent& event);
	void OnVegHTML(wxCommandEvent& event);
	void OnUpdateVegExportSHP(wxUpdateUIEvent& event);

	void OnFeatureSelect(wxCommandEvent& event);
	void OnFeaturePick(wxCommandEvent& event);
	void OnFeatureTable(wxCommandEvent& event);
	void OnBuildingEdit(wxCommandEvent& event);
	void OnBuildingAddPoints(wxCommandEvent& event);
	void OnBuildingDeletePoints(wxCommandEvent& event);
	void OnStructureAddLinear(wxCommandEvent& event);
	void OnStructureEditLinear(wxCommandEvent& event);
	void OnStructureAddInstances(wxCommandEvent& event);
	void OnStructureAddFoundation(wxCommandEvent& event);
	void OnStructureConstrain(wxCommandEvent& event);
	void OnStructureSelectUsingPolygons(wxCommandEvent& event);
	void OnStructureColourSelectedRoofs(wxCommandEvent& event);
	void OnStructureCleanFootprints(wxCommandEvent& event);
	void OnStructureSelectIndex(wxCommandEvent& event);
	void OnStructureExportFootprints(wxCommandEvent& event);
	void OnStructureExportCanoma(wxCommandEvent& event);

	void OnUpdateFeatureSelect(wxUpdateUIEvent& event);
	void OnUpdateFeaturePick(wxUpdateUIEvent& event);
	void OnUpdateFeatureTable(wxUpdateUIEvent& event);
	void OnUpdateBuildingEdit(wxUpdateUIEvent& event);
	void OnUpdateBuildingAddPoints(wxUpdateUIEvent& event);
	void OnUpdateBuildingDeletePoints(wxUpdateUIEvent& event);
	void OnUpdateStructureAddLinear(wxUpdateUIEvent& event);
	void OnUpdateStructureAddInstances(wxUpdateUIEvent& event);
	void OnUpdateStructureEditLinear(wxUpdateUIEvent& event);
	void OnUpdateStructureAddFoundation(wxUpdateUIEvent& event);
	void OnUpdateStructureConstrain(wxUpdateUIEvent& event);
	void OnUpdateStructureSelectUsingPolygons(wxUpdateUIEvent& event);
	void OnUpdateStructureColourSelectedRoofs(wxUpdateUIEvent& event);
	void OnUpdateStructureExportFootprints(wxUpdateUIEvent& event);

	void OnRawSetType(wxCommandEvent& event);
	void OnRawAddPoints(wxCommandEvent& event);
	void OnRawAddPointText(wxCommandEvent& event);
	void OnRawAddPointsGPS(wxCommandEvent& event);
	void OnRawAddFeatureWKT(wxCommandEvent& event);
	void OnRawSelectCondition(wxCommandEvent& event);
	void OnRawGenerateTIN(wxCommandEvent& event);
	void OnRawConvertToPolygons(wxCommandEvent& event);
	void OnRawExportKML(wxCommandEvent& event);
	void OnRawGenElevation(wxCommandEvent& event);
	void OnRawStyle(wxCommandEvent& event);
	void OnRawScaleH(wxCommandEvent& event);
	void OnRawScaleV(wxCommandEvent& event);
	void OnRawOffsetV(wxCommandEvent& event);
	void OnRawClean(wxCommandEvent& event);
	void OnRawSelectBad(wxCommandEvent& event);

	void OnUpdateRawSetType(wxUpdateUIEvent& event);
	void OnUpdateRawAddPoints(wxUpdateUIEvent& event);
	void OnUpdateRawAddPointText(wxUpdateUIEvent& event);
	void OnUpdateRawAddPointsGPS(wxUpdateUIEvent& event);
	void OnUpdateRawIsActive(wxUpdateUIEvent& event);
	void OnUpdateRawIsActive3D(wxUpdateUIEvent& event);
	void OnUpdateRawIsPolygon(wxUpdateUIEvent& event);
	void OnUpdateRawIsPoint(wxUpdateUIEvent& event);
	void OnUpdateRawHasPolylines(wxUpdateUIEvent& event);
	void OnUpdateRawGenElevation(wxUpdateUIEvent& event);

	void OnAreaClear(wxCommandEvent& event);
	void OnAreaZoomAll(wxCommandEvent& event);
	void OnAreaZoomLayer(wxCommandEvent& event);
	void OnAreaTypeIn(wxCommandEvent& event);
	void OnAreaMatch(wxCommandEvent& event);
	void OnAreaSampleElev(wxCommandEvent& event);
	void OnAreaSampleElevTileset(wxCommandEvent& event);
	void OnAreaSampleImage(wxCommandEvent& event);
	void OnAreaSampleImageTileset(wxCommandEvent& event);
	void OnAreaGenerateVeg(wxCommandEvent& event);
	void OnAreaVegDensity(wxCommandEvent& event);
	void OnAreaRequestWFS(wxCommandEvent& event);
	void OnAreaRequestWMS(wxCommandEvent& event);

	void OnUpdateAreaZoomAll(wxUpdateUIEvent& event);
	void OnUpdateAreaZoomLayer(wxUpdateUIEvent& event);
	void OnUpdateAreaMatch(wxUpdateUIEvent& event);
	void OnUpdateAreaSampleElev(wxUpdateUIEvent& event);
	void OnUpdateAreaSampleImage(wxUpdateUIEvent& event);
	void OnUpdateAreaGenerateVeg(wxUpdateUIEvent& event);
	void OnUpdateAreaVegDensity(wxUpdateUIEvent& event);
	void OnUpdateAreaRequestWMS(wxUpdateUIEvent& event);

	void OnHelpAbout(wxCommandEvent& event);
	void OnHelpDocLocal(wxCommandEvent& event);
	void OnHelpDocOnline(wxCommandEvent& event);

	// Popup menu items
	void OnDistanceClear(wxCommandEvent& event);
	void OnShowAll(wxCommandEvent& event);
	void OnHideAll(wxCommandEvent& event);
	void OnLayerPropsPopup(wxCommandEvent& event);
	void OnLayerToTop(wxCommandEvent& event);
	void OnLayerToBottom(wxCommandEvent& event);
	void OnLayerOverviewDisk(wxCommandEvent& event);
	void OnLayerOverviewMem(wxCommandEvent& event);

	// MRU dynamic menus
	void OnMRUFileProject(wxCommandEvent& event);
	void OnMRUFileLayer(wxCommandEvent& event);
	void OnMRUFileImport(wxCommandEvent& event);

	// keys (used for shortcuts)
	void OnChar(wxKeyEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnMouseWheel(wxMouseEvent& event);

public:
	// project
	bool LoadProject(const wxString &strPathName);
	bool SaveProject(const wxString &strPathName) const;
	void ShowOptionsDialog();
	void OnDrop(const wxString &str);

	// Special menu
	void DoProcessBillboard();
	void DoElevCopy();
	void DoElevPasteNew();
	void DoGeocode();
	void DoDymaxTexture();
	void DoDymaxMap();

	// Layer methods
	virtual bool AddLayerWithCheck(vtLayer *pLayer, bool bRefresh = true);
	virtual void RemoveLayer(vtLayer *lp);
	virtual void SetActiveLayer(vtLayer *lp, bool refresh = false);
	void ShowLayerProperties(vtLayer *lp);

	// UI
	virtual void RefreshTreeStatus();
	void RefreshTreeView();
	void RefreshStatusBar();
	BuilderView *GetView() { return m_pView; }
	virtual void UpdateFeatureDialog(vtRawLayer *raw, vtFeatureSet *set, int iEntity);
	virtual void OnSetMode(LBMode m);
	virtual void OnSelectionChanged();

	// Raw features
	FeatInfoDlg	*ShowFeatInfoDlg();
	FeatInfoDlg	*m_pFeatInfoDlg;

	// Distance and Elevation Profile
	DistanceDlg2d	*ShowDistanceDlg();
	DistanceDlg2d *m_pDistanceDlg;
	void UpdateDistance(const DPoint2 &p1, const DPoint2 &p2);
	void UpdateDistance(const DLine2 &path);
	void ClearDistance();
	ProfileDlg	*ShowProfileDlg();
	ProfileDlg	*m_pProfileDlg;

	// Structures
	LinearStructureDlg *ShowLinearStructureDlg(bool bShow = true);
	LinearStructureDlg2d *m_pLinearStructureDlg;
	InstanceDlg *ShowInstanceDlg(bool bShow);
	void CreateInstance(const DPoint2 &pos, vtTagArray *tags);

	// Vegetation
	SpeciesListDlg *m_SpeciesListDlg;
	BioRegionDlg *m_BioRegionDlg;

	// Web Access
	OGCServerArray m_wms_servers;
	OGCServerArray m_wfs_servers;

	void RefreshView();

protected:
	// INI File
	bool ReadINI(const char *fname);
	//bool WriteINI();
	bool ReadXML(const char *fname);
	bool WriteXML(const char *fname);
	void ApplyOptions();

	MapServerDlg *m_pMapServerDlg;

	// UI members
	wxMenu *fileMenu;
	wxMenu *mruMenu;
	wxMenu *editMenu;
	wxMenu *layerMenu;
	wxMenu *mruLayerMenu;
	wxMenu *mruImportMenu;
	wxMenu *viewMenu;
	wxMenu *elevMenu;
	wxMenu *imgMenu;
	wxMenu *vegMenu;
	wxMenu *bldMenu;
	wxMenu *roadMenu;
	wxMenu *utilityMenu;
	wxMenu *rawMenu;
	wxMenu *areaMenu;
	wxMenu *helpMenu;
	wxMenuBar *m_pMenuBar;
	wxToolBar *m_pToolbar;				// main toolbar
	wxToolBar *m_pLayBar[LAYER_TYPES];	// layer type-specific toolbars
	MyStatusBar *m_statbar;
	int		m_iMainButtons;

	MyTreeCtrl	*m_pTree;		// left child of splitter
	BuilderView	*m_pView;		// right child of splitter
	vtScaleBar *m_pScaleBar;

	// menu numbers, for each layer type that has a corresponding menu
	int		m_iLayerMenu[LAYER_TYPES];

	wxAuiManager m_mgr;

	DECLARE_EVENT_TABLE()
};

#if wxUSE_DRAG_AND_DROP
class DnDFile : public wxFileDropTarget
{
public:
	virtual bool OnDropFiles(wxCoord x, wxCoord y,
		const wxArrayString& filenames);
};
#endif

extern MainFrame *GetMainFrame();
wxString GetImportFilterString(LayerType ltype);
float ElevLayerArrayValue(std::vector<vtElevLayer*> &elevs, const DPoint2 &p);
void ElevLayerArrayRange(std::vector<vtElevLayer*> &elevs,
						 float &minval, float &maxval);

