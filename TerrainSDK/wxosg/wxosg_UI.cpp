///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  8 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#include "wxosg_UI.h"

///////////////////////////////////////////////////////////////////////////

TimeDlgBase::TimeDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer185;
	bSizer185 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer186;
	bSizer186 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text111 = new wxStaticText( this, ID_TEXT, _("Year:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text111->Wrap( 0 );
	bSizer186->Add( m_text111, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_spin_year = new wxSpinCtrl( this, ID_SPIN_YEAR, wxT("2000"), wxDefaultPosition, wxSize( 60,-1 ), wxSP_ARROW_KEYS, 1970, 2038, 2000 );
	bSizer186->Add( m_spin_year, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text112 = new wxStaticText( this, ID_TEXT, _("Month"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text112->Wrap( 0 );
	bSizer186->Add( m_text112, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_spin_month = new wxSpinCtrl( this, ID_SPIN_MONTH, wxT("1"), wxDefaultPosition, wxSize( 60,-1 ), wxSP_ARROW_KEYS, 1, 12, 1 );
	bSizer186->Add( m_spin_month, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text113 = new wxStaticText( this, ID_TEXT, _("Day"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text113->Wrap( 0 );
	bSizer186->Add( m_text113, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_spin_day = new wxSpinCtrl( this, ID_SPIN_DAY, wxT("1"), wxDefaultPosition, wxSize( 60,-1 ), wxSP_ARROW_KEYS, 1, 32, 1 );
	bSizer186->Add( m_spin_day, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer185->Add( bSizer186, 0, wxALIGN_CENTER|wxALL, 0 );
	
	wxBoxSizer* bSizer187;
	bSizer187 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text114 = new wxStaticText( this, ID_TEXT, _("Hour:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text114->Wrap( 0 );
	bSizer187->Add( m_text114, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_spin_hour = new wxSpinCtrl( this, ID_SPIN_HOUR, wxT("0"), wxDefaultPosition, wxSize( 60,-1 ), wxSP_ARROW_KEYS, 0, 24, 0 );
	bSizer187->Add( m_spin_hour, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text115 = new wxStaticText( this, ID_TEXT, _("Minute"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text115->Wrap( 0 );
	bSizer187->Add( m_text115, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_spin_minute = new wxSpinCtrl( this, ID_SPIN_MINUTE, wxT("0"), wxDefaultPosition, wxSize( 60,-1 ), wxSP_ARROW_KEYS, 0, 60, 0 );
	bSizer187->Add( m_spin_minute, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text116 = new wxStaticText( this, ID_TEXT, _("Second"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text116->Wrap( 0 );
	bSizer187->Add( m_text116, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_spin_second = new wxSpinCtrl( this, ID_SPIN_SECOND, wxT("0"), wxDefaultPosition, wxSize( 60,-1 ), wxSP_ARROW_KEYS, 0, 60, 0 );
	bSizer187->Add( m_spin_second, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer185->Add( bSizer187, 0, wxALIGN_CENTER|wxALL, 0 );
	
	wxBoxSizer* bSizer188;
	bSizer188 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text117 = new wxStaticText( this, ID_TEXT, _("Speed:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text117->Wrap( 0 );
	bSizer188->Add( m_text117, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text_speed = new wxTextCtrl( this, ID_TEXT_SPEED, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	bSizer188->Add( m_text_speed, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_slider_speed = new wxSlider( this, ID_SLIDER_SPEED, 0, 0, 100, wxDefaultPosition, wxSize( 140,-1 ), wxSL_HORIZONTAL );
	bSizer188->Add( m_slider_speed, 0, wxALIGN_CENTER, 5 );
	
	m_stop = new wxButton( this, ID_BUTTON_STOP, _("Stop"), wxDefaultPosition, wxSize( 70,-1 ), 0 );
	m_stop->SetDefault(); 
	bSizer188->Add( m_stop, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer185->Add( bSizer188, 0, wxALIGN_CENTER|wxALL, 0 );
	
	this->SetSizer( bSizer185 );
	this->Layout();
	bSizer185->Fit( this );
	
	this->Centre( wxBOTH );
}

TimeDlgBase::~TimeDlgBase()
{
}

SceneGraphDlgBase::SceneGraphDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxVERTICAL );
	
	m_scenetree = new wxTreeCtrl( this, ID_SCENETREE, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS|wxSUNKEN_BORDER );
	bSizer11->Add( m_scenetree, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxHORIZONTAL );
	
	m_enabled = new wxCheckBox( this, ID_ENABLED, _("Enabled"), wxDefaultPosition, wxDefaultSize, 0 );
	m_enabled->SetValue(true); 
	bSizer12->Add( m_enabled, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_zoomto = new wxButton( this, ID_ZOOMTO, _("Zoom To"), wxDefaultPosition, wxDefaultSize, 0 );
	m_zoomto->SetDefault(); 
	bSizer12->Add( m_zoomto, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_refresh = new wxButton( this, ID_REFRESH, _("Refresh"), wxDefaultPosition, wxDefaultSize, 0 );
	m_refresh->SetDefault(); 
	bSizer12->Add( m_refresh, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_log = new wxButton( this, ID_LOG, _("Log"), wxDefaultPosition, wxDefaultSize, 0 );
	m_log->SetDefault(); 
	bSizer12->Add( m_log, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer11->Add( bSizer12, 0, wxALIGN_CENTER|wxALL, 0 );
	
	this->SetSizer( bSizer11 );
	this->Layout();
	bSizer11->Fit( this );
	
	this->Centre( wxBOTH );
}

SceneGraphDlgBase::~SceneGraphDlgBase()
{
}
