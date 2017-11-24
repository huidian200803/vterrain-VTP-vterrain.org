//
// MultiTexture.h
//
// Copyright (c) 2006-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTLIB_MULTI_TEXTURE_H
#define VTLIB_MULTI_TEXTURE_H

#include "vtdata/HeightField.h"

class vtMultiTexture : public osg::Referenced
{
public:
	vtMultiTexture();
	~vtMultiTexture();

	void Create(osg::Node *pSurfaceNode, vtImage *pImage,
		const FPoint2 &scale, const FPoint2 &offset,
		int iTextureUnit, int iTextureMode);
	void Enable(bool bEnable);
	bool IsEnabled();

	int	m_iTextureUnit;
#if VTLISPSM
	int	m_iMode;
#endif
	osg::Node *m_pNode;

	// These three are set/unset as stateset attributes:
	osg::ref_ptr<osg::Texture2D> m_pTexture;
	osg::ref_ptr<osg::TexGen> m_pTexgen;
	osg::ref_ptr<osg::TexEnv> m_pTexEnv;
};

#endif  // VTLIB_MULTI_TEXTURE_H

