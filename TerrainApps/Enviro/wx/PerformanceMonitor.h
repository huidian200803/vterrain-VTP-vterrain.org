/////////////////////////////////////////////////////////////////////////////
// Name:        PerformanceMonitor.h
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifndef __PerformanceMonitor_H__
#define __PerformanceMonitor_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "PerformanceMonitor.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "EnviroUI.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// CPerformanceMonitorDialog
//----------------------------------------------------------------------------

class CPerformanceMonitorDialog: public PerformanceMonitorDlgBase
{
public:
    // constructors and destructors
    CPerformanceMonitorDialog( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );

    // WDR: method declarations for CPerformanceMonitorDailog
    wxListCtrl* GetPmListctrl()  { return (wxListCtrl*) FindWindow( ID_PM_LISTCTRL ); }

    // Static functions
    static void NVPM_init();
    static void NVPM_shutdown();
    static void NVPM_frame();

private:
    // WDR: member variable declarations for CPerformanceMonitorDialog

private:
    // WDR: handler declarations for CPerformanceMonitorDialog
    virtual void OnListItemRightClick( wxListEvent &event );

private:
    void UpdateCounters();
    static bool m_NVPMInitialised;
};

#endif
