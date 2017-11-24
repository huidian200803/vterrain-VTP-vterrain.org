//
// Features.cpp
//
// Copyright (c) 2002-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "Features.h"
#include "vtLog.h"
#include "DxfParser.h"
#include "FilePath.h"
#include "GDALWrapper.h"

//
// Construct / Destruct
//
vtFeatureSet::vtFeatureSet()
{
	m_eGeomType = wkbNone;
}

vtFeatureSet::~vtFeatureSet()
{
	DeleteFields();

	for (uint i = 0; i < m_Features.size(); i++)
		delete m_Features[i];
}

/**
 * Save a featureset from a SHP (ESRI Shapefile) with corresponding DBF file
 *
 * \param filename	Filename in UTF-8 encoding.
 * \param progress_callback Provide a callback function if you want to receive
 *		progress indication.
 *
 * \return true if successful.
 */
bool vtFeatureSet::SaveToSHP(const char *filename, bool progress_callback(int)) const
{
	VTLOG1("vtFeatureSet::SaveToSHP:\n");

	// Must use "C" locale in case we write any floating-point fields
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	int nSHPType = OGRToShapelib(m_eGeomType);

	// SHPOpen doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(filename);

	VTLOG1(" SaveToSHP: writing SHP\n");
	SHPHandle hSHP = SHPCreate(fname_local, nSHPType);
	if (!hSHP)
	{
		VTLOG1("SHPCreate failed.\n");
		return false;
	}
	SaveGeomToSHP(hSHP, progress_callback);
	SHPClose(hSHP);

	if (m_fields.GetSize() > 0)
	{
		VTLOG1(" SaveToSHP: writing DBF\n");

		// Save DBF File also
		vtString dbfname = fname_local;
		dbfname = dbfname.Left(dbfname.GetLength() - 4);
		dbfname += ".dbf";
		DBFHandle db = DBFCreate(dbfname);
		if (db == NULL)
		{
			VTLOG1("DBFCreate failed.\n");
			return false;
		}

		Field *field;
		for (uint i = 0; i < m_fields.GetSize(); i++)
		{
			field = m_fields[i];

			DBFFieldType dbtype = ConvertFieldType(field->m_type);
			DBFAddField(db, (const char *) field->m_name, dbtype,
				field->m_width, field->m_decimals );
		}

		// Write DBF Attributes, one record per entity
		uint entities = NumEntities();
		for (uint i = 0; i < entities; i++)
		{
			if (progress_callback && ((i%16)==0))
				progress_callback(i * 100 / entities);

			for (uint j = 0; j < m_fields.GetSize(); j++)
			{
				field = m_fields[j];
				switch (field->m_type)
				{
				case FT_Boolean:
					DBFWriteLogicalAttribute(db, i, j, field->m_bool[i]);
					break;
				case FT_Integer:
					DBFWriteIntegerAttribute(db, i, j, field->m_int[i]);
					break;
				case FT_Short:
					DBFWriteIntegerAttribute(db, i, j, field->m_short[i]);
					break;
				case FT_Float:
					// SHP does do floats, only doubles
					DBFWriteDoubleAttribute(db, i, j, field->m_float[i]);
					break;
				case FT_Double:
					DBFWriteDoubleAttribute(db, i, j, field->m_double[i]);
					break;
				case FT_String:
					DBFWriteStringAttribute(db, i, j, (const char *) field->m_string[i]);
					break;
				case FT_Unknown:
					// Should never get here.
					break;
				}
			}
		}
		DBFClose(db);
	}

	// Try saving CRS to PRJ
	VTLOG1(" SaveToSHP: writing PRJ\n");
	vtString prjname = filename;
	prjname = prjname.Left(prjname.GetLength() - 4);
	prjname += ".prj";
	m_crs.WriteProjFile(prjname);

	VTLOG1(" SaveToSHP: Done\n");
	return true;
}

/**
 * Load a featureset from a SHP (ESRI Shapefile).
 *
 * \param fname	filename in UTF-8 encoding.
 * \param progress_callback Provide a callback function if you want to receive
 *		progress indication.
 *
 * \return true if successful.
 */
bool vtFeatureSet::LoadFromSHP(const char *fname, bool progress_callback(int))
{
	VTLOG(" LoadFromSHP '%s': ", fname);

	// SHPOpen doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(fname);

	// Open the SHP File & Get Info from SHP:
	SHPHandle hSHP = SHPOpen(fname_local, "rb");
	if (hSHP == NULL)
	{
		VTLOG("Couldn't open.\n");
		return false;
	}

	VTLOG("Opened.\n");
	LoadGeomFromSHP(hSHP, progress_callback);
	SHPClose(hSHP);

	SetFilename(fname);

	// Attempt to load corresponding .prj file, if there is one
	m_crs.ReadProjFile(fname);

	// Read corresponding attributes (DBF fields and records)
	LoadDataFromDBF(fname, progress_callback);

	AllocateFeatures();

	return true;
}


/**
 * Load a featureset from a file.  It may be SHP, IGC, or DXF format.
 *
 * \param filename	Filename in UTF-8 encoding.
 *
 * \return a new vtFeatureSet if successful, otherwise NULL;
 */
vtFeatureSet *vtFeatureLoader::LoadFrom(const char *filename)
{
	vtString fname = filename;
	vtString ext = fname.Right(3);
	if (!ext.CompareNoCase("shp"))
		return LoadFromSHP(filename);
	else
	if (!ext.CompareNoCase("igc"))
		return LoadFromIGC(filename);
	else
	if (!ext.CompareNoCase("dxf"))
		return LoadFromDXF(filename);
	else
		// Try OGR for any other vector format it might be
		return LoadWithOGR(filename);
}

OGRwkbGeometryType GetTypeFromOGR(OGRLayer *pLayer)
{
	// Get basic information about the layer we're reading
	pLayer->ResetReading();
	OGRFeatureDefn *defn = pLayer->GetLayerDefn();
	if (!defn)
		return wkbUnknown;
	OGRwkbGeometryType geom_type = defn->GetGeomType();

	if (geom_type == wkbUnknown)
	{
		// This usually indicates that the file contains a mix of different
		// geometry types.  Look at the first geometry.
		OGRFeature *pFeature = pLayer->GetNextFeature();
		OGRGeometry *pGeom = pFeature->GetGeometryRef();
		geom_type = pGeom->getGeometryType();
	}
	return geom_type;
}

OGRwkbGeometryType GetTypeFromOGR(const char *filename)
{
	// OGR doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(filename);

	GDALDataset *pDatasource = (GDALDataset *) GDALOpen(fname_local, GA_ReadOnly);
	if (!pDatasource)
	{
		VTLOG("unknown Datasource!");
		return wkbUnknown;
	}

	// Take the contents of the first layer only.
	OGRLayer *pLayer = pDatasource->GetLayer(0);
	OGRwkbGeometryType type = GetTypeFromOGR(pLayer);
	delete pDatasource;
	return type;
}

OGRwkbGeometryType GetFeatureGeomType(const char *filename)
{
	vtString fname = filename;
	vtString ext = fname.Right(3);
	if (!ext.CompareNoCase("shp"))
	{
		// SHPOpen doesn't yet support utf-8 or wide filenames, so convert
		vtString fname_local = UTF8ToLocal(filename);

		// Open the SHP File & Get Info from SHP:
		SHPHandle hSHP = SHPOpen(fname_local, "rb");
		if (hSHP == NULL)
			return wkbUnknown;
		int nElems, nShapeType;
		SHPGetInfo(hSHP, &nElems, &nShapeType, NULL, NULL);
		SHPClose(hSHP);
		return ShapelibToOGR(nShapeType);
	}
	else if (!ext.CompareNoCase("igc"))
	{
		// IGC is always 3D linestring
		return wkbLineString25D;
	}
	else
		return GetTypeFromOGR(filename);
}

/**
 * Load a featureset from a SHP (ESRI Shapefile).
 *
 * \param filename	Filename in UTF-8 encoding.
 * \param progress_callback	Provide if you want a callback on progress.
 *
 * \return a new vtFeatureSet if successful, otherwise NULL.
 */
