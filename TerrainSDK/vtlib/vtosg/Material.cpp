//
// Material.cpp - Materials for vtlib-OSG
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include <osg/PolygonMode>
#include <osg/Texture1D>

///////////////////////////////////

#define SA_ON	osg::StateAttribute::ON
#define SA_OFF	osg::StateAttribute::OFF

vtMaterial::vtMaterial() : osg::StateSet()
{
	m_pMaterial = new osg::Material;
	setAttributeAndModes(m_pMaterial.get());
}

/**
 * Copy this material from another.
 */
void vtMaterial::CopyFrom(vtMaterial *pFrom)
{
	SetDiffuse(pFrom->GetDiffuse());
	SetSpecular(pFrom->GetSpecular());
	SetAmbient(pFrom->GetAmbient());
	SetEmission(pFrom->GetEmission());

	SetCulling(pFrom->GetCulling());
	SetLighting(pFrom->GetLighting());

//	SetTexture(pFrom->GetTexture());
	SetTransparent(pFrom->GetTransparent());
}

/**
 * Set the diffuse color of this material.
 *
 * \param r,g,b	The rgb value (0.0 to 1.0) of this material
 * \param a		For a material with transparency enabled, the alpha component
 * of the diffuse color determines the overall transparency of the material.
 * This value ranges from 0 (totally transparent) to 1 (totally opaque.)
 *
 */
void vtMaterial::SetDiffuse(float r, float g, float b, float a)
{
	m_pMaterial->setDiffuse(FAB, osg::Vec4(r, g, b, a));

	if (a < 1.0f)
		SetTransparent(true);
}
/**
 * Get the diffuse color of this material.
 */
RGBAf vtMaterial::GetDiffuse() const
{
	osg::Vec4 col = m_pMaterial->getDiffuse(FAB);
	return RGBAf(col[0], col[1], col[2], col[3]);
}

/**
 * Set the specular color of this material.
 */
void vtMaterial::SetSpecular(float r, float g, float b)
{
	m_pMaterial->setSpecular(FAB, osg::Vec4(r, g, b, 1.0f));
}
/**
 * Get the specular color of this material.
 */
RGBf vtMaterial::GetSpecular() const
{
	osg::Vec4 col = m_pMaterial->getSpecular(FAB);
	return RGBf(col[0], col[1], col[2]);
}

/**
 * Set the ambient color of this material.
 */
void vtMaterial::SetAmbient(float r, float g, float b)
{
	m_pMaterial->setAmbient(FAB, osg::Vec4(r, g, b, 1.0f));
}
/**
 * Get the ambient color of this material.
 */
RGBf vtMaterial::GetAmbient() const
{
	osg::Vec4 col = m_pMaterial->getAmbient(FAB);
	return RGBf(col[0], col[1], col[2]);
}

/**
 * Set the emissive color of this material.
 */
void vtMaterial::SetEmission(float r, float g, float b)
{
	m_pMaterial->setEmission(FAB, osg::Vec4(r, g, b, 1.0f));
}
/**
 * Get the emissive color of this material.
 */
RGBf vtMaterial::GetEmission() const
{
	osg::Vec4 col = m_pMaterial->getEmission(FAB);
	return RGBf(col[0], col[1], col[2]);
}

/**
 * Set the backface culling property of this material.
 */
void vtMaterial::SetCulling(bool bCulling)
{
	setMode(GL_CULL_FACE, bCulling ? SA_ON : SA_OFF);
}
/**
 * Get the backface culling property of this material.
 */
bool vtMaterial::GetCulling() const
{
	osg::StateAttribute::GLModeValue m;
	m = getMode(GL_CULL_FACE);
	return (m == SA_ON);
}

/**
 * Set the lighting property of this material.
 */
void vtMaterial::SetLighting(bool bLighting)
{
	setMode(GL_LIGHTING, bLighting ? SA_ON : SA_OFF);
}
/**
 * Get the lighting property of this material.
 */
bool vtMaterial::GetLighting() const
{
	osg::StateAttribute::GLModeValue m;
	m = getMode(GL_LIGHTING);
	return (m == SA_ON);
}

/**
 * Set the transparent property of this material.
 *
 * \param bOn True to turn on transparency (blending).
 * \param bAdd True for additive blending.
 */
