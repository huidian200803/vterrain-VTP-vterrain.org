//
// Implementation of matrix methods for the basic math classes.
//
// Copyright (c) 2001-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "MathTypes.h"

static float det3x3(float a1, float a2, float a3, float b1, float b2, float b3, float c1, float c2, float c3)
{
		return (float) (a1 * (b2 * c3 - b3 * c2) - b1 * (a2 * c3 - a3 * c2) + c1 * (a2 * b3 - a3 * b2));
}

static double det3x3(double a1, double a2, double a3, double b1, double b2, double b3, double c1, double c2, double c3)
{
		return (double) (a1 * (b2 * c3 - b3 * c2) - b1 * (a2 * c3 - a3 * c2) + c1 * (a2 * b3 - a3 * b2));
}
#define SMALL_NUMBER		1.e-12f

//
// DMatrix methods
//

static double Dot3f(const double *d1, const double *d2)
{
	return d1[0]*d2[0]+d1[1]*d2[1]+d1[2]*d2[2];
}

static float Dot3f(const float *d1, const float *d2)
{
	return d1[0]*d2[0]+d1[1]*d2[1]+d1[2]*d2[2];
}

void DMatrix3::Identity()
{
	data[0][0] = 1.0;
	data[0][1] = 0.0;
	data[0][2] = 0.0;
	data[1][0] = 0.0;
	data[1][1] = 1.0;
	data[1][2] = 0.0;
	data[2][0] = 0.0;
	data[2][1] = 0.0;
	data[2][2] = 1.0;
}

void DMatrix3::AxisAngle(const DPoint3 &vec, double theta)
{
	double cost = cos(theta), sint = sin(theta);

	double a2, b2, c2, abm, acm, bcm;
	double mcos, asin, bsin, csin;
	mcos = 1.0f - cost;
	a2 = vec.x * vec.x;
	b2 = vec.y * vec.y;
	c2 = vec.z * vec.z;
	abm = vec.x * vec.y * mcos;
	acm = vec.x * vec.z * mcos;
	bcm = vec.y * vec.z * mcos;
	asin = vec.x * sint;
	bsin = vec.y * sint;
	csin = vec.z * sint;
	data[0][0] = a2 * mcos + cost;
	data[0][1] = abm - csin;
	data[0][2] = acm + bsin;
	data[1][0] = abm + csin;
	data[1][1] = b2 * mcos + cost;
	data[1][2] = bcm - asin;
	data[2][0] = acm - bsin;
	data[2][1] = bcm + asin;
	data[2][2] = c2 * mcos + cost;
}

void DMatrix3::Transform(const DPoint3 &tmp, DPoint3 &dst) const
{
	dst.x = Dot3f(&tmp.x, data[0]);
	dst.y = Dot3f(&tmp.x, data[1]);
	dst.z = Dot3f(&tmp.x, data[2]);
}

void DMatrix3::SetByMatrix4(const DMatrix4 &m)
{
	data[0][0] = m.Get(0,0); data[0][1] = m.Get(0,1); data[0][2] = m.Get(0,2);
	data[1][0] = m.Get(1,0); data[1][1] = m.Get(1,1); data[1][2] = m.Get(1,2);
	data[2][0] = m.Get(2,0); data[2][1] = m.Get(2,1); data[2][2] = m.Get(2,2);
}

/////////////////////////////////////////////////////////////////////
// DMatrix4

void DMatrix4::Identity()
{
	data[0][0] = 1.0;
	data[0][1] = 0.0;
	data[0][2] = 0.0;
	data[0][3] = 0.0;
	data[1][0] = 0.0;
	data[1][1] = 1.0;
	data[1][2] = 0.0;
	data[1][3] = 0.0;
	data[2][0] = 0.0;
	data[2][1] = 0.0;
	data[2][2] = 1.0;
	data[2][3] = 0.0;
	data[3][0] = 0.0;
	data[3][1] = 0.0;
	data[3][2] = 0.0;
	data[3][3] = 1.0;
}

void DMatrix4::AxisAngle(const DPoint3 &vec, double theta)
{
	double cost = cos(theta), sint = sin(theta);

	double a2, b2, c2, abm, acm, bcm;
	double mcos, asin, bsin, csin;
	mcos = 1.0f - cost;
	a2 = vec.x * vec.x;
	b2 = vec.y * vec.y;
	c2 = vec.z * vec.z;
	abm = vec.x * vec.y * mcos;
	acm = vec.x * vec.z * mcos;
	bcm = vec.y * vec.z * mcos;
	asin = vec.x * sint;
	bsin = vec.y * sint;
	csin = vec.z * sint;

	data[0][0] = a2 * mcos + cost;
	data[0][1] = abm - csin;
	data[0][2] = acm + bsin;
	data[0][3] = 0;

	data[1][0] = abm + csin;
	data[1][1] = b2 * mcos + cost;
	data[1][2] = bcm - asin;
	data[1][3] = 0;

	data[2][0] = acm - bsin;
	data[2][1] = bcm + asin;
	data[2][2] = c2 * mcos + cost;
	data[2][3] = 0;

	data[3][0] = 0;
	data[3][1] = 0;
	data[3][2] = 0;
	data[3][3] = 1;
}

