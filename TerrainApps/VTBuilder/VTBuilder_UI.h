///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Nov 10 2014)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __VTBUILDER_UI_H__
#define __VTBUILDER_UI_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/radiobut.h>
#include <wx/choice.h>
#include <wx/statbox.h>
#include <wx/spinctrl.h>
#include <wx/checkbox.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/combobox.h>
#include <wx/listbox.h>
#include <wx/spinbutt.h>
#include <wx/bmpbuttn.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/splitter.h>

///////////////////////////////////////////////////////////////////////////

#define ID_TEXT 1000
#define ID_DEPTH 1001
#define ID_MAXERROR 1002
#define ID_SAMPLING 1003
#define ID_SCARCITY 1004
#define ID_SPECIES1 1005
#define ID_CHOICE_SPECIES 1006
#define ID_SPECIES2 1007
#define ID_CHOICE_BIOTYPE 1008
#define ID_SPECIES3 1009
#define ID_CHOICE_BIOTYPE_LAYER 1010
#define ID_DENSITY1 1011
#define ID_TEXT_FIXED_DENSITY 1012
#define ID_DENSITY2 1013
#define ID_DENSITY3 1014
#define ID_CHOICE_DENSITY_LAYER 1015
#define ID_SIZE1 1016
#define ID_TEXT_FIXED_SIZE 1017
#define ID_SIZE2 1018
#define ID_SPIN_RANDOM_FROM 1019
#define ID_SPIN_RANDOM_TO 1020
#define ID_EXTENT_N 1021
#define ID_EXTENT_W 1022
#define ID_EXTENT_E 1023
#define ID_EXTENT_S 1024
#define ID_DMS 1025
#define ID_EXTENT_ALL 1026
#define ID_SPACINGX 1027
#define ID_SPACINGY 1028
#define ID_SIZEX 1029
#define ID_SIZEY 1030
#define ID_TEXT_DIST_CUTOFF 1031
#define ID_FILE_DATA 1032
#define ID_GET_FILE_DATA 1033
#define ID_CHECK_USE1 1034
#define ID_CHECK_USE2 1035
#define ID_FILE_GAZ 1036
#define ID_GET_FILE_GAZ 1037
#define ID_FILE_ZIP 1038
#define ID_GET_FILE_ZIP 1039
#define ID_CHECK_USE3 1040
#define ID_FILE_GNS 1041
#define ID_GET_FILE_GNS 1042
#define ID_LINK_FIELD 1043
#define ID_EASTING 1044
#define ID_NORTHING 1045
#define ID_CHECK_ELEVATION 1046
#define ID_ELEVATION 1047
#define ID_CHECK_IMPORT_FIELD 1048
#define ID_IMPORT_FIELD 1049
#define ID_CRS 1050
#define ID_SET_CRS 1051
#define ID_FORMAT_DECIMAL 1052
#define ID_FORMAT_DMS 1053
#define ID_FORMAT_HDM 1054
#define ID_LONGITUDE_WEST 1055
#define ID_TYPE_LINEAR 1056
#define ID_TYPE_INSTANCE 1057
#define ID_CHOICE_FILE_FIELD 1058
#define ID_TYPE_CENTER 1059
#define ID_TYPE_FOOTPRINT 1060
#define ID_CHOICE_HEIGHT_FIELD 1061
#define ID_CHOICE_HEIGHT_TYPE 1062
#define ID_RADIO_COLOR_DEFAULT 1063
#define ID_RADIO_COLOR_FIXED 1064
#define ID_COLOR3 1065
#define ID_SET_COLOR1 1066
#define ID_COLOR4 1067
#define ID_SET_COLOR2 1068
#define ID_RADIO_ROOF_DEFAULT 1069
#define ID_RADIO_ROOF_SINGLE 1070
#define ID_CHOICE_ROOF_TYPE 1071
#define ID_SLOPECNTR 1072
#define ID_RADIO_ROOF_FIELD 1073
#define ID_CHOICE_ROOF_FIELD 1074
#define ID_DEFAULTS_FILE 1075
#define ID_INSIDE_AREA 1076
#define ID_LAYERNAME 1077
#define ID_TYPE_BUILDING 1078
#define ID_BUILD_FOUNDATIONS 1079
#define ID_USE_25D 1080
#define ID_FIELD 1081
#define ID_DENSITY 1082
#define ID_BIOTYPE1 1083
#define ID_BIOTYPE2 1084
#define ID_PROPS 1085
#define ID_LEFT 1086
#define ID_TOP 1087
#define ID_RIGHT 1088
#define ID_BOTTOM 1089
#define ID_LEVEL_CHOICE 1090
#define ID_BASE_URL 1091
#define ID_QUERY_LAYERS 1092
#define ID_LIST_LAYERS 1093
#define ID_LAYER_DESC 1094
#define ID_LIST_STYLES 1095
#define ID_STYLE_DESC 1096
#define ID_WIDTH 1097
#define ID_HEIGHT 1098
#define ID_CHOICE_FORMAT 1099
#define ID_QUERY 1100
#define ID_RADIO_CREATE_NEW 1101
#define ID_RADIO_TO_FILE 1102
#define ID_TEXT_TO_FILE 1103
#define ID_DOTDOTDOT 1104
#define ID_EXTENT1 1105
#define ID_EXTENT2 1106
#define ID_MATCH_LAYER 1107
#define ID_LAYER_RES 1108
#define ID_GROW 1109
#define ID_SHRINK 1110
#define ID_TILE_SIZE 1111
#define ID_SIZE_SPIN 1112
#define ID_TILING 1113
#define ID_INTTYPE 1114
#define ID_ROADNUM 1115
#define ID_BEHAVIOR 1116
#define ID_MINUTES 1117
#define ID_ELEVUNIT 1118
#define ID_RADIO_OUTLINE_ONLY 1119
#define ID_RADIO_COLOR 1120
#define ID_RENDER_OPTIONS 1121
#define ID_CHECK_DRAW_TIN_SIMPLE 1122
#define ID_CHECK_SHOW_ROAD_WIDTH 1123
#define ID_CHECK_DRAW_RAW_SIMPLE 1124
#define ID_PATHNAMES 1125
#define ID_RADIO1 1126
#define ID_RADIO2 1127
#define ID_RADIO3 1128
#define ID_RADIO4 1129
#define ID_RADIO5 1130
#define ID_MAX_MEGAPIXELS 1131
#define ID_SAMPLING_N 1132
#define ID_BLACK_TRANSP 1133
#define ID_DEFLATE_TIFF 1134
#define ID_RADIO6 1135
#define ID_RADIO7 1136
#define ID_RADIO8 1137
#define ID_RADIO9 1138
#define ID_RADIO10 1139
#define ID_RADIO11 1140
#define ID_ELEV_MAX_SIZE 1141
#define ID_BT_GZIP 1142
#define ID_DELAY_LOAD 1143
#define ID_MAX_MEM_GRID 1144
#define ID_BYTES 1145
#define ID_LITTLE_ENDIAN 1146
#define ID_BIG_ENDIAN 1147
#define ID_VUNITS 1148
#define ID_CRS_SIMPLE 1149
#define ID_CRS_CURRENT 1150
#define ID_CRS_EXACT 1151
#define ID_EXT_SPACING 1152
#define ID_SPACING 1153
#define ID_EXT_EXACT 1154
#define ID_EXTENTS 1155
#define ID_GEOTIFF 1156
#define ID_JPEG 1157
#define ID_CHOICE_COLORS 1158
#define ID_EDIT_COLORS 1159
#define ID_SHADING 1160
#define ID_COLOR_NODATA 1161
#define ID_CONSTRAIN 1162
#define ID_SMALLER 1163
#define ID_BIGGER 1164
#define ID_RADIO_SHADING_NONE 1165
#define ID_RADIO_SHADING_QUICK 1166
#define ID_RADIO_SHADING_DOT 1167
#define ID_SPIN_CAST_ANGLE 1168
#define ID_SPIN_CAST_DIRECTION 1169
#define ID_AMBIENT 1170
#define ID_GAMMA 1171
#define ID_CHECK_SHADOWS 1172
#define ID_NUMLANES 1173
#define ID_HWYNAME 1174
#define ID_SIDEWALK 1175
#define ID_PARKING 1176
#define ID_MARGIN 1177
#define ID_LANE_WIDTH 1178
#define ID_SIDEWALK_WIDTH 1179
#define ID_CURB_HEIGHT 1180
#define ID_PARKING_WIDTH 1181
#define ID_MARGIN_WIDTH 1182
#define ID_SURFTYPE 1183
#define ID_CONDITION 1184
#define ID_COMBO_VALUE 1185
#define ID_FLOATS 1186
#define ID_SHORTS 1187
#define ID_FILL_GAPS 1188
#define ID_AREAX 1189
#define ID_AREAY 1190
#define ID_ESTX 1191
#define ID_ESTY 1192
#define ID_SPLITTER1 1193
#define ID_LISTCTRL_SPECIES 1194
#define ID_LISTCTRL_APPEARANCES 1195
#define ID_TEXT_TO_FOLDER 1196
#define ID_COLUMNS 1197
#define ID_ROWS 1198
#define ID_CHOICE_LOD0_SIZE 1199
#define ID_SPIN_NUM_LODS 1200
#define ID_TOTALX 1201
#define ID_TOTALY 1202
#define ID_CURX 1203
#define ID_CURY 1204
#define ID_OMIT_FLAT 1205
#define ID_MASK_UNKNOWN 1206
#define ID_TEXTURE_ALPHA 1207
#define ID_TC_NONE 1208
#define ID_TC_OGL 1209
#define ID_TC_SQUISH_FAST 1210
#define ID_TC_SQUISH_SLOW 1211
#define ID_TC_JPEG 1212
#define ID_USE_SPECIES 1213
#define ID_SPECIES_CHOICE 1214
#define ID_SPECIES_USE_FIELD 1215
#define ID_SPECIES_FIELD 1216
#define ID_SPECIES_ID 1217
#define ID_SPECIES_NAME 1218
#define ID_COMMON_NAME 1219
#define ID_BIOTYPE_INT 1220
#define ID_BIOTYPE_STRING 1221
#define ID_HEIGHT_RANDOM 1222
#define ID_HEIGHT_FIXED 1223
#define ID_HEIGHT_FIXED_VALUE 1224
#define ID_HEIGHT_USE_FIELD 1225
#define ID_HEIGHT_FIELD 1226
#define ID_LAYER1 1227
#define ID_OPERATION 1228
#define ID_LAYER2 1229
#define ID_SPACING_X 1230
#define ID_SPACING_Y 1231
#define ID_GRID_X 1232
#define ID_GRID_Y 1233

