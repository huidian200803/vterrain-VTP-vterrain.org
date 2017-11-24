//
// Name: VehicleDlg.h
//
// Copyright (c) 2006-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __VehicleDlg_H__
#define __VehicleDlg_H__

#if wxCHECK_VERSION(2, 9, 0)
  #include <wx/colourdata.h>
#else
  // The older include
  #include <wx/cmndata.h>
#endif

#include "EnviroUI.h"
#include "../PlantingOptions.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// VehicleDlg
//----------------------------------------------------------------------------

class VehicleDlg: public VehicleDlgBase
{
public:
	// constructors and destructors
	VehicleDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void UpdateColorControl();
	void UpdateEnabling();
	void ItemChanged();

	// public attributes
	VehicleOptions m_options;

	// WDR: method declarations for VehicleDlg
	wxButton* GetSetColor()  { return (wxButton*) FindWindow( ID_SET_VEHICLE_COLOR ); }
	wxStaticBitmap* GetColor()  { return (wxStaticBitmap*) FindWindow( ID_COLOR3 ); }
	wxChoice* GetChoice()  { return (wxChoice*) FindWindow( ID_CHOICE_VEHICLES ); }

private:
	// WDR: member variable declarations for VehicleDlg
	wxColourData m_ColorData;

private:
	// WDR: handler declarations for VehicleDlg
	void OnChoiceVehicle( wxCommandEvent &event );
	void OnSetColor( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __VehicleDlg_H__