vtFeatureSet *vtFeatureLoader::LoadFromSHP(const char *filename, bool progress_callback(int))
{
	VTLOG(" FeatureLoader LoadFromSHP\n");

	// SHPOpen doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(filename);

	// Open the SHP File & Get Info from SHP:
	SHPHandle hSHP = SHPOpen(fname_local, "rb");
	if (hSHP == NULL)
	{
		m_strErrorMsg = "Could not open file.";
		return NULL;
	}

	// Get number of entities (nElem) and type of data (nShapeType)
	int		nElems, nShapeType;
	SHPGetInfo(hSHP, &nElems, &nShapeType, NULL, NULL);

	//  Check shape type, we only support a few types
	vtFeatureSet *pSet = NULL;

	switch (nShapeType)
	{
	case SHPT_POINT:
		pSet = new vtFeatureSetPoint2D;
		break;
	case SHPT_POINTZ:
		pSet = new vtFeatureSetPoint3D;
		break;
	case SHPT_ARC:
		pSet = new vtFeatureSetLineString;
		break;
	case SHPT_ARCZ:
		pSet = new vtFeatureSetLineString3D;
		break;
	case SHPT_POLYGON:
		pSet = new vtFeatureSetPolygon;
		break;
	default:
		m_strErrorMsg = "Unsupported SHP geometry type: ";
		m_strErrorMsg += GetShapeTypeName(nShapeType);
		SHPClose(hSHP);
		return NULL;
	}
	SHPClose(hSHP);

	// Read SHP header and geometry from SHP into memory
	pSet->LoadFromSHP(filename, progress_callback);

	return pSet;
}

/////////////////////////////////////////////////////////////////////

/**
 * Load a featureset from a SHP (ESRI Shapefile).
 *
 * \param filename	Filename in UTF-8 encoding.
 * \param progress_callback Provide a callback function if you want to receive
 *		progress indication.
 *
 * \return a new vtFeatureSet if successful, otherwise NULL.
 */
vtFeatureSet *vtFeatureLoader::LoadFromDXF(const char *filename,
										   bool progress_callback(int))
{
	VTLOG("vtFeatureLoader::LoadFromDXF():\n");

	std::vector<DxfEntity> entities;
	std::vector<DxfLayer> layers;

	DxfParser parser(filename, entities, layers);
	bool bSuccess = parser.RetrieveEntities(progress_callback);
	if (!bSuccess)
	{
		VTLOG(parser.GetLastError());
		return NULL;
	}
	uint iNumEntities = entities.size();
	if (iNumEntities == 0)
		return NULL;

	// Look at first entity
	vtFeatureSetLineString *pSetP2;
	const DxfEntity &ent = entities[0];
	if (ent.m_iType == DET_Polyline)
		pSetP2 = new vtFeatureSetLineString;
//	else if (ent.m_iType == DET_Polygon)	// TODO? Other types.
//		pSetP2 = new vtFeatureSetPolygon;
	else
		return NULL;

	int vtx = 0;
	int polylines = 0;
	for (uint i = 0; i < iNumEntities; i++)
	{
		const DxfEntity &ent = entities[i];
		if (ent.m_iType == DET_Polyline)
		{
			DLine2 dline;
			int iNumVerts = ent.m_points.size();
			for (int j = 0; j < iNumVerts; j++)
			{
				DPoint2 p(ent.m_points[j].x, ent.m_points[j].y);
				float z = (float) ent.m_points[j].z;

				// AddVert(p, z);
				dline.Append(p);
			}
			pSetP2->AddPolyLine(dline);
			polylines ++;
		}
	}
	VTLOG("  Found %d entities of type Polyline.\n", polylines);

	// If we didn't find any polylines, we haven't got a featureset
	if (polylines == 0)
		return NULL;

	return pSetP2;
}


///////////////////////////////////////////////////////////////////////

// helpers
double GetMinutes(const char *buf)
{
	char copy[20];
	strncpy(copy, buf, 2);
	copy[2] = '.';
	strncpy(copy+3, buf+2, 3);
	copy[6] = 0;
	return atof(copy);
}

/**
 * Load a featureset from an IGC file.
 *
 * \param filename	Filename in UTF-8 encoding.
 *
 * \return a new vtFeatureSet if successful, otherwise NULL.
 */
vtFeatureSet *vtFeatureLoader::LoadFromIGC(const char *filename)
{
	// Must use "C" locale because we use atof()
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	VTLOG(" FeatureLoader LoadFromIGC\n");

	FILE *fp = vtFileOpen(filename, "rb");
	if (!fp)
		return NULL;

	//  IGC is lat-lon with altitude
	vtFeatureSetLineString3D *pSet = new vtFeatureSetLineString3D;

	// Geographic WGS-84 can be assumed
	vtCRS crs;
	crs.SetSimple(false, 0, EPSG_DATUM_WGS84);
	pSet->SetCRS(crs);

	DLine3 dline;

	char buf[80];
	while (fgets(buf, 80, fp) != NULL)
	{
		if (buf[0] == 'B')	// 'Fix' record which contains elevation
		{
			// 0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3
			// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4
			// B H H M M S S D D M M M M M N D D D M M M M M E V P P P P P G G G G G CR LF
			DPoint3 p;
			int deg;
			double min;

			deg = GetIntFromString(buf+7, 2);
			min = GetMinutes(buf+9);
			if (buf[14] == 'N')
				p.y = deg + (min / 60.0);
			else if (buf[14] == 'S')
				p.y = -(deg + (min / 60.0));

			deg = GetIntFromString(buf+15, 3);
			min = GetMinutes(buf+18);
			if (buf[23] == 'E')
				p.x = deg + (min / 60.0);
			else if (buf[23] == 'W')
				p.x = -(deg + (min / 60.0));

			// Pressure altitude
			// "to the ICAO ISA above the 1013.25 HPa sea level datum, valid characters 0-9"
			int alt_pressure = GetIntFromString(buf+25, 5);

			// GPS altitude
			// "Altitude above the WGS84 ellipsoid, valid characters 0-9"
			int alt_gnss = GetIntFromString(buf+30, 5);

			p.z = alt_pressure;

			dline.Append(p);
		}
	}
	pSet->AddPolyLine(dline);
	fclose(fp);
	pSet->SetFilename(filename);

	return pSet;
}

/////////////////////////////////////////////////////////////////////////////

vtFeatureSet *vtFeatureLoader::LoadWithOGR(OGRLayer *pLayer,
							 bool progress_callback(int))
{
	if (!pLayer)
	{
		VTLOG("OGRLayer is NULL in LoadWithOGR(OGRLayer)\n");
		return NULL;
	}

	// Get basic information about the layer we're reading
	OGRwkbGeometryType geom_type = GetTypeFromOGR(pLayer);

	vtFeatureSet *pSet = NULL;
	switch (geom_type)
	{
	case wkbPoint:
		pSet = new vtFeatureSetPoint2D;
		break;
	case wkbPoint25D:
		pSet = new vtFeatureSetPoint3D;
		break;
	case wkbLineString:
	case wkbMultiLineString:
		pSet = new vtFeatureSetLineString;
		break;
	case wkbLineString25D:
		pSet = new vtFeatureSetLineString3D;
		break;
	case wkbPolygon:
	case wkbMultiPolygon:
		pSet = new vtFeatureSetPolygon;
		break;
	default:
		return NULL;
	}

	if (!pSet->LoadFromOGR(pLayer, progress_callback))
	{
		delete pSet;
		return NULL;
	}
	return pSet;
}

/**
 * Load a featureset from a file using the OGR library.  It can be in any
 *  file format that OGR supports.
 *
 * \param filename	Filename in UTF-8 encoding.
 * \param progress_callback Provide a callback function if you want to receive
 *		progress indication.
 *
 * \return a new vtFeatureSet if successful, otherwise NULL.
 */
vtFeatureSet *vtFeatureLoader::LoadWithOGR(const char *filename,
							 bool progress_callback(int))
{
	// try using OGR
	g_GDALWrapper.RequestOGRFormats();

	// OGR doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(filename);

	GDALDataset *pDatasource = (GDALDataset *)GDALOpen(fname_local, GA_ReadOnly);
	if (!pDatasource)
	{
		VTLOG("LoadWithOGR: data source is NULL.\n");
		return NULL;
	}

	// Take the contents of the first layer only.
	OGRLayer *pLayer = pDatasource->GetLayer(0);
	if (!pLayer)
	{
		VTLOG("LoadWithOGR: OGRLayer is NULL.\n");
	}

	vtFeatureSet *pSet = LoadWithOGR(pLayer, progress_callback);
	if (pSet)
	{
		// We've read the file now, so take it's name
		pSet->SetFilename(filename);
	}
	else
		VTLOG("LoadWithOGR: FeatureSet is NULL.\n");

	delete pDatasource;

	return pSet;
}

