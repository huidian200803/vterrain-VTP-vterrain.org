//
// SOG.h
//
// Header for Simple Open Geometry format
//

#ifndef SOGH
#define SOGH

#define SOG_VERSION	1
#define SOG_HEADER	"SOGF"

//
// Tokens: do NOT remove or reorder these values, or
// backward/forward compatibility breaks.  New tokens
// may be added at the end of the list.
//
enum FileToken
{
	FT_VERSION = 1,
	FT_NUM_MATERIALS,
	FT_NUM_GEOMETRIES,

	FT_GEOMETRY,
	FT_GEOMNAME,
	FT_GEOMID,
	FT_GEOMPARENTID,

	FT_NUM_MESHES,

	FT_MATERIAL,
	FT_MESH,

	FT_DIFFUSE,
	FT_SPECULAR,
	FT_AMBIENT,
	FT_EMISSION,
	FT_CULLING,
	FT_LIGHTING,
	FT_TRANSPARENT,
	FT_IMAGEFNAME,
	FT_CLAMP,

	FT_MAT_INDEX,
	FT_VTX_FLAGS,
	FT_PRIM_TYPE,
	FT_NUM_VERTICES,
	FT_NUM_INDICES,
	FT_NUM_PRIMITIVES,
	FT_VTX_COMPONENTS,

	FT_VTX_ARRAY,
	FT_VTX_DATA,
	FT_VTX_POS,
	FT_VTX_NORMAL,
	FT_VTX_COLOR,
	FT_VTX_COORD1,
	FT_VTX_COORD2,
	FT_INDEX_ARRAY,
	FT_PRIM_LEN_ARRAY
};

#endif // SOGH