///////////////////////////////////////////////////////////////////////////////
/// Class ChunkDlgBase
///////////////////////////////////////////////////////////////////////////////
class ChunkDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text36;
		wxTextCtrl* m_depth;
		wxStaticText* m_text37;
		wxTextCtrl* m_maxerror;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		ChunkDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 337,124 ), long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~ChunkDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class DistribVegDlgBase
///////////////////////////////////////////////////////////////////////////////
class DistribVegDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text16;
		wxTextCtrl* m_sampling;
		wxStaticText* m_text17;
		wxTextCtrl* m_scarcity;
		wxRadioButton* m_species1;
		wxChoice* m_choice_species;
		wxRadioButton* m_species2;
		wxChoice* m_choice_biotype;
		wxRadioButton* m_species3;
		wxChoice* m_choice_biotype_layer;
		wxRadioButton* m_density1;
		wxTextCtrl* m_text18_fixed_density;
		wxStaticText* m_text18;
		wxRadioButton* m_density2;
		wxRadioButton* m_density3;
		wxChoice* m_choice_density_layer;
		wxRadioButton* m_size1;
		wxTextCtrl* m_text_fixed_size;
		wxStaticText* m_text19;
		wxRadioButton* m_size2;
		wxSpinCtrl* m_spin_random_from;
		wxStaticText* m_text20;
		wxSpinCtrl* m_spin_random_to;
		wxStaticText* m_text21;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		DistribVegDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~DistribVegDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ExtentDlgBase
