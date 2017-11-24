//
// TParams.cpp
//
// Defines all the construction parameters for a terrain.
//
// Copyright (c) 2001-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/FilePath.h"
#include "vtdata/vtLog.h"
#include "vtdata/vtTime.h"
#include <string.h>
#include "TParams.h"


ScenarioParams::ScenarioParams()
{
	AddTag(STR_SCENARIO_NAME, "");
}

//
// copy constructor
//
ScenarioParams::ScenarioParams(const ScenarioParams &paramsSrc)
{
	*this = paramsSrc;
}

//
// assignment operator
//
ScenarioParams &ScenarioParams::operator = (const ScenarioParams &rhs)
{
	// copy parent class first
	*((vtTagArray*)this) = rhs;

	// copy the elements of this class
	m_ActiveLayers = rhs.m_ActiveLayers;

	return *this;
}

void ScenarioParams::WriteOverridesToXML(FILE *fp) const
{
	uint i;
	for (i = 0; i < m_ActiveLayers.size(); i++)
	{
		const vtString &str = m_ActiveLayers[i];
		fprintf(fp, "\t\t<ActiveLayer>");
		fputs(str, fp);
		fprintf(fp, "</ActiveLayer>\n");
	}
}

TParams::TParams() : vtTagArray()
{
	// Define tags and provide default values
	AddTag(STR_NAME, "");
	AddTag(STR_ELEVFILE, "");
	AddTag(STR_VERTICALEXAG, "1.0");

	AddTag(STR_MINHEIGHT, "20");
	AddTag(STR_NAVSTYLE, "0");
	AddTag(STR_NAVSPEED, "100");
	AddTag(STR_NAVDAMPING, "5");
	AddTag(STR_LOCFILE, "");
	AddTag(STR_INITLOCATION, "");
	AddTag(STR_HITHER, "5");
	AddTag(STR_ACCEL, "false");
	AddTag(STR_ALLOW_ROLL, "false");

	AddTag(STR_SURFACE_TYPE, "0");	// 0=grid, 1=TIN, 2=tiled grid
	AddTag(STR_LODMETHOD, "0");
	AddTag(STR_TRICOUNT, "10000");
	AddTag(STR_VERTCOUNT, "20000");
	AddTag(STR_TILE_CACHE_SIZE, "80");	// 80 MB
	AddTag(STR_TILE_THREADING, "false");

	AddTag(STR_TIMEON, "false");
	AddTag(STR_INITTIME, "104 3 21 10 0 0");	// 2004, spring equinox, 10am
	AddTag(STR_TIMESPEED, "1");

	AddTag(STR_TEXTURE, "3");		// 3 = Derived
	AddTag(STR_TEXTUREFILE, "");
	AddTag(STR_COLOR_MAP, "");
	AddTag(STR_TEXTURE_GRADUAL, "false");
	AddTag(STR_TEXURE_LOD_FACTOR, "0.25");

	AddTag(STR_PRELIGHT, "true");
	AddTag(STR_PRELIGHTFACTOR, "1.0");
	AddTag(STR_CAST_SHADOWS, "false");
	AddTag(STR_MIPMAP, "false");
	AddTag(STR_REQUEST16BIT, "true");
	AddTag(STR_SHOW_UNDERSIDE, "false");
	AddTag(STR_OPACITY, "1.0");

	AddTag(STR_ROADS, "false");
	AddTag(STR_ROADFILE, "");
	AddTag(STR_HWY, "true");
	AddTag(STR_PAVED, "true");
	AddTag(STR_DIRT, "true");
	AddTag(STR_ROADHEIGHT, "2");
	AddTag(STR_ROADDISTANCE, "2");
	AddTag(STR_TEXROADS, "true");
	AddTag(STR_ROADCULTURE, "false");

	AddTag(STR_VEGDISTANCE, "4000");	// 4 km
	AddTag(STR_TREES_USE_SHADERS, "false");

	AddTag(STR_FOG, "false");
	AddTag(STR_FOGDISTANCE, "50");		// 50 km
	AddTag(STR_FOGCOLOR, "-1 -1 -1");	// unset

	AddTag(STR_CONTENT_FILE, "");

	AddTag(STR_STRUCTDIST, "1000");		// 1 km
	AddTag(STR_STRUCT_SHADOWS, "false");
	AddTag(STR_SHADOW_REZ, "1024");
	AddTag(STR_SHADOW_DARKNESS, "0.5");

	AddTag(STR_SHADOWS_DEFAULT_ON, "true");
	AddTag(STR_SHADOWS_EVERY_FRAME, "true");
	AddTag(STR_LIMIT_SHADOW_AREA, "false");
	AddTag(STR_SHADOW_RADIUS, "300");

	AddTag(STR_STRUCTURE_PAGING, "false");
	AddTag(STR_STRUCTURE_PAGING_MAX, "2000");	// 2000 structures
	AddTag(STR_STRUCTURE_PAGING_DIST, "2000");	// 2 km

	AddTag(STR_TOWERS, "false");
	AddTag(STR_TOWERFILE, "");

	AddTag(STR_VEHICLES, "false");
	AddTag(STR_VEHICLESIZE, "1");
	AddTag(STR_VEHICLESPEED, "1");

	AddTag(STR_SKY, "true");
	AddTag(STR_SKYTEXTURE, "");
	AddTag(STR_OCEANPLANE, "false");
	AddTag(STR_OCEANPLANELEVEL, "-20");
	AddTag(STR_DEPRESSOCEAN, "false");
	AddTag(STR_DEPRESSOCEANLEVEL, "-40");
	AddTag(STR_BGCOLOR, "0 0 0");	// black

	AddTag(STR_UTILITY_FILE, "");

	AddTag(STR_DIST_TOOL_HEIGHT, "5");
	AddTag(STR_HUD_OVERLAY, "");
}

