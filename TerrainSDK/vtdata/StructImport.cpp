//
// Import methods for the vtStructureArray class.
//
// Copyright (c) 2003-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "Building.h"
#include "Features.h"
#include "Fence.h"
#include "MaterialDescriptor.h"
#include "PolyChecker.h"
#include "StructArray.h"
#include "vtLog.h"

#include "shapelib/shapefil.h"
#include "ogrsf_frmts.h"

//
// Helper: find the index of a field in a DBF file, given the name of the field.
// Returns -1 if not found.
//
int FindDBField(DBFHandle db, const char *field_name)
{
	int count = DBFGetFieldCount(db);
	for (int i = 0; i < count; i++)
	{
		int pnWidth, pnDecimals;
		char pszFieldName[80];
		DBFGetFieldInfo(db, i, pszFieldName, &pnWidth, &pnDecimals);
		if (!stricmp(field_name, pszFieldName))
			return i;
	}
	return -1;
}

#define BCFVERSION_SUPPORTED	1.1f

bool vtStructureArray::ReadBCF(const char* pathname)
{
	FILE* fp;
	if ( (fp = vtFileOpen(pathname, "rb")) == NULL )
		return false;

	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	char buf[4];
	if (fread(buf, 3, 1, fp) != 1)
		return false;
	if (strncmp(buf, "bcf", 3))
	{
		// not current bcf, try reading old format
		rewind(fp);
		return ReadBCF_Old(fp);
	}

	float version;
	if (fscanf(fp, "%f\n", &version) != 1)
		return false;

	if (version < BCFVERSION_SUPPORTED)
	{
		// too old, unsupported version
		fclose(fp);
		return false;
	}

	int quiet;
	int zone = 1;
	if (version == 1.2f)
	{
		quiet = fscanf(fp, "utm_zone %d\n", &zone);
	}
	m_crs.SetUTMZone(zone);

	int i, j, count;
	DPoint2 p;
	int points;
	char key[80];
	RGBi color;
	float fRotation;

	quiet = fscanf(fp, "buildings %d\n", &count);
	for (i = 0; i < count; i++)	//for each building
	{
		vtBuilding *bld = AddNewBuilding();

		int type;
		quiet = fscanf(fp, "type %d\n", &type);

		int stories = 1;
		DPoint2 loc;
		fRotation = 0.0f;
		while (1)
		{
			long start = ftell(fp);

			int result = fscanf(fp, "%s ", key);
			if (result == -1)
				break;

			if (!strcmp(key, "type"))
			{
				fseek(fp, start, SEEK_SET);
				break;
			}
			if (!strcmp(key, "loc"))
			{
				quiet = fscanf(fp, "%lf %lf\n", &loc.x, &loc.y);
				bld->SetRectangle(loc, 10, 10);
			}
			else if (!strcmp(key, "rot"))
			{
				quiet = fscanf(fp, "%f\n", &fRotation);
			}
			else if (!strcmp(key, "stories"))
			{
				quiet = fscanf(fp, "%d\n", &stories);

				// Fix bad values that might be encountered
				if (stories < 1 || stories > 20) stories = 1;
				bld->SetNumStories(stories);
			}
			else if (!strcmp(key, "color"))
			{
				quiet = fscanf(fp, "%hd %hd %hd\n", &color.r, &color.g, &color.b);
				bld->SetColor(BLD_BASIC, color);
			}
			else if (!strcmp(key, "color_roof"))
			{
				quiet = fscanf(fp, "%hd %hd %hd\n", &color.r, &color.g, &color.b);
				bld->SetColor(BLD_ROOF, color);
			}
			else if (!strcmp(key, "size"))
			{
				float w, d;
				quiet = fscanf(fp, "%f %f\n", &w, &d);
				bld->SetRectangle(loc, w, d, fRotation);
			}
			else if (!strcmp(key, "radius"))
			{
				float rad;
				quiet = fscanf(fp, "%f\n", &rad);
				bld->SetCircle(loc, rad);
			}
			else if (!strcmp(key, "footprint"))
			{
				DLine2 dl;
				quiet = fscanf(fp, "%d", &points);
				dl.SetSize(points);

				for (j = 0; j < points; j++)
				{
					quiet = fscanf(fp, " %lf %lf", &p.x, &p.y);
					dl.SetAt(j, p);
				}
				quiet = fscanf(fp, "\n");
				bld->SetFootprint(0, dl);
			}
			else if (!strcmp(key, "trim"))
			{
				int trim;
				quiet = fscanf(fp, "%d\n", &trim);
			}
			else if (!strcmp(key, "roof_type"))
			{
				int rt;
				quiet = fscanf(fp, "%d\n", &rt);
				bld->SetRoofType((RoofType) rt);
			}
		}
	}
	fclose(fp);
	return true;
}

