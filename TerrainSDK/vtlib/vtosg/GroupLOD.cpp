//
// GroupGroupLOD.cpp
//
// Copyright (c) 2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "GroupLOD.h"

using namespace osg;

float GroupLOD::_groupDistance = 0.0;

GroupLOD::GroupLOD(): _radius(-1.0f)
{
}

GroupLOD::GroupLOD(const GroupLOD& GroupLOD,const CopyOp& copyop):
        Group(GroupLOD,copyop),
        _radius(GroupLOD._radius),
        _distance(GroupLOD._distance)
{
}

void GroupLOD::traverse(NodeVisitor& nv)
{
    switch(nv.getTraversalMode())
    {
        case(NodeVisitor::TRAVERSE_ALL_CHILDREN):
            std::for_each(_children.begin(),_children.end(),NodeAcceptOp(nv));
            break;
        case(NodeVisitor::TRAVERSE_ACTIVE_CHILDREN):
        {
            float dist = nv.getDistanceToViewPoint(getCenter(),true);
            if (dist < (_distance + _groupDistance))
			{
		        std::for_each(_children.begin(),_children.end(),NodeAcceptOp(nv));
            }
           break;
        }
        default:
            break;
    }
}
