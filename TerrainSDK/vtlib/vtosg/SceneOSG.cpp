//
// SceneOSG.cpp
//
// Implementation of vtScene for the OSG library
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

#include <osgViewer/ViewerEventHandlers>

#include <osg/PolygonMode>	// SetGlobalWireframe
#include <osgDB/Registry>	// for clearObjectCache

#ifdef __FreeBSD__
#  include <sys/types.h>
#  include <sys/time.h>
#  include <sys/resource.h>
#endif

#include <iostream>			// For redirecting OSG's stdout messages
#include "vtdata/vtLog.h"	// to the VTP log.

/** A way to catch OSG messages */
class OsgMsgTrap : public std::streambuf
{
public:
	inline virtual int_type overflow(int_type c = std::streambuf::traits_type::eof())
	{
		if (c == std::streambuf::traits_type::eof()) return std::streambuf::traits_type::not_eof(c);
		VTLOG1((char) c);
		return c;
	}
} g_Trap;

// preserve and restore
static std::streambuf *previous_cout;
static std::streambuf *previous_cerr;

///////////////////////////////////////////////////////////////

/// There one and only global vtScene object
vtScene g_Scene;


///////////////////////////////////////////////////////////////////////

vtScene::vtScene()
{
	m_pCamera = NULL;
	m_pRoot = NULL;
	m_pRootEngine = NULL;
	m_pRootEnginePostDraw = NULL;
	m_piKeyState = NULL;
	m_pDefaultCamera = NULL;
	m_pDefaultWindow = NULL;

	m_bInitialized = false;
	m_bWireframe = false;
	m_bWinInfo = false;
	m_pHUD = NULL;
}

vtScene::~vtScene()
{
	// Cleanup engines.  They are in a tree, connected by ref_ptr, so we only need
	//  to release the top of the tree.
	m_pRootEngine = NULL;

	m_pOsgViewer = NULL;	// derefs

	// Do not release camera or window, that is left for the application.
}

vtScene *vtGetScene()
{
	return &g_Scene;
}

float vtGetTime()
{
	return g_Scene.GetTime();
}

float vtGetFrameTime()
{
	return g_Scene.GetFrameTime();
}

#include <osg/GLExtensions>

int vtGetMaxTextureSize()
{
	osg::GraphicsContext *context = vtGetScene()->GetGraphicsContext();
	if (!context) {
		VTLOG1("Error: Called vtGetMaxTextureSize without a known context\n");
		return 0;
	}

	osg::State *state = context->getState();
	if (!state)
		return 0;

#if OSG_VERSION_GREATER_THAN(3, 3, 0)
	// get extension object
	const osg::GLExtensions* extensions = state->get<osg::GLExtensions>();
	if (!extensions)
		return 0;

	return extensions->maxTextureSize;
#else
    // Do not try to create an Extensions object if one does not already exist
    // as we cannot guarantee a valid rendering context at this point.
	osg::ref_ptr<osg::Texture::Extensions> pTextureExtensions =
		osg::Texture::getExtensions(context->getState()->getContextID(), false);
    if (pTextureExtensions.valid())
        return pTextureExtensions->maxTextureSize();
    else
        return 0;
#endif
}

#if 0
class MyCull : public osg::NodeCallback
{
public:
    /** Callback method called by the NodeVisitor when visiting a node.*/
	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
		const GLubyte *test = glGetString(GL_VERSION);
        traverse(node,nv);
    }
};

class MyUpdate : public osg::NodeCallback
{
public:
    /** Callback method called by the NodeVisitor when visiting a node.*/
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
		const GLubyte *test = glGetString(GL_VERSION);
        traverse(node,nv);
    }
};

class MyEvent : public osg::NodeCallback
{
public:
    /** Callback method called by the NodeVisitor when visiting a node.*/
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
		const GLubyte *test = glGetString(GL_VERSION);
        traverse(node,nv);
    }
};
#endif

