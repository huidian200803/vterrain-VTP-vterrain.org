//
// VegLayer.cpp
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/shapelib/shapefil.h"
#include "vtdata/vtLog.h"
#include "ScaledView.h"
#include "VegLayer.h"
#include "vtui/Helper.h"	// for DisplayAndLog

#include "Builder.h"	// for access to the application's plants and biotypes

//////////////////////////////////////////////////////////////////////////

vtVegLayer::vtVegLayer() : vtRawLayer()
{
	m_type = LT_VEG;
	m_VLType = VLT_None;

	// default dark green
	m_DrawStyle.m_LineColor.Set(0,128,0);

	m_DrawStyle.m_MarkerSize = 3;	// pixels
}

void vtVegLayer::SetVegType(VegLayerType type)
{
	m_VLType = type;
	if (m_pSet == NULL)
	{
		if (type == VLT_Instances)
			m_pSet = new vtPlantInstanceArray;

		if (type == VLT_Density || type == VLT_BioMap)
			m_pSet = new vtFeatureSetPolygon;
	}
}

vtVegLayer::~vtVegLayer()
{
}

void vtVegLayer::GetPropertyText(wxString &str)
{
	wxString s;
	str = _("Vegetation layer type: ");
	switch (m_VLType)
	{
	case VLT_None: str += _("None\n"); break;
	case VLT_Density: str += _("Density\n"); break;
	case VLT_BioMap: str += _("BioMap\n"); break;
	case VLT_Instances: str += _("Plant Instances\n"); break;
	}
	if (m_VLType == VLT_Instances)
	{
		vtPlantInstanceArray *pPIA = (vtPlantInstanceArray*) m_pSet;

		int ent = m_pSet->NumEntities();
		s.Printf(_("Number of Instances: %d\n"), ent);
		str += s;

		vtSpeciesList *list = g_bld->GetSpeciesList();
		for (uint i = 0; i < list->NumSpecies(); i++)
		{
			int num = pPIA->InstancesOfSpecies(i);
			if (num != 0)
			{
				vtPlantSpecies *spe = list->GetSpecies(i);
				s.Printf(_("  %d instances of species %hs\n"), num, spe->GetSciName());
				str += s;
			}
		}
	}
}

bool vtVegLayer::CanBeSaved()
{
	if (m_VLType != VLT_None)
		return true;
	else
		return false;
}

wxString vtVegLayer::GetFileExtension()
{
	if (m_VLType == VLT_Instances)
		return _T(".vf");
	else
		return _T(".shp");
}

bool vtVegLayer::OnSave(bool progress_callback(int))
{
	// currently we can load and save VF files (Plant Instances)
	if (m_VLType == VLT_Instances)
		return GetPIA()->WriteVF(GetLayerFilename().mb_str(wxConvUTF8));
	else
		return vtRawLayer::OnSave(progress_callback);
}

bool vtVegLayer::OnLoad()
{
	vtSpeciesList *plants = g_bld->GetSpeciesList();
	if (plants->NumSpecies() == 0)
	{
		DisplayAndLog(_("You must specify a species file (plant list) to use when working with vegetation files."));
		return false;
	}

	wxString fname = GetLayerFilename();
	wxString ext = fname.Right(3);

	wxString dbfname = fname.Left(fname.Length() - 4);
	dbfname += _T(".dbf");

	if (!ext.CmpNoCase(_T(".vf")))
	{
		// read VF file
		SetVegType(VLT_Instances);
		GetPIA()->SetSpeciesList(plants);
		if (GetPIA()->ReadVF(fname.mb_str(wxConvUTF8)))
		{
			m_pSet->SetFilename((const char *)fname.mb_str(wxConvUTF8));
			return true;
		}
		else
		{
			delete m_pSet;
			m_pSet = NULL;
			m_VLType = VLT_None;
			return false;
		}
	}
	else if (!ext.CmpNoCase(_T("shp")))
	{
		// SHPOpen doesn't yet support utf-8 or wide filenames, so convert
		vtString fname_local = UTF8ToLocal(fname.mb_str(wxConvUTF8));

		// Study this SHP file, look at what it might be
		int		nElems, nShapeType;
		SHPHandle hSHP = SHPOpen(fname_local, "rb");
		if (hSHP == NULL)
			return false;
		SHPGetInfo(hSHP, &nElems, &nShapeType, NULL, NULL);
		SHPClose(hSHP);
		if (nShapeType == SHPT_POINT)
			SetVegType(VLT_Instances);
		else if (nShapeType == SHPT_POLYGON)
		{
			// DBFOpen doesn't yet support utf-8 or wide filenames, so convert
			vtString fname_localdbf = UTF8ToLocal(dbfname.mb_str(wxConvUTF8));

			DBFHandle db = DBFOpen(fname_localdbf, "rb");
			if (db == NULL)
				return false;

			m_field_density = DBFGetFieldIndex(db, "Density");
			m_field_biotype = DBFGetFieldIndex(db, "Biotype");
			DBFClose(db);

			if (m_field_density != -1)
				SetVegType(VLT_Density);
			else if (m_field_biotype != -1)
				SetVegType(VLT_BioMap);
			else
				return false;
		}
		// OK, read the rest of the file
		return m_pSet->LoadFromSHP(fname.mb_str(wxConvUTF8));
	}

	// don't know this file
	return false;
}

