//
// vtocxPropScreen.cpp : implementation file
//

#include "stdafx.h"
#include "vtocx.h"
#include "vtocxPropScreen.h"


// CvtocxPropScreen dialog

IMPLEMENT_DYNCREATE(CvtocxPropScreen, COlePropertyPage)

// Message map

BEGIN_MESSAGE_MAP(CvtocxPropScreen, COlePropertyPage)
END_MESSAGE_MAP()

// Initialize class factory and guid

// {EDE9938E-2E17-4E99-88E4-1C43CB492247}
IMPLEMENT_OLECREATE_EX(CvtocxPropScreen, "vtocx.vtocxPropScreen",
	0xede9938e, 0x2e17, 0x4e99, 0x88, 0xe4, 0x1c, 0x43, 0xcb, 0x49, 0x22, 0x47)

// CvtocxPropScreen::CvtocxPropScreenFactory::UpdateRegistry -
// Adds or removes system registry entries for CvtocxPropScreen

BOOL CvtocxPropScreen::CvtocxPropScreenFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_VTOCX_PSC);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}

// CvtocxPropScreen::CvtocxPropScreen - Constructor

CvtocxPropScreen::CvtocxPropScreen() :
	COlePropertyPage(IDD, IDS_VTOCX_PSC_CAPTION)
{
}

// CvtocxPropScreen::DoDataExchange - Moves data between page and properties

void CvtocxPropScreen::DoDataExchange(CDataExchange* pDX)
{
	DDP_PostProcessing(pDX);
}

// CvtocxPropScreen message handlers
