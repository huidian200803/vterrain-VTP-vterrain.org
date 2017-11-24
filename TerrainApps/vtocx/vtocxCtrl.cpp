//
// vtocxCtrl.cpp : Implementation of the CvtocxCtrl ActiveX Control class.
//

#include "stdafx.h"
#include <ObjSafe.h>
#include "vtocx.h"

#define FULL_LOGGING		0
#define CALL_VTP			1
#define	RENDER_ON_PAINT		1
#define TIMER_MILISECONDS	30	// 1 sec / 30 ms = ~30 fps

//
// There are three methods to make continuous rendering happen:
//
// 1. Create a thread (using OpenThreads), which calls frame() repeatedly.
//  Problem: in some cases an OnDraw event can arrive in the middle of the
//  thread calling frame(), which causes a collision with the DC and GL context.
//  We can't use a mutex to avoid it, because the DC is already created by
//  Windows/MFC before OnDraw is called.
//
// 2. Continuous Paint, draw in OnPaint and invalidate to get another OnPaint.
//  Problem: it sends so many Paint events that the rest of the UI doesn't
//  get a chance to refresh itself.
//
// 3. Use a timer.
//
#define USE_GLTHREAD		 0
#define USE_CONTINUOUS_PAINT 0
#define USE_TIMER			 1

#if CALL_VTP
// Headers for the vtlib library
#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/TerrainScene.h"
#include "vtlib/core/NavEngines.h"
#endif

#include "vtdata/DataPath.h"
#include "vtdata/vtLog.h"

#include "vtocxCtrl.h"
#include "vtocxPropScreen.h"
#include "vtocxPropNavigation.h"

#include <osg/Viewport>
#include <osg/Texture2D>
#include <osg/TextureCubeMap>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include ".\vtocxctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CvtocxCtrl, COleControl)

// Message map
BEGIN_MESSAGE_MAP(CvtocxCtrl, COleControl)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_CONTEXTMENU()
	ON_WM_SHOWWINDOW()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
END_MESSAGE_MAP()


// Dispatch map
BEGIN_DISPATCH_MAP(CvtocxCtrl, COleControl)
	DISP_FUNCTION_ID(CvtocxCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
	DISP_PROPERTY_NOTIFY_ID(CvtocxCtrl, "ClearColor", dispidClearColor, m_ClearColor, OnClearColorChanged, VT_COLOR)
	DISP_PROPERTY_NOTIFY_ID(CvtocxCtrl, "Stereo", dispidStereo, m_Stereo, OnStereoChanged, VT_BOOL)
	DISP_PROPERTY_EX_ID(CvtocxCtrl, "TerrainFile", dispidTerrainFile, GetTerrainFile, SetTerrainFile, VT_BSTR)
	DISP_PROPERTY_EX_ID(CvtocxCtrl, "TerrainPaths", dispidTerrainPaths, GetTerrainPaths, SetTerrainPaths, VT_BSTR)
	DISP_PROPERTY_EX_ID(CvtocxCtrl, "Src", dispidSrc, GetSrc, SetSrc, VT_BSTR)
	DISP_PROPERTY_EX_ID(CvtocxCtrl, "LocalSrc", dispidLocalSrc, GetLocalSrc, SetLocalSrc, VT_BSTR)
	DISP_PROPERTY_NOTIFY_ID(CvtocxCtrl, "Fullscreen", dispidFullscreen, m_Fullscreen, OnFullscreenChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY_ID(CvtocxCtrl, "PositionX", dispidPositionX, m_PositionX, OnPositionChanged, VT_R8)
	DISP_PROPERTY_NOTIFY_ID(CvtocxCtrl, "PositionY", dispidPositionY, m_PositionY, OnPositionChanged, VT_R8)
	DISP_PROPERTY_NOTIFY_ID(CvtocxCtrl, "PositionZ", dispidPositionZ, m_PositionZ, OnPositionChanged, VT_R8)
	DISP_PROPERTY_NOTIFY_ID(CvtocxCtrl, "NavSpeed", dispidNavSpeed, m_NavSpeed, OnNavSpeedChanged, VT_R4)
	DISP_FUNCTION_ID(CvtocxCtrl, "JumpToNo", dispidJumpToNo, JumpToNo, VT_EMPTY, VTS_I4)
	DISP_FUNCTION_ID(CvtocxCtrl, "ResetView", dispidResetView, ResetView, VT_EMPTY, VTS_NONE)
	DISP_PROPERTY_NOTIFY_ID(CvtocxCtrl, "ContextMenu", dispidContextMenu, m_ContextMenu, OnContextMenuChanged, VT_BOOL)
	DISP_STOCKPROP_READYSTATE()
END_DISPATCH_MAP()


// Event map
BEGIN_EVENT_MAP(CvtocxCtrl, COleControl)
	EVENT_STOCK_KEYPRESS()
	EVENT_STOCK_READYSTATECHANGE()
END_EVENT_MAP()


// Property pages
// Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CvtocxCtrl, 2)
	PROPPAGEID(CvtocxPropScreen::guid)
	PROPPAGEID(CvtocxPropNavigation::guid)
END_PROPPAGEIDS(CvtocxCtrl)


// Initialize class factory and guid
IMPLEMENT_OLECREATE_EX(CvtocxCtrl, "VTOCX.vtocxCtrl.1",
	0x246039db, 0x3d65, 0x47ec, 0xb9, 0x29, 0xc6, 0xa0, 0x40, 0x98, 0x14, 0x83)


// Type library ID and version
IMPLEMENT_OLETYPELIB(CvtocxCtrl, _tlid, _wVerMajor, _wVerMinor)


// Interface IDs
const IID BASED_CODE IID_Dvtocx =
		{ 0x36A29705, 0xDCF3, 0x4012, { 0x8F, 0xDA, 0x7, 0x57, 0x51, 0x31, 0xBC, 0x8 } };
const IID BASED_CODE IID_DvtocxEvents =
		{ 0x66FCF5BE, 0x102F, 0x45FA, { 0x85, 0x74, 0xCC, 0x67, 0x34, 0x56, 0x24, 0xA9 } };


// Control type information
static const DWORD BASED_CODE _dwvtocxOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CvtocxCtrl, IDS_VTOCX, _dwvtocxOleMisc)


 HRESULT CreateComponentCategory(CATID catid, WCHAR* catDescription)
 {
	  ICatRegister* pcr = NULL ;
	  HRESULT hr = S_OK ;

	  hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
			 NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void**)&pcr);
	  if (FAILED(hr))
			 return hr;

	  // Make sure the HKCR\Component Categories\{..catid...}
	  // key is registered
	  CATEGORYINFO catinfo;
	  catinfo.catid = catid;
	  catinfo.lcid = 0x0409 ; // english

	  // Make sure the provided description is not too long.
	  // Only copy the first 127 characters if it is
	  int len = wcslen(catDescription);
	  if (len>127)
			len = 127;
	  wcsncpy(catinfo.szDescription, catDescription, len);
	  // Make sure the description is null terminated
	  catinfo.szDescription[len] = '\0';

	  hr = pcr->RegisterCategories(1, &catinfo);
	  pcr->Release();

	  return hr;
}

