//
// The vtStructureArray class is an array of Structure objects.
//
// It supports operations including loading and saving to a file
// and picking of building elements.
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdlib.h>
#include <string.h>

#include "shapelib/shapefil.h"
#include "xmlhelper/easyxml.hpp"
#include "zlib.h"

#include "Building.h"
#include "Fence.h"
#include "FilePath.h"
#include "MaterialDescriptor.h"
#include "PolyChecker.h"
#include "StructArray.h"
#include "vtLog.h"

vtStructureArray g_DefaultStructures;


/////////////////////////////////////////////////////////////////////////////

vtStructureArray::vtStructureArray()
{
	m_strFilename = "Untitled.vtst";
	m_pEditBuilding = NULL;
	m_iLastSelected = -1;
}

vtStructureArray::~vtStructureArray()
{
	DestructItems();
}

// Factories
vtBuilding *vtStructureArray::NewBuilding()
{
	return new vtBuilding;
}

vtFence *vtStructureArray::NewFence()
{
	return new vtFence;
}

vtStructInstance *vtStructureArray::NewInstance()
{
	return new vtStructInstance;
}

vtBuilding *vtStructureArray::AddNewBuilding()
{
	vtBuilding *nb = NewBuilding();

	// Every building needs a link back up to its containing array's CRS.
	nb->SetCRS(&m_crs);

	push_back(nb);
	return nb;
}

vtFence *vtStructureArray::AddNewFence()
{
	vtFence *nf = NewFence();
	push_back(nf);
	return nf;
}

vtStructInstance *vtStructureArray::AddNewInstance()
{
	vtStructInstance *ni = NewInstance();
	push_back(ni);
	return ni;
}

void vtStructureArray::DestructItems()
{
	for (uint i = 0; i < size(); i++)
		delete at(i);
}

void vtStructureArray::SetEditedEdge(vtBuilding *bld, int lev, int edge)
{
	m_pEditBuilding = bld;
	m_iEditLevel = lev;
	m_iEditEdge = edge;
}

/** Find the building corner closest to the given point, if it is within
 * 'epsilon' distance.  The building index, corner index, and distance from
 * the given point are all returned by reference.
 *
 * \return True if a building was found.
 */
bool vtStructureArray::FindClosestBuildingCorner(const DPoint2 &point,
			double epsilon, int &building, int &corner, double &closest)
{
	if (IsEmpty())
		return false;

	building = -1;
	closest = 1E8;

	for (uint i = 0; i < size(); i++)
	{
		vtStructure *str = at(i);
		if (str->GetType() != ST_BUILDING)
			continue;
		vtBuilding *bld = str->GetBuilding();

		const DPolygon2 &dl = bld->GetFootprint(0);

		int test_index;
		double test_distance;
		dl.NearestPoint(point, test_index, test_distance);
		if (test_distance < epsilon && test_distance < closest)
		{
			building = i;
			corner = test_index;
			closest = test_distance;
		}
	}
	return (building != -1);
}

/** Find the building center closest to the given point, if it is within
 * 'epsilon' distance.  The building index, and distance from the given
 * point are returned by reference.
 *
 * \return True if a building was found.
 */
bool vtStructureArray::FindClosestBuildingCenter(const DPoint2 &point,
				double epsilon, int &building, double &closest)
{
	if (IsEmpty())
		return false;

	building = -1;
	DPoint2 loc;
	double dist;
	closest = 1E8;

	for (uint i = 0; i < size(); i++)
	{
		vtStructure *str = at(i);
		vtBuilding *bld = str->GetBuilding();
		if (!bld)
			continue;

		bld->GetBaseLevelCenter(loc);
		dist = (loc - point).Length();
		if (dist > epsilon)
			continue;
		if (dist < closest)
		{
			building = i;
			closest = dist;
		}
	}
	return (building != -1);
}

bool vtStructureArray::FindClosestLinearCorner(const DPoint2 &point, double epsilon,
					   int &structure, int &corner, double &closest)
{
	DPoint2 loc;
	double dist;
	uint i, j;

	structure = -1;
	corner = -1;
	closest = 1E8;

	for (i = 0; i < size(); i++)
	{
		vtStructure *str = at(i);
		vtFence *fen = str->GetFence();
		if (!fen)
			continue;
		DLine2 &dl = fen->GetFencePoints();
		for (j = 0; j < dl.GetSize(); j++)
		{
			dist = (dl[j] - point).Length();
			if (dist > epsilon)
				continue;
			if (dist < closest)
			{
				structure = i;
				corner = j;
				closest = dist;
			}
		}
	}
	return (structure != -1);
}

/**
 * Find the structure which is closest to the given point, if it is within
 * 'epsilon' distance.  The structure index and distance are returned by
 * reference.
 *
 * \return True if a building was found.
 */
bool vtStructureArray::FindClosestStructure(const DPoint2 &point, double epsilon,
					   int &structure, double &closest, float fMaxInstRadius,
					   float fLinearWidthBuffer)
{
	structure = -1;
	closest = 1E8;

	if (IsEmpty())
		return false;

	DPoint2 loc;
	double dist;

	// Check buildings and instances first
	for (uint i = 0; i < size(); i++)
	{
		vtStructure *str = at(i);
		dist = 1E9;

		// a building
		vtBuilding *bld = str->GetBuilding();
		if (bld)
			dist = bld->GetDistanceToInterior(point);

		// or an instance
		vtStructInstance *inst = str->GetInstance();
		if (inst)
			dist = inst->DistanceToPoint(point, fMaxInstRadius);

		if (dist > epsilon)
			continue;
		if (dist < closest)
		{
			structure = i;
			closest = dist;
		}
	}
	// then check linears
	for (uint i = 0; i < size(); i++)
	{
		vtStructure *str = at(i);

		vtFence *fen = str->GetFence();
		if (fen)
		{
			dist = fen->GetDistanceToLine(point);

			if (dist > epsilon)
				continue;

			// If the point is within the buffer around the line, consider it
			//  closer than the other structures.  This allows picking of
			//  linear structures on top of other structures.
			if (dist < fLinearWidthBuffer && dist > closest)
				dist = closest - (1E-6);
			if (dist < closest)
			{
				structure = i;
				closest = dist;
			}
		}
	}
	return (structure != -1);
}


/**
 * Find the building which is closest to the given point, if it is within
 * 'epsilon' distance.  The structure index and distance are returned by
 * reference.
 *
 * \return True if a building was found.
 */