void vtMaterial::SetTransparent(bool bOn, bool bAdd)
{
//	setMode(GL_BLEND, bOn ? SA_ON : SA_OFF);
	if (bOn)
	{
		if (!m_pBlendFunc.valid())
			m_pBlendFunc = new osg::BlendFunc;
		setAttributeAndModes(m_pBlendFunc.get(), SA_ON);
		if (!m_pAlphaFunc.valid())
			m_pAlphaFunc = new osg::AlphaFunc;
		m_pAlphaFunc->setFunction(osg::AlphaFunc::GEQUAL,0.05f);
		setAttributeAndModes(m_pAlphaFunc.get(), SA_ON );
		setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

#if MAYBE_SOMEDAY
		// RJ says he needed this to make multiple transparent surfaces work
		//  properly.  In general, the transparent bin takes care of that,
		//  but there are cases where polygons (sorted by center) can end up
		//  in the wrong order.
		ref_ptr<Depth> pDepth  = new osg::Depth;
		pDepth->setWriteMask(false);
		setAttribute(pDepth.get());
#endif
	}
	else
	{
		setMode(GL_BLEND, SA_OFF);
		setRenderingHint( StateSet::OPAQUE_BIN );
	}

	if (bAdd)
	{
		if (!m_pBlendFunc.valid())
			m_pBlendFunc = new osg::BlendFunc;
//		m_pBlendFunc->setFunction(GL_ONE, GL_ONE);
//		m_pBlendFunc->setFunction(GL_SRC_COLOR, GL_DST_COLOR);
		m_pBlendFunc->setFunction(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
		setAttribute(m_pBlendFunc.get());
	}
}
/**
 * Get the transparent property of this material.
 */
bool vtMaterial::GetTransparent() const
{
	// OSG 0.8.45 and before
//	osg::StateAttribute::GLModeValue m = getMode(osg::StateAttribute::TRANSPARENCY);
	// OSG 0.9.0 onwards
	osg::StateAttribute::GLModeValue m;
	m = getMode(GL_BLEND);
	return (m == SA_ON);
}


/**
 * Set the wireframe property of this material.
 *
 * \param bOn True to turn on wireframe.
 */
void vtMaterial::SetWireframe(bool bOn)
{
	if (bOn)
	{
		osg::PolygonMode *pm = new osg::PolygonMode;
		pm->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
		setAttributeAndModes(pm, osg::StateAttribute::OVERRIDE | SA_ON);
	}
	else
	{
		// turn wireframe off
		setMode(GL_POLYGON_MODE, osg::StateAttribute::OFF);
	}
}
/**
 * Get the wireframe property of this material.
 */
bool vtMaterial::GetWireframe() const
{
	// OSG 0.9.0
	osg::StateAttribute::GLModeValue m;
	m = getMode(osg::StateAttribute::POLYGONMODE);
	return (m == SA_ON);
}

/**
 * Set the texture for this material.
 */
void vtMaterial::SetTexture2D(osg::Image *pImage, int unit, bool bCompression)
{
	osg::Texture2D *texture2d = new osg::Texture2D;

	// this stores a reference so that it won't get deleted without this material's permission
	texture2d->setImage(pImage);

	if (bCompression)
		texture2d->setInternalFormatMode(osg::Texture::USE_S3TC_DXT3_COMPRESSION);

	setTextureAttributeAndModes(unit, texture2d, SA_ON);

	// Remember for later
	if (unit >= (int) m_Textures.size())
		m_Textures.resize(unit+1);
	m_Textures[unit] = texture2d;

	// also store a refpointer to the image
	if (unit >= (int) m_Images.size())
		m_Images.resize(unit+1);
	m_Images[unit] = pImage;
}

/**
 * Set the texture for this material.
 */
void vtMaterial::SetTexture1D(osg::Image *pImage, int unit)
{
	osg::Texture1D *texture1d = new osg::Texture1D;

	texture1d->setImage(pImage);

	// Remember for later
	if (unit >= (int) m_Textures.size())
		m_Textures.resize(unit+1);
	m_Textures[unit] = texture1d;

	// also store a refpointer to the image
	if (unit >= (int) m_Images.size())
		m_Images.resize(unit+1);
	m_Images[unit] = pImage;

	setTextureAttributeAndModes(unit, texture1d, SA_ON);
}

/**
 * Returns the texture (image) associated with a material.
 */
osg::Image *vtMaterial::GetTextureImage(int unit) const
{
	// It is valid to return a non-const pointer to the image, since the image
	//  can be modified entirely independently of the material.
	if (unit < 0 || unit >= (int) m_Images.size())
		return nullptr;
	return m_Images[unit].get();
}

/**
 * Call this method to tell vtlib that you have modified the contents of a
 *  texture so it needs to be sent again to the graphics card.
 */
void vtMaterial::ModifiedTexture(int unit)
{
	if (m_Textures[unit] == NULL)
		return;

	// Two steps: first we tell the Texture it's changed, then we tell the
	//  Image it's changed.
	m_Textures[unit]->dirtyTextureObject();

	// OSG calls a modified image 'dirty'
	osg::Texture *texture = m_Textures[unit];
	osg::Texture1D *texture1D = dynamic_cast<osg::Texture1D *>(texture);
	osg::Texture2D *texture2D = dynamic_cast<osg::Texture2D *>(texture);
	if (texture1D)
		texture1D->getImage()->dirty();
	if (texture2D)
		texture2D->getImage()->dirty();
}

void vtMaterial::SetTexGen1D(const FPoint3 &scale, float offset, int unit)
{
	osg::ref_ptr<osg::TexGen> pTexgen = new osg::TexGen;

	pTexgen->setMode(osg::TexGen::EYE_LINEAR);
	pTexgen->setPlane(osg::TexGen::S, osg::Vec4(scale.x, scale.y, scale.z, offset));
	
	setTextureAttributeAndModes(unit, pTexgen.get(), osg::StateAttribute::ON);
	setTextureMode(unit, GL_TEXTURE_GEN_S,  osg::StateAttribute::ON);
}

void vtMaterial::SetTexGen2D(const FPoint2 &scale, const FPoint2 &offset,  int unit)
{
	osg::ref_ptr<osg::TexGen> pTexgen = new osg::TexGen;

	pTexgen->setMode(osg::TexGen::EYE_LINEAR);
	pTexgen->setPlane(osg::TexGen::S, osg::Vec4(scale.x, 0.0f, 0.0f, offset.x));
	pTexgen->setPlane(osg::TexGen::T, osg::Vec4(0.0f, 0.0f, scale.y, offset.y));
	
	setTextureAttributeAndModes(unit, pTexgen.get(), osg::StateAttribute::ON);
	setTextureMode(unit, GL_TEXTURE_GEN_S,  osg::StateAttribute::ON);
	setTextureMode(unit, GL_TEXTURE_GEN_T,  osg::StateAttribute::ON);
}

void vtMaterial::SetTextureMode(int iTextureMode, int unit)
{
	osg::TexEnv::Mode mode;
	if (iTextureMode == GL_ADD) mode = osg::TexEnv::ADD;
	if (iTextureMode == GL_BLEND) mode = osg::TexEnv::BLEND;
	if (iTextureMode == GL_REPLACE) mode = osg::TexEnv::REPLACE;
	if (iTextureMode == GL_MODULATE) mode = osg::TexEnv::MODULATE;
	if (iTextureMode == GL_DECAL) mode = osg::TexEnv::DECAL;
	osg::ref_ptr<osg::TexEnv> pTexEnv = new osg::TexEnv(mode);
	setTextureAttributeAndModes(unit, pTexEnv.get(), osg::StateAttribute::ON);
}

uint vtMaterial::NextAvailableTextureUnit()
{
	return m_Textures.size();
}


/**
 * Set the texture clamping property for this material.  Clamp=true prevents
 *  sub-texel interpolation at the edge of the texture.  Default is false.
 */
void vtMaterial::SetClamp(bool bClamp, int unit)
{
	if (m_Textures[unit] == NULL)
		return;
	if (bClamp)
	{
		// TODO: try   texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
		m_Textures[unit]->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
		m_Textures[unit]->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);
	}
	else
	{
		m_Textures[unit]->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
		m_Textures[unit]->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
	}
}

