//
// ElevDrawOptions.h
//
// Copyright (c) 2003-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ElevDrawOptions_H
#define ElevDrawOptions_H

#include "vtdata/Content.h"

#define TAG_SHOW_ELEVATION	"ShowElevation"
#define TAG_SHADING_QUICK	"ShadingQuick"
#define TAG_SHADING_DOT		"ShadingDot"
#define TAG_CAST_SHADOWS	"CastShadows"
#define TAG_CAST_ANGLE		"CastAngle"
#define TAG_CAST_DIRECTION	"CastDirection"
#define TAG_AMBIENT			"Ambient"
#define TAG_GAMMA			"Gamma"
#define TAG_COLOR_MAP_FILE	"ColorMapFile"

class ElevDrawOptions
{
public:
	ElevDrawOptions();
	bool operator != (const ElevDrawOptions &val);
	void SetFromTags(const vtTagArray &tags);
	void SetToTags(vtTagArray &tags);

	bool m_bShowElevation;
	bool m_bShadingQuick;
	bool m_bShadingDot;
	bool m_bCastShadows;
	int m_iCastAngle;
	int m_iCastDirection;
	float m_fAmbient;
	float m_fGamma;
	vtString m_strColorMapFile;
};

#endif // ElevDrawOptions_H