bool vtVegLayer::AppendDataFrom(vtLayer *pL)
{
	if (pL->GetType() != LT_VEG)
		return false;

	vtVegLayer *pVL = (vtVegLayer *)pL;

	if (m_VLType == VLT_Instances)
	{
		m_pSet->AppendDataFrom(pVL->m_pSet);
		return true;
	}
	else
	{
		// Must be of compatible types
		if (m_VLType != pVL->m_VLType)
			return false;

		return vtRawLayer::AppendDataFrom(pL);
	}
}

void vtVegLayer::AddElementsFromLULC(vtLULCFile *pLULC)
{
	LULCSection *section;
	LULCPoly *poly;

	SetVegType(VLT_Density);

	// Set CRS.
	vtCRS crs_new;
	crs_new.SetSimple(0, -1, EPSG_DATUM_WGS84);
	SetCRS(crs_new);

	// Figure out the number of polygons in file.
	uint size = 0;
	for (uint sec = 0; sec < pLULC->NumSections(); sec++)
	{
		section = pLULC->GetSection(sec);
		size = size + section->m_iNumPolys;
	}

	// Create density field.
	m_field_density = m_pSet->AddField("Density", FT_Float);
	m_pSet->SetNumEntities(size);

	// Get each poly from LULC file.
	uint i, s, p, count = 0;
	float density=0;
	for (s = 0; s < pLULC->NumSections(); s++)
	{
		section = pLULC->GetSection(s);
		for (p = 0; p < section->m_iNumPolys; p++)
		{
			poly = section->m_pPoly + p;

			bool wild = false;
			switch (poly->Attribute)
			{
				case 42:	// forest
					wild = true;
					density = 1.0f;
					break;
				case 32:
				case 33:
					wild = true;
					density = 0.5;
					break;
				case 22:	// orchards
					wild = false;
					// no crops for now
					break;
				default:
					density = 0.0f;
					break;
			}
			DLine2 dline;
			dline.SetSize(poly->m_iCoords);

			// get Coords of LULCpoly and store as latlon, then save in VPoly
			for (i = 0; i < dline.GetSize(); i++)
				dline.SetAt(i, poly->m_p[i]);

			DPolygon2 dpoly;
			dpoly.push_back(dline);

			GetPS()->SetPolygon(count, dpoly);
			m_pSet->SetValue(count, m_field_density, density);

			count++;
		}
	}
}


/**
 * Extract data from a SHP/DBF file and intepret it as a vegetation layer.
 * This produces a single-valued polygonal coverage.
 *
 * 'iField' is the index of the field from which to pull the single value.
 * 'datatype' is either 0, 1, or 2 for whether the indicated field should be
 *		intepreted as a density value (double), the name of a biotype
 *		(string), or the ID of a biotype (int).
 */
