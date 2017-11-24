//
// Name:        VIADlg.h
//
// Copyright (c) Beard and Sandals Ltd.
// Free for all uses, see license.txt for details.
//

#ifndef __VIADlg_H__
#define __VIADlg_H__

#ifdef __GNUG__
    #pragma interface "VIADlg.cpp"
#endif

#include "EnviroUI.h"

//----------------------------------------------------------------------------
// VIADlg
//----------------------------------------------------------------------------

class VIADlg: public VIADlgBase
{
public:
    // constructors and destructors
    VIADlg(wxWindow *parent) : VIADlgBase(parent) {}

private:
	void OnInitDialog(wxInitDialogEvent& event);
	void OnRecalculate(wxCommandEvent &event);
};




#endif
