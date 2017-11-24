//
// SaveImageOSG.h
//
// Copyright (c) 2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "SaveImageOSG.h"
#include "core/TemporaryGraphicsContext.h"
#include <osgDB/WriteFile>
#include <osg/GLExtensions>
#include "vtdata/vtLog.h"

static const int MAX_HORIZONTAL_FBO_RESOLUTION = 4096;

static const GLint MAX_RENDERBUFFER_SIZE_EXT = 0x84E8;

// I cannot believe the following code proved so complex.
// If anyone can find a simpler way to do this in a
// multiprocessor environment then please replace this.
// It reminds me of a set of comments I once saw in the
// top of a piece of code (dates are approximately correct!)
// January 1972 - When I wrote this code only two people understood
// it. God and myself.
// January 1973 - Now God only knows!

struct MyDeleteCallback: public osg::Camera::DrawCallback
{
	MyDeleteCallback(osg::Camera *pCamera) : m_pCamera(pCamera) {}
	virtual void operator () (osg::RenderInfo& renderInfo) const
	{
		m_pCamera->setFinalDrawCallback(NULL);
		renderInfo.getCurrentCamera()->setPreDrawCallback(NULL);
	}
	osg::Camera *m_pCamera;
};

CSaveImageOSG::CSaveImageOSG(std::string& FileName, osg::Image* pImage, osg::Camera *pCamera)
	: m_FileName(FileName), m_pImage(pImage), m_pCamera(pCamera)
{
}

void CSaveImageOSG::operator () (osg::RenderInfo& renderInfo) const
{
	osgViewer::Viewer *pViewer = vtGetScene()->getViewer();

	// Save the file
    osgDB::writeImageFile(*m_pImage, m_FileName);

	// Remove the camera from the viewer
	pViewer->removeSlave(pViewer->findSlaveIndexForCamera(m_pCamera));
	// Take the camera out of the rendering context
	m_pCamera->setGraphicsContext(NULL);

	// Delay deleting the camera until the next frame
	pViewer->getCamera()->setPreDrawCallback(new MyDeleteCallback(m_pCamera.get()));
}

bool CSaveImageOSG::SaveImage(std::string& FilePath, int Width, int Height)
{
#if 0	// TODO update for new OSG
	osg::FBOExtensions* fbo_ext = osg::FBOExtensions::instance(vtGetScene()->GetGraphicsContext()->getState()->getContextID(), true);
    if (!(fbo_ext && fbo_ext->isSupported()))
		return false;

	GLint MaxRenderBufferSize = 3000;

    vtTemporaryGraphicsContext TempContext;

	glGetIntegerv(MAX_RENDERBUFFER_SIZE_EXT, &MaxRenderBufferSize);

	osgViewer::Viewer *pViewer = vtGetScene()->getViewer();

	osg::ref_ptr<osg::Camera> pCamera = new osg::Camera;
	if (!pCamera.valid())
		return false;

	ImagePtr pImage = new osg::Image;
	if (!pImage.valid())
	{
		pCamera = NULL;
		return false;
	}

	// Clone any global state
	// I am only going to mess about with viewport so SHALLOW_COPY should be OK.
	pCamera->setStateSet((osg::StateSet*)pViewer->getCamera()->getOrCreateStateSet()->clone(osg::CopyOp::SHALLOW_COPY));

	float AspectRatio = (float)Width / (float)Height;

	if (AspectRatio > 1.0)
	{
		if (Width > MaxRenderBufferSize)
		{
			Width = MaxRenderBufferSize;
			Height = MaxRenderBufferSize / AspectRatio;
		}
	}
	else
	{
		if (Height > MaxRenderBufferSize)
		{
			Height = MaxRenderBufferSize;
			Width = MaxRenderBufferSize * AspectRatio;
		}
	}

	VTLOG("CSaveImageOSG::SaveImage - Width %d Height %d Total %d\n", Width, Height, Width* Height);

	pImage->allocateImage(Width, Height, 1, GL_RGB, GL_UNSIGNED_BYTE);

	pCamera->setClearColor(pViewer->getCamera()->getClearColor());
	pCamera->setViewport(0, 0, Width, Height);
	pCamera->setRenderOrder(osg::Camera::POST_RENDER); // Have to set this to PRE_RENDER to stop
													  // annoying white semicircle flashing up
	pCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT, osg::Camera::FRAME_BUFFER);
	pCamera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);
	pCamera->attach(osg::Camera::COLOR_BUFFER, pImage.get());
	pCamera->setGraphicsContext(pViewer->getCamera()->getGraphicsContext());
	// Set draw callback - we cannot assume the camera will be rendered on this thread.
	pCamera->setFinalDrawCallback(new CSaveImageOSG(FilePath, pImage.get(), pCamera.get()));

	// Ensure that all threads have finished rendering. We probably do not need to do this
	// but it seems cleaner to me.
	osg::Object::DataVariance dv = pViewer->getSceneData()->getDataVariance();
	pViewer->getSceneData()->setDataVariance(osg::Object::DYNAMIC);
	pViewer->frame();
	// Restore the data variance ready for next frame
	pViewer->getSceneData()->setDataVariance(dv);

	pViewer->addSlave(pCamera.get());
	// The call to addSlave creates a default renderer with _compileOnNextDraw set to true
	// Because I am using the master cameras scene graph and graphics context I can assume that the display lists have
	// already been compiled. This is another way of avoiding the annoying white flash which is caused by the display
	// lists being recompiled into the main frame buffer with the projection and view matrices set to identity, rather than
	// into this cameras frame buffer object with the projection and view matrices set to something reasonable.
	((osgViewer::Renderer*)pCamera->getRenderer())->setCompileOnNextDraw(false);

	return true;
#else
	return false;
#endif
}

