//
// class Enviro: Main functionality of the Enviro application
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ENVIROH
#define ENVIROH

#include "vtlib/core/TerrainScene.h"

#include "vtdata/Building.h"
#include "vtdata/Fence.h"
#include "vtdata/vtCRS.h"
#include "vtlib/core/AnimPath.h"
#include "vtlib/core/Elastic.h"
#include "vtlib/core/NavEngines.h"
#include "vtlib/core/Vehicles.h"
#include "EnviroEnum.h"
#include "PlantingOptions.h"

#define INITIAL_SPACE_DIST	3.1f

// Use forward declarations to minimize rebuild dependencies
class vtTerrainScene;
class vtTerrain;
class TerrainPicker;
class vtIcoGlobe;
class vtLine3d;
class vtPole3d;
class vtUtilNode;
class vtFence3d;
class vtAbstractLayer;
class GlobeLayer;

// Engines
class GlobePicker;
class GrabFlyer;
class FlatFlyer;
class MapOverviewEngine;
class CarEngine;
class vtSpriteSizer;

// Plants
class vtSpeciesList3d;

struct ControlEngine : public vtEngine
{
	ControlEngine() : m_pEnvironment(NULL) { }
	void Eval();
	class Enviro *m_pEnvironment;
};

/**
 The central, core class of the Enviro applcation.  It maintains a collection
 of terrains (by extending vtTerrainScene) and adds all the non-GUI functionality,
 including handling mouse events in the 3D view, navigating, creating and
 editing objects.

 It is extended by the GUI code as the class EnviroGUI, of which there is one
 global instance (singleton), called "g_App".
 */
class Enviro : public vtTerrainScene
{
public:
	Enviro();
	virtual ~Enviro();

	// methods
	void Startup();
	void Shutdown();
	virtual void StartupArgument(int i, const char *str);

	void LoadAllTerrainDescriptions();
	void LoadGlobalContent();
	void StartControlEngine();
	void DoControl();
	void DoControlOrbit();
	void DoControlTerrain();
	void SwitchToTerrain(vtTerrain *pTerrain);
	void SelectInitialViewpoint(vtTerrain *pTerrain);
	vtGroup *GetRoot() { return m_pRoot; }
	void StoreTerrainParameters();
	osgText::Font *GetArial() { return m_pArial; }

	void ShowElevationLegend(bool bShow);
	bool GetShowElevationLegend();
	void ShowCompass(bool bShow);
	bool GetShowCompass();
	void ShowMapOverview(bool bShow);
	bool GetShowMapOverview();
	void TextureHasChanged();

	// navigation and camera
	virtual void SetFlightSpeed(float speed);
	float GetFlightSpeed();
	virtual void SetNavDamping(float factor);
	float GetNavDamping();
	void SetFlightAccel(bool bAccel);
	bool GetFlightAccel();
	void SetTopDown(bool bTopDown);
	void SetCurrentNavigator(vtTerrainFlyer *pE);
	void EnableFlyerEngine(bool bEnable);
	void SetNavType(NavType nav);
	NavType GetNavType() {return m_nav; }
	void SetMaintain(bool bOn);
	bool GetMaintain();
	void ResetCamera();

	void SetMode(MouseMode mode);
	void DumpCameraInfo();
	void SetSpeed(float x);
	float GetSpeed();
	void GetStatusString(int which, vtString &str1, vtString &str2);

	void ActivateAStructureLayer();
	void ActivateAVegetationLayer();

	// go to space or a terrain
	void FlyToSpace();
	bool RequestTerrain(const char *name);
	void RequestTerrain(vtTerrain *pTerr);

