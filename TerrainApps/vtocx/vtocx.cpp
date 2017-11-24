// vtocx.cpp : Implementation of CvtocxApp and DLL registration.

#include "stdafx.h"
#include "vtocx.h"
#include "vtdata/vtLog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CvtocxApp NEAR theApp;

const GUID CDECL BASED_CODE _tlid =
		{ 0xAD34DE08, 0x561, 0x4AC1, { 0xB7, 0x40, 0x7F, 0xAE, 0x0, 0xCA, 0x8D, 0x28 } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;


// CvtocxApp::InitInstance - DLL initialization

BOOL CvtocxApp::InitInstance()
{
	BOOL bInit = COleControlModule::InitInstance();

	if (bInit)
	{
		// Add your own module initialization code here.
	}
	return bInit;
}


// CvtocxApp::ExitInstance - DLL termination

int CvtocxApp::ExitInstance()
{
	// Add your own module termination code here.
	VTLOG("CvtocxApp::ExitInstance\n");

	return COleControlModule::ExitInstance();
}

BOOL CvtocxApp::OnIdle(LONG lCount)
{
	VTLOG("CvtocxApp::OnIdle\n");

	return COleControlModule::OnIdle(lCount);
}

void CvtocxApp::Update()
{
#if 0
	// We could repaint on idle (if we ever GOT idle)
	if (m_pMainView)
		m_pMainView->PaintOnIdle();
#endif
}

int CvtocxApp::Run()
{
	// NEVER GETS HERE!  The Run() in the client is used, not this ocx's Run.
	return COleControlModule::Run();
}

// DllRegisterServer - Adds entries to the system registry
//
STDAPI DllRegisterServer(void)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE))
		return ResultFromScode(SELFREG_E_CLASS);

	return NOERROR;
}

// DllUnregisterServer - Removes entries from the system registry
//
STDAPI DllUnregisterServer(void)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE))
		return ResultFromScode(SELFREG_E_CLASS);

	return NOERROR;
}

