//
// Icosa.cpp
//
// Implements the DymaxIcosa class for operations on a dymaxion-style
// icosahedron.
//
// Thanks go to Robert W. Gray (http://www.rwgrayprojects.com/) and the
// Buckminster Fuller Institute (http://www.bfi.org/) for notes, references,
// permission, and example source for the following implementation.
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "Icosa.h"

//
// C arrays which contain the vertex index and face pair information for
// an icosahedron in the dymaxion orientation.
//
int icosa_face_v[20][3] =
{
	{ 0, 1, 2 },
	{ 0, 2, 3 },
	{ 0, 3, 4 },
	{ 0, 4, 5 },
	{ 0, 5, 1 },
	{ 7, 2, 1 },
	{ 2, 7, 8 },
	{ 8, 3, 2 },
	{ 3, 8, 9 },
	{ 9, 4, 3 },
	{ 4, 9, 10 },
	{ 10, 5, 4 },
	{ 5, 10, 6 },
	{ 6, 1, 5 },
	{ 1, 6, 7 },
	{ 11, 8, 7 },
	{ 11, 9, 8 },
	{ 11, 10, 9 },
	{ 11, 6, 10 },
	{ 11, 7, 6 }
};

int icosa_face_pairs[10][2] =
{
	{ 0, 5 },
	{ 1, 7 },
	{ 2, 9 },
	{ 3, 11 },
	{ 4, 13 },
	{ 6, 15 },
	{ 8, 16 },
	{ 10, 17 },
	{ 12, 18 },
	{ 14, 19 }
};

/**
 * Convert cartesian coordinates into spherical polar coordinates.
 */
void CartesianToSpherical(double *lng, double *lat,
						  double x, double y, double z)
{
	double a=0.0;

	if (x>0.0 && y>0.0){a = 0;}
	if (x<0.0 && y>0.0){a = PId;}
	if (x<0.0 && y<0.0){a = PId;}
	if (x>0.0 && y<0.0){a = PI2d;}
	*lat = acos(z);
	if (x==0.0 && y>0.0){*lng = PID2d;}
	if (x==0.0 && y<0.0){*lng = PId * 3 / 2;}
	if (x>0.0 && y==0.0){*lng = 0;}
	if (x<0.0 && y==0.0){*lng = PId;}
	if (x!=0.0 && y!=0.0){*lng = atan(y/x) + a;}
}


/**
 * Geographic coordinate to 3D point.  Input in degrees, output assumes
 * unit sphere.
 */
void geo_to_xyz(const DPoint2 &geo, DPoint3 &p)
{
	double gx = geo.x * (PId / 180.0);
	double gy = geo.y * (PId / 180.0);

	gx += PId;
	gy -= PID2d;

	p.x = sin(gy) * cos(gx);
	p.z = -sin(gy) * sin(gx);
	p.y = cos(gy);
}


DymaxIcosa::DymaxIcosa()
{
	InitIcosa();
}


/**
 * Determine which part of which icosahedral face a point is on.
 *
 * \param p A 3D point.
 * \param face Will receive the face number, from 0 to 19.
 * \param subface Will receive the subface number, from 0 to 5.
 */
void DymaxIcosa::FindFace(const DPoint3 &p, int &face, int &subface)
{
	double dist1, dist2, dist3;
	int i;

	face = 0;
	dist1 = 1E9;

	// The face center closest to the given point is the face which the
	//  point is in.
	for (i = 0; i < 20; i++)
	{
		dist2 = (m_face[i].center - p).Length();
		if (dist2 < dist1)
		{
			face = i;
			dist1 = dist2;
		}
	}

	// Now the subface is determined:
	// get the corner vertices of this face
	int v0 = icosa_face_v[face][0];
	int v1 = icosa_face_v[face][1];
	int v2 = icosa_face_v[face][2];

	// distance to each corner
	dist1 = (p - m_verts[v0]).Length();
	dist2 = (p - m_verts[v1]).Length();
	dist3 = (p - m_verts[v2]).Length();

	if ((dist1 <= dist2) && (dist2 <= dist3)) { subface = 0; }
	if ((dist1 <= dist3) && (dist3 <= dist2)) { subface = 5; }
	if ((dist2 <= dist1) && (dist1 <= dist3)) { subface = 1; }
	if ((dist2 <= dist3) && (dist3 <= dist1)) { subface = 2; }
	if ((dist3 <= dist1) && (dist1 <= dist2)) { subface = 4; }
	if ((dist3 <= dist2) && (dist2 <= dist1)) { subface = 3; }
}