HRESULT RegisterCLSIDInCategory(REFCLSID clsid, CATID catid)
{
	// Register your component categories information.
	ICatRegister* pcr = NULL ;
	HRESULT hr = S_OK ;
	hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
			NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void**)&pcr);
	if (SUCCEEDED(hr))
	{
		   // Register this category as being "implemented" by
		   // the class.
		   CATID rgcatid[1] ;
		   rgcatid[0] = catid;
		   hr = pcr->RegisterClassImplCategories(clsid, 1, rgcatid);
	}

	if (pcr != NULL)
		  pcr->Release();

	return hr;
}

HRESULT UnRegisterCLSIDInCategory(REFCLSID clsid, CATID catid)
{
	ICatRegister* pcr = NULL ;
	HRESULT hr = S_OK ;
	hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
			 NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void**)&pcr);
	if (SUCCEEDED(hr))
	{
		// Unregister this category as being "implemented" by
		// the class.
		CATID rgcatid[1] ;
		rgcatid[0] = catid;
		hr = pcr->UnRegisterClassImplCategories(clsid, 1, rgcatid);
	}

	if (pcr != NULL)
	pcr->Release();

	return hr;
}

//
// Adds or removes system registry entries for CvtocxCtrl
//
BOOL CvtocxCtrl::CvtocxCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// Verify that your control follows apartment-model threading rules.
	// Refer to MFC TechNote 64 for more information.
	// If your control does not conform to the apartment-model rules, then
	// you must modify the code below, changing the 6th parameter from
	// afxRegApartmentThreading to 0.

	if (bRegister)
	{
		HRESULT hr = S_OK ;

		// register as safe for scripting
		hr = CreateComponentCategory(CATID_SafeForScripting,
				L"Controls that are safely scriptable");
		if (FAILED(hr))
			return FALSE;

		hr = RegisterCLSIDInCategory(m_clsid, CATID_SafeForScripting);
		if (FAILED(hr))
			return FALSE;

		// register as safe for initializing
		hr = CreateComponentCategory(CATID_SafeForInitializing,
				L"Controls safely initializable from persistent data");
		if (FAILED(hr))
			return FALSE;

		hr = RegisterCLSIDInCategory(m_clsid, CATID_SafeForInitializing);
		if (FAILED(hr))
			return FALSE;

		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_VTOCX,
			IDB_VTOCX,
			afxRegInsertable | afxRegApartmentThreading,
//			0,
			_dwvtocxOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	}
	else
	{
		HRESULT hr = S_OK ;
		hr = UnRegisterCLSIDInCategory(m_clsid, CATID_SafeForScripting);

		if (FAILED(hr))
			return FALSE;

		hr = UnRegisterCLSIDInCategory(m_clsid, CATID_SafeForInitializing);

		if (FAILED(hr))
			return FALSE;

		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
	}

/*
	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_VTOCX,
			IDB_VTOCX,
			afxRegApartmentThreading,
			_dwvtocxOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
*/
}


class VTCapture : public Capture
{
public:
	VTCapture(void* winId) : Capture(winId) {}
	void OnLButtonDown(UINT flags, int x, int y);
	void OnMButtonDown(UINT flags, int x, int y);
	void OnRButtonDown(UINT flags, int x, int y);
	void OnLButtonUp(UINT flags, int x, int y);
	void OnMButtonUp(UINT flags, int x, int y);
	void OnRButtonUp(UINT flags, int x, int y);
	void OnMouseMove(UINT flags, int x, int y);
};


// Singleton pointer
CvtocxCtrl *CvtocxCtrl::s_pSingleton;