bool vtStructureArray::FindClosestBuilding(const DPoint2 &point,
						double epsilon, int &structure, double &closest)
{
	structure = -1;
	closest = 1E8;

	DPoint2 loc;
	double dist;

	for (uint i = 0; i < size(); i++)
	{
		vtStructure *str = at(i);
		vtBuilding *bld = str->GetBuilding();
		if (!bld) continue;

		dist = bld->GetDistanceToInterior(point);
		if (dist > epsilon)
			continue;
		if (dist < closest)
		{
			structure = i;
			closest = dist;
		}
	}
	return (structure != -1);
}


void vtStructureArray::GetExtents(DRECT &rect) const
{
	if (size() == 0)
		return;

	rect.SetInsideOut();

	DRECT rect2;
	for (uint i = 0; i < size(); i++)
	{
		vtStructure *str = at(i);
		if (str->GetExtents(rect2))
			rect.GrowToContainRect(rect2);
	}
}

void vtStructureArray::Offset(const DPoint2 &delta)
{
	uint npoints = size();
	if (!npoints)
		return;

	uint i, j;
	DPoint2 temp;
	for (i = 0; i < npoints; i++)
	{
		vtStructure *str = at(i);
		vtBuilding *bld = str->GetBuilding();
		if (bld)
			bld->Offset(delta);
		vtFence *fen = str->GetFence();
		if (fen)
		{
			DLine2 &line = fen->GetFencePoints();
			for (j = 0; j < line.GetSize(); j++)
				line[j] += delta;
		}
		vtStructInstance *inst = str->GetInstance();
		if (inst)
			inst->Offset(delta);
	}
}

int vtStructureArray::AddFoundations(vtHeightField *pHF, bool progress_callback(int))
{
	vtLevel *pLev, *pNewLev;
	int j, pts, built = 0;
	float fElev;

	int selected = NumSelected();
	VTLOG("AddFoundations: %d selected, %d total, ", selected, size());

	for (uint i = 0; i < size(); i++)
	{
		if (progress_callback != NULL)
			progress_callback(i * 99 / size());

		vtStructure *str = at(i);
		vtBuilding *bld = str->GetBuilding();
		if (!bld)
			continue;
		if (selected > 0 && !str->IsSelected())
			continue;

		// Get the outer footprint of the lowest level
		pLev = bld->GetLevel(0);
		const DLine2 &foot = pLev->GetOuterFootprint();
		pts = foot.GetSize();

		float fMin = 1E9, fMax = -1E9;
		for (j = 0; j < pts; j++)
		{
			pHF->FindAltitudeOnEarth(foot[j], fElev);

			if (fElev < fMin) fMin = fElev;
			if (fElev > fMax) fMax = fElev;
		}
		float fDiff = fMax - fMin;

		// if there's less than 50cm of depth, don't bother building
		// a foundation
		if (fDiff < 0.5f)
			continue;

		// Create and add a foundation level
		pNewLev = new vtLevel;
		pNewLev->m_iStories = 1;
		pNewLev->m_fStoryHeight = fDiff;
		bld->InsertLevel(0, pNewLev);
		bld->SetFootprint(0, foot);
		pNewLev->SetEdgeMaterial(BMAT_NAME_CEMENT);
		pNewLev->SetEdgeColor(RGBi(255, 255, 255));
		built++;
	}
	VTLOG("%d added.\n", built);
	return built;
}

void vtStructureArray::RemoveFoundations()
{
	vtLevel *pLev;
	for (uint i = 0; i < size(); i++)
	{
		vtStructure *str = at(i);
		vtBuilding *bld = str->GetBuilding();
		if (!bld)
			continue;
		pLev = bld->GetLevel(0);
		const vtString *mat = pLev->GetEdge(0)->m_pMaterial;
		if (mat && *mat == BMAT_NAME_CEMENT)
		{
			bld->DeleteLevel(0);
		}
	}
}

int vtStructureArray::NumSelected()
{
	int sel = 0;
	for (uint i = 0; i < size(); i++)
	{
		if (at(i)->IsSelected()) sel++;
	}
	return sel;
}

int vtStructureArray::NumSelectedOfType(vtStructureType t)
{
	int sel = 0;
	for (uint i = 0; i < size(); i++)
	{
		if (at(i)->IsSelected() && at(i)->GetType() == t) sel++;
	}
	return sel;
}

void vtStructureArray::DeselectAll()
{
	for (uint i = 0; i < size(); i++)
		at(i)->Select(false);
}

/**
 * Delete any structures which are selected.
 * \return the number that were deleted.
 */
int vtStructureArray::DeleteSelected()
{
	int num_deleted = 0;
	for (uint i = 0; i < size();)
	{
		vtStructure *str = at(i);
		if (str->IsSelected())
		{
			DestroyStructure(i);
			delete str;
			erase(begin() + i);
			num_deleted++;
		}
		else
			i++;
	}
	return num_deleted;
}

/**
 * Return the index of the first selected structure, or -1 if no
 *  structure is selected.
 */
int vtStructureArray::GetFirstSelected()
{
	for (uint i = 0; i < size(); i++)
		if (at(i)->IsSelected())
		{
			m_iLastSelected = i;
			return i;
		}
	return -1;
}

int vtStructureArray::GetNextSelected()
{
	if (-1 == m_iLastSelected)
		return -1;
	for (uint i = m_iLastSelected + 1; i < size(); i++)
		if (at(i)->IsSelected())
		{
			m_iLastSelected = i;
			return i;
		}
	return -1;
}

vtStructure *vtStructureArray::GetFirstSelectedStructure() const
{
	for (uint i = 0; i < size(); i++)
		if (at(i)->IsSelected())
			return at(i);
	return NULL;
}


////////////////////////////////////////////////////////////////////////
// Visitor class, for XML parsing of Structure files.
////////////////////////////////////////////////////////////////////////

// helper
RGBi ParseRGB(const char *str)
{
	RGBi color;
	sscanf(str, "%hd %hd %hd\n", &color.r, &color.g, &color.b);
	return color;
}

class StructureVisitor : public XMLVisitor
{
public:
	StructureVisitor(vtStructureArray *sa) :
		 _level(0), m_pSA(sa) {}

	virtual ~StructureVisitor () {}

	void startXML ();
	void endXML ();
	void startElement (const char * name, const XMLAttributes &atts);
	void endElement (const char * name);
	void data (const char * s, int length);

private:
	struct State
	{
		State () : item(0), type("") {}
		State (vtStructure * _item, const char * _type)
			: item(_item), type(_type) {}
		vtStructure * item;
		string type;
		RGBi wall_color;
	};

	State &state () { return _state_stack[_state_stack.size() - 1]; }

	void push_state (vtStructure *_item, const char *type)
	{
		if (type == 0)
			_state_stack.push_back(State(_item, "unspecified"));
		else
			_state_stack.push_back(State(_item, type));
		_level++;
		_data = "";
	}

