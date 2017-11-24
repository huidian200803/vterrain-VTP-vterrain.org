//
// Name: UtilDlg.cpp
//
// Copyright (c) 2002-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "UtilDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "vtlib/vtlib.h"
#include "vtlib/core/UtilityMap3d.h"
#include "UtilDlg.h"
#include "EnviroGUI.h"
#include "vtdata/vtLog.h"


// WDR: class implementations

//----------------------------------------------------------------------------
// UtilDlg
//----------------------------------------------------------------------------

// WDR: event table for UtilDlg

BEGIN_EVENT_TABLE(UtilDlg,UtilDlgBase)
	EVT_INIT_DIALOG (UtilDlg::OnInitDialog)
	EVT_CHOICE( ID_STRUCTTYPE, UtilDlg::OnStructType )
END_EVENT_TABLE()

UtilDlg::UtilDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	UtilDlgBase( parent, id, title, position, size, style )
{
	m_pChoice = GetStructtype();
	m_iType = 0;

	AddValidator(this, ID_STRUCTTYPE, &m_iType);

	GetSizer()->SetSizeHints(this);
}

// WDR: handler implementations for UtilDlg

void UtilDlg::OnStructType( wxCommandEvent &event )
{
	TransferDataFromWindow();
	wxString val = m_pChoice->GetStringSelection();
	g_App.SetPowerOptions((const char *) val.mb_str(wxConvUTF8));
}

void UtilDlg::OnInitDialog(wxInitDialogEvent& event)
{
	VTLOG("UtilDlg looking for items of type utility pole.\n");
	vtContentManager &mng = vtGetContent();

	int found = 0;
	m_pChoice->Clear();
	for (uint i = 0; i < mng.NumItems(); i++)
	{
		vtString str;
		vtItem *item = mng.GetItem(i);
		if (item->GetValueString("type", str))
		{
			if (str == "utility pole")
			{
				m_pChoice->Append(wxString::FromAscii(item->m_name));
				found++;
			}
		}
	}
	TransferDataToWindow();

	VTLOG("\t%d items, %d found.\n", mng.NumItems(), found);

	wxString val = m_pChoice->GetStringSelection();
	g_App.SetPowerOptions((const char *) val.mb_str(wxConvUTF8));
}

