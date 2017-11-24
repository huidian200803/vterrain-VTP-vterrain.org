//
//  The menus functions of the main Frame window of the VTBuilder application.
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/progdlg.h>
#include <wx/choicdlg.h>
#include <wx/colordlg.h>
#include <wx/numdlg.h>

#include "vtdata/config_vtdata.h"
#include "vtdata/ChunkLOD.h"
#include "vtdata/DataPath.h"
#include "vtdata/ElevationGrid.h"
#include "vtdata/FileFilters.h"
#include "vtdata/Icosa.h"
#include "vtdata/QuikGrid.h"
#include "vtdata/TripDub.h"
#include "vtdata/Version.h"
#include "vtdata/vtDIB.h"
#include "vtdata/vtLog.h"
#include "vtdata/WFSClient.h"

#include "vtui/Helper.h"
#include "vtui/ContourDlg.h"
#include "vtui/ProfileDlg.h"
#include "vtui/ProjectionDlg.h"

#include "gdal_priv.h"

#include "App.h"
#include "BuilderView.h"
#include "Frame.h"
#include "MenuEnum.h"
#include "Options.h"
#include "Tin2d.h"
#include "TreeView.h"
#include "vtImage.h"
// Layers
#include "ElevLayer.h"
#include "ImageLayer.h"
#include "RawLayer.h"
#include "RoadLayer.h"
#include "StructLayer.h"
#include "UtilityLayer.h"
#include "VegLayer.h"
#include "WaterLayer.h"
// Dialogs
#include "DistanceDlg2d.h"
#include "DistribVegDlg.h"
#include "ElevMathDlg.h"
#include "ExtentDlg.h"
#include "FeatInfoDlg.h"
#include "GenGridDlg.h"
#include "GeocodeDlg.h"
#include "ImageMapDlg.h"
#include "LayerPropDlg.h"
#include "MapServerDlg.h"
#include "MatchDlg.h"
#include "PrefDlg.h"
#include "RenderDlg.h"
#include "SelectDlg.h"
#include "VegDlg.h"

DECLARE_APP(BuilderApp)

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(ID_FILE_NEW,		MainFrame::OnProjectNew)
EVT_MENU(ID_FILE_OPEN,		MainFrame::OnProjectOpen)
EVT_MENU(ID_FILE_SAVE,		MainFrame::OnProjectSave)
EVT_MENU(ID_FILE_PREFS,		MainFrame::OnProjectPrefs)
EVT_MENU(ID_SPECIAL_FLIP,	MainFrame::OnElevFlip)
EVT_MENU(ID_SPECIAL_BATCH,	MainFrame::OnBatchConvert)
EVT_MENU(ID_SPECIAL_DYMAX_TEXTURES,	MainFrame::OnDymaxTexture)
EVT_MENU(ID_SPECIAL_DYMAX_MAP,	MainFrame::OnDymaxMap)
EVT_MENU(ID_SPECIAL_PROCESS_BILLBOARD,	MainFrame::OnProcessBillboard)
EVT_MENU(ID_SPECIAL_GEOCODE,	MainFrame::OnGeocode)
EVT_MENU(ID_SPECIAL_RUN_TEST,	MainFrame::OnRunTest)
EVT_MENU(ID_FILE_EXIT,		MainFrame::OnQuit)

EVT_UPDATE_UI(ID_FILE_MRU,	MainFrame::OnUpdateFileMRU)

EVT_MENU(ID_EDIT_DELETE, MainFrame::OnEditDelete)
EVT_MENU(ID_EDIT_DESELECTALL, MainFrame::OnEditDeselectAll)
EVT_MENU(ID_EDIT_INVERTSELECTION, MainFrame::OnEditInvertSelection)
EVT_MENU(ID_EDIT_CROSSINGSELECTION, MainFrame::OnEditCrossingSelection)

EVT_UPDATE_UI(ID_EDIT_DELETE,	MainFrame::OnUpdateEditDelete)
EVT_UPDATE_UI(ID_EDIT_CROSSINGSELECTION,	MainFrame::OnUpdateCrossingSelection)

EVT_MENU(ID_LAYER_NEW,			MainFrame::OnLayerNew)
EVT_MENU(ID_LAYER_OPEN,			MainFrame::OnLayerOpen)
EVT_MENU(ID_LAYER_SAVE,			MainFrame::OnLayerSave)
EVT_MENU(ID_LAYER_SAVE_AS,		MainFrame::OnLayerSaveAs)
EVT_MENU(ID_LAYER_IMPORT,		MainFrame::OnLayerImport)
EVT_MENU(ID_LAYER_IMPORTTIGER,	MainFrame::OnLayerImportTIGER)
EVT_MENU(ID_LAYER_IMPORTOSM,	MainFrame::OnLayerImportOSM)
EVT_MENU(ID_LAYER_IMPORTNTF,	MainFrame::OnLayerImportNTF)
EVT_MENU(ID_LAYER_IMPORT_MS,	MainFrame::OnLayerImportMapSource)
EVT_MENU(ID_LAYER_IMPORT_POINT,	MainFrame::OnLayerImportPoint)
EVT_MENU(ID_LAYER_IMPORT_XML,	MainFrame::OnLayerImportXML)
EVT_MENU(ID_LAYER_IMPORT_DXF,	MainFrame::OnLayerImportDXF)
EVT_MENU(ID_LAYER_PROPS,		MainFrame::OnLayerProperties)
EVT_MENU(ID_LAYER_CONVERTCRS,	MainFrame::OnLayerConvertCRS)
EVT_MENU(ID_LAYER_SETCRS,		MainFrame::OnLayerSetCRS)
EVT_MENU(ID_LAYER_COMBINE,		MainFrame::OnLayerCombine)
EVT_MENU(ID_EDIT_OFFSET,		MainFrame::OnEditOffset)

EVT_UPDATE_UI(ID_LAYER_SAVE,	MainFrame::OnUpdateLayerSave)
EVT_UPDATE_UI(ID_LAYER_SAVE_AS,	MainFrame::OnUpdateLayerSaveAs)
EVT_UPDATE_UI(ID_MRU_LAYER,		MainFrame::OnUpdateMRULayer)
EVT_UPDATE_UI(ID_MRU_IMPORT,	MainFrame::OnUpdateMRUImport)
EVT_UPDATE_UI(ID_LAYER_PROPS,	MainFrame::OnUpdateLayerProperties)
EVT_UPDATE_UI(ID_LAYER_COMBINE,	MainFrame::OnUpdateLayerCombine)
EVT_UPDATE_UI(ID_EDIT_OFFSET,	MainFrame::OnUpdateEditOffset)

EVT_MENU(ID_VIEW_SHOWLAYER,		MainFrame::OnLayerShow)
EVT_MENU(ID_VIEW_LAYER_UP,		MainFrame::OnLayerUp)
EVT_MENU(ID_VIEW_LAYER_DOWN,	MainFrame::OnLayerDown)
EVT_MENU(ID_VIEW_ZOOMIN,		MainFrame::OnViewZoomIn)
EVT_MENU(ID_VIEW_ZOOMOUT,		MainFrame::OnViewZoomOut)
EVT_MENU(ID_VIEW_ZOOMALL,		MainFrame::OnViewZoomAll)
EVT_MENU(ID_VIEW_ZOOM_LAYER,	MainFrame::OnViewZoomToLayer)
EVT_MENU(ID_VIEW_FULLVIEW,		MainFrame::OnViewFull)
EVT_MENU(ID_VIEW_ZOOM_AREA,		MainFrame::OnViewZoomArea)
EVT_MENU(ID_VIEW_TOOLBAR,		MainFrame::OnViewToolbar)
EVT_MENU(ID_VIEW_LAYERS,		MainFrame::OnViewLayers)
EVT_MENU(ID_VIEW_MAGNIFIER,		MainFrame::OnViewMagnifier)
EVT_MENU(ID_VIEW_PAN,			MainFrame::OnViewPan)
EVT_MENU(ID_VIEW_DISTANCE,		MainFrame::OnViewDistance)
EVT_MENU(ID_VIEW_SETAREA,		MainFrame::OnViewSetArea)
EVT_MENU(ID_VIEW_WORLDMAP,		MainFrame::OnViewWorldMap)
EVT_MENU(ID_VIEW_SHOWUTM,		MainFrame::OnViewUTMBounds)
EVT_MENU(ID_VIEW_PROFILE,		MainFrame::OnViewProfile)
EVT_MENU(ID_VIEW_SCALE_BAR,		MainFrame::OnViewScaleBar)
EVT_MENU(ID_VIEW_OPTIONS,		MainFrame::OnViewOptions)

EVT_UPDATE_UI(ID_VIEW_SHOWLAYER,	MainFrame::OnUpdateLayerShow)
EVT_UPDATE_UI(ID_VIEW_LAYER_UP,		MainFrame::OnUpdateLayerUp)
EVT_UPDATE_UI(ID_VIEW_LAYER_DOWN,	MainFrame::OnUpdateLayerDown)
EVT_UPDATE_UI(ID_VIEW_MAGNIFIER,	MainFrame::OnUpdateMagnifier)
EVT_UPDATE_UI(ID_VIEW_PAN,			MainFrame::OnUpdatePan)
EVT_UPDATE_UI(ID_VIEW_DISTANCE,		MainFrame::OnUpdateDistance)
EVT_UPDATE_UI(ID_VIEW_ZOOM_LAYER,	MainFrame::OnUpdateViewZoomToLayer)
EVT_UPDATE_UI(ID_VIEW_FULLVIEW,		MainFrame::OnUpdateViewFull)
EVT_UPDATE_UI(ID_VIEW_ZOOM_AREA,	MainFrame::OnUpdateViewZoomArea)
EVT_UPDATE_UI(ID_VIEW_TOOLBAR,		MainFrame::OnUpdateViewToolbar)
EVT_UPDATE_UI(ID_VIEW_LAYERS,		MainFrame::OnUpdateViewLayers)
EVT_UPDATE_UI(ID_VIEW_SETAREA,		MainFrame::OnUpdateViewSetArea)
EVT_UPDATE_UI(ID_VIEW_WORLDMAP,		MainFrame::OnUpdateWorldMap)
EVT_UPDATE_UI(ID_VIEW_SHOWUTM,		MainFrame::OnUpdateUTMBounds)
EVT_UPDATE_UI(ID_VIEW_PROFILE,		MainFrame::OnUpdateViewProfile)
EVT_UPDATE_UI(ID_VIEW_SCALE_BAR,	MainFrame::OnUpdateViewScaleBar)

EVT_MENU(ID_ROAD_SELECTROAD,	MainFrame::OnSelectLink)
EVT_MENU(ID_ROAD_SELECTNODE,	MainFrame::OnSelectNode)
EVT_MENU(ID_ROAD_SELECTWHOLE,	MainFrame::OnSelectWhole)
EVT_MENU(ID_ROAD_DIRECTION,		MainFrame::OnDirection)
EVT_MENU(ID_ROAD_EDIT,			MainFrame::OnRoadEdit)
EVT_MENU(ID_ROAD_SHOWNODES,		MainFrame::OnRoadShowNodes)
EVT_MENU(ID_ROAD_SELECTHWY,		MainFrame::OnSelectHwy)
EVT_MENU(ID_ROAD_CLEAN,			MainFrame::OnRoadClean)
EVT_MENU(ID_ROAD_GUESS,			MainFrame::OnRoadGuess)

EVT_UPDATE_UI(ID_ROAD_SELECTROAD,	MainFrame::OnUpdateSelectLink)
EVT_UPDATE_UI(ID_ROAD_SELECTNODE,	MainFrame::OnUpdateSelectNode)
EVT_UPDATE_UI(ID_ROAD_SELECTWHOLE,	MainFrame::OnUpdateSelectWhole)
EVT_UPDATE_UI(ID_ROAD_DIRECTION,	MainFrame::OnUpdateDirection)
EVT_UPDATE_UI(ID_ROAD_EDIT,			MainFrame::OnUpdateRoadEdit)
EVT_UPDATE_UI(ID_ROAD_SHOWNODES,	MainFrame::OnUpdateRoadShowNodes)

EVT_MENU(ID_ELEV_SELECT,			MainFrame::OnElevSelect)
EVT_MENU(ID_ELEV_REMOVERANGE,		MainFrame::OnElevRemoveRange)
EVT_MENU(ID_ELEV_ARITHMETIC,		MainFrame::OnElevArithmetic)
EVT_MENU(ID_ELEV_SETUNKNOWN,		MainFrame::OnElevSetUnknown)
EVT_MENU(ID_ELEV_FILL_FAST,			MainFrame::OnFillFast)
EVT_MENU(ID_ELEV_FILL_SLOW,			MainFrame::OnFillSlow)
EVT_MENU(ID_ELEV_FILL_REGIONS,		MainFrame::OnFillRegions)
EVT_MENU(ID_ELEV_SCALE,				MainFrame::OnElevScale)
EVT_MENU(ID_ELEV_VERT_OFFSET,		MainFrame::OnElevVertOffset)
EVT_MENU(ID_ELEV_EXPORT,			MainFrame::OnElevExport)
EVT_MENU(ID_ELEV_EXPORT_TILES,		MainFrame::OnElevExportTiles)
EVT_MENU(ID_ELEV_COPY,				MainFrame::OnElevCopy)
EVT_MENU(ID_ELEV_PASTE_NEW,			MainFrame::OnElevPasteNew)
EVT_MENU(ID_ELEV_BITMAP,			MainFrame::OnElevExportBitmap)
EVT_MENU(ID_ELEV_TOTIN,				MainFrame::OnElevToTin)
EVT_MENU(ID_ELEV_CONTOURS,			MainFrame::OnElevContours)
EVT_MENU(ID_ELEV_CARVE,				MainFrame::OnElevCarve)
EVT_MENU(ID_ELEV_MERGETIN,			MainFrame::OnElevMergeTin)
EVT_MENU(ID_ELEV_TRIMTIN,			MainFrame::OnElevTrimTin)

EVT_UPDATE_UI(ID_ELEV_SELECT,		MainFrame::OnUpdateElevSelect)
EVT_UPDATE_UI(ID_ELEV_REMOVERANGE,	MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_ARITHMETIC,	MainFrame::OnUpdateArithmetic)
EVT_UPDATE_UI(ID_ELEV_SETUNKNOWN,	MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_FILL_FAST,	MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_FILL_SLOW,	MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_FILL_REGIONS,	MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_SCALE,		MainFrame::OnUpdateIsElevation)
EVT_UPDATE_UI(ID_ELEV_VERT_OFFSET,	MainFrame::OnUpdateIsElevation)
EVT_UPDATE_UI(ID_ELEV_EXPORT,		MainFrame::OnUpdateIsElevation)
EVT_UPDATE_UI(ID_ELEV_EXPORT_TILES,	MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_COPY,			MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_BITMAP,		MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_TOTIN,		MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_CONTOURS,		MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_CARVE,		MainFrame::OnUpdateIsGrid)
EVT_UPDATE_UI(ID_ELEV_MERGETIN,		MainFrame::OnUpdateElevMergeTin)
EVT_UPDATE_UI(ID_ELEV_TRIMTIN,		MainFrame::OnUpdateElevTrimTin)

EVT_MENU(ID_IMAGE_REPLACE_RGB,		MainFrame::OnImageReplaceRGB)
EVT_MENU(ID_IMAGE_CREATE_OVERVIEWS,	MainFrame::OnImageCreateOverviews)
EVT_MENU(ID_IMAGE_CREATE_OVER_ALL,	MainFrame::OnImageCreateOverviewsAll)
EVT_MENU(ID_IMAGE_CREATE_MIPMAPS,	MainFrame::OnImageCreateMipMaps)
EVT_MENU(ID_IMAGE_LOAD_MIPMAPS,		MainFrame::OnImageLoadMipMaps)
EVT_MENU(ID_IMAGE_EXPORT_TILES,		MainFrame::OnImageExportTiles)
EVT_MENU(ID_IMAGE_EXPORT_PPM,		MainFrame::OnImageExportPPM)

EVT_UPDATE_UI(ID_IMAGE_REPLACE_RGB,	MainFrame::OnUpdateHaveImageLayer)
EVT_UPDATE_UI(ID_IMAGE_CREATE_OVERVIEWS, MainFrame::OnUpdateHaveImageLayer)
EVT_UPDATE_UI(ID_IMAGE_CREATE_OVER_ALL, MainFrame::OnUpdateHaveImageLayer)
EVT_UPDATE_UI(ID_IMAGE_CREATE_MIPMAPS, MainFrame::OnUpdateHaveImageLayerInMem)
EVT_UPDATE_UI(ID_IMAGE_LOAD_MIPMAPS, MainFrame::OnUpdateHaveImageLayer)
EVT_UPDATE_UI(ID_IMAGE_EXPORT_TILES,MainFrame::OnUpdateHaveImageLayer)
EVT_UPDATE_UI(ID_IMAGE_EXPORT_PPM,	MainFrame::OnUpdateHaveImageLayer)

EVT_MENU(ID_TOWER_ADD,				MainFrame::OnTowerAdd)
EVT_MENU(ID_TOWER_SELECT,			MainFrame::OnTowerSelect)
EVT_MENU(ID_TOWER_EDIT,				MainFrame::OnTowerEdit)

EVT_UPDATE_UI(ID_TOWER_ADD,			MainFrame::OnUpdateTowerAdd)
EVT_UPDATE_UI(ID_TOWER_SELECT,		MainFrame::OnUpdateTowerSelect)
EVT_UPDATE_UI(ID_TOWER_EDIT,		MainFrame::OnUpdateTowerEdit)

EVT_MENU(ID_VEG_PLANTS,				MainFrame::OnVegPlants)
EVT_MENU(ID_VEG_BIOREGIONS,			MainFrame::OnVegBioregions)
EVT_MENU(ID_VEG_REMAP,				MainFrame::OnVegRemap)
EVT_MENU(ID_VEG_EXPORTSHP,			MainFrame::OnVegExportSHP)
EVT_MENU(ID_VEG_HTML,				MainFrame::OnVegHTML)

EVT_UPDATE_UI(ID_VEG_REMAP,			MainFrame::OnUpdateVegExportSHP)
EVT_UPDATE_UI(ID_VEG_EXPORTSHP,		MainFrame::OnUpdateVegExportSHP)

EVT_MENU(ID_FEATURE_SELECT,			MainFrame::OnFeatureSelect)
EVT_MENU(ID_FEATURE_PICK,			MainFrame::OnFeaturePick)
EVT_MENU(ID_FEATURE_TABLE,			MainFrame::OnFeatureTable)
EVT_MENU(ID_STRUCTURE_EDIT_BLD,		MainFrame::OnBuildingEdit)
EVT_MENU(ID_STRUCTURE_ADD_POINTS,	MainFrame::OnBuildingAddPoints)
EVT_MENU(ID_STRUCTURE_DELETE_POINTS, MainFrame::OnBuildingDeletePoints)
EVT_MENU(ID_STRUCTURE_ADD_LINEAR,	MainFrame::OnStructureAddLinear)
EVT_MENU(ID_STRUCTURE_EDIT_LINEAR,	MainFrame::OnStructureEditLinear)
EVT_MENU(ID_STRUCTURE_ADD_INST,		MainFrame::OnStructureAddInstances)
EVT_MENU(ID_STRUCTURE_ADD_FOUNDATION, MainFrame::OnStructureAddFoundation)
EVT_MENU(ID_STRUCTURE_CONSTRAIN,	MainFrame::OnStructureConstrain)
EVT_MENU(ID_STRUCTURE_SELECT_USING_POLYGONS, MainFrame::OnStructureSelectUsingPolygons)
EVT_MENU(ID_STRUCTURE_COLOUR_SELECTED_ROOFS, MainFrame::OnStructureColourSelectedRoofs)
EVT_MENU(ID_STRUCTURE_CLEAN_FOOTPRINTS, MainFrame::OnStructureCleanFootprints)
EVT_MENU(ID_STRUCTURE_SELECT_INDEX, MainFrame::OnStructureSelectIndex)
EVT_MENU(ID_STRUCTURE_EXPORT_FOOTPRINTS, MainFrame::OnStructureExportFootprints)
EVT_MENU(ID_STRUCTURE_EXPORT_CANOMA, MainFrame::OnStructureExportCanoma)

