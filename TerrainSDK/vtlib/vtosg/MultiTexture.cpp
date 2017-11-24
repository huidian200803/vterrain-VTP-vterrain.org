//
// MultiTexture.cpp
//
// Copyright (c) 2006-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "MultiTexture.h"


vtMultiTexture::vtMultiTexture()
{
	m_pNode = NULL;
}

vtMultiTexture::~vtMultiTexture()
{
	m_pTexture = NULL;
}

void vtMultiTexture::Create(osg::Node *pSurfaceNode, vtImage *pImage,
	const FPoint2 &scale, const FPoint2 &offset,
	int iTextureUnit, int iTextureMode)
{
	m_pNode = pSurfaceNode;
	m_iTextureUnit = iTextureUnit;
#if VTLISPSM
	m_iMode = iTextureMode;
#endif

	// Get a stateset to work with
	osg::ref_ptr<osg::StateSet> pStateSet = pSurfaceNode->getOrCreateStateSet();

	// Setup and enable the texture
	m_pTexture = new osg::Texture2D(pImage);
	m_pTexture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
	m_pTexture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
	m_pTexture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_BORDER);
	m_pTexture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_BORDER);
	pStateSet->setTextureAttributeAndModes(iTextureUnit, m_pTexture.get(), osg::StateAttribute::ON);

	// Set up the texgen
	m_pTexgen = new osg::TexGen;
	m_pTexgen->setMode(osg::TexGen::EYE_LINEAR);
	m_pTexgen->setPlane(osg::TexGen::S, osg::Vec4(scale.x, 0.0f, 0.0f, -offset.x));
	m_pTexgen->setPlane(osg::TexGen::T, osg::Vec4(0.0f, 0.0f, scale.y, -offset.y));
	pStateSet->setTextureAttributeAndModes(iTextureUnit, m_pTexgen.get(), osg::StateAttribute::ON);
	pStateSet->setTextureMode(iTextureUnit, GL_TEXTURE_GEN_S,  osg::StateAttribute::ON);
	pStateSet->setTextureMode(iTextureUnit, GL_TEXTURE_GEN_T,  osg::StateAttribute::ON);

	// Set up the texenv
	osg::TexEnv::Mode mode;
	if (iTextureMode == GL_ADD) mode = osg::TexEnv::ADD;
	if (iTextureMode == GL_BLEND) mode = osg::TexEnv::BLEND;
	if (iTextureMode == GL_REPLACE) mode = osg::TexEnv::REPLACE;
	if (iTextureMode == GL_MODULATE) mode = osg::TexEnv::MODULATE;
	if (iTextureMode == GL_DECAL) mode = osg::TexEnv::DECAL;
	m_pTexEnv = new osg::TexEnv(mode);
	pStateSet->setTextureAttributeAndModes(iTextureUnit, m_pTexEnv.get(), osg::StateAttribute::ON);

	// If texture mode is DECAL and internal texture format does not have an alpha channel then
	// force the format to be converted on texture binding
	if ((GL_DECAL == iTextureMode) &&
		(pImage->getInternalTextureFormat() != GL_RGBA))
	{
		// Force the internal format to RGBA
		pImage->setInternalTextureFormat(GL_RGBA);
	}
}

void vtMultiTexture::Enable(bool bEnable)
{
	osg::ref_ptr<osg::StateSet> pStateSet = m_pNode->getOrCreateStateSet();
	if (bEnable)
	{
		pStateSet->setTextureAttributeAndModes(m_iTextureUnit, m_pTexture.get(), osg::StateAttribute::ON);
		pStateSet->setTextureAttributeAndModes(m_iTextureUnit, m_pTexgen.get(), osg::StateAttribute::ON);
		pStateSet->setTextureAttributeAndModes(m_iTextureUnit, m_pTexEnv.get(), osg::StateAttribute::ON);
	}
	else
	{
		pStateSet->setTextureAttributeAndModes(m_iTextureUnit, m_pTexture.get(), osg::StateAttribute::OFF);
		pStateSet->setTextureAttributeAndModes(m_iTextureUnit, m_pTexgen.get(), osg::StateAttribute::OFF);
		pStateSet->setTextureAttributeAndModes(m_iTextureUnit, m_pTexEnv.get(), osg::StateAttribute::OFF);
	}
}

bool vtMultiTexture::IsEnabled()
{
	osg::ref_ptr<osg::StateSet> pStateSet = m_pNode->getOrCreateStateSet();
	osg::StateAttribute *attr = pStateSet->getTextureAttribute(m_iTextureUnit, osg::StateAttribute::TEXTURE);
	return (attr != NULL);
}