/**
 * Given a point and a face number, determine the u,v,w coordinates of
 * the point in the reference frame of the face triangle.
 */
void DymaxIcosa::FindUV(const DPoint3 &p_in, int face, DPoint3 &uvw)
{
#if 0
	//
	// This R.W.Gray's algorithm for determining arc distances ("true" dymaxion
	//  projection).  If you use it, you need his permission.  Also, there is
	//  no reverse projection known, and finally, i don't understand it :)
	//
	DPoint3 Pn = m_face[face].vec_c;		// normal

	// Rd = [Xd, Yd, Zd]
	DPoint3 Rd = p_in;

	// Pn * Rd = Vd
	double Vd = Pn * Rd;

	// V0 = -(Pn * R0 + D)
	double V0 = -m_face[face].d;

	// t = V0 / Vd
	double t = V0 / Vd;

	// intersection point Pi = [Xi Yi Zi] = [X0 + Xd * t, Y0 + Yd * t, Z0 + Zd * t]
	DPoint3 Pi = Rd * t;
#else
	// We use simple Gnomonic
	double t = -m_face[face].d / (m_face[face].vec_c * p_in);
	DPoint3 Pi = p_in * t;
#endif

	// compute point relative to triangle origin
	DPoint3 p = Pi - m_face[face].base;

	// transform to find u,v,w face coordinates
	m_face[face].trans.Transform(p, uvw);
}

/**
 * Given a geographic coordinate (lon, lat), find the corresponding
 * face, subface, and UVW coordinates on surface of the icosahedron.
 */
void DymaxIcosa::FindFaceUV(const DPoint2 &p, int &face, int &subface,
							DPoint3 &uvw)
{
	DPoint3 p3;

	geo_to_xyz(p, p3);
	FindFace(p3, face, subface);
	FindUV(p3, face, uvw);
}

/**
 * Given a geographic coordinate (lon, lat), find the corresponding
 * point on the surface of the icosahedron.
 */
void DymaxIcosa::GeoToFacePoint(const DPoint2 &p, int &face, int &subface,
							 DPoint3 &p_out)
{
	DPoint3 p3, uvw;

	geo_to_xyz(p, p3);
	FindFace(p3, face, subface);
	FindUV(p3, face, uvw);

	p_out = m_face[face].base +
		   (m_face[face].vec_a * uvw.x) +
		   (m_face[face].vec_b * uvw.y);
}

/**
 * Given information about a point in the referenced frame of an icosahedral
 * face (tri, uvw), find the corresponding surface location in geographic
 * coordinate (lon, lat)
 */
void DymaxIcosa::FaceUVToGeo(int face, DPoint3 &uvw, double &lon, double &lat)
{
	DPoint3 p_out = m_face[face].base +
				   (m_face[face].vec_a * m_edge_length * uvw.x) +
				   (m_face[face].vec_b * m_edge_length * uvw.y);
	p_out.Normalize();
	DPoint3 p2;
	p2.x = p_out.x;
	p2.y = -p_out.z;
	p2.z = p_out.y;
	CartesianToSpherical(&lon, &lat, p2.x, p2.y, p2.z);
	lon += PId;
	if (lon > PI2d)
		lon -= PI2d;
}

/**
 * Initializes the object with the values of the icosahedron and
 * precomputes information for the faces.
 */
