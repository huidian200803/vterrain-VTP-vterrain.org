///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __VTUI_UI_H__
#define __VTUI_UI_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/listbox.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/statbox.h>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/checkbox.h>
#include <wx/listctrl.h>
#include <wx/radiobut.h>
#include <wx/slider.h>

///////////////////////////////////////////////////////////////////////////

#define ID_TEXT 1000
#define ID_LEVEL 1001
#define ID_LEVEL_COPY 1002
#define ID_LEVEL_DEL 1003
#define ID_LEVEL_UP 1004
#define ID_LEVEL_DOWN 1005
#define ID_LINE1 1006
#define ID_EDITHEIGHTS 1007
#define ID_SET_ROOF_TYPE 1008
#define ID_STORIES 1009
#define ID_STORY_HEIGHT 1010
#define ID_MATERIAL1 1011
#define ID_SET_MATERIAL 1012
#define ID_COLOR1 1013
#define ID_SET_COLOR 1014
#define ID_EDGE_SLOPES 1015
#define ID_SET_EDGE_SLOPES 1016
#define ID_EDGE 1017
#define ID_MATERIAL2 1018
#define ID_FACADE 1019
#define ID_COLOR2 1020
#define ID_EDGE_SLOPE 1021
#define ID_FEATURES 1022
#define ID_FEAT_CLEAR 1023
#define ID_FEAT_WALL 1024
#define ID_FEAT_WINDOW 1025
#define ID_FEAT_DOOR 1026
#define ID_EDGES 1027
#define ID_CMAP_FILE 1028
#define ID_RELATIVE 1029
#define ID_BLEND 1030
#define ID_COLORLIST 1031
#define ID_CHANGE_COLOR 1032
#define ID_DELETE_ELEVATION 1033
#define ID_HEIGHT_TO_ADD 1034
#define ID_ADD 1035
#define ID_SAVE_CMAP 1036
#define ID_SAVE_AS_CMAP 1037
#define ID_LOAD_CMAP 1038
#define ID_RADIO_SINGLE 1039
#define ID_ELEV1 1040
#define ID_RADIO_EVERY 1041
#define ID_ELEV2 1042
#define ID_RADIO_CREATE 1043
#define ID_RADIO_ADD 1044
#define ID_CHOICE_LAYER 1045
#define ID_RADIO_LINE 1046
#define ID_RADIO_PATH 1047
#define ID_DIST_LOAD_PATH 1048
#define ID_DIST_TOOL_CLEAR 1049
#define ID_MAP_OFFSET 1050
#define ID_UNITS1 1051
#define ID_MAP_DIST 1052
#define ID_UNITS2 1053
#define ID_GEOD_DIST 1054
#define ID_UNITS3 1055
#define ID_GROUND_DIST 1056
#define ID_UNITS4 1057
#define ID_VERTICAL 1058
#define ID_UNITS5 1059
#define ID_TEXT_SHOW 1060
#define ID_CHOICE_SHOW 1061
#define ID_TEXT_VERTICAL 1062
#define ID_CHOICE_VERTICAL 1063
#define ID_DEL_HIGH 1064
#define ID_LIST 1065
#define ID_RADIO_CONTENT 1066
#define ID_CHOICE_FILE 1067
#define ID_CHOICE_TYPE 1068
#define ID_CHOICE_ITEM 1069
#define ID_RADIO_MODEL 1070
#define ID_MODEL_FILE 1071
#define ID_BROWSE_MODEL_FILE 1072
#define ID_LOCATION 1073
#define ID_CREATE 1074
#define ID_LINEAR_STRUCTURE_STYLE 1075
#define ID_POST_TYPE 1076
#define ID_POST_SPACING_EDIT 1077
#define ID_POST_SPACING_SLIDER 1078
#define ID_POST_HEIGHT_EDIT 1079
#define ID_POST_HEIGHT_SLIDER 1080
#define ID_POST_SIZE_EDIT 1081
#define ID_POST_SIZE_SLIDER 1082
#define ID_CHOICE_EXTENSION 1083
#define ID_CONN_TYPE 1084
#define ID_CONN_MATERIAL 1085
#define ID_CONN_TOP_EDIT 1086
#define ID_CONN_TOP_SLIDER 1087
#define ID_CONN_BOTTOM_EDIT 1088
#define ID_CONN_BOTTOM_SLIDER 1089
#define ID_CONN_WIDTH_EDIT 1090
#define ID_CONN_WIDTH_SLIDER 1091
#define ID_SLOPE 1092
#define ID_SLOPE_SLIDER 1093
#define ID_CHOICE_PROFILE 1094
#define ID_PROFILE_EDIT 1095
#define ID_CONSTANT_TOP 1096
#define ID_LINE_OF_SIGHT 1097
#define ID_VISIBILITY 1098
#define ID_FRESNEL 1099
#define ID_USE_EFFECTIVE 1100
#define ID_SHOW_CULTURE 1101
#define ID_LINE2 1102
#define ID_HEIGHT1 1103
#define ID_HEIGHT2 1104
#define ID_RF 1105
#define ID_CURVATURE 1106
#define ID_STATUS_TEXT 1107
#define ID_EXPORT_DXF 1108
#define ID_EXPORT_TRACE 1109
#define ID_EXPORT_CSV 1110
#define ID_ADD_POINT 1111
#define ID_MOVE_POINT 1112
#define ID_REMOVE_POINT 1113
#define ID_SAVE_PROF 1114
#define ID_SAVE_AS_PROF 1115
#define ID_LOAD_PROF 1116
#define ID_PROJ 1117
#define ID_SHOW_ALL_DATUMS 1118
#define ID_DATUM 1119
#define ID_ELLIPSOID 1120
#define ID_HORUNITS 1121
#define ID_ZONE 1122
#define ID_PROJPARAM 1123
#define ID_STATEPLANE 1124
#define ID_SET_EPSG 1125
#define ID_PROJ_LOAD 1126
#define ID_PROJ_SAVE 1127
#define ID_TEXTCTRL 1128
#define ID_SLIDER_RATIO 1129
#define ID_TEXT_X 1130
#define ID_TEXT_Y 1131
#define ID_STATEPLANES 1132
#define ID_NAD27 1133
#define ID_NAD83 1134
#define ID_RADIO_METERS 1135
#define ID_RADIO_FEET 1136
#define ID_RADIO_FEET_US 1137
#define ID_TAGLIST 1138