/**
 * Initialize the vtlib library, including the display and scene graph.
 * You should call this function only once, before any other vtlib calls.
 *
 * \param argc, argv Command-line arguments.
 * \param bStereo True for a stereo display output.
 * \param iStereoMode Currently for vtosg, supported values are 0 for
 *		Anaglyphic (red-blue) and 1 for Quad-buffer (shutter glasses).
 */
bool vtScene::Init(int argc, char** argv, bool bStereo, int iStereoMode)
{
	VTLOG1("vtScene::Init\n");

	// Redirect cout messages (where OSG sends its messages) to our own log
	previous_cout =  std::cout.rdbuf(&g_Trap);
	previous_cerr = std::cerr.rdbuf(&g_Trap);

#if 0
	// If you encounter trouble in OSG that you want to debug, enable this
	//  to get a LOT of diagnostic messages from OSG.
	osg::setNotifyLevel(osg::INFO);
#endif

	m_pDefaultCamera = new vtCamera;
	m_pDefaultWindow = new vtWindow;
	SetCamera(m_pDefaultCamera);
	AddWindow(m_pDefaultWindow);

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

	m_pOsgViewer = new osgViewer::Viewer(arguments);

	m_pOsgViewer->setDisplaySettings(osg::DisplaySettings::instance());
	if (bStereo)
	{
		osg::DisplaySettings* displaySettings = m_pOsgViewer->getDisplaySettings();
		displaySettings->setStereo(true);
		osg::DisplaySettings::StereoMode mode;
		if (iStereoMode == 0) mode = osg::DisplaySettings::ANAGLYPHIC;
		if (iStereoMode == 1) mode = osg::DisplaySettings::QUAD_BUFFER;
		if (iStereoMode == 2) mode = osg::DisplaySettings::HORIZONTAL_SPLIT;
		if (iStereoMode == 3) mode = osg::DisplaySettings::VERTICAL_SPLIT;
		displaySettings->setStereoMode(mode);
	}
#ifdef __DARWIN_OSX__
	// Kill multi-threading on OSX until wxGLContext properly implemented on that platform
	m_pOsgViewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
#endif

	// We can't use displaySettings->setNumMultiSamples here to enable anti-
	// aliasing, because it has to be done eariler (at the time the OpenGL
	// context is made).

#ifdef VTP_USE_OSG_STATS
	osgViewer::StatsHandler* pStatsHandler = new osgViewer::StatsHandler;
	pStatsHandler->setKeyEventPrintsOutStats(0);
	pStatsHandler->setKeyEventTogglesOnScreenStats('x'); // I dont think this is used for anything else at the moment
	m_pOsgViewer->addEventHandler(pStatsHandler);
#endif

	// Kill the event visitor (saves a scenegraph traversal)
	// This will need to be restored if we need to use FRAME events etc. in the scenegraph
	m_pOsgViewer->setEventVisitor(NULL);

	if (bStereo)
	{
		// displaySettings->getScreenDistance(); default is 0.5
		// m_pOsgSceneView->getFusionDistanceMode(); default is PROPORTIONAL_TO_SCREEN_DISTANCE
		// m_pOsgSceneView->getFusionDistanceValue(); default is 1.0
		// The FusionDistanceValue is only used for USE_FUSION_DISTANCE_VALUE & PROPORTIONAL_TO_SCREEN_DISTANCE modes.

		// We use real-world units for fusion distance value
		m_pOsgViewer->setFusionDistance(osgUtil::SceneView::USE_FUSION_DISTANCE_VALUE, 100.0f);
	}

	// From the OSG mailing list: You must specify the lighting mode in
	// setDefaults() and override the default options. If you call
	// setDefaults() with the default options, a headlight is added to the
	// global state set of the SceneView.  With the default options applied,
	// I have tried subsequently calling setLightingMode(NO_SCENE_LIGHT)
	// and setLight(NULL), but I still get a headlight.
	m_pOsgViewer->setLightingMode(osg::View::NO_LIGHT);
	m_pOsgViewer->getCamera()->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);
	m_pOsgViewer->getCamera()->setCullingMode(m_pOsgViewer->getCamera()->getCullingMode() & ~osg::CullSettings::SMALL_FEATURE_CULLING);

	// We maintain a node between OSG's viewer/camera and the vtlib Root, to
	//  control global state
	m_StateRoot = new osg::Group;
	m_pOsgViewer->setSceneData(m_StateRoot);

	// By default, things are lit, unless they ask not to be
	m_StateRoot->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);

	//m_StateRoot->addCullCallback(new MyCull);
	//m_StateRoot->setUpdateCallback(new MyUpdate);
	//m_StateRoot->setEventCallback(new MyEvent);

	m_bInitialized = true;

	_initialTick = _timer.tick();
	_frameTick = _initialTick;

	return true;
}