//
// Constructor
//
CvtocxCtrl::CvtocxCtrl() :
	_rendering(true),
	_ready(false)
{
	s_pSingleton = this;

	InitializeIIDs(&IID_Dvtocx, &IID_DvtocxEvents);

#if CALL_VTP
	m_pTScene = NULL;
	m_pCapture = NULL;
	m_pFlyer = NULL;
#endif
	m_pDC = NULL;
	m_glThread = NULL;
	m_nTimer = NULL;

	// Get the installation folder from windows registry
	HKEY key;
	RegOpenKeyEx(HKEY_CLASSES_ROOT, "CLSID\\{246039DB-3D65-47EC-B929-C6A040981483}\\InprocServer32", 0, KEY_QUERY_VALUE, &key);
	DWORD type;
	DWORD size = 1024;
	char data[1024];
	RegQueryValueEx(key, 0, 0, &type, (LPBYTE)data, &size);
	while (data[--size] != '\\'); data[size] = 0;
	osgDB::Registry *reg = osgDB::Registry::instance();
	reg->setLibraryFilePathList(data);

	// Hack to avoid assertion failure at exit time.
	reg->ref();

	VTSTARTLOG("debug_ocx.txt");
	VTLOG("Started debug_ocx.txt...\n\n");

#if USE_GLTHREAD
	m_glThread = new CGLThread<CvtocxCtrl>(this);
#endif

	m_lReadyState = READYSTATE_LOADING;
	CWinApp* pApp = AfxGetApp();

	m_NavSpeed = atof((LPCTSTR) (pApp->GetProfileString("Navigation", "NavSpeed", "0")));

	// Don't allow right-click to open a context menu
	m_ContextMenu = false;

	m_Src.SetControl(this);
}


// CvtocxCtrl::~CvtocxCtrl - Destructor

CvtocxCtrl::~CvtocxCtrl()
{
	// Cleanup your control's instance data here.
}


void CvtocxCtrl::OnTimer(UINT_PTR nIDEvent)
{
#if FULL_LOGGING
	VTLOG("OnTimer event: %d\n", nIDEvent);
#endif

	// Call base class handler.
	//COleControl::OnTimer(nIDEvent);

	// Invalidate to cause a Paint
	InvalidateRect(NULL,FALSE);	//for Continuous Rendering
}

BOOL CvtocxCtrl::OnIdle(LONG lCount)
{
	// NEVER GETS HERE.  OnIdle is only called by the message pump in the
	//  client's framework, not in this control.

	// That's also why the article 'Continuous Updating in MFC':
	//  http://www.gamedev.net/reference/articles/article2204.asp
	// does not work for an OCX.

	VTLOG("OnIdle %d\n", lCount);
	// return TRUE if more idle processing
	//return CWnd::OnIdle(lCount);
	return FALSE;
}

// Drawing function
void CvtocxCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	VTLOG("OnDraw, dc %x (m_hDC %x), bounds LRTB [%d %d %d %d] invalid [%d %d %d %d]\n",
		pdc,
		pdc->m_hDC,
		rcBounds.left, rcBounds.right, rcBounds.top, rcBounds.bottom,
		rcInvalid.left, rcInvalid.right, rcInvalid.top, rcInvalid.bottom
		);

/*	if(!_licenseOK)
	{
		RECT rect = rcBounds;
		pdc->FillRect(rcBounds, CBrush::FromHandle((HBRUSH)GetStockObject(WHITE_BRUSH)));
		pdc->Ellipse(rcBounds);
		pdc->DrawText("License Expired!", &rect, DT_CENTER|DT_VCENTER);
	} */
}

// Persistence support
void CvtocxCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// Call PX_ functions for each persistent custom property.
	PX_Color( pPX, "ClearColor", m_ClearColor, RGB(51, 51, 102));
	PX_Bool( pPX, "Stereo", m_Stereo, FALSE);

	PX_String( pPX, "TerrainFile", m_TerrainFile, m_TerrainFile);
	PX_String( pPX, "TerrainPaths", m_TerrainPaths, m_TerrainPaths);
	PX_String( pPX, "LocalSrc", m_LocalSrc, m_LocalSrc);

	PX_Float( pPX, "NavSpeed", m_NavSpeed, m_NavSpeed);

	PX_Bool( pPX, "ContextMenu", m_ContextMenu, m_ContextMenu);

	PX_DataPath( pPX, _T("SRC"), m_Src);
}


// CvtocxCtrl::OnResetState - Reset control to default state

void CvtocxCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	VTLOG("OnResetState\n");
	// Reset any other control state here.
}


// CvtocxCtrl::AboutBox - Display an "About" box to the user

void CvtocxCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_VTOCX);
	dlgAbout.DoModal();
}

bool CvtocxCtrl::SetupGLContext(void)
{
	VTLOG1("SetupGLContext\n");

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW|
		PFD_SUPPORT_OPENGL|
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0,0,0,0,0,0,
		0,0,0,0,0,0,0,
		32,
		0,0,
		PFD_MAIN_PLANE,
		0,
		0,0,0
	};

	int pixelFormat =
		ChoosePixelFormat(m_pDC->m_hDC, &pfd);
	BOOL success =
		SetPixelFormat(m_pDC->m_hDC, pixelFormat, &pfd);
	if (success == 0)
	{
		DWORD err = GetLastError();
		VTLOG("SetPixelFormat error: %d (0x%x)\n", err, err);
		return false;
	}

	DescribePixelFormat(m_pDC->m_hDC, pixelFormat, sizeof(pfd), &pfd);

	// if(pfd.dwFlags & PFD_NEED_PALETTE) SetupPalette();

	m_hGLContext = wglCreateContext(m_pDC->m_hDC);
	VTLOG(" made glcontext: %x\n", m_hGLContext);

	if (m_hGLContext == NULL)
	{
		DWORD err = GetLastError();
		VTLOG("wglCreateContext error: %d (0x%x)\n", err, err);
		return false;
	}

	wglMakeCurrent(m_pDC->m_hDC,m_hGLContext);
	//wglMakeCurrent(m_pDC->m_hDC, NULL);
	// now we have a GL context !
	return true;
}

