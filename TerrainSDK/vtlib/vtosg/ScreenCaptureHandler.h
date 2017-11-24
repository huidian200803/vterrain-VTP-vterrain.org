//
// ScreenCaptureHandler.h
//
// Copyright (c) 2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

/** Callback which will be added to a viewer's camera to do the actual screen capture. */
class CScreenCaptureHandler : public osg::Camera::DrawCallback
{
public:

	CScreenCaptureHandler(std::string& FileName);

	virtual void operator () (osg::RenderInfo& renderInfo) const;

	static void SetupScreenCapture(std::string& FileName);

protected:
	std::string m_FileName;
};


