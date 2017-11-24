//
// NodeLog.cpp
//
// Functions for logging the nodes of the scene graph
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/vtLog.h"

#include <osg/AutoTransform>
#include <osg/Billboard>
#include <osg/ClipNode>
#include <osg/CoordinateSystemNode>
#include <osg/OccluderNode>
#include <osg/PositionAttitudeTransform>
#include <osg/Switch>
#include <osg/Sequence>
#include <osg/TexGenNode>

#include <osgParticle/ModularEmitter>
#include <osgParticle/ParticleSystemUpdater>


/**
 Diagnostic function to help debugging: Log the scene graph from a given node downwards.
*/
void vtLogGraph(osg::Node *node, bool bExtents, bool bRefCounts, int indent)
{
	for (int i = 0; i < indent; i++)
		VTLOG1(" ");
	if (node)
	{
		VTLOG("<%x>", node);

		if (dynamic_cast<osg::PositionAttitudeTransform*>(node))
			VTLOG1(" (PositionAttitudeTransform)");
		else if (dynamic_cast<osg::MatrixTransform*>(node))
			VTLOG1(" (MatrixTransform)");
		else if (dynamic_cast<osg::AutoTransform*>(node))
			VTLOG1(" (AutoTransform)");
		else if (dynamic_cast<osg::Transform*>(node))
			VTLOG1(" (Transform)");

		else if (dynamic_cast<osg::TexGenNode*>(node))
			VTLOG1(" (TexGenNode)");
		else if (dynamic_cast<osg::Switch*>(node))
			VTLOG1(" (Switch)");
		else if (dynamic_cast<osg::Sequence*>(node))
			VTLOG1(" (Sequence)");
		else if (dynamic_cast<osg::Projection*>(node))
			VTLOG1(" (Projection)");
		else if (dynamic_cast<osg::OccluderNode*>(node))
			VTLOG1(" (OccluderNode)");
		else if (dynamic_cast<osg::LightSource*>(node))
			VTLOG1(" (LightSource)");

		else if (dynamic_cast<osg::LOD*>(node))
			VTLOG1(" (LOD)");
		else if (dynamic_cast<osg::CoordinateSystemNode*>(node))
			VTLOG1(" (CoordinateSystemNode)");
		else if (dynamic_cast<osg::ClipNode*>(node))
			VTLOG1(" (ClipNode)");
		else if (dynamic_cast<osg::ClearNode*>(node))
			VTLOG1(" (ClearNode)");
		else if (dynamic_cast<osg::Group*>(node))
			VTLOG1(" (Group)");

		else if (dynamic_cast<osg::Billboard*>(node))
			VTLOG1(" (Billboard)");
		else if (dynamic_cast<osg::Geode*>(node))
			VTLOG1(" (Geode)");

		else if (dynamic_cast<osgParticle::ModularEmitter*>(node))
			VTLOG1(" (Geode)");
		else if (dynamic_cast<osgParticle::ParticleSystemUpdater*>(node))
			VTLOG1(" (Geode)");
		else
			VTLOG1(" (non-node!)");

		VTLOG(" '%s'", node->getName().c_str());

		if (node->getNodeMask() != 0xffffffff)
			VTLOG(" mask=%x", node->getNodeMask());

		if (node->getStateSet() != NULL)
			VTLOG(" (has stateset)");

		if (bExtents)
		{
			const osg::BoundingSphere &bs = node->getBound();
			if (bs._radius != -1)
				VTLOG(" (bs: %.1f %.1f %.1f %1.f)", bs._center[0], bs._center[1], bs._center[2], bs._radius);
		}
		if (bRefCounts)
		{
			VTLOG(" {rc:%d}", node->referenceCount());
		}

		VTLOG1("\n");
	}
	else
		VTLOG1("<null>\n");

	osg::MatrixTransform *mt = dynamic_cast<osg::MatrixTransform*>(node);
	if (mt)
	{
		for (int i = 0; i < indent+1; i++)
			VTLOG1(" ");
		osg::Vec3 v = mt->getMatrix().getTrans();
		VTLOG("[Pos %.2f %.2f %.2f]\n", v.x(), v.y(), v.z());
	}
	osg::Group *grp = dynamic_cast<osg::Group*>(node);
	if (grp)
	{
		for (uint i = 0; i < grp->getNumChildren(); i++)
			vtLogGraph(grp->getChild(i), bExtents, bRefCounts, indent+2);
	}
	osg::Geode *geode = dynamic_cast<osg::Geode*>(node);
	if (geode)
	{
		for (uint i = 0; i < geode->getNumDrawables(); i++)
		{
			osg::Geometry *geo = dynamic_cast<osg::Geometry *>(geode->getDrawable(i));
			if (!geo) continue;

			osg::StateSet *stateset = geo->getStateSet();
			if (!stateset) continue;

			for (int j = 0; j < indent+3; j++)
				VTLOG1(" ");

			VTLOG("drawable %d: geometry %x, stateset %x", i, geo, stateset);

			osg::StateAttribute *state = stateset->getAttribute(osg::StateAttribute::MATERIAL);
			if (state)
			{
				osg::Material *mat = dynamic_cast<osg::Material *>(state);
				if (mat)
					VTLOG(", mat %x", mat);
			}
			VTLOG1("\n");
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// Functions for writing to file to make 'dot' graphs
//

void WriteDot(void *parent, void *node, const char *label, int color, int depth,
			  FILE *fp)
{
	fprintf(fp, "\n");
	for (int i = 0; i < depth; i++)
		fprintf(fp, " ");

	const char *cname = "#FFFFFF";
	switch (color)
	{
	case 0:		// camera
		cname = "#FFFFD0";
		break;
	case 2:		// geode
		cname = "#CCFFCC";
		break;
	case 3:		// group
		cname = "#E6E6E6";
		break;
	case 4:		// light
		cname = "#FFFFD0";
		break;
	case 5:		// lod
		cname = "#D9C9B9";
		break;
	case 6:		// mesh
		cname = "#D0D0FF";
		break;
	case 7:		// top
		cname = "#E6E6E6";
		break;
	case 8:		// unknown
		break;
	case 9:		// xform
		cname = "#FFCCCC";
		break;
	case 10:	// fog
		cname = "#F3F3F3";
		break;
	case 11:	// shadow
		cname = "#CCCCCC";
		break;
	case 12:	// hud
		break;
	case 13:	// dyngeom
		cname = "#D0FFFF";
		break;
	}

	fprintf(fp, "n%p [label=\"%s\", fillcolor=\"%s\"];\n", node, label, cname);

	if (parent != NULL)
	{
		for (int i = 0; i < depth; i++)
			fprintf(fp, " ");
		fprintf(fp, "n%p -> n%p;\n", parent, node);
	}
}

void LogToDot(osg::Group *pParent, osg::Node *pNode, int depth, FILE *fp, std::vector<void*> &visited)
{
	vtString label;
	int nColor;

	if (!pNode) return;

	if (dynamic_cast<vtLightSource*>(pNode))
	{
		label = "Light";
		nColor = 4;
	}
	else if (dynamic_cast<vtDynGeom*>(pNode))
	{
		label = "DynGeom";
		nColor = 13;
	}
	else if (dynamic_cast<vtGeode*>(pNode))
	{
		label = "Geode";
		nColor = 2;
	}
	else if (dynamic_cast<vtLOD*>(pNode))
	{
		label = "LOD";
		nColor = 5;
	}
	else if (dynamic_cast<vtTransform*>(pNode))
	{
		label = "Transform";
		nColor = 9;
	}
	else if (dynamic_cast<vtFog*>(pNode))
	{
		label = "Fog";
		nColor = 10;
	}
	else if (dynamic_cast<vtShadow*>(pNode))
	{
		label = "Shadow";
		nColor = 11;
	}
	else if (dynamic_cast<vtHUD*>(pNode))
	{
		label = "HUD";
		nColor = 11;
	}
	else if (dynamic_cast<vtGroup*>(pNode))
	{
		// may be just a group for grouping's sake
		label = "Group";
		nColor = 3;
	}
	// Or, raw OSG nodes
	else if (dynamic_cast<osg::MatrixTransform*>(pNode))
	{
		label = "MatrixTransform";
		nColor = 9;
	}
	else if (dynamic_cast<osg::LOD*>(pNode))
	{
		label = "LOD";
		nColor = 5;
	}
	else if (dynamic_cast<osg::Group*>(pNode))
	{
		label = "Group";
		nColor = 3;
	}
	else if (dynamic_cast<osg::Geode*>(pNode))
	{
		label = "Geode";
		nColor = 2;
	}
	else
	{
		// must be something else
		label = "Other";
		nColor = 8;
	}
	if (pNode->getName() != "")
	{
		label += "\\n\'";
		label += pNode->getName().c_str();
		label += "\'";
	}

	// (add node)
	WriteDot(pParent, pNode, label, nColor, depth, fp);

	char buf[80];
	vtGeode *pGeode = dynamic_cast<vtGeode*>(pNode);
	osg::Geode *geode = dynamic_cast<osg::Geode*>(pNode);
	if (pGeode)
	{
		int num_mesh = pGeode->NumMeshes();

		for (int i = 0; i < num_mesh; i++)
		{
			osg::Drawable *draw = pGeode->getDrawable(i);

			// make sure we haven't already visited this drawable
			bool SeenIt = false;
			for (size_t j = 0; j < visited.size(); j++)
				if (visited[j] == draw)
					SeenIt = true;
			if (SeenIt)
				continue;
			visited.push_back(draw);

			vtMesh *pMesh = pGeode->GetMesh(i);
			vtTextMesh *pTextMesh = pGeode->GetTextMesh(i);
			osg::Geometry *geom = dynamic_cast<osg::Geometry*>(draw);

			if (pMesh)
			{
				int iNumPrim = pMesh->NumPrims();
				int iNumVert = pMesh->NumVertices();

				vtMesh::PrimType ptype = pMesh->getPrimType();
				const char *mtype="";
				switch (ptype)
				{
				case osg::PrimitiveSet::POINTS: mtype = "Points"; break;
				case osg::PrimitiveSet::LINES: mtype = "Lines"; break;
				case osg::PrimitiveSet::LINE_STRIP: mtype = "LineStrip"; break;
				case osg::PrimitiveSet::TRIANGLES: mtype = "Triangles"; break;
				case osg::PrimitiveSet::TRIANGLE_STRIP: mtype = "TriStrip"; break;
				case osg::PrimitiveSet::TRIANGLE_FAN: mtype = "TriFan"; break;
				case osg::PrimitiveSet::QUADS: mtype = "Quads"; break;
				case osg::PrimitiveSet::QUAD_STRIP: mtype = "QuadStrip"; break;
				case osg::PrimitiveSet::POLYGON: mtype = "Polygon"; break;
				default:	// Keep picky compilers quiet.
					break;
				}
				sprintf(buf, "%hs, %d/%d prims", mtype, iNumVert, iNumPrim);
				label = buf;
				nColor = 6;
			}
			else if (pTextMesh)
			{
				label = "Text";
				nColor = 6;
			}
			else if (geom)
			{
				int iNumVert = geom->getVertexArray()->getNumElements();
				sprintf(buf, "Drawable, %d verts", iNumVert);
				nColor = 6;
			}
			else
			{
				label = "Drawable";
				nColor = 6;
			}
			WriteDot(pNode, draw, label, nColor, depth+1, fp);
		}
	}
	else if (geode)
	{
		int num_draw = geode->getNumDrawables();

		for (int i = 0; i < num_draw; i++)
		{
			osg::Drawable *draw = pGeode->getDrawable(i);

			// make sure we haven't already visited this drawable
			bool SeenIt = false;
			for (size_t j = 0; j < visited.size(); j++)
				if (visited[j] == draw)
					SeenIt = true;
			if (SeenIt)
				continue;
			visited.push_back(draw);

			osg::Geometry *geom = dynamic_cast<osg::Geometry*>(draw);
			if (geom)
			{
				sprintf(buf, "%d verts", geom->getVertexArray()->getNumElements());
				label = buf;
				nColor = 6;
			}
			else
			{
				label = "drawable";
				nColor = 6;
			}
			WriteDot(pNode, draw, label, nColor, depth+1, fp);
		}
	}

	// make sure we haven't already visited this node
	bool SeenIt = false;
	for (size_t j = 0; j < visited.size(); j++)
		if (visited[j] == pNode)
			SeenIt = true;
	if (SeenIt)
		return;

	visited.push_back(pNode);

	osg::Group *group = dynamic_cast<osg::Group*>(pNode);
	if (group)
	{
		int num_children = group->getNumChildren();
		if (num_children > 200)
		{
			sprintf(buf, "(%d children)", num_children);
			label = buf;
			nColor = 8;
			void *dummy = pNode+1;
			WriteDot(pNode, dummy, label, nColor, depth+1, fp);
		}
		else
		{
			for (int i = 0; i < num_children; i++)
			{
				osg::Node *pChild = group->getChild(i);

				LogToDot(group, pChild, depth+1, fp, visited);
			}
		}
	}
}

/**
 Write the scene graph to a file for use with the 'dot' graphing application
 which is part of GraphViz (http://www.graphviz.org/).

 For example, to write the whole scenegraph to a file:

\code
	WriteDotFile(vtGetScene()->GetRoot(), "scene.dot");
\endcode

Then to use dot to produce a visual graph, from a command line:

\code
> dot -Tpng scene.dot -oscene.png
\endcode
 */
void WriteDotFile(osg::Group *node, const char *filename)
{
	FILE *fp = fopen(filename, "wb");
	if (!fp)
		return;
	fprintf(fp, "digraph G\n{\n");

	fprintf(fp, "node [fontname=\"Arial.ttf\",fontsize=\"9\",shape=ellipse,height=0.2,width=0.4,color=\"black\",style=\"filled\"];\n");
	fprintf(fp, "edge [color=\"midnightblue\",style=\"solid\",penwidth=\"1.5\"];\n");

	std::vector<void*> visited;

	LogToDot(NULL, node, 0, fp, visited);
	fprintf(fp, "}\n");
	fclose(fp);
}

