//
// Structure.h
//
// Implements the vtStructure class which represents a single built structure.
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
/** \file Structure.h */

#ifndef VTDATA_STRUCTURE_H
#define VTDATA_STRUCTURE_H

#include "MathTypes.h"
#include "Selectable.h"
#include "Content.h"
#include "FilePath.h"	// for GZOutput

class vtBuilding;
class vtFence;
class vtStructInstance;

/**
 * Structure type.
 */
enum vtStructureType
{
	ST_BUILDING,	/**< A Building (vtBuilding) */
	ST_LINEAR,		/**< A Linear (vtFence) */
	ST_INSTANCE,	/**< A Structure Instance (vtStructInstance) */
	ST_NONE
};

/**
 * The vtStructure class represents any "built structure".  These are
 * generally immobile, artificial entities of human-scale and larger, such
 * as buildings and fences.
 * \par
 * Structures are implemented as 3 types:
 *  - Buildings (vtBuilding)
 *  - Fences and walls (vtFence)
 *  - Instances (vtStructInstance)
 * \par
 * For enclosed and linear structures which can be well-described
 * parametrically, vtBuilding and vtFence provide efficient data
 * representation.  For other structures which are not easily reduced to
 * parameters, the Instance type allows you to reference any external model,
 * such as a unique building which has been created in a 3D Modelling Tool.
 */
class vtStructure : public Selectable, public vtTagArray
{
public:
	vtStructure();
	virtual ~vtStructure();

	// use an explicit method to avoid assignment operator
	void CopyFrom(const vtStructure &v);

	void SetType(vtStructureType t) { m_type = t; }
	vtStructureType GetType() const { return m_type; }

	void SetElevationOffset(float fOffset) { m_fElevationOffset = fOffset; }
	float GetElevationOffset() const { return m_fElevationOffset; }
	void SetAbsolute(bool b) { m_bAbsolute = b; }
	bool GetAbsolute() const { return m_bAbsolute; }

	vtBuilding *GetBuilding() { if (m_type == ST_BUILDING) return (vtBuilding *)this; else return NULL; }
	vtFence *GetFence() { if (m_type == ST_LINEAR) return (vtFence *)this; else return NULL; }
	vtStructInstance *GetInstance() { if (m_type == ST_INSTANCE) return (vtStructInstance *)this; else return NULL; }

	virtual bool GetExtents(DRECT &rect) const = 0;
	virtual bool IsContainedBy(const DRECT &rect) const = 0;
	virtual void WriteXML(GZOutput &out, bool bDegrees) const = 0;

	void WriteTags(GZOutput &out) const;

#ifdef VIAVTDATA
	bool m_bIsVIAContributor;
	bool m_bIsVIATarget;
#endif

protected:
	vtStructureType m_type;

	// Offset that the structure should be moved up or down relative to its
	// default position on the ground
	// for buildings this is (lowest corner of its base footprint)
	// for linear features this is the lowest point of the feature.
	// for instances this is the datum point
	float m_fElevationOffset;

	// If true, elevation offset is relative to sealevel, not to the
	//  heightfield surface.
	bool m_bAbsolute;

private:
	// Don't let unsuspecting users stumble into assuming that object
	// copy semantics will work.  Declare them private and never
	// define them:
	vtStructure( const vtStructure & );
	vtStructure &operator=( const vtStructure & );
};

/**
 * This class represents a reference to an external model, such as a unique
 * building which has been created in a 3D Modelling Tool.  It is derived from
 * vtTagArray which provides a set of arbitrary tags (name/value pairs).
 * At least one of the following two tags should be present:
 * - filename, which contains a resolvable path to an external 3d model file.
 *	 An example is filename="MyModels/GasStation.3ds"
 * - itemname, which contains the name of a content item which will be resolved
 *	 by a list maintained by a vtContentManager.  An example is
 *	 itemname="stopsign"
 */
class vtStructInstance : public vtStructure
{
public:
	vtStructInstance();

	// copy operator
	vtStructInstance &operator=(const vtStructInstance &v);

	void WriteXML(GZOutput &out, bool bDegrees) const;
	void Offset(const DPoint2 &delta) { m_p += delta; }

	bool GetExtents(DRECT &rect) const;
	bool IsContainedBy(const DRECT &rect) const;

	virtual double DistanceToPoint(const DPoint2 &p, float fMaxRadius) const;

	void SetPoint(const DPoint2 &p) { m_p = p; }
	DPoint2 GetPoint() const { return m_p; }
	void SetRotation(float f) { m_fRotation = f; }
	float GetRotation() const { return m_fRotation; }
	void SetScale(float f) { m_fScale = f; }
	float GetScale() const { return m_fScale; }
	void SetItem(vtItem *pItem) { m_pItem = pItem; }
	vtItem *GetItem() { return m_pItem; }

protected:
	DPoint2	m_p;			// earth position
	float	m_fRotation;	// in radians
	float	m_fScale;		// meters per unit

	vtItem *m_pItem;	// If this is an instance of a content item
};

#endif // VTDATA_STRUCTURE_H

