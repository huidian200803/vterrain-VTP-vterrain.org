#include "vtlib/vtlib.h"
#include "vtdata/vtString.h"
#include <osgDB/ReadFile>
#include <osg/PagedLOD>
#include <osgTerrain/TerrainTile>
#include <osgSim/HeightAboveTerrain>
#include "ExternalHeightField3d.h"
#ifdef USE_OSGEARTH
#include <osgEarth/MapNode>
#endif


vtExternalHeightField3d::vtExternalHeightField3d(void)
{
	m_pHat = NULL;
	m_bOsgEarth = false;
}

vtExternalHeightField3d::~vtExternalHeightField3d(void)
{
	delete m_pHat;
}

bool vtExternalHeightField3d::Initialize(const char *external_data)
{
    // Set up a visitor for calculating height
	m_pHat = new osgSim::HeightAboveTerrain;
	if (NULL == m_pHat)
		return false;

	// OSG doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(external_data);
	osg::Node *pNode = osgDB::readNodeFile((const char *)fname_local);
	if (NULL == pNode)
		return false;

// !!!! TODO - Scaling from non metric co-ordinate systems !!!!!!!!!!!!!!!!!

#ifdef USE_OSGEARTH
    osgEarth::MapNode *pMapNode = dynamic_cast<osgEarth::MapNode*>(pNode);
    if (NULL != pMapNode)
    {
    	osgEarth::Map *pMap = pMapNode->getMap();
    	if (pMap->isGeocentric())
			return false;
		m_bOsgEarth = true;
		const osgEarth::Profile *pProfile = pMap->getProfile();
		if (!pProfile->isOK())
			return false;
		const osgEarth::GeoExtent& GeoExtent = pProfile->getExtent();
		const osgEarth::SpatialReference *pSRS = pProfile->getSRS();
		m_Projection.SetTextDescription((const char *)"wkt", pSRS->getWKT().c_str());
		vtHeightField3d::Initialize(m_Projection.GetUnits(), DRECT(GeoExtent.xMin(), GeoExtent.yMax(), GeoExtent.xMax(), GeoExtent.yMin()), 0, 1000); // TODO find min and max height
		// osgEarth coords are  - Left Handed Z up origin the origin of the coordinate system
		// Translate to origin
		m_TransfromOSGModel2VTPWorld.set(osg::Matrix::translate(osg::Vec3(-GeoExtent.xMin(), -GeoExtent.yMin(), 0)));
		// Spin Y up
		m_TransfromOSGModel2VTPWorld.postMult(osg::Matrix::rotate(-PID2d, osg::Vec3(1,0,0)));
		m_TransformVTPWorld2OSGModel.invert(m_TransfromOSGModel2VTPWorld);
		osg::MatrixTransform *transform = new osg::MatrixTransform;
		m_pNode = transform;
		transform->setName("Translate then spin Y up");
		transform->setMatrix(m_TransfromOSGModel2VTPWorld);
		transform->setDataVariance(osg::Object::STATIC);
		// Place the Terrain Geometry under the transform
		transform->addChild(pNode);

		m_pElevationManager = new osgEarth::Util::ElevationManager(pMap);
		osgEarth::MapFrame MapFrame(pMap, osgEarth::Map::ELEVATION_LAYERS);

		MapFrame.sync();
		int TileSizeInHeixels = 0;
		uint MaxLOD = 0;

        for( osgEarth::ElevationLayerVector::const_iterator i = MapFrame.elevationLayers().begin();
            i != MapFrame.elevationLayers().end();
            ++i )
        {
            // I need the maximum tile size in heixels
            int layerTileSize = i->get()->getTileSize();
            if ( layerTileSize > TileSizeInHeixels)
                TileSizeInHeixels = layerTileSize;

            // we also need the maximum available data level.
            uint layerMaxDataLevel = i->get()->getMaxDataLevel();
            if ( layerMaxDataLevel > MaxLOD )
                MaxLOD = layerMaxDataLevel;
        }
		uint tiles_wide, tiles_high;
        pProfile->getNumTiles(0, tiles_wide, tiles_high);
        double WidthResolution = GeoExtent.width() / (double)tiles_wide / (double)TileSizeInHeixels;
        double HeightResolution = GeoExtent.height() / (double)tiles_high / (double)TileSizeInHeixels;


        m_ResolutionAtLevel0 = (float)(std::min(WidthResolution, HeightResolution) + 0.00001);
        m_CompromiseResolution = (float)(std::min(WidthResolution, HeightResolution) / (2 ^ (MaxLOD / 2)) + 0.00001);

		return true;
    }
#endif
    // Find the top level TerrainTile
    // Top level node may be a coordinate system node
	osg::CoordinateSystemNode *pCoordSystem = dynamic_cast<osg::CoordinateSystemNode*>(pNode);
	// Look for the PagedLOD node
	osg::PagedLOD *pLod;
	if (NULL != pCoordSystem)
		pLod = dynamic_cast<osg::PagedLOD*>(pCoordSystem->getChild(0));
	else
		pLod = dynamic_cast<osg::PagedLOD*>(pNode);
	if (NULL == pLod)
		return false;
	m_pLOD = pLod;
	osgTerrain::TerrainTile *pTopTile = dynamic_cast<osgTerrain::TerrainTile*>(pLod->getChild(0));
	if (NULL == pTopTile)
		return false;

    // OK -got the top tile now get the top level elevation layer and initialise the vtHeightField3d
    // from the information in the layer
	m_pLayer = pTopTile->getElevationLayer();
	if (osgTerrain::Locator::GEOCENTRIC == m_pLayer->getLocator()->getCoordinateSystemType())
		// I have not worked out how to do something about the ellipsoidal model yet
		return false;
	// Use coordinate system of top tile for conversions
	m_Projection.SetTextDescription((const char *)"wkt", m_pLayer->getLocator()->getCoordinateSystem().c_str());
	// Calculate the rough minimum and maximum heights
	osg::BoundingBox bb;
	uint numColumns = m_pLayer->getNumColumns();
	uint numRows = m_pLayer->getNumRows();
	osg::Vec3d Local;
	for(uint r = 0; r<numRows; ++r)
	{
		for(uint c = 0;c<numColumns;++c)
		{
			float value = 0.0f;
			bool validValue = m_pLayer->getValidValue(c,r, value);
			if (validValue)
			{
				Local.x() = 0;
				Local.y() = 0;
				Local.z() = value;

				bb.expandBy(Local);
			}
		}
	}
	// OSG local cordinates are in range 0 to 1
	osg::Vec3d ModelBottomLeft, ModelTopRight;
	m_pLayer->getLocator()->convertLocalToModel(Local, ModelBottomLeft);
	Local.x() = 1;
	Local.y() = 1;
	m_pLayer->getLocator()->convertLocalToModel(Local, ModelTopRight);

	// This OSG Model coordinate system can be either projected, geographic
	// (lat/long), or geocentric (geo xyz).
	// I do not handle geocentric at the moment(see above)
	// So I treat projected and geographic as basically the same as VTP Earth
	// coordinates but Y up
	vtHeightField3d::Initialize(m_Projection.GetUnits(), DRECT(ModelBottomLeft.x(), ModelTopRight.y(), ModelTopRight.x(), ModelBottomLeft.y()), bb.zMin(), bb.zMax());

	// Finished initalising the heightfield.
	// Computer a matrix to take OSG model coordinates - Left Handed Z up
	// origin the origin of the coordinate system into VTP world - Left Handed
	// Y up origin always at the bottom left of the terrain extents.
	// So...
	// Translate to origin
	m_TransfromOSGModel2VTPWorld.set(osg::Matrix::translate(osg::Vec3(-ModelBottomLeft.x(), -ModelBottomLeft.y(), 0)));
	// Spin Y up
	m_TransfromOSGModel2VTPWorld.postMult(osg::Matrix::rotate(-PID2d, osg::Vec3(1,0,0)));
	m_TransformVTPWorld2OSGModel.invert(m_TransfromOSGModel2VTPWorld);
	osg::MatrixTransform *transform = new osg::MatrixTransform;
	m_pNode = transform;
	transform->setName("Translate then spin Y up");
	transform->setMatrix(m_TransfromOSGModel2VTPWorld);
	transform->setDataVariance(osg::Object::STATIC);
	// Place the Terrain Geometry under the transform
	transform->addChild(pNode);

	return true;
}