//
// copy constructor
//
TParams::TParams(const TParams &paramsSrc)
{
	*this = paramsSrc;
}

//
// assignment operator
//
TParams &TParams::operator = (const TParams &rhs)
{
	// copy parent class first
	*((vtTagArray*)this) = rhs;

	// copy the elements of this class
	m_Layers = rhs.m_Layers;
	m_AnimPaths = rhs.m_AnimPaths;
	m_Scenarios = rhs.m_Scenarios;

	return *this;
}

bool TParams::LoadFrom(const char *fname)
{
	return LoadFromXML(fname);
}

void TParams::ConvertOldTimeValue()
{
	// Convert old time values to new values
	const char *str = GetValueString(STR_INITTIME);
	if (str)
	{
		int one, two;
		int num = sscanf(str, "%d %d", &one, &two);
		if (num == 1)
		{
			vtTime time;
			time.SetTimeOfDay(one, 0, 0);
			SetValueString(STR_INITTIME, time.GetAsString());
		}
	}
}

//////////////////////////////
// Visitor class for XML parsing of TParams files.

class TParamsVisitor : public TagVisitor
{
public:
	TParamsVisitor(TParams *pParams) : TagVisitor(pParams), m_pParams(pParams), m_bInLayer(false), m_bInScenario(false) {}
	void startElement(const char *name, const XMLAttributes &atts);
	void endElement (const char *name);

protected:
	TParams *m_pParams;
	vtTagArray m_layer;
	bool m_bViz;
	ScenarioParams m_Scenario;
	bool m_bInLayer;
	bool m_bInScenario;
};

void TParamsVisitor::startElement(const char *name, const XMLAttributes &atts)
{
	TagVisitor::startElement(name, atts);

	// Detect and support old "Structure_File" parameter
	if (m_level == 2 && !strcmp(name, STR_STRUCTFILE))
	{
		m_bViz = true;
		const char *attval = atts.getValue("Visible");
		if (attval && !strcmp(attval, "false"))
			m_bViz = false;
	}
	else if (m_level == 2 && !strcmp(name, "Layer"))
	{
		m_bInLayer = true;
		m_layer.Clear();
	}
	else if (m_level == 2 && !strcmp(name, "Scenario"))
	{
		ScenarioParams EmptyScenario;

		m_Scenario = EmptyScenario;
		m_bInScenario = true;
	}
}

