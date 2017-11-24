/////////////////////////////////////////////////////////////////////////////
// Name:        VIAGDALOptionsDlg.h
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifndef __VIAGDALOptionsDlg_H__
#define __VIAGDALOptionsDlg_H__

#ifdef __GNUG__
    #pragma interface "VIAGDALOptionsDlg.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "EnviroUI.h"

//----------------------------------------------------------------------------
// CVIAGDALOptionsDlg
//----------------------------------------------------------------------------

class CVIAGDALOptionsDlg: public VIAGDALOptionsDlgBase
{
public:
    // constructors and destructors
    CVIAGDALOptionsDlg(wxWindow *parent);

    void Setup(int iDriverIndex);

};




#endif