	void pop_state () {
		_state_stack.pop_back();
		_level--;
	}

	string _data;
	int _level;
	vector<State> _state_stack;

	vtStructureArray *m_pSA;
};

void StructureVisitor::startXML ()
{
  _level = 0;
  _state_stack.resize(0);
}

void StructureVisitor::endXML ()
{
  _level = 0;
  _state_stack.resize(0);
}

void StructureVisitor::startElement (const char * name, const XMLAttributes &atts)
{
	int i;

	if (_level == 0)
	{
		if (string(name) != (string)"structures-file")
		{
			string message = "Root element name is ";
			message += name;
			message += "; expected structures-file";
			throw xh_io_exception(message, "XML Reader");
		}
		push_state(NULL, "top");
		return;
	}

	if (_level == 1)
	{
		if (string(name) == (string)"coordinates")
		{
			const char *type  = atts.getValue("type");
			const char *value = atts.getValue("value");
			m_pSA->m_crs.SetTextDescription(type, value);
		}
		else if (string(name) == (string)"structures")
		{
			push_state(NULL, "structures");
		}
		else
		{
			// Unknown element: ignore
			push_state(NULL, "dummy");
		}
		return;
	}

	const char *attval;

	if (_level == 2)
	{
		vtStructure *pStruct = NULL;
		if (string(name) == (string)"structure")
		{
			// Get the name.
			attval = atts.getValue("type");
			if (attval != NULL)
			{
				if (string(attval) == (string)"building")
				{
					vtBuilding *bld = m_pSA->AddNewBuilding();
					pStruct = bld;
				}
				if (string(attval) == (string)"linear")
				{
					vtFence *fen = m_pSA->AddNewFence();
					pStruct = fen;
				}
				if (string(attval) == (string)"instance")
				{
					vtStructInstance *inst = m_pSA->AddNewInstance();
					pStruct = inst;
				}
			}
			push_state(pStruct, "structure");
		}
		else
		{
			// Unknown field, ignore.
			pStruct = NULL;
			push_state(pStruct, "dummy");
		}
		return;
	}

	State &st = state();
	vtStructure *pStruct = st.item;
	if (!pStruct)
		return;
	vtFence *fen = pStruct->GetFence();
	vtBuilding *bld = pStruct->GetBuilding();
	vtStructInstance *inst = pStruct->GetInstance();

	if (_level == 3 && bld != NULL)
	{
		if (string(name) == (string)"height")
		{
			attval = atts.getValue("stories");
			if (attval)
			{
				// height in stories ("floors")
				int stories = atoi(attval);
				if (bld)
					bld->SetNumStories(stories);
			}
		}
		if (string(name) == (string)"walls")
		{
			attval = atts.getValue("color");
			if (bld && attval)
				st.wall_color = ParseRGB(attval);
		}
		if (string(name) == (string)"shapes")
		{
			push_state(pStruct, "shapes");
			return;
		}
		if (string(name) == (string)"roof")
		{
			// hack to postpone setting building color until now
			bld->SetColor(BLD_BASIC, st.wall_color);

			const char *type = atts.getValue("type");
			if (bld && (string)type == (string)"flat")
				bld->SetRoofType(ROOF_FLAT);
			if (bld && (string)type == (string)"shed")
				bld->SetRoofType(ROOF_SHED);
			if (bld && (string)type == (string)"gable")
				bld->SetRoofType(ROOF_GABLE);
			if (bld && (string)type == (string)"hip")
				bld->SetRoofType(ROOF_HIP);
			attval = atts.getValue("color");
			if (bld && attval)
				bld->SetColor(BLD_ROOF, ParseRGB(attval));
		}
		if (string(name) == (string)"points")
		{
			attval = atts.getValue("num");
		}
		return;
	}
	if (_level == 3 && fen != NULL)
	{
		if (string(name) == (string)"points")
		{
			int points;
			const char *num = atts.getValue("num");
			points = atoi(num);

			DLine2 &fencepts = fen->GetFencePoints();

			DPoint2 loc;
			const char *coords = atts.getValue("coords");
			const char *cp = coords;
			for (i = 0; i < points; i++)
			{
				sscanf(cp, "%lf %lf", &loc.x, &loc.y);
				fencepts.Append(loc);
				cp = strchr(cp, ' ');
				cp++;
				cp = strchr(cp, ' ');
				cp++;
			}
		}
		if (string(name) == (string)"height")
		{
			// absolute height in meters
			const char *abs_str = atts.getValue("abs");
			if (abs_str)
				fen->GetParams().m_fPostHeight = (float)atof(abs_str);
		}
		if (string(name) == (string)"posts")
		{
			// this linear structure has posts
			const char *type = atts.getValue("type");
			if (0 == strcmp(type, "wood"))
				fen->ApplyStyle(FS_WOOD_POSTS_WIRE);
			else if (0 == strcmp(type, "steel"))
				fen->ApplyStyle(FS_CHAINLINK);
			else
				fen->ApplyStyle(FS_METAL_POSTS_WIRE);

			const char *size = atts.getValue("size");
			FPoint3 postsize;
			postsize.y = fen->GetParams().m_fPostHeight;
			sscanf(size, "%f, %f", &postsize.x, &postsize.z);
			fen->GetParams().m_fPostWidth = postsize.x;
			fen->GetParams().m_fPostDepth = postsize.z;

			const char *spacing = atts.getValue("spacing");
			if (spacing)
				fen->GetParams().m_fPostSpacing = (float)atof(spacing);
		}
		if (string(name) == (string)"connect")
		{
			attval = atts.getValue("type");
			// not supported here
		}
		return;
	}
	if (_level == 3 && inst != NULL)
	{
		if (string(name) == (string)"placement")
		{
			const char *loc = atts.getValue("location");
			if (loc)
			{
				DPoint2 p;
				sscanf(loc, "%lf %lf", &p.x, &p.y);
				inst->SetPoint(p);
			}
			const char *rot = atts.getValue("rotation");
			if (rot)
			{
				float fRot;
				sscanf(rot, "%f", &fRot);
				inst->SetRotation(fRot);
			}
		}
		else
			_data = "";
	}
	if (_level == 4 && bld != NULL)
	{
		if (string(name) == (string)"rect")
		{
			DPoint2 loc;
			FPoint2 size2;
			const char *ref_point = atts.getValue("ref_point");
			if (ref_point)
			{
				sscanf(ref_point, "%lf %lf", &loc.x, &loc.y);
				bld->SetRectangle(loc, 10, 10);
			}
			float fRotation = 0.0f;
			const char *rot = atts.getValue("rot");
			if (rot)
			{
				fRotation = (float)atof(rot);
			}
			const char *size = atts.getValue("size");
			if (size)
			{
				sscanf(size, "%f, %f", &size2.x, &size2.y);
				bld->SetRectangle(loc, size2.x, size2.y, fRotation);
			}
		}
		if (string(name) == (string)"circle")
		{
			DPoint2 loc;
			const char *ref_point = atts.getValue("ref_point");
			if (ref_point)
			{
				sscanf(ref_point, "%lf %lf", &loc.x, &loc.y);
				bld->SetCircle(loc, 10);
			}
			const char *radius = atts.getValue("radius");
			if (radius)
			{
				bld->SetCircle(loc, (float)atof(radius));
			}
		}
		if (string(name) == (string)"poly")
		{
			int points;
			const char *num = atts.getValue("num");
			points = atoi(num);

			DLine2 foot;
			DPoint2 loc;
			const char *coords = atts.getValue("coords");
			const char *cp = coords;
			for (i = 0; i < points; i++)
			{
				sscanf(cp, "%lf %lf", &loc.x, &loc.y);
				foot.Append(loc);
				cp = strchr(cp, ' ');
				cp++;
				cp = strchr(cp, ' ');
				cp++;
			}
			bld->SetFootprint(0, foot);
		}
	}
}