osg::Node *vtExternalHeightField3d::CreateGeometry()
{
	return m_pNode.get();
}

vtCRS &vtExternalHeightField3d::GetCRS()
{
	return m_Projection;
}

const vtCRS &vtExternalHeightField3d::GetCRS() const
{
	return m_Projection;
}


bool vtExternalHeightField3d::FindAltitudeOnEarth(const DPoint2 &p, float &fAltitude, bool bTrue) const
{
	FPoint3 VTPWorld;
	m_LocalCS.EarthToLocal(DPoint3(p.x, p.y, 0.0), VTPWorld);
	return FindAltitudeAtPoint(VTPWorld, fAltitude, bTrue);
}

bool vtExternalHeightField3d::FindAltitudeAtPoint(const FPoint3 &p3, float &fAltitude, bool bTrue, int iCultureFlags, FPoint3 *vNormal) const
{
	if (NULL != vNormal)
		return false; // Cannot do normals at the moment
	// Transform to OSG Model and then to OSG Local
	osg::Vec3d Model = osg::Vec3d(p3.x, p3.y, p3.z) * m_TransformVTPWorld2OSGModel;
	if (m_bOsgEarth)
	{
#ifdef USE_OSGEARTH
        double Resolution;
        double Elevation;
        double Requested_Resolution = bTrue ? 0 : m_CompromiseResolution;
		bool bRet = m_pElevationManager->getElevation(Model.x(), Model.y(), Requested_Resolution, NULL, Elevation, Resolution);
		fAltitude = (float)Elevation;
		return bRet;
#else
		return false;
#endif
	}
else
	{
		osg::Vec3d Local;
		m_pLayer->getLocator()->convertModelToLocal(Model, Local);
		// Check if over heightfield
		if (Local.x() < 0.0 || Local.x() > 1.0 || Local.y() < 0.0 || Local.y() > 1.0)
			return false;
		// Use HAT
		Model.z() = m_fMaxHeight + 1;
		if (m_pHat->getNumPoints() == 0)
			m_pHat->addPoint(Model);
		else
			m_pHat->setPoint(0, Model);
		if (bTrue)
			m_pHat->computeIntersections(m_pLOD);
		else
		{
			// Kill database reading
			osg::ref_ptr<osgSim::DatabaseCacheReadCallback> pCallback = m_pHat->getDatabaseCacheReadCallback();
			m_pHat->setDatabaseCacheReadCallback(NULL);
			m_pHat->computeIntersections(m_pLOD);
			m_pHat->setDatabaseCacheReadCallback(pCallback.get());
		}
		fAltitude = (float)(Model.z() - m_pHat->getHeightAboveTerrain(0));
		return true;
//		Just use the top level
//		return m_pLayer->getInterpolatedValue(Local.x(), Local.y(), fAltitude);
	}
}

