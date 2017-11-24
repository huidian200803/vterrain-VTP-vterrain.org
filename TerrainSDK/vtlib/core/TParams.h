//
// class TParams
//
// defines all the construction parameters for a terrain
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TPARAMSH
#define TPARAMSH

#include "vtdata/MathTypes.h"
#include "vtdata/Content.h"
#include "vtdata/LayerBase.h"

/** \addtogroup terrain */
/*@{*/

enum TextureEnum {
	TE_OBSOLETE0,
	TE_SINGLE,
	TE_OBSOLETE2,
	TE_DERIVED,
	TE_OBSOLETE4
};

/**
 * Enumeration for CLOD methods which vtlib knows about.  To extend vtlib
 * with your own CLOD algorithm class, add a value here.
 */
enum LodMethodEnum {
	LM_ROETTGER,	///< SRTerrain
	LM_TOPOVISTA,	///< TVTerrain
	LM_MCNALLY,		///< SMTerrain
	LM_DEMETER,		///< DemeterTerrain
	LM_BRUTE		///< BruteTerrain
};

// TParam Layer Types
#define TERR_LTYPE_STRUCTURE	"Structure"
#define TERR_LTYPE_ABSTRACT		"Abstract"
#define TERR_LTYPE_IMAGE		"Image"
#define TERR_LTYPE_VEGETATION	"Vegetation"
#define TERR_LTYPE_ELEVATION	"Elevation"

////////////////////////////////////////////////////////////////////////
// Class to encapsulate a scenarios parameters.

class ScenarioParams : public vtTagArray
{
public:
	ScenarioParams();

	// copy constructor and assignment operator
	ScenarioParams(const ScenarioParams& paramsSrc);
	ScenarioParams &operator = (const ScenarioParams &rhs);

	// override to catch active layers list
	void WriteOverridesToXML(FILE *fp) const;

	vtStringArray& GetActiveLayers() { return m_ActiveLayers; }

protected:
	vtStringArray m_ActiveLayers;
};