void StructureVisitor::endElement(const char * name)
{
	State &st = state();
	vtStructure *pStruct = st.item;

	if (string(name) == (string)"structures")
	{
		pop_state();
	}
	if (string(name) == (string)"structure")
	{
		pop_state();
	}
	if (string(name) == (string)"shapes")
	{
		// currently, building wall information is not saved or restored, so
		// we must manually indicate that detail should be implied upon loading
		vtBuilding *bld = pStruct->GetBuilding();
		bld->AddDefaultDetails();

		pop_state();
	}
	if (_level == 3)
	{
		if (pStruct->GetType() == ST_INSTANCE &&
			string(name) != (string)"placement")
		{
			// save all other tags as literal strings
			vtTag tag;
			tag.name = name;
			tag.value = _data.c_str();

			pStruct->AddTag(tag);
		}
	}
}

void StructureVisitor::data(const char *s, int length)
{
	if (state().item != NULL)
		_data.append(string(s, length));
}


////////////////////////////////////////////////////////////////////////
// Visitor class, for XML parsing of GML Structure files.
////////////////////////////////////////////////////////////////////////

// helper
RGBi ParseHexColor(const char *str)
{
	RGBi color;
	sscanf(str,   "%2hx", &color.r);
	sscanf(str+2, "%2hx", &color.g);
	sscanf(str+4, "%2hx", &color.b);
	return color;
}

class StructVisitorGML : public XMLVisitor
{
public:
	StructVisitorGML(vtStructureArray *sa) :
	  m_state(0), m_pSA(sa) {}
	void startXML() { m_state = 0; }
	void endXML() { m_state = 0; }
	void startElement(const char *name, const XMLAttributes &atts);
	void endElement(const char *name);
	void data(const char *s, int length);

private:
	string m_data;
	int m_state;

	vtStructureArray *m_pSA;
	vtStructure *m_pStructure;
	vtBuilding *m_pBuilding;
	vtStructInstance *m_pInstance;
	vtFence *m_pFence;
	vtLevel *m_pLevel;
	vtEdge *m_pEdge;

	DPolygon2 m_Footprint;
	int m_iLevel;
	int m_iEdge;
};