bool vtVegLayer::AddElementsFromSHP_Polys(const wxString &filename,
										  const vtCRS &crs,
										  int iField, VegImportFieldType datatype)
{
	// When working with float field data, must use C locale
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	// SHPOpen doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(filename.mb_str(wxConvUTF8));

	// Open the SHP File
	SHPHandle hSHP = SHPOpen(fname_local, "rb");
	if (hSHP == NULL)
		return false;

	// Get number of polys and type of data
	int		nElem;
	int		nShapeType;
	SHPGetInfo(hSHP, &nElem, &nShapeType, NULL, NULL);

	// Check Shape Type, Veg Layer should be Poly data
	if (nShapeType != SHPT_POLYGON)
		return false;

	// Open DBF File
	DBFHandle db = DBFOpen(fname_local, "rb");
	if (db == NULL)
		return false;

	// Check for field of poly id, current default field in dbf is Id
	int *pnWidth = 0, *pnDecimals = 0;
	char *pszFieldName = NULL;

	DBFFieldType fieldtype = DBFGetFieldInfo(db, iField,
		pszFieldName, pnWidth, pnDecimals );

	if (datatype == VIFT_Density)
	{
		if (fieldtype != FTDouble)
		{
			VTLOG(" Expected the DBF field '%s' to be of type 'Double', but found '%s' instead.\n",
				pszFieldName, DescribeFieldType(fieldtype));
			return false;
		}
	}
	if (datatype == VIFT_BiotypeName)
	{
		if (fieldtype != FTString)
		{
			VTLOG(" Expected the DBF field '%s' to be of type 'String', but found '%s' instead.\n",
				pszFieldName, DescribeFieldType(fieldtype));
			return false;
		}
	}
	if (datatype == VIFT_BiotypeID)
	{
		if (fieldtype != FTInteger)
		{
			VTLOG(" Expected the DBF field '%s' to be of type 'Integer', but found '%s' instead.\n",
				pszFieldName, DescribeFieldType(fieldtype));
			return false;
		}
	}

	// OK, ready to allocate our featureset
	if (datatype == VIFT_Density)
	{
		SetVegType(VLT_Density);
		m_field_density = m_pSet->AddField("Density", FT_Float);
	}
	if (datatype == VIFT_BiotypeName || datatype == VIFT_BiotypeID)
	{
		SetVegType(VLT_BioMap);
		m_field_biotype = m_pSet->AddField("Biotype", FT_Integer);
	}

	SetCRS(crs);

	// Read Polys from SHP into Veg Poly
	m_pSet->LoadGeomFromSHP(hSHP);
	SHPClose(hSHP);

	// Read fields
	int biotype_id;
	for (uint i = 0; i < (uint) nElem; i++)
	{
		int record = m_pSet->AddRecord();
		// Read DBF Attributes per poly
		if (datatype == VIFT_Density)
		{
			// density
			m_pSet->SetValue(record, m_field_density, (float) DBFReadDoubleAttribute(db, i, iField));
		}
		if (datatype == VIFT_BiotypeName)
		{
			const char *str = DBFReadStringAttribute(db, i, iField);
			biotype_id = g_bld->GetBioRegion()->FindBiotypeIdByName(str);
			m_pSet->SetValue(record, m_field_biotype, biotype_id);
		}
		if (datatype == VIFT_BiotypeID)
		{
			biotype_id = DBFReadIntegerAttribute(db, i, iField);
			m_pSet->SetValue(record, m_field_biotype, biotype_id);
		}
	}
	DBFClose(db);
	return true;
}

/**
 * Extract point data from a SHP/DBF file and intepret it as a vegetation
 * layer.  This produces a set of vegetation instances.
 *
 * The 'opt' parameter contains a description of how the fields in the
 * imported file are to be interpreted.
 */
