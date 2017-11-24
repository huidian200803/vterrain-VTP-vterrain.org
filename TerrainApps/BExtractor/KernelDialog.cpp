//
// KernelDialog.cpp : implementation file
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "StdAfx.h"
#include "BExtractor.h"
#include "KernelDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKernelDialog dialog


CKernelDialog::CKernelDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CKernelDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CKernelDialog)
	m_iSelection = -1;
	//}}AFX_DATA_INIT
}


void CKernelDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CKernelDialog)
	DDX_Radio(pDX, IDC_KERNEL1, m_iSelection);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CKernelDialog, CDialog)
	//{{AFX_MSG_MAP(CKernelDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKernelDialog message handlers
