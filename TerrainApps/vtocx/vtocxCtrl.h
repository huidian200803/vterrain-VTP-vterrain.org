//
// vtocxCtrl.h
//

#pragma once

#include "SrcProperty.h"
#include "GLThread.h"

#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>
#include <osg/FrameStamp>
#include <osg/Timer>

#include "Capture.h"

class vtTerrainFlyer;
class vtTerrainScene;
class vtHeightConstrain;

//
// CvtocxCtrl : See vtocxCtrl.cpp for implementation.
//
class CvtocxCtrl : public COleControl
{
	DECLARE_DYNCREATE(CvtocxCtrl)

// Constructor
public:
	CvtocxCtrl();

// Overrides
public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	virtual BOOL OnSetExtent(LPSIZEL lpSizeL);

// Implementation
protected:
	~CvtocxCtrl();

	DECLARE_OLECREATE_EX(CvtocxCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CvtocxCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CvtocxCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CvtocxCtrl)		// Type name and misc status

// Message maps
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

	bool SetupGLContext(void);
// Event maps
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
		dispidResetView = 31,
		dispidJumpToNo = 30,
		dispidContextMenu = 22,
		dispidNavSpeed = 21,
		dispidPositionZ = 20,
		dispidPositionY = 19,
		dispidPositionX = 18,
		dispidFullscreen = 16,
		dispidLocalSrc = 15,
		dispidSrc = 14,
		dispidTerrainPaths = 13,
		dispidTerrainFile = 12,
		dispidStereo = 2,
		dispidClearColor = 1
	};

	void frame(CDC *pdc = NULL);
	void resize(const long w, const long h);
	bool rendering() { return _rendering; }
#if CALL_VTP
	void Create3DScene();
	bool LoadTerrain(const char *terrain_file);
	void Cleanup3DScene();
	void AddTerrainPaths(const CString &terr_paths);
#endif
	void PrepareToRender(HDC dc);

	static CvtocxCtrl *s_pSingleton;
	void OnProgress(int pos) { m_Progress.SetPos(pos); }

private:
	HGLRC    m_hGLContext;		//Rendering context
	CDC* m_pDC;
	bool _ready;
	Capture *m_pCapture;

	CGLThread<CvtocxCtrl> * m_glThread;
	UINT_PTR m_nTimer;
	bool _rendering;
#if CALL_VTP
	vtTerrainScene *m_pTScene;
	vtTerrainFlyer *m_pFlyer;
	vtHeightConstrain *m_pConstrain;
	bool m_bKeyState[512];
#endif
	CProgressCtrl	m_Progress;

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL OnIdle(LONG lCount); // return TRUE if more idle processing

protected:
	void OnClearColorChanged(void);
	OLE_COLOR m_ClearColor;
	void OnStereoChanged(void);
	BOOL m_Stereo;
	void OnScreenChanged(void);

	BSTR GetTerrainFile(void);
	void SetTerrainFile(LPCTSTR newVal);
	CString m_TerrainFile;

	BSTR GetTerrainPaths(void);
	void SetTerrainPaths(LPCTSTR newVal);
	CString m_TerrainPaths;

	BSTR GetSrc(void);
	void SetSrc(LPCTSTR newVal);
	CSrcProperty m_Src;

	BSTR GetLocalSrc(void);
	void SetLocalSrc(LPCTSTR newVal);
	CString m_LocalSrc;

	void OnFullscreenChanged(void);
	BOOL m_Fullscreen;
	void OnPositionChanged(void);
	DOUBLE m_PositionX;
	DOUBLE m_PositionY;
	DOUBLE m_PositionZ;
public:
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
protected:
	void OnNavSpeedChanged(void);
	FLOAT m_NavSpeed;
	void JumpToNo(LONG vp);
	void ResetView();
	void OnContextMenuChanged(void);
	BOOL m_ContextMenu;
public:
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
};