bool vtFeatureSet::LoadFromOGR(OGRLayer *pLayer,
							 bool progress_callback(int))
{
	VTLOG(" LoadFromOGR\n");

	// get information from the datasource
	OGRFeatureDefn *defn = pLayer->GetLayerDefn();
	int feature_count = (int) pLayer->GetFeatureCount();
	int num_fields = defn->GetFieldCount();

	// Get the projection (SpatialReference) from this layer, if we can.
	// Sometimes (e.g. for GML) the layer doesn't have it; may have to
	// use the first Geometry instead.
	bool bGotCS = false;
	OGRSpatialReference *pSpatialRef = pLayer->GetSpatialRef();
	if (pSpatialRef)
	{
		m_crs.SetSpatialReference(pSpatialRef);
		bGotCS = true;
	}

	int i, j, count;

	for (j = 0; j < num_fields; j++)
	{
		OGRFieldDefn *field_def = defn->GetFieldDefn(j);
		const char *field_name = field_def->GetNameRef();
		OGRFieldType field_type = field_def->GetType();
		int width = field_def->GetWidth();

		FieldType ftype;
		switch (field_type)
		{
		case OFTInteger:
			ftype = FT_Integer;
			break;
		case OFTReal:
			ftype = FT_Double;
			break;
		case OFTString:
			ftype = FT_String;
			break;
		default:
			continue;
		}
		AddField(field_name, ftype, width);
	}

	// For efficiency, pre-allocate room for the number of features
	//  we expect, although there may be a few more.
	Reserve(feature_count);

	// Read Data from OGR into memory
	DPoint2 p2, first_p2;
	DPoint3 p3;
	int num_geoms, num_points;
	OGRPoint		*pPoint;
	OGRPolygon		*pPolygon;
	OGRLinearRing	*pRing;
	OGRLineString   *pLineString;
	OGRMultiLineString   *pMulti;
	OGRMultiPolygon *pMultiPoly;

	vtFeatureSetPoint2D *pSetP2 = dynamic_cast<vtFeatureSetPoint2D *>(this);
	vtFeatureSetPoint3D *pSetP3 = dynamic_cast<vtFeatureSetPoint3D *>(this);
	vtFeatureSetLineString *pSetLine = dynamic_cast<vtFeatureSetLineString *>(this);
	vtFeatureSetLineString3D *pSetLine3 = dynamic_cast<vtFeatureSetLineString3D *>(this);
	vtFeatureSetPolygon *pSetPoly = dynamic_cast<vtFeatureSetPolygon *>(this);

	pLayer->ResetReading();
	count = 0;
	OGRFeature *pFeature;
	while( (pFeature = pLayer->GetNextFeature()) != NULL )
	{
		if (progress_callback != NULL)
			progress_callback(count * 100 / feature_count);

		OGRGeometry	*pGeom = pFeature->GetGeometryRef();
		if (!pGeom)
			continue;

		if (!bGotCS)
		{
			OGRSpatialReference *pSpatialRef = pGeom->getSpatialReference();
			if (pSpatialRef)
			{
				m_crs.SetSpatialReference(pSpatialRef);
				bGotCS = true;
			}
		}
		// Beware - some OGR-supported formats, such as MapInfo,
		//  will have more than one kind of geometry per layer,
		//  for example, both LineString and MultiLineString
		// Get the geometry type from the Geometry, not the Layer.
		OGRwkbGeometryType geom_type = pGeom->getGeometryType();
		num_geoms = 1;

		DLine2 dline2;
		DLine3 dline3;
		DPolygon2 dpoly;

		switch (geom_type)
		{
		case wkbPoint:
			pPoint = (OGRPoint *) pGeom;
			if (pSetP2)
				pSetP2->AddPoint(DPoint2(pPoint->getX(), pPoint->getY()));
			break;

		case wkbPoint25D:
			pPoint = (OGRPoint *) pGeom;
			if (pSetP3)
				pSetP3->AddPoint(DPoint3(pPoint->getX(), pPoint->getY(), pPoint->getZ()));
			break;

		case wkbLineString:
			pLineString = (OGRLineString *) pGeom;
			if (pSetLine)
			{
				num_points = pLineString->getNumPoints();
				dline2.SetSize(num_points);
				for (j = 0; j < num_points; j++)
				{
					p2.Set(pLineString->getX(j), pLineString->getY(j));
					dline2.SetAt(j, p2);
				}
				pSetLine->AddPolyLine(dline2);
			}
			break;

		case wkbLineString25D:
			pLineString = (OGRLineString *) pGeom;
			if (pSetLine3)
			{
				num_points = pLineString->getNumPoints();
				dline3.SetSize(num_points);
				for (j = 0; j < num_points; j++)
				{
					p3.Set(pLineString->getX(j), pLineString->getY(j), pLineString->getZ(j));
					dline3.SetAt(j, p3);
				}
				pSetLine3->AddPolyLine(dline3);
			}
			break;

		case wkbMultiLineString:
			pMulti = (OGRMultiLineString *) pGeom;
			if (pSetLine)
			{
				num_geoms = pMulti->getNumGeometries();
				for (i = 0; i < num_geoms; i++)
				{
					pLineString = (OGRLineString *) pMulti->getGeometryRef(i);
					num_points = pLineString->getNumPoints();
					dline2.SetSize(num_points);
					for (j = 0; j < num_points; j++)
					{
						p2.Set(pLineString->getX(j), pLineString->getY(j));
						dline2.SetAt(j, p2);
					}
					pSetLine->AddPolyLine(dline2);
				}
			}
			break;

		case wkbPolygon:
			pPolygon = (OGRPolygon *) pGeom;
			pRing = pPolygon->getExteriorRing();
			num_points = pRing->getNumPoints();

			dpoly.resize(0);

			// do exterior ring
			dline2.SetSize(0);
			dline2.SetMaxSize(num_points);
			for (j = 0; j < num_points; j++)
			{
				p2.Set(pRing->getX(j), pRing->getY(j));

				// ignore last point if it's the same as the first
				if (j == 0)
					first_p2 = p2;
				if (j == num_points-1 && p2 == first_p2)
					continue;

				dline2.Append(p2);
			}
			dpoly.push_back(dline2);

			// do interior ring(s)
			for (i = 0; i < pPolygon->getNumInteriorRings(); i++)
			{
				pRing = pPolygon->getInteriorRing(i);
				num_points = pRing->getNumPoints();
				dline2.SetSize(0);
				dline2.SetMaxSize(num_points);
				for (j = 0; j < num_points; j++)
				{
					p2.Set(pRing->getX(j), pRing->getY(j));

					// ignore last point if it's the same as the first
					if (j == 0)
						first_p2 = p2;
					if (j == num_points-1 && p2 == first_p2)
						continue;

					dline2.Append(p2);
				}
				dpoly.push_back(dline2);
			}
			if (pSetPoly)
				pSetPoly->AddPolygon(dpoly);
			break;

		case wkbMultiPolygon:
			pMultiPoly = (OGRMultiPolygon *) pGeom;
			if (pSetPoly)
			{
				num_geoms = pMultiPoly->getNumGeometries();
				for (i=0; i < num_geoms; ++i)
				{
					pPolygon = (OGRPolygon *) pMultiPoly->getGeometryRef(i);

					//handle each polygon's exterior ring
					pRing = pPolygon->getExteriorRing();
					num_points = pRing->getNumPoints();

					dpoly.resize(0);

					// do exterior ring
					dline2.SetSize(0);
					dline2.SetMaxSize(num_points);
					for (j = 0; j < num_points; j++)
					{
						p2.Set(pRing->getX(j), pRing->getY(j));

						// ignore last point if it's the same as the first
						if (j == 0)
							first_p2 = p2;
						if (j == num_points-1 && p2 == first_p2)
							continue;

						dline2.Append(p2);
					}
					dpoly.push_back(dline2);

					//handle the interior rings of each polygon
					// do interior ring(s)
					for (i = 0; i < pPolygon->getNumInteriorRings(); i++)
					{
						pRing = pPolygon->getInteriorRing(i);
						num_points = pRing->getNumPoints();
						dline2.SetSize(0);
						dline2.SetMaxSize(num_points);
						for (j = 0; j < num_points; j++)
						{
							p2.Set(pRing->getX(j), pRing->getY(j));

							// ignore last point if it's the same as the first
							if (j == 0)
								first_p2 = p2;
							if (j == num_points-1 && p2 == first_p2)
								continue;

							dline2.Append(p2);
						}
						dpoly.push_back(dline2);
					}

					pSetPoly->AddPolygon(dpoly);
				}
			}
			break;

		case wkbMultiPoint:
		case wkbGeometryCollection:
		default:
			// Hopefully we won't encounter unexpected geometries, but
			// if we do, just skip them for now.
			continue;
			break;

		}

		// In case more than one geometry was encountered, we need to add
		// a record with attributes for each one.
		for (i = 0; i < num_geoms; i++)
		{
			AddRecord();

			for (j = 0; j < num_fields; j++)
			{
				Field *pField = GetField(j);
				switch (pField->m_type)
				{
				case FT_Boolean:
					SetValue(count, j, pFeature->GetFieldAsInteger(j) != 0);
					break;
				case FT_Integer:
					SetValue(count, j, pFeature->GetFieldAsInteger(j));
					break;
				case FT_Double:
					SetValue(count, j, pFeature->GetFieldAsDouble(j));
					break;
				case FT_String:
					SetValue(count, j, pFeature->GetFieldAsString(j));
					break;
				case FT_Short:
				case FT_Float:
				case FT_Unknown:
					// we cannot get these, because we don't create them here
					break;
				}
			}
			count++;
		}
		// track total features
		feature_count += (num_geoms-1);

		// features returned from OGRLayer::GetNextFeature are our responsibility to delete!
		delete pFeature;
	}
	return true;
}


