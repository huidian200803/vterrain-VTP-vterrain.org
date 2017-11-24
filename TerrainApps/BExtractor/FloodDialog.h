//
// FloodDialog.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#if !defined(AFX_FLOODDIALOG_H__A1B39CB6_5421_11D3_8196_009027076B56__INCLUDED_)
#define AFX_FLOODDIALOG_H__A1B39CB6_5421_11D3_8196_009027076B56__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FloodDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFloodDialog dialog

class CFloodDialog : public CDialog
{
// Construction
public:
	CFloodDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFloodDialog)
	enum { IDD = IDD_PREFLOOD };
	int		m_iSelection;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFloodDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFloodDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FLOODDIALOG_H__A1B39CB6_5421_11D3_8196_009027076B56__INCLUDED_)