EVT_UPDATE_UI(ID_FEATURE_SELECT,		MainFrame::OnUpdateFeatureSelect)
EVT_UPDATE_UI(ID_FEATURE_PICK,			MainFrame::OnUpdateFeaturePick)
EVT_UPDATE_UI(ID_FEATURE_TABLE,			MainFrame::OnUpdateFeatureTable)
EVT_UPDATE_UI(ID_STRUCTURE_EDIT_BLD,	MainFrame::OnUpdateBuildingEdit)
EVT_UPDATE_UI(ID_STRUCTURE_ADD_POINTS,	MainFrame::OnUpdateBuildingAddPoints)
EVT_UPDATE_UI(ID_STRUCTURE_DELETE_POINTS,	MainFrame::OnUpdateBuildingDeletePoints)
EVT_UPDATE_UI(ID_STRUCTURE_ADD_LINEAR,	MainFrame::OnUpdateStructureAddLinear)
EVT_UPDATE_UI(ID_STRUCTURE_EDIT_LINEAR,	MainFrame::OnUpdateStructureEditLinear)
EVT_UPDATE_UI(ID_STRUCTURE_ADD_INST,	MainFrame::OnUpdateStructureAddInstances)
EVT_UPDATE_UI(ID_STRUCTURE_ADD_FOUNDATION,	MainFrame::OnUpdateStructureAddFoundation)
EVT_UPDATE_UI(ID_STRUCTURE_CONSTRAIN,	MainFrame::OnUpdateStructureConstrain)
EVT_UPDATE_UI(ID_STRUCTURE_SELECT_USING_POLYGONS, MainFrame::OnUpdateStructureSelectUsingPolygons)
EVT_UPDATE_UI(ID_STRUCTURE_COLOUR_SELECTED_ROOFS, MainFrame::OnUpdateStructureColourSelectedRoofs)
EVT_UPDATE_UI(ID_STRUCTURE_EXPORT_FOOTPRINTS, MainFrame::OnUpdateStructureExportFootprints)
EVT_UPDATE_UI(ID_STRUCTURE_EXPORT_CANOMA, MainFrame::OnUpdateStructureExportFootprints)

EVT_MENU(ID_RAW_SETTYPE,			MainFrame::OnRawSetType)
EVT_MENU(ID_RAW_ADDPOINTS,			MainFrame::OnRawAddPoints)
EVT_MENU(ID_RAW_ADDPOINT_TEXT,		MainFrame::OnRawAddPointText)
EVT_MENU(ID_RAW_ADDPOINTS_GPS,		MainFrame::OnRawAddPointsGPS)
EVT_MENU(ID_RAW_ADDFEATURE_WKT,		MainFrame::OnRawAddFeatureWKT)
EVT_MENU(ID_RAW_STYLE,				MainFrame::OnRawStyle)
EVT_MENU(ID_RAW_SCALE_H,			MainFrame::OnRawScaleH)
EVT_MENU(ID_RAW_SCALE_V,			MainFrame::OnRawScaleV)
EVT_MENU(ID_RAW_OFFSET_V,			MainFrame::OnRawOffsetV)
EVT_MENU(ID_RAW_CLEAN,				MainFrame::OnRawClean)
EVT_MENU(ID_RAW_SELECT_BAD,			MainFrame::OnRawSelectBad)
EVT_MENU(ID_RAW_SELECTCONDITION,	MainFrame::OnRawSelectCondition)
EVT_MENU(ID_RAW_EXPORT_KML,			MainFrame::OnRawExportKML)
EVT_MENU(ID_RAW_GENERATE_ELEVATION,	MainFrame::OnRawGenElevation)
EVT_MENU(ID_RAW_GENERATE_TIN,		MainFrame::OnRawGenerateTIN)
EVT_MENU(ID_RAW_CONVERT_TOPOLYS,	MainFrame::OnRawConvertToPolygons)

EVT_UPDATE_UI(ID_RAW_SETTYPE,			MainFrame::OnUpdateRawSetType)
EVT_UPDATE_UI(ID_RAW_ADDPOINTS,			MainFrame::OnUpdateRawAddPoints)
EVT_UPDATE_UI(ID_RAW_ADDPOINT_TEXT,		MainFrame::OnUpdateRawAddPointText)
EVT_UPDATE_UI(ID_RAW_ADDPOINTS_GPS,		MainFrame::OnUpdateRawAddPointsGPS)
EVT_UPDATE_UI(ID_RAW_ADDFEATURE_WKT,	MainFrame::OnUpdateRawIsActive)
EVT_UPDATE_UI(ID_RAW_STYLE,				MainFrame::OnUpdateRawIsActive)
EVT_UPDATE_UI(ID_RAW_SCALE_H,			MainFrame::OnUpdateRawIsActive)
EVT_UPDATE_UI(ID_RAW_SCALE_V,			MainFrame::OnUpdateRawIsActive3D)
EVT_UPDATE_UI(ID_RAW_OFFSET_V,			MainFrame::OnUpdateRawIsActive3D)
EVT_UPDATE_UI(ID_RAW_CLEAN,				MainFrame::OnUpdateRawHasPolylines)
EVT_UPDATE_UI(ID_RAW_SELECT_BAD,		MainFrame::OnUpdateRawIsPolygon)
EVT_UPDATE_UI(ID_RAW_SELECTCONDITION,	MainFrame::OnUpdateRawIsActive)
EVT_UPDATE_UI(ID_RAW_EXPORT_IMAGEMAP,	MainFrame::OnUpdateRawIsPolygon)
EVT_UPDATE_UI(ID_RAW_EXPORT_KML,		MainFrame::OnUpdateRawIsPoint)
EVT_UPDATE_UI(ID_RAW_GENERATE_ELEVATION,MainFrame::OnUpdateRawGenElevation)
EVT_UPDATE_UI(ID_RAW_GENERATE_TIN,		MainFrame::OnUpdateRawIsActive)
EVT_UPDATE_UI(ID_RAW_CONVERT_TOPOLYS,	MainFrame::OnUpdateRawIsActive)

EVT_MENU(ID_AREA_CLEAR,				MainFrame::OnAreaClear)
EVT_MENU(ID_AREA_ZOOM_ALL,			MainFrame::OnAreaZoomAll)
EVT_MENU(ID_AREA_ZOOM_LAYER,		MainFrame::OnAreaZoomLayer)
EVT_MENU(ID_AREA_TYPEIN,			MainFrame::OnAreaTypeIn)
EVT_MENU(ID_AREA_MATCH,				MainFrame::OnAreaMatch)
EVT_MENU(ID_AREA_SAMPLE_ELEV,		MainFrame::OnAreaSampleElev)
EVT_MENU(ID_AREA_SAMPLE_IMAGE,		MainFrame::OnAreaSampleImage)
EVT_MENU(ID_AREA_GENERATE_VEG,		MainFrame::OnAreaGenerateVeg)
EVT_MENU(ID_AREA_VEG_DENSITY,		MainFrame::OnAreaVegDensity)
EVT_MENU(ID_AREA_REQUEST_WFS,		MainFrame::OnAreaRequestWFS)
EVT_MENU(ID_AREA_REQUEST_WMS,		MainFrame::OnAreaRequestWMS)
EVT_MENU(ID_AREA_SAMPLE_ELEV_OPT,	MainFrame::OnAreaSampleElevTileset)
EVT_MENU(ID_AREA_SAMPLE_IMAGE_OPT,	MainFrame::OnAreaSampleImageTileset)

EVT_UPDATE_UI(ID_AREA_ZOOM_ALL,		MainFrame::OnUpdateAreaZoomAll)
EVT_UPDATE_UI(ID_AREA_ZOOM_LAYER,	MainFrame::OnUpdateAreaZoomLayer)
EVT_UPDATE_UI(ID_AREA_MATCH,		MainFrame::OnUpdateAreaMatch)
EVT_UPDATE_UI(ID_AREA_SAMPLE_ELEV,	MainFrame::OnUpdateAreaSampleElev)
EVT_UPDATE_UI(ID_AREA_SAMPLE_IMAGE,	MainFrame::OnUpdateAreaSampleImage)
EVT_UPDATE_UI(ID_AREA_GENERATE_VEG,	MainFrame::OnUpdateAreaGenerateVeg)
EVT_UPDATE_UI(ID_AREA_VEG_DENSITY,	MainFrame::OnUpdateAreaVegDensity)
EVT_UPDATE_UI(ID_AREA_REQUEST_WFS,	MainFrame::OnUpdateAreaRequestWMS)
EVT_UPDATE_UI(ID_AREA_REQUEST_WMS,	MainFrame::OnUpdateAreaRequestWMS)
EVT_UPDATE_UI(ID_AREA_SAMPLE_ELEV_OPT,	MainFrame::OnUpdateAreaSampleElev)
EVT_UPDATE_UI(ID_AREA_SAMPLE_IMAGE_OPT,	MainFrame::OnUpdateAreaSampleImage)

EVT_MENU(wxID_HELP,				MainFrame::OnHelpAbout)
EVT_MENU(ID_HELP_DOC_LOCAL,		MainFrame::OnHelpDocLocal)
EVT_MENU(ID_HELP_DOC_ONLINE,	MainFrame::OnHelpDocOnline)

// Popup menu items
EVT_MENU(ID_DISTANCE_CLEAR,		MainFrame::OnDistanceClear)
EVT_MENU(ID_POPUP_SHOWALL,		MainFrame::OnShowAll)
EVT_MENU(ID_POPUP_HIDEALL,		MainFrame::OnHideAll)
EVT_MENU(ID_POPUP_PROPS,		MainFrame::OnLayerPropsPopup)
EVT_MENU(ID_POPUP_TO_TOP,		MainFrame::OnLayerToTop)
EVT_MENU(ID_POPUP_TO_BOTTOM,	MainFrame::OnLayerToBottom)
EVT_MENU(ID_POPUP_OVR_DISK,		MainFrame::OnLayerOverviewDisk)
EVT_MENU(ID_POPUP_OVR_MEM,		MainFrame::OnLayerOverviewMem)

// MRU dynamic menus
EVT_MENU_RANGE(ID_FIRST_MRU_FILE, ID_FIRST_MRU_FILE+40, MainFrame::OnMRUFileProject)
EVT_MENU_RANGE(ID_FIRST_MRU_LAYER, ID_FIRST_MRU_LAYER+40, MainFrame::OnMRUFileLayer)
EVT_MENU_RANGE(ID_FIRST_MRU_IMPORT, ID_FIRST_MRU_IMPORT+40, MainFrame::OnMRUFileImport)

EVT_CHAR(MainFrame::OnChar)
EVT_KEY_DOWN(MainFrame::OnKeyDown)
EVT_MOUSEWHEEL(MainFrame::OnMouseWheel)
EVT_CLOSE(MainFrame::OnClose)

END_EVENT_TABLE()


