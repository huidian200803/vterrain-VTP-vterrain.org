//
// SceneOSG.h
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_SCENEH
#define VTOSG_SCENEH

#include <osgViewer/Viewer>
#include <osgViewer/Renderer>
#include <osg/Timer>

#include "../core/Engine.h"

#include "VisualImpactCalculatorOSG.h"

/** \defgroup sg Scene graph
 * These classes define the node of a scene graph and the Scene class which
 * contains them.
 */
/*@{*/

/**
 * This class represents a window, an area of the display which corresponds
 * to an OpenGL drawing surface and has the potential to receive UI events.
 */
class vtWindow
{
public:
	vtWindow();

	/// Set the background color for the window.
	void SetBgColor(const RGBf &color) { m_BgColor = color; }
	RGBf GetBgColor() { return m_BgColor; }

	void SetSize(int w, int h);
	IPoint2 GetSize() { return m_Size; }

protected:
	RGBf m_BgColor;
	IPoint2 m_Size;
};

/**
 * A Scene is the all-encompassing container for all 3D objects
 * that are to be managed and drawn by the scene graph / graphics
 * pipeline functionality of vtlib.
 *
 * A Scene currently encapsulates:
	- A scene graph
	- A set of engines (vtEngine)
	- A window
	- A current camera (vtCamera)
 */
class vtScene
{
public:
	vtScene();
	~vtScene();

	/// Set the root node, the top node of the scene graph.
	void SetRoot(vtGroup *pRoot);

	/// Set global wireframe, which will force all objects to be drawn wireframe.
	void SetGlobalWireframe(bool bWire);
	/// Get the global wireframe state.
	bool GetGlobalWireframe();

	/// Call this method once before calling any other vtlib methods.
	bool Init(int argc, char** argv, bool bStereo = false, int iStereoMode = 0);
	void SetGraphicsContext(osg::GraphicsContext *pGraphicsContext);
	osg::GraphicsContext *GetGraphicsContext();
	osgViewer::GraphicsWindow *GetGraphicsWindow();

	/// Call this method after all other vtlib methods, to free memory.
	void Shutdown();

	void TimerRunning(bool bRun);
	void UpdateBegin();
	void UpdateEngines();
	void UpdateWindow(vtWindow *window);
	void PostDrawEngines();

	// backward compatibility
	void DoUpdate();

	/// Return the instantaneous framerate in frames per seconds estimated.
	float GetFrameRate()
	{
		return 1.0 / m_fLastFrameTime;
	}

	/// Time in seconds since the scene began.
	float GetTime()
	{
		return _timer.delta_s(_initialTick,_frameTick);
	}
	/// Time in seconds between the start of the previous frame and the current frame.
	float GetFrameTime()
	{
		return m_fLastFrameTime;
	}

	// View methods
	bool CameraRay(const IPoint2 &win, FPoint3 &pos, FPoint3 &dir, vtWindow *pWindow = NULL);
	void WorldToScreen(const FPoint3 &point, IPoint2 &result);

	/// Set the top engine in the Engine graph
	void SetRootEngine(vtEngine *ptr) { m_pRootEngine = ptr; }

	/// Get the top engine in the Engine graph
	vtEngine *GetRootEngine() { return m_pRootEngine.get(); }

	/// Set the top engine in the Engine graph
	void SetPostDrawEngine(vtEngine *ptr) { m_pRootEnginePostDraw = ptr; }

	/// Get the top engine in the Engine graph
	vtEngine *GetPostDrawEngine() { return m_pRootEnginePostDraw; }

	/// Add an Engine to the scene. (for backward compatibility only)
	void AddEngine(vtEngine *ptr);

	/// Inform all engines in the scene that a target no longer exists
	void TargetRemoved(osg::Referenced *tar);

	/** Set the camera associated with this scene.  The scene has
	 * a default camera already supplied; you can use GetCamera()
	 * to simply use it instead of making your own.
	 */
	void SetCamera(vtCamera *cam) { m_pCamera = cam; }
	/// Get the camera associated with the scene.
	vtCamera *GetCamera() { return m_pCamera; }

	void OnMouse(vtMouseEvent &event, vtWindow *pWindow = NULL);
	void OnKey(int key, int flags, vtWindow *pWindow = NULL);
	void SetKeyStates(bool *piKeyState) { m_piKeyState = piKeyState; }
	bool GetKeyState(int key);

	void SetWindowSize(int w, int h, vtWindow *pWindow = NULL);
	IPoint2 GetWindowSize(vtWindow *pWindow = NULL);

	bool GetWindowSizeFromOSG();

	/// Get the root node, which is the top of the scene graph.
	vtGroup *GetRoot() { return m_pRoot; }
	osg::StateSet *GetRootState() { return m_StateRoot->getOrCreateStateSet(); }

	// Windows
	void AddWindow(vtWindow *pWindow) {
		m_Windows.Append(pWindow);
	}
	vtWindow *GetWindow(uint i) {
		if (m_Windows.GetSize() > i)
			return m_Windows[i];
		else
			return NULL;
	}

	void SetHUD(vtHUD *hud) { m_pHUD = hud; }
	vtHUD *GetHUD() { return m_pHUD; }

	// For backward compatibility
	void SetBgColor(const RGBf &color) {
		if (GetWindow(0))
			GetWindow(0)->SetBgColor(color);
	}

	bool IsStereo() const;;
	void SetStereoSeparation(float fSep);
	float GetStereoSeparation() const;
	void SetStereoFusionDistance(float fDist);
	float GetStereoFusionDistance();

	void ComputeViewMatrix(FMatrix4 &mat);

	// OSG access
	osgViewer::Viewer *getViewer() { return m_pOsgViewer.get(); }

#if VTP_VISUAL_IMPACT_CALCULATOR
    // Visual Impact Calculation
	virtual CVisualImpactCalculatorOSG& GetVisualImpactCalculator() { return m_VisualImpactCalculator; }
#endif

protected:
	void DoEngines(vtEngine *eng);

	vtArray<vtWindow*> m_Windows;
	vtCamera	*m_pCamera;
	vtGroup		*m_pRoot;
	vtEnginePtr	m_pRootEngine;
	vtEngine	*m_pRootEnginePostDraw;
	bool		*m_piKeyState;
	vtHUD		*m_pHUD;

	vtCameraPtr	 m_pDefaultCamera;
	vtWindow	*m_pDefaultWindow;

	osg::ref_ptr<osgViewer::Viewer>	m_pOsgViewer;
	osg::ref_ptr<osg::GraphicsContext>	m_pGraphicsContext;

	osg::ref_ptr<osg::Group>	m_StateRoot;

	osg::Timer   _timer;
	osg::Timer_t _initialTick;
	osg::Timer_t _lastFrameTick;
	osg::Timer_t _lastRunningTick;
	osg::Timer_t _frameTick;
	double	m_fAccumulatedFrameTime, m_fLastFrameTime;

	bool	m_bWinInfo;
	bool	m_bInitialized;
	bool	m_bWireframe;

#if VTP_VISUAL_IMPACT_CALCULATOR
	CVisualImpactCalculatorOSG m_VisualImpactCalculator;
#endif
};

// global
vtScene *vtGetScene();
float vtGetTime();
float vtGetFrameTime();
int vtGetMaxTextureSize();

/*@}*/	// Group sg

#endif	// VTOSG_SCENEH

