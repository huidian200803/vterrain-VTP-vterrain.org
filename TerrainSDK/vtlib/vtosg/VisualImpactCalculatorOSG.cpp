//
// VisualImpactCalculatorOSG.cpp
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

#include "vtlib/vtlib.h"

#if VTP_VISUAL_IMPACT_CALCULATOR

#include "VisualImpactCalculatorOSG.h"
#include <osg/GLExtensions>
#include <osgDB/WriteFile>
#include "vtdata/HeightField.h"
#include "vtlib/core/TerrainScene.h"
#include "vtlib/core/Terrain.h"
#include <gdal_priv.h>
#include "vtdata/vtLog.h"

// I think this may need more work in a multiprocessor environment
// If threading is not CullDrawThreadPerContext then pViewer->frame may return before
// drawing has finished. I have put in a clumsy fix to prevent this.

// TBD
// This code is very clunky. It may be better to do the solid angle calculation in a fragment shader.
// However OpenGL does not guarantee that the depth value given to the fragment shader is consistent
// with the depth value computed in the fixed pipeline, only that it is consistent with other depth values
// passed to fragment shaders. The other problem is accessing the current depth buffer from a shader. My best
// guess on this at the moment is to map the depth buffer object in the draw callback and use memcpy to copy into another
// mapped buffer object attached to a texture which can then be read in the fragment shader.
// RFJ February 2012

static const char VISUAL_IMPACT_BIN_NAME[] = "Visual Impact Bin";
static const int VISUAL_IMPACT_BIN_NUMBER = 9999; // Set this number sufficiently high so that this bin will be rendered after everything else
static const int DEFAULT_VISUAL_IMPACT_RESOLUTION = 256;
// The following are accepted standard values for the field of view of normal binocular (non peripheral) vision
static const float DEFAULT_HUMAN_FOV_ASPECT_RATIO = 1.0;
static const float DEFAULT_HUMAN_FOV_DEGREES = 120;
static const float DEFAULT_HUMAN_FOV_SOLID_ANGLE = PIf; // 2pi(1 - cos(120/2)) = pi using formula for area of spherical cap
// The standard for foveal (high detail) vision is 2 degrees and 1.0 aspect. Which is a bit too small for us.
// The standard for peripheral vision is 180 degrees horizontal 130 degrees vertical. (Some sources go as far as 210
// horizontal but vision is very limited at the extremes.)
static const uint DEFAULT_GRAPHICS_CONTEXT = 0;

typedef struct CameraMask
{
    CameraMask(osg::Camera *pCamera, osg::Node::NodeMask NodeMask) : m_pCamera(pCamera), m_NodeMask(NodeMask) {}
    osg::Camera* m_pCamera;
    osg::Node::NodeMask m_NodeMask;
} CameraMask;

class MyRenderBinDrawCallback : public osgUtil::RenderBin::DrawCallback
{
public:
	MyRenderBinDrawCallback(CVisualImpactCalculatorOSG* pVisualImpactCalculator) :m_pVisualImpactCalculator(pVisualImpactCalculator) {}
	virtual void drawImplementation(osgUtil::RenderBin *pBin, osg::RenderInfo& renderInfo, osgUtil::RenderLeaf* &Previous);
protected:
    CVisualImpactCalculatorOSG* m_pVisualImpactCalculator;
};

class VisualImpactBin : public osgUtil::RenderBin
{
public:
    VisualImpactBin(CVisualImpactCalculatorOSG* pVisualImpactCalculator) : m_pVisualImpactCalculator(pVisualImpactCalculator)
    {
        setDrawCallback(new MyRenderBinDrawCallback(m_pVisualImpactCalculator));
    }
    virtual osg::Object* clone(const osg::CopyOp& copyop) const{ return new VisualImpactBin(m_pVisualImpactCalculator); }
private:
    CVisualImpactCalculatorOSG* m_pVisualImpactCalculator;
};