void MainFrame::CreateMenus()
{
	int menu_num = 0;
	wxString ampersand = _T("&");

	m_pMenuBar = new wxMenuBar;

	// Project menu
	fileMenu = new wxMenu;
	fileMenu->Append(ID_FILE_NEW, _("&New\tCtrl+N"), _("New Project"));
	fileMenu->Append(ID_FILE_OPEN, _("Open Project\tCtrl+O"), _("Open Project"));
	fileMenu->Append(ID_FILE_SAVE, _("Save Project\tCtrl+S"), _("Save Project As"));
	mruMenu = new wxMenu;
	fileMenu->Append(ID_FILE_MRU, _("Recent Projects"), mruMenu);
	fileMenu->AppendSeparator();
	wxMenu *specialMenu = new wxMenu;
	specialMenu->Append(ID_SPECIAL_FLIP, _("&Flip Elevation North/South"));
	specialMenu->Append(ID_SPECIAL_BATCH, _("Batch Conversion of Elevation"));
	specialMenu->Append(ID_SPECIAL_DYMAX_TEXTURES, _("Create Dymaxion Textures"));
	specialMenu->Append(ID_SPECIAL_DYMAX_MAP, _("Create Dymaxion Map"));
	specialMenu->Append(ID_SPECIAL_PROCESS_BILLBOARD, _("Process Billboard Texture"));
	specialMenu->Append(ID_SPECIAL_GEOCODE, _("Geocode"));
	specialMenu->Append(ID_SPECIAL_RUN_TEST, _("Run test"));
	specialMenu->Append(ID_ELEV_COPY, _("Copy Elevation Layer to Clipboard"));
	specialMenu->Append(ID_ELEV_PASTE_NEW, _("New Elevation Layer from Clipboard"));
	fileMenu->Append(0, ampersand + _("Special"), specialMenu);
	fileMenu->AppendSeparator();
	fileMenu->Append(ID_FILE_PREFS, _("Preferences"));
	fileMenu->AppendSeparator();
	fileMenu->Append(ID_FILE_EXIT, _("E&xit\tAlt-X"), _("Exit"));
#ifdef __WXMAC__
	wxApp::s_macPreferencesMenuItemId = ID_FILE_PREFS;
	wxApp::s_macExitMenuItemId = ID_FILE_EXIT;
#endif
	m_pMenuBar->Append(fileMenu, _("&Project"));
	menu_num++;

	// Edit
	editMenu = new wxMenu;
	editMenu->Append(ID_EDIT_DELETE, _("Delete\tDEL"), _("Delete"));
	editMenu->AppendSeparator();
	editMenu->Append(ID_EDIT_DESELECTALL, _("Deselect All"), _("Clears selection"));
	editMenu->Append(ID_EDIT_INVERTSELECTION, _("Invert Selection"));
	editMenu->AppendCheckItem(ID_EDIT_CROSSINGSELECTION, _("Crossing Selection"));
	m_pMenuBar->Append(editMenu, _("&Edit"));
	menu_num++;

	// Layer
	layerMenu = new wxMenu;
	layerMenu->Append(ID_LAYER_NEW, _("&New Layer"), _("Create New Layer"));
	layerMenu->Append(ID_LAYER_OPEN, _("Open Layer"), _("Open Existing Layer"));
	layerMenu->Append(ID_LAYER_SAVE, _("Save Layer"), _("Save Active Layer"));
	layerMenu->Append(ID_LAYER_SAVE_AS, _("Save Layer As..."), _("Save Active Layer As"));
	layerMenu->Append(ID_LAYER_IMPORT, _("Import Data\tCtrl+I"), _("Import Data"));
	mruLayerMenu = new wxMenu;
	mruImportMenu = new wxMenu;
	layerMenu->Append(ID_MRU_LAYER, _("Recent Layers"), mruLayerMenu);
	layerMenu->Append(ID_MRU_IMPORT, _("Recent Imports"), mruImportMenu);
	layerMenu->AppendSeparator();
	layerMenu->Append(ID_LAYER_IMPORTTIGER, _("Import Layers From TIGER"));
	layerMenu->Append(ID_LAYER_IMPORTOSM, _("Import Layers From OSM"),
		_("Import multiple layers from a OpenStreetMap .osm File"));
	layerMenu->Append(ID_LAYER_IMPORTNTF, _("Import Layers From NTF"),
		_("Import multiple layers from an OSGB NTF File"));
	layerMenu->Append(ID_LAYER_IMPORT_MS, _("Import From MapSource File"));
	layerMenu->Append(ID_LAYER_IMPORT_POINT, _("Import Point Data From Table"));
	layerMenu->Append(ID_LAYER_IMPORT_XML, _("Import Point Data From XML"));
	layerMenu->Append(ID_LAYER_IMPORT_DXF, _("Import Raw Layers from DXF"));
	layerMenu->AppendSeparator();
	layerMenu->Append(ID_LAYER_PROPS, _("Layer Properties"), _("Layer Properties"));
	layerMenu->Append(ID_EDIT_OFFSET, _("Offset Coordinates"), _("Offset"));
	layerMenu->AppendSeparator();
	layerMenu->Append(ID_LAYER_COMBINE, _("&Combine Layers"), _("Combine"));
	layerMenu->AppendSeparator();
	layerMenu->Append(ID_LAYER_CONVERTCRS, _("Convert Coordinate System"), _("Convert Coordinate System"));
	layerMenu->Append(ID_LAYER_SETCRS, _("Set Coordinate System"), _("Set Coordinate System"));
	m_pMenuBar->Append(layerMenu, _("&Layer"));
	menu_num++;

	// View
	viewMenu = new wxMenu;
	viewMenu->AppendCheckItem(ID_VIEW_SHOWLAYER, _("Current Layer &Visible"),
		_("Toggle Visibility of the current Layer"));
	viewMenu->Append(ID_VIEW_LAYER_UP, _("Move Layer &Up"));
	viewMenu->Append(ID_VIEW_LAYER_DOWN, _("Move Layer &Down"));
	viewMenu->AppendSeparator();
	viewMenu->Append(ID_VIEW_ZOOMIN, _("Zoom &In\tCtrl++"));
	viewMenu->Append(ID_VIEW_ZOOMOUT, _("Zoom Out\tCtrl+-"));
	viewMenu->Append(ID_VIEW_ZOOMALL, _("Zoom &All"));
	viewMenu->Append(ID_VIEW_ZOOM_LAYER, _("Zoom to Current &Layer"));
	viewMenu->Append(ID_VIEW_FULLVIEW, _("Zoom to &Full Res (1:1)"));
	viewMenu->Append(ID_VIEW_ZOOM_AREA, _("Zoom to Area Tool"));
	viewMenu->AppendSeparator();
	viewMenu->AppendCheckItem(ID_VIEW_TOOLBAR, _("Toolbar"));
	viewMenu->AppendCheckItem(ID_VIEW_LAYERS, _("Layers"));
	viewMenu->AppendSeparator();
	viewMenu->AppendCheckItem(ID_VIEW_MAGNIFIER, _("&Magnifier\tZ"));
	viewMenu->AppendCheckItem(ID_VIEW_PAN, _("&Pan\tSPACE"));
	viewMenu->AppendCheckItem(ID_VIEW_DISTANCE, _("Obtain &Distance"));
	viewMenu->AppendCheckItem(ID_VIEW_SETAREA, _("Area &Tool"));
	viewMenu->AppendSeparator();
	viewMenu->AppendCheckItem(ID_VIEW_WORLDMAP, _("&World Map"), _("Show/Hide World Map"));
	viewMenu->AppendCheckItem(ID_VIEW_SHOWUTM, _("Show &UTM Boundaries"));
//	viewMenu->AppendCheckItem(ID_VIEW_SHOWGRID, _("Show 7.5\" Grid"), _("Show 7.5\" Grid"), true);
	viewMenu->AppendCheckItem(ID_VIEW_PROFILE, _("Elevation Profile"));
	viewMenu->AppendCheckItem(ID_VIEW_SCALE_BAR, _("Scale Bar"));
	viewMenu->AppendSeparator();
	viewMenu->Append(ID_VIEW_OPTIONS, _("&Options"));
	m_pMenuBar->Append(viewMenu, _("&View"));
	menu_num++;

	// Roads
	roadMenu = new wxMenu;
	roadMenu->AppendCheckItem(ID_ROAD_SELECTROAD, _("Select/Modify Roads"));
	roadMenu->AppendCheckItem(ID_ROAD_SELECTNODE, _("Select/Modify Nodes"));
	roadMenu->AppendCheckItem(ID_ROAD_SELECTWHOLE, _("Select Whole Roads"));
	roadMenu->AppendCheckItem(ID_ROAD_DIRECTION, _("Set Road Direction"));
	roadMenu->AppendCheckItem(ID_ROAD_EDIT, _("Edit Road Points"));
	roadMenu->AppendSeparator();
	roadMenu->AppendCheckItem(ID_ROAD_SHOWNODES, _("Show Nodes"));
	roadMenu->AppendCheckItem(ID_ROAD_SELECTHWY, _("Select by Highway Number"));
	roadMenu->AppendSeparator();
	roadMenu->Append(ID_ROAD_CLEAN, _("Clean RoadMap"), _("Clean"));
	roadMenu->Append(ID_ROAD_GUESS, _("Guess Intersection Types"));
	m_pMenuBar->Append(roadMenu, _("&Roads"));
	m_iLayerMenu[LT_ROAD] = menu_num;
	menu_num++;

	// Utilities
	utilityMenu = new wxMenu;
	utilityMenu->AppendCheckItem(ID_TOWER_ADD, _("Add a Transmission Tower"));
	utilityMenu->AppendSeparator();
	utilityMenu->AppendCheckItem(ID_TOWER_SELECT, _("Select Utility Layer"));
	utilityMenu->AppendCheckItem(ID_TOWER_EDIT, _("Edit Transmission Towers"));
	m_pMenuBar->Append(utilityMenu, _("Util&ities"));
	m_iLayerMenu[LT_UTILITY] = menu_num;
	menu_num++;

	// Elevation
	elevMenu = new wxMenu;
	elevMenu->AppendCheckItem(ID_ELEV_SELECT, _("Se&lect Elevation Layer"));
	elevMenu->AppendSeparator();
	elevMenu->Append(ID_ELEV_SCALE, _("Sc&ale Elevation"));
	elevMenu->Append(ID_ELEV_VERT_OFFSET, _("Offset Elevation Vertically"));
	elevMenu->Append(ID_ELEV_REMOVERANGE, _("&Remove Elevation Range..."));
	elevMenu->Append(ID_ELEV_ARITHMETIC, _("&Create Layer from Arithmetic"));

	wxMenu *fillMenu = new wxMenu;
	fillMenu->Append(ID_ELEV_FILL_FAST, _("Fast"));
	fillMenu->Append(ID_ELEV_FILL_SLOW, _("Slow and smooth"));
	fillMenu->Append(ID_ELEV_FILL_REGIONS, _("Extrapolation via partial derivatives"));

	elevMenu->Append(0, _("&Fill In Unknown Areas"), fillMenu);

	elevMenu->Append(ID_ELEV_SETUNKNOWN, _("&Set Unknown Areas"));
	elevMenu->AppendSeparator();
	elevMenu->Append(ID_ELEV_EXPORT, _("E&xport To..."));
	elevMenu->Append(ID_ELEV_EXPORT_TILES, _("Export to libMini tileset..."));
	elevMenu->Append(ID_ELEV_BITMAP, _("Re&nder to Bitmap..."));
	elevMenu->Append(ID_ELEV_TOTIN, _("Convert Grid to TIN"));
	elevMenu->Append(ID_ELEV_CONTOURS, _("Generate Contours"));
	elevMenu->Append(ID_ELEV_CARVE, _("Carve Grid with Culture"));
	elevMenu->AppendSeparator();
	elevMenu->Append(ID_ELEV_MERGETIN, _("&Merge shared TIN vertices"));
	elevMenu->AppendCheckItem(ID_ELEV_TRIMTIN, _("Trim TIN triangles by line segment"));
	m_pMenuBar->Append(elevMenu, _("Eleva&tion"));
	m_iLayerMenu[LT_ELEVATION] = menu_num;
	menu_num++;

	// Imagery
	imgMenu = new wxMenu;
	imgMenu->Append(ID_IMAGE_REPLACE_RGB, _("Replace RGB..."));
	imgMenu->Append(ID_IMAGE_CREATE_OVERVIEWS, _("Create Overviews on Disk"));
	imgMenu->Append(ID_IMAGE_CREATE_OVER_ALL, _("Create Overviews on Disk for All Images"));
	imgMenu->Append(ID_IMAGE_CREATE_MIPMAPS, _("Create Overviews in Memory"));
	//imgMenu->Append(ID_IMAGE_LOAD_MIPMAPS, _("Load Overviews into Memory"));
	imgMenu->AppendSeparator();
	imgMenu->Append(ID_IMAGE_EXPORT_TILES, _("Export to libMini tileset..."));
	imgMenu->Append(ID_IMAGE_EXPORT_PPM, _("Export to PPM"));
	m_pMenuBar->Append(imgMenu, _("Imagery"));
	m_iLayerMenu[LT_IMAGE] = menu_num;
	menu_num++;

	// Vegetation
	vegMenu = new wxMenu;
	vegMenu->Append(ID_VEG_PLANTS, _("Species List"), _("View/Edit list of available plant species"));
	vegMenu->Append(ID_VEG_BIOREGIONS, _("BioRegions"), _("View/Edit list of species & density for each BioRegion"));
	vegMenu->AppendSeparator();
	vegMenu->Append(ID_VEG_REMAP, _("Remap Species"));
	vegMenu->Append(ID_VEG_EXPORTSHP, _("Export SHP"));
	vegMenu->Append(ID_VEG_HTML, _("Write species to HTML"));
	m_pMenuBar->Append(vegMenu, _("Veg&etation"));
	m_iLayerMenu[LT_VEG] = menu_num;
	menu_num++;

	// Structures
	bldMenu = new wxMenu;
	bldMenu->AppendCheckItem(ID_FEATURE_SELECT, _("Select Features"));
	bldMenu->AppendCheckItem(ID_STRUCTURE_EDIT_BLD, _("Edit Buildings"));
	bldMenu->AppendCheckItem(ID_STRUCTURE_ADD_POINTS, _("Add points to building footprints"), _T(""));
	bldMenu->AppendCheckItem(ID_STRUCTURE_DELETE_POINTS, _("Delete points from building footprints"), _T(""));
	bldMenu->AppendCheckItem(ID_STRUCTURE_ADD_LINEAR, _("Add Linear Structures"));
	bldMenu->AppendCheckItem(ID_STRUCTURE_EDIT_LINEAR, _("Edit Linear Structures"));
	bldMenu->AppendCheckItem(ID_STRUCTURE_ADD_INST, _("Add Instances"));
	bldMenu->AppendSeparator();
	bldMenu->Append(ID_STRUCTURE_ADD_FOUNDATION, _("Add Foundation Levels to Buildings"), _T(""));
	bldMenu->Append(ID_STRUCTURE_SELECT_USING_POLYGONS, _("Select Using Polygons"), _("Select buildings using selected raw layer polygons"));
	bldMenu->Append(ID_STRUCTURE_COLOUR_SELECTED_ROOFS, _("Colour Selected Roofs"), _("Set roof colour on selected buildings"));
	bldMenu->Append(ID_STRUCTURE_CLEAN_FOOTPRINTS, _("Clean Footprints"), _("Clean up degenerate footprint geometry"));
	bldMenu->Append(ID_STRUCTURE_SELECT_INDEX, _("Select structure by index"));
	bldMenu->AppendSeparator();
	bldMenu->Append(ID_STRUCTURE_EXPORT_FOOTPRINTS, _("Export footprints to SHP"));
	bldMenu->Append(ID_STRUCTURE_EXPORT_CANOMA, _("Export footprints to Canoma3DV"));

	bldMenu->AppendSeparator();
	bldMenu->AppendCheckItem(ID_STRUCTURE_CONSTRAIN, _("Constrain angles on footprint edit"));
	m_pMenuBar->Append(bldMenu, _("&Structures"));
	m_iLayerMenu[LT_STRUCTURE] = menu_num;
	menu_num++;

	// Raw
	rawMenu = new wxMenu;
	rawMenu->AppendCheckItem(ID_FEATURE_SELECT, _("Select Features"));
	rawMenu->AppendCheckItem(ID_FEATURE_PICK, _("Pick Features"));
	rawMenu->AppendCheckItem(ID_FEATURE_TABLE, _("Show Attribute Table"));
	rawMenu->AppendSeparator();
	rawMenu->Append(ID_RAW_SETTYPE, _("Set Entity Type"), _("Set Entity Type"));
	rawMenu->AppendCheckItem(ID_RAW_ADDPOINTS, _("Add Points with Mouse"));
	rawMenu->Append(ID_RAW_ADDPOINT_TEXT, _("Add Point with Text\tCtrl+T"), _("Add point"));
	rawMenu->Append(ID_RAW_ADDPOINTS_GPS, _("Add Points with GPS"), _("Add points with GPS"));
	rawMenu->Append(ID_RAW_ADDFEATURE_WKT, _("Add Feature from WKT"), _("Add Feature from WKT"));
	rawMenu->Append(ID_RAW_STYLE, _("Style..."));
	rawMenu->Append(ID_RAW_SCALE_H, _("Scale horizontally"));
	rawMenu->Append(ID_RAW_SCALE_V, _("Scale vertically"));
	rawMenu->Append(ID_RAW_OFFSET_V, _("Offset vertically"));
	rawMenu->Append(ID_RAW_CLEAN, _("Clean polygon geometry"));
	rawMenu->Append(ID_RAW_SELECT_BAD, _("Select bad geometry"));
	rawMenu->AppendSeparator();
	rawMenu->Append(ID_RAW_SELECTCONDITION, _("Select Features by Condition"));
	rawMenu->Append(ID_RAW_EXPORT_IMAGEMAP, _("Export as HTML ImageMap"));
	rawMenu->Append(ID_RAW_EXPORT_KML, _("Export as KML"));
	rawMenu->Append(ID_RAW_GENERATE_ELEVATION, _("Generate Grid from 3D Points"));
	rawMenu->Append(ID_RAW_GENERATE_TIN, _("Generate TIN"));
	rawMenu->Append(ID_RAW_CONVERT_TOPOLYS, _("Generate Polygons from Polylines"));
	m_pMenuBar->Append(rawMenu, _("Ra&w"));
	m_iLayerMenu[LT_RAW] = menu_num;
	menu_num++;

	// Area
	areaMenu = new wxMenu;
	areaMenu->Append(ID_AREA_CLEAR, _("Clear (Set to zero)"));
	areaMenu->Append(ID_AREA_ZOOM_ALL, _("Set to Full Extents"),
		_("Set the Area Tool rectangle to the combined extent of all layers."));
	areaMenu->Append(ID_AREA_ZOOM_LAYER, _("Set to Layer Extents"),
		_("Set the Area Tool rectangle to the extent of the active layer."));
	areaMenu->Append(ID_AREA_TYPEIN, _("Numeric Values"),
		_("Set the Area Tool rectangle by text entry of coordinates."));
	areaMenu->Append(ID_AREA_MATCH, _("Match Area and Tiling to Layer"),
		_("Set the Area Tool rectangle by matching the resolution of a layer."));
	areaMenu->AppendSeparator();
	areaMenu->Append(ID_AREA_SAMPLE_ELEV, _("Sample &Elevation"),
		_("Sample all elevation data within the Area Tool to produce a single, new elevation."));
	areaMenu->Append(ID_AREA_SAMPLE_IMAGE, _("Sample &Imagery"),
		_("Sample imagery within the Area Tool to produce a single, new image."));
	areaMenu->Append(ID_AREA_GENERATE_VEG, _("Generate Vegetation"),
		_("Generate Vegetation File (*.vf) containing plant distribution."));
	areaMenu->Append(ID_AREA_VEG_DENSITY, _("Compute Vegetation Density"),
		_("Compute and display the density of each species of vegetation in the given area."));
#if SUPPORT_CURL
	areaMenu->Append(ID_AREA_REQUEST_WFS, _("Request Layer from WFS"));
	areaMenu->Append(ID_AREA_REQUEST_WMS, _("Request Image from WMS"));
#endif // SUPPORT_CURL
	areaMenu->AppendSeparator();
	areaMenu->Append(ID_AREA_SAMPLE_ELEV_OPT, _("Sample Elevation to Tileset"),
		_("Sample all elevation data within the Area Tool efficiently to produce an elevation tileset."));
	areaMenu->Append(ID_AREA_SAMPLE_IMAGE_OPT, _("Sample Imagery to Tileset"),
		_("Sample all image data within the Area Tool efficiently to produce an image tileset."));
	m_pMenuBar->Append(areaMenu, _("&Area Tool"));
	menu_num++;

	// Help
	helpMenu = new wxMenu;
	wxString msg = _("About ");
	msg += wxString("VTBuilder", wxConvUTF8);
#ifdef __WXMAC__
#endif
	helpMenu->Append(wxID_HELP, _("&About"), msg);
	helpMenu->Append(ID_HELP_DOC_LOCAL, _("Documentation (local)"), msg);
	helpMenu->Append(ID_HELP_DOC_ONLINE, _("Documentation (on the web)"), msg);
	m_pMenuBar->Append(helpMenu, _("&Help"));
#ifdef __WXMAC__
	wxApp::s_macAboutMenuItemId = wxID_HELP;
	wxApp::s_macHelpMenuTitleName = _("&Help");
#endif
	menu_num++;

	SetMenuBar(m_pMenuBar);
}

void MainFrame::UpdateMRU(wxMenu *menu, const vtStringArray &files, int first_id)
{
	while (menu->GetMenuItemCount() > 0)
		menu->Delete(menu->FindItemByPosition(0));
	for (size_t i = 0; i < files.size(); i++)
		menu->Append(first_id+i, wxString(files[i], wxConvUTF8), wxEmptyString);
}


////////////////////////////////////////////////////////////////
// Project menu

void MainFrame::OnProjectNew(wxCommandEvent &event)
{
	SetActiveLayer(NULL);
	DeleteContents();
	m_area.SetRect(0.0, 0.0, 0.0, 0.0);
	m_pView->Refresh();
	Refresh();

	// reset veg too
	m_strSpeciesFilename = "";
	m_strBiotypesFilename = "";
	m_SpeciesList.Clear();
	m_BioRegion.Clear();

	RefreshTreeView();
	RefreshToolbars();

	vtCRS p;
	SetCRS(p);
}

wxString GetProjectFilter()
{
	return FSTRING_VTB;
}

void MainFrame::OnProjectOpen(wxCommandEvent &event)
{
	wxFileDialog loadFile(NULL, _("Load Project"), _T(""), _T(""),
		GetProjectFilter(), wxFD_OPEN);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	LoadProject(loadFile.GetPath());
}