void StructVisitorGML::startElement(const char *name, const XMLAttributes &atts)
{
	const char *attval;
	float f;

	// clear data at the start of each element
	m_data = "";

	if (m_state == 0 && !strcmp(name, "StructureCollection"))
	{
		m_state = 1;
		return;
	}

	if (m_state == 1)
	{
		if (!strcmp(name, "Building"))
		{
			m_pBuilding = m_pSA->AddNewBuilding();
			m_pStructure = m_pBuilding;
			m_state = 2;
			m_iLevel = 0;
		}
		else if (!strcmp(name, "Linear"))
		{
			m_pFence = m_pSA->AddNewFence();
			m_pFence->GetParams().Blank();
			m_pStructure = m_pFence;

			// support obsolete attribute
			attval = atts.getValue("Height");
			if (attval)
			{
				f = (float) atof(attval);
				m_pFence->GetParams().m_fPostHeight = f;
				m_pFence->GetParams().m_fConnectTop = f;
			}
			m_state = 10;
		}
		else if (!strcmp(name, "Imported"))
		{
			m_pInstance = m_pSA->AddNewInstance();
			m_pStructure = m_pInstance;

			m_state = 20;
		}
		attval = atts.getValue("ElevationOffset");
		if (attval)
			m_pStructure->SetElevationOffset((float) atof(attval));
		attval = atts.getValue("Absolute");
		if (attval)
			m_pStructure->SetAbsolute(*attval == 't');

		return;
	}

	if (m_state == 2)	// Building
	{
		if (!strcmp(name, "Level"))
		{
			m_pLevel = m_pBuilding->CreateLevel();
			attval = atts.getValue("FloorHeight");
			if (attval)
				m_pLevel->m_fStoryHeight = (float) atof(attval);
			attval = atts.getValue("StoryCount");
			if (attval)
				m_pLevel->m_iStories = atoi(attval);
			m_state = 3;
			m_iEdge = 0;
		}
		return;
	}

	if (m_state == 3)	// Level
	{
		if (!strcmp(name, "Footprint"))
		{
			m_state = 4;
			m_Footprint.clear();
		}
		else if (!strcmp(name, "Edge"))
		{
			m_pEdge = m_pLevel->GetEdge(m_iEdge);
			if (m_pEdge)	// safety check
			{
				m_pEdge->m_Features.clear();

				attval = atts.getValue("Material");
				if (attval)
				{
					vtMaterialDescriptorArray *mats = GetGlobalMaterials();
					m_pEdge->m_pMaterial = mats->FindName(attval);
					if (m_pEdge->m_pMaterial == NULL)
					{
						// What to do when a VTST references a material that
						// we don't have?  We don't want to lose the material
						// name information, and we also don't want to crash
						// later with a NULL material.  So, make a dummy.
						vtMaterialDescriptor *mat;
						mat = new vtMaterialDescriptor(attval, "", VT_MATERIAL_COLOURABLE);
						mat->SetRGB(RGBi(255,255,255));	// white means: missing
						mats->push_back(mat);
						m_pEdge->m_pMaterial = &mat->GetName();
					}
				}
				attval = atts.getValue("Color");
				if (attval)
					m_pEdge->m_Color = ParseHexColor(attval);
				attval = atts.getValue("Slope");
				if (attval)
					m_pEdge->m_iSlope = atoi(attval);
				attval = atts.getValue("Facade");
				if (attval)
					m_pEdge->m_Facade = attval;
			}

			m_state = 7;	// in Edge
		}
		return;
	}

	if (m_state == 4)	// Footprint
	{
		if (!strcmp(name, "gml:outerBoundaryIs"))
			m_state = 5;
		if (!strcmp(name, "gml:innerBoundaryIs"))
			m_state = 6;
	}

	if (m_state == 5)	// Footprint outerBoundaryIs
	{
		// nothing necessary here, catch the end of element
	}

	if (m_state == 6)	// Footprint innerBoundaryIs
	{
		// nothing necessary here, catch the end of element
	}

	if (m_state == 7)	// Edge
	{
		if (!strcmp(name, "EdgeElement"))
		{
			vtEdgeFeature ef;

			attval = atts.getValue("Type");
			if (attval)
				ef.m_code = vtBuilding::GetEdgeFeatureValue(attval);
			attval = atts.getValue("Begin");
			if (attval)
				ef.m_vf1 = (float) atof(attval);
			attval = atts.getValue("End");
			if (attval)
				ef.m_vf2 = (float) atof(attval);
			if (m_pEdge)
				m_pEdge->m_Features.push_back(ef);
		}
	}

	if (m_state == 10)	// Linear
	{
		vtLinearParams &param = m_pFence->GetParams();
		if (!strcmp(name, "Path"))
		{
			m_state = 11;
		}
		else if (!strcmp(name, "Posts"))
		{
			// this linear structure has posts
			const char *type = atts.getValue("Type");
			if (type)
				param.m_PostType = type;
			else
				param.m_PostType = "none";

			const char *spacing = atts.getValue("Spacing");
			if (spacing)
				param.m_fPostSpacing = (float)atof(spacing);

			const char *height = atts.getValue("Height");
			if (height)
				param.m_fPostHeight = (float)atof(height);

			const char *size = atts.getValue("Size");
			if (size)
			{
				FPoint3 postsize;
				sscanf(size, "%f, %f", &postsize.x, &postsize.z);
				param.m_fPostWidth = postsize.x;
				param.m_fPostDepth = postsize.z;
			}

			const char *exten = atts.getValue("Extension");
			if (exten)
				param.m_PostExtension = exten;
		}
		else if (!strcmp(name, "Connect"))
		{
			const char *type = atts.getValue("Type");
			if (type)
			{
				// Older format version had string "none", "wire", or a material for type
				// Newer format has integer 0,1,2,3 for none,wire,simple,profile
				if (*type >= '0' && *type <= '9')
					param.m_iConnectType = atoi(type);
				else
				{
					// Convert old to new
					if (!strcmp(type, "none"))
						param.m_iConnectType = 0;
					else if (!strcmp(type, "wire"))
						param.m_iConnectType = 1;
					else
					{
						param.m_iConnectType = 2;
						param.m_ConnectMaterial = type;
					}
				}
			}
			else
				param.m_iConnectType = 0;

			attval = atts.getValue("Material");
			if (attval)
				param.m_ConnectMaterial = attval;

			attval = atts.getValue("Top");
			if (attval)
				param.m_fConnectTop = (float)atof(attval);
			attval = atts.getValue("Bottom");
			if (attval)
				param.m_fConnectBottom = (float)atof(attval);
			attval = atts.getValue("Width");
			if (attval)
				param.m_fConnectWidth = (float)atof(attval);
			attval = atts.getValue("Slope");
			if (attval)
				param.m_iConnectSlope = atoi(attval);
			attval = atts.getValue("ConstantTop");
			if (attval)
				param.m_bConstantTop = (*attval == 't');

			attval = atts.getValue("Profile");
			if (attval)
				param.m_ConnectProfile = attval;
		}
	}
	if (m_state == 20)	// Imported
	{
		if (!strcmp(name, "Location"))
		{
			m_state = 21;
		}
	}
}

void DLine2FromString(const char *data, DLine2 &line)
{
	// Speed/memory optimization: quick check of how many vertices
	//  there are, then preallocate that many
	uint verts = 0;
	for (size_t i = 0; i < strlen(data); i++)
		if (data[i] == ',')
			verts++;
	line.Clear();
	line.SetMaxSize(verts);

	double x, y;
	while (sscanf(data, "%lf,%lf", &x, &y) == 2)
	{
		line.Append(DPoint2(x,y));
		data = strchr(data, ' ');
		if (!data)
			break;
		data++;
	}
}

void StructVisitorGML::endElement(const char *name)
{
	bool bGrabAttribute = false;
	const char *data = m_data.c_str();

	if (m_state == 7 && !strcmp(name, "Edge"))
	{
		m_iEdge++;
		m_state = 3;
	}
	else if (m_state == 6)	// inside Footprint innerBoundaryIs
	{
		if (!strcmp(name, "gml:coordinates"))
		{
			DLine2 line;
			DLine2FromString(data, line);
			m_Footprint.push_back(line);
		}
		else if (!strcmp(name, "gml:innerBoundaryIs"))
			m_state = 4;
	}
	else if (m_state == 5)	// inside Footprint outerBoundaryIs
	{
		if (!strcmp(name, "gml:coordinates"))
		{
			DLine2 line;
			DLine2FromString(data, line);
			m_Footprint.push_back(line);
		}
		else if (!strcmp(name, "gml:outerBoundaryIs"))
			m_state = 4;
	}
	else if (m_state == 4)	// inside Footprint
	{
		if (!strcmp(name, "Footprint"))
		{
			m_pLevel->SetFootprint(m_Footprint);
			m_state = 3;
		}
	}
	else if (m_state == 3 && !strcmp(name, "Level"))
	{
		m_state = 2;
		m_iLevel ++;
	}
	else if (m_state == 2)
	{
		if (!strcmp(name, "Building"))
		{
			m_state = 1;

			// do this once after we done adding levels
			m_pBuilding->DetermineLocalFootprints();

			m_pStructure = NULL;
		}
		else
			bGrabAttribute = true;
	}
	else if (m_state == 1 && (!strcmp(name, "SRS")))
	{
		m_pSA->m_crs.SetTextDescription("wkt", data);
	}
	else if (m_state == 10)
	{
		if (!strcmp(name, "Linear"))
		{
			m_state = 1;

			m_pStructure = NULL;
		}
	}
	else if (m_state == 11)
	{
		if (!strcmp(name, "gml:coordinates"))
		{
			DLine2 &fencepts = m_pFence->GetFencePoints();
			double x, y;
			while (sscanf(data, "%lf,%lf", &x, &y) == 2)
			{
				fencepts.Append(DPoint2(x,y));
				data = strchr(data, ' ');
				if (data)
					data++;
				else
					break;
			}
		}
		else if (!strcmp(name, "Path"))
			m_state = 10;
	}
	else if (m_state == 20)
	{
		if (!strcmp(name, "Imported"))
		{
			m_state = 1;
			m_pStructure = NULL;
		}
		else if (!strcmp(name, "Rotation"))
		{
			float fRot;
			sscanf(data, "%f", &fRot);
			m_pInstance->SetRotation(fRot);
		}
		else if (!strcmp(name, "Scale"))
		{
			float fScale;
			sscanf(data, "%f", &fScale);
			m_pInstance->SetScale(fScale);
		}
		else
			bGrabAttribute = true;
	}
	else if (m_state == 21)
	{
		if (!strcmp(name, "gml:coordinates"))
		{
			double x, y;
			sscanf(data, "%lf,%lf", &x, &y);
			m_pInstance->SetPoint(DPoint2(x,y));
		}
		else if (!strcmp(name, "Location"))
			m_state = 20;
	}

	// first check for Attribute nodes
	if (bGrabAttribute)
	{
		// save these elements as literal strings
		vtTag tag;
		tag.name = name;
		tag.value = data;
		m_pStructure->AddTag(tag);		// where does the tag go?
	}
}