	// these work in space
	vtIcoGlobe *GetGlobe() { return m_pIcoGlobe; }
	vtIcoGlobe *GetOverlayGlobe() { return m_pOverlayGlobe; }
	void SetEarthShading(bool bShade);
	bool GetEarthShading();
	void SetEarthTilt(bool bTilt);
	bool GetEarthTilt();
	void SetEarthShape(bool Flat);
	bool GetEarthShape() { return m_bGlobeFlat; }
	void SetEarthUnfold(bool Flat);
	bool GetEarthUnfold() { return m_bGlobeUnfolded; }
	void SetSpaceAxes(bool bShow);
	bool GetSpaceAxes();
	int AddGlobeAbstractLayer(const char *fname);
	void RemoveGlobeAbstractLayer(GlobeLayer *glay);
	void UpdateEarthArc();
	void SetDisplayedArc(const DPoint2 &g1, const DPoint2 &g2);
	void SetDisplayedArc(const DLine2 &path);
	void SetEarthLines(double lon, double lat);
	void ShowEarthLines(bool bShow);
	vtTerrain *FindTerrainOnEarth(const DPoint2 &p);

	vtString GetMessage1() { return m_strMessage1; }
	vtString GetMessage2() { return m_strMessage2; }
	void SetMessage(const vtString &str1, const vtString &str2 = "", float time = 0.0f);
	void FormatCoordString(vtString &str, const DPoint3 &coord, LinearUnits units, bool seconds = false);
	void DescribeCoordinatesEarth(vtString &str1, vtString &str2);
	void DescribeCoordinatesTerrain(vtString &str1, vtString &str2);
	void DescribeCLOD(vtString &str);

	// location of 3d cursor on the ground, in earth coordinates
	DPoint3			m_EarthPos;
	TerrainPicker	*m_pTerrainPicker;
	GlobePicker		*m_pGlobePicker;
	vtMovGeode		*m_pCursorMGeom;

	// navigation engines
	vtTerrainFlyer	*m_pTFlyer;
	VFlyer			*m_pVFlyer;
	vtOrthoFlyer	*m_pOrthoFlyer;
	QuakeFlyer		*m_pQuakeFlyer;
	vtTerrainFlyer	*m_pCurrentFlyer;
	GrabFlyer		*m_pGFlyer;
	FlatFlyer		*m_pFlatFlyer;
	vtPanoFlyer		*m_pPanoFlyer;
	// engine to keep the camera above the terrain
	vtHeightConstrain *m_pHeightEngine;

	// event handlers
	bool OnMouse(vtMouseEvent &event);
	void OnMouseLeftDownTerrain(vtMouseEvent &event);
	void OnMouseLeftDownTerrainSelect(vtMouseEvent &event);
	void OnMouseLeftDownTerrainMove(vtMouseEvent &event);
	void OnMouseLeftDownOrbit(vtMouseEvent &event);
	void OnMouseLeftUp(vtMouseEvent &event);
	void OnMouseLeftUpBox(vtMouseEvent &event);
	void OnMouseRightDown(vtMouseEvent &event);
	void OnMouseRightUp(vtMouseEvent &event);
	void OnMouseMove(vtMouseEvent &event);
	void OnMouseMoveTerrain(vtMouseEvent &event);
	void OnMouseSelectRayPick(vtMouseEvent &event);
	void OnMouseSelectCursorPick(vtMouseEvent &event);
	bool OnMouseCompass(vtMouseEvent &event);

	// Elastic geometry useful for drawing polylines, such as for building or linears
	void AddElasticPoint(const DPoint2 &p);
	bool IsMakingElastic();
	void CancelElastic();

	// linear structure methods
	void FinishLinear();
	void SetFenceOptions(const vtLinearParams &param, bool bProfileChanged = false);

	// building methods
	void FinishBuilding();
	void FlipBuildingFooprints();
	void SetBuildingEaves(float fLength);
	void CopyBuildingStyle();
	void PasteBuildingStyle();
	bool HaveBuildingStyle();

	// instance methods
	void CreateInstance();
	void CreateInstanceAt(const DPoint2 &pos, vtTagArray *tags);

	// UtilityMap methods
	void StartPowerline();
	void FinishPowerline();
	void SetPowerOptions(const vtString &sStructType);

	// plants
	void LoadSpeciesList();
	vtSpeciesList3d	*GetSpeciesList() { return m_pSpeciesList; }
	PlantingOptions &GetPlantOptions() { return m_PlantOpt; }
	bool PlantATree(const DPoint2 &epos);
	void SetPlantOptions(const PlantingOptions &opt);

