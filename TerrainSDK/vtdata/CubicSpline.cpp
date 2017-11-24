//
// (Natural) Cubic Spline class.
//
// Originally adapted from some LGPL code found around the net.
//
// Copyright (c) 2004-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "CubicSpline.h"

CubicSpline::CubicSpline()
{
	m_pCoeff0 = m_pCoeff1 = m_pCoeff2 = m_pCoeff3 = NULL;
	m_iPoints = 0;
}

CubicSpline::~CubicSpline()
{
	Cleanup();
}

/** Re-initialization. */
void CubicSpline::Cleanup()
{
	if (m_veclist.size() > 0)
		m_veclist.erase(m_veclist.begin(), m_veclist.end());

	delete [] m_pCoeff0;
	delete [] m_pCoeff1;
	delete [] m_pCoeff2;
	delete [] m_pCoeff3;
	m_pCoeff0 = m_pCoeff1 = m_pCoeff2 = m_pCoeff3 = NULL;

	m_iPoints = 0;
}

/** Adds an interpolation point. */
int CubicSpline::AddPoint(const DPoint3 &vec)
{
	// TO DO: error check
	int npar = m_veclist.size();
	m_veclist.push_back(vec);
	return npar;
}

/** Generate the spline coeffs. */
bool CubicSpline::Generate()
{
	int i;
	m_iPoints = m_veclist.size();

	if (m_iPoints==0)
		return false; // there are no interp points!!

	// allocate coefficient table
	m_pCoeff0 = new DPoint3[m_iPoints];
	m_pCoeff1 = new DPoint3[m_iPoints];
	m_pCoeff2 = new DPoint3[m_iPoints];
	m_pCoeff3 = new DPoint3[m_iPoints];

	std::list<DPoint3>::const_iterator iter = m_veclist.begin();

	if (m_iPoints==1)
	{
		// cannot calculate spline, return constant polynom
		DPoint3 p0 = *iter;

		m_pCoeff3[0] = DPoint3(0,0,0);
		m_pCoeff2[0] = DPoint3(0,0,0);
		m_pCoeff1[0] = DPoint3(0,0,0);
		m_pCoeff0[0] = p0;
		return true;
	}
	else if (m_iPoints==2)
	{
		DPoint3 p0 = *iter;
		iter++;
		DPoint3 p1 = *iter;

		m_pCoeff3[0] = DPoint3(0,0,0);
		m_pCoeff2[0] = DPoint3(0,0,0);
		m_pCoeff1[0] = p1-p0;
		m_pCoeff0[0] = p0;
		return true;
	}

	///////////////////////////////////////////////////
	// calculate natural spline coeffs

	int intNo = m_iPoints - 1;	// number of intervals
	int equNo = intNo - 1;	// number of equations

	// interval sizes
	double *h = new double[intNo];
	double *ih = new double[intNo];

	// diagonal of tridiagonal matrix
	double *a = new double[equNo];
	// constant part of linear equations
	DPoint3 *dvec = new DPoint3[equNo];

	// LR decomposition of tridiagonal matrix
	double *m = new double[equNo];
	double *l = new double[equNo - 1];
	// ??
	DPoint3 *yvec = new DPoint3[equNo];
	DPoint3 *xvec = new DPoint3[equNo];

	DPoint3 d0, d1;

	DPoint3 *invec = new DPoint3[m_iPoints];
	for (i=0; i<m_iPoints; i++, iter++)
	{
		invec[i] = *iter;
	}

	// calculate interval sizes as distance between points
	for (i = 0; i < intNo; i++)
	{
		//h[i] = Vec3DiffAbs(invec[i], invec[i + 1]);
		h[i] = ::sqrt((invec[i]-invec[i+1]).LengthSquared());

		// identical points are not allowed, because a curve through them is
		//  not mathematically defined
		if (h[i] == 0.0)
		{
			delete [] h;
			delete [] ih;
			delete [] a;
			delete [] dvec;
			delete [] m;
			delete [] l;
			delete [] yvec;
			delete [] xvec;
			delete [] invec;
			return false;
		}

		ih[i] = 1.0 / h[i];
	}

	// calculate diagonal of tridiagonal matrix
	for (i = 0; i < equNo; i++)
		a[i] = 2.0 * (h[i] + h[i + 1]);

	// calculate LR decomposition of tridiagonal matrix
	m[0] = a[0];
	for (i = 0; i < equNo - 1; i++)
	{
		l[i] = h[i + 1] / m[i];
		m[i + 1] = a[i + 1] - l[i] * h[i + 1];
	}

	// interpolation is done separately for all 3 coordinates

	for (i = 0; i < equNo; i++)
	{
		// dvec[i] = 6.0*(ih[i]*(invec[i+1] - invec[i]) - ih[i+1]*(invec[i+2] - invec[i+1]));
		DPoint3 dif1 = invec[i+1] - invec[i];
		DPoint3 dif2 = invec[i+2] - invec[i+1];
		dvec[i] = dif1 * (ih[i]) - dif2 * (ih[i+1]);
		dvec[i] = dvec[i] * (6.0);
	}

	// forward elimination
	yvec[0] = dvec[0];
	for (i = 1; i < equNo; i++)
		yvec[i] = dvec[i] - yvec[i-1] * (l[i-1]);

	// back substitution
	xvec[equNo-1] = yvec[equNo-1] * (-1.0/m[equNo-1]);
	for (i = equNo - 2; i >= 0; i--)
	{
		xvec[i] = yvec[i] + xvec[i+1] * (h[i+1]);
		xvec[i] = xvec[i] * (-1.0/m[i]);
	}

	// calculate spline points
	for (i = 0; i < intNo; i++)
	{
		// calculate polynom coefficients
		if (i == 0)
			d0.Set(0,0,0);	// zero vector
		else
			d0 = xvec[i-1];

		if (i == intNo-1)
			d1.Set(0,0,0);	// zero vector
		else
			d1 = xvec[i];

		double hsq = h[i]*h[i];
		m_pCoeff3[i] = (d1 - d0) * (hsq/6.0);
		m_pCoeff2[i] = d0 * (0.5*hsq);
		m_pCoeff1[i] = invec[i+1] - invec[i] - (d1 + d0 * (2.0)) * (hsq/6.0);
		m_pCoeff0[i] = invec[i];
	}

	delete [] h;
	delete [] ih;
	delete [] a;
	delete [] dvec;
	delete [] m;
	delete [] l;
	delete [] yvec;
	delete [] xvec;
	delete [] invec;

	// discard original vectors (not required...)
	if (m_veclist.size()>0)
		m_veclist.erase(m_veclist.begin(), m_veclist.end());

	return true;
}