void CvtocxCtrl::frame(CDC *pdc)
{
	if (pdc == NULL)
		pdc = m_pDC;

#if FULL_LOGGING
	VTLOG1("frame()\n");
#endif
	if (!pdc || !m_hGLContext)
		return;

	//if(!_sceneView.valid()) return;
	if (m_glThread) m_glThread->lock();

#if FULL_LOGGING
	//VTLOG(" making glcontext %x active\n", m_hGLContext);
#endif
	//wglMakeCurrent(m_pDC->m_hDC, m_hGLContext);

	//_sceneView->update();
	//_sceneView->cull();

#if CALL_VTP
	// Render the scene
	//_sceneView->draw();
	vtGetScene()->DoUpdate();
#else
	// test code: draw a red-green-blue axis in X and Y
	static int step = 0;
	step++;
	if (step == 60)
	{
		//VTLOG1("tick\n");
		step = 0;
	}
	glColor3f(1, 0, 0);
	glBegin(GL_LINES);
	glVertex3f(-1,0,0);
	glVertex3f( 1,0,0);
	glEnd();

	glColor3f(0, 1, 0);
	glBegin(GL_LINES);
#if NDEBUG
	glVertex3f(-(float)step/120,-1,0);
	glVertex3f(-(float)step/120, 1,0);
#else
	glVertex3f((float)step/120,-1,0);
	glVertex3f((float)step/120, 1,0);
#endif
	glEnd();

	glColor3f(0, 0, 1);
	glBegin(GL_LINES);
	glVertex3f(-1,-1, -1);
	glVertex3f(1, 1, 1);
	glEnd();
#endif
	if (m_glThread) m_glThread->unlock();

	BOOL success = SwapBuffers(pdc->m_hDC);
	if (success == 0)
	{
		DWORD err = GetLastError();
		VTLOG("SwapBuffers error: %d (0x%x)\n", err, err);
	}

#if CALL_VTP
	vtCamera *cam = vtGetScene()->GetCamera();
	FPoint3 pos = cam->GetTrans();
	DPoint3 earth;
	g_Conv.ConvertToEarth(pos, earth);

	m_PositionX = earth.x;
	m_PositionY = earth.y;
	m_PositionZ = earth.z;
#endif

#if FULL_LOGGING
	//VTLOG(" making glcontext %x active\n", NULL);
#endif
	//wglMakeCurrent(m_pDC->m_hDC, NULL);
#if FULL_LOGGING
	VTLOG1("\tframe() exit.\n");
#endif
}

void CvtocxCtrl::resize(const long w, const long h)
{
	VTLOG("resize(%d, %d)\n", w, h);

	glViewport(0,0,w,h);
#if CALL_VTP
	vtGetScene()->SetWindowSize(w,h);
#endif
	OnScreenChanged();
}

// CvtocxCtrl message handlers

int CvtocxCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	VTLOG1("OnCreate\n");

	// We set CS_OWNDC so that we have our own DC which won't go away
	lpCreateStruct->style |= (WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CS_OWNDC);

	if (COleControl::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create progress control
	BOOL success = m_Progress.Create(WS_CHILD|WS_VISIBLE,
		CRect(0,0,256,30), this, 1);
	if ( !success )
	{
		::AfxMessageBox( _T("Unable to create progress render control.") );
	}
	m_Progress.SetRange(0, 100);

	// Safe to do this here if the client is not IE
	if (m_pDC == NULL)
	{
		CDC *pdc = GetDC();
		PrepareToRender(pdc->m_hDC);
	}

	Refresh();
	SetFocus();

	VTLOG1("OnCreate exit.\n");
	return 0;
}

void CvtocxCtrl::PrepareToRender(HDC dc)
{
	//m_pDC = GetDC();
	m_pDC = CDC::FromHandle(dc);
	VTLOG(" got from handle dc: %x (m_hDC %x)\n", m_pDC, m_pDC->m_hDC);

	if (!SetupGLContext())
	{
		// Go no further
		m_pDC = NULL;
		return;
	}

#if CALL_VTP
	m_pCapture = new VTCapture(GetSafeHwnd());
	m_pCapture->init();

	// We'll track key state by waching MFC calls and putting the values into
	//  an array.  Tell vtlib about the array.
	for (int i = 0; i < 512; i++)
		m_bKeyState[i] = false;
	vtGetScene()->SetKeyStates(m_bKeyState);

	Create3DScene();
#endif

#if USE_TIMER
	int timer_id = 1;
	int timer_miliseconds = TIMER_MILISECONDS;
	VTLOG1("Calling SetTimer\n");
	m_nTimer = SetTimer(timer_id, timer_miliseconds, NULL);
	VTLOG(" nTimer: %d\n", m_nTimer);
#endif

	if (m_glThread) m_glThread->startThread();
}

void CvtocxCtrl::OnDestroy()
{
	VTLOG1("OnDestroy\n");

#if USE_TIMER
	if (m_nTimer != NULL)
		KillTimer(m_nTimer);
#endif

#if CALL_VTP
	Cleanup3DScene();
#endif

	VTLOG1("calling COleControl::OnDestroy\n");
	COleControl::OnDestroy();

	_rendering = false;;
	if (m_glThread)
	{
		VTLOG1("calling _glThread->join\n");
		m_glThread->join();
		VTLOG1("calling delete _glThread\n");
		delete m_glThread;
	}

	//if (wglGetCurrentContext() != NULL)
	//	wglMakeCurrent(NULL,NULL);
	//if (m_hGLContext != NULL)
	//{
	//	wglDeleteContext(m_hGLContext);
	//	m_hGLContext = NULL;
	//}

	VTLOG1("calling wglDeleteContext\n");
	wglDeleteContext(m_hGLContext);

	VTLOG1("OnDestroy done.\n");
}

