//
// NodeOSG.cpp
//
// Extend the behavior of OSG scene graph nodes.
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/vtLog.h"
#include "vtdata/vtString.h"

#if VTLISPSM
#include "LightSpacePerspectiveShadowTechnique.h"
#endif
#include "SimpleInterimShadowTechnique.h"

#include <osg/Polytope>
#include <osg/Projection>
#include <osg/Depth>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osgUtil/Optimizer>
#include <osg/Version>
#include <osg/TexGen>
#include <osgParticle/ModularEmitter>
#include <osgParticle/ParticleSystemUpdater>
#include <osgShadow/ShadowMap>
#include <osgShadow/ShadowTexture>

// We use bits 1 and 2 of the node mask as shadow flags.
const int ReceivesShadowTraversalMask = 0x1;
const int CastsShadowTraversalMask = 0x2;


//
// This visitor looks in a node tree for any instance of ModularEmitter.
//
#include <osgParticle/ModularEmitter>
class findParticlesVisitor : public osg::NodeVisitor
{
public:
	findParticlesVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN) {
		bFound = false;
	}
	virtual void apply(osg::Node &searchNode) {
		if (osgParticle::ModularEmitter* me = dynamic_cast<osgParticle::ModularEmitter*> (&searchNode) )
			bFound = true;
		else
			traverse(searchNode);
	}
	bool bFound;
};


/**
 * Transform a 3D point from a node's local frame of reference to world
 * coordinates.  This is done by walking the scene graph upwards, applying
 * all transforms that are encountered.
 *
 * \param node The node to consider.
 * \param point A reference to the input point is modified in-place with
 *	world coordinate result.
 */
void LocalToWorld(osg::Node *node, FPoint3 &point)
{
	// Suport any OSG nodes
	osg::Vec3 pos = v2s(point);
	while (node = node->getParent(0))
	{
		osg::MatrixTransform *mt = dynamic_cast<osg::MatrixTransform *>(node);
		if (mt != NULL)
		{
			const osg::Matrix &mat = mt->getMatrix();
			pos = mat.preMult(pos);
		}
		if (node->getNumParents() == 0)
			break;
	}
	s2v(pos, point);
}

bool ContainsParticleSystem(osg::Node *node)
{
	osg::ref_ptr<findParticlesVisitor> fp = new findParticlesVisitor;
	findParticlesVisitor *fpp = fp.get();
	node->accept(*fpp);
	return fp->bFound;
}

/** Get the Bounding Sphere of the node */
void GetBoundSphere(osg::Node *node, FSphere &sphere, bool bGlobal)
{
	// Try to just get the bounds normally
	osg::BoundingSphere bs = node->getBound();

	// Hack to support particle systems, which do not return a reliably
	//  correct bounding sphere, because of the extra transform inside which
	//  provides the particle effects with an absolute position
	if (ContainsParticleSystem(node))
	{
		osg::Group *grp = dynamic_cast<osg::Group*>(node);
		for (int i = 0; i < 3; i++)
		{
			grp = dynamic_cast<osg::Group*>(grp->getChild(0));
			if (grp)
				bs = grp->getBound();
			else
				break;
		}
	}

	s2v(bs, sphere);
	if (bGlobal)
	{
		// Note that this isn't 100% complete; we should be
		//  transforming the radius as well, with scale.
		LocalToWorld(node, sphere.center);
	}
}


//////////////////////////////////////////////////////////////////////////////
/**
 This class visits all nodes and computes bounding box extents.
*/
class ExtentsVisitor : public osg::NodeVisitor
{
public:
	// constructor
	ExtentsVisitor():NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

	virtual void apply(osg::Geode &node)
	{
		// update bounding box
		osg::BoundingBox bb;
		for (uint i = 0; i < node.getNumDrawables(); ++i)
		{
			// expand overall bounding box
			bb.expandBy(node.getDrawable(i)->getBound());
		}

		// transform corners by current matrix
		osg::BoundingBox xbb;
		for (uint i = 0; i < 8; ++i)
		{
			osg::Vec3 xv = bb.corner(i) * m_TransformMatrix;
			xbb.expandBy(xv);
		}

		// update overall bounding box size
		m_BoundingBox.expandBy(xbb);

		// continue traversing the graph
		traverse(node);
	}
	// handle geode drawable extents to expand the box
	virtual void apply(osg::MatrixTransform &node)
	{
		m_TransformMatrix *= node.getMatrix();
		// continue traversing the graph
		traverse(node);
	}
	// handle transform to expand bounding box
	// return bounding box
	osg::BoundingBox &GetBound() { return m_BoundingBox; }

protected:
	osg::BoundingBox m_BoundingBox;	// bound box
	osg::Matrix m_TransformMatrix;	// current transform matrix
};

/**
 Calculates the bounding box of the geometry contained in and under this node
 in the scene graph.  Note that unlike the bounding sphere which is cached,
 this value is calculated each time this method is called.

 \param node The node to visit.
 \param box Will receive the bounding box.
 */
void GetNodeBoundBox(osg::Node *node, FBox3 &box)
{
	ExtentsVisitor ev;
	node->accept(ev);
	osg::BoundingBox extents = ev.GetBound();
	s2v(extents, box);
}


/////////////////////////////////////////////////////////////////////////////
/**
	This class visits all matrixtransform nodes and set their datavariance to "STATIC"
*/
class TransformStaticVisitor : public osg::NodeVisitor
{
public:
	// constructor
	TransformStaticVisitor() : NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN)
	{
		count = 0;
	}
	// handle matrixtransform objects
	virtual void apply(osg::MatrixTransform &node)
	{
		if (node.getDataVariance() == osg::Object::UNSPECIFIED ||
			node.getDataVariance() == osg::Object::DYNAMIC)
		{
			node.setDataVariance(osg::Object::STATIC);
			count++;
		}
		// continue traversing the graph
		traverse(node);
	}
	int count;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

class PolygonCountVisitor : public osg::NodeVisitor
{
public:
	int numVertices, numFaces, numObjects;
	PolygonCountVisitor() :
		osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
	{
		reset();
		memset(&info, 0, sizeof(info));
	}
	virtual void reset() { numVertices=numFaces=numObjects=0; }
	virtual void apply(osg::Geode& geode);
	vtPrimInfo info;
};

void PolygonCountVisitor::apply(osg::Geode& geode)
{
	numObjects++;
	for (uint i=0; i<geode.getNumDrawables(); ++i)
	{
		osg::Geometry* geometry = geode.getDrawable(i)->asGeometry();
		if (!geometry) continue;
		for (uint j=0; j<geometry->getPrimitiveSetList().size(); ++j)
		{
			osg::PrimitiveSet *pset = geometry->getPrimitiveSet(j);
			osg::DrawArrayLengths *dal = dynamic_cast<osg::DrawArrayLengths*>(pset);
			//osg::DrawArray *da = dynamic_cast<osg::DrawArray*>(pset);

			int numIndices = pset->getNumIndices();
			int numPrimitives = pset->getNumPrimitives();

			info.Vertices += numIndices;
			info.Primitives += numPrimitives;

			// This code isn't completely finished as it doesn't iterate down
			//  into each variable-length indexed primitives to count each
			//  component line or triangle, but it does get most useful info.
			GLenum mode = pset->getMode();
			switch (mode)
			{
			case GL_POINTS:			info.Points += numPrimitives;	 break;
			case GL_LINES:			info.LineSegments += numPrimitives; break;
			case GL_TRIANGLES:		info.Triangles += numPrimitives; break;
			case GL_QUADS:			info.Quads += numPrimitives;
									info.Triangles += numPrimitives*2; break;
			case GL_POLYGON:		info.Polygons += numPrimitives;
									//info.Triangles += ...;
									break;
			case GL_LINE_STRIP:		info.LineStrips += numPrimitives;
									//info.LineSegments += ...;
									break;
			case GL_TRIANGLE_STRIP:	info.TriStrips += numPrimitives;
									//info.Triangles += ...;
									break;
			case GL_TRIANGLE_FAN:	info.TriFans += numPrimitives;
									//info.Triangles += ...;
									break;
			case GL_QUAD_STRIP:		info.QuadStrips += numPrimitives;
									//info.Quads += ...;
									//info.Triangles += ...;
									break;
			}
			// This kind of thing is incomplete because pset can be something
			//  called 'osg::DrawElementsUShort'
			if (mode == GL_TRIANGLE_STRIP && dal != NULL)
			{
				for (osg::DrawArrayLengths::const_iterator itr=dal->begin(); itr!=dal->end(); ++itr)
				{
					int iIndicesInThisStrip = static_cast<int>(*itr);
					info.Triangles += (iIndicesInThisStrip-1);
				}
			}
		}
		info.MemVertices += geometry->getVertexArray()->getNumElements();
	}
	traverse(geode);
}

