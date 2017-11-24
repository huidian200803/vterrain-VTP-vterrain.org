//
// ProjectionDlg.cpp : implementation file
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "StdAfx.h"
#include "BExtractor.h"
#include "ProjectionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProjectionDlg dialog


CProjectionDlg::CProjectionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProjectionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProjectionDlg)
	m_iZone = 0;
	m_iProjection = -1;
	//}}AFX_DATA_INIT
}


void CProjectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProjectionDlg)
	DDX_Control(pDX, IDC_PROJECTION, m_Projection);
	DDX_Control(pDX, IDC_UTMZONE, m_pcZone);
	DDX_Text(pDX, IDC_UTMZONE, m_iZone);
	DDV_MinMaxInt(pDX, m_iZone, -1, 30);
	DDX_CBIndex(pDX, IDC_PROJECTION, m_iProjection);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProjectionDlg, CDialog)
	//{{AFX_MSG_MAP(CProjectionDlg)
	ON_CBN_SELCHANGE(IDC_PROJECTION, OnSelchangeProjection)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProjectionDlg message handlers

BOOL CProjectionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_pcZone.EnableWindow(TRUE);
	m_pcZone.SetSel(0, -1);
	m_Projection.SetFocus();

	m_Projection.SetCurSel(0);

	return FALSE;  // return TRUE unless you set the focus to a control
}

void CProjectionDlg::OnSelchangeProjection()
{
	int iCurSel = m_Projection.GetCurSel();

	switch (iCurSel)
	{
	case 0: // UTM
		m_pcZone.EnableWindow(TRUE);
		break;
	default:
		m_pcZone.EnableWindow(FALSE);
		break;
	}
}
