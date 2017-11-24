//
// BExtractor.cpp : Defines the class behaviors for the application.
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "StdAfx.h"
#include "BExtractor.h"

#include "MainFrm.h"
#include "BExtractorDoc.h"
#include "BExtractorView.h"
#include "vtdata/vtLog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBExtractorApp

BEGIN_MESSAGE_MAP(CBExtractorApp, CWinApp)
	//{{AFX_MSG_MAP(CBExtractorApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//	DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBExtractorApp construction

CBExtractorApp::CBExtractorApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CBExtractorApp object

CBExtractorApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CBExtractorApp initialization

BOOL CBExtractorApp::InitInstance()
{
	VTSTARTLOG("debug.txt");
	VTLOG("BExtractor\nBuild:");
#if VTDEBUG
	VTLOG(" Debug");
#else
	VTLOG(" Release");
#endif
	VTLOG("\n");

	// Try to guess GDAL and PROJ.4 data paths, in case the user doesn't have
	//  their GDAL_DATA and PROJ_LIB environment variables set.
	g_GDALWrapper.Init();

	AfxEnableControlContainer();

	// Change the registry key under which our settings are stored.
	// This string should be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Virtual Terrain Project"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(BExtractorDoc),
		RUNTIME_CLASS(CMainFrame),		// main SDI frame window
		RUNTIME_CLASS(BExtractorView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	m_pMainWnd->DragAcceptFiles(TRUE);

#if 0
	// Test code
	POSITION pos = pDocTemplate->GetFirstDocPosition();
	BExtractorDoc *doc = (BExtractorDoc *) pDocTemplate->GetNextDoc(pos);
	doc->OnImportimage2("E:/Locations-USA/MD/Annapolis/o38076h4.tif");
#endif

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CString	m_strDate;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	m_strDate = _T("");
	//}}AFX_DATA_INIT
	m_strDate = __DATE__;
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Text(pDX, IDC_DATE, m_strDate);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CBExtractorApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CBExtractorApp commands

