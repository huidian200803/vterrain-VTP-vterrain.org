//
// TemporaryGraphicsContext.cpp
//
//
// Copyright (c) 2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "TemporaryGraphicsContext.h"

vtTemporaryGraphicsContext::vtTemporaryGraphicsContext(void)
{
	osg::ref_ptr<osg::GraphicsContext::Traits> m_pTraits = new osg::GraphicsContext::Traits;
	if (!m_pTraits.valid())
        return;
	m_pTraits->width = 1;
	m_pTraits->height = 1;
	m_pGraphicsContext = osg::GraphicsContext::createGraphicsContext(m_pTraits.get());
	if (!m_pGraphicsContext.valid())
        return;
	m_pGraphicsContext->realize();
	m_pGraphicsContext->makeCurrent();
}

vtTemporaryGraphicsContext::~vtTemporaryGraphicsContext(void)
{
    // There is probably no need for an explicit destructor
    // but this is easier to debug
	m_pGraphicsContext = NULL;
	m_pTraits = NULL;
}

