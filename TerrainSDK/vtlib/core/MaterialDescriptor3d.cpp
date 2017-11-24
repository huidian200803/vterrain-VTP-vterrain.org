//
// MaterialDescriptor3d.cpp
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/vtLog.h"
#include "vtdata/FilePath.h"
#include "vtdata/DataPath.h"

#include "MaterialDescriptor3d.h"

const vtString BMAT_NAME_HIGHLIGHT = "Highlight";

// Helper: Linear distance in RGB space
float ColorDiff(const RGBi &c1, const RGBi &c2)
{
	FPoint3 diff;
	diff.x = (float) (c1.r - c2.r);
	diff.y = (float) (c1.g - c2.g);
	diff.z = (float) (c1.b - c2.b);
	return diff.Length();
}

/////////////////////////////////////////////////////////////////////////////
// vtMaterialDescriptorArray3d
//

vtMaterialDescriptorArray3d::vtMaterialDescriptorArray3d()
{
	m_pMaterials = NULL;
}

void vtMaterialDescriptorArray3d::InitializeMaterials()
{
	if (m_pMaterials != NULL)	// already initialized
		return;

	VTLOG("Initializing MaterialDescriptorArray3d\n");

	m_pMaterials = new vtMaterialArray;
	m_pMaterials->reserve(500);

	// Create internal materials (only needed by vtlib, not vtdata)
	m_hightlight1 = m_pMaterials->AddRGBMaterial(RGBf(1,1,1), false, false, true);
	m_hightlight2 = m_pMaterials->AddRGBMaterial(RGBf(1,0,0), false, false, true);
	m_hightlight3 = m_pMaterials->AddRGBMaterial(RGBf(1,1,0), false, false, true);

	// wire material
	m_wire = m_pMaterials->AddRGBMaterial(RGBf(0.0f, 0.0f, 0.0f), // diffuse
		false, true, false,		// culling, lighting, wireframe
		0.6f);					// alpha
	m_pMaterials->at(m_wire)->SetAmbient(RGBf(0.4f, 0.4f, 0.4f));
}

int vtMaterialDescriptorArray3d::CreateColoredMaterial(vtMaterialDescriptor *descriptor,
	const RGBf &color)
{
	// Do we already have this material in this color?
	ColorIndexMap::iterator it = descriptor->m_ColorIndexMap.find(color);
	if (it != descriptor->m_ColorIndexMap.end())
		return it->second;

	// Otherwise, make it.
	vtMaterial *pMat = MakeMaterial(descriptor, color);
	if (descriptor->GetBlending())
		pMat->SetTransparent(true);

	int index = m_pMaterials->AppendMaterial(pMat);

	// Store in map
	descriptor->m_ColorIndexMap[color] = index;

	return index;
}

int vtMaterialDescriptorArray3d::CreateSelfColoredMaterial(vtMaterialDescriptor *descriptor)
{
	// Do we already have this material?
	ColorIndexMap::iterator it = descriptor->m_ColorIndexMap.begin();
	if (it != descriptor->m_ColorIndexMap.end())
	{
		return it->second;
	}

	RGBf color(1.0f, 1.0f, 1.0f);
	vtMaterial *pMat = MakeMaterial(descriptor, color);

	vtString path = FindFileOnPaths(vtGetDataPath(), descriptor->GetTextureFilename());
	pMat->SetTexture2D(osgDB::readImageFile((const char *)path));
	pMat->SetClamp(false);	// material needs to repeat

	if (descriptor->GetBlending())
		pMat->SetTransparent(true);

	int index = m_pMaterials->AppendMaterial(pMat);

	// Store in map
	descriptor->m_ColorIndexMap[descriptor->GetRGB()] = index;

	return index;
}

