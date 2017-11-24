//
// MaterialDescriptor3d.cpp
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTLIB_MATERIALDESCRIPTOR3D_H
#define VTLIB_MATERIALDESCRIPTOR3D_H

extern const vtString BMAT_NAME_HIGHLIGHT;

#include "vtdata/MaterialDescriptor.h"

/**
 * This class extents vtMaterialDescriptorArray with the ability to construct
 * real vtlib Materials.  You won't need to use this class directly, unless
 * you wish to use the shared pool of materials used by the vtBuilding3d
 * class.
 */
class vtMaterialDescriptorArray3d : public vtMaterialDescriptorArray
{
public:
	vtMaterialDescriptorArray3d();

	// You can get a material index directly from its name.
	int GetMatIndex(const vtString &Material, const RGBf &inputColor = RGBf(), int iType = -1);

	// Or you can get its descriptor, then use that to get its material index.
	vtMaterialDescriptor *GetMatDescriptor(const vtString& MaterialName,
		const RGBf &color = RGBf(), int iType = -1) const;
	int GetMatIndex(vtMaterialDescriptor *desc, const RGBf &inputColor = RGBf());

	void InitializeMaterials();
	vtMaterialArray *GetMatArray() const { return m_pMaterials; };

protected:
	// There is a single array of materials, shared by all buildings.
	// This is done to save memory.  For a list of 16000+ buildings, this can
	//  save about 200MB of RAM.
	vtMaterialArrayPtr m_pMaterials;

	vtMaterial *MakeMaterial(vtMaterialDescriptor *desc, const RGBf &color);
	int CreateColoredMaterial(vtMaterialDescriptor *descriptor, const RGBf &color);
	int CreateSelfColoredMaterial(vtMaterialDescriptor *descriptor);
	int CreateColorableTextureMaterial(vtMaterialDescriptor *descriptor, const RGBf &color);

	// indices of internal materials
	int m_hightlight1, m_hightlight2, m_hightlight3, m_wire;
};

#endif // VTLIB_MATERIALDESCRIPTOR3D_H
