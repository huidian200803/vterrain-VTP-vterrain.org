//
// ImageSprite.cpp
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "ImageSprite.h"


///////////////////////////////////////////////////////////////////////
// vtImageSprite

vtImageSprite::vtImageSprite()
{
	m_pMats = NULL;
	m_pGeode = NULL;
	m_pMesh = NULL;
}

vtImageSprite::~vtImageSprite()
{
	// Do not explicitly free geometry, if it was added to the scene.
}

/**
 * Create a vtImageSprite.
 *
 * \param szTextureName The filename of a texture image.
 * \param bBlending Set to true for alpha-blending, which produces smooth
 *		edges on transparent textures.
 */
bool vtImageSprite::Create(const char *szTextureName, bool bBlending)
{
	ImagePtr image = osgDB::readImageFile(szTextureName);
	if (!image.valid())
		return false;
	bool success = Create(image, bBlending);
	return success;
}

/**
 * Create a vtImageSprite.
 *
 * \param pImage A texture image.
 * \param bBlending Set to true for alpha-blending, which produces smooth
 *		edges on transparent textures.
 */
bool vtImageSprite::Create(osg::Image *pImage, bool bBlending)
{
	m_Size.x = GetWidth(pImage);
	m_Size.y = GetHeight(pImage);

	// set up material and geometry container
	m_pMats = new vtMaterialArray;
	m_pGeode = new vtGeode;
	m_pGeode->SetMaterials(m_pMats);

	m_pMats->AddTextureMaterial(pImage, false, false, bBlending);

	// default position of the mesh is just 0,0-1,1
	m_pMesh = new vtMesh(osg::PrimitiveSet::QUADS, VT_TexCoords, 4);
	m_pMesh->AddVertexUV(FPoint3(0,0,0), FPoint2(0,0));
	m_pMesh->AddVertexUV(FPoint3(1,0,0), FPoint2(1,0));
	m_pMesh->AddVertexUV(FPoint3(1,1,0), FPoint2(1,1));
	m_pMesh->AddVertexUV(FPoint3(0,1,0), FPoint2(0,1));
	m_pMesh->AddQuad(0, 1, 2, 3);
	m_pGeode->AddMesh(m_pMesh, 0);
	return true;
}

/**
 * Set the XY position of the sprite.  These are in world coordinates,
 *  unless this sprite is the child of a vtHUD, in which case they are
 *  pixel coordinates measured from the lower-left corner of the window.
 *
 * \param l Left.
 * \param t Top.
 * \param r Right.
 * \param b Bottom.
 * \param rot Rotation in radians.
 */
void vtImageSprite::SetPosition(float l, float t, float r, float b, float rot)
{
	if (!m_pMesh)	// safety check
		return;

	FPoint2 p[4];
	p[0].Set(l, b);
	p[1].Set(r, b);
	p[2].Set(r, t);
	p[3].Set(l, t);

	if (rot != 0.0f)
	{
		FPoint2 center((l+r)/2, (b+t)/2);
		for (int i = 0; i < 4; i++)
		{
			p[i] -= center;
			p[i].Rotate(rot);
			p[i] += center;
		}
	}

	for (int i = 0; i < 4; i++)
		m_pMesh->SetVtxPos(i, FPoint3(p[i].x, p[i].y, 0));

	m_pMesh->ReOptimize();
}

/**
 * Set (replace) the image on a sprite that has already been created.
 */
void vtImageSprite::SetImage(osg::Image *pImage)
{
	// Sprite must already be created
	if (!m_pMats)
		return;
	vtMaterial *mat = m_pMats->at(0).get();
	mat->SetTexture2D(pImage);
}


///////////////////////////////////////////////////////////////////////
// vtSpriteSizer
//   An engine to put sprites in the right place.
//
vtSpriteSizer::vtSpriteSizer(vtImageSprite *pSprite, float l, float t, float r, float b)
{
	m_pSprite = pSprite;
	m_fRotation = 0.0f;
	m_rect.SetRect(l, t, r, b);
	IPoint2 size = vtGetScene()->GetWindowSize();
	OnWindowSize(size.x, size.y);
}

void vtSpriteSizer::OnWindowSize(int width, int height)
{
	m_window_rect = m_rect;
	if (m_window_rect.left < 0) m_window_rect.left += width;
	if (m_window_rect.top < 0) m_window_rect.top += height;
	if (m_window_rect.right < 0) m_window_rect.right += width;
	if (m_window_rect.bottom < 0) m_window_rect.bottom += height;

	m_pSprite->SetPosition(m_window_rect.left, m_window_rect.top, m_window_rect.right, m_window_rect.bottom, m_fRotation);
}

FPoint2 vtSpriteSizer::GetWindowCenter()
{
	FPoint2 center = m_window_rect.Center();
	IPoint2 size = vtGetScene()->GetWindowSize();
	center.y = size.y - center.y;
	return center;
}

