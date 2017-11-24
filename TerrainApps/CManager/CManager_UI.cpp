///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Nov 10 2014)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#include "CManager_UI.h"

#include "../../TerrainSDK/vtui/bitmaps/dummy_32x18.xpm"

///////////////////////////////////////////////////////////////////////////

PropPanelBase::PropPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer80;
	bSizer80 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer8;
	sbSizer8 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Item") ), wxVERTICAL );
	
	wxBoxSizer* bSizer81;
	bSizer81 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text1 = new wxStaticText( sbSizer8->GetStaticBox(), ID_TEXT, _("Item Name:"), wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_text1->Wrap( -1 );
	bSizer81->Add( m_text1, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_item = new wxTextCtrl( sbSizer8->GetStaticBox(), ID_ITEM, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_item->SetMaxLength( 0 ); 
	bSizer81->Add( m_item, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	sbSizer8->Add( bSizer81, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer82;
	bSizer82 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text2 = new wxStaticText( sbSizer8->GetStaticBox(), ID_TEXT, _("Type:"), wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_text2->Wrap( 0 );
	bSizer82->Add( m_text2, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxArrayString m_typechoiceChoices;
	m_typechoice = new wxChoice( sbSizer8->GetStaticBox(), ID_TYPECHOICE, wxDefaultPosition, wxSize( 100,-1 ), m_typechoiceChoices, 0 );
	m_typechoice->SetSelection( 0 );
	bSizer82->Add( m_typechoice, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	sbSizer8->Add( bSizer82, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer83;
	bSizer83 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text3 = new wxStaticText( sbSizer8->GetStaticBox(), ID_TEXT, _("Subtype:"), wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_text3->Wrap( 0 );
	bSizer83->Add( m_text3, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxArrayString m_subtypechoiceChoices;
	m_subtypechoice = new wxChoice( sbSizer8->GetStaticBox(), ID_SUBTYPECHOICE, wxDefaultPosition, wxSize( 100,-1 ), m_subtypechoiceChoices, 0 );
	m_subtypechoice->SetSelection( 0 );
	bSizer83->Add( m_subtypechoice, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	sbSizer8->Add( bSizer83, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer84;
	bSizer84 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text4 = new wxStaticText( sbSizer8->GetStaticBox(), ID_TEXT, _("Tags:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text4->Wrap( 0 );
	bSizer84->Add( m_text4, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_addtag = new wxButton( sbSizer8->GetStaticBox(), ID_ADDTAG, _("&Add"), wxDefaultPosition, wxSize( 38,-1 ), 0 );
	m_addtag->SetDefault(); 
	bSizer84->Add( m_addtag, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_removetag = new wxButton( sbSizer8->GetStaticBox(), ID_REMOVETAG, _("&Remove"), wxDefaultPosition, wxSize( 62,-1 ), 0 );
	m_removetag->SetDefault(); 
	bSizer84->Add( m_removetag, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_edittag = new wxButton( sbSizer8->GetStaticBox(), ID_EDITTAG, _("&Edit"), wxDefaultPosition, wxSize( 42,-1 ), 0 );
	m_edittag->SetDefault(); 
	bSizer84->Add( m_edittag, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer8->Add( bSizer84, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_taglist = new wxListCtrl( sbSizer8->GetStaticBox(), ID_TAGLIST, wxDefaultPosition, wxSize( 160,120 ), wxLC_REPORT|wxSUNKEN_BORDER );
	sbSizer8->Add( m_taglist, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer80->Add( sbSizer8, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	this->SetSizer( bSizer80 );
	this->Layout();
	bSizer80->Fit( this );
}

PropPanelBase::~PropPanelBase()
{
}

ModelPanelBase::ModelPanelBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer85;
	bSizer85 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer9;
	sbSizer9 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Model") ), wxVERTICAL );
	
	wxBoxSizer* bSizer86;
	bSizer86 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text5 = new wxStaticText( sbSizer9->GetStaticBox(), ID_TEXT, _("Filename:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text5->Wrap( 0 );
	bSizer86->Add( m_text5, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_filename = new wxTextCtrl( sbSizer9->GetStaticBox(), ID_FILENAME, wxEmptyString, wxDefaultPosition, wxSize( 160,-1 ), 0 );
	m_filename->SetMaxLength( 0 ); 
	bSizer86->Add( m_filename, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer9->Add( bSizer86, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer87;
	bSizer87 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text6 = new wxStaticText( sbSizer9->GetStaticBox(), ID_TEXT, _("Visible at distance:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text6->Wrap( -1 );
	bSizer87->Add( m_text6, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_distance = new wxTextCtrl( sbSizer9->GetStaticBox(), ID_DISTANCE, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxTE_PROCESS_ENTER );
	m_distance->SetMaxLength( 0 ); 
	bSizer87->Add( m_distance, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer9->Add( bSizer87, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer88;
	bSizer88 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text7 = new wxStaticText( sbSizer9->GetStaticBox(), ID_TEXT, _("Scale:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text7->Wrap( 0 );
	bSizer88->Add( m_text7, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_scale = new wxTextCtrl( sbSizer9->GetStaticBox(), ID_SCALE, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxTE_PROCESS_ENTER );
	m_scale->SetMaxLength( 0 ); 
	bSizer88->Add( m_scale, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text8 = new wxStaticText( sbSizer9->GetStaticBox(), ID_TEXT, _("(meters/unit)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text8->Wrap( 0 );
	bSizer88->Add( m_text8, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer9->Add( bSizer88, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer89;
	bSizer89 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text9 = new wxStaticText( sbSizer9->GetStaticBox(), ID_TEXT, _("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text9->Wrap( 0 );
	bSizer89->Add( m_text9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	m_status = new wxTextCtrl( sbSizer9->GetStaticBox(), ID_STATUS, wxEmptyString, wxDefaultPosition, wxSize( 80,40 ), wxTE_MULTILINE );
	bSizer89->Add( m_status, 1, wxALIGN_CENTER, 5 );
	
	
	sbSizer9->Add( bSizer89, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer85->Add( sbSizer9, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	this->SetSizer( bSizer85 );
	this->Layout();
	bSizer85->Fit( this );
}

ModelPanelBase::~ModelPanelBase()
{
}

TagDlgBase::TagDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer90;
	bSizer90 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer91;
	bSizer91 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text10 = new wxStaticText( this, ID_TEXT, _("Tag Name"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text10->Wrap( -1 );
	bSizer91->Add( m_text10, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_tagnameChoices;
	m_tagname = new wxChoice( this, ID_TAGNAME, wxDefaultPosition, wxSize( 100,-1 ), m_tagnameChoices, 0 );
	m_tagname->SetSelection( 0 );
	bSizer91->Add( m_tagname, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer90->Add( bSizer91, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer92;
	bSizer92 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text11 = new wxStaticText( this, ID_TEXT, _("Tag Value"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text11->Wrap( -1 );
	bSizer92->Add( m_text11, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_tagtext = new wxTextCtrl( this, ID_TAGTEXT, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_tagtext->SetMaxLength( 0 ); 
	bSizer92->Add( m_tagtext, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer90->Add( bSizer92, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5 );
	
	wxBoxSizer* bSizer93;
	bSizer93 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer93->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cancel->SetDefault(); 
	bSizer93->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer90->Add( bSizer93, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	this->SetSizer( bSizer90 );
	this->Layout();
	bSizer90->Fit( this );
	
	this->Centre( wxBOTH );
}

TagDlgBase::~TagDlgBase()
{
}

LightDlgBase::LightDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer94;
	bSizer94 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer95;
	bSizer95 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text12 = new wxStaticText( this, ID_TEXT, _("Light:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text12->Wrap( 0 );
	bSizer95->Add( m_text12, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_lightChoices[] = { _("Default") };
	int m_lightNChoices = sizeof( m_lightChoices ) / sizeof( wxString );
	m_light = new wxChoice( this, ID_LIGHT, wxDefaultPosition, wxSize( 100,-1 ), m_lightNChoices, m_lightChoices, 0 );
	m_light->SetSelection( 0 );
	bSizer95->Add( m_light, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer94->Add( bSizer95, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxFlexGridSizer* fgSizer5;
	fgSizer5 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer5->SetFlexibleDirection( wxBOTH );
	fgSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_text13 = new wxStaticText( this, ID_TEXT, _("Ambient"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text13->Wrap( 0 );
	fgSizer5->Add( m_text13, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_ambient = new wxBitmapButton( this, ID_AMBIENT, wxBitmap( dummy_32x18_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_ambient->SetDefault(); 
	fgSizer5->Add( m_ambient, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text14 = new wxStaticText( this, ID_TEXT, _("Diffuse"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text14->Wrap( 0 );
	fgSizer5->Add( m_text14, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_diffuse = new wxBitmapButton( this, ID_DIFFUSE, wxBitmap( dummy_32x18_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_diffuse->SetDefault(); 
	fgSizer5->Add( m_diffuse, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer94->Add( fgSizer5, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer96;
	bSizer96 = new wxBoxSizer( wxVERTICAL );
	
	m_text15 = new wxStaticText( this, ID_TEXT, _("Direction"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text15->Wrap( 0 );
	bSizer96->Add( m_text15, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_dirx = new wxTextCtrl( this, ID_DIRX, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), wxTE_PROCESS_ENTER );
	m_dirx->SetMaxLength( 0 ); 
	bSizer96->Add( m_dirx, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_diry = new wxTextCtrl( this, ID_DIRY, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), wxTE_PROCESS_ENTER );
	m_diry->SetMaxLength( 0 ); 
	bSizer96->Add( m_diry, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_dirz = new wxTextCtrl( this, ID_DIRZ, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), wxTE_PROCESS_ENTER );
	m_dirz->SetMaxLength( 0 ); 
	bSizer96->Add( m_dirz, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer94->Add( bSizer96, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer94 );
	this->Layout();
	bSizer94->Fit( this );
	
	this->Centre( wxBOTH );
}

LightDlgBase::~LightDlgBase()
{
}