static void Full_Inverse_Xform3(const double b[4][4], double a[4][4])
{
	long indxc[4], indxr[4], ipiv[4];
	long i, icol=0, irow=0, j, ir, ic;	// set to 0 to avoid compiler warning
	double big, dum, pivinv, temp, bb;
	ipiv[0] = -1;
	ipiv[1] = -1;
	ipiv[2] = -1;
	ipiv[3] = -1;
	a[0][0] = b[0][0];
	a[1][0] = b[1][0];
	a[2][0] = b[2][0];
	a[3][0] = b[3][0];
	a[0][1] = b[0][1];
	a[1][1] = b[1][1];
	a[2][1] = b[2][1];
	a[3][1] = b[3][1];
	a[0][2] = b[0][2];
	a[1][2] = b[1][2];
	a[2][2] = b[2][2];
	a[3][2] = b[3][2];
	a[0][3] = b[0][3];
	a[1][3] = b[1][3];
	a[2][3] = b[2][3];
	a[3][3] = b[3][3];
	for (i = 0; i < 4; i++) {
		big = 0.0f;
		for (j = 0; j < 4; j++) {
			if (ipiv[j] != 0) {
				if (ipiv[0] == -1) {
					if ((bb = (double) fabs(a[j][0])) > big) {
						big = bb;
						irow = j;
						icol = 0;
					}
				} else if (ipiv[0] > 0) {
					return;
				}
				if (ipiv[1] == -1) {
					if ((bb = (double) fabs((double) a[j][1])) > big) {
						big = bb;
						irow = j;
						icol = 1;
					}
				} else if (ipiv[1] > 0) {
					return;
				}
				if (ipiv[2] == -1) {
					if ((bb = (double) fabs((double) a[j][2])) > big) {
						big = bb;
						irow = j;
						icol = 2;
					}
				} else if (ipiv[2] > 0) {
					return;
				}
				if (ipiv[3] == -1) {
					if ((bb = (double) fabs((double) a[j][3])) > big) {
						big = bb;
						irow = j;
						icol = 3;
					}
				} else if (ipiv[3] > 0) {
					return;
				}
			}
		}
		++(ipiv[icol]);
		if (irow != icol) {
			temp = a[irow][0];
			a[irow][0] = a[icol][0];
			a[icol][0] = temp;
			temp = a[irow][1];
			a[irow][1] = a[icol][1];
			a[icol][1] = temp;
			temp = a[irow][2];
			a[irow][2] = a[icol][2];
			a[icol][2] = temp;
			temp = a[irow][3];
			a[irow][3] = a[icol][3];
			a[icol][3] = temp;
		}
		indxr[i] = irow;
		indxc[i] = icol;
		if (a[icol][icol] == 0.0) {
			return;
		}
		pivinv = 1.0f / a[icol][icol];
		a[icol][icol] = 1.0f;
		a[icol][0] *= pivinv;
		a[icol][1] *= pivinv;
		a[icol][2] *= pivinv;
		a[icol][3] *= pivinv;
		if (icol != 0) {
			dum = a[0][icol];
			a[0][icol] = 0.0f;
			a[0][0] -= a[icol][0] * dum;
			a[0][1] -= a[icol][1] * dum;
			a[0][2] -= a[icol][2] * dum;
			a[0][3] -= a[icol][3] * dum;
		}
		if (icol != 1) {
			dum = a[1][icol];
			a[1][icol] = 0.0f;
			a[1][0] -= a[icol][0] * dum;
			a[1][1] -= a[icol][1] * dum;
			a[1][2] -= a[icol][2] * dum;
			a[1][3] -= a[icol][3] * dum;
		}
		if (icol != 2) {
			dum = a[2][icol];
			a[2][icol] = 0.0f;
			a[2][0] -= a[icol][0] * dum;
			a[2][1] -= a[icol][1] * dum;
			a[2][2] -= a[icol][2] * dum;
			a[2][3] -= a[icol][3] * dum;
		}
		if (icol != 3) {
			dum = a[3][icol];
			a[3][icol] = 0.0f;
			a[3][0] -= a[icol][0] * dum;
			a[3][1] -= a[icol][1] * dum;
			a[3][2] -= a[icol][2] * dum;
			a[3][3] -= a[icol][3] * dum;
		}
	}
	if (indxr[3] != indxc[3]) {
		ir = indxr[3];
		ic = indxc[3];
		temp = a[0][ir];
		a[0][ir] = a[0][ic];
		a[0][ic] = temp;
		temp = a[1][ir];
		a[1][ir] = a[1][ic];
		a[1][ic] = temp;
		temp = a[2][ir];
		a[2][ir] = a[2][ic];
		a[2][ic] = temp;
		temp = a[3][ir];
		a[3][ir] = a[3][ic];
		a[3][ic] = temp;
	}
	if (indxr[2] != indxc[2]) {
		ir = indxr[2];
		ic = indxc[2];
		temp = a[0][ir];
		a[0][ir] = a[0][ic];
		a[0][ic] = temp;
		temp = a[1][ir];
		a[1][ir] = a[1][ic];
		a[1][ic] = temp;
		temp = a[2][ir];
		a[2][ir] = a[2][ic];
		a[2][ic] = temp;
		temp = a[3][ir];
		a[3][ir] = a[3][ic];
		a[3][ic] = temp;
	}
	if (indxr[1] != indxc[1]) {
		ir = indxr[1];
		ic = indxc[1];
		temp = a[0][ir];
		a[0][ir] = a[0][ic];
		a[0][ic] = temp;
		temp = a[1][ir];
		a[1][ir] = a[1][ic];
		a[1][ic] = temp;
		temp = a[2][ir];
		a[2][ir] = a[2][ic];
		a[2][ic] = temp;
		temp = a[3][ir];
		a[3][ir] = a[3][ic];
		a[3][ic] = temp;
	}
	if (indxr[0] != indxc[0]) {
		ir = indxr[0];
		ic = indxc[0];
		temp = a[0][ir];
		a[0][ir] = a[0][ic];
		a[0][ic] = temp;
		temp = a[1][ir];
		a[1][ir] = a[1][ic];
		a[1][ic] = temp;
		temp = a[2][ir];
		a[2][ir] = a[2][ic];
		a[2][ic] = temp;
		temp = a[3][ir];
		a[3][ir] = a[3][ic];
		a[3][ic] = temp;
	}
}