void DymaxIcosa::InitIcosa()
{
	// Cartesian coordinates for the 12 vertices of icosahedron
	m_verts[0].Set(  0.420152426708710003,  0.904082550615019298, -0.078145249402782959);
	m_verts[1].Set(  0.995009439436241649,  0.040147175877166645,  0.091347795276427931);
	m_verts[2].Set(  0.518836730327364437,  0.181331837557262454, -0.835420380378235850);
	m_verts[3].Set( -0.414682225320335218,  0.630675807891475371, -0.655962405434800777);
	m_verts[4].Set( -0.515455959944041808,  0.767200992517747538,  0.381716898287133011);
	m_verts[5].Set(  0.355781402532944713,  0.402234226602925571,  0.843580002466178147);
	m_verts[6].Set(  0.414682225320335218, -0.630675807891475371,  0.655962405434800777);
	m_verts[7].Set(  0.515455959944041808, -0.767200992517747538, -0.381716898287133011);
	m_verts[8].Set( -0.355781402532944713, -0.402234226602925571, -0.843580002466178147);
	m_verts[9].Set( -0.995009439436241649, -0.040147175877166645, -0.091347795276427931);
	m_verts[10].Set(-0.518836730327364437, -0.181331837557262454,  0.835420380378235850);
	m_verts[11].Set(-0.420152426708710003, -0.904082550615019298,  0.078145249402782959);

	for (int i = 0; i < 20; i++)
	{
		// look up corners of the face
		DPoint3 v1 = m_verts[icosa_face_v[i][0]];
		DPoint3 v2 = m_verts[icosa_face_v[i][1]];
		DPoint3 v3 = m_verts[icosa_face_v[i][2]];

		m_face[i].center = (v1 + v2 + v3) / 3.0f;
		m_face[i].center.Normalize();

		m_face[i].base = v1;

		// form edge vectors
		DPoint3 vec_a = v2 - v1;
		DPoint3 vec_b = v3 - v1;

		m_edge_length = vec_a.Length();

		// normalize and find orthogonal vector with cross product
		vec_a.Normalize();
		vec_b.Normalize();
		DPoint3 vec_c = vec_a.Cross(vec_b);
		vec_c.Normalize();

		m_face[i].vec_a = vec_a;
		m_face[i].vec_b = vec_b;
		m_face[i].vec_c = vec_c;

		// form a matrix expressing the tranformation from B->A
		DMatrix4 forwards4, inverse4;
		forwards4.Identity();
		forwards4.Set(0, 0, vec_a.x);
		forwards4.Set(1, 0, vec_a.y);
		forwards4.Set(2, 0, vec_a.z);
		forwards4.Set(0, 1, vec_b.x);
		forwards4.Set(1, 1, vec_b.y);
		forwards4.Set(2, 1, vec_b.z);
		forwards4.Set(0, 2, vec_c.x);
		forwards4.Set(1, 2, vec_c.y);
		forwards4.Set(2, 2, vec_c.z);

		// invert for the A->B transformation
		inverse4.Invert(forwards4);

		// extract a 3x3 matrix for simpler multiplication
		m_face[i].trans.SetByMatrix4(inverse4);

		// plane equation: ax + by + cz + d = 0
		// so, d = -ax0 -by0 -cz0
		m_face[i].d = -(vec_c.x * v1.x) -(vec_c.y * v1.y) -(vec_c.z * v1.z);
	}

	// parts of a unit-edge 2D triangle
#define SR3		1.732050807568877293527446341	// square root of 3
#define SR3D2	0.866025403784438646763723170	// square root of 3 divided by 2
#define TRI_H	SR3D2		// triangle height
#define TRI_DE	(1/(2*SR3))	// distance center to edge
#define TRI_DC	(1/SR3)		// distance center to corner

	// The 2D vertices of the projected map faces
	m_flatverts[0].Set(0.5, 0);
	m_flatverts[1].Set(1.5, 0);
	m_flatverts[2].Set(2.5, 0);
	m_flatverts[3].Set(3.5, 0);
	m_flatverts[4].Set(4.5, 0);
	m_flatverts[5].Set(0.75, TRI_H / 2);
	m_flatverts[6].Set(1.5, TRI_DC);
	m_flatverts[7].Set(2, TRI_DE);
	m_flatverts[8].Set(0, TRI_H);
	m_flatverts[9].Set(1, TRI_H);
	m_flatverts[10].Set(2, TRI_H);
	m_flatverts[11].Set(3, TRI_H);
	m_flatverts[12].Set(4, TRI_H);
	m_flatverts[13].Set(5, TRI_H);
	m_flatverts[14].Set(5.5, TRI_H);
	m_flatverts[15].Set(0.5, TRI_H*2);
	m_flatverts[16].Set(1.5, TRI_H*2);
	m_flatverts[17].Set(2.5, TRI_H*2);
	m_flatverts[18].Set(3.5, TRI_H*2);
	m_flatverts[19].Set(4.5, TRI_H*2);
	m_flatverts[20].Set(5.5, TRI_H*2);
	m_flatverts[21].Set(1, TRI_H*3);
	m_flatverts[22].Set(2, TRI_H*3);
	m_flatverts[23].Set(3, TRI_H*3);
	m_flatverts[24].Set(4, TRI_H*3);
	m_flatverts[25].Set(5, TRI_H*3);
	m_flatverts[26].Set(6, TRI_H);
}

double DymaxIcosa::DihedralAngle()
{
	double dot = m_face[1].vec_c * m_face[2].vec_c;
	double dihedral = acos(dot);
	return dihedral;
}

