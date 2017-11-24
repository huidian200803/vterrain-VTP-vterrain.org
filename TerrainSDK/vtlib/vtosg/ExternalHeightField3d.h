//
// ExternalHeightField3d.h
//
// Copyright (c) 2010-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

#include "vtdata/HeightField.h"
#include <osg/Node>
#ifdef USE_OSGEARTH
#include <osgEarthUtil/ElevationManager>
#endif

class vtGeode;
class vtCRS;
namespace osgTerrain
{
	class Layer;
};
namespace osgSim
{
	class HeightAboveTerrain;
};

class vtExternalHeightField3d : public vtHeightField3d, public osg::Referenced
{
public:
	vtExternalHeightField3d(void);
	~vtExternalHeightField3d(void);

	bool Initialize(const char *external_data);
	osg::Node *CreateGeometry();

	vtCRS &GetCRS();
	const vtCRS &GetCRS() const;

	bool FindAltitudeOnEarth(const DPoint2 &p, float &fAltitude, bool bTrue = false) const;
	bool FindAltitudeAtPoint(const FPoint3 &p3, float &fAltitude, bool bTrue = false, int iCultureFlags = 0, FPoint3 *vNormal = NULL) const;
	bool CastRayToSurface(const FPoint3 &point, const FPoint3 &dir, FPoint3 &result) const;

private:
	osg::ref_ptr<osg::Node> m_pNode;
	osg::PagedLOD			*m_pLOD;
	osgTerrain::Layer		*m_pLayer;
	vtCRS			m_Projection;
	osg::Matrix				m_TransfromOSGModel2VTPWorld;
	osg::Matrix				m_TransformVTPWorld2OSGModel;
	osgSim::HeightAboveTerrain *m_pHat;
	bool					m_bOsgEarth;

#ifdef USE_OSGEARTH
	osg::ref_ptr<osgEarth::Util::ElevationManager> m_pElevationManager;
	float					m_ResolutionAtLevel0;
	float					m_CompromiseResolution;
#endif
};
