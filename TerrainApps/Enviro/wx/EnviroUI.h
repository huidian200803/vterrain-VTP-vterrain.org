///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Nov 10 2014)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __ENVIROUI_H__
#define __ENVIROUI_H__

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
#include <wx/slider.h>
#include <wx/statline.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/dialog.h>
#include <wx/scrolwin.h>
#include <wx/combobox.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/button.h>
#include <wx/bmpbuttn.h>
#include <wx/listbox.h>
#include <wx/treectrl.h>
#include <wx/radiobut.h>
#include <wx/panel.h>
#include <wx/spinbutt.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>
#include <wx/valtext.h>
#include <wx/html/htmlwin.h>

///////////////////////////////////////////////////////////////////////////

#define ID_TEXT 1000
#define ID_CAMX 1001
#define ID_CAMY 1002
#define ID_CAMZ 1003
#define ID_FOV_TEXT 1004
#define ID_FOV 1005
#define ID_FOVSLIDER 1006
#define ID_NEAR 1007
#define ID_NEARSLIDER 1008
#define ID_FAR 1009
#define ID_FARSLIDER 1010
#define ID_EYE_SEP 1011
#define ID_EYE_SEPSLIDER 1012
#define ID_FUSION_DIST 1013
#define ID_FUSION_DIST_SLIDER 1014
#define ID_LINE 1015
#define ID_SPEED 1016
#define ID_SPEEDSLIDER 1017
#define ID_SPEED_UNITS 1018
#define ID_ACCEL 1019
#define ID_DAMPING 1020
#define ID_DAMPINGSLIDER 1021
#define ID_LOD_VEG 1022
#define ID_SLIDER_VEG 1023
#define ID_LOD_STRUCT 1024
#define ID_SLIDER_STRUCT 1025
#define ID_LOD_ROAD 1026
#define ID_SLIDER_ROAD 1027
#define ID_TURN 1028
#define ID_DISTANCE 1029
#define ID_OCEANPLANE 1030
#define ID_OCEANPLANEOFFSET 1031
#define ID_SKY 1032
#define ID_SKYTEXTURE 1033
#define ID_FOG 1034
#define ID_FOG_DISTANCE 1035
#define ID_SLIDER_FOG_DISTANCE 1036
#define ID_SHADOWS 1037
#define ID_AMBIENT_BIAS 1038
#define ID_SLIDER_AMBIENT_BIAS 1039
#define ID_SHADOWS_EVERY_FRAME 1040
#define ID_SHADOW_LIMIT 1041
#define ID_SHADOW_LIMIT_RADIUS 1042
#define ID_COLOR3 1043
#define ID_BGCOLOR 1044
#define ID_TEXT_WIND_DIRECTION 1045
#define ID_SLIDER_WIND_DIRECTION 1046
#define ID_TEXT_WIND_SPEED 1047
#define ID_SLIDER_WIND_SPEED 1048
#define ID_ANIM_POS 1049
#define ID_RESET 1050
#define ID_STOP 1051
#define ID_PLAY 1052
#define ID_LOCLIST 1053
#define ID_SAVE 1054
#define ID_LOAD 1055
#define ID_RECALL 1056
#define ID_STORE 1057
#define ID_STOREAS 1058
#define ID_REMOVE 1059
#define ID_ANIMTREE 1060
#define ID_NEW_ANIM 1061
#define ID_SAVE_ANIM 1062
#define ID_LOAD_ANIM 1063
#define ID_RECORD1 1064
#define ID_ACTIVE 1065
#define ID_LOOP 1066
#define ID_CONTINUOUS 1067
#define ID_SMOOTH 1068
#define ID_POS_ONLY 1069
#define ID_PLAY_TO_DISK 1070
#define ID_RECORD_LINEAR 1071
#define ID_RECORD_INTERVAL 1072
#define ID_RECORD_SPACING 1073
#define ID_FULLSCREEN 1074
#define ID_STEREO 1075
#define ID_STEREO1 1076
#define ID_STEREO2 1077
#define ID_STEREO3 1078
#define ID_STEREO4 1079
#define ID_TEXTURE_COMPRESSION 1080
#define ID_DISABLE_MIPMAPS 1081
#define ID_WINX 1082
#define ID_WINY 1083
#define ID_WIN_XSIZE 1084
#define ID_WIN_YSIZE 1085
#define ID_SIZE_INSIDE 1086
#define ID_DIRECT_PICKING 1087
#define ID_SELECTION_CUTOFF 1088
#define ID_SELECTION_RADIUS 1089
#define ID_PLANTSIZE 1090
#define ID_ONLY_AVAILABLE_SPECIES 1091
#define ID_CHOICE_CONTENT 1092
#define ID_TERRAIN_PROGRESS 1093
#define ID_FLY_IN 1094
#define ID_ENABLE_JOYSTICK 1095
#define ID_ENABLE_SPACENAV 1096
#define ID_NOTEBOOK 1097
#define ID_PANEL1 1098
#define ID_TARGET 1099
#define ID_SPIN 1100
#define ID_CURRENT 1101
#define ID_TILESET_STATUS 1102
#define ID_SLIDER_PRANGE 1103
#define ID_TEXT_PRANGE 1104
#define ID_PANEL2 1105
#define ID_TILE_STATUS 1106
#define ID_TEXT_PAGEOUT 1107
#define ID_SLIDER_PAGEOUT 1108
#define ID_COUNT_CURRENT 1109
#define ID_COUNT_MAXIMUM 1110
#define ID_PANEL3 1111
#define ID_PM_LISTCTRL 1112
#define ID_SPECIES 1113
#define ID_COMMON_NAMES 1114
#define ID_LANGUAGE 1115
#define ID_PLANT_HEIGHT_EDIT 1116
#define ID_HEIGHT_SLIDER 1117
#define ID_PLANT_INDIVIDUAL 1118
#define ID_PLANT_LINEAR 1119
#define ID_PLANT_CONTINUOUS 1120
#define ID_PLANT_VARIANCE_EDIT 1121
#define ID_PLANT_VARIANCE_SLIDER 1122
#define ID_PLANT_SPACING_EDIT 1123
#define ID_SCENARIO_LIST 1124
#define ID_NEW_SCENARIO 1125
#define ID_DELETE_SCENARIO 1126
#define ID_EDIT_SCENARIO 1127
#define ID_MOVEUP_SCENARIO 1128
#define ID_MOVEDOWN_SCENARIO 1129
#define ID_CHOICE_SCENARIO 1130
#define ID_SCENARIO_NAME 1131
#define ID_SCENARIO_VISIBLE_LAYERS 1132
#define ID_SCENARIO_ADD_VISIBLE_LAYER 1133
#define ID_SCENARIO_REMOVE_VISIBLE_LAYER 1134
#define ID_SCENARIO_AVAILABLE_LAYERS 1135
#define ID_SCENARIO_PREVIOUS 1136
#define ID_SCENARIO_NEXT 1137
#define ID_EARTHVIEW 1138
#define ID_IMAGETEXT 1139
#define ID_IMAGE 1140
#define ID_TERRAIN 1141
#define ID_TNAME 1142
#define ID_EDITPROP 1143
#define ID_TERRMAN 1144
#define ID_OPTIONS 1145
#define ID_OPENGL 1146
#define ID_FEATURE_TYPE 1147
#define ID_ENABLE_OBJECT_GEOM 1148
#define ID_RADIO1 1149
#define ID_OBJECT_GEOM_COLOR 1150
#define ID_RADIO_USE_OBJECT_COLOR_FIELD 1151
#define ID_OBJECT_COLOR_FIELD 1152
#define ID_OBJECT_GEOM_HEIGHT 1153
#define ID_OBJECT_GEOM_SIZE 1154
#define ID_ENABLE_LINE_GEOM 1155
#define ID_RADIO2 1156
#define ID_LINE_GEOM_COLOR 1157
#define ID_RADIO_USE_LINE_COLOR_FIELD 1158
#define ID_LINE_COLOR_FIELD 1159
#define ID_LINE_GEOM_HEIGHT 1160
#define ID_LINE_WIDTH 1161
#define ID_TESSELLATE 1162
#define ID_ENABLE_TEXT_LABELS 1163
#define ID_RADIO3 1164
#define ID_TEXT_COLOR 1165
#define ID_RADIO_USE_TEXT_COLOR_FIELD 1166
#define ID_TEXT_COLOR_FIELD 1167
#define ID_TEXT_FIELD 1168
#define ID_LABEL_HEIGHT 1169
#define ID_LABEL_SIZE 1170
#define ID_FONT 1171
#define ID_OUTLINE 1172
#define ID_TREECTRL 1173
#define ID_ADD_PATH 1174
#define ID_ADD_TERRAIN 1175
#define ID_COPY 1176
#define ID_DELETE 1177
#define ID_EDIT_PARAMS 1178
#define ID_SINGLE 1179
#define ID_TFILE_SINGLE 1180
#define ID_DERIVED 1181
#define ID_CHOICE_COLORS 1182
#define ID_EDIT_COLORS 1183
#define ID_PRELIGHT 1184
#define ID_LIGHT_FACTOR 1185
#define ID_CAST_SHADOWS 1186
#define ID_MIPMAP 1187
#define ID_16BIT 1188
#define ID_DERIVE_COLOR 1189
#define ID_GEOTYPICAL 1190
#define ID_TFILE_GEOTYPICAL 1191
#define ID_GEOTYPICAL_SCALE 1192
#define ID_OPACITY 1193
#define ID_OPACITY_SLIDER 1194
#define ID_USE_GRID 1195
#define ID_FILENAME 1196
#define ID_PRIMARY_TEXTURE 1197
#define ID_LODMETHOD 1198
#define ID_TRI_COUNT 1199
#define ID_USE_TIN 1200
#define ID_FILENAME_TIN 1201
#define ID_SET_TIN_TEXTURE 1202
#define ID_USE_TILESET 1203
#define ID_FILENAME_TILES 1204
#define ID_VTX_COUNT 1205
#define ID_TILE_THREADING 1206
#define ID_TFILE_TILESET 1207
#define ID_TEX_LOD 1208
#define ID_TEXTURE_GRADUAL 1209
#define ID_USE_EXTERNAL 1210
#define ID_TT_EXTERNAL_DATA 1211
#define ID_VERTEXAG 1212
#define ID_ELEVFILES 1213
#define ID_SET_TEXTURE 1214
#define ID_PLANTFILES 1215
#define ID_VEGDISTANCE 1216
#define ID_TREES_USE_SHADERS 1217
#define ID_ROADS 1218
#define ID_ROADFILE 1219
#define ID_ROADHEIGHT 1220
#define ID_ROADDISTANCE 1221
#define ID_TEXROADS 1222
#define ID_ROADCULTURE 1223
#define ID_HIGHWAYS 1224
#define ID_PAVED 1225
#define ID_DIRT 1226
#define ID_STRUCTFILES 1227
#define ID_STRUCT_DISTANCE 1228
#define ID_CONTENT_FILE 1229
#define ID_CHECK_STRUCTURE_SHADOWS 1230
#define ID_CHOICE_SHADOW_REZ 1231
#define ID_DARKNESS 1232
#define ID_SHADOWS_DEFAULT_ON 1233
#define ID_CHECK_STRUCTURE_PAGING 1234
#define ID_PAGING_MAX_STRUCTURES 1235
#define ID_PAGE_OUT_DISTANCE 1236
#define ID_RAWFILES 1237
#define ID_STYLE 1238
#define ID_IMAGEFILES 1239
#define ID_WATER 1240
#define ID_FILENAME_WATER 1241
#define ID_DEPRESSOCEAN 1242
#define ID_DEPRESSOCEANOFFSET 1243
#define ID_TEXT_INIT_TIME 1244
#define ID_SET_INIT_TIME 1245
#define ID_TIMEMOVES 1246
#define ID_TIMESPEED 1247
#define ID_OVERLAY_FILE 1248
#define ID_OVERLAY_DOTDOTDOT 1249
#define ID_OVERLAY_X 1250
#define ID_OVERLAY_Y 1251
#define ID_CHECK_OVERVIEW 1252
#define ID_CHECK_COMPASS 1253
#define ID_NAV_STYLE 1254
#define ID_MINHEIGHT 1255
#define ID_NAVSPEED 1256
#define ID_ALLOW_ROLL 1257
#define ID_LOCFILE 1258
#define ID_INIT_LOCATION 1259
#define ID_HITHER 1260
#define ID_ANIM_PATHS 1261
#define ID_STRUCTTYPE 1262
#define ID_CHOICE_VEHICLES 1263
#define ID_SET_VEHICLE_COLOR 1264
#define ID_VIF_RECALCULATE 1265