void MainFrame::OnProjectSave(wxCommandEvent &event)
{
	wxFileDialog saveFile(NULL, _("Save Project"), _T(""), _T(""),
		GetProjectFilter(), wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;
	wxString strPathName = saveFile.GetPath();

	SaveProject(strPathName);

	// Add saved projects to the MRU list
	AddToMRU(m_ProjectFiles, (const char *) strPathName.mb_str(wxConvUTF8));
}

void MainFrame::OnProjectPrefs(wxCommandEvent &event)
{
	PrefDlg dlg(this, wxID_ANY, _("Preferences"));
	dlg.b1 = (g_Options.GetValueBool(TAG_USE_CURRENT_CRS) == true);
	dlg.b2 = (g_Options.GetValueBool(TAG_USE_CURRENT_CRS) == false);

	dlg.b3 = g_Options.GetValueBool(TAG_LOAD_IMAGES_ALWAYS);
	dlg.b4 = g_Options.GetValueBool(TAG_LOAD_IMAGES_NEVER);
	dlg.b5 = (!g_Options.GetValueBool(TAG_LOAD_IMAGES_ALWAYS) &&
			  !g_Options.GetValueBool(TAG_LOAD_IMAGES_NEVER));

	dlg.b6 = g_Options.GetValueBool(TAG_REPRO_TO_FLOAT_NEVER);
	dlg.b7 = g_Options.GetValueBool(TAG_REPRO_TO_FLOAT_ALWAYS);
	dlg.b8 = (!g_Options.GetValueBool(TAG_REPRO_TO_FLOAT_ALWAYS) &&
			  !g_Options.GetValueBool(TAG_REPRO_TO_FLOAT_NEVER));

	dlg.b9 =  (g_Options.GetValueInt(TAG_GAP_FILL_METHOD) == 1);
	dlg.b10 = (g_Options.GetValueInt(TAG_GAP_FILL_METHOD) == 2);
	dlg.b11 = (g_Options.GetValueInt(TAG_GAP_FILL_METHOD) == 3);

	dlg.b12 = g_Options.GetValueBool(TAG_BLACK_TRANSP);
	dlg.b13 = g_Options.GetValueBool(TAG_TIFF_COMPRESS);
	dlg.b14 = g_Options.GetValueBool(TAG_DEFAULT_GZIP_BT);
	dlg.b15 = g_Options.GetValueBool(TAG_DELAY_LOAD_GRID);
	dlg.i1 =  g_Options.GetValueInt(TAG_SAMPLING_N);
	dlg.i2 =  g_Options.GetValueInt(TAG_MAX_MEGAPIXELS);
	dlg.i3 =  g_Options.GetValueInt(TAG_ELEV_MAX_SIZE);
	if (dlg.b15)
		dlg.i4 =  g_Options.GetValueInt(TAG_MAX_MEM_GRID);
	else
		dlg.i4 =  128;

	dlg.TransferDataToWindow();

	if (dlg.ShowModal() == wxID_OK)
	{
		g_Options.SetValueBool(TAG_USE_CURRENT_CRS, dlg.b1);
		g_Options.SetValueBool(TAG_LOAD_IMAGES_ALWAYS, dlg.b3);
		g_Options.SetValueBool(TAG_LOAD_IMAGES_NEVER, dlg.b4);
		g_Options.SetValueBool(TAG_REPRO_TO_FLOAT_ALWAYS, dlg.b7);
		g_Options.SetValueBool(TAG_REPRO_TO_FLOAT_NEVER, dlg.b6);

		if (dlg.b9)  g_Options.SetValueInt(TAG_GAP_FILL_METHOD, 1);
		if (dlg.b10) g_Options.SetValueInt(TAG_GAP_FILL_METHOD, 2);
		if (dlg.b11) g_Options.SetValueInt(TAG_GAP_FILL_METHOD, 3);

		g_Options.SetValueBool(TAG_BLACK_TRANSP, dlg.b12);
		g_Options.SetValueBool(TAG_TIFF_COMPRESS, dlg.b13);
		g_Options.SetValueBool(TAG_DEFAULT_GZIP_BT, dlg.b14);
		g_Options.SetValueBool(TAG_DELAY_LOAD_GRID, dlg.b15);
		g_Options.SetValueInt(TAG_SAMPLING_N, dlg.i1);
		g_Options.SetValueInt(TAG_MAX_MEGAPIXELS, dlg.i2);
		g_Options.SetValueInt(TAG_ELEV_MAX_SIZE, dlg.i3);
		g_Options.SetValueInt(TAG_MAX_MEM_GRID, dlg.i4);

		vtImage::bTreatBlackAsTransparent = dlg.b11;
		vtElevLayer::m_bDefaultGZip = dlg.b13;
		if (dlg.b15)
			vtElevLayer::m_iElevMemLimit = dlg.i4;
		else
			vtElevLayer::m_iElevMemLimit = -1;

		// safety checks
		CheckOptionBounds();
	}
}

void MainFrame::OnElevFlip(wxCommandEvent &event)
{
	vtElevLayer *t = GetActiveElevLayer();
	if (!t || !t->GetGrid())
		return;

	// Quick and dirty flip code.  Yes, this could be optimized.
	vtElevationGrid *grid = t->GetGrid();
	IPoint2 dim = grid->GetDimensions();
	for (int j = 0; j < dim.y / 2; j++)
	{
		for (int i = 0; i < dim.x; i++)
		{
			float f = grid->GetFValue(i, j);
			grid->SetFValue(i, j, grid->GetFValue(i, dim.y - 1 - j));
			grid->SetFValue(i, dim.y - 1 - j, f);
		}
	}
	t->SetModified(true);
	t->ReRender();
	m_pView->Refresh();
}

void MainFrame::OnBatchConvert(wxCommandEvent &event)
{
	wxArrayString aChoices;
	aChoices.push_back(_("Import elevation grid data, write BT"));
	aChoices.push_back(_("Import 3D point data, produce a TIN and write ITF"));

	int result = wxGetSingleChoiceIndex(_("Choose operation:"),
		_T("Batch processing"), aChoices, this);

	if (result == -1)
		return;

	wxString dir1 = wxDirSelector(_("Choose directory that has the input files"),
		_T(""), 0, wxDefaultPosition, this);
	if (dir1 == _T(""))
		return;

	wxString dir2 = wxDirSelector(_("Choose directory for output files"),
		dir1, 0, wxDefaultPosition, this);
	if (dir2 == _T(""))
		return;

	std::string path1 = (const char *) dir1.mb_str(wxConvUTF8);
	std::string path2 = (const char *) dir2.mb_str(wxConvUTF8);
	wxString msg;
	int succeeded = 0;

	int count = 0, total = 0;
	for (dir_iter it(path1); it != dir_iter(); ++it)
		total ++;

	OpenProgressDialog2(_T("Processing"), true, this);
	// some paths are long, and the progress dialog doesn't automatically widen
	SetProgressDialog2Width(600);

	for (dir_iter it(path1); it != dir_iter(); ++it)
	{
		if (it.is_hidden() || it.is_directory())
			continue;
		std::string name1 = path1 + "/" + it.filename();

		// progress
		count++;
		msg.Printf(_T("%d: Read "), count);
		msg += wxString(name1.c_str(), wxConvUTF8);
		if (UpdateProgressDialog2(count * 99 / total, 0, msg))
			break;	// cancel

		if (result == 0)
		{
			bool bGZip = false;

			msg.Printf(_T("%d: Import from %hs"), count, name1.c_str());
			if (UpdateProgressDialog2(count * 99 / total, 0, msg))
				break;	// cancel

			vtElevationGrid grid;
			if (!grid.LoadFromFile(name1.c_str(), progress_callback_minor))
				break;

			vtString name2 = path2.c_str();
			name2 += "/";
			name2 += it.filename().c_str();
			RemoveFileExtensions(name2);
			name2 += ".bt";

			grid.SaveToBT(name2, progress_callback_minor, bGZip);
		}
		else if (result == 1)
		{
			vtFeatureSet *pSet = g_bld->ImportPointsFromXYZ(name1.c_str(), progress_callback_minor);
			if (!pSet)
				continue;
			vtFeatureSetPoint3D *setpo3 = dynamic_cast<vtFeatureSetPoint3D *>(pSet);
			if (!setpo3)
				continue;

			msg.Printf(_T("%d: Creating TIN"), count);
			if (UpdateProgressDialog2(count * 99 / total, 0, msg))
				break;	// cancel

			// points -> TIN algorithm -> TIN
			vtTin2d *tin = new vtTin2d(setpo3);

			// inherit CRS from application
			vtCRS crs;
			g_bld->GetCRS(crs);
			tin->m_crs = crs;

			vtElevLayer *pEL = new vtElevLayer;
			pEL->SetTin(tin);

			// inherit name
			wxString output_name = dir2;
			output_name += _T("/");
			output_name += wxString(it.filename().c_str(), wxConvUTF8);
			RemoveFileExtensions(output_name);
			output_name += _T(".itf");

			// progress
			msg.Printf(_T("%d: Write "), count);
			msg += output_name;
			if (UpdateProgressDialog2(count * 99 / total, 0, msg))
				break;	// cancel

			bool success = pEL->SaveAs(output_name, progress_callback_minor);
			if (success)
				succeeded ++;

			// clean up
			delete pEL;
			delete pSet;
		}
	}
	msg.Printf(_T("Successfully wrote %d files"), succeeded);
	wxMessageBox(msg, _T(""), 4|wxCENTRE, this);

	CloseProgressDialog2();
}

void MainFrame::OnDymaxTexture(wxCommandEvent &event)
{
	DoDymaxTexture();
}

void MainFrame::OnDymaxMap(wxCommandEvent &event)
{
	DoDymaxMap();
}

void MainFrame::OnProcessBillboard(wxCommandEvent &event)
{
	DoProcessBillboard();
}

void MainFrame::OnGeocode(wxCommandEvent &event)
{
	DoGeocode();
}

void MainFrame::OnRunTest(wxCommandEvent &event)
{
	m_pView->RunTest();
}

void MainFrame::OnQuit(wxCommandEvent &event)
{
	Close(FALSE);
}

void MainFrame::OnUpdateFileMRU(wxUpdateUIEvent& event)
{
	UpdateMRU(mruMenu, m_ProjectFiles, ID_FIRST_MRU_FILE);
	event.Enable(m_ProjectFiles.size() > 0);
}


//////////////////////////////////////////////////
// Edit menu

void MainFrame::OnEditDelete(wxCommandEvent &event)
{
	vtRoadLayer *pRL = GetActiveRoadLayer();
	if (pRL && (pRL->NumSelectedNodes() != 0 || pRL->NumSelectedLinks() != 0))
	{
		wxString str;
		str.Printf(_("Deleting road selection: %d nodes and %d roads"),
			pRL->NumSelectedNodes(), pRL->NumSelectedLinks());
		SetStatusText(str);
		m_pView->DeleteSelected(pRL);
		pRL->SetModified(true);
		return;
	}
	vtStructureLayer *pSL = GetActiveStructureLayer();
	if (pSL && pSL->NumSelected() != 0)
	{
		pSL->DeleteSelected();
		pSL->SetModified(true);
		m_pView->Refresh();
		return;
	}
	vtRawLayer *pRawL = GetActiveRawLayer();
	if (pRawL)
	{
		vtFeatureSet *pSet = pRawL->GetFeatureSet();
		if (pSet && pSet->NumSelected() != 0)
		{
			pSet->DeleteSelected();
			pRawL->SetModified(true);
			m_pView->Refresh();
			OnSelectionChanged();
			return;
		}
	}

	vtLayer *pL = GetActiveLayer();
	if (pL)
	{
		int result = wxMessageBox(_("Are you sure you want to delete the current layer?"),
				_("Question"), wxYES_NO | wxICON_QUESTION, this);
		if (result == wxYES)
			RemoveLayer(pL);
	}
}

void MainFrame::OnUpdateEditDelete(wxUpdateUIEvent& event)
{
	event.Enable(GetActiveLayer() != NULL);
}

void MainFrame::OnEditDeselectAll(wxCommandEvent &event)
{
	m_pView->DeselectAll();
}

void MainFrame::OnEditInvertSelection(wxCommandEvent &event)
{
	vtRoadLayer *pRL = GetActiveRoadLayer();
	if (pRL) {
		pRL->InvertSelection();
		m_pView->Refresh(false);
	}
	vtStructureLayer *pSL = GetActiveStructureLayer();
	if (pSL) {
		pSL->InvertSelection();
		m_pView->Refresh(false);
	}
	vtRawLayer *pRawL = GetActiveRawLayer();
	if (pRawL) {
		pRawL->GetFeatureSet()->InvertSelection();
		m_pView->Refresh(false);
		OnSelectionChanged();
	}
}

void MainFrame::OnEditCrossingSelection(wxCommandEvent &event)
{
	m_pView->m_bCrossSelect = !m_pView->m_bCrossSelect;
}

void MainFrame::OnUpdateCrossingSelection(wxUpdateUIEvent& event)
{
	event.Check(m_pView->m_bCrossSelect);
}

void MainFrame::OnEditOffset(wxCommandEvent &event)
{
	wxTextEntryDialog dlg(this, _("Offset"),
		_("Please enter horizontal offset X, Y"), _T("0, 0"));
	if (dlg.ShowModal() != wxID_OK)
		return;

	DPoint2 offset;
	wxString str = dlg.GetValue();
	sscanf(str.mb_str(wxConvUTF8), "%lf, %lf", &offset.x, &offset.y);

	GetActiveLayer()->Offset(offset);
	GetActiveLayer()->SetModified(true);
	m_pView->Refresh();
}

void MainFrame::OnUpdateEditOffset(wxUpdateUIEvent& event)
{
	event.Enable(GetActiveLayer() != NULL);
}


//////////////////////////////////////////////////
// Layer menu

void MainFrame::OnLayerNew(wxCommandEvent &event)
{
	LayerType lt = AskLayerType();
	if (lt == LT_UNKNOWN)
		return;

	vtLayer *pL = vtLayer::CreateNewLayer(lt);
	if (!pL)
		return;

	if (lt == LT_ELEVATION)
	{
		vtElevLayer *pEL = (vtElevLayer *)pL;
		vtElevationGrid *grid = new vtElevationGrid(m_area, IPoint2(1025, 1025),
			false, m_crs);
		grid->FillWithSingleValue(1000);
		pEL->SetGrid(grid);
	}
	else
	{
		pL->SetCRS(m_crs);
	}

	SetActiveLayer(pL);
	m_pView->SetActiveLayer(pL);
	AddLayer(pL);
	RefreshTreeView();
	RefreshToolbars();
	RefreshView();
}

void MainFrame::OnLayerOpen(wxCommandEvent &event)
{
	wxString filter = _("Native Layer Formats|");

	AddType(filter, FSTRING_BT);	// elevation
	AddType(filter, FSTRING_BTGZ);	// compressed elevation
	AddType(filter, FSTRING_TIN);	// elevation
	AddType(filter, FSTRING_RMF);	// roads
	AddType(filter, FSTRING_GML);	// raw
	AddType(filter, FSTRING_UTL);	// utility towers
	AddType(filter, FSTRING_VTST);	// structures
	AddType(filter, FSTRING_VTSTGZ);// compressed structures
	AddType(filter, FSTRING_VF);	// vegetation files
	AddType(filter, FSTRING_TIF);	// image files
	AddType(filter, FSTRING_IMG);	// image or elevation file
	AddType(filter, FSTRING_SHP);	// raw files

	// ask the user for a filename, allow multiple select
	wxFileDialog loadFile(NULL, _("Open Layer"), _T(""), _T(""), filter,
		wxFD_OPEN | wxFD_MULTIPLE);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	wxArrayString Paths;
	loadFile.GetPaths(Paths);

	for (size_t i = 0; i < Paths.GetCount(); i++)
	{
		// if succeeded, will be added to the MRU
		LoadLayer(Paths[i]);
	}
}

void MainFrame::OnLayerSave(wxCommandEvent &event)
{
	vtLayer *lp = GetActiveLayer();
	if (lp->GetLayerFilename().Left(8).CmpNoCase(_("Untitled")) == 0)
	{
		if (!lp->AskForSaveFilename())
			return;
	}
	wxString msg = _("Saving layer to file ") + lp->GetLayerFilename();
	SetStatusText(msg);
	VTLOG(msg.mb_str(wxConvUTF8));
	VTLOG("\n");

	if (lp->Save())
		msg = _("Saved layer to file ") + lp->GetLayerFilename();
	else
		msg = _("Save failed.");
	SetStatusText(msg);
	VTLOG(msg.mb_str(wxConvUTF8));
	VTLOG("\n");
}

void MainFrame::OnUpdateLayerSave(wxUpdateUIEvent& event)
{
	vtLayer *lp = GetActiveLayer();
	event.Enable(lp != NULL && lp->GetModified() && lp->CanBeSaved());
}

void MainFrame::OnLayerSaveAs(wxCommandEvent &event)
{
	vtLayer *lp = GetActiveLayer();

	if (!lp->AskForSaveFilename())
		return;

	wxString fname = lp->GetLayerFilename();
	wxString msg = _("Saving layer to file as ") + fname;
	SetStatusText(msg);

	VTLOG1(msg.mb_str(wxConvUTF8));
	VTLOG1("\n");

	bool success = lp->Save();
	if (success)
	{
		lp->SetModified(false);
		msg = _("Saved layer to file as ") + fname;

		// Add newly-saved layers to the MRU list
		AddToMRU(m_LayerFiles, (const char *) fname.mb_str(wxConvUTF8));
	}
	else
	{
		msg = _("Failed to save layer to ") + fname;
		wxMessageBox(msg, _("Problem"));
	}
	SetStatusText(msg);

	VTLOG1(msg.mb_str(wxConvUTF8));
	VTLOG1("\n");
}

void MainFrame::OnUpdateLayerSaveAs(wxUpdateUIEvent& event)
{
	vtLayer *lp = GetActiveLayer();
	event.Enable(lp != NULL && lp->CanBeSaved());
}

void MainFrame::OnUpdateMRULayer(wxUpdateUIEvent& event)
{
	UpdateMRU(mruLayerMenu, m_LayerFiles, ID_FIRST_MRU_LAYER);
	event.Enable(m_LayerFiles.size() > 0);
}

void MainFrame::OnUpdateMRUImport(wxUpdateUIEvent& event)
{
	UpdateMRU(mruImportMenu, m_ImportFiles, ID_FIRST_MRU_IMPORT);
	event.Enable(m_ImportFiles.size() > 0);
}

void MainFrame::OnUpdateLayerProperties(wxUpdateUIEvent& event)
{
	event.Enable(GetActiveLayer() != NULL);
}

void MainFrame::OnLayerImport(wxCommandEvent &event)
{
	LayerType lt;

	// first ask what kind of data layer
	lt = AskLayerType();
	if (lt == LT_UNKNOWN)
		return;

	ImportData(lt);
}

void MainFrame::OnLayerImportTIGER(wxCommandEvent &event)
{
	// Ask the user for a directory
	wxDirDialog getDir(NULL, _("Import TIGER Data From Directory"));

	if (getDir.ShowModal() == wxID_OK)
		ImportDataFromTIGER(getDir.GetPath());
}

void MainFrame::OnLayerImportOSM(wxCommandEvent &event)
{
	wxFileDialog loadFile(NULL, _("Import Layers from OpenStreetMap File"),
		_T(""), _T(""), FSTRING_OSM, wxFD_OPEN);

	if (loadFile.ShowModal() != wxID_OK)
		return;

	OpenProgressDialog(_("Importing from OpenStreetMap"), loadFile.GetPath(),
		false, m_pParentWindow);
	UpdateProgressDialog(0, loadFile.GetPath());

	LayerArray layers;
	layers.SetOwnership(false);
	ImportDataFromOSM(loadFile.GetPath(), layers, progress_callback);

	CloseProgressDialog();

	for (uint i = 0; i < layers.size(); i++)
		AddLayerWithCheck(layers[i], true);
}

void MainFrame::OnLayerImportNTF(wxCommandEvent &event)
{
	wxFileDialog loadFile(NULL, _("Import Layers from NTF File"),
		_T(""), _T(""), FSTRING_NTF, wxFD_OPEN);

	if (loadFile.ShowModal() != wxID_OK)
		return;

	LayerArray layers;
	layers.SetOwnership(false);
	ImportDataFromNTF(loadFile.GetPath(), layers);
	for (uint i = 0; i < layers.size(); i++)
		AddLayerWithCheck(layers[i], true);
}

//
// Import from a Garmin MapSource GPS export file (.txt)
//
void MainFrame::OnLayerImportMapSource(wxCommandEvent &event)
{
	wxFileDialog loadFile(NULL, _("Import MapSource File"), _T(""), _T(""),
		_("MapSource Export Files (*.txt)|*.txt"), wxFD_OPEN);

	if (loadFile.ShowModal() != wxID_OK)
		return;

	wxString str = loadFile.GetPath();
	ImportFromMapSource(str.mb_str(wxConvUTF8));
}

void MainFrame::OnLayerImportPoint(wxCommandEvent &event)
{
	wxString filter = _("Tabular Data Files|");

	AddType(filter, FSTRING_DBF);	// old-style database
	AddType(filter, FSTRING_CSV);	// comma-separated values
	AddType(filter, FSTRING_XYZ);	// space-separated X Y Z
	AddType(filter, FSTRING_ENZI);	// space-separated East North Z i

	wxFileDialog loadFile(NULL, _("Import Point Data"), _T(""), _T(""),
		filter, wxFD_OPEN);

	if (loadFile.ShowModal() != wxID_OK)
		return;

	OpenProgressDialog(_T("Importing"), loadFile.GetPath());

	wxString str = loadFile.GetPath();
	ImportDataPointsFromTable(str.mb_str(wxConvUTF8), progress_callback);

	CloseProgressDialog();
}

void MainFrame::OnLayerImportXML(wxCommandEvent &event)
{
	wxFileDialog loadFile(NULL, _("Import XML Data"), _T(""), _T(""),
		FSTRING_XML, wxFD_OPEN);

	if (loadFile.ShowModal() != wxID_OK)
		return;

	wxString str = loadFile.GetPath();
	vtRawLayer *pRL = new vtRawLayer;
	if (pRL->ImportFromXML(str.mb_str(wxConvUTF8)))
	{
		pRL->SetLayerFilename(str);
		pRL->SetModified(true);

		if (!AddLayerWithCheck(pRL, true))
			delete pRL;
	}
	else
		delete pRL;
}

void MainFrame::OnLayerImportDXF(wxCommandEvent &event)
{
	wxFileDialog loadFile(NULL, _("Import DXF Data"), _T(""), _T(""),
		FSTRING_DXF, wxFD_OPEN);

	if (loadFile.ShowModal() != wxID_OK)
		return;

	wxString str = loadFile.GetPath();
	ImportDataFromDXF(str.mb_str(wxConvUTF8));
}

void MainFrame::OnLayerProperties(wxCommandEvent &event)
{
	vtLayer *lp = GetActiveLayer();
	if (lp)
		ShowLayerProperties(lp);
}

void MainFrame::ShowLayerProperties(vtLayer *lp)
{
	// All layers have some common properties, others are specific to the
	//  type of layer.
	LayerType ltype = lp->GetType();

	wxString title;
	title += vtLayer::LayerTypeNames[ltype];
	title += _(" Layer Properties");
	LayerPropDlg dlg(NULL, -1, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

	// Fill in initial values for the dialog
	DRECT rect, rect2;
	lp->GetExtent(rect);
	dlg.m_fLeft = rect.left;
	dlg.m_fTop = rect.top;
	dlg.m_fRight = rect.right;
	dlg.m_fBottom = rect.bottom;

	lp->GetPropertyText(dlg.m_strText);

	// For elevation and image layers, if the user changes the extents, apply.
	if (dlg.ShowModal() != wxID_OK)
		return;

	rect2.left = dlg.m_fLeft;
	rect2.top = dlg.m_fTop;
	rect2.right = dlg.m_fRight;
	rect2.bottom = dlg.m_fBottom;
	if (rect2 != rect)
	{
		// user changed the extents
		if (lp->SetExtent(rect2))
		{
			wxMessageBox(_("Changed extents."));
			m_pView->Refresh();
		}
		else
			wxMessageBox(_("Could not change extents."));
	}
}

void MainFrame::OnAreaSampleElev(wxCommandEvent &event)
{
	AreaSampleElevation(m_pView);
}

void MainFrame::OnAreaSampleElevTileset(wxCommandEvent &event)
{
	AreaSampleElevTileset(m_pView);
}

void MainFrame::OnAreaSampleImageTileset(wxCommandEvent &event)
{
	AreaSampleImageTileset(m_pView);
}

void MainFrame::OnUpdateAreaSampleElev(wxUpdateUIEvent& event)
{
	event.Enable(LayersOfType(LT_ELEVATION) > 0 && !m_area.IsEmpty());
}

void MainFrame::OnAreaSampleImage(wxCommandEvent &event)
{
	AreaSampleImages(m_pView);
}

void MainFrame::OnUpdateAreaSampleImage(wxUpdateUIEvent& event)
{
	event.Enable(LayersOfType(LT_IMAGE) > 0 && !m_area.IsEmpty());
}

void MainFrame::OnLayerConvertCRS(wxCommandEvent &event)
{
	// Ask for what CRS to convert to
	ProjectionDlg dlg(NULL, 200, _("Convert to what coordinate system?"));
	dlg.SetCRS(m_crs);

	// They might switch to UTM, so help provide a good guess for UTM zone.
	dlg.SetGeoRefPoint(EstimateGeoDataCenter());

	if (dlg.ShowModal() == wxID_CANCEL)
		return;
	vtCRS crs;
	dlg.GetCRS(crs);

	// Count through the layer array, converting.
	int layers = m_Layers.size();
	int succeeded = 0;
	for (int i = 0; i < layers; i++)
	{
		vtLayer *lp = m_Layers[i];

		OpenProgressDialog(_("Converting"), _T(""), false, this);
		bool success = lp->TransformCoords(crs);
		CloseProgressDialog();

		if (success)
		{
			succeeded++;
			lp->SetModified(true);
		}
	}
	if (succeeded < layers)
	{
		if (layers == 1)
			DisplayAndLog("Failed to convert.");
		else
			DisplayAndLog("Failed to convert %d of %d layers.",
				layers-succeeded, layers);
	}

	SetCRS(crs);
	ZoomAll();
	RefreshStatusBar();
}

void MainFrame::OnLayerSetCRS(wxCommandEvent &event)
{
	// Allow the user to directly specify the CRS for all loaded layers
	// (override it, without reprojecting the layer's data)
	// Ask for what projection to set.
	ProjectionDlg dlg(NULL, -1, _("Set to what coordinate system?"));
	dlg.SetCRS(m_crs);

	// They might switch to UTM, so help provide a good guess for UTM zone.
	dlg.SetGeoRefPoint(EstimateGeoDataCenter());

	if (dlg.ShowModal() == wxID_CANCEL)
		return;
	vtCRS crs;
	dlg.GetCRS(crs);

	// Count through the layer array, setting.
	int layers = m_Layers.size();
	for (int i = 0; i < layers; i++)
		m_Layers[i]->SetCRS(crs);

	SetCRS(crs);
	ZoomAll();
	RefreshStatusBar();
}

void MainFrame::OnUpdateLayerConvert(wxUpdateUIEvent& event)
{
	event.Enable(m_Layers.size() != 0);
}

void MainFrame::OnLayerCombine(wxCommandEvent &event)
{
	vtLayer *pActive = GetActiveLayer();
	LayerType t = pActive->GetType();

	int layers_merged = 0;

	// Count down through the layer array, flattening.
	int layers = m_Layers.size();
	for (int i = layers-1; i >= 0; i--)
	{
		vtLayer *pL = m_Layers[i];
		if (pL == pActive) continue;
		if (pL->GetType() != t) continue;

//		TRACE("Merging layer %s/%x with %s/%x\n",
//			pL->GetFilename(), pL, pActive->GetFilename(), pActive);
		if (pActive->AppendDataFrom(pL))
		{
			// successfully merged contents, so second layer can be deleted
			RemoveLayer(pL);
			layers_merged++;
		}
	}

	if (layers_merged > 0)
	{
		wxString newname = _("Untitled");
		newname += pActive->GetFileExtension();
		pActive->SetLayerFilename(newname);
		pActive->SetModified(true);
	}
}

void MainFrame::OnUpdateLayerCombine(wxUpdateUIEvent& event)
{
	vtLayer *lp = GetActiveLayer();
	vtElevLayer *ep = GetActiveElevLayer();
	event.Enable(lp &&
			(lp->GetType() == LT_ROAD ||
			 lp->GetType() == LT_VEG ||
			 lp->GetType() == LT_WATER ||
			 lp->GetType() == LT_STRUCTURE ||
			 lp->GetType() == LT_RAW ||
			 (ep && ep->GetTin() != NULL)));	// TINs can also combine
}


////////////////////////////////////////////////////////////
// View menu

void MainFrame::OnLayerShow(wxCommandEvent &event)
{
	vtLayer *pLayer = GetActiveLayer();
	if (!pLayer)
		return;
	pLayer->SetVisible(!pLayer->GetVisible());
	RefreshView();
	RefreshTreeStatus();
}

void MainFrame::OnUpdateLayerShow(wxUpdateUIEvent& event)
{
	vtLayer *pLayer = GetActiveLayer();

	event.Enable(pLayer != NULL);
	event.Check(pLayer && pLayer->GetVisible());
}

void MainFrame::OnLayerUp(wxCommandEvent &event)
{
	vtLayer *pLayer = GetActiveLayer();
	if (!pLayer)
		return;
	int num = LayerNum(pLayer);
	if (num < (int) NumLayers() - 1)
		SwapLayerOrder(num, num+1);

	RefreshView();
	RefreshTreeView();
}

void MainFrame::OnUpdateLayerUp(wxUpdateUIEvent& event)
{
	vtLayer *pLayer = GetActiveLayer();
	event.Enable(pLayer != NULL && LayerNum(pLayer) < (int) NumLayers() - 1);
}

void MainFrame::OnLayerDown(wxCommandEvent &event)
{
	vtLayer *pLayer = GetActiveLayer();
	if (!pLayer)
		return;
	int num = LayerNum(pLayer);
	if (num > 0)
		SwapLayerOrder(num-1, num);

	RefreshView();
	RefreshTreeView();
}

void MainFrame::OnUpdateLayerDown(wxUpdateUIEvent& event)
{
	vtLayer *pLayer = GetActiveLayer();
	event.Enable(pLayer != NULL && LayerNum(pLayer) > 0);
}

void MainFrame::OnViewMagnifier(wxCommandEvent &event)
{
	m_pView->SetMode(LB_Mag);
	m_pView->SetCorrectCursor();
}

void MainFrame::OnUpdateMagnifier(wxUpdateUIEvent& event)
{
	event.Check(m_pView && m_pView->GetMode() == LB_Mag);
}

void MainFrame::OnViewPan(wxCommandEvent &event)
{
	m_pView->SetMode(LB_Pan);
	m_pView->SetCorrectCursor();
}

void MainFrame::OnUpdatePan(wxUpdateUIEvent& event)
{
	event.Check(m_pView && m_pView->GetMode() == LB_Pan);
}

void MainFrame::OnViewDistance(wxCommandEvent &event)
{
	m_pView->SetMode(LB_Dist);
	m_pView->SetCorrectCursor();
	ShowDistanceDlg();
}

void MainFrame::OnUpdateDistance(wxUpdateUIEvent& event)
{
	event.Check(m_pView && m_pView->GetMode() == LB_Dist);
}

void MainFrame::OnViewSetArea(wxCommandEvent& event)
{
	m_pView->SetMode(LB_Box);
}

void MainFrame::OnUpdateViewSetArea(wxUpdateUIEvent& event)
{
	event.Check(m_pView && m_pView->GetMode() == LB_Box);
}

void MainFrame::OnViewZoomIn(wxCommandEvent &event)
{
	// Directly towards center of view.
	const wxSize client = m_pView->GetClientSize();
	DPoint2 p;
	m_pView->ClientToWorld(wxPoint(client.x / 2, client.y / 2), p);
	m_pView->ScaleAroundPoint(p, m_pView->GetScale() * sqrt(2.0));
	RefreshStatusBar();
}

void MainFrame::OnViewZoomOut(wxCommandEvent &event)
{
	const wxSize client = m_pView->GetClientSize();
	DPoint2 p;
	m_pView->ClientToWorld(wxPoint(client.x / 2, client.y / 2), p);
	m_pView->ScaleAroundPoint(p, m_pView->GetScale() / sqrt(2.0));
	RefreshStatusBar();
}

void MainFrame::OnViewZoomAll(wxCommandEvent &event)
{
	ZoomAll();
}

void MainFrame::OnViewZoomToLayer(wxCommandEvent &event)
{
	vtLayer *lp = GetActiveLayer();
	DRECT rect;
	if (lp->GetExtent(rect))
		m_pView->ZoomToRect(rect, 0.1f);
}

void MainFrame::OnUpdateViewZoomToLayer(wxUpdateUIEvent& event)
{
	event.Enable(GetActiveLayer() != NULL);
}

void MainFrame::OnViewFull(wxCommandEvent& event)
{
	vtElevLayer *pEL = GetActiveElevLayer();
	if (pEL)
		m_pView->MatchZoomToElev(pEL);
	vtImageLayer *pIL = GetActiveImageLayer();
	if (pIL)
		m_pView->MatchZoomToImage(pIL);
}

void MainFrame::OnUpdateViewFull(wxUpdateUIEvent& event)
{
	vtLayer *lp = GetActiveLayer();
	event.Enable(lp &&
			(lp->GetType() == LT_ELEVATION || lp->GetType() == LT_IMAGE));
}

void MainFrame::OnViewZoomArea(wxCommandEvent& event)
{
	m_pView->ZoomToRect(m_area, 0.1f);
}

void MainFrame::OnUpdateViewZoomArea(wxUpdateUIEvent& event)
{
	event.Enable(!m_area.IsEmpty());
}

void MainFrame::OnViewToolbar(wxCommandEvent& event)
{
	wxAuiPaneInfo &info = m_mgr.GetPane(m_pToolbar);
	info.Show(!info.IsShown());
	m_mgr.Update();
}

void MainFrame::OnUpdateViewToolbar(wxUpdateUIEvent& event)
{
	wxAuiPaneInfo &info = m_mgr.GetPane(m_pToolbar);
	event.Check(info.IsShown());
}

void MainFrame::OnViewLayers(wxCommandEvent& event)
{
	wxAuiPaneInfo &info = m_mgr.GetPane(m_pTree);
	info.Show(!info.IsShown());
	m_mgr.Update();
}

void MainFrame::OnUpdateViewLayers(wxUpdateUIEvent& event)
{
	wxAuiPaneInfo &info = m_mgr.GetPane(m_pTree);
	event.Check(info.IsShown());
}

void MainFrame::OnViewWorldMap(wxCommandEvent& event)
{
	m_pView->SetShowMap(!m_pView->GetShowMap());
	m_pView->Refresh();
}

void MainFrame::OnUpdateWorldMap(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetShowMap());
}

void MainFrame::OnViewUTMBounds(wxCommandEvent& event)
{
	m_pView->m_bShowUTMBounds = !m_pView->m_bShowUTMBounds;
	m_pView->Refresh();
}

void MainFrame::OnUpdateUTMBounds(wxUpdateUIEvent& event)
{
	event.Check(m_pView->m_bShowUTMBounds);
}

void MainFrame::OnViewProfile(wxCommandEvent& event)
{
	if (m_pProfileDlg && m_pProfileDlg->IsShown())
		m_pProfileDlg->Hide();
	else
		ShowProfileDlg();
}

void MainFrame::OnUpdateViewProfile(wxUpdateUIEvent& event)
{
	event.Check(m_pProfileDlg && m_pProfileDlg->IsShown());
	event.Enable(LayersOfType(LT_ELEVATION) > 0);
}

void MainFrame::OnViewScaleBar(wxCommandEvent& event)
{
	m_pView->SetShowScaleBar(!m_pView->GetShowScaleBar());
}

void MainFrame::OnUpdateViewScaleBar(wxUpdateUIEvent& event)
{
	event.Check(m_pView && m_pView->GetShowScaleBar());
}

void MainFrame::OnViewOptions(wxCommandEvent& event)
{
	ShowOptionsDialog();
}


//////////////////////////
// Road

void MainFrame::OnSelectLink(wxCommandEvent &event)
{
	m_pView->SetMode(LB_Link);
}

void MainFrame::OnUpdateSelectLink(wxUpdateUIEvent& event)
{
	event.Check( m_pView->GetMode() == LB_Link );
}

void MainFrame::OnSelectNode(wxCommandEvent &event)
{
	m_pView->SetMode(LB_Node);
}

void MainFrame::OnUpdateSelectNode(wxUpdateUIEvent& event)
{
	event.Check( m_pView->GetMode() == LB_Node );
}

void MainFrame::OnSelectWhole(wxCommandEvent &event)
{
	m_pView->SetMode(LB_LinkExtend);
}

void MainFrame::OnUpdateSelectWhole(wxUpdateUIEvent& event)
{
	event.Check( m_pView->GetMode() == LB_LinkExtend );
}

void MainFrame::OnDirection(wxCommandEvent &event)
{
	m_pView->SetMode(LB_Dir);
}

void MainFrame::OnUpdateDirection(wxUpdateUIEvent& event)
{
	event.Check( m_pView->GetMode() == LB_Dir );
}

void MainFrame::OnRoadEdit(wxCommandEvent &event)
{
	m_pView->SetMode(LB_LinkEdit);
}

void MainFrame::OnUpdateRoadEdit(wxUpdateUIEvent& event)
{
	event.Check( m_pView->GetMode() == LB_LinkEdit );
}

void MainFrame::OnRoadShowNodes(wxCommandEvent &event)
{
	bool state = vtRoadLayer::GetDrawNodes();
	vtRoadLayer::SetDrawNodes(!state);
	m_pView->Refresh(state);
}

void MainFrame::OnUpdateRoadShowNodes(wxUpdateUIEvent& event)
{
	event.Check(vtRoadLayer::GetDrawNodes());
}

void MainFrame::OnSelectHwy(wxCommandEvent &event)
{
	vtRoadLayer *pRL = GetActiveRoadLayer();
	if (!pRL) return;

	wxTextEntryDialog dlg(this, _("Please enter highway number"),
		_("Select Highway"), _T(""));
	if (dlg.ShowModal() == wxID_OK)
	{
		int num;
		wxString str = dlg.GetValue();
		sscanf(str.mb_str(wxConvUTF8), "%d", &num);
		if (pRL->SelectHwyNum(num))
			m_pView->Refresh();
	}
}

void MainFrame::OnRoadClean(wxCommandEvent &event)
{
	vtRoadLayer *pRL = GetActiveRoadLayer();
	if (!pRL) return;

	double dEpsilon;
	if (m_crs.GetUnits() == LU_DEGREES)
		dEpsilon = 1E-7;
	else
		dEpsilon = 1E-2;

	wxString str;
	str.Printf(_T("%g"), dEpsilon);
	str = wxGetTextFromUser(_("How close are degenerate points? (epsilon)"),
		_("Clean RoadMap"), str, this);
	if (str == _T(""))
		return;

	dEpsilon = atof(str.mb_str(wxConvUTF8));

	pRL->DoClean(dEpsilon);

	m_pView->Refresh();
}

void MainFrame::OnRoadGuess(wxCommandEvent &event)
{
	vtRoadLayer *pRL = GetActiveRoadLayer();
	if (!pRL) return;

	// Set visual properties
	pRL->GuessIntersectionTypes();

	for (NodeEdit *pN = pRL->GetFirstNode(); pN; pN = pN->GetNext())
		pN->DetermineVisualFromLinks();

	m_pView->Refresh();
}

void MainFrame::OnUpdateRoadFlatten(wxUpdateUIEvent& event)
{
	vtElevLayer *pE = (vtElevLayer *)GetMainFrame()->FindLayerOfType(LT_ELEVATION);

	event.Enable(pE != NULL && pE->GetGrid() != NULL);
}


//////////////////////////
// Elevation

void MainFrame::OnUpdateIsElevation(wxUpdateUIEvent& event)
{
	event.Enable(GetActiveElevLayer() != NULL);
}

void MainFrame::OnUpdateIsGrid(wxUpdateUIEvent& event)
{
	vtElevLayer *pEL = GetActiveElevLayer();
	event.Enable(pEL && pEL->IsGrid());
}

void MainFrame::OnUpdateArithmetic(wxUpdateUIEvent& event)
{
	// Must have at least two elevation layers to do arithmetic on them.
	event.Enable(LayersOfType(LT_ELEVATION) > 1);
}

void MainFrame::OnElevSelect(wxCommandEvent& event)
{
	m_pView->SetMode(LB_TSelect);
}

void MainFrame::OnUpdateElevSelect(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode() == LB_TSelect);
}

