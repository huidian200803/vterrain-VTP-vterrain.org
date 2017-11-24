//
// ImageSprite.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTLIB_IMAGESPRITE
#define VTLIB_IMAGESPRITE

/**
 * This class which contains a geometry with a single textured rectangle
 *  mesh.  It is particularly useful in conjunction with vtHUD, for
 *  superimposing a single image on the window.
 */
class vtImageSprite : public osg::Referenced
{
public:
	vtImageSprite();
	~vtImageSprite();
	bool Create(const char *szTextureName, bool bBlending = false);
	bool Create(osg::Image *pImage, bool bBlending = false);
	IPoint2 GetSize() const { return m_Size; }
	void SetPosition(float l, float t, float r, float b, float rot = 0.0f);
	void SetImage(osg::Image *image);
	vtGeode *GetGeode() const { return m_pGeode; }

protected:
	vtMaterialArrayPtr m_pMats;
	vtGeode *m_pGeode;
	vtMesh *m_pMesh;
	IPoint2 m_Size;
};

/**
 * vtSpriteSizer:
 *   An engine to put sprites in the right place, even when the window resizes.
 */
class vtSpriteSizer : public vtEngine
{
public:
	vtSpriteSizer(vtImageSprite *pSprite, float l, float t, float r, float b);

	void SetRect(const FRECT &rect) { m_rect = rect; }
	FRECT GetRect() { return m_rect; }
	FRECT GetWindowRect() { return m_window_rect; }
	FPoint2 GetWindowCenter();

	void SetRotation(float rot) { m_fRotation = rot; }
	float GetRotation() { return m_fRotation; }

	void OnWindowSize(int width, int height);

protected:
	FRECT m_rect;
	FRECT m_window_rect;
	float m_fRotation;
	osg::ref_ptr<vtImageSprite> m_pSprite;
};

#endif // VTLIB_IMAGESPRITE