///////////////////////////////////////////////////////////////////////////////
/// Class CameraDlgBase
///////////////////////////////////////////////////////////////////////////////
class CameraDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text51;
		wxTextCtrl* m_camx;
		wxTextCtrl* m_camy;
		wxStaticText* m_text52;
		wxTextCtrl* m_camz;
		wxStaticText* m_fov_text;
		wxTextCtrl* m_fov;
		wxSlider* m_fovslider;
		wxStaticText* m_text53;
		wxTextCtrl* m_near;
		wxSlider* m_nearslider;
		wxStaticText* m_text54;
		wxTextCtrl* m_far;
		wxSlider* m_farslider;
		wxStaticText* m_text55;
		wxTextCtrl* m_eye_sep;
		wxSlider* m_eye_sepslider;
		wxStaticText* m_text56;
		wxTextCtrl* m_fusion_dist;
		wxSlider* m_fusion_dist_slider;
		wxStaticLine* m_line;
		wxStaticText* id_text8;
		wxTextCtrl* m_speed;
		wxSlider* m_speedslider;
		wxStaticText* m_text57;
		wxChoice* m_speed_units;
		wxCheckBox* m_accel;
		wxStaticText* id_text81;
		wxTextCtrl* m_damping;
		wxSlider* m_dampingslider;
		wxStaticText* m_text58;
		wxTextCtrl* m_lod_veg;
		wxSlider* m_slider_veg;
		wxStaticText* m_text59;
		wxTextCtrl* m_lod_struct;
		wxSlider* m_slider_struct;
		wxStaticText* m_text60;
		wxTextCtrl* m_lod_road;
		wxSlider* m_slider_road;
	
	public:
		
		CameraDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~CameraDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class DriveDlgBase
