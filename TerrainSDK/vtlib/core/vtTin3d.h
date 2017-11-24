//
// vtTin3d.h
//
// Copyright (c) 2002-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TIN3DH
#define TIN3DH

#include "vtdata/vtTin.h"
#include "vtdata/HeightField.h"
#include "TParams.h"

/** \defgroup tin TINs
 * These classes are used Triangulated Irregular Networks (TINs).
 */
/*@{*/

/**
 This class extends vtTin with the ability to call vtlib to create 3d
 geometry for the TIN.  It also subclasses vtHeightField so it provides
 the ability to do height-testing and ray-picking.

 There are three ways to apply materials to the surface.
 1. If you have an existing material you want the TIN to use, pass it with
 SetMaterial(). It can be textured or not.
 2. Or, pass some colors with SetColorMap, and those will be used (as
 vertex colors, no texturing.
 3. Or, add some surface types with vtTin::AddSurfaceType, and those
 types will be used
 */
class vtTin3d : public vtTin, public osg::Referenced
{
public:
	vtTin3d();
	~vtTin3d();

	bool Read(const char *fname, bool progress_callback(int) = NULL);

	vtGeode *CreateGeometry(bool bDropShadowMesh);
	vtGeode *GetGeometry() { return m_pGeode; }

	void SetMaterial(vtMaterialArray *pMats, int mat_idx);

	/** Takes ownership of the colormap you provide */
	void SetColorMap(ColorMap *color_map) { m_pColorMap = color_map; }

	// implement HeightField3d virtual methods
	virtual bool FindAltitudeAtPoint(const FPoint3 &p3, float &fAltitude,
		bool bTrue = false, int iCultureFlags = 0,
		FPoint3 *vNormal = NULL) const;
	virtual bool CastRayToSurface(const FPoint3 &point, const FPoint3 &dir,
		FPoint3 &result) const;

	FPoint3 FindVectorToClosestVertex(const FPoint3 &pos);
	void MakeMaterialsFromOptions(const vtTagArray &options, bool bTextureCompression);
	void MakeMaterials(ColorMap *cmap, osg::Image *image, float fScale,
		float fOpacity, bool bTextureCompression);
	vtMaterial *GetSurfaceMaterial();

protected:
	virtual void MakeSurfaceMaterials();

	vtArray<vtMesh*> m_Meshes;
	vtMaterialArrayPtr m_pMats;
	int			 m_MatIndex, m_ShadowMatIndex;
	vtGeode		*m_pGeode;
	vtGeode		*m_pDropGeode;
	ColorMap	*m_pColorMap;
	int			m_StartOfSurfaceMaterials;
};
typedef osg::ref_ptr<vtTin3d> vtTin3dPtr;

/*@}*/	// Group tin

#endif // TIN3DH