void TParamsVisitor::endElement(const char *name)
{
	// Detect and support old "Structure_File" parameter
	if (m_level == 2 && !strcmp(name, STR_STRUCTFILE))
	{
		vtTagArray lay;
		lay.SetValueString("Type", TERR_LTYPE_STRUCTURE, true);
		lay.SetValueString("Filename", m_data.c_str(), true);
		lay.SetValueBool("Visible", m_bViz, true);
		m_pParams->m_Layers.push_back(lay);
		m_level--;
	}
	else if (m_level == 2 && !strcmp(name, "Layer"))
	{
		if (m_layer.GetValueString("Type") ==  TERR_LTYPE_ABSTRACT)
		{
			// Older files without LabelOutline should default it to true
			if (m_layer.FindTag("LabelOutline") == NULL)
				m_layer.AddTag("LabelOutline", "true");
		}

		m_pParams->m_Layers.push_back(m_layer);
		m_level--;
		m_bInLayer = false;
	}
	else if (m_level == 2 && !strcmp(name, "AnimPath"))
	{
		m_pParams->m_AnimPaths.push_back(m_data.c_str());
		m_level--;
	}
	else if (m_level == 2 && !strcmp(name, "Scenario"))
	{
		m_pParams->m_Scenarios.push_back(m_Scenario);
		m_level--;
		m_bInScenario = false;
	}
	else if ((m_level == 3) && m_bInLayer)
	{
		// Layer properties
		m_layer.SetValueString(name, m_data.c_str(), true);
		m_level--;
	}
	else if ((m_level == 3) && m_bInScenario)
	{
		// Scenario properties
		if (!strcmp(name, "ActiveLayer"))
			m_Scenario.GetActiveLayers().push_back(m_data.c_str());
		else
			m_Scenario.SetValueString(name, m_data.c_str(), true);
		m_level--;
	}
	else
		TagVisitor::endElement(name);
}

//////////////////////////////

bool TParams::LoadFromXML(const char *fname)
{
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	VTLOG("\tReading TParams from '%s'\n", fname);

	TParamsVisitor visitor(this);
	try
	{
		std::string fname2(fname);
		readXML(fname2, visitor);
	}
	catch (xh_io_exception &ex)
	{
		const string msg = ex.getFormattedMessage();
		VTLOG(" XML problem: %s\n", msg.c_str());
		return false;
	}

	// Convert old time values to new values
	ConvertOldTimeValue();

	// Remove some obsolete stuff
	RemoveTag("Labels");
	RemoveTag("LabelFile");
	RemoveTag("Label_Field");
	RemoveTag("Label_Height");
	RemoveTag("Label_Size");
	RemoveTag("Overlay");
	RemoveTag("Num_Tiles");
	RemoveTag("Pixel_Error");
	RemoveTag("Texture_Format");
	RemoveTag("Tile_Size");
	RemoveTag("Base_Texture");
	RemoveTag("Texture_4by4");
	RemoveTag("Tristrips");

	// Is_TIN is obsolete, use Surface_Type=1 instead
	bool bOldTin = GetValueBool("Is_TIN");
	if (bOldTin)
		SetValueInt(STR_SURFACE_TYPE, 1, true);
	RemoveTag("Is_TIN");

	// Filename is obsolete, use Elevation_Filename instead
	vtTag *tag;
	tag = FindTag("Filename");
	if (tag)
	{
		SetValueString(STR_ELEVFILE, tag->value);
		RemoveTag("Filename");
	}

	// Single_Texture is obsolete, use Texture_Filename instead
	tag = FindTag("Single_Texture");
	if (tag)
	{
		SetValueString(STR_TEXTUREFILE, tag->value);
		RemoveTag("Single_Texture");
	}

	// Trees/Tree_File is obsolete, make a vegetation layer instead
	bool bOldTrees;
	if (GetValueBool("Trees", bOldTrees))
	{
		tag = FindTag("Tree_File");
		if (tag && bOldTrees)
		{
			vtTagArray layer;
			layer.SetValueString("Type", TERR_LTYPE_VEGETATION, true);
			layer.SetValueString("Filename", tag->value, true);
			m_Layers.push_back(layer);

		}
		RemoveTag("Trees");
		RemoveTag("Tree_File");
	}

	// Some texture types are obsolete
	int tex_type = GetValueInt(STR_TEXTURE);
	if (tex_type == TE_OBSOLETE0 ||
		tex_type == TE_OBSOLETE2 ||
		tex_type == TE_OBSOLETE4)
		SetValueInt(STR_TEXTURE, TE_SINGLE);

	return true;
}

void TParams::SetLodMethod(LodMethodEnum method)
{
	SetValueInt(STR_LODMETHOD, (int) method);
}

LodMethodEnum TParams::GetLodMethod() const
{
	return (LodMethodEnum) GetValueInt(STR_LODMETHOD);
}

void TParams::SetTextureEnum(TextureEnum tex)
{
	SetValueInt(STR_TEXTURE, (int) tex);
}

TextureEnum TParams::GetTextureEnum() const
{
	return (TextureEnum) GetValueInt(STR_TEXTURE);
}

void TParams::SetOverlay(const vtString &fname, int x, int y)
{
	vtString str;
	str.Format("%s,%d,%d", (const char *)fname, x, y);
	SetValueString(STR_HUD_OVERLAY, str, true);
}

bool TParams::GetOverlay(vtString &fname, int &x, int &y) const
{
	vtString ovstring = GetValueString(STR_HUD_OVERLAY);
	if (ovstring == "") return false;
	char buf[256];
	strcpy(buf, ovstring);
	const char *name = strtok(buf, ",");
	const char *xstr = strtok(NULL, ",");
	const char *ystr = strtok(NULL, ",");
	if (!fname || !xstr || !ystr)
		return false;
	fname = name;
	x = atoi(xstr);
	y = atoi(ystr);
	return true;
}

int TParams::NumLayersOfType(const vtString &layer_type)
{
	int count = 0;
	for (uint i = 0; i < m_Layers.size(); i++)
	{
		const vtTagArray &lay = m_Layers[i];
		const vtString type = lay.GetValueString("Type");
		if (type == layer_type)
			count++;
	}
	return count;
}

LayerType TParams::GetLayerType(int iLayerNum)
{
	vtString ltype = m_Layers[iLayerNum].GetValueString("Type");
	if (ltype == TERR_LTYPE_STRUCTURE)
		return LT_STRUCTURE;
	if (ltype == TERR_LTYPE_ABSTRACT)
		return LT_RAW;
	if (ltype == TERR_LTYPE_IMAGE)
		return LT_IMAGE;
	if (ltype == TERR_LTYPE_VEGETATION)
		return LT_VEG;
	if (ltype == TERR_LTYPE_ELEVATION)
		return LT_ELEVATION;
	return LT_UNKNOWN;
}

void TParams::WriteOverridesToXML(FILE *fp) const
{
	uint i;
	for (i = 0; i < m_Layers.size(); i++)
	{
		const vtTagArray &lay = m_Layers[i];
		fprintf(fp, "\t<Layer>\n");
		lay.WriteToXMLBody(fp, 2);
		fprintf(fp, "\t</Layer>\n");
	}
	for (i = 0; i < m_AnimPaths.size(); i++)
	{
		const vtString &str = m_AnimPaths[i];
		fprintf(fp, "\t<AnimPath>");
		fputs(str, fp);
		fprintf(fp, "</AnimPath>\n");
	}
	for (i = 0; i < m_Scenarios.size(); i++)
	{
		const ScenarioParams &Scenario = m_Scenarios[i];
		fprintf(fp, "\t<Scenario>\n");
		Scenario.WriteToXMLBody(fp, 2);
		fprintf(fp, "\t</Scenario>\n");
	}
}

