//
// PolyChecker.cpp: implementation of the PolyChecker class by Roger James.
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "PolyChecker.h"

//
// Construction/Destruction
//
PolyChecker::PolyChecker()
{
}

PolyChecker::~PolyChecker()
{
}

bool PolyChecker::Xor(bool x, bool y)
{
   return !x ^ !y;
}

bool PolyChecker::Intersect(const DPoint2 &a, const DPoint2 &b, const DPoint2 &c, const DPoint2 &d)
{
	if (IntersectProp(a, b, c, d))
		return true;
	else if (Between(a, b, c) || Between(a, b, d) ||
			 Between(c, d, a) || Between(c, d, b))
		return true;
	else
		return false;
}

bool PolyChecker::IntersectProp(const DPoint2 &a, const DPoint2 &b, const DPoint2 &c, const DPoint2 &d)
{
	if (Collinear(a, b, c) || Collinear(a, b, d) ||
		Collinear(c, d, a) || Collinear(c, d, b))
		return false;

	return Xor(Left(a, b, c), Left(a, b, d)) && Xor(Left(c, d, a), Left(c, d, b));
}

bool PolyChecker::Between(const DPoint2 &a, const DPoint2 &b, const DPoint2 &c)
{
	DPoint2	ba, ca;

	if (!Collinear( a, b, c ))
		return false;

	if (a.x != b.x)
		return ((a.x <= c.x) && (c.x <= b.x)) || ((a.x >= c.x) && (c.x >= b.x));
	else
		return ((a.y <= c.y) && (c.y <= b.y)) || ((a.y >= c.y) && (c.y >= b.y));
}

bool PolyChecker::Left(const DPoint2 &a, const DPoint2 &b, const DPoint2 &c)
{
   return AreaSign( a, b, c ) > 0;
}

bool PolyChecker::LeftOn(const DPoint2 &a, const DPoint2 &b, const DPoint2 &c)
{
   return AreaSign( a, b, c ) >= 0;
}

bool PolyChecker::Collinear(const DPoint2 &a, const DPoint2 &b, const DPoint2 &c)
{
	return AreaSign(a, b, c) == 0;
}