bool vtStructureArray::ReadBCF_Old(FILE *fp)
{
	int ncoords;
	int num = fscanf(fp, "%d\n", &ncoords);
	if (num != 1)
		return false;

	DPoint2 point;
	for (int i = 0; i < ncoords; i++)
	{
		int quiet = fscanf(fp, "%lf %lf\n", &point.x, &point.y);
		vtBuilding *bld = AddNewBuilding();
		bld->SetRectangle(point, 10, 10);
	}

	fclose(fp);
	return true;
}

/**
 * Import structure information from a Shapefile.
 *
 * \param pathname A resolvable filename of a Shapefile (.shp)
 * \param opt	Options which specify how to import structures from the file.
 * \param progress_callback If supplied, this function will be called back
 *				with a value of 0 to 100 as the operation progresses.
 *
 * \return True if successful.
 */
bool vtStructureArray::ReadSHP(const char *pathname, StructImportOptions &opt,
							   bool progress_callback(int))
{
	VTLOG("vtStructureArray::ReadSHP(%s)\n", pathname);

	// SHPOpen doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(pathname);

	SHPHandle hSHP = SHPOpen(fname_local, "rb");
	if (hSHP == NULL)
		return false;

	int		nEntities, nShapeType;
	DPoint2 point;
	DLine2	line;
	int		i, j;
	int		field_height = -1;
	int		field_roof = -1;
	int		field_filename = -1;
	int		field_itemname = -1;
	int		field_scale = -1;
	int		field_rotation = -1;
	PolyChecker PolyChecker;

	SHPGetInfo(hSHP, &nEntities, &nShapeType, NULL, NULL);

	// Open DBF File & Get DBF Info:
	DBFHandle db = DBFOpen(fname_local, "rb");

	// Make sure that entities are of the expected type
	if (opt.type == ST_BUILDING)
	{
		if (nShapeType != SHPT_POINT &&
			nShapeType != SHPT_POINTZ &&
			nShapeType != SHPT_POLYGON &&
			nShapeType != SHPT_ARC &&
			nShapeType != SHPT_POLYGONZ)
			return false;
		if (db != NULL)
		{
			// Check for field with number of stories
			field_height = FindDBField(db, opt.m_strFieldNameHeight);
			// Check for field with number of stories
			field_roof = FindDBField(db, opt.m_strFieldNameRoof);
		}
	}
	if (opt.type == ST_INSTANCE)
	{
		if (nShapeType != SHPT_POINT &&
			nShapeType != SHPT_POINTZ)
			return false;
		if (db == NULL)
			return false;

		field_filename = FindDBField(db, (const char *)opt.m_strFieldNameFile);
		field_itemname = FindDBField(db, "itemname");
		if (field_filename == -1 && field_itemname == -1)
			return false;

		field_scale = FindDBField(db, "scale");
		field_rotation = FindDBField(db, "rotation");
	}
	if (opt.type == ST_LINEAR)
	{
		if (nShapeType != SHPT_ARC && nShapeType != SHPT_POLYGON)
			return false;
	}

	int iEmptyEntities = 0;
	for (i = 0; i < nEntities; i++)
	{
		if (progress_callback != NULL && (i & 0xff) == 0)
			progress_callback(i * 100 / nEntities);

		SHPObject *psShape = SHPReadObject(hSHP, i);
		if (!psShape)
		{
			iEmptyEntities++;
			continue;
		}

		if (opt.bInsideOnly)
		{
			// do exclusion of shapes outside the indicated extents
			if (psShape->dfXMax < opt.rect.left ||
				psShape->dfXMin > opt.rect.right ||
				psShape->dfYMax < opt.rect.bottom ||
				psShape->dfYMin > opt.rect.top)
			{
				SHPDestroyObject(psShape);
				continue;
			}
		}

		int num_points = psShape->nVertices;
		if (opt.type == ST_BUILDING)
		{
			vtBuilding *bld = AddNewBuilding();
			if (nShapeType == SHPT_POINT || nShapeType == SHPT_POINTZ)
			{
				point.x = psShape->padfX[0];
				point.y = psShape->padfY[0];
				bld->SetRectangle(point, 10, 10);	// default size
			}
			if (nShapeType == SHPT_POLYGON || nShapeType == SHPT_POLYGONZ ||
				nShapeType == SHPT_ARC)
			{
				DPolygon2 foot;
				SHPToDPolygon2(psShape, foot);

				// test clockwisdom and reverse if necessary
				if (PolyChecker.IsClockwisePolygon(foot[0]))
					foot.ReverseOrder();

				bld->SetFootprint(0, foot);
				// Give it a flat roof with the same footprint
				bld->SetFootprint(1, foot);
				bld->SetRoofType(ROOF_FLAT);
			}

			// attempt to get height from the DBF
			int stories;
			if (field_height != -1)
			{
				double height = DBFReadDoubleAttribute(db, i, field_height);
				switch (opt.m_HeightType)
				{
				case StructImportOptions::STORIES:
					stories = (int) height;
					if (stories >= 1)
						bld->SetNumStories(stories);
					break;
				case StructImportOptions::FEET:
					height = height * 0.3048;
				case StructImportOptions::METERS:
					stories = (int) (height / 3.2);
					if (stories < 1)
						stories = 1;
					bld->SetNumStories((int) stories);
					bld->GetLevel(0)->m_fStoryHeight = (float) (height / stories);
					break;
				case StructImportOptions::FEETNOSTORIES:
					height = height * 0.3048;
				case StructImportOptions::METERSNOSTORIES:
					stories = (int) (height / 3.2);
					if (stories < 1)
						stories = 1;
					bld->SetNumStories(1);
					bld->GetLevel(0)->m_fStoryHeight = (float) (height);
					break;
				}
			}
			// if DBF didn't have height info, get it from default building
			bool bDoHeight = (field_height == -1);

			// Apply materials, edge slopes and other things from the default
			//  building, if there is one.
			vtBuilding *pDefBld = GetClosestDefault(bld);
			if (pDefBld)
				bld->CopyStyleFrom(pDefBld, bDoHeight);

			// Apply explicit colors, if they were specified.
			if (opt.m_bFixedColor)
			{
				bld->SetColor(BLD_BASIC, opt.m_BuildingColor);
				bld->SetColor(BLD_ROOF, opt.m_RoofColor);
			}

			// Now deal with roof type, which the user might have specified.
			if (field_roof != -1)
			{
				vtString type = DBFReadStringAttribute(db, i, field_roof);
				if (!type.CompareNoCase("flat") || !type.CompareNoCase("plat"))
					bld->SetRoofType(ROOF_FLAT);
				if (!type.CompareNoCase("shed") || !type.CompareNoCase("hangar"))
					bld->SetRoofType(ROOF_SHED, opt.m_iSlope);
				if (!type.CompareNoCase("gable") || !type.CompareNoCase("pignon"))
					bld->SetRoofType(ROOF_GABLE, opt.m_iSlope);
				if (!type.CompareNoCase("hip") || !type.CompareNoCase("arete"))
					bld->SetRoofType(ROOF_HIP, opt.m_iSlope);
			}
			// Apply explicit roof type, if specified.
			else if (opt.m_eRoofType != ROOF_UNKNOWN)
				bld->SetRoofType(opt.m_eRoofType, opt.m_iSlope);
		}
		if (opt.type == ST_INSTANCE)
		{
			vtStructInstance *inst = AddNewInstance();
			inst->SetPoint(DPoint2(psShape->padfX[0], psShape->padfY[0]));
			// attempt to get properties from the DBF
			const char *string;
			vtTag tag;
			if (field_filename != -1)
			{
				string = DBFReadStringAttribute(db, i, field_filename);
				tag.name = "filename";
				tag.value = string;
				inst->AddTag(tag);
			}
			if (field_itemname != -1)
			{
				string = DBFReadStringAttribute(db, i, field_itemname);
				tag.name = "itemname";
				tag.value = string;
				inst->AddTag(tag);
			}
			if (field_scale != -1)
			{
				double scale = DBFReadDoubleAttribute(db, i, field_scale);
				if (scale != 1.0)
				{
					tag.name = "scale";
					tag.value.Format("%lf", scale);
					inst->AddTag(tag);
				}
			}
			if (field_rotation != -1)
			{
				double rotation = DBFReadDoubleAttribute(db, i, field_rotation);
				inst->SetRotation((float)rotation / 180.0f * PIf);
			}
		}
		if (opt.type == ST_LINEAR)
		{
			vtFence *fen = AddNewFence();
			for (j = 0; j < num_points; j++)
			{
				point.x = psShape->padfX[j];
				point.y = psShape->padfY[j];
				fen->AddPoint(point);
			}
		}
		SHPDestroyObject(psShape);
	}
	if (db != NULL)
		DBFClose(db);
	SHPClose(hSHP);

	if (iEmptyEntities)
		VTLOG("\t Warning: %d of %d entities were empty.\n", iEmptyEntities, nEntities);

	VTLOG1("\tReadSHP done.\n");
	return true;
}

