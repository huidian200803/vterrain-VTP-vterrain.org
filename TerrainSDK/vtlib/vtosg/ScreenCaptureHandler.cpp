//
// ScreenCaptureHandler.cpp
//
// Draw callback to capture current current 3d screen.
//
// Copyright (c) 2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//


#include "vtlib/vtlib.h"
#include "ScreenCaptureHandler.h"
#include <osgDB/WriteFile>

CScreenCaptureHandler::CScreenCaptureHandler(std::string& FileName) : m_FileName(FileName)
{
}

void CScreenCaptureHandler::operator () (osg::RenderInfo& renderInfo) const
{
	// Select the data source
	glReadBuffer(GL_BACK);

	// Create a new image object
	ImagePtr pImage = new osg::Image;

	// Read the pixels
    osg::GraphicsContext* gc = renderInfo.getState()->getGraphicsContext();
	int Width = gc->getTraits()->width;
	int Height = gc->getTraits()->height;
	pImage->readPixels(0, 0, Width, Height, GL_RGB, GL_UNSIGNED_BYTE);

	// Save the file
    osgDB::writeImageFile(*pImage, m_FileName);

	// Remove the callback
	renderInfo.getCurrentCamera()->setFinalDrawCallback(NULL);
}

void CScreenCaptureHandler::SetupScreenCapture(std::string& FileName)
{
	osgViewer::Viewer *pViewer = vtGetScene()->getViewer();

	// Ensure that all threads have finished rendering
	osg::Object::DataVariance dv = pViewer->getSceneData()->getDataVariance();
	pViewer->getSceneData()->setDataVariance(osg::Object::DYNAMIC);
	pViewer->frame();
	pViewer->getSceneData()->setDataVariance(dv);

	// Set draw callback
	pViewer->getCamera()->setFinalDrawCallback(new CScreenCaptureHandler(FileName));
}