void MainFrame::OnElevRemoveRange(wxCommandEvent &event)
{
	vtElevLayer *t = GetActiveElevLayer();
	if (!t && !t->GetGrid())
		return;

	wxString str;
	str = wxGetTextFromUser(_("Please specify the elevation range\n(minimum and maximum in the form \"X Y\")\nAll values within this range (and within the area\ntool, if it is defined) will be set to Unknown."));

	float zmin, zmax;
	vtString text = (const char *) str.mb_str(wxConvUTF8);
	if (sscanf(text, "%f %f", &zmin, &zmax) != 2)
	{
		wxMessageBox(_("Didn't get two numbers."));
		return;
	}
	int count = t->RemoveElevRange(zmin, zmax, m_area.IsEmpty() ? NULL : &m_area);
	if (count)
	{
		wxString str;
		str.Printf(_("Set %d heixels to unknown"), count);
		wxMessageBox(str);
		t->SetModified(true);
		t->ReRender();
		m_pView->Refresh();
	}
}

void MainFrame::OnElevArithmetic(wxCommandEvent &event)
{
	ElevMathDlg dlg(this, -1, _("Arithmetic"));
	int res = dlg.ShowModal();
	if (res != wxID_OK)
		return;

	vtElevLayer *t = GetActiveElevLayer();
	vtElevLayer *result = ElevationMath(dlg.m_pLayer1, dlg.m_pLayer2,
		dlg.m_extent, dlg.m_spacing, dlg.m_iOperation == 0);

	if (result)
	{
		SetActiveLayer(result);
		m_pView->SetActiveLayer(result);
		RefreshTreeView();
		RefreshToolbars();
		RefreshView();
	}
}

void MainFrame::OnElevSetUnknown(wxCommandEvent &event)
{
	vtElevLayer *t = GetActiveElevLayer();
	if (!t)	return;

	static float fValue = 1.0f;
	wxString str;
	str.Printf(_T("%g"), fValue);
	str = wxGetTextFromUser(_("Set unknown areas to what value?"),
		_("Set Unknown Areas"), str, this);
	if (str == _T(""))
		return;

	fValue = atof(str.mb_str(wxConvUTF8));
	int count = t->SetUnknown(fValue, &m_area);
	if (count)
	{
		t->SetModified(true);
		t->ReRender();
		m_pView->Refresh();
	}
}

void MainFrame::OnFillIn(int method)
{
	vtElevLayer *el = GetActiveElevLayer();

	DRECT *area = NULL;
	if (!m_area.IsEmpty())
		area = &m_area;

	if (FillElevGaps(el, area, method))
	{
		el->SetModified(true);
		el->ReRender();
		m_pView->Refresh();
	}
}

void MainFrame::OnFillFast(wxCommandEvent &event)
{
	OnFillIn(1);
}

void MainFrame::OnFillSlow(wxCommandEvent &event)
{
	OnFillIn(2);
}

void MainFrame::OnFillRegions(wxCommandEvent &event)
{
	OnFillIn(3);
}

void MainFrame::OnElevScale(wxCommandEvent &event)
{
	vtElevLayer *el = GetActiveElevLayer();
	if (!el)
		return;

	wxString str = wxGetTextFromUser(_("Please enter a scale factor"),
		_("Scale Elevation"), _T("1.0"), this);
	if (str == _T(""))
		return;

	float fScale;
	fScale = atof(str.mb_str(wxConvUTF8));
	if (fScale == 0.0f)
	{
		wxMessageBox(_("Couldn't parse the number you typed."));
		return;
	}
	if (fScale == 1.0f)
		return;

	vtElevationGrid *grid = el->GetGrid();
	if (grid)
	{
		grid->Scale(fScale, true);
		el->ReRender();
	}
	vtTin2d *tin = el->GetTin();
	if (tin)
	{
		tin->Scale(fScale);
	}
	el->SetModified(true);
	m_pView->Refresh();
}

void MainFrame::OnElevVertOffset(wxCommandEvent &event)
{
	vtElevLayer *el = GetActiveElevLayer();
	if (!el)
		return;

	wxString str = wxGetTextFromUser(_("Please enter an amout to offset"),
		_("Offset Elevation Vertically"), _T("0.0"), this);
	if (str == _T(""))
		return;

	float fValue;
	fValue = atof(str.mb_str(wxConvUTF8));
	if (fValue == 0.0f)
	{
		wxMessageBox(_("Couldn't parse the number you typed."));
		return;
	}

	vtElevationGrid *grid = el->GetGrid();
	if (grid)
	{
		grid->VertOffset(fValue);
		el->ReRender();
	}
	vtTin2d *tin = el->GetTin();
	if (tin)
	{
		tin->VertOffset(fValue);
	}
	el->SetModified(true);
	m_pView->Refresh();
}

void MainFrame::OnElevExport(wxCommandEvent &event)
{
	vtElevLayer *pEL = GetActiveElevLayer();
	if (!pEL)
		return;

	bool bIsGrid = (pEL->GetGrid() != NULL);

	wxArrayString choices;
	if (bIsGrid)
	{
		choices.Add(_T("3TX"));
		choices.Add(_T("ArcInfo ASCII Grid"));
		choices.Add(_T("ChunkLOD (.chu)"));
		choices.Add(_T("GeoTIFF"));
		choices.Add(_T("MSI Planet"));
		choices.Add(_T("PNG (16-bit greyscale)"));
		choices.Add(_T("RAW/INF for MS Flight Simulator"));
		choices.Add(_T("RAW (Scaled to 8-bit) for Unity"));
		choices.Add(_T("STM"));
		choices.Add(_T("TIN (.itf)"));
		choices.Add(_T("TerraGen"));
		choices.Add(_T("VRML ElevationGrid"));
		choices.Add(_T("XYZ ASCII Points"));
	}
	else	// is a TIN
	{
		choices.Add(_T("GMS (Aquaveo) .tin"));
		choices.Add(_T("DXF (AutoCAD) .dxf"));
		choices.Add(_T("DAE (Collada) .dae"));
		choices.Add(_T("WRL (VRML) .wrl"));
		choices.Add(_T("OBJ (Alias Wavefront OBJ) .obj"));
		choices.Add(_T("PLY (Stanford Polygon PLY) .ply"));
	}

	wxSingleChoiceDialog dlg(this, _("Please choose"),
		_("Export to file format:"), choices);
	dlg.SetSize(-1, 340);		// List is long, make the dialog taller.
	if (dlg.ShowModal() != wxID_OK)
		return;

	if (bIsGrid)
	{
		switch (dlg.GetSelection())
		{
		case 0: Export3TX(); break;
		case 1: ExportASC(); break;
		case 2: ExportChunkLOD(); break;
		case 3: ExportGeoTIFF(); break;
		case 4: ExportPlanet(); break;
		case 5: ExportPNG16(); break;
		case 6: ExportRAWINF(); break;
		case 7: ExportRAW_Unity(); break;
		case 8: ExportSTM(); break;
		case 9: ExportTIN(); break;
		case 10: ExportTerragen(); break;
		case 11: ExportVRML(); break;
		case 12: ExportXYZ(); break;
		}
	}
	else	// is a TIN
	{
		bool success;
		vtString fname;
		switch (dlg.GetSelection())
		{
		case 0:
			fname = pEL->GetExportFilename(FSTRING_GMS);
			if (fname == "")
				return;
			OpenProgressDialog(_T("Writing TIN"), wxString::FromUTF8((const char *) fname), false, this);
			success = pEL->GetTin()->WriteGMS(fname, progress_callback);
			break;
		case 1:
			fname = pEL->GetExportFilename(FSTRING_DXF);
			if (fname == "")
				return;
			OpenProgressDialog(_T("Writing DXF"), wxString::FromUTF8((const char *) fname), false, this);
			success = pEL->GetTin()->WriteDXF(fname, progress_callback);
			break;
		case 2:
			fname = pEL->GetExportFilename(FSTRING_DAE);
			if (fname == "")
				return;
			OpenProgressDialog(_T("Writing DAE"), wxString::FromUTF8((const char *) fname), false, this);
			success = pEL->GetTin()->WriteDAE(fname, progress_callback);
			break;
		case 3:
			fname = pEL->GetExportFilename(FSTRING_WRL);
			if (fname == "")
				return;
			OpenProgressDialog(_T("Writing WRL"), wxString::FromUTF8((const char *) fname), false, this);
			success = pEL->GetTin()->WriteWRL(fname, progress_callback);
			break;
		case 4:
			fname = pEL->GetExportFilename(FSTRING_OBJ);
			if (fname == "")
				return;
			OpenProgressDialog(_T("Writing OBJ"), wxString::FromUTF8((const char *) fname), false, this);
			success = pEL->GetTin()->WriteOBJ(fname, progress_callback);
			break;
		case 5:
			fname = pEL->GetExportFilename(FSTRING_PLY);
			if (fname == "")
				return;
			OpenProgressDialog(_T("Writing PLY"), wxString::FromUTF8((const char *) fname), false, this);
			success = pEL->GetTin()->WritePLY(fname, progress_callback);
			break;
		}
		CloseProgressDialog();
		if (success)
			DisplayAndLog("Successfully wrote file '%s'", (const char *) fname);
		else
			DisplayAndLog("Error writing file.");
	}
}