void CvtocxCtrl::OnSize(UINT nType, int cx, int cy)
{
	VTLOG("OnSize(%d, %d)\n", cx, cy);
	COleControl::OnSize(nType, cx, cy);

	glViewport(0, 0, cx, cy);
#if CALL_VTP
	vtGetScene()->SetWindowSize(cx, cy);
#endif

	// Inform progress dialog of new size
	RECT rect;
	GetClientRect(&rect);
	rect.top = rect.bottom - 16;
	m_Progress.SetWindowPos(NULL, 0, rect.top, rect.right, 16,
		SWP_NOREPOSITION);

	OnScreenChanged();
}

void CvtocxCtrl::OnPaint()
{
#if FULL_LOGGING
	VTLOG1("OnPaint\n");
#endif

#if RENDER_ON_PAINT
	CPaintDC dc(this); // device context for painting

	// Render the scene
	frame(&dc);
#endif

	// Upon first paint, if there is a terrain we should load, load it.
	static bool bFirstPaint = true;
	if (bFirstPaint)
	{
		if (m_TerrainFile != CString(""))
			LoadTerrain(m_TerrainFile);
		bFirstPaint = false;
	}

#if USE_CONTINUOUS_PAINT
	// Try to be more friendly with the event loop to give the rest of the UI
	//  a chance to refresh:

	// Calling Sleep does slow down drawing, but doesn't help the other events:
	//Sleep(500);	// milisec

	// Calling OnIdle just seems to lock up the application.
	//CWinApp* pApp = AfxGetApp();
	//LONG lCount = 0;
	//while (pApp->OnIdle(lCount))
	//{
	//	lCount++;
	//	if (lCount == 20)
	//		break;
	//}
	//VTLOG("lCount %d\n", lCount);

	InvalidateRect(NULL,FALSE);	//for Continuous Rendering
#endif
#if FULL_LOGGING
	VTLOG("\tOnPaint exit.\n");
#endif
}

void CvtocxCtrl::OnClearColorChanged(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	SetModifiedFlag();
}

void CvtocxCtrl::OnStereoChanged(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	SetModifiedFlag();
}

BOOL CvtocxCtrl::OnSetExtent(LPSIZEL lpSizeL)
{
	VTLOG("OnSetExtent(%d, %d)\n", lpSizeL->cx, lpSizeL->cy);

	SIZEL size;
	size.cx = lpSizeL->cx; size.cy = lpSizeL->cy;
	if(m_pDC)
	{
		m_pDC->HIMETRICtoDP(&size);
		//if(_sceneView.valid())
		//	_glThread->resizeViewport(size.cx, size.cy);
	}

	return COleControl::OnSetExtent(lpSizeL);
}

void CvtocxCtrl::OnScreenChanged(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//_sceneView->setProjectionMatrixAsFrustum(left, right, bottom, top, zNear, zFar);
}

BSTR CvtocxCtrl::GetTerrainFile(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return m_TerrainFile.AllocSysString();
}

void CvtocxCtrl::SetTerrainFile(LPCTSTR newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CString oldVal = m_TerrainFile;
	m_TerrainFile = newVal;
#if CALL_VTP
	if (m_TerrainFile != oldVal)
		LoadTerrain(m_TerrainFile);
#endif
}

BSTR CvtocxCtrl::GetTerrainPaths(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return m_TerrainPaths.AllocSysString();
}

void CvtocxCtrl::SetTerrainPaths(LPCTSTR newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CString oldVal = m_TerrainPaths;
	m_TerrainPaths = newVal;
#if CALL_VTP
	if (m_TerrainPaths != oldVal)
		AddTerrainPaths(m_TerrainPaths);
#endif
}

BSTR CvtocxCtrl::GetSrc(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CString strResult;

	strResult = m_Src.GetPath();

	return strResult.AllocSysString();
}

void CvtocxCtrl::SetSrc(LPCTSTR newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	Load(newVal, m_Src);

	SetModifiedFlag();

	if (m_glThread) m_glThread->lock();
	//_sceneView->setSceneData(m_Src.getNode().get());
	if (m_glThread) m_glThread->unlock();

	SetModifiedFlag();
}

BSTR CvtocxCtrl::GetLocalSrc(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return m_LocalSrc.AllocSysString();
}

void CvtocxCtrl::SetLocalSrc(LPCTSTR newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	m_LocalSrc = newVal;
	//osg::Node* node = osgDB::readNodeFile(m_LocalSrc.GetBuffer(0));

	//if (m_glThread) m_glThread->lock();

	/*_sceneView->setSceneData(node);*/
	//if (m_glThread) m_glThread->unlock();

	SetModifiedFlag();
}

#define ID_MENU_STEREO 1100
#define ID_MENU_FULLSCREEN 1200
#define ID_MENU_VIEWPOINTS 2300
#define ID_MENU_PROPERTIES 9000