///////////////////////////////////////////////////////////////////////////////
class ExtentDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text1;
		wxTextCtrl* m_extent_n;
		wxStaticText* m_text2;
		wxTextCtrl* m_extent_w;
		wxTextCtrl* m_extent_e;
		wxStaticText* m_text3;
		wxTextCtrl* m_extent_s;
		wxStaticText* m_text4;
		wxCheckBox* m_dms;
		wxButton* m_ok;
		wxButton* m_cancel;
		wxStaticText* m_text5;
		wxTextCtrl* m_extent_all;
	
	public:
		
		ExtentDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~ExtentDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class GenGridDlgBase
///////////////////////////////////////////////////////////////////////////////
class GenGridDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text64;
		wxTextCtrl* m_spacingx;
		wxTextCtrl* m_spacingy;
		wxStaticText* m_text65;
		wxTextCtrl* m_sizex;
		wxTextCtrl* m_sizey;
		wxStaticText* m_text66;
		wxTextCtrl* m_text_dist_cutoff;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		GenGridDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 278,194 ), long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~GenGridDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class GeocodeDlgBase
///////////////////////////////////////////////////////////////////////////////
class GeocodeDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxTextCtrl* m_file_data;
		wxButton* m_get_file_data;
		wxCheckBox* m_check_use1;
		wxCheckBox* m_check_use2;
		wxStaticText* m_text34;
		wxTextCtrl* m_file_gaz;
		wxButton* m_get_file_gaz;
		wxStaticText* m_text35;
		wxTextCtrl* m_file_zip;
		wxButton* m_get_file_zip;
		wxCheckBox* m_check_use3;
		wxTextCtrl* m_file_gns;
		wxButton* m_get_file_gns;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		GeocodeDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~GeocodeDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ImageMapDlgBase
