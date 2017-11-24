///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Nov 10 2014)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#include "VTBuilder_UI.h"

#include "../../TerrainSDK/vtui/bitmaps/dummy_32x18.xpm"

///////////////////////////////////////////////////////////////////////////

ChunkDlgBase::ChunkDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer144;
	bSizer144 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer8;
	fgSizer8 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer8->SetFlexibleDirection( wxBOTH );
	fgSizer8->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_text36 = new wxStaticText( this, ID_TEXT, _("Depth of the quadtree of chunks to generate:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text36->Wrap( -1 );
	fgSizer8->Add( m_text36, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_depth = new wxTextCtrl( this, ID_DEPTH, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_depth->SetMaxLength( 0 ); 
	fgSizer8->Add( m_depth, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_text37 = new wxStaticText( this, ID_TEXT, _("Maximum geometric error to allow at full LOD:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text37->Wrap( -1 );
	fgSizer8->Add( m_text37, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_maxerror = new wxTextCtrl( this, ID_MAXERROR, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_maxerror->SetMaxLength( 0 ); 
	fgSizer8->Add( m_maxerror, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	
	bSizer144->Add( fgSizer8, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer145;
	bSizer145 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer145->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer145->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer144->Add( bSizer145, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	this->SetSizer( bSizer144 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

ChunkDlgBase::~ChunkDlgBase()
{
}

DistribVegDlgBase::DistribVegDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer25;
	bSizer25 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_text16 = new wxStaticText( this, ID_TEXT, _("Grid spacing of sampling, in meters:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text16->Wrap( -1 );
	fgSizer1->Add( m_text16, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_sampling = new wxTextCtrl( this, ID_SAMPLING, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_sampling->SetMaxLength( 0 ); 
	fgSizer1->Add( m_sampling, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text17 = new wxStaticText( this, ID_TEXT, _("Artificial scarcity, set to 1.0 for full density:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text17->Wrap( -1 );
	fgSizer1->Add( m_text17, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_scarcity = new wxTextCtrl( this, ID_SCARCITY, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_scarcity->SetMaxLength( 0 ); 
	fgSizer1->Add( m_scarcity, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer25->Add( fgSizer1, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxStaticBoxSizer* sbSizer5;
	sbSizer5 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Species") ), wxVERTICAL );
	
	wxBoxSizer* bSizer26;
	bSizer26 = new wxBoxSizer( wxHORIZONTAL );
	
	m_species1 = new wxRadioButton( sbSizer5->GetStaticBox(), ID_SPECIES1, _("Single species:"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_species1->SetValue( true ); 
	bSizer26->Add( m_species1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxArrayString m_choice_speciesChoices;
	m_choice_species = new wxChoice( sbSizer5->GetStaticBox(), ID_CHOICE_SPECIES, wxDefaultPosition, wxSize( 200,-1 ), m_choice_speciesChoices, 0 );
	m_choice_species->SetSelection( 0 );
	bSizer26->Add( m_choice_species, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer5->Add( bSizer26, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer27;
	bSizer27 = new wxBoxSizer( wxHORIZONTAL );
	
	m_species2 = new wxRadioButton( sbSizer5->GetStaticBox(), ID_SPECIES2, _("Single biotype:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_species2->SetValue( true ); 
	bSizer27->Add( m_species2, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_choice_biotypeChoices;
	m_choice_biotype = new wxChoice( sbSizer5->GetStaticBox(), ID_CHOICE_BIOTYPE, wxDefaultPosition, wxSize( 200,-1 ), m_choice_biotypeChoices, 0 );
	m_choice_biotype->SetSelection( 0 );
	bSizer27->Add( m_choice_biotype, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer5->Add( bSizer27, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer28;
	bSizer28 = new wxBoxSizer( wxHORIZONTAL );
	
	m_species3 = new wxRadioButton( sbSizer5->GetStaticBox(), ID_SPECIES3, _("Biotype from layer:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_species3->SetValue( true ); 
	bSizer28->Add( m_species3, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_choice_biotype_layerChoices;
	m_choice_biotype_layer = new wxChoice( sbSizer5->GetStaticBox(), ID_CHOICE_BIOTYPE_LAYER, wxDefaultPosition, wxSize( 200,-1 ), m_choice_biotype_layerChoices, 0 );
	m_choice_biotype_layer->SetSelection( 0 );
	bSizer28->Add( m_choice_biotype_layer, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer5->Add( bSizer28, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer25->Add( sbSizer5, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer6;
	sbSizer6 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Density") ), wxVERTICAL );
	
	wxBoxSizer* bSizer29;
	bSizer29 = new wxBoxSizer( wxHORIZONTAL );
	
	m_density1 = new wxRadioButton( sbSizer6->GetStaticBox(), ID_DENSITY1, _("Fixed density:"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_density1->SetValue( true ); 
	bSizer29->Add( m_density1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text18_fixed_density = new wxTextCtrl( sbSizer6->GetStaticBox(), ID_TEXT_FIXED_DENSITY, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_text18_fixed_density->SetMaxLength( 0 ); 
	bSizer29->Add( m_text18_fixed_density, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text18 = new wxStaticText( sbSizer6->GetStaticBox(), wxID_ANY, _("plants per m2"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text18->Wrap( -1 );
	bSizer29->Add( m_text18, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer6->Add( bSizer29, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_density2 = new wxRadioButton( sbSizer6->GetStaticBox(), ID_DENSITY2, _("Inherit from biotype"), wxDefaultPosition, wxDefaultSize, 0 );
	m_density2->SetValue( true ); 
	sbSizer6->Add( m_density2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer30;
	bSizer30 = new wxBoxSizer( wxHORIZONTAL );
	
	m_density3 = new wxRadioButton( sbSizer6->GetStaticBox(), ID_DENSITY3, _("Combine density from layer:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_density3->SetValue( true ); 
	bSizer30->Add( m_density3, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_choice_density_layerChoices;
	m_choice_density_layer = new wxChoice( sbSizer6->GetStaticBox(), ID_CHOICE_DENSITY_LAYER, wxDefaultPosition, wxSize( 200,-1 ), m_choice_density_layerChoices, 0 );
	m_choice_density_layer->SetSelection( 0 );
	bSizer30->Add( m_choice_density_layer, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer6->Add( bSizer30, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer25->Add( sbSizer6, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer7;
	sbSizer7 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Size") ), wxVERTICAL );
	
	wxBoxSizer* bSizer31;
	bSizer31 = new wxBoxSizer( wxHORIZONTAL );
	
	m_size1 = new wxRadioButton( sbSizer7->GetStaticBox(), ID_SIZE1, _("Fixed size:"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_size1->SetValue( true ); 
	bSizer31->Add( m_size1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text_fixed_size = new wxTextCtrl( sbSizer7->GetStaticBox(), ID_TEXT_FIXED_SIZE, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_text_fixed_size->SetMaxLength( 0 ); 
	bSizer31->Add( m_text_fixed_size, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text19 = new wxStaticText( sbSizer7->GetStaticBox(), ID_TEXT, _("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text19->Wrap( 0 );
	bSizer31->Add( m_text19, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer7->Add( bSizer31, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer32;
	bSizer32 = new wxBoxSizer( wxHORIZONTAL );
	
	m_size2 = new wxRadioButton( sbSizer7->GetStaticBox(), ID_SIZE2, _("Randomize from:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_size2->SetValue( true ); 
	bSizer32->Add( m_size2, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_spin_random_from = new wxSpinCtrl( sbSizer7->GetStaticBox(), ID_SPIN_RANDOM_FROM, wxT("0"), wxDefaultPosition, wxSize( 60,-1 ), wxSP_ARROW_KEYS, 0, 100, 0 );
	bSizer32->Add( m_spin_random_from, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text20 = new wxStaticText( sbSizer7->GetStaticBox(), ID_TEXT, _("to"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text20->Wrap( 0 );
	bSizer32->Add( m_text20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_spin_random_to = new wxSpinCtrl( sbSizer7->GetStaticBox(), ID_SPIN_RANDOM_TO, wxT("0"), wxDefaultPosition, wxSize( 60,-1 ), wxSP_ARROW_KEYS, 0, 100, 0 );
	bSizer32->Add( m_spin_random_to, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text21 = new wxStaticText( sbSizer7->GetStaticBox(), ID_TEXT, _("percent of maximum"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text21->Wrap( -1 );
	bSizer32->Add( m_text21, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer7->Add( bSizer32, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer25->Add( sbSizer7, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer33;
	bSizer33 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer33->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer33->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer25->Add( bSizer33, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer25 );
	this->Layout();
	bSizer25->Fit( this );
	
	this->Centre( wxBOTH );
}

DistribVegDlgBase::~DistribVegDlgBase()
{
}

ExtentDlgBase::ExtentDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Extents") ), wxVERTICAL );
	
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text1 = new wxStaticText( sbSizer1->GetStaticBox(), ID_TEXT, _("North"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text1->Wrap( 0 );
	bSizer3->Add( m_text1, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer1->Add( bSizer3, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxTOP, 0 );
	
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxHORIZONTAL );
	
	m_extent_n = new wxTextCtrl( sbSizer1->GetStaticBox(), ID_EXTENT_N, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	m_extent_n->SetMaxLength( 0 ); 
	bSizer4->Add( m_extent_n, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	sbSizer1->Add( bSizer4, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text2 = new wxStaticText( sbSizer1->GetStaticBox(), ID_TEXT, _("West"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text2->Wrap( 0 );
	bSizer5->Add( m_text2, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_extent_w = new wxTextCtrl( sbSizer1->GetStaticBox(), ID_EXTENT_W, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	m_extent_w->SetMaxLength( 0 ); 
	bSizer5->Add( m_extent_w, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer5->Add( 10, 10, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_extent_e = new wxTextCtrl( sbSizer1->GetStaticBox(), ID_EXTENT_E, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	m_extent_e->SetMaxLength( 0 ); 
	bSizer5->Add( m_extent_e, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text3 = new wxStaticText( sbSizer1->GetStaticBox(), ID_TEXT, _("East"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text3->Wrap( 0 );
	bSizer5->Add( m_text3, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer1->Add( bSizer5, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxHORIZONTAL );
	
	m_extent_s = new wxTextCtrl( sbSizer1->GetStaticBox(), ID_EXTENT_S, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	m_extent_s->SetMaxLength( 0 ); 
	bSizer6->Add( m_extent_s, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	
	sbSizer1->Add( bSizer6, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text4 = new wxStaticText( sbSizer1->GetStaticBox(), ID_TEXT, _("South"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text4->Wrap( 0 );
	bSizer7->Add( m_text4, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer1->Add( bSizer7, 0, wxALIGN_CENTER, 5 );
	
	m_dms = new wxCheckBox( sbSizer1->GetStaticBox(), ID_DMS, _("Display coordinates as Degrees-Minutes-Seconds"), wxDefaultPosition, wxDefaultSize, 0 );
	m_dms->SetValue(true); 
	sbSizer1->Add( m_dms, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer2->Add( sbSizer1, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer8->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer8->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer2->Add( bSizer8, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer1->Add( bSizer2, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text5 = new wxStaticText( this, ID_TEXT, _("Origin and Size:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text5->Wrap( -1 );
	bSizer9->Add( m_text5, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_extent_all = new wxTextCtrl( this, ID_EXTENT_ALL, wxEmptyString, wxDefaultPosition, wxSize( 310,-1 ), 0 );
	m_extent_all->SetMaxLength( 0 ); 
	bSizer9->Add( m_extent_all, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer1->Add( bSizer9, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	this->SetSizer( bSizer1 );
	this->Layout();
	bSizer1->Fit( this );
	
	this->Centre( wxBOTH );
}

ExtentDlgBase::~ExtentDlgBase()
{
}

GenGridDlgBase::GenGridDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer176;
	bSizer176 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer40;
	sbSizer40 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Sampling") ), wxVERTICAL );
	
	wxBoxSizer* bSizer177;
	bSizer177 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text64 = new wxStaticText( sbSizer40->GetStaticBox(), ID_TEXT, _("Grid spacing:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text64->Wrap( -1 );
	bSizer177->Add( m_text64, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_spacingx = new wxTextCtrl( sbSizer40->GetStaticBox(), ID_SPACINGX, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_spacingx->SetMaxLength( 0 ); 
	bSizer177->Add( m_spacingx, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_spacingy = new wxTextCtrl( sbSizer40->GetStaticBox(), ID_SPACINGY, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_spacingy->SetMaxLength( 0 ); 
	bSizer177->Add( m_spacingy, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer40->Add( bSizer177, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer178;
	bSizer178 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text65 = new wxStaticText( sbSizer40->GetStaticBox(), ID_TEXT, _("Grid size:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text65->Wrap( -1 );
	bSizer178->Add( m_text65, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_sizex = new wxTextCtrl( sbSizer40->GetStaticBox(), ID_SIZEX, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_sizex->SetMaxLength( 0 ); 
	bSizer178->Add( m_sizex, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_sizey = new wxTextCtrl( sbSizer40->GetStaticBox(), ID_SIZEY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_sizey->SetMaxLength( 0 ); 
	bSizer178->Add( m_sizey, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer40->Add( bSizer178, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer179;
	bSizer179 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text66 = new wxStaticText( sbSizer40->GetStaticBox(), ID_TEXT, _("Distance cutoff factor (1-10):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text66->Wrap( -1 );
	bSizer179->Add( m_text66, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text_dist_cutoff = new wxTextCtrl( sbSizer40->GetStaticBox(), ID_TEXT_DIST_CUTOFF, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_text_dist_cutoff->SetMaxLength( 0 ); 
	bSizer179->Add( m_text_dist_cutoff, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer40->Add( bSizer179, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer176->Add( sbSizer40, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer180;
	bSizer180 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer180->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer180->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer176->Add( bSizer180, 0, wxALIGN_CENTER, 5 );
	
	
	this->SetSizer( bSizer176 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

GenGridDlgBase::~GenGridDlgBase()
{
}

GeocodeDlgBase::GeocodeDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer139;
	bSizer139 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer29;
	sbSizer29 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Data File") ), wxHORIZONTAL );
	
	m_file_data = new wxTextCtrl( sbSizer29->GetStaticBox(), ID_FILE_DATA, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_file_data->SetMaxLength( 0 ); 
	sbSizer29->Add( m_file_data, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_get_file_data = new wxButton( sbSizer29->GetStaticBox(), ID_GET_FILE_DATA, _("..."), wxDefaultPosition, wxSize( 22,-1 ), 0 );
	m_get_file_data->SetDefault(); 
	sbSizer29->Add( m_get_file_data, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer139->Add( sbSizer29, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer30;
	sbSizer30 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Geocoding") ), wxVERTICAL );
	
	m_check_use1 = new wxCheckBox( sbSizer30->GetStaticBox(), ID_CHECK_USE1, _("Use Geocode.us (Web, US street addresses)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_use1->SetValue(true); 
	sbSizer30->Add( m_check_use1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_check_use2 = new wxCheckBox( sbSizer30->GetStaticBox(), ID_CHECK_USE2, _("Use Gazeteer (US Cities)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_use2->SetValue(true); 
	sbSizer30->Add( m_check_use2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer140;
	bSizer140 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer140->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text34 = new wxStaticText( sbSizer30->GetStaticBox(), ID_TEXT, _("Places:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text34->Wrap( 0 );
	bSizer140->Add( m_text34, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_file_gaz = new wxTextCtrl( sbSizer30->GetStaticBox(), ID_FILE_GAZ, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_file_gaz->SetMaxLength( 0 ); 
	bSizer140->Add( m_file_gaz, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_get_file_gaz = new wxButton( sbSizer30->GetStaticBox(), ID_GET_FILE_GAZ, _("..."), wxDefaultPosition, wxSize( 22,-1 ), 0 );
	m_get_file_gaz->SetDefault(); 
	bSizer140->Add( m_get_file_gaz, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	
	sbSizer30->Add( bSizer140, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer141;
	bSizer141 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer141->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text35 = new wxStaticText( sbSizer30->GetStaticBox(), ID_TEXT, _("Zipcodes:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text35->Wrap( 0 );
	bSizer141->Add( m_text35, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_file_zip = new wxTextCtrl( sbSizer30->GetStaticBox(), ID_FILE_ZIP, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_file_zip->SetMaxLength( 0 ); 
	bSizer141->Add( m_file_zip, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_get_file_zip = new wxButton( sbSizer30->GetStaticBox(), ID_GET_FILE_ZIP, _("..."), wxDefaultPosition, wxSize( 22,-1 ), 0 );
	m_get_file_zip->SetDefault(); 
	bSizer141->Add( m_get_file_zip, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	
	sbSizer30->Add( bSizer141, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_check_use3 = new wxCheckBox( sbSizer30->GetStaticBox(), ID_CHECK_USE3, _("Use NIMA GEOnet (International Cities)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_use3->SetValue(true); 
	sbSizer30->Add( m_check_use3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer142;
	bSizer142 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer142->Add( 16, 16, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_file_gns = new wxTextCtrl( sbSizer30->GetStaticBox(), ID_FILE_GNS, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_file_gns->SetMaxLength( 0 ); 
	bSizer142->Add( m_file_gns, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_get_file_gns = new wxButton( sbSizer30->GetStaticBox(), ID_GET_FILE_GNS, _("..."), wxDefaultPosition, wxSize( 22,-1 ), 0 );
	m_get_file_gns->SetDefault(); 
	bSizer142->Add( m_get_file_gns, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer30->Add( bSizer142, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer139->Add( sbSizer30, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5 );
	
	wxBoxSizer* bSizer143;
	bSizer143 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer143->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer143->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer139->Add( bSizer143, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer139 );
	this->Layout();
	bSizer139->Fit( this );
	
	this->Centre( wxBOTH );
}

GeocodeDlgBase::~GeocodeDlgBase()
{
}

ImageMapDlgBase::ImageMapDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer127;
	bSizer127 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer128;
	bSizer128 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text29 = new wxStaticText( this, ID_TEXT, _("This operation will take the current contents of the\nwindow and export them a PNG file.\nA corresponding HTML file will be written with an\nimage map with clickable areas derived from\nthe polygon features of the active Raw layer."), wxDefaultPosition, wxDefaultSize, 0 );
	m_text29->Wrap( -1 );
	bSizer128->Add( m_text29, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer127->Add( bSizer128, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer129;
	bSizer129 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text30 = new wxStaticText( this, ID_TEXT, _("Link field:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text30->Wrap( -1 );
	bSizer129->Add( m_text30, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_link_fieldChoices;
	m_link_field = new wxChoice( this, ID_LINK_FIELD, wxDefaultPosition, wxSize( 100,-1 ), m_link_fieldChoices, 0 );
	m_link_field->SetSelection( 0 );
	bSizer129->Add( m_link_field, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer127->Add( bSizer129, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer130;
	bSizer130 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer130->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer130->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer127->Add( bSizer130, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	this->SetSizer( bSizer127 );
	this->Layout();
	bSizer127->Fit( this );
	
	this->Centre( wxBOTH );
}

ImageMapDlgBase::~ImageMapDlgBase()
{
}

ImportPointDlgBase::ImportPointDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer131;
	bSizer131 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer132;
	bSizer132 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text31 = new wxStaticText( this, ID_TEXT, _("Easting field:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text31->Wrap( -1 );
	bSizer132->Add( m_text31, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_eastingChoices;
	m_easting = new wxChoice( this, ID_EASTING, wxDefaultPosition, wxSize( 160,-1 ), m_eastingChoices, 0 );
	m_easting->SetSelection( 0 );
	bSizer132->Add( m_easting, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer131->Add( bSizer132, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer133;
	bSizer133 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text32 = new wxStaticText( this, ID_TEXT, _("Northing field:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text32->Wrap( -1 );
	bSizer133->Add( m_text32, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_northingChoices;
	m_northing = new wxChoice( this, ID_NORTHING, wxDefaultPosition, wxSize( 160,-1 ), m_northingChoices, 0 );
	m_northing->SetSelection( 0 );
	bSizer133->Add( m_northing, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer131->Add( bSizer133, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer134;
	bSizer134 = new wxBoxSizer( wxHORIZONTAL );
	
	m_check_elevation = new wxCheckBox( this, ID_CHECK_ELEVATION, _("Elevation field:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_elevation->SetValue(true); 
	bSizer134->Add( m_check_elevation, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_elevationChoices;
	m_elevation = new wxChoice( this, ID_ELEVATION, wxDefaultPosition, wxSize( 160,-1 ), m_elevationChoices, 0 );
	m_elevation->SetSelection( 0 );
	bSizer134->Add( m_elevation, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer131->Add( bSizer134, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer135;
	bSizer135 = new wxBoxSizer( wxHORIZONTAL );
	
	m_check_import_field = new wxCheckBox( this, ID_CHECK_IMPORT_FIELD, _("Import field directly:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_import_field->SetValue(true); 
	bSizer135->Add( m_check_import_field, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_import_fieldChoices;
	m_import_field = new wxChoice( this, ID_IMPORT_FIELD, wxDefaultPosition, wxSize( 160,-1 ), m_import_fieldChoices, 0 );
	m_import_field->SetSelection( 0 );
	bSizer135->Add( m_import_field, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer131->Add( bSizer135, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer136;
	bSizer136 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text33 = new wxStaticText( this, ID_TEXT, _("CRS:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text33->Wrap( 0 );
	bSizer136->Add( m_text33, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_crs = new wxTextCtrl( this, ID_CRS, wxEmptyString, wxDefaultPosition, wxSize( 180,-1 ), 0 );
	m_crs->SetMaxLength( 0 ); 
	bSizer136->Add( m_crs, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_set_crs = new wxButton( this, ID_SET_CRS, _("Set"), wxDefaultPosition, wxSize( 40,-1 ), 0 );
	m_set_crs->SetDefault(); 
	bSizer136->Add( m_set_crs, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer131->Add( bSizer136, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxStaticBoxSizer* sbSizer28;
	sbSizer28 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Number format:") ), wxVERTICAL );
	
	m_format_decimal = new wxRadioButton( sbSizer28->GetStaticBox(), ID_FORMAT_DECIMAL, _("Decimal"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_format_decimal->SetValue( true ); 
	sbSizer28->Add( m_format_decimal, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_format_dms = new wxRadioButton( sbSizer28->GetStaticBox(), ID_FORMAT_DMS, _("Packed degrees: DDDMMSSSS"), wxDefaultPosition, wxDefaultSize, 0 );
	m_format_dms->SetValue( true ); 
	sbSizer28->Add( m_format_dms, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_format_hdm = new wxRadioButton( sbSizer28->GetStaticBox(), ID_FORMAT_HDM, _("Hemisphere Degrees Minutes: HDDD MM.MMM"), wxDefaultPosition, wxDefaultSize, 0 );
	m_format_hdm->SetValue( true ); 
	sbSizer28->Add( m_format_hdm, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer131->Add( sbSizer28, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer137;
	bSizer137 = new wxBoxSizer( wxHORIZONTAL );
	
	m_longitude_west = new wxCheckBox( this, ID_LONGITUDE_WEST, _("Longitude is west"), wxDefaultPosition, wxDefaultSize, 0 );
	m_longitude_west->SetValue(true); 
	bSizer137->Add( m_longitude_west, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer131->Add( bSizer137, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	wxBoxSizer* bSizer138;
	bSizer138 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer138->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer138->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer131->Add( bSizer138, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer131 );
	this->Layout();
	bSizer131->Fit( this );
	
	this->Centre( wxBOTH );
}

ImportPointDlgBase::~ImportPointDlgBase()
{
}

ImportStructDlgBase::ImportStructDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer72;
	bSizer72 = new wxBoxSizer( wxVERTICAL );
	
	m_text47 = new wxStaticText( this, ID_TEXT, _("Structure Type:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text47->Wrap( -1 );
	bSizer72->Add( m_text47, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer73;
	bSizer73 = new wxBoxSizer( wxVERTICAL );
	
	m_type_linear = new wxRadioButton( this, ID_TYPE_LINEAR, _("Linear (fences or walls)"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_type_linear->SetValue( true ); 
	bSizer73->Add( m_type_linear, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_type_instance = new wxRadioButton( this, ID_TYPE_INSTANCE, _("Instances (external model references)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_type_instance->SetValue( true ); 
	bSizer73->Add( m_type_instance, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer74;
	bSizer74 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer74->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	m_text48 = new wxStaticText( this, ID_TEXT, _("Filename field:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text48->Wrap( -1 );
	bSizer74->Add( m_text48, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	wxArrayString m_choice_file_fieldChoices;
	m_choice_file_field = new wxChoice( this, ID_CHOICE_FILE_FIELD, wxDefaultPosition, wxSize( 100,-1 ), m_choice_file_fieldChoices, 0 );
	m_choice_file_field->SetSelection( 0 );
	bSizer74->Add( m_choice_file_field, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer73->Add( bSizer74, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_type_center = new wxRadioButton( this, ID_TYPE_CENTER, _("Buildings (parametric by center)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_type_center->SetValue( true ); 
	bSizer73->Add( m_type_center, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_type_footprint = new wxRadioButton( this, ID_TYPE_FOOTPRINT, _("Buildings (parametric by footprint, polygon or closed polyline)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_type_footprint->SetValue( true ); 
	bSizer73->Add( m_type_footprint, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer75;
	bSizer75 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer75->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	m_text49 = new wxStaticText( this, ID_TEXT, _("Height field:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text49->Wrap( -1 );
	bSizer75->Add( m_text49, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_choice_height_fieldChoices;
	m_choice_height_field = new wxChoice( this, ID_CHOICE_HEIGHT_FIELD, wxDefaultPosition, wxSize( 100,-1 ), m_choice_height_fieldChoices, 0 );
	m_choice_height_field->SetSelection( 0 );
	bSizer75->Add( m_choice_height_field, 1, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	wxString m_choice_height_typeChoices[] = { _("Stories"), _("Meters"), _("Feet"), _("Meters (single story)"), _("Feet (single story)") };
	int m_choice_height_typeNChoices = sizeof( m_choice_height_typeChoices ) / sizeof( wxString );
	m_choice_height_type = new wxChoice( this, ID_CHOICE_HEIGHT_TYPE, wxDefaultPosition, wxSize( 80,-1 ), m_choice_height_typeNChoices, m_choice_height_typeChoices, 0 );
	m_choice_height_type->SetSelection( 0 );
	bSizer75->Add( m_choice_height_type, 1, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer73->Add( bSizer75, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer76;
	bSizer76 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer76->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	m_text50 = new wxStaticText( this, ID_TEXT, _("Color:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text50->Wrap( 0 );
	bSizer76->Add( m_text50, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer73->Add( bSizer76, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer77;
	bSizer77 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer77->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	m_radio_color_default = new wxRadioButton( this, ID_RADIO_COLOR_DEFAULT, _("Default"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radio_color_default->SetValue( true ); 
	bSizer77->Add( m_radio_color_default, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer73->Add( bSizer77, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer78;
	bSizer78 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer78->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	m_radio_color_fixed = new wxRadioButton( this, ID_RADIO_COLOR_FIXED, _("Building:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio_color_fixed->SetValue( true ); 
	bSizer78->Add( m_radio_color_fixed, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_color3 = new wxStaticBitmap( this, ID_COLOR3, wxBitmap( dummy_32x18_xpm ), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer78->Add( m_color3, 0, wxALIGN_CENTER|wxLEFT|wxTOP|wxBOTTOM, 5 );
	
	m_set_color1 = new wxButton( this, ID_SET_COLOR1, _("Set"), wxDefaultPosition, wxSize( 40,-1 ), 0 );
	m_set_color1->SetDefault(); 
	bSizer78->Add( m_set_color1, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	m_text1 = new wxStaticText( this, ID_TEXT, _("Roof:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text1->Wrap( 0 );
	bSizer78->Add( m_text1, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_color4 = new wxStaticBitmap( this, ID_COLOR4, wxBitmap( dummy_32x18_xpm ), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer78->Add( m_color4, 0, wxALIGN_CENTER|wxLEFT|wxTOP|wxBOTTOM, 5 );
	
	m_set_color2 = new wxButton( this, ID_SET_COLOR2, _("Set"), wxDefaultPosition, wxSize( 40,-1 ), 0 );
	m_set_color2->SetDefault(); 
	bSizer78->Add( m_set_color2, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	
	bSizer73->Add( bSizer78, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer79;
	bSizer79 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer79->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	m_text2 = new wxStaticText( this, ID_TEXT, _("Roof type:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text2->Wrap( -1 );
	bSizer79->Add( m_text2, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer73->Add( bSizer79, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer80;
	bSizer80 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer80->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	m_radio_roof_default = new wxRadioButton( this, ID_RADIO_ROOF_DEFAULT, _("Default"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radio_roof_default->SetValue( true ); 
	bSizer80->Add( m_radio_roof_default, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer73->Add( bSizer80, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer81;
	bSizer81 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer81->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	m_radio_roof_single = new wxRadioButton( this, ID_RADIO_ROOF_SINGLE, _("Single type:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio_roof_single->SetValue( true ); 
	bSizer81->Add( m_radio_roof_single, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_choice_roof_typeChoices[] = { _("Flat"), _("Shed"), _("Gable"), _("Hip") };
	int m_choice_roof_typeNChoices = sizeof( m_choice_roof_typeChoices ) / sizeof( wxString );
	m_choice_roof_type = new wxChoice( this, ID_CHOICE_ROOF_TYPE, wxDefaultPosition, wxSize( 100,-1 ), m_choice_roof_typeNChoices, m_choice_roof_typeChoices, 0 );
	m_choice_roof_type->SetSelection( 0 );
	bSizer81->Add( m_choice_roof_type, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	m_text3 = new wxStaticText( this, ID_TEXT, _("Slope:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text3->Wrap( 0 );
	bSizer81->Add( m_text3, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_slopecntr = new wxSpinCtrl( this, ID_SLOPECNTR, wxT("15"), wxDefaultPosition, wxSize( 55,-1 ), wxSP_ARROW_KEYS, 1, 89, 15 );
	bSizer81->Add( m_slopecntr, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer73->Add( bSizer81, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer82;
	bSizer82 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer82->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	m_radio_roof_field = new wxRadioButton( this, ID_RADIO_ROOF_FIELD, _("Field:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio_roof_field->SetValue( true ); 
	bSizer82->Add( m_radio_roof_field, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_choice_roof_fieldChoices;
	m_choice_roof_field = new wxChoice( this, ID_CHOICE_ROOF_FIELD, wxDefaultPosition, wxSize( 100,-1 ), m_choice_roof_fieldChoices, 0 );
	m_choice_roof_field->SetSelection( 0 );
	bSizer82->Add( m_choice_roof_field, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer73->Add( bSizer82, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer72->Add( bSizer73, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer83;
	bSizer83 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text4 = new wxStaticText( this, ID_TEXT, _("Defaults from:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text4->Wrap( -1 );
	bSizer83->Add( m_text4, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_defaults_file = new wxTextCtrl( this, ID_DEFAULTS_FILE, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), wxTE_READONLY );
	m_defaults_file->SetMaxLength( 0 ); 
	bSizer83->Add( m_defaults_file, 1, wxALIGN_CENTER|wxRIGHT|wxTOP|wxBOTTOM, 5 );
	
	
	bSizer72->Add( bSizer83, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_inside_area = new wxCheckBox( this, ID_INSIDE_AREA, _("Only import features inside the Area Tool extents"), wxDefaultPosition, wxDefaultSize, 0 );
	m_inside_area->SetValue(true); 
	bSizer72->Add( m_inside_area, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer84;
	bSizer84 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer84->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer84->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer72->Add( bSizer84, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer72 );
	this->Layout();
	bSizer72->Fit( this );
	
	this->Centre( wxBOTH );
}

ImportStructDlgBase::~ImportStructDlgBase()
{
}

ImportStructOGRDlgBase::ImportStructOGRDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer85;
	bSizer85 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer16;
	sbSizer16 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Layers") ), wxHORIZONTAL );
	
	m_text5 = new wxStaticText( sbSizer16->GetStaticBox(), ID_TEXT, _("Import from"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text5->Wrap( -1 );
	sbSizer16->Add( m_text5, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_layernameChoices;
	m_layername = new wxChoice( sbSizer16->GetStaticBox(), ID_LAYERNAME, wxDefaultPosition, wxSize( 200,-1 ), m_layernameChoices, 0 );
	m_layername->SetSelection( 0 );
	sbSizer16->Add( m_layername, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer85->Add( sbSizer16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer17;
	sbSizer17 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Fields") ), wxVERTICAL );
	
	wxBoxSizer* bSizer86;
	bSizer86 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text6 = new wxStaticText( sbSizer17->GetStaticBox(), ID_TEXT, _("Feature height"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text6->Wrap( -1 );
	bSizer86->Add( m_text6, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_choice_height_fieldChoices;
	m_choice_height_field = new wxChoice( sbSizer17->GetStaticBox(), ID_CHOICE_HEIGHT_FIELD, wxDefaultPosition, wxSize( 100,-1 ), m_choice_height_fieldChoices, 0 );
	m_choice_height_field->SetSelection( 0 );
	bSizer86->Add( m_choice_height_field, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_choice_height_typeChoices[] = { _("Stories"), _("Meters"), _("Feet") };
	int m_choice_height_typeNChoices = sizeof( m_choice_height_typeChoices ) / sizeof( wxString );
	m_choice_height_type = new wxChoice( sbSizer17->GetStaticBox(), ID_CHOICE_HEIGHT_TYPE, wxDefaultPosition, wxSize( 80,-1 ), m_choice_height_typeNChoices, m_choice_height_typeChoices, 0 );
	m_choice_height_type->SetSelection( 0 );
	bSizer86->Add( m_choice_height_type, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer17->Add( bSizer86, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer85->Add( sbSizer17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer18;
	sbSizer18 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Structure type") ), wxVERTICAL );
	
	m_type_building = new wxRadioButton( sbSizer18->GetStaticBox(), ID_TYPE_BUILDING, _("Buildings"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_type_building->SetValue( true ); 
	sbSizer18->Add( m_type_building, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer87;
	bSizer87 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer87->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	m_build_foundations = new wxCheckBox( sbSizer18->GetStaticBox(), ID_BUILD_FOUNDATIONS, _("Build foundations"), wxDefaultPosition, wxDefaultSize, 0 );
	m_build_foundations->SetValue(true); 
	bSizer87->Add( m_build_foundations, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer18->Add( bSizer87, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	m_type_linear = new wxRadioButton( sbSizer18->GetStaticBox(), ID_TYPE_LINEAR, _("Linear (fences or walls)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_type_linear->SetValue( true ); 
	sbSizer18->Add( m_type_linear, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_type_instance = new wxRadioButton( sbSizer18->GetStaticBox(), ID_TYPE_INSTANCE, _("Instances (external model references)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_type_instance->SetValue( true ); 
	sbSizer18->Add( m_type_instance, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer88;
	bSizer88 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer88->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text7 = new wxStaticText( sbSizer18->GetStaticBox(), ID_TEXT, _("Filename field:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text7->Wrap( -1 );
	bSizer88->Add( m_text7, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_choice_file_fieldChoices;
	m_choice_file_field = new wxChoice( sbSizer18->GetStaticBox(), ID_CHOICE_FILE_FIELD, wxDefaultPosition, wxSize( 100,-1 ), m_choice_file_fieldChoices, 0 );
	m_choice_file_field->SetSelection( 0 );
	bSizer88->Add( m_choice_file_field, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer18->Add( bSizer88, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer85->Add( sbSizer18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer19;
	sbSizer19 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Options") ), wxVERTICAL );
	
	m_inside_area = new wxCheckBox( sbSizer19->GetStaticBox(), ID_INSIDE_AREA, _("Only import features inside the Area Tool extents"), wxDefaultPosition, wxDefaultSize, 0 );
	m_inside_area->SetValue(true); 
	sbSizer19->Add( m_inside_area, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_use_25d = new wxCheckBox( sbSizer19->GetStaticBox(), ID_USE_25D, _("Use 2.5D for elevation"), wxDefaultPosition, wxDefaultSize, 0 );
	m_use_25d->SetValue(true); 
	sbSizer19->Add( m_use_25d, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer85->Add( sbSizer19, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer89;
	bSizer89 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer89->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer89->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer85->Add( bSizer89, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer85 );
	this->Layout();
	bSizer85->Fit( this );
	
	this->Centre( wxBOTH );
}

ImportStructOGRDlgBase::~ImportStructOGRDlgBase()
{
}

ImportVegDlgBase::ImportVegDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer22;
	bSizer22 = new wxBoxSizer( wxVERTICAL );
	
	m_text14 = new wxStaticText( this, ID_TEXT, _("Field to use:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text14->Wrap( -1 );
	bSizer22->Add( m_text14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_field = new wxComboBox( this, ID_FIELD, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN|wxCB_READONLY ); 
	bSizer22->Add( m_field, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer21->Add( bSizer22, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer23;
	bSizer23 = new wxBoxSizer( wxVERTICAL );
	
	m_text15 = new wxStaticText( this, ID_TEXT, _("Interpretation of the field:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text15->Wrap( -1 );
	bSizer23->Add( m_text15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_density = new wxRadioButton( this, ID_DENSITY, _("Density (0 to 1) (double)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_density->SetValue( true ); 
	bSizer23->Add( m_density, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_biotype1 = new wxRadioButton( this, ID_BIOTYPE1, _("Biotype by name (string)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_biotype1->SetValue( true ); 
	bSizer23->Add( m_biotype1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_biotype2 = new wxRadioButton( this, ID_BIOTYPE2, _("Biotype by ID (integer)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_biotype2->SetValue( true ); 
	bSizer23->Add( m_biotype2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer21->Add( bSizer23, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer24;
	bSizer24 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer24->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer24->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer21->Add( bSizer24, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer21 );
	this->Layout();
	bSizer21->Fit( this );
	
	this->Centre( wxBOTH );
}

ImportVegDlgBase::~ImportVegDlgBase()
{
}

LayerPropDlgBase::LayerPropDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer15;
	bSizer15 = new wxBoxSizer( wxVERTICAL );
	
	m_props = new wxTextCtrl( this, ID_PROPS, wxEmptyString, wxDefaultPosition, wxSize( 300,120 ), wxTE_MULTILINE );
	bSizer15->Add( m_props, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer4;
	sbSizer4 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Extents") ), wxVERTICAL );
	
	wxBoxSizer* bSizer16;
	bSizer16 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text10 = new wxStaticText( sbSizer4->GetStaticBox(), ID_TEXT, _("Left"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text10->Wrap( 0 );
	bSizer16->Add( m_text10, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_left = new wxTextCtrl( sbSizer4->GetStaticBox(), ID_LEFT, wxEmptyString, wxDefaultPosition, wxSize( 100,-1 ), 0 );
	m_left->SetMaxLength( 0 ); 
	bSizer16->Add( m_left, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	sbSizer4->Add( bSizer16, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer17;
	bSizer17 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text11 = new wxStaticText( sbSizer4->GetStaticBox(), ID_TEXT, _("Top"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text11->Wrap( 0 );
	bSizer17->Add( m_text11, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_top = new wxTextCtrl( sbSizer4->GetStaticBox(), ID_TOP, wxEmptyString, wxDefaultPosition, wxSize( 100,-1 ), 0 );
	m_top->SetMaxLength( 0 ); 
	bSizer17->Add( m_top, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	sbSizer4->Add( bSizer17, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text12 = new wxStaticText( sbSizer4->GetStaticBox(), ID_TEXT, _("Right"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text12->Wrap( 0 );
	bSizer18->Add( m_text12, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_right = new wxTextCtrl( sbSizer4->GetStaticBox(), ID_RIGHT, wxEmptyString, wxDefaultPosition, wxSize( 100,-1 ), 0 );
	m_right->SetMaxLength( 0 ); 
	bSizer18->Add( m_right, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	sbSizer4->Add( bSizer18, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	wxBoxSizer* bSizer19;
	bSizer19 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text13 = new wxStaticText( sbSizer4->GetStaticBox(), ID_TEXT, _("Bottom"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text13->Wrap( 0 );
	bSizer19->Add( m_text13, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_bottom = new wxTextCtrl( sbSizer4->GetStaticBox(), ID_BOTTOM, wxEmptyString, wxDefaultPosition, wxSize( 100,-1 ), 0 );
	m_bottom->SetMaxLength( 0 ); 
	bSizer19->Add( m_bottom, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	sbSizer4->Add( bSizer19, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	
	bSizer15->Add( sbSizer4, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer20->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer20->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer15->Add( bSizer20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer15 );
	this->Layout();
	bSizer15->Fit( this );
	
	this->Centre( wxBOTH );
}

LayerPropDlgBase::~LayerPropDlgBase()
{
}

LevelSelectionDlgBase::LevelSelectionDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer90;
	bSizer90 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer91;
	bSizer91 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text8 = new wxStaticText( this, ID_TEXT, _("Select level to edit"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text8->Wrap( -1 );
	bSizer91->Add( m_text8, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_level_choiceChoices;
	m_level_choice = new wxChoice( this, ID_LEVEL_CHOICE, wxDefaultPosition, wxSize( 100,-1 ), m_level_choiceChoices, 0 );
	m_level_choice->SetSelection( 0 );
	bSizer91->Add( m_level_choice, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer90->Add( bSizer91, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer92;
	bSizer92 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer92->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer92->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer90->Add( bSizer92, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer90 );
	this->Layout();
	bSizer90->Fit( this );
	
	this->Centre( wxBOTH );
}

LevelSelectionDlgBase::~LevelSelectionDlgBase()
{
}

MapServerDlgBase::MapServerDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer103;
	bSizer103 = new wxBoxSizer( wxVERTICAL );
	
	m_text13 = new wxStaticText( this, ID_TEXT, _("WMS base URL:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text13->Wrap( -1 );
	bSizer103->Add( m_text13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_base_url = new wxComboBox( this, ID_BASE_URL, wxEmptyString, wxDefaultPosition, wxSize( 400,-1 ), 0, NULL, wxCB_DROPDOWN ); 
	bSizer103->Add( m_base_url, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_query_layers = new wxButton( this, ID_QUERY_LAYERS, _("Query Layers"), wxDefaultPosition, wxDefaultSize, 0 );
	m_query_layers->SetDefault(); 
	bSizer103->Add( m_query_layers, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxFlexGridSizer* fgSizer6;
	fgSizer6 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer6->AddGrowableCol( 0 );
	fgSizer6->AddGrowableCol( 1 );
	fgSizer6->AddGrowableRow( 1 );
	fgSizer6->SetFlexibleDirection( wxBOTH );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_text14 = new wxStaticText( this, ID_TEXT, _("Layer:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text14->Wrap( 0 );
	fgSizer6->Add( m_text14, 0, wxALL, 5 );
	
	m_text15 = new wxStaticText( this, ID_TEXT, _("Description:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text15->Wrap( 0 );
	fgSizer6->Add( m_text15, 0, wxALL, 5 );
	
	m_list_layers = new wxListBox( this, ID_LIST_LAYERS, wxDefaultPosition, wxSize( -1,100 ), 0, NULL, wxLB_SINGLE ); 
	fgSizer6->Add( m_list_layers, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	m_layer_desc = new wxTextCtrl( this, ID_LAYER_DESC, wxEmptyString, wxDefaultPosition, wxSize( -1,100 ), wxTE_MULTILINE );
	fgSizer6->Add( m_layer_desc, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer103->Add( fgSizer6, 2, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	wxFlexGridSizer* fgSizer7;
	fgSizer7 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer7->AddGrowableCol( 0 );
	fgSizer7->AddGrowableCol( 1 );
	fgSizer7->AddGrowableRow( 1 );
	fgSizer7->SetFlexibleDirection( wxBOTH );
	fgSizer7->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_text16 = new wxStaticText( this, ID_TEXT, _("Style:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text16->Wrap( 0 );
	fgSizer7->Add( m_text16, 0, wxALL, 5 );
	
	m_text17 = new wxStaticText( this, ID_TEXT, _("Description:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text17->Wrap( 0 );
	fgSizer7->Add( m_text17, 0, wxALL, 5 );
	
	m_list_styles = new wxListBox( this, ID_LIST_STYLES, wxDefaultPosition, wxSize( -1,40 ), 0, NULL, wxLB_SINGLE ); 
	fgSizer7->Add( m_list_styles, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	m_style_desc = new wxTextCtrl( this, ID_STYLE_DESC, wxEmptyString, wxDefaultPosition, wxSize( -1,40 ), wxTE_MULTILINE );
	fgSizer7->Add( m_style_desc, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer103->Add( fgSizer7, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	wxBoxSizer* bSizer104;
	bSizer104 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text18 = new wxStaticText( this, ID_TEXT, _("Width:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text18->Wrap( 0 );
	bSizer104->Add( m_text18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_width = new wxTextCtrl( this, ID_WIDTH, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_width->SetMaxLength( 0 ); 
	bSizer104->Add( m_width, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text19 = new wxStaticText( this, ID_TEXT, _("Height:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text19->Wrap( 0 );
	bSizer104->Add( m_text19, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_height = new wxTextCtrl( this, ID_HEIGHT, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_height->SetMaxLength( 0 ); 
	bSizer104->Add( m_height, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text20 = new wxStaticText( this, ID_TEXT, _("Format"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text20->Wrap( 0 );
	bSizer104->Add( m_text20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_choice_formatChoices;
	m_choice_format = new wxChoice( this, ID_CHOICE_FORMAT, wxDefaultPosition, wxSize( 100,-1 ), m_choice_formatChoices, 0 );
	m_choice_format->SetSelection( 0 );
	bSizer104->Add( m_choice_format, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer103->Add( bSizer104, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 0 );
	
	m_text21 = new wxStaticText( this, ID_TEXT, _("Query URL:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text21->Wrap( -1 );
	bSizer103->Add( m_text21, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_query = new wxTextCtrl( this, ID_QUERY, wxEmptyString, wxDefaultPosition, wxSize( -1,60 ), wxTE_MULTILINE );
	bSizer103->Add( m_query, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer23;
	sbSizer23 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Output") ), wxVERTICAL );
	
	m_radio_create_new = new wxRadioButton( sbSizer23->GetStaticBox(), ID_RADIO_CREATE_NEW, _("Create new layer"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radio_create_new->SetValue( true ); 
	sbSizer23->Add( m_radio_create_new, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer105;
	bSizer105 = new wxBoxSizer( wxHORIZONTAL );
	
	m_radio_to_file = new wxRadioButton( sbSizer23->GetStaticBox(), ID_RADIO_TO_FILE, _("To file"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio_to_file->SetValue( true ); 
	bSizer105->Add( m_radio_to_file, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text_to_file = new wxTextCtrl( sbSizer23->GetStaticBox(), ID_TEXT_TO_FILE, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_text_to_file->SetMaxLength( 0 ); 
	bSizer105->Add( m_text_to_file, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_dotdotdot = new wxButton( sbSizer23->GetStaticBox(), ID_DOTDOTDOT, _("..."), wxDefaultPosition, wxSize( 24,-1 ), 0 );
	m_dotdotdot->SetDefault(); 
	m_dotdotdot->SetFont( wxFont( 11, 72, 90, 92, false, wxEmptyString ) );
	
	bSizer105->Add( m_dotdotdot, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer23->Add( bSizer105, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer103->Add( sbSizer23, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer106;
	bSizer106 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer106->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer106->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer103->Add( bSizer106, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer103 );
	this->Layout();
	bSizer103->Fit( this );
	
	this->Centre( wxBOTH );
}

MapServerDlgBase::~MapServerDlgBase()
{
}

MatchDlgBase::MatchDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer159;
	bSizer159 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer35;
	sbSizer35 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Origin and Size:") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer11;
	fgSizer11 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer11->AddGrowableCol( 1 );
	fgSizer11->SetFlexibleDirection( wxBOTH );
	fgSizer11->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_text52 = new wxStaticText( sbSizer35->GetStaticBox(), ID_TEXT, _("Previous:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text52->Wrap( 0 );
	fgSizer11->Add( m_text52, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_extent1 = new wxTextCtrl( sbSizer35->GetStaticBox(), ID_EXTENT1, wxEmptyString, wxDefaultPosition, wxSize( 280,-1 ), wxTE_READONLY );
	m_extent1->SetMaxLength( 0 ); 
	fgSizer11->Add( m_extent1, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text53 = new wxStaticText( sbSizer35->GetStaticBox(), ID_TEXT, _("Current:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text53->Wrap( 0 );
	fgSizer11->Add( m_text53, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_extent2 = new wxTextCtrl( sbSizer35->GetStaticBox(), ID_EXTENT2, wxEmptyString, wxDefaultPosition, wxSize( 280,-1 ), wxTE_READONLY );
	m_extent2->SetMaxLength( 0 ); 
	fgSizer11->Add( m_extent2, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	sbSizer35->Add( fgSizer11, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer159->Add( sbSizer35, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer160;
	bSizer160 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text54 = new wxStaticText( this, ID_TEXT, _("Match Layer:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text54->Wrap( -1 );
	bSizer160->Add( m_text54, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxString m_match_layerChoices[] = { _("ChoiceItem") };
	int m_match_layerNChoices = sizeof( m_match_layerChoices ) / sizeof( wxString );
	m_match_layer = new wxChoice( this, ID_MATCH_LAYER, wxDefaultPosition, wxSize( 200,-1 ), m_match_layerNChoices, m_match_layerChoices, 0 );
	m_match_layer->SetSelection( 0 );
	bSizer160->Add( m_match_layer, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer159->Add( bSizer160, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer161;
	bSizer161 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text55 = new wxStaticText( this, ID_TEXT, _("Layer Resolution:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text55->Wrap( -1 );
	bSizer161->Add( m_text55, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_layer_res = new wxTextCtrl( this, ID_LAYER_RES, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_layer_res->SetMaxLength( 0 ); 
	bSizer161->Add( m_layer_res, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer159->Add( bSizer161, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer162;
	bSizer162 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text56 = new wxStaticText( this, ID_TEXT, _("Allow area to:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text56->Wrap( -1 );
	bSizer162->Add( m_text56, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_grow = new wxCheckBox( this, ID_GROW, _("Grow"), wxDefaultPosition, wxDefaultSize, 0 );
	m_grow->SetValue(true); 
	bSizer162->Add( m_grow, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_shrink = new wxCheckBox( this, ID_SHRINK, _("Shrink"), wxDefaultPosition, wxDefaultSize, 0 );
	m_shrink->SetValue(true); 
	bSizer162->Add( m_shrink, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer159->Add( bSizer162, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer163;
	bSizer163 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text57 = new wxStaticText( this, ID_TEXT, _("Tile LOD0 Size:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text57->Wrap( -1 );
	bSizer163->Add( m_text57, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_tile_size = new wxTextCtrl( this, ID_TILE_SIZE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_tile_size->SetMaxLength( 0 ); 
	bSizer163->Add( m_tile_size, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_size_spin = new wxSpinButton( this, ID_SIZE_SPIN, wxDefaultPosition, wxSize( -1,26 ), 0 );
	bSizer163->Add( m_size_spin, 0, wxALIGN_CENTER|wxRIGHT, 5 );
	
	
	bSizer159->Add( bSizer163, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer164;
	bSizer164 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text58 = new wxStaticText( this, ID_TEXT, _("Tiling:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text58->Wrap( 0 );
	bSizer164->Add( m_text58, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_tiling = new wxTextCtrl( this, ID_TILING, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_tiling->SetMaxLength( 0 ); 
	bSizer164->Add( m_tiling, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer159->Add( bSizer164, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer165;
	bSizer165 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer165->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer165->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer159->Add( bSizer165, 0, wxALIGN_CENTER, 5 );
	
	
	this->SetSizer( bSizer159 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

MatchDlgBase::~MatchDlgBase()
{
}

NodeDlgBase::NodeDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer40;
	bSizer40 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer41;
	bSizer41 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer4;
	fgSizer4 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer4->SetFlexibleDirection( wxBOTH );
	fgSizer4->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_text36 = new wxStaticText( this, ID_TEXT, _("Intersection Type"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text36->Wrap( 0 );
	fgSizer4->Add( m_text36, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	m_inttype = new wxListBox( this, ID_INTTYPE, wxDefaultPosition, wxSize( 115,130 ), 0, NULL, wxLB_SINGLE ); 
	fgSizer4->Add( m_inttype, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text37 = new wxStaticText( this, ID_TEXT, _("Road Number"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text37->Wrap( 0 );
	fgSizer4->Add( m_text37, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	m_roadnum = new wxListBox( this, ID_ROADNUM, wxDefaultPosition, wxSize( 70,70 ), 0, NULL, wxLB_SINGLE ); 
	fgSizer4->Add( m_roadnum, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text38 = new wxStaticText( this, ID_TEXT, _("Behavior"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text38->Wrap( 0 );
	fgSizer4->Add( m_text38, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	m_behavior = new wxListBox( this, ID_BEHAVIOR, wxDefaultPosition, wxSize( 120,55 ), 0, NULL, wxLB_SINGLE ); 
	fgSizer4->Add( m_behavior, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer41->Add( fgSizer4, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer42;
	bSizer42 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer42->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer42->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer41->Add( bSizer42, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer40->Add( bSizer41, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	
	this->SetSizer( bSizer40 );
	this->Layout();
	bSizer40->Fit( this );
	
	this->Centre( wxBOTH );
}

NodeDlgBase::~NodeDlgBase()
{
}

OptionsDlgBase::OptionsDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer61;
	bSizer61 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer12;
	sbSizer12 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Status Bar") ), wxVERTICAL );
	
	m_minutes = new wxCheckBox( sbSizer12->GetStaticBox(), ID_MINUTES, _("Show minutes and seconds for degree values"), wxDefaultPosition, wxDefaultSize, 0 );
	m_minutes->SetValue(true); 
	sbSizer12->Add( m_minutes, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer62;
	bSizer62 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text46 = new wxStaticText( sbSizer12->GetStaticBox(), ID_TEXT, _("Show elevation values as:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text46->Wrap( -1 );
	bSizer62->Add( m_text46, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_elevunitChoices;
	m_elevunit = new wxChoice( sbSizer12->GetStaticBox(), ID_ELEVUNIT, wxDefaultPosition, wxSize( 100,-1 ), m_elevunitChoices, 0 );
	m_elevunit->SetSelection( 0 );
	bSizer62->Add( m_elevunit, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer12->Add( bSizer62, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer61->Add( sbSizer12, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer13;
	sbSizer13 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Elevation") ), wxVERTICAL );
	
	m_radio_outline_only = new wxRadioButton( sbSizer13->GetStaticBox(), ID_RADIO_OUTLINE_ONLY, _("Show outline only (faster, saves memory)"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radio_outline_only->SetValue( true ); 
	sbSizer13->Add( m_radio_outline_only, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_radio_color = new wxRadioButton( sbSizer13->GetStaticBox(), ID_RADIO_COLOR, _("Artificial color by elevation value"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio_color->SetValue( true ); 
	sbSizer13->Add( m_radio_color, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxFlexGridSizer* fgSizer5;
	fgSizer5 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer5->SetFlexibleDirection( wxBOTH );
	fgSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	fgSizer5->Add( 20, 5, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_render_options = new wxButton( sbSizer13->GetStaticBox(), ID_RENDER_OPTIONS, _("Rendering options..."), wxDefaultPosition, wxDefaultSize, 0 );
	m_render_options->SetDefault(); 
	fgSizer5->Add( m_render_options, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	sbSizer13->Add( fgSizer5, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_check_draw_tin_simple = new wxCheckBox( sbSizer13->GetStaticBox(), ID_CHECK_DRAW_TIN_SIMPLE, _("Draw TIN elevation simplified"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_draw_tin_simple->SetValue(true); 
	sbSizer13->Add( m_check_draw_tin_simple, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer61->Add( sbSizer13, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer63;
	bSizer63 = new wxBoxSizer( wxVERTICAL );
	
	m_check_show_road_width = new wxCheckBox( this, ID_CHECK_SHOW_ROAD_WIDTH, _("Show roads with width"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_show_road_width->SetValue(true); 
	bSizer63->Add( m_check_show_road_width, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_check_draw_raw_simple = new wxCheckBox( this, ID_CHECK_DRAW_RAW_SIMPLE, _("Draw raw layers simplified"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_draw_raw_simple->SetValue(true); 
	bSizer63->Add( m_check_draw_raw_simple, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_pathnames = new wxCheckBox( this, ID_PATHNAMES, _("Show full pathnames in Layer View"), wxDefaultPosition, wxDefaultSize, 0 );
	m_pathnames->SetValue(true); 
	bSizer63->Add( m_pathnames, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer61->Add( bSizer63, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer64;
	bSizer64 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer64->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer64->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer61->Add( bSizer64, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer61 );
	this->Layout();
	bSizer61->Fit( this );
	
	this->Centre( wxBOTH );
}

OptionsDlgBase::~OptionsDlgBase()
{
}

PrefDlgBase::PrefDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer166;
	bSizer166 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer167;
	bSizer167 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer168;
	bSizer168 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer36;
	sbSizer36 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("When importing data that lacks a CRS:") ), wxVERTICAL );
	
	m_radio1 = new wxRadioButton( sbSizer36->GetStaticBox(), ID_RADIO1, _("Use the current CRS"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radio1->SetValue( true ); 
	sbSizer36->Add( m_radio1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_radio2 = new wxRadioButton( sbSizer36->GetStaticBox(), ID_RADIO2, _("Ask"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio2->SetValue( true ); 
	sbSizer36->Add( m_radio2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer168->Add( sbSizer36, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer37;
	sbSizer37 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("When loading an image which is very large:") ), wxVERTICAL );
	
	m_radio3 = new wxRadioButton( sbSizer37->GetStaticBox(), ID_RADIO3, _("Load it into memory"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radio3->SetValue( true ); 
	sbSizer37->Add( m_radio3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_radio4 = new wxRadioButton( sbSizer37->GetStaticBox(), ID_RADIO4, _("Use out-of-core access"), wxDefaultPosition, wxSize( 220,-1 ), 0 );
	m_radio4->SetValue( true ); 
	sbSizer37->Add( m_radio4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_radio5 = new wxRadioButton( sbSizer37->GetStaticBox(), ID_RADIO5, _("Ask"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio5->SetValue( true ); 
	sbSizer37->Add( m_radio5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer169;
	bSizer169 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text59 = new wxStaticText( sbSizer37->GetStaticBox(), ID_TEXT, _("Large image is more than:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text59->Wrap( -1 );
	bSizer169->Add( m_text59, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_max_megapixels = new wxTextCtrl( sbSizer37->GetStaticBox(), ID_MAX_MEGAPIXELS, wxEmptyString, wxDefaultPosition, wxSize( 32,-1 ), 0 );
	m_max_megapixels->SetMaxLength( 0 ); 
	bSizer169->Add( m_max_megapixels, 0, wxALIGN_CENTER|wxTOP|wxBOTTOM, 5 );
	
	m_text60 = new wxStaticText( sbSizer37->GetStaticBox(), ID_TEXT, _("megapixels"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text60->Wrap( 0 );
	bSizer169->Add( m_text60, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer37->Add( bSizer169, 0, wxALIGN_CENTER|wxALL, 0 );
	
	
	bSizer168->Add( sbSizer37, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer170;
	bSizer170 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text61 = new wxStaticText( this, ID_TEXT, _("Imagery: N > 1 for NxN multisampling"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text61->Wrap( -1 );
	bSizer170->Add( m_text61, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_sampling_n = new wxTextCtrl( this, ID_SAMPLING_N, wxEmptyString, wxDefaultPosition, wxSize( 40,-1 ), 0 );
	m_sampling_n->SetMaxLength( 0 ); 
	bSizer170->Add( m_sampling_n, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer168->Add( bSizer170, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	wxBoxSizer* bSizer171;
	bSizer171 = new wxBoxSizer( wxVERTICAL );
	
	m_black_transp = new wxCheckBox( this, ID_BLACK_TRANSP, _("Treat black as transparent when sampling images"), wxDefaultPosition, wxDefaultSize, 0 );
	m_black_transp->SetValue(true); 
	bSizer171->Add( m_black_transp, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_deflate_tiff = new wxCheckBox( this, ID_DEFLATE_TIFF, _("Deflate TIFF images when writing"), wxDefaultPosition, wxDefaultSize, 0 );
	m_deflate_tiff->SetValue(true); 
	bSizer171->Add( m_deflate_tiff, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer168->Add( bSizer171, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer167->Add( bSizer168, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer172;
	bSizer172 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer38;
	sbSizer38 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("When reprojecting an integer elevation grid:") ), wxVERTICAL );
	
	m_radio6 = new wxRadioButton( sbSizer38->GetStaticBox(), ID_RADIO6, _("Leave it as integer"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radio6->SetValue( true ); 
	sbSizer38->Add( m_radio6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_radio7 = new wxRadioButton( sbSizer38->GetStaticBox(), ID_RADIO7, _("Upgrade it to floating-point values"), wxDefaultPosition, wxSize( 220,-1 ), 0 );
	m_radio7->SetValue( true ); 
	sbSizer38->Add( m_radio7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_radio8 = new wxRadioButton( sbSizer38->GetStaticBox(), ID_RADIO8, _("Ask"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio8->SetValue( true ); 
	sbSizer38->Add( m_radio8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer172->Add( sbSizer38, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer39;
	sbSizer39 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Filling unknown areas of elevation grids:") ), wxVERTICAL );
	
	m_radio9 = new wxRadioButton( sbSizer39->GetStaticBox(), ID_RADIO9, _("Fast"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radio9->SetValue( true ); 
	sbSizer39->Add( m_radio9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_radio10 = new wxRadioButton( sbSizer39->GetStaticBox(), ID_RADIO10, _("Slow and smooth"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio10->SetValue( true ); 
	sbSizer39->Add( m_radio10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_radio11 = new wxRadioButton( sbSizer39->GetStaticBox(), ID_RADIO11, _("Extrapolation via partial derivatives"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio11->SetValue( true ); 
	sbSizer39->Add( m_radio11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer172->Add( sbSizer39, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer173;
	bSizer173 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text62 = new wxStaticText( this, ID_TEXT, _("Maximum size for rendered elevation layers:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text62->Wrap( -1 );
	bSizer173->Add( m_text62, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_elev_max_size = new wxTextCtrl( this, ID_ELEV_MAX_SIZE, wxEmptyString, wxDefaultPosition, wxSize( 40,-1 ), 0 );
	m_elev_max_size->SetMaxLength( 0 ); 
	bSizer173->Add( m_elev_max_size, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer172->Add( bSizer173, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	m_bt_gzip = new wxCheckBox( this, ID_BT_GZIP, _("Default to gzip-compressed BT files"), wxDefaultPosition, wxDefaultSize, 0 );
	m_bt_gzip->SetValue(true); 
	bSizer172->Add( m_bt_gzip, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_delay_load = new wxCheckBox( this, ID_DELAY_LOAD, _("Delay loading elevation from BT and ITF until needed for sampling"), wxDefaultPosition, wxDefaultSize, 0 );
	m_delay_load->SetValue(true); 
	bSizer172->Add( m_delay_load, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxBoxSizer* bSizer174;
	bSizer174 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer174->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	m_text63 = new wxStaticText( this, ID_TEXT, _("Maximum memory used for elevation (MB):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text63->Wrap( -1 );
	bSizer174->Add( m_text63, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_max_mem_grid = new wxTextCtrl( this, ID_MAX_MEM_GRID, wxEmptyString, wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_max_mem_grid->SetMaxLength( 0 ); 
	bSizer174->Add( m_max_mem_grid, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer172->Add( bSizer174, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5 );
	
	
	bSizer167->Add( bSizer172, 0, wxALIGN_CENTER_HORIZONTAL, 5 );
	
	
	bSizer166->Add( bSizer167, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer175;
	bSizer175 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer175->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer175->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer166->Add( bSizer175, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer166 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

PrefDlgBase::~PrefDlgBase()
{
}

RawDlgBase::RawDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxVERTICAL );
	
	wxGridSizer* gSizer1;
	gSizer1 = new wxGridSizer( 0, 2, 0, 0 );
	
	m_text6 = new wxStaticText( this, ID_TEXT, _("Bytes per sample"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text6->Wrap( -1 );
	gSizer1->Add( m_text6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_bytes = new wxTextCtrl( this, ID_BYTES, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_bytes->SetMaxLength( 0 ); 
	gSizer1->Add( m_bytes, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text7 = new wxStaticText( this, ID_TEXT, _("Width"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text7->Wrap( -1 );
	gSizer1->Add( m_text7, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_width = new wxTextCtrl( this, ID_WIDTH, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_width->SetMaxLength( 0 ); 
	gSizer1->Add( m_width, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text8 = new wxStaticText( this, ID_TEXT, _("Height"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text8->Wrap( -1 );
	gSizer1->Add( m_text8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_height = new wxTextCtrl( this, ID_HEIGHT, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_height->SetMaxLength( 0 ); 
	gSizer1->Add( m_height, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_little_endian = new wxRadioButton( this, ID_LITTLE_ENDIAN, _("Little Endian"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_little_endian->SetValue( true ); 
	gSizer1->Add( m_little_endian, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_big_endian = new wxRadioButton( this, ID_BIG_ENDIAN, _("Big Endian  "), wxDefaultPosition, wxDefaultSize, 0 );
	m_big_endian->SetValue( true ); 
	gSizer1->Add( m_big_endian, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text9 = new wxStaticText( this, ID_TEXT, _("Vertical units in meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text9->Wrap( -1 );
	gSizer1->Add( m_text9, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_vunits = new wxTextCtrl( this, ID_VUNITS, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_vunits->SetMaxLength( 0 ); 
	gSizer1->Add( m_vunits, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer10->Add( gSizer1, 0, wxALIGN_CENTER, 5 );
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Coordinate Reference System (CRS)") ), wxVERTICAL );
	
	m_crs_simple = new wxRadioButton( sbSizer2->GetStaticBox(), ID_CRS_SIMPLE, _("Simple meters (UTM zone 1)"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_crs_simple->SetValue( true ); 
	sbSizer2->Add( m_crs_simple, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_crs_current = new wxRadioButton( sbSizer2->GetStaticBox(), ID_CRS_CURRENT, _("Use current"), wxDefaultPosition, wxDefaultSize, 0 );
	m_crs_current->SetValue( true ); 
	sbSizer2->Add( m_crs_current, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxHORIZONTAL );
	
	m_crs_exact = new wxRadioButton( sbSizer2->GetStaticBox(), ID_CRS_EXACT, _("Enter exact CRS:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_crs_exact->SetValue( true ); 
	bSizer11->Add( m_crs_exact, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_crs = new wxButton( sbSizer2->GetStaticBox(), ID_CRS, _("CRS"), wxDefaultPosition, wxDefaultSize, 0 );
	m_crs->SetDefault(); 
	bSizer11->Add( m_crs, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	sbSizer2->Add( bSizer11, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer10->Add( sbSizer2, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Extents") ), wxVERTICAL );
	
	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ext_spacing = new wxRadioButton( sbSizer3->GetStaticBox(), ID_EXT_SPACING, _("Based on grid spacing:"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_ext_spacing->SetValue( true ); 
	bSizer12->Add( m_ext_spacing, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_spacing = new wxTextCtrl( sbSizer3->GetStaticBox(), ID_SPACING, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_spacing->SetMaxLength( 0 ); 
	bSizer12->Add( m_spacing, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer3->Add( bSizer12, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ext_exact = new wxRadioButton( sbSizer3->GetStaticBox(), ID_EXT_EXACT, _("Enter exact extents: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_ext_exact->SetValue( true ); 
	bSizer13->Add( m_ext_exact, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_extents = new wxButton( sbSizer3->GetStaticBox(), ID_EXTENTS, _("Extents"), wxDefaultPosition, wxDefaultSize, 0 );
	m_extents->SetDefault(); 
	bSizer13->Add( m_extents, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	sbSizer3->Add( bSizer13, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer10->Add( sbSizer3, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer14->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer14->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer10->Add( bSizer14, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer10 );
	this->Layout();
	bSizer10->Fit( this );
	
	this->Centre( wxBOTH );
}

RawDlgBase::~RawDlgBase()
{
}

RenderDlgBase::RenderDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer116;
	bSizer116 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer25;
	sbSizer25 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Output") ), wxVERTICAL );
	
	m_radio_create_new = new wxRadioButton( sbSizer25->GetStaticBox(), ID_RADIO_CREATE_NEW, _("Create new image layer"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radio_create_new->SetValue( true ); 
	sbSizer25->Add( m_radio_create_new, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxBoxSizer* bSizer117;
	bSizer117 = new wxBoxSizer( wxHORIZONTAL );
	
	m_radio_to_file = new wxRadioButton( sbSizer25->GetStaticBox(), ID_RADIO_TO_FILE, _("Write to file:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio_to_file->SetValue( true ); 
	bSizer117->Add( m_radio_to_file, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	sbSizer25->Add( bSizer117, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer118;
	bSizer118 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer118->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text_to_file = new wxTextCtrl( sbSizer25->GetStaticBox(), ID_TEXT_TO_FILE, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_text_to_file->SetMaxLength( 0 ); 
	bSizer118->Add( m_text_to_file, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_dotdotdot = new wxButton( sbSizer25->GetStaticBox(), ID_DOTDOTDOT, _("..."), wxDefaultPosition, wxSize( 24,-1 ), 0 );
	m_dotdotdot->SetDefault(); 
	bSizer118->Add( m_dotdotdot, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer25->Add( bSizer118, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer119;
	bSizer119 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text25 = new wxStaticText( sbSizer25->GetStaticBox(), ID_TEXT, _("Format:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text25->Wrap( 0 );
	bSizer119->Add( m_text25, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_geotiff = new wxRadioButton( sbSizer25->GetStaticBox(), ID_GEOTIFF, _("GeoTIFF"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_geotiff->SetValue( true ); 
	bSizer119->Add( m_geotiff, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_jpeg = new wxRadioButton( sbSizer25->GetStaticBox(), ID_JPEG, _("JPEG"), wxDefaultPosition, wxDefaultSize, 0 );
	m_jpeg->SetValue( true ); 
	bSizer119->Add( m_jpeg, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer25->Add( bSizer119, 0, wxALIGN_CENTER, 5 );
	
	
	bSizer116->Add( sbSizer25, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer26;
	sbSizer26 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Appearance") ), wxVERTICAL );
	
	wxBoxSizer* bSizer120;
	bSizer120 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text26 = new wxStaticText( sbSizer26->GetStaticBox(), ID_TEXT, _("Colors:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text26->Wrap( 0 );
	bSizer120->Add( m_text26, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxString m_choice_colorsChoices[] = { _("Item") };
	int m_choice_colorsNChoices = sizeof( m_choice_colorsChoices ) / sizeof( wxString );
	m_choice_colors = new wxChoice( sbSizer26->GetStaticBox(), ID_CHOICE_COLORS, wxDefaultPosition, wxSize( 180,-1 ), m_choice_colorsNChoices, m_choice_colorsChoices, 0 );
	m_choice_colors->SetSelection( 0 );
	bSizer120->Add( m_choice_colors, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_edit_colors = new wxButton( sbSizer26->GetStaticBox(), ID_EDIT_COLORS, _("Edit Colors..."), wxDefaultPosition, wxDefaultSize, 0 );
	m_edit_colors->SetDefault(); 
	bSizer120->Add( m_edit_colors, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer26->Add( bSizer120, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer121;
	bSizer121 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer121->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	m_shading = new wxCheckBox( sbSizer26->GetStaticBox(), ID_SHADING, _("Use view options for shading"), wxDefaultPosition, wxDefaultSize, 0 );
	m_shading->SetValue(true); 
	bSizer121->Add( m_shading, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer26->Add( bSizer121, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer122;
	bSizer122 = new wxBoxSizer( wxHORIZONTAL );
	
	
	bSizer122->Add( 20, 20, 0, wxALIGN_CENTER, 5 );
	
	m_text27 = new wxStaticText( sbSizer26->GetStaticBox(), ID_TEXT, _("Color to use for NODATA heixels:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text27->Wrap( -1 );
	bSizer122->Add( m_text27, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_color_nodata = new wxBitmapButton( sbSizer26->GetStaticBox(), ID_COLOR_NODATA, wxBitmap( dummy_32x18_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_color_nodata->SetDefault(); 
	bSizer122->Add( m_color_nodata, 0, wxALIGN_CENTER, 5 );
	
	
	sbSizer26->Add( bSizer122, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM, 5 );
	
	
	bSizer116->Add( sbSizer26, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	wxStaticBoxSizer* sbSizer27;
	sbSizer27 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Bitmap Size") ), wxVERTICAL );
	
	wxBoxSizer* bSizer123;
	bSizer123 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text28 = new wxStaticText( sbSizer27->GetStaticBox(), ID_TEXT, _("Image size:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text28->Wrap( -1 );
	bSizer123->Add( m_text28, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_sizex = new wxTextCtrl( sbSizer27->GetStaticBox(), ID_SIZEX, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_sizex->SetMaxLength( 0 ); 
	bSizer123->Add( m_sizex, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_sizey = new wxTextCtrl( sbSizer27->GetStaticBox(), ID_SIZEY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_sizey->SetMaxLength( 0 ); 
	bSizer123->Add( m_sizey, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer27->Add( bSizer123, 0, wxALIGN_CENTER, 5 );
	
	m_constrain = new wxCheckBox( sbSizer27->GetStaticBox(), ID_CONSTRAIN, _("Size constraint: power of 2 for texture map"), wxDefaultPosition, wxDefaultSize, 0 );
	m_constrain->SetValue(true); 
	sbSizer27->Add( m_constrain, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer125;
	bSizer125 = new wxBoxSizer( wxHORIZONTAL );
	
	m_smaller = new wxButton( sbSizer27->GetStaticBox(), ID_SMALLER, _("<< Smaller"), wxDefaultPosition, wxDefaultSize, 0 );
	m_smaller->SetDefault(); 
	bSizer125->Add( m_smaller, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_bigger = new wxButton( sbSizer27->GetStaticBox(), ID_BIGGER, _(">> Bigger"), wxDefaultPosition, wxDefaultSize, 0 );
	m_bigger->SetDefault(); 
	bSizer125->Add( m_bigger, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer27->Add( bSizer125, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer116->Add( sbSizer27, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	wxBoxSizer* bSizer126;
	bSizer126 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer126->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer126->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer116->Add( bSizer126, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer116 );
	this->Layout();
	bSizer116->Fit( this );
	
	this->Centre( wxBOTH );
}

RenderDlgBase::~RenderDlgBase()
{
}

RenderOptionsDlgBase::RenderOptionsDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer155;
	bSizer155 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer33;
	sbSizer33 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Colors") ), wxHORIZONTAL );
	
	m_text47 = new wxStaticText( sbSizer33->GetStaticBox(), ID_TEXT, _("Color map file:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text47->Wrap( -1 );
	sbSizer33->Add( m_text47, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer156;
	bSizer156 = new wxBoxSizer( wxVERTICAL );
	
	wxArrayString m_choice_colorsChoices;
	m_choice_colors = new wxChoice( sbSizer33->GetStaticBox(), ID_CHOICE_COLORS, wxDefaultPosition, wxSize( 180,-1 ), m_choice_colorsChoices, 0 );
	m_choice_colors->SetSelection( 0 );
	bSizer156->Add( m_choice_colors, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_edit_colors = new wxButton( sbSizer33->GetStaticBox(), ID_EDIT_COLORS, _("Edit Colors..."), wxDefaultPosition, wxDefaultSize, 0 );
	m_edit_colors->SetDefault(); 
	bSizer156->Add( m_edit_colors, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	sbSizer33->Add( bSizer156, 1, wxALIGN_CENTER, 5 );
	
	
	bSizer155->Add( sbSizer33, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer34;
	sbSizer34 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Shading") ), wxHORIZONTAL );
	
	wxBoxSizer* bSizer157;
	bSizer157 = new wxBoxSizer( wxVERTICAL );
	
	m_radio_shading_none = new wxRadioButton( sbSizer34->GetStaticBox(), ID_RADIO_SHADING_NONE, _("None"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radio_shading_none->SetValue( true ); 
	bSizer157->Add( m_radio_shading_none, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_radio_shading_quick = new wxRadioButton( sbSizer34->GetStaticBox(), ID_RADIO_SHADING_QUICK, _("Quick (East-West)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio_shading_quick->SetValue( true ); 
	bSizer157->Add( m_radio_shading_quick, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_radio_shading_dot = new wxRadioButton( sbSizer34->GetStaticBox(), ID_RADIO_SHADING_DOT, _("Dot-product with surface"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio_shading_dot->SetValue( true ); 
	bSizer157->Add( m_radio_shading_dot, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxFlexGridSizer* fgSizer10;
	fgSizer10 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer10->SetFlexibleDirection( wxBOTH );
	fgSizer10->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	fgSizer10->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text48 = new wxStaticText( sbSizer34->GetStaticBox(), ID_TEXT, _("Angle (0-90):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text48->Wrap( -1 );
	fgSizer10->Add( m_text48, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	m_spin_cast_angle = new wxSpinCtrl( sbSizer34->GetStaticBox(), ID_SPIN_CAST_ANGLE, wxT("45"), wxDefaultPosition, wxSize( 70,-1 ), wxSP_ARROW_KEYS, 0, 90, 45 );
	fgSizer10->Add( m_spin_cast_angle, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	fgSizer10->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text49 = new wxStaticText( sbSizer34->GetStaticBox(), ID_TEXT, _("Direction (0-360):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text49->Wrap( -1 );
	fgSizer10->Add( m_text49, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	m_spin_cast_direction = new wxSpinCtrl( sbSizer34->GetStaticBox(), ID_SPIN_CAST_DIRECTION, wxT("45"), wxDefaultPosition, wxSize( 70,-1 ), wxSP_ARROW_KEYS, 0, 360, 45 );
	fgSizer10->Add( m_spin_cast_direction, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	fgSizer10->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text50 = new wxStaticText( sbSizer34->GetStaticBox(), ID_TEXT, _("Ambient light (0-1):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text50->Wrap( -1 );
	fgSizer10->Add( m_text50, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	m_ambient = new wxTextCtrl( sbSizer34->GetStaticBox(), ID_AMBIENT, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_ambient->SetMaxLength( 0 ); 
	fgSizer10->Add( m_ambient, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	fgSizer10->Add( 20, 20, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text51 = new wxStaticText( sbSizer34->GetStaticBox(), ID_TEXT, _("Gamma (0-1):"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text51->Wrap( -1 );
	fgSizer10->Add( m_text51, 0, wxALIGN_RIGHT|wxALL, 5 );
	
	m_gamma = new wxTextCtrl( sbSizer34->GetStaticBox(), ID_GAMMA, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_gamma->SetMaxLength( 0 ); 
	fgSizer10->Add( m_gamma, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer157->Add( fgSizer10, 0, wxALIGN_CENTER, 5 );
	
	m_check_shadows = new wxCheckBox( sbSizer34->GetStaticBox(), ID_CHECK_SHADOWS, _("Cast Shadows"), wxDefaultPosition, wxDefaultSize, 0 );
	m_check_shadows->SetValue(true); 
	bSizer157->Add( m_check_shadows, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	sbSizer34->Add( bSizer157, 0, wxALIGN_CENTER, 5 );
	
	
	bSizer155->Add( sbSizer34, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer158;
	bSizer158 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer158->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer158->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer155->Add( bSizer158, 0, wxALIGN_CENTER, 5 );
	
	
	this->SetSizer( bSizer155 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

RenderOptionsDlgBase::~RenderOptionsDlgBase()
{
}

RoadDlgBase::RoadDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer36;
	bSizer36 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer37;
	bSizer37 = new wxBoxSizer( wxHORIZONTAL );
	
	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer3->SetFlexibleDirection( wxBOTH );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_text25 = new wxStaticText( this, ID_TEXT, _("Number of Lanes"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text25->Wrap( -1 );
	fgSizer3->Add( m_text25, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_numlanes = new wxTextCtrl( this, ID_NUMLANES, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_numlanes->SetMaxLength( 0 ); 
	fgSizer3->Add( m_numlanes, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_text26 = new wxStaticText( this, ID_TEXT, _("Highway Name"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text26->Wrap( -1 );
	fgSizer3->Add( m_text26, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_hwyname = new wxTextCtrl( this, ID_HWYNAME, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_hwyname->SetMaxLength( 0 ); 
	fgSizer3->Add( m_hwyname, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_text27 = new wxStaticText( this, ID_TEXT, _("Sidewalk"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text27->Wrap( -1 );
	fgSizer3->Add( m_text27, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_sidewalkChoices;
	m_sidewalk = new wxChoice( this, ID_SIDEWALK, wxDefaultPosition, wxSize( 100,-1 ), m_sidewalkChoices, 0 );
	m_sidewalk->SetSelection( 0 );
	fgSizer3->Add( m_sidewalk, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_text28 = new wxStaticText( this, ID_TEXT, _("Parking"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text28->Wrap( 0 );
	fgSizer3->Add( m_text28, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_parkingChoices;
	m_parking = new wxChoice( this, ID_PARKING, wxDefaultPosition, wxSize( 100,-1 ), m_parkingChoices, 0 );
	m_parking->SetSelection( 0 );
	fgSizer3->Add( m_parking, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_text29 = new wxStaticText( this, ID_TEXT, _("Margin"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text29->Wrap( 0 );
	fgSizer3->Add( m_text29, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_marginChoices;
	m_margin = new wxChoice( this, ID_MARGIN, wxDefaultPosition, wxSize( 100,-1 ), m_marginChoices, 0 );
	m_margin->SetSelection( 0 );
	fgSizer3->Add( m_margin, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_text30 = new wxStaticText( this, ID_TEXT, _("Lane Width"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text30->Wrap( -1 );
	fgSizer3->Add( m_text30, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_lane_width = new wxTextCtrl( this, ID_LANE_WIDTH, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_lane_width->SetMaxLength( 0 ); 
	fgSizer3->Add( m_lane_width, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_text31 = new wxStaticText( this, ID_TEXT, _("Pavement Width"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text31->Wrap( -1 );
	fgSizer3->Add( m_text31, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_sidewalk_width = new wxTextCtrl( this, ID_SIDEWALK_WIDTH, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_sidewalk_width->SetMaxLength( 0 ); 
	fgSizer3->Add( m_sidewalk_width, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_text32 = new wxStaticText( this, ID_TEXT, _("Curb Height"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text32->Wrap( -1 );
	fgSizer3->Add( m_text32, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_curb_height = new wxTextCtrl( this, ID_CURB_HEIGHT, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_curb_height->SetMaxLength( 0 ); 
	fgSizer3->Add( m_curb_height, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_text33 = new wxStaticText( this, ID_TEXT, _("Parking Width"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text33->Wrap( -1 );
	fgSizer3->Add( m_text33, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_parking_width = new wxTextCtrl( this, ID_PARKING_WIDTH, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_parking_width->SetMaxLength( 0 ); 
	fgSizer3->Add( m_parking_width, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	m_text34 = new wxStaticText( this, ID_TEXT, _("Margin Width"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text34->Wrap( -1 );
	fgSizer3->Add( m_text34, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_margin_width = new wxTextCtrl( this, ID_MARGIN_WIDTH, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_margin_width->SetMaxLength( 0 ); 
	fgSizer3->Add( m_margin_width, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	
	bSizer37->Add( fgSizer3, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer38;
	bSizer38 = new wxBoxSizer( wxVERTICAL );
	
	m_text35 = new wxStaticText( this, ID_TEXT, _("Surface Type:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text35->Wrap( -1 );
	bSizer38->Add( m_text35, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_surftype = new wxListBox( this, ID_SURFTYPE, wxDefaultPosition, wxSize( 100,100 ), 0, NULL, wxLB_SINGLE ); 
	bSizer38->Add( m_surftype, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer37->Add( bSizer38, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	
	bSizer36->Add( bSizer37, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxBoxSizer* bSizer39;
	bSizer39 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer39->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer39->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer36->Add( bSizer39, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer36 );
	this->Layout();
	bSizer36->Fit( this );
	
	this->Centre( wxBOTH );
}

RoadDlgBase::~RoadDlgBase()
{
}

SelectDlgBase::SelectDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer58;
	bSizer58 = new wxBoxSizer( wxVERTICAL );
	
	m_text45 = new wxStaticText( this, ID_TEXT, _("Select all features which satisfy the condition:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text45->Wrap( -1 );
	bSizer58->Add( m_text45, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer59;
	bSizer59 = new wxBoxSizer( wxHORIZONTAL );
	
	m_field = new wxListBox( this, ID_FIELD, wxDefaultPosition, wxSize( 100,110 ), 0, NULL, wxLB_SINGLE ); 
	bSizer59->Add( m_field, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
	
	wxArrayString m_conditionChoices;
	m_condition = new wxChoice( this, ID_CONDITION, wxDefaultPosition, wxSize( 50,-1 ), m_conditionChoices, 0 );
	m_condition->SetSelection( 0 );
	bSizer59->Add( m_condition, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_combo_value = new wxComboBox( this, ID_COMBO_VALUE, wxEmptyString, wxDefaultPosition, wxSize( 180,-1 ), 0, NULL, wxCB_DROPDOWN ); 
	bSizer59->Add( m_combo_value, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer58->Add( bSizer59, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5 );
	
	wxBoxSizer* bSizer60;
	bSizer60 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer60->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer60->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer58->Add( bSizer60, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer58 );
	this->Layout();
	bSizer58->Fit( this );
	
	this->Centre( wxBOTH );
}

SelectDlgBase::~SelectDlgBase()
{
}

SampleElevationDlgBase::SampleElevationDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer43;
	bSizer43 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer9;
	sbSizer9 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Output") ), wxVERTICAL );
	
	m_radio_create_new = new wxRadioButton( sbSizer9->GetStaticBox(), ID_RADIO_CREATE_NEW, _("Create new layer"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radio_create_new->SetValue( true ); 
	sbSizer9->Add( m_radio_create_new, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer44;
	bSizer44 = new wxBoxSizer( wxHORIZONTAL );
	
	m_radio_to_file = new wxRadioButton( sbSizer9->GetStaticBox(), ID_RADIO_TO_FILE, _("To file"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio_to_file->SetValue( true ); 
	bSizer44->Add( m_radio_to_file, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text_to_file = new wxTextCtrl( sbSizer9->GetStaticBox(), ID_TEXT_TO_FILE, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_text_to_file->SetMaxLength( 0 ); 
	bSizer44->Add( m_text_to_file, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_dotdotdot = new wxButton( sbSizer9->GetStaticBox(), ID_DOTDOTDOT, _("..."), wxDefaultPosition, wxSize( 24,-1 ), 0 );
	m_dotdotdot->SetDefault(); 
	m_dotdotdot->SetFont( wxFont( 11, 72, 90, 92, false, wxEmptyString ) );
	
	bSizer44->Add( m_dotdotdot, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer9->Add( bSizer44, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer43->Add( sbSizer9, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer10;
	sbSizer10 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Sampling") ), wxVERTICAL );
	
	wxBoxSizer* bSizer48;
	bSizer48 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text39 = new wxStaticText( sbSizer10->GetStaticBox(), ID_TEXT, _("Grid spacing:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text39->Wrap( -1 );
	bSizer48->Add( m_text39, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_spacingx = new wxTextCtrl( sbSizer10->GetStaticBox(), ID_SPACINGX, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_spacingx->SetMaxLength( 0 ); 
	bSizer48->Add( m_spacingx, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_spacingy = new wxTextCtrl( sbSizer10->GetStaticBox(), ID_SPACINGY, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_spacingy->SetMaxLength( 0 ); 
	bSizer48->Add( m_spacingy, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer10->Add( bSizer48, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer49;
	bSizer49 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text40 = new wxStaticText( sbSizer10->GetStaticBox(), ID_TEXT, _("Grid size:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text40->Wrap( -1 );
	bSizer49->Add( m_text40, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_sizex = new wxTextCtrl( sbSizer10->GetStaticBox(), ID_SIZEX, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_sizex->SetMaxLength( 0 ); 
	bSizer49->Add( m_sizex, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_sizey = new wxTextCtrl( sbSizer10->GetStaticBox(), ID_SIZEY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_sizey->SetMaxLength( 0 ); 
	bSizer49->Add( m_sizey, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer10->Add( bSizer49, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer50;
	bSizer50 = new wxBoxSizer( wxHORIZONTAL );
	
	m_constrain = new wxCheckBox( sbSizer10->GetStaticBox(), ID_CONSTRAIN, _("Size constraint: power of 2 plus 1 for terrain LOD"), wxDefaultPosition, wxDefaultSize, 0 );
	m_constrain->SetValue(true); 
	bSizer50->Add( m_constrain, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer10->Add( bSizer50, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer51;
	bSizer51 = new wxBoxSizer( wxHORIZONTAL );
	
	m_smaller = new wxButton( sbSizer10->GetStaticBox(), ID_SMALLER, _("<< Smaller"), wxDefaultPosition, wxSize( 73,-1 ), 0 );
	m_smaller->SetDefault(); 
	bSizer51->Add( m_smaller, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_bigger = new wxButton( sbSizer10->GetStaticBox(), ID_BIGGER, _(">> Bigger"), wxDefaultPosition, wxSize( 70,-1 ), 0 );
	m_bigger->SetDefault(); 
	bSizer51->Add( m_bigger, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer10->Add( bSizer51, 0, wxALIGN_CENTER, 5 );
	
	
	bSizer43->Add( sbSizer10, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer52;
	bSizer52 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer53;
	bSizer53 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer54;
	bSizer54 = new wxBoxSizer( wxVERTICAL );
	
	m_floats = new wxRadioButton( this, ID_FLOATS, _("Floating point"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_floats->SetValue( true ); 
	bSizer54->Add( m_floats, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_shorts = new wxRadioButton( this, ID_SHORTS, _("Short integer"), wxDefaultPosition, wxDefaultSize, 0 );
	m_shorts->SetValue( true ); 
	bSizer54->Add( m_shorts, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer53->Add( bSizer54, 0, wxALIGN_CENTER, 5 );
	
	m_text41 = new wxStaticText( this, ID_TEXT, _("Vertical units:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text41->Wrap( -1 );
	bSizer53->Add( m_text41, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_vunits = new wxTextCtrl( this, ID_VUNITS, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), 0 );
	m_vunits->SetMaxLength( 0 ); 
	bSizer53->Add( m_vunits, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text42 = new wxStaticText( this, ID_TEXT, _("meters"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text42->Wrap( 0 );
	bSizer53->Add( m_text42, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer52->Add( bSizer53, 0, wxALIGN_CENTER, 5 );
	
	m_fill_gaps = new wxCheckBox( this, ID_FILL_GAPS, _("Fill gaps (NODATA) in the sampled grid with interpolated values"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer52->Add( m_fill_gaps, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer43->Add( bSizer52, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer11;
	sbSizer11 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Information") ), wxVERTICAL );
	
	wxBoxSizer* bSizer55;
	bSizer55 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text43 = new wxStaticText( sbSizer11->GetStaticBox(), ID_TEXT, _("Size of sample area:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text43->Wrap( -1 );
	bSizer55->Add( m_text43, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_areax = new wxTextCtrl( sbSizer11->GetStaticBox(), ID_AREAX, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), wxTE_READONLY );
	m_areax->SetMaxLength( 0 ); 
	bSizer55->Add( m_areax, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_areay = new wxTextCtrl( sbSizer11->GetStaticBox(), ID_AREAY, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), wxTE_READONLY );
	m_areay->SetMaxLength( 0 ); 
	bSizer55->Add( m_areay, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer11->Add( bSizer55, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer56;
	bSizer56 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text44 = new wxStaticText( sbSizer11->GetStaticBox(), ID_TEXT, _("Estimated spacing of existing data:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text44->Wrap( -1 );
	bSizer56->Add( m_text44, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_estx = new wxTextCtrl( sbSizer11->GetStaticBox(), ID_ESTX, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxTE_READONLY );
	m_estx->SetMaxLength( 0 ); 
	bSizer56->Add( m_estx, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_esty = new wxTextCtrl( sbSizer11->GetStaticBox(), ID_ESTY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxTE_READONLY );
	m_esty->SetMaxLength( 0 ); 
	bSizer56->Add( m_esty, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer11->Add( bSizer56, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer43->Add( sbSizer11, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer57;
	bSizer57 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer57->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer57->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer43->Add( bSizer57, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer43 );
	this->Layout();
	bSizer43->Fit( this );
	
	this->Centre( wxBOTH );
}

SampleElevationDlgBase::~SampleElevationDlgBase()
{
}

SampleImageDlgBase::SampleImageDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer93;
	bSizer93 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer20;
	sbSizer20 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Output") ), wxVERTICAL );
	
	m_radio_create_new = new wxRadioButton( sbSizer20->GetStaticBox(), ID_RADIO_CREATE_NEW, _("Create new layer"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_radio_create_new->SetValue( true ); 
	sbSizer20->Add( m_radio_create_new, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer94;
	bSizer94 = new wxBoxSizer( wxHORIZONTAL );
	
	m_radio_to_file = new wxRadioButton( sbSizer20->GetStaticBox(), ID_RADIO_TO_FILE, _("To file"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radio_to_file->SetValue( true ); 
	bSizer94->Add( m_radio_to_file, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text_to_file = new wxTextCtrl( sbSizer20->GetStaticBox(), ID_TEXT_TO_FILE, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), 0 );
	m_text_to_file->SetMaxLength( 0 ); 
	bSizer94->Add( m_text_to_file, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_dotdotdot = new wxButton( sbSizer20->GetStaticBox(), ID_DOTDOTDOT, _("..."), wxDefaultPosition, wxSize( 24,-1 ), 0 );
	m_dotdotdot->SetDefault(); 
	m_dotdotdot->SetFont( wxFont( 11, 72, 90, 92, false, wxEmptyString ) );
	
	bSizer94->Add( m_dotdotdot, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer20->Add( bSizer94, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer93->Add( sbSizer20, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer21;
	sbSizer21 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Sampling") ), wxVERTICAL );
	
	wxBoxSizer* bSizer96;
	bSizer96 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text9 = new wxStaticText( sbSizer21->GetStaticBox(), ID_TEXT, _("Image size:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text9->Wrap( -1 );
	bSizer96->Add( m_text9, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_sizex = new wxTextCtrl( sbSizer21->GetStaticBox(), ID_SIZEX, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_sizex->SetMaxLength( 0 ); 
	bSizer96->Add( m_sizex, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_sizey = new wxTextCtrl( sbSizer21->GetStaticBox(), ID_SIZEY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_sizey->SetMaxLength( 0 ); 
	bSizer96->Add( m_sizey, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer21->Add( bSizer96, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer97;
	bSizer97 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text10 = new wxStaticText( sbSizer21->GetStaticBox(), ID_TEXT, _("Pixel spacing:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text10->Wrap( -1 );
	bSizer97->Add( m_text10, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_spacingx = new wxTextCtrl( sbSizer21->GetStaticBox(), ID_SPACINGX, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), 0 );
	m_spacingx->SetMaxLength( 0 ); 
	bSizer97->Add( m_spacingx, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_spacingy = new wxTextCtrl( sbSizer21->GetStaticBox(), ID_SPACINGY, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), 0 );
	m_spacingy->SetMaxLength( 0 ); 
	bSizer97->Add( m_spacingy, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer21->Add( bSizer97, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer98;
	bSizer98 = new wxBoxSizer( wxVERTICAL );
	
	m_constrain = new wxCheckBox( sbSizer21->GetStaticBox(), ID_CONSTRAIN, _("Size constraint: power of 2 for texture map"), wxDefaultPosition, wxDefaultSize, 0 );
	m_constrain->SetValue(true); 
	bSizer98->Add( m_constrain, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer21->Add( bSizer98, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT, 5 );
	
	wxBoxSizer* bSizer99;
	bSizer99 = new wxBoxSizer( wxHORIZONTAL );
	
	m_smaller = new wxButton( sbSizer21->GetStaticBox(), ID_SMALLER, _("<< Smaller"), wxDefaultPosition, wxDefaultSize, 0 );
	m_smaller->SetDefault(); 
	bSizer99->Add( m_smaller, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_bigger = new wxButton( sbSizer21->GetStaticBox(), ID_BIGGER, _(">> Bigger"), wxDefaultPosition, wxDefaultSize, 0 );
	m_bigger->SetDefault(); 
	bSizer99->Add( m_bigger, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer21->Add( bSizer99, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer93->Add( sbSizer21, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer22;
	sbSizer22 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Information") ), wxVERTICAL );
	
	wxBoxSizer* bSizer100;
	bSizer100 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text11 = new wxStaticText( sbSizer22->GetStaticBox(), ID_TEXT, _("Size of sample area:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text11->Wrap( -1 );
	bSizer100->Add( m_text11, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_areax = new wxTextCtrl( sbSizer22->GetStaticBox(), ID_AREAX, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), wxTE_READONLY );
	m_areax->SetMaxLength( 0 ); 
	bSizer100->Add( m_areax, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_areay = new wxTextCtrl( sbSizer22->GetStaticBox(), ID_AREAY, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), wxTE_READONLY );
	m_areay->SetMaxLength( 0 ); 
	bSizer100->Add( m_areay, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer22->Add( bSizer100, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	wxBoxSizer* bSizer101;
	bSizer101 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text12 = new wxStaticText( sbSizer22->GetStaticBox(), ID_TEXT, _("Estimated spacing of existing data:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text12->Wrap( -1 );
	bSizer101->Add( m_text12, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_estx = new wxTextCtrl( sbSizer22->GetStaticBox(), ID_ESTX, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxTE_READONLY );
	m_estx->SetMaxLength( 0 ); 
	bSizer101->Add( m_estx, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_esty = new wxTextCtrl( sbSizer22->GetStaticBox(), ID_ESTY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxTE_READONLY );
	m_esty->SetMaxLength( 0 ); 
	bSizer101->Add( m_esty, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer22->Add( bSizer101, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer93->Add( sbSizer22, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxBoxSizer* bSizer102;
	bSizer102 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer102->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer102->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer93->Add( bSizer102, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer93 );
	this->Layout();
	bSizer93->Fit( this );
	
	this->Centre( wxBOTH );
}

SampleImageDlgBase::~SampleImageDlgBase()
{
}

SpeciesListDlgBase::SpeciesListDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer107;
	bSizer107 = new wxBoxSizer( wxVERTICAL );
	
	m_splitter1 = new wxSplitterWindow( this, ID_SPLITTER1, wxDefaultPosition, wxSize( 850,400 ), wxSP_3D );
	m_splitter1->Connect( wxEVT_IDLE, wxIdleEventHandler( SpeciesListDlgBase::m_splitter1OnIdle ), NULL, this );
	
	m_panel1 = new wxPanel( m_splitter1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer109;
	bSizer109 = new wxBoxSizer( wxVERTICAL );
	
	m_text22 = new wxStaticText( m_panel1, ID_TEXT, _("Species"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text22->Wrap( 0 );
	bSizer109->Add( m_text22, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_listctrl_species = new wxListCtrl( m_panel1, ID_LISTCTRL_SPECIES, wxDefaultPosition, wxSize( 240,200 ), wxLC_REPORT|wxSUNKEN_BORDER );
	bSizer109->Add( m_listctrl_species, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	
	m_panel1->SetSizer( bSizer109 );
	m_panel1->Layout();
	bSizer109->Fit( m_panel1 );
	m_panel2 = new wxPanel( m_splitter1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer110;
	bSizer110 = new wxBoxSizer( wxVERTICAL );
	
	m_text1 = new wxStaticText( m_panel2, ID_TEXT, _("Appearances"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text1->Wrap( 0 );
	bSizer110->Add( m_text1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_listctrl_appearances = new wxListCtrl( m_panel2, ID_LISTCTRL_APPEARANCES, wxDefaultPosition, wxSize( 240,200 ), wxLC_REPORT|wxSUNKEN_BORDER );
	bSizer110->Add( m_listctrl_appearances, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	
	m_panel2->SetSizer( bSizer110 );
	m_panel2->Layout();
	bSizer110->Fit( m_panel2 );
	m_splitter1->SplitVertically( m_panel1, m_panel2, 0 );
	bSizer107->Add( m_splitter1, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer108;
	bSizer108 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer108->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer107->Add( bSizer108, 0, wxALIGN_CENTER, 5 );
	
	
	this->SetSizer( bSizer107 );
	this->Layout();
	bSizer107->Fit( this );
	
	this->Centre( wxBOTH );
}

SpeciesListDlgBase::~SpeciesListDlgBase()
{
}

TileDlgBase::TileDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer146;
	bSizer146 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer147;
	bSizer147 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text38 = new wxStaticText( this, ID_TEXT, _("Tile output file:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text38->Wrap( -1 );
	bSizer147->Add( m_text38, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_text_to_folder = new wxTextCtrl( this, ID_TEXT_TO_FOLDER, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_text_to_folder->SetMaxLength( 0 ); 
	bSizer147->Add( m_text_to_folder, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_dotdotdot = new wxButton( this, ID_DOTDOTDOT, _("..."), wxDefaultPosition, wxSize( 25,-1 ), 0 );
	m_dotdotdot->SetDefault(); 
	m_dotdotdot->SetFont( wxFont( 11, 72, 90, 92, false, wxEmptyString ) );
	
	bSizer147->Add( m_dotdotdot, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer146->Add( bSizer147, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer148;
	bSizer148 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text39 = new wxStaticText( this, ID_TEXT, _("Columns"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text39->Wrap( 0 );
	bSizer148->Add( m_text39, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_columns = new wxTextCtrl( this, ID_COLUMNS, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_columns->SetMaxLength( 0 ); 
	bSizer148->Add( m_columns, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text40 = new wxStaticText( this, ID_TEXT, _("Rows"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text40->Wrap( 0 );
	bSizer148->Add( m_text40, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_rows = new wxTextCtrl( this, ID_ROWS, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_rows->SetMaxLength( 0 ); 
	bSizer148->Add( m_rows, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer146->Add( bSizer148, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer149;
	bSizer149 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text41 = new wxStaticText( this, ID_TEXT, _("LOD 0 Size:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text41->Wrap( -1 );
	bSizer149->Add( m_text41, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxArrayString m_choice_lod0_sizeChoices;
	m_choice_lod0_size = new wxChoice( this, ID_CHOICE_LOD0_SIZE, wxDefaultPosition, wxSize( 100,-1 ), m_choice_lod0_sizeChoices, 0 );
	m_choice_lod0_size->SetSelection( 0 );
	bSizer149->Add( m_choice_lod0_size, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer146->Add( bSizer149, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer150;
	bSizer150 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text42 = new wxStaticText( this, ID_TEXT, _("Number of LODs:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text42->Wrap( -1 );
	bSizer150->Add( m_text42, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_spin_num_lods = new wxSpinCtrl( this, ID_SPIN_NUM_LODS, wxT("3"), wxDefaultPosition, wxSize( 60,-1 ), wxSP_ARROW_KEYS, 1, 8, 3 );
	bSizer150->Add( m_spin_num_lods, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer146->Add( bSizer150, 0, wxALIGN_CENTER, 5 );
	
	wxStaticBoxSizer* sbSizer31;
	sbSizer31 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Information") ), wxVERTICAL );
	
	wxBoxSizer* bSizer151;
	bSizer151 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text43 = new wxStaticText( sbSizer31->GetStaticBox(), ID_TEXT, _("Total size of all tiles:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text43->Wrap( -1 );
	bSizer151->Add( m_text43, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_totalx = new wxTextCtrl( sbSizer31->GetStaticBox(), ID_TOTALX, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxTE_READONLY );
	m_totalx->SetMaxLength( 0 ); 
	bSizer151->Add( m_totalx, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_totaly = new wxTextCtrl( sbSizer31->GetStaticBox(), ID_TOTALY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxTE_READONLY );
	m_totaly->SetMaxLength( 0 ); 
	bSizer151->Add( m_totaly, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer31->Add( bSizer151, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer152;
	bSizer152 = new wxBoxSizer( wxHORIZONTAL );
	
	m_text44 = new wxStaticText( sbSizer31->GetStaticBox(), ID_TEXT, _("Size of sample area:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text44->Wrap( -1 );
	bSizer152->Add( m_text44, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_areax = new wxTextCtrl( sbSizer31->GetStaticBox(), ID_AREAX, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), wxTE_READONLY );
	m_areax->SetMaxLength( 0 ); 
	bSizer152->Add( m_areax, 1, wxALIGN_CENTER|wxALL, 5 );
	
	m_areay = new wxTextCtrl( sbSizer31->GetStaticBox(), ID_AREAY, wxEmptyString, wxDefaultPosition, wxSize( 80,-1 ), wxTE_READONLY );
	m_areay->SetMaxLength( 0 ); 
	bSizer152->Add( m_areay, 1, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer31->Add( bSizer152, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxFlexGridSizer* fgSizer9;
	fgSizer9 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer9->SetFlexibleDirection( wxBOTH );
	fgSizer9->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_text45 = new wxStaticText( sbSizer31->GetStaticBox(), ID_TEXT, _("Spacing of existing data:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text45->Wrap( -1 );
	fgSizer9->Add( m_text45, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_estx = new wxTextCtrl( sbSizer31->GetStaticBox(), ID_ESTX, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_estx->SetMaxLength( 0 ); 
	fgSizer9->Add( m_estx, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_esty = new wxTextCtrl( sbSizer31->GetStaticBox(), ID_ESTY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_esty->SetMaxLength( 0 ); 
	fgSizer9->Add( m_esty, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_text46 = new wxStaticText( sbSizer31->GetStaticBox(), ID_TEXT, _("Spacing of LOD 0:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_text46->Wrap( -1 );
	fgSizer9->Add( m_text46, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_curx = new wxTextCtrl( sbSizer31->GetStaticBox(), ID_CURX, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxTE_READONLY );
	m_curx->SetMaxLength( 0 ); 
	fgSizer9->Add( m_curx, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_cury = new wxTextCtrl( sbSizer31->GetStaticBox(), ID_CURY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxTE_READONLY );
	m_cury->SetMaxLength( 0 ); 
	fgSizer9->Add( m_cury, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	sbSizer31->Add( fgSizer9, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer146->Add( sbSizer31, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	wxBoxSizer* bSizer153;
	bSizer153 = new wxBoxSizer( wxVERTICAL );
	
	m_omit_flat = new wxCheckBox( this, ID_OMIT_FLAT, _("Omit flat elevation tiles (sea level)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_omit_flat->SetValue(true); 
	bSizer153->Add( m_omit_flat, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_mask_unknown = new wxCheckBox( this, ID_MASK_UNKNOWN, _("Mask unknown elevation areas with texture alpha values"), wxDefaultPosition, wxDefaultSize, 0 );
	m_mask_unknown->SetValue(true); 
	bSizer153->Add( m_mask_unknown, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_texture_alpha = new wxCheckBox( this, ID_TEXTURE_ALPHA, _("Use texture alpha values"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer153->Add( m_texture_alpha, 0, wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer32;
	sbSizer32 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Texture Compression") ), wxVERTICAL );
	
	m_tc_none = new wxRadioButton( sbSizer32->GetStaticBox(), ID_TC_NONE, _("None"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_tc_none->SetValue( true ); 
	sbSizer32->Add( m_tc_none, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_tc_ogl = new wxRadioButton( sbSizer32->GetStaticBox(), ID_TC_OGL, _("OpenGL"), wxDefaultPosition, wxDefaultSize, 0 );
	m_tc_ogl->SetValue( true ); 
	sbSizer32->Add( m_tc_ogl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_tc_squish_fast = new wxRadioButton( sbSizer32->GetStaticBox(), ID_TC_SQUISH_FAST, _("Squish fast"), wxDefaultPosition, wxDefaultSize, 0 );
	m_tc_squish_fast->SetValue( true ); 
	sbSizer32->Add( m_tc_squish_fast, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_tc_squish_slow = new wxRadioButton( sbSizer32->GetStaticBox(), ID_TC_SQUISH_SLOW, _("Squish slow (6x slower, slightly fewer artifacts)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_tc_squish_slow->SetValue( true ); 
	sbSizer32->Add( m_tc_squish_slow, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_tc_jpeg = new wxRadioButton( sbSizer32->GetStaticBox(), ID_TC_JPEG, _("JPEG"), wxDefaultPosition, wxDefaultSize, 0 );
	m_tc_jpeg->SetValue( true ); 
	sbSizer32->Add( m_tc_jpeg, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer153->Add( sbSizer32, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer146->Add( bSizer153, 0, wxALIGN_CENTER, 5 );
	
	wxBoxSizer* bSizer154;
	bSizer154 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer154->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer154->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer146->Add( bSizer154, 0, wxALIGN_CENTER, 5 );
	
	
	this->SetSizer( bSizer146 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

TileDlgBase::~TileDlgBase()
{
}

VegFieldsDlgBase::VegFieldsDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer65;
	bSizer65 = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* sbSizer14;
	sbSizer14 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Species") ), wxVERTICAL );
	
	wxBoxSizer* bSizer66;
	bSizer66 = new wxBoxSizer( wxHORIZONTAL );
	
	m_use_species = new wxRadioButton( sbSizer14->GetStaticBox(), ID_USE_SPECIES, _("Use species:"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_use_species->SetValue( true ); 
	bSizer66->Add( m_use_species, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_species_choiceChoices;
	m_species_choice = new wxChoice( sbSizer14->GetStaticBox(), ID_SPECIES_CHOICE, wxDefaultPosition, wxSize( 140,-1 ), m_species_choiceChoices, 0 );
	m_species_choice->SetSelection( 0 );
	bSizer66->Add( m_species_choice, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	
	sbSizer14->Add( bSizer66, 0, wxALIGN_CENTER|wxTOP, 5 );
	
	wxBoxSizer* bSizer67;
	bSizer67 = new wxBoxSizer( wxHORIZONTAL );
	
	m_species_use_field = new wxRadioButton( sbSizer14->GetStaticBox(), ID_SPECIES_USE_FIELD, _("Use field:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_species_use_field->SetValue( true ); 
	bSizer67->Add( m_species_use_field, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_species_fieldChoices;
	m_species_field = new wxChoice( sbSizer14->GetStaticBox(), ID_SPECIES_FIELD, wxDefaultPosition, wxSize( 140,-1 ), m_species_fieldChoices, 0 );
	m_species_field->SetSelection( 0 );
	bSizer67->Add( m_species_field, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxTOP, 5 );
	
	
	sbSizer14->Add( bSizer67, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer68;
	bSizer68 = new wxBoxSizer( wxVERTICAL );
	
	m_species_id = new wxRadioButton( sbSizer14->GetStaticBox(), ID_SPECIES_ID, _("Species ID (integer)"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_species_id->SetValue( true ); 
	bSizer68->Add( m_species_id, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_species_name = new wxRadioButton( sbSizer14->GetStaticBox(), ID_SPECIES_NAME, _("Species Name (string)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_species_name->SetValue( true ); 
	bSizer68->Add( m_species_name, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	m_common_name = new wxRadioButton( sbSizer14->GetStaticBox(), ID_COMMON_NAME, _("Common Name (string)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_common_name->SetValue( true ); 
	bSizer68->Add( m_common_name, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	m_biotype_int = new wxRadioButton( sbSizer14->GetStaticBox(), ID_BIOTYPE_INT, _("BioType ID (integer)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_biotype_int->SetValue( true ); 
	bSizer68->Add( m_biotype_int, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	m_biotype_string = new wxRadioButton( sbSizer14->GetStaticBox(), ID_BIOTYPE_STRING, _("BioType Name (string)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_biotype_string->SetValue( true ); 
	bSizer68->Add( m_biotype_string, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	sbSizer14->Add( bSizer68, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	bSizer65->Add( sbSizer14, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxStaticBoxSizer* sbSizer15;
	sbSizer15 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Height") ), wxVERTICAL );
	
	m_height_random = new wxRadioButton( sbSizer15->GetStaticBox(), ID_HEIGHT_RANDOM, _("Randomize"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	m_height_random->SetValue( true ); 
	sbSizer15->Add( m_height_random, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer69;
	bSizer69 = new wxBoxSizer( wxHORIZONTAL );
	
	m_height_fixed = new wxRadioButton( sbSizer15->GetStaticBox(), ID_HEIGHT_FIXED, _("Fixed:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_height_fixed->SetValue( true ); 
	bSizer69->Add( m_height_fixed, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_height_fixed_value = new wxTextCtrl( sbSizer15->GetStaticBox(), ID_HEIGHT_FIXED_VALUE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_height_fixed_value->SetMaxLength( 0 ); 
	bSizer69->Add( m_height_fixed_value, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	sbSizer15->Add( bSizer69, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	wxBoxSizer* bSizer70;
	bSizer70 = new wxBoxSizer( wxHORIZONTAL );
	
	m_height_use_field = new wxRadioButton( sbSizer15->GetStaticBox(), ID_HEIGHT_USE_FIELD, _("Use field:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_height_use_field->SetValue( true ); 
	bSizer70->Add( m_height_use_field, 0, wxALIGN_CENTER|wxALL, 5 );
	
	wxArrayString m_height_fieldChoices;
	m_height_field = new wxChoice( sbSizer15->GetStaticBox(), ID_HEIGHT_FIELD, wxDefaultPosition, wxSize( 140,-1 ), m_height_fieldChoices, 0 );
	m_height_field->SetSelection( 0 );
	bSizer70->Add( m_height_field, 0, wxALIGN_CENTER|wxLEFT|wxRIGHT|wxBOTTOM, 5 );
	
	
	sbSizer15->Add( bSizer70, 0, wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer65->Add( sbSizer15, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	wxBoxSizer* bSizer71;
	bSizer71 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ok->SetDefault(); 
	bSizer71->Add( m_ok, 0, wxALIGN_CENTER|wxALL, 5 );
	
	m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer71->Add( m_cancel, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	bSizer65->Add( bSizer71, 0, wxALIGN_CENTER|wxALL, 5 );
	
	
	this->SetSizer( bSizer65 );
	this->Layout();
	bSizer65->Fit( this );
	
	this->Centre( wxBOTH );
}

VegFieldsDlgBase::~VegFieldsDlgBase()
{
}

ElevMathDlgBase::ElevMathDlgBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
	
	wxBoxSizer* bSizer176;
	bSizer176 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText118 = new wxStaticText( this, wxID_ANY, _("Create new elevation grid layer from:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText118->Wrap( -1 );
	bSizer176->Add( m_staticText118, 0, wxALL, 5 );
	
	wxArrayString m_layer1Choices;
	m_layer1 = new wxChoice( this, ID_LAYER1, wxDefaultPosition, wxDefaultSize, m_layer1Choices, 0 );
	m_layer1->SetSelection( 0 );
	bSizer176->Add( m_layer1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );
	
	wxString m_operationChoices[] = { _("+ (plus)"), _("- (minus)") };
	int m_operationNChoices = sizeof( m_operationChoices ) / sizeof( wxString );
	m_operation = new wxChoice( this, ID_OPERATION, wxDefaultPosition, wxSize( 70,-1 ), m_operationNChoices, m_operationChoices, 0 );
	m_operation->SetSelection( 0 );
	bSizer176->Add( m_operation, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	wxArrayString m_layer2Choices;
	m_layer2 = new wxChoice( this, ID_LAYER2, wxDefaultPosition, wxDefaultSize, m_layer2Choices, 0 );
	m_layer2->SetSelection( 0 );
	bSizer176->Add( m_layer2, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer11;
	fgSizer11 = new wxFlexGridSizer( 2, 3, 0, 0 );
	fgSizer11->SetFlexibleDirection( wxBOTH );
	fgSizer11->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText119 = new wxStaticText( this, wxID_ANY, _("Sample spacing:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText119->Wrap( -1 );
	fgSizer11->Add( m_staticText119, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_spacing_x = new wxTextCtrl( this, ID_SPACING_X, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_spacing_x->SetMaxLength( 0 ); 
	fgSizer11->Add( m_spacing_x, 0, wxALL, 5 );
	
	m_spacing_y = new wxTextCtrl( this, ID_SPACING_Y, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_spacing_y->SetMaxLength( 0 ); 
	fgSizer11->Add( m_spacing_y, 0, wxALL, 5 );
	
	m_staticText1191 = new wxStaticText( this, wxID_ANY, _("Grid size:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1191->Wrap( -1 );
	fgSizer11->Add( m_staticText1191, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_grid_x = new wxTextCtrl( this, ID_GRID_X, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_grid_x->SetMaxLength( 0 ); 
	fgSizer11->Add( m_grid_x, 0, wxALL, 5 );
	
	m_grid_y = new wxTextCtrl( this, ID_GRID_Y, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_grid_y->SetMaxLength( 0 ); 
	fgSizer11->Add( m_grid_y, 0, wxALL, 5 );
	
	
	bSizer176->Add( fgSizer11, 1, wxEXPAND, 5 );
	
	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton( this, wxID_OK );
	m_sdbSizer1->AddButton( m_sdbSizer1OK );
	m_sdbSizer1Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer1->AddButton( m_sdbSizer1Cancel );
	m_sdbSizer1->Realize();
	
	bSizer176->Add( m_sdbSizer1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 10 );
	
	
	this->SetSizer( bSizer176 );
	this->Layout();
	bSizer176->Fit( this );
	
	this->Centre( wxBOTH );
}

ElevMathDlgBase::~ElevMathDlgBase()
{
}