//
// MFC calls this method whenever the user right-clicks in the window, so if
//  we don't want to open a context menu, all we have to do is ignore it.
//
void CvtocxCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if(!m_ContextMenu)
		return;

	CMenu menu;
	CMenu viewMenu;

	menu.CreatePopupMenu();
	//menu.AppendMenu(MF_ENABLED|(m_Stereo?MF_CHECKED:MF_UNCHECKED), ID_MENU_STEREO, (LPCTSTR)"Stereo");
	//menu.AppendMenu(MF_ENABLED, ID_MENU_FULLSCREEN, (LPCTSTR)"&Fullscreen");

	//viewMenu.CreatePopupMenu();
	//osg::ref_ptr<osg::Group> viewPoints = _sceneView->getViewPoints();
	//if(viewPoints.valid())
	//{
	//	if(viewPoints->getNumChildren())
	//	{
	//		for(unsigned int i = 0; i < viewPoints->getNumChildren(); i++)
	//			viewMenu.AppendMenu(MF_ENABLED, ID_MENU_VIEWPOINTS + i + 1, (LPCTSTR) viewPoints->getChild(i)->getName().c_str());
	//	}
	//}
	//viewMenu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)0L);
	//viewMenu.AppendMenu(MF_ENABLED, ID_MENU_VIEWPOINTS, (LPCTSTR) "Reset");
	//menu.AppendMenu(MF_ENABLED|MF_POPUP, (UINT_PTR) viewMenu.m_hMenu, (LPCTSTR)"&Viewpoints");
	//menu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)0L);
	menu.AppendMenu(MF_ENABLED, ID_MENU_PROPERTIES, (LPCTSTR)"&Properties...");

	POINT pt;
	GetCursorPos(&pt);
	int cmd = menu.TrackPopupMenuEx(TPM_LEFTALIGN|TPM_TOPALIGN|TPM_NONOTIFY|TPM_RETURNCMD|TPM_RIGHTBUTTON, pt.x, pt.y, this, 0L);

	switch(cmd - cmd%100) // the whole hundred
	{
	case(ID_MENU_STEREO):
		m_Stereo = !m_Stereo;
		OnStereoChanged();
		break;
	case(ID_MENU_FULLSCREEN):
		m_Fullscreen = !m_Fullscreen;
		OnFullscreenChanged();
		break;
	case(ID_MENU_VIEWPOINTS):
		//if(cmd%100 == 0)
		//	_sceneView->getMotionModel()->home();
		//else
		//	_sceneView->getMotionModel()->setViewPoint(dynamic_cast<osg::PositionAttitudeTransform*>(viewPoints->getChild(cmd%100 - 1)));
		break;
	case(ID_MENU_PROPERTIES):
		OnProperties(0, NULL, NULL);
		break;
	}
}
void CvtocxCtrl::OnFullscreenChanged(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(m_Fullscreen) // just switch to full screen
	{
		DWORD s = GetWindowLong(GetSafeHwnd(), GWL_STYLE);
		s &= ~WS_OVERLAPPEDWINDOW;
		s |= WS_POPUP;
		SetWindowLong(GetSafeHwnd(), GWL_STYLE, s);
		SetWindowPos(&CWnd::wndTop,
					 0, 0,
					 1600, 1000,
					 SWP_FRAMECHANGED);
	}
	else
	{
		// destroy the fullscreen dialog
	}

	SetModifiedFlag();
}

void CvtocxCtrl::OnPositionChanged(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Add your property handler code here

	SetModifiedFlag();
}

void CvtocxCtrl::OnShowWindow(BOOL bShow, UINT nStatus)
{
	VTLOG1("OnShowWindow: ");
	COleControl::OnShowWindow(bShow, nStatus);

	if(bShow)
	{
		RECT rect;
		GetClientRect(&rect);
		VTLOG("ClientRect: %d, %d\n", rect.right, rect.bottom);
		if (m_glThread) m_glThread->resizeViewport(rect.right, rect.bottom);
	}
}

void CvtocxCtrl::OnNavSpeedChanged(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	VTLOG("OnNavSpeedChanged: %f\n", m_NavSpeed);
#if CALL_VTP
	if (m_pFlyer)
		m_pFlyer->SetSpeed(m_NavSpeed);
#endif
	SetModifiedFlag();
}

void CvtocxCtrl::JumpToNo(LONG vp)
{
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());

	//if(vp == 0)
	//	_sceneView->getMotionModel()->home();
	//else
	//{
	//	osg::ref_ptr<osg::Group> viewPoints = _sceneView->getViewPoints();
	//	if(vp <= viewPoints->getNumChildren())
	//		_sceneView->getMotionModel()->setViewPoint(dynamic_cast<osg::PositionAttitudeTransform*>(viewPoints->getChild(vp - 1)));
	//}
	VTLOG("JumpToNo(%d)\n", vp);
}

void CvtocxCtrl::ResetView()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	VTLOG("ResetView()\n");

#if CALL_VTP
	vtCamera *cam = vtGetScene()->GetCamera();
	cam->SetTrans(FPoint3(0,0,0));
	cam->PointTowards(FPoint3(0,0,-1));
#endif
}

void CvtocxCtrl::OnContextMenuChanged(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Add your property handler code here

	SetModifiedFlag();
}

#if CALL_VTP
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
void VTCapture::OnMouseMove(UINT nFlags, int x, int y)
{
	vtMouseEvent event;

	event.type = VT_MOVE;
	event.button = VT_NONE;
	event.flags = MFCFlagsToVT(nFlags);
	event.pos.Set(x, y);

	vtGetScene()->OnMouse(event);
}

void VTCapture::OnLButtonUp(UINT nFlags, int x, int y)
{
	VTLOG("Got OnLButtonUp(%d, %d)\n", x, y);
	vtMouseEvent event;

	event.type = VT_UP;
	event.button = VT_LEFT;
	event.flags = MFCFlagsToVT(nFlags);
	event.pos.Set(x, y);

	vtGetScene()->OnMouse(event);
}

