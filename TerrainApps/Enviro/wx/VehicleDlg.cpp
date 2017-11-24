//
// Name: VehicleDlg.cpp
//
// Copyright (c) 2006-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/colordlg.h>

#include "vtlib/vtlib.h"
#include "vtui/Helper.h"				// for FillWithColor

#include "VehicleDlg.h"
#include "EnviroGUI.h"		// for g_App

extern void EnableContinuousRendering(bool bTrue);

// WDR: class implementations

//----------------------------------------------------------------------------
// VehicleDlg
//----------------------------------------------------------------------------

// WDR: event table for VehicleDlg

BEGIN_EVENT_TABLE(VehicleDlg,VehicleDlgBase)
	EVT_INIT_DIALOG (VehicleDlg::OnInitDialog)
	EVT_BUTTON( ID_SET_VEHICLE_COLOR, VehicleDlg::OnSetColor )
	EVT_CHOICE( ID_CHOICE_VEHICLES, VehicleDlg::OnChoiceVehicle )
END_EVENT_TABLE()

VehicleDlg::VehicleDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	VehicleDlgBase( parent, id, title, position, size, style )
{
	m_options.m_Color.Set(255, 255, 0);
	wxColour rgb(255, 255, 0);
	m_ColorData.SetChooseFull(true);
	m_ColorData.SetColour(rgb);

	// Work around wxFormDesigner's lack of support for limiting to smallest size
	GetSizer()->SetSizeHints(this);
}

void VehicleDlg::UpdateEnabling()
{
	vtContentManager3d &con = vtGetContent();

	GetSetColor()->Enable(false);
	vtItem *item = con.FindItemByName(m_options.m_Itemname);
	if (item)
	{
		if (item->GetValueBool("colorable"))
			GetSetColor()->Enable(true);
	}
}

void VehicleDlg::ItemChanged()
{
	m_options.m_Itemname = (const char *) GetChoice()->GetStringSelection().mb_str(wxConvUTF8);
	g_App.SetVehicleOptions(m_options);
}

void VehicleDlg::UpdateColorControl()
{
	// Case of a single edge, very simple.
	FillWithColorSize(GetColor(), 32, 18, m_options.m_Color);
}


// WDR: handler implementations for VehicleDlg

void VehicleDlg::OnChoiceVehicle( wxCommandEvent &event )
{
	ItemChanged();
	UpdateEnabling();
}


void VehicleDlg::OnSetColor( wxCommandEvent &event )
{
	EnableContinuousRendering(false);

	wxColourDialog dlg(this, &m_ColorData);
	if (dlg.ShowModal() == wxID_OK)
	{
		// Get the color from the dialog
		m_ColorData = dlg.GetColourData();
		wxColour rgb = m_ColorData.GetColour();
		m_options.m_Color.Set(rgb.Red(), rgb.Green(), rgb.Blue());

		UpdateColorControl();
		g_App.SetVehicleOptions(m_options);
	}
	EnableContinuousRendering(true);
}

void VehicleDlg::OnInitDialog(wxInitDialogEvent& event)
{
	GetChoice()->Clear();

	// Refresh vehicle list
	wxString prev = GetChoice()->GetStringSelection();
	wxArrayString names;
	vtContentManager3d &con = vtGetContent();
	for (uint i = 0; i < con.NumItems(); i++)
	{
		vtItem *item = con.GetItem(i);
		const char *type = item->GetValueString("type");
		int wheels = item->GetValueInt("num_wheels");
		if (type && vtString(type) == "ground vehicle" && wheels == 4)
		{
			GetChoice()->Append(wxString(item->m_name, wxConvUTF8));
		}
	}
	if (prev != _T(""))
		GetChoice()->SetStringSelection(prev);
	else
		GetChoice()->SetSelection(0);

	ItemChanged();
	UpdateColorControl();
	UpdateEnabling();

	wxWindow::OnInitDialog(event);
}

