//
// MaterialDescriptor.h
//
// Higher-level material descriptions for use in built structures.
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
/** \file Structure.h */

#ifndef VTDATA_MATERIALDESCRIPTOR_H
#define VTDATA_MATERIALDESCRIPTOR_H

#include <map>

#include "MathTypes.h"
#include "vtString.h"

// Well known material names
#define BMAT_NAME_PLAIN			"Plain"
#define BMAT_NAME_WOOD			"Wood"
#define BMAT_NAME_SIDING		"Siding"
#define BMAT_NAME_BRICK			"Brick"
#define BMAT_NAME_PAINTED_BRICK	"Painted-Brick"
#define BMAT_NAME_ROLLED_ROOFING "Rolled-Roofing"
#define BMAT_NAME_CEMENT		"Cement"
#define BMAT_NAME_CORRUGATED	"Corrugated"
#define BMAT_NAME_DOOR			"Door"
#define BMAT_NAME_WINDOW		"Window"
#define BMAT_NAME_WINDOWWALL	"WindowWall"

enum vtMaterialColorEnum
{
	VT_MATERIAL_COLOURABLE,				// any color, untextured
	VT_MATERIAL_SELFCOLOURED_TEXTURE,	// a single texture
	VT_MATERIAL_COLOURABLE_TEXTURE		// any color, textured
};

typedef std::map<RGBf,int> ColorIndexMap;

/**
 * This class encapsulates the description of a shared material
 */
class vtMaterialDescriptor
{
public:
	vtMaterialDescriptor();
	vtMaterialDescriptor(const char *name,
					const vtString &TextureFilename,
					const vtMaterialColorEnum Colorable = VT_MATERIAL_SELFCOLOURED_TEXTURE,
					const float fUVScaleX=-1,
					const float fUVScaleY=-1,
					const bool bTwoSided = false,
					const bool bAmbient = false,
					const bool bBlended = false,
					const RGBi &Color = RGBi(0,0,0));
	~vtMaterialDescriptor();

	void SetName(const vtString& Name)
	{
		m_Name = Name;
	}
	const vtString& GetName() const
	{
		return m_Name;
	}
	/**
	\param type One of:
	- 0: A surface material, such as brick, siding, or stucco.
	- 1: An element of a structure edge, such as a door or window.
	- 2: Reserved for "Window Wall", an efficiency optimization material
		which contains both a window and a wall.
	- 3: A post material, for linear structures, such as a fencepost.
	*/
	void SetMatType(int type)
	{
		m_Type = type;
	}
	int GetMatType() const
	{
		return m_Type;
	}
	// UV Scale: texture units per meter, or -1 to scale to fit
	void SetUVScale(const float fScaleX, const float fScaleY)
	{
		m_UVScale.Set(fScaleX, fScaleY);
	}
	FPoint2 GetUVScale() const
	{
		return m_UVScale;
	}
	void SetColorable(const vtMaterialColorEnum Type)
	{
		m_Colorable = Type;
	}
	const vtMaterialColorEnum GetColorable() const
	{
		return m_Colorable;
	}
	void SetTextureFilename(const vtString &TextureFilename)
	{
		m_TextureFilename = TextureFilename;
	}
	const vtString& GetTextureFilename() const
	{
		return m_TextureFilename;
	}
	void SetRGB(const RGBi Color)
	{
		m_RGB = Color;
	}
	const RGBi GetRGB() const
	{
		return m_RGB;
	}
	void SetTwoSided(bool bTwoSided)
	{
		m_bTwoSided = bTwoSided;
	}
	const bool GetTwoSided()
	{
		return m_bTwoSided;
	}
	void SetAmbient(bool bAmbient)
	{
		m_bAmbient = bAmbient;
	}
	const bool GetAmbient()
	{
		return m_bAmbient;
	}
	void SetBlending(bool bBlending)
	{
		m_bBlending = bBlending;
	}
	const bool GetBlending()
	{
		return m_bBlending;
	}
	// Operator  overloads
	bool operator == (const vtMaterialDescriptor& rhs) const
	{
		return (m_Name == rhs.m_Name);
	}
	void WriteToFile(FILE *fp);

	// The following field is only used in 3d construction, but it's not
	//  enough distinction to warrant creating a subclass to contain it.
	// For colourable materials, maps color to material indices.
	ColorIndexMap m_ColorIndexMap;

private:
	vtString m_Name;		// Name of material
	int m_Type;				// 0 for surface materials, >0 for classification type
	vtMaterialColorEnum m_Colorable;
	vtString m_TextureFilename;	// Filename of texture, or ""
	FPoint2 m_UVScale;		// Texel scale;
	RGBi m_RGB;				// Color for VT_MATERIAL_SELFCOLOURED_TEXTURE
	bool m_bTwoSided;		// default false
	bool m_bAmbient;		// default false
	bool m_bBlending;		// default false
};

/**
 * Contains a set of material descriptors.
 */
class vtMaterialDescriptorArray : public std::vector<vtMaterialDescriptor*>
{
public:
	~vtMaterialDescriptorArray() { DestructItems(); }
	void DestructItems()
	{
		for (uint i = 0; i < size(); i++)
			delete at(i);
	}
	bool LoadExternalMaterials();
	const vtString *FindName(const char *matname) const;
	void CreatePlain();

	bool Load(const char *szFileName);
	bool Save(const char *szFileName);
};

bool LoadGlobalMaterials();
void SetGlobalMaterials(vtMaterialDescriptorArray *mats);
vtMaterialDescriptorArray *GetGlobalMaterials();
void FreeGlobalMaterials();

#endif // VTDATA_MATERIALDESCRIPTOR_H