void vtStructureArray::AddElementsFromOGR(GDALDataset *pDatasource,
		StructImportOptions &opt, bool progress_callback(int))
{
	if (opt.m_strLayerName == "")
		AddElementsFromOGR_SDTS(pDatasource, progress_callback);
	else
		AddElementsFromOGR_RAW(pDatasource, opt, progress_callback);
}

void vtStructureArray::AddElementsFromOGR_SDTS(GDALDataset *pDatasource,
		bool progress_callback(int))
{
	int i, j, feature_count, count;
	OGRLayer		*pLayer;
	OGRFeature		*pFeature;
	OGRGeometry		*pGeom;
	OGRPoint		*pPoint;
	OGRPolygon		*pPolygon;
	vtBuilding		*pBld;
	DPoint2			point;
	DLine2 foot;
	OGRLinearRing *ring;

	int num_layers = pDatasource->GetLayerCount();

	//
	// Iterate through the layers looking
	// Test for known USGS SDTS DLG layer names
	// Treat unknown ones as containing feature polygons
	//
	for (i = 0; i < num_layers; i++)
	{
		pLayer = pDatasource->GetLayer(i);
		if (!pLayer)
			continue;

		feature_count = (int) pLayer->GetFeatureCount();
		pLayer->ResetReading();

		OGRFeatureDefn *pLayerDefn = pLayer->GetLayerDefn();
		if (!pLayerDefn)
			return;

		const char *layer_name = pLayerDefn->GetName();

		// Nodes
		if (!strcmp(layer_name, "NO01"))
		{
			// only 1 field: RCID - not enough to do anything useful
		}
		else if (!strcmp(layer_name, "NE01"))
		{
			count = 0;
			while( (pFeature = pLayer->GetNextFeature()) != NULL )
			{
				pGeom = pFeature->GetGeometryRef();
				if (!pGeom) continue;
				pPoint = (OGRPoint *) pGeom;
				pBld = AddNewBuilding();

				point.x = pPoint->getX();
				point.y = pPoint->getY();
				pBld->SetRectangle(point, 10, 10);
				pBld->SetNumStories(1);

				count++;
			}
		}
		// Lines
		else if (!strcmp(layer_name, "LE01"))
		{
			// only 3 field: RCID, SNID, ENID - not enough to do anything useful
		}
		// Areas (buildings, built-up areas, other areas like golf courses)
		else if (!strcmp(layer_name, "PC01"))
		{
			// get field indices
			int index_entity = pLayerDefn->GetFieldIndex("ENTITY_LABEL");

			count = 0;
			while( (pFeature = pLayer->GetNextFeature()) != NULL )
			{
				count++;
				progress_callback(count * 100 / feature_count);

				// Ignore non-entities
				if (!pFeature->IsFieldSet(index_entity))
					continue;

				// The "ENTITY_LABEL" contains the same information as the old
				// DLG classification.  First, try to use this field to guess
				// values such as number of lanes, etc.
				const char *str_entity = pFeature->GetFieldAsString(index_entity);
				int numEntity = atoi(str_entity);
				// int iMajorAttr = numEntity / 10000;
				int iMinorAttr = numEntity % 10000;

				int num_stories = 1; // Use this as a multiplier
				pBld = NULL;
				switch (iMinorAttr)
				{
				case 123:	// golf course
				case 150:	// built-up area
				case 420:	// cemetary
					break;
				case 402:	// church
				case 403:	// school
				case 405:	// courthouse
				case 406:	// post office
				case 407:	// city/town hall
				case 408:	// hospital
				case 409:	// prison
				case 412:	// customs building
				case 413:	// capitol
				case 414:	// community center
				case 415:	// muesum
				case 418:	// library
					num_stories = 2;
				case 400:	// building (general case)
					pBld = AddNewBuilding();
					break;
				}
				if (!pBld)
					continue;
				pGeom = pFeature->GetGeometryRef();
				if (!pGeom) continue;
				pPolygon = (OGRPolygon *) pGeom;

				ring = pPolygon->getExteriorRing();
				int num_points = ring->getNumPoints();

				foot.SetSize(num_points);
				for (j = 0; j < num_points; j++)
					foot.SetAt(j, DPoint2(ring->getX(j),
						ring->getY(j)));

				pBld->SetFootprint(0, foot);

				vtBuilding *pDefBld = GetClosestDefault(pBld);
				if (pDefBld)
					pBld->CopyStyleFrom(pDefBld, true);
				else
					pBld->SetNumStories(1);
			}
		}
	}
}


