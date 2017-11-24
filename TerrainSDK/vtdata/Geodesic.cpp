//
// Geodesic Inverse: given two points, determines azimuths and distance
//
// Extracted from PROJ.4
// http://www.remotesensing.org/proj/
//
// The original author said: "The algorithm used is good to a cm or two
// at most large distances."
//

#include "vtCRS.h"

double adjlon (double lon)
{
	if (fabs(lon) <= PId)
		return( lon );
	lon += PId;  /* adjust to 0..2pi rad */
	lon -= PI2d * floor(lon / PI2d); /* remove integral # of 'revolutions'*/
	lon -= PId;  /* adjust back to -pi..pi rad */
	return( lon );
}

# define DTOL	1e-12

void Geodesic::CalculateInverse()
{
	double	th1,th2,thm,dthm,dlamm,dlam,sindlamm,costhm,sinthm,cosdthm,
		sindthm,L,E,cosd,d,X,Y,T,sind,D,A,B;

	th1 = atan(onef * tan(phi1));
	th2 = atan(onef * tan(phi2));

	thm = .5 * (th1 + th2);
	dthm = .5 * (th2 - th1);
	dlamm = .5 * ( dlam = adjlon(lam2 - lam1) );
	if (fabs(dlam) < DTOL && fabs(dthm) < DTOL)
	{
		S = 0;
		return;
	}
	sindlamm = sin(dlamm);
	costhm = cos(thm);	sinthm = sin(thm);
	cosdthm = cos(dthm);	sindthm = sin(dthm);
	L = sindthm * sindthm + (cosdthm * cosdthm - sinthm * sinthm)
		* sindlamm * sindlamm;
	d = acos(cosd = 1 - L - L);

	E = cosd + cosd;
	sind = sin( d );
	Y = sinthm * cosdthm;
	Y *= (Y + Y) / (1. - L);
	T = sindthm * costhm;
	T *= (T + T) / L;
	X = Y + T;
	Y -= T;
	T = d / sind;
	D = 4. * T * T;
	A = D * E;
	B = D + D;
	S = a * sind * (T - f4 * (T * X - Y) +
		f64 * (X * (A + (T - .5 * (A - E)) * X) -
		Y * (B + E * Y) + D * X * Y));
}