/**
 * Terrain Parameters.  These are all the values which describe how a terrain
 *  will be created.
 *
 * This class inherits from vtTagArray, a simple collection of tags with
 *  values, which provides the methods to set and get values.  A standard
 *  set of tags which TParams stores is listed below.  Related user
 *  documentation for these parameters as they are displayed in the Enviro
 *  user interface is at http://vterrain.org/Doc/Enviro/terrain_creation.html
 *
 * \par Example of working with parameters:
 \code
 TParams params;
 int height = params.GetValueInt("Min_Height");
 params.SetValueInt("Min_Height", 100);
 \endcode
 *
 * \par The standard terrain parameter tags:
 *
<table border="1" cellpadding="4" cellspacing="0" bordercolor="#111111" style="border-collapse: collapse; vertical-align:top">
<tr>
	<th>Tag</th>
	<th>Type</th>
	<th>Default</th>
	<th>Description</th>
</tr>
<tr>
	<td>Name</td>
	<td>String</td>
	<td></td>
	<td>The name of the terrain.  Example: "Big Island of Hawai'i".</td>
</tr>
<tr>
	<td>Elevation_Filename</td>
	<td>String</td>
	<td></td>
	<td>Filename for source of elevation data.</td>
</tr>
<tr>
	<td>Vertical_Exag</td>
	<td>Float</td>
	<td>1.0</td>
	<td>Vertical exaggeration, 1.0 to draw the terrain surface at its true elevation.</td>
</tr>
<tr>
	<td>Suppress_Surface</td>
	<td>Bool</td>
	<td>false</td>
	<td>True to not draw the terrain surface at all.  It will be loaded, and used
	for purposes such as planting objects, but not drawn.</td>
</tr>
<tr>
	<td>Min_Height</td>
	<td>Float</td>
	<td>20</td>
	<td>For navigation, minimum height above the ground in meters.</td>
</tr>
<tr>
	<td>Nav_Style</td>
	<td>Int</td>
	<td>0</td>
	<td>For navigation, tells the viewer what stlye to use: 0=Normal, 1=Velocity, 2=Grab-Pivot</td>
</tr>
<tr>
	<td>Nav_Speed</td>
	<td>Float</td>
	<td>100</td>
	<td>For navigation, peak speed in meters per second.</td>
</tr>
<tr>
	<td>Locations_File</td>
	<td>String</td>
	<td></td>
	<td>File which contains the stored camera locations for this terrain.</td>
</tr>
<tr>
	<td>Init_Location</td>
	<td>String</td>
	<td></td>
	<td>Name of the location at which the camera should start when the terrain
	is first visited.</td>
</tr>
<tr>
	<td>Hither_Distance</td>
	<td>String</td>
	<td>5</td>
	<td>The hither ("near") clipping plan distance, in meters.</td>
</tr>
<tr>
	<td>Accel</td>
	<td>Bool</td>
	<td>false</td>
	<td>True means to accelerate navigation by the camera's height above the
	terrain surface.</td>
</tr>
<tr>
	<td>Allow_Roll</td>
	<td>Bool</td>
	<td>false</td>
	<td>True to allow the camera to roll (in addition to pitch and yaw).</td>
</tr>
<tr>
	<td>Surface_Type</td>
	<td>Int</td>
	<td>0</td>
	<td>The type of input elevation data. 0=single grid, 1=TIN, 2=Tileset</td>
</tr>
<tr>
	<td>LOD_Method</td>
	<td>Int</td>
	<td>0</td>
	<td>The type of CLOD to use for single elevation grids (Surface_Type=0).
		See #LodMethodEnum for values.</td>
</tr>
<tr>
	<td>Tri_Count</td>
	<td>Int</td>
	<td>10000</td>
	<td>The number of triangles for the CLOD algorithm to target.</td>
</tr>
<tr>
	<td>Tristrips</td>
	<td>Bool</td>
	<td>true</td>
	<td>For the McNally CLOD, True to use triangle strips.</td>
</tr>
<tr>
	<td>Vert_Count</td>
	<td>Int</td>
	<td>20000</td>
	<td>For tiled terrain (Surface_Type=2), the number of vertices for the
	algorithm to target.</td>
</tr>
<tr>
	<td>Tile_Cache_Size</td>
	<td>Int</td>
	<td>80</td>
	<td>For tiled terrain (Surface_Type=2), the size of the tile cache to
	keep in host RAM, in MB.</td>
</tr>
<tr>
	<td>Time_On</td>
	<td>Bool</td>
	<td>false</td>
	<td>Try for the time-of-day engine to be on.</td>
</tr>
<tr>
	<td>Init_Time</td>
	<td>String</td>
	<td>104 2 21 10 0 0</td>
	<td>Initial time for the time-of-day engine.  The values are: year (+1900),
		month (0-based), day, hour, minute, second.  The default is 2004,
		March 21, 10am.</td>
</tr>
<tr>
	<td>Time_Speed</td>
	<td>Float</td>
	<td>1.0</td>
	<td>Speed of the time-of-day engine.  A value of 1 matched reality.  A value
		of 10 means that time moves 10 times faster than reality.</td>
</tr>
<tr>
	<td>Texture</td>
	<td>Int</td>
	<td>0</td>
	<td>Enumeration for source of ground texture. 0=none, 1=single,
		3=derived from elevation, 4=tileset</td>
</tr>
<tr>
	<td>Texture_Filename</td>
	<td>String</td>
	<td></td>
	<td>For the single texture or tileset, the filename to load from.</td>
</tr>
<tr>
	<td>MIP_Map</td>
	<td>Bool</td>
	<td>false</td>
	<td>Turn on mipmapping for the ground texture.</td>
</tr>
<tr>
	<td>Request_16_Bit</td>
	<td>Bool</td>
	<td>true</td>
	<td>Tells OpenGL to store the textures on the graphics card with 16-bit
		depth, which can save some graphics memory at a small cost in quality.</td>
</tr>
<tr>
	<td>Pre-Light</td>
	<td>Bool</td>
	<td>true</td>
	<td>Shades the texture (darking based on the location of the sunlight).
		This shading directly affects the texels of the ground texture, often
		called "baked-in" lighting.</td>
</tr>
<tr>
	<td>PreLight_Factor</td>
	<td>Float</td>
	<td>1.0</td>
	<td>The intensity of the pre-light shading.  1.0 gives full shading,
		lower values give a more subtle effect.</td>
</tr>
<tr>
	<td>Cast_Shadows</td>
	<td>Bool</td>
	<td>false</td>
	<td>In addition to the normal (dot-product) shading of the texture based
		on the elevation surface, also cast shadows from the terrain onto
		itself.  This can take up to a few seconds.  The time taken is
		proportional to the number of texels in shadow.</td>
</tr>
</table>

Remaining to be documented in the table:

Color_Map
Texture_Retain
Detail_Texture
DTexture_Name
Roads
Road_File
Highway
Paved
Dirt
Road_Height
Road_Distance
Road_Texture
Road_Culture
Trees
Tree_File
Tree_Distance
Trees_Use_Shaders
Fog
Fog_Distance
Fog_Color
Structure_File
Structure_Distance
Structure_Shadows
Shadow_Resolution
Shadow_Darkness
Content_File
Vehicles
Vehicle_Size
Vehicle_Speed
Sky
Sky_Texture
Ocean_Plane
Ocean_Plane_Level
Depress_Ocean
Depress_Ocean_Level
Background_Color
Distance_Tool_Height
HUD_Overlay
Scenario_Name

 * <h3>Abstract Layers</h3>
 * A terrain can have any number of abstract layers, which are (generally) loaded
 * from GIS files on disk, then created in 3D as geometry or floating text labels.
 * Each layer has a number of properties which indicates how the features should
 * appear.  See vtAbstractLayer for a description of the properties.
 */
