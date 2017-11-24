//
// GLTexture.cpp
//

#include "wx/wxprec.h"

#include "GLTexture.h"
#include "vtdata/vtDIB.h"
#include "ScaledView.h"

GLTexture::GLTexture()
{
	m_iTextureId = 0;
}

void GLTexture::CreateFromBitmap(vtDIB *pDib)
{
	// allocate a texture name
	glGenTextures(1, &m_iTextureId);
	glBindTexture(GL_TEXTURE_2D, m_iTextureId);

	const IPoint2 size = pDib->GetSize();

	if (pDib->GetDepth() == 8)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, size.x, size.y, 0, GL_LUMINANCE,
			GL_UNSIGNED_BYTE, pDib->GetData());
	if (pDib->GetDepth() == 24)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB,
			GL_UNSIGNED_BYTE, pDib->GetData());
	else if (pDib->GetDepth() == 32)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, pDib->GetData());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}

void GLTexture::CopyImageFromBitmap(vtDIB *pDib)
{
	glBindTexture(GL_TEXTURE_2D, m_iTextureId);

	const IPoint2 size = pDib->GetSize();
	int depth = pDib->GetDepth();
	switch (depth)
	{
	case 8:
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y, GL_LUMINANCE,
			GL_UNSIGNED_BYTE, pDib->GetData());
		break;
	case 24:
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y, GL_RGB,
			GL_UNSIGNED_BYTE, pDib->GetData());
		break;
	case 32:
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y, GL_RGBA,
			GL_UNSIGNED_BYTE, pDib->GetData());
		break;
	}
}

void GLTexture::Draw(vtScaledView *pView, const DRECT &rect)
{
	// Enable textures
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_iTextureId);
	glColor3f(1.0f, 1.0f, 1.0f);

	glAlphaFunc(GL_GREATER, 0.5);
	glEnable(GL_ALPHA_TEST);

	glBegin(GL_QUADS);
	glTexCoord2d(0.0, 0.0);	pView->SendVertex(rect.left, rect.bottom);
	glTexCoord2d(1.0, 0.0);	pView->SendVertex(rect.right, rect.bottom);
	glTexCoord2d(1.0, 1.0);	pView->SendVertex(rect.right, rect.top);
	glTexCoord2d(0.0, 1.0);	pView->SendVertex(rect.left, rect.top);
	glEnd();

	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
}