	// vehicles
	void SetVehicleOptions(const VehicleOptions &opt);
	CarEngine *CreateGroundVehicle(const VehicleOptions &opt);
	VehicleManager m_VehicleManager;
	VehicleSet m_Vehicles;

	// import
	bool ImportModelFromKML(const char *kmlfile);

	// abstract layers
	vtAbstractLayer *GetLabelLayer() const;
	int NumSelectedAbstractFeatures() const;

	// distance tool
	void SetDistanceToolMode(bool bPath);
	void SetDistanceTool(const DLine2 &line);
	void ResetDistanceTool();
	void UpdateDistanceTool();

	// Wind
	void SetWind(int iDirection, float fSpeed);

	// UI
	void UpdateCompass();
	void SetHUDMessageText(const char *message);
	void ShowVerticalLine(bool bShow);
	bool GetShowVerticalLine();

	// global state
	AppState	m_state;
	MouseMode	m_mode;
	NavType		m_nav;
	bool		m_bOnTerrain;
	vtString	m_strMessage1, m_strMessage2;

	// used for mouse interaction
	bool		m_bDragging;
	bool		m_bDragUpDown;
	bool		m_bRotating;
	bool		m_bSelectedStruct;
	bool		m_bSelectedPlant;
	bool		m_bSelectedUtil;
	bool		m_bSelectedVehicle;
	vtFence3d	*m_pDraggingFence;
	int			m_iDraggingFencePoint;
	DPoint3		m_EarthPosDown;
	DPoint3		m_EarthPosLast;
	vtLine3d	*m_pCurUtilLine;
	vtPole3d	*m_pSelUtilPole;
	IPoint2		m_MouseDown, m_MouseLast;
	float		m_StartRotation;
	bool		m_bConstrainAngles;
	bool		m_bShowVerticalLine;

	// The following can be overridden by GUI code, to handle situations
	//  in which the GUI may need to be informed of what happens.
	virtual void ShowPopupMenu(const IPoint2 &pos) {}
	virtual void SetTerrainToGUI(vtTerrain *pTerrain) {}
	virtual void ShowLayerView() {}
	virtual void RefreshLayerView() {}
	virtual void UpdateLayerView() {}
	virtual void CameraChanged() {}
	virtual void EarthPosUpdated() {}
	virtual void ShowDistance(const DPoint2 &p1, const DPoint2 &p2,
		double fGround, double fVertical) {}
	virtual void ShowDistance(const DLine2 &path,
		double fGround, double fVertical) {}
	virtual vtTagArray *GetInstanceFromGUI() { return NULL; }
	virtual bool OnMouseEvent(vtMouseEvent &event) { return false; }
	virtual void SetTimeEngineToGUI(vtTimeEngine *pEngine) {}
	virtual bool IsAcceptable(vtTerrain *pTerr) { return true; }
	virtual void OnCreateTerrain(vtTerrain *pTerr) {}
	virtual void ShowMessage(const vtString &str) {}
	virtual void SetState(AppState s) { m_state = s; }
	virtual vtString GetStringFromUser(const vtString &title, const vtString &msg) = 0;
	virtual void ShowProgress(bool bShow) {}
	virtual void SetProgressTerrain(vtTerrain *pTerr) {}
	virtual void UpdateProgress(const char *msg1, const char *msg2, int amount1, int amount2) {}
	virtual void AddVehicle(CarEngine *eng) {}
	virtual void RemoveVehicle(CarEngine *eng) {}