bool vtVegLayer::AddElementsFromSHP_Points(const wxString &filename,
										   const vtCRS &crs,
										   VegPointOptions &opt)
{
	// We will be creating plant instances
	SetVegType(VLT_Instances);

	vtSpeciesList *pSpeciesList = g_bld->GetSpeciesList();
	vtBioRegion *pBioRegion = g_bld->GetBioRegion();
	GetPIA()->SetSpeciesList(pSpeciesList);

	// SHPOpen doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(filename.mb_str(wxConvUTF8));

	// Open the SHP File
	SHPHandle hSHP = SHPOpen(fname_local, "rb");
	if (hSHP == NULL)
		return false;

	// Get number of points and type of data
	int		nElem;
	int		nShapeType;
	SHPGetInfo(hSHP, &nElem, &nShapeType, NULL, NULL);

	// Check Shape Type, Veg Layer should be Point data
	if (nShapeType != SHPT_POINT)
		return false;

	// Open DBF File
	DBFHandle db = DBFOpen(fname_local, "rb");
	if (db == NULL)
		return false;

	// Confirm that the field types are correct
	int *pnWidth = 0, *pnDecimals = 0;
	char pszFieldName[80];
	DBFFieldType fieldtype;

	if (!opt.bFixedSpecies)
	{
		// we're going to get species info from a field
		fieldtype = DBFGetFieldInfo(db, opt.iSpeciesFieldIndex, pszFieldName,
			pnWidth, pnDecimals);
		if (opt.iInterpretSpeciesField == 0 || opt.iInterpretSpeciesField == 3)
		{
			if ((fieldtype != FTInteger) && (fieldtype != FTDouble))
			{
				DisplayAndLog("Can't import field '%hs' as an integer, it is type %d.",
					pszFieldName, fieldtype);
				return false;
			}
		}
		else
		{
			if (fieldtype != FTString)
			{
				DisplayAndLog("Can't import field '%hs' as a string, it is type %d.",
					pszFieldName, fieldtype);
				return false;
			}
		}
	}

	// Set CRS.
	SetCRS(crs);

	// Initialize arrays.
	m_pSet->Reserve(nElem);

	// Read Points from SHP and intepret fields.
	SHPObject *psShape;
	const char *str;
	int biotype;
	vtBioType *pBioType;
	DPoint2 pos;

	int unfound = 0;

	for (int i = 0; i < nElem; i++)
	{
		// Get the i-th Point in the SHP file
		psShape = SHPReadObject(hSHP, i);
		pos.x = psShape->padfX[0];
		pos.y = psShape->padfY[0];
		SHPDestroyObject(psShape);

		// Read DBF Attributes per point
		int species_id = -1;
		if (opt.bFixedSpecies)
			species_id = pSpeciesList->GetSpeciesIdByName(opt.strFixedSpeciesName.mb_str(wxConvUTF8));
		else
		{
			switch (opt.iInterpretSpeciesField)
			{
			case 0:
				species_id = DBFReadIntegerAttribute(db, i, opt.iSpeciesFieldIndex);
				break;
			case 1:
				str = DBFReadStringAttribute(db, i, opt.iSpeciesFieldIndex);
				species_id = pSpeciesList->GetSpeciesIdByName(str);
				if (species_id == -1)
					unfound++;
				break;
			case 2:
				str = DBFReadStringAttribute(db, i, opt.iSpeciesFieldIndex);
				species_id = pSpeciesList->GetSpeciesIdByCommonName(str);
				if (species_id == -1)
					unfound++;
				break;
			case 3:
				biotype = DBFReadIntegerAttribute(db, i, opt.iSpeciesFieldIndex);
				pBioType = pBioRegion->GetBioType(biotype);
				if (pBioType)
					species_id = pBioType->GetWeightedRandomPlant();
				break;
			case 4:
				str = DBFReadStringAttribute(db, i, opt.iSpeciesFieldIndex);
				biotype = pBioRegion->FindBiotypeIdByName(str);
				pBioType = pBioRegion->GetBioType(biotype);
				if (pBioType)
					species_id = pBioType->GetWeightedRandomPlant();
				break;
			}
		}

		// Make sure we have a valid species
		if (species_id == -1)
			continue;
		vtPlantSpecies *pSpecies = pSpeciesList->GetSpecies(species_id);
		if (!pSpecies)
			continue;

		// Set height
		float size;
		if (opt.bHeightRandom)
			size = random(pSpecies->GetMaxHeight());
		else if (opt.iHeightFieldIndex != -1)
			size = (float) DBFReadDoubleAttribute(db, i, opt.iHeightFieldIndex);
		else
			// fixed height
			size = opt.fHeightFixed;

		// If we get here, there is a valid plant to append
		GetPIA()->AddPlant(pos, size, species_id);
	}
	if (unfound)
		DisplayAndLog("Couldn't find species for %d out of %d instances.", unfound, nElem);
	else
		DisplayAndLog("Imported %d plant instances.", nElem);

	DBFClose(db);
	SHPClose(hSHP);
	return true;
}

float vtVegLayer::FindDensity(const DPoint2 &p)
{
	if (m_VLType != VLT_Density)
		return -1;

	int poly = ((vtFeatureSetPolygon*)m_pSet)->FindPolygon(p);
	if (poly != -1)
		return m_pSet->GetFloatValue(poly, m_field_density);
	else
		return -1;
}

int vtVegLayer::FindBiotype(const DPoint2 &p)
{
	if (m_VLType != VLT_BioMap)
		return -1;

	int poly = ((vtFeatureSetPolygon*)m_pSet)->FindPolygon(p);
	if (poly != -1)
		return m_pSet->GetIntegerValue(poly, m_field_biotype);
	else
		return -1;
}

bool vtVegLayer::ExportToSHP(const char *fname)
{
	if (m_VLType != VLT_Instances)
		return false;

	return m_pSet->SaveToSHP(fname);
}

