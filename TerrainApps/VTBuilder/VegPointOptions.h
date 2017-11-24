//
// Name: VegPointOptions.h
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __VegPointOptions_H__
#define __VegPointOptions_H__

/** This structure stores a description of how an imported file should be
   intepreted as vegetation point data. */
class VegPointOptions
{
public:
	bool bFixedSpecies;
	wxString strFixedSpeciesName;

	int iSpeciesFieldIndex;

	// 0 - species ID (int)
	// 1 - species name (string)
	// 2 - common name (string)
	// 3 - biotype id (int)
	// 4 - biotype name (string)
	int iInterpretSpeciesField;

	bool bHeightRandom;
	float fHeightFixed;
	int iHeightFieldIndex;
};

#endif // __VegPointOptions_H__
