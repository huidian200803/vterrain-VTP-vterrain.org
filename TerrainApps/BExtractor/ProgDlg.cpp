//
// ProgDlg.cpp : implementation file
// CG: This file was added by the Progress Dialog component
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "StdAfx.h"
#include "resource.h"
#include "ProgDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg dialog

CProgressDlg::CProgressDlg(UINT nCaptionID)
{
	m_nCaptionID = CG_IDS_PROGRESS_CAPTION;
	if (nCaptionID != 0)
		m_nCaptionID = nCaptionID;

	m_nLower=0;
	m_nUpper=200;
	m_nStep=2;
	//{{AFX_DATA_INIT(CProgressDlg)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bParentDisabled = FALSE;
}

CProgressDlg::~CProgressDlg()
{
	if(m_hWnd!=NULL)
	  DestroyWindow();
}

BOOL CProgressDlg::DestroyWindow()
{
	ReEnableParent();
	return CDialog::DestroyWindow();
}

void CProgressDlg::ReEnableParent()
{
	if(m_bParentDisabled && (m_pParentWnd!=NULL))
	  m_pParentWnd->EnableWindow(TRUE);
	m_bParentDisabled=FALSE;
}

BOOL CProgressDlg::Create(CWnd *pParent)
{
	// Get the true parent of the dialog
	m_pParentWnd = CWnd::GetSafeOwner(pParent);

	// m_bParentDisabled is used to re-enable the parent window
	// when the dialog is destroyed. So we don't want to set
	// it to TRUE unless the parent was already enabled.

	if((m_pParentWnd!=NULL) && m_pParentWnd->IsWindowEnabled())
	{
	  m_pParentWnd->EnableWindow(FALSE);
	  m_bParentDisabled = TRUE;
	}

	if(!CDialog::Create(CProgressDlg::IDD,pParent))
	{
	  ReEnableParent();
	  return FALSE;
	}

	return TRUE;
}

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProgressDlg)
	DDX_Control(pDX, CG_IDC_PROGDLG_PROGRESS, m_Progress);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CProgressDlg, CDialog)
	//{{AFX_MSG_MAP(CProgressDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CProgressDlg::SetStatus(LPCTSTR szMessage)
{
	ASSERT(m_hWnd); // Don't call this _before_ the dialog has
					// been created. Can be called from OnInitDialog
	CWnd *pWndStatus = GetDlgItem(CG_IDC_PROGDLG_STATUS);

	// Verify that the static text control exists
	ASSERT(pWndStatus!=NULL);
	pWndStatus->SetWindowText(szMessage);
}

void CProgressDlg::OnCancel()
{
}

void CProgressDlg::SetRange(int nLower,int nUpper)
{
	m_nLower = nLower;
	m_nUpper = nUpper;
	m_Progress.SetRange32(nLower,nUpper);
}

int CProgressDlg::SetPos(int nPos)
{
	PumpMessages();
	return m_Progress.SetPos(nPos);
}

int CProgressDlg::SetStep(int nStep)
{
	m_nStep = nStep; // Store for later use in calculating percentage
	return m_Progress.SetStep(nStep);
}

int CProgressDlg::OffsetPos(int nPos)
{
	PumpMessages();
	return m_Progress.OffsetPos(nPos);
}

int CProgressDlg::StepIt()
{
	PumpMessages();
	return m_Progress.StepIt();
}

void CProgressDlg::PumpMessages()
{
	// Must call Create() before using the dialog
	ASSERT(m_hWnd!=NULL);

	MSG msg;
	// Handle dialog messages
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
	  if(!IsDialogMessage(&msg))
	  {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	  }
	}
}


/////////////////////////////////////////////////////////////////////////////
// CProgressDlg message handlers

BOOL CProgressDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_Progress.SetRange(m_nLower,m_nUpper);
	m_Progress.SetStep(m_nStep);
	m_Progress.SetPos(m_nLower);

	CString strCaption;
	VERIFY(strCaption.LoadString(m_nCaptionID));
	SetWindowText(strCaption);

	return TRUE;
}