void vtScene::Shutdown()
{
	VTLOG("vtScene::Shutdown\n");
	m_pDefaultCamera = NULL;
	m_pCamera = NULL;

	delete m_pDefaultWindow;
	m_pDefaultWindow = NULL;
	m_Windows.Clear();

	// Also clear the OSG cache
	osgDB::Registry::instance()->clearObjectCache();

	m_pRoot = NULL;
	m_pRootEnginePostDraw = NULL;
	m_piKeyState = NULL;

	// remove our hold on refcounted objects
	m_pOsgViewer = NULL;
	m_pGraphicsContext = NULL;

	// restore
	std::cout.rdbuf(previous_cout);
	std::cerr.rdbuf(previous_cerr);
}

void vtScene::OnMouse(vtMouseEvent &event, vtWindow *pWindow)
{
	// Pass event to Engines
	vtEngineArray list(m_pRootEngine);
	for (uint i = 0; i < list.size(); i++)
	{
		vtEngine *pEng = list[i];
		if (pEng->GetEnabled() &&
			(pEng->GetWindow() == NULL || pEng->GetWindow() == pWindow))
			pEng->OnMouse(event);
	}
}

void vtScene::OnKey(int key, int flags, vtWindow *pWindow)
{
	// Pass event to Engines
	vtEngineArray list(m_pRootEngine);
	for (uint i = 0; i < list.size(); i++)
	{
		vtEngine *pEng = list[i];
		if (pEng->GetEnabled() &&
			(pEng->GetWindow() == NULL || pEng->GetWindow() == pWindow))
			pEng->OnKey(key, flags);
	}
}

bool vtScene::GetKeyState(int key)
{
	if (m_piKeyState)
		return m_piKeyState[key];
	else
		return false;
}

IPoint2 vtScene::GetWindowSize(vtWindow *pWindow)
{
	if (!pWindow)
		pWindow = GetWindow(0);
	return pWindow->GetSize();
}

void vtScene::DoEngines(vtEngine *eng)
{
	// Evaluate Engines
	vtEngineArray list(eng);
	for (uint i = 0; i < list.size(); i++)
	{
		vtEngine *pEng = list[i];
		if (pEng->GetEnabled())
			pEng->Eval();
	}
}

// (for backward compatibility only)
void vtScene::AddEngine(vtEngine *ptr)
{
	if (m_pRootEngine)
		m_pRootEngine->AddChild(ptr);
	else
		m_pRootEngine = ptr;
}

void vtScene::TargetRemoved(osg::Referenced *tar)
{
	// Look at all Engines
	vtEngineArray list(m_pRootEngine);
	for (uint i = 0; i < list.size(); i++)
	{
		// If this engine targets something that is no longer there
		vtEngine *pEng = list[i];
		for (uint j = 0; j < pEng->NumTargets(); j++)
		{
			// Then remove it
			if (pEng->GetTarget(j) == tar)
				pEng->RemoveTarget(tar);
		}
	}
}

void vtScene::TimerRunning(bool bRun)
{
	if (!bRun)
	{
		// stop timer, count how much running time has already elapsed
		m_fAccumulatedFrameTime = _timer.delta_s(_lastRunningTick,_timer.tick());
		//VTLOG("partial frame: %lf seconds\n", m_fAccumulatedFrameTime);
	}
	else
		// start again
		_lastRunningTick = _timer.tick();
}

