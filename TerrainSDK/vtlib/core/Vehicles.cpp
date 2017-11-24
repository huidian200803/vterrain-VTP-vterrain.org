//
// Vehicles.cpp
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/Content3d.h"	// for vtGetContent
#include "vtlib/core/GeomUtil.h"		// for CreateBoundSphereGeode
#include "vtdata/vtLog.h"
#include "CarEngine.h"
#include "Vehicles.h"


///////////////////////////////
// helpers

// Walk an OSG scenegraph looking for geodes with statesets, change the color
//  of any materials found.
class RemapDiffuseVisitor : public osg::NodeVisitor
{
public:
	RemapDiffuseVisitor() : NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
	virtual void apply(osg::Geode& geode)
	{
		for (unsigned i=0; i<geode.getNumDrawables(); ++i)
		{
			osg::Geometry *geo = dynamic_cast<osg::Geometry *>(geode.getDrawable(i));
			if (!geo) continue;

			osg::StateSet *stateset = geo->getStateSet();
			if (!stateset) continue;

			osg::StateAttribute *state = stateset->getAttribute(osg::StateAttribute::MATERIAL);
			if (!state) continue;

			osg::Material *mat = dynamic_cast<osg::Material *>(state);
			if (!mat) continue;

			const osg::Vec4 v4 = mat->getDiffuse(FAB);
			if (v4.r() == 1.0f && v4.g() == 0.0f && v4.b() == 1.0f)
			{
				//VTLOG("oldmat rc %d, ", mat->referenceCount());

				osg::Material *newmat = (osg::Material *)mat->clone(osg::CopyOp::DEEP_COPY_ALL);
				newmat->setDiffuse(FAB, osg::Vec4(c.r*2/3,c.g*2/3,c.b*2/3,1));
				newmat->setAmbient(FAB, osg::Vec4(c.r*1/3,c.g*1/3,c.b*1/3,1));

				//VTLOG("newmat rc %d\n", newmat->referenceCount());

				stateset->setAttribute(newmat);
				//VTLOG(" -> %d %d\n", mat->referenceCount(), newmat->referenceCount());
			}
		}
		osg::NodeVisitor::apply(geode);
	}
	RGBf c;
};

void ConvertPurpleToColor(osg::Node *pModel, RGBf replace)
{
	RemapDiffuseVisitor viz;
	viz.c = replace;
	pModel->accept(viz);
}

/////////////////////////////////////////

VehicleManager::VehicleManager()
{
}

VehicleManager::~VehicleManager()
{
}

Vehicle *VehicleManager::CreateVehicleFromNode(osg::Node *node, const RGBf &cColor)
{
	// Deep copy
	NodePtr pNewModel = (osg::Node *) node->clone(osg::CopyOp::DEEP_COPY_NODES);
	if (!pNewModel.valid())
		return NULL;

	osg::Group *group = dynamic_cast<osg::Group*>(pNewModel.get());

	//VTLOG1("-----------------\n");
	//vtLogGraph(node);

	//VTLOG1("-----------------\n");
	//vtLogGraph(pNewModel);

	osg::Node *pFrontLeft = FindDescendent(group, "front_left");
	osg::Node *pFrontRight = FindDescendent(group, "front_right");
	osg::Node *pRearLeft = FindDescendent(group, "rear_left");
	osg::Node *pRearRight = FindDescendent(group, "rear_right");

	if (!pFrontLeft || !pFrontRight || !pRearLeft || !pRearRight)
	{
		// Didn't find them.
		return NULL;
	}

	Vehicle *pNewVehicle = new Vehicle;
	pNewVehicle->addChild(pNewModel);

	// Stick transform above them
	pNewVehicle->m_pFrontLeft = new vtTransform;
	pNewVehicle->m_pFrontLeft->setName("front_left_xform");
	InsertNodeBelow(pFrontLeft->asGroup(), pNewVehicle->m_pFrontLeft);

	pNewVehicle->m_pFrontRight = new vtTransform;
	pNewVehicle->m_pFrontRight->setName("front_right_xform");
	InsertNodeBelow(pFrontRight->asGroup(), pNewVehicle->m_pFrontRight);

	pNewVehicle->m_pRearLeft = new vtTransform;
	pNewVehicle->m_pRearLeft->setName("rear_left_xform");
	InsertNodeBelow(pRearLeft->asGroup(), pNewVehicle->m_pRearLeft);

	pNewVehicle->m_pRearRight = new vtTransform;
	pNewVehicle->m_pRearRight->setName("rear_right_xform");
	InsertNodeBelow(pRearRight->asGroup(), pNewVehicle->m_pRearRight);

	//VTLOG1("-----------------\n");
	//vtLogGraph(pNewModel);

	// Replace the special 'purple' materials in the model with our color of choice
	ConvertPurpleToColor(pNewVehicle, cColor);

	return pNewVehicle;
}


