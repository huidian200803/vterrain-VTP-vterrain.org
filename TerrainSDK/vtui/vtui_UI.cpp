///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#include "vtui_UI.h"

#include "bitmaps/dummy_32x18.xpm"

///////////////////////////////////////////////////////////////////////////

BuildingDlgBase::BuildingDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer308;
	bSizer308 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer309;
	bSizer309 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer310;
	bSizer310 = new wxBoxSizer( wxVERTICAL );
	
	m_text1 = new wxStaticText( this, ID_TEXT, _("Building Levels"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text1->Wrap( -1 );
	bSizer310->Add( m_text1, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_level1 = new wxListBox( this, ID_LEVEL, wxDefaultPosition, wxSize( 60,100 ), 0, NULL, wxLB_SINGLE ); 
	bSizer310->Add( m_level1, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxFlexGridSizer* fgSizer9;
	fgSizer9 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer9->SetFlexibleDirection( wxBOTH );
	fgSizer9->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_level_copy = new wxButton( this, ID_LEVEL_COPY, _("Copy"), wxDefaultPosition, wxDefaultSize, 0 );
	m_level_copy->SetDefault(); 
	m_level_copy->Enable( false );
	
	fgSizer9->Add( m_level_copy, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_level_del = new wxButton( this, ID_LEVEL_DEL, _("Del"), wxDefaultPosition, wxDefaultSize, 0 );
	m_level_del->SetDefault(); 
	m_level_del->Enable( false );
	
	fgSizer9->Add( m_level_del, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_level_up = new wxButton( this, ID_LEVEL_UP, _("Up"), wxDefaultPosition, wxDefaultSize, 0 );
	m_level_up->SetDefault(); 
	fgSizer9->Add( m_level_up, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_level_down = new wxButton( this, ID_LEVEL_DOWN, _("Down"), wxDefaultPosition, wxDefaultSize, 0 );
	m_level_down->SetDefault(); 
	fgSizer9->Add( m_level_down, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer310->Add( fgSizer9, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	m_line1 = new wxStaticLine( this, ID_LINE1, wxDefaultPosition, wxSize( 20,-1 ), wxLI_HORIZONTAL );
	bSizer310->Add( m_line1, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer311;
	bSizer311 = new wxBoxSizer( wxHORIZONTAL );
	
	m_editheights = new wxButton( this, ID_EDITHEIGHTS, _("Baseline Editor"), wxDefaultPosition, wxDefaultSize, 0 );
	m_editheights->SetDefault(); 
	m_editheights->Enable( false );
	
	bSizer311->Add( m_editheights, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_set_roof_type = new wxButton( this, ID_SET_ROOF_TYPE, _("Set Roof Type"), wxDefaultPosition, wxDefaultSize, 0 );
	m_set_roof_type->SetDefault(); 
	bSizer311->Add( m_set_roof_type, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer310->Add( bSizer311, 0, wxALIGN_CENTER, 5 );
	
	
	bSizer309->Add( bSizer310, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	m_panel1 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	sbSizer57 = new wxStaticBoxSizer( new wxStaticBox( m_panel1, wxID_ANY, _("Level Properties") ), wxVERTICAL );
	
	wxBoxSizer* bSizer312;
	bSizer312 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text2 = new wxStaticText( sbSizer57->GetStaticBox(), ID_TEXT, _("Stories:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text2->Wrap( 0 );
	bSizer312->Add( m_text2, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_stories = new wxSpinCtrl( sbSizer57->GetStaticBox(), ID_STORIES, wxT("0"), wxDefaultPosition, wxSize( 60,-1 ), wxSP_ARROW_KEYS, 0, 100, 0 );
	bSizer312->Add( m_stories, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	
	sbSizer57->Add( bSizer312, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer313;
	bSizer313 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text3 = new wxStaticText( sbSizer57->GetStaticBox(), ID_TEXT, _("Story Height:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text3->Wrap( -1 );
	bSizer313->Add( m_text3, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_story_height = new wxTextCtrl( sbSizer57->GetStaticBox(), ID_STORY_HEIGHT, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
    //m_story_height->SetMaxLength( 0 );
	bSizer313->Add( m_story_height, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_text4 = new wxStaticText( sbSizer57->GetStaticBox(), ID_TEXT, _("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text4->Wrap( 0 );
	bSizer313->Add( m_text4, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	
	sbSizer57->Add( bSizer313, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer314;
	bSizer314 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text5 = new wxStaticText( sbSizer57->GetStaticBox(), ID_TEXT, _("Material:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text5->Wrap( 0 );
	bSizer314->Add( m_text5, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_material1 = new wxTextCtrl( sbSizer57->GetStaticBox(), ID_MATERIAL1, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), wxTE_READONLY );
    //m_material1->SetMaxLength( 0 );
	bSizer314->Add( m_material1, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_set_material = new wxButton( sbSizer57->GetStaticBox(), ID_SET_MATERIAL, _("Set"), wxDefaultPosition, wxDefaultSize, 0 );
	m_set_material->SetDefault(); 
	bSizer314->Add( m_set_material, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer57->Add( bSizer314, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer315;
	bSizer315 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text6 = new wxStaticText( sbSizer57->GetStaticBox(), ID_TEXT, _("Color:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text6->Wrap( 0 );
	bSizer315->Add( m_text6, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_color1 = new wxStaticBitmap( sbSizer57->GetStaticBox(), ID_COLOR1, wxBitmap( dummy_32x18_xpm ), wxDefaultPosition, wxSize( 32,18 ), 0 );
	bSizer315->Add( m_color1, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_set_color = new wxButton( sbSizer57->GetStaticBox(), ID_SET_COLOR, _("Set"), wxDefaultPosition, wxDefaultSize, 0 );
	m_set_color->SetDefault(); 
	bSizer315->Add( m_set_color, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer57->Add( bSizer315, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer316;
	bSizer316 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text7 = new wxStaticText( sbSizer57->GetStaticBox(), ID_TEXT, _("Edge Slopes:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text7->Wrap( -1 );
	bSizer316->Add( m_text7, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_edge_slopes = new wxTextCtrl( sbSizer57->GetStaticBox(), ID_EDGE_SLOPES, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), wxTE_READONLY );
    //m_edge_slopes->SetMaxLength( 0 );
	bSizer316->Add( m_edge_slopes, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_set_edge_slopes = new wxButton( sbSizer57->GetStaticBox(), ID_SET_EDGE_SLOPES, _("Set"), wxDefaultPosition, wxDefaultSize, 0 );
	m_set_edge_slopes->SetDefault(); 
	bSizer316->Add( m_set_edge_slopes, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer57->Add( bSizer316, 0, wxALIGN_CENTER, 5 );
	
	
	m_panel1->SetSizer( sbSizer57 );
	m_panel1->Layout();
	sbSizer57->Fit( m_panel1 );
	bSizer309->Add( m_panel1, 0, wxEXPAND | wxALL, 5 );
	
	m_panel2 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxStaticBoxSizer* sbSizer8;
	sbSizer8 = new wxStaticBoxSizer( new wxStaticBox( m_panel2, wxID_ANY, _("Edges") ), wxVERTICAL );
	
	m_edge1 = new wxListBox( sbSizer8->GetStaticBox(), ID_EDGE, wxDefaultPosition, wxSize( 60,60 ), 0, NULL, wxLB_SINGLE ); 
	m_edge1->SetMinSize( wxSize( 80,-1 ) );
	
	sbSizer8->Add( m_edge1, 1, wxALL|wxEXPAND, 5 );
	
	
	m_panel2->SetSizer( sbSizer8 );
	m_panel2->Layout();
	sbSizer8->Fit( m_panel2 );
	bSizer309->Add( m_panel2, 0, wxEXPAND | wxALL, 5 );
	
	m_panel3 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxStaticBoxSizer* g_pEdgeGroup1;
	g_pEdgeGroup1 = new wxStaticBoxSizer( new wxStaticBox( m_panel3, wxID_ANY, _("Edge Properties") ), wxVERTICAL );
	
	wxBoxSizer* bSizer3241;
	bSizer3241 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text131 = new wxStaticText( g_pEdgeGroup1->GetStaticBox(), ID_TEXT, _("Material:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text131->Wrap( 0 );
	bSizer3241->Add( m_text131, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_material21 = new wxTextCtrl( g_pEdgeGroup1->GetStaticBox(), ID_MATERIAL2, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), wxTE_READONLY );
    //m_material21->SetMaxLength( 0 );
	bSizer3241->Add( m_material21, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_set_material11 = new wxButton( g_pEdgeGroup1->GetStaticBox(), ID_SET_MATERIAL, _("Set"), wxDefaultPosition, wxDefaultSize, 0 );
	m_set_material11->SetDefault(); 
	bSizer3241->Add( m_set_material11, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	
	g_pEdgeGroup1->Add( bSizer3241, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer3251;
	bSizer3251 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text141 = new wxStaticText( g_pEdgeGroup1->GetStaticBox(), ID_TEXT, _("Facade:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text141->Wrap( 0 );
	bSizer3251->Add( m_text141, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_facade1Choices[] = { _("ChoiceItem") };
	int m_facade1NChoices = sizeof( m_facade1Choices ) / sizeof( wxString );
	m_facade1 = new wxChoice( g_pEdgeGroup1->GetStaticBox(), ID_FACADE, wxDefaultPosition, wxSize( 100,-1 ), m_facade1NChoices, m_facade1Choices, 0 );
	m_facade1->SetSelection( 0 );
	bSizer3251->Add( m_facade1, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	g_pEdgeGroup1->Add( bSizer3251, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer3261;
	bSizer3261 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text151 = new wxStaticText( g_pEdgeGroup1->GetStaticBox(), ID_TEXT, _("Color:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text151->Wrap( 0 );
	bSizer3261->Add( m_text151, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_color21 = new wxStaticBitmap( g_pEdgeGroup1->GetStaticBox(), ID_COLOR2, wxBitmap( dummy_32x18_xpm ), wxDefaultPosition, wxSize( 32,18 ), 0 );
	bSizer3261->Add( m_color21, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_set_color11 = new wxButton( g_pEdgeGroup1->GetStaticBox(), ID_SET_COLOR, _("Set"), wxDefaultPosition, wxDefaultSize, 0 );
	m_set_color11->SetDefault(); 
	bSizer3261->Add( m_set_color11, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	g_pEdgeGroup1->Add( bSizer3261, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer3271;
	bSizer3271 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text161 = new wxStaticText( g_pEdgeGroup1->GetStaticBox(), ID_TEXT, _("Slope:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text161->Wrap( 0 );
	bSizer3271->Add( m_text161, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_edge_slope1 = new wxTextCtrl( g_pEdgeGroup1->GetStaticBox(), ID_EDGE_SLOPE, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
    //m_edge_slope1->SetMaxLength( 0 );
	bSizer3271->Add( m_edge_slope1, 0, wxALIGN_CENTER|wxRIGHT|wxBOTTOM, 5 );
	
	
    g_pEdgeGroup1->Add( bSizer3271, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_features1 = new wxTextCtrl( g_pEdgeGroup1->GetStaticBox(), ID_FEATURES, wxEmptyString, wxDefaultPosition, wxSize( -1,55 ), wxTE_MULTILINE );
    //m_features1->SetMaxLength( 0 );
	g_pEdgeGroup1->Add( m_features1, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5 );
	
	wxBoxSizer* bSizer3281;
	bSizer3281 = new wxBoxSizer( wxHORIZONTAL );
	
	m_feat_clear1 = new wxButton( g_pEdgeGroup1->GetStaticBox(), ID_FEAT_CLEAR, _("Clear"), wxDefaultPosition, wxSize( 70,-1 ), 0 );
	m_feat_clear1->SetDefault(); 
	bSizer3281->Add( m_feat_clear1, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_feat_wall1 = new wxButton( g_pEdgeGroup1->GetStaticBox(), ID_FEAT_WALL, _("Wall"), wxDefaultPosition, wxSize( 70,-1 ), 0 );
	m_feat_wall1->SetDefault(); 
	bSizer3281->Add( m_feat_wall1, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_feat_window1 = new wxButton( g_pEdgeGroup1->GetStaticBox(), ID_FEAT_WINDOW, _("Window"), wxDefaultPosition, wxSize( 70,-1 ), 0 );
	m_feat_window1->SetDefault(); 
	bSizer3281->Add( m_feat_window1, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_feat_door1 = new wxButton( g_pEdgeGroup1->GetStaticBox(), ID_FEAT_DOOR, _("Door"), wxDefaultPosition, wxSize( 70,-1 ), 0 );
	m_feat_door1->SetDefault(); 
	bSizer3281->Add( m_feat_door1, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	g_pEdgeGroup1->Add( bSizer3281, 0, wxALIGN_CENTER, 5 );
	
	
	m_panel3->SetSizer( g_pEdgeGroup1 );
	m_panel3->Layout();
	g_pEdgeGroup1->Fit( m_panel3 );
	bSizer309->Add( m_panel3, 1, wxEXPAND | wxALL, 5 );
	
	
	bSizer308->Add( bSizer309, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxTOP|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer317;
	bSizer317 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer317->Add( 80, 20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_ok1 = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok1->SetDefault(); 
	bSizer317->Add( m_ok1, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer317->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_edges = new wxButton( this, ID_EDGES, _("Edges >>"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer317->Add( m_edges, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer308->Add( bSizer317, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	this->SetSizer( bSizer308 );
	this->Layout();
	bSizer308->Fit( this );
	
	this->Centre( wxBOTH );
}

BuildingDlgBase::~BuildingDlgBase()
{
}

ColorMapDlgBase::ColorMapDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer352;
	bSizer352 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer353;
	bSizer353 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text38 = new wxStaticText( this, ID_TEXT, _("File:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text38->Wrap( 0 );
	bSizer353->Add( m_text38, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cmap_file = new wxTextCtrl( this, ID_CMAP_FILE, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), wxTE_READONLY );
    //m_cmap_file->SetMaxLength( 0 );
	bSizer353->Add( m_cmap_file, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer352->Add( bSizer353, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxBoxSizer* bSizer354;
	bSizer354 = new wxBoxSizer( wxHORIZONTAL );
	
	m_relative = new wxCheckBox( this, ID_RELATIVE, _("Scale relative to elevation range"), wxDefaultPosition, wxDefaultSize, 0 );
	m_relative->SetValue(true); 
	bSizer354->Add( m_relative, 0, wxALIGN_CENTER|wxALL, 5 );
	
	mmlend = new wxCheckBox( this, ID_BLEND, _("Blend colors smoothly"), wxDefaultPosition, wxDefaultSize, 0 );
	mmlend->SetValue(true); 
	bSizer354->Add( mmlend, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer352->Add( bSizer354, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer355;
	bSizer355 = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer62;
	sbSizer62 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Colors") ), wxVERTICAL );
	
	id_colorlist = new wxListCtrl( sbSizer62->GetStaticBox(), ID_COLORLIST, wxDefaultPosition, wxSize( -1,120 ), wxLC_REPORT|wxLC_NO_HEADER|wxSUNKEN_BORDER );
	sbSizer62->Add( id_colorlist, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer356;
	bSizer356 = new wxBoxSizer( wxHORIZONTAL );
	
	m_change_color = new wxButton( sbSizer62->GetStaticBox(), ID_CHANGE_COLOR, _("Change Color..."), wxDefaultPosition, wxDefaultSize, 0 );
	m_change_color->SetDefault(); 
	bSizer356->Add( m_change_color, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_delete_elevation = new wxButton( sbSizer62->GetStaticBox(), ID_DELETE_ELEVATION, _("Delete Color"), wxDefaultPosition, wxDefaultSize, 0 );
	m_delete_elevation->SetDefault(); 
	bSizer356->Add( m_delete_elevation, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer62->Add( bSizer356, 0, wxALIGN_CENTER, 5 );
	
	wxStaticBoxSizer* sbSizer63;
	sbSizer63 = new wxStaticBoxSizer( new wxStaticBox( sbSizer62->GetStaticBox(), wxID_ANY, _("New Color") ), wxHORIZONTAL );
	
	m_text39 = new wxStaticText( sbSizer63->GetStaticBox(), ID_TEXT, _("Height:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text39->Wrap( 0 );
	sbSizer63->Add( m_text39, 0, wxALIGN_CENTER|wxALL, 5 );
	
	mmeight_to_add = new wxTextCtrl( sbSizer63->GetStaticBox(), ID_HEIGHT_TO_ADD, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
    //mmeight_to_add->SetMaxLength( 0 );
	sbSizer63->Add( mmeight_to_add, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_add = new wxButton( sbSizer63->GetStaticBox(), ID_ADD, _("Add"), wxDefaultPosition, wxDefaultSize, 0 );
	m_add->SetDefault(); 
	sbSizer63->Add( m_add, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer62->Add( sbSizer63, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer355->Add( sbSizer62, 1, wxEXPAND|wxALL, 5 );
	
	wxBoxSizer* bSizer357;
	bSizer357 = new wxBoxSizer( wxVERTICAL );
	
	m_save_cmap = new wxButton( this, ID_SAVE_CMAP, _("Save"), wxDefaultPosition, wxDefaultSize, 0 );
	m_save_cmap->SetDefault(); 
	bSizer357->Add( m_save_cmap, 0, wxALIGN_CENTER|wxALL, 5 );
	
	id_save_as_cmap = new wxButton( this, ID_SAVE_AS_CMAP, _("Save As..."), wxDefaultPosition, wxDefaultSize, 0 );
	id_save_as_cmap->SetDefault(); 
	bSizer357->Add( id_save_as_cmap, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_load_cmap = new wxButton( this, ID_LOAD_CMAP, _("Load"), wxDefaultPosition, wxDefaultSize, 0 );
	m_load_cmap->SetDefault(); 
	bSizer357->Add( m_load_cmap, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_line1 = new wxStaticLine( this, ID_LINE1, wxDefaultPosition, wxSize( 20,-1 ), wxLI_HORIZONTAL );
	bSizer357->Add( m_line1, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_close = new wxButton( this, wxID_OK, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
	m_close->SetDefault(); 
	bSizer357->Add( m_close, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer355->Add( bSizer357, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	
	bSizer352->Add( bSizer355, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	this->SetSizer( bSizer352 );
	this->Layout();
	bSizer352->Fit( this );
	
	this->Centre( wxBOTH );
}

ColorMapDlgBase::~ColorMapDlgBase()
{
}

ContourDlgBase::ContourDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer307;
	bSizer307 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer308;
	bSizer308 = new wxBoxSizer( wxHORIZONTAL );
	
	m_radioBtn31 = new wxRadioButton( this, ID_RADIO_SINGLE, _("Single contour at:"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radioBtn31->SetValue( true ); 
	bSizer308->Add( m_radioBtn31, 0, wxALL, 5 );
	
	m_elev = new wxTextCtrl( this, ID_ELEV1, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
    //m_elev->SetMaxLength( 0 );
	bSizer308->Add( m_elev, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text211 = new wxStaticText( this, ID_TEXT, _("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text211->Wrap( 0 );
	bSizer308->Add( m_text211, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer307->Add( bSizer308, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer3081;
	bSizer3081 = new wxBoxSizer( wxHORIZONTAL );
	
	m_radioBtn30 = new wxRadioButton( this, ID_RADIO_EVERY, _("Contours every:"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer3081->Add( m_radioBtn30, 0, wxALL, 5 );
	
	m_elev2 = new wxTextCtrl( this, ID_ELEV2, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
    //m_elev2->SetMaxLength( 0 );
	bSizer3081->Add( m_elev2, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text2111 = new wxStaticText( this, ID_TEXT, _("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text2111->Wrap( 0 );
	bSizer3081->Add( m_text2111, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer307->Add( bSizer3081, 1, wxEXPAND, 5 );
	
	m_staticline5 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer307->Add( m_staticline5, 0, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* bSizer309;
	bSizer309 = new wxBoxSizer( wxHORIZONTAL );
	
	m_radio_create = new wxRadioButton( this, ID_RADIO_CREATE, _("Create new layer"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radio_create->SetValue( true ); 
	bSizer309->Add( m_radio_create, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer307->Add( bSizer309, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer310;
	bSizer310 = new wxBoxSizer( wxHORIZONTAL );
	
	m_radio_add = new wxRadioButton( this, ID_RADIO_ADD, _("Add to layer:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio_add->SetValue( true ); 
	bSizer310->Add( m_radio_add, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_choice_layerChoices[] = { _("ChoiceItem") };
	int m_choice_layerNChoices = sizeof( m_choice_layerChoices ) / sizeof( wxString );
	m_choice_layer = new wxChoice( this, ID_CHOICE_LAYER, wxDefaultPosition, wxSize( 100,-1 ), m_choice_layerNChoices, m_choice_layerChoices, 0 );
	m_choice_layer->SetSelection( 0 );
	bSizer310->Add( m_choice_layer, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer307->Add( bSizer310, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_sdbSizer4 = new wxStdDialogButtonSizer();
	m_sdbSizer4OK = new wxButton( this, wxID_OK );
	m_sdbSizer4->AddButton( m_sdbSizer4OK );
	m_sdbSizer4Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer4->AddButton( m_sdbSizer4Cancel );
	m_sdbSizer4->Realize();
	
	bSizer307->Add( m_sdbSizer4, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer307 );
	this->Layout();
	bSizer307->Fit( this );
	
	this->Centre( wxBOTH );
}

ContourDlgBase::~ContourDlgBase()
{
}

DistanceDlgBase::DistanceDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer350;
	bSizer350 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer351;
	bSizer351 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer351->Add( 10, 10, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_radio_line = new wxRadioButton( this, ID_RADIO_LINE, _("Line"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radio_line->SetValue( true ); 
	bSizer351->Add( m_radio_line, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_radio_path = new wxRadioButton( this, ID_RADIO_PATH, _("Path"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer351->Add( m_radio_path, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer351->Add( 20, 10, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_dist_load_path = new wxButton( this, ID_DIST_LOAD_PATH, _("Load Path..."), wxDefaultPosition, wxDefaultSize, 0 );
	m_dist_load_path->SetDefault(); 
	bSizer351->Add( m_dist_load_path, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_dist_tool_clear = new wxButton( this, ID_DIST_TOOL_CLEAR, _("Clear"), wxDefaultPosition, wxDefaultSize, 0 );
	m_dist_tool_clear->SetDefault(); 
	bSizer351->Add( m_dist_tool_clear, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer350->Add( bSizer351, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxFlexGridSizer* fgSizer11;
	fgSizer11 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer11->SetFlexibleDirection( wxBOTH );
	fgSizer11->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_text34 = new wxStaticText( this, ID_TEXT, _("Map Offset"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text34->Wrap( -1 );
	fgSizer11->Add( m_text34, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_map_offset = new wxTextCtrl( this, ID_MAP_OFFSET, wxEmptyString, wxDefaultPosition, wxSize( 180,-1 ), 0 );
    //m_map_offset->SetMaxLength( 0 );
	fgSizer11->Add( m_map_offset, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxArrayString m_units1Choices;
	m_units1 = new wxChoice( this, ID_UNITS1, wxDefaultPosition, wxSize( 100,-1 ), m_units1Choices, 0 );
	m_units1->SetSelection( 0 );
	fgSizer11->Add( m_units1, 0, wxALIGN_CENTER|wxALL, 5 );
	
	id_text = new wxStaticText( this, ID_TEXT, _("Map Distance"), wxDefaultPosition, wxDefaultSize, 0 );
	id_text->Wrap( -1 );
	fgSizer11->Add( id_text, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_map_dist = new wxTextCtrl( this, ID_MAP_DIST, wxEmptyString, wxDefaultPosition, wxSize( 180,-1 ), 0 );
    //m_map_dist->SetMaxLength( 0 );
	fgSizer11->Add( m_map_dist, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxArrayString m_units2Choices;
	m_units2 = new wxChoice( this, ID_UNITS2, wxDefaultPosition, wxSize( 100,-1 ), m_units2Choices, 0 );
	m_units2->SetSelection( 0 );
	fgSizer11->Add( m_units2, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text35 = new wxStaticText( this, ID_TEXT, _("Geodesic Distance"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text35->Wrap( -1 );
	fgSizer11->Add( m_text35, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	mmeod_dist = new wxTextCtrl( this, ID_GEOD_DIST, wxEmptyString, wxDefaultPosition, wxSize( 180,-1 ), 0 );
    //mmeod_dist->SetMaxLength( 0 );
	fgSizer11->Add( mmeod_dist, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_units3Choices;
	m_units3 = new wxChoice( this, ID_UNITS3, wxDefaultPosition, wxSize( 100,-1 ), m_units3Choices, 0 );
	m_units3->SetSelection( 0 );
	fgSizer11->Add( m_units3, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text36 = new wxStaticText( this, ID_TEXT, _("Approximate Ground"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text36->Wrap( -1 );
	fgSizer11->Add( m_text36, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_ground_dist = new wxTextCtrl( this, ID_GROUND_DIST, wxEmptyString, wxDefaultPosition, wxSize( 180,-1 ), 0 );
    //m_ground_dist->SetMaxLength( 0 );
	fgSizer11->Add( m_ground_dist, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_units4Choices;
	m_units4 = new wxChoice( this, ID_UNITS4, wxDefaultPosition, wxSize( 100,-1 ), m_units4Choices, 0 );
	m_units4->SetSelection( 0 );
	fgSizer11->Add( m_units4, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text37 = new wxStaticText( this, ID_TEXT, _("Vertical Difference"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text37->Wrap( -1 );
	fgSizer11->Add( m_text37, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_vertical = new wxTextCtrl( this, ID_VERTICAL, wxEmptyString, wxDefaultPosition, wxSize( 180,-1 ), 0 );
    //m_vertical->SetMaxLength( 0 );
	fgSizer11->Add( m_vertical, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString id_units5Choices;
	id_units5 = new wxChoice( this, ID_UNITS5, wxDefaultPosition, wxSize( 100,-1 ), id_units5Choices, 0 );
	id_units5->SetSelection( 0 );
	fgSizer11->Add( id_units5, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer350->Add( fgSizer11, 0, wxALIGN_CENTER, 5 );
	
	
	this->SetSizer( bSizer350 );
	this->Layout();
	bSizer350->Fit( this );
	
	this->Centre( wxBOTH );
}

DistanceDlgBase::~DistanceDlgBase()
{
}

FeatureTableDlgBase::FeatureTableDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer379;
	bSizer379 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer380;
	bSizer380 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text49_show = new wxStaticText( this, ID_TEXT_SHOW, _("Show features:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text49_show->Wrap( -1 );
	bSizer380->Add( m_text49_show, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_choice_showChoices;
	m_choice_show = new wxChoice( this, ID_CHOICE_SHOW, wxDefaultPosition, wxSize( 105,-1 ), m_choice_showChoices, 0 );
	m_choice_show->SetSelection( 0 );
	bSizer380->Add( m_choice_show, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	mmext_vertical = new wxStaticText( this, ID_TEXT_VERTICAL, _("Vertical units:"), wxDefaultPosition, wxDefaultSize, 0 );
	mmext_vertical->Wrap( -1 );
	bSizer380->Add( mmext_vertical, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_choice_verticalChoices;
	m_choice_vertical = new wxChoice( this, ID_CHOICE_VERTICAL, wxDefaultPosition, wxSize( 95,-1 ), m_choice_verticalChoices, 0 );
	m_choice_vertical->SetSelection( 0 );
	bSizer380->Add( m_choice_vertical, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	m_del_high = new wxButton( this, ID_DEL_HIGH, _("Delete Highlighted"), wxDefaultPosition, wxDefaultSize, 0 );
	m_del_high->SetDefault(); 
	bSizer380->Add( m_del_high, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer379->Add( bSizer380, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_list = new wxListCtrl( this, ID_LIST, wxDefaultPosition, wxSize( 550,150 ), wxLC_REPORT|wxSUNKEN_BORDER );
	bSizer379->Add( m_list, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	this->SetSizer( bSizer379 );
	this->Layout();
	bSizer379->Fit( this );
	
	this->Centre( wxBOTH );
}

FeatureTableDlgBase::~FeatureTableDlgBase()
{
}

InstanceDlgBase::InstanceDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer344;
	bSizer344 = new wxBoxSizer( wxVERTICAL );
	
	m_radio_content = new wxRadioButton( this, ID_RADIO_CONTENT, _("Instance from Content File"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radio_content->SetValue( true ); 
	bSizer344->Add( m_radio_content, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxBoxSizer* bSizer345;
	bSizer345 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer345->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text30 = new wxStaticText( this, ID_TEXT, _("File:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text30->Wrap( -1 );
	bSizer345->Add( m_text30, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_choice_fileChoices[] = { _("Item") };
	int m_choice_fileNChoices = sizeof( m_choice_fileChoices ) / sizeof( wxString );
	m_choice_file = new wxChoice( this, ID_CHOICE_FILE, wxDefaultPosition, wxSize( 200,-1 ), m_choice_fileNChoices, m_choice_fileChoices, 0 );
	m_choice_file->SetSelection( 0 );
	bSizer345->Add( m_choice_file, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer344->Add( bSizer345, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer346;
	bSizer346 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer346->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text31 = new wxStaticText( this, ID_TEXT, _("Type:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text31->Wrap( -1 );
	bSizer346->Add( m_text31, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_choice_typeChoices[] = { _("Item") };
	int m_choice_typeNChoices = sizeof( m_choice_typeChoices ) / sizeof( wxString );
	m_choice_type = new wxChoice( this, ID_CHOICE_TYPE, wxDefaultPosition, wxSize( 200,-1 ), m_choice_typeNChoices, m_choice_typeChoices, 0 );
	m_choice_type->SetSelection( 0 );
	bSizer346->Add( m_choice_type, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer344->Add( bSizer346, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer347;
	bSizer347 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer347->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text32 = new wxStaticText( this, ID_TEXT, _("Item:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text32->Wrap( -1 );
	bSizer347->Add( m_text32, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_choice_itemChoices[] = { _("Item") };
	int m_choice_itemNChoices = sizeof( m_choice_itemChoices ) / sizeof( wxString );
	m_choice_item = new wxChoice( this, ID_CHOICE_ITEM, wxDefaultPosition, wxSize( 220,-1 ), m_choice_itemNChoices, m_choice_itemChoices, 0 );
	m_choice_item->SetSelection( 0 );
	bSizer347->Add( m_choice_item, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer344->Add( bSizer347, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_radio_model = new wxRadioButton( this, ID_RADIO_MODEL, _("Instance from 3D Model File"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio_model->SetValue( true ); 
	bSizer344->Add( m_radio_model, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxBoxSizer* bSizer348;
	bSizer348 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer348->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_model_file = new wxTextCtrl( this, ID_MODEL_FILE, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), wxTE_READONLY );
    //m_model_file->SetMaxLength( 0 );
	bSizer348->Add( m_model_file, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_browse_model_file = new wxButton( this, ID_BROWSE_MODEL_FILE, _("..."), wxDefaultPosition, wxSize( 30,-1 ), 0 );
	m_browse_model_file->SetDefault(); 
	m_browse_model_file->SetFont( wxFont( 12, 72, 90, 90, false, wxEmptyString ) );
	
	bSizer348->Add( m_browse_model_file, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer344->Add( bSizer348, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer349;
	bSizer349 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text33 = new wxStaticText( this, ID_TEXT, _("Location:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text33->Wrap( -1 );
	bSizer349->Add( m_text33, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_location = new wxTextCtrl( this, ID_LOCATION, wxEmptyString, wxDefaultPosition, wxSize( 200,-1 ), 0 );
    //m_location->SetMaxLength( 0 );
	bSizer349->Add( m_location, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_create = new wxButton( this, ID_CREATE, _("Create"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer349->Add( m_create, 0, wxALL, 5 );
	
	
	bSizer344->Add( bSizer349, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	this->SetSizer( bSizer344 );
	this->Layout();
	bSizer344->Fit( this );
	
	this->Centre( wxBOTH );
}

InstanceDlgBase::~InstanceDlgBase()
{
}

LinearStructDlgBase::LinearStructDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer330;
	bSizer330 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer331;
	bSizer331 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text17 = new wxStaticText( this, ID_TEXT, _("Defined Style:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text17->Wrap( -1 );
	bSizer331->Add( m_text17, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_linear_structure_styleChoices[] = { _("Item") };
	int m_linear_structure_styleNChoices = sizeof( m_linear_structure_styleChoices ) / sizeof( wxString );
	m_linear_structure_style = new wxChoice( this, ID_LINEAR_STRUCTURE_STYLE, wxDefaultPosition, wxSize( 160,-1 ), m_linear_structure_styleNChoices, m_linear_structure_styleChoices, 0 );
	m_linear_structure_style->SetSelection( 0 );
	bSizer331->Add( m_linear_structure_style, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer330->Add( bSizer331, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxStaticBoxSizer* sbSizer60;
	sbSizer60 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Posts") ), wxVERTICAL );
	
	wxBoxSizer* bSizer332;
	bSizer332 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text18 = new wxStaticText( sbSizer60->GetStaticBox(), ID_TEXT, _("Type:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text18->Wrap( -1 );
	bSizer332->Add( m_text18, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_post_typeChoices[] = { _("Item") };
	int m_post_typeNChoices = sizeof( m_post_typeChoices ) / sizeof( wxString );
	m_post_type = new wxChoice( sbSizer60->GetStaticBox(), ID_POST_TYPE, wxDefaultPosition, wxSize( 100,-1 ), m_post_typeNChoices, m_post_typeChoices, 0 );
	m_post_type->SetSelection( 0 );
	bSizer332->Add( m_post_type, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer60->Add( bSizer332, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer333;
	bSizer333 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text19 = new wxStaticText( sbSizer60->GetStaticBox(), ID_TEXT, _("Spacing:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text19->Wrap( 0 );
	bSizer333->Add( m_text19, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_post_spacing_edit = new wxTextCtrl( sbSizer60->GetStaticBox(), ID_POST_SPACING_EDIT, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
    //m_post_spacing_edit->SetMaxLength( 0 );
	bSizer333->Add( m_post_spacing_edit, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_post_spacing_slider = new wxSlider( sbSizer60->GetStaticBox(), ID_POST_SPACING_SLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 110,-1 ), wxSL_HORIZONTAL );
	bSizer333->Add( m_post_spacing_slider, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	sbSizer60->Add( bSizer333, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer334;
	bSizer334 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text20 = new wxStaticText( sbSizer60->GetStaticBox(), ID_TEXT, _("Height:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text20->Wrap( 0 );
	bSizer334->Add( m_text20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_post_height_edit = new wxTextCtrl( sbSizer60->GetStaticBox(), ID_POST_HEIGHT_EDIT, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
    //m_post_height_edit->SetMaxLength( 0 );
	bSizer334->Add( m_post_height_edit, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_post_height_slider = new wxSlider( sbSizer60->GetStaticBox(), ID_POST_HEIGHT_SLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 110,-1 ), wxSL_HORIZONTAL );
	bSizer334->Add( m_post_height_slider, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	sbSizer60->Add( bSizer334, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer335;
	bSizer335 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text21 = new wxStaticText( sbSizer60->GetStaticBox(), ID_TEXT, _("Size:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text21->Wrap( -1 );
	bSizer335->Add( m_text21, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_post_size_edit = new wxTextCtrl( sbSizer60->GetStaticBox(), ID_POST_SIZE_EDIT, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
    //m_post_size_edit->SetMaxLength( 0 );
	bSizer335->Add( m_post_size_edit, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_post_size_slider = new wxSlider( sbSizer60->GetStaticBox(), ID_POST_SIZE_SLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 110,-1 ), wxSL_HORIZONTAL );
	bSizer335->Add( m_post_size_slider, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	sbSizer60->Add( bSizer335, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer336;
	bSizer336 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text22 = new wxStaticText( sbSizer60->GetStaticBox(), ID_TEXT, _("Extension:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text22->Wrap( -1 );
	bSizer336->Add( m_text22, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_choice_extensionChoices[] = { _("None"), _("Left"), _("Right"), _("Double") };
	int m_choice_extensionNChoices = sizeof( m_choice_extensionChoices ) / sizeof( wxString );
	m_choice_extension = new wxChoice( sbSizer60->GetStaticBox(), ID_CHOICE_EXTENSION, wxDefaultPosition, wxSize( 100,-1 ), m_choice_extensionNChoices, m_choice_extensionChoices, 0 );
	m_choice_extension->SetSelection( 0 );
	bSizer336->Add( m_choice_extension, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer60->Add( bSizer336, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	bSizer330->Add( sbSizer60, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer61;
	sbSizer61 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Connection") ), wxVERTICAL );
	
	wxBoxSizer* bSizer337;
	bSizer337 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text23 = new wxStaticText( sbSizer61->GetStaticBox(), ID_TEXT, _("Type:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text23->Wrap( -1 );
	bSizer337->Add( m_text23, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_conn_typeChoices[] = { _("Item") };
	int m_conn_typeNChoices = sizeof( m_conn_typeChoices ) / sizeof( wxString );
	m_conn_type = new wxChoice( sbSizer61->GetStaticBox(), ID_CONN_TYPE, wxDefaultPosition, wxSize( 100,-1 ), m_conn_typeNChoices, m_conn_typeChoices, 0 );
	m_conn_type->SetSelection( 0 );
	bSizer337->Add( m_conn_type, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer61->Add( bSizer337, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer338;
	bSizer338 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text24 = new wxStaticText( sbSizer61->GetStaticBox(), ID_TEXT, _("Material:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text24->Wrap( -1 );
	bSizer338->Add( m_text24, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_conn_materialChoices[] = { _("Item") };
	int m_conn_materialNChoices = sizeof( m_conn_materialChoices ) / sizeof( wxString );
	m_conn_material = new wxChoice( sbSizer61->GetStaticBox(), ID_CONN_MATERIAL, wxDefaultPosition, wxSize( 100,-1 ), m_conn_materialNChoices, m_conn_materialChoices, 0 );
	m_conn_material->SetSelection( 0 );
	bSizer338->Add( m_conn_material, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer61->Add( bSizer338, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer339;
	bSizer339 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text25 = new wxStaticText( sbSizer61->GetStaticBox(), ID_TEXT, _("Top Height:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text25->Wrap( -1 );
	bSizer339->Add( m_text25, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_conn_top_edit = new wxTextCtrl( sbSizer61->GetStaticBox(), ID_CONN_TOP_EDIT, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
    //m_conn_top_edit->SetMaxLength( 0 );
	bSizer339->Add( m_conn_top_edit, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_conn_top_slider = new wxSlider( sbSizer61->GetStaticBox(), ID_CONN_TOP_SLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 110,-1 ), wxSL_HORIZONTAL );
	bSizer339->Add( m_conn_top_slider, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	sbSizer61->Add( bSizer339, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer340;
	bSizer340 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text26 = new wxStaticText( sbSizer61->GetStaticBox(), ID_TEXT, _("Bottom Height:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text26->Wrap( -1 );
	bSizer340->Add( m_text26, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_conn_bottom_edit = new wxTextCtrl( sbSizer61->GetStaticBox(), ID_CONN_BOTTOM_EDIT, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
    //m_conn_bottom_edit->SetMaxLength( 0 );
	bSizer340->Add( m_conn_bottom_edit, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_conn_bottom_slider = new wxSlider( sbSizer61->GetStaticBox(), ID_CONN_BOTTOM_SLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 110,-1 ), wxSL_HORIZONTAL );
	bSizer340->Add( m_conn_bottom_slider, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	sbSizer61->Add( bSizer340, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer341;
	bSizer341 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text27 = new wxStaticText( sbSizer61->GetStaticBox(), ID_TEXT, _("Width:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text27->Wrap( -1 );
	bSizer341->Add( m_text27, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_conn_width_edit = new wxTextCtrl( sbSizer61->GetStaticBox(), ID_CONN_WIDTH_EDIT, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
    //m_conn_width_edit->SetMaxLength( 0 );
	bSizer341->Add( m_conn_width_edit, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_conn_width_slider = new wxSlider( sbSizer61->GetStaticBox(), ID_CONN_WIDTH_SLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 110,-1 ), wxSL_HORIZONTAL );
	bSizer341->Add( m_conn_width_slider, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	sbSizer61->Add( bSizer341, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer342;
	bSizer342 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text28 = new wxStaticText( sbSizer61->GetStaticBox(), ID_TEXT, _("Side Slope:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text28->Wrap( -1 );
	bSizer342->Add( m_text28, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_slope = new wxTextCtrl( sbSizer61->GetStaticBox(), ID_SLOPE, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
    //m_slope->SetMaxLength( 0 );
	bSizer342->Add( m_slope, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_slope_slider = new wxSlider( sbSizer61->GetStaticBox(), ID_SLOPE_SLIDER, 0, 0, 100, wxDefaultPosition, wxSize( 110,-1 ), wxSL_HORIZONTAL );
	bSizer342->Add( m_slope_slider, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	sbSizer61->Add( bSizer342, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer343;
	bSizer343 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text29 = new wxStaticText( sbSizer61->GetStaticBox(), ID_TEXT, _("Profile:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text29->Wrap( -1 );
	bSizer343->Add( m_text29, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_choice_profileChoices[] = { _("ChoiceItem") };
	int m_choice_profileNChoices = sizeof( m_choice_profileChoices ) / sizeof( wxString );
	m_choice_profile = new wxChoice( sbSizer61->GetStaticBox(), ID_CHOICE_PROFILE, wxDefaultPosition, wxSize( 130,-1 ), m_choice_profileNChoices, m_choice_profileChoices, 0 );
	m_choice_profile->SetSelection( 0 );
	bSizer343->Add( m_choice_profile, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_profile_edit = new wxButton( sbSizer61->GetStaticBox(), ID_PROFILE_EDIT, _("Edit"), wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_profile_edit->SetDefault(); 
	bSizer343->Add( m_profile_edit, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer61->Add( bSizer343, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_constant_top = new wxCheckBox( sbSizer61->GetStaticBox(), ID_CONSTANT_TOP, _("Constant Top Height"), wxDefaultPosition, wxDefaultSize, 0 );
	m_constant_top->SetValue(true); 
	sbSizer61->Add( m_constant_top, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer330->Add( sbSizer61, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	this->SetSizer( bSizer330 );
	this->Layout();
	bSizer330->Fit( this );
	
	this->Centre( wxBOTH );
}

LinearStructDlgBase::~LinearStructDlgBase()
{
}

ProfileDlgBase::ProfileDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer358;
	bSizer358 = new wxBoxSizer( wxVERTICAL );
	
	
	bSizer358->Add( 500, 200, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer359;
	bSizer359 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer360;
	bSizer360 = new wxBoxSizer( wxVERTICAL );
	
	m_line_of_sight = new wxCheckBox( this, ID_LINE_OF_SIGHT, _("Line of Sight"), wxDefaultPosition, wxDefaultSize, 0 );
	m_line_of_sight->SetValue(true); 
	bSizer360->Add( m_line_of_sight, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_visibility = new wxCheckBox( this, ID_VISIBILITY, _("Visibility"), wxDefaultPosition, wxDefaultSize, 0 );
	m_visibility->SetValue(true); 
	bSizer360->Add( m_visibility, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_fresnel = new wxCheckBox( this, ID_FRESNEL, _("Show Fresnel zones"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fresnel->SetValue(true); 
	bSizer360->Add( m_fresnel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_use_effective = new wxCheckBox( this, ID_USE_EFFECTIVE, _("Use effective radius of earth"), wxDefaultPosition, wxDefaultSize, 0 );
	m_use_effective->SetValue(true); 
	bSizer360->Add( m_use_effective, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_show_culture = new wxButton( this, ID_SHOW_CULTURE, _("Show Culture Objects"), wxDefaultPosition, wxDefaultSize, 0 );
	m_show_culture->SetDefault(); 
	bSizer360->Add( m_show_culture, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer359->Add( bSizer360, 0, wxALIGN_CENTER|wxRIGHT, 5 );
	
	mmine2 = new wxStaticLine( this, ID_LINE2, wxDefaultPosition, wxSize( -1,20 ), wxLI_VERTICAL );
	bSizer359->Add( mmine2, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	wxFlexGridSizer* fgSizer12;
	fgSizer12 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer12->SetFlexibleDirection( wxBOTH );
	fgSizer12->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_text40 = new wxStaticText( this, ID_TEXT, _("Start height:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text40->Wrap( -1 );
	fgSizer12->Add( m_text40, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_height1 = new wxTextCtrl( this, ID_HEIGHT1, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
    //m_height1->SetMaxLength( 0 );
	fgSizer12->Add( m_height1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	id_text = new wxStaticText( this, ID_TEXT, _("End height:"), wxDefaultPosition, wxDefaultSize, 0 );
	id_text->Wrap( -1 );
	fgSizer12->Add( id_text, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_height2 = new wxTextCtrl( this, ID_HEIGHT2, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
    //m_height2->SetMaxLength( 0 );
	fgSizer12->Add( m_height2, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	m_text41 = new wxStaticText( this, ID_TEXT, _("Radio Frequency:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text41->Wrap( -1 );
	fgSizer12->Add( m_text41, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer361;
	bSizer361 = new wxBoxSizer( wxHORIZONTAL );
	
	m_rf = new wxTextCtrl( this, ID_RF, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
    //m_rf->SetMaxLength( 0 );
	bSizer361->Add( m_rf, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	m_text42 = new wxStaticText( this, ID_TEXT, _("MHz"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text42->Wrap( 0 );
	bSizer361->Add( m_text42, 0, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	
	fgSizer12->Add( bSizer361, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_text43 = new wxStaticText( this, ID_TEXT, _("Apply curvature to:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text43->Wrap( -1 );
	fgSizer12->Add( m_text43, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_curvatureChoices[] = { _("None"), _("Terrain"), _("Line of Sight") };
	int m_curvatureNChoices = sizeof( m_curvatureChoices ) / sizeof( wxString );
	m_curvature = new wxChoice( this, ID_CURVATURE, wxDefaultPosition, wxDefaultSize, m_curvatureNChoices, m_curvatureChoices, 0 );
	m_curvature->SetSelection( 0 );
	fgSizer12->Add( m_curvature, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer359->Add( fgSizer12, 0, wxALIGN_CENTER_HORIZONTAL, 5 );
	
	m_line2 = new wxStaticLine( this, ID_LINE2, wxDefaultPosition, wxSize( -1,20 ), wxLI_VERTICAL );
	bSizer359->Add( m_line2, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	wxBoxSizer* bSizer362;
	bSizer362 = new wxBoxSizer( wxVERTICAL );
	
	m_status_text = new wxTextCtrl( this, ID_STATUS_TEXT, wxEmptyString, wxDefaultPosition, wxSize( 210,-1 ), wxTE_MULTILINE );
    //m_status_text->SetMaxLength( 0 );
	bSizer362->Add( m_status_text, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxBoxSizer* bSizer363;
	bSizer363 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text44 = new wxStaticText( this, ID_TEXT, _("Export:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text44->Wrap( 0 );
	bSizer363->Add( m_text44, 0, wxALIGN_CENTER|wxLEFT|wxTOP|wxBOTTOM, 5 );
	
	m_export_dxf = new wxButton( this, ID_EXPORT_DXF, _("Profile"), wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_export_dxf->SetDefault(); 
	bSizer363->Add( m_export_dxf, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_export_trace = new wxButton( this, ID_EXPORT_TRACE, _("Trace"), wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_export_trace->SetDefault(); 
	bSizer363->Add( m_export_trace, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_export_csv = new wxButton( this, ID_EXPORT_CSV, _("CSV"), wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_export_csv->SetDefault(); 
	bSizer363->Add( m_export_csv, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer362->Add( bSizer363, 0, wxALIGN_CENTER, 5 );
	
	
	bSizer359->Add( bSizer362, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	
	bSizer358->Add( bSizer359, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	this->SetSizer( bSizer358 );
	this->Layout();
	bSizer358->Fit( this );
	
	this->Centre( wxBOTH );
}

ProfileDlgBase::~ProfileDlgBase()
{
}

ProfileEditDlgBase::ProfileEditDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer375;
	bSizer375 = new wxBoxSizer( wxVERTICAL );
	
	viewsizer = new wxBoxSizer( wxVERTICAL );
	
	viewsizer->SetMinSize( wxSize( -1,240 ) ); 
	
	bSizer375->Add( viewsizer, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer376;
	bSizer376 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer377;
	bSizer377 = new wxBoxSizer( wxVERTICAL );
	
	m_add_point = new wxRadioButton( this, ID_ADD_POINT, _("Add"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_add_point->SetValue( true ); 
	bSizer377->Add( m_add_point, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_move_point = new wxRadioButton( this, ID_MOVE_POINT, _("Move"), wxDefaultPosition, wxDefaultSize, 0 );
	m_move_point->SetValue( true ); 
	bSizer377->Add( m_move_point, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_remove_point = new wxRadioButton( this, ID_REMOVE_POINT, _("Remove"), wxDefaultPosition, wxDefaultSize, 0 );
	m_remove_point->SetValue( true ); 
	bSizer377->Add( m_remove_point, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer376->Add( bSizer377, 0, wxALIGN_CENTER, 0 );
	
	m_line2 = new wxStaticLine( this, ID_LINE2, wxDefaultPosition, wxSize( -1,20 ), wxLI_VERTICAL );
	bSizer376->Add( m_line2, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	wxGridSizer* gSizer1;
	gSizer1 = new wxGridSizer( 0, 2, 0, 0 );
	
	m_save_prof = new wxButton( this, ID_SAVE_PROF, _("Save"), wxDefaultPosition, wxDefaultSize, 0 );
	m_save_prof->SetDefault(); 
	gSizer1->Add( m_save_prof, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_save_as_prof = new wxButton( this, ID_SAVE_AS_PROF, _("Save As..."), wxDefaultPosition, wxDefaultSize, 0 );
	m_save_as_prof->SetDefault(); 
	gSizer1->Add( m_save_as_prof, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_load_prof = new wxButton( this, ID_LOAD_PROF, _("Load"), wxDefaultPosition, wxDefaultSize, 0 );
	m_load_prof->SetDefault(); 
	gSizer1->Add( m_load_prof, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_close = new wxButton( this, wxID_OK, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
	m_close->SetDefault(); 
	gSizer1->Add( m_close, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer376->Add( gSizer1, 0, wxALIGN_CENTER, 0 );
	
	
	bSizer375->Add( bSizer376, 0, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer375 );
	this->Layout();
	bSizer375->Fit( this );
	
	this->Centre( wxBOTH );
}

ProfileEditDlgBase::~ProfileEditDlgBase()
{
}

ProjectionDlgBase::ProjectionDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer364;
	bSizer364 = new wxBoxSizer( wxVERTICAL );
	
	m_text45 = new wxStaticText( this, ID_TEXT, _("Projection:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text45->Wrap( -1 );
	bSizer364->Add( m_text45, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxArrayString m_projChoices;
	m_proj = new wxChoice( this, ID_PROJ, wxDefaultPosition, wxSize( 170,-1 ), m_projChoices, 0 );
	m_proj->SetSelection( 0 );
	bSizer364->Add( m_proj, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	wxBoxSizer* bSizer365;
	bSizer365 = new wxBoxSizer( wxHORIZONTAL );
	
	mmext = new wxStaticText( this, ID_TEXT, _("Datum:"), wxDefaultPosition, wxDefaultSize, 0 );
	mmext->Wrap( 0 );
	bSizer365->Add( mmext, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_show_all_datums = new wxCheckBox( this, ID_SHOW_ALL_DATUMS, _("Show All Datums"), wxDefaultPosition, wxDefaultSize, 0 );
	m_show_all_datums->SetValue(true); 
	bSizer365->Add( m_show_all_datums, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer364->Add( bSizer365, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxArrayString m_datumChoices;
	m_datum = new wxChoice( this, ID_DATUM, wxDefaultPosition, wxSize( 100,-1 ), m_datumChoices, 0 );
	m_datum->SetSelection( 0 );
	bSizer364->Add( m_datum, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	wxBoxSizer* bSizer366;
	bSizer366 = new wxBoxSizer( wxHORIZONTAL );
	
	id_text = new wxStaticText( this, ID_TEXT, _("Ellipsoid:"), wxDefaultPosition, wxDefaultSize, 0 );
	id_text->Wrap( 0 );
	bSizer366->Add( id_text, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_ellipsoid = new wxTextCtrl( this, ID_ELLIPSOID, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), wxTE_READONLY );
    //m_ellipsoid->SetMaxLength( 0 );
	bSizer366->Add( m_ellipsoid, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer364->Add( bSizer366, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_text46 = new wxStaticText( this, ID_TEXT, _("Horizontal Units:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text46->Wrap( -1 );
	bSizer364->Add( m_text46, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxArrayString m_horunitsChoices;
	m_horunits = new wxChoice( this, ID_HORUNITS, wxDefaultPosition, wxSize( 140,-1 ), m_horunitsChoices, 0 );
	m_horunits->SetSelection( 0 );
	bSizer364->Add( m_horunits, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	m_text47 = new wxStaticText( this, ID_TEXT, _("Zone:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text47->Wrap( -1 );
	bSizer364->Add( m_text47, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxArrayString m_zoneChoices;
	m_zone = new wxChoice( this, ID_ZONE, wxDefaultPosition, wxSize( 140,-1 ), m_zoneChoices, 0 );
	m_zone->SetSelection( 0 );
	bSizer364->Add( m_zone, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	m_text48 = new wxStaticText( this, ID_TEXT, _("Parameters:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text48->Wrap( 0 );
	bSizer364->Add( m_text48, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_projparam = new wxListCtrl( this, ID_PROJPARAM, wxDefaultPosition, wxSize( 230,120 ), wxLC_REPORT|wxSUNKEN_BORDER );
	bSizer364->Add( m_projparam, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	wxBoxSizer* bSizer367;
	bSizer367 = new wxBoxSizer( wxHORIZONTAL );
	
	mmtateplane = new wxButton( this, ID_STATEPLANE, _("Set State Plane Projection"), wxDefaultPosition, wxDefaultSize, 0 );
	mmtateplane->SetDefault(); 
	bSizer367->Add( mmtateplane, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_set_epsg = new wxButton( this, ID_SET_EPSG, _("Set EPSG"), wxDefaultPosition, wxDefaultSize, 0 );
	m_set_epsg->SetDefault(); 
	bSizer367->Add( m_set_epsg, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer364->Add( bSizer367, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer368;
	bSizer368 = new wxBoxSizer( wxHORIZONTAL );
	
	m_proj_load = new wxButton( this, ID_PROJ_LOAD, _("Load from File"), wxDefaultPosition, wxDefaultSize, 0 );
	m_proj_load->SetDefault(); 
	bSizer368->Add( m_proj_load, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_proj_save = new wxButton( this, ID_PROJ_SAVE, _("Save to File"), wxDefaultPosition, wxDefaultSize, 0 );
	m_proj_save->SetDefault(); 
	bSizer368->Add( m_proj_save, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer364->Add( bSizer368, 0, wxALIGN_CENTER|wxALL, 0 );
	
	m_line2 = new wxStaticLine( this, ID_LINE2, wxDefaultPosition, wxSize( 20,-1 ), wxLI_HORIZONTAL );
	bSizer364->Add( m_line2, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_sdbSizer3 = new wxStdDialogButtonSizer();
	m_sdbSizer3OK = new wxButton( this, wxID_OK );
	m_sdbSizer3->AddButton( m_sdbSizer3OK );
	m_sdbSizer3Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer3->AddButton( m_sdbSizer3Cancel );
	m_sdbSizer3->Realize();
	
	bSizer364->Add( m_sdbSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	
	this->SetSizer( bSizer364 );
	this->Layout();
	bSizer364->Fit( this );
	
	this->Centre( wxBOTH );
}

ProjectionDlgBase::~ProjectionDlgBase()
{
}

SizeDlgBase::SizeDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer381;
	bSizer381 = new wxBoxSizer( wxVERTICAL );
	
	m_textctrl = new wxTextCtrl( this, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize( 320,60 ), wxTE_MULTILINE|wxTE_READONLY );
    //m_textctrl->SetMaxLength( 0 );
	bSizer381->Add( m_textctrl, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_slider_ratio = new wxSlider( this, ID_SLIDER_RATIO, 0, 0, 320, wxDefaultPosition, wxSize( 320,-1 ), wxSL_HORIZONTAL );
	bSizer381->Add( m_slider_ratio, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer382;
	bSizer382 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text_x = new wxTextCtrl( this, ID_TEXT_X, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
    //m_text_x->SetMaxLength( 0 );
	bSizer382->Add( m_text_x, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text_y = new wxTextCtrl( this, ID_TEXT_Y, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
    //m_text_y->SetMaxLength( 0 );
	bSizer382->Add( m_text_y, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer382->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_sdbSizer2 = new wxStdDialogButtonSizer();
	m_sdbSizer2OK = new wxButton( this, wxID_OK );
	m_sdbSizer2->AddButton( m_sdbSizer2OK );
	m_sdbSizer2Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer2->AddButton( m_sdbSizer2Cancel );
	m_sdbSizer2->Realize();
	
	bSizer382->Add( m_sdbSizer2, 1, wxEXPAND, 5 );
	
	
	bSizer381->Add( bSizer382, 0, wxALIGN_CENTER, 5 );
	
	
	this->SetSizer( bSizer381 );
	this->Layout();
	bSizer381->Fit( this );
	
	this->Centre( wxBOTH );
}

SizeDlgBase::~SizeDlgBase()
{
}

StatePlaneDlgBase::StatePlaneDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer370;
	bSizer370 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer371;
	bSizer371 = new wxBoxSizer( wxHORIZONTAL );
	
	id_stateplanes = new wxListBox( this, ID_STATEPLANES, wxDefaultPosition, wxSize( 180,240 ), 0, NULL, wxLB_SINGLE ); 
	bSizer371->Add( id_stateplanes, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	
	bSizer370->Add( bSizer371, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxBoxSizer* bSizer372;
	bSizer372 = new wxBoxSizer( wxHORIZONTAL );
	
	m_nad27 = new wxRadioButton( this, ID_NAD27, _("NAD27"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_nad27->SetValue( true ); 
	bSizer372->Add( m_nad27, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_nad83 = new wxRadioButton( this, ID_NAD83, _("NAD83"), wxDefaultPosition, wxDefaultSize, 0 );
	m_nad83->SetValue( true ); 
	bSizer372->Add( m_nad83, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer370->Add( bSizer372, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	wxBoxSizer* bSizer373;
	bSizer373 = new wxBoxSizer( wxHORIZONTAL );
	
	m_radio_meters = new wxRadioButton( this, ID_RADIO_METERS, _("Meters"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radio_meters->SetValue( true ); 
	bSizer373->Add( m_radio_meters, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_radio_feet = new wxRadioButton( this, ID_RADIO_FEET, _("Feet"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio_feet->SetValue( true ); 
	bSizer373->Add( m_radio_feet, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_radio_feet_us = new wxRadioButton( this, ID_RADIO_FEET_US, _("U.S. Feet"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio_feet_us->SetValue( true ); 
	bSizer373->Add( m_radio_feet_us, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer370->Add( bSizer373, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	wxBoxSizer* bSizer374;
	bSizer374 = new wxBoxSizer( wxHORIZONTAL );
	
	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton( this, wxID_OK );
	m_sdbSizer1->AddButton( m_sdbSizer1OK );
	m_sdbSizer1Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer1->AddButton( m_sdbSizer1Cancel );
	m_sdbSizer1->Realize();
	
	bSizer374->Add( m_sdbSizer1, 1, wxEXPAND|wxALL, 5 );
	
	
	bSizer370->Add( bSizer374, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer370 );
	this->Layout();
	bSizer370->Fit( this );
	
	this->Centre( wxBOTH );
}

StatePlaneDlgBase::~StatePlaneDlgBase()
{
}

TagDlgBase::TagDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer378;
	bSizer378 = new wxBoxSizer( wxVERTICAL );
	
	m_taglist = new wxListCtrl( this, ID_TAGLIST, wxDefaultPosition, wxSize( 160,120 ), wxLC_REPORT|wxLC_EDIT_LABELS|wxSUNKEN_BORDER );
	bSizer378->Add( m_taglist, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	this->SetSizer( bSizer378 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

TagDlgBase::~TagDlgBase()
{
}