/**
 * Get the texture clamping property of this material.
 */
bool vtMaterial::GetClamp(int unit) const
{
	if (m_Textures[unit] == NULL)
		return false;
	osg::Texture::WrapMode w = m_Textures[unit]->getWrap(osg::Texture::WRAP_S);
	return (w == osg::Texture::CLAMP);
}

/**
 * Set the texture mipmapping property for this material.  Mip-mapping provides
 * smoother interpolation of the textured geometry when it is under-sampled
 * (generally when it is in the distance.)  Useful for avoiding unpleasant
 * aliasing artifacts, but costs 1/3 more texture memory.
 */
void vtMaterial::SetMipMap(bool bMipMap, int unit)
{
	if (m_Textures[unit] == NULL)
		return;
	if (bMipMap)
		m_Textures[unit]->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
	else
		m_Textures[unit]->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
}

/**
 * Get the texture mipmapping property of this material.
 */
bool vtMaterial::GetMipMap(int unit) const
{
	if (m_Textures[unit] == NULL)
		return false;
	osg::Texture::FilterMode m = m_Textures[unit]->getFilter(osg::Texture::MIN_FILTER);
	return (m == osg::Texture::LINEAR_MIPMAP_LINEAR);
}


/////////////////////////////////////////////////////////////////////////////
// vtMaterialArray
//