static void Down_triangle_Inverse_Xform(const double src[4][4], double dst[4][4])
{
	double det1, o33;
	double a1, a2, a3, b1, b2, b3, c1, c2, c3;
	a1 = src[0][0];
	b1 = src[0][1];
	c1 = src[0][2];
	a2 = src[1][0];
	b2 = src[1][1];
	c2 = src[1][2];
	a3 = src[2][0];
	b3 = src[2][1];
	c3 = src[2][2];
	det1 = det3x3(a1, a2, a3, b1, b2, b3, c1, c2, c3);
	if (fabs(det1) < SMALL_NUMBER) {
		return;
	} else {
		det1 = 1.0f / det1;
		o33 = 1.0f / src[3][3];
		dst[0][0] =  (b2 * c3 - c2 * b3) * det1;
		dst[1][0] = -(a2 * c3 - c2 * a3) * det1;
		dst[2][0] =  (a2 * b3 - b2 * a3) * det1;
		dst[0][1] = -(b1 * c3 - c1 * b3) * det1;
		dst[1][1] =  (a1 * c3 - c1 * a3) * det1;
		dst[2][1] = -(a1 * b3 - b1 * a3) * det1;
		dst[0][2] =  (b1 * c2 - c1 * b2) * det1;
		dst[1][2] = -(a1 * c2 - c1 * a2) * det1;
		dst[2][2] =  (a1 * b2 - b1 * a2) * det1;
		dst[3][0] = -(src[3][0] * dst[0][0] + src[3][1] * dst[1][0] + src[3][2] * dst[2][0]) * o33;
		dst[3][1] = -(src[3][0] * dst[0][1] + src[3][1] * dst[1][1] + src[3][2] * dst[2][1]) * o33;
		dst[3][2] = -(src[3][0] * dst[0][2] + src[3][1] * dst[1][2] + src[3][2] * dst[2][2]) * o33;
		dst[0][3] = 0.0f;
		dst[1][3] = 0.0f;
		dst[2][3] = 0.0f;
		dst[3][3] = o33;
	}
}