/**
 * This method walks through a node (and all its children), collecting
 * information about all the geometric primitives.  The result is placed
 * in an object of type vtPrimInfo.  This includes information such as
 * number of vertices, number of triangles, and so forth.  Note that this
 * can be a time-consuming operation if your geometry is large or complex.
 * The results are not cached, so this method should be called only when
 * needed.
 *
 * \param node The node to investigate.
 * \param info A vtPrimInfo object which will receive all the information
 *	about this node and its children.
 */
void GetNodePrimCounts(osg::Node *node, vtPrimInfo &info)
{
	PolygonCountVisitor pv;
	node->accept(pv);
	info = pv.info;
}

/**
 * Given a node with geometry under it, rotate the vertices of that geometry
 *  by a given axis/angle.
 */
void ApplyVertexRotation(osg::Node *node, const FPoint3 &axis, float angle)
{
	FMatrix4 mat;
	mat.AxisAngle(axis, angle);
	ApplyVertexTransform(node, mat);
}

/**
 * Given any node with geometry under it, transform the vertices of that geometry.
 */
void ApplyVertexTransform(osg::Node *node, const FMatrix4 &mat)
{
	if (!node)
		return;

	// Remember the parent (TODO: beware possible case of multiple parents)
	//  We must remove the node from the parent temporarily in order for the
	//  optimization below to work, but we don't want to de-ref the node, so
	//  order is important.
	GroupPtr parent = node->getParent(0);

	// Put it under a temporary parent instead
	vtGroupPtr temp = new vtGroup;

	osg::Matrix omat;
	ConvertMatrix4(&mat, &omat);

	osg::MatrixTransform *transform = new osg::MatrixTransform;
	transform->setMatrix(omat);
	// Tell OSG that it can be optimized
	transform->setDataVariance(osg::Object::STATIC);

    temp->addChild(transform);
	transform->addChild(node);

	// carefully remove from the true parent
	parent->removeChild(node);

	// Now do some OSG voodoo, which should spread the transform downward
	//  through the loaded model, and delete the transform.
	osgUtil::Optimizer optimizer;
	optimizer.optimize(temp.get(), osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS);

	// now carefully add back again, whatever remains after the optimization,
	//  to the true parent. Hopefully, our corrective transform has been applied.
	// Our original node may have been optimized away too.
	parent->addChild(temp->getChild(0));
}


///////////////////////////////////////////////////////////////////////
// NodeExtension
//

NodeExtension::NodeExtension()
{
	m_bCastShadow = true; // osg nodes default to all mask bits set
						  // so set this true to match
}

void NodeExtension::SetOsgNode(osg::Node *n)
{
	// observer pointer to ourselves
	m_pNode = n;
}

/**
 * Set the enabled state of this node.  When the node is not enabled, it
 * is not rendered.  If it is a group node, all of the nodes children are
 * also not rendered.
 */
void NodeExtension::SetEnabled(bool bOn)
{
	osg::Node::NodeMask nm = m_pNode->getNodeMask();
	if (bOn)
	{
		if (m_bCastShadow)
			m_pNode->setNodeMask(nm | 3);
		else
			m_pNode->setNodeMask((nm & ~3) | 1);
	}
	else
		m_pNode->setNodeMask(nm & ~3);
}

/**
 * Return the enabled state of a node.
 */
bool NodeExtension::GetEnabled() const
{
	int mask = m_pNode->getNodeMask();
	return ((mask&0x3) != 0);
}

void NodeExtension::SetCastShadow(bool b)
{
	m_bCastShadow = b;
	if (GetEnabled())
		SetEnabled(true);
}

bool NodeExtension::GetCastShadow()
{
	return m_bCastShadow;
}

void NodeExtension::GetBoundSphere(FSphere &sphere, bool bGlobal)
{
	// Try to just get the bounds normally
	osg::BoundingSphere bs = m_pNode->getBound();

	// Hack to support particle systems, which do not return a reliably
	//  correct bounding sphere, because of the extra transform inside which
	//  provides the particle effects with an absolute position
	if (ContainsParticleSystem(m_pNode))
	{
		osg::Group *grp = dynamic_cast<osg::Group*>(m_pNode);
		for (int i = 0; i < 3; i++)
		{
			grp = dynamic_cast<osg::Group*>(grp->getChild(0));
			if (grp)
				bs = grp->getBound();
			else
				break;
		}
	}

	s2v(bs, sphere);
	if (bGlobal)
	{
		// Note that this isn't 100% complete; we should be
		//  transforming the radius as well, with scale.
		LocalToWorld(m_pNode, sphere.center);
	}
}


//////////////////////////////////////////////////////////////////////////

void TransformExtension::Identity()
{
	m_pTransform->setMatrix(osg::Matrix::identity());
}

FPoint3 TransformExtension::GetTrans() const
{
	osg::Vec3 v = m_pTransform->getMatrix().getTrans();
	return FPoint3(v[0], v[1], v[2]);
}

void TransformExtension::SetTrans(const FPoint3 &pos)
{
	osg::Matrix m = m_pTransform->getMatrix();
	m.setTrans(pos.x, pos.y, pos.z);
	m_pTransform->setMatrix(m);

	m_pTransform->dirtyBound();
}

void TransformExtension::Translate(const FPoint3 &pos)
{
	m_pTransform->postMult(osg::Matrix::translate(pos.x, pos.y, pos.z));
}

void TransformExtension::TranslateLocal(const FPoint3 &pos)
{
	m_pTransform->preMult(osg::Matrix::translate(pos.x, pos.y, pos.z));
}

void TransformExtension::Rotate(const FPoint3 &axis, double angle)
{
	m_pTransform->postMult(osg::Matrix::rotate(angle, axis.x, axis.y, axis.z));
}

void TransformExtension::RotateLocal(const FPoint3 &axis, double angle)
{
	m_pTransform->preMult(osg::Matrix::rotate(angle, axis.x, axis.y, axis.z));
}

void TransformExtension::RotateParent(const FPoint3 &axis, double angle)
{
	osg::Vec3 trans = m_pTransform->getMatrix().getTrans();
	m_pTransform->postMult(osg::Matrix::translate(-trans)*
			  osg::Matrix::rotate(angle, axis.x, axis.y, axis.z)*
			  osg::Matrix::translate(trans));
}

FQuat TransformExtension::GetOrient() const
{
	const osg::Matrix &xform = m_pTransform->getMatrix();
	osg::Quat q;
	xform.get(q);
	return FQuat(q.x(), q.y(), q.z(), q.w());
}