double PolyChecker::Area2(const DPoint2 &a, const DPoint2 &b, const DPoint2 &c)
{
	return (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
}

int PolyChecker::AreaSign(const DPoint2 &a, const DPoint2 &b, const DPoint2 &c, double dEpsilon)
{
	double area2 = (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);

	if (area2 > dEpsilon )
		return 1;
	else if (area2 < -dEpsilon )
		return -1;
	else
		return 0;
}

bool PolyChecker::IsSimplePolygon(const DLine2 &vertices)
{
	int iSize = vertices.GetSize();
	int i,j;

	for (i = 0; i < iSize - 2; i++)
		for (j = i + 2; j < (i == 0 ? iSize - 1 : iSize); j++)
			if (Intersect(vertices[i], vertices[(i + 1) % iSize],
						  vertices[j], vertices[(j+ 1) % iSize]))
				return false;
	return true;
}

bool PolyChecker::IsSimplePolygon(const DLine3 &vertices)
{
	int iSize = vertices.GetSize();
	DPoint2 v1, v2, v3, v4;
	int i,j;

	for (i = 0; i < iSize - 2; i++)
		for (j = i + 2; j < (i == 0 ? iSize - 1 : iSize); j++)
		{
			v1.x = vertices[i].x;
			v1.y = vertices[i].z;
			v2.x = vertices[(i + 1) % iSize].x;
			v2.y = vertices[(i + 1) % iSize].z;
			v3.x = vertices[j].x;
			v3.y = vertices[j].z;
			v4.x = vertices[(j+ 1) % iSize].x;
			v4.y = vertices[(j+ 1) % iSize].z;
			if (Intersect(v1, v2, v3, v4))
				return false;
		}
	return true;
}

bool PolyChecker::Intersect(const FPoint2 &a, const FPoint2 &b, const FPoint2 &c, const FPoint2 &d)
{
	if (IntersectProp(a, b, c, d))
		return true;
	else if (Between(a, b, c) || Between(a, b, d) ||
			 Between(c, d, a) || Between(c, d, b))
		return true;
	else
		return false;
}

bool PolyChecker::IntersectProp(const FPoint2 &a, const FPoint2 &b, const FPoint2 &c, const FPoint2 &d)
{
	if (Collinear(a, b, c) || Collinear(a, b, d) ||
		Collinear(c, d, a) || Collinear(c, d, b))
		return false;

	return Xor(Left(a, b, c), Left(a, b, d)) && Xor(Left(c, d, a), Left(c, d, b));
}

bool PolyChecker::Between(const FPoint2 &a, const FPoint2 &b, const FPoint2 &c)
{
	FPoint2	ba, ca;

	if (!Collinear( a, b, c ))
		return false;

	if (a.x != b.x)
		return ((a.x <= c.x) && (c.x <= b.x)) || ((a.x >= c.x) && (c.x >= b.x));
	else
		return ((a.y <= c.y) && (c.y <= b.y)) || ((a.y >= c.y) && (c.y >= b.y));
}

bool PolyChecker::Left(const FPoint2 &a, const FPoint2 &b, const FPoint2 &c)
{
   return AreaSign( a, b, c ) > 0;
}

bool PolyChecker::LeftOn(const FPoint2 &a, const FPoint2 &b, const FPoint2 &c)
{
   return AreaSign( a, b, c ) >= 0;
}

bool PolyChecker::Collinear(const FPoint2 &a, const FPoint2 &b, const FPoint2 &c)
{
	return AreaSign(a, b, c) == 0;
}

float PolyChecker::Area2(const FPoint2 &a, const FPoint2 &b, const FPoint2 &c)
{
	return (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
}

int PolyChecker::AreaSign(const FPoint2 &a, const FPoint2 &b, const FPoint2 &c, float dEpsilon)
{
	float area2 = (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);

	if (area2 > dEpsilon )
		return 1;
	else if (area2 < -dEpsilon )
		return -1;
	else
		return 0;
}

bool PolyChecker::IsSimplePolygon(const FLine2 &vertices)
{
	int iSize = vertices.GetSize();
	int i,j;

	for (i = 0; i < iSize - 2; i++)
		for (j = i + 2; j < (i == 0 ? iSize - 1 : iSize); j++)
			if (Intersect(vertices[i], vertices[(i + 1) % iSize],
						  vertices[j], vertices[(j+ 1) % iSize]))
				return false;
	return true;
}

bool PolyChecker::IsSimplePolygon(const FLine3 &vertices)
{
	int iSize = vertices.GetSize();
	FPoint2 v1, v2, v3, v4;
	int i,j;

	for (i = 0; i < iSize - 2; i++)
		for (j = i + 2; j < (i == 0 ? iSize - 1 : iSize); j++)
		{
			v1.x = vertices[i].x;
			v1.y = vertices[i].z;
			v2.x = vertices[(i + 1) % iSize].x;
			v2.y = vertices[(i + 1) % iSize].z;
			v3.x = vertices[j].x;
			v3.y = vertices[j].z;
			v4.x = vertices[(j+ 1) % iSize].x;
			v4.y = vertices[(j+ 1) % iSize].z;
			if (Intersect(v1, v2, v3, v4))
				return false;
		}
	return true;
}

bool PolyChecker::IsClockwisePolygon(const FLine2 &vertices)
{
	FPoint2 p1, p2;
	int iSize = vertices.GetSize();
	// Cannot remember if this works for all Jordan
	double Area = 0;

	for (int i = 0; i < iSize; i++)
	{
		p1 = vertices[i];
		p2 = vertices[(i+1)%iSize];

		Area += (p2.x - p1.x) * (p2.y + p1.y);
	}

	if (Area > 0)
		return true;
	else
		return false;
}

bool PolyChecker::IsClockwisePolygon(const DLine2 &vertices)
{
	DPoint2 p1, p2;
	int iSize = vertices.GetSize();
	// Cannot remember if this works for all Jordan
	double Area = 0;

	for (int i = 0; i < iSize; i++)
	{
		p1 = vertices[i];
		p2 = vertices[(i+1)%iSize];

		Area += (p2.x - p1.x) * (p2.y + p1.y);
	}

	if (Area > 0)
		return true;
	else
		return false;
}

bool PolyChecker::IsClockwisePolygon(const FLine3 &vertices)
{
	FPoint2 p1, p2;
	int iSize = vertices.GetSize();
	double Area = 0;

	for (int i = 0; i < iSize; i++)
	{
		p1.x = vertices[i].x;
		p1.y = vertices[i].z;
		p2.x = vertices[(i+1)%iSize].x;
		p2.y = vertices[(i+1)%iSize].z;

		Area += (p2.x - p1.x) * (p2.y + p1.y);
	}

	if (Area > 0)
		return true;
	else
		return false;
}
bool PolyChecker::IsClockwisePolygon(const DLine3 &vertices)
{
	DPoint2 p1, p2;
	int iSize = vertices.GetSize();
	double Area = 0;

	for (int i = 0; i < iSize; i++)
	{
		p1.x = vertices[i].x;
		p1.y = vertices[i].z;
		p2.x = vertices[(i+1)%iSize].x;
		p2.y = vertices[(i+1)%iSize].z;

		Area += (p2.x - p1.x) * (p2.y + p1.y);
	}

	if (Area > 0)
		return true;
	else
		return false;
}