///////////////////////////////////////////////////////////////////////////////
/// Class BuildingDlgBase
///////////////////////////////////////////////////////////////////////////////
class BuildingDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text1;
		wxListBox* m_level1;
		wxButton* m_level_copy;
		wxButton* m_level_del;
		wxButton* m_level_up;
		wxButton* m_level_down;
		wxStaticLine* m_line1;
		wxButton* m_editheights;
		wxButton* m_set_roof_type;
		wxPanel* m_panel1;
		wxStaticBoxSizer* sbSizer57;
		wxStaticText* m_text2;
		wxSpinCtrl* m_stories;
		wxStaticText* m_text3;
		wxTextCtrl* m_story_height;
		wxStaticText* m_text4;
		wxStaticText* m_text5;
		wxTextCtrl* m_material1;
		wxButton* m_set_material;
		wxStaticText* m_text6;
		wxStaticBitmap* m_color1;
		wxButton* m_set_color;
		wxStaticText* m_text7;
		wxTextCtrl* m_edge_slopes;
		wxButton* m_set_edge_slopes;
		wxPanel* m_panel2;
		wxListBox* m_edge1;
		wxPanel* m_panel3;
		wxStaticText* m_text131;
		wxTextCtrl* m_material21;
		wxButton* m_set_material11;
		wxStaticText* m_text141;
		wxChoice* m_facade1;
		wxStaticText* m_text151;
		wxStaticBitmap* m_color21;
		wxButton* m_set_color11;
		wxStaticText* m_text161;
		wxTextCtrl* m_edge_slope1;
		wxTextCtrl* m_features1;
		wxButton* m_feat_clear1;
		wxButton* m_feat_wall1;
		wxButton* m_feat_window1;
		wxButton* m_feat_door1;
		wxButton* m_ok1;
		wxButton* m_edges;
	
	public:
		
		BuildingDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~BuildingDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ColorMapDlgBase