FPoint3 TransformExtension::GetDirection() const
{
	const osg::Matrix &xform = m_pTransform->getMatrix();
	const osg_matrix_value *ptr = xform.ptr();
	return FPoint3(-ptr[8], -ptr[9], -ptr[10]);
}

void TransformExtension::SetDirection(const FPoint3 &point, bool bPitch)
{
	// get current matrix
	FMatrix4 m4;
	GetTransform(m4);

	// remember where it is now
	FPoint3 trans = m4.GetTrans();

	// orient it in the desired direction
	FMatrix3 m3;
	m3.MakeOrientation(point, bPitch);
	m4.SetFromMatrix3(m3);

	// restore translation
	m4.SetTrans(trans);

	// set current matrix
	SetTransform(m4);
}

void TransformExtension::Scale(float factor)
{
	m_pTransform->preMult(osg::Matrix::scale(factor, factor, factor));
}

void TransformExtension::Scale(float x, float y, float z)
{
	m_pTransform->preMult(osg::Matrix::scale(x, y, z));
}

void TransformExtension::SetTransform(const FMatrix4 &mat)
{
	osg::Matrix mat_osg;

	ConvertMatrix4(&mat, &mat_osg);

	m_pTransform->setMatrix(mat_osg);
	m_pTransform->dirtyBound();
}

void TransformExtension::GetTransform(FMatrix4 &mat) const
{
	const osg::Matrix &xform = m_pTransform->getMatrix();
	ConvertMatrix4(&xform, &mat);
}

void TransformExtension::PointTowards(const FPoint3 &point, bool bPitch)
{
	SetDirection(point - GetTrans(), bPitch);
}


//////////////////////////////////////////////////////////////////////////

/**
 Recursively travel up the scene graph, looking for a specific parent node.
 Return true if it is found.
 */
bool FindAncestor(osg::Node *node, osg::Node *parent)
{
	for (uint i = 0; i < node->getNumParents(); i++)
	{
		if (node->getParent(i) == parent)
			return true;
		// recurse upwards
		if (FindAncestor(node->getParent(i), parent))
			return true;
	}
	return false;
}

/**
 Recursively travel down the scene graph, looking for a specific child node by
 name.  Return it if it is found.
 */
osg::Node *FindDescendent(osg::Group *group, const char *pName)
{
	for (uint i = 0; i < group->getNumChildren(); i++)
	{
		osg::Node *child = group->getChild(i);
		if (child->getName() == pName)
			return child;
		osg::Group *group = dynamic_cast<osg::Group*>(child);
		if (group)
		{
			osg::Node *found = FindDescendent(group, pName);
			if (found)
				return found;
		}
	}
	return NULL;
}

/**
 Insert a node into a scene graph.  For example, if the graph is A-B-C,
 where B is the child of A and C is the child of B, then inserting a node D
 above C results in A-B-D-C.

 \param node The node to insert above.
 \param newnode The node to insert.
 */
void InsertNodeAbove(osg::Node *node, osg::Group *newnode)
{
	osg::Group *parent = (osg::Group*) node->getParent(0);
	newnode->addChild(node);
	parent->addChild(newnode);
	parent->removeChild(node);
}

/**
 Insert a node into a scene graph.  For example, if the graph is A-B-C,
 where B is the child of A and C is the child of B, then inserting a node D
 below A results in A-D-B-C.

 \param group The node to insert below.
 \param newnode The node to insert.
 */
void InsertNodeBelow(osg::Group *group, osg::Group *newnode)
{
	for (uint i = 0; i < group->getNumChildren(); i++)
		newnode->addChild(group->getChild(i));

	group->removeChildren(0, group->getNumChildren());
	group->addChild(newnode);
}

FSphere GetGlobalBoundSphere(osg::Node *node)
{
	// Try to just get the bounds normally
	osg::BoundingSphere bs = node->getBound();

	// Hack to support particle systems, which do not return a reliably
	//  correct bounding sphere, because of the extra transform inside which
	//  provides the particle effects with an absolute position
	if (ContainsParticleSystem(node))
	{
		osg::Group *grp = dynamic_cast<osg::Group*>(node);
		for (int i = 0; i < 3; i++)
		{
			grp = dynamic_cast<osg::Group*>(grp->getChild(0));
			if (grp)
				bs = grp->getBound();
			else
				break;
		}
	}

	FSphere sphere;
	s2v(bs, sphere);

	// Note that this isn't 100% complete; we should be
	//  transforming the radius as well, with scale.
	LocalToWorld(node, sphere.center);
	return sphere;
}

void SetEnabled(osg::Node *node, bool bOn)
{
	NodeExtension *ne = dynamic_cast<NodeExtension*>(node);
	if (ne)
		ne->SetEnabled(bOn);
}

bool GetEnabled(osg::Node *node)
{
	int mask = node->getNodeMask();
	return ((mask&0x3) != 0);
}

bool NodeIsEnabled(osg::Node *node)
{
	return GetEnabled(node);
}

// Walk an OSG scenegraph looking for Texture states, and disable mipmap.
class MipmapVisitor : public osg::NodeVisitor
{
public:
	MipmapVisitor() : NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
	virtual void apply(osg::Geode& geode)
	{
		for (unsigned i=0; i<geode.getNumDrawables(); ++i)
		{
			osg::Geometry *geo = dynamic_cast<osg::Geometry *>(geode.getDrawable(i));
			if (!geo) continue;

			osg::StateSet *stateset = geo->getStateSet();
			if (!stateset) continue;

			osg::StateAttribute *state = stateset->getTextureAttribute(0, osg::StateAttribute::TEXTURE);
			if (!state) continue;

			osg::Texture2D *texture = dynamic_cast<osg::Texture2D *>(state);
			if (texture)
				texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
		}
		NodeVisitor::apply(geode);
	}
};

// Walk an OSG scenegraph looking for geodes with textures that have an alpha
//	map, and enable alpha blending for them.  I cannot imagine why this is not
//  the default OSG behavior, but since it isn't, we have this visitor.
class AlphaVisitor : public osg::NodeVisitor
{
public:
	AlphaVisitor() : NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
	virtual void apply(osg::Geode& geode)
	{
		for (unsigned i=0; i<geode.getNumDrawables(); ++i)
		{
			osg::Geometry *geo = dynamic_cast<osg::Geometry *>(geode.getDrawable(i));
			if (!geo) continue;

			osg::StateSet *stateset = geo->getStateSet();
			if (!stateset) continue;

			osg::StateAttribute *state = stateset->getTextureAttribute(0, osg::StateAttribute::TEXTURE);
			if (!state) continue;

			osg::Texture2D *texture = dynamic_cast<osg::Texture2D *>(state);
			if (!texture) continue;

			osg::Image *image = texture->getImage();
			if (!image) continue;

			if (image->isImageTranslucent())
			{
				stateset->setAttributeAndModes(new osg::BlendFunc, osg::StateAttribute::ON);
				stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
			}
		}
		NodeVisitor::apply(geode);
	}
};

bool g_bDisableMipmaps = false;		// global

/**
 * Load a 3D model from a file.
 *
 * The underlying scenegraph (i.e. OSG) is used to load the model, which is
 * returned as an osg::Node.  You can then use this node normally, for example
 * add it to your scenegraph with addChild(), or to your terrain's subgraph
 * with vtTerrain::AddNode().
 *
 * \param filename The filename to load from.
 * \param bAllowCache Default is true, to allow OSG to cache models.
 *	This means that if you load from the same filename more than once, you
 *  will get the same model again instantly.  If you don't want this, for
 *	example if the model has changed on disk and you want to force loading,
 *	pass false.
 * \param bDisableMipmaps Pass true to turn off mipmapping in the texture maps
 *	in the loaded model.  Default is false (enable mipmapping).
 *
 * \return A node pointer if successful, or NULL if the load failed.
 */