	// temporary for demo use
	void MakeDemoGlobe();
	void ToggleDemo();
	vtGroup *m_pDemoGroup;
	vtGeode *m_pDemoTrails;
	void CreateSomeTestVehicles(vtTerrain *pTerrain);
	void MakeOverlayGlobe(vtImage *image, bool progress_callback(int) = NULL);

protected:
	// startup methods
	void LoadTerrainDescriptions(const vtString &path);
	void SetupScene1();
	void SetupScene2();
	virtual void SetupScene3() {}
	// other implementation methods
	void DoCursorOnEarth();
	void DoCursorOnTerrain();
	void MakeGlobe();
	void SetupGlobe();
	void LookUpTerrainLocations();
	void SetupTerrain(vtTerrain *pTerr);
	void SetupArcMaterials();
	void SetupArcMesh();
	void FreeArc();
	void FreeArcMesh();
	void SetTerrainMeasure(const DPoint2 &g1, const DPoint2 &g2);
	void SetTerrainMeasure(const DLine2 &path);
	void CreateElevationLegend();
	void CreateCompass();
	void CreateMapOverview();
	void StartFlyIn();
	void FlyInStage1();
	void FlyInStage2();
	bool IsFlyingInFromSpace() { return (m_iFlightStage != 0); }
	void SetWindowBox(const IPoint2 &ul, const IPoint2 &lr);
	void MakeVerticalLine();
	void UpdateVerticalLine();

	// plants
	vtSpeciesList3d	*m_pSpeciesList;
	PlantingOptions m_PlantOpt;
	bool		m_bPlantsLoaded;

	// vehicles
	VehicleOptions m_VehicleOpt;

	// fence members
	vtLinearParams m_FenceParams;

	// buildings
	vtBuilding m_BuildingStyle;

	// a visible, dynamic line for making buildings, and other uses
	ElasticPolyline m_Elastic;

	// utility map members
	bool		m_bActiveUtilLine;
	vtString	m_sStructType;

	// linear arc on Earth (or Distance Tool on the Terrain)
	vtGeode		*m_pArc;
	vtMaterialArray *m_pArcMats;
	double		m_fArcLength;
	float		m_fDistToolHeight;
	bool		m_bMeasurePath;
	DLine2		m_distance_path;

	// vertical line
	vtGeode		*m_pVertLine;

	// view and navigation
	vtCameraPtr	m_pNormalCamera;
	vtCameraPtr	m_pTopDownCamera;
	bool		m_bTopDown;
	vtEngine	*m_pNavEngines;

	vtGroup		*m_pRoot;		// top of the scene graph

	// globe
	vtTimeEngine	*m_pGlobeTime;
	vtGroup		*m_pGlobeContainer;
	vtIcoGlobe	*m_pIcoGlobe;
	vtIcoGlobe	*m_pOverlayGlobe;
	bool		m_bEarthShade;
	bool		m_bGlobeFlat;
	float		m_fFlattening, m_fFlattenDir;
	bool		m_bGlobeUnfolded;
	float		m_fFolding, m_fFoldDir;
	vtTrackball	*m_pTrackball;
	vtGeode		*m_pSpaceAxes;
	vtGeode		*m_pEarthLines;
	vtMesh		*m_pLineMesh;

	// flattened globe view
	FPQ			m_SpaceLoc;
	FPQ			m_FlatLoc;

	ControlEngine	*m_pControlEng;

	int			m_iInitStep;			// initialization stage
	vtTerrain	*m_pTargetTerrain;		// terrain we are switching to
	FPoint3		m_SpaceTrackballState[3];

	// Animated fly-in from space
	bool		m_bFlyIn;
	int			m_iFlightStage;		// 1, 2
	int			m_iFlightStep;		// 0-100
	FPoint3		m_TrackStart[3], m_TrackPosDiff;
	vtAnimPath	m_FlyInAnim;
	DPoint2		m_FlyInCenter;
	float		m_fTransitionHeight;	// in meters

	// HUD UI
	vtMaterialArrayPtr m_pHUDMaterials;
	vtHUD			*m_pHUD;
	vtTextMesh		*m_pHUDMessage;
	vtFontPtr		m_pArial;
	float			m_fMessageStart, m_fMessageTime;

	vtGeode		*m_pLegendGeom;
	bool		m_bCreatedLegend;

	vtSpriteSizer	*m_pCompassSizer;
	vtGeode		*m_pCompassGeom;
	bool		m_bCreatedCompass;
	bool		m_bDragCompass;
	float		m_fDragAngle;

	vtMesh		*m_pWindowBoxMesh;

	// mapoverviewengine
	MapOverviewEngine *m_pMapOverview;
};

#endif	// ENVIROH

