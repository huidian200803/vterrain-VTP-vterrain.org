//
// SaveImageOSG.h
//
// Copyright (c) 2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

class CSaveImageOSG : public osg::Camera::DrawCallback
{
public:
	CSaveImageOSG(std::string& FileName, osg::Image *pImage, osg::Camera *pCamera);
	virtual void operator () (osg::RenderInfo& renderInfo) const;
	// Save an image using a FBO if available
	static bool SaveImage(std::string& FilePath, int Width, int Height);
protected:
	std::string m_FileName;
	osg::ref_ptr<osg::Image> m_pImage;
	osg::ref_ptr<osg::Camera> m_pCamera;
};
