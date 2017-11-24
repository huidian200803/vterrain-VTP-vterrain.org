//
// StructArray.h
//
// Copyright (c) 2001-2014 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

#include "MathTypes.h"
#include "vtCRS.h"
#include "Building.h"
#include "HeightField.h"
#include <stdio.h>

/** This structure stores a description of how an imported file should be
   intepreted as built structure data. */
class StructImportOptions
{
public:
	vtStructureType type;
	DRECT			rect;
	bool			bInsideOnly;
	bool			bBuildFoundations;
	bool			bUse25DForElevation;

	/// If there are several layers in the input, this is the name of the layer to use
	vtString		m_strLayerName;

	// If height is read from a field, name of that field.
	vtString		m_strFieldNameHeight;
	enum HeightType { STORIES, METERS, FEET, METERSNOSTORIES, FEETNOSTORIES } m_HeightType;

	/// If StructureInstance filename is taken from a field, name of that field
	vtString		m_strFieldNameFile;

	/// If Building Roof Type is taken from a field, name of that field
	vtString		m_strFieldNameRoof;
	/// If Building Roof Type is fixed, this is the type
	RoofType		m_eRoofType;
	int				m_iSlope;			// default: -1

	vtHeightField	*pHeightField;

	bool	m_bFixedColor;
	RGBi	m_BuildingColor;	// default: -1 -1 -1
	RGBi	m_RoofColor;		// default: -1 -1 -1
};

// Well known import schemas

typedef enum
{
	SCHEMA_OSGB_TOPO_AREA,
	SCHEMA_OSGB_TOPO_POINT,
	SCHEMA_OSGB_TOPO_LINE,
	SCHEMA_MAPINFO_OSGB_TOPO_AREA,
	SCHEMA_MAPINFO_OSGB_TOPO_POINT,
	SCHEMA_UI
} SchemaType;

/**
 * The vtStructureArray class contains a list of Built Structures
 * (vtStructure objects).  It can be loaded and saved to VTST files
 * with the ReadXML and WriteXML methods.
 *
 */
class vtStructureArray : public std::vector<vtStructure*>
{
public:
	vtStructureArray();
	virtual ~vtStructureArray();
	void DestructItems();

	void SetFilename(const vtString &str) { m_strFilename = str; }
	vtString GetFilename() { return m_strFilename; }

	int GetFirstSelected();
	int GetNextSelected();
	vtStructure *GetFirstSelectedStructure() const;
	int DeleteSelected();
	virtual void DestroyStructure(int i) {}

	bool ReadSHP(const char *pathname, StructImportOptions &opt,
		bool progress_callback(int) = NULL);
	void AddElementsFromOGR(class GDALDataset *datasource,
		StructImportOptions &opt, bool progress_callback(int) = NULL);

	void AddElementsFromOGR_SDTS(class GDALDataset *datasource,
		bool progress_callback(int) = NULL);
	void AddElementsFromOGR_RAW(class GDALDataset *datasource,
		StructImportOptions &opt, bool progress_callback(int) = NULL);

	void AddBuildingsFromOGR(class OGRLayer *pLayer,
		StructImportOptions &opt, bool progress_callback(int) = NULL);
	void AddLinearsFromOGR(class OGRLayer *pLayer,
		StructImportOptions &opt, bool progress_callback(int) = NULL);
	void AddInstancesFromOGR(class OGRLayer *pLayer,
		StructImportOptions &opt, bool progress_callback(int) = NULL);

	bool ReadBCF(const char *pathname);		// read a .bcf file
	bool ReadBCF_Old(FILE *fp);				// support obsolete format
	bool ReadXML(const char *pathname, bool progress_callback(int) = NULL);

	bool WriteXML(const char *pathname, bool bGZip = false) const;
	bool WriteFootprintsToSHP(const char *pathname);
	bool WriteFootprintsToCanoma3DV(const char *pathname, const DRECT *area,
		const vtHeightField *pHF);	

	bool FindClosestBuildingCorner(const DPoint2 &point, double epsilon,
						   int &building, int &corner, double &distance);
	bool FindClosestBuildingCenter(const DPoint2 &point, double epsilon,
						   int &building, double &distance);
	bool FindClosestLinearCorner(const DPoint2 &point, double epsilon,
						   int &structure, int &corner, double &distance);

	bool FindClosestStructure(const DPoint2 &point, double epsilon,
			int &structure, double &distance, float fMaxInstRadius = 1E9f,
			float fLinearWidthBuffer = 0.0f);
	bool FindClosestBuilding(const DPoint2 &point, double epsilon,
			int &structure, double &closest);

	bool IsEmpty() { return (size() == 0); }
	void GetExtents(DRECT &ext) const;
	void Offset(const DPoint2 &delta);

	int AddFoundations(vtHeightField *pHF, bool progress_callback(int) = NULL);
	void RemoveFoundations();

	// selection
	int NumSelected();
	int NumSelectedOfType(vtStructureType t);
	void DeselectAll();

	// Override these 'Factory' methods so that the vtStructureArray base
	// methods can be capable of handling subclasses of vtBuilding etc.
	virtual vtBuilding *NewBuilding();
	virtual vtFence *NewFence();
	virtual vtStructInstance *NewInstance();

	// convenience methods that create, add, and return a new structure
	vtBuilding *AddNewBuilding();
	vtFence *AddNewFence();
	vtStructInstance *AddNewInstance();

	vtBuilding *AddBuildingFromLineString(class OGRLineString *pLineString);

	// override to catch edit hightlighting
	virtual void SetEditedEdge(vtBuilding *bld, int lev, int edge);

	vtCRS m_crs;

protected:
	vtString	m_strFilename;

	// used to indicate which edge should be hightlighted during editing
	vtBuilding *m_pEditBuilding;
	int m_iEditLevel;
	int m_iEditEdge;
	int m_iLastSelected;
};

extern vtStructureArray g_DefaultStructures;

// Helpers
int GetSHPType(const char *filename);

bool SetupDefaultStructures(const vtString &fname);
vtBuilding *GetClosestDefault(vtBuilding *pBld);
vtFence *GetClosestDefault(vtFence *pFence);
vtStructInstance *GetClosestDefault(vtStructInstance *pInstance);