void VTCapture::OnLButtonDown(UINT nFlags, int x, int y)
{
	VTLOG("Got OnLButtonDown(%d, %d)\n", x, y);
	vtMouseEvent event;

	event.type = VT_DOWN;
	event.button = VT_LEFT;
	event.flags = MFCFlagsToVT(nFlags);
	event.pos.Set(x, y);

	vtGetScene()->OnMouse(event);
}

void VTCapture::OnRButtonDown(UINT nFlags, int x, int y)
{
	VTLOG("Got OnRButtonDown(%d, %d)\n", x, y);
	vtMouseEvent event;

	event.type = VT_DOWN;
	event.button = VT_RIGHT;
	event.flags = MFCFlagsToVT(nFlags);
	event.pos.Set(x, y);

	vtGetScene()->OnMouse(event);
}

void VTCapture::OnRButtonUp(UINT nFlags, int x, int y)
{
	VTLOG("Got OnRButtonUp(%d, %d)\n", x, y);
	vtMouseEvent event;

	event.type = VT_UP;
	event.button = VT_RIGHT;
	event.flags = MFCFlagsToVT(nFlags);
	event.pos.Set(x, y);

	vtGetScene()->OnMouse(event);
}

void VTCapture::OnMButtonDown(UINT nFlags, int x, int y)
{
	vtMouseEvent event;

	event.type = VT_DOWN;
	event.button = VT_MIDDLE;
	event.flags = MFCFlagsToVT(nFlags);
	event.pos.Set(x, y);

	vtGetScene()->OnMouse(event);
}

void VTCapture::OnMButtonUp(UINT nFlags, int x, int y)
{
	vtMouseEvent event;

	event.type = VT_UP;
	event.button = VT_MIDDLE;
	event.flags = MFCFlagsToVT(nFlags);
	event.pos.Set(x, y);

	vtGetScene()->OnMouse(event);
}

//-----------------------The Following is VTerrain Code-----------------

bool progress_callback(int pos)
{
	CvtocxCtrl::s_pSingleton->OnProgress(pos);
	return false;	// Not cancelled
}

//
// Create the 3d scene
//
void CvtocxCtrl::Create3DScene()
{
	int argc = 0;
	char **argv = 0;

	VTLOG1("CreateScene\n");
	// Get a handle to the vtScene - one is already created for you
	vtScene *pScene = vtGetScene();
	pScene->Init(argc, argv);

	// Look up the camera
	vtCamera *pCamera = pScene->GetCamera();
	pCamera->SetHither(10);
	pCamera->SetYon(100000);

	// The  terrain scene will contain all the terrains that are created.
	m_pTScene = new vtTerrainScene;

	// Set the global data path
	// Read the vt datapaths from the user's vtp.xml file, if they have one.
	char buf1[MAX_PATH];
	SHGetFolderPathA(
		NULL,			// parent window, not used
		CSIDL_APPDATA,
		NULL,			// access token (current user)
		SHGFP_TYPE_CURRENT,	// current path, not just default value
		buf1);
	vtString AppDataUser = buf1;
	vtLoadDataPath(AppDataUser, NULL);

	AddTerrainPaths(m_TerrainPaths);

	// Begin creating the scene, including the sun and sky
	vtGroup *pTopGroup = m_pTScene->BeginTerrainScene();

	// Tell the scene graph to point to this terrain scene
	pScene->SetRoot(pTopGroup);

	// Create a navigation engine to move around on the terrain
	m_pFlyer = new vtTerrainFlyer(1.0f);
	m_pFlyer->SetTarget(pCamera);
	pScene->AddEngine(m_pFlyer);

	// Minimum height over terrain is 100 m
	m_pConstrain = new vtHeightConstrain(100);
	m_pConstrain->SetTarget(pCamera);
	pScene->AddEngine(m_pConstrain);

	// We no longer do this here, instead we wait for first paint, so that the
	//  progress bar can be shown.
//	if (m_TerrainFile != CString(""))
//		LoadTerrain(m_TerrainFile);
}

bool CvtocxCtrl::LoadTerrain(const char *terrain_file)
{
	VTLOG("LoadTerrain(%s)\n", terrain_file);

	// Create a new vtTerrain, read its paramters from a file
	vtString path = FindFileOnPaths(vtGetDataPath(), terrain_file);
	if (path == "")
	{
		vtString msg = "Couldn't find terrain file: ";
		msg += terrain_file;
		AfxMessageBox(msg);
		return false;
	}
	vtTerrain *pTerr = new vtTerrain;
	pTerr->SetParamFile(path);
	pTerr->SetProgressCallback(progress_callback);

	m_Progress.ShowWindow(SW_SHOW);

	// Add the terrain to the scene, and contruct it
	m_pTScene->AppendTerrain(pTerr);
	if (!m_pTScene->BuildTerrain(pTerr))
	{
		m_Progress.ShowWindow(SW_HIDE);
		AfxMessageBox("Terrain creation failed.");
		return false;
	}

	m_pTScene->SetCurrentTerrain(pTerr);

	// Get flight speed from terrain parameters
	float fSpeed = pTerr->GetParams().GetValueFloat(STR_NAVSPEED);
	if (m_NavSpeed != 0)
		fSpeed = m_NavSpeed;
	else
		m_NavSpeed = fSpeed;
	VTLOG("using NavSpeed: %f\n", m_NavSpeed);
	m_pFlyer->SetSpeed(m_NavSpeed);

	vtHeightField3d *hf = pTerr->GetHeightField();
	m_pConstrain->SetHeightField(hf);
	m_pFlyer->SetHeightField(hf);

	m_Progress.ShowWindow(SW_HIDE);
	return true;
}

