//
// VTBuilder ImageGLCanvas.h
//
// Copyright (c) 2001-2015 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

#include "vtdata/vtCRS.h"
#include "vtdata/MathTypes.h"

/////////////////
class vtMiniDatabuf;
class ImageGLCanvas;
class TilingOptions;

void WriteMiniImage(const vtString &fname, const TilingOptions &opts,
					uchar *rgb_bytes, vtMiniDatabuf &output_buf,
					int iUncompressedSize, ImageGLCanvas *pCanvas);
void CheckCompressionMethod(TilingOptions &opts);

#if USE_OPENGL
#include "wx/glcanvas.h"
void DoTextureCompress(uchar *rgb_bytes, vtMiniDatabuf &output_buf,
					   GLuint &iTex, bool bAlpha);

//
// ImageGLCanvas class:
//  We need to open an OpenGL context in order to do the texture compression,
//  so we may as well draw something into it, since it requires little extra
//  work, and provides interesting visual feedback to the user.
//
class ImageGLCanvas : public wxGLCanvas
{
public:
	ImageGLCanvas(wxWindow *parent, const wxWindowID id = -1,
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
		long style = 0, const wxString& name = wxT(""),
	  int* gl_attrib = NULL);
	~ImageGLCanvas();

	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnEraseBackground(wxEraseEvent& event) {}	// Do nothing, to avoid flashing.

	GLuint m_iTex;
    wxGLContext *m_context;
protected:
	DECLARE_EVENT_TABLE()
};

#endif	// USE_OPENGL

#if SUPPORT_SQUISH
void DoTextureSquish(uchar *rgb_bytes, vtMiniDatabuf &output_buf, bool bFast);
#endif