void StructVisitorGML::data(const char *s, int length)
{
	m_data.append(string(s, length));
}

/////////////////////////////////////////////////////////////////////////


bool vtStructureArray::WriteXML(const char* filename, bool bGZip) const
{
	VTLOG("WriteXML(%s)\n", filename);

	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	GZOutput out(bGZip);
	if (!gfopen(out, filename))
	{
		throw xh_io_exception("Could not open output file", xh_location(filename),
				"XML Writer");
	}

	gfprintf(out, "<?xml version=\"1.0\"?>\n");
	gfprintf(out, "\n");

	gfprintf(out, "<StructureCollection xmlns=\"http://www.openplans.net\"\n"
		"\t\t\t\t\t xmlns:gml=\"http://www.opengis.net/gml\"\n"
		"\t\t\t\t\t xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
		"\t\t\t\t\t xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
		"\t\t\t\t\t xsi:schemaLocation=\"http://www.openplans.net/buildings.xsd\">\n");
	gfprintf(out, "\n");

	// Write the extents (required by gml:StructureCollection)
	DRECT ext;
	GetExtents(ext);
	gfprintf(out, "\t<gml:boundedBy>\n");
	gfprintf(out, "\t\t<gml:Box>\n");
	gfprintf(out, "\t\t\t<gml:coordinates>");
	gfprintf(out, "%.9lf,%.9lf %.9lf,%.9lf", ext.left, ext.bottom, ext.right, ext.top);
	gfprintf(out, "</gml:coordinates>\n");
	gfprintf(out, "\t\t</gml:Box>\n");
	gfprintf(out, "\t</gml:boundedBy>\n");
	gfprintf(out, "\n");

	// Write CRS
	char *wkt;
	OGRErr err = m_crs.exportToWkt(&wkt);
	if (err != OGRERR_NONE)
	{
		throw xh_io_exception("Couldn't write CRS to file", xh_location(filename),
				"XML Writer");
	}
	gfprintf(out, "\t<SRS>%s</SRS>\n", wkt);
	gfprintf(out, "\n");
	OGRFree(wkt);

	bool bDegrees = (m_crs.IsGeographic() == 1);

	for (uint i = 0; i < size(); i++)
	{
		vtStructure *str = at(i);
		str->WriteXML(out, bDegrees);
	}
	gfprintf(out, "</StructureCollection>\n");
	gfclose(out);
	return true;
}

bool vtStructureArray::ReadXML(const char *pathname, bool progress_callback(int))
{
	VTLOG("vtStructureArray::ReadXML: ");

	// The locale might be set to something European that interprets '.' as ','
	//  and vice versa, which would break our usage of sscanf/atof terribly.
	//  So, push the 'standard' locale, it is restored when it goes out of scope.
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	// check to see if it's old or new format
	bool bOldFormat = false;
	gzFile fp = vtGZOpen(pathname, "r");
	if (!fp)
		return false;

	m_strFilename = pathname;

	gzseek(fp, 24, SEEK_SET);
	char buf[10];
	gzread(fp, buf, 10);
	if (!strncmp(buf, "structures", 10))
		bOldFormat = true;
	else
	{
		// RFJ quick hack for extra carriage returns
		gzseek(fp, 26, SEEK_SET);
		gzread(fp, buf, 10);
		if (!strncmp(buf, "structures", 10))
			bOldFormat = true;
	}

	gzclose(fp);

	bool success = false;
	if (bOldFormat)
	{
		StructureVisitor visitor(this);
		try
		{
			readXML(pathname, visitor, progress_callback);
			success = true;
		}
		catch (xh_exception &ex)
		{
			// TODO: would be good to pass back the error message.
			VTLOG1("XML Error: ");
			VTLOG1(ex.getFormattedMessage().c_str());
			return false;
		}
	}
	else
	{
		StructVisitorGML visitor(this);
		try
		{
			readXML(pathname, visitor, progress_callback);
			success = true;
		}
		catch (xh_exception &ex)
		{
			// TODO: would be good to pass back the error message.
			VTLOG1("XML Error: ");
			VTLOG1(ex.getFormattedMessage().c_str());
			return false;
		}
	}

	return success;
}

bool vtStructureArray::WriteFootprintsToSHP(const char* filename)
{
	SHPHandle hSHP = SHPCreate(filename, SHPT_POLYGON);
	if (!hSHP)
		return false;

	vtString dbfname = filename;
	dbfname = dbfname.Left(dbfname.GetLength() - 4);
	dbfname += ".dbf";
	DBFHandle db = DBFCreate(dbfname);
	if (!db)
		return false;

	// Field 0: height in meters
	DBFAddField(db, "Height", FTDouble, 3, 2);	// width, decimals

	uint i, j, count = size(), record = 0;
	for (i = 0; i < count; i++)	//for each coordinate
	{
		vtBuilding *bld = at(i)->GetBuilding();
		if (!bld)
			continue;

		const DLine2 &poly = bld->GetLevel(0)->GetOuterFootprint();
		int total = poly.GetSize() + 1;

		double *dX = new double[total];
		double *dY = new double[total];

		int vert = 0;
		for (j=0; j < poly.GetSize(); j++) //for each vertex
		{
			DPoint2 pt = poly[j];
			dX[vert] = pt.x;
			dY[vert] = pt.y;
			vert++;
		}
		// duplicate first vertex, it's just what SHP files do.
		DPoint2 pt = poly[0];
		dX[vert] = pt.x;
		dY[vert] = pt.y;
		vert++;

		// Save to SHP
		SHPObject *obj = SHPCreateSimpleObject(SHPT_POLYGON, total, dX, dY, NULL);

		SHPWriteObject(hSHP, -1, obj);
		SHPDestroyObject(obj);

		delete [] dY;
		delete [] dX;

		// Save to DBF
		float h = bld->GetTotalHeight();
		DBFWriteDoubleAttribute(db, record, 0, h);

		// Because not every structure may be a building, there may be fewer
		//  records than structures.
		record++;
	}
	DBFClose(db);
	SHPClose(hSHP);
	return true;
}

