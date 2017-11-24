//
// BExtractorDoc.h : interface of the BExtractorDoc class
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#if !defined(B_EXTRACTORDOC_H)
#define B_EXTRACTORDOC_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "vtdata/StructArray.h"
#include "vtdata/Roadmap.h"
#include "Dib.h"

class CBImage;
class BExtractorView;

class BExtractorDoc : public CDocument
{
protected: // create from serialization only
	BExtractorDoc();
	DECLARE_DYNCREATE(BExtractorDoc)

// Attributes
public:
	HDRAWDIB	m_hdd;
	CBImage		*m_pImage;
	bool		m_picLoaded;
	vtStructureArray	m_Buildings;
	vtRoadMap	m_Links;
	float		m_fScale;

private:
	CString		m_roadFileName;

public:
	const vtProjection &GetProjection() { return m_proj; }
	void OnImportimage2(LPCTSTR szPathName);

	void PreFloodFillDIB(CDib *bm);
	void FloodFillDIB(CDib *bm, class CProgressDlg *prog);
	int Fill(CDib *bm, int x, int y, byte ov, byte nv, int w, int h, int diag, int count = 0);

	// transform image space -> UTM space
	double i_UTMx(int ix);
	double i_UTMy(int iy);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(BExtractorDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR szPathName);
	virtual BOOL OnSaveDocument(LPCTSTR szPathName);
	//}}AFX_VIRTUAL
	void DeleteContents();

// Implementation
public:
	virtual ~BExtractorDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	BExtractorView *GetView();

protected:
	vtProjection m_proj;

// Generated message map functions
protected:
	//{{AFX_MSG(BExtractorDoc)
	afx_msg void OnImportimage();
	afx_msg void OnFunctionsClearbuildings();
	afx_msg void OnBcfcombine();
	afx_msg void OnUpdateBcfcombine(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileOpen(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFullres(CCmdUI* pCmdUI);
	afx_msg void OnRmfOpen();
	afx_msg void OnRmfSave();
	afx_msg void OnRmfSaveAs();
	afx_msg void OnUpdateRmfOpen(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(B_EXTRACTORDOC_H)
