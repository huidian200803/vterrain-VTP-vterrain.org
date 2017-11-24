//
// Name: VegGenOptions.h
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __VegGenOptions_H__
#define __VegGenOptions_H__

class vtVegLayer;

/** This structure stores a description of how vegetation should be generated. */
class VegGenOptions
{
public:
	VegGenOptions()
	{
		// defaults
		m_fSampling = 40.0f;
		m_fScarcity = 0.02f;
		m_iSingleSpecies = -1;
		m_iSingleBiotype = -1;
		m_pBiotypeLayer = NULL;
		m_fFixedDensity = 0.1f;
		m_pDensityLayer = NULL;
		m_fFixedSize = 5.0f;
		m_fRandomFrom = 0.0f;
		m_fRandomTo = 1.0f;
	}

	// sampling
	float m_fSampling, m_fScarcity;

	// species
	int m_iSingleSpecies;
	int m_iSingleBiotype;
	vtVegLayer *m_pBiotypeLayer;

	// density
	float m_fFixedDensity;
	vtVegLayer *m_pDensityLayer;

	// size
	float m_fFixedSize;
	float m_fRandomFrom, m_fRandomTo;
};

#endif // __VegGenOptions_H__