/**
 * Write footprints to a Canoma file.
 * Author of this code: BobMaX (Roberto Angeletti)
 */
bool vtStructureArray::WriteFootprintsToCanoma3DV(const char* filename, const DRECT *area,
												  const vtHeightField *pHF)
{
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	FILE *fp3DV;
	double x1, y1, x2, y2;
	double minX, minY, maxX, maxY;
	double centerX, centerY, centerZ;
	double deltaX, deltaY;
	double alpha, beta, gamma, focale;
	double CX, CY, EX, EY, EZ;
	double numPixelX, numPixelY, PixelRapp;
	int groupName =1, numero =-1;
	int iii;
	float fElev;

	const float SCALE = 1.0f;
	const float IMAGEFIXEDY = 800.0f;

	VTLOG1("WriteFootprintsToCanoma3DV\n");

	fp3DV = vtFileOpen(filename, "wt");
	if (fp3DV == NULL) {
		VTLOG(" couldn't open file.\n");
		return false;
	}

	x1 = area->left;
	y1 = area->bottom;
	x2 = area->right;
	y2 = area->top;

	fprintf(fp3DV, "version 1\n");

	uint i, j, count = size(), record = 0;
	for (i = 0; i < count; i++)	//for each coordinate
	{
		vtBuilding *bld = at(i)->GetBuilding();
		if (!bld)
			continue;

		const DLine2 &poly = bld->GetLevel(0)->GetOuterFootprint();
		int total = poly.GetSize() + 1;

		double *dX = new double[total];
		double *dY = new double[total];

		minX =  1E9;
		minY =  1E9;
		maxX = -1E9;
		maxY = -1E9;

		int vert = 0;
		for (j=0; j < poly.GetSize(); j++) //for each vertex
		{
			DPoint2 pt = poly[j];

			if (pHF != NULL)
				pHF->FindAltitudeOnEarth(pt, fElev);
			else
				fElev = 0.00;

			if (pt.x < minX){minX = pt.x;}
			if (pt.y < minY){minY = pt.y;}
			if (pt.x > maxX){maxX = pt.x;}
			if (pt.y > maxY){maxY = pt.y;}

			dX[vert] = pt.x;
			dY[vert] = pt.y;
			vert++;
		}
		// duplicate first vertex, it's just what SHP files do.
		DPoint2 pt = poly[0];
		dX[vert] = pt.x;
		dY[vert] = pt.y;
		vert++;

		//	float h = bld->GetHeight();
		float h = bld->GetTotalHeight();

		double Hight = h;

		//    WriteTranslationSweep();  quello che segue
		centerX = (maxX + minX)/2.;
		centerY = (maxY + minY)/2.;
		centerZ = 0.00;

		//VTLOG("EXPORTTOCANOMA center %lf %lf\n", centerX, centerY);
		numero++;

		if (centerX > x1 &&
			centerY > y1 &&
			centerX < x2 &&
			centerY < y2 )
		{
			//    	VTLOG("EXPORTTOCANOMA building contained\n");
			fprintf(fp3DV, "translationsweep TSW_%d 2 %d { \n",  numero, vert-1);
			fprintf(fp3DV, "	state { \n");
			fprintf(fp3DV, "		alpha { 0.00000 f } \n");
			fprintf(fp3DV, "		beta { 0.00000 f } \n");
			fprintf(fp3DV, "		gamma { 0.00000 f } \n");
			fprintf(fp3DV, "		X0 { %lf f } \n", (centerX - x1) / SCALE);
			fprintf(fp3DV, "		Y0 { %lf f } \n", (centerY - y1) / SCALE);
			fprintf(fp3DV, "		Z0 { %lf f } \n", fElev / SCALE);
			fprintf(fp3DV, "		majorAxis { %lf f }\n", Hight /SCALE);

			for (iii = 0; iii < vert-1; iii++)
			{
				fprintf(fp3DV, "		u%d { %lf f }\n", iii, (centerX - dX[iii]  ) / SCALE );
				fprintf(fp3DV, "		v%d { %lf f }\n", iii, (dY[iii] - centerY  ) / SCALE );
			}  // End For

			fprintf(fp3DV, "		} \n");
			fprintf(fp3DV, "	} \n");

		} // If CONTAINED

		delete [] dY;
		delete [] dX;

		// Because not every structure may be a building, there may be fewer
		//  records than structures.
		record++;
	}
	deltaX = x2-x1;
	deltaY = y2-y1;

	EX = deltaX/2.; //((x2 - x1) /2.); //+ x1;
	EX = EX / SCALE;
	EY = deltaY/2.; //((y2 - y1) /2.); //+ y1;
	EY = EY / SCALE;
	EZ = 202.020865;

	alpha = -3.14159; //0.000000; // -2.85841;  // 0.000000;
	beta = 0.000000;  // 0.03926;    // 1.110737;
	gamma = -3.14159; // 3.14159;   // 0.000000;
	focale = 60.343804;
	CX = 0.000000;
	CY = 0.000000;

	fprintf(fp3DV, "rectangle Floor {\n");
	fprintf(fp3DV, "	state { \n");
	fprintf(fp3DV, "		alpha { 0.00000 f } \n");
	fprintf(fp3DV, "		beta { 0.00000 f } \n");
	fprintf(fp3DV, "		gamma { 0.00000 f } \n");
	fprintf(fp3DV, "		X0 { %lf f }\n", deltaX/2. /SCALE);
	fprintf(fp3DV, "		Y0 { %lf f }\n", deltaY/2. /SCALE);
	fprintf(fp3DV, "		Z0 { 0.00000 f }\n");
	fprintf(fp3DV, "		L { %lf f }\n",deltaX /SCALE);
	fprintf(fp3DV, "		W { %lf f }\n",deltaY /SCALE);
	fprintf(fp3DV, "		}\n");
	fprintf(fp3DV, "	} \n");

	fprintf(fp3DV, "camera { \n");
	fprintf(fp3DV, "	state { \n");
	fprintf(fp3DV, "		EX { %lf } \n", EX);
	fprintf(fp3DV, "		EY { %lf } \n", EY);
	fprintf(fp3DV, "		EZ { %lf } \n", EZ);
	fprintf(fp3DV, "		alpha { %lf } \n", alpha);
	fprintf(fp3DV, "		beta { %lf } \n", beta);
	fprintf(fp3DV, "		gamma { %lf } \n", gamma);
	fprintf(fp3DV, "		f { %lf } \n", focale);
	fprintf(fp3DV, "		CX { %lf } \n", CX);
	fprintf(fp3DV, "		CY { %lf } \n", CY);
	fprintf(fp3DV, "		} \n");
	fprintf(fp3DV, "	} \n");

	numPixelY = IMAGEFIXEDY;
	PixelRapp = deltaY / numPixelY;
	numPixelX = deltaX / PixelRapp;

	fprintf(fp3DV, "controls {\n");
	fprintf(fp3DV, "	Point Floor 0 0.00000 0.00000 GEImage.jpg ;\n");
	fprintf(fp3DV, "	Point Floor 6 %lf 0.00000 GEImage.jpg ;\n", numPixelX);
	fprintf(fp3DV, "	Point Floor 9 %lf %lf GEImage.jpg ;\n", numPixelX, numPixelY);
	fprintf(fp3DV, "	Point Floor 3 0.00000 %lf GEImage.jpg ;\n", numPixelY);
	fprintf(fp3DV, "	}\n");

	fprintf(fp3DV, "image GEImage.jpg { \n");
	fprintf(fp3DV, "	camera { \n");
	fprintf(fp3DV, "		state { \n");
	fprintf(fp3DV, "			EX { %lf } \n", EX);
	fprintf(fp3DV, "			EY { %lf } \n", EY);
	fprintf(fp3DV, "			EZ { %lf } \n", EZ);
	fprintf(fp3DV, "			alpha { %lf } \n", alpha);
	fprintf(fp3DV, "			beta { %lf } \n", beta);
	fprintf(fp3DV, "			gamma { %lf } \n", gamma);
	fprintf(fp3DV, "			f { %lf } \n", focale);
	fprintf(fp3DV, "			CX { %lf } \n", CX);
	fprintf(fp3DV, "			CY { %lf } \n", CY);
	fprintf(fp3DV, "		      } \n");
	fprintf(fp3DV, "	        } \n");
	fprintf(fp3DV, "	{ %lf %lf } \n", numPixelX, numPixelY);
	fprintf(fp3DV, "	} \n");
	fprintf(fp3DV, "selection GEImage.jpg Floor n \n");
	fprintf(fp3DV, "calibration 1.00000 \n");

	fclose(fp3DV);

	return true;
}

