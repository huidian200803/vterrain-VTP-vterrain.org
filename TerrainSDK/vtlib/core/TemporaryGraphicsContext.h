//
// TemporaryGraphicsContext.h
//
//
// Copyright (c) 2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TEMPORYGRAPHICSCONTEXTH
#define TEMPORYGRAPHICSCONTEXTH

class vtTemporaryGraphicsContext
{
public:
	vtTemporaryGraphicsContext(void);
	virtual ~vtTemporaryGraphicsContext(void);
private:
	osg::ref_ptr<osg::GraphicsContext> m_pGraphicsContext;
	osg::ref_ptr<osg::GraphicsContext::Traits> m_pTraits;
};

#endif // TEMPORYGRAPHICSCONTEXTH