void MainFrame::OnElevExportTiles(wxCommandEvent& event)
{
	ElevExportTiles(m_pView);
}

void MainFrame::OnElevCopy(wxCommandEvent& event)
{
	DoElevCopy();
}

void MainFrame::OnElevPasteNew(wxCommandEvent& event)
{
	DoElevPasteNew();
}

void MainFrame::OnElevExportBitmap(wxCommandEvent& event)
{
	int cols, rows;
	vtElevLayer *pEL = GetActiveElevLayer();
	pEL->GetGrid()->GetDimensions(cols, rows);

	RenderDlg dlg(this, -1, _("Render Elevation to Bitmap"));
	dlg.m_Size.x = cols;
	dlg.m_Size.y = rows;

	if (dlg.ShowModal() == wxID_CANCEL)
		return;

	OpenProgressDialog(_("Generating Bitmap"), _T(""));
	ExportBitmap(GetActiveElevLayer(), dlg);
	CloseProgressDialog();
}

void MainFrame::OnElevToTin(wxCommandEvent& event)
{
	vtElevLayer *pEL1 = GetActiveElevLayer();
	vtElevationGrid *grid = pEL1->GetGrid();

	vtTin2d *tin = new vtTin2d(grid);
	vtElevLayer *pEL = new vtElevLayer;
	pEL->SetTin(tin);

	// Inherit the name
	wxString name = pEL1->GetLayerFilename();
	name = name.BeforeLast('.');
	name += _T(".itf");
	pEL->SetLayerFilename(name);

	AddLayer(pEL);
	SetActiveLayer(pEL);

	m_pView->Refresh();
	RefreshTreeView();
}

void MainFrame::OnElevContours(wxCommandEvent& event)
{
	vtElevLayer *pEL = GetActiveElevLayer();
	vtElevationGrid *grid = pEL->GetGrid();

	const IPoint2 size = grid->GetDimensions();
	VTLOG("OnElevContours: using grid of size %d x %d, spacing %lf * %lf\n",
		size.x, size.y, grid->GetSpacing().x, grid->GetSpacing().y);

#if SUPPORT_QUIKGRID
	ContourDlg dlg(this, -1, _("Add Contours"));

	// Put any existing raw polyline layers in the drop-down choice
	dlg.LayerChoice()->Clear();
	int layers = m_Layers.size();
	for (int i = 0; i < layers; i++)
	{
		vtLayer *pL = m_Layers[i];
		if (pL->GetType() != LT_RAW) continue;
		vtRawLayer *raw = (vtRawLayer*) pL;
		if (raw->GetGeomType() == wkbLineString)
			dlg.LayerChoice()->Append(raw->GetLayerFilename());
	}
	dlg.LayerChoice()->SetSelection(0);

	bool bResult = (dlg.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	vtRawLayer *raw;
	if (dlg.m_bCreate)
	{
		// create new (abstract polyline) layer to receive contour lines
		raw = new vtRawLayer;
		raw->SetGeomType(wkbLineString);
		raw->SetLayerFilename(_T("Untitled.shp"));

		// We will add an elevation value to each contour
		raw->GetFeatureSet()->AddField("Elevation", FT_Float);

		// copy CRS
		vtCRS crs;
		pEL->GetCRS(crs);
		raw->SetCRS(crs);

		AddLayer(raw);
		RefreshTreeView();
	}
	else
	{
		// get the existing layer from the dialog's choice, by name
		raw = (vtRawLayer*) m_Layers.FindByFilename(dlg.m_strLayer);
	}
	if (!raw)
		return;

	vtFeatureSetLineString *fsls = (vtFeatureSetLineString *) raw->GetFeatureSet();

	VTLOG1(" Setting up ContourConverter\n");
	ContourConverter cc;
	if (!cc.Setup(grid, fsls))
		return;

	VTLOG1(" GenerateContour\n");
	if (dlg.m_bSingle)
		cc.GenerateContour(dlg.m_fElevSingle);
	else
		cc.GenerateContours(dlg.m_fElevEvery);
	cc.Finish();

	// The contour generator tends to make a lot of extra points. Clean them up.
	// Use an epsilon based on the grid's spacing; anything smaller than that is
	// too small to matter.
	double dEpsilon = grid->GetSpacing().Length() / 4.0;
	VTLOG1(" Cleaning up resulting polylines\n");
	int removed = fsls->FixGeometry(dEpsilon);
	VTLOG(" Removed %d points, done\n", removed);

	m_pView->Refresh();
#endif // SUPPORT_QUIKGRID
}

void MainFrame::OnElevCarve(wxCommandEvent &event)
{
	if (m_crs.IsGeographic())
	{
		wxMessageBox(_("Sorry, but precise grid operations require a non-geographic coordinate\n system (meters as horizontal units, not degrees.)"),
			_("Info"), wxOK);
		return;
	}

	// Must have at least some culture to carve
	vtRoadLayer *pR = (vtRoadLayer *) FindLayerOfType(LT_ROAD);
	vtStructureLayer *pS = (vtStructureLayer *) FindLayerOfType(LT_STRUCTURE);
	if (!pR && !pS)
		return;

	vtElevLayer *pEL = GetActiveElevLayer();
	vtElevationGrid *grid = pEL->GetGrid();

	float margin = 2.0;
	wxString str;
	str.Printf(_T("%g"), margin);
	str = wxGetTextFromUser(_("How many meters for the margin at the edge of each feature?"),
		_("Carve elevation grid under culture"), str, this);
	if (str == _T(""))
		return;

	margin = atof(str.mb_str(wxConvUTF8));

	CarveWithCulture(pEL, margin);
	m_pView->Refresh();
}

void MainFrame::OnElevMergeTin(wxCommandEvent& event)
{
	vtElevLayer *pEL = GetActiveElevLayer();
	pEL->MergeSharedVerts();
	RefreshTreeStatus();
}

void MainFrame::OnUpdateElevMergeTin(wxUpdateUIEvent& event)
{
	vtElevLayer *pEL = GetActiveElevLayer();
	event.Enable(pEL && !pEL->IsGrid());
}

void MainFrame::OnElevTrimTin(wxCommandEvent& event)
{
	m_pView->SetMode(LB_TrimTIN);
	m_pView->SetCorrectCursor();
}

void MainFrame::OnUpdateElevTrimTin(wxUpdateUIEvent& event)
{
	vtElevLayer *pEL = GetActiveElevLayer();
	event.Enable(pEL && !pEL->IsGrid());
	event.Check(m_pView->GetMode() == LB_TrimTIN);
}



//////////////////////////////////////////////////////////////////////////
// Image Menu
//

void MainFrame::OnImageReplaceRGB(wxCommandEvent& event)
{
	wxString msg = _("R G B in the range 0-255:");
	wxString str = wxGetTextFromUser(msg, _("Replace color:"));
	if (str == _T(""))
		return;
	RGBi rgb1, rgb2;
	int count = sscanf(str.mb_str(), "%hd %hd %hd", &rgb1.r, &rgb1.g, &rgb1.b);
	if (count != 3 || rgb1.r < 0 || rgb1.r > 255 || rgb1.g < 0 ||
		rgb1.g > 255 || rgb1.b < 0 || rgb1.b > 255)
	{
		wxMessageBox(_("Didn't get three R G B values in range."));
		return;
	}
	str = wxGetTextFromUser(msg, _("With color:"));
	if (str == _T(""))
		return;
	count = sscanf(str.mb_str(), "%hd %hd %hd", &rgb2.r, &rgb2.g, &rgb2.b);
	if (count != 3 || rgb2.r < 0 || rgb2.r > 255 || rgb2.g < 0 ||
		rgb2.g > 255 || rgb2.b < 0 || rgb2.b > 255)
	{
		wxMessageBox(_("Didn't get three R G B values in range."));
		return;
	}
	GetActiveImageLayer()->ReplaceColor(rgb1, rgb2);
	RefreshView();
}

void MainFrame::OnImageCreateOverviews(wxCommandEvent& event)
{
	vtImageLayer *pIL = GetActiveImageLayer();

	OpenProgressDialog(_("Creating Overviews"), _T(""), false, this);

	pIL->GetImage()->CreateOverviews();

	CloseProgressDialog();
}

void MainFrame::OnImageCreateOverviewsAll(wxCommandEvent& event)
{
	OpenProgressDialog(_("Creating Overviews"), _T(""), false, this);
	for (uint i = 0; i < NumLayers(); i++)
	{
		vtImageLayer *pIL = dynamic_cast<vtImageLayer *>(GetLayer(i));
		if (pIL)
			pIL->GetImage()->CreateOverviews();
	}
	CloseProgressDialog();
}

void MainFrame::OnImageCreateMipMaps(wxCommandEvent& event)
{
	vtImageLayer *pIL = GetActiveImageLayer();

	OpenProgressDialog(_("Creating MipMaps"), _T(""), false, this);

	pIL->GetImage()->AllocMipMaps();
	pIL->GetImage()->DrawMipMaps();

	CloseProgressDialog();
}

void MainFrame::OnImageLoadMipMaps(wxCommandEvent& event)
{
	vtImageLayer *pIL = GetActiveImageLayer();

	OpenProgressDialog(_("Loading Overviews"), _T(""), false, this);

	pIL->GetImage()->LoadOverviews();

	CloseProgressDialog();
}

void MainFrame::OnImageExportTiles(wxCommandEvent& event)
{
	ImageExportTiles(m_pView);
}

void MainFrame::OnImageExportPPM(wxCommandEvent& event)
{
	ImageExportPPM();
}

void MainFrame::OnUpdateHaveImageLayer(wxUpdateUIEvent& event)
{
	vtImageLayer *pIL = GetActiveImageLayer();
	event.Enable(pIL != NULL);
}

void MainFrame::OnUpdateHaveImageLayerInMem(wxUpdateUIEvent& event)
{
	vtImageLayer *pIL = GetActiveImageLayer();
	if (pIL)
	{
		vtImage *im = pIL->GetImage();
		event.Enable(im && im->GetBitmap());
	}
	else
		event.Enable(false);
}


//////////////////////////////////////////////////////////////////////////
// Area Menu
//

void MainFrame::OnAreaClear(wxCommandEvent &event)
{
	m_pView->InvertAreaTool(m_area);
	m_area.SetRect(0, 0, 0, 0);
	m_pView->InvertAreaTool(m_area);
}

void MainFrame::OnAreaZoomAll(wxCommandEvent &event)
{
	m_pView->InvertAreaTool(m_area);
	m_area = GetExtents();
	m_pView->InvertAreaTool(m_area);
}

void MainFrame::OnUpdateAreaZoomAll(wxUpdateUIEvent& event)
{
	event.Enable(NumLayers() != 0);
}

void MainFrame::OnAreaZoomLayer(wxCommandEvent &event)
{
	DRECT area;
	if (GetActiveLayer()->GetExtent(area))
	{
		m_pView->InvertAreaTool(m_area);
		m_area = area;
		m_pView->InvertAreaTool(m_area);
	}
}

void MainFrame::OnUpdateAreaZoomLayer(wxUpdateUIEvent& event)
{
	event.Enable(GetActiveLayer() != NULL);
}

void MainFrame::OnUpdateAreaMatch(wxUpdateUIEvent& event)
{
	int iRasters = LayersOfType(LT_ELEVATION) + LayersOfType(LT_IMAGE);
	event.Enable(!m_area.IsEmpty() && iRasters > 0);
}

void MainFrame::OnAreaTypeIn(wxCommandEvent &event)
{
	ExtentDlg dlg(NULL, -1, _("Edit Area"));
	dlg.SetArea(m_area, (m_crs.IsGeographic() != 0));
	if (dlg.ShowModal() == wxID_OK)
	{
		m_area = dlg.m_area;
		m_pView->Refresh();
	}
}

void MainFrame::OnAreaMatch(wxCommandEvent &event)
{
	MatchDlg dlg(NULL, -1, _("Match Area and Tiling to Layer"));
	dlg.SetView(GetView());
	dlg.SetArea(m_area, (m_crs.IsGeographic() != 0));
	if (dlg.ShowModal() == wxID_OK)
	{
		m_tileopts.cols = dlg.m_tile.x;
		m_tileopts.rows = dlg.m_tile.y;
		m_tileopts.lod0size = dlg.m_iTileSize;
		m_area = dlg.m_area;
		m_pView->Refresh();
	}
	GetView()->HideGridMarks();
}

void MainFrame::OnAreaRequestWFS(wxCommandEvent& event)
{
#if SUPPORT_CURL
	bool success;

	wxTextEntryDialog dlg(this, _T("WFS Server address"),
		_T("Please enter server base URL"), _T("http://10.254.0.29:8081/"));
	if (dlg.ShowModal() != wxID_OK)
		return;
	wxString value = dlg.GetValue();
	vtString server = (const char *) value.mb_str(wxConvUTF8);

	OGCLayerArray layers;
	success = GetLayersFromWFS(server, layers);

	int numlayers = layers.size();
	wxString choices[100];
	for (int i = 0; i < numlayers; i++)
	{
		const char *string = layers[i]->GetValueString("Name");
		choices[i] = wxString::FromAscii(string);
	}

	wxSingleChoiceDialog dlg2(this, _T("Choice Layer"),
		_T("Please indicate layer:"), numlayers, (const wxString *)choices);

	if (dlg2.ShowModal() != wxID_OK)
		return;

	vtRawLayer *pRL = new vtRawLayer;
	success = pRL->ReadFeaturesFromWFS(server, "rail");
	if (success)
		AddLayerWithCheck(pRL);
	else
		delete pRL;
#endif
}

void MainFrame::OnAreaRequestWMS(wxCommandEvent& event)
{
	VTLOG1("OnAreaRequestWMS\n");

	if (m_wms_servers.empty())
	{
		// supply some hardcoded well-known servers
		OGCServer s;
		s.m_url = "http://wmt.jpl.nasa.gov/wms.cgi";
		m_wms_servers.push_back(s);
		s.m_url = "http://globe.digitalearth.gov/viz-bin/wmt.cgi";
		m_wms_servers.push_back(s);
	}

#if SUPPORT_CURL
	// Ask the user for what server and layer they want
	if (!m_pMapServerDlg)
		m_pMapServerDlg = new MapServerDlg(this, -1, _T("WMS Request"));

	m_pMapServerDlg->m_area = m_area;
	m_pMapServerDlg->m_crs = m_crs;
	m_pMapServerDlg->SetServerArray(m_wms_servers);

	if (m_pMapServerDlg->ShowModal() != wxID_OK)
		return;

	// Prepare to receive the WMS data
	if (m_pMapServerDlg->m_bNewLayer)
	{
		// Enforce PNG, that's all we support so far
		m_pMapServerDlg->m_iFormat = 1;	// png
		m_pMapServerDlg->UpdateURL();
	}
	FILE *fp;
	wxString str;
	vtString fname;
	if (m_pMapServerDlg->m_bToFile)
	{
		// Very simple: just write the buffer to disk
		fname = m_pMapServerDlg->m_strToFile.mb_str(wxConvUTF8);
		fp = vtFileOpen(fname, "wb");
		if (!fp)
		{
			str = _("Could not open file");
			str += _T(" '");
			str += m_pMapServerDlg->m_strToFile;
			str += _T("'");
			wxMessageBox(str);
			return;
		}
	}

	// Bring down the WMS data
	VTLOG1("  Requesting data\n");
	OpenProgressDialog(_("Requesting data"), _T(""), false, this);

	vtString url = (const char*)m_pMapServerDlg->m_strQueryURL.mb_str(wxConvUTF8);
	VTLOG("  URL: %s\n", (const char *)url);

	ReqContext rc;
	rc.SetProgressCallback(progress_callback);
	vtBytes data;
	bool success = rc.GetURL(url, data);
	CloseProgressDialog();

	if (!success)
	{
		str = wxString(rc.GetErrorMsg(), wxConvUTF8);	// the HTTP request failed
		wxMessageBox(str);
		return;
	}
	if (data.Len() > 5 &&
		(!strncmp((char *)data.Get(), "<?xml", 5) ||
		 !strncmp((char *)data.Get(), "<WMT", 4) ||
		 !strncmp((char *)data.Get(), "<!DOC", 5)))
	{
		// We got an XML-formatted response, not the image we were expecting.
		// The XML probably contains diagnostic error msg.
		// So show it to the user.
		uchar ch = 0;
		data.Append(&ch, 1);
		VTLOG1("  Got response: ");
		VTLOG1((const char*) data.Get());
		str = wxString((const char*) data.Get(), wxConvUTF8);
		wxMessageBox(str);
		return;
	}

	if (m_pMapServerDlg->m_bNewLayer)
	{
		VTLOG1("  Creating new layer\n");
		// Now data contains the PNG file in memory, so parse it.
		vtImageLayer *pIL = new vtImageLayer;
		success = pIL->GetImage()->ReadPNGFromMemory(data.Get(), data.Len());
		if (success)
		{
			pIL->SetExtent(m_area);
			pIL->SetCRS(m_crs);
			AddLayerWithCheck(pIL);
		}
		else
			delete pIL;
	}
	if (m_pMapServerDlg->m_bToFile)
	{
		VTLOG("  Writing %d bytes to file '%s'\n", data.Len(), (const char *)fname);
		fwrite(data.Get(), data.Len(), 1, fp);
		fclose(fp);
	}
#endif
}

void MainFrame::OnUpdateAreaRequestWMS(wxUpdateUIEvent& event)
{
	event.Enable(!m_area.IsEmpty() && SUPPORT_CURL);
}


//////////////////////////
// Vegetation menu

void MainFrame::OnVegPlants(wxCommandEvent& event)
{
	// if SpeciesList has not previously been open, get the data from file first
	if (m_strSpeciesFilename == "")
	{
		// To make it easier for the user, look for a species.xml on the path and
		// suggest that as the folder to look in.
		wxString default_dir;
		wxString default_file(_T("species.xml"));
		vtString species_path = FindFileOnPaths(vtGetDataPath(), "PlantData/species.xml");
		if (species_path != "")
		{
			vtString just_path(ExtractPath(species_path, false));
			default_dir = wxString::FromUTF8((const char *) just_path);
#if WIN32
			// An ugly workaround for Windows 7 File Dialog's behavior.
			// We really want it to respect default_dir, but it wants to give us
			// folder that the user last opened instead.
			// Ref: http://msdn.microsoft.com/en-us/library/windows/desktop/ms646839(v=vs.85).aspx
			// Using a random value (that isn't a valid path) avoids the behavior.
			default_dir.Printf("%d", clock());
			default_file = wxString::FromUTF8(species_path);
			default_file.Replace("/", "\\");
#endif
		}
		wxString filter = _("Plant Species List Files (*species.xml)|*.xml");

		// Use file dialog to open plant list text file.
		wxFileDialog loadFile(NULL, _("Load Plant Info"), default_dir,
			default_file, filter, wxFD_OPEN);

		if (loadFile.ShowModal() != wxID_OK)
			return;

		wxString str = loadFile.GetPath();
		if (!LoadSpeciesFile(str.mb_str(wxConvUTF8)))
			return;
	}
	if (!m_SpeciesListDlg)
	{
		// Create new Plant List Dialog
		m_SpeciesListDlg = new SpeciesListDlg(this, wxID_ANY, _("Plants List"),
				wxPoint(140, 100), wxSize(950, 400), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	}

	// Display plant list data, calling OnInitDialog.
	m_SpeciesListDlg->Show(true);
}

void MainFrame::OnVegBioregions(wxCommandEvent& event)
{
	// if data isn't there, get the data first
	if (m_strBiotypesFilename == "")
	{
		wxString filter = _("Bioregion Files (*.txt)|*.txt");

		// Use file dialog to open bioregion text file.
		wxFileDialog loadFile(NULL, _("Load BioRegion Info"), _T(""), _T(""),
			filter, wxFD_OPEN);

		if (loadFile.ShowModal() != wxID_OK)
			return;

		// Read bioregions, data kept on frame with m_pBioRegion.
		wxString str = loadFile.GetPath();
		if (!LoadBiotypesFile(str.mb_str(wxConvUTF8)))
			return;
	}
	if (!m_BioRegionDlg)
	{
		// Create new Bioregion Dialog
		m_BioRegionDlg = new BioRegionDlg(this, wxID_ANY, _("BioRegions List"),
				wxPoint(120, 80), wxSize(300, 500), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	}

	// Display bioregion data, calling OnInitDialog.
	m_BioRegionDlg->Show(true);
}

void MainFrame::OnVegRemap(wxCommandEvent& event)
{
	vtVegLayer *pVeg = GetMainFrame()->GetActiveVegLayer();
	if (!pVeg) return;

	vtSpeciesList *list = GetSpeciesList();

	wxArrayString choices;
	uint i, n = list->NumSpecies();
	for (i = 0; i < n; i++)
	{
		vtPlantSpecies *spe = list->GetSpecies(i);
		wxString str(spe->GetSciName(), wxConvUTF8);
		choices.Add(str);
	}

	wxString result1 = wxGetSingleChoice(_("Remap FROM Species"), _("Species"),
		choices, this);
	if (result1 == _T(""))	// cancelled
		return;
	short species_from = list->GetSpeciesIdByName(result1.mb_str(wxConvUTF8));

	wxString result2 = wxGetSingleChoice(_("Remap TO Species"), _("Species"),
		choices, this);
	if (result2 == _T(""))	// cancelled
		return;
	short species_to = list->GetSpeciesIdByName(result2.mb_str(wxConvUTF8));

	vtFeatureSet *pSet = pVeg->GetFeatureSet();
	vtPlantInstanceArray *pPIA = dynamic_cast<vtPlantInstanceArray *>(pSet);
	if (!pPIA)
		return;

	float size;
	short species_id;
	int count = 0;
	for (i = 0; i < pPIA->NumEntities(); i++)
	{
		pPIA->GetPlant(i, size, species_id);
		if (species_id == species_from)
		{
			pPIA->SetPlant(i, size, species_to);
			count++;
		}
	}
	wxString str;
	str.Printf(_("Remap successful, %d plants remapped.\n"), count);
	wxMessageBox(str, _("Info"));
	if (count > 0)
		pVeg->SetModified(true);
}

void MainFrame::OnVegExportSHP(wxCommandEvent& event)
{
	vtVegLayer *pVeg = GetMainFrame()->GetActiveVegLayer();
	if (!pVeg) return;

	// Open File Save Dialog
	wxFileDialog saveFile(NULL, _("Export vegetation to SHP"), _T(""), _T(""),
		FSTRING_SHP, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (saveFile.ShowModal() == wxID_CANCEL)
		return;
	wxString strPathName = saveFile.GetPath();

	pVeg->ExportToSHP(strPathName.mb_str(wxConvUTF8));
}

void MainFrame::OnVegHTML(wxCommandEvent& event)
{
	vtSpeciesList *list = GetSpeciesList();
	if (list->NumSpecies() == 0)
		return;

	// Open File Save Dialog
	wxFileDialog saveFile(NULL, _("Export vegetation to SHP"), _T(""), _T("plant_list.html"),
		FSTRING_SHP, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (saveFile.ShowModal() == wxID_CANCEL)
		return;
	wxString strPathName = saveFile.GetPath();

	list->WriteHTML(strPathName.mb_str());
}

void MainFrame::OnUpdateVegExportSHP(wxUpdateUIEvent& event)
{
	vtVegLayer *pVeg = GetMainFrame()->GetActiveVegLayer();
	event.Enable(pVeg && pVeg->IsNative());
}

void MainFrame::OnAreaGenerateVeg(wxCommandEvent& event)
{
	// Open File Save Dialog
	wxFileDialog saveFile(NULL, _("Save Vegetation File"), _T(""), _T(""),
		FSTRING_VF, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (saveFile.ShowModal() == wxID_CANCEL)
		return;
	wxString strPathName = saveFile.GetPath();

	DistribVegDlg dlg(this, -1, _("Vegetation Distribution Options"));

	if (dlg.ShowModal() == wxID_CANCEL)
		return;

	// Generate the plants
	GenerateVegetation(strPathName.mb_str(wxConvUTF8), m_area, dlg.m_opt);
}

void MainFrame::OnUpdateAreaGenerateVeg(wxUpdateUIEvent& event)
{
	// we needs some species, and an area to plant them in
	event.Enable(m_strSpeciesFilename != "" && !m_area.IsEmpty());
}

void MainFrame::OnAreaVegDensity(wxCommandEvent& event)
{
	wxString str, s;

	LinearUnits lu = m_crs.GetUnits();
	float xsize = m_area.Width() * GetMetersPerUnit(lu);
	float ysize = m_area.Height() * GetMetersPerUnit(lu);
	float area = xsize * ysize;

	s.Printf(_("Total area: %.1f square meters (%.3f hectares)\n"), area, area/10000);
	str += s;

	// Get all the objects we'll need
	vtVegLayer *vlay = (vtVegLayer *) FindLayerOfType(LT_VEG);
	if (!vlay) return;
	vtPlantInstanceArray *pia = vlay->GetPIA();
	if (!pia) return;
	uint ent = pia->NumEntities();
	vtSpeciesList *list = GetMainFrame()->GetSpeciesList();

	// Put the results in a biotype as well
	vtBioType btype;

	float size;
	short species;
	int total = 0;
	for (uint i = 0; i < list->NumSpecies(); i++)
	{
		int count = 0;
		float height = 0;
		for (uint j = 0; j < ent; j++)
		{
			pia->GetPlant(j, size, species);
			DPoint2 &p = pia->GetPoint(j);
			if (species == i && m_area.ContainsPoint(p))
			{
				total++;
				count++;
				height += size;
			}
		}
		if (count != 0)
		{
			vtPlantSpecies *spe = list->GetSpecies(i);
			float density = (float) count / area;
			s.Printf(_("  %d instances of species %hs: %.5f per m^2, average height %.1f\n"),
				count, spe->GetSciName(), density, height/count);
			str += s;

			btype.AddPlant(spe, density, height/count);
		}
	}
	s.Printf(_("Total plant instances: %d\n"), total);
	str += s;
	wxMessageBox(str, _("Info"));

	vtBioRegion bregion;
	btype.m_name = "Default";
	bregion.AddType(&btype);
	bregion.WriteXML("bioregion.xml");
	bregion.Clear();
}

void MainFrame::OnUpdateAreaVegDensity(wxUpdateUIEvent& event)
{
	// we needs some plants, and an area to estimate
	vtVegLayer *vlay = (vtVegLayer *) FindLayerOfType(LT_VEG);
	event.Enable(m_strSpeciesFilename != "" && vlay != NULL &&
		vlay->GetVegType() == VLT_Instances && !m_area.IsEmpty());
}


//////////////////////////////
// Utilities Menu

void MainFrame::OnTowerSelect(wxCommandEvent& event)
{
	m_pView->SetMode(LB_TowerSelect);

}
void MainFrame::OnUpdateTowerSelect(wxUpdateUIEvent &event)
{
	event.Check(m_pView->GetMode()== LB_TowerSelect);
}
void MainFrame::OnTowerEdit(wxCommandEvent& event)
{
}
void MainFrame::OnUpdateTowerEdit(wxUpdateUIEvent &event)
{
}
void MainFrame::OnTowerAdd(wxCommandEvent& event)
{
	m_pView->SetMode(LB_TowerAdd);
}
void MainFrame::OnUpdateTowerAdd(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode()==LB_TowerAdd);
}


/////////////////////////////////////
// Buildings / Features

void MainFrame::OnFeatureSelect(wxCommandEvent &event)
{
	m_pView->SetMode(LB_FSelect);
}

void MainFrame::OnUpdateFeatureSelect(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode() == LB_FSelect);
}

void MainFrame::OnFeaturePick(wxCommandEvent &event)
{
	m_pView->SetMode(LB_FeatInfo);
}

void MainFrame::OnFeatureTable(wxCommandEvent &event)
{
	if (m_pFeatInfoDlg && m_pFeatInfoDlg->IsShown())
		m_pFeatInfoDlg->Show(false);
	else
	{
		ShowFeatInfoDlg();
		m_pFeatInfoDlg->SetLayer(GetActiveLayer());
		m_pFeatInfoDlg->SetFeatureSet(GetActiveRawLayer()->GetFeatureSet());
	}
}

void MainFrame::OnUpdateFeaturePick(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode() == LB_FeatInfo);
}

void MainFrame::OnUpdateFeatureTable(wxUpdateUIEvent& event)
{
	event.Check(m_pFeatInfoDlg && m_pFeatInfoDlg->IsShown());
}

void MainFrame::OnBuildingEdit(wxCommandEvent &event)
{
	m_pView->SetMode(LB_BldEdit);
}

void MainFrame::OnUpdateBuildingEdit(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode() == LB_BldEdit);
}

void MainFrame::OnBuildingAddPoints(wxCommandEvent &event)
{
	m_pView->SetMode(LB_BldAddPoints);
}

void MainFrame::OnUpdateBuildingAddPoints(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode() == LB_BldAddPoints);
}

void MainFrame::OnBuildingDeletePoints(wxCommandEvent &event)
{
	m_pView->SetMode(LB_BldDeletePoints);
}

void MainFrame::OnUpdateBuildingDeletePoints(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode() == LB_BldDeletePoints);
}