static void Up_triangle_Inverse_Xform(const double src[4][4], double dst[4][4])
{
	double det1, o33;
	double a1, a2, a3, b1, b2, b3, c1, c2, c3;
	a1 = src[0][0];
	b1 = src[0][1];
	c1 = src[0][2];
	a2 = src[1][0];
	b2 = src[1][1];
	c2 = src[1][2];
	a3 = src[2][0];
	b3 = src[2][1];
	c3 = src[2][2];
	det1 = det3x3(a1, a2, a3, b1, b2, b3, c1, c2, c3);
	if (fabs(det1) < SMALL_NUMBER) {
		return;
	} else {
		det1 = (double) (1.0f / det1);
		o33 = 1.0f / src[3][3];
		dst[0][0] =  (b2 * c3 - c2 * b3) * det1;
		dst[1][0] = -(a2 * c3 - c2 * a3) * det1;
		dst[2][0] =  (a2 * b3 - b2 * a3) * det1;
		dst[0][1] = -(b1 * c3 - c1 * b3) * det1;
		dst[1][1] =  (a1 * c3 - c1 * a3) * det1;
		dst[2][1] = -(a1 * b3 - b1 * a3) * det1;
		dst[0][2] =  (b1 * c2 - c1 * b2) * det1;
		dst[1][2] = -(a1 * c2 - c1 * a2) * det1;
		dst[2][2] =  (a1 * b2 - b1 * a2) * det1;
		dst[0][3] = -(src[0][3] * dst[0][0] + src[1][3] * dst[0][1] + src[2][3] * dst[0][2]) * o33;
		dst[1][3] = -(src[0][3] * dst[1][0] + src[1][3] * dst[1][1] + src[2][3] * dst[1][2]) * o33;
		dst[2][3] = -(src[0][3] * dst[2][0] + src[1][3] * dst[2][1] + src[2][3] * dst[2][2]) * o33;
		dst[3][0] = 0.0f;
		dst[3][1] = 0.0f;
		dst[3][2] = 0.0f;
		dst[3][3] = o33;
	}
}

void DMatrix4::Invert(const DMatrix4 &src)
{
	if ((src.data[0][3] == 0) && (src.data[1][3] == 0) && (src.data[2][3] == 0)) {
		Down_triangle_Inverse_Xform(src.data, data);
		return;
	} else if ((src.data[3][0] == 0) && (src.data[3][1] == 0) && (src.data[3][2] == 0)) {
		Up_triangle_Inverse_Xform(src.data, data);
		return;
	} else {
		Full_Inverse_Xform3(src.data, data);
		return;
	}
}


//////////////////////////////////////////////////////////////
// FMatrix3

void FMatrix3::SetRow(int row, float f0, float f1, float f2)
{
	// matrices are stored as [column][row]
	data[0][row] = f0;
	data[1][row] = f1;
	data[2][row] = f2;
}

void FMatrix3::Identity()
{
	data[0][0] = 1.0f;
	data[0][1] = 0.0f;
	data[0][2] = 0.0f;
	data[1][0] = 0.0f;
	data[1][1] = 1.0f;
	data[1][2] = 0.0f;
	data[2][0] = 0.0f;
	data[2][1] = 0.0f;
	data[2][2] = 1.0f;
}

bool FMatrix3::IsIdentity() const
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (i == j)
			{
				if (data[i][j] != 1.0f) return false;
			}
			else
			{
				if (data[i][j] != 0.0f) return false;
			}
		}
	}
	return true;
}

void FMatrix3::AxisAngle(const FPoint3 &vec, double theta)
{
	double cost = cos(theta), sint = sin(theta);

	double a2, b2, c2, abm, acm, bcm;
	double mcos, asin, bsin, csin;
	mcos = 1.0f - cost;
	a2 = vec.x * vec.x;
	b2 = vec.y * vec.y;
	c2 = vec.z * vec.z;
	abm = vec.x * vec.y * mcos;
	acm = vec.x * vec.z * mcos;
	bcm = vec.y * vec.z * mcos;
	asin = vec.x * sint;
	bsin = vec.y * sint;
	csin = vec.z * sint;
	data[0][0] = (float) (a2 * mcos + cost);
	data[0][1] = (float) (abm - csin);
	data[0][2] = (float) (acm + bsin);
	data[1][0] = (float) (abm + csin);
	data[1][1] = (float) (b2 * mcos + cost);
	data[1][2] = (float) (bcm - asin);
	data[2][0] = (float) (acm - bsin);
	data[2][1] = (float) (bcm + asin);
	data[2][2] = (float) (c2 * mcos + cost);
}

/**
 * Create a rotation matrix that represents an orientation that points
 * along a given vector.
 */
void FMatrix3::MakeOrientation(const FPoint3 &vector, bool bPitch)
{
	float theta = atan2f(-vector.z, vector.x) - PID2f;
	float phi = 0;
	if (bPitch)
		phi = asinf(vector.y / vector.Length());

	Identity();

	FMatrix3 tmp;
	tmp.AxisAngle(FPoint3(0,1,0), theta);
	PreMult(tmp);

	tmp.AxisAngle(FPoint3(1,0,0), phi);
	PreMult(tmp);
}

void FMatrix3::Transform(const FPoint3 &tmp, FPoint3 &dst) const
{
	dst.x = Dot3f(&tmp.x, data[0]);
	dst.y = Dot3f(&tmp.x, data[1]);
	dst.z = Dot3f(&tmp.x, data[2]);
}