osg::Node *vtLoadModel(const char *filename, bool bAllowCache, bool bDisableMipmaps)
{
	// Some of OSG's file readers, such as the Wavefront OBJ reader, have
	//  sensitivity to stdio issues with '.' and ',' in European locales.
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	// Workaround for OSG's OBJ-MTL reader which doesn't like backslashes
	vtString fname = filename;
	fname.Replace('\\', '/');

#define HINT osgDB::ReaderWriter::Options::CacheHintOptions
	// In case of reloading a previously loaded model, we must empty
	//  our own cache as well as disable OSG's cache.
	osgDB::Registry *reg = osgDB::Registry::instance();
	osgDB::ReaderWriter::Options *opts;

	if (!bAllowCache)
	{
		opts = reg->getOptions();
		if (!opts)
		{
			opts = new osgDB::ReaderWriter::Options;
			opts->ref();	// workaround!  otherwise OSG might crash when
				// closing its DLL, as the options get deleted twice (?) or
				// perhaps it doesn't like deleting the object WE allocated.
		}
		// Disable OSG's cache
		opts->setObjectCacheHint((HINT) ((opts->getObjectCacheHint() & ~(osgDB::ReaderWriter::Options::CACHE_NODES))));
		reg->setOptions(opts);
	}

	// OSG doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(fname);

	// Now actually request the node from OSG
#if VTDEBUG
	VTLOG("[");
#endif
	NodePtr node = osgDB::readNodeFile((const char *)fname_local);
#if VTDEBUG
	VTLOG("]");
#endif
	// If OSG could not load it, there is nothing more to do
	if (!node.valid())
		return NULL;

	// We must insert a 'Normalize' state above the geometry objects
	// that we load, otherwise when they are scaled, the vertex normals
	// will cause strange lighting.  Fortunately, we only need to create
	// a single State object which is shared by all loaded models.
	osg::StateSet *stateset = node->getOrCreateStateSet();
	stateset->setMode(GL_NORMALIZE, osg::StateAttribute::ON);

	// For some reason, some file readers (at least .obj) will load models with
	//  alpha textures, but _not_ enable blending for them or put them in the
	//  transparent bin.  This visitor walks the tree and corrects that.
	AlphaVisitor visitor_a;
	node->accept(visitor_a);

	// If the user wants to, we can disable mipmaps at this point, using
	//  another visitor.
	if (bDisableMipmaps || g_bDisableMipmaps)
	{
		MipmapVisitor visitor;
		node->accept(visitor);
	}

#if 1
	// We must insert a rotation transform above the model, because OSG's
	//  file loaders (now mostly consistently) tweak the model to put Z
	//  up, and the VTP uses OpenGL coordinates which have Y up.
	float fRotation = -PID2f;
	osg::MatrixTransform *transform = new osg::MatrixTransform;
	transform->setName("corrective 90 degrees");
	transform->setMatrix(osg::Matrix::rotate(fRotation, osg::Vec3(1,0,0)));
	transform->addChild(node);
	// it's not going to change, so tell OSG that it can be optimized
	transform->setDataVariance(osg::Object::STATIC);

	// Disable flattening for now; some things like vehicles need the transforms intact.
	bool flatten = false;
	if (flatten)
	{
		// Now do some OSG voodoo, which should spread ("flatten") the
		//  transform downward through the loaded model, and delete the transform.
		// The voodoo is picky: there must be no other parents of this branch
		//  of the scenegraph, and all matrices must be STATIC
		TransformStaticVisitor tsv;
		node->accept(tsv);
		if (tsv.count != 0)
			VTLOG("  Set %d transforms to STATIC\n", tsv.count);

		vtGroupPtr group = new vtGroup;
		group->addChild(transform);

		//vtLogGraph(group);
		osgUtil::Optimizer optimizer;
		optimizer.optimize(group, osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS);
		//vtLogGraph(group);

		// If it worked, the transform will be replaced by a Group
		osg::Node *child = group->getChild(0);
		if (dynamic_cast<osg::MatrixTransform*>(child) != NULL)
		{
			VTLOG1("  Transform flatten FAILED.\n");

			// We have to return it with the transform still above it
			node = transform;
		}
		else
		{
			VTLOG1("  Transform flatten Succeeded.\n");

			// The resulting node is whatever remains under the optimization parent
			node = child;
		}
	}
	else
	{
		node = transform;
	}
	//VTLOG1("--------------\n");
	//vtLogGraph(node);
#endif

	// Use the filename as the node's name
	node->setName(fname);

	// We are holding a ref_ptr to the object (with refcount=1) so we can't just
	//  return node.get(), because node will go out of scope, decrement the count
	//  and delete the object.  Instead, use release(), which will decrease the
	//  refcount to 0, but NOT delete the object.
	return node.release();
}

bool vtSaveModel(osg::Node *node, const char *filename)
{
	osgDB::ReaderWriter::WriteResult result;
#if (OPENSCENEGRAPH_MAJOR_VERSION==2 && OPENSCENEGRAPH_MINOR_VERSION>=2) || OPENSCENEGRAPH_MAJOR_VERSION>2
	result = osgDB::Registry::instance()->writeNode(*node, std::string((const char *)filename), NULL);
#else
	result = osgDB::Registry::instance()->writeNode(*node, (const char *)filename);
#endif
	return result.success();
}


///////////////////////////////////////////////////////////////////////
// vtGroup
//

vtGroup::vtGroup()
{
	SetOsgNode(this);
}


///////////////////////////////////////////////////////////////////////
// vtTransform
//

vtTransform::vtTransform()
{
	SetOsgTransform(this);
}

///////////////////////////////////////////////////////////////////////
// vtFog
//

RGBf vtFog::s_white(1, 1, 1);

vtFog::vtFog()
{
	SetOsgNode(this);
}

/**
 * Set the Fog state.
 *
 * You can turn fog on or off.  When you turn fog on, it affects all others
 * below it in the scene graph.
 *
 * \param bOn True to turn fog on, false to turn it off.
 * \param start The distance from the camera at which fog starts, in meters.
 * \param end The distance from the camera at which fog end, in meters.  This
 *		is the point at which it becomes totally opaque.
 * \param color The color of the fog.  All geometry will be faded toward this
 *		color.
 * \param eType Can be FM_LINEAR, FM_EXP, or FM_EXP2 for linear or exponential
 *		increase of the fog density.
 */
