// mfcSimpleView.cpp : implementation of the CSimpleView class
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "stdafx.h"
#include "mfcSimple.h"

#include "mfcSimpleDoc.h"
#include "mfcSimpleView.h"

bool CreateScene();
void CleanupScene();

// Header for the vtlib library
#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/TerrainScene.h"
#include "vtlib/core/NavEngines.h"
#include "vtdata/DataPath.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSimpleView

IMPLEMENT_DYNCREATE(CSimpleView, CView)

BEGIN_MESSAGE_MAP(CSimpleView, CView)
	//{{AFX_MSG_MAP(CSimpleView)
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSimpleView construction/destruction

CSimpleView::CSimpleView()
{
	m_hGLContext = NULL;
	m_GLPixelIndex = 0;
}

CSimpleView::~CSimpleView()
{
}

BOOL CSimpleView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CSimpleView diagnostics

#ifdef _DEBUG
void CSimpleView::AssertValid() const
{
	CView::AssertValid();
}

void CSimpleView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSimpleDoc* CSimpleView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSimpleDoc)));
	return (CSimpleDoc*)m_pDocument;
}
#endif //_DEBUG

void CSimpleView::OnDraw(CDC* pDC)
{
	CSimpleDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
}

/////////////////////////////////////////////////////////////////////////////
// CSimpleView message handlers
// set Windows Pixel Format
BOOL CSimpleView::SetWindowPixelFormat(HDC hDC)
{
	PIXELFORMATDESCRIPTOR pixelDesc;

	pixelDesc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pixelDesc.nVersion = 1;

	pixelDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER | PFD_STEREO_DONTCARE;

	pixelDesc.iPixelType = PFD_TYPE_RGBA;
	pixelDesc.cColorBits = 32;
	pixelDesc.cRedBits = 8;
	pixelDesc.cRedShift = 16;
	pixelDesc.cGreenBits = 8;
	pixelDesc.cGreenShift = 8;
	pixelDesc.cBlueBits = 8;
	pixelDesc.cBlueShift = 0;
	pixelDesc.cAlphaBits = 0;
	pixelDesc.cAlphaShift = 0;
	pixelDesc.cAccumBits = 64;
	pixelDesc.cAccumRedBits = 16;
	pixelDesc.cAccumGreenBits = 16;
	pixelDesc.cAccumBlueBits = 16;
	pixelDesc.cAccumAlphaBits = 0;
	pixelDesc.cDepthBits = 32;
	pixelDesc.cStencilBits = 8;
	pixelDesc.cAuxBuffers = 0;
	pixelDesc.iLayerType = PFD_MAIN_PLANE;
	pixelDesc.bReserved = 0;
	pixelDesc.dwLayerMask = 0;
	pixelDesc.dwVisibleMask = 0;
	pixelDesc.dwDamageMask = 0;

	m_GLPixelIndex = ChoosePixelFormat(hDC,&pixelDesc);
	if (m_GLPixelIndex == 0) // Choose default
	{
		m_GLPixelIndex = 1;
		if (DescribePixelFormat(hDC,m_GLPixelIndex,
			sizeof(PIXELFORMATDESCRIPTOR),&pixelDesc)==0)
			return FALSE;
	}

	if (!SetPixelFormat(hDC,m_GLPixelIndex,&pixelDesc))
		return FALSE;

	return TRUE;
}

//********************************************
// CreateViewGLContext
// Create an OpenGL rendering context
//********************************************
BOOL CSimpleView::CreateViewGLContext(HDC hDC)
{
	m_hGLContext = wglCreateContext(hDC);

	if (m_hGLContext==NULL)
		return FALSE;

	if (wglMakeCurrent(hDC,m_hGLContext)==FALSE)
		return FALSE;

	return TRUE;
}


void CSimpleView::OnDestroy()
{
	CleanupScene();

	if (wglGetCurrentContext() != NULL)
		wglMakeCurrent(NULL,NULL);
	if (m_hGLContext != NULL)
	{
		wglDeleteContext(m_hGLContext);
		m_hGLContext = NULL;
	}
	CView::OnDestroy();
}

int CSimpleView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	HWND hWnd = GetSafeHwnd();
	HDC hDC = ::GetDC(hWnd);

	if (SetWindowPixelFormat(hDC)==FALSE)
		return 0;

	if (CreateViewGLContext(hDC)==FALSE)
		return 0;

	int dummy_argc = 1;
	char *dummy_argv = "mfcSimple.exe";

	// Get a handle to the vtScene - one is already created for you
	vtScene *pScene = vtGetScene();
	pScene->Init(dummy_argc, &dummy_argv);

	pScene->getViewer()->setThreadingModel(osgViewer::Viewer::SingleThreaded);
	pScene->SetGraphicsContext(new osgViewer::GraphicsWindowEmbedded(0, 0, 800, 600));

	return CreateScene();
}

void CSimpleView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	glViewport(0,0,cx, cy);
	vtGetScene()->SetWindowSize(cx, cy);
}

void CSimpleView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	// Render the scene
	vtGetScene()->DoUpdate();
	//this would occur, for example, after your call to glClear() and before calling SwapBuffers()
	SwapBuffers(dc.m_ps.hdc);
	InvalidateRect(NULL,FALSE);	//for Continuous Rendering
}

