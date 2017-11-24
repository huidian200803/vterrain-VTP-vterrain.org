//
// KernelDialog.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#if !defined(AFX_KERNELDIALOG_H__A1B39CB5_5421_11D3_8196_009027076B56__INCLUDED_)
#define AFX_KERNELDIALOG_H__A1B39CB5_5421_11D3_8196_009027076B56__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// KernelDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CKernelDialog dialog

class CKernelDialog : public CDialog
{
// Construction
public:
	CKernelDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CKernelDialog)
	enum { IDD = IDD_KERNEL };
	int		m_iSelection;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKernelDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CKernelDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KERNELDIALOG_H__A1B39CB5_5421_11D3_8196_009027076B56__INCLUDED_)