void MainFrame::OnStructureAddLinear(wxCommandEvent &event)
{
	m_pView->SetMode(LB_AddLinear);
}

void MainFrame::OnUpdateStructureAddLinear(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode() == LB_AddLinear);
}

void MainFrame::OnStructureEditLinear(wxCommandEvent &event)
{
	m_pView->SetMode(LB_EditLinear);
}

void MainFrame::OnUpdateStructureEditLinear(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode() == LB_EditLinear);
}

void MainFrame::OnStructureAddInstances(wxCommandEvent &event)
{
	m_pView->SetMode(LB_AddInstance);
}

void MainFrame::OnUpdateStructureAddInstances(wxUpdateUIEvent& event)
{
	event.Check(m_pView->GetMode() == LB_AddInstance);
}

void MainFrame::OnStructureAddFoundation(wxCommandEvent &event)
{
	vtStructureLayer *pSL = GetActiveStructureLayer();
	vtElevLayer *pEL = (vtElevLayer *) FindLayerOfType(LT_ELEVATION);
	pSL->AddFoundations(pEL);
}

void MainFrame::OnUpdateStructureAddFoundation(wxUpdateUIEvent& event)
{
	vtStructureLayer *pSL = GetActiveStructureLayer();
	vtElevLayer *pEL = (vtElevLayer *) FindLayerOfType(LT_ELEVATION);
	event.Enable(pSL != NULL && pEL != NULL);
}

void MainFrame::OnStructureConstrain(wxCommandEvent &event)
{
	m_pView->m_bConstrain = !m_pView->m_bConstrain;
}

void MainFrame::OnUpdateStructureConstrain(wxUpdateUIEvent& event)
{
	event.Check(m_pView->m_bConstrain);
}

void MainFrame::OnStructureSelectUsingPolygons(wxCommandEvent &event)
{
	vtStructureLayer *pStructureLayer = GetActiveStructureLayer();

	if (!pStructureLayer)
		return;

	pStructureLayer->DeselectAll();

	int iNumLayers = m_Layers.size();
	for (int i = 0; i < iNumLayers; i++)
	{
		vtLayer *pLayer = m_Layers[i];
		if (LT_RAW != pLayer->GetType())
			continue;

		vtRawLayer* pRawLayer = dynamic_cast<vtRawLayer*>(pLayer);
		if ((NULL != pRawLayer) && (wkbPolygon == wkbFlatten(pRawLayer->GetGeomType())))
		{
			vtFeatureSetPolygon *pFeatureSetPolygon = dynamic_cast<vtFeatureSetPolygon*>(pRawLayer->GetFeatureSet());
			if (NULL != pFeatureSetPolygon)
			{
				uint iNumEntities = pFeatureSetPolygon->NumEntities();
				uint iIndex;
				for (iIndex = 0; iIndex < iNumEntities; iIndex++)
				{
					if (pFeatureSetPolygon->IsSelected(iIndex))
					{
						uint iIndex2;
						const DPolygon2 Polygon = pFeatureSetPolygon->GetPolygon(iIndex);
						uint iNumStructures = pStructureLayer->size();
						for (iIndex2 = 0; iIndex2 < iNumStructures; iIndex2++)
						{
							DRECT Extents;
							if (pStructureLayer->at(iIndex2)->GetExtents(Extents))
							{
								DPoint2 Point((Extents.left + Extents.right)/2, (Extents.bottom + Extents.top)/2);
								if (Polygon.ContainsPoint(Point))
									pStructureLayer->at(iIndex2)->Select(true);
							}
						}
					}
				}
			}
		}
	}
	m_pView->Refresh();
}

void MainFrame::OnUpdateStructureSelectUsingPolygons(wxUpdateUIEvent &event)
{
	bool bFoundSelectedPolygons = false;
	int iNumLayers = m_Layers.size();
	for (int i = 0; i < iNumLayers; i++)
	{
		vtLayer *pLayer = m_Layers[i];
		if (LT_RAW == pLayer->GetType())
		{
			vtRawLayer* pRawLayer = dynamic_cast<vtRawLayer*>(pLayer);
			if ((NULL != pRawLayer) && (wkbPolygon == wkbFlatten(pRawLayer->GetGeomType())))
			{
				vtFeatureSet *pFeatureSet = pRawLayer->GetFeatureSet();
				if ((NULL != pFeatureSet) && (pFeatureSet->NumSelected() > 0))
				{
					bFoundSelectedPolygons = true;
					break;
				}
			}
		}
	}
	event.Enable(bFoundSelectedPolygons);
}

void MainFrame::OnStructureColourSelectedRoofs(wxCommandEvent& event)
{
	vtStructureLayer *pLayer = GetActiveStructureLayer();
	if (!pLayer)
		return;

	wxColour Colour = wxGetColourFromUser(this);
	if (Colour.Ok())
	{
		RGBi RoofColour(Colour.Red(), Colour.Green(), Colour.Blue());
		for (uint i = 0; i < pLayer->size(); i++)
		{
			vtStructure *pStructure = pLayer->at(i);
			if (!pStructure->IsSelected())
				continue;

			vtBuilding* pBuilding = pStructure->GetBuilding();
			if (pBuilding)
				pBuilding->GetLevel(pBuilding->NumLevels() - 1)->SetEdgeColor(RoofColour);
		}
	}
}

void MainFrame::OnStructureCleanFootprints(wxCommandEvent& event)
{
	vtStructureLayer *pLayer = GetActiveStructureLayer();
	if (!pLayer)
		return;

	double dEpsilon;
	if (m_crs.GetUnits() == LU_DEGREES)
		dEpsilon = 1E-7;
	else
		dEpsilon = 1E-2;

	wxString str;
	str.Printf(_T("%g"), dEpsilon);
	str = wxGetTextFromUser(_("How close are degenerate points? (epsilon)"),
		_("Clean Footprints"), str, this);
	if (str == _T(""))
		return;

	dEpsilon = atof(str.mb_str(wxConvUTF8));

	int degen = 0;
	int overlap = 0;
	pLayer->CleanFootprints(dEpsilon, degen, overlap);

	if (degen)
		DisplayAndLog("%d degenerate points were removed.", degen);
	if (overlap)
		DisplayAndLog("%d overlapping points were removed.", overlap);
	if (!degen && !overlap)
		DisplayAndLog("No degenerate or overlapping points were found.");
}

void MainFrame::OnStructureSelectIndex(wxCommandEvent& event)
{
	vtStructureLayer *pLayer = GetActiveStructureLayer();
	if (!pLayer)
		return;
	int num = pLayer->size();
	if (num == 0)
		return;
	wxString msg;
	msg.Printf(_("Index (0 .. %d)"), num-1);
	int idx = wxGetNumberFromUser(msg, _T(""), _("Index"), 0, 0, num-1);
	if (idx == -1)
		return;

	pLayer->DeselectAll();
	vtStructure *stru = pLayer->at(idx);
	if (stru)
	{
		stru->Select(true);
		DRECT r;
		stru->GetExtents(r);
		m_pView->ZoomToRect(r, 0.1f);
	}
}

void MainFrame::OnUpdateStructureColourSelectedRoofs(wxUpdateUIEvent& event)
{
	event.Enable((NULL != GetActiveStructureLayer()) && (GetActiveStructureLayer()->NumSelected() > 0));
}