//-----------------------The Following is VTerrain Code-----------------

vtTerrainScene *ts = NULL;

//
// Create the 3d scene
//
bool CreateScene()
{
	// Look up the camera
	vtScene *pScene = vtGetScene();
	vtCamera *pCamera = pScene->GetCamera();
	pCamera->SetHither(10);
	pCamera->SetYon(150000);

	// The  terrain scene will contain all the terrains that are created.
	ts = new vtTerrainScene;

	// Get the global data path
	char user_config_dir[MAX_PATH];
	SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, user_config_dir);
	vtLoadDataPath(user_config_dir, NULL);

	// And look locally too, just in case the global path isn't set yet
	vtGetDataPath().push_back("../Data/");

	// Begin creating the scene, including the sun and sky
	vtGroup *pTopGroup = ts->BeginTerrainScene();

	// Tell the scene graph to point to this terrain scene
	pScene->SetRoot(pTopGroup);

	// Create a new vtTerrain, read its paramters from a file
	vtTerrain *pTerr = new vtTerrain;
	pTerr->SetParamFile("Data/Simple.xml");
	pTerr->LoadParams();

	// Add the terrain to the scene, and contruct it
	ts->AppendTerrain(pTerr);
	if (!ts->BuildTerrain(pTerr))
	{
		AfxMessageBox("Terrain creation failed.");
		return false;
	}

	ts->SetCurrentTerrain(pTerr);

	// Create a navigation engine to move around on the terrain
	// Get flight speed from terrain parameters
	float fSpeed = pTerr->GetParams().GetValueFloat(STR_NAVSPEED);

	vtTerrainFlyer *pFlyer = new vtTerrainFlyer(fSpeed);
	pFlyer->SetTarget(pCamera);
	pFlyer->SetHeightField(pTerr->GetHeightField());
	pScene->AddEngine(pFlyer);

	// Minimum height over terrain is 100 m
	vtHeightConstrain *pConstrain = new vtHeightConstrain(100);
	pConstrain->SetTarget(pCamera);
	pConstrain->SetHeightField(pTerr->GetHeightField());
	pScene->AddEngine(pConstrain);

	return true;
}

void CleanupScene()
{
	vtGetScene()->SetRoot(NULL);
	if (ts)
	{
		ts->CleanupScene();
		delete ts;
	}
	vtGetScene()->Shutdown();
}

//--------------Mouse EVENTS-----------------

//
// turn MFC events flags in VT flags
//
int MFCFlagsToVT(int nFlags)
{
	int flags = 0;
	if ( (nFlags & MK_CONTROL)!=0 )
		flags |= VT_CONTROL;
	if ( (nFlags & MK_SHIFT)!=0 )
		flags |= VT_SHIFT;
	return flags;
}

//
// turn MFC mouse events into VT mouse events
//
void CSimpleView::OnMouseMove(UINT nFlags, CPoint point)
{
	vtMouseEvent event;

	event.type = VT_MOVE;
	event.button = VT_NONE;
	event.flags = MFCFlagsToVT(nFlags);
	event.pos.Set(point.x,point.y);

	vtGetScene()->OnMouse(event);
}

void CSimpleView::OnLButtonUp(UINT nFlags, CPoint point)
{
	vtMouseEvent event;

	event.type = VT_UP;
	event.button = VT_LEFT;
	event.flags = MFCFlagsToVT(nFlags);
	event.pos.Set(point.x,point.y);

	vtGetScene()->OnMouse(event);
}

void CSimpleView::OnLButtonDown(UINT nFlags, CPoint point)
{
	vtMouseEvent event;

	event.type = VT_DOWN;
	event.button = VT_LEFT;
	event.flags = MFCFlagsToVT(nFlags);
	event.pos.Set(point.x,point.y);

	vtGetScene()->OnMouse(event);
}

void CSimpleView::OnRButtonDown(UINT nFlags, CPoint point)
{
	vtMouseEvent event;

	event.type = VT_DOWN;
	event.button = VT_RIGHT;
	event.flags = MFCFlagsToVT(nFlags);
	event.pos.Set(point.x,point.y);

	vtGetScene()->OnMouse(event);
}

void CSimpleView::OnRButtonUp(UINT nFlags, CPoint point)
{
	vtMouseEvent event;

	event.type = VT_UP;
	event.button = VT_RIGHT;
	event.flags = MFCFlagsToVT(nFlags);
	event.pos.Set(point.x,point.y);

	vtGetScene()->OnMouse(event);
}

void CSimpleView::OnMButtonDown(UINT nFlags, CPoint point)
{
	vtMouseEvent event;

	event.type = VT_DOWN;
	event.button = VT_MIDDLE;
	event.flags = MFCFlagsToVT(nFlags);
	event.pos.Set(point.x,point.y);

	vtGetScene()->OnMouse(event);
}

void CSimpleView::OnMButtonUp(UINT nFlags, CPoint point)
{
	vtMouseEvent event;

	event.type = VT_UP;
	event.button = VT_MIDDLE;
	event.flags = MFCFlagsToVT(nFlags);
	event.pos.Set(point.x,point.y);

	vtGetScene()->OnMouse(event);
}