/** Find a material in this array, by pointer */
int vtMaterialArray::Find(vtMaterial *mat)
{
	for (size_t i = 0; i < size(); i++)
		if (at(i).get() == mat)
			return i;
	return -1;
}

/**
 * Create and add a simple textured material.  This method takes a osg::Image
 * and let you control many other aspects of the material.  Only the first
 * three parameters are required, the rest will be assumed with default
 * values if desired.
 *
 * \param pImage  A valid image (bitmap) which you have created.
 *
 * \param bCulling  true to cull backfaces (only the front side
 *		of each polygon is rendered.)
 *
 * \param bLighting  true to "light" the material.  This means it will
 *		use the material's color values, and any active lights to
 *		determine the color of the drawn geometry.  If false, then
 *		only the material's diffuse color is used, and it is not affected
 *		by any lights.
 *
 * \param bTransp  true for a material with some transparency (default
 *		is false).
 *
 * \param bAdditive  true for an additive material (default is false).
 *		This means that the rendered color will be added to, rather than
 *		replace, the rendering surface.  This is useful for some effects
 *		such as drawing stars against a twilight sky, in which the light
 *		of a dim star should be added to the background sky.
 *
 * \param fAmbient  Ambient material value, ranges from 0 to 1 (default 0).
 *		If lighting is enabled, this determines how this material is affected
 *		by the Ambient component of each existing light.
 *
 * \param fDiffuse  Diffuse material value, ranges from 0 to 1 (default 1).
 *		If lighting is enabled, this determines how this material is affected
 *		by the Diffuse component of each existing light.  If lighting is
 *		not enabled, this indicates the exact lighting value to use for the
 *		material.
 *
 * \param fAlpha	Alpha value (opacity), ranges from 0 (completely
 *		transparent) to 1 (opaque).  Default is 1.  If transparency is
 *		not enabled, this value is ignored.
 *
 * \param fEmissive  Emmisive material value, ranges from 0 to 1 (default 0).
 *		If lighting is enabled, this value is added to the combined
 *		effect of each existing light.  This is useful for geometry which
 *		is brighter than the existing light level, such as illuminated
 *		objects at night.
 *
 * \param bCompression  For textured materials, requests texture compression.
 *
 * \return The index of the added material.
 */
int vtMaterialArray::AddTextureMaterial(osg::Image *pImage,
						 bool bCulling, bool bLighting,
						 bool bTransp, bool bAdditive,
						 float fAmbient, float fDiffuse,
						 float fAlpha, float fEmissive,
						 bool bCompression)
{
	if (pImage == NULL)
		return -1;

	vtMaterial *pMat = new vtMaterial;
	pMat->SetTexture2D(pImage, 0, bCompression);
	pMat->SetCulling(bCulling);
	pMat->SetLighting(bLighting);
	pMat->SetTransparent(bTransp, bAdditive);
	pMat->SetAmbient(fAmbient, fAmbient, fAmbient);
	pMat->SetDiffuse(fDiffuse, fDiffuse, fDiffuse, fAlpha);
	pMat->SetEmission(fEmissive, fEmissive, fEmissive);

	return AppendMaterial(pMat);
}

