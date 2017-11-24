//
// BruteTerrain class : a generic, brute-force implementation of vtDynTerrainGeom
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "BruteTerrain.h"

//
// Macro used to determine the index of a vertex (element of the height
// field) given it's (x,y) location in the grid
//
#define offset(a, b)  ((b) * m_iSize.x + (a))

//
// Macros used to generate vertex locations from a heightfield index
// (You don't have to use them, especially for height)
//
#define MAKE_XYZ1(x,y) m_fXLookup[(x)], m_pData[offset((x),(y))]*m_fZScale, m_fZLookup[(y)]
#define MAKE_XYZ1_TRUE(x,y) m_fXLookup[(x)], m_pData[offset((x),(y))], m_fZLookup[(y)]
#define MAKE_XYZ2(x,y) (float)(x), m_pData[offset((x),(y))], (float)(y)

#ifdef USE_OSGEARTH
namespace VTP {}
using namespace VTP;
#endif


//
// Constructor/destructor
//
BruteTerrain::BruteTerrain() : vtDynTerrainGeom()
{
	m_pData = NULL;
}

BruteTerrain::~BruteTerrain()
{
	delete m_pData;
}


//
// Initialize the terrain data
// fZScale converts from height values (meters) to world coordinates
//
DTErr BruteTerrain::Init(const vtElevationGrid *pGrid, float fZScale)
{
	// Initializes necessary field of the parent class
	DTErr err = BasicInit(pGrid);
	if (err != DTErr_OK)
		return err;

	//
	// allocate array, copy data from supplied elevation grid
	//
	// (replace this with your own storage representation)
	//
	m_pData = new float[m_iSize.x * m_iSize.y];
	int i, j;
	for (i = 0; i < m_iSize.x; i++)
	{
		for (j = 0; j < m_iSize.y; j++)
			m_pData[offset(i,j)] = pGrid->GetFValue(i, j);
	}

	m_fZScale = fZScale;
	m_iDrawnTriangles = -1;

	return DTErr_OK;
}


//
// This will be called once per frame, during the culling pass.
//
void BruteTerrain::DoCulling(const vtCamera *pCam)
{
	// Do your visibility testing here.
	// (Compute which detail will actually gets drawn)
	// There are handy IsVisible methods on the parent class.
}


void BruteTerrain::DoRender()
{
	// Prepare the render state for our OpenGL usage
	PreRender();

	// Render the triangles
	RenderSurface();

	// Clean up
	PostRender();
}


void BruteTerrain::LoadSingleMaterial()
{
	// single texture for the whole terrain
	vtMaterial *pMat = GetMaterial(0);
	if (pMat)
	{
		ApplyMaterial(pMat);
		SetupTexGen(1.0f);
	}
}


void BruteTerrain::RenderSurface()
{
	LoadSingleMaterial();
	RenderPass();
}


void BruteTerrain::RenderPass()
{
	//
	// Very naive code which draws the grid as immediate-mode
	// triangle strips.  (Replace with your own algorithm.)
	//
	int i, j;
	for (i = 0; i < m_iSize.x-2; i++)
	{
		glBegin(GL_TRIANGLE_STRIP);
		for (j = 0; j < m_iSize.y; j++)
		{
			glVertex3f(MAKE_XYZ2(i, j));
			glVertex3f(MAKE_XYZ2(i+2, j));
			m_iDrawnTriangles += 2;
		}
		glEnd();
	}
}

//
// These methods are called when the framework needs to know the surface
// position of the terrain at a given grid point.  Supply the height
// value from your own data structures.
//
float BruteTerrain::GetElevation(int iX, int iZ, bool bTrue) const
{
	float f = m_pData[offset(iX,iZ)];
	if (bTrue)
		f /= m_fZScale;
	return f;
}

void BruteTerrain::GetWorldLocation(int iX, int iZ, FPoint3 &p, bool bTrue) const
{
	if (bTrue)
		p.Set(MAKE_XYZ1_TRUE(iX, iZ));
	else
		p.Set(MAKE_XYZ1(iX, iZ));
}