void vtFog::SetFog(bool bOn, float start, float end, const RGBf &color, osg::Fog::Mode eType)
{
	osg::StateSet *set = getOrCreateStateSet();
	if (bOn)
	{
		m_pFog = new osg::Fog;
		m_pFog->setMode(eType);
		m_pFog->setDensity(0.25f);	// not used for linear
		m_pFog->setStart(start);
		m_pFog->setEnd(end);
		m_pFog->setColor(osg::Vec4(color.r, color.g, color.b, 1));

		set->setAttributeAndModes(m_pFog.get(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
	}
	else
	{
		// turn fog off
		set->setMode(GL_FOG, osg::StateAttribute::OFF);
	}
}


//////////////////////////////////////////////////////////////////////
// vtShadow
//

vtShadow::vtShadow(const int ShadowTextureUnit, int LightNumber) :
	m_ShadowTextureUnit(ShadowTextureUnit), m_LightNumber(LightNumber)
{
	setReceivesShadowTraversalMask(ReceivesShadowTraversalMask);
	setCastsShadowTraversalMask(CastsShadowTraversalMask);

#if VTLISPSM
	osg::ref_ptr<CLightSpacePerspectiveShadowTechnique> pShadowTechnique = new CLightSpacePerspectiveShadowTechnique;
	// No need to set the BaseTextureUnit as the default of zero is OK for us
	// But the ShadowTextureUnit might be different (default 1)
	pShadowTechnique->setShadowTextureUnit(m_ShadowTextureUnit);
	pShadowTechnique->setLightNumber(m_LightNumber);
#else
	osg::ref_ptr<CSimpleInterimShadowTechnique> pShadowTechnique = new CSimpleInterimShadowTechnique;
#endif

#if !VTLISPSM
#if VTDEBUGSHADOWS
	// add some instrumentation
	pShadowTechnique->m_pParent = this;
#endif

	pShadowTechnique->SetLightNumber(LightNumber);
	pShadowTechnique->SetShadowTextureUnit(m_ShadowTextureUnit);
	pShadowTechnique->SetShadowSphereRadius(50.0);
#endif
	setShadowTechnique(pShadowTechnique.get());

	SetOsgNode(this);
}

void vtShadow::SetDarkness(float bias)
{
	CSimpleInterimShadowTechnique *pTechnique = dynamic_cast<CSimpleInterimShadowTechnique *>(getShadowTechnique());
	if (pTechnique)
		pTechnique->SetShadowDarkness(bias);
}

float vtShadow::GetDarkness()
{
	CSimpleInterimShadowTechnique *pTechnique = dynamic_cast<CSimpleInterimShadowTechnique *>(getShadowTechnique());
	if (pTechnique)
		return pTechnique->GetShadowDarkness();
	else
		return 1.0f;
}

void vtShadow::AddAdditionalTerrainTextureUnit(const uint Unit, const uint Mode)
{
#if VTLISPSM
	CLightSpacePerspectiveShadowTechnique *pTechnique = dynamic_cast<CLightSpacePerspectiveShadowTechnique *>(getShadowTechnique());
	if (pTechnique)
		pTechnique->AddAdditionalTerrainTextureUnit(Unit, Mode);
#else
	CSimpleInterimShadowTechnique *pTechnique = dynamic_cast<CSimpleInterimShadowTechnique *>(getShadowTechnique());
	if (pTechnique)
		pTechnique->AddMainSceneTextureUnit(Unit, Mode);
#endif
}

void vtShadow::RemoveAdditionalTerrainTextureUnit(const uint Unit)
{
#if VTLISPSM
	CLightSpacePerspectiveShadowTechnique *pTechnique = dynamic_cast<CLightSpacePerspectiveShadowTechnique *>(getShadowTechnique());
	if (pTechnique)
		pTechnique->RemoveAdditionalTerrainTextureUnit(Unit);
#else
	CSimpleInterimShadowTechnique *pTechnique = dynamic_cast<CSimpleInterimShadowTechnique *>(getShadowTechnique());
	if (pTechnique)
		pTechnique->RemoveMainSceneTextureUnit(Unit);
#endif
}

void vtShadow::RemoveAllAdditionalTerrainTextureUnits()
{
#if VTLISPSM
	CLightSpacePerspectiveShadowTechnique *pTechnique = dynamic_cast<CLightSpacePerspectiveShadowTechnique *>(getShadowTechnique());
	if (pTechnique)
		pTechnique->RemoveAllAdditionalTerrainTextureUnits();
#endif
}

void vtShadow::SetShadowTextureResolution(const uint ShadowTextureResolution)
{
#if VTLISPSM
	CLightSpacePerspectiveShadowTechnique *pTechnique = dynamic_cast<CLightSpacePerspectiveShadowTechnique *>(getShadowTechnique());
	if (pTechnique)
		pTechnique->setTextureSize(osg::Vec2s(ShadowTextureResolution,ShadowTextureResolution));
#else
	CSimpleInterimShadowTechnique *pTechnique = dynamic_cast<CSimpleInterimShadowTechnique *>(getShadowTechnique());
	if (pTechnique)
		pTechnique->SetShadowTextureResolution(ShadowTextureResolution);
#endif
}

void vtShadow::SetRecalculateEveryFrame(const bool RecalculateEveryFrame)
{
	CSimpleInterimShadowTechnique *pTechnique = dynamic_cast<CSimpleInterimShadowTechnique *>(getShadowTechnique());
	if (pTechnique)
		pTechnique->SetRecalculateEveryFrame(RecalculateEveryFrame);
}

bool vtShadow::GetRecalculateEveryFrame() const
{
	const CSimpleInterimShadowTechnique *pTechnique = dynamic_cast<const CSimpleInterimShadowTechnique *>(getShadowTechnique());
	if (pTechnique)
		return pTechnique->GetRecalculateEveryFrame();
	else
		return false;
}

void vtShadow::SetShadowSphereRadius(const float ShadowSphereRadius)
{
#if VTLISPSM
	CLightSpacePerspectiveShadowTechnique *pTechnique = dynamic_cast<CLightSpacePerspectiveShadowTechnique *>(getShadowTechnique());
	if (pTechnique)
		pTechnique->setMaxFarPlane(ShadowSphereRadius);
#else
	CSimpleInterimShadowTechnique *pTechnique = dynamic_cast<CSimpleInterimShadowTechnique *>(getShadowTechnique());
	if (pTechnique)
		pTechnique->SetShadowSphereRadius(ShadowSphereRadius);
#endif
}

void vtShadow::SetHeightField3d(vtHeightField3d *pHeightField3d)
{
	CSimpleInterimShadowTechnique *pTechnique = dynamic_cast<CSimpleInterimShadowTechnique *>(getShadowTechnique());
	if (pTechnique)
		pTechnique->SetHeightField3d(pHeightField3d);
}

void vtShadow::AddLodGridToIgnore(vtLodGrid* pLodGrid)
{
	CSimpleInterimShadowTechnique *pTechnique = dynamic_cast<CSimpleInterimShadowTechnique *>(getShadowTechnique());
	if (pTechnique)
		pTechnique->AddLodGridToIgnore(pLodGrid);
}

void vtShadow::ForceShadowUpdate()
{
	CSimpleInterimShadowTechnique *pTechnique = dynamic_cast<CSimpleInterimShadowTechnique *>(getShadowTechnique());
	if (pTechnique)
		pTechnique->ForceShadowUpdate();
}

void vtShadow::SetDebugHUD(vtGroup *pGroup)
{
#if defined (VTDEBUG) && defined (VTDEBUGSHADOWS)
	CSimpleInterimShadowTechnique *pTechnique = dynamic_cast<CSimpleInterimShadowTechnique *>(getShadowTechnique());
	if (pTechnique)
	{
		osg::ref_ptr<osg::Camera> pCamera = pTechnique->makeDebugHUD();
		pCamera->setName("Shadow DEBUG HUD camera");
		pGroup->addChild(pCamera.get());
	}
#endif
}


///////////////////////////////////////////////////////////////////////
// vtLightSource
//

/**
	Makes a lightsource, which is placed in the scenegraph in order to
	position the light.

	\param LightNumber Each light should use a different light number which
	should be one of the valid light numbers for the current OpenGL implementation.
	Only light numbers up to 7 are guaranteed by default.  The convention,
	higher up in vtlib, is to use light number 0 for the sun.
*/
vtLightSource::vtLightSource(int LightNumber)
{
	// Lights can now go into the scene graph in OSG, with LightSource.
	// A lightsource creates a light, which we can get with getLight().
	getLight()->setLightNum(LightNumber);
	SetOsgNode(this);

	// What does this do, exactly?
	setLocalStateSetModes(osg::StateAttribute::ON);

	// Because lighting is also a 'state', we need to inform
	// the whole scene graph that this light is enabled.
	setStateSetModes(*(vtGetScene()->GetRootState()), osg::StateAttribute::ON);
}

void vtLightSource::SetDiffuse(const RGBf &color)
{
	getLight()->setDiffuse(v2s(color));
}

RGBf vtLightSource::GetDiffuse() const
{
	return s2v(getLight()->getDiffuse());
}

void vtLightSource::SetAmbient(const RGBf &color)
{
	getLight()->setAmbient(v2s(color));
}

RGBf vtLightSource::GetAmbient() const
{
	return s2v(getLight()->getAmbient());
}

void vtLightSource::SetSpecular(const RGBf &color)
{
	getLight()->setSpecular(v2s(color));
}

RGBf vtLightSource::GetSpecular() const
{
	return s2v(getLight()->getSpecular());
}


///////////////////////////////////////////////////////////////////////
// vtCamera
//

vtCamera::vtCamera() : vtTransform()
{
	m_fHither = 1;
	m_fYon = 100;
	m_fFOV = PIf/3.0f;
	m_bOrtho = false;
	m_fWidth = 1;
}

/**
 * Set the hither (near) clipping plane distance.
 */
void vtCamera::SetHither(float f)
{
	m_fHither = f;
}

/**
 * Get the hither (near) clipping plane distance.
 */
float vtCamera::GetHither() const
{
	return m_fHither;
}

/**
 * Set the yon (far) clipping plane distance.
 */
void vtCamera::SetYon(float f)
{
	m_fYon = f;
}

/**
 * Get the yon (far) clipping plane distance.
 */
float vtCamera::GetYon() const
{
	return m_fYon;
}

/**
 * Set the camera's horizontal field of view (FOV) in radians.
 */
void vtCamera::SetFOV(float fov_x)
{
	m_fFOV = fov_x;
}

/**
 * Return the camera's horizontal field of view (FOV) in radians.
 */
float vtCamera::GetFOV() const
{
	return m_fFOV;
}

/**
 * Return the camera's vertical field of view (FOV) in radians.
 */
float vtCamera::GetVertFOV() const
{
	IPoint2 size = vtGetScene()->GetWindowSize();
	float aspect = (float) size.x / size.y;

	double a = tan(m_fFOV/2);
	double b = a / aspect;
	return atan(b) * 2;
}

/**
 * Zoom (move) the camera to a sphere, generally the bounding sphere of
 *  something you want to look at.  The camera will be pointing directly
 *  down the -Z axis at the center of the sphere.
 *
 * \param sphere The sphere to look at.
 * \param fPitch An optional pitch angle to look, in radians.  For example,
 *		a pitch of -PIf/10 would be looking down at a mild angle.
 */
void vtCamera::ZoomToSphere(const FSphere &sphere, float fPitch)
{
	Identity();
	Translate(sphere.center);
	RotateLocal(FPoint3(1,0,0), fPitch);
	TranslateLocal(FPoint3(0.0f, 0.0f, sphere.radius));
}

/**
 * Set this camera to use an orthographic view.  An orthographic view has
 *  no FOV angle, so Set/GetFOV have no affect.  Instead, use Get/SetWidth
 *  to control the width of the orthogonal view.
 */
void vtCamera::SetOrtho(bool bOrtho)
{
	m_bOrtho = bOrtho;
}

/**
 * Return true if the camera is orthographic.
 */
bool vtCamera::IsOrtho() const
{
	return m_bOrtho;
}

/**
 * Set the view width of an orthographic camera.
 */
void vtCamera::SetWidth(float fWidth)
{
	m_fWidth = fWidth;
}

/**
 * Get the view width of an orthographic camera.
 */
float vtCamera::GetWidth() const
{
	return m_fWidth;
}


///////////////////////////////////////////////////////////////////////
// vtGeode
//

vtGeode::vtGeode()
{
	SetOsgNode(this);
}

vtGeode *vtGeode::CloneGeode()
{
	vtGeode *newgeom = new vtGeode;
	newgeom->CloneFromGeode(this);
	return newgeom;
}

void vtGeode::CloneFromGeode(const vtGeode *rhs)
{
	// Shallow copy: just reference the meshes and materials of the
	//  geometry that we are copying from.
	SetMaterials(rhs->GetMaterials());
	int idx;
	for (uint i = 0; i < rhs->NumMeshes(); i++)
	{
		vtMesh *mesh = rhs->GetMesh(i);
		if (mesh)
		{
			idx = mesh->GetMatIndex();
			AddMesh(mesh, idx);
		}
		else
		{
			vtTextMesh *tm = rhs->GetTextMesh(i);
			if (tm)
			{
				idx = tm->GetMatIndex();
				AddTextMesh(tm, idx);
			}
		}
	}
}

void vtGeode::AddMesh(vtMesh *pMesh, int iMatIdx)
{
	addDrawable(pMesh);
	SetMeshMatIndex(pMesh, iMatIdx);
}

void vtGeode::AddTextMesh(vtTextMesh *pTextMesh, int iMatIdx, bool bOutline)
{
	// connect the underlying OSG objects
	addDrawable(pTextMesh);

	// Normally, we would assign the material state to the drawable.
	// However, OSG treats Text specially, it cannot be affected by normal
	//  material statesets.  For example, it always sets its own color,
	//  ignore any State settings.  So, disabled the following.
#if 0
	pTextMesh->SetMatIndex(iMatIdx);
	vtMaterial *pMat = GetMaterial(iMatIdx);
	if (pMat)
	{
		StateSet *pState = pMat->m_pStateSet.get();
		pTextMesh->m_pOsgText->setStateSet(pState);
	}
#endif

	// In fact, we need to avoid lighting the text, yet text messes with
	//  own StateSet, so we can't set it there.  We set it here.
	osg::StateSet *sset = getOrCreateStateSet();
	sset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	// also not useful to see the back of text (mirror writing)
	sset->setMode(GL_CULL_FACE, osg::StateAttribute::ON);

	// A black outline around the font makes it easier to read against
	// most backgrounds.
	// TODO: expose a method to disable this behavior for special cases.
	if (bOutline)
		pTextMesh->setBackdropType(osgText::Text::OUTLINE);

	// In most cases, it is very helpful for text to face the user.
	// TODO: expose a method to disable this behavior for special cases.
	pTextMesh->setAutoRotateToScreen(true);
}

void vtGeode::SetMeshMatIndex(vtMesh *pMesh, int iMatIdx)
{
	vtMaterial *pMat = GetMaterial(iMatIdx);
	if (pMat)
	{
#if 0
		// Beware: the mesh may already have its own stateset?
		//  In what case would this arise?  The user might be calling this
		//   method on a mesh which already has a material.  In that case,
		//	 we want to cleanly switch to the new material, not merge into
		//	 the old one.
		StateSet *pStateMesh = pMesh->getStateSet();
		if (pStateMesh)
			pStateMesh->merge(*pState);
		else
#endif
		pMesh->setStateSet(pMat);

		// Try to provide color for un-lit meshes
		if (!pMat->GetLighting())
		{
			// unless it's using vertex colors...
			osg::Geometry::AttributeBinding bd = pMesh->getColorBinding();
			if (bd != osg::Geometry::BIND_PER_VERTEX)
			{
				// not lit, not vertex colors
				// here is a sneaky way of forcing OSG to use the diffuse
				// color for the unlit color

				// This will leave the original color array alllocated in the vtMesh
				osg::Vec4Array *pColors = new osg::Vec4Array;
				pColors->push_back(pMat->m_pMaterial->getDiffuse(FAB));
				pMesh->setColorArray(pColors);
				pMesh->setColorBinding(osg::Geometry::BIND_OVERALL);
			}
		}
	}
	pMesh->SetMatIndex(iMatIdx);
}

void vtGeode::RemoveMesh(vtMesh *pMesh)
{
	// If this geom has this mesh, remove it
	removeDrawable(pMesh);
}

void vtGeode::RemoveAllMeshes()
{
	// This is a vector of ref_ptrs, so it will free meshes as appropriate.
	removeDrawables(0, getNumDrawables());
}

uint vtGeode::NumMeshes() const
{
	return getNumDrawables();
}

vtMesh *vtGeode::GetMesh(int i) const
{
	// It is valid to return a non-const pointer to the mesh, since the mesh
	//  can be modified entirely independently of the geometry.
	osg::Drawable *draw = const_cast<osg::Drawable *>( getDrawable(i) );
	vtMesh *mesh = dynamic_cast<vtMesh*>(draw);
	return mesh;
}

vtTextMesh *vtGeode::GetTextMesh(int i) const
{
	// It is valid to return a non-const pointer to the mesh, since the mesh
	//  can be modified entirely independently of the geometry.
	osg::Drawable *draw = const_cast<osg::Drawable *>( getDrawable(i) );
	vtTextMesh *mesh = dynamic_cast<vtTextMesh*>(draw);
	return mesh;
}

void vtGeode::SetMaterials(const class vtMaterialArray *mats)
{
	m_pMaterialArray = mats;	// increases reference count
}

const vtMaterialArray *vtGeode::GetMaterials() const
{
	return m_pMaterialArray.get();
}

vtMaterial *vtGeode::GetMaterial(int idx)
{
	if (!m_pMaterialArray.valid())
		return NULL;
	if (idx < 0 || idx >= (int) m_pMaterialArray->size())
		return NULL;
	return m_pMaterialArray->at(idx).get();
}


///////////////////////////////////////////////////////////////////////
// vtLOD
//

vtLOD::vtLOD()
{
	setCenter(osg::Vec3(0, 0, 0));
	SetOsgNode(this);
}


///////////////////////////////////////////////////////////////////////
// OsgDynMesh
//

OsgDynMesh::OsgDynMesh()
{
	// The following line code is a workaround provided by Robert Osfield himself
	// create an empty stateset, to force the traversers
	// to nest any state above it in the inheritance path.
	setStateSet(new osg::StateSet);
	// Stop osgViewer::Frame from returning before this node
	// was been fully rendered
	setDataVariance(osg::Object::DYNAMIC);
}

osg::BoundingBox OsgDynMesh::computeBoundingBox() const
{
	FBox3 box;
	m_pDynGeom->DoCalcBoundBox(box);

	// convert it to OSG bounds
	v2s(box.min, _boundingBox._min);
	v2s(box.max, _boundingBox._max);

	_boundingBoxComputed=true;
	return _boundingBox;
}


void OsgDynMesh::drawImplementation(osg::RenderInfo& renderInfo) const
{
	OsgDynMesh *cthis = const_cast<OsgDynMesh*>(this);
	cthis->m_pDrawState = renderInfo.getState();

	// Our dyamic mesh might use Vertex Arrays, and this can conflict with
	//  other objects in the OSG scene graph which are also using Vertex
	//  Arrays.  To avoid this, we disable the OSG arrays before the
	//  dyamic geometry draws itself.
	// NOTE: I would guess we should be pushing/popping the state here,
	//  but i don't understand how to use osg::State that way, and just
	//  disabling the arrays seems to make things work!
	cthis->m_pDrawState->disableAllVertexArrays();

    // For the time being convert the osg camera into a vtCamera.
    // In the longer term it would probably be better to convert all
    // the implementations vtDynGeon::DocCull to using an osg camera directlty
	osg::ref_ptr<osg::Camera> pOsgCamera = renderInfo.getCurrentCamera();
	osg::ref_ptr<vtCamera> pVtCamera = new vtCamera;

	// Set up the vtCamera transform
	const osg::Matrix &mat2 = pOsgCamera->getViewMatrix();
	osg::Matrix imat;
	imat.invert(mat2);
	pVtCamera->setMatrix(imat);

	// Perspective only
	double fovy, aspectRatio;
	// Both
	double zNear, zFar;
	// Ortho only
	double left, right, bottom, top;

    if (pOsgCamera->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar))
    {
        pVtCamera->SetOrtho(false);
 		double a = tan(osg::DegreesToRadians(fovy/2));
		double b = a * aspectRatio;
		double fovx = atan(b) * 2;
		pVtCamera->SetFOV(fovx);
    }
    else if(pOsgCamera->getProjectionMatrixAsOrtho(left, right, bottom, top, zNear, zFar))
    {
        pVtCamera->SetOrtho(true);
        pVtCamera->SetWidth(right - left);
    }
    else
    {
        VTLOG("OsgDynMesh::drawImplementation - Cannot set up vtCamera\n");
        return;
    }

    pVtCamera->SetHither(zNear);
    pVtCamera->SetYon(zFar);


	// setup the culling planes
	// Get the view frustum clipping planes directly from OSG.
	// We can't get the planes from the state, because the state
	//  includes the funny modelview matrix used to scale the
	//  heightfield.  We must get it from the camera instead.

	const osg::Matrixd &_projection = renderInfo.getCurrentCamera()->getProjectionMatrix();
	const osg::Matrixd &_modelView = renderInfo.getCurrentCamera()->getViewMatrix();

	osg::Polytope tope;
	tope.setToUnitFrustum();
	tope.transformProvidingInverse((_modelView)*(_projection));

	const osg::Polytope::PlaneList &planes = tope.getPlaneList();

	int i = 0;
	for (osg::Polytope::PlaneList::const_iterator itr=planes.begin();
		itr!=planes.end(); ++itr)
	{
		// make a copy of the clipping plane
		osg::Plane plane = *itr;

		// extract the OSG plane to our own structure
		osg::Vec4 pvec = plane.asVec4();
		m_pDynGeom->m_cullPlanes[i++].Set(-pvec.x(), -pvec.y(), -pvec.z(), -pvec.w());
	}

	m_pDynGeom->DoCull(pVtCamera.get());
	m_pDynGeom->DoRender();
}


///////////////////////////////////////////////////////////////////////
// vtDynGeom
//

vtDynGeom::vtDynGeom() : vtGeode()
{
	m_pDynMesh = new OsgDynMesh;
	m_pDynMesh->m_pDynGeom = this;
	m_pDynMesh->setSupportsDisplayList(false);

	SetOsgNode(this);

#if VTLISPSM
	m_pGeode->getOrCreateStateSet()->addUniform( new osg::Uniform( "renderingVTPBaseTexture", int( 1 ) ) );
#endif

	addDrawable(m_pDynMesh);
}


/**
 * Test a sphere against the view volume.
 *
 * \return VT_AllVisible if entirely inside the volume,
 *		   VT_Visible if partly inside,
 *		   otherwise 0.
 */
int vtDynGeom::IsVisible(const FSphere &sphere) const
{
	return IsVisible(sphere.center, sphere.radius);
}


/**
 * Test a single point against the view volume.
 *
 * \return true if inside, false if outside.
 */
bool vtDynGeom::IsVisible(const FPoint3& point) const
{
	// cull against standard frustum
	for (unsigned i = 0; i < 4; i++)
	{
		float dist = m_cullPlanes[i].Distance(point);
		if (dist > 0.0f)
			return false;
	}
	return true;
}


/**
 * Test a 3d triangle against the view volume.
 *
 * \return VT_AllVisible if entirely inside the volume,
 *			VT_Visible if partly intersecting,
 *			otherwise 0.
 */
int vtDynGeom::IsVisible(const FPoint3& point0,
						 const FPoint3& point1,
						 const FPoint3& point2,
						 const float fTolerance) const
{
	uint outcode0 = 0, outcode1 = 0, outcode2 = 0;
	register float dist;

	// cull against standard frustum
	int i;
	for (i = 0; i < 4; i++)
	{
		dist = m_cullPlanes[i].Distance(point0);
		if (dist > fTolerance)
			outcode0 |= (1 << i);

		dist = m_cullPlanes[i].Distance(point1);
		if (dist > fTolerance)
			outcode1 |= (1 << i);

		dist = m_cullPlanes[i].Distance(point2);
		if (dist > fTolerance)
			outcode2 |= (1 << i);
	}
	if (outcode0 == 0 && outcode1 == 0 && outcode2 == 0)
		return VT_AllVisible;
	if (outcode0 != 0 && outcode1 != 0 && outcode2 != 0)
	{
		if ((outcode0 & outcode1 & outcode2) != 0)
			return 0;
		else
		{
			// tricky case - just be conservative and assume some intersection
			return VT_Visible;
		}
	}
	// not all in, and not all out
	return VT_Visible;
}

/**
 * Test a sphere against the view volume.
 *
 * \return VT_AllVisible if entirely inside the volume,
 *			VT_Visible if partly intersecting,
 *			otherwise 0.
 */
int vtDynGeom::IsVisible(const FPoint3 &point, float radius) const
{
	uint incode = 0;

	// cull against standard frustum
	for (int i = 0; i < 4; i++)
	{
		float dist = m_cullPlanes[i].Distance(point);
		if (dist > radius)
			return 0;			// entirely outside this plane
		if (dist < -radius)
			incode |= (1 << i);	// entirely inside this plane
	}
	if (incode == 0x0f)
		return VT_AllVisible;	// entirely inside all planes
	else
		return VT_Visible;
}

void vtDynGeom::ApplyMaterial(vtMaterial *mat)
{
	if (m_pDynMesh && m_pDynMesh->m_pDrawState)
	{
		m_pDynMesh->m_pDrawState->apply(mat);
		// Dynamic terrain assumes texture unit 0
		m_pDynMesh->m_pDrawState->setActiveTextureUnit(0);
	}
}


///////////////////////////////////////////////////////////
// vtHUD

/**
 * Create a HUD node.  A HUD ("heads-up display") is a group whose whose
 * children are transformed to be drawn in window coordinates, rather
 * than world coordinates.
 *
 * You should only ever create one HUD node in your scenegraph.
 *
 * \param bPixelCoords If true, the child transforms should be interpreted
 *		as pixel coordinates, from (0,0) in the lower-left of the viewpoint.
 *		Otherwise, they are considered in normalized window coordinates,
 *		from (0,0) in the lower-left to (1,1) in the upper right.
 */
vtHUD::vtHUD(bool bPixelCoords)
{
	modelview_abs = new osg::MatrixTransform;
	modelview_abs->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	modelview_abs->setMatrix(osg::Matrix::identity());
	addChild(modelview_abs);

	// We can set the projection to pixels (0,width,0,height) or
	//	normalized (0,1,0,1)
	m_bPixelCoords = bPixelCoords;
	if (m_bPixelCoords)
	{
		IPoint2 winsize = vtGetScene()->GetWindowSize();

		// safety check first, avoid /0 crash
		if (winsize.x != 0 && winsize.y != 0)
			setMatrix(osg::Matrix::ortho2D(0, winsize.x, 0, winsize.y));
	}
	else
	{
		// Normalized window coordinates, 0 to 1
		setMatrix(osg::Matrix::ortho2D(0, 1, 0, 1));
	}

	// To ensure that the sprite appears on top we can use osg::Depth to
	//  force the depth fragments to be placed at the front of the screen.
	osg::StateSet* stateset = getOrCreateStateSet();
	stateset->setAttribute(new osg::Depth(osg::Depth::LESS,0.0,0.0001));

	SetOsgNode(this);

	vtGetScene()->SetHUD(this);
}

void vtHUD::SetWindowSize(int w, int h)
{
	if (m_bPixelCoords)
	{
		if (w != 0 && h != 0)
		{
			setMatrix(osg::Matrix::ortho2D(0, w, 0, h));
			// VTLOG("HUD SetWindowSize %d %d\n", w, h);
		}
	}
}


///////////////////////////////////////////////////////////////////////
// Intersection methods

#include <osgUtil/IntersectVisitor>

/**
 * Check for surface intersections along a line segment in space.
 *
 * \param pTop The top of the scene graph that you want to search for
 *		intersections.  This can be the root node if you want to seach your
 *		entire scene, or any other node to search a subgraph.
 * \param start The start point (world coordinates) of the line segment.
 * \param end	The end point (world coordinates) of the line segment.
 * \param hitlist The results.  If there are intersections (hits), they are
 *		placed in this list, which is simply a std::vector of vtHit objects.
 *		Each vtHit object gives information about the hit point.
 * \param bLocalCoords Pass true to get your results in local coordinates
 *		(in the frame of the object which was hit).  Otherwise, result points
 *		are in world coordinates.
 * \param bNativeNodes Pass true to return the internal (native) scene graph
 *		node that was hit, when there is no corresponding vtNode.  Pass false
 *		to always return a vtNode, by looking up the scene graph as needed.
 *
 * \return The number of intersection hits (size of the hitlist array).
 */
int vtIntersect(osg::Node *pTop, const FPoint3 &start, const FPoint3 &end,
				vtHitList &hitlist, bool bLocalCoords, bool bNativeNodes)
{
	// set up intersect visitor and create the line segment
	osgUtil::IntersectVisitor visitor;
	osgUtil::IntersectVisitor::HitList hlist;

	osg::ref_ptr<osg::LineSegment> segment = new osg::LineSegment;
	segment->set(v2s(start), v2s(end));
	visitor.addLineSegment(segment.get());

	// the accept() method does the intersection testing work
	pTop->accept(visitor);

	hlist = visitor.getHitList(segment.get());

#if 0	// Diagnostic code
	for(osgUtil::IntersectVisitor::HitList::iterator hitr=hlist.begin();
		hitr!=hlist.end(); ++hitr)
	{
		if (hitr->_geode.valid())
		{
			if (hitr->_geode->getName().empty())
				VTLOG("Geode %lx\n", hitr->_geode.get());
			else
				// the geodes are identified by name.
				VTLOG("Geode '%s'\n", hitr->_geode->getName().c_str());
		}
		else if (hitr->_drawable.valid())
		{
			VTLOG("Drawable class '%s'\n", hitr->_drawable->className());
		}
		else
		{
			osg::Vec3 point = hitr->getLocalIntersectPoint();
			VTLOG("Hitpoint %.1f %.1f %.1f\n", point.x(), point.y(), point.z());
		}
	}
	if (hlist.size() > 0)
		VTLOG("\n");
#endif

	// look through the node hits that OSG returned
	for(osgUtil::IntersectVisitor::HitList::iterator hitr=hlist.begin();
		hitr!=hlist.end(); ++hitr)
	{
		if (!hitr->_geode.valid())
			continue;

		// put it on the list of hit results
		vtHit hit;
		hit.geode = hitr->_geode.get();
		if (bLocalCoords)
		{
			hit.point = s2v(hitr->getLocalIntersectPoint());
		}
		else
		{
			hit.point = s2v(hitr->getWorldIntersectPoint());
		}
		hit.distance = (s2v(hitr->getWorldIntersectPoint()) - start).Length();
		hitlist.push_back(hit);
	}
	std::sort(hitlist.begin(), hitlist.end());
	return hitlist.size();
}

