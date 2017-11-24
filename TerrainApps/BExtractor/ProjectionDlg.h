//
// ProjectionDlg.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#if !defined(AFX_PROJECTIONDLG_H__32ADECCD_44EB_450B_99E2_951275987AB1__INCLUDED_)
#define AFX_PROJECTIONDLG_H__32ADECCD_44EB_450B_99E2_951275987AB1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProjectionDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProjectionDlg dialog

class CProjectionDlg : public CDialog
{
// Construction
public:
	CProjectionDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CProjectionDlg)
	enum { IDD = IDD_ZONEDLG };
	CComboBox	m_Projection;
	CEdit	m_pcZone;
	int		m_iZone;
	int		m_iProjection;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProjectionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CProjectionDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeProjection();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROJECTIONDLG_H__32ADECCD_44EB_450B_99E2_951275987AB1__INCLUDED_)
