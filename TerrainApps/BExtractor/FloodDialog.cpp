//
// FloodDialog.cpp : implementation file
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "StdAfx.h"
#include "BExtractor.h"
#include "FloodDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFloodDialog dialog


CFloodDialog::CFloodDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CFloodDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFloodDialog)
	m_iSelection = -1;
	//}}AFX_DATA_INIT
}


void CFloodDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFloodDialog)
	DDX_Radio(pDX, IDC_RADIO1, m_iSelection);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFloodDialog, CDialog)
	//{{AFX_MSG_MAP(CFloodDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFloodDialog message handlers