bool CVisualImpactCalculatorOSG::Initialise()
{
	osgViewer::Viewer *pViewer = vtGetScene()->getViewer();

    osgUtil::RenderBin::addRenderBinPrototype(VISUAL_IMPACT_BIN_NAME, new VisualImpactBin(this));

	m_ProjectionMatrix.makePerspective(DEFAULT_HUMAN_FOV_DEGREES, DEFAULT_HUMAN_FOV_ASPECT_RATIO, 10.0, 40000.0);

	osg::FBOExtensions* fbo_ext = osg::FBOExtensions::instance(DEFAULT_GRAPHICS_CONTEXT, true);
//	if ((fbo_ext && fbo_ext->isSupported()) || osg::isGLExtensionSupported(DEFAULT_GRAPHICS_CONTEXT, "ARB_render_texture"))
	if (fbo_ext && fbo_ext->isSupported())
		m_bUsingLiveFrameBuffer = false;
	else
		m_bUsingLiveFrameBuffer = true;

	m_pIntermediateImage = new osg::Image;
	if (!m_pIntermediateImage.valid())
	{
		VTLOG("CVisualImpactCalculatorOSG::Implementation - Cannot create intermediate image\n");
		return false;
	}
	m_pIntermediateImage->allocateImage(DEFAULT_VISUAL_IMPACT_RESOLUTION,
							DEFAULT_VISUAL_IMPACT_RESOLUTION,
							1,
							GL_DEPTH_COMPONENT,
							GL_FLOAT);
	if (!m_pIntermediateImage->valid())
	{
		VTLOG("CVisualImpactCalculatorOSG::Implementation - Cannot allocate intermediate image\n");
		return false;
	}

	// Even though the camera node knows that you have attached the image to the depth buffer
	// it does not set this up correctly for you. There is no way to set the dataType, so
	// preallocation of the data is easiest
	m_pFinalImage= new osg::Image;
	if (!m_pFinalImage.valid())
	{
		VTLOG("CVisualImpactCalculatorOSG::Implementation - Cannot create final image\n");
		return false;
	}
	m_pFinalImage->allocateImage(DEFAULT_VISUAL_IMPACT_RESOLUTION,
							DEFAULT_VISUAL_IMPACT_RESOLUTION,
							1,
							GL_DEPTH_COMPONENT,
							GL_FLOAT);
	if (!m_pFinalImage->valid())
	{
		VTLOG("CVisualImpactCalculatorOSG::Implementation - Cannot allocate final image\n");
		return false;
	}

	m_pVisualImpactCamera = new osg::Camera;
	if (!m_pVisualImpactCamera.valid())
	{
		VTLOG("CVisualImpactCalculatorOSG::Implementation - Cannot create visual impact camera\n");
		return false;
	}
	m_pVisualImpactCamera->setName("Visual impact calculator camera");
	m_pVisualImpactCamera->setViewport(0, 0, DEFAULT_VISUAL_IMPACT_RESOLUTION, DEFAULT_VISUAL_IMPACT_RESOLUTION);
	m_pVisualImpactCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT, osg::Camera::FRAME_BUFFER);
	m_pVisualImpactCamera->setRenderOrder(osg::Camera::PRE_RENDER);
	m_pVisualImpactCamera->attach(osg::Camera::DEPTH_BUFFER, GL_DEPTH_COMPONENT); // Force a renderBuffer
//	m_pVisualImpactCamera->attach(osg::Camera::COLOR_BUFFER, GL_UNSIGNED_BYTE); // Force a renderBuffer
	m_pVisualImpactCamera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);
	m_pVisualImpactCamera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
	m_pVisualImpactCamera->setProjectionMatrix(m_ProjectionMatrix);
	m_pVisualImpactCamera->setGraphicsContext(pViewer->getCamera()->getGraphicsContext());
	m_pVisualImpactCamera->setNodeMask(0); // Initially disabled
	pViewer->addSlave(m_pVisualImpactCamera.get());

	m_bInitialised = true;
	return true;
}


// The following pair of functions assume that the existing RenderBinMode is INHERIT_RENDERBIN_DETAILS (default)
void CVisualImpactCalculatorOSG::AddVisualImpactContributor(osg::Node *pOsgNode)
{
	if (NULL != pOsgNode)
	    m_VisualImpactContributors.insert(m_VisualImpactContributors.begin(), pOsgNode);
}

void CVisualImpactCalculatorOSG::RemoveVisualImpactContributor(osg::Node *pOsgNode)
{
	if (NULL != pOsgNode)
	    m_VisualImpactContributors.erase(pOsgNode);
}

void CVisualImpactCalculatorOSG::SetVisualImpactTarget(const FPoint3 Target)
{
	m_Target = Target;
}

const FPoint3& CVisualImpactCalculatorOSG::GetVisualImpactTarget() const
{
	return m_Target;
}


bool CVisualImpactCalculatorOSG::UsingLiveFrameBuffer()
{
	return m_bUsingLiveFrameBuffer;
}

float CVisualImpactCalculatorOSG::Calculate()
{
	m_ViewMatrix = dynamic_cast<osgViewer::Renderer*>(vtGetScene()->getViewer()->getCamera()->getRenderer())->getSceneView(0)->getViewMatrix();
	return Implementation(true);
}

