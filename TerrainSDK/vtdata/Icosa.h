//
// Icosa.h
//
// Thanks go to Robert W. Gray (http://www.rwgrayprojects.com/) and the
// Buckminster Fuller Institute (http://www.bfi.org/) for notes, references,
// permission, and ancestral source for the following implementation.
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ICOSAH
#define ICOSAH

#include "MathTypes.h"

struct icoface
{
	DPoint3 base;				// a designated "base corner" of each face
	DPoint3 center;				// the center of the face
	DPoint3 vec_a, vec_b, vec_c;	// normalized basis vectors

	/// Transform to convert a point to (u,v,w) face coordinates
	DMatrix3 trans;

	double d;	// forth component of the plane equation
};

/**
 * This class contains a full description of an icosahedron in the Dymaxion
 * orientation.  It provides the ability to convert points to and from a
 * simple Gnomonic projection based on the icosahedron.  No attempt is made
 * to implement the precise "Dymaxion Projection" which would require using
 * arc lengths.
 *
 * The basis for this decision is Robert Gray's notes which state:
 * "Fuller's world map image is indistinguishable to the world map image
 * created by the Gnomonic projection method indicates that it is the use
 * of the icosahedron, and not the projection method, that seems to be the
 * critical point."
 */
class DymaxIcosa
{
public:
	DymaxIcosa();

	void InitIcosa();

	void FindFaceUV(const DPoint2 &p, int &face, int &subface, DPoint3 &uvw);
	void FindFace(const DPoint3 &p, int &face, int &subface);
	void FindUV(const DPoint3 &p_in, int face, DPoint3 &uvw);

	void GeoToFacePoint(const DPoint2 &p, int &face, int &subface, DPoint3 &p_out);
	void FaceUVToGeo(int face, DPoint3 &uvw, double &lon, double &lat);
	bool GeoToDymax(const DPoint2 &geo, DPoint2 &dymax);

	double DihedralAngle();

	void GetDymaxEdges(DLine2Array &polys);

protected:
	void AddFlatTri(DLine2Array &polys, int a, int b, int c, int d=-1);

	// icosahedron data
	DPoint3 m_verts[12];
	icoface m_face[20];
	DPoint2 m_flatverts[27];
	double m_edge_length;	// edge length of icosahedron (~1.05 for unit radius)
};

// vertex indices for each face of the icosahedron
extern int icosa_face_v[20][3];
extern int icosa_face_pairs[10][2];

#endif	// ICOSAH