///////////////////////////////////////////////////////////////////////////////
class ImageMapDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text29;
		wxStaticText* m_text30;
		wxChoice* m_link_field;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		ImageMapDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~ImageMapDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ImportPointDlgBase
///////////////////////////////////////////////////////////////////////////////
class ImportPointDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text31;
		wxChoice* m_easting;
		wxStaticText* m_text32;
		wxChoice* m_northing;
		wxCheckBox* m_check_elevation;
		wxChoice* m_elevation;
		wxCheckBox* m_check_import_field;
		wxChoice* m_import_field;
		wxStaticText* m_text33;
		wxTextCtrl* m_crs;
		wxButton* m_set_crs;
		wxRadioButton* m_format_decimal;
		wxRadioButton* m_format_dms;
		wxRadioButton* m_format_hdm;
		wxCheckBox* m_longitude_west;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		ImportPointDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~ImportPointDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ImportStructDlgBase
///////////////////////////////////////////////////////////////////////////////
class ImportStructDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text47;
		wxRadioButton* m_type_linear;
		wxRadioButton* m_type_instance;
		wxStaticText* m_text48;
		wxChoice* m_choice_file_field;
		wxRadioButton* m_type_center;
		wxRadioButton* m_type_footprint;
		wxStaticText* m_text49;
		wxChoice* m_choice_height_field;
		wxChoice* m_choice_height_type;
		wxStaticText* m_text50;
		wxRadioButton* m_radio_color_default;
		wxRadioButton* m_radio_color_fixed;
		wxStaticBitmap* m_color3;
		wxButton* m_set_color1;
		wxStaticText* m_text1;
		wxStaticBitmap* m_color4;
		wxButton* m_set_color2;
		wxStaticText* m_text2;
		wxRadioButton* m_radio_roof_default;
		wxRadioButton* m_radio_roof_single;
		wxChoice* m_choice_roof_type;
		wxStaticText* m_text3;
		wxSpinCtrl* m_slopecntr;
		wxRadioButton* m_radio_roof_field;
		wxChoice* m_choice_roof_field;
		wxStaticText* m_text4;
		wxTextCtrl* m_defaults_file;
		wxCheckBox* m_inside_area;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		ImportStructDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~ImportStructDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ImportStructOGRDlgBase
