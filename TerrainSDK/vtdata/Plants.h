//
// Plants.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTDATA_PLANTSH
#define VTDATA_PLANTSH

#include "Array.h"
#include "Features.h"

enum AppearType {
	AT_UNKNOWN,
	AT_BILLBOARD,
	AT_MODEL,
	AT_XFROG
};

/**
 * An appearance is an representation of how a plant looks, which can be
 * an image or a model.
 */
class vtPlantAppearance
{
public:
	vtPlantAppearance();
	vtPlantAppearance(AppearType type, const char *filename, float width,
		float height, float shadow_radius, float shadow_darkness);
	virtual ~vtPlantAppearance();

	AppearType	m_eType;
	vtString	m_filename;
	float		m_width;
	float		m_height;
	float		m_shadow_radius;
	float		m_shadow_darkness;
};

/**
 * A species consists of a scientific name, any number of common names, and
 * any number of appearances.
 */
class vtPlantSpecies
{
public:
	vtPlantSpecies();
	virtual ~vtPlantSpecies();

	// copy
	vtPlantSpecies &operator=(const vtPlantSpecies &v);

	struct CommonName { vtString m_strName, m_strLang; };

	void AddCommonName(const char *Name, const char *Lang = NULL);
	size_t NumCommonNames() const { return m_CommonNames.size(); }
	CommonName GetCommonName(int n = 0) const { return m_CommonNames[n]; }
	uint CommonNamesOfLanguage(const char *lang);

	void SetSciName(const char *szSciName);
	const char *GetSciName() const { return m_szSciName; }

	void SetMaxHeight(float f) { m_fMaxHeight = f; }
	float GetMaxHeight() const { return m_fMaxHeight; }

	virtual void AddAppearance(AppearType type, const char *filename,
		float width, float height, float shadow_radius, float shadow_darkness)
	{
		vtPlantAppearance *pApp = new vtPlantAppearance(type, filename,
			width, height, shadow_radius, shadow_darkness);
		m_Apps.Append(pApp);
	}

	uint NumAppearances() const { return m_Apps.GetSize(); }
	vtPlantAppearance *GetAppearance(int i) const { return m_Apps[i]; }

protected:
	std::vector<CommonName> m_CommonNames;

	vtString	m_szSciName;
	float		m_fMaxHeight;
	vtArray<vtPlantAppearance*> m_Apps;
};


/**
 * A plant density is a representation of how densely a plant is distributed,
 * consisting of:
	- a reference to a species
	- a density, in instances per square meter.
	- optionally, a typical size
 */
class vtPlantDensity
{
public:
	void ResetAmounts() { m_amount = 0.0f; m_iNumPlanted = 0; }

	vtPlantSpecies	*m_pSpecies;
	float		m_plant_per_m2;
	float		m_typical_size;

	float		m_amount;			// these two fields are using during the
	int			m_iNumPlanted;		// plant distribution process
};

/**
 * A biotype is like a ecological type classification.  It consists of any
 * number of plant densities, which together describe how many species are
 * found in a certain area, and the density of each.  For example,
 * "Hawaiian Mid-Elevation Rainforest" would be a biotype.
 */
class vtBioType
{
public:
	vtBioType();
	~vtBioType();

	void AddPlant(vtPlantSpecies *pSpecies, float plant_per_m2, float typical_size = -1.0f);
	void ResetAmounts();
	int GetWeightedRandomPlant();

	vtArray<vtPlantDensity *> m_Densities;

	vtString	m_name;
};

/**
 * A species list is a list of species.
 */
class vtSpeciesList
{
public:
	vtSpeciesList();
	virtual ~vtSpeciesList();

	bool ReadXML(const char *fname, vtString *msg = NULL);
	bool WriteXML(const char *fname) const;
	bool WriteHTML(const char *fname) const;

	uint NumSpecies() const { return m_Species.GetSize();  }
	vtPlantSpecies *GetSpecies(uint i) const
	{
		if (i < m_Species.GetSize())
			return m_Species[i];
		else
			return NULL;
	}
	short GetSpeciesIdByName(const char *name) const;
	short GetSpeciesIdByCommonName(const char *name) const;
	void Append(vtPlantSpecies *pSpecies)
	{
		m_Species.Append(pSpecies);
	}
	short FindSpeciesId(vtPlantSpecies *ps);
	void Clear() { m_Species.Clear(); }

protected:
	vtArray<vtPlantSpecies*> m_Species;
};

/**
 * A bioregion is a set of all the biotypes (vtBioType) found in a given
 * region.  For example, the California bioregion contains biotypes for
 * coastal wetlands, temperate rainforest, dry grasslands, alpine forest,
 * etc.
 */
class vtBioRegion
{
public:
	vtBioRegion();
	~vtBioRegion();

	bool Read(const char *fname, const vtSpeciesList &species);
	bool ReadXML(const char *fname, const vtSpeciesList &species,
		vtString *msg = NULL);
	bool WriteXML(const char *fname) const;

	int AddType(vtBioType *bt) { return m_Types.Append(bt); }
	int NumTypes() const { return m_Types.GetSize(); }
	vtBioType *GetBioType(int i) const { return m_Types[i]; }
	int FindBiotypeIdByName(const char *name) const;
	void ResetAmounts();
	void Clear() { m_Types.Clear(); }

	vtArray<vtBioType *> m_Types;
};

/**
 * This class contains a set of Plant Instances.  It is implemented as a
 * subclass of vtFeatureSetPoint2D, a set of 2D locations of the plant
 * stems.
 * It can be read from/written to the VF format ("vegetation file")
 * designed specifically for the purpose of storing plants, which makes
 * it very compact and much more efficient than, e.g. SHP format.
 */
class vtPlantInstanceArray : public vtFeatureSetPoint2D
{
public:
	vtPlantInstanceArray();

	void SetSpeciesList(vtSpeciesList *list) { m_pSpeciesList = list; }
	int AddPlant(const DPoint2 &pos, float size, short species_id);
	int AddPlant(const DPoint2 &pos, float size, vtPlantSpecies *ps);
	void SetPlant(int iNum, float size, short species_id);
	void GetPlant(int iNum, float &size, short &species_id) const;
	uint InstancesOfSpecies(short species_id);

	bool ReadVF_version11(const char *fname);
	bool ReadVF(const char *fname);
	bool ReadSHP(const char *fname);
	bool WriteVF(const char *fname) const;

protected:
	vtSpeciesList *m_pSpeciesList;

	int m_SizeField;
	int m_SpeciesField;
};

#endif	// VTDATA_PLANTSH