//#pragma message( "Look here for more things to do " __FILE__ )

// 1. Better copy syntax for vtStructure and its descendants
// 2. extra import fields for fences and structures.
// 3. Handle tags

void vtStructureArray::AddElementsFromOGR_RAW(GDALDataset *pDatasource,
		StructImportOptions &opt, bool progress_callback(int))
{
	OGRLayer		*pLayer;

	pLayer = pDatasource->GetLayerByName(opt.m_strLayerName);
	if (!pLayer)
		return;

	// Get the CRS (SpatialReference) from this layer
	OGRSpatialReference *pSpatialRef = pLayer->GetSpatialRef();
	if (pSpatialRef)
		m_crs.SetSpatialReference(pSpatialRef);

	switch (opt.type)
	{
		case ST_BUILDING:
			AddBuildingsFromOGR(pLayer, opt, progress_callback);
			break;
		case ST_LINEAR:
			AddLinearsFromOGR(pLayer, opt, progress_callback);
			break;
		case ST_INSTANCE:
			AddInstancesFromOGR(pLayer, opt, progress_callback);
			break;
		default:
			break;
	}
}

void vtStructureArray::AddBuildingsFromOGR(OGRLayer *pLayer,
		StructImportOptions &opt, bool progress_callback(int))
{
	int feature_count = (int) pLayer->GetFeatureCount();
	pLayer->ResetReading();

	OGRFeatureDefn *pLayerDefn = pLayer->GetLayerDefn();
	if (!pLayerDefn)
		return;

	int iHeightIndex = pLayerDefn->GetFieldIndex(opt.m_strFieldNameHeight);

	// Check for layers with known schemas
	const char *layer_name = pLayerDefn->GetName();
	SchemaType Schema = SCHEMA_UI;
	if (!strcmp(layer_name, "osgb:TopographicArea"))
		Schema = SCHEMA_OSGB_TOPO_AREA;
	else if (!strcmp(layer_name, "osgb:TopographicPoint"))
		Schema = SCHEMA_OSGB_TOPO_POINT;
	else if (!strcmp(layer_name, "TopographicArea"))
		Schema = SCHEMA_OSGB_TOPO_AREA;
	else if (!strcmp(layer_name, "TopographicPoint"))
		Schema = SCHEMA_OSGB_TOPO_POINT;
	else if (!strcmp(layer_name, "topographicarea"))
		Schema = SCHEMA_MAPINFO_OSGB_TOPO_AREA;
	else if (!strcmp(layer_name, "topographicpoint"))
		Schema = SCHEMA_MAPINFO_OSGB_TOPO_POINT;

	PolyChecker PolyChecker;
	int count = 0;
	OGRFeature	 *pFeature;
	int iFeatureCode;
	float fMinZ, fMaxZ, fTotalZ;
	float fZ;
	float fMin, fMax, fDiff, fElev;
	uint i, j;
	while( (pFeature = pLayer->GetNextFeature()) != NULL )
	{
		count++;
		progress_callback(count * 100 / feature_count);
		// Preprocess according to schema
		switch(Schema)
		{
			case SCHEMA_OSGB_TOPO_AREA:
			case SCHEMA_MAPINFO_OSGB_TOPO_AREA:
				// Get feature code
				if (Schema == SCHEMA_OSGB_TOPO_AREA)
					iFeatureCode = pFeature->GetFieldAsInteger(pLayerDefn->GetFieldIndex("featureCode"));
				else
					iFeatureCode = pFeature->GetFieldAsInteger(pLayerDefn->GetFieldIndex("FC"));

				// Skip things that are not buildings
				iFeatureCode = pFeature->GetFieldAsInteger(pLayerDefn->GetFieldIndex("osgb:featureCode"));
				switch(iFeatureCode)
				{
					// Just do polygons for the time being
					case 10021: // Building defined by area
					case 10062: // Glasshouse
					case 10185: // Generic structure
					case 10190: // Archway
					// case 10193: // Pylon
					case 10187: // Upper level of communication
					case 10025: // Buildings or structure
						break;
					default:
						continue;
				}
				break;
			default:
				break;
		}

		OGRGeometry *pGeom = pFeature->GetGeometryRef();
		if (!pGeom)
		{
			OGRFeature::DestroyFeature(pFeature);
			continue;
		}
		OGRwkbGeometryType GeometryType = pGeom->getGeometryType();

		// For the moment ignore multi polygons .. although we could treat
		// them as multiple buildings !!
		DPolygon2 footprint;
		OGRPolygon	 *pPolygon;
		OGRLineString *pLineString;
		uint line_points = 0;

		switch (wkbFlatten(GeometryType))
		{
			case wkbPolygon:
				pPolygon = (OGRPolygon *) pGeom;
				OGRToDPolygon2(*pPolygon, footprint);
				break;

			case wkbLineString:
				pLineString = (OGRLineString *) pGeom;
				line_points = pLineString->getNumPoints();

				// Ignore last point if it is the same as the first
				if (DPoint2(pLineString->getX(0), pLineString->getY(0)) ==
					DPoint2(pLineString->getX(line_points - 1), pLineString->getY(line_points - 1)))
					line_points--;

				footprint.resize(1);
				footprint[0].SetSize(line_points);
				fMaxZ = -1E9;
				fMinZ = 1E9;
				fTotalZ = 0;
				for (j = 0; j < line_points; j++)
				{
					fZ = (float)pLineString->getZ(j);
					if (fZ > fMaxZ)
						fMaxZ = fZ;
					if (fZ < fMinZ)
						fMinZ = fZ;
					fTotalZ += fZ;
					footprint[0].SetAt(j, DPoint2(pLineString->getX(j), pLineString->getY(j)));
				}
				break;

			case wkbPoint:
				{
				DPoint2 dPoint(((OGRPoint *)pGeom)->getX(), ((OGRPoint *)pGeom)->getY());

				footprint.resize(1);
				footprint[0].Append(dPoint + DPoint2(- DEFAULT_BUILDING_SIZE / 2, - DEFAULT_BUILDING_SIZE / 2));
				footprint[0].Append(dPoint + DPoint2(DEFAULT_BUILDING_SIZE / 2, - DEFAULT_BUILDING_SIZE / 2));
				footprint[0].Append(dPoint + DPoint2(DEFAULT_BUILDING_SIZE / 2, DEFAULT_BUILDING_SIZE / 2));
				footprint[0].Append(dPoint + DPoint2(- DEFAULT_BUILDING_SIZE / 2, DEFAULT_BUILDING_SIZE / 2));
				}
				break;

			default:
				OGRFeature::DestroyFeature(pFeature);
				continue;
		}

		const DLine2 &outer_ring = footprint[0];
		uint outer_ring_size = outer_ring.GetSize();

		if (opt.bInsideOnly)
		{
			// Exclude footprints outside the indicated extents
			for (j = 0; j < outer_ring_size; j++)
				if (!opt.rect.ContainsPoint(outer_ring[j]))
					break;
			if (j != outer_ring_size)
				continue;
		}

		vtBuilding *pBld = AddNewBuilding();
		if (!pBld)
			return;

		// Force footprint anticlockwise
		if (PolyChecker.IsClockwisePolygon(outer_ring))
			footprint.ReverseOrder();
		pBld->SetFootprint(0, footprint);

		vtBuilding *pDefBld = GetClosestDefault(pBld);
		if (pDefBld)
			pBld->CopyStyleFrom(pDefBld, true);
		else
			pBld->SetNumStories(1);

		// Set the correct height for the roof level if neccessary
		vtLevel *pLevel = pBld->GetLevel(pBld->NumLevels() - 1);
		pBld->SetRoofType(pLevel->GuessRoofType(), pLevel->GetEdge(0)->m_iSlope);

		// Modify the height of the building if neccessary
		if (iHeightIndex != -1)
		{
			float fTotalHeight = 0;
			float fScaleFactor;
			uint iNumLevels = pBld->NumLevels();
			RoofType eRoofType = pBld->GetRoofType();
			float fRoofHeight = pBld->GetLevel(iNumLevels - 1)->m_fStoryHeight;

			// If building has a roof I must exclude this from the calculation
			if (ROOF_UNKNOWN != eRoofType)
				iNumLevels--;
			else
				fRoofHeight = 0;

			for (i = 0; i < iNumLevels; i++)
				fTotalHeight += pBld->GetLevel(i)->m_fStoryHeight;

			fScaleFactor = ((float)pFeature->GetFieldAsDouble(iHeightIndex) - fRoofHeight)/fTotalHeight;
			for (i = 0; i < iNumLevels; i++)
				pBld->GetLevel(i)->m_fStoryHeight *= fScaleFactor;
		}

		// Add foundation
		if ((opt.bBuildFoundations) && (NULL != opt.pHeightField))
		{
			// Use the outer footprint of the lowest level
			fMin = 1E9;
			fMax = -1E9;
			for (uint v = 0; v < outer_ring_size; v++)
			{
				if (opt.pHeightField->FindAltitudeOnEarth(outer_ring[v], fElev))
				{
					if (fElev < fMin)
						fMin = fElev;
					if (fElev > fMax)
						fMax = fElev;
				}
			}
			fDiff = fMax - fMin;
			if (fDiff > MINIMUM_BASEMENT_SIZE)
			{
				// Create and add a foundation level
				vtLevel *pNewLevel = new vtLevel;
				pNewLevel->m_iStories = 1;
				pNewLevel->m_fStoryHeight = fDiff;
				pBld->InsertLevel(0, pNewLevel);
				pBld->SetFootprint(0, footprint);
				pNewLevel->SetEdgeMaterial(BMAT_NAME_PLAIN);
				pNewLevel->SetEdgeColor(RGBi(128, 128, 128));
			}
			else
				pBld->SetElevationOffset(fDiff);
		}
	}
}

