//
// TextureUnitManager.h
//
// Originally written by RJ.
//
// Copyright (c) 2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TEXTUREUNITMANAGERH
#define TEXTUREUNITMANAGERH

class vtTextureUnitManager
{
public:
	vtTextureUnitManager(void);
	~vtTextureUnitManager(void);
	int ReserveTextureUnit(bool bHighest = false);
	void FreeTextureUnit(int iUnit);

private:
	void Initialise();
	int m_iNumTextureUnits;
	bool m_bInitialised;
	bool *m_pAllocationArray;
};

#endif // TEXTUREUNITMANAGERH