void FMatrix3::SetFromVectors(const FPoint3 &forward, const FPoint3 &up)
{
	FPoint3 f = forward;
	f.Normalize();
	FPoint3 side = f.Cross(up);
	side.Normalize();

	SetRow(0,	side.x,		side.y,		side.z);
	SetRow(1,	up.x,		up.y,		up.z);
	SetRow(2,	-forward.x,	-forward.y,	-forward.z);
}

void FMatrix3::SetFromMatrix4(const FMatrix4 &mat)
{
	SetRow(0, mat(0,0), mat(1,0), mat(2,0));
	SetRow(1, mat(0,1), mat(1,1), mat(2,1));
	SetRow(2, mat(0,2), mat(1,2), mat(2,2));
}

#define INNER_PRODUCT3(a, b, c, r) \
	 ((a).data[0][r] * (b).data[c][0]) \
	+((a).data[1][r] * (b).data[c][1]) \
	+((a).data[2][r] * (b).data[c][2])

void FMatrix3::PreMult(const FMatrix3 &mat)
{
	float t[3];
	for (int col = 0; col < 3; ++col)
	{
		t[0] = INNER_PRODUCT3(mat, *this, col, 0);
		t[1] = INNER_PRODUCT3(mat, *this, col, 1);
		t[2] = INNER_PRODUCT3(mat, *this, col, 2);
		data[col][0] = t[0];
		data[col][1] = t[1];
		data[col][2] = t[2];
	}
}

void FMatrix3::PostMult(const FMatrix3 &mat)
{
	float t[3];
	for (int row = 0; row < 3; ++row)
	{
		t[0] = INNER_PRODUCT3(*this, mat, 0, row);
		t[1] = INNER_PRODUCT3(*this, mat, 1, row);
		t[2] = INNER_PRODUCT3(*this, mat, 2, row);
		SetRow(row, t[0], t[1], t[2]);
	}
}


//////////////////////////////////////////////////////////////
// FMatrix4

void FMatrix4::SetRow(int row, float f0, float f1, float f2, float f3)
{
	// our matrices are stored as [column][row]
	data[0][row] = f0;
	data[1][row] = f1;
	data[2][row] = f2;
	data[3][row] = f3;
}

void FMatrix4::Identity()
{
	data[0][0] = 1.0f;
	data[0][1] = 0.0f;
	data[0][2] = 0.0f;
	data[0][3] = 0.0f;
	data[1][0] = 0.0f;
	data[1][1] = 1.0f;
	data[1][2] = 0.0f;
	data[1][3] = 0.0f;
	data[2][0] = 0.0f;
	data[2][1] = 0.0f;
	data[2][2] = 1.0f;
	data[2][3] = 0.0f;
	data[3][0] = 0.0f;
	data[3][1] = 0.0f;
	data[3][2] = 0.0f;
	data[3][3] = 1.0f;
}

bool FMatrix4::IsIdentity() const
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (i == j)
			{
				if (data[i][j] != 1.0f) return false;
			}
			else
			{
				if (data[i][j] != 0.0f) return false;
			}
		}
	}
	return true;
}

void FMatrix4::AxisAngle(const FPoint3 &vec, double theta)
{
	double cost = cos(theta), sint = sin(theta);

	double a2, b2, c2, abm, acm, bcm;
	double mcos, asin, bsin, csin;
	mcos = 1.0f - cost;
	a2 = vec.x * vec.x;
	b2 = vec.y * vec.y;
	c2 = vec.z * vec.z;
	abm = vec.x * vec.y * mcos;
	acm = vec.x * vec.z * mcos;
	bcm = vec.y * vec.z * mcos;
	asin = vec.x * sint;
	bsin = vec.y * sint;
	csin = vec.z * sint;

	data[0][0] = (float) (a2 * mcos + cost);
	data[0][1] = (float) (abm - csin);
	data[0][2] = (float) (acm + bsin);
	data[0][3] = 0;

	data[1][0] = (float) (abm + csin);
	data[1][1] = (float) (b2 * mcos + cost);
	data[1][2] = (float) (bcm - asin);
	data[1][3] = 0;

	data[2][0] = (float) (acm - bsin);
	data[2][1] = (float) (bcm + asin);
	data[2][2] = (float) (c2 * mcos + cost);
	data[2][3] = 0;

	data[3][0] = 0;
	data[3][1] = 0;
	data[3][2] = 0;
	data[3][3] = 1;
}

void FMatrix4::SetFromVectors(const FPoint3 &pos, const FPoint3 &forward, const FPoint3 &up)
{
	FPoint3 f = forward;
	f.Normalize();
	FPoint3 side = f.Cross(up);
	side.Normalize();

	SetRow(0,	side.x,		side.y,		side.z,		0.0f);
	SetRow(1,	up.x,		up.y,		up.z,		0.0f);
	SetRow(2,	-forward.x,	-forward.y,	-forward.z,	0.0f);
	SetRow(3,	0.0f,		0.0f,		0.0f,		1.0f);

	Translate(pos);
}

