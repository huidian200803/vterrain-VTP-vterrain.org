//
// BExtractor.h : main header file for the B_EXTRACTOR application
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#if !defined(AFX_B_EXTRACTOR_H__5C7EF02F_1F80_11D3_8172_00A0C96173FD__INCLUDED_)
#define AFX_B_EXTRACTOR_H__5C7EF02F_1F80_11D3_8172_00A0C96173FD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"	// main symbols

#define UTM_ERROR 6

/////////////////////////////////////////////////////////////////////////////
// CBExtractorApp:
// See BExtractor.cpp for the implementation of this class
//

class CBExtractorApp : public CWinApp
{
public:
	CBExtractorApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBExtractorApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CBExtractorApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//	DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_B_EXTRACTOR_H__5C7EF02F_1F80_11D3_8172_00A0C96173FD__INCLUDED_)
