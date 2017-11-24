#pragma once


// vtocxFullScreen dialog

class vtocxFullScreen : public CDialog
{
	DECLARE_DYNAMIC(vtocxFullScreen)

public:
	vtocxFullScreen(CWnd* pParent = NULL);   // standard constructor
	virtual ~vtocxFullScreen();

// Dialog Data
	enum { IDD = IDD_FULLSCREEN_VTOCX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