void FMatrix4::SetFromMatrix3(const FMatrix3 &mat)
{
	SetRow(0, mat(0,0), mat(1,0), mat(2,0), 0);
	SetRow(1, mat(0,1), mat(1,1), mat(2,1), 0);
	SetRow(2, mat(0,2), mat(1,2), mat(2,2), 0);
	SetRow(3, 0, 0, 0, 1);
}

void FMatrix4::MakeScale(float x, float y, float z)
{
	SetRow(0, x, 0, 0, 0 );
	SetRow(1, 0, y, 0, 0 );
	SetRow(2, 0, 0, z, 0 );
	SetRow(3, 0, 0, 0, 1 );
}

#define INNER_PRODUCT(a, b, c, r) \
	 ((a).data[0][r] * (b).data[c][0]) \
	+((a).data[1][r] * (b).data[c][1]) \
	+((a).data[2][r] * (b).data[c][2]) \
	+((a).data[3][r] * (b).data[c][3])

void FMatrix4::PreMult(const FMatrix4 &mat)
{
	float t[4];
	for (int col = 0; col < 4; ++col)
	{
		t[0] = INNER_PRODUCT(mat, *this, col, 0);
		t[1] = INNER_PRODUCT(mat, *this, col, 1);
		t[2] = INNER_PRODUCT(mat, *this, col, 2);
		t[3] = INNER_PRODUCT(mat, *this, col, 3);
		data[col][0] = t[0];
		data[col][1] = t[1];
		data[col][2] = t[2];
		data[col][3] = t[3];
	}
}

void FMatrix4::PostMult(const FMatrix4 &mat)
{
	float t[4];
	for (int row = 0; row < 4; ++row)
	{
		t[0] = INNER_PRODUCT(*this, mat, 0, row);
		t[1] = INNER_PRODUCT(*this, mat, 1, row);
		t[2] = INNER_PRODUCT(*this, mat, 2, row);
		t[3] = INNER_PRODUCT(*this, mat, 3, row);
		SetRow(row, t[0], t[1], t[2], t[3]);
	}
}

