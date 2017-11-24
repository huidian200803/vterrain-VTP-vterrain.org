// vtocxFullScreen.cpp : implementation file
//

#include "stdafx.h"
#include "vtocx.h"
#include "vtocxFullScreen.h"
#include "GLThread.h"


// vtocxFullScreen dialog

IMPLEMENT_DYNAMIC(vtocxFullScreen, CDialog)
vtocxFullScreen::vtocxFullScreen(CWnd* pParent /*=NULL*/)
	: CDialog(vtocxFullScreen::IDD, pParent)
{
}

vtocxFullScreen::~vtocxFullScreen()
{
}

void vtocxFullScreen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(vtocxFullScreen, CDialog)
END_MESSAGE_MAP()


// vtocxFullScreen message handlers