Vehicle *VehicleManager::CreateVehicle(const char *szType, const RGBf &cColor)
{
	vtContentManager3d &con = vtGetContent();
	osg::Node *node = con.CreateNodeFromItemname(szType);
	if (!node)
		return NULL;
	Vehicle *v = CreateVehicleFromNode(node, cColor);
	if (v)
		v->setName(vtString("Vehicle-") + szType);
	return v;
}


///////////////////////////////////////////////////////////////////////

Vehicle::Vehicle()
{
	m_pHighlight = NULL;
	m_pFrontLeft = NULL;
	m_pFrontRight = NULL;
	m_pRearLeft = NULL;
	m_pRearRight = NULL;
}

Vehicle::~Vehicle()
{
}


void Vehicle::ShowBounds(bool bShow)
{
	if (bShow)
	{
		if (!m_pHighlight)
		{
			// the highlight geometry doesn't exist, so create it
			// get bounding sphere
			osg::Node *contents = getChild(0);
			if (contents)
			{
				FSphere sphere;
				GetBoundSphere(sphere);
				sphere.center.Set(0,0,0);

				m_pHighlight = CreateBoundSphereGeode(sphere);
				addChild(m_pHighlight);
			}
		}
		m_pHighlight->SetEnabled(true);
	}
	else
	{
		if (m_pHighlight)
			m_pHighlight->SetEnabled(false);
	}
}


///////////////////////////////////////////////////////////////////////

VehicleSet::VehicleSet()
{
	m_iSelected = -1;
}

void VehicleSet::AddEngine(CarEngine *e)
{
	m_Engines.push_back(e);
}

int VehicleSet::FindClosestVehicle(const FPoint3 &point, double &closest)
{
	closest = 1E9;
	int vehicle = -1;
	for (uint i = 0; i < m_Engines.size(); i++)
	{
		FPoint3 vepos = m_Engines[i]->GetCurPos();
		float dist = (point - vepos).Length();
		if (dist < closest)
		{
			closest = dist;
			vehicle = i;
		}
	}
	return vehicle;
}

void VehicleSet::VisualSelect(int vehicle)
{
	// Stop vehicle simulation while it is selected
	CarEngine *eng = m_Engines[vehicle];
	eng->SetEnabled(false);

	Vehicle *car = dynamic_cast<Vehicle*> (eng->GetTarget());
	if (!car)
		return;
	car->ShowBounds(true);
	m_iSelected = vehicle;
}

void VehicleSet::VisualDeselectAll()
{
	uint size = m_Engines.size();
	for (uint i = 0; i < size; i++)
	{
		// Resume vehicle simulation while it is deselected
		CarEngine *eng = m_Engines[i];
		eng->SetEnabled(true);
		eng->IgnoreElapsedTime();

		Vehicle *car = dynamic_cast<Vehicle*> (eng->GetTarget());
		if (car)
			car->ShowBounds(false);
	}
	m_iSelected = -1;
}

void VehicleSet::SetVehicleSpeed(int vehicle, float fMetersPerSec)
{
	CarEngine *eng = m_Engines[vehicle];
	eng->SetSpeed(fMetersPerSec);
}

CarEngine *VehicleSet::GetSelectedCarEngine()
{
	if (m_iSelected != -1)
		return m_Engines[m_iSelected];
	else
		return NULL;
}