/**
 * Perform interpolation.
 *
 * \param par The interpolation parameter, in the range of 0 to N, where
 *		N is the number of segments (one less than the number of control
 *		points that define the curve).
 * \param vec The point that results from the interpolation.
 * \param dvec Tangential vector (optional).
 * \param ddvec Curvature vector (optional).
 */
bool CubicSpline::Interpolate(double par, DPoint3 *vec,
							  DPoint3 *dvec /*= NULL*/,
							  DPoint3 *ddvec /*= NULL*/) const
{
	// check parameter value f
	int ncoeff = (int)::floor(par);
	if (ncoeff<0)
		ncoeff = 0;
	if (ncoeff>=(m_iPoints-1))
		ncoeff = m_iPoints-2;

	const DPoint3 &coeff0 = m_pCoeff0[ncoeff];
	const DPoint3 &coeff1 = m_pCoeff1[ncoeff];
	const DPoint3 &coeff2 = m_pCoeff2[ncoeff];
	const DPoint3 &coeff3 = m_pCoeff3[ncoeff];

	double f = par - (double)ncoeff;

	DPoint3 tmp;
	tmp = coeff3 * (f) + coeff2;
	tmp = tmp * (f) + coeff1;
	tmp = tmp * (f) + coeff0;
	*vec = tmp;

	if (dvec != NULL)
	{
		// calculate tangential vector
		tmp = coeff3 * (3.0*f) + coeff2 * (2.0);
		tmp = tmp * (f) + coeff1;
		*dvec = tmp;
	}

	if (ddvec != NULL)
	{
		// calculate curvature vector
		tmp = coeff3 * (6.0*f) + coeff2 * (2.0);
		*ddvec = tmp;
	}
	return true;
}