///////////////////////////////////////////////////////////////////////////////
class ImportStructOGRDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text5;
		wxChoice* m_layername;
		wxStaticText* m_text6;
		wxChoice* m_choice_height_field;
		wxChoice* m_choice_height_type;
		wxRadioButton* m_type_building;
		wxCheckBox* m_build_foundations;
		wxRadioButton* m_type_linear;
		wxRadioButton* m_type_instance;
		wxStaticText* m_text7;
		wxChoice* m_choice_file_field;
		wxCheckBox* m_inside_area;
		wxCheckBox* m_use_25d;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		ImportStructOGRDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~ImportStructOGRDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ImportVegDlgBase
///////////////////////////////////////////////////////////////////////////////
class ImportVegDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text14;
		wxComboBox* m_field;
		wxStaticText* m_text15;
		wxRadioButton* m_density;
		wxRadioButton* m_biotype1;
		wxRadioButton* m_biotype2;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		ImportVegDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~ImportVegDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class LayerPropDlgBase
///////////////////////////////////////////////////////////////////////////////
class LayerPropDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxTextCtrl* m_props;
		wxStaticText* m_text10;
		wxTextCtrl* m_left;
		wxStaticText* m_text11;
		wxTextCtrl* m_top;
		wxStaticText* m_text12;
		wxTextCtrl* m_right;
		wxStaticText* m_text13;
		wxTextCtrl* m_bottom;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		LayerPropDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~LayerPropDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class LevelSelectionDlgBase
///////////////////////////////////////////////////////////////////////////////
class LevelSelectionDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text8;
		wxChoice* m_level_choice;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		LevelSelectionDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~LevelSelectionDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class MapServerDlgBase
///////////////////////////////////////////////////////////////////////////////
class MapServerDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text13;
		wxComboBox* m_base_url;
		wxButton* m_query_layers;
		wxStaticText* m_text14;
		wxStaticText* m_text15;
		wxListBox* m_list_layers;
		wxTextCtrl* m_layer_desc;
		wxStaticText* m_text16;
		wxStaticText* m_text17;
		wxListBox* m_list_styles;
		wxTextCtrl* m_style_desc;
		wxStaticText* m_text18;
		wxTextCtrl* m_width;
		wxStaticText* m_text19;
		wxTextCtrl* m_height;
		wxStaticText* m_text20;
		wxChoice* m_choice_format;
		wxStaticText* m_text21;
		wxTextCtrl* m_query;
		wxRadioButton* m_radio_create_new;
		wxRadioButton* m_radio_to_file;
		wxTextCtrl* m_text_to_file;
		wxButton* m_dotdotdot;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		MapServerDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~MapServerDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class MatchDlgBase
///////////////////////////////////////////////////////////////////////////////
class MatchDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text52;
		wxTextCtrl* m_extent1;
		wxStaticText* m_text53;
		wxTextCtrl* m_extent2;
		wxStaticText* m_text54;
		wxChoice* m_match_layer;
		wxStaticText* m_text55;
		wxTextCtrl* m_layer_res;
		wxStaticText* m_text56;
		wxCheckBox* m_grow;
		wxCheckBox* m_shrink;
		wxStaticText* m_text57;
		wxTextCtrl* m_tile_size;
		wxSpinButton* m_size_spin;
		wxStaticText* m_text58;
		wxTextCtrl* m_tiling;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		MatchDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 371,305 ), long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~MatchDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class NodeDlgBase
