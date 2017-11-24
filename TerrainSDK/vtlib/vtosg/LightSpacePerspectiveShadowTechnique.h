//
// LightSpacePerspectiveShadowTechnique.h
//
// It's simple because it is.
// It's interim because it should be replaced with something better.
// It's a shadow technique.
//
// Copyright (c) 2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_LIGHTSPACEPERSPECTIVESHADOWTECHNIQUEH
#define VTOSG_LIGHTSPACEPERSPECTIVESHADOWTECHNIQUEH

#include <osg/Version>
#if (OSG_VERSION_MAJOR==2 && OSG_VERSION_MINOR>6) || OSG_VERSION_MAJOR>2

#include <osgShadow/LightSpacePerspectiveShadowMap>

// Define one of these to select the area for which shadows will be calculated
#define VTP_LISPSM_DRAW_BOUNDS 
//#define VTP_LISPSM_CULL_BOUNDS 
//#define VTP_LISPSM_VIEW_BOUNDS

/*
The following methods from osgShadow::MinimalShadow are available to play with
I haven't got a clue what they do!
void setModellingSpaceToWorldTransform( const osg::Matrix & modellingSpaceToWorld )
void setMaxFarPlane( float maxFarPlane )
void setMinLightMargin( float minLightMargin )
void setShadowReceivingCoarseBoundAccuracy( ShadowReceivingCoarseBoundAccuracy accuracy )
*/

#if defined(VTP_LISPSM_DRAW_BOUNDS)
class CLightSpacePerspectiveShadowTechnique : public osgShadow::LightSpacePerspectiveShadowMapDB
#elif defined(VTP_LISPSM_CULL_BOUNDS)
class CLightSpacePerspectiveShadowTechnique : public osgShadow::LightSpacePerspectiveShadowMapCB
#elif defined(VTP_LISPSM_VIEW_BOUNDS)
class CLightSpacePerspectiveShadowTechnique : public osgShadow::LightSpacePerspectiveShadowMapVB
#endif
{    
public:
	/** Convenient typedef used in definition of ViewData struct and methods */
	typedef CLightSpacePerspectiveShadowTechnique                     ThisClass;
	/** Convenient typedef used in definition of ViewData struct and methods */
#if defined(VTP_LISPSM_DRAW_BOUNDS)
	typedef osgShadow::LightSpacePerspectiveShadowMapDB BaseClass;
#elif defined(VTP_LISPSM_CULL_BOUNDS)
	typedef osgShadow::LightSpacePerspectiveShadowMapCB BaseClass;
#elif defined(VTP_LISPSM_VIEW_BOUNDS)
	typedef osgShadow::LightSpacePerspectiveShadowMapVB BaseClass;
#endif

	CLightSpacePerspectiveShadowTechnique();

	void InitJittering(osg::StateSet *pStateSet);
	uint m_JitterTextureUnit;
	void AddAdditionalTerrainTextureUnit(const uint Unit, const uint Mode);
	void RemoveAdditionalTerrainTextureUnit(const uint Unit);
	void RemoveAllAdditionalTerrainTextureUnits();

protected:
	std::string GenerateFragmentShaderSource();

	struct ViewData: public BaseClass::ViewData
	{
		virtual void init( ThisClass * st, osgUtil::CullVisitor * cv );           
	};
	std::map<uint, uint>	m_AdditionalTerrainTextureUnits;

	// This macro is required if you override ViewData and ViewData::init
	// It generates virtual stub function in the Base class which 
	// calls associated ViewData::init. 
	META_ViewDependentShadowTechniqueData( ThisClass, ThisClass::ViewData )
};

#endif

#endif	// VTOSG_LIGHTSPACEPERSPECTIVESHADOWTECHNIQUEH