void CvtocxCtrl::Cleanup3DScene()
{
	VTLOG1("Cleanup3DScene\n");
	vtGetScene()->SetRoot(NULL);
	if (m_pTScene)
	{
		m_pTScene->CleanupScene();
		delete m_pTScene;
	}
	vtGetScene()->Shutdown();
}

// Append any paths that the OCX's client wants.
void CvtocxCtrl::AddTerrainPaths(const CString &terr_paths)
{
	vtStringArray paths = vtGetDataPath();
	vtStringArray patharray;
	char buf[4096];
	strcpy(buf, terr_paths);
	vtTokenize(buf, ";", patharray);

	VTLOG("From client, got %d:\n", patharray.size());
	for (unsigned int i = 0; i < patharray.size(); i++)
	{
		VTLOG1("\t");
		VTLOG1(patharray[i]);
		VTLOG1("\n");
		paths.push_back(patharray[i]);
	}
	vtSetDataPath(paths);
}

#endif	// CALL_VTP


void CvtocxCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	//VTLOG("OnChar(%d)\n", nChar);

	int flags = 0;

#if CALL_VTP
	if (m_bKeyState[VT_CONTROL])
		flags |= VT_CONTROL;

	if (m_bKeyState[VT_SHIFT])
		flags |= VT_SHIFT;

	if (nFlags & KF_ALTDOWN)
		flags |= VT_ALT;

	// pass the char (UTF-16) to the vtlib Scene
	vtGetScene()->OnKey(nChar, flags);
#endif

	COleControl::OnChar(nChar, nRepCnt, nFlags);
}

#if CALL_VTP
int Win32VirtualKeyToVtlib(UINT nChar)
{
	switch (nChar)
	{
	case VK_LBUTTON: return VTK_LBUTTON; break;
	case VK_RBUTTON: return VTK_RBUTTON; break;
	case VK_CANCEL: return VTK_CANCEL; break;
	case VK_MBUTTON: return VTK_MBUTTON; break;
	case VK_CLEAR: return VTK_CLEAR; break;
	case VK_SHIFT: return VTK_SHIFT; break;
	case VK_MENU: return VTK_ALT; break;
	case VK_CONTROL: return VTK_CONTROL; break;
	case VK_PAUSE: return VTK_PAUSE; break;
	case VK_CAPITAL: return VTK_CAPITAL; break;
	case VK_END: return VTK_END; break;
	case VK_HOME: return VTK_HOME; break;
	case VK_LEFT: return VTK_LEFT; break;
	case VK_UP: return VTK_UP; break;
	case VK_RIGHT: return VTK_RIGHT; break;
	case VK_DOWN: return VTK_DOWN; break;
	case VK_SELECT: return VTK_SELECT; break;
	case VK_PRINT: return VTK_PRINT; break;
	case VK_EXECUTE: return VTK_EXECUTE; break;
	case VK_SNAPSHOT: return VTK_SNAPSHOT; break;
	case VK_INSERT: return VTK_INSERT; break;
	case VK_HELP: return VTK_HELP; break;
	case VK_NUMPAD0: return VTK_NUMPAD0; break;
	case VK_NUMPAD1: return VTK_NUMPAD1; break;
	case VK_NUMPAD2: return VTK_NUMPAD2; break;
	case VK_NUMPAD3: return VTK_NUMPAD3; break;
	case VK_NUMPAD4: return VTK_NUMPAD4; break;
	case VK_NUMPAD5: return VTK_NUMPAD5; break;
	case VK_NUMPAD6: return VTK_NUMPAD6; break;
	case VK_NUMPAD7: return VTK_NUMPAD7; break;
	case VK_NUMPAD8: return VTK_NUMPAD8; break;
	case VK_NUMPAD9: return VTK_NUMPAD9; break;
	case VK_MULTIPLY: return VTK_MULTIPLY; break;
	case VK_ADD: return VTK_ADD; break;
	case VK_SEPARATOR: return VTK_SEPARATOR; break;
	case VK_SUBTRACT: return VTK_SUBTRACT; break;
	case VK_DECIMAL: return VTK_DECIMAL; break;
	case VK_DIVIDE: return VTK_DIVIDE; break;
	case VK_F1: return VTK_F1; break;
	case VK_F2: return VTK_F2; break;
	case VK_F3: return VTK_F3; break;
	case VK_F4: return VTK_F4; break;
	case VK_F5: return VTK_F5; break;
	case VK_F6: return VTK_F6; break;
	case VK_F7: return VTK_F7; break;
	case VK_F8: return VTK_F8; break;
	case VK_F9: return VTK_F9; break;
	case VK_F10: return VTK_F10; break;
	case VK_F11: return VTK_F11; break;
	case VK_F12: return VTK_F12; break;
	case VK_NUMLOCK: return VTK_NUMLOCK; break;
	case VK_SCROLL: return VTK_SCROLL; break;
	case VK_PRIOR: return VTK_PAGEUP; break;
	case VK_NEXT: return VTK_PAGEDOWN; break;
	}
	return -1;
}
#endif

void CvtocxCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	//VTLOG("OnKeyDown(%d)\n", nChar);
#if CALL_VTP
	int key = Win32VirtualKeyToVtlib(nChar);
	if (key != -1)
		m_bKeyState[key] = true;
#endif
	COleControl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CvtocxCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	//VTLOG("OnKeyUp(%d)\n", nChar);
#if CALL_VTP
	int key = Win32VirtualKeyToVtlib(nChar);
	if (key != -1)
		m_bKeyState[key] = false;
#endif
	COleControl::OnKeyUp(nChar, nRepCnt, nFlags);
}
