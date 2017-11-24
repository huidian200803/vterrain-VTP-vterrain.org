//
// MapOverviewEngine.cpp
//
// Copyright (c) 2004-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "MapOverviewEngine.h"


////////////////////////////////////////////////////////////////////////
//

MapOverviewEngine::MapOverviewEngine(vtTerrain *pTerr)
{
	// boolean to avoid double mouse action clicked and released
	m_bDown = true;
	anglePrec = 0.0;
	MapRatio = 0.0;
	ratioMapTerrain = 0.0;
	MapWidth = 256;
	MapMargin = 10;
	m_pOwnedImage = NULL;

	m_pMapGroup = new vtGroup;
	m_pMapGroup->setName("MapOverview group");

	vtGetScene()->GetHUD()->GetContainer()->addChild(m_pMapGroup);
	CreateMapView(pTerr);
}

MapOverviewEngine::~MapOverviewEngine()
{
}

void MapOverviewEngine::Eval()
{
	RefreshMapView();
}

void MapOverviewEngine::OnMouse(vtMouseEvent &event)
{
	if (!m_pMapGroup->GetEnabled())
		return;

	IPoint2 position = IPoint2(event.pos.x,vtGetScene()->GetWindowSize().y - event.pos.y);
	if (event.button == VT_MIDDLE
		&& m_bDown
		&& position.x < (MapWidth + MapMargin)
		&& position.y < ((float)MapWidth / MapRatio + (float)MapMargin)
		&& position.x > MapMargin
		&& position.y > MapMargin)
	{
		vtCamera *cam = vtGetScene()->GetCamera();
		FPoint3 PreviousPosition = cam->GetTrans();

		FPoint3 NewPos((position.x - MapMargin) / ratioMapTerrain, 0,
			-(position.y - MapMargin) / ratioMapTerrain);
		cam->SetTrans(NewPos);

		// Set camera direction towards previous point
		cam->SetDirection(FPoint3((PreviousPosition.x - NewPos.x), 0,
									PreviousPosition.z - NewPos.z));
	}
	if(event.button == VT_MIDDLE)
		m_bDown = !m_bDown;
}

void MapOverviewEngine::CreateMapView(vtTerrain *pTerrain)
{
	// Create the image-sprite
	m_pMapView = new vtImageSprite;

	CreateArrow();

	// Set terrain-related aspects
	SetTerrain(pTerrain);
	m_pMapGroup->addChild(m_pMapView->GetGeode());
}

void MapOverviewEngine::SetTerrain(vtTerrain *pTerr)
{
	osg::Image *image;

	// We only support overviews for 'single' textures
	int depth;
	TextureEnum eTex = pTerr->GetParams().GetTextureEnum();
	if (eTex == TE_SINGLE || eTex == TE_DERIVED)
	{
		image = pTerr->GetTextureImage();
		if (!image)
			return;
		depth = GetDepth(image);
	}
	else
		return;		// not supported

	if (m_pMapView->GetGeode())
		m_pMapView->SetImage(image);	// already created
	else
	{
		m_pMapView->Create(image, depth == 32);
		m_pMapView->GetGeode()->setName("Map Overview Image Sprite");
	}

	FPoint2 terrainSize(pTerr->GetHeightField()->m_WorldExtents.Width(),
						pTerr->GetHeightField()->m_WorldExtents.Height());
	MapRatio = fabs(terrainSize.x / terrainSize.y);

	ratioMapTerrain = (float)MapWidth / (float)terrainSize.x;
	m_pMapView->SetPosition((float) MapMargin,
							(float) MapMargin - terrainSize.y * ratioMapTerrain,
							(float) MapMargin + MapWidth,
							(float) MapMargin);
	RefreshMapView();
}

void MapOverviewEngine::CreateArrow()
{
	// Create the "arrow"
	m_pArrow = new vtTransform;
	vtGeode * arrowGeom = new vtGeode;

	vtMaterialArray *pMats = new vtMaterialArray;
	pMats->AddRGBMaterial(RGBf(1, 0, 0), false, false); // red
	pMats->AddRGBMaterial(RGBf(0, 0, 0), false, false); // black
	arrowGeom->SetMaterials(pMats);

	int ind[7];
	vtMesh *mesh = new vtMesh(osg::PrimitiveSet::LINES, 0, 7);

	ind[0] = mesh->AddVertex( 0.0, 0.0, 0.0);
	ind[1] = mesh->AddVertex( 0.0, 5.0, 0.0);
	ind[2] = mesh->AddVertex( 0.0,-5.0, 0.0);
	ind[3] = mesh->AddVertex(-4.5,-4.0, 0.0);
	ind[6] = mesh->AddVertex(-4.5, 4.0, 0.0);
	ind[4] = mesh->AddVertex( 5.0, 7.5, 0.0);
	ind[5] = mesh->AddVertex( 5.0,-7.5, 0.0);

	mesh->AddLine(ind[1],ind[2]);
	mesh->AddLine(ind[1],ind[6]);
	mesh->AddLine(ind[2],ind[3]);
	mesh->AddLine(ind[3],ind[6]);
	mesh->AddLine(ind[1],ind[4]);
	mesh->AddLine(ind[2],ind[5]);

	m_pArrow->addChild(arrowGeom);

	// the second argument is the indice of the RGB color added into the material array
	arrowGeom->AddMesh(mesh, 1);

	mesh = new vtMesh(osg::PrimitiveSet::QUADS, 0, 4);

	ind[0] = mesh->AddVertex(-4.3, 3.8, 0.0);
	ind[1] = mesh->AddVertex(-4.3,-3.8, 0.0);
	ind[2] = mesh->AddVertex( 0.0, 4.8, 0.0);
	ind[3] = mesh->AddVertex( 0.0,-4.8, 0.0);

	mesh->AddQuad(ind[2],ind[3],ind[1],ind[0]);

	arrowGeom->AddMesh(mesh,0);

	m_pMapGroup->addChild(m_pArrow);
}

void MapOverviewEngine::RefreshMapView()
{
	// Arrow position
	FPoint3 camPos = vtGetScene()->GetCamera()->GetTrans();
	FPoint3 ArrowPos;
	ArrowPos.x = ( camPos.x) * ratioMapTerrain + MapMargin;
	ArrowPos.y = (-camPos.z) * ratioMapTerrain + MapMargin;
	ArrowPos.z = 0.0f;
	m_pArrow->SetTrans(ArrowPos);

	// Arrow orientation
	FPoint3 camDir = vtGetScene()->GetCamera()->GetDirection();
	float angle = atan2(camDir.z, camDir.x);

	if (fabs(anglePrec - angle) > 0.0001)
	{
		m_pArrow->RotateLocal(FPoint3(0,0,1), -(angle - anglePrec));
		anglePrec = angle;
	}
}