bool CVisualImpactCalculatorOSG::Plot(GDALRasterBand *pRasterBand, float fScaleFactor, double dXSampleInterval, double dYSampleInterval, bool progress_callback(int))
{
	if (0.0f == Implementation(false, pRasterBand, fScaleFactor, dXSampleInterval, dYSampleInterval, progress_callback))
		return true;
	else
		return false;
}

float CVisualImpactCalculatorOSG::Implementation(bool bOneOffMode, GDALRasterBand *pRasterBand, float fScaleFactor, double dXSampleInterval, double dYSampleInterval, bool progress_callback(int))
{
	if (!m_bInitialised)
		return 0.0;

	osgViewer::Viewer *pViewer = vtGetScene()->getViewer();

	osgViewer::Viewer::Cameras ActiveCameras;
	std::vector<CameraMask> NodeMasks;

    pViewer->getCameras(ActiveCameras, true);


    // Stop any other cameras rendering the scene
    for (osgViewer::Viewer::Cameras::iterator itr = ActiveCameras.begin(); itr != ActiveCameras.end(); ++itr)
    {
        if (*itr != m_pVisualImpactCamera.get())
        {
            NodeMasks.push_back(CameraMask(*itr, (*itr)->getNodeMask()));
            (*itr)->setNodeMask(0);
        }
    }

   // Set up the render bins
    for (VisualImpactContributors::iterator itr = m_VisualImpactContributors.begin(); itr != m_VisualImpactContributors.end(); itr++)
		(*itr)->getOrCreateStateSet()->setRenderBinDetails(VISUAL_IMPACT_BIN_NUMBER, VISUAL_IMPACT_BIN_NAME);

    // Pick up the current main scene camera state
    osg::StateSet* pStateSet = new osg::StateSet(*ActiveCameras[0]->getOrCreateStateSet(), osg::CopyOp::DEEP_COPY_ALL);
    pStateSet->setAttribute(m_pVisualImpactCamera->getViewport());
    m_pVisualImpactCamera->setStateSet(pStateSet);
    m_pVisualImpactCamera->setClearColor(ActiveCameras[0]->getClearColor());
    // Enable the visual impact camera
    m_pVisualImpactCamera->setNodeMask(0xffffffff);

	if (bOneOffMode)
	{
        m_pVisualImpactCamera->setViewMatrix(m_ViewMatrix);

        pViewer->frame();

        // Disable the visual impact camera
        m_pVisualImpactCamera->setNodeMask(0);

        for(std::vector<CameraMask>::iterator itr = NodeMasks.begin(); itr != NodeMasks.end(); ++itr)
            itr->m_pCamera->setNodeMask(itr->m_NodeMask);

        for (VisualImpactContributors::iterator itr = m_VisualImpactContributors.begin(); itr != m_VisualImpactContributors.end(); itr++)
        {
            osg::StateSet *pStateSet = (*itr)->getOrCreateStateSet();
            pStateSet->setRenderBinMode(osg::StateSet::INHERIT_RENDERBIN_DETAILS);
            pStateSet->setRenderingHint(osg::StateSet::DEFAULT_BIN);
        }

		return InnerImplementation();
	}
	else
	{
		DPoint2 CameraOrigin;
		DPoint2 CurrentCamera;
		vtHeightField3d *pHeightField = vtGetTS()->GetCurrentTerrain()->GetHeightField();
		DRECT EarthExtents = pHeightField->GetEarthExtents();

		CameraOrigin.x = EarthExtents.left;
		CameraOrigin.y = EarthExtents.bottom;

		int iCurrentY = 0;
		int iXsize = (int)((EarthExtents.right - EarthExtents.left)/dXSampleInterval);
		int iYsize = (int)((EarthExtents.top - EarthExtents.bottom)/dYSampleInterval);
		int iTotalProgress = iXsize * iYsize;

#ifdef _DEBUG
		int iBlockSizeX, iBlockSizeY;
		pRasterBand->GetBlockSize(&iBlockSizeX, &iBlockSizeY);
#endif

		for (CurrentCamera.y = CameraOrigin.y; CurrentCamera.y < EarthExtents.top; CurrentCamera.y += dYSampleInterval)
		{
			// Process horizontal scanline
			int iCurrentX = 0;
			for (CurrentCamera.x = CameraOrigin.x; CurrentCamera.x < EarthExtents.right; CurrentCamera.x += dXSampleInterval)
			{
				FPoint3 CameraTranslate;

				pHeightField->ConvertEarthToSurfacePoint(CurrentCamera, CameraTranslate);
				m_pVisualImpactCamera->setViewMatrixAsLookAt(v2s(CameraTranslate), v2s(m_Target), osg::Vec3(0.0, 1.0, 0.0));

				pViewer->frame();

				float fFactor = InnerImplementation();

				pRasterBand->RasterIO(GF_Write, iCurrentX, iYsize - iCurrentY - 1, 1, 1, &fFactor, 1, 1, GDT_Float32, 0, 0);

				iCurrentX++;
				if ((*progress_callback)(100 * (iCurrentY * iXsize + iCurrentX) / iTotalProgress))
				{
                    // Disable the visual impact camera
                    m_pVisualImpactCamera->setNodeMask(0);

                    for(std::vector<CameraMask>::iterator itr = NodeMasks.begin(); itr != NodeMasks.end(); ++itr)
                        itr->m_pCamera->setNodeMask(itr->m_NodeMask);

                    for (VisualImpactContributors::iterator itr = m_VisualImpactContributors.begin(); itr != m_VisualImpactContributors.end(); itr++)
                    {
                        osg::StateSet *pStateSet = (*itr)->getOrCreateStateSet();
                        pStateSet->setRenderBinMode(osg::StateSet::INHERIT_RENDERBIN_DETAILS);
                        pStateSet->setRenderingHint(osg::StateSet::DEFAULT_BIN);
                    }

					VTLOG("CVisualImpactCalculatorOSG::Implementation - Cancelled by user\n");
					return -1.0f;
				}
			}
			iCurrentY++;
		}
        // Disable the visual impact camera
        m_pVisualImpactCamera->setNodeMask(0);

        for(std::vector<CameraMask>::iterator itr = NodeMasks.begin(); itr != NodeMasks.end(); ++itr)
            itr->m_pCamera->setNodeMask(itr->m_NodeMask);

        for (VisualImpactContributors::iterator itr = m_VisualImpactContributors.begin(); itr != m_VisualImpactContributors.end(); itr++)
        {
            osg::StateSet *pStateSet = (*itr)->getOrCreateStateSet();
            pStateSet->setRenderBinMode(osg::StateSet::INHERIT_RENDERBIN_DETAILS);
            pStateSet->setRenderingHint(osg::StateSet::DEFAULT_BIN);
        }

		return 0.0f;
	}
}