void vtScene::UpdateBegin()
{
	_lastFrameTick = _frameTick;
	_frameTick = _timer.tick();

	// finish counting the split frame's elapsed time
	if (_lastRunningTick != _lastFrameTick)
	{
		m_fAccumulatedFrameTime += _timer.delta_s(_lastRunningTick,_frameTick);
		//VTLOG("   full frame: %lf seconds\n", m_fAccumulatedFrameTime);
		m_fLastFrameTime = m_fAccumulatedFrameTime;
	}
	else
		m_fLastFrameTime = _timer.delta_s(_lastFrameTick,_frameTick);

	_lastRunningTick = _frameTick;
}

void vtScene::UpdateEngines()
{
	if (!m_bInitialized) return;
	DoEngines(m_pRootEngine);
}

void vtScene::PostDrawEngines()
{
	if (!m_bInitialized) return;
	DoEngines(m_pRootEnginePostDraw);
}

void vtScene::UpdateWindow(vtWindow *pWindow)
{
	if (!m_bInitialized) return;

	// window background color
	osg::Vec4 color2;
	v2s(pWindow->GetBgColor(), color2);
	m_pOsgViewer->getCamera()->setClearColor(color2);

	// window size
	IPoint2 winsize = pWindow->GetSize();
	if (winsize.x == 0 || winsize.y == 0)
	{
		VTLOG("Warning: winsize %d %d\n", winsize.x, winsize.y);
		return;
	}
	m_pOsgViewer->getCamera()->setViewport(0, 0, winsize.x, winsize.y);

	// As of OSG 0.9.5, we need to store our own camera params and recreate
	//  the projection matrix each frame.
	float aspect;
	if (winsize.x == 0 || winsize.y == 0)		// safety
		aspect = 1.0;
	else
		aspect = (float) winsize.x / winsize.y;

	if (m_pCamera->IsOrtho())
	{
		// Arguments are left, right, bottom, top, zNear, zFar
		float w2 = m_pCamera->GetWidth() /2;
		float h2 = w2 / aspect;
		m_pOsgViewer->getCamera()->setProjectionMatrixAsOrtho(-w2, w2, -h2, h2,
			m_pCamera->GetHither(), m_pCamera->GetYon());
	}
	else
	{
		float fov_x = m_pCamera->GetFOV();
		float a = tan (fov_x/2);
		float b = a / aspect;
		float fov_y_div2 = atan(b);
		float fov_y_deg = osg::RadiansToDegrees(fov_y_div2 * 2);

		m_pOsgViewer->getCamera()->setProjectionMatrixAsPerspective(fov_y_deg,
			aspect, m_pCamera->GetHither(), m_pCamera->GetYon());
	}

	// And apply the rotation and translation of the camera itself
	const osg::Matrix &mat2 = m_pCamera->getMatrix();
	osg::Matrix imat;
	imat.invert(mat2);
	m_pOsgViewer->getCamera()->setViewMatrix(imat);

	m_pOsgViewer->getCamera()->setCullMask(0x3);
	// Also set the mask for the case of split-screen stereo
	m_pOsgViewer->getCamera()->setCullMaskLeft(0x3);
	m_pOsgViewer->getCamera()->setCullMaskRight(0x3);

	m_pOsgViewer->frame();
}

/**
 * Compute the full current view transform as a matrix, which includes
 * the projection of the camera and the transform to window coordinates.
 *
 * This transform is the one used to convert XYZ points in world coodinates
 * into XY window coordinates.
 *
 * By inverting this matrix, you can "un-project" window coordinates back
 * into the world.
 *
 * \param mat This matrix will receive the current view transform.
 */