///////////////////////////////////////////////////////////////////////////////
class ColorMapDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text38;
		wxTextCtrl* m_cmap_file;
		wxCheckBox* m_relative;
		wxCheckBox* mmlend;
		wxListCtrl* id_colorlist;
		wxButton* m_change_color;
		wxButton* m_delete_elevation;
		wxStaticText* m_text39;
		wxTextCtrl* mmeight_to_add;
		wxButton* m_add;
		wxButton* m_save_cmap;
		wxButton* id_save_as_cmap;
		wxButton* m_load_cmap;
		wxStaticLine* m_line1;
		wxButton* m_close;
	
	public:
		
		ColorMapDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~ColorMapDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ContourDlgBase
///////////////////////////////////////////////////////////////////////////////
class ContourDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxRadioButton* m_radioBtn31;
		wxTextCtrl* m_elev;
		wxStaticText* m_text211;
		wxRadioButton* m_radioBtn30;
		wxTextCtrl* m_elev2;
		wxStaticText* m_text2111;
		wxStaticLine* m_staticline5;
		wxRadioButton* m_radio_create;
		wxRadioButton* m_radio_add;
		wxChoice* m_choice_layer;
		wxStdDialogButtonSizer* m_sdbSizer4;
		wxButton* m_sdbSizer4OK;
		wxButton* m_sdbSizer4Cancel;
	
	public:
		
		ContourDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~ContourDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class DistanceDlgBase
///////////////////////////////////////////////////////////////////////////////
class DistanceDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxRadioButton* m_radio_line;
		wxRadioButton* m_radio_path;
		wxButton* m_dist_load_path;
		wxButton* m_dist_tool_clear;
		wxStaticText* m_text34;
		wxTextCtrl* m_map_offset;
		wxChoice* m_units1;
		wxStaticText* id_text;
		wxTextCtrl* m_map_dist;
		wxChoice* m_units2;
		wxStaticText* m_text35;
		wxTextCtrl* mmeod_dist;
		wxChoice* m_units3;
		wxStaticText* m_text36;
		wxTextCtrl* m_ground_dist;
		wxChoice* m_units4;
		wxStaticText* m_text37;
		wxTextCtrl* m_vertical;
		wxChoice* id_units5;
	
	public:
		
		DistanceDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~DistanceDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class FeatureTableDlgBase
///////////////////////////////////////////////////////////////////////////////
class FeatureTableDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text49_show;
		wxChoice* m_choice_show;
		wxStaticText* mmext_vertical;
		wxChoice* m_choice_vertical;
		wxButton* m_del_high;
		wxListCtrl* m_list;
	
	public:
		
		FeatureTableDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~FeatureTableDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class InstanceDlgBase
///////////////////////////////////////////////////////////////////////////////
class InstanceDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxRadioButton* m_radio_content;
		wxStaticText* m_text30;
		wxChoice* m_choice_file;
		wxStaticText* m_text31;
		wxChoice* m_choice_type;
		wxStaticText* m_text32;
		wxChoice* m_choice_item;
		wxRadioButton* m_radio_model;
		wxTextCtrl* m_model_file;
		wxButton* m_browse_model_file;
		wxStaticText* m_text33;
		wxTextCtrl* m_location;
		wxButton* m_create;
	
	public:
		
		InstanceDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~InstanceDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class LinearStructDlgBase
///////////////////////////////////////////////////////////////////////////////
class LinearStructDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text17;
		wxChoice* m_linear_structure_style;
		wxStaticText* m_text18;
		wxChoice* m_post_type;
		wxStaticText* m_text19;
		wxTextCtrl* m_post_spacing_edit;
		wxSlider* m_post_spacing_slider;
		wxStaticText* m_text20;
		wxTextCtrl* m_post_height_edit;
		wxSlider* m_post_height_slider;
		wxStaticText* m_text21;
		wxTextCtrl* m_post_size_edit;
		wxSlider* m_post_size_slider;
		wxStaticText* m_text22;
		wxChoice* m_choice_extension;
		wxStaticText* m_text23;
		wxChoice* m_conn_type;
		wxStaticText* m_text24;
		wxChoice* m_conn_material;
		wxStaticText* m_text25;
		wxTextCtrl* m_conn_top_edit;
		wxSlider* m_conn_top_slider;
		wxStaticText* m_text26;
		wxTextCtrl* m_conn_bottom_edit;
		wxSlider* m_conn_bottom_slider;
		wxStaticText* m_text27;
		wxTextCtrl* m_conn_width_edit;
		wxSlider* m_conn_width_slider;
		wxStaticText* m_text28;
		wxTextCtrl* m_slope;
		wxSlider* m_slope_slider;
		wxStaticText* m_text29;
		wxChoice* m_choice_profile;
		wxButton* m_profile_edit;
		wxCheckBox* m_constant_top;
	
	public:
		
		LinearStructDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~LinearStructDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ProfileDlgBase