class TParams : public vtTagArray
{
public:
	TParams();

	// copy constructor and assignment operator
	TParams(const TParams& paramsSrc);
	TParams &operator = (const TParams &rhs);

	// File IO
	bool LoadFrom(const char *fname);
	bool LoadFromXML(const char *fname);

	// override to catch structure files
	void WriteOverridesToXML(FILE *fp) const;

	void SetLodMethod(LodMethodEnum method);
	LodMethodEnum GetLodMethod() const;

	void SetTextureEnum(TextureEnum tex);
	TextureEnum GetTextureEnum() const;

	// Convenience method
	void SetOverlay(const vtString &fname, int x, int y);
	bool GetOverlay(vtString &fname, int &x, int &y) const;

	// Convenience
	uint NumLayers() { return m_Layers.size(); }
	int NumLayersOfType(const vtString &layer_type);
	LayerType GetLayerType(int iLayerNum);

public:
	// this must be a public member (currently..)
	std::vector<vtTagArray> m_Layers;
	vtStringArray m_AnimPaths;
	std::vector<ScenarioParams> m_Scenarios;

private:
	void ConvertOldTimeValue();
};

////////////////////////////////////////////////////////////////////////

#define STR_TPARAMS_FORMAT_NAME "Terrain_Parameters"

#define STR_NAME "Name"
#define STR_ELEVFILE "Elevation_Filename"
#define STR_VERTICALEXAG "Vertical_Exag"
#define STR_SUPPRESS "Suppress_Surface"

#define STR_MINHEIGHT "Min_Height"
#define STR_NAVSTYLE "Nav_Style"
#define STR_NAVSPEED "Nav_Speed"
#define STR_NAVDAMPING "Nav_Damping"
#define STR_LOCFILE "Locations_File"
#define STR_INITLOCATION "Init_Location"
#define STR_HITHER "Hither_Distance"
#define STR_ACCEL "Accel"
#define STR_ALLOW_ROLL "Allow_Roll"

#define STR_SURFACE_TYPE "Surface_Type"	// 0=grid, 1=TIN, 2=tiled grid
#define STR_LODMETHOD "LOD_Method"
#define STR_TRICOUNT "Tri_Count"
#define STR_VERTCOUNT "Vert_Count"
#define STR_TILE_CACHE_SIZE "Tile_Cache_Size"	// in MB
#define STR_TILE_THREADING "Tile_Threading"

#define STR_TIMEON "Time_On"
#define STR_INITTIME "Init_Time"
#define STR_TIMESPEED "Time_Speed"