int vtMaterialDescriptorArray3d::CreateColorableTextureMaterial(vtMaterialDescriptor *descriptor,
	const RGBf &color)
{
	// Do we already have this material in this color?
	ColorIndexMap::iterator it = descriptor->m_ColorIndexMap.find(color);
	if (it != descriptor->m_ColorIndexMap.end())
	{
		return it->second;
	}
	// Otherwise, try to make it.
	vtString texture_filename = descriptor->GetTextureFilename();
	vtString path = FindFileOnPaths(vtGetDataPath(), texture_filename);
	if (path == "")
	{
		VTLOG("\n\tMissing texture: %s\n", (const char *) texture_filename);
		return -1;
	}
	ImagePtr img = osgDB::readImageFile((const char *)path);
	if (!img.valid())
		return -1;

	vtMaterial *pMat = MakeMaterial(descriptor, color);
	pMat->SetTexture2D(img);
	pMat->SetMipMap(true);
	pMat->SetClamp(false);
	if (descriptor->GetBlending())
		pMat->SetTransparent(true);

	int index = m_pMaterials->AppendMaterial(pMat);

	// Store in map
	descriptor->m_ColorIndexMap[color] = index;

	return index;
}

//
// Takes the building material and color, and tries to find the closest
// existing vtMaterial.
//
int vtMaterialDescriptorArray3d::GetMatIndex(const vtString& Material,
											 const RGBf &inputColor,
											 int iType)
{
	// handle special case of internal materials
	if (Material == "Highlight")
	{
		// Choose the correct highlight
		if (inputColor == RGBf(1,1,1))
			return m_hightlight1;
		else if (inputColor == RGBf(1,0,0))
			return m_hightlight2;
		else
			return m_hightlight3;
	}
	if (Material == "Wire")
		return m_wire;

	vtMaterialDescriptor *desc = GetMatDescriptor(Material, inputColor, iType);
	if (desc == NULL)
		return -1;

	return GetMatIndex(desc, inputColor);
}

int vtMaterialDescriptorArray3d::GetMatIndex(vtMaterialDescriptor *desc,
											 const RGBf &inputColor)
{
	switch (desc->GetColorable())
	{
		case VT_MATERIAL_COLOURABLE:
			return CreateColoredMaterial(desc, inputColor);

		case VT_MATERIAL_SELFCOLOURED_TEXTURE:
			return CreateSelfColoredMaterial(desc);

		case VT_MATERIAL_COLOURABLE_TEXTURE:
			return CreateColorableTextureMaterial(desc, inputColor);
	}
	return -1;
}

vtMaterialDescriptor *vtMaterialDescriptorArray3d::GetMatDescriptor(const vtString& MaterialName,
	const RGBf &color, int iType) const
{
	if (&MaterialName == NULL)
		return NULL;

	float bestError = 1E8;
	int bestMatch = -1;
	float error;

	vtMaterialDescriptor *desc;
	int i, iSize = size();
	for (i = 0; i < iSize; i++)
	{
		desc = at(i);

		// omit if the name does not match
		if (desc->GetName().CompareNoCase(MaterialName) != 0)
			continue;

		// omit if the desired type is not matched
		if (iType != -1 && iType != desc->GetMatType())
			continue;

		// look for matching name with closest color
		const RGBi rgb = desc->GetRGB();
		error = ColorDiff(rgb, color);
		if (error < bestError)
		{
			bestMatch  = i;
			bestError = error;
		}
	}
	if (bestMatch != -1)
		return at(bestMatch);
	return NULL;
}

//
// Helper to make a material
//
vtMaterial *vtMaterialDescriptorArray3d::MakeMaterial(vtMaterialDescriptor *desc,
													  const RGBf &color)
{
	vtMaterial *pMat = new vtMaterial;
	if (desc->GetAmbient())
	{
		// a purely ambient material
		pMat->SetDiffuse(0,0,0);
		pMat->SetAmbient(1,1,1);
	}
	else
	{
		pMat->SetDiffuse(color * 0.7f);
		pMat->SetAmbient(color * 0.4f);
	}
	pMat->SetSpecular(0.0f);
	pMat->SetCulling(!desc->GetTwoSided());
	pMat->SetLighting(true);
	return pMat;
}