void MainFrame::OnStructureExportFootprints(wxCommandEvent& event)
{
	// Open File Save Dialog
	wxFileDialog saveFile(NULL, _("Export footprints to SHP"), _T(""), _T(""),
		FSTRING_SHP, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (saveFile.ShowModal() == wxID_CANCEL)
		return;
	wxString strPathName = saveFile.GetPath();

	vtStructureLayer *pLayer = GetActiveStructureLayer();
	pLayer->WriteFootprintsToSHP(strPathName.mb_str(wxConvUTF8));
}

void MainFrame::OnStructureExportCanoma(wxCommandEvent& event)
{
	// Open File Save Dialog
	wxFileDialog saveFile(NULL, _("Export footprints to Canoma3DV"), _T(""), _T(""),
		FSTRING_3DV, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (saveFile.ShowModal() == wxID_CANCEL)
		return;
	wxString strPathName = saveFile.GetPath();

	vtStructureLayer *pLayer = GetActiveStructureLayer();
	DRECT area;
	if (m_area.IsEmpty())
		// If the area tool isn't set, use the whole layer extents
		pLayer->GetExtents(area);
	else
		area = m_area;

	vtHeightField *pHF = NULL;
	vtElevLayer *pE = (vtElevLayer *)GetMainFrame()->FindLayerOfType(LT_ELEVATION);
	if (pE)
		pHF = pE->GetHeightField();

	pLayer->WriteFootprintsToCanoma3DV(strPathName.mb_str(wxConvUTF8), &area, pHF);
}

void MainFrame::OnUpdateStructureExportFootprints(wxUpdateUIEvent& event)
{
	event.Enable(NULL != GetActiveStructureLayer());
}


///////////////////////////////////
// Raw menu

void MainFrame::OnUpdateRawIsActive(wxUpdateUIEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();
	event.Enable(pRL != NULL);
}

void MainFrame::OnUpdateRawIsActive3D(wxUpdateUIEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();
	event.Enable(pRL != NULL &&
		// if the current layer is a 3D type
		(pRL->GetGeomType() == wkbPoint25D || pRL->GetGeomType() == wkbLineString25D));
}

void MainFrame::OnUpdateRawIsPolygon(wxUpdateUIEvent& event)
{
	// if the current layer is polygon
	vtRawLayer *pRL = GetActiveRawLayer();
	event.Enable(pRL != NULL && pRL->GetGeomType() == wkbPolygon);
}

void MainFrame::OnUpdateRawIsPoint(wxUpdateUIEvent& event)
{
	// if the current layer is polygon
	vtRawLayer *pRL = GetActiveRawLayer();
	event.Enable(pRL != NULL && (pRL->GetGeomType() == wkbPoint ||
								 pRL->GetGeomType() == wkbPoint25D));
}

void MainFrame::OnUpdateRawHasPolylines(wxUpdateUIEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();
	event.Enable(pRL != NULL && (pRL->GetGeomType() == wkbPolygon ||
		pRL->GetGeomType() == wkbLineString));
}

void MainFrame::OnRawSetType(wxCommandEvent& event)
{
	static OGRwkbGeometryType types[5] = {
		wkbNone,
		wkbPoint,
		wkbPoint25D,
		wkbLineString,
		wkbPolygon
	};
	wxString choices[5];
	for (int i = 0; i < 5; i++)
		choices[i] = wxString::FromAscii(OGRGeometryTypeToName(types[i]));

	int n = 5;
	int cur_type = 0;

	wxSingleChoiceDialog dialog(this, _("Raw Layer Type"),
		_("Please indicate entity type:"), n, (const wxString *)choices);

	dialog.SetSelection(cur_type);

	if (dialog.ShowModal() == wxID_OK)
	{
		cur_type = dialog.GetSelection();
		vtRawLayer *pRL = (vtRawLayer *) GetActiveLayer();

		// must set the projection and layername again, as they are reset on
		//  setting geom type
		wxString name = pRL->GetLayerFilename();
		pRL->SetGeomType(types[cur_type]);
		pRL->SetCRS(m_crs);
		pRL->SetLayerFilename(name);
		RefreshTreeStatus();
	}
}

void MainFrame::OnUpdateRawSetType(wxUpdateUIEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();
	event.Enable(pRL != NULL && pRL->GetGeomType() == wkbNone);
}

void MainFrame::OnRawAddPoints(wxCommandEvent& event)
{
	m_pView->SetMode(LB_AddPoints);
}

void MainFrame::OnUpdateRawAddPoints(wxUpdateUIEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();
	event.Enable(pRL != NULL &&
		(pRL->GetGeomType() == wkbPoint ||
		 pRL->GetGeomType() == wkbPoint25D));
	event.Check(m_pView->GetMode() == LB_AddPoints);
}

void MainFrame::OnRawAddPointText(wxCommandEvent& event)
{
	wxString str = wxGetTextFromUser(_("(X, Y) in current projection"),
			_("Enter coordinate"));
	if (str == _T(""))
		return;
	DPoint2 p;
	int num = sscanf(str.mb_str(wxConvUTF8), "%lf, %lf", &p.x, &p.y);
	if (num != 2)
		return;

	vtRawLayer *pRL = GetActiveRawLayer();
	pRL->AddPoint(p);
	m_pView->Refresh();
}

void MainFrame::OnUpdateRawAddPointText(wxUpdateUIEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();
	event.Enable(pRL != NULL && pRL->GetGeomType() == wkbPoint);
}

void MainFrame::OnRawAddPointsGPS(wxCommandEvent& event)
{
}

void MainFrame::OnUpdateRawAddPointsGPS(wxUpdateUIEvent& event)
{
//	vtRawLayer *pRL = GetActiveRawLayer();
//	event.Enable(pRL != NULL && pRL->GetEntityType() == SHPT_POINT);
	event.Enable(false);	// not implemented yet
}

void MainFrame::OnRawAddFeatureWKT(wxCommandEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();
	if (pRL == NULL || pRL->GetGeomType() != wkbLineString)
		return;
	vtFeatureSetLineString *fsls = (vtFeatureSetLineString*) pRL->GetFeatureSet();

	wxString str = wxGetTextFromUser(_("Enter well-known-text (WKT) of a feature (in current CRS)"));
	if (str == _T(""))
		return;
	vtString vs = (const char *) str.ToAscii();

	OGRLineString ols;

	//  Convert from OGC WKT to an OGR feature
	int len = vs.GetLength();
	char *buffer = (char *) CPLMalloc(len+1);
	strcpy(buffer, (const char *) vs);
	char *buffer2 = buffer;
	OGRErr oe = ols.importFromWkt(&buffer2);
	CPLFree(buffer);

	if (oe != OGRERR_NONE)
	{
		DisplayAndLog("Sorry, couldn't parse that text.");
		return;
	}

	// Convert OGR feature to polyline (DLine2)
	DLine2 pline;
	int num_points = ols.getNumPoints();
	for (int j = 0; j < num_points; j++)
		pline.Append(DPoint2(ols.getX(j), ols.getY(j)));

	fsls->AddPolyLine(pline);
	m_pView->Refresh();
}

void MainFrame::OnRawSelectCondition(wxCommandEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();
	vtFeatureSet *pFS = pRL->GetFeatureSet();

	if (!pFS || pFS->NumFields() == 0)
	{
		DisplayAndLog("Can't select by condition because the current\n"
					  "layer has no fields defined.");
		return;
	}
	SelectDlg dlg(this, -1, _("Select"));
	dlg.SetRawLayer(pRL);
	if (dlg.ShowModal() == wxID_OK)
	{
		wxString str = dlg.m_strValue;
		int selected = pFS->SelectByCondition(dlg.m_iField, dlg.m_iCondition,
			str.mb_str(wxConvUTF8));

		wxString msg;
		if (selected == -1)
			msg = _("Unable to select");
		else if (selected == 1)
			msg.Printf(_("Selected 1 entity"));
		else
			msg.Printf(_("Selected %d entities"), selected);
		SetStatusText(msg);

		msg += _T("\n");
		VTLOG1(msg.mb_str(wxConvUTF8));

		m_pView->Refresh(false);
		OnSelectionChanged();
	}
}

void MainFrame::OnRawGenerateTIN(wxCommandEvent& event)
{
	VTLOG1("OnRawGenerateTIN\n");

	vtRawLayer *pRaw = GetActiveRawLayer();
	vtFeatureSet *pSet = pRaw->GetFeatureSet();
	vtTin2d *tin;
	vtFeatureSetPoint3D *setpo3 = dynamic_cast<vtFeatureSetPoint3D *>(pSet);
	vtFeatureSetPolygon *setpg = dynamic_cast<vtFeatureSetPolygon *>(pSet);
	if (setpo3)
		tin = new vtTin2d(setpo3);
	else if (setpg)
	{
		uint n = setpg->NumFields();

		int field_num = -1;
		float height = 0.0f;
		if (n > 0)
		{
			wxArrayString choices;
			for (uint i = 0; i < n; i++)
				choices.Add(wxString(setpg->GetField(i)->m_name, wxConvUTF8));

			// We need to know which field contains height
			field_num = wxGetSingleChoiceIndex(_("Height Field"), _("Generate TIN"),
				choices, this);
			if (field_num == -1)
				return;
		}
		else
		{
			wxString str;
			str.Printf(_T("%g"), height);
			str = wxGetTextFromUser(_("What height to use for the TIN surface?"),
				_("Generate TIN"), str, this);
			if (str == _T(""))
				return;

			height = atof(str.mb_str(wxConvUTF8));
		}
		tin = new vtTin2d(setpg, field_num, height);
	}
	else
	{
		DisplayAndLog("Must be polygons with height attribute, or 3D points");
		return;
	}

	vtElevLayer *pEL = new vtElevLayer;
	pEL->SetTin(tin);

	// inherit name
	wxString lname = pRaw->GetLayerFilename();
	RemoveFileExtensions(lname);
	pEL->SetLayerFilename(lname + wxString(".itf", wxConvUTF8));

	AddLayer(pEL);
	SetActiveLayer(pEL);

	m_pView->Refresh();
	RefreshTreeView();
}

void MainFrame::OnRawConvertToPolygons(wxCommandEvent& event)
{
	vtRawLayer *pRaw = GetActiveRawLayer();
	vtFeatureSet *pSet = pRaw->GetFeatureSet();

	vtFeatureSetLineString *setls2 = dynamic_cast<vtFeatureSetLineString *>(pSet);
	vtFeatureSetLineString3D *setls3 = dynamic_cast<vtFeatureSetLineString3D *>(pSet);
	vtFeatureSet *newset = NULL;

	if (setls2)
	{
		vtFeatureSetPolygon *polys = new vtFeatureSetPolygon;

		for (uint i = 0; i < setls2->NumEntities(); i++)
		{
			const DLine2 &polyline = setls2->GetPolyLine(i);
			int npoints = polyline.GetSize();
			if (polyline[0] == polyline[npoints-1])
			{
				DPolygon2 dpoly;
				dpoly.push_back(polyline);

				// Omit the first/last point (duplicate)
				dpoly[0].RemoveAt(npoints-1);

				polys->AddPolygon(dpoly);
			}
		}
		if (polys->NumEntities() == 0)
		{
			DisplayAndLog("Didn't find any closed polylines");
			delete polys;
			return;
		}
		newset = polys;
	}
	else if (setls3)
	{
		// TODO; we don't actually do 3D polygons yet.
	}
	else
	{
		DisplayAndLog("Must be polylines (linestrings), either 2D or 3D");
		return;
	}

	vtRawLayer *pNewRaw = new vtRawLayer;
	pNewRaw->SetFeatureSet(newset);

	// inherit name
	wxString fname = pRaw->GetLayerFilename();
	RemoveFileExtensions(fname);
	pNewRaw->SetLayerFilename(fname + wxString("-poly.shp", wxConvUTF8));

	AddLayer(pNewRaw);
	SetActiveLayer(pNewRaw);

	m_pView->Refresh();
	RefreshTreeView();
}

void CapWords(vtString &str)
{
	bool bStart = true;
	for (int i = 0; i < str.GetLength(); i++)
	{
		char ch = str[i];
		if (bStart)
			ch = toupper(ch);
		else
			ch = tolower(ch);
		str.SetAt(i, ch);

		if (ch == ' ')
			bStart = true;
		else
			bStart = false;
	}
}

void MainFrame::OnRawExportKML(wxCommandEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();
	if (!pRL)
		return;

	vtFeatureSet *fset = pRL->GetFeatureSet();

	//ImageMapDlg dlg(this, -1, _("Export Image Map"));
	//dlg.SetFields(fset);
	//if (dlg.ShowModal() != wxID_OK)
	//	return;

	wxFileDialog loadFile(NULL, _("Save to KML File"), _T(""), _T(""),
		FSTRING_KML, wxFD_SAVE);
	if (loadFile.ShowModal() != wxID_OK)
		return;
	fset->SaveToKML(loadFile.GetPath().mb_str(wxConvUTF8));
}

void MainFrame::OnRawGenElevation(wxCommandEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();
	if (!pRL)
		return;

	vtFeatureSet *pSet = pRL->GetFeatureSet();
	DRECT extent;
	pSet->ComputeExtent(extent);

	bool bIsGeo = (m_crs.IsGeographic() != 0);

	GenGridDlg dlg(this, -1, _("Generate Grid from 3D Points"), bIsGeo);
	dlg.m_fAreaX = extent.Width();
	dlg.m_fAreaY = extent.Height();
	dlg.m_Size.x = 512;
	dlg.m_Size.y = 512;
	dlg.RecomputeSize();
	dlg.m_fDistanceCutoff = 1.5f;

	int ret = dlg.ShowModal();
	if (ret == wxID_CANCEL)
		return;

	vtElevLayer *el = new vtElevLayer;

	OpenProgressDialog(_T("Creating Grid"), _T(""), true);
	int xsize = 800;
	int ysize = 300;
	if (el->CreateFromPoints(pSet, dlg.m_Size, dlg.m_fDistanceCutoff))
		AddLayerWithCheck(el);
	else
		delete el;
	CloseProgressDialog();
}

void MainFrame::OnUpdateRawGenElevation(wxUpdateUIEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();
	event.Enable(pRL != NULL && pRL->GetGeomType() == wkbPoint25D);
}

void MainFrame::OnRawStyle(wxCommandEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();

	DrawStyle style = pRL->GetDrawStyle();
	style.m_LineColor.Set(0,0,0);
	pRL->SetDrawStyle(style);
}

void MainFrame::OnRawScaleH(wxCommandEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();

	wxString str = _T("1");
	str = wxGetTextFromUser(_("Scale factor?"),	_("Scale Raw Layer"),
		str, this);
	if (str == _T(""))
		return;

	double value = atof(str.mb_str(wxConvUTF8));
	pRL->ScaleHorizontally(value);
	m_pView->Refresh();
}

void MainFrame::OnRawScaleV(wxCommandEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();

	wxString str = _T("1");
	str = wxGetTextFromUser(_("Scale factor?"),	_("Scale Raw Layer"),
		str, this);
	if (str == _T(""))
		return;

	double value = atof(str.mb_str(wxConvUTF8));
	pRL->ScaleVertically(value);
	m_pView->Refresh();
}

void MainFrame::OnRawOffsetV(wxCommandEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();

	wxString str = _T("0");
	str = wxGetTextFromUser(_("Vertical offset?"),	_("Offset Raw Layer"),
		str, this);
	if (str == _T(""))
		return;

	double value = atof(str.mb_str(wxConvUTF8));
	pRL->OffsetVertically(value);
	m_pView->Refresh();
}

void MainFrame::OnRawClean(wxCommandEvent& event)
{
	VTLOG1("OnRawClean\n");

	// Get the featureset we're going to clean.
	vtRawLayer *pRL = GetActiveRawLayer();
	vtFeatureSet *fset = pRL->GetFeatureSet();
	vtFeatureSetPolygon *fspoly = dynamic_cast<vtFeatureSetPolygon*>(fset);
	vtFeatureSetLineString *fsline = dynamic_cast<vtFeatureSetLineString*>(fset);
	if (!fspoly && !fsline)
		return;

	wxString str = _T("0.10");
	str = wxGetTextFromUser(_("Distance threshhold for proximity and co-linearity?"),
		_("Clean Raw Layer"), str, this);
	if (str == _T(""))
		return;

	double value = atof(str.mb_str(wxConvUTF8));
	int removed;
	if (fspoly)
		removed = fspoly->FixGeometry(value);
	if (fsline)
		removed = fsline->FixGeometry(value);

	str.Printf(_("Removed %d degenerate points"), removed);
	DisplayAndLog(str);

	if (removed != 0)
		pRL->SetModified(true);

	m_pView->Refresh();
}

void MainFrame::OnRawSelectBad(wxCommandEvent& event)
{
	vtRawLayer *pRL = GetActiveRawLayer();
	vtFeatureSetPolygon *fsp = (vtFeatureSetPolygon*) pRL->GetFeatureSet();

	wxString str = _T("0.10");
	str = wxGetTextFromUser(_("Distance threshhold for proximity?"),
		_("Select bad polygons"), str, this);
	if (str == _T(""))
		return;

	double value = atof(str.mb_str(wxConvUTF8));
	int bad = fsp->SelectBadFeatures(value);

	str.Printf(_("Found %d degenerate polygons"), bad);
	wxMessageBox(str);

	m_pView->Refresh();
}


////////////////////
// Help

void MainFrame::OnHelpAbout(wxCommandEvent &event)
{
	wxString str = _("Virtual Terrain Builder\nPowerful, easy to use, free!\n");
	str += _T("\n");
	str += _("Please read the HTML documentation and license.\n");
	str += _T("\n");
	str += _("Send feedback to: ");
	str += _T("ben@vterrain.org\n");
	str += _T("\nVersion: ");
	str += _T(VTP_VERSION);
	str += _T("\n");
	str += _("Build date: ");
	str += wxString(__DATE__, wxConvUTF8);
	str += _T("\n");

#if defined(_MSC_VER) && defined(_DEBUG)	// == 1300 for VC7.1
	_CrtMemState state;
	_CrtMemCheckpoint(&state);
	int iAllocated = state.lSizes[1] + state.lSizes[2];
	wxString str3;
	str3.Printf(_T("Memory in use: %d bytes (%.0fK, %.1fMB)"), iAllocated,
		(float)iAllocated/1024, (float)iAllocated/1024/1024);
	str += _T("\n");
	str += str3;
#endif

	wxString str2 = _("About ");
	str2 += wxString("VTBuilder", wxConvUTF8);
	wxMessageBox(str, str2);
}

void MainFrame::OnHelpDocLocal(wxCommandEvent &event)
{
	vtString local_lang_code = (const char *) wxGetApp().GetLanguageCode().mb_str(wxConvUTF8);
	local_lang_code = local_lang_code.Left(2);

	// Launch default web browser with documentation pages
	LaunchAppDocumentation("VTBuilder", local_lang_code);
}

void MainFrame::OnHelpDocOnline(wxCommandEvent &event)
{
	// Launch default web browser with documentation pages
	wxLaunchDefaultBrowser(_T("http://vterrain.org/Doc/VTBuilder/"));
}

////////////////////
// Popup menu items

void MainFrame::OnDistanceClear(wxCommandEvent &event)
{
	ClearDistance();
}

void MainFrame::OnShowAll(wxCommandEvent& event)
{
	for (uint i = 0; i < m_Layers.size(); i++)
	{
		vtLayer *lp = m_Layers[i];
		if (lp->GetType() == m_pTree->m_clicked_layer_type)
		{
			lp->SetVisible(true);
			RefreshView();
		}
	}
	RefreshTreeStatus();
}

void MainFrame::OnHideAll(wxCommandEvent& event)
{
	for (uint i = 0; i < m_Layers.size(); i++)
	{
		vtLayer *lp = m_Layers[i];
		if (lp->GetType() == m_pTree->m_clicked_layer_type)
		{
			lp->SetVisible(false);
			RefreshView();
		}
	}
	RefreshTreeStatus();
}

void MainFrame::OnLayerPropsPopup(wxCommandEvent& event)
{
	wxTreeItemId itemId = m_pTree->GetSelection();
	MyTreeItemData *data = (MyTreeItemData *)m_pTree->GetItemData(itemId);
	if (!data)
		return;
	ShowLayerProperties(data->m_pLayer);
}

void MainFrame::OnLayerToTop(wxCommandEvent& event)
{
	wxTreeItemId itemId = m_pTree->GetSelection();
	MyTreeItemData *data = (MyTreeItemData *)m_pTree->GetItemData(itemId);
	if (!data)
		return;

	int num = LayerNum(data->m_pLayer);
	if (num != 0)
	{
		SwapLayerOrder(0, num);
		RefreshView();
		RefreshTreeView();
	}
}

void MainFrame::OnLayerToBottom(wxCommandEvent& event)
{
	wxTreeItemId itemId = m_pTree->GetSelection();
	MyTreeItemData *data = (MyTreeItemData *)m_pTree->GetItemData(itemId); 
	if (!data)
		return;

	int total = NumLayers();
	int num = LayerNum(data->m_pLayer);
	if (num != total-1)
	{
		SwapLayerOrder(num, total-1);
		RefreshView();
		RefreshTreeView();
	}
}

void MainFrame::OnLayerOverviewDisk(wxCommandEvent& event)
{
	wxTreeItemId itemId = m_pTree->GetSelection();
	MyTreeItemData *data = (MyTreeItemData *)m_pTree->GetItemData(itemId);
	if (!data)
		return;
	vtImageLayer *pIL = (vtImageLayer *) data->m_pLayer;

	OpenProgressDialog(_("Creating Overviews"), _T(""), false, this);

	pIL->GetImage()->CreateOverviews();

	CloseProgressDialog();
}

void MainFrame::OnLayerOverviewMem(wxCommandEvent& event)
{
	wxTreeItemId itemId = m_pTree->GetSelection();
	MyTreeItemData *data = (MyTreeItemData *)m_pTree->GetItemData(itemId);
	if (!data)
		return;
	vtImageLayer *pIL = (vtImageLayer *) data->m_pLayer;

	OpenProgressDialog(_("Creating MipMaps"), _T(""), false, this);

	pIL->GetImage()->AllocMipMaps();
	pIL->GetImage()->DrawMipMaps();

	CloseProgressDialog();
}

void MainFrame::OnMRUFileProject(wxCommandEvent& event)
{
    int n = event.GetId() - ID_FIRST_MRU_FILE;  // the index in MRU list
	wxString fname(m_ProjectFiles[n], wxConvUTF8);

	// this method will update the MRU if successful
	LoadProject(fname);
}

void MainFrame::OnMRUFileLayer(wxCommandEvent& event)
{
    int n = event.GetId() - ID_FIRST_MRU_LAYER;  // the index in MRU list
	wxString fname(m_LayerFiles[n], wxConvUTF8);

	// this method will update the MRU if successful
	LoadLayer(fname);
}

void MainFrame::OnMRUFileImport(wxCommandEvent& event)
{
    int n = event.GetId() - ID_FIRST_MRU_IMPORT;  // the index in MRU list
	wxString fname(m_ImportFiles[n], wxConvUTF8);

	if (ImportDataFromArchive(LT_UNKNOWN, fname, true) != 0)
	{
		// succeeded, bring to the top of the MRU
		AddToMRU(m_ImportFiles, (const char *) fname.mb_str(wxConvUTF8));
	}
}