///////////////////////////////////////////////////////////////////////////////
class NodeDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text36;
		wxListBox* m_inttype;
		wxStaticText* m_text37;
		wxListBox* m_roadnum;
		wxStaticText* m_text38;
		wxListBox* m_behavior;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		NodeDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~NodeDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class OptionsDlgBase
///////////////////////////////////////////////////////////////////////////////
class OptionsDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxCheckBox* m_minutes;
		wxStaticText* m_text46;
		wxChoice* m_elevunit;
		wxRadioButton* m_radio_outline_only;
		wxRadioButton* m_radio_color;
		wxButton* m_render_options;
		wxCheckBox* m_check_draw_tin_simple;
		wxCheckBox* m_check_show_road_width;
		wxCheckBox* m_check_draw_raw_simple;
		wxCheckBox* m_pathnames;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		OptionsDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~OptionsDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class PrefDlgBase
///////////////////////////////////////////////////////////////////////////////
class PrefDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxRadioButton* m_radio1;
		wxRadioButton* m_radio2;
		wxRadioButton* m_radio3;
		wxRadioButton* m_radio4;
		wxRadioButton* m_radio5;
		wxStaticText* m_text59;
		wxTextCtrl* m_max_megapixels;
		wxStaticText* m_text60;
		wxStaticText* m_text61;
		wxTextCtrl* m_sampling_n;
		wxCheckBox* m_black_transp;
		wxCheckBox* m_deflate_tiff;
		wxRadioButton* m_radio6;
		wxRadioButton* m_radio7;
		wxRadioButton* m_radio8;
		wxRadioButton* m_radio9;
		wxRadioButton* m_radio10;
		wxRadioButton* m_radio11;
		wxStaticText* m_text62;
		wxTextCtrl* m_elev_max_size;
		wxCheckBox* m_bt_gzip;
		wxCheckBox* m_delay_load;
		wxStaticText* m_text63;
		wxTextCtrl* m_max_mem_grid;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		PrefDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 623,376 ), long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~PrefDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class RawDlgBase
///////////////////////////////////////////////////////////////////////////////
class RawDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text6;
		wxTextCtrl* m_bytes;
		wxStaticText* m_text7;
		wxTextCtrl* m_width;
		wxStaticText* m_text8;
		wxTextCtrl* m_height;
		wxRadioButton* m_little_endian;
		wxRadioButton* m_big_endian;
		wxStaticText* m_text9;
		wxTextCtrl* m_vunits;
		wxRadioButton* m_crs_simple;
		wxRadioButton* m_crs_current;
		wxRadioButton* m_crs_exact;
		wxButton* m_crs;
		wxRadioButton* m_ext_spacing;
		wxTextCtrl* m_spacing;
		wxRadioButton* m_ext_exact;
		wxButton* m_extents;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		RawDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~RawDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class RenderDlgBase
///////////////////////////////////////////////////////////////////////////////
class RenderDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxRadioButton* m_radio_create_new;
		wxRadioButton* m_radio_to_file;
		wxTextCtrl* m_text_to_file;
		wxButton* m_dotdotdot;
		wxStaticText* m_text25;
		wxRadioButton* m_geotiff;
		wxRadioButton* m_jpeg;
		wxStaticText* m_text26;
		wxChoice* m_choice_colors;
		wxButton* m_edit_colors;
		wxCheckBox* m_shading;
		wxStaticText* m_text27;
		wxBitmapButton* m_color_nodata;
		wxStaticText* m_text28;
		wxTextCtrl* m_sizex;
		wxTextCtrl* m_sizey;
		wxCheckBox* m_constrain;
		wxButton* m_smaller;
		wxButton* m_bigger;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		RenderDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~RenderDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class RenderOptionsDlgBase
