//
// GLTexture.h
//

#pragma once

#include "vtdata/MathTypes.h"

class vtDIB;
class vtScaledView;

class GLTexture
{
public:
	GLTexture();

	void CreateFromBitmap(vtDIB *pDib);
	void CopyImageFromBitmap(vtDIB *pDib);
	void Draw(vtScaledView *pView, const DRECT &rect);
protected:
	unsigned int m_iTextureId;
};

