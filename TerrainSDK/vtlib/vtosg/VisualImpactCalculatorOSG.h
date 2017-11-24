#pragma once
//
// VisualImpactCalculatorOSG.h
//
// Framework for calculating visual impact of selected geometry in a scene.
// This code calculates the position of all the pixels that a particular set of
// drawing primitives has contributed to the frame buffer at the end of a rendering pass.
// i.e. All the pixels of a particular geometry or geometries that are visible in the current view.
// It then summates the solid angle that these pixels represent using Gauss Bonnett.
// This data can be used to give a quantatative assessment of the visual impact of the geometry.
//
// Contributed by Roger James (roger@beardandsandals.co.uk) and Virtual Outlooks Limited.
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//


class GDALRasterBand;

class CVisualImpactCalculatorOSG
{
public:
	CVisualImpactCalculatorOSG() : m_bInitialised(false) {};
	void AddVisualImpactContributor(osg::Node *pOsgNode);
	void RemoveVisualImpactContributor(osg::Node *pOsgNode);
	void SetVisualImpactTarget(const FPoint3 Target);
	const FPoint3& GetVisualImpactTarget() const;
	osg::Image* GetIntermediateImage() { return m_pIntermediateImage.get(); }
	osg::Image* GetFinalImage() { return m_pFinalImage.get(); }
	bool UsingLiveFrameBuffer();
	float Calculate();
	bool Plot(GDALRasterBand *pRasterBand, float fScaleFactor, double dXSampleInterval, double dYSampleInterval, bool progress_callback(int));
	bool Initialise();

protected:
	float Implementation(bool bOneOffMode, GDALRasterBand *pRasterBand = NULL, float fScaleFactor = 1.0f, double dXSampleInterval = 1.0f, double dYSampleInterval = 1.0f, bool progress_callback(int) = NULL);
	float InnerImplementation() const;
	FPoint3 m_Target;
	osg::Matrix m_ViewMatrix;
	mutable osg::Matrix m_ProjectionMatrix;
	mutable bool m_bUsingLiveFrameBuffer;
	mutable bool m_bInitialised;
	ImagePtr m_pIntermediateImage;
	ImagePtr m_pFinalImage;
	osg::ref_ptr<osg::Camera> m_pVisualImpactCamera;
	typedef std::set<osg::Node*> VisualImpactContributors;
	VisualImpactContributors m_VisualImpactContributors;
};
