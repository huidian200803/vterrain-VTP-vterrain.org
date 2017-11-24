//
// ElevDrawOptions.cpp
//
// Copyright (c) 2003-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "ElevDrawOptions.h"

ElevDrawOptions::ElevDrawOptions()
{
	m_bShowElevation = true;
	m_bShadingQuick = true;
	m_bShadingDot = false;
	m_bCastShadows = false;
	m_iCastAngle = 30;
	m_iCastDirection = 90;
	m_fAmbient = 0.1f;
	m_fGamma = 0.8f;
	m_strColorMapFile = "VTBuilder.cmt";
}

bool ElevDrawOptions::operator != (const ElevDrawOptions &val)
{
	return (m_bShowElevation != val.m_bShowElevation ||
		m_bShadingQuick != val.m_bShadingQuick ||
		m_bShadingDot != val.m_bShadingDot ||
		m_bCastShadows != val.m_bCastShadows ||
		m_iCastAngle != val.m_iCastAngle ||
		m_iCastDirection != val.m_iCastDirection ||
		m_fAmbient != val.m_fAmbient ||
		m_fGamma != val.m_fGamma ||
		m_strColorMapFile != val.m_strColorMapFile);
}

void ElevDrawOptions::SetFromTags(const vtTagArray &tags)
{
	m_bShowElevation = tags.GetValueBool(TAG_SHOW_ELEVATION);
	m_bShadingQuick = tags.GetValueBool(TAG_SHADING_QUICK);
	m_bShadingDot = tags.GetValueBool(TAG_SHADING_DOT);
	m_bCastShadows = tags.GetValueBool(TAG_CAST_SHADOWS);
	m_iCastAngle = tags.GetValueInt(TAG_CAST_ANGLE);
	m_iCastDirection = tags.GetValueInt(TAG_CAST_DIRECTION);
	m_fAmbient = tags.GetValueFloat(TAG_AMBIENT);
	m_fGamma = tags.GetValueFloat(TAG_GAMMA);
	m_strColorMapFile = tags.GetValueString(TAG_COLOR_MAP_FILE);
}

void ElevDrawOptions::SetToTags(vtTagArray &tags)
{
	tags.SetValueBool(TAG_SHOW_ELEVATION, m_bShowElevation);
	tags.SetValueBool(TAG_SHADING_QUICK, m_bShadingQuick);
	tags.SetValueBool(TAG_SHADING_DOT, m_bShadingDot);
	tags.SetValueBool(TAG_CAST_SHADOWS, m_bCastShadows);
	tags.SetValueInt(TAG_CAST_ANGLE, m_iCastAngle);
	tags.SetValueInt(TAG_CAST_DIRECTION, m_iCastDirection);
	tags.SetValueFloat(TAG_AMBIENT, m_fAmbient);
	tags.SetValueFloat(TAG_GAMMA, m_fGamma);
	tags.SetValueString(TAG_COLOR_MAP_FILE, m_strColorMapFile);
}
