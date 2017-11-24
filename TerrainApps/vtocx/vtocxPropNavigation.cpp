// vtocxPropNavigation.cpp : implementation file
//

#include "stdafx.h"
#include "vtocx.h"
#include "vtocxPropNavigation.h"

// CvtocxPropNavigation dialog
//
IMPLEMENT_DYNCREATE(CvtocxPropNavigation, COlePropertyPage)

//
// Message map
//
BEGIN_MESSAGE_MAP(CvtocxPropNavigation, COlePropertyPage)
END_MESSAGE_MAP()


// Initialize class factory and guid
//
// {CE665C44-0E20-4080-8B93-33BE7013E394}
IMPLEMENT_OLECREATE_EX(CvtocxPropNavigation, "vtocx.vtocxPropNavigation",
	0xce665c44, 0xe20, 0x4080, 0x8b, 0x93, 0x33, 0xbe, 0x70, 0x13, 0xe3, 0x94)


//
// Adds or removes system registry entries for CvtocxPropNavigation
//
BOOL CvtocxPropNavigation::CvtocxPropNavigationFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_VTOCX_PNV);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


//
// Constructor
//
CvtocxPropNavigation::CvtocxPropNavigation() :
	COlePropertyPage(IDD, IDS_VTOCX_PNV_CAPTION)
		, m_Velocity(0)
		, m_Rotation(0)
		, m_Pan(0)
	{
}

//
// CvtocxPropNavigation::DoDataExchange - Moves data between page and properties
//
void CvtocxPropNavigation::DoDataExchange(CDataExchange* pDX)
{
	DDP_Text(pDX, IDC_EDIT_VELOCITY, m_Velocity, _T("NavSpeed") );
	DDX_Text(pDX, IDC_EDIT_VELOCITY, m_Velocity);
	DDV_MinMaxFloat(pDX, m_Velocity, 0.001f, 1000.f);

	DDP_PostProcessing(pDX);
}

// CvtocxPropNavigation message handlers