///////////////////////////////////////////////////////////////////////////////
class RenderOptionsDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text47;
		wxChoice* m_choice_colors;
		wxButton* m_edit_colors;
		wxRadioButton* m_radio_shading_none;
		wxRadioButton* m_radio_shading_quick;
		wxRadioButton* m_radio_shading_dot;
		wxStaticText* m_text48;
		wxSpinCtrl* m_spin_cast_angle;
		wxStaticText* m_text49;
		wxSpinCtrl* m_spin_cast_direction;
		wxStaticText* m_text50;
		wxTextCtrl* m_ambient;
		wxStaticText* m_text51;
		wxTextCtrl* m_gamma;
		wxCheckBox* m_check_shadows;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		RenderOptionsDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 295,402 ), long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~RenderOptionsDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class RoadDlgBase
///////////////////////////////////////////////////////////////////////////////
class RoadDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text25;
		wxTextCtrl* m_numlanes;
		wxStaticText* m_text26;
		wxTextCtrl* m_hwyname;
		wxStaticText* m_text27;
		wxChoice* m_sidewalk;
		wxStaticText* m_text28;
		wxChoice* m_parking;
		wxStaticText* m_text29;
		wxChoice* m_margin;
		wxStaticText* m_text30;
		wxTextCtrl* m_lane_width;
		wxStaticText* m_text31;
		wxTextCtrl* m_sidewalk_width;
		wxStaticText* m_text32;
		wxTextCtrl* m_curb_height;
		wxStaticText* m_text33;
		wxTextCtrl* m_parking_width;
		wxStaticText* m_text34;
		wxTextCtrl* m_margin_width;
		wxStaticText* m_text35;
		wxListBox* m_surftype;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		RoadDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~RoadDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class SelectDlgBase
///////////////////////////////////////////////////////////////////////////////
class SelectDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text45;
		wxListBox* m_field;
		wxChoice* m_condition;
		wxComboBox* m_combo_value;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		SelectDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~SelectDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class SampleElevationDlgBase
///////////////////////////////////////////////////////////////////////////////
class SampleElevationDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxRadioButton* m_radio_create_new;
		wxRadioButton* m_radio_to_file;
		wxTextCtrl* m_text_to_file;
		wxButton* m_dotdotdot;
		wxStaticText* m_text39;
		wxTextCtrl* m_spacingx;
		wxTextCtrl* m_spacingy;
		wxStaticText* m_text40;
		wxTextCtrl* m_sizex;
		wxTextCtrl* m_sizey;
		wxCheckBox* m_constrain;
		wxButton* m_smaller;
		wxButton* m_bigger;
		wxRadioButton* m_floats;
		wxRadioButton* m_shorts;
		wxStaticText* m_text41;
		wxTextCtrl* m_vunits;
		wxStaticText* m_text42;
		wxCheckBox* m_fill_gaps;
		wxStaticText* m_text43;
		wxTextCtrl* m_areax;
		wxTextCtrl* m_areay;
		wxStaticText* m_text44;
		wxTextCtrl* m_estx;
		wxTextCtrl* m_esty;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		SampleElevationDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~SampleElevationDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class SampleImageDlgBase
///////////////////////////////////////////////////////////////////////////////
class SampleImageDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxRadioButton* m_radio_create_new;
		wxRadioButton* m_radio_to_file;
		wxTextCtrl* m_text_to_file;
		wxButton* m_dotdotdot;
		wxStaticText* m_text9;
		wxTextCtrl* m_sizex;
		wxTextCtrl* m_sizey;
		wxStaticText* m_text10;
		wxTextCtrl* m_spacingx;
		wxTextCtrl* m_spacingy;
		wxCheckBox* m_constrain;
		wxButton* m_smaller;
		wxButton* m_bigger;
		wxStaticText* m_text11;
		wxTextCtrl* m_areax;
		wxTextCtrl* m_areay;
		wxStaticText* m_text12;
		wxTextCtrl* m_estx;
		wxTextCtrl* m_esty;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		SampleImageDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~SampleImageDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class SpeciesListDlgBase
