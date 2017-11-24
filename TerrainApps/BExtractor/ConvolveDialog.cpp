//
// ConvolveDialog.cpp : implementation file
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "StdAfx.h"
#include "BExtractor.h"
#include "ConvolveDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConvolveDialog dialog


CConvolveDialog::CConvolveDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CConvolveDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConvolveDialog)
	m_iSelection = -1;
	//}}AFX_DATA_INIT
}


void CConvolveDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConvolveDialog)
	DDX_Radio(pDX, IDC_Thresh1, m_iSelection);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConvolveDialog, CDialog)
	//{{AFX_MSG_MAP(CConvolveDialog)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConvolveDialog message handlers