void vtScene::ComputeViewMatrix(FMatrix4 &mat)
{
	osg::Matrix _viewMatrix = m_pOsgViewer->getCamera()->getViewMatrix();
	osg::Matrix _projectionMatrix = m_pOsgViewer->getCamera()->getProjectionMatrix();
	osg::Viewport *_viewport = m_pOsgViewer->getCamera()->getViewport();

	osg::Matrix matrix( _viewMatrix * _projectionMatrix);

	if (_viewport != NULL)
		matrix.postMult(_viewport->computeWindowMatrix());

	ConvertMatrix4(&matrix, &mat);
}

void vtScene::DoUpdate()
{
	UpdateBegin();
	UpdateEngines();
	UpdateWindow(GetWindow(0));

	// Some engines need to run after the cull-draw phase
	PostDrawEngines();
}

void vtScene::SetRoot(vtGroup *pRoot)
{
	// Remove previous root, add this one
	m_StateRoot->removeChildren(0, m_StateRoot->getNumChildren());
	if (pRoot)
		m_StateRoot->addChild(pRoot);

	// Remember it
	m_pRoot = pRoot;
}

bool vtScene::IsStereo() const
{
	const osg::DisplaySettings* displaySettings = m_pOsgViewer->getDisplaySettings();
	return displaySettings->getStereo();
}

void vtScene::SetStereoSeparation(float fSep)
{
	osg::DisplaySettings* displaySettings = m_pOsgViewer->getDisplaySettings();
	displaySettings->setEyeSeparation(fSep);
}

float vtScene::GetStereoSeparation() const
{
	const osg::DisplaySettings* displaySettings = m_pOsgViewer->getDisplaySettings();
	return displaySettings->getEyeSeparation();
}

void vtScene::SetStereoFusionDistance(float fDist)
{
	m_pOsgViewer->setFusionDistance(osgUtil::SceneView::USE_FUSION_DISTANCE_VALUE, fDist);
}

float vtScene::GetStereoFusionDistance()
{
	return m_pOsgViewer->getFusionDistanceValue();
}

/**
 * Convert window coordinates (in pixels) to a ray from the camera
 * in world coordinates.  Pixel coordinates are measured from the
 * top left corner of the window: X right, Y down.
 */
bool vtScene::CameraRay(const IPoint2 &win, FPoint3 &pos, FPoint3 &dir, vtWindow *pWindow)
{
	if (!pWindow)
		pWindow = GetWindow(0);

	osg::Vec3 near_point, far_point, diff;

	// There used to be a handy function for this
	osg::Matrix MVPW;
	osg::Matrix InverseMVPW;
	osg::Camera *pCamera = m_pOsgViewer->getCamera();
	MVPW = pCamera->getViewMatrix() * pCamera->getProjectionMatrix();
	if (pCamera->getViewport())
		MVPW.postMult(pCamera->getViewport()->computeWindowMatrix());
	InverseMVPW.invert(MVPW);

	IPoint2 winsize = pWindow->GetSize();

	near_point = osg::Vec3d(win.x, winsize.y - 1 - win.y, 0.0) * InverseMVPW;
	far_point = osg::Vec3d(win.x, winsize.y - 1 - win.y, 1.0) * InverseMVPW;

	diff = far_point - near_point;
	diff.normalize();

	s2v(near_point, pos);
	s2v(diff, dir);

	return true;
}

/**
 * Convert a point in world coordinates to a window pixel coordinate, measured
 * from the top left corner of the window: X right, Y down.
 */
void vtScene::WorldToScreen(const FPoint3 &point, IPoint2 &result)
{
	osg::Vec3 object;
	v2s(point, object);
	osg::Vec3 window;

	osg::Matrix MVPW;
	osg::Camera *pCamera = m_pOsgViewer->getCamera();
	MVPW = pCamera->getViewMatrix() * pCamera->getProjectionMatrix();
	if (pCamera->getViewport())
		MVPW.postMult(pCamera->getViewport()->computeWindowMatrix());

	window = object * MVPW;

	result.x = (int) window.x();
	result.y = (int) window.y();
}