void vtStructureArray::AddLinearsFromOGR(OGRLayer *pLayer,
		StructImportOptions &opt, bool progress_callback(int))
{
	int iFeatureCount;
	OGRFeatureDefn *pLayerDefn;
	const char *pLayerName;
	SchemaType eSchema = SCHEMA_UI;
	int iCount;
	int iFeatureCode;
	OGRFeature		*pFeature;
	OGRGeometry		*pGeom;
	OGRwkbGeometryType GeometryType;
	OGRLineString *pLineString;
	DLine2 FencePoints;
	int iNumPoints;
	int i;
	float fMinZ;
	float fMaxZ;
	float fTotalZ;
	float fZ;
	vtFence *pFence;
	vtFence *pDefaultFence;

	iFeatureCount = (int) pLayer->GetFeatureCount();
	pLayer->ResetReading();

	pLayerDefn = pLayer->GetLayerDefn();
	if (!pLayerDefn)
		return;

	pLayerName = pLayerDefn->GetName();

	int iHeightIndex = pLayerDefn->GetFieldIndex(opt.m_strFieldNameHeight);

	// Check for layers with known schemas
	if (!strcmp(pLayerName, "osgb:TopographicLine"))
		eSchema = SCHEMA_OSGB_TOPO_LINE;
	else if (!strcmp(pLayerName, "osgb:TopographicPoint"))
		eSchema = SCHEMA_OSGB_TOPO_POINT;

	iCount = 0;
	while((pFeature = pLayer->GetNextFeature()) != NULL )
	{
		iCount++;
		progress_callback(iCount * 100 / iFeatureCount);
		// Preprocess according to schema
		switch(eSchema)
		{
			case SCHEMA_OSGB_TOPO_LINE:
			case SCHEMA_OSGB_TOPO_POINT:
				// Skip things that are not linears
				iFeatureCode = pFeature->GetFieldAsInteger(pLayerDefn->GetFieldIndex("osgb:featureCode"));
				switch(iFeatureCode)
				{
					case 10045: // General feature - point
					case 10046: // General feature - line
						break;
					default:
						continue;
				}
				break;
			default:
				break;
		}

		pGeom = pFeature->GetGeometryRef();
		if (!pGeom)
			continue;
		GeometryType = pGeom->getGeometryType();

		if (wkbLineString != wkbFlatten(GeometryType))
			continue;

		pLineString = (OGRLineString *) pGeom;

		iNumPoints = pLineString->getNumPoints();

		FencePoints.SetSize(iNumPoints);
		fMaxZ = -1E9;
		fMinZ = 1E9;
		fTotalZ = 0;
		for (i = 0; i < iNumPoints; i++)
		{
			fZ = (float)pLineString->getZ(i);
			if (fZ > fMaxZ)
				fMaxZ = fZ;
			if (fZ < fMinZ)
				fMinZ = fZ;
			fTotalZ += fZ;
			FencePoints.SetAt(i, DPoint2(pLineString->getX(i), pLineString->getY(i)));
		}

		if (opt.bInsideOnly)
		{
			// Exclude fences outside the indicated extents
			for (i = 0; i < iNumPoints; i++)
				if (!opt.rect.ContainsPoint(FencePoints[i]))
					break;
			if (i != iNumPoints)
				continue;
		}

		pFence = AddNewFence();
		if (!pFence)
			return;

		pDefaultFence = GetClosestDefault(pFence);
		if (NULL != pDefaultFence)
			*pFence = *pDefaultFence;

		for (i = 0; i < iNumPoints; i++)
			pFence->AddPoint(FencePoints[i]);

		// Modify height of fence
		if (iHeightIndex != -1)
		{
			float f = (float)pFeature->GetFieldAsDouble(iHeightIndex);
			pFence->GetParams().m_fPostHeight = f;
			pFence->GetParams().m_fConnectTop = f;
		}
	}
}