static void Full_Inverse_Xform3(const float b[4][4], float a[4][4])
{
	long indxc[4], indxr[4], ipiv[4];
	long i, icol=0, irow=0, j, ir, ic;	// set to 0 to avoid compiler warning
	float big, dum, pivinv, temp, bb;
	ipiv[0] = -1;
	ipiv[1] = -1;
	ipiv[2] = -1;
	ipiv[3] = -1;
	a[0][0] = b[0][0];
	a[1][0] = b[1][0];
	a[2][0] = b[2][0];
	a[3][0] = b[3][0];
	a[0][1] = b[0][1];
	a[1][1] = b[1][1];
	a[2][1] = b[2][1];
	a[3][1] = b[3][1];
	a[0][2] = b[0][2];
	a[1][2] = b[1][2];
	a[2][2] = b[2][2];
	a[3][2] = b[3][2];
	a[0][3] = b[0][3];
	a[1][3] = b[1][3];
	a[2][3] = b[2][3];
	a[3][3] = b[3][3];
	for (i = 0; i < 4; i++) {
		big = 0.0f;
		for (j = 0; j < 4; j++) {
			if (ipiv[j] != 0) {
				if (ipiv[0] == -1) {
					if ((bb = (float) fabs(a[j][0])) > big) {
						big = bb;
						irow = j;
						icol = 0;
					}
				} else if (ipiv[0] > 0) {
					return;
				}
				if (ipiv[1] == -1) {
					if ((bb = (float) fabs((float) a[j][1])) > big) {
						big = bb;
						irow = j;
						icol = 1;
					}
				} else if (ipiv[1] > 0) {
					return;
				}
				if (ipiv[2] == -1) {
					if ((bb = (float) fabs((float) a[j][2])) > big) {
						big = bb;
						irow = j;
						icol = 2;
					}
				} else if (ipiv[2] > 0) {
					return;
				}
				if (ipiv[3] == -1) {
					if ((bb = (float) fabs((float) a[j][3])) > big) {
						big = bb;
						irow = j;
						icol = 3;
					}
				} else if (ipiv[3] > 0) {
					return;
				}
			}
		}
		++(ipiv[icol]);
		if (irow != icol) {
			temp = a[irow][0];
			a[irow][0] = a[icol][0];
			a[icol][0] = temp;
			temp = a[irow][1];
			a[irow][1] = a[icol][1];
			a[icol][1] = temp;
			temp = a[irow][2];
			a[irow][2] = a[icol][2];
			a[icol][2] = temp;
			temp = a[irow][3];
			a[irow][3] = a[icol][3];
			a[icol][3] = temp;
		}
		indxr[i] = irow;
		indxc[i] = icol;
		if (a[icol][icol] == 0.0) {
			return;
		}
		pivinv = 1.0f / a[icol][icol];
		a[icol][icol] = 1.0f;
		a[icol][0] *= pivinv;
		a[icol][1] *= pivinv;
		a[icol][2] *= pivinv;
		a[icol][3] *= pivinv;
		if (icol != 0) {
			dum = a[0][icol];
			a[0][icol] = 0.0f;
			a[0][0] -= a[icol][0] * dum;
			a[0][1] -= a[icol][1] * dum;
			a[0][2] -= a[icol][2] * dum;
			a[0][3] -= a[icol][3] * dum;
		}
		if (icol != 1) {
			dum = a[1][icol];
			a[1][icol] = 0.0f;
			a[1][0] -= a[icol][0] * dum;
			a[1][1] -= a[icol][1] * dum;
			a[1][2] -= a[icol][2] * dum;
			a[1][3] -= a[icol][3] * dum;
		}
		if (icol != 2) {
			dum = a[2][icol];
			a[2][icol] = 0.0f;
			a[2][0] -= a[icol][0] * dum;
			a[2][1] -= a[icol][1] * dum;
			a[2][2] -= a[icol][2] * dum;
			a[2][3] -= a[icol][3] * dum;
		}
		if (icol != 3) {
			dum = a[3][icol];
			a[3][icol] = 0.0f;
			a[3][0] -= a[icol][0] * dum;
			a[3][1] -= a[icol][1] * dum;
			a[3][2] -= a[icol][2] * dum;
			a[3][3] -= a[icol][3] * dum;
		}
	}
	if (indxr[3] != indxc[3]) {
		ir = indxr[3];
		ic = indxc[3];
		temp = a[0][ir];
		a[0][ir] = a[0][ic];
		a[0][ic] = temp;
		temp = a[1][ir];
		a[1][ir] = a[1][ic];
		a[1][ic] = temp;
		temp = a[2][ir];
		a[2][ir] = a[2][ic];
		a[2][ic] = temp;
		temp = a[3][ir];
		a[3][ir] = a[3][ic];
		a[3][ic] = temp;
	}
	if (indxr[2] != indxc[2]) {
		ir = indxr[2];
		ic = indxc[2];
		temp = a[0][ir];
		a[0][ir] = a[0][ic];
		a[0][ic] = temp;
		temp = a[1][ir];
		a[1][ir] = a[1][ic];
		a[1][ic] = temp;
		temp = a[2][ir];
		a[2][ir] = a[2][ic];
		a[2][ic] = temp;
		temp = a[3][ir];
		a[3][ir] = a[3][ic];
		a[3][ic] = temp;
	}
	if (indxr[1] != indxc[1]) {
		ir = indxr[1];
		ic = indxc[1];
		temp = a[0][ir];
		a[0][ir] = a[0][ic];
		a[0][ic] = temp;
		temp = a[1][ir];
		a[1][ir] = a[1][ic];
		a[1][ic] = temp;
		temp = a[2][ir];
		a[2][ir] = a[2][ic];
		a[2][ic] = temp;
		temp = a[3][ir];
		a[3][ir] = a[3][ic];
		a[3][ic] = temp;
	}
	if (indxr[0] != indxc[0]) {
		ir = indxr[0];
		ic = indxc[0];
		temp = a[0][ir];
		a[0][ir] = a[0][ic];
		a[0][ic] = temp;
		temp = a[1][ir];
		a[1][ir] = a[1][ic];
		a[1][ic] = temp;
		temp = a[2][ir];
		a[2][ir] = a[2][ic];
		a[2][ic] = temp;
		temp = a[3][ir];
		a[3][ir] = a[3][ic];
		a[3][ic] = temp;
	}
}