///////////////////////////////////////////////////////////////////////////////
class ProfileDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxCheckBox* m_line_of_sight;
		wxCheckBox* m_visibility;
		wxCheckBox* m_fresnel;
		wxCheckBox* m_use_effective;
		wxButton* m_show_culture;
		wxStaticLine* mmine2;
		wxStaticText* m_text40;
		wxTextCtrl* m_height1;
		wxStaticText* id_text;
		wxTextCtrl* m_height2;
		wxStaticText* m_text41;
		wxTextCtrl* m_rf;
		wxStaticText* m_text42;
		wxStaticText* m_text43;
		wxChoice* m_curvature;
		wxStaticLine* m_line2;
		wxTextCtrl* m_status_text;
		wxStaticText* m_text44;
		wxButton* m_export_dxf;
		wxButton* m_export_trace;
		wxButton* m_export_csv;
	
	public:
		
		ProfileDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~ProfileDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ProfileEditDlgBase
///////////////////////////////////////////////////////////////////////////////
class ProfileEditDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxRadioButton* m_add_point;
		wxRadioButton* m_move_point;
		wxRadioButton* m_remove_point;
		wxStaticLine* m_line2;
		wxButton* m_save_prof;
		wxButton* m_save_as_prof;
		wxButton* m_load_prof;
		wxButton* m_close;
	
	public:
		wxBoxSizer* viewsizer;
		
		ProfileEditDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~ProfileEditDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ProjectionDlgBase
///////////////////////////////////////////////////////////////////////////////
class ProjectionDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text45;
		wxChoice* m_proj;
		wxStaticText* mmext;
		wxCheckBox* m_show_all_datums;
		wxChoice* m_datum;
		wxStaticText* id_text;
		wxTextCtrl* m_ellipsoid;
		wxStaticText* m_text46;
		wxChoice* m_horunits;
		wxStaticText* m_text47;
		wxChoice* m_zone;
		wxStaticText* m_text48;
		wxListCtrl* m_projparam;
		wxButton* mmtateplane;
		wxButton* m_set_epsg;
		wxButton* m_proj_load;
		wxButton* m_proj_save;
		wxStaticLine* m_line2;
		wxStdDialogButtonSizer* m_sdbSizer3;
		wxButton* m_sdbSizer3OK;
		wxButton* m_sdbSizer3Cancel;
	
	public:
		
		ProjectionDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~ProjectionDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class SizeDlgBase
///////////////////////////////////////////////////////////////////////////////
class SizeDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxTextCtrl* m_textctrl;
		wxSlider* m_slider_ratio;
		wxTextCtrl* m_text_x;
		wxTextCtrl* m_text_y;
		wxStdDialogButtonSizer* m_sdbSizer2;
		wxButton* m_sdbSizer2OK;
		wxButton* m_sdbSizer2Cancel;
	
	public:
		
		SizeDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~SizeDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class StatePlaneDlgBase
///////////////////////////////////////////////////////////////////////////////
class StatePlaneDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxListBox* id_stateplanes;
		wxRadioButton* m_nad27;
		wxRadioButton* m_nad83;
		wxRadioButton* m_radio_meters;
		wxRadioButton* m_radio_feet;
		wxRadioButton* m_radio_feet_us;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
		wxButton* m_sdbSizer1Cancel;
	
	public:
		
		StatePlaneDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~StatePlaneDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class TagDlgBase
///////////////////////////////////////////////////////////////////////////////
class TagDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxListCtrl* m_taglist;
	
	public:
		
		TagDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 231,147 ), long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~TagDlgBase();
	
};

#endif //__VTUI_UI_H__