bool vtExternalHeightField3d::CastRayToSurface(const FPoint3 &point, const FPoint3 &dir, FPoint3 &result) const
{
	float alt;
	bool bOn = FindAltitudeAtPoint(point, alt);

	// special case: straight up or down
	float mag2 = sqrt(dir.x*dir.x+dir.z*dir.z);
	if (fabs(mag2) < .000001)
	{
		result = point;
		result.y = alt;
		if (!bOn)
			return false;
		if (dir.y > 0)	// points up
			return (point.y < alt);
		else
			return (point.y > alt);
	}

	if (bOn && point.y < alt)
		return false;	// already firmly underground

    float smallest;
	if (m_bOsgEarth)
	{
#ifdef USE_OSGEARTH
		smallest = m_CompromiseResolution;
#else
		return false;
#endif
	}
else
	{
        uint NumColumns = m_pLayer->getNumColumns();
        uint NumRows = m_pLayer->getNumRows();
        osg::Vec3d Local((double)1.0/(double)(NumColumns - 1), double(1.0)/(double)(NumRows - 1), 0.0);
        osg::Vec3d Model;
        m_pLayer->getLocator()->convertLocalToModel(Local, Model);
        osg::Vec3d World = Model * m_TransfromOSGModel2VTPWorld;

        // adjust magnitude of dir until 2D component has a good magnitude
        smallest = std::min(World.x(), -World.z());
	}
    float adjust = smallest / mag2;
    FPoint3 dir2 = dir * adjust;

    bool found_above = false;
    FPoint3 p = point, lastp = point;
    while (true)
    {
        // are we out of bounds and moving away?
        if (p.x < m_WorldExtents.left && dir2.x < 0)
            return false;
        if (p.x > m_WorldExtents.right && dir2.x > 0)
            return false;
        if (p.z < m_WorldExtents.top && dir2.z < 0)
            return false;
        if (p.z > m_WorldExtents.bottom && dir2.z > 0)
            return false;

        bOn = FindAltitudeAtPoint(p, alt);
        if (bOn)
        {
            if (p.y > alt)
                found_above = true;
            else
                break;
        }
        lastp = p;
        p += dir2;
    }
    if (!found_above)
        return false;

    // now, do a binary search to refine the result
    FPoint3 p0 = lastp, p1 = p, p2;
    for (int i = 0; i < 10; i++)
    {
        p2 = (p0 + p1) / 2.0f;
        int above = PointIsAboveTerrain(p2);
        if (above == 1)	// above
            p0 = p2;
        else if (above == 0)	// below
            p1 = p2;
    }
    p2 = (p0 + p1) / 2.0f;
    // make sure it's precisely on the ground
    FindAltitudeAtPoint(p2, p2.y);
    result = p2;
    return true;
}