/**
 * Load a featureset's field data from a DBF file
 *
 * \param filename	Filename in UTF-8 encoding.
 * \param progress_callback Provide a callback function if you want to receive
 *		progress indication.
 *
 * \return true if successful.
 */
bool vtFeatureSet::LoadDataFromDBF(const char *filename, bool progress_callback(int))
{
	// Must use "C" locale in case we read any floating-point fields
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	VTLOG(" LoadDataFromDBF\n");

	// Try loading DBF File
	vtString dbfname = MakeDBFName(filename);

	// DBFOpen doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(dbfname);

	DBFHandle db = DBFOpen(fname_local, "rb");
	if (db == NULL)
		return false;

	ParseDBFFields(db);
	ParseDBFRecords(db, progress_callback);
	DBFClose(db);

	return true;
}

/**
 * A lightweight alternative to LoadDataFromDBF, which simply reads the
 * field descriptions from the DBF file.
 */
bool vtFeatureSet::LoadFieldInfoFromDBF(const char *filename)
{
	VTLOG(" LoadFieldInfoFromDBF\n");
	vtString dbfname = MakeDBFName(filename);

	// DBFOpen doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(dbfname);

	DBFHandle db = DBFOpen(fname_local, "rb");
	if (db == NULL)
		return false;

	ParseDBFFields(db);
	DBFClose(db);
	return true;
}

void vtFeatureSet::ParseDBFFields(DBFHandle db)
{
	// Check for field of poly id, current default field in dbf is Id
	int iSHPFields = DBFGetFieldCount(db);
	int pnWidth, pnDecimals;
	DBFFieldType fieldtype;
	char szFieldName[80];
	int iField;

	for (iField = 0; iField < iSHPFields; iField++)
	{
		fieldtype = DBFGetFieldInfo(db, iField, szFieldName,
			&pnWidth, &pnDecimals);

		FieldType ftype = ConvertFieldType(fieldtype);

		AddField(szFieldName, ftype, pnWidth);
	}
}

void vtFeatureSet::ParseDBFRecords(DBFHandle db, bool progress_callback(int))
{
	int iRecords = DBFGetRecordCount(db);

	// safety check
	// i have seen some DBF to have more records than the SHP has entities
	if ((uint) iRecords > NumEntities())
		iRecords = NumEntities();

	for (int i = 0; i < iRecords; i++)
	{
		if (progress_callback && ((i%16)==0))
			progress_callback(i*100/iRecords);
		uint iField;
		for (iField = 0; iField < NumFields(); iField++)
		{
			Field *field = m_fields[iField];
			switch (field->m_type)
			{
			case FT_String:
				SetValue(i, iField, DBFReadStringAttribute(db, i, iField));
				break;
			case FT_Integer:
				SetValue(i, iField, DBFReadIntegerAttribute(db, i, iField));
				break;
			case FT_Double:
				SetValue(i, iField, DBFReadDoubleAttribute(db, i, iField));
				break;
			case FT_Boolean:
				SetValue(i, iField, DBFReadLogicalAttribute(db, i, iField));
				break;
			case FT_Short:
			case FT_Float:
			case FT_Unknown:
				// we cannot get these, because we don't create them from DBF
				break;
			}
		}
	}
}

void ParseQuotedCSV(const char *buf, vtStringArray &strings)
{
	const char *c = buf;
	bool instring = false;
	vtString newstring;
	while (*c)
	{
		if (instring)
		{
			if (*c == '"')
			{
				// might be end of string, might be an escaped quote
				if (*(c+1) == '"')
				{
					newstring += '"';
					c++;
				}
				else
				{
					instring = false;
					strings.push_back(newstring);
					newstring = "";
				}
			}
			else
				newstring += *c;
		}
		else
		{
			if (*c == '"')
				instring = true;
			// also beware the possibility of completely empty entries
			else if (*c == ',' && *(c+1) == ',')
				strings.push_back(vtString(""));
		}
		c++;
	}
}

int charcount(const char *buf, char ch)
{
	int count = 0;
	for (const char *c = buf; *c; c++)
		if (*c == ch) count++;
	return count;
}

bool isodd(int n)
{
	return (n&1) != 0;
}

/**
 * Load a featureset's field data from a comma-separated-value (CSV) file.
 *
 * \param filename	filename in UTF-8 encoding.
 * \param progress_callback Provide a callback function if you want to receive
 *		progress indication.
 *
 * \return true if successful.
 */
bool vtFeatureSet::LoadDataFromCSV(const char *filename, bool progress_callback(int))
{
	VTLOG(" LoadDataFromCSV\n");

	FILE *fp = vtFileOpen(filename, "rb");
	if (fp == NULL)
		return false;

	char buf[4096];
	if (!fgets(buf, 4096, fp))
	{
		fclose(fp);
		return false;
	}

	// parse first line: field names
	vtStringArray words;
	ParseQuotedCSV(buf, words);
	uint iFields = words.size();
	if (iFields == 0)
	{
		// no fields
		fclose(fp);
		return false;
	}
	uint f;
	for (f = 0; f < iFields; f++)
	{
		// 1 is dummy value replaced later when we know max field width
		AddField(words[f], FT_String, 1);
	}
	uint iEntities = 0;
	char buf2[4096];
	while (fgets(buf, 4096, fp) != NULL)
	{
		// beware of LF in the middle of quoted strings
		while (isodd(charcount(buf, '"')))
		{
			fgets(buf2, 4096, fp);
			strcat(buf, buf2);
		}

		words.clear();
		ParseQuotedCSV(buf, words);
		if (words.size() != iFields)
		{
			int error = 1;
		}
		int rec = AddRecord();
		for (f = 0; f < words.size(); f++)
		{
			SetValue(rec, f, words[f]);

			// look up maximum string lengths for DBF field widths
			int len = strlen(words[f]);
			Field *field = GetField(f);
			if (len > field->m_width)
				field->m_width = len;
		}
		iEntities++;
	}
	// create blank geometries to match the records
	SetNumGeometries(iEntities);

	fclose(fp);
	return true;
}


/**
 * Export a featureset to a KML file.  This is still very limited and experimental.
 *
 * \param filename	Filename in UTF-8 encoding.
 * \param progress_callback Provide a callback function if you want to receive
 *		progress indication.
 *
 * \return true if successful.
 */
