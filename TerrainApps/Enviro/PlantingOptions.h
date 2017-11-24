
#ifndef __PlantingOptions_h__
#define __PlantingOptions_h__

#include "vtdata/MathTypes.h"
#include "vtdata/vtString.h"

struct PlantingOptions
{
	int		m_iMode;
	int		m_iSpecies;
	float	m_fHeight;
	int		m_iVariance;
	float	m_fSpacing;
};

struct VehicleOptions
{
	RGBi m_Color;
	vtString m_Itemname;
};

#endif // __PlantingOptions_h__
