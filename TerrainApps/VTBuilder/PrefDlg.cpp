//
// Name: PrefDlg.cpp
//
// Copyright (c) 2007-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "PrefDlg.h"
#include "vtui/AutoDialog.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// PrefDlg
//----------------------------------------------------------------------------

// WDR: event table for PrefDlg

BEGIN_EVENT_TABLE(PrefDlg, PrefDlgBase)
	EVT_INIT_DIALOG (PrefDlg::OnInitDialog)
	EVT_BUTTON( wxID_OK, PrefDlg::OnOK )
	EVT_RADIOBUTTON( ID_RADIO1, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO2, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO3, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO4, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO5, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO6, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO7, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO8, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO9, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO10, PrefDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO11, PrefDlg::OnRadio )
	EVT_CHECKBOX( ID_BLACK_TRANSP, PrefDlg::OnCheck )
	EVT_CHECKBOX( ID_DEFLATE_TIFF, PrefDlg::OnCheck )
	EVT_CHECKBOX( ID_DELAY_LOAD, PrefDlg::OnCheck )
END_EVENT_TABLE()

PrefDlg::PrefDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	PrefDlgBase( parent, id, title, position, size, style )
{
	AddValidator(this, ID_RADIO1, &b1);
	AddValidator(this, ID_RADIO2, &b2);
	AddValidator(this, ID_RADIO3, &b3);
	AddValidator(this, ID_RADIO4, &b4);
	AddValidator(this, ID_RADIO5, &b5);
	AddValidator(this, ID_RADIO6, &b6);
	AddValidator(this, ID_RADIO7, &b7);
	AddValidator(this, ID_RADIO8, &b8);
	AddValidator(this, ID_RADIO9, &b9);
	AddValidator(this, ID_RADIO10, &b10);
	AddValidator(this, ID_RADIO11, &b11);
	AddValidator(this, ID_BLACK_TRANSP, &b12);
	AddValidator(this, ID_DEFLATE_TIFF, &b13);
	AddValidator(this, ID_BT_GZIP, &b14);
	AddValidator(this, ID_DELAY_LOAD, &b15);
	AddNumValidator(this, ID_SAMPLING_N, &i1);
	AddNumValidator(this, ID_MAX_MEGAPIXELS, &i2);
	AddNumValidator(this, ID_ELEV_MAX_SIZE, &i3);
	AddNumValidator(this, ID_MAX_MEM_GRID, &i4);

	GetSizer()->SetSizeHints(this);
}

void PrefDlg::UpdateEnable()
{
	FindWindow(ID_MAX_MEM_GRID)->Enable(b15);
}


// WDR: handler implementations for PrefDlg

void PrefDlg::OnInitDialog(wxInitDialogEvent& event)
{
	UpdateEnable();
}

void PrefDlg::OnRadio( wxCommandEvent &event )
{
	TransferDataFromWindow();
}

void PrefDlg::OnCheck( wxCommandEvent &event )
{
	TransferDataFromWindow();

	UpdateEnable();
}

void PrefDlg::OnOK( wxCommandEvent &event )
{
	TransferDataFromWindow();
	event.Skip();
}