bool vtFeatureSet::SaveToKML(const char *filename, bool progress_callback(int)) const
{
	// Must use "C" locale in case we write any floating-point fields
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	const vtFeatureSetPoint2D *P2D = dynamic_cast<const vtFeatureSetPoint2D*>(this);
	const vtFeatureSetPoint3D *P3D = dynamic_cast<const vtFeatureSetPoint3D*>(this);

	if (P2D || P3D)
	{
		FILE *fp = vtFileOpen(filename, "wb");
		if (!fp)
			return false;

		fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		fprintf(fp, "<kml xmlns=\"http://earth.google.com/kml/2.2\">\n");
		fprintf(fp, "<Document>\n");
		fprintf(fp, "\t<name>%s</name>\n", filename);
		fprintf(fp, "\t<Style id=\"sn_icon12\">\n");
		fprintf(fp, "\t\t<IconStyle>\n");
		fprintf(fp, "\t\t\t<Icon>\n");
		fprintf(fp, "\t\t\t\t<href>http://maps.google.com/mapfiles/kml/pal2/icon12.png</href>\n");
		fprintf(fp, "\t\t\t</Icon>\n");
		fprintf(fp, "\t\t</IconStyle>\n");
		fprintf(fp, "\t\t<LabelStyle>\n");
		fprintf(fp, "\t\t\t<scale>0.8</scale>\n");
		fprintf(fp, "\t\t</LabelStyle>\n");
		fprintf(fp, "\t</Style>\n");

		// Write entities
		DPoint2 p2;
		DPoint3 p3;
		uint entities = NumEntities();
		for (uint i = 0; i < entities; i++)
		{
			if (progress_callback && ((i%16)==0))
				progress_callback(i * 100 / entities);

			fprintf(fp, "\t<Placemark>\n");

			vtString str;
			for (uint j = 0; j < m_fields.GetSize(); j++)
			{
				GetValueAsString(i, j, str);
				Field *field = m_fields[j];

				fprintf(fp, "\t\t<%s>%s</%s>\n", (const char *) field->m_name,
					(const char *) str, (const char *) field->m_name);
			}
			fprintf(fp, "\t\t<styleUrl>#sn_icon12</styleUrl>\n");
			fprintf(fp, "\t\t<Point>\n");
			fprintf(fp, "\t\t\t<coordinates>");
			if (P2D)
			{
				P2D->GetPoint(i, p2);
				fprintf(fp, "%lf,%lf,0", p2.x, p2.y);
			}
			if (P3D)
			{
				P3D->GetPoint(i, p3);
				fprintf(fp, "%lf,%lf,%.2lf", p3.x, p3.y, p3.z);
			}
			fprintf(fp, "</coordinates>\n");
			fprintf(fp, "\t\t</Point>\n");
			fprintf(fp, "\t</Placemark>\n");
		}
		fprintf(fp, "\t</Document>\n");
		fprintf(fp, "\t</kml>\n");
		fclose(fp);
		return true;
	}
	return false;
}


/**
 * Set the number of entities.  This expands (or contracts) the number of
 * geometry entities and corresponding records.
 */
void vtFeatureSet::SetNumEntities(int iNum)
{
	int previous = NumEntities();

	// First set the number of geometries
	SetNumGeometries(iNum);

	// Then set the number of records for each field
	for (uint iField = 0; iField < NumFields(); iField++)
		m_fields[iField]->SetNumRecords(iNum);

	// Also keep size of flag array in synch
	m_Features.resize(iNum);
	for (int i = 0; i < (iNum - previous); i++)
	{
		vtFeature *f = new vtFeature;
		f->flags = 0;
		m_Features[i] = f;
	}
}

void vtFeatureSet::AllocateFeatures()
{
	// Set up Features array
	for (uint i = 0; i < NumEntities(); i++)
	{
		vtFeature *f = new vtFeature;
		f->flags = 0;
		m_Features.push_back(f);
	}
}

/**
 * Returns the type of geometry that each feature has.
 *
 * \return
 *		- wkbPoint for 2D points
 *		- wkbPoint25D fpr 3D points
 *		- wkbLineString for 2D polylines
 *		- wkbPolygon for 2D polygons
 */
OGRwkbGeometryType vtFeatureSet::GetGeomType() const
{
	return m_eGeomType;
}

/**
 * Set the type of geometry that each feature will have.
 *
 * \param eGeomType
 *		- wkbPoint for 2D points
 *		- wkbPoint25D fpr 3D points
 *		- wkbLineString for 2D polylines
 *		- wkbPolygon for 2D polygons
 */
void vtFeatureSet::SetGeomType(OGRwkbGeometryType eGeomType)
{
	m_eGeomType = eGeomType;
}

/**
 * Append the contents of another featureset to this one.
 * The two featuresets must have the same geometry type.
 * Only fields with matching names are copied in the record data.
 */
