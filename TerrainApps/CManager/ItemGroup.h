//
// Name:		ItemGroup.h
//
// Copyright (c) 2003-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ITEMGROUPH
#define ITEMGROUPH

///////////////////////////////////////////////////////////////////////
// ItemGroup
//
class ItemGroup
{
public:
	ItemGroup(vtItem *pItem);
	void CreateNodes();
	void AttemptToLoadModels();
	void AttachModels(osgText::Font *font);
	void ShowLOD(bool bTrue);
	void SetRanges();
	vtGroup	*GetTop() { return m_pTop; }
	void ShowOrigin(bool bShow);
	void ShowRulers(bool bShow);
	void UpdateCrosshair(const FSphere &sph);
	void UpdateRulers(osgText::Font *font, const FSphere &sph);

protected:
	vtItem	*m_pItem;
	vtGroup	*m_pTop;
	vtGroup *m_pGroup;
	vtLOD	*m_pLOD;
	vtGeode	*m_pAxes;
	vtGeode	*m_pRulers;
	float m_ranges[20];		// One LOD distance for each child
};

// helper
vtGeode *CreateRulers(osgText::Font *font, float fSize);

#endif // ITEMGROUPH