void vtScene::SetGlobalWireframe(bool bWire)
{
	m_bWireframe = bWire;

	// Set the scene's global PolygonMode attribute, which will affect all
	// other materials in the scene, except those which explicitly override
	// the attribute themselves.
	osg::PolygonMode *npm = new osg::PolygonMode;
	if (m_bWireframe)
		npm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
	else
		npm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);
	GetRootState()->setAttributeAndModes(npm, osg::StateAttribute::ON);
}

bool vtScene::GetGlobalWireframe()
{
	return m_bWireframe;
}

void vtScene::SetWindowSize(int w, int h, vtWindow *pWindow)
{
	if (!m_bInitialized) return;
	if (m_pHUD)
		m_pHUD->SetWindowSize(w, h);

	if (!pWindow)
		pWindow = GetWindow(0);
	pWindow->SetSize(w, h);

	// Pass event to Engines
	vtEngineArray list(m_pRootEngine);
	for (uint i = 0; i < list.size(); i++)
	{
		vtEngine *pEng = list[i];
		if (pEng->GetEnabled() &&
			(pEng->GetWindow() == NULL || pEng->GetWindow() == pWindow))
			pEng->OnWindowSize(w, h);
	}

	osgViewer::GraphicsWindow* pGW = GetGraphicsWindow();
	if ((NULL != pGW) && pGW->valid())
	{
		pGW->getEventQueue()->windowResize(0, 0, w, h);
		pGW->resized(0, 0, w, h);
	}
}

bool vtScene::GetWindowSizeFromOSG()
{
	bool bNeedSize = true;

	// Get initial size from OSG?
	osg::Camera *cam = m_pOsgViewer->getCamera();
	osg::Viewport *vport = cam->getViewport();
	if (vport)
	{
		vtGetScene()->SetWindowSize(vport->width(), vport->height());
		bNeedSize = false;
	}

	if (bNeedSize)
	{
		// or another way
		osgViewer::ViewerBase::Contexts contexts;
		m_pOsgViewer->getContexts(contexts);
		if (contexts.size() > 0)
		{
			const osg::GraphicsContext::Traits *traits = contexts[0]->getTraits();
			vtGetScene()->SetWindowSize(traits->width, traits->height);
			bNeedSize = false;
		}
	}

	//// or another way
	//osgViewer::ViewerBase::Windows windows;
	//m_pOsgViewer->getWindows(windows);

	//// or another
	//osgViewer::ViewerBase::Views views;
	//m_pOsgViewer->getViews(views);

	return !bNeedSize;
}

////////////////////////////////////////
// Shadow methods

void vtScene::SetGraphicsContext(osg::GraphicsContext* pGraphicsContext)
{
	m_pGraphicsContext = pGraphicsContext;
	m_pOsgViewer->getCamera()->setGraphicsContext(pGraphicsContext);
}

osg::GraphicsContext *vtScene::GetGraphicsContext()
{
	return m_pGraphicsContext.get();
}

osgViewer::GraphicsWindow *vtScene::GetGraphicsWindow()
{
	return dynamic_cast<osgViewer::GraphicsWindow*>(m_pGraphicsContext.get());
}

////////////////////////////////////////

// Helper fn for dumping an OSG scenegraph
void printnode(osg::Node *node, int tab)
{
	for (int i = 0; i < tab*2; i++) {
	   osg::notify(osg::WARN) << " ";
	}
	osg::notify(osg::WARN) << node->className() << " - " << node->getName() << " @ " << node << std::endl;
	osg::Group *group = node->asGroup();
	if (group) {
		for (uint i = 0; i < group->getNumChildren(); i++) {
			printnode(group->getChild(i), tab+1);
		}
	}
}

//////////////////////////////////////////////
// Window

vtWindow::vtWindow()
{
	m_BgColor.Set(0.2f, 0.2f, 0.4f);
	m_Size.Set(0, 0);
}

void vtWindow::SetSize(int w, int h)
{
    VTLOG("vtWindow::SetSize(%d, %d)\n", w, h);
	m_Size.Set(w,h);
}

