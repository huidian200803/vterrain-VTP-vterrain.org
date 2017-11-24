//
// A handy set of inline functions to convert between the VTP and OSG
// math classes.  v2s() converts from VTP to OSG, s2v() goes the other way.
//
// Copyright (c) 2001-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTOSG_MATHH
#define VTOSG_MATHH

// Before OSG 2.6, we assume that OSG was built with float matrices.  OSG did
//  not expose whether it was built with OSG_USE_FLOAT_MATRIX or not, so we
//  must assume.  From OSG 2.6 onwards, OSG exposes it in include/osg/Config.
#include <osg/Version>
#if (OSG_VERSION_MAJOR==2 && OSG_VERSION_MINOR<6) || OSG_VERSION_MAJOR<2
#define OSG_USE_FLOAT_MATRIX
#endif

#include <osg/Vec2>
#include <osg/Vec4>
#include <osg/BoundingBox>
#include <osg/BoundingSphere>
#include <osg/Matrix>

#ifdef OSG_USE_FLOAT_MATRIX
typedef float osg_matrix_value;
#else
typedef double osg_matrix_value;
#endif

///////////////////////
// Math helpers.
//
// The s2v methods convert from "S" (the underlying scenegraph, OSG) to "v" (VTP).
// The v2s methods convert from "v" (VTP) to "S" (the underlying scenegraph, OSG).

inline void v2s(const FPoint2 &f, osg::Vec2 &s) { s[0] = f.x; s[1] = f.y; }
inline void v2s(const FPoint3 &f, osg::Vec3 &s) { s[0] = f.x; s[1] = f.y; s[2] = f.z; }
inline void v2s(const RGBf &f, osg::Vec3 &s) { s[0] = f.r; s[1] = f.g; s[2] = f.b; }
inline void v2s(const RGBAf &f, osg::Vec4 &s) { s[0] = f.r; s[1] = f.g; s[2] = f.b; s[3] = f.a; }

inline osg::Vec3 v2s(const FPoint3 &f)
{
	osg::Vec3 s;
	s[0] = f.x; s[1] = f.y; s[2] = f.z;
	return s;
}

inline osg::Vec4 v2s(const RGBf &f)
{
	osg::Vec4 s;
	s[0] = f.r; s[1] = f.g; s[2] = f.b; s[3] = 1.0f;
	return s;
}

inline osg::Vec4 v2s(const RGBAf &f)
{
	osg::Vec4 s;
	s[0] = f.r; s[1] = f.g; s[2] = f.b; s[3] = f.a;
	return s;
}

inline void v2s(const FSphere &sph, osg::BoundingSphere &bs)
{
	v2s(sph.center, bs._center);
	bs._radius = sph.radius;
}

inline void s2v(const osg::Vec3 &s, FPoint3 &f) { f.x = s[0]; f.y = s[1]; f.z = s[2]; }
inline void s2v(const osg::Vec2 &s, FPoint2 &f) { f.x = s[0]; f.y = s[1]; }
inline void s2v(const osg::Vec3 &s, RGBf &f) { f.r = s[0]; f.g = s[1]; f.b = s[2]; }
inline void s2v(const osg::Vec4 &s, RGBAf &f) { f.r = s[0]; f.g = s[1]; f.b = s[2]; f.a = s[3]; }

inline void s2v(const osg::BoundingSphere &bs, FSphere &sph)
{
	s2v(bs._center, sph.center);
	sph.radius = bs._radius;
}

inline void s2v(const osg::BoundingBox &bs, FBox3 &box)
{
	box.min.x = bs._min[0];
	box.min.y = bs._min[1];
	box.min.z = bs._min[2];

	box.max.x = bs._max[0];
	box.max.y = bs._max[1];
	box.max.z = bs._max[2];
}

inline FPoint3 s2v(const osg::Vec3 &s)
{
	FPoint3 f;
	f.x = s[0]; f.y = s[1]; f.z = s[2];
	return f;
}

inline RGBf s2v(const osg::Vec4 &s)
{
	RGBf f;
	f.r = s[0]; f.g = s[1]; f.b = s[2];
	return f;
}

inline void ConvertMatrix4(const osg::Matrix *mat_osg, FMatrix4 *mat)
{
	const osg_matrix_value *ptr = mat_osg->ptr();
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
		{
			mat->Set(j, i, ptr[(i<<2)+j]);
		}
}

inline void ConvertMatrix4(const FMatrix4 *mat, osg::Matrix *mat_osg)
{
	osg_matrix_value *ptr = mat_osg->ptr();
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
		{
			ptr[(i<<2)+j] = mat->Get(j, i);
		}
}

class vtVec3 : public osg::Vec3
{
public:
	vtVec3() {}
	vtVec3(const FPoint3& v) : osg::Vec3(v.x, v.y, v.z) {}
	vtVec3(const osg::Vec3& v) : osg::Vec3(v) {}
};

class vtVec2 : public osg::Vec2
{
public:
	vtVec2() {}
	vtVec2(const FPoint2& v) : osg::Vec2(v.x, v.y) {}
	vtVec2(const osg::Vec2& v) : osg::Vec2(v) {}
	inline vtVec2& operator /= (const vtVec2& rhs)
	{
		_v[0] /= rhs._v[0];
		_v[1]/= rhs._v[1];
		return *this;
	}
};

#endif	// VTOSG_MATHH