#define STR_TEXTURE "Texture"	// 1=single, 3=derived
#define STR_TEXTUREFILE "Texture_Filename"
#define STR_COLOR_MAP "Color_Map"
#define STR_TEXTURE_GEOTYPICAL "Texture_Geotypical"
#define STR_GEOTYPICAL_SCALE "Texture_Geotypical_Scale"
#define STR_TEXTURE_GRADUAL "Texture_Gradual"
#define STR_TEXURE_LOD_FACTOR "Texture_LOD_Factor"

#define STR_PRELIGHT "Pre-Light"
#define STR_PRELIGHTFACTOR "PreLight_Factor"
#define STR_CAST_SHADOWS "Cast_Shadows"
#define STR_MIPMAP "MIP_Map"
#define STR_REQUEST16BIT "Request_16_Bit"
#define STR_SHOW_UNDERSIDE "Show_Underside"
#define STR_OPACITY "Opacity"

#define STR_ROADS "Roads"
#define STR_ROADFILE "Road_File"
#define STR_HWY "Highway"
#define STR_PAVED "Paved"
#define STR_DIRT "Dirt"
#define STR_ROADHEIGHT "Road_Height"
#define STR_ROADDISTANCE "Road_Distance"
#define STR_TEXROADS "Road_Texture"
#define STR_ROADCULTURE "Road_Culture"

#define STR_TREES "Trees"
#define STR_TREEFILE "Tree_File"
#define STR_VEGDISTANCE "Tree_Distance"
#define STR_TREES_USE_SHADERS "Trees_Use_Shaders"

#define STR_FOG "Fog"
#define STR_FOGDISTANCE "Fog_Distance"
#define STR_FOGCOLOR "Fog_Color"

#define STR_CONTENT_FILE "Content_File"

#define STR_STRUCTFILE	 "Structure_File"	// Obsolete, use layers now
#define STR_STRUCTDIST	 "Structure_Distance"
#define STR_STRUCT_SHADOWS "Structure_Shadows"
#define STR_SHADOW_REZ	"Shadow_Resolution"
#define STR_SHADOW_DARKNESS "Shadow_Darkness"

#define STR_SHADOWS_DEFAULT_ON "Shadows_Default_On"
#define STR_SHADOWS_EVERY_FRAME "Shadows_Every_Frame"
#define STR_LIMIT_SHADOW_AREA	"Limit_Shadow_Area"
#define STR_SHADOW_RADIUS	"Shadow_Radius"

#define STR_STRUCTURE_PAGING		"PagingStructures"
#define STR_STRUCTURE_PAGING_MAX	"PagingStructureMax"
#define STR_STRUCTURE_PAGING_DIST	"PagingStructureDist"

#define STR_TOWERS "Trans_Towers"
#define	STR_TOWERFILE "Tower_File"

#define STR_VEHICLES "Vehicles"
#define STR_VEHICLESIZE "Vehicle_Size"
#define STR_VEHICLESPEED "Vehicle_Speed"

#define STR_SKY "Sky"
#define STR_SKYTEXTURE "Sky_Texture"
#define STR_OCEANPLANE "Ocean_Plane"
#define STR_OCEANPLANELEVEL "Ocean_Plane_Level"
#define STR_WATER "Water"
#define STR_WATERFILE "Water_File"
#define STR_DEPRESSOCEAN "Depress_Ocean"
#define STR_DEPRESSOCEANLEVEL "Depress_Ocean_Level"
#define STR_BGCOLOR "Background_Color"

#define STR_UTILITY_FILE "Utility_File"

#define STR_DIST_TOOL_HEIGHT "Distance_Tool_Height"
#define STR_HUD_OVERLAY "HUD_Overlay"
#define STR_OVERVIEW "HUD_Overview"
#define STR_COMPASS "HUD_Compass"

#define STR_INIT_SCENARIO "Init_Scenario"
#define STR_SCENARIO_NAME "Scenario_Name"

#define STR_ALLOW_GRID_SCULPTING "Allow_Sculpting"

/*@}*/	// Group terrain

#endif	// TPARAMSH