static void Down_triangle_Inverse_Xform(const float src[4][4], float dst[4][4])
{
	float det1, o33;
	float a1, a2, a3, b1, b2, b3, c1, c2, c3;
	a1 = src[0][0];
	b1 = src[0][1];
	c1 = src[0][2];
	a2 = src[1][0];
	b2 = src[1][1];
	c2 = src[1][2];
	a3 = src[2][0];
	b3 = src[2][1];
	c3 = src[2][2];
	det1 = det3x3(a1, a2, a3, b1, b2, b3, c1, c2, c3);
	if (fabs(det1) < SMALL_NUMBER) {
		return;
	} else {
		det1 = 1.0f / det1;
		o33 = 1.0f / src[3][3];
		dst[0][0] =  (b2 * c3 - c2 * b3) * det1;
		dst[1][0] = -(a2 * c3 - c2 * a3) * det1;
		dst[2][0] =  (a2 * b3 - b2 * a3) * det1;
		dst[0][1] = -(b1 * c3 - c1 * b3) * det1;
		dst[1][1] =  (a1 * c3 - c1 * a3) * det1;
		dst[2][1] = -(a1 * b3 - b1 * a3) * det1;
		dst[0][2] =  (b1 * c2 - c1 * b2) * det1;
		dst[1][2] = -(a1 * c2 - c1 * a2) * det1;
		dst[2][2] =  (a1 * b2 - b1 * a2) * det1;
		dst[3][0] = -(src[3][0] * dst[0][0] + src[3][1] * dst[1][0] + src[3][2] * dst[2][0]) * o33;
		dst[3][1] = -(src[3][0] * dst[0][1] + src[3][1] * dst[1][1] + src[3][2] * dst[2][1]) * o33;
		dst[3][2] = -(src[3][0] * dst[0][2] + src[3][1] * dst[1][2] + src[3][2] * dst[2][2]) * o33;
		dst[0][3] = 0.0f;
		dst[1][3] = 0.0f;
		dst[2][3] = 0.0f;
		dst[3][3] = o33;
	}
}

static void Up_triangle_Inverse_Xform(const float src[4][4], float dst[4][4])
{
	float det1, o33;
	float a1, a2, a3, b1, b2, b3, c1, c2, c3;
	a1 = src[0][0];
	b1 = src[0][1];
	c1 = src[0][2];
	a2 = src[1][0];
	b2 = src[1][1];
	c2 = src[1][2];
	a3 = src[2][0];
	b3 = src[2][1];
	c3 = src[2][2];
	det1 = det3x3(a1, a2, a3, b1, b2, b3, c1, c2, c3);
	if (fabs(det1) < SMALL_NUMBER) {
		return;
	} else {
		det1 = (float) (1.0f / det1);
		o33 = 1.0f / src[3][3];
		dst[0][0] =  (b2 * c3 - c2 * b3) * det1;
		dst[1][0] = -(a2 * c3 - c2 * a3) * det1;
		dst[2][0] =  (a2 * b3 - b2 * a3) * det1;
		dst[0][1] = -(b1 * c3 - c1 * b3) * det1;
		dst[1][1] =  (a1 * c3 - c1 * a3) * det1;
		dst[2][1] = -(a1 * b3 - b1 * a3) * det1;
		dst[0][2] =  (b1 * c2 - c1 * b2) * det1;
		dst[1][2] = -(a1 * c2 - c1 * a2) * det1;
		dst[2][2] =  (a1 * b2 - b1 * a2) * det1;
		dst[0][3] = -(src[0][3] * dst[0][0] + src[1][3] * dst[0][1] + src[2][3] * dst[0][2]) * o33;
		dst[1][3] = -(src[0][3] * dst[1][0] + src[1][3] * dst[1][1] + src[2][3] * dst[1][2]) * o33;
		dst[2][3] = -(src[0][3] * dst[2][0] + src[1][3] * dst[2][1] + src[2][3] * dst[2][2]) * o33;
		dst[3][0] = 0.0f;
		dst[3][1] = 0.0f;
		dst[3][2] = 0.0f;
		dst[3][3] = o33;
	}
}

void FMatrix4::Invert(const FMatrix4 &src)
{
	if ((src.data[0][3] == 0) && (src.data[1][3] == 0) && (src.data[2][3] == 0)) {
		Down_triangle_Inverse_Xform(src.data, data);
		return;
	} else if ((src.data[3][0] == 0) && (src.data[3][1] == 0) && (src.data[3][2] == 0)) {
		Up_triangle_Inverse_Xform(src.data, data);
		return;
	} else {
		Full_Inverse_Xform3(src.data, data);
		return;
	}
}

void FMatrix4::Translate(const FPoint3 &vec)
{
	data[0][3] += vec.x;
	data[1][3] += vec.y;
	data[2][3] += vec.z;
}

FPoint3 FMatrix4::GetTrans() const
{
	return FPoint3(data[0][3], data[1][3], data[2][3]);
}

void FMatrix4::SetTrans(FPoint3 pos)
{
	data[0][3] = pos.x;
	data[1][3] = pos.y;
	data[2][3] = pos.z;
}

void FMatrix4::Transform(const FPoint3 &tmp, FPoint3 &dst) const
{
	dst.x = Dot3f(&tmp.x, data[0]) + data[0][3];
	dst.y = Dot3f(&tmp.x, data[1]) + data[1][3];
	dst.z = Dot3f(&tmp.x, data[2]) + data[2][3];
}

void FMatrix4::TransformVector(const FPoint3 &tmp, FPoint3 &dst) const
{
	dst.x = Dot3f(&tmp.x, data[0]);
	dst.y = Dot3f(&tmp.x, data[1]);
	dst.z = Dot3f(&tmp.x, data[2]);
}