/**
 * Given a geographic coordinate (lon, lat), find the corresponding
 * dymaxion map coordinate, on the classic flattened dymaxion map.
 * The output is unit-edge triangles, which means the whole output
 * extents are x [0, 5.5] and y [0, 2.6].
 */
bool DymaxIcosa::GeoToDymax(const DPoint2 &geo, DPoint2 &dymax)
{
	DPoint3 p3, uvw;
	int face, subface;

	geo_to_xyz(geo, p3);
	FindFace(p3, face, subface);
	FindUV(p3, face, uvw);

	DPoint2 uv(uvw.x, uvw.y);
	//assert(uv.x <= 1 && uv.y <= 1);

	// Not exactly sure why we need to do this - apparently the UV are
	//  assuming a unit-radius, rather than unit-edge icosahedron
	uv /= m_edge_length;

	int a, b, c;
	switch (face)
	{
	case 0:
		a = 17; b = 22; c = 16; break;
	case 1:
		a = 17; b = 16; c = 10; break;
	case 2:
		a = 17; b = 10; c = 11; break;
	case 3:
		a = 17; b = 11; c = 18; break;
	case 4:
		a = 17; b = 23; c = 22; break;
	case 5:
		a = 21; b = 16; c = 22; break;
	case 6:
		a = 16; b = 15; c = 9; break;
	case 7:
		a = 9; b = 10; c = 16; break;
	case 8:
		if (subface == 4 || subface == 5)
			{ a = 10; b = 1; c = 2; }
		else
			{ a = 10; b = 9; c = 1; }
		break;
	case 9:
		a = 2; b = 11; c = 10; break;
	case 10:
		a = 11; b = 3; c = 12; break;
	case 11:
		a = 12; b = 18; c = 11; break;
	case 12:
		a = 18; b = 12; c = 19; break;
	case 13:
		a = 19; b = 24; c = 18; break;
	case 14:
		a = 25; b = 19; c = 20; break;
	case 15:
		if (subface == 0 || subface == 4 || subface == 5)
			{ a = 13; b = 26; c = 20; }
		else
			{ a = 0; b = 9; c = 8; }
		break;
	case 16:
		a = 0; b = 1; c = 9; break;
	case 17:
		a = 4; b = 12; c = 3; break;
	case 18:
		a = 13; b = 19; c = 12; break;
	case 19:
		a = 13; b = 20; c = 19; break;
	default:
		return false;
	}
	DPoint2 base = m_flatverts[a];
	DPoint2 vec1 = m_flatverts[b] - base;
	DPoint2 vec2 = m_flatverts[c] - base;

	dymax = base + (vec1 * uv.x) + (vec2 * uv.y);
	return true;
}

void DymaxIcosa::AddFlatTri(DLine2Array &polys, int a, int b, int c, int d)
{
	DLine2 poly;
	poly.Append(m_flatverts[a]);
	poly.Append(m_flatverts[b]);
	poly.Append(m_flatverts[c]);
	if (d != -1)
		poly.Append(m_flatverts[d]);
	polys.push_back(poly);
}

void DymaxIcosa::GetDymaxEdges(DLine2Array &polys)
{
	AddFlatTri(polys, 5, 9, 8);
	AddFlatTri(polys, 0, 1, 9);
	AddFlatTri(polys, 1, 6, 10, 9);
	AddFlatTri(polys, 7, 2, 10);
	AddFlatTri(polys, 2, 11, 10);
	AddFlatTri(polys, 11, 3, 12);
	AddFlatTri(polys, 3, 4, 12);

	AddFlatTri(polys, 15, 9, 16);
	AddFlatTri(polys, 9, 10, 16);
	AddFlatTri(polys, 16, 10, 17);
	AddFlatTri(polys, 10, 11, 17);
	AddFlatTri(polys, 17, 11, 18);
	AddFlatTri(polys, 11, 12, 18);
	AddFlatTri(polys, 18, 12, 19);
	AddFlatTri(polys, 12, 13, 19);
	AddFlatTri(polys, 19, 13, 20);
	AddFlatTri(polys, 13, 14, 20);

	AddFlatTri(polys, 21, 16, 22);
	AddFlatTri(polys, 16, 17, 22);
	AddFlatTri(polys, 22, 17, 23);
	AddFlatTri(polys, 18, 19, 24);
	AddFlatTri(polys, 19, 20, 25);
}

