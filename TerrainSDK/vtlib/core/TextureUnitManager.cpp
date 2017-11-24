//
// TextureUnitManager.cpp
//
// Originally written by RJ.
//
// Copyright (c) 2006-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "TextureUnitManager.h"
#include "TemporaryGraphicsContext.h"

//
// RJ's warning: NONE OF THIS IS THREAD SAFE!!!
//

vtTextureUnitManager::vtTextureUnitManager(void)
{
	m_bInitialised = false;
	m_pAllocationArray = NULL;
}

vtTextureUnitManager::~vtTextureUnitManager(void)
{
	if (NULL != m_pAllocationArray)
		delete[] m_pAllocationArray;
}

void vtTextureUnitManager::Initialise()
{
	// Ensure this thread has a valid graphics context
	// before making any OpenGL calls
    vtTemporaryGraphicsContext TempContext;

	m_iNumTextureUnits = -1;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &m_iNumTextureUnits);
	if (m_iNumTextureUnits < 0)
	{
		// got a bogus value, probably because there is no OpenGL context yet.
		m_iNumTextureUnits = 4;
	}

	m_pAllocationArray = new bool[m_iNumTextureUnits];
	for (int i = 0; i < m_iNumTextureUnits; i++)
		m_pAllocationArray[i] = false;
	m_bInitialised = true;
}

int vtTextureUnitManager::ReserveTextureUnit(bool bHighest)
{
	int iUnit = -1;
	if (!m_bInitialised)
		Initialise();
	if (bHighest)
	{
		for (int i = m_iNumTextureUnits - 1; i >= 0; i--)
		{
			if (m_pAllocationArray[i] == false)
			{
				m_pAllocationArray[i] = true;
				iUnit = i;
				break;
			}
		}
	}
	else
	{
		for (int i = 0; i < m_iNumTextureUnits; i++)
		{
			if (m_pAllocationArray[i] == false)
			{
				m_pAllocationArray[i] = true;
				iUnit = i;
				break;
			}
		}
	}
	return iUnit;
}

void vtTextureUnitManager::FreeTextureUnit(int iUnit)
{
	if (!m_bInitialised || (iUnit >= m_iNumTextureUnits))
		return;
	m_pAllocationArray[iUnit] = false;
}

