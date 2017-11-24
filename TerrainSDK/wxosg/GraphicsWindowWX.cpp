//
// Name: GraphicsWindowsWX.cpp
//
// Copyright (c) 2011-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "GraphicsWindowWX.h"
#include "vtdata/vtLog.h"

GraphicsWindowWX::GraphicsWindowWX(wxGLCanvas *pCanvas, osg::DisplaySettings *pSettings)
{
	m_pCanvas = pCanvas;
	m_pGLContext = NULL;
	m_bValid = true;
	m_bIsRealized = false;

	wxPoint pos = m_pCanvas->GetPosition();
	wxSize  size = m_pCanvas->GetSize();

	// Set up traits to match the canvas
	_traits = new GraphicsContext::Traits(pSettings);
	_traits->x = pos.x;
	_traits->y = pos.y;
	_traits->width = size.x;
	_traits->height = size.y;

	// Set up a new context ID - I don't think we make use of this at the moment
	setState( new osg::State );
	getState()->setGraphicsContext(this);
    // if (_traits.valid() && _traits->sharedContext)
	if (_traits.valid() && _traits->sharedContext.valid())
	{
		// I left this code in just in case we want shared contexts in the future
		// they would need to be passed in and set up in the traits object above
        getState()->setContextID( _traits->sharedContext->getState()->getContextID() );
        incrementContextIDUsageCount( getState()->getContextID() );
    }
    else
        getState()->setContextID( osg::GraphicsContext::createNewContextID() );
}

GraphicsWindowWX::~GraphicsWindowWX()
{
	VTLOG1("1. ~GraphicsWindowWX()\n");
	delete m_pGLContext;
	m_pGLContext = NULL;
}

void GraphicsWindowWX::CloseOsgContext()
{
	VTLOG1("2. CloseOsgContext()\n");
	getEventQueue()->closeWindow(0);
	// Force handling of event before the idle loop can call frame();
	dynamic_cast<osgViewer::View*>(getCameras().front()->getView())->getViewerBase()->eventTraversal();
}

bool GraphicsWindowWX::makeCurrentImplementation()
{
	VTLOG("3. makeCurrentImplementation(canvas %p, context %p)\n",
		m_pCanvas, m_pGLContext);

	if (!m_pCanvas)
		return false;

	if (m_pCanvas->IsShownOnScreen())
		m_pCanvas->SetCurrent(*m_pGLContext);
	else
		VTLOG1(" Trying to make a context current for a window that isn't visible.\n");

	return true;
}

void GraphicsWindowWX::swapBuffersImplementation()
{
	if (m_pCanvas)
		m_pCanvas->SwapBuffers();
}

 bool GraphicsWindowWX::releaseContextImplementation()
{
	VTLOG("4. releaseContextImplementation(canvas %p, context %p)\n",
		m_pCanvas, m_pGLContext);

	if (!m_pCanvas)
		return false;

	m_pGLContext->ReleaseContext(*m_pCanvas);
	return true;
}

void GraphicsWindowWX::closeImplementation()
{
	VTLOG1("5. closeImplementation()\n");
	m_bValid = false;
}

bool GraphicsWindowWX::realizeImplementation()
{
	m_pGLContext = new LocalGLContext(m_pCanvas);
	VTLOG("6. realizeImplementation(canvas %p, context %p)\n",
		m_pCanvas, m_pGLContext);

	m_bIsRealized = true;
	return true;
}

