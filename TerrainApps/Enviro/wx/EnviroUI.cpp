///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Nov 10 2014)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#include "EnviroUI.h"

#include "../../../TerrainSDK/vtui/bitmaps/dummy_32x18.xpm"
#include "bitmap/LOD128.xpm"
#include "bitmap/LOD16.xpm"
#include "bitmap/LOD1k.xpm"
#include "bitmap/LOD256.xpm"
#include "bitmap/LOD2k.xpm"
#include "bitmap/LOD32.xpm"
#include "bitmap/LOD512.xpm"
#include "bitmap/LOD64.xpm"
#include "bitmap/play_back.xpm"
#include "bitmap/play_play.xpm"
#include "bitmap/play_record1.xpm"
#include "bitmap/play_stop.xpm"

///////////////////////////////////////////////////////////////////////////

CameraDlgBase::CameraDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* CamSizerTop;
	CamSizerTop = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer105;
	bSizer105 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text51 = new wxStaticText( this, ID_TEXT, _("Position:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text51->Wrap( 0 );
	bSizer105->Add( m_text51, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_camx = new wxTextCtrl( this, ID_CAMX, wxEmptyString, wxDefaultPosition, wxSize( 120,-1 ), wxTE_PROCESS_ENTER );
	m_camx->SetMaxLength( 0 ); 
	bSizer105->Add( m_camx, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_camy = new wxTextCtrl( this, ID_CAMY, wxEmptyString, wxDefaultPosition, wxSize( 120,-1 ), wxTE_PROCESS_ENTER );
	m_camy->SetMaxLength( 0 ); 
	bSizer105->Add( m_camy, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	
	CamSizerTop->Add( bSizer105, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP, 5 );
	
	wxBoxSizer* bSizer106;
	bSizer106 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text52 = new wxStaticText( this, ID_TEXT, _("Elevation:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text52->Wrap( 0 );
	bSizer106->Add( m_text52, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_camz = new wxTextCtrl( this, ID_CAMZ, wxEmptyString, wxDefaultPosition, wxSize( 100,-1 ), wxTE_PROCESS_ENTER );
	m_camz->SetMaxLength( 0 ); 
	bSizer106->Add( m_camz, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	CamSizerTop->Add( bSizer106, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer107;
	bSizer107 = new wxBoxSizer( wxHORIZONTAL );
	
	m_fov_text = new wxStaticText( this, ID_FOV_TEXT, _("Horizontal FOV (degrees)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fov_text->Wrap( -1 );
	bSizer107->Add( m_fov_text, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_fov = new wxTextCtrl( this, ID_FOV, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_fov->SetMaxLength( 0 ); 
	bSizer107->Add( m_fov, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_fovslider = new wxSlider( this, ID_FOVSLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer107->Add( m_fovslider, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	CamSizerTop->Add( bSizer107, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP, 5 );
	
	wxBoxSizer* CamSizerNear;
	CamSizerNear = new wxBoxSizer( wxHORIZONTAL );
	
	m_text53 = new wxStaticText( this, ID_TEXT, _("Near Clipping Plane (meters):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text53->Wrap( -1 );
	CamSizerNear->Add( m_text53, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_near = new wxTextCtrl( this, ID_NEAR, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_near->SetMaxLength( 0 ); 
	CamSizerNear->Add( m_near, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_nearslider = new wxSlider( this, ID_NEARSLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	CamSizerNear->Add( m_nearslider, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	CamSizerTop->Add( CamSizerNear, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* CamSizerFar;
	CamSizerFar = new wxBoxSizer( wxHORIZONTAL );
	
	m_text54 = new wxStaticText( this, ID_TEXT, _("Far Clipping Plane (meters):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text54->Wrap( -1 );
	CamSizerFar->Add( m_text54, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_far = new wxTextCtrl( this, ID_FAR, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_far->SetMaxLength( 0 ); 
	CamSizerFar->Add( m_far, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_farslider = new wxSlider( this, ID_FARSLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	CamSizerFar->Add( m_farslider, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	CamSizerTop->Add( CamSizerFar, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer110;
	bSizer110 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text55 = new wxStaticText( this, ID_TEXT, _("Stereo Eye Separation:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text55->Wrap( -1 );
	bSizer110->Add( m_text55, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_eye_sep = new wxTextCtrl( this, ID_EYE_SEP, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_eye_sep->SetMaxLength( 0 ); 
	bSizer110->Add( m_eye_sep, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_eye_sepslider = new wxSlider( this, ID_EYE_SEPSLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer110->Add( m_eye_sepslider, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	CamSizerTop->Add( bSizer110, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer111;
	bSizer111 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text56 = new wxStaticText( this, ID_TEXT, _("Stereo Fusion Distance:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text56->Wrap( -1 );
	bSizer111->Add( m_text56, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_fusion_dist = new wxTextCtrl( this, ID_FUSION_DIST, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_fusion_dist->SetMaxLength( 0 ); 
	bSizer111->Add( m_fusion_dist, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_fusion_dist_slider = new wxSlider( this, ID_FUSION_DIST_SLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer111->Add( m_fusion_dist_slider, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	CamSizerTop->Add( bSizer111, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_line = new wxStaticLine( this, ID_LINE, wxDefaultPosition, wxSize( 20,-1 ), wxLI_HORIZONTAL );
	CamSizerTop->Add( m_line, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer112;
	bSizer112 = new wxBoxSizer( wxHORIZONTAL );
	
	id_text8 = new wxStaticText( this, ID_TEXT, _("Navigation Speed:"), wxDefaultPosition, wxDefaultSize, 0 );
	id_text8->Wrap( -1 );
	bSizer112->Add( id_text8, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_speed = new wxTextCtrl( this, ID_SPEED, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_speed->SetMaxLength( 0 ); 
	bSizer112->Add( m_speed, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_speedslider = new wxSlider( this, ID_SPEEDSLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer112->Add( m_speedslider, 0, wxALIGN_CENTER, 0 );
	
	
	CamSizerTop->Add( bSizer112, 0, wxALIGN_RIGHT, 5 );
	
	wxBoxSizer* bSizer113;
	bSizer113 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text57 = new wxStaticText( this, ID_TEXT, _("Units"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text57->Wrap( 0 );
	bSizer113->Add( m_text57, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_speed_unitsChoices;
	m_speed_units = new wxChoice( this, ID_SPEED_UNITS, wxDefaultPosition, wxSize( 100,-1 ), m_speed_unitsChoices, 0 );
	m_speed_units->SetSelection( 0 );
	bSizer113->Add( m_speed_units, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer113->Add( 55, 20, 0, wxALIGN_CENTER, 0 );
	
	
	CamSizerTop->Add( bSizer113, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer114;
	bSizer114 = new wxBoxSizer( wxHORIZONTAL );
	
	m_accel = new wxCheckBox( this, ID_ACCEL, _("Accelerate by height above ground"), wxDefaultPosition, wxDefaultSize, 0 );
	m_accel->SetValue(true); 
	bSizer114->Add( m_accel, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	CamSizerTop->Add( bSizer114, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer1121;
	bSizer1121 = new wxBoxSizer( wxHORIZONTAL );
	
	id_text81 = new wxStaticText( this, ID_TEXT, _("Velocity Damping:"), wxDefaultPosition, wxDefaultSize, 0 );
	id_text81->Wrap( -1 );
	bSizer1121->Add( id_text81, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_damping = new wxTextCtrl( this, ID_DAMPING, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_damping->SetMaxLength( 0 ); 
	bSizer1121->Add( m_damping, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_dampingslider = new wxSlider( this, ID_DAMPINGSLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer1121->Add( m_dampingslider, 0, wxALIGN_CENTER, 0 );
	
	
	CamSizerTop->Add( bSizer1121, 1, wxALIGN_RIGHT, 5 );
	
	wxStaticBoxSizer* CamSizerLOD;
	CamSizerLOD = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("LOD Distance") ), wxVERTICAL );
	
	wxBoxSizer* bSizer115;
	bSizer115 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text58 = new wxStaticText( CamSizerLOD->GetStaticBox(), ID_TEXT, _("Vegetation"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text58->Wrap( 0 );
	bSizer115->Add( m_text58, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_lod_veg = new wxTextCtrl( CamSizerLOD->GetStaticBox(), ID_LOD_VEG, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_lod_veg->SetMaxLength( 0 ); 
	bSizer115->Add( m_lod_veg, 0, wxALIGN_CENTER, 5 );
	
	m_slider_veg = new wxSlider( CamSizerLOD->GetStaticBox(), ID_SLIDER_VEG, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer115->Add( m_slider_veg, 0, wxALIGN_CENTER, 0 );
	
	
	CamSizerLOD->Add( bSizer115, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer116;
	bSizer116 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text59 = new wxStaticText( CamSizerLOD->GetStaticBox(), ID_TEXT, _("Structures"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text59->Wrap( 0 );
	bSizer116->Add( m_text59, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_lod_struct = new wxTextCtrl( CamSizerLOD->GetStaticBox(), ID_LOD_STRUCT, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_lod_struct->SetMaxLength( 0 ); 
	bSizer116->Add( m_lod_struct, 0, wxALIGN_CENTER, 5 );
	
	m_slider_struct = new wxSlider( CamSizerLOD->GetStaticBox(), ID_SLIDER_STRUCT, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer116->Add( m_slider_struct, 0, wxALIGN_CENTER, 0 );
	
	
	CamSizerLOD->Add( bSizer116, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer117;
	bSizer117 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text60 = new wxStaticText( CamSizerLOD->GetStaticBox(), ID_TEXT, _("Roads"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text60->Wrap( 0 );
	bSizer117->Add( m_text60, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_lod_road = new wxTextCtrl( CamSizerLOD->GetStaticBox(), ID_LOD_ROAD, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_lod_road->SetMaxLength( 0 ); 
	bSizer117->Add( m_lod_road, 0, wxALIGN_CENTER, 5 );
	
	m_slider_road = new wxSlider( CamSizerLOD->GetStaticBox(), ID_SLIDER_ROAD, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer117->Add( m_slider_road, 0, wxALIGN_CENTER, 0 );
	
	
	CamSizerLOD->Add( bSizer117, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	CamSizerTop->Add( CamSizerLOD, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	this->SetSizer( CamSizerTop );
	this->Layout();
	CamSizerTop->Fit( this );
	
	this->Centre( wxBOTH );
}

CameraDlgBase::~CameraDlgBase()
{
}

DriveDlgBase::DriveDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer210;
	bSizer210 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer7;
	fgSizer7 = new wxFlexGridSizer( 3, 4, 0, 0 );
	fgSizer7->SetFlexibleDirection( wxBOTH );
	fgSizer7->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText149 = new wxStaticText( this, wxID_ANY, _("Speed"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText149->Wrap( -1 );
	fgSizer7->Add( m_staticText149, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );
	
	m_speed = new wxTextCtrl( this, ID_SPEED, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_speed->SetMaxLength( 5 ); 
	m_speed->SetMaxSize( wxSize( 100,-1 ) );
	
	fgSizer7->Add( m_speed, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticText150 = new wxStaticText( this, wxID_ANY, _("kmph"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText150->Wrap( -1 );
	fgSizer7->Add( m_staticText150, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_follow = new wxCheckBox( this, wxID_ANY, _("Follow with camera"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer7->Add( m_follow, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticText1501 = new wxStaticText( this, wxID_ANY, _("Rate of turn"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1501->Wrap( -1 );
	fgSizer7->Add( m_staticText1501, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );
	
	m_turn = new wxTextCtrl( this, ID_TURN, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_turn->SetMaxLength( 5 ); 
	fgSizer7->Add( m_turn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_staticText1502 = new wxStaticText( this, wxID_ANY, _("radians"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1502->Wrap( -1 );
	fgSizer7->Add( m_staticText1502, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_slider = new wxSlider( this, ID_DISTANCE, 15, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	fgSizer7->Add( m_slider, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT|wxEXPAND, 5 );
	
	
	bSizer210->Add( fgSizer7, 0, wxALIGN_CENTER_HORIZONTAL, 5 );
	
	m_area = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	m_area->SetScrollRate( 5, 5 );
	m_area->SetMinSize( wxSize( 320,140 ) );
	m_area->SetMaxSize( wxSize( 320,140 ) );
	
	bSizer210->Add( m_area, 1, wxEXPAND | wxALL, 5 );
	
	
	this->SetSizer( bSizer210 );
	this->Layout();
	bSizer210->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_follow->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DriveDlgBase::OnFollow ), NULL, this );
	m_slider->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( DriveDlgBase::OnScroll ), NULL, this );
	m_slider->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( DriveDlgBase::OnScroll ), NULL, this );
	m_slider->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( DriveDlgBase::OnScroll ), NULL, this );
	m_slider->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( DriveDlgBase::OnScroll ), NULL, this );
	m_slider->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( DriveDlgBase::OnScroll ), NULL, this );
	m_slider->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( DriveDlgBase::OnScroll ), NULL, this );
	m_slider->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( DriveDlgBase::OnScroll ), NULL, this );
	m_slider->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( DriveDlgBase::OnScroll ), NULL, this );
	m_slider->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( DriveDlgBase::OnScroll ), NULL, this );
	m_area->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( DriveDlgBase::OnLeftDClick ), NULL, this );
	m_area->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( DriveDlgBase::OnLeftDown ), NULL, this );
	m_area->Connect( wxEVT_LEFT_UP, wxMouseEventHandler( DriveDlgBase::OnLeftUp ), NULL, this );
	m_area->Connect( wxEVT_MOTION, wxMouseEventHandler( DriveDlgBase::OnMotion ), NULL, this );
	m_area->Connect( wxEVT_PAINT, wxPaintEventHandler( DriveDlgBase::OnAreaPaint ), NULL, this );
}

DriveDlgBase::~DriveDlgBase()
{
	// Disconnect Events
	m_follow->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DriveDlgBase::OnFollow ), NULL, this );
	m_slider->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( DriveDlgBase::OnScroll ), NULL, this );
	m_slider->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( DriveDlgBase::OnScroll ), NULL, this );
	m_slider->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( DriveDlgBase::OnScroll ), NULL, this );
	m_slider->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( DriveDlgBase::OnScroll ), NULL, this );
	m_slider->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( DriveDlgBase::OnScroll ), NULL, this );
	m_slider->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( DriveDlgBase::OnScroll ), NULL, this );
	m_slider->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( DriveDlgBase::OnScroll ), NULL, this );
	m_slider->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( DriveDlgBase::OnScroll ), NULL, this );
	m_slider->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( DriveDlgBase::OnScroll ), NULL, this );
	m_area->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( DriveDlgBase::OnLeftDClick ), NULL, this );
	m_area->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( DriveDlgBase::OnLeftDown ), NULL, this );
	m_area->Disconnect( wxEVT_LEFT_UP, wxMouseEventHandler( DriveDlgBase::OnLeftUp ), NULL, this );
	m_area->Disconnect( wxEVT_MOTION, wxMouseEventHandler( DriveDlgBase::OnMotion ), NULL, this );
	m_area->Disconnect( wxEVT_PAINT, wxPaintEventHandler( DriveDlgBase::OnAreaPaint ), NULL, this );
	
}

EphemDlgBase::EphemDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer297;
	bSizer297 = new wxBoxSizer( wxVERTICAL );
	
	m_oceanplane = new wxCheckBox( this, ID_OCEANPLANE, _("Ocean plane"), wxDefaultPosition, wxDefaultSize, 0 );
	m_oceanplane->SetValue(true); 
	bSizer297->Add( m_oceanplane, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer298;
	bSizer298 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer298->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_text204 = new wxStaticText( this, ID_TEXT, _("Level (meters)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text204->Wrap( -1 );
	bSizer298->Add( m_text204, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_oceanplaneoffset = new wxTextCtrl( this, ID_OCEANPLANEOFFSET, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_oceanplaneoffset->SetMaxLength( 0 ); 
	bSizer298->Add( m_oceanplaneoffset, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer297->Add( bSizer298, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_sky = new wxCheckBox( this, ID_SKY, _("Sky dome"), wxDefaultPosition, wxDefaultSize, 0 );
	m_sky->SetValue(true); 
	bSizer297->Add( m_sky, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer299;
	bSizer299 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer299->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	m_skytexture = new wxComboBox( this, ID_SKYTEXTURE, wxEmptyString, wxDefaultPosition, wxSize( 200,-1 ), 0, NULL, wxCB_DROPDOWN ); 
	bSizer299->Add( m_skytexture, 1, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer297->Add( bSizer299, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxBoxSizer* bSizer300;
	bSizer300 = new wxBoxSizer( wxHORIZONTAL );
	
	m_fog = new wxCheckBox( this, ID_FOG, _("Fog"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fog->SetValue(true); 
	bSizer300->Add( m_fog, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text205 = new wxStaticText( this, ID_TEXT, _("Distance (m)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text205->Wrap( -1 );
	bSizer300->Add( m_text205, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_fog_distance = new wxTextCtrl( this, ID_FOG_DISTANCE, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_fog_distance->SetMaxLength( 0 ); 
	bSizer300->Add( m_fog_distance, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_slider_fog_distance = new wxSlider( this, ID_SLIDER_FOG_DISTANCE, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer300->Add( m_slider_fog_distance, 0, wxALIGN_CENTER|wxTOP, 5 );
	
	
	bSizer297->Add( bSizer300, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer301;
	bSizer301 = new wxBoxSizer( wxHORIZONTAL );
	
	m_shadows = new wxCheckBox( this, ID_SHADOWS, _("Shadows"), wxDefaultPosition, wxDefaultSize, 0 );
	m_shadows->SetValue(true); 
	bSizer301->Add( m_shadows, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer297->Add( bSizer301, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer302;
	bSizer302 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer302->Add( 20, 20, 0, wxALIGN_CENTER|wxLEFT, 5 );
	
	wxBoxSizer* bSizer303;
	bSizer303 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer304;
	bSizer304 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text206 = new wxStaticText( this, ID_TEXT, _("Darkness:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text206->Wrap( 0 );
	bSizer304->Add( m_text206, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_ambient_bias = new wxTextCtrl( this, ID_AMBIENT_BIAS, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_ambient_bias->SetMaxLength( 0 ); 
	bSizer304->Add( m_ambient_bias, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	m_slider_ambient_bias = new wxSlider( this, ID_SLIDER_AMBIENT_BIAS, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer304->Add( m_slider_ambient_bias, 0, wxALIGN_CENTER, 5 );
	
	
	bSizer303->Add( bSizer304, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_shadows_every_frame = new wxCheckBox( this, ID_SHADOWS_EVERY_FRAME, _("Recompute shadows every frame"), wxDefaultPosition, wxDefaultSize, 0 );
	m_shadows_every_frame->SetValue(true); 
	bSizer303->Add( m_shadows_every_frame, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer305;
	bSizer305 = new wxBoxSizer( wxHORIZONTAL );
	
	m_shadow_limit = new wxCheckBox( this, ID_SHADOW_LIMIT, _("Limit shadow area:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_shadow_limit->SetValue(true); 
	bSizer305->Add( m_shadow_limit, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_shadow_limit_radius = new wxTextCtrl( this, ID_SHADOW_LIMIT_RADIUS, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), 0 );
	m_shadow_limit_radius->SetMaxLength( 0 ); 
	bSizer305->Add( m_shadow_limit_radius, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_text222 = new wxStaticText( this, ID_TEXT, _("m"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text222->Wrap( 0 );
	bSizer305->Add( m_text222, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer303->Add( bSizer305, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer302->Add( bSizer303, 0, wxALIGN_CENTER, 5 );
	
	
	bSizer297->Add( bSizer302, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer306;
	bSizer306 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text207 = new wxStaticText( this, ID_TEXT, _("Scene background color:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text207->Wrap( -1 );
	bSizer306->Add( m_text207, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_color3 = new wxStaticBitmap( this, ID_COLOR3, wxBitmap( dummy_32x18_xpm ), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer306->Add( m_color3, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_bgcolor = new wxButton( this, ID_BGCOLOR, _("Set"), wxDefaultPosition, wxDefaultSize, 0 );
	m_bgcolor->SetDefault(); 
	bSizer306->Add( m_bgcolor, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer297->Add( bSizer306, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxFlexGridSizer* fgSizer8;
	fgSizer8 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer8->SetFlexibleDirection( wxBOTH );
	fgSizer8->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_text208 = new wxStaticText( this, ID_TEXT, _("Wind Direction:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text208->Wrap( -1 );
	fgSizer8->Add( m_text208, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text_wind_direction = new wxTextCtrl( this, ID_TEXT_WIND_DIRECTION, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_text_wind_direction->SetMaxLength( 0 ); 
	fgSizer8->Add( m_text_wind_direction, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	m_slider_wind_direction = new wxSlider( this, ID_SLIDER_WIND_DIRECTION, 0, 0, 150, wxDefaultPosition, wxSize( 150,-1 ), wxSL_HORIZONTAL );
	fgSizer8->Add( m_slider_wind_direction, 0, wxALIGN_CENTER|wxTOP, 5 );
	
	m_text209 = new wxStaticText( this, ID_TEXT, _("Wind Speed:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text209->Wrap( -1 );
	fgSizer8->Add( m_text209, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text_wind_speed = new wxTextCtrl( this, ID_TEXT_WIND_SPEED, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_text_wind_speed->SetMaxLength( 0 ); 
	fgSizer8->Add( m_text_wind_speed, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	m_slider_wind_speed = new wxSlider( this, ID_SLIDER_WIND_SPEED, 0, 0, 150, wxDefaultPosition, wxSize( 150,-1 ), wxSL_HORIZONTAL );
	fgSizer8->Add( m_slider_wind_speed, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxTOP, 5 );
	
	
	bSizer297->Add( fgSizer8, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer297 );
	this->Layout();
	bSizer297->Fit( this );
	
	this->Centre( wxBOTH );
}

EphemDlgBase::~EphemDlgBase()
{
}

LayerAnimDlgBase::LayerAnimDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer251;
	bSizer251 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer252;
	bSizer252 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer253;
	bSizer253 = new wxBoxSizer( wxVERTICAL );
	
	m_anim_pos = new wxSlider( this, ID_ANIM_POS, 0, 0, 1000, wxDefaultPosition, wxSize( -1,24 ), wxSL_HORIZONTAL );
	bSizer253->Add( m_anim_pos, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer254;
	bSizer254 = new wxBoxSizer( wxHORIZONTAL );
	
	m_reset = new wxBitmapButton( this, ID_RESET, wxBitmap( play_back_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_reset->SetDefault(); 
	bSizer254->Add( m_reset, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_stop = new wxBitmapButton( this, ID_STOP, wxBitmap( play_stop_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_stop->SetDefault(); 
	bSizer254->Add( m_stop, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_play = new wxBitmapButton( this, ID_PLAY, wxBitmap( play_play_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_play->SetDefault(); 
	bSizer254->Add( m_play, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text163 = new wxStaticText( this, ID_TEXT, _("Speed:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text163->Wrap( 0 );
	bSizer254->Add( m_text163, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_speed = new wxTextCtrl( this, ID_SPEED, _("2.0"), wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_speed->SetMaxLength( 0 ); 
	bSizer254->Add( m_speed, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text164 = new wxStaticText( this, ID_TEXT, _("fps"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text164->Wrap( 0 );
	bSizer254->Add( m_text164, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer253->Add( bSizer254, 0, wxALIGN_CENTER, 5 );
	
	
	bSizer252->Add( bSizer253, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	
	bSizer251->Add( bSizer252, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	this->SetSizer( bSizer251 );
	this->Layout();
	bSizer251->Fit( this );
	
	this->Centre( wxBOTH );
}

LayerAnimDlgBase::~LayerAnimDlgBase()
{
}

LocationDlgBase::LocationDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer92;
	bSizer92 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer93;
	bSizer93 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer94;
	bSizer94 = new wxBoxSizer( wxVERTICAL );
	
	m_loclist = new wxListBox( this, ID_LOCLIST, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	bSizer94->Add( m_loclist, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer95;
	bSizer95 = new wxBoxSizer( wxHORIZONTAL );
	
	m_save = new wxButton( this, ID_SAVE, _("Save"), wxDefaultPosition, wxDefaultSize, 0 );
	m_save->SetDefault(); 
	bSizer95->Add( m_save, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	m_load = new wxButton( this, ID_LOAD, _("Load"), wxDefaultPosition, wxDefaultSize, 0 );
	m_load->SetDefault(); 
	bSizer95->Add( m_load, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer94->Add( bSizer95, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	bSizer93->Add( bSizer94, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 0 );
	
	wxBoxSizer* bSizer96;
	bSizer96 = new wxBoxSizer( wxVERTICAL );
	
	m_recall = new wxButton( this, ID_RECALL, _("Recall ->"), wxDefaultPosition, wxDefaultSize, 0 );
	m_recall->SetDefault(); 
	bSizer96->Add( m_recall, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_store = new wxButton( this, ID_STORE, _("<- Store"), wxDefaultPosition, wxDefaultSize, 0 );
	m_store->SetDefault(); 
	bSizer96->Add( m_store, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_storeas = new wxButton( this, ID_STOREAS, _("Store As.."), wxDefaultPosition, wxDefaultSize, 0 );
	m_storeas->SetDefault(); 
	bSizer96->Add( m_storeas, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_remove = new wxButton( this, ID_REMOVE, _("Remove"), wxDefaultPosition, wxDefaultSize, 0 );
	m_remove->SetDefault(); 
	bSizer96->Add( m_remove, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer93->Add( bSizer96, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxTOP, 5 );
	
	
	bSizer92->Add( bSizer93, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_line = new wxStaticLine( this, ID_LINE, wxDefaultPosition, wxSize( 20,-1 ), wxLI_HORIZONTAL );
	bSizer92->Add( m_line, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer97;
	bSizer97 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer98;
	bSizer98 = new wxBoxSizer( wxVERTICAL );
	
	m_animtree = new wxTreeCtrl( this, ID_ANIMTREE, wxDefaultPosition, wxSize( 120,-1 ), wxTR_HAS_BUTTONS|wxTR_HIDE_ROOT|wxSUNKEN_BORDER );
	bSizer98->Add( m_animtree, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer99;
	bSizer99 = new wxBoxSizer( wxHORIZONTAL );
	
	m_new_anim = new wxButton( this, ID_NEW_ANIM, _("New"), wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_new_anim->SetDefault(); 
	bSizer99->Add( m_new_anim, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_BOTTOM, 5 );
	
	m_save_anim = new wxButton( this, ID_SAVE_ANIM, _("Save"), wxDefaultPosition, wxDefaultSize, 0 );
	m_save_anim->SetDefault(); 
	bSizer99->Add( m_save_anim, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_BOTTOM, 5 );
	
	m_load_anim = new wxButton( this, ID_LOAD_ANIM, _("Load"), wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_load_anim->SetDefault(); 
	bSizer99->Add( m_load_anim, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_BOTTOM, 5 );
	
	
	bSizer99->Add( 7, 14, 0, wxALIGN_CENTER, 5 );
	
	m_reset = new wxBitmapButton( this, ID_RESET, wxBitmap( play_back_xpm ), wxDefaultPosition, wxSize( -1,-1 ), wxBU_AUTODRAW );
	m_reset->SetDefault(); 
	bSizer99->Add( m_reset, 0, wxALIGN_CENTER|wxALL|wxALIGN_BOTTOM, 5 );
	
	m_stop = new wxBitmapButton( this, ID_STOP, wxBitmap( play_stop_xpm ), wxDefaultPosition, wxSize( -1,-1 ), wxBU_AUTODRAW );
	m_stop->SetDefault(); 
	bSizer99->Add( m_stop, 0, wxALIGN_CENTER|wxALL|wxALIGN_BOTTOM, 5 );
	
	m_record1 = new wxBitmapButton( this, ID_RECORD1, wxBitmap( play_record1_xpm ), wxDefaultPosition, wxSize( -1,-1 ), wxBU_AUTODRAW );
	m_record1->SetDefault(); 
	bSizer99->Add( m_record1, 0, wxALIGN_CENTER|wxALL|wxALIGN_BOTTOM, 5 );
	
	m_play = new wxBitmapButton( this, ID_PLAY, wxBitmap( play_play_xpm ), wxDefaultPosition, wxSize( 25,23 ), wxBU_AUTODRAW );
	m_play->SetDefault(); 
	bSizer99->Add( m_play, 0, wxALIGN_CENTER|wxALL|wxALIGN_BOTTOM, 5 );
	
	
	bSizer98->Add( bSizer99, 0, wxALIGN_CENTER, 5 );
	
	
	bSizer97->Add( bSizer98, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	
	bSizer92->Add( bSizer97, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer100;
	bSizer100 = new wxBoxSizer( wxHORIZONTAL );
	
	m_anim_pos = new wxSlider( this, ID_ANIM_POS, 0, 0, 1000, wxDefaultPosition, wxSize( -1,24 ), wxSL_HORIZONTAL );
	bSizer100->Add( m_anim_pos, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_active = new wxCheckBox( this, ID_ACTIVE, _("Active"), wxDefaultPosition, wxDefaultSize, 0 );
	m_active->SetValue(true); 
	bSizer100->Add( m_active, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer92->Add( bSizer100, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxStaticBoxSizer* sbSizer19;
	sbSizer19 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Playback") ), wxVERTICAL );
	
	wxBoxSizer* bSizer101;
	bSizer101 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text49 = new wxStaticText( sbSizer19->GetStaticBox(), ID_TEXT, _("Speed:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text49->Wrap( 0 );
	bSizer101->Add( m_text49, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_speedslider = new wxSlider( sbSizer19->GetStaticBox(), ID_SPEEDSLIDER, 0, 0, 100, wxDefaultPosition, wxSize( -1,24 ), wxSL_HORIZONTAL );
	bSizer101->Add( m_speedslider, 1, wxALIGN_CENTER, 5 );
	
	m_speed = new wxTextCtrl( sbSizer19->GetStaticBox(), ID_SPEED, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_speed->SetMaxLength( 0 ); 
	bSizer101->Add( m_speed, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text223 = new wxStaticText( sbSizer19->GetStaticBox(), wxID_ANY, _("m/s"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text223->Wrap( 0 );
	bSizer101->Add( m_text223, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer19->Add( bSizer101, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer102;
	bSizer102 = new wxBoxSizer( wxHORIZONTAL );
	
	m_loop = new wxCheckBox( sbSizer19->GetStaticBox(), ID_LOOP, _("Loop"), wxDefaultPosition, wxDefaultSize, 0 );
	m_loop->SetValue(true); 
	bSizer102->Add( m_loop, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_continuous = new wxCheckBox( sbSizer19->GetStaticBox(), ID_CONTINUOUS, _("Continuous"), wxDefaultPosition, wxDefaultSize, 0 );
	m_continuous->SetValue(true); 
	bSizer102->Add( m_continuous, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_smooth = new wxCheckBox( sbSizer19->GetStaticBox(), ID_SMOOTH, _("Smooth"), wxDefaultPosition, wxDefaultSize, 0 );
	m_smooth->SetValue(true); 
	bSizer102->Add( m_smooth, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_pos_only = new wxCheckBox( sbSizer19->GetStaticBox(), ID_POS_ONLY, _("Position Only"), wxDefaultPosition, wxDefaultSize, 0 );
	m_pos_only->SetValue(true); 
	bSizer102->Add( m_pos_only, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer19->Add( bSizer102, 0, wxALIGN_CENTER_HORIZONTAL, 5 );
	
	m_play_to_disk = new wxButton( sbSizer19->GetStaticBox(), ID_PLAY_TO_DISK, _("Play animation to disk as a series of images"), wxDefaultPosition, wxDefaultSize, 0 );
	m_play_to_disk->SetDefault(); 
	sbSizer19->Add( m_play_to_disk, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer92->Add( sbSizer19, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer20;
	sbSizer20 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Recording") ), wxVERTICAL );
	
	m_record_linear = new wxRadioButton( sbSizer20->GetStaticBox(), ID_RECORD_LINEAR, _("Use linear distance"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_record_linear->SetValue( true ); 
	sbSizer20->Add( m_record_linear, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer103;
	bSizer103 = new wxBoxSizer( wxHORIZONTAL );
	
	m_record_interval = new wxRadioButton( sbSizer20->GetStaticBox(), ID_RECORD_INTERVAL, _("Time interval"), wxDefaultPosition, wxDefaultSize, 0 );
	m_record_interval->SetValue( true ); 
	bSizer103->Add( m_record_interval, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_record_spacing = new wxTextCtrl( sbSizer20->GetStaticBox(), ID_RECORD_SPACING, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_record_spacing->SetMaxLength( 0 ); 
	bSizer103->Add( m_record_spacing, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text50 = new wxStaticText( sbSizer20->GetStaticBox(), ID_TEXT, _("seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text50->Wrap( 0 );
	bSizer103->Add( m_text50, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer20->Add( bSizer103, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer92->Add( sbSizer20, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	this->SetSizer( bSizer92 );
	this->Layout();
	bSizer92->Fit( this );
	
	this->Centre( wxBOTH );
}

LocationDlgBase::~LocationDlgBase()
{
}

OptionsDlgBase::OptionsDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer189;
	bSizer189 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer190;
	bSizer190 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer36;
	sbSizer36 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Display") ), wxVERTICAL );
	
	m_fullscreen = new wxCheckBox( sbSizer36->GetStaticBox(), ID_FULLSCREEN, _("Start in full screen"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fullscreen->SetValue(true); 
	sbSizer36->Add( m_fullscreen, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_stereo = new wxCheckBox( sbSizer36->GetStaticBox(), ID_STEREO, _("Stereo"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stereo->SetValue(true); 
	sbSizer36->Add( m_stereo, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxFlexGridSizer* fgSizer5;
	fgSizer5 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer5->SetFlexibleDirection( wxBOTH );
	fgSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	fgSizer5->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	m_stereo1 = new wxRadioButton( sbSizer36->GetStaticBox(), ID_STEREO1, _("Anaglyphic (red-blue)"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_stereo1->SetValue( true ); 
	fgSizer5->Add( m_stereo1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	fgSizer5->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	m_stereo2 = new wxRadioButton( sbSizer36->GetStaticBox(), ID_STEREO2, _("Quad buffer (shutter glasses)"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer5->Add( m_stereo2, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	fgSizer5->Add( 20, 20, 1, wxEXPAND, 5 );
	
	m_stereo3 = new wxRadioButton( sbSizer36->GetStaticBox(), ID_STEREO3, _("Horizontal split"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer5->Add( m_stereo3, 0, wxALL, 5 );
	
	
	fgSizer5->Add( 20, 20, 1, wxEXPAND, 5 );
	
	m_stereo4 = new wxRadioButton( sbSizer36->GetStaticBox(), ID_STEREO4, _("Vertical split"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer5->Add( m_stereo4, 0, wxALL, 5 );
	
	
	sbSizer36->Add( fgSizer5, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticText143 = new wxStaticText( sbSizer36->GetStaticBox(), wxID_ANY, _("Anti-aliasing (Multisamples):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText143->Wrap( -1 );
	sbSizer36->Add( m_staticText143, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer202;
	bSizer202 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer202->Add( 20, 0, 1, 0, 5 );
	
	m_samples0 = new wxRadioButton( sbSizer36->GetStaticBox(), wxID_ANY, _("None / System Default"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	bSizer202->Add( m_samples0, 0, wxALL, 5 );
	
	m_samples4 = new wxRadioButton( sbSizer36->GetStaticBox(), wxID_ANY, _("4x"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer202->Add( m_samples4, 0, wxALL, 5 );
	
	m_samples8 = new wxRadioButton( sbSizer36->GetStaticBox(), wxID_ANY, _("8x"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer202->Add( m_samples8, 0, wxALL, 5 );
	
	m_samples16 = new wxRadioButton( sbSizer36->GetStaticBox(), wxID_ANY, _("16x"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer202->Add( m_samples16, 0, wxALL, 5 );
	
	
	sbSizer36->Add( bSizer202, 1, 0, 5 );
	
	m_texture_compression = new wxCheckBox( sbSizer36->GetStaticBox(), ID_TEXTURE_COMPRESSION, _("Texture compression"), wxDefaultPosition, wxDefaultSize, 0 );
	m_texture_compression->SetValue(true); 
	sbSizer36->Add( m_texture_compression, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_disable_mipmaps = new wxCheckBox( sbSizer36->GetStaticBox(), ID_DISABLE_MIPMAPS, _("Disable model mipmaps"), wxDefaultPosition, wxDefaultSize, 0 );
	m_disable_mipmaps->SetValue(true); 
	sbSizer36->Add( m_disable_mipmaps, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer190->Add( sbSizer36, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer37;
	sbSizer37 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Window size") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer6;
	fgSizer6 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer6->SetFlexibleDirection( wxBOTH );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_text118 = new wxStaticText( sbSizer37->GetStaticBox(), ID_TEXT, _("Window X"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text118->Wrap( -1 );
	fgSizer6->Add( m_text118, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_winx = new wxTextCtrl( sbSizer37->GetStaticBox(), ID_WINX, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_winx->SetMaxLength( 0 ); 
	fgSizer6->Add( m_winx, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text119 = new wxStaticText( sbSizer37->GetStaticBox(), ID_TEXT, _("Window Y"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text119->Wrap( -1 );
	fgSizer6->Add( m_text119, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_winy = new wxTextCtrl( sbSizer37->GetStaticBox(), ID_WINY, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_winy->SetMaxLength( 0 ); 
	fgSizer6->Add( m_winy, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text120 = new wxStaticText( sbSizer37->GetStaticBox(), ID_TEXT, _("Width"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text120->Wrap( -1 );
	fgSizer6->Add( m_text120, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_win_xsize = new wxTextCtrl( sbSizer37->GetStaticBox(), ID_WIN_XSIZE, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_win_xsize->SetMaxLength( 0 ); 
	fgSizer6->Add( m_win_xsize, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text121 = new wxStaticText( sbSizer37->GetStaticBox(), ID_TEXT, _("Height"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text121->Wrap( -1 );
	fgSizer6->Add( m_text121, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_win_ysize = new wxTextCtrl( sbSizer37->GetStaticBox(), ID_WIN_YSIZE, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_win_ysize->SetMaxLength( 0 ); 
	fgSizer6->Add( m_win_ysize, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer37->Add( fgSizer6, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_size_inside = new wxCheckBox( sbSizer37->GetStaticBox(), ID_SIZE_INSIDE, _("Size is inside (3D view area, not frame)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_size_inside->SetValue(true); 
	sbSizer37->Add( m_size_inside, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer190->Add( sbSizer37, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer189->Add( bSizer190, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer208;
	bSizer208 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer38;
	sbSizer38 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Selection") ), wxVERTICAL );
	
	m_direct_picking = new wxCheckBox( sbSizer38->GetStaticBox(), ID_DIRECT_PICKING, _("Use direct picking of object under 2D cursor"), wxDefaultPosition, wxDefaultSize, 0 );
	m_direct_picking->SetValue(true); 
	sbSizer38->Add( m_direct_picking, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer191;
	bSizer191 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text122 = new wxStaticText( sbSizer38->GetStaticBox(), ID_TEXT, _("Selection cutoff distance:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text122->Wrap( -1 );
	bSizer191->Add( m_text122, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_selection_cutoff = new wxTextCtrl( sbSizer38->GetStaticBox(), ID_SELECTION_CUTOFF, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_selection_cutoff->SetMaxLength( 0 ); 
	bSizer191->Add( m_selection_cutoff, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text123 = new wxStaticText( sbSizer38->GetStaticBox(), ID_TEXT, _("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text123->Wrap( 0 );
	bSizer191->Add( m_text123, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer38->Add( bSizer191, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxBoxSizer* bSizer192;
	bSizer192 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text124 = new wxStaticText( sbSizer38->GetStaticBox(), ID_TEXT, _("Maximum radius for selecting models:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text124->Wrap( -1 );
	bSizer192->Add( m_text124, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_selection_radius = new wxTextCtrl( sbSizer38->GetStaticBox(), ID_SELECTION_RADIUS, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_selection_radius->SetMaxLength( 0 ); 
	bSizer192->Add( m_selection_radius, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text125 = new wxStaticText( sbSizer38->GetStaticBox(), ID_TEXT, _("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text125->Wrap( 0 );
	bSizer192->Add( m_text125, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer38->Add( bSizer192, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	
	bSizer208->Add( sbSizer38, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer193;
	bSizer193 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text126 = new wxStaticText( this, ID_TEXT, _("Size exaggeration for plants and fences: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text126->Wrap( -1 );
	bSizer193->Add( m_text126, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_plantsize = new wxTextCtrl( this, ID_PLANTSIZE, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_plantsize->SetMaxLength( 0 ); 
	bSizer193->Add( m_plantsize, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer208->Add( bSizer193, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	m_only_available_species = new wxCheckBox( this, ID_ONLY_AVAILABLE_SPECIES, _("Show only species with available appearances"), wxDefaultPosition, wxDefaultSize, 0 );
	m_only_available_species->SetValue(true); 
	bSizer208->Add( m_only_available_species, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer194;
	bSizer194 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text127 = new wxStaticText( this, ID_TEXT, _("Global content file:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text127->Wrap( -1 );
	bSizer194->Add( m_text127, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_choice_contentChoices[] = { _("Item") };
	int m_choice_contentNChoices = sizeof( m_choice_contentChoices ) / sizeof( wxString );
	m_choice_content = new wxChoice( this, ID_CHOICE_CONTENT, wxDefaultPosition, wxSize( 200,-1 ), m_choice_contentNChoices, m_choice_contentChoices, 0 );
	m_choice_content->SetSelection( 0 );
	bSizer194->Add( m_choice_content, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer208->Add( bSizer194, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_terrain_progress = new wxCheckBox( this, ID_TERRAIN_PROGRESS, _("Show progress dialog during terrain creation"), wxDefaultPosition, wxDefaultSize, 0 );
	m_terrain_progress->SetValue(true); 
	bSizer208->Add( m_terrain_progress, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_fly_in = new wxCheckBox( this, ID_FLY_IN, _("Fly in gradually from earth view"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fly_in->SetValue(true); 
	bSizer208->Add( m_fly_in, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_fly_in1 = new wxCheckBox( this, ID_ENABLE_JOYSTICK, _("Enable joystick navigation"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fly_in1->SetValue(true); 
	bSizer208->Add( m_fly_in1, 0, wxALL, 5 );
	
	m_fly_in2 = new wxCheckBox( this, ID_ENABLE_SPACENAV, _("Enable SpaceNavigator"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer208->Add( m_fly_in2, 0, wxALL, 5 );
	
	
	bSizer208->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer195;
	bSizer195 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer195->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer195->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer208->Add( bSizer195, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer189->Add( bSizer208, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer189 );
	this->Layout();
	bSizer189->Fit( this );
	
	this->Centre( wxBOTH );
}

OptionsDlgBase::~OptionsDlgBase()
{
}

PagingDlgBase::PagingDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer285;
	bSizer285 = new wxBoxSizer( wxVERTICAL );
	
	m_notebook = new wxNotebook( this, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, 0 );
	SurfaceLODPanel = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer286;
	bSizer286 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer54;
	sbSizer54 = new wxStaticBoxSizer( new wxStaticBox( SurfaceLODPanel, wxID_ANY, _("Detail levels chart") ), wxVERTICAL );
	
	m_panel1 = new wxPanel( sbSizer54->GetStaticBox(), ID_PANEL1, wxDefaultPosition, wxSize( 300,40 ), wxTAB_TRAVERSAL );
	sbSizer54->Add( m_panel1, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer286->Add( sbSizer54, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer287;
	bSizer287 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text187 = new wxStaticText( SurfaceLODPanel, ID_TEXT, _("Number of Triangles"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text187->Wrap( -1 );
	bSizer287->Add( m_text187, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_textCtrl84 = new wxTextCtrl( SurfaceLODPanel, ID_TARGET, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_textCtrl84->SetMaxLength( 0 ); 
	bSizer287->Add( m_textCtrl84, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	
	m_spinBtn1 = new wxSpinButton( SurfaceLODPanel, ID_SPIN, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer287->Add( m_spinBtn1, 0, wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );
	
	m_text188 = new wxStaticText( SurfaceLODPanel, ID_TEXT, _("Current"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text188->Wrap( 0 );
	bSizer287->Add( m_text188, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_current = new wxTextCtrl( SurfaceLODPanel, ID_CURRENT, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_current->SetMaxLength( 0 ); 
	bSizer287->Add( m_current, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer286->Add( bSizer287, 0, wxALIGN_CENTER, 5 );
	
	wxStaticBoxSizer* sbSizer222;
	sbSizer222 = new wxStaticBoxSizer( new wxStaticBox( SurfaceLODPanel, ID_TILESET_STATUS, _("Tileset Texture Status") ), wxVERTICAL );
	
	wxBoxSizer* bSizer288;
	bSizer288 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer289;
	bSizer289 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text189 = new wxStaticText( sbSizer222->GetStaticBox(), ID_TEXT, _("prange"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text189->Wrap( 0 );
	bSizer289->Add( m_text189, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_slider_prange = new wxSlider( sbSizer222->GetStaticBox(), ID_SLIDER_PRANGE, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer289->Add( m_slider_prange, 1, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	m_text_prange = new wxTextCtrl( sbSizer222->GetStaticBox(), ID_TEXT_PRANGE, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_text_prange->SetMaxLength( 0 ); 
	bSizer289->Add( m_text_prange, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer288->Add( bSizer289, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_panel2 = new wxPanel( sbSizer222->GetStaticBox(), ID_PANEL2, wxDefaultPosition, wxSize( 300,300 ), wxTAB_TRAVERSAL );
	bSizer288->Add( m_panel2, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer290;
	bSizer290 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticbitmap1 = new wxStaticBitmap( sbSizer222->GetStaticBox(), wxID_ANY, wxBitmap( LOD16_xpm ), wxDefaultPosition, wxSize( 20,20 ), 0 );
	bSizer290->Add( m_staticbitmap1, 0, wxALIGN_CENTER, 0 );
	
	m_text190 = new wxStaticText( sbSizer222->GetStaticBox(), ID_TEXT, _("16"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text190->Wrap( 0 );
	bSizer290->Add( m_text190, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_staticbitmap2 = new wxStaticBitmap( sbSizer222->GetStaticBox(), wxID_ANY, wxBitmap( LOD32_xpm ), wxDefaultPosition, wxSize( 20,20 ), 0 );
	bSizer290->Add( m_staticbitmap2, 0, wxALIGN_CENTER, 5 );
	
	m_text191 = new wxStaticText( sbSizer222->GetStaticBox(), ID_TEXT, _("32"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text191->Wrap( 0 );
	bSizer290->Add( m_text191, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_staticbitmap3 = new wxStaticBitmap( sbSizer222->GetStaticBox(), wxID_ANY, wxBitmap( LOD64_xpm ), wxDefaultPosition, wxSize( 20,20 ), 0 );
	bSizer290->Add( m_staticbitmap3, 0, wxALIGN_CENTER, 5 );
	
	m_text192 = new wxStaticText( sbSizer222->GetStaticBox(), ID_TEXT, _("64"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text192->Wrap( 0 );
	bSizer290->Add( m_text192, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_staticbitmap4 = new wxStaticBitmap( sbSizer222->GetStaticBox(), wxID_ANY, wxBitmap( LOD128_xpm ), wxDefaultPosition, wxSize( 20,20 ), 0 );
	bSizer290->Add( m_staticbitmap4, 0, wxALIGN_CENTER, 5 );
	
	m_text193 = new wxStaticText( sbSizer222->GetStaticBox(), ID_TEXT, _("128"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text193->Wrap( 0 );
	bSizer290->Add( m_text193, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_staticbitmap5 = new wxStaticBitmap( sbSizer222->GetStaticBox(), wxID_ANY, wxBitmap( LOD256_xpm ), wxDefaultPosition, wxSize( 20,20 ), 0 );
	bSizer290->Add( m_staticbitmap5, 0, wxALIGN_CENTER, 5 );
	
	m_text194 = new wxStaticText( sbSizer222->GetStaticBox(), ID_TEXT, _("256"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text194->Wrap( 0 );
	bSizer290->Add( m_text194, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_staticbitmap6 = new wxStaticBitmap( sbSizer222->GetStaticBox(), wxID_ANY, wxBitmap( LOD512_xpm ), wxDefaultPosition, wxSize( 20,20 ), 0 );
	bSizer290->Add( m_staticbitmap6, 0, wxALIGN_CENTER, 5 );
	
	m_text195 = new wxStaticText( sbSizer222->GetStaticBox(), ID_TEXT, _("512"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text195->Wrap( 0 );
	bSizer290->Add( m_text195, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_staticbitmap7 = new wxStaticBitmap( sbSizer222->GetStaticBox(), wxID_ANY, wxBitmap( LOD1k_xpm ), wxDefaultPosition, wxSize( 20,20 ), 0 );
	bSizer290->Add( m_staticbitmap7, 0, wxALIGN_CENTER, 5 );
	
	m_text196 = new wxStaticText( sbSizer222->GetStaticBox(), ID_TEXT, _("1k"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text196->Wrap( 0 );
	bSizer290->Add( m_text196, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_staticbitmap8 = new wxStaticBitmap( sbSizer222->GetStaticBox(), wxID_ANY, wxBitmap( LOD2k_xpm ), wxDefaultPosition, wxSize( 20,20 ), 0 );
	bSizer290->Add( m_staticbitmap8, 0, wxALIGN_CENTER, 5 );
	
	m_text197 = new wxStaticText( sbSizer222->GetStaticBox(), ID_TEXT, _("2k"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text197->Wrap( 0 );
	bSizer290->Add( m_text197, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	
	bSizer288->Add( bSizer290, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_tile_status = new wxTextCtrl( sbSizer222->GetStaticBox(), ID_TILE_STATUS, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_tile_status->SetMaxLength( 0 ); 
	bSizer288->Add( m_tile_status, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	sbSizer222->Add( bSizer288, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer286->Add( sbSizer222, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	SurfaceLODPanel->SetSizer( bSizer286 );
	SurfaceLODPanel->Layout();
	bSizer286->Fit( SurfaceLODPanel );
	m_notebook->AddPage( SurfaceLODPanel, _("Surface and Textures"), true );
	StructureLODPanel = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxStaticBoxSizer* sbSizer56;
	sbSizer56 = new wxStaticBoxSizer( new wxStaticBox( StructureLODPanel, wxID_ANY, _("Structure Paging Status") ), wxVERTICAL );
	
	wxBoxSizer* bSizer291;
	bSizer291 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text198 = new wxStaticText( sbSizer56->GetStaticBox(), ID_TEXT, _("Page-out distance:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text198->Wrap( -1 );
	bSizer291->Add( m_text198, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text_pageout = new wxTextCtrl( sbSizer56->GetStaticBox(), ID_TEXT_PAGEOUT, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), 0 );
	m_text_pageout->SetMaxLength( 0 ); 
	bSizer291->Add( m_text_pageout, 0, wxALIGN_CENTER, 5 );
	
	m_slider_pageout = new wxSlider( sbSizer56->GetStaticBox(), ID_SLIDER_PAGEOUT, 0, 0, 200, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	bSizer291->Add( m_slider_pageout, 1, wxALIGN_CENTER, 0 );
	
	
	sbSizer56->Add( bSizer291, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer292;
	bSizer292 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text199 = new wxStaticText( sbSizer56->GetStaticBox(), ID_TEXT, _("Currently constructed:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text199->Wrap( -1 );
	bSizer292->Add( m_text199, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_count_current = new wxTextCtrl( sbSizer56->GetStaticBox(), ID_COUNT_CURRENT, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_count_current->SetMaxLength( 0 ); 
	bSizer292->Add( m_count_current, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text200 = new wxStaticText( sbSizer56->GetStaticBox(), ID_TEXT, _("/"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text200->Wrap( 0 );
	bSizer292->Add( m_text200, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_count_maximum = new wxTextCtrl( sbSizer56->GetStaticBox(), ID_COUNT_MAXIMUM, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_count_maximum->SetMaxLength( 0 ); 
	bSizer292->Add( m_count_maximum, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer56->Add( bSizer292, 0, wxALIGN_CENTER, 5 );
	
	m_panel3 = new wxPanel( sbSizer56->GetStaticBox(), ID_PANEL3, wxDefaultPosition, wxSize( 300,300 ), wxTAB_TRAVERSAL );
	sbSizer56->Add( m_panel3, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	StructureLODPanel->SetSizer( sbSizer56 );
	StructureLODPanel->Layout();
	sbSizer56->Fit( StructureLODPanel );
	m_notebook->AddPage( StructureLODPanel, _("Structures"), false );
	
	bSizer285->Add( m_notebook, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	this->SetSizer( bSizer285 );
	this->Layout();
	bSizer285->Fit( this );
	
	this->Centre( wxBOTH );
}

PagingDlgBase::~PagingDlgBase()
{
}

PerformanceMonitorDlgBase::PerformanceMonitorDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer312;
	bSizer312 = new wxBoxSizer( wxVERTICAL );
	
	m_pm_listctrl = new wxListCtrl( this, ID_PM_LISTCTRL, wxDefaultPosition, wxSize( 800,400 ), wxLC_REPORT|wxSUNKEN_BORDER );
	bSizer312->Add( m_pm_listctrl, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text212 = new wxStaticText( this, ID_TEXT, _("Right click on counter to enable/disable"), wxDefaultPosition, wxSize( 300,-1 ), 0 );
	m_text212->Wrap( -1 );
	bSizer312->Add( m_text212, 0, wxALIGN_CENTER|wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	
	this->SetSizer( bSizer312 );
	this->Layout();
	bSizer312->Fit( this );
	
	// Connect Events
	m_pm_listctrl->Connect( wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK, wxListEventHandler( PerformanceMonitorDlgBase::OnListItemRightClick ), NULL, this );
}

PerformanceMonitorDlgBase::~PerformanceMonitorDlgBase()
{
	// Disconnect Events
	m_pm_listctrl->Disconnect( wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK, wxListEventHandler( PerformanceMonitorDlgBase::OnListItemRightClick ), NULL, this );
	
}

PlantDlgBase::PlantDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer85;
	bSizer85 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer86;
	bSizer86 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text41 = new wxStaticText( this, ID_TEXT, _(" Species:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text41->Wrap( -1 );
	bSizer86->Add( m_text41, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString mmpeciesChoices;
	mmpecies = new wxChoice( this, ID_SPECIES, wxDefaultPosition, wxSize( 160,-1 ), mmpeciesChoices, 0 );
	mmpecies->SetSelection( 0 );
	bSizer86->Add( mmpecies, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer85->Add( bSizer86, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer87;
	bSizer87 = new wxBoxSizer( wxHORIZONTAL );
	
	m_common_names = new wxCheckBox( this, ID_COMMON_NAMES, _("Common names"), wxDefaultPosition, wxDefaultSize, 0 );
	m_common_names->SetValue(true); 
	bSizer87->Add( m_common_names, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text42 = new wxStaticText( this, ID_TEXT, _("Language:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text42->Wrap( -1 );
	bSizer87->Add( m_text42, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString id_languageChoices;
	id_language = new wxChoice( this, ID_LANGUAGE, wxDefaultPosition, wxSize( 60,-1 ), id_languageChoices, 0 );
	id_language->SetSelection( 0 );
	bSizer87->Add( id_language, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer85->Add( bSizer87, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer88;
	bSizer88 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text221 = new wxStaticText( this, ID_TEXT, _("    Height:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text221->Wrap( -1 );
	bSizer88->Add( m_text221, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_plant_height_edit = new wxTextCtrl( this, ID_PLANT_HEIGHT_EDIT, wxEmptyString, wxDefaultPosition, wxSize( 40,-1 ), 0 );
	m_plant_height_edit->SetMaxLength( 0 ); 
	bSizer88->Add( m_plant_height_edit, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text43 = new wxStaticText( this, ID_TEXT, _("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text43->Wrap( -1 );
	bSizer88->Add( m_text43, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_height_slider = new wxSlider( this, ID_HEIGHT_SLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 120,-1 ), wxSL_HORIZONTAL );
	bSizer88->Add( m_height_slider, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer85->Add( bSizer88, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxStaticBoxSizer* sbSizer18;
	sbSizer18 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Planting Options") ), wxVERTICAL );
	
	wxBoxSizer* bSizer89;
	bSizer89 = new wxBoxSizer( wxVERTICAL );
	
	m_plant_individual = new wxRadioButton( sbSizer18->GetStaticBox(), ID_PLANT_INDIVIDUAL, _("Individual instances"), wxDefaultPosition, wxDefaultSize, 0 );
	m_plant_individual->SetValue( true ); 
	bSizer89->Add( m_plant_individual, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_plant_linear = new wxRadioButton( sbSizer18->GetStaticBox(), ID_PLANT_LINEAR, _("Straight line"), wxDefaultPosition, wxDefaultSize, 0 );
	m_plant_linear->SetValue( true ); 
	m_plant_linear->Enable( false );
	
	bSizer89->Add( m_plant_linear, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	m_plant_continuous = new wxRadioButton( sbSizer18->GetStaticBox(), ID_PLANT_CONTINUOUS, _("Continuously"), wxDefaultPosition, wxDefaultSize, 0 );
	m_plant_continuous->SetValue( true ); 
	m_plant_continuous->Enable( false );
	
	bSizer89->Add( m_plant_continuous, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	sbSizer18->Add( bSizer89, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer90;
	bSizer90 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text44 = new wxStaticText( sbSizer18->GetStaticBox(), ID_TEXT, _("Height variance:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text44->Wrap( -1 );
	bSizer90->Add( m_text44, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_plant_variance_edit = new wxTextCtrl( sbSizer18->GetStaticBox(), ID_PLANT_VARIANCE_EDIT, wxEmptyString, wxDefaultPosition, wxSize( 40,-1 ), 0 );
	m_plant_variance_edit->SetMaxLength( 0 ); 
	bSizer90->Add( m_plant_variance_edit, 0, wxALIGN_CENTER|wxLEFT|wxTOP|wxBOTTOM, 5 );
	
	m_text45 = new wxStaticText( sbSizer18->GetStaticBox(), ID_TEXT, _("%"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text45->Wrap( 0 );
	bSizer90->Add( m_text45, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_plant_variance_slider = new wxSlider( sbSizer18->GetStaticBox(), ID_PLANT_VARIANCE_SLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer90->Add( m_plant_variance_slider, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer18->Add( bSizer90, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer91;
	bSizer91 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text46 = new wxStaticText( sbSizer18->GetStaticBox(), ID_TEXT, _("Minimum spacing:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text46->Wrap( -1 );
	bSizer91->Add( m_text46, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_plant_spacing_edit = new wxTextCtrl( sbSizer18->GetStaticBox(), ID_PLANT_SPACING_EDIT, wxEmptyString, wxDefaultPosition, wxSize( 40,-1 ), 0 );
	m_plant_spacing_edit->SetMaxLength( 0 ); 
	bSizer91->Add( m_plant_spacing_edit, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text47 = new wxStaticText( sbSizer18->GetStaticBox(), ID_TEXT, _("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text47->Wrap( 0 );
	bSizer91->Add( m_text47, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer18->Add( bSizer91, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	
	bSizer85->Add( sbSizer18, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	this->SetSizer( bSizer85 );
	this->Layout();
	bSizer85->Fit( this );
	
	this->Centre( wxBOTH );
}

PlantDlgBase::~PlantDlgBase()
{
}

ScenariosPaneBase::ScenariosPaneBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer225;
	bSizer225 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer226;
	bSizer226 = new wxBoxSizer( wxHORIZONTAL );
	
	m_scenario_list = new wxListBox( this, ID_SCENARIO_LIST, wxDefaultPosition, wxSize( 200,150 ), 0, NULL, wxLB_SINGLE ); 
	bSizer226->Add( m_scenario_list, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	wxBoxSizer* bSizer227;
	bSizer227 = new wxBoxSizer( wxVERTICAL );
	
	m_new_scenario = new wxButton( this, ID_NEW_SCENARIO, _("New"), wxDefaultPosition, wxDefaultSize, 0 );
	m_new_scenario->SetDefault(); 
	bSizer227->Add( m_new_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_delete_scenario = new wxButton( this, ID_DELETE_SCENARIO, _("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	m_delete_scenario->SetDefault(); 
	bSizer227->Add( m_delete_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_edit_scenario = new wxButton( this, ID_EDIT_SCENARIO, _("Edit"), wxDefaultPosition, wxDefaultSize, 0 );
	m_edit_scenario->SetDefault(); 
	bSizer227->Add( m_edit_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_moveup_scenario = new wxButton( this, ID_MOVEUP_SCENARIO, _("Move Up"), wxDefaultPosition, wxDefaultSize, 0 );
	m_moveup_scenario->SetDefault(); 
	bSizer227->Add( m_moveup_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_movedown_scenario = new wxButton( this, ID_MOVEDOWN_SCENARIO, _("Move Down"), wxDefaultPosition, wxDefaultSize, 0 );
	m_movedown_scenario->SetDefault(); 
	bSizer227->Add( m_movedown_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer226->Add( bSizer227, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer225->Add( bSizer226, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer228;
	bSizer228 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text147 = new wxStaticText( this, ID_TEXT, _("Start with active scenario:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text147->Wrap( -1 );
	bSizer228->Add( m_text147, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_choice_scenarioChoices[] = { _("ChoiceItem") };
	int m_choice_scenarioNChoices = sizeof( m_choice_scenarioChoices ) / sizeof( wxString );
	m_choice_scenario = new wxChoice( this, ID_CHOICE_SCENARIO, wxDefaultPosition, wxSize( 200,-1 ), m_choice_scenarioNChoices, m_choice_scenarioChoices, 0 );
	m_choice_scenario->SetSelection( 0 );
	bSizer228->Add( m_choice_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer225->Add( bSizer228, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	this->SetSizer( bSizer225 );
	this->Layout();
	bSizer225->Fit( this );
	
	this->Centre( wxBOTH );
}

ScenariosPaneBase::~ScenariosPaneBase()
{
}

ScenarioParamsDlgBase::ScenarioParamsDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer232;
	bSizer232 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer233;
	bSizer233 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text148 = new wxStaticText( this, ID_TEXT, _("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text148->Wrap( 0 );
	bSizer233->Add( m_text148, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_scenario_name = new wxTextCtrl( this, ID_SCENARIO_NAME, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_scenario_name->SetMaxLength( 0 ); 
	bSizer233->Add( m_scenario_name, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer232->Add( bSizer233, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_notebook = new wxNotebook( this, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, 0 );
	ScenarioVisibleLayersPane = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer234;
	bSizer234 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer235;
	bSizer235 = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer45;
	sbSizer45 = new wxStaticBoxSizer( new wxStaticBox( ScenarioVisibleLayersPane, wxID_ANY, _("Visible Layers") ), wxVERTICAL );
	
	m_scenario_visible_layers = new wxListBox( sbSizer45->GetStaticBox(), ID_SCENARIO_VISIBLE_LAYERS, wxDefaultPosition, wxSize( 150,200 ), 0, NULL, wxLB_SINGLE ); 
	sbSizer45->Add( m_scenario_visible_layers, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer235->Add( sbSizer45, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer236;
	bSizer236 = new wxBoxSizer( wxVERTICAL );
	
	m_scenario_add_visible_layer = new wxButton( ScenarioVisibleLayersPane, ID_SCENARIO_ADD_VISIBLE_LAYER, _("<<"), wxDefaultPosition, wxDefaultSize, 0 );
	m_scenario_add_visible_layer->SetDefault(); 
	bSizer236->Add( m_scenario_add_visible_layer, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_scenario_remove_visible_layer = new wxButton( ScenarioVisibleLayersPane, ID_SCENARIO_REMOVE_VISIBLE_LAYER, _(">>"), wxDefaultPosition, wxDefaultSize, 0 );
	m_scenario_remove_visible_layer->SetDefault(); 
	bSizer236->Add( m_scenario_remove_visible_layer, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer235->Add( bSizer236, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer46;
	sbSizer46 = new wxStaticBoxSizer( new wxStaticBox( ScenarioVisibleLayersPane, wxID_ANY, _("Available Layers") ), wxVERTICAL );
	
	m_scenario_available_layers = new wxListBox( sbSizer46->GetStaticBox(), ID_SCENARIO_AVAILABLE_LAYERS, wxDefaultPosition, wxSize( 150,200 ), 0, NULL, wxLB_SINGLE ); 
	sbSizer46->Add( m_scenario_available_layers, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer235->Add( sbSizer46, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer234->Add( bSizer235, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	ScenarioVisibleLayersPane->SetSizer( bSizer234 );
	ScenarioVisibleLayersPane->Layout();
	bSizer234->Fit( ScenarioVisibleLayersPane );
	m_notebook->AddPage( ScenarioVisibleLayersPane, _("Visible Layers"), true );
	
	bSizer232->Add( m_notebook, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer237;
	bSizer237 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer237->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer237->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer232->Add( bSizer237, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer232 );
	this->Layout();
	bSizer232->Fit( this );
	
	this->Centre( wxBOTH );
}

ScenarioParamsDlgBase::~ScenarioParamsDlgBase()
{
}

ScenarioSelectDlgBase::ScenarioSelectDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* ScenarioSizerTop;
	ScenarioSizerTop = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* ScenarioSizerMid;
	ScenarioSizerMid = new wxBoxSizer( wxHORIZONTAL );
	
	m_scenario_list = new wxListBox( this, ID_SCENARIO_LIST, wxDefaultPosition, wxSize( 200,200 ), 0, NULL, wxLB_SINGLE ); 
	ScenarioSizerMid->Add( m_scenario_list, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	wxBoxSizer* ScenarioSizerSide;
	ScenarioSizerSide = new wxBoxSizer( wxVERTICAL );
	
	m_new_scenario = new wxButton( this, ID_NEW_SCENARIO, _("New"), wxDefaultPosition, wxDefaultSize, 0 );
	m_new_scenario->SetDefault(); 
	ScenarioSizerSide->Add( m_new_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_delete_scenario = new wxButton( this, ID_DELETE_SCENARIO, _("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	m_delete_scenario->SetDefault(); 
	ScenarioSizerSide->Add( m_delete_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_edit_scenario = new wxButton( this, ID_EDIT_SCENARIO, _("Edit"), wxDefaultPosition, wxDefaultSize, 0 );
	m_edit_scenario->SetDefault(); 
	ScenarioSizerSide->Add( m_edit_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_moveup_scenario = new wxButton( this, ID_MOVEUP_SCENARIO, _("Move Up"), wxDefaultPosition, wxDefaultSize, 0 );
	m_moveup_scenario->SetDefault(); 
	ScenarioSizerSide->Add( m_moveup_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_movedown_scenario = new wxButton( this, ID_MOVEDOWN_SCENARIO, _("Move Down"), wxDefaultPosition, wxDefaultSize, 0 );
	m_movedown_scenario->SetDefault(); 
	ScenarioSizerSide->Add( m_movedown_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_scenario_previous = new wxButton( this, ID_SCENARIO_PREVIOUS, _("Previous"), wxDefaultPosition, wxDefaultSize, 0 );
	m_scenario_previous->SetDefault(); 
	ScenarioSizerSide->Add( m_scenario_previous, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_scenario_next = new wxButton( this, ID_SCENARIO_NEXT, _("Next"), wxDefaultPosition, wxDefaultSize, 0 );
	m_scenario_next->SetDefault(); 
	ScenarioSizerSide->Add( m_scenario_next, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	ScenarioSizerMid->Add( ScenarioSizerSide, 0, wxALIGN_CENTER, 5 );
	
	
	ScenarioSizerTop->Add( ScenarioSizerMid, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer241;
	bSizer241 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer241->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cancel->SetDefault(); 
	bSizer241->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_apply = new wxButton( this, wxID_APPLY, _("Apply"), wxDefaultPosition, wxDefaultSize, 0 );
	m_apply->SetDefault(); 
	bSizer241->Add( m_apply, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	ScenarioSizerTop->Add( bSizer241, 0, wxALIGN_CENTER, 5 );
	
	
	this->SetSizer( ScenarioSizerTop );
	this->Layout();
	ScenarioSizerTop->Fit( this );
	
	this->Centre( wxBOTH );
}

ScenarioSelectDlgBase::~ScenarioSelectDlgBase()
{
}

StartupDlgBase::StartupDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxVERTICAL );
	
	m_text1 = new wxStaticText( this, ID_TEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_text1->Wrap( 0 );
	bSizer6->Add( m_text1, 0, wxALIGN_CENTER, 0 );
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Launch with:") ), wxVERTICAL );
	
	id_earthview = new wxRadioButton( sbSizer2->GetStaticBox(), ID_EARTHVIEW, _("Earth View"), wxDefaultPosition, wxDefaultSize, 0 );
	id_earthview->SetValue( true ); 
	sbSizer2->Add( id_earthview, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP, 10 );
	
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer7->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_imagetext = new wxStaticText( sbSizer2->GetStaticBox(), ID_IMAGETEXT, _("Image:  "), wxDefaultPosition, wxDefaultSize, 0 );
	m_imagetext->Wrap( -1 );
	bSizer7->Add( m_imagetext, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_image = new wxComboBox( sbSizer2->GetStaticBox(), ID_IMAGE, wxEmptyString, wxDefaultPosition, wxSize( 125,-1 ), 0, NULL, wxCB_DROPDOWN ); 
	bSizer7->Add( m_image, 1, wxALIGN_CENTER|wxALL, 0 );
	
	
	sbSizer2->Add( bSizer7, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_terrain = new wxRadioButton( sbSizer2->GetStaticBox(), ID_TERRAIN, _("Terrain"), wxDefaultPosition, wxDefaultSize, 0 );
	m_terrain->SetValue( true ); 
	sbSizer2->Add( m_terrain, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer8->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_tnameChoices[] = { _("Item") };
	int m_tnameNChoices = sizeof( m_tnameChoices ) / sizeof( wxString );
	m_tname = new wxChoice( sbSizer2->GetStaticBox(), ID_TNAME, wxDefaultPosition, wxSize( 190,-1 ), m_tnameNChoices, m_tnameChoices, 0 );
	m_tname->SetSelection( 0 );
	bSizer8->Add( m_tname, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_editprop = new wxButton( sbSizer2->GetStaticBox(), ID_EDITPROP, _("Edit Properties"), wxDefaultPosition, wxDefaultSize, 0 );
	m_editprop->SetDefault(); 
	bSizer8->Add( m_editprop, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer2->Add( bSizer8, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	
	bSizer6->Add( sbSizer2, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 10 );
	
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxHORIZONTAL );
	
	m_terrman = new wxButton( this, ID_TERRMAN, _("Terrain Manager"), wxDefaultPosition, wxDefaultSize, 0 );
	m_terrman->SetDefault(); 
	bSizer9->Add( m_terrman, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_options = new wxButton( this, ID_OPTIONS, _("Global Options"), wxDefaultPosition, wxDefaultSize, 0 );
	m_options->SetDefault(); 
	bSizer9->Add( m_options, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_opengl = new wxButton( this, ID_OPENGL, _("OpenGL Info"), wxDefaultPosition, wxDefaultSize, 0 );
	m_opengl->SetDefault(); 
	bSizer9->Add( m_opengl, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer6->Add( bSizer9, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );
	
	ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	ok->SetDefault(); 
	bSizer10->Add( ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	cancel->SetDefault(); 
	bSizer10->Add( cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer6->Add( bSizer10, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer6 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

StartupDlgBase::~StartupDlgBase()
{
}

StyleDlgBase::StyleDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer196;
	bSizer196 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer197;
	bSizer197 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text128 = new wxStaticText( this, ID_TEXT, _("Feature type:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text128->Wrap( -1 );
	bSizer197->Add( m_text128, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_feature_type = new wxTextCtrl( this, ID_FEATURE_TYPE, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), wxTE_READONLY );
	m_feature_type->SetMaxLength( 0 ); 
	bSizer197->Add( m_feature_type, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer196->Add( bSizer197, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_notebook = new wxNotebook( this, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, 0 );
	StylePanel1 = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer198;
	bSizer198 = new wxBoxSizer( wxVERTICAL );
	
	m_enable_object_geom = new wxCheckBox( StylePanel1, ID_ENABLE_OBJECT_GEOM, _("Enable"), wxDefaultPosition, wxDefaultSize, 0 );
	m_enable_object_geom->SetValue(true); 
	bSizer198->Add( m_enable_object_geom, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxStaticBoxSizer* sbSizer39;
	sbSizer39 = new wxStaticBoxSizer( new wxStaticBox( StylePanel1, wxID_ANY, wxEmptyString ), wxVERTICAL );
	
	wxBoxSizer* bSizer199;
	bSizer199 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text129 = new wxStaticText( sbSizer39->GetStaticBox(), ID_TEXT, _("Color:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text129->Wrap( 0 );
	bSizer199->Add( m_text129, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	wxBoxSizer* bSizer200;
	bSizer200 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer201;
	bSizer201 = new wxBoxSizer( wxHORIZONTAL );
	
	m_radio1 = new wxRadioButton( sbSizer39->GetStaticBox(), ID_RADIO1, _("Fixed"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radio1->SetValue( true ); 
	bSizer201->Add( m_radio1, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_object_geom_color = new wxBitmapButton( sbSizer39->GetStaticBox(), ID_OBJECT_GEOM_COLOR, wxBitmap( dummy_32x18_xpm ), wxDefaultPosition, wxSize( 40,26 ), wxBU_AUTODRAW );
	m_object_geom_color->SetDefault(); 
	bSizer201->Add( m_object_geom_color, 0, wxALIGN_CENTER, 5 );
	
	
	bSizer200->Add( bSizer201, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer202;
	bSizer202 = new wxBoxSizer( wxHORIZONTAL );
	
	m_radio_use_object_color_field = new wxRadioButton( sbSizer39->GetStaticBox(), ID_RADIO_USE_OBJECT_COLOR_FIELD, _("Field"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer202->Add( m_radio_use_object_color_field, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_object_color_fieldChoices[] = { _("Item") };
	int m_object_color_fieldNChoices = sizeof( m_object_color_fieldChoices ) / sizeof( wxString );
	m_object_color_field = new wxChoice( sbSizer39->GetStaticBox(), ID_OBJECT_COLOR_FIELD, wxDefaultPosition, wxSize( 180,-1 ), m_object_color_fieldNChoices, m_object_color_fieldChoices, 0 );
	m_object_color_field->SetSelection( 0 );
	m_object_color_field->SetMinSize( wxSize( 300,-1 ) );
	
	bSizer202->Add( m_object_color_field, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer200->Add( bSizer202, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer199->Add( bSizer200, 1, wxALIGN_CENTER, 5 );
	
	
	sbSizer39->Add( bSizer199, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxBoxSizer* bSizer203;
	bSizer203 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text130 = new wxStaticText( sbSizer39->GetStaticBox(), ID_TEXT, _("Height above ground"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text130->Wrap( -1 );
	bSizer203->Add( m_text130, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_object_geom_height = new wxTextCtrl( sbSizer39->GetStaticBox(), ID_OBJECT_GEOM_HEIGHT, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_object_geom_height->SetMaxLength( 0 ); 
	bSizer203->Add( m_object_geom_height, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text131 = new wxStaticText( sbSizer39->GetStaticBox(), ID_TEXT, _("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text131->Wrap( 0 );
	bSizer203->Add( m_text131, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer39->Add( bSizer203, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	wxBoxSizer* bSizer204;
	bSizer204 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text132 = new wxStaticText( sbSizer39->GetStaticBox(), ID_TEXT, _("Size/scale"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text132->Wrap( 0 );
	bSizer204->Add( m_text132, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_object_geom_size = new wxTextCtrl( sbSizer39->GetStaticBox(), ID_OBJECT_GEOM_SIZE, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_object_geom_size->SetMaxLength( 0 ); 
	bSizer204->Add( m_object_geom_size, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text133 = new wxStaticText( sbSizer39->GetStaticBox(), ID_TEXT, _("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text133->Wrap( 0 );
	bSizer204->Add( m_text133, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer39->Add( bSizer204, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	
	bSizer198->Add( sbSizer39, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	StylePanel1->SetSizer( bSizer198 );
	StylePanel1->Layout();
	bSizer198->Fit( StylePanel1 );
	m_notebook->AddPage( StylePanel1, _("Object Geometry"), true );
	StylePanel2 = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer205;
	bSizer205 = new wxBoxSizer( wxVERTICAL );
	
	m_enable_line_geom = new wxCheckBox( StylePanel2, ID_ENABLE_LINE_GEOM, _("Enable"), wxDefaultPosition, wxDefaultSize, 0 );
	m_enable_line_geom->SetValue(true); 
	bSizer205->Add( m_enable_line_geom, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxStaticBoxSizer* sbSizer40;
	sbSizer40 = new wxStaticBoxSizer( new wxStaticBox( StylePanel2, wxID_ANY, wxEmptyString ), wxVERTICAL );
	
	wxBoxSizer* bSizer206;
	bSizer206 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text134 = new wxStaticText( sbSizer40->GetStaticBox(), ID_TEXT, _("Color:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text134->Wrap( 0 );
	bSizer206->Add( m_text134, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	wxBoxSizer* bSizer207;
	bSizer207 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer208;
	bSizer208 = new wxBoxSizer( wxHORIZONTAL );
	
	m_radio2 = new wxRadioButton( sbSizer40->GetStaticBox(), ID_RADIO2, _("Fixed"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radio2->SetValue( true ); 
	bSizer208->Add( m_radio2, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_line_geom_color = new wxBitmapButton( sbSizer40->GetStaticBox(), ID_LINE_GEOM_COLOR, wxBitmap( dummy_32x18_xpm ), wxDefaultPosition, wxSize( 40,26 ), wxBU_AUTODRAW );
	m_line_geom_color->SetDefault(); 
	bSizer208->Add( m_line_geom_color, 0, wxALIGN_CENTER, 5 );
	
	
	bSizer207->Add( bSizer208, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer209;
	bSizer209 = new wxBoxSizer( wxHORIZONTAL );
	
	m_radio_use_line_color_field = new wxRadioButton( sbSizer40->GetStaticBox(), ID_RADIO_USE_LINE_COLOR_FIELD, _("Field"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio_use_line_color_field->SetValue( true ); 
	bSizer209->Add( m_radio_use_line_color_field, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_line_color_fieldChoices[] = { _("Item") };
	int m_line_color_fieldNChoices = sizeof( m_line_color_fieldChoices ) / sizeof( wxString );
	m_line_color_field = new wxChoice( sbSizer40->GetStaticBox(), ID_LINE_COLOR_FIELD, wxDefaultPosition, wxSize( 180,-1 ), m_line_color_fieldNChoices, m_line_color_fieldChoices, 0 );
	m_line_color_field->SetSelection( 0 );
	bSizer209->Add( m_line_color_field, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer207->Add( bSizer209, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer206->Add( bSizer207, 1, wxALIGN_CENTER, 5 );
	
	
	sbSizer40->Add( bSizer206, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxBoxSizer* bSizer210;
	bSizer210 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text135 = new wxStaticText( sbSizer40->GetStaticBox(), ID_TEXT, _("Height above ground"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text135->Wrap( -1 );
	bSizer210->Add( m_text135, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_line_geom_height = new wxTextCtrl( sbSizer40->GetStaticBox(), ID_LINE_GEOM_HEIGHT, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_line_geom_height->SetMaxLength( 0 ); 
	bSizer210->Add( m_line_geom_height, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text136 = new wxStaticText( sbSizer40->GetStaticBox(), ID_TEXT, _("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text136->Wrap( 0 );
	bSizer210->Add( m_text136, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer40->Add( bSizer210, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	wxBoxSizer* bSizer211;
	bSizer211 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text137 = new wxStaticText( sbSizer40->GetStaticBox(), ID_TEXT, _("Line width"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text137->Wrap( -1 );
	bSizer211->Add( m_text137, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_line_width = new wxTextCtrl( sbSizer40->GetStaticBox(), ID_LINE_WIDTH, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_line_width->SetMaxLength( 0 ); 
	bSizer211->Add( m_line_width, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text138 = new wxStaticText( sbSizer40->GetStaticBox(), ID_TEXT, _("pixels"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text138->Wrap( 0 );
	bSizer211->Add( m_text138, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer40->Add( bSizer211, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	wxBoxSizer* bSizer212;
	bSizer212 = new wxBoxSizer( wxHORIZONTAL );
	
	m_tessellate = new wxCheckBox( sbSizer40->GetStaticBox(), ID_TESSELLATE, _("Tessellate edges"), wxDefaultPosition, wxDefaultSize, 0 );
	m_tessellate->SetValue(true); 
	bSizer212->Add( m_tessellate, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer40->Add( bSizer212, 0, wxALIGN_CENTER_VERTICAL, 0 );
	
	
	bSizer205->Add( sbSizer40, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	StylePanel2->SetSizer( bSizer205 );
	StylePanel2->Layout();
	bSizer205->Fit( StylePanel2 );
	m_notebook->AddPage( StylePanel2, _("Line Geometry"), false );
	StylePanel3 = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer213;
	bSizer213 = new wxBoxSizer( wxVERTICAL );
	
	m_enable_text_labels = new wxCheckBox( StylePanel3, ID_ENABLE_TEXT_LABELS, _("Enable"), wxDefaultPosition, wxDefaultSize, 0 );
	m_enable_text_labels->SetValue(true); 
	bSizer213->Add( m_enable_text_labels, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxStaticBoxSizer* sbSizer41;
	sbSizer41 = new wxStaticBoxSizer( new wxStaticBox( StylePanel3, wxID_ANY, wxEmptyString ), wxVERTICAL );
	
	wxBoxSizer* bSizer214;
	bSizer214 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text139 = new wxStaticText( sbSizer41->GetStaticBox(), ID_TEXT, _("Color:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text139->Wrap( 0 );
	bSizer214->Add( m_text139, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	wxBoxSizer* bSizer215;
	bSizer215 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer216;
	bSizer216 = new wxBoxSizer( wxHORIZONTAL );
	
	m_radio3 = new wxRadioButton( sbSizer41->GetStaticBox(), ID_RADIO3, _("Fixed"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radio3->SetValue( true ); 
	bSizer216->Add( m_radio3, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text_color = new wxBitmapButton( sbSizer41->GetStaticBox(), ID_TEXT_COLOR, wxBitmap( dummy_32x18_xpm ), wxDefaultPosition, wxSize( 40,26 ), wxBU_AUTODRAW );
	m_text_color->SetDefault(); 
	bSizer216->Add( m_text_color, 0, wxALIGN_CENTER, 5 );
	
	
	bSizer215->Add( bSizer216, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer217;
	bSizer217 = new wxBoxSizer( wxHORIZONTAL );
	
	m_radio_use_text_color_field = new wxRadioButton( sbSizer41->GetStaticBox(), ID_RADIO_USE_TEXT_COLOR_FIELD, _("Field"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer217->Add( m_radio_use_text_color_field, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_text_color_fieldChoices[] = { _("Item") };
	int m_text_color_fieldNChoices = sizeof( m_text_color_fieldChoices ) / sizeof( wxString );
	m_text_color_field = new wxChoice( sbSizer41->GetStaticBox(), ID_TEXT_COLOR_FIELD, wxDefaultPosition, wxSize( 180,-1 ), m_text_color_fieldNChoices, m_text_color_fieldChoices, 0 );
	m_text_color_field->SetSelection( 0 );
	bSizer217->Add( m_text_color_field, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer215->Add( bSizer217, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer214->Add( bSizer215, 1, wxALIGN_CENTER, 5 );
	
	
	sbSizer41->Add( bSizer214, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxBoxSizer* bSizer218;
	bSizer218 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text140 = new wxStaticText( sbSizer41->GetStaticBox(), ID_TEXT, _("Text field:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text140->Wrap( -1 );
	bSizer218->Add( m_text140, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_text_fieldChoices[] = { _("Item") };
	int m_text_fieldNChoices = sizeof( m_text_fieldChoices ) / sizeof( wxString );
	m_text_field = new wxChoice( sbSizer41->GetStaticBox(), ID_TEXT_FIELD, wxDefaultPosition, wxSize( 160,-1 ), m_text_fieldNChoices, m_text_fieldChoices, 0 );
	m_text_field->SetSelection( 0 );
	bSizer218->Add( m_text_field, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer41->Add( bSizer218, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer219;
	bSizer219 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text141 = new wxStaticText( sbSizer41->GetStaticBox(), ID_TEXT, _("Height above ground"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text141->Wrap( -1 );
	bSizer219->Add( m_text141, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_label_height = new wxTextCtrl( sbSizer41->GetStaticBox(), ID_LABEL_HEIGHT, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_label_height->SetMaxLength( 0 ); 
	bSizer219->Add( m_label_height, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text142 = new wxStaticText( sbSizer41->GetStaticBox(), ID_TEXT, _("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text142->Wrap( 0 );
	bSizer219->Add( m_text142, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer41->Add( bSizer219, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer220;
	bSizer220 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text143 = new wxStaticText( sbSizer41->GetStaticBox(), ID_TEXT, _("Label Size (vertical)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text143->Wrap( -1 );
	bSizer220->Add( m_text143, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_label_size = new wxTextCtrl( sbSizer41->GetStaticBox(), ID_LABEL_SIZE, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_label_size->SetMaxLength( 0 ); 
	bSizer220->Add( m_label_size, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text144 = new wxStaticText( sbSizer41->GetStaticBox(), ID_TEXT, _("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text144->Wrap( 0 );
	bSizer220->Add( m_text144, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer41->Add( bSizer220, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer221;
	bSizer221 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text145 = new wxStaticText( sbSizer41->GetStaticBox(), ID_TEXT, _("Font:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text145->Wrap( 0 );
	bSizer221->Add( m_text145, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_font = new wxTextCtrl( sbSizer41->GetStaticBox(), ID_FONT, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_font->SetMaxLength( 0 ); 
	bSizer221->Add( m_font, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_outline = new wxCheckBox( sbSizer41->GetStaticBox(), ID_OUTLINE, _("Outline Text"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer221->Add( m_outline, 0, wxALL, 5 );
	
	
	sbSizer41->Add( bSizer221, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer213->Add( sbSizer41, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	StylePanel3->SetSizer( bSizer213 );
	StylePanel3->Layout();
	bSizer213->Fit( StylePanel3 );
	m_notebook->AddPage( StylePanel3, _("Text Labels"), false );
	
	bSizer196->Add( m_notebook, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer224;
	bSizer224 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer224->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancell = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer224->Add( m_cancell, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer196->Add( bSizer224, 0, wxALIGN_CENTER, 5 );
	
	
	this->SetSizer( bSizer196 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

StyleDlgBase::~StyleDlgBase()
{
}

TerrManDlgBase::TerrManDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer171;
	bSizer171 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer172;
	bSizer172 = new wxBoxSizer( wxHORIZONTAL );
	
	m_treectrl = new wxTreeCtrl( this, ID_TREECTRL, wxDefaultPosition, wxSize( 380,240 ), wxTR_HAS_BUTTONS|wxTR_LINES_AT_ROOT|wxSUNKEN_BORDER );
	bSizer172->Add( m_treectrl, 1, wxEXPAND|wxALL, 5 );
	
	wxBoxSizer* bSizer173;
	bSizer173 = new wxBoxSizer( wxVERTICAL );
	
	m_add_path = new wxButton( this, ID_ADD_PATH, _("Add Path"), wxDefaultPosition, wxDefaultSize, 0 );
	m_add_path->SetDefault(); 
	bSizer173->Add( m_add_path, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_add_terrain = new wxButton( this, ID_ADD_TERRAIN, _("Add Terrain"), wxDefaultPosition, wxDefaultSize, 0 );
	m_add_terrain->SetDefault(); 
	bSizer173->Add( m_add_terrain, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_line1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxSize( 20,-1 ), wxLI_HORIZONTAL );
	bSizer173->Add( m_line1, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_copy = new wxButton( this, ID_COPY, _("Add Copy"), wxDefaultPosition, wxDefaultSize, 0 );
	m_copy->SetDefault(); 
	bSizer173->Add( m_copy, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_delete = new wxButton( this, ID_DELETE, _("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	m_delete->SetDefault(); 
	bSizer173->Add( m_delete, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_line2 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxSize( 20,-1 ), wxLI_HORIZONTAL );
	bSizer173->Add( m_line2, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_edit_params = new wxButton( this, ID_EDIT_PARAMS, _("Edit Parameters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_edit_params->SetDefault(); 
	bSizer173->Add( m_edit_params, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer173->Add( 20, 10, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer173->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer172->Add( bSizer173, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	
	bSizer171->Add( bSizer172, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	this->SetSizer( bSizer171 );
	this->Layout();
	bSizer171->Fit( this );
	
	this->Centre( wxBOTH );
}

TerrManDlgBase::~TerrManDlgBase()
{
}

TextDlgBase::TextDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer118;
	bSizer118 = new wxBoxSizer( wxVERTICAL );
	
	m_text61 = new wxTextCtrl( this, ID_TEXT, wxEmptyString, wxDefaultPosition, wxSize( 450,250 ), wxTE_MULTILINE|wxTE_READONLY );
	bSizer118->Add( m_text61, 0, wxALIGN_CENTER|wxALL, 5 );
	
	ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	ok->SetDefault(); 
	bSizer118->Add( ok, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	this->SetSizer( bSizer118 );
	this->Layout();
	bSizer118->Fit( this );
	
	this->Centre( wxBOTH );
}

TextDlgBase::~TextDlgBase()
{
}

TextureDlgBase::TextureDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer247;
	bSizer247 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer25;
	bSizer25 = new wxBoxSizer( wxHORIZONTAL );
	
	m_single = new wxRadioButton( this, ID_SINGLE, _("Single texture "), wxDefaultPosition, wxDefaultSize, 0 );
	m_single->SetValue( true ); 
	bSizer25->Add( m_single, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_tfile_single = new wxComboBox( this, ID_TFILE_SINGLE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN|wxCB_SORT );
	m_tfile_single->Append( _("Item") );
	bSizer25->Add( m_tfile_single, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer247->Add( bSizer25, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 0 );
	
	m_derived = new wxRadioButton( this, ID_DERIVED, _("Derive color from elevation"), wxDefaultPosition, wxDefaultSize, 0 );
	m_derived->SetValue( true ); 
	bSizer247->Add( m_derived, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer26;
	bSizer26 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer26->Add( 20, 10, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_choice_colorsChoices[] = { _("Item") };
	int m_choice_colorsNChoices = sizeof( m_choice_colorsChoices ) / sizeof( wxString );
	m_choice_colors = new wxChoice( this, ID_CHOICE_COLORS, wxDefaultPosition, wxSize( 180,-1 ), m_choice_colorsNChoices, m_choice_colorsChoices, 0 );
	m_choice_colors->SetSelection( 0 );
	bSizer26->Add( m_choice_colors, 1, wxALIGN_CENTER|wxALL, 5 );
	
	id_edit_colors = new wxButton( this, ID_EDIT_COLORS, _("Edit Colors..."), wxDefaultPosition, wxDefaultSize, 0 );
	id_edit_colors->SetDefault(); 
	bSizer26->Add( id_edit_colors, 0, wxALIGN_CENTER|wxALIGN_BOTTOM|wxALL, 5 );
	
	
	bSizer247->Add( bSizer26, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer30;
	bSizer30 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer32;
	bSizer32 = new wxBoxSizer( wxVERTICAL );
	
	m_prelight = new wxCheckBox( this, ID_PRELIGHT, _("Precompute lighting"), wxDefaultPosition, wxDefaultSize, 0 );
	m_prelight->SetValue(true); 
	bSizer32->Add( m_prelight, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer33;
	bSizer33 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer33->Add( 20, 10, 1, wxEXPAND, 5 );
	
	m_text9 = new wxStaticText( this, ID_TEXT, _("Lighting factor: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text9->Wrap( -1 );
	bSizer33->Add( m_text9, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_light_factor = new wxTextCtrl( this, ID_LIGHT_FACTOR, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_light_factor->SetMaxLength( 0 ); 
	bSizer33->Add( m_light_factor, 0, wxALIGN_CENTER|wxRIGHT, 5 );
	
	
	bSizer32->Add( bSizer33, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_cast_shadows = new wxCheckBox( this, ID_CAST_SHADOWS, _("Cast shadows"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cast_shadows->SetValue(true); 
	bSizer32->Add( m_cast_shadows, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer30->Add( bSizer32, 1, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 0 );
	
	wxBoxSizer* bSizer31;
	bSizer31 = new wxBoxSizer( wxVERTICAL );
	
	mmipmap = new wxCheckBox( this, ID_MIPMAP, _("Mipmapping"), wxDefaultPosition, wxDefaultSize, 0 );
	mmipmap->SetValue(true); 
	bSizer31->Add( mmipmap, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_16bit = new wxCheckBox( this, ID_16BIT, _("Request 16-bit"), wxDefaultPosition, wxDefaultSize, 0 );
	m_16bit->SetValue(true); 
	bSizer31->Add( m_16bit, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer30->Add( bSizer31, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer247->Add( bSizer30, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxALL, 5 );
	
	wxBoxSizer* bSizer34;
	bSizer34 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer247->Add( bSizer34, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5 );
	
	wxBoxSizer* bSizer84;
	bSizer84 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer84->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer84->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer247->Add( bSizer84, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer247 );
	this->Layout();
	bSizer247->Fit( this );
	
	this->Centre( wxBOTH );
}

TextureDlgBase::~TextureDlgBase()
{
}

TinTextureDlgBase::TinTextureDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer247;
	bSizer247 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer242;
	bSizer242 = new wxBoxSizer( wxVERTICAL );
	
	m_derive_color = new wxCheckBox( this, ID_DERIVE_COLOR, _("Derive color from elevation"), wxDefaultPosition, wxDefaultSize, 0 );
	m_derive_color->SetValue(true); 
	bSizer242->Add( m_derive_color, 0, wxALL, 5 );
	
	
	bSizer247->Add( bSizer242, 0, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );
	
	wxBoxSizer* bSizer26;
	bSizer26 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer26->Add( 20, 10, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_choice_colorsChoices[] = { _("Item") };
	int m_choice_colorsNChoices = sizeof( m_choice_colorsChoices ) / sizeof( wxString );
	m_choice_colors = new wxChoice( this, ID_CHOICE_COLORS, wxDefaultPosition, wxSize( -1,-1 ), m_choice_colorsNChoices, m_choice_colorsChoices, 0 );
	m_choice_colors->SetSelection( 0 );
	m_choice_colors->SetMinSize( wxSize( 200,-1 ) );
	
	bSizer26->Add( m_choice_colors, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_edit_colors = new wxButton( this, ID_EDIT_COLORS, _("Edit Colors..."), wxDefaultPosition, wxDefaultSize, 0 );
	m_edit_colors->SetDefault(); 
	bSizer26->Add( m_edit_colors, 0, wxALIGN_CENTER|wxALIGN_BOTTOM|wxALL, 5 );
	
	
	bSizer247->Add( bSizer26, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer261;
	bSizer261 = new wxBoxSizer( wxHORIZONTAL );
	
	m_geotypical = new wxCheckBox( this, ID_GEOTYPICAL, _("Geotypical texture"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer261->Add( m_geotypical, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_tfile_geotypical = new wxComboBox( this, ID_TFILE_GEOTYPICAL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN|wxCB_SORT );
	m_tfile_geotypical->Append( _("Item") );
	m_tfile_geotypical->SetMinSize( wxSize( 180,-1 ) );
	
	bSizer261->Add( m_tfile_geotypical, 1, wxALIGN_CENTER|wxTOP|wxBOTTOM|wxRIGHT, 5 );
	
	
	bSizer247->Add( bSizer261, 1, wxEXPAND|wxLEFT, 5 );
	
	wxBoxSizer* bSizer2611;
	bSizer2611 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer2611->Add( 20, 0, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_staticText1491 = new wxStaticText( this, wxID_ANY, _("Tiling:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1491->Wrap( -1 );
	bSizer2611->Add( m_staticText1491, 0, wxALL, 5 );
	
	m_geotypical_scale = new wxTextCtrl( this, ID_GEOTYPICAL_SCALE, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_geotypical_scale->SetMaxLength( 0 ); 
	m_geotypical_scale->SetMinSize( wxSize( 60,-1 ) );
	
	bSizer2611->Add( m_geotypical_scale, 0, wxBOTTOM|wxRIGHT, 5 );
	
	m_staticText1501 = new wxStaticText( this, wxID_ANY, _("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1501->Wrap( -1 );
	bSizer2611->Add( m_staticText1501, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );
	
	
	bSizer247->Add( bSizer2611, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer26111;
	bSizer26111 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText14911 = new wxStaticText( this, wxID_ANY, _("Opacity:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText14911->Wrap( -1 );
	bSizer26111->Add( m_staticText14911, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_opacity = new wxTextCtrl( this, ID_OPACITY, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_opacity->SetMaxLength( 0 ); 
	m_opacity->SetMinSize( wxSize( 60,-1 ) );
	
	bSizer26111->Add( m_opacity, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );
	
	m_opacity_slider = new wxSlider( this, ID_OPACITY_SLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 100,-1 ), wxSL_HORIZONTAL );
	bSizer26111->Add( m_opacity_slider, 1, wxALL, 5 );
	
	
	bSizer247->Add( bSizer26111, 1, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	wxBoxSizer* bSizer84;
	bSizer84 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer84->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer84->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer247->Add( bSizer84, 1, wxALIGN_CENTER, 5 );
	
	
	this->SetSizer( bSizer247 );
	this->Layout();
	bSizer247->Fit( this );
	
	this->Centre( wxBOTH );
}

TinTextureDlgBase::~TinTextureDlgBase()
{
}

TParamsDlgBase::TParamsDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxVERTICAL );
	
	bSizer13->SetMinSize( wxSize( 650,-1 ) ); 
	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text2 = new wxStaticText( this, ID_TEXT, _("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text2->Wrap( 0 );
	bSizer14->Add( m_text2, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_tname = new wxTextCtrl( this, ID_TNAME, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_tname->SetMaxLength( 0 ); 
	bSizer14->Add( m_tname, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer13->Add( bSizer14, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_notebook = new wxNotebook( this, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, 0 );
	TParamsPanel1 = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer15;
	bSizer15 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel1, wxID_ANY, _("Grid") ), wxVERTICAL );
	
	wxBoxSizer* bSizer16;
	bSizer16 = new wxBoxSizer( wxHORIZONTAL );
	
	m_use_grid = new wxRadioButton( sbSizer3->GetStaticBox(), ID_USE_GRID, _("Grid Filename: "), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_use_grid->SetValue( true ); 
	bSizer16->Add( m_use_grid, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	m_filename = new wxComboBox( sbSizer3->GetStaticBox(), ID_FILENAME, wxEmptyString, wxDefaultPosition, wxSize( 180,-1 ), 0, NULL, wxCB_DROPDOWN|wxCB_SORT );
	m_filename->Append( _("Item") );
	bSizer16->Add( m_filename, 1, wxALIGN_CENTER|wxRIGHT|wxLEFT, 5 );
	
	m_primary_texture = new wxButton( sbSizer3->GetStaticBox(), ID_PRIMARY_TEXTURE, _("Primary Texture"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer16->Add( m_primary_texture, 0, wxALL, 5 );
	
	
	sbSizer3->Add( bSizer16, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer17;
	bSizer17 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text3 = new wxStaticText( sbSizer3->GetStaticBox(), ID_TEXT, _("Terrain LOD Method:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text3->Wrap( -1 );
	bSizer17->Add( m_text3, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_lodmethodChoices[] = { _("Item") };
	int m_lodmethodNChoices = sizeof( m_lodmethodChoices ) / sizeof( wxString );
	m_lodmethod = new wxChoice( sbSizer3->GetStaticBox(), ID_LODMETHOD, wxDefaultPosition, wxSize( 120,-1 ), m_lodmethodNChoices, m_lodmethodChoices, 0 );
	m_lodmethod->SetSelection( 0 );
	bSizer17->Add( m_lodmethod, 0, wxALIGN_CENTER|wxTOP|wxBOTTOM|wxRIGHT, 5 );
	
	m_text4 = new wxStaticText( sbSizer3->GetStaticBox(), ID_TEXT, _("Triangle count target: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text4->Wrap( -1 );
	bSizer17->Add( m_text4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_tri_count = new wxTextCtrl( sbSizer3->GetStaticBox(), ID_TRI_COUNT, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_tri_count->SetMaxLength( 0 ); 
	bSizer17->Add( m_tri_count, 0, wxALIGN_CENTER|wxTOP|wxBOTTOM|wxRIGHT, 5 );
	
	
	sbSizer3->Add( bSizer17, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer( wxHORIZONTAL );
	
	
	sbSizer3->Add( bSizer18, 0, wxALIGN_CENTER, 5 );
	
	
	bSizer15->Add( sbSizer3, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 10 );
	
	wxStaticBoxSizer* sbSizer4;
	sbSizer4 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel1, wxID_ANY, _("TIN") ), wxVERTICAL );
	
	wxBoxSizer* bSizer19;
	bSizer19 = new wxBoxSizer( wxHORIZONTAL );
	
	m_use_tin = new wxRadioButton( sbSizer4->GetStaticBox(), ID_USE_TIN, _("TIN Filename:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_use_tin->SetValue( true ); 
	bSizer19->Add( m_use_tin, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_filename_tin = new wxComboBox( sbSizer4->GetStaticBox(), ID_FILENAME_TIN, wxEmptyString, wxDefaultPosition, wxSize( 180,-1 ), 0, NULL, wxCB_DROPDOWN|wxCB_SORT );
	m_filename_tin->Append( _("Item") );
	bSizer19->Add( m_filename_tin, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_set_tin_texture = new wxButton( sbSizer4->GetStaticBox(), ID_SET_TIN_TEXTURE, _("Set Texture"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer19->Add( m_set_tin_texture, 0, wxALL, 5 );
	
	
	sbSizer4->Add( bSizer19, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );
	
	
	bSizer15->Add( sbSizer4, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 10 );
	
	wxStaticBoxSizer* sbSizer5;
	sbSizer5 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel1, wxID_ANY, _("Tileset") ), wxVERTICAL );
	
	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxHORIZONTAL );
	
	m_use_tileset = new wxRadioButton( sbSizer5->GetStaticBox(), ID_USE_TILESET, _("Tileset .ini file:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_use_tileset->SetValue( true ); 
	bSizer20->Add( m_use_tileset, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	m_filename_tileset = new wxComboBox( sbSizer5->GetStaticBox(), ID_FILENAME_TILES, wxEmptyString, wxDefaultPosition, wxSize( 180,-1 ), 0, NULL, wxCB_DROPDOWN|wxCB_SORT );
	m_filename_tileset->Append( _("Item") );
	bSizer20->Add( m_filename_tileset, 1, wxALIGN_CENTER|wxRIGHT|wxLEFT, 5 );
	
	
	sbSizer5->Add( bSizer20, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5 );
	
	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text5 = new wxStaticText( sbSizer5->GetStaticBox(), ID_TEXT, _("Vertex count target: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text5->Wrap( -1 );
	bSizer21->Add( m_text5, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_vtx_count = new wxTextCtrl( sbSizer5->GetStaticBox(), ID_VTX_COUNT, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_vtx_count->SetMaxLength( 0 ); 
	bSizer21->Add( m_vtx_count, 0, wxALIGN_CENTER|wxTOP|wxBOTTOM|wxRIGHT, 5 );
	
	m_tile_threading = new wxCheckBox( sbSizer5->GetStaticBox(), ID_TILE_THREADING, _("Use multithreading for asynchronous tile loading"), wxDefaultPosition, wxDefaultSize, 0 );
	m_tile_threading->SetValue(true); 
	bSizer21->Add( m_tile_threading, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer5->Add( bSizer21, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer271;
	bSizer271 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer271->Add( 20, 0, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticText1441 = new wxStaticText( sbSizer5->GetStaticBox(), wxID_ANY, _("Texture tileset:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1441->Wrap( -1 );
	bSizer271->Add( m_staticText1441, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_tfile_tileset = new wxComboBox( sbSizer5->GetStaticBox(), ID_TFILE_TILESET, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN|wxCB_SORT );
	m_tfile_tileset->Append( _("Item") );
	bSizer271->Add( m_tfile_tileset, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer5->Add( bSizer271, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer291;
	bSizer291 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer291->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	id_text31 = new wxStaticText( sbSizer5->GetStaticBox(), ID_TEXT, _("Texture LOD factor:"), wxDefaultPosition, wxDefaultSize, 0 );
	id_text31->Wrap( -1 );
	bSizer291->Add( id_text31, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_tex_lod = new wxTextCtrl( sbSizer5->GetStaticBox(), ID_TEX_LOD, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_tex_lod->SetMaxLength( 0 ); 
	bSizer291->Add( m_tex_lod, 0, wxALIGN_CENTER, 5 );
	
	
	bSizer291->Add( 20, 0, 0, wxALIGN_CENTER, 5 );
	
	m_texture_gradual = new wxCheckBox( sbSizer5->GetStaticBox(), ID_TEXTURE_GRADUAL, _("Start with minimal texture tiles and load gradually"), wxDefaultPosition, wxDefaultSize, 0 );
	m_texture_gradual->SetValue(true); 
	bSizer291->Add( m_texture_gradual, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer5->Add( bSizer291, 1, wxEXPAND, 5 );
	
	
	bSizer15->Add( sbSizer5, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 10 );
	
	wxStaticBoxSizer* sbSizer6;
	sbSizer6 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel1, wxID_ANY, _("External") ), wxVERTICAL );
	
	wxBoxSizer* bSizer22;
	bSizer22 = new wxBoxSizer( wxHORIZONTAL );
	
	m_use_external = new wxRadioButton( sbSizer6->GetStaticBox(), ID_USE_EXTERNAL, _("External Data:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_use_external->SetValue( true ); 
	bSizer22->Add( m_use_external, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_tt_external_data = new wxTextCtrl( sbSizer6->GetStaticBox(), ID_TT_EXTERNAL_DATA, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_tt_external_data->SetMaxLength( 0 ); 
	bSizer22->Add( m_tt_external_data, 1, wxALIGN_CENTER|wxRIGHT|wxLEFT, 5 );
	
	
	sbSizer6->Add( bSizer22, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer15->Add( sbSizer6, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 10 );
	
	wxBoxSizer* bSizer23;
	bSizer23 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text7 = new wxStaticText( TParamsPanel1, ID_TEXT, _("Vertical Exaggeration:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text7->Wrap( -1 );
	bSizer23->Add( m_text7, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_vertexag = new wxTextCtrl( TParamsPanel1, ID_VERTEXAG, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_vertexag->SetMaxLength( 0 ); 
	bSizer23->Add( m_vertexag, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text8 = new wxStaticText( TParamsPanel1, ID_TEXT, _("x"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text8->Wrap( 0 );
	bSizer23->Add( m_text8, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	bSizer15->Add( bSizer23, 0, wxALIGN_CENTER|wxRIGHT|wxLEFT, 5 );
	
	
	TParamsPanel1->SetSizer( bSizer15 );
	TParamsPanel1->Layout();
	bSizer15->Fit( TParamsPanel1 );
	m_notebook->AddPage( TParamsPanel1, _("Primary Elevation"), true );
	TParamsPanelPlus = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer241;
	bSizer241 = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer71;
	sbSizer71 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanelPlus, wxID_ANY, _("Elevation Layers") ), wxHORIZONTAL );
	
	m_elev_files = new wxListBox( sbSizer71->GetStaticBox(), ID_ELEVFILES, wxDefaultPosition, wxSize( 80,90 ), 0, NULL, wxLB_SINGLE ); 
	m_elev_files->SetMinSize( wxSize( -1,100 ) );
	
	sbSizer71->Add( m_elev_files, 1, wxALL, 5 );
	
	m_set_texture = new wxButton( sbSizer71->GetStaticBox(), ID_SET_TEXTURE, _("Set Texture"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer71->Add( m_set_texture, 0, wxALL, 5 );
	
	
	bSizer241->Add( sbSizer71, 2, wxALL, 5 );
	
	
	TParamsPanelPlus->SetSizer( bSizer241 );
	TParamsPanelPlus->Layout();
	bSizer241->Fit( TParamsPanelPlus );
	m_notebook->AddPage( TParamsPanelPlus, _("Extra"), false );
	TParamsPanel3 = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer38;
	bSizer38 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer39;
	bSizer39 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer38;
	sbSizer38 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel3, wxID_ANY, _("Plant Layers") ), wxVERTICAL );
	
	m_plant_files = new wxListBox( sbSizer38->GetStaticBox(), ID_PLANTFILES, wxDefaultPosition, wxSize( 80,90 ), 0, NULL, wxLB_SINGLE ); 
	sbSizer38->Add( m_plant_files, 0, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer41;
	bSizer41 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text14 = new wxStaticText( sbSizer38->GetStaticBox(), ID_TEXT, _("Visibility distance:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text14->Wrap( -1 );
	bSizer41->Add( m_text14, 0, wxALIGN_CENTER|wxALL, 5 );
	
	mmegdistance = new wxTextCtrl( sbSizer38->GetStaticBox(), ID_VEGDISTANCE, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
	mmegdistance->SetMaxLength( 0 ); 
	bSizer41->Add( mmegdistance, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_text15 = new wxStaticText( sbSizer38->GetStaticBox(), ID_TEXT, _("m"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text15->Wrap( 0 );
	bSizer41->Add( m_text15, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer38->Add( bSizer41, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_checkBox64 = new wxCheckBox( sbSizer38->GetStaticBox(), ID_TREES_USE_SHADERS, _("Use Shaders for Plants"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer38->Add( m_checkBox64, 0, wxALL, 5 );
	
	
	bSizer39->Add( sbSizer38, 0, wxEXPAND|wxALL, 5 );
	
	
	bSizer38->Add( bSizer39, 1, wxALIGN_CENTER_HORIZONTAL, 5 );
	
	wxBoxSizer* bSizer47;
	bSizer47 = new wxBoxSizer( wxVERTICAL );
	
	id_roads = new wxCheckBox( TParamsPanel3, ID_ROADS, _("Roads"), wxDefaultPosition, wxDefaultSize, 0 );
	id_roads->SetValue(true); 
	bSizer47->Add( id_roads, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxStaticBoxSizer* sbSizer10;
	sbSizer10 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel3, wxID_ANY, wxEmptyString ), wxVERTICAL );
	
	wxBoxSizer* bSizer42;
	bSizer42 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text16 = new wxStaticText( sbSizer10->GetStaticBox(), ID_TEXT, _("Source file: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text16->Wrap( -1 );
	bSizer42->Add( m_text16, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_roadfile = new wxComboBox( sbSizer10->GetStaticBox(), ID_ROADFILE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN|wxCB_SORT ); 
	bSizer42->Add( m_roadfile, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer10->Add( bSizer42, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer43;
	bSizer43 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer44;
	bSizer44 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_text214 = new wxStaticText( sbSizer10->GetStaticBox(), ID_TEXT, _("Ground offset: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text214->Wrap( -1 );
	fgSizer2->Add( m_text214, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_roadheight = new wxTextCtrl( sbSizer10->GetStaticBox(), ID_ROADHEIGHT, wxEmptyString, wxDefaultPosition, wxSize( 40,-1 ), 0 );
	m_roadheight->SetMaxLength( 0 ); 
	fgSizer2->Add( m_roadheight, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_text17 = new wxStaticText( sbSizer10->GetStaticBox(), ID_TEXT, _("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text17->Wrap( 0 );
	fgSizer2->Add( m_text17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	id_text4 = new wxStaticText( sbSizer10->GetStaticBox(), ID_TEXT, _("Visibility distance:"), wxDefaultPosition, wxDefaultSize, 0 );
	id_text4->Wrap( -1 );
	fgSizer2->Add( id_text4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_roaddistance = new wxTextCtrl( sbSizer10->GetStaticBox(), ID_ROADDISTANCE, wxEmptyString, wxDefaultPosition, wxSize( 40,-1 ), 0 );
	m_roaddistance->SetMaxLength( 0 ); 
	fgSizer2->Add( m_roaddistance, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_text18 = new wxStaticText( sbSizer10->GetStaticBox(), ID_TEXT, _("km"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text18->Wrap( 0 );
	fgSizer2->Add( m_text18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer44->Add( fgSizer2, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_texroads = new wxCheckBox( sbSizer10->GetStaticBox(), ID_TEXROADS, _("Texture-map roads"), wxDefaultPosition, wxDefaultSize, 0 );
	m_texroads->SetValue(true); 
	bSizer44->Add( m_texroads, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_roadculture = new wxCheckBox( sbSizer10->GetStaticBox(), ID_ROADCULTURE, _("Road culture (stoplights, etc.)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_roadculture->SetValue(true); 
	bSizer44->Add( m_roadculture, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer43->Add( bSizer44, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	sbSizer10->Add( bSizer43, 0, wxALIGN_CENTER|wxALL, 0 );
	
	wxBoxSizer* bSizer45;
	bSizer45 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text215 = new wxStaticText( sbSizer10->GetStaticBox(), ID_TEXT, _("Include:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text215->Wrap( 0 );
	bSizer45->Add( m_text215, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_highways = new wxCheckBox( sbSizer10->GetStaticBox(), ID_HIGHWAYS, _("Highways"), wxDefaultPosition, wxDefaultSize, 0 );
	m_highways->SetValue(true); 
	bSizer45->Add( m_highways, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	id_paved = new wxCheckBox( sbSizer10->GetStaticBox(), ID_PAVED, _("Paved"), wxDefaultPosition, wxDefaultSize, 0 );
	id_paved->SetValue(true); 
	bSizer45->Add( id_paved, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_dirt = new wxCheckBox( sbSizer10->GetStaticBox(), ID_DIRT, _("Dirt"), wxDefaultPosition, wxDefaultSize, 0 );
	m_dirt->SetValue(true); 
	bSizer45->Add( m_dirt, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	
	sbSizer10->Add( bSizer45, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer47->Add( sbSizer10, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer38->Add( bSizer47, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 0 );
	
	
	TParamsPanel3->SetSizer( bSizer38 );
	TParamsPanel3->Layout();
	bSizer38->Fit( TParamsPanel3 );
	m_notebook->AddPage( TParamsPanel3, _("Plants and Roads"), false );
	TParamsPanel8 = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer202;
	bSizer202 = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer11;
	sbSizer11 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel8, wxID_ANY, _("Structure Layers") ), wxHORIZONTAL );
	
	wxBoxSizer* bSizer203;
	bSizer203 = new wxBoxSizer( wxVERTICAL );
	
	m_structure_files = new wxListBox( sbSizer11->GetStaticBox(), ID_STRUCTFILES, wxDefaultPosition, wxSize( 80,90 ), 0, NULL, wxLB_SINGLE ); 
	bSizer203->Add( m_structure_files, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer48;
	bSizer48 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text19 = new wxStaticText( sbSizer11->GetStaticBox(), ID_TEXT, _("Visibility distance:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text19->Wrap( -1 );
	bSizer48->Add( m_text19, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_struct_distance = new wxTextCtrl( sbSizer11->GetStaticBox(), ID_STRUCT_DISTANCE, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), 0 );
	m_struct_distance->SetMaxLength( 0 ); 
	bSizer48->Add( m_struct_distance, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_text20 = new wxStaticText( sbSizer11->GetStaticBox(), ID_TEXT, _("m"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text20->Wrap( 0 );
	bSizer48->Add( m_text20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer203->Add( bSizer48, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer46;
	bSizer46 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text216 = new wxStaticText( sbSizer11->GetStaticBox(), ID_TEXT, _("Terrain-specific content:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text216->Wrap( -1 );
	bSizer46->Add( m_text216, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_content_file = new wxComboBox( sbSizer11->GetStaticBox(), ID_CONTENT_FILE, wxEmptyString, wxDefaultPosition, wxSize( 100,-1 ), 0, NULL, wxCB_DROPDOWN ); 
	bSizer46->Add( m_content_file, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer203->Add( bSizer46, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer11->Add( bSizer203, 1, 0, 5 );
	
	wxBoxSizer* bSizer204;
	bSizer204 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer49;
	bSizer49 = new wxBoxSizer( wxVERTICAL );
	
	mmheck_structure_shadows = new wxCheckBox( sbSizer11->GetStaticBox(), ID_CHECK_STRUCTURE_SHADOWS, _("Shadows"), wxDefaultPosition, wxDefaultSize, 0 );
	mmheck_structure_shadows->SetValue(true); 
	bSizer49->Add( mmheck_structure_shadows, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer50;
	bSizer50 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer50->Add( 20, 20, 0, wxALIGN_CENTER|wxLEFT, 5 );
	
	wxBoxSizer* bSizer51;
	bSizer51 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer52;
	bSizer52 = new wxBoxSizer( wxHORIZONTAL );
	
	id_text5 = new wxStaticText( sbSizer11->GetStaticBox(), ID_TEXT, _("Resolution:"), wxDefaultPosition, wxDefaultSize, 0 );
	id_text5->Wrap( -1 );
	bSizer52->Add( id_text5, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_choice_shadow_rezChoices;
	m_choice_shadow_rez = new wxChoice( sbSizer11->GetStaticBox(), ID_CHOICE_SHADOW_REZ, wxDefaultPosition, wxSize( 100,-1 ), m_choice_shadow_rezChoices, 0 );
	m_choice_shadow_rez->SetSelection( 0 );
	bSizer52->Add( m_choice_shadow_rez, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer51->Add( bSizer52, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer53;
	bSizer53 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text21 = new wxStaticText( sbSizer11->GetStaticBox(), ID_TEXT, _("Darkness:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text21->Wrap( -1 );
	bSizer53->Add( m_text21, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_darkness = new wxTextCtrl( sbSizer11->GetStaticBox(), ID_DARKNESS, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_darkness->SetMaxLength( 0 ); 
	bSizer53->Add( m_darkness, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer51->Add( bSizer53, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_shadows_default_on = new wxCheckBox( sbSizer11->GetStaticBox(), ID_SHADOWS_DEFAULT_ON, _("All structures cast shadows by default"), wxDefaultPosition, wxDefaultSize, 0 );
	m_shadows_default_on->SetValue(true); 
	bSizer51->Add( m_shadows_default_on, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_shadows_every_frame = new wxCheckBox( sbSizer11->GetStaticBox(), ID_SHADOWS_EVERY_FRAME, _("Recompute shadows every frame"), wxDefaultPosition, wxDefaultSize, 0 );
	m_shadows_every_frame->SetValue(true); 
	bSizer51->Add( m_shadows_every_frame, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer54;
	bSizer54 = new wxBoxSizer( wxHORIZONTAL );
	
	id_shadow_limit = new wxCheckBox( sbSizer11->GetStaticBox(), ID_SHADOW_LIMIT, _("Limit shadow area:"), wxDefaultPosition, wxDefaultSize, 0 );
	id_shadow_limit->SetValue(true); 
	bSizer54->Add( id_shadow_limit, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_shadow_limit_radius = new wxTextCtrl( sbSizer11->GetStaticBox(), ID_SHADOW_LIMIT_RADIUS, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), 0 );
	m_shadow_limit_radius->SetMaxLength( 0 ); 
	bSizer54->Add( m_shadow_limit_radius, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_text22 = new wxStaticText( sbSizer11->GetStaticBox(), ID_TEXT, _("m"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text22->Wrap( 0 );
	bSizer54->Add( m_text22, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer51->Add( bSizer54, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer50->Add( bSizer51, 0, wxALIGN_CENTER, 5 );
	
	
	bSizer49->Add( bSizer50, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_check_structure_paging = new wxCheckBox( sbSizer11->GetStaticBox(), ID_CHECK_STRUCTURE_PAGING, _("Paging"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_structure_paging->SetValue(true); 
	bSizer49->Add( m_check_structure_paging, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer55;
	bSizer55 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer55->Add( 15, 10, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text23 = new wxStaticText( sbSizer11->GetStaticBox(), ID_TEXT, _("Maximum structures:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text23->Wrap( -1 );
	bSizer55->Add( m_text23, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_paging_max_structures = new wxTextCtrl( sbSizer11->GetStaticBox(), ID_PAGING_MAX_STRUCTURES, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_paging_max_structures->SetMaxLength( 0 ); 
	bSizer55->Add( m_paging_max_structures, 1, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer49->Add( bSizer55, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer56;
	bSizer56 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer56->Add( 15, 10, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text24 = new wxStaticText( sbSizer11->GetStaticBox(), ID_TEXT, _("Page-out distance:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text24->Wrap( -1 );
	bSizer56->Add( m_text24, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_page_out_distance = new wxTextCtrl( sbSizer11->GetStaticBox(), ID_PAGE_OUT_DISTANCE, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_page_out_distance->SetMaxLength( 0 ); 
	bSizer56->Add( m_page_out_distance, 1, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	m_text25 = new wxStaticText( sbSizer11->GetStaticBox(), ID_TEXT, _("m"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text25->Wrap( 0 );
	bSizer56->Add( m_text25, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer49->Add( bSizer56, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer204->Add( bSizer49, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	sbSizer11->Add( bSizer204, 0, 0, 5 );
	
	
	bSizer202->Add( sbSizer11, 1, wxALL|wxEXPAND, 5 );
	
	
	TParamsPanel8->SetSizer( bSizer202 );
	TParamsPanel8->Layout();
	bSizer202->Fit( TParamsPanel8 );
	m_notebook->AddPage( TParamsPanel8, _("Structures"), false );
	TParamsPanel5 = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer66;
	bSizer66 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer14;
	sbSizer14 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel5, wxID_ANY, _("Abstract Layers") ), wxHORIZONTAL );
	
	m_raw_files = new wxListBox( sbSizer14->GetStaticBox(), ID_RAWFILES, wxDefaultPosition, wxSize( 180,120 ), 0, NULL, wxLB_SINGLE ); 
	sbSizer14->Add( m_raw_files, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer67;
	bSizer67 = new wxBoxSizer( wxVERTICAL );
	
	m_style = new wxButton( sbSizer14->GetStaticBox(), ID_STYLE, _("Style..."), wxDefaultPosition, wxDefaultSize, 0 );
	m_style->SetDefault(); 
	bSizer67->Add( m_style, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer14->Add( bSizer67, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0 );
	
	
	bSizer66->Add( sbSizer14, 3, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer15;
	sbSizer15 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel5, wxID_ANY, _("Image Layers") ), wxHORIZONTAL );
	
	m_image_files = new wxListBox( sbSizer15->GetStaticBox(), ID_IMAGEFILES, wxDefaultPosition, wxSize( 180,-1 ), 0, NULL, wxLB_SINGLE ); 
	sbSizer15->Add( m_image_files, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer66->Add( sbSizer15, 2, wxEXPAND|wxALL, 5 );
	
	
	TParamsPanel5->SetSizer( bSizer66 );
	TParamsPanel5->Layout();
	bSizer66->Fit( TParamsPanel5 );
	m_notebook->AddPage( TParamsPanel5, _("Abstracts and Images"), false );
	TParamsPanel4 = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer57;
	bSizer57 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer57->Add( 5, 20, 1, wxALIGN_CENTER|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer12;
	sbSizer12 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel4, wxID_ANY, _("Sky && Water") ), wxVERTICAL );
	
	m_oceanplane = new wxCheckBox( sbSizer12->GetStaticBox(), ID_OCEANPLANE, _("Ocean plane"), wxDefaultPosition, wxDefaultSize, 0 );
	m_oceanplane->SetValue(true); 
	sbSizer12->Add( m_oceanplane, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer58;
	bSizer58 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer58->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_text217 = new wxStaticText( sbSizer12->GetStaticBox(), ID_TEXT, _("Level (meters)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text217->Wrap( -1 );
	bSizer58->Add( m_text217, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_oceanplaneoffset = new wxTextCtrl( sbSizer12->GetStaticBox(), ID_OCEANPLANEOFFSET, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_oceanplaneoffset->SetMaxLength( 0 ); 
	bSizer58->Add( m_oceanplaneoffset, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	sbSizer12->Add( bSizer58, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_water = new wxCheckBox( sbSizer12->GetStaticBox(), ID_WATER, _("Water surface"), wxDefaultPosition, wxDefaultSize, 0 );
	m_water->SetValue(true); 
	sbSizer12->Add( m_water, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_filename_water = new wxComboBox( sbSizer12->GetStaticBox(), ID_FILENAME_WATER, wxEmptyString, wxDefaultPosition, wxSize( 180,-1 ), 0, NULL, wxCB_DROPDOWN|wxCB_SORT );
	m_filename_water->Append( _("Item") );
	sbSizer12->Add( m_filename_water, 1, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	m_depressocean = new wxCheckBox( sbSizer12->GetStaticBox(), ID_DEPRESSOCEAN, _("Depress Ocean"), wxDefaultPosition, wxDefaultSize, 0 );
	m_depressocean->SetValue(true); 
	sbSizer12->Add( m_depressocean, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer59;
	bSizer59 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer59->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_text218 = new wxStaticText( sbSizer12->GetStaticBox(), ID_TEXT, _("Level (meters)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text218->Wrap( -1 );
	bSizer59->Add( m_text218, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_depressoceanoffset = new wxTextCtrl( sbSizer12->GetStaticBox(), ID_DEPRESSOCEANOFFSET, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_depressoceanoffset->SetMaxLength( 0 ); 
	bSizer59->Add( m_depressoceanoffset, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	sbSizer12->Add( bSizer59, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 0 );
	
	id_sky = new wxCheckBox( sbSizer12->GetStaticBox(), ID_SKY, _("Sky dome"), wxDefaultPosition, wxDefaultSize, 0 );
	id_sky->SetValue(true); 
	sbSizer12->Add( id_sky, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer60;
	bSizer60 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer60->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	m_skytexture = new wxComboBox( sbSizer12->GetStaticBox(), ID_SKYTEXTURE, wxEmptyString, wxDefaultPosition, wxSize( 200,-1 ), 0, NULL, wxCB_DROPDOWN ); 
	bSizer60->Add( m_skytexture, 1, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	sbSizer12->Add( bSizer60, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 0 );
	
	wxBoxSizer* bSizer61;
	bSizer61 = new wxBoxSizer( wxHORIZONTAL );
	
	mmog = new wxCheckBox( sbSizer12->GetStaticBox(), ID_FOG, _("Fog"), wxDefaultPosition, wxDefaultSize, 0 );
	mmog->SetValue(true); 
	bSizer61->Add( mmog, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text26 = new wxStaticText( sbSizer12->GetStaticBox(), ID_TEXT, _("Distance (km)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text26->Wrap( -1 );
	bSizer61->Add( m_text26, 0, wxALIGN_CENTER|wxALL, 5 );
	
	id_fog_distance = new wxTextCtrl( sbSizer12->GetStaticBox(), ID_FOG_DISTANCE, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	id_fog_distance->SetMaxLength( 0 ); 
	bSizer61->Add( id_fog_distance, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer12->Add( bSizer61, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer62;
	bSizer62 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text27 = new wxStaticText( sbSizer12->GetStaticBox(), ID_TEXT, _("Scene background color:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text27->Wrap( -1 );
	bSizer62->Add( m_text27, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_color3 = new wxStaticBitmap( sbSizer12->GetStaticBox(), ID_COLOR3, wxBitmap( dummy_32x18_xpm ), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer62->Add( m_color3, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_bgcolor = new wxButton( sbSizer12->GetStaticBox(), ID_BGCOLOR, _("Set"), wxDefaultPosition, wxDefaultSize, 0 );
	m_bgcolor->SetDefault(); 
	bSizer62->Add( m_bgcolor, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer12->Add( bSizer62, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer57->Add( sbSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxStaticBoxSizer* sbSizer13;
	sbSizer13 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel4, wxID_ANY, _("Time") ), wxVERTICAL );
	
	wxBoxSizer* bSizer63;
	bSizer63 = new wxBoxSizer( wxVERTICAL );
	
	m_text219 = new wxStaticText( sbSizer13->GetStaticBox(), ID_TEXT, _("Initial Time: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text219->Wrap( -1 );
	bSizer63->Add( m_text219, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text_init_time = new wxTextCtrl( sbSizer13->GetStaticBox(), ID_TEXT_INIT_TIME, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), wxTE_READONLY );
	m_text_init_time->SetMaxLength( 0 ); 
	bSizer63->Add( m_text_init_time, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	id_set_init_time = new wxButton( sbSizer13->GetStaticBox(), ID_SET_INIT_TIME, _("Set"), wxDefaultPosition, wxDefaultSize, 0 );
	id_set_init_time->SetDefault(); 
	bSizer63->Add( id_set_init_time, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer13->Add( bSizer63, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer64;
	bSizer64 = new wxBoxSizer( wxHORIZONTAL );
	
	m_timemoves = new wxCheckBox( sbSizer13->GetStaticBox(), ID_TIMEMOVES, _("Time Moves"), wxDefaultPosition, wxDefaultSize, 0 );
	m_timemoves->SetValue(true); 
	bSizer64->Add( m_timemoves, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer13->Add( bSizer64, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer65;
	bSizer65 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text28 = new wxStaticText( sbSizer13->GetStaticBox(), ID_TEXT, _("Faster than real: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text28->Wrap( -1 );
	bSizer65->Add( m_text28, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_timespeed = new wxTextCtrl( sbSizer13->GetStaticBox(), ID_TIMESPEED, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_timespeed->SetMaxLength( 0 ); 
	bSizer65->Add( m_timespeed, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_text29 = new wxStaticText( sbSizer13->GetStaticBox(), ID_TEXT, _("x"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text29->Wrap( 0 );
	bSizer65->Add( m_text29, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer13->Add( bSizer65, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	
	bSizer57->Add( sbSizer13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	
	bSizer57->Add( 5, 20, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	TParamsPanel4->SetSizer( bSizer57 );
	TParamsPanel4->Layout();
	bSizer57->Fit( TParamsPanel4 );
	m_notebook->AddPage( TParamsPanel4, _("Ephemeris"), false );
	TParamsPanel6 = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer68;
	bSizer68 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer69;
	bSizer69 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer16;
	sbSizer16 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel6, wxID_ANY, _("Window Overlay") ), wxVERTICAL );
	
	id_text6 = new wxStaticText( sbSizer16->GetStaticBox(), ID_TEXT, _("Image file:"), wxDefaultPosition, wxDefaultSize, 0 );
	id_text6->Wrap( -1 );
	sbSizer16->Add( id_text6, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxBoxSizer* bSizer70;
	bSizer70 = new wxBoxSizer( wxHORIZONTAL );
	
	m_overlay_file = new wxTextCtrl( sbSizer16->GetStaticBox(), ID_OVERLAY_FILE, wxEmptyString, wxDefaultPosition, wxSize( 200,-1 ), 0 );
	m_overlay_file->SetMaxLength( 0 ); 
	bSizer70->Add( m_overlay_file, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_overlay_dotdotdot = new wxButton( sbSizer16->GetStaticBox(), ID_OVERLAY_DOTDOTDOT, _("..."), wxDefaultPosition, wxSize( 22,-1 ), 0 );
	m_overlay_dotdotdot->SetDefault(); 
	bSizer70->Add( m_overlay_dotdotdot, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer16->Add( bSizer70, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer71;
	bSizer71 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text30 = new wxStaticText( sbSizer16->GetStaticBox(), ID_TEXT, _("Placement:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text30->Wrap( 0 );
	bSizer71->Add( m_text30, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_overlay_x = new wxTextCtrl( sbSizer16->GetStaticBox(), ID_OVERLAY_X, wxEmptyString, wxDefaultPosition, wxSize( 44,-1 ), 0 );
	m_overlay_x->SetMaxLength( 0 ); 
	bSizer71->Add( m_overlay_x, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_overlay_y = new wxTextCtrl( sbSizer16->GetStaticBox(), ID_OVERLAY_Y, wxEmptyString, wxDefaultPosition, wxSize( 44,-1 ), 0 );
	m_overlay_y->SetMaxLength( 0 ); 
	bSizer71->Add( m_overlay_y, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer16->Add( bSizer71, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_text31 = new wxStaticText( sbSizer16->GetStaticBox(), ID_TEXT, _("(pixels from the lower-left corner)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text31->Wrap( -1 );
	sbSizer16->Add( m_text31, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer69->Add( sbSizer16, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	m_check_overview = new wxCheckBox( TParamsPanel6, ID_CHECK_OVERVIEW, _("Terrain overview map"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_overview->SetValue(true); 
	bSizer69->Add( m_check_overview, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_check_compass = new wxCheckBox( TParamsPanel6, ID_CHECK_COMPASS, _("Compass"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_compass->SetValue(true); 
	bSizer69->Add( m_check_compass, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer68->Add( bSizer69, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	TParamsPanel6->SetSizer( bSizer68 );
	TParamsPanel6->Layout();
	bSizer68->Fit( TParamsPanel6 );
	m_notebook->AddPage( TParamsPanel6, _("HUD"), false );
	TParamsPanel7 = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer72;
	bSizer72 = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer17;
	sbSizer17 = new wxStaticBoxSizer( new wxStaticBox( TParamsPanel7, wxID_ANY, _("Navigation") ), wxVERTICAL );
	
	wxBoxSizer* bSizer73;
	bSizer73 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text32 = new wxStaticText( sbSizer17->GetStaticBox(), ID_TEXT, _("Style:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text32->Wrap( 0 );
	bSizer73->Add( m_text32, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_nav_styleChoices[] = { _("Item") };
	int m_nav_styleNChoices = sizeof( m_nav_styleChoices ) / sizeof( wxString );
	m_nav_style = new wxChoice( sbSizer17->GetStaticBox(), ID_NAV_STYLE, wxDefaultPosition, wxSize( 100,-1 ), m_nav_styleNChoices, m_nav_styleChoices, 0 );
	m_nav_style->SetSelection( 0 );
	bSizer73->Add( m_nav_style, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer17->Add( bSizer73, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer74;
	bSizer74 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer74->Add( 20, 0, 1, wxEXPAND, 5 );
	
	m_text33 = new wxStaticText( sbSizer17->GetStaticBox(), ID_TEXT, _("Minimum height above ground: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_text33->Wrap( -1 );
	bSizer74->Add( m_text33, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_minheight = new wxTextCtrl( sbSizer17->GetStaticBox(), ID_MINHEIGHT, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_minheight->SetMaxLength( 0 ); 
	bSizer74->Add( m_minheight, 0, wxALIGN_CENTER|wxTOP|wxBOTTOM|wxRIGHT, 5 );
	
	m_text34 = new wxStaticText( sbSizer17->GetStaticBox(), ID_TEXT, _("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text34->Wrap( 0 );
	bSizer74->Add( m_text34, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	sbSizer17->Add( bSizer74, 0, wxALIGN_LEFT, 5 );
	
	wxBoxSizer* bSizer75;
	bSizer75 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer75->Add( 20, 0, 1, wxEXPAND, 5 );
	
	id_text7 = new wxStaticText( sbSizer17->GetStaticBox(), ID_TEXT, _("Navigation speed:"), wxDefaultPosition, wxDefaultSize, 0 );
	id_text7->Wrap( -1 );
	bSizer75->Add( id_text7, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_navspeed = new wxTextCtrl( sbSizer17->GetStaticBox(), ID_NAVSPEED, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_navspeed->SetMaxLength( 0 ); 
	bSizer75->Add( m_navspeed, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_text35 = new wxStaticText( sbSizer17->GetStaticBox(), ID_TEXT, _("meters/frame"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text35->Wrap( 0 );
	bSizer75->Add( m_text35, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	sbSizer17->Add( bSizer75, 0, wxALIGN_LEFT, 5 );
	
	wxBoxSizer* bSizer751;
	bSizer751 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer751->Add( 20, 0, 1, wxEXPAND, 5 );
	
	id_text71 = new wxStaticText( sbSizer17->GetStaticBox(), ID_TEXT, _("Velocity damping:"), wxDefaultPosition, wxDefaultSize, 0 );
	id_text71->Wrap( -1 );
	bSizer751->Add( id_text71, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_damping = new wxTextCtrl( sbSizer17->GetStaticBox(), ID_DAMPING, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_damping->SetMaxLength( 0 ); 
	bSizer751->Add( m_damping, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	
	sbSizer17->Add( bSizer751, 0, wxALIGN_LEFT, 5 );
	
	wxBoxSizer* bSizer76;
	bSizer76 = new wxBoxSizer( wxVERTICAL );
	
	m_accel = new wxCheckBox( sbSizer17->GetStaticBox(), ID_ACCEL, _("Accelerate by height above ground"), wxDefaultPosition, wxDefaultSize, 0 );
	m_accel->SetValue(true); 
	bSizer76->Add( m_accel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_allow_roll = new wxCheckBox( sbSizer17->GetStaticBox(), ID_ALLOW_ROLL, _("Allow Roll"), wxDefaultPosition, wxDefaultSize, 0 );
	m_allow_roll->SetValue(true); 
	bSizer76->Add( m_allow_roll, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	sbSizer17->Add( bSizer76, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer77;
	bSizer77 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text36 = new wxStaticText( sbSizer17->GetStaticBox(), ID_TEXT, _("Default Locations File:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text36->Wrap( -1 );
	bSizer77->Add( m_text36, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_locfile = new wxComboBox( sbSizer17->GetStaticBox(), ID_LOCFILE, wxEmptyString, wxDefaultPosition, wxSize( 140,-1 ), 0, NULL, wxCB_DROPDOWN|wxCB_SORT ); 
	bSizer77->Add( m_locfile, 1, wxALIGN_CENTER|wxRIGHT|wxBOTTOM, 5 );
	
	
	sbSizer17->Add( bSizer77, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer78;
	bSizer78 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text37 = new wxStaticText( sbSizer17->GetStaticBox(), ID_TEXT, _("Initial Camera Location"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text37->Wrap( -1 );
	bSizer78->Add( m_text37, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_init_locationChoices[] = { _("Item") };
	int m_init_locationNChoices = sizeof( m_init_locationChoices ) / sizeof( wxString );
	m_init_location = new wxChoice( sbSizer17->GetStaticBox(), ID_INIT_LOCATION, wxDefaultPosition, wxSize( 140,-1 ), m_init_locationNChoices, m_init_locationChoices, 0 );
	m_init_location->SetSelection( 0 );
	bSizer78->Add( m_init_location, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	
	sbSizer17->Add( bSizer78, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer79;
	bSizer79 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text38 = new wxStaticText( sbSizer17->GetStaticBox(), ID_TEXT, _("Near clipping (\"Hither\") distance"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text38->Wrap( -1 );
	bSizer79->Add( m_text38, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_hither = new wxTextCtrl( sbSizer17->GetStaticBox(), ID_HITHER, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_hither->SetMaxLength( 0 ); 
	bSizer79->Add( m_hither, 0, wxALIGN_CENTER|wxBOTTOM, 5 );
	
	m_text39 = new wxStaticText( sbSizer17->GetStaticBox(), ID_TEXT, _("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text39->Wrap( 0 );
	bSizer79->Add( m_text39, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer17->Add( bSizer79, 0, wxALIGN_CENTER, 5 );
	
	
	bSizer72->Add( sbSizer17, 0, wxEXPAND|wxALL, 10 );
	
	wxBoxSizer* bSizer205;
	bSizer205 = new wxBoxSizer( wxVERTICAL );
	
	m_text220 = new wxStaticText( TParamsPanel7, ID_TEXT, _("Animation Paths:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text220->Wrap( -1 );
	bSizer205->Add( m_text220, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_anim_paths = new wxListBox( TParamsPanel7, ID_ANIM_PATHS, wxDefaultPosition, wxSize( 80,160 ), 0, NULL, wxLB_SINGLE ); 
	bSizer205->Add( m_anim_paths, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
	
	
	bSizer72->Add( bSizer205, 1, wxEXPAND|wxRIGHT, 5 );
	
	
	TParamsPanel7->SetSizer( bSizer72 );
	TParamsPanel7->Layout();
	bSizer72->Fit( TParamsPanel7 );
	m_notebook->AddPage( TParamsPanel7, _("Camera"), false );
	ScenariosPanel = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer80;
	bSizer80 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer81;
	bSizer81 = new wxBoxSizer( wxHORIZONTAL );
	
	m_scenario_list = new wxListBox( ScenariosPanel, ID_SCENARIO_LIST, wxDefaultPosition, wxSize( 200,150 ), 0, NULL, wxLB_SINGLE ); 
	bSizer81->Add( m_scenario_list, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	wxBoxSizer* bSizer82;
	bSizer82 = new wxBoxSizer( wxVERTICAL );
	
	m_new_scenario = new wxButton( ScenariosPanel, ID_NEW_SCENARIO, _("New"), wxDefaultPosition, wxDefaultSize, 0 );
	m_new_scenario->SetDefault(); 
	bSizer82->Add( m_new_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	id_delete_scenario = new wxButton( ScenariosPanel, ID_DELETE_SCENARIO, _("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	id_delete_scenario->SetDefault(); 
	bSizer82->Add( id_delete_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_edit_scenario = new wxButton( ScenariosPanel, ID_EDIT_SCENARIO, _("Edit"), wxDefaultPosition, wxDefaultSize, 0 );
	m_edit_scenario->SetDefault(); 
	bSizer82->Add( m_edit_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_moveup_scenario = new wxButton( ScenariosPanel, ID_MOVEUP_SCENARIO, _("Move Up"), wxDefaultPosition, wxDefaultSize, 0 );
	m_moveup_scenario->SetDefault(); 
	bSizer82->Add( m_moveup_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_movedown_scenario = new wxButton( ScenariosPanel, ID_MOVEDOWN_SCENARIO, _("Move Down"), wxDefaultPosition, wxDefaultSize, 0 );
	m_movedown_scenario->SetDefault(); 
	bSizer82->Add( m_movedown_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer81->Add( bSizer82, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer80->Add( bSizer81, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer83;
	bSizer83 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text40 = new wxStaticText( ScenariosPanel, ID_TEXT, _("Start with active scenario:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text40->Wrap( -1 );
	bSizer83->Add( m_text40, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_choice_scenarioChoices[] = { _("ChoiceItem") };
	int m_choice_scenarioNChoices = sizeof( m_choice_scenarioChoices ) / sizeof( wxString );
	m_choice_scenario = new wxChoice( ScenariosPanel, ID_CHOICE_SCENARIO, wxDefaultPosition, wxSize( 200,-1 ), m_choice_scenarioNChoices, m_choice_scenarioChoices, 0 );
	m_choice_scenario->SetSelection( 0 );
	bSizer83->Add( m_choice_scenario, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer80->Add( bSizer83, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	ScenariosPanel->SetSizer( bSizer80 );
	ScenariosPanel->Layout();
	bSizer80->Fit( ScenariosPanel );
	m_notebook->AddPage( ScenariosPanel, _("Scenarios"), false );
	
	bSizer13->Add( m_notebook, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer84;
	bSizer84 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer84->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer84->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer13->Add( bSizer84, 0, wxALIGN_CENTER|wxRIGHT|wxLEFT, 5 );
	
	
	this->SetSizer( bSizer13 );
	this->Layout();
	bSizer13->Fit( this );
	
	this->Centre( wxBOTH );
}

TParamsDlgBase::~TParamsDlgBase()
{
}

UtilDlgBase::UtilDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer119;
	bSizer119 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer120;
	bSizer120 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text62 = new wxStaticText( this, ID_TEXT, _("Structure Type:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text62->Wrap( -1 );
	bSizer120->Add( m_text62, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_structtypeChoices[] = { _("Item") };
	int m_structtypeNChoices = sizeof( m_structtypeChoices ) / sizeof( wxString );
	m_structtype = new wxChoice( this, ID_STRUCTTYPE, wxDefaultPosition, wxSize( 250,-1 ), m_structtypeNChoices, m_structtypeChoices, 0 );
	m_structtype->SetSelection( 0 );
	bSizer120->Add( m_structtype, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer119->Add( bSizer120, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer119 );
	this->Layout();
	bSizer119->Fit( this );
	
	this->Centre( wxBOTH );
}

UtilDlgBase::~UtilDlgBase()
{
}

VehicleDlgBase::VehicleDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer255;
	bSizer255 = new wxBoxSizer( wxVERTICAL );
	
	wxString m_choice_vehiclesChoices[] = { _("ChoiceItem") };
	int m_choice_vehiclesNChoices = sizeof( m_choice_vehiclesChoices ) / sizeof( wxString );
	m_choice_vehicles = new wxChoice( this, ID_CHOICE_VEHICLES, wxDefaultPosition, wxSize( 100,-1 ), m_choice_vehiclesNChoices, m_choice_vehiclesChoices, 0 );
	m_choice_vehicles->SetSelection( 0 );
	bSizer255->Add( m_choice_vehicles, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer256;
	bSizer256 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text165 = new wxStaticText( this, ID_TEXT, _("Body color:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text165->Wrap( -1 );
	bSizer256->Add( m_text165, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_color3 = new wxStaticBitmap( this, ID_COLOR3, wxBitmap( dummy_32x18_xpm ), wxDefaultPosition, wxSize( 32,18 ), 0 );
	bSizer256->Add( m_color3, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_set_vehicle_color = new wxButton( this, ID_SET_VEHICLE_COLOR, _("Set"), wxDefaultPosition, wxSize( 40,-1 ), 0 );
	m_set_vehicle_color->SetDefault(); 
	bSizer256->Add( m_set_vehicle_color, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer255->Add( bSizer256, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	
	this->SetSizer( bSizer255 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

VehicleDlgBase::~VehicleDlgBase()
{
}

VIADlgBase::VIADlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer197;
	bSizer197 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer198;
	bSizer198 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText142 = new wxStaticText( this, wxID_ANY, _("Percentage of human binocular field of view"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText142->Wrap( -1 );
	bSizer198->Add( m_staticText142, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	m_VIFCtl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE|wxTE_READONLY );
	m_VIFCtl->SetMaxLength( 0 ); 
	m_VIFCtl->SetValidator( wxTextValidator( wxFILTER_NUMERIC, &m_VIF ) );
	
	bSizer198->Add( m_VIFCtl, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	
	bSizer197->Add( bSizer198, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer199;
	bSizer199 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer199->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_button64 = new wxButton( this, ID_VIF_RECALCULATE, _("Recalculate"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer199->Add( m_button64, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer199->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_button65 = new wxButton( this, wxID_OK, _("Done"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer199->Add( m_button65, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer199->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	bSizer197->Add( bSizer199, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer197 );
	this->Layout();
	bSizer197->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( VIADlgBase::OnInitDialog ) );
	m_button64->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( VIADlgBase::OnRecalculate ), NULL, this );
}

VIADlgBase::~VIADlgBase()
{
	// Disconnect Events
	this->Disconnect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( VIADlgBase::OnInitDialog ) );
	m_button64->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( VIADlgBase::OnRecalculate ), NULL, this );
	
}

VIAGDALOptionsDlgBase::VIAGDALOptionsDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer200;
	bSizer200 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer33;
	sbSizer33 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Raster Creation Options") ), wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer34;
	sbSizer34 = new wxStaticBoxSizer( new wxStaticBox( sbSizer33->GetStaticBox(), wxID_ANY, _("Help") ), wxVERTICAL );
	
	m_pHtmlWindow = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxSize(600, 300));
	sbSizer34->Add( m_pHtmlWindow, 0, wxALL, 5 );
	
	
	sbSizer33->Add( sbSizer34, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer35;
	sbSizer35 = new wxStaticBoxSizer( new wxStaticBox( sbSizer33->GetStaticBox(), wxID_ANY, _("Creation Options") ), wxHORIZONTAL );
	
	m_textCtrl80 = new wxTextCtrl( sbSizer35->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_textCtrl80->SetMaxLength( 0 ); 
	m_textCtrl80->SetValidator( wxTextValidator( wxFILTER_NONE, &m_CreationOptions ) );
	
	sbSizer35->Add( m_textCtrl80, 1, wxALL, 5 );
	
	
	sbSizer33->Add( sbSizer35, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer36;
	sbSizer36 = new wxStaticBoxSizer( new wxStaticBox( sbSizer33->GetStaticBox(), wxID_ANY, _("Sample Grid") ), wxHORIZONTAL );
	
	m_staticText138 = new wxStaticText( sbSizer36->GetStaticBox(), wxID_ANY, _("X(E-W) sample interval"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText138->Wrap( -1 );
	sbSizer36->Add( m_staticText138, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_textCtrl81 = new wxTextCtrl( sbSizer36->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_textCtrl81->SetMaxLength( 0 ); 
	m_textCtrl81->SetValidator( wxTextValidator( wxFILTER_NUMERIC, &m_XSampleInterval ) );
	
	sbSizer36->Add( m_textCtrl81, 0, wxALL, 5 );
	
	m_staticText139 = new wxStaticText( sbSizer36->GetStaticBox(), wxID_ANY, _("Y(N-S) sample interval"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText139->Wrap( -1 );
	sbSizer36->Add( m_staticText139, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_textCtrl82 = new wxTextCtrl( sbSizer36->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_textCtrl82->SetMaxLength( 0 ); 
	m_textCtrl82->SetValidator( wxTextValidator( wxFILTER_NUMERIC, &m_YSampleInterval ) );
	
	sbSizer36->Add( m_textCtrl82, 0, wxALL, 5 );
	
	
	sbSizer33->Add( sbSizer36, 0, wxALIGN_CENTER_HORIZONTAL, 5 );
	
	wxStaticBoxSizer* sbSizer37;
	sbSizer37 = new wxStaticBoxSizer( new wxStaticBox( sbSizer33->GetStaticBox(), wxID_ANY, _("Data conversion options") ), wxHORIZONTAL );
	
	m_staticText140 = new wxStaticText( sbSizer37->GetStaticBox(), wxID_ANY, _("Data Type"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText140->Wrap( -1 );
	sbSizer37->Add( m_staticText140, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_pDataTypeChoice = new wxComboBox( sbSizer37->GetStaticBox(), wxID_ANY, _("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	m_pDataTypeChoice->SetValidator( wxTextValidator( wxFILTER_NONE, &m_DataType ) );
	
	sbSizer37->Add( m_pDataTypeChoice, 0, wxALL, 5 );
	
	
	sbSizer37->Add( 200, 20, 1, wxEXPAND, 5 );
	
	m_staticText141 = new wxStaticText( sbSizer37->GetStaticBox(), wxID_ANY, _("Scale Factor"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText141->Wrap( -1 );
	sbSizer37->Add( m_staticText141, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_textCtrl83 = new wxTextCtrl( sbSizer37->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_textCtrl83->SetMaxLength( 0 ); 
	m_textCtrl83->SetValidator( wxTextValidator( wxFILTER_NUMERIC, &m_ScaleFactor ) );
	
	sbSizer37->Add( m_textCtrl83, 0, wxALL, 5 );
	
	
	sbSizer33->Add( sbSizer37, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer201;
	bSizer201 = new wxBoxSizer( wxHORIZONTAL );
	
	m_button66 = new wxButton( sbSizer33->GetStaticBox(), wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer201->Add( m_button66, 0, wxALL, 5 );
	
	
	bSizer201->Add( 80, 20, 1, wxEXPAND, 5 );
	
	m_button67 = new wxButton( sbSizer33->GetStaticBox(), wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer201->Add( m_button67, 0, wxALL, 5 );
	
	
	sbSizer33->Add( bSizer201, 0, wxALIGN_CENTER_HORIZONTAL, 5 );
	
	
	bSizer200->Add( sbSizer33, 1, 0, 5 );
	
	
	this->SetSizer( bSizer200 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

VIAGDALOptionsDlgBase::~VIAGDALOptionsDlgBase()
{
}