///////////////////////////////////////////////////////////////////////////////
class SpeciesListDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxSplitterWindow* m_splitter1;
		wxPanel* m_panel1;
		wxStaticText* m_text22;
		wxListCtrl* m_listctrl_species;
		wxPanel* m_panel2;
		wxStaticText* m_text1;
		wxListCtrl* m_listctrl_appearances;
		wxButton* m_ok;
	
	public:
		
		SpeciesListDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~SpeciesListDlgBase();
		
		void m_splitter1OnIdle( wxIdleEvent& )
		{
			m_splitter1->SetSashPosition( 0 );
			m_splitter1->Disconnect( wxEVT_IDLE, wxIdleEventHandler( SpeciesListDlgBase::m_splitter1OnIdle ), NULL, this );
		}
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class TileDlgBase
///////////////////////////////////////////////////////////////////////////////
class TileDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text38;
		wxTextCtrl* m_text_to_folder;
		wxButton* m_dotdotdot;
		wxStaticText* m_text39;
		wxTextCtrl* m_columns;
		wxStaticText* m_text40;
		wxTextCtrl* m_rows;
		wxStaticText* m_text41;
		wxChoice* m_choice_lod0_size;
		wxStaticText* m_text42;
		wxSpinCtrl* m_spin_num_lods;
		wxStaticText* m_text43;
		wxTextCtrl* m_totalx;
		wxTextCtrl* m_totaly;
		wxStaticText* m_text44;
		wxTextCtrl* m_areax;
		wxTextCtrl* m_areay;
		wxStaticText* m_text45;
		wxTextCtrl* m_estx;
		wxTextCtrl* m_esty;
		wxStaticText* m_text46;
		wxTextCtrl* m_curx;
		wxTextCtrl* m_cury;
		wxCheckBox* m_omit_flat;
		wxCheckBox* m_mask_unknown;
		wxCheckBox* m_texture_alpha;
		wxRadioButton* m_tc_none;
		wxRadioButton* m_tc_ogl;
		wxRadioButton* m_tc_squish_fast;
		wxRadioButton* m_tc_squish_slow;
		wxRadioButton* m_tc_jpeg;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		TileDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 375,559 ), long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~TileDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class VegFieldsDlgBase
///////////////////////////////////////////////////////////////////////////////
class VegFieldsDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxRadioButton* m_use_species;
		wxChoice* m_species_choice;
		wxRadioButton* m_species_use_field;
		wxChoice* m_species_field;
		wxRadioButton* m_species_id;
		wxRadioButton* m_species_name;
		wxRadioButton* m_common_name;
		wxRadioButton* m_biotype_int;
		wxRadioButton* m_biotype_string;
		wxRadioButton* m_height_random;
		wxRadioButton* m_height_fixed;
		wxTextCtrl* m_height_fixed_value;
		wxRadioButton* m_height_use_field;
		wxChoice* m_height_field;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		VegFieldsDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~VegFieldsDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ElevMathDlgBase
///////////////////////////////////////////////////////////////////////////////
class ElevMathDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticText118;
		wxChoice* m_layer1;
		wxChoice* m_operation;
		wxChoice* m_layer2;
		wxStaticText* m_staticText119;
		wxTextCtrl* m_spacing_x;
		wxTextCtrl* m_spacing_y;
		wxStaticText* m_staticText1191;
		wxTextCtrl* m_grid_x;
		wxTextCtrl* m_grid_y;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
		wxButton* m_sdbSizer1Cancel;
	
	public:
		
		ElevMathDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
		~ElevMathDlgBase();
	
};

#endif //__VTBUILDER_UI_H__