///////////////////////////////////////////////////////////////////////////////
class DriveDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticText149;
		wxTextCtrl* m_speed;
		wxStaticText* m_staticText150;
		wxCheckBox* m_follow;
		wxStaticText* m_staticText1501;
		wxTextCtrl* m_turn;
		wxStaticText* m_staticText1502;
		wxSlider* m_slider;
		wxScrolledWindow* m_area;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnFollow( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnScroll( wxScrollEvent& event ) { event.Skip(); }
		virtual void OnLeftDClick( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnLeftDown( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnLeftUp( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnMotion( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnAreaPaint( wxPaintEvent& event ) { event.Skip(); }
		
	
	public:
		
		DriveDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Drive"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE ); 
		~DriveDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class EphemDlgBase
///////////////////////////////////////////////////////////////////////////////
class EphemDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxCheckBox* m_oceanplane;
		wxStaticText* m_text204;
		wxTextCtrl* m_oceanplaneoffset;
		wxCheckBox* m_sky;
		wxComboBox* m_skytexture;
		wxCheckBox* m_fog;
		wxStaticText* m_text205;
		wxTextCtrl* m_fog_distance;
		wxSlider* m_slider_fog_distance;
		wxCheckBox* m_shadows;
		wxStaticText* m_text206;
		wxTextCtrl* m_ambient_bias;
		wxSlider* m_slider_ambient_bias;
		wxCheckBox* m_shadows_every_frame;
		wxCheckBox* m_shadow_limit;
		wxTextCtrl* m_shadow_limit_radius;
		wxStaticText* m_text222;
		wxStaticText* m_text207;
		wxStaticBitmap* m_color3;
		wxButton* m_bgcolor;
		wxStaticText* m_text208;
		wxTextCtrl* m_text_wind_direction;
		wxSlider* m_slider_wind_direction;
		wxStaticText* m_text209;
		wxTextCtrl* m_text_wind_speed;
		wxSlider* m_slider_wind_speed;
	
	public:
		
		EphemDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~EphemDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class LayerAnimDlgBase
///////////////////////////////////////////////////////////////////////////////
class LayerAnimDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxSlider* m_anim_pos;
		wxBitmapButton* m_reset;
		wxBitmapButton* m_stop;
		wxBitmapButton* m_play;
		wxStaticText* m_text163;
		wxTextCtrl* m_speed;
		wxStaticText* m_text164;
	
	public:
		
		LayerAnimDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~LayerAnimDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class LocationDlgBase
///////////////////////////////////////////////////////////////////////////////
class LocationDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxListBox* m_loclist;
		wxButton* m_save;
		wxButton* m_load;
		wxButton* m_recall;
		wxButton* m_store;
		wxButton* m_storeas;
		wxButton* m_remove;
		wxStaticLine* m_line;
		wxTreeCtrl* m_animtree;
		wxButton* m_new_anim;
		wxButton* m_save_anim;
		wxButton* m_load_anim;
		wxBitmapButton* m_reset;
		wxBitmapButton* m_stop;
		wxBitmapButton* m_record1;
		wxBitmapButton* m_play;
		wxSlider* m_anim_pos;
		wxCheckBox* m_active;
		wxStaticText* m_text49;
		wxSlider* m_speedslider;
		wxTextCtrl* m_speed;
		wxStaticText* m_text223;
		wxCheckBox* m_loop;
		wxCheckBox* m_continuous;
		wxCheckBox* m_smooth;
		wxCheckBox* m_pos_only;
		wxButton* m_play_to_disk;
		wxRadioButton* m_record_linear;
		wxRadioButton* m_record_interval;
		wxTextCtrl* m_record_spacing;
		wxStaticText* m_text50;
	
	public:
		
		LocationDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~LocationDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class OptionsDlgBase
///////////////////////////////////////////////////////////////////////////////
class OptionsDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxCheckBox* m_fullscreen;
		wxCheckBox* m_stereo;
		wxRadioButton* m_stereo1;
		wxRadioButton* m_stereo2;
		wxRadioButton* m_stereo3;
		wxRadioButton* m_stereo4;
		wxStaticText* m_staticText143;
		wxRadioButton* m_samples0;
		wxRadioButton* m_samples4;
		wxRadioButton* m_samples8;
		wxRadioButton* m_samples16;
		wxCheckBox* m_texture_compression;
		wxCheckBox* m_disable_mipmaps;
		wxStaticText* m_text118;
		wxTextCtrl* m_winx;
		wxStaticText* m_text119;
		wxTextCtrl* m_winy;
		wxStaticText* m_text120;
		wxTextCtrl* m_win_xsize;
		wxStaticText* m_text121;
		wxTextCtrl* m_win_ysize;
		wxCheckBox* m_size_inside;
		wxCheckBox* m_direct_picking;
		wxStaticText* m_text122;
		wxTextCtrl* m_selection_cutoff;
		wxStaticText* m_text123;
		wxStaticText* m_text124;
		wxTextCtrl* m_selection_radius;
		wxStaticText* m_text125;
		wxStaticText* m_text126;
		wxTextCtrl* m_plantsize;
		wxCheckBox* m_only_available_species;
		wxStaticText* m_text127;
		wxChoice* m_choice_content;
		wxCheckBox* m_terrain_progress;
		wxCheckBox* m_fly_in;
		wxCheckBox* m_fly_in1;
		wxCheckBox* m_fly_in2;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		OptionsDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~OptionsDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class PagingDlgBase
///////////////////////////////////////////////////////////////////////////////
class PagingDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxNotebook* m_notebook;
		wxPanel* SurfaceLODPanel;
		wxPanel* m_panel1;
		wxStaticText* m_text187;
		wxTextCtrl* m_textCtrl84;
		wxSpinButton* m_spinBtn1;
		wxStaticText* m_text188;
		wxTextCtrl* m_current;
		wxStaticText* m_text189;
		wxSlider* m_slider_prange;
		wxTextCtrl* m_text_prange;
		wxPanel* m_panel2;
		wxStaticBitmap* m_staticbitmap1;
		wxStaticText* m_text190;
		wxStaticBitmap* m_staticbitmap2;
		wxStaticText* m_text191;
		wxStaticBitmap* m_staticbitmap3;
		wxStaticText* m_text192;
		wxStaticBitmap* m_staticbitmap4;
		wxStaticText* m_text193;
		wxStaticBitmap* m_staticbitmap5;
		wxStaticText* m_text194;
		wxStaticBitmap* m_staticbitmap6;
		wxStaticText* m_text195;
		wxStaticBitmap* m_staticbitmap7;
		wxStaticText* m_text196;
		wxStaticBitmap* m_staticbitmap8;
		wxStaticText* m_text197;
		wxTextCtrl* m_tile_status;
		wxPanel* StructureLODPanel;
		wxStaticText* m_text198;
		wxTextCtrl* m_text_pageout;
		wxSlider* m_slider_pageout;
		wxStaticText* m_text199;
		wxTextCtrl* m_count_current;
		wxStaticText* m_text200;
		wxTextCtrl* m_count_maximum;
		wxPanel* m_panel3;
	
	public:
		
		PagingDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~PagingDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class PerformanceMonitorDlgBase
///////////////////////////////////////////////////////////////////////////////
class PerformanceMonitorDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxListCtrl* m_pm_listctrl;
		wxStaticText* m_text212;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnListItemRightClick( wxListEvent& event ) { event.Skip(); }
		
	
	public:
		
		PerformanceMonitorDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Performance Monitor"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
		~PerformanceMonitorDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class PlantDlgBase
///////////////////////////////////////////////////////////////////////////////
class PlantDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text41;
		wxChoice* mmpecies;
		wxCheckBox* m_common_names;
		wxStaticText* m_text42;
		wxChoice* id_language;
		wxStaticText* m_text221;
		wxTextCtrl* m_plant_height_edit;
		wxStaticText* m_text43;
		wxSlider* m_height_slider;
		wxRadioButton* m_plant_individual;
		wxRadioButton* m_plant_linear;
		wxRadioButton* m_plant_continuous;
		wxStaticText* m_text44;
		wxTextCtrl* m_plant_variance_edit;
		wxStaticText* m_text45;
		wxSlider* m_plant_variance_slider;
		wxStaticText* m_text46;
		wxTextCtrl* m_plant_spacing_edit;
		wxStaticText* m_text47;
	
	public:
		
		PlantDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~PlantDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ScenariosPaneBase
///////////////////////////////////////////////////////////////////////////////
class ScenariosPaneBase : public wxDialog 
{
	private:
	
	protected:
		wxListBox* m_scenario_list;
		wxButton* m_new_scenario;
		wxButton* m_delete_scenario;
		wxButton* m_edit_scenario;
		wxButton* m_moveup_scenario;
		wxButton* m_movedown_scenario;
		wxStaticText* m_text147;
		wxChoice* m_choice_scenario;
	
	public:
		
		ScenariosPaneBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~ScenariosPaneBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ScenarioParamsDlgBase
///////////////////////////////////////////////////////////////////////////////
class ScenarioParamsDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text148;
		wxTextCtrl* m_scenario_name;
		wxNotebook* m_notebook;
		wxPanel* ScenarioVisibleLayersPane;
		wxListBox* m_scenario_visible_layers;
		wxButton* m_scenario_add_visible_layer;
		wxButton* m_scenario_remove_visible_layer;
		wxListBox* m_scenario_available_layers;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		ScenarioParamsDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~ScenarioParamsDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class ScenarioSelectDlgBase
///////////////////////////////////////////////////////////////////////////////
class ScenarioSelectDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxListBox* m_scenario_list;
		wxButton* m_new_scenario;
		wxButton* m_delete_scenario;
		wxButton* m_edit_scenario;
		wxButton* m_moveup_scenario;
		wxButton* m_movedown_scenario;
		wxButton* m_scenario_previous;
		wxButton* m_scenario_next;
		wxButton* m_ok;
		wxButton* m_cancel;
		wxButton* m_apply;
	
	public:
		
		ScenarioSelectDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~ScenarioSelectDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class StartupDlgBase
///////////////////////////////////////////////////////////////////////////////
class StartupDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text1;
		wxRadioButton* id_earthview;
		wxStaticText* m_imagetext;
		wxComboBox* m_image;
		wxRadioButton* m_terrain;
		wxChoice* m_tname;
		wxButton* m_editprop;
		wxButton* m_terrman;
		wxButton* m_options;
		wxButton* m_opengl;
		wxButton* ok;
		wxButton* cancel;
	
	public:
		
		StartupDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 364,241 ), long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~StartupDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class StyleDlgBase
///////////////////////////////////////////////////////////////////////////////
class StyleDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text128;
		wxTextCtrl* m_feature_type;
		wxNotebook* m_notebook;
		wxPanel* StylePanel1;
		wxCheckBox* m_enable_object_geom;
		wxStaticText* m_text129;
		wxRadioButton* m_radio1;
		wxBitmapButton* m_object_geom_color;
		wxRadioButton* m_radio_use_object_color_field;
		wxChoice* m_object_color_field;
		wxStaticText* m_text130;
		wxTextCtrl* m_object_geom_height;
		wxStaticText* m_text131;
		wxStaticText* m_text132;
		wxTextCtrl* m_object_geom_size;
		wxStaticText* m_text133;
		wxPanel* StylePanel2;
		wxCheckBox* m_enable_line_geom;
		wxStaticText* m_text134;
		wxRadioButton* m_radio2;
		wxBitmapButton* m_line_geom_color;
		wxRadioButton* m_radio_use_line_color_field;
		wxChoice* m_line_color_field;
		wxStaticText* m_text135;
		wxTextCtrl* m_line_geom_height;
		wxStaticText* m_text136;
		wxStaticText* m_text137;
		wxTextCtrl* m_line_width;
		wxStaticText* m_text138;
		wxCheckBox* m_tessellate;
		wxPanel* StylePanel3;
		wxCheckBox* m_enable_text_labels;
		wxStaticText* m_text139;
		wxRadioButton* m_radio3;
		wxBitmapButton* m_text_color;
		wxRadioButton* m_radio_use_text_color_field;
		wxChoice* m_text_color_field;
		wxStaticText* m_text140;
		wxChoice* m_text_field;
		wxStaticText* m_text141;
		wxTextCtrl* m_label_height;
		wxStaticText* m_text142;
		wxStaticText* m_text143;
		wxTextCtrl* m_label_size;
		wxStaticText* m_text144;
		wxStaticText* m_text145;
		wxTextCtrl* m_font;
		wxCheckBox* m_outline;
		wxButton* m_ok;
		wxButton* m_cancell;
	
	public:
		
		StyleDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 480,-1 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER ); 
		~StyleDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class TerrManDlgBase
///////////////////////////////////////////////////////////////////////////////
class TerrManDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxTreeCtrl* m_treectrl;
		wxButton* m_add_path;
		wxButton* m_add_terrain;
		wxStaticLine* m_line1;
		wxButton* m_copy;
		wxButton* m_delete;
		wxStaticLine* m_line2;
		wxButton* m_edit_params;
		wxButton* m_ok;
	
	public:
		
		TerrManDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~TerrManDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class TextDlgBase
///////////////////////////////////////////////////////////////////////////////
class TextDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxTextCtrl* m_text61;
		wxButton* ok;
	
	public:
		
		TextDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~TextDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class TextureDlgBase
///////////////////////////////////////////////////////////////////////////////
class TextureDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxRadioButton* m_single;
		wxComboBox* m_tfile_single;
		wxRadioButton* m_derived;
		wxChoice* m_choice_colors;
		wxButton* id_edit_colors;
		wxCheckBox* m_prelight;
		wxStaticText* m_text9;
		wxTextCtrl* m_light_factor;
		wxCheckBox* m_cast_shadows;
		wxCheckBox* mmipmap;
		wxCheckBox* m_16bit;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		TextureDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~TextureDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class TinTextureDlgBase
///////////////////////////////////////////////////////////////////////////////
class TinTextureDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxCheckBox* m_derive_color;
		wxChoice* m_choice_colors;
		wxButton* m_edit_colors;
		wxCheckBox* m_geotypical;
		wxComboBox* m_tfile_geotypical;
		wxStaticText* m_staticText1491;
		wxTextCtrl* m_geotypical_scale;
		wxStaticText* m_staticText1501;
		wxStaticText* m_staticText14911;
		wxTextCtrl* m_opacity;
		wxSlider* m_opacity_slider;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		TinTextureDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~TinTextureDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class TParamsDlgBase
///////////////////////////////////////////////////////////////////////////////
class TParamsDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text2;
		wxTextCtrl* m_tname;
		wxNotebook* m_notebook;
		wxPanel* TParamsPanel1;
		wxRadioButton* m_use_grid;
		wxComboBox* m_filename;
		wxButton* m_primary_texture;
		wxStaticText* m_text3;
		wxChoice* m_lodmethod;
		wxStaticText* m_text4;
		wxTextCtrl* m_tri_count;
		wxRadioButton* m_use_tin;
		wxComboBox* m_filename_tin;
		wxButton* m_set_tin_texture;
		wxRadioButton* m_use_tileset;
		wxComboBox* m_filename_tileset;
		wxStaticText* m_text5;
		wxTextCtrl* m_vtx_count;
		wxCheckBox* m_tile_threading;
		wxStaticText* m_staticText1441;
		wxComboBox* m_tfile_tileset;
		wxStaticText* id_text31;
		wxTextCtrl* m_tex_lod;
		wxCheckBox* m_texture_gradual;
		wxRadioButton* m_use_external;
		wxTextCtrl* m_tt_external_data;
		wxStaticText* m_text7;
		wxTextCtrl* m_vertexag;
		wxStaticText* m_text8;
		wxPanel* TParamsPanelPlus;
		wxListBox* m_elev_files;
		wxButton* m_set_texture;
		wxPanel* TParamsPanel3;
		wxListBox* m_plant_files;
		wxStaticText* m_text14;
		wxTextCtrl* mmegdistance;
		wxStaticText* m_text15;
		wxCheckBox* m_checkBox64;
		wxCheckBox* id_roads;
		wxStaticText* m_text16;
		wxComboBox* m_roadfile;
		wxStaticText* m_text214;
		wxTextCtrl* m_roadheight;
		wxStaticText* m_text17;
		wxStaticText* id_text4;
		wxTextCtrl* m_roaddistance;
		wxStaticText* m_text18;
		wxCheckBox* m_texroads;
		wxCheckBox* m_roadculture;
		wxStaticText* m_text215;
		wxCheckBox* m_highways;
		wxCheckBox* id_paved;
		wxCheckBox* m_dirt;
		wxPanel* TParamsPanel8;
		wxListBox* m_structure_files;
		wxStaticText* m_text19;
		wxTextCtrl* m_struct_distance;
		wxStaticText* m_text20;
		wxStaticText* m_text216;
		wxComboBox* m_content_file;
		wxCheckBox* mmheck_structure_shadows;
		wxStaticText* id_text5;
		wxChoice* m_choice_shadow_rez;
		wxStaticText* m_text21;
		wxTextCtrl* m_darkness;
		wxCheckBox* m_shadows_default_on;
		wxCheckBox* m_shadows_every_frame;
		wxCheckBox* id_shadow_limit;
		wxTextCtrl* m_shadow_limit_radius;
		wxStaticText* m_text22;
		wxCheckBox* m_check_structure_paging;
		wxStaticText* m_text23;
		wxTextCtrl* m_paging_max_structures;
		wxStaticText* m_text24;
		wxTextCtrl* m_page_out_distance;
		wxStaticText* m_text25;
		wxPanel* TParamsPanel5;
		wxListBox* m_raw_files;
		wxButton* m_style;
		wxListBox* m_image_files;
		wxPanel* TParamsPanel4;
		wxCheckBox* m_oceanplane;
		wxStaticText* m_text217;
		wxTextCtrl* m_oceanplaneoffset;
		wxCheckBox* m_water;
		wxComboBox* m_filename_water;
		wxCheckBox* m_depressocean;
		wxStaticText* m_text218;
		wxTextCtrl* m_depressoceanoffset;
		wxCheckBox* id_sky;
		wxComboBox* m_skytexture;
		wxCheckBox* mmog;
		wxStaticText* m_text26;
		wxTextCtrl* id_fog_distance;
		wxStaticText* m_text27;
		wxStaticBitmap* m_color3;
		wxButton* m_bgcolor;
		wxStaticText* m_text219;
		wxTextCtrl* m_text_init_time;
		wxButton* id_set_init_time;
		wxCheckBox* m_timemoves;
		wxStaticText* m_text28;
		wxTextCtrl* m_timespeed;
		wxStaticText* m_text29;
		wxPanel* TParamsPanel6;
		wxStaticText* id_text6;
		wxTextCtrl* m_overlay_file;
		wxButton* m_overlay_dotdotdot;
		wxStaticText* m_text30;
		wxTextCtrl* m_overlay_x;
		wxTextCtrl* m_overlay_y;
		wxStaticText* m_text31;
		wxCheckBox* m_check_overview;
		wxCheckBox* m_check_compass;
		wxPanel* TParamsPanel7;
		wxStaticText* m_text32;
		wxChoice* m_nav_style;
		wxStaticText* m_text33;
		wxTextCtrl* m_minheight;
		wxStaticText* m_text34;
		wxStaticText* id_text7;
		wxTextCtrl* m_navspeed;
		wxStaticText* m_text35;
		wxStaticText* id_text71;
		wxTextCtrl* m_damping;
		wxCheckBox* m_accel;
		wxCheckBox* m_allow_roll;
		wxStaticText* m_text36;
		wxComboBox* m_locfile;
		wxStaticText* m_text37;
		wxChoice* m_init_location;
		wxStaticText* m_text38;
		wxTextCtrl* m_hither;
		wxStaticText* m_text39;
		wxStaticText* m_text220;
		wxListBox* m_anim_paths;
		wxPanel* ScenariosPanel;
		wxListBox* m_scenario_list;
		wxButton* m_new_scenario;
		wxButton* id_delete_scenario;
		wxButton* m_edit_scenario;
		wxButton* m_moveup_scenario;
		wxButton* m_movedown_scenario;
		wxStaticText* m_text40;
		wxChoice* m_choice_scenario;
		wxButton* m_ok;
		wxButton* m_cancel;
	
	public:
		
		TParamsDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~TParamsDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class UtilDlgBase
///////////////////////////////////////////////////////////////////////////////
class UtilDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_text62;
		wxChoice* m_structtype;
	
	public:
		
		UtilDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~UtilDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class VehicleDlgBase
///////////////////////////////////////////////////////////////////////////////
class VehicleDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxChoice* m_choice_vehicles;
		wxStaticText* m_text165;
		wxStaticBitmap* m_color3;
		wxButton* m_set_vehicle_color;
	
	public:
		
		VehicleDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 164,92 ), long style = wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~VehicleDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class VIADlgBase
///////////////////////////////////////////////////////////////////////////////
class VIADlgBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticText142;
		wxTextCtrl* m_VIFCtl;
		wxButton* m_button64;
		wxButton* m_button65;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnInitDialog( wxInitDialogEvent& event ) { event.Skip(); }
		virtual void OnRecalculate( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		wxString m_VIF; 
		
		VIADlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Visual Impact Calculation"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE ); 
		~VIADlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class VIAGDALOptionsDlgBase
///////////////////////////////////////////////////////////////////////////////
class VIAGDALOptionsDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxHtmlWindow *m_pHtmlWindow;
		wxTextCtrl* m_textCtrl80;
		wxStaticText* m_staticText138;
		wxTextCtrl* m_textCtrl81;
		wxStaticText* m_staticText139;
		wxTextCtrl* m_textCtrl82;
		wxStaticText* m_staticText140;
		wxComboBox* m_pDataTypeChoice;
		wxStaticText* m_staticText141;
		wxTextCtrl* m_textCtrl83;
		wxButton* m_button66;
		wxButton* m_button67;
	
	public:
		wxString m_CreationOptions; 
		wxString m_XSampleInterval; 
		wxString m_YSampleInterval; 
		wxString m_DataType; 
		wxString m_ScaleFactor; 
		
		VIAGDALOptionsDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Creation Options"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 692,594 ), long style = wxDEFAULT_DIALOG_STYLE ); 
		~VIAGDALOptionsDlgBase();
	
};

#endif //__ENVIROUI_H__