void vtStructureArray::AddInstancesFromOGR(OGRLayer *pLayer,
		StructImportOptions &opt, bool progress_callback(int))
{
	int			iCount, iFeatureCount;
	SchemaType	eSchema = SCHEMA_UI;
	OGRFeature	*pFeature;
	OGRGeometry	*pGeom;
	int			iFilenameIndex = -1;

	iFeatureCount = (int) pLayer->GetFeatureCount();
	pLayer->ResetReading();

	OGRFeatureDefn *pLayerDefn = pLayer->GetLayerDefn();
	if (!pLayerDefn)
		return;

	const char *pLayerName = pLayerDefn->GetName();

	// Check for layers with known schemas
	if (!strcmp(pLayerName, "osgb:TopographicArea"))
		eSchema = SCHEMA_OSGB_TOPO_AREA;
	else if (!strcmp(pLayerName, "osgb:TopographicPoint"))
		eSchema = SCHEMA_OSGB_TOPO_POINT;
	else
		iFilenameIndex = pLayerDefn->GetFieldIndex(opt.m_strFieldNameFile);

	if (-1 == iFilenameIndex)
		return;

	int iFeatureCode;
	iCount = 0;
	while((pFeature = pLayer->GetNextFeature()) != NULL )
	{
		iCount++;
		progress_callback(iCount * 100 / iFeatureCount);
		// Preprocess according to schema
		switch(eSchema)
		{
			case SCHEMA_OSGB_TOPO_AREA:
				// Skip things that are not buildings
				iFeatureCode = pFeature->GetFieldAsInteger(pLayerDefn->GetFieldIndex("osgb:featureCode"));
				switch(iFeatureCode)
				{
					case 1:
					default:
						continue;
				}
				break;
			default:
				break;
		}

		pGeom = pFeature->GetGeometryRef();
		if (!pGeom)
			continue;
		OGRwkbGeometryType GeometryType = pGeom->getGeometryType();

		if (wkbPoint != wkbFlatten(GeometryType))
			continue;

		DPoint2 p2(((OGRPoint *)pGeom)->getX(), ((OGRPoint *)pGeom)->getY());

		if (opt.bInsideOnly && !opt.rect.ContainsPoint(p2))
			// Exclude instances outside the indicated extents
			continue;

		vtStructInstance *pInstance = AddNewInstance();
		if (!pInstance)
			return;

		vtStructInstance *pDefaultInstance = GetClosestDefault(pInstance);
		if (NULL != pDefaultInstance)
		{
			pInstance->SetRotation(pDefaultInstance->GetRotation());
			pInstance->SetScale(pDefaultInstance->GetScale());
		}
		pInstance->SetPoint(p2);
	}
}