//  BobMaX ExportToCanoma End --------------------------------------------


vtBuilding *vtStructureArray::AddBuildingFromLineString(OGRLineString *pLineString)
{
	int num_points = pLineString->getNumPoints();
	int j;

	// Ignore last point if it is the same as the first
	DPoint2 p1(pLineString->getX(0), pLineString->getY(0));
	DPoint2 p2(pLineString->getX(num_points - 1), pLineString->getY(num_points - 1));
	if (p1 == p2)
		num_points--;

	// Copy from OGR
	DLine2 footprint(num_points);
	for (j = 0; j < num_points; j++)
		footprint.SetAt(j, DPoint2(pLineString->getX(j), pLineString->getY(j)));

	PolyChecker PolyChecker;

	// Remove redundant points
	double dEpsilon = 1;
	for (j = 0; j < num_points && num_points > 2; j++)
	{
		DPoint2 p0 = footprint.GetSafePoint(j-1);
		DPoint2 p1 = footprint.GetSafePoint(j);
		DPoint2 p2 = footprint.GetSafePoint(j+1);
		if (PolyChecker.AreaSign(p0, p1, p2, dEpsilon) == 0)
		{
			footprint.RemoveAt(j);
			num_points--;
			j--;
		}
	}

	if (num_points < 3)
		return NULL;

	// Force footprint anticlockwise
	if (PolyChecker.IsClockwisePolygon(footprint))
		footprint.ReverseOrder();

	vtBuilding *bld = AddNewBuilding();
	bld->SetFootprint(0, footprint);
	return bld;
}


/////////////////////
// Helpers

int GetSHPType(const char *filename)
{
	// SHPOpen doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(filename);

	SHPHandle hSHP = SHPOpen(fname_local, "rb");
	if (hSHP == NULL)
		return SHPT_NULL;

	int nEntities, nShapeType;

	SHPGetInfo(hSHP, &nEntities, &nShapeType, NULL, NULL);
	SHPClose(hSHP);
	return nShapeType;
}

vtBuilding *GetClosestDefault(vtBuilding *pBld)
{
	// For now, just grab the first building from the defaults
	int i, num = g_DefaultStructures.size();
	for (i = 0; i < num; i++)
	{
		vtStructure *pStr = g_DefaultStructures[i];
		vtBuilding *pDefBld = pStr->GetBuilding();
		if (pDefBld)
			return pDefBld;
	}
	return NULL;
}

vtFence *GetClosestDefault(vtFence *pFence)
{
	// For now, just grab the first fence from the defaults
	int i, num = g_DefaultStructures.size();
	for (i = 0; i < num; i++)
	{
		vtStructure *pStr = g_DefaultStructures[i];
		vtFence *pDefFence = pStr->GetFence();
		if (pDefFence)
			return pDefFence;
	}
	return NULL;
}

vtStructInstance *GetClosestDefault(vtStructInstance *pInstance)
{
	// For now, just grab the first instance from the defaults
	int i, num = g_DefaultStructures.size();
	for (i = 0; i < num; i++)
	{
		vtStructure *pStr = g_DefaultStructures[i];
		vtStructInstance *pDefInstance = pStr->GetInstance();
		if (pDefInstance)
			return pDefInstance;
	}
	return NULL;
}

bool SetupDefaultStructures(const vtString &fname)
{
	VTLOG("SetupDefaultStructures('%s')\n", (const char *) fname);

	if (fname != "")
	{
		VTLOG1(" reading from XML: ");
		if (g_DefaultStructures.ReadXML(fname))
		{
			VTLOG1("succeeded.\n");
			return true;
		}
		VTLOG1("failed.\n");
	}
	return false;
}

