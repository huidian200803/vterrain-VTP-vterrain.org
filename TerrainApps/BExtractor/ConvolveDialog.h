//
// ConvolveDialog.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#if !defined(AFX_CONVOLVEDIALOG_H__6EC309F2_4A95_11D3_818E_009027076B56__INCLUDED_)
#define AFX_CONVOLVEDIALOG_H__6EC309F2_4A95_11D3_818E_009027076B56__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConvolveDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConvolveDialog dialog

class CConvolveDialog : public CDialog
{
// Construction
public:
	CConvolveDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConvolveDialog)
	enum { IDD = IDD_THRESH_DIALOG };
	int		m_iSelection;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConvolveDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConvolveDialog)

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONVOLVEDIALOG_H__6EC309F2_4A95_11D3_818E_009027076B56__INCLUDED_)
