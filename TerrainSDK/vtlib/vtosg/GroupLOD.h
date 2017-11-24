//
// GroupLOD.h
//
// Copyright (c) 2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTLIB_GROUPLOD_H
#define VTLIB_GROUPLOD_H

namespace osg {

/**
 * A simplified version of OSG's LOD node, which simply switches all its
 * on or off based on a single distance.  It also allows an extra distance
 * to globally increase the viewable distance for all node of this type.
 */
class GroupLOD : public Group
{
public :
	GroupLOD();

	/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
	GroupLOD(const GroupLOD&,const CopyOp& copyop=CopyOp::SHALLOW_COPY);

	typedef osg::BoundingSphere::vec_type vec_type;
	typedef osg::BoundingSphere::value_type value_type;

	virtual void traverse(NodeVisitor& nv);

	/** Set the object-space reference radius of the volume enclosed by the LOD. 
	* Used to determine the bounding sphere of the LOD in the absence of any children.*/
	inline void setRadius(value_type radius) { _radius = radius; }

	/** Get the object-space radius of the volume enclosed by the LOD.*/
	inline value_type getRadius() const { return _radius; }

	/** Set the LOD center point. */
	inline void setCenter(const Vec3 &center) { _userDefinedCenter = center; }

	/** return the LOD center point. */
	inline const Vec3& getCenter() const { return _userDefinedCenter; }

	/** Sets the maximum visible distance for this node. */
	void setDistance(float max) { _distance = max; }

	/** returns the maximum visible distance */
	inline float getDistance() const { return _distance; }

	/** Sets the (global, extra) visible distance for nodes of this type. */
	static void setGroupDistance(float max) { _groupDistance = max; }

	/** Gets the (global, extra) visible distance for nodes of this type. */
	static float getGroupDistance() { return _groupDistance; }

protected :
	virtual ~GroupLOD() {}

	Vec3		 _userDefinedCenter;
	float        _radius;
	float        _distance;
	static float _groupDistance;
};

}  // namespace osg

#endif // VTLIB_GROUPLOD_H