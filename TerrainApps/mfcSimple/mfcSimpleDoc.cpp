// mfcSimpleDoc.cpp : implementation of the CSimpleDoc class
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "stdafx.h"
#include "mfcSimple.h"
#include "mfcSimpleDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSimpleDoc

IMPLEMENT_DYNCREATE(CSimpleDoc, CDocument)

BEGIN_MESSAGE_MAP(CSimpleDoc, CDocument)
	//{{AFX_MSG_MAP(CSimpleDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSimpleDoc construction/destruction

CSimpleDoc::CSimpleDoc()
{
}

CSimpleDoc::~CSimpleDoc()
{
}

BOOL CSimpleDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CSimpleDoc serialization

void CSimpleDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSimpleDoc diagnostics

#ifdef _DEBUG
void CSimpleDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSimpleDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSimpleDoc commands