/**
 * Load an image.
 */
osg::Image *LoadOsgImage(const char *fname)
{
	// safety checks
	if (fname == NULL || *fname == 0)
		return NULL;

	osg::Image *image = osgDB::readImageFile(fname);
	if (!image)
		return NULL;

	return image;
}

/**
 * Create and add a simple colored material.  This method takes diffuse
 * color and let you control several other aspects of the material.
 * Ambient color will be assumed to a be a dimmer shade of the supplied
 * diffuse color (diffuse / 4).
 *
 * \param diffuse The Diffuse color component of the material.
 *
 * \param bCulling  true to cull backfaces (only the front side
 *		of each polygon is rendered.)
 *
 * \param bLighting  true to "light" the material.  This means it will
 *		use the material's color values, and any active lights to
 *		determine the color of the drawn geometry.  If false, then
 *		only the material's diffuse color is used, and it is not affected
 *		by any lights.
 *
 * \param bWireframe True for a material which will render only the edges
 *		of polygons.
 *
 * \param fAlpha	Alpha value (opacity), ranges from 0 (completely
 *		transparent) to 1 (opaque).  Default is 1.  If transparency is
 *		not enabled, this value is ignored.
 *
 * \param fEmissive  Emmisive material value, ranges from 0 to 1 (default 0).
 *		If lighting is enabled, this value is added to the combined
 *		effect of each existing light.  This is useful for geometry which
 *		is brighter than the existing light level, such as illuminated
 *		objects at night.
 *
 * \return The index of the added material.
 */
int vtMaterialArray::AddRGBMaterial(const RGBf &diffuse,
				 bool bCulling, bool bLighting, bool bWireframe,
				 float fAlpha, float fEmissive)
{
	vtMaterial *pMat = new vtMaterial;
	pMat->SetCulling(bCulling);
	pMat->SetLighting(bLighting);
	pMat->SetWireframe(bWireframe);
	pMat->SetDiffuse(diffuse.r, diffuse.g, diffuse.b, fAlpha);
	pMat->SetSpecular(0.0f, 0.0f, 0.0f);
	pMat->SetAmbient(diffuse.r / 4, diffuse.g / 4, diffuse.b / 4);
	pMat->SetEmission(fEmissive, fEmissive, fEmissive);
	return AppendMaterial(pMat);
}

/**
 * Copies all the materials from one vtMaterialArray to another.
 *
 * The materials copied from the source are simply appending to this
 * array.
 */
void vtMaterialArray::CopyFrom(vtMaterialArray *pFrom)
{
	int num = pFrom->size();
	for (int i = 0; i < num; i++)
	{
		vtMaterial *pMat1 = pFrom->at(i).get();
		vtMaterial *pMat2 = new vtMaterial;
		pMat2->CopyFrom(pMat1);
		AppendMaterial(pMat2);
	}
}

void vtMaterialArray::RemoveMaterial(vtMaterial *pMat)
{
	for (size_t i = 0; i < size(); i++)
	{
		if (at(i).get() == pMat)
		{
			erase(begin() + i);
			return;
		}
	}
}

/**
 * Find a material in an array by looking for a specific diffuse color.
 *
 * \return The index of the material if found, otherwise -1.
 */
int vtMaterialArray::FindByDiffuse(const RGBAf &rgba) const
{
	for (uint i = 0; i < size(); i++)
	{
		if (rgba == at(i)->GetDiffuse())
			return i;
	}
	return -1;
}

/**
 * Find a material in an array by looking for a specific texture image.
 *
 * \return The index of the material if found, otherwise -1.
 */
int vtMaterialArray::FindByImage(const osg::Image *image) const
{
	for (uint i = 0; i < size(); i++)
	{
		const osg::Image *tex = at(i)->GetTextureImage();
		if (tex == image)
			return i;
	}
	return -1;
}

/**
 * Adds a material to this material array.
 *
 * \return The index of the material that was added.
 */
int vtMaterialArray::AppendMaterial(vtMaterial *pMat)
{
	// nothing special to do
	push_back(pMat);
	return (int) size() - 1;
}