float CVisualImpactCalculatorOSG::InnerImplementation() const
{
	float fSolidAngle = 0.0f;
	// Compute the PW matrix
	osg::Matrixd PWmatrix(osg::Matrixd(m_pVisualImpactCamera->getProjectionMatrix()));
	PWmatrix.postMult(osg::Matrixd(m_pVisualImpactCamera->getViewport()->computeWindowMatrix()));

	// Compute the inverse PW matrix
	osg::Matrixd InversePWmatrix;
	InversePWmatrix.invert(PWmatrix);

	// Eye coordinates of pixel for solid angle calculation
	osg::Vec3d BottomLeft, BottomRight, TopLeft, TopRight;

	// Dihedral angles for solid angle calculation
	double d1, d2, d3, d4, d5, d6;
	// Normals for solid angle computation
	osg::Vec3d BLBR, BLTR, BRTR, BRBL, TRBL, TRBR, BLTL, TRTL, TLBL, TLTR;

#define DUMP_VIA_IMAGE
#ifdef DUMP_VIA_IMAGE
	ImagePtr pDebugImage = new osg::Image;
	pDebugImage->allocateImage(DEFAULT_VISUAL_IMPACT_RESOLUTION,
							DEFAULT_VISUAL_IMPACT_RESOLUTION,
							1,
							GL_RGB,
							GL_UNSIGNED_BYTE);
#endif

	// Every pixel that has been written to by a contributing geometry should have a different depth value to
	// the one in the intermediate buffer.
	// For each one I find compute the solid angle of that patch using Gauss Bonnett and add to the sum
	float* pFinalBuffer = (float*)m_pFinalImage->data();
	float* pIntermediateBuffer = (float*)m_pIntermediateImage->data();
	int x, y;
#ifdef _DEBUG
	uint Hits = 0;
#endif
	if (NULL != pIntermediateBuffer) // Buffer will be NULL if our bin has not been rendered
	{
		for (x = 0; x < DEFAULT_VISUAL_IMPACT_RESOLUTION; x++)
			for (y = 0; y < DEFAULT_VISUAL_IMPACT_RESOLUTION; y++)
				if (*(GLfloat*)m_pFinalImage->data(x, y) != *(GLfloat*)m_pIntermediateImage->data(x, y))
			{
#ifdef DUMP_VIA_IMAGE
			    *pDebugImage->data(x, y) = 0xff;
			    *(pDebugImage->data(x, y) + 1) = 0x00;
			    *(pDebugImage->data(x, y) + 2) = 0x00;
#endif
				// Get patch corners in eye coordinates
				BottomLeft = osg::Vec3d(x, y, 0.0f) * InversePWmatrix;
				BottomRight = osg::Vec3d(x + 1, y, 0.0f) * InversePWmatrix;
				TopLeft = osg::Vec3d(x, y + 1, 0.0f) * InversePWmatrix;
				TopRight = osg::Vec3d(x + 1, y + 1, 0.0f) * InversePWmatrix;
				// Split the rectangle into two triangles calculate the dihedral angles
				// First get the normals to the planes
				BLBR = BottomLeft ^ BottomRight; // Normal to plane containing BL BR Origin
				BLTR = BottomLeft ^ TopRight; // Normal to plane containing BL TR Origin
				BRTR = BottomRight ^ TopRight; // Normal to plane containing BR TR Origin
				BRBL = BottomRight ^ BottomLeft; // Normal to plane containing BR BL Origin
				TRBL = TopRight ^ BottomLeft; // Normal to plane containing TR BL Origin
				TRBR = TopRight ^ BottomRight; // Normal to plane containing TR BR Origin
				BLTL = BottomLeft ^ TopLeft; // Normal to plane containing BL TL Origin
				TRTL = TopRight ^ TopLeft; // Normal to plane containing TR TL Origin
				TLBL = TopLeft ^ BottomLeft; // Normal to plane containing TL BL Origin
				TLTR = TopLeft ^ TopRight; // Normal to plane containing TL TR Origin
				BLBR.normalize();
				BLTR.normalize();
				BRTR.normalize();
				BRBL.normalize();
				TRBL.normalize();
				TRBR.normalize();
				BLTL.normalize();
				TRTL.normalize();
				TLBL.normalize();
				TLTR.normalize();
				// Dihedral angles (angles between planes)
				d1 = acos(BLBR * BLTR);
				d2 = acos(BRTR * BRBL);
				d3 = acos(TRBL * TRBR);
				d4 = acos(BLTR * BLTL);
				d5 = acos(TRTL * TRBL);
				d6 = acos(TLBL * TLTR);
				// Gauss Bonnet gives spherical excess which is the solid angle of the patch
				fSolidAngle += d1 + d2 + d3 + d4 + d5 + d6 - PI2d;
#ifdef _DEBUG
				Hits++;
#endif
			}
#ifdef DUMP_VIA_IMAGE
			else
			{
			    *pDebugImage->data(x, y) = 0xff;
			    *(pDebugImage->data(x, y) + 1) = 0xff;
			    *(pDebugImage->data(x, y) + 2) = 0xff;
			}
#endif
	}

#ifdef DUMP_VIA_IMAGE
	osgDB::writeImageFile(*pDebugImage, "DebugDepthBufferImage.jpeg");
#endif

#ifdef _DEBUG
	float HitPercentage;
	HitPercentage = (float)Hits * 100.0f / (float)(DEFAULT_VISUAL_IMPACT_RESOLUTION * DEFAULT_VISUAL_IMPACT_RESOLUTION);
#endif

	return 100 * fSolidAngle / DEFAULT_HUMAN_FOV_SOLID_ANGLE;
}

void MyRenderBinDrawCallback::drawImplementation(osgUtil::RenderBin *pBin, osg::RenderInfo& renderInfo, osgUtil::RenderLeaf* &Previous)
{
    m_pVisualImpactCalculator->GetIntermediateImage()->readPixels(0, 0, DEFAULT_VISUAL_IMPACT_RESOLUTION, DEFAULT_VISUAL_IMPACT_RESOLUTION, GL_DEPTH_COMPONENT, GL_FLOAT);
	pBin->drawImplementation(renderInfo, Previous);
	m_pVisualImpactCalculator->GetFinalImage()->readPixels(0, 0, DEFAULT_VISUAL_IMPACT_RESOLUTION, DEFAULT_VISUAL_IMPACT_RESOLUTION, GL_DEPTH_COMPONENT, GL_FLOAT);
}

#endif // VTP_VISUAL_IMPACT_CALCULATOR