bool vtFeatureSet::AppendDataFrom(vtFeatureSet *pFromSet)
{
	// When copying field data, must use C locale
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	// Must be the same geometry type
	if (pFromSet->GetGeomType() != GetGeomType())
		return false;

	int first_appended_ent = NumEntities();

	// copy geometry
	if (!AppendGeometryFrom(pFromSet))
		return false;

	// copy entities
	vtString str;
	uint i, num = pFromSet->NumEntities();
	for (i = 0; i < num; i++)
	{
		// copy record data for all field names which match
		for (uint f = 0; f < pFromSet->NumFields(); f++)
		{
			Field *field1 = pFromSet->GetField(f);
			Field *field2 = GetField((const char *) field1->m_name);
			if (!field2)
				continue;
			field1->GetValueAsString(i, str);
			field2->SetValueFromString(first_appended_ent+i, str);
		}
		// copy flags
		vtFeature *feat = new vtFeature;
		feat->flags = pFromSet->m_Features[i]->flags;
		m_Features.push_back(feat);
	}

	// empty the source layer
	pFromSet->SetNumEntities(0);

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// Selection of Entities

uint vtFeatureSet::NumSelected() const
{
	uint count = 0;
	uint size = m_Features.size();
	for (uint i = 0; i < size; i++)
		if (m_Features[i]->flags & FF_SELECTED)
			count++;
	return count;
}

void vtFeatureSet::DeselectAll()
{
	for (uint i = 0; i < m_Features.size(); i++)
		m_Features[i]->flags &= ~FF_SELECTED;
}

void vtFeatureSet::InvertSelection()
{
	for (uint i = 0; i < m_Features.size(); i++)
		m_Features[i]->flags ^= FF_SELECTED;
}

int vtFeatureSet::DoBoxSelect(const DRECT &rect, SelectionType st)
{
	int affected = 0;
	int entities = NumEntities();

	bool bIn;
	bool bWas;
	for (int i = 0; i < entities; i++)
	{
		bWas = (m_Features[i]->flags & FF_SELECTED);
		if (st == ST_NORMAL)
			Select(i, false);

		bIn = IsInsideRect(i, rect);
		if (!bIn)
			continue;

		switch (st)
		{
		case ST_NORMAL:
			Select(i, true);
			affected++;
			break;
		case ST_ADD:
			Select(i, true);
			if (!bWas) affected++;
			break;
		case ST_SUBTRACT:
			Select(i, false);
			if (bWas) affected++;
			break;
		case ST_TOGGLE:
			Select(i, !bWas);
			affected++;
			break;
		}
	}
	return affected;
}

int vtFeatureSet::SelectByCondition(int iField, int iCondition,
								  const char *szValue)
{
	bool bval, btest;
	int i, ival, itest;
	short sval;
	double dval, dtest=0;
	int entities = NumEntities(), selected = 0;
	int con = iCondition;
	bool result=false;
	DPoint2 p2;
	DPoint3 p3;

	if (iField < 0)
	{
		dval = atof(szValue);
		for (i = 0; i < entities; i++)
		{
			// special field numbers are used to refer to the spatial components
			if (m_eGeomType == wkbPoint)
			{
				vtFeatureSetPoint2D *pSetP2 = dynamic_cast<vtFeatureSetPoint2D *>(this);
				pSetP2->GetPoint(i, p2);
				if (iField == -1) dtest = p2.x;
				if (iField == -2) dtest = p2.y;
				if (iField == -3) return -1;
			}
			else if (m_eGeomType == wkbPoint25D)
			{
				vtFeatureSetPoint3D *pSetP3 = dynamic_cast<vtFeatureSetPoint3D *>(this);
				pSetP3->GetPoint(i, p3);
				if (iField == -1) dtest = p3.x;
				if (iField == -2) dtest = p3.y;
				if (iField == -3) dtest = p3.z;
			}
			else
				return -1;	// TODO: support non-point types
			if (con == 0) result = (dtest == dval);
			if (con == 1) result = (dtest > dval);
			if (con == 2) result = (dtest < dval);
			if (con == 3) result = (dtest >= dval);
			if (con == 4) result = (dtest <= dval);
			if (con == 5) result = (dtest != dval);
			if (result)
			{
				Select(i);
				selected++;
			}
		}
		return selected;
	}
	Field *field = m_fields[iField];
	switch (field->m_type)
	{
	case FT_String:
		for (i = 0; i < entities; i++)
		{
			const vtString &sp = field->m_string[i];
			if (con == 0) result = (sp.Compare(szValue) == 0);
			if (con == 1) result = (sp.Compare(szValue) > 0);
			if (con == 2) result = (sp.Compare(szValue) < 0);
			if (con == 3) result = (sp.Compare(szValue) >= 0);
			if (con == 4) result = (sp.Compare(szValue) <= 0);
			if (con == 5) result = (sp.Compare(szValue) != 0);
			if (result)
			{
				Select(i);
				selected++;
			}
		}
		break;
	case FT_Integer:
		ival = atoi(szValue);
		for (i = 0; i < entities; i++)
		{
			itest = field->m_int[i];
			if (con == 0) result = (itest == ival);
			if (con == 1) result = (itest > ival);
			if (con == 2) result = (itest < ival);
			if (con == 3) result = (itest >= ival);
			if (con == 4) result = (itest <= ival);
			if (con == 5) result = (itest != ival);
			if (result)
			{
				Select(i);
				selected++;
			}
		}
		break;
	case FT_Short:
		sval = (short) atoi(szValue);
		for (i = 0; i < entities; i++)
		{
			itest = field->m_short[i];
			if (con == 0) result = (itest == sval);
			if (con == 1) result = (itest > sval);
			if (con == 2) result = (itest < sval);
			if (con == 3) result = (itest >= sval);
			if (con == 4) result = (itest <= sval);
			if (con == 5) result = (itest != sval);
			if (result)
			{
				Select(i);
				selected++;
			}
		}
		break;
	case FT_Double:
		dval = atof(szValue);
		for (i = 0; i < entities; i++)
		{
			dtest = field->m_double[i];
			if (con == 0) result = (dtest == dval);
			if (con == 1) result = (dtest > dval);
			if (con == 2) result = (dtest < dval);
			if (con == 3) result = (dtest >= dval);
			if (con == 4) result = (dtest <= dval);
			if (con == 5) result = (dtest != dval);
			if (result)
			{
				Select(i);
				selected++;
			}
		}
		break;
	case FT_Boolean:
		bval = (atoi(szValue) != 0);
		for (i = 0; i < entities; i++)
		{
			btest = field->m_bool[i];
			if (con == 0) result = (btest == bval);
//			if (con == 1) result = (btest > ival);
//			if (con == 2) result = (btest < ival);
//			if (con == 3) result = (btest >= ival);
//			if (con == 4) result = (btest <= ival);
			if (con > 0 && con < 5)
				continue;
			if (con == 5) result = (btest != bval);
			if (result)
			{
				Select(i);
				selected++;
			}
		}
		break;
	}
	return selected;
}

void vtFeatureSet::DeleteSelected()
{
	int i, entities = NumEntities();
	for (i = 0; i < entities; i++)
	{
		if (IsSelected(i))
		{
			Select(i, false);
			SetToDelete(i);
		}
	}
	ApplyDeletion();
}

void vtFeatureSet::SetToDelete(int iFeature)
{
	m_Features[iFeature]->flags |= FF_DELETE;
}

int vtFeatureSet::ApplyDeletion()
{
	int entities = NumEntities();

	int target = 0;
	int deleted = 0;
	int newtotal = entities;
	for (int i = 0; i < entities; i++)
	{
		if ((m_Features[i]->flags & FF_DELETE))
		{
			delete m_Features[i];
			newtotal--;
			deleted++;
		}
		else
		{
			if (target != i)
			{
				CopyEntity(i, target);
				m_Features[target] = m_Features[i];
			}
			target++;
		}
	}
	SetNumEntities(newtotal);
	return deleted;
}

void vtFeatureSet::CopyEntity(uint from, uint to)
{
	// copy geometry
	CopyGeometry(from, to);

	// copy record fields
	for (uint i = 0; i < m_fields.GetSize(); i++)
		m_fields[i]->CopyValue(from, to);
}

void vtFeatureSet::DePickAll()
{
	int i, entities = NumEntities();
	for (i = 0; i < entities; i++)
		m_Features[i]->flags &= ~FF_PICKED;
}


/////////////////////////////////////////////////////////////////////////////
// Data Fields

Field *vtFeatureSet::GetField(const char *name)
{
	int i, num = m_fields.GetSize();
	for (i = 0; i < num; i++)
	{
		if (!m_fields[i]->m_name.CompareNoCase(name))
			return m_fields[i];
	}
	return NULL;
}

/**
 * Return the index of the field with the given name, or -1 if no field
 * with that name was found.
 */
int vtFeatureSet::GetFieldIndex(const char *name) const
{
	uint i, num = m_fields.GetSize();
	for (i = 0; i < num; i++)
	{
		if (!m_fields[i]->m_name.CompareNoCase(name))
			return i;
	}
	return -1;
}

/**
 * Add a data field to this featureset.  The field contain a value of any type,
 * for every entity.
 *
 * \param name Name of the new field.
 * \param ftype Type of the new field.
 * \param string_length For backward compatibility with the old SHP/DBF file formats,
 *  this is the maximum length of a string, for string fields.  It has no effect
 *  unless you save this featureset to a SHP/DBF file.
 */
int vtFeatureSet::AddField(const char *name, FieldType ftype, int string_length)
{
	Field *f = new Field(name, ftype);
	if (ftype == FT_Integer)
	{
		// 10 is the maximum number of digits that Shapelib will recognize as
		//  an integer.  More than that and it assume it's a FTDouble.
		f->m_width = 10;
		f->m_decimals = 0;
	}
	else if (ftype == FT_Short)
	{
		f->m_width = 6;
		f->m_decimals = 0;
	}
	else if (ftype == FT_Float)
	{
		f->m_width = 8;
		f->m_decimals = 6;
	}
	else if (ftype == FT_Double)
	{
		f->m_width = 12;
		f->m_decimals = 12;
	}
	else if (ftype == FT_Boolean)
	{
		f->m_width = 1;
		f->m_decimals = 0;
	}
	else if (ftype == FT_String)
	{
		f->m_width = string_length;
		f->m_decimals = 0;
	}
	else
	{
		VTLOG("Attempting to add field '%s' of type 'invalid', adding an integer field instead.\n", name, ftype);
		f->m_type = FT_Integer;
		f->m_width = 1;
		f->m_decimals = 0;
	}
	int field_index = m_fields.Append(f);

	// The new field should match the number of records
	f->SetNumRecords(NumEntities());

	return field_index;
}

int vtFeatureSet::AddRecord()
{
	int recs=-1;
	for (uint i = 0; i < m_fields.GetSize(); i++)
	{
		recs = m_fields[i]->AddRecord();
	}

	vtFeature *f = new vtFeature;
	f->flags = 0;
	m_Features.push_back(f);

	return recs;
}

void vtFeatureSet::DeleteFields()
{
	int count = m_fields.GetSize();
	for (int i = 0; i < count; i++)
	{
		Field *field = m_fields[i];
		delete field;
	}
	m_fields.SetSize(0);
}

void vtFeatureSet::SetValue(uint record, uint field, const char *value)
{
	m_fields[field]->SetValue(record, value);
}

void vtFeatureSet::SetValue(uint record, uint field, int value)
{
	m_fields[field]->SetValue(record, value);
}

void vtFeatureSet::SetValue(uint record, uint field, double value)
{
	m_fields[field]->SetValue(record, value);
}

void vtFeatureSet::SetValue(uint record, uint field, bool value)
{
	m_fields[field]->SetValue(record, value);
}

void vtFeatureSet::GetValueAsString(uint iRecord, uint iField, vtString &str) const
{
	if (iField >= m_fields.GetSize())
	{
		VTLOG("FeatureSet '%s' has %d fields, no field %d\n", (const char *)m_strFilename, m_fields.GetSize(), iField);
		return;
	}
	Field *field = m_fields[iField];
	field->GetValueAsString(iRecord, str);
}

void vtFeatureSet::SetValueFromString(uint iRecord, uint iField, const vtString &str)
{
	Field *field = m_fields[iField];
	field->SetValueFromString(iRecord, str);
}

void vtFeatureSet::SetValueFromString(uint iRecord, uint iField, const char *str)
{
	Field *field = m_fields[iField];
	field->SetValueFromString(iRecord, str);
}

int vtFeatureSet::GetIntegerValue(uint iRecord, uint iField) const
{
	Field *field = m_fields[iField];
	return field->m_int[iRecord];
}

short vtFeatureSet::GetShortValue(uint iRecord, uint iField) const
{
	Field *field = m_fields[iField];
	short val;
	field->GetValue(iRecord, val);
	return val;
}

float vtFeatureSet::GetFloatValue(uint iRecord, uint iField) const
{
	Field *field = m_fields[iField];
	float val;
	field->GetValue(iRecord, val);
	return val;
}

double vtFeatureSet::GetDoubleValue(uint iRecord, uint iField) const
{
	Field *field = m_fields[iField];
	double val;
	field->GetValue(iRecord, val);
	return val;
}

bool vtFeatureSet::GetBoolValue(uint iRecord, uint iField) const
{
	Field *field = m_fields[iField];
	return field->m_bool[iRecord];
}

vtFeature *vtFeatureSet::GetFirstSelectedFeature() const
{
	for (uint i = 0; i < m_Features.size(); i++)
	{
		if (IsSelected(i))
			return m_Features[i];
	}
	return NULL;
}

/////////////////////////////////////////////////

//
// Fields
//
Field::Field(const char *name, FieldType ftype)
{
	m_name = name;
	m_type = ftype;
}

Field::~Field()
{
}

void Field::SetNumRecords(int iNum)
{
	switch (m_type)
	{
	case FT_Boolean: m_bool.SetSize(iNum);	break;
	case FT_Short: m_short.SetSize(iNum);	break;
	case FT_Integer: m_int.SetSize(iNum);	break;
	case FT_Float:	m_float.SetSize(iNum);	break;
	case FT_Double:	m_double.SetSize(iNum);	break;
	case FT_String: m_string.resize(iNum);	break;
	case FT_Unknown: break;
	}
}

int Field::AddRecord()
{
	int index = 0;
	switch (m_type)
	{
	case FT_Boolean: return	m_bool.Append(false);	break;
	case FT_Short:	return	m_short.Append(0);		break;
	case FT_Integer: return	m_int.Append(0);		break;
	case FT_Float:	return	m_float.Append(0.0f);	break;
	case FT_Double:	return	m_double.Append(0.0);	break;
	case FT_String:
		index = m_string.size();
		m_string.push_back(vtString(""));
		return index;
	case FT_Unknown: break;
	}
	return -1;
}

void Field::SetValue(uint record, const char *value)
{
	if (m_type != FT_String)
		return;
	m_string[record] = value;
}

void Field::SetValue(uint record, int value)
{
	if (m_type == FT_Integer)
		m_int[record] = value;
	else if (m_type == FT_Short)
		m_short[record] = (short) value;
	else if (m_type == FT_Double)
		m_double[record] = value;
	else if (m_type == FT_Float)
		m_float[record] = (float) value;
}

void Field::SetValue(uint record, double value)
{
	if (m_type == FT_Double)
		m_double[record] = value;
	else if (m_type == FT_Float)
		m_float[record] = (float) value;
	else if (m_type == FT_Integer)
		m_int[record] = (int) value;
	else if (m_type == FT_Short)
		m_short[record] = (short) value;
}

void Field::SetValue(uint record, bool value)
{
	if (m_type == FT_Boolean)
		m_bool[record] = value;
	else if (m_type == FT_Integer)
		m_int[record] = (int) value;
	else if (m_type == FT_Short)
		m_short[record] = (short) value;
}

void Field::GetValue(uint record, vtString &string)
{
	if (m_type != FT_String)
		return;
	string = m_string[record];
}

void Field::GetValue(uint record, short &value)
{
	if (m_type == FT_Short)
		value = m_short[record];
	else if (m_type == FT_Integer)
		value = (short) m_int[record];
	else if (m_type == FT_Double)
		value = (short) m_double[record];
	else if (m_type == FT_Boolean)
		value = (short) m_bool[record];
}

void Field::GetValue(uint record, int &value)
{
	if (m_type == FT_Integer)
		value = m_int[record];
	else if (m_type == FT_Short)
		value = m_short[record];
	else if (m_type == FT_Double)
		value = (int) m_double[record];
	else if (m_type == FT_Boolean)
		value = (int) m_bool[record];
}

void Field::GetValue(uint record, float &value)
{
	if (m_type == FT_Float)
		value = m_float[record];
	else if (m_type == FT_Double)
		value = (float) m_double[record];
	else if (m_type == FT_Integer)
		value = (float) m_int[record];
	else if (m_type == FT_Short)
		value = (float) m_short[record];
}

void Field::GetValue(uint record, double &value)
{
	if (m_type == FT_Double)
		value = m_double[record];
	else if (m_type == FT_Float)
		value = m_float[record];
	else if (m_type == FT_Integer)
		value = (double) m_int[record];
	else if (m_type == FT_Short)
		value = (double) m_short[record];
}

void Field::GetValue(uint record, bool &value)
{
	if (m_type == FT_Boolean)
		value = m_bool[record];
	else if (m_type == FT_Integer)
		value = (m_int[record] != 0);
	else if (m_type == FT_Short)
		value = (m_short[record] != 0);
}

void Field::CopyValue(uint FromRecord, int ToRecord)
{
	if (m_type == FT_Integer)
		m_int[ToRecord] = m_int[FromRecord];
	else if (m_type == FT_Short)
		m_short[ToRecord] = m_short[FromRecord];
	else if (m_type == FT_Float)
		m_float[ToRecord] = m_float[FromRecord];
	else if (m_type == FT_Double)
		m_double[ToRecord] = m_double[FromRecord];

	// when dealing with strings, copy by value not reference, to
	// avoid memory tracking issues
	else if (m_type == FT_String)
		m_string[ToRecord] = m_string[FromRecord];

	else if (m_type == FT_Boolean)
		m_bool[ToRecord] = m_bool[FromRecord];
}

void Field::GetValueAsString(uint iRecord, vtString &str)
{
	switch (m_type)
	{
	case FT_String:
		str = m_string[iRecord];
		break;
	case FT_Integer:
		str.Format("%d", m_int[iRecord]);
		break;
	case FT_Short:
		str.Format("%d", m_short[iRecord]);
		break;
	case FT_Float:
		str.Format("%f", m_float[iRecord]);
		break;
	case FT_Double:
		str.Format("%lf", m_double[iRecord]);
		break;
	case FT_Boolean:
		str = m_bool[iRecord] ? "true" : "false";
		break;
	case FT_Unknown:
		break;
	}
}

void Field::SetValueFromString(uint iRecord, const vtString &str)
{
	const char *cstr = str;
	SetValueFromString(iRecord, cstr);
}

void Field::SetValueFromString(uint iRecord, const char *str)
{
	int i;
	double d;
	float f;

	switch (m_type)
	{
	case FT_String:
		if (iRecord < (int) m_string.size())
			m_string[iRecord] = str;
		else
			m_string.push_back(vtString(str));
		break;
	case FT_Integer:
		i = atoi(str);
		if (iRecord < m_int.GetSize())
			m_int[iRecord] = i;
		else
			m_int.Append(i);
		break;
	case FT_Short:
		i = atoi(str);
		if (iRecord < m_short.GetSize())
			m_short[iRecord] = (short) i;
		else
			m_short.Append((short) i);
		break;
	case FT_Float:
		f = (float) atof(str);
		if (iRecord < m_float.GetSize())
			m_float[iRecord] = f;
		else
			m_float.Append(f);
		break;
	case FT_Double:
		d = atof(str);
		if (iRecord < m_double.GetSize())
			m_double[iRecord] = d;
		else
			m_double.Append(d);
		break;
	case FT_Boolean:
		if (!strcmp(str, "true"))
			m_bool[iRecord] = true;
		else
			m_bool[iRecord] = false;
		break;
	case FT_Unknown:
		break;
	}
}


/////////////////////////////////////////////////////////////////////////////
// Helpers

const char *DescribeFieldType(FieldType type)
{
	switch (type)
	{
	case FT_Boolean: return "Boolean";
	case FT_Integer: return "Integer";
	case FT_Short: return "Short";
	case FT_Float: return "Float";
	case FT_Double: return "Double";
	case FT_String: return "String";
	case FT_Unknown:
	default:
		return "Unknown";
	}
}

const char *DescribeFieldType(DBFFieldType type)
{
	switch (type)
	{
	case FTString: return "String";
	case FTInteger: return "Integer";
	case FTDouble: return "Double";
	case FTLogical: return "Logical";
	case FTInvalid:
	default:
		return "Unknown";
	}
}

DBFFieldType ConvertFieldType(FieldType type)
{
	switch (type)
	{
	case FT_Boolean: return FTLogical;
	case FT_Integer: return FTInteger;
	case FT_Short: return FTInteger;
	case FT_Float: return FTDouble;
	case FT_Double: return FTDouble;
	case FT_String: return FTString;
	case FT_Unknown:
	default:
		return FTInvalid;
	}
}

FieldType ConvertFieldType(DBFFieldType type)
{
	switch (type)
	{
	case FTLogical: return FT_Boolean;
	case FTInteger: return FT_Integer;
	case FTDouble:	return FT_Double;
	case FTString:  return FT_String;
	case FTInvalid:
	default:
		return FT_Unknown;
	}
}

/**
 * Convert a Shapelib geometry type to an OGR type.
 */
OGRwkbGeometryType ShapelibToOGR(int nSHPType)
{
	switch (nSHPType)
	{
	case SHPT_NULL: return wkbNone;
	case SHPT_POINT: return wkbPoint;
	case SHPT_ARC: return wkbLineString;
	case SHPT_POLYGON: return wkbPolygon;
	case SHPT_MULTIPOINT: return wkbMultiPoint;
	case SHPT_POINTZ: return wkbPoint25D;
	case SHPT_ARCZ: return wkbLineString25D;
	case SHPT_POLYGONZ: return wkbPolygon25D;
	case SHPT_MULTIPOINTZ: return wkbMultiPoint25D;

	// the following are unknown
	case SHPT_MULTIPATCH: return wkbUnknown;
	case SHPT_POINTM: return wkbUnknown;
	case SHPT_ARCM: return wkbUnknown;
	case SHPT_POLYGONM: return wkbUnknown;
	case SHPT_MULTIPOINTM: return wkbUnknown;
	}
	return wkbUnknown;
}

/**
 * Convert a OGR geometry type to an Shapelib type.
 */
int OGRToShapelib(OGRwkbGeometryType eGeomType)
{
	switch (eGeomType)
	{
	// some of the following are guesses
	case wkbUnknown: return SHPT_NULL;
	case wkbPoint: return SHPT_POINT;
	case wkbLineString: return SHPT_ARC;
	case wkbPolygon: return SHPT_POLYGON;
	case wkbMultiPoint: return SHPT_MULTIPOINT;
	case wkbMultiLineString: return SHPT_ARCM;
	case wkbMultiPolygon: return SHPT_POLYGONM;

	case wkbGeometryCollection: return SHPT_NULL;
	case wkbNone: return SHPT_NULL;

	case wkbPoint25D: return SHPT_POINTZ;
	case wkbLineString25D: return SHPT_ARCZ;
	case wkbPolygon25D: return SHPT_POLYGONZ;

	case wkbMultiPoint25D: return SHPT_MULTIPOINTZ;
	case wkbMultiLineString25D: return SHPT_NULL;
	case wkbGeometryCollection25D: return SHPT_NULL;
	}
	return SHPT_NULL;
}

vtString MakeDBFName(const char *filename)
{
	vtString fname = filename;
	fname = fname.Left(fname.GetLength() - 4);
	fname += ".dbf";
	return fname;
}

//
// Extract an integer with known character length from a string.
//
int GetIntFromString(const char *buf, int len)
{
	char copy[32];
	strncpy(copy, buf, len);
	copy[len] = 0;
	return atoi(copy);
}

/**
 * Test if a geometry type is 2D (x, y) or 3D (x, y, z) - also known as "2.5D".
 */
bool GeometryTypeIs3D(OGRwkbGeometryType type)
{
	switch (type)
	{
	case wkbPoint:
	case wkbLineString:
	case wkbPolygon:
	case wkbMultiPoint:
	case wkbMultiLineString:
	case wkbMultiPolygon:
		return false;
	case wkbPoint25D:
	case wkbLineString25D:
	case wkbPolygon25D:
	case wkbMultiPoint25D:
	case wkbMultiLineString25D:
	case wkbMultiPolygon25D:
		return true;
	case wkbNone:
	case wkbUnknown:
	default:
		return false;
	}
}

vtString GetShapeTypeName(int nShapeType)
{
	switch (nShapeType)
	{
	case SHPT_NULL:			return "NULL"; break;
	case SHPT_POINT:		return "POINT"; break;
	case SHPT_ARC:			return "ARC"; break;
	case SHPT_POLYGON:		return "POLYGON"; break;
	case SHPT_MULTIPOINT:	return "MULTIPOINT"; break;
	case SHPT_POINTZ:		return "POINTZ"; break;
	case SHPT_ARCZ:			return "ARCZ"; break;
	case SHPT_POLYGONZ:		return "POLYGONZ"; break;
	case SHPT_MULTIPOINTZ:	return "MULTIPOINTZ"; break;
	case SHPT_POINTM:		return "POINTM"; break;
	case SHPT_ARCM:			return "ARCM"; break;
	case SHPT_POLYGONM:		return "POLYGONM"; break;
	case SHPT_MULTIPOINTM:	return "MULTIPOINTM"; break;
	case SHPT_MULTIPATCH:	return "MULTIPATCH"; break;
	}
	return "Unknown";
}

/**
 * When using Shapelib to read a Shapefile, this method will convert
 * a shp polygon object (SHPT_POLYGON or SHPT_POLYGONZ) into a
 * DPolygon2.
 */
bool SHPToDPolygon2(SHPObject *pObj, DPolygon2 &dpoly)
{
	// Beware: it is possible for the shape to not actually have vertices, or
	//  to have less than the minimum needed to define a polygon.  Ignore any
	//  such degenerate cases
	if (pObj->nVertices < 3)
		return false;

	DLine2 dline;

	// Convert and store each part - each is a polyline.  The first is the
	//  'outer' ring, any subsequent parts are 'inside' rings.
	for (int part = 0; part < pObj->nParts; part++)
	{
		int start, end;

		start = pObj->panPartStart[part];
		if (part+1 < pObj->nParts)
			end = pObj->panPartStart[part+1]-1;
		else
			end = pObj->nVertices-1;

		// SHP files always duplicate the first point of each ring (part)
		// which we can ignore
		end--;

		dline.SetSize(end - start + 1);
		for (int j = start; j <= end; j++)
			dline.SetAt(j-start, DPoint2(pObj->padfX[j], pObj->padfY[j]));

		dpoly.push_back(dline);
	}
	return true;
}

void DPolygon2ToOGR(const DPolygon2 &dp, OGRPolygon &op)
{
	op.empty();
	for (uint ringnum = 0; ringnum < dp.size(); ringnum++)
	{
		const DLine2 &ring = dp[ringnum];
		OGRLinearRing *poNewRing = new OGRLinearRing;

		uint numpoints = ring.GetSize();
		poNewRing->setNumPoints(numpoints);

		OGRPoint p;
		for (uint i = 0; i < numpoints; i++)
		{
			p.setX(ring[i].x);
			p.setY(ring[i].y);
			poNewRing->setPoint(i, &p);
		}
		op.addRingDirectly(poNewRing);
	}
}

void OGRToDPolygon2(const OGRPolygon &op, DPolygon2 &dp)
{
	int irings = op.getNumInteriorRings();
	dp.resize(1 + irings);

	const OGRLinearRing *outer = op.getExteriorRing();
	DLine2 &line = dp[0];

	// OGRPolygon's convention is to duplicate the first point on each ring
	// So omit this point when we convert
	line.SetSize(outer->getNumPoints()-1);
	for (int i = 0; i < outer->getNumPoints()-1; i++)
		line[i].Set(outer->getX(i), outer->getY(i));

	for (int ring = 0; ring < irings; ring++)
	{
		const OGRLinearRing *inner = op.getInteriorRing(ring);
		line = dp[1+ring];
		line.SetSize(inner->getNumPoints()-1);
		for (int i = 0; i < inner->getNumPoints()-1; i++)
			line[i].Set(inner->getX(i), inner->getY(i));
	}
}
