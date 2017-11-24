//
// UtilityMap.h
//
// High-level representation of a utility network, currently supporting
// power networks (poles and lines).
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef UTILITYH
#define UTILITYH

#include "MathTypes.h"
#include "vtCRS.h"
#include "Content.h"

/**
 * vtPole represent any kind of node in a power network, such as a
 * simple utility pole, a transmission tower, or simply a connection
 * point at a switch or transformer.
 */
struct vtPole : public vtTagArray
{
	void Offset(const DPoint2 &delta) { m_p += delta; }

	int m_id;		// More efficient than storing id in the tagarray.
	DPoint2 m_p;
};

typedef std::vector<vtPole*> vtPoleArray;

/**
 * vtLine represent any connection between two or more poles.  It does not
 * explicitly specify the numer or location of each conductor, merely
 * the high-level topology of the power network.
 */
struct vtLine : public vtTagArray
{
	void MakePolyline(DLine2 &polyline);
	uint NumPoles() const { return m_poles.size(); }
	void AddPole(vtPole *pole) { m_poles.push_back(pole); }

	int m_id;		// More efficient than storing id in the tagarray.
	vtPoleArray m_poles;
};

/**
 * vtUtilityMap represents a full high-level network of poles and lines.
 */
class vtUtilityMap
{
public:
	vtUtilityMap();
	~vtUtilityMap();

	virtual vtPole *NewPole() { return new vtPole; }
	virtual vtLine *NewLine() { return new vtLine; }

	vtPole *AddNewPole() {
		vtPole *pole = NewPole();
		m_Poles.push_back(pole);
		return pole;
	}
	vtLine *AddNewLine() {
		vtLine *line = NewLine();
		m_Lines.push_back(line);
		return line;
	}

	uint NumPoles() const { return m_Poles.size(); }
	uint NumLines() const { return m_Lines.size(); }

	void GetPoleExtents(DRECT &rect);
	bool WriteOSM(const char *pathname);
	bool ReadOSM(const char *pathname, bool progress_callback(int) = NULL);

	void SetCRS(const vtCRS &crs);
	bool TransformTo(vtCRS &crs);
	void Offset(const DPoint2 &delta);

protected:
	vtPole *ClosestPole(const DPoint2 &p);

	std::vector<vtPole *> m_Poles;
	std::vector<vtLine *> m_Lines;

	vtCRS m_crs;

	int	m_iNextAvailableID;
};

#endif // UTILITYH
