///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  8 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __wxosg_UI__
#define __wxosg_UI__

#include <wx/intl.h>

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/spinctrl.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/slider.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/treectrl.h>
#include <wx/checkbox.h>

///////////////////////////////////////////////////////////////////////////

#define ID_TEXT 1000
#define ID_SPIN_YEAR 1001
#define ID_SPIN_MONTH 1002
#define ID_SPIN_DAY 1003
#define ID_SPIN_HOUR 1004
#define ID_SPIN_MINUTE 1005
#define ID_SPIN_SECOND 1006
#define ID_TEXT_SPEED 1007
#define ID_SLIDER_SPEED 1008
#define ID_BUTTON_STOP 1009
#define ID_SCENETREE 1010
#define ID_ENABLED 1011
#define ID_ZOOMTO 1012
#define ID_REFRESH 1013
#define ID_LOG 1014

///////////////////////////////////////////////////////////////////////////////
/// Class TimeDlgBase
///////////////////////////////////////////////////////////////////////////////
class TimeDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text111;
		wxSpinCtrl* m_spin_year;
		wxStaticText* m_text112;
		wxSpinCtrl* m_spin_month;
		wxStaticText* m_text113;
		wxSpinCtrl* m_spin_day;
		wxStaticText* m_text114;
		wxSpinCtrl* m_spin_hour;
		wxStaticText* m_text115;
		wxSpinCtrl* m_spin_minute;
		wxStaticText* m_text116;
		wxSpinCtrl* m_spin_second;
		wxStaticText* m_text117;
		wxTextCtrl* m_text_speed;
		wxSlider* m_slider_speed;
		wxButton* m_stop;
	
	public:
		
		TimeDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL );
		~TimeDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class SceneGraphDlgBase
///////////////////////////////////////////////////////////////////////////////
class SceneGraphDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxTreeCtrl* m_scenetree;
		wxCheckBox* m_enabled;
		wxButton* m_zoomto;
		wxButton* m_refresh;
		wxButton* m_log;
	
	public:
		
		SceneGraphDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxTAB_TRAVERSAL );
		~SceneGraphDlgBase();
	
};

#endif //__wxosg_UI__
