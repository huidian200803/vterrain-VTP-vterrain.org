//
// Name:	 ItemGroup.cpp
//
// Copyright (c) 2003-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/GeomUtil.h"	// For Create3DCursor
#include "vtdata/vtLog.h"
#include "frame.h"
#include "ItemGroup.h"


ItemGroup::ItemGroup(vtItem *pItem)
{
	m_pItem = pItem;
}

void ItemGroup::CreateNodes()
{
	m_pAxes = NULL;
	m_pRulers = NULL;
	m_pGroup = new vtGroup;
	m_pLOD = new vtLOD;
	m_pLOD->setName("LOD Container");
	m_pTop = new vtGroup;
	m_pLOD->setName("Individual Container");
	m_pTop->setName("ItemGroupTop");
	m_pTop->addChild(m_pLOD);
	m_pTop->addChild(m_pGroup);
}

void ItemGroup::AttemptToLoadModels()
{
	int i, num_models = m_pItem->NumModels();

	for (i = 0; i < num_models; i++)
	{
		vtModel *mod = m_pItem->GetModel(i);
		vtTransform *trans = GetMainFrame()->m_nodemap[mod];
		if (!trans && !mod->m_attempted_load)
		{
			// haven't tried to load it yet
			GetMainFrame()->AttemptLoad(mod);
		}
	}
}

void ItemGroup::AttachModels(osgText::Font *font)
{
	// Undo previous attachments
	m_pLOD->removeChildren(0, m_pLOD->getNumChildren());
	m_pGroup->removeChildren(0, m_pGroup->getNumChildren());

	// re-attach
	int i, num_models = m_pItem->NumModels();
	FSphere sph(FPoint3(0,0,0), 0.001f);
	FSphere largest_sph = sph;

	for (i = 0; i < num_models; i++)
	{
		vtModel *mod = m_pItem->GetModel(i);
		osg::Node *node = GetMainFrame()->m_nodemap[mod];
		if (node)
		{
			m_pGroup->addChild(node);
			m_pLOD->addChild(node);
			GetBoundSphere(node, sph);
			if (sph.radius > largest_sph.radius)
				largest_sph = sph;
		}
	}

	UpdateCrosshair(largest_sph);
	UpdateRulers(font, largest_sph);

	// Update the item's approximate extents
	FRECT ext;
	m_pGroup->GetBoundSphere(sph);
	ext.left = sph.center.x - sph.radius;
	ext.right = sph.center.x + sph.radius;
	// the XY extents of the extents have Y pointing up, whereas the
	//  world coords have Z pointing down.
	ext.top = -sph.center.z + sph.radius;
	ext.bottom = -sph.center.z - sph.radius;
	m_pItem->m_extents = ext;
}

void ItemGroup::UpdateCrosshair(const FSphere &sph)
{
	// Update origin crosshair
	if (m_pAxes)
		m_pTop->removeChild(m_pAxes);

	float size = sph.radius * 2;
	m_pAxes = Create3DCursor(size, size/100, 0.4f);
	m_pAxes->setName("Origin Axes");
	m_pTop->addChild(m_pAxes);
}

void ItemGroup::UpdateRulers(osgText::Font *font, const FSphere &sph)
{
	// Update rulers
	if (m_pRulers)
		m_pTop->removeChild(m_pRulers);

	float size = sph.radius * 2;
	m_pRulers = CreateRulers(font, size);
	m_pRulers->setName("Rulers");
	m_pTop->addChild(m_pRulers);
}

void ItemGroup::ShowOrigin(bool bShow)
{
	m_pAxes->SetEnabled(bShow);
}

void ItemGroup::ShowRulers(bool bShow)
{
	m_pRulers->SetEnabled(bShow);
}

void ItemGroup::SetRanges()
{
	// Set the LOD ranges for each model
	int num_models = m_pItem->NumModels();

	float start = 0.0f, end;
	for (int i = 0; i < num_models; i++)
	{
		end = m_pItem->GetModel(i)->m_distance;

		// if we don't have a valid range for a model, just make it visible
		if (end == 0)	
			end = 1e9;

		m_pLOD->setRange(i, start, end);
		start = end;
	}
}

void ItemGroup::ShowLOD(bool bTrue)
{
	SetEnabled(m_pLOD, bTrue);
	m_pGroup->SetEnabled(!bTrue);
	if (bTrue)
	{
		// LOD requires all models to be enabled
		int i, num_models = m_pItem->NumModels();
		for (i = 0; i < num_models; i++)
		{
			vtModel *mod = m_pItem->GetModel(i);
			vtTransform *trans = GetMainFrame()->m_nodemap[mod];
			if (trans)
				trans->SetEnabled(true);
		}
	}
	else
	{
		// Group requires all models to be (initially) disabled
		int i, num_models = m_pItem->NumModels();
		for (i = 0; i < num_models; i++)
		{
			vtModel *mod = m_pItem->GetModel(i);
			vtTransform *trans = GetMainFrame()->m_nodemap[mod];
			if (trans)
				trans->SetEnabled(false);
		}
	}
}


///////////////////////////////////////////////////////////////////////
// Ruler geometry

vtGeode *CreateRulers(osgText::Font *font, float fSize)
{
	int i, j;

	vtGeode *pGeode = new vtGeode;
	vtMaterialArrayPtr pMats = new vtMaterialArray;
	pMats->AddRGBMaterial(RGBf(1.0f, 1.0f, 1.0f), false, false, false);
	pGeode->SetMaterials(pMats);

	int up = 0;
	float interval = 0.001f;
	while (fSize / interval > 22)
	{
		if (up == 0)
			interval *= 2;
		if (up == 1)
			interval *= 2.5;
		if (up == 1)
			interval *= 2;
		up++;
		if (up > 2) up = 0;
	}
	int ticks = int( fSize / interval );
	FPoint3 p;
	float *wide;
	float *thin;

	// First create the three axes (XYZ) with tick marks
	for (i = 0; i < 3; i++)
	{
		p.Set(0,0,0);

		vtMesh *mesh = new vtMesh(osg::PrimitiveSet::LINES, 0, 24);

		if (i == 0) { wide = &p.x; thin = &p.z; }
		if (i == 1) { wide = &p.y; thin = &p.x; }
		if (i == 2) { wide = &p.z; thin = &p.x; }

		*wide = -fSize;
		mesh->AddVertex(p);
		*wide =  fSize;
		mesh->AddVertex(p);
		mesh->AddLine(0, 1);

		for (j = 1; j <= ticks; j++)
		{
			*wide = j * interval;
			*thin = -interval/2;
			int start = mesh->AddVertex(p);
			*thin =  interval/2;
			mesh->AddVertex(p);
			mesh->AddLine(start, start+1);
		}
		pGeode->AddMesh(mesh, 0);
	}

	// then the text labels on each tick
	vtString str;
	for (i = 0; i < 3; i++)
	{
		p.Set(0,0,0);

		if (i == 0) { wide = &p.x; thin = &p.z; }
		if (i == 1) { wide = &p.y; thin = &p.x; }
		if (i == 2) { wide = &p.z; thin = &p.x; }

		for (j = 1; j <= ticks; j++)
		{
			*wide = j * interval;
			*thin = interval/2;

			if (font)
			{
				str.Format("%g", j * interval);
				vtTextMesh *text = new vtTextMesh(font, interval/2, false);
				text->SetPosition(p);
				text->SetText(str);
				pGeode->AddTextMesh(text, 0);
			}
		}
	}
	return pGeode;
}

