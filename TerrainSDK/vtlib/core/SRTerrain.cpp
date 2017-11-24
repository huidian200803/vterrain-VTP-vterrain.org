//
// SRTerrain class : a subclass of vtDynTerrainGeom which encapsulates
//  Stefan Roettger's CLOD algorithm.
//
// Utilizes: Roettger's MINI library implementation
// http://stereofx.org/#Terrain
//
// Copyright (c) 2002-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "SRTerrain.h"
#include "vtdata/vtLog.h"

#include <mini/mini.h>
#include <mini/ministub.h>

using namespace mini;

#if 0
#ifdef _MSC_VER
  #if _MSC_VER == 1500	// vc9
	#pragma message( "Adding link with libMini-vc9.lib" )
	#pragma comment( lib, "libMini-vc9.lib" )
  #elif _MSC_VER == 1400 // vc8
	#pragma message( "Adding link with libMini-vc8.lib" )
	#pragma comment( lib, "libMini-vc8.lib" )
  #else					// vc71
	#pragma message( "Adding link with libMini-vc7.lib" )
	#pragma comment( lib, "libMini-vc7.lib" )
  #endif
#endif
#endif

/////////////////////////////////////////////////////////////////////////////

//
// Constructor/destructor
//
SRTerrain::SRTerrain() : vtDynTerrainGeom()
{
	m_fResolution	= 100.0f;
	m_fHResolution	= 200.0f;
	m_fLResolution	=   0.0f;
	m_pMini = NULL;
}

SRTerrain::~SRTerrain()
{
	delete m_pMini;
	m_pMini = NULL;
}

/////////////////////////////////////////////////////////////////////////////

//
// Unfortunately the following statics are required because libMini
// supports some functionality by callback, and some of the callbacks
// have no context to tell us which terrain.
//
static SRTerrain *s_pSRTerrain;
static int myfancnt;
static int s_iRows;

void beginfan_vtp()
{
	if (myfancnt++>0)
		glEnd();
	glBegin(GL_TRIANGLE_FAN);
	s_pSRTerrain->m_iDrawnTriangles-=2;	// 2 vertices are needed to start each fan
}

void fanvertex_vtp(float x, float y, float z)
{
	glVertex3f(x,y,z);
	s_pSRTerrain->m_iDrawnTriangles++;
}

short int getelevation_vtp1(int i, int j, int size, void *objref)
{
//	return ((vtElevationGrid *)objref)->GetShortValue(i, s_iRows-1-j) / 4;
	return (short) ((vtElevationGrid *)objref)->GetFValue(i, s_iRows-1-j);
}

float getelevation_vtp2(int i, int j, int size, void *objref)
{
	return ((vtElevationGrid *)objref)->GetFValue(i, s_iRows-1-j);
}

//
// Initialize the terrain data
// fZScale converts from height values (meters) to world coordinates
//
DTErr SRTerrain::Init(const vtElevationGrid *pGrid, float fZScale)
{
	// Initializes necessary field of the parent class
	DTErr err = BasicInit(pGrid);
	if (err != DTErr_OK)
		return err;

	if (m_iSize.x != m_iSize.y)
		return DTErr_NOTSQUARE;

	// compute n (log2 of grid size)
	// ensure that the grid is size (1 << n) + 1
	const int n = vt_log2(m_iSize.x - 1);
	const int required_size = (1<<n) + 1;
	if (m_iSize.x != required_size || m_iSize.y != required_size)
		return DTErr_NOTPOWER2;

	int size = m_iSize.x;
	float dim = m_fStep.x;
	const float cellaspect = m_fStep.y / m_fStep.x;

	s_iRows = m_iSize.y;

	// This maximum scale is a reasonable tradeoff between the exaggeration
	//  that the user is likely to need, and numerical precision issues.
	m_fMaximumScale = 10;

	m_fHeightScale = fZScale;
	m_fDrawScale = m_fHeightScale / m_fMaximumScale;

	void *objref = (void *) pGrid;
	if (pGrid->IsFloatMode())
	{
		m_bFloat = true;
		float *image = NULL;
		m_pMini = new ministub(image,
				&size, &dim, m_fMaximumScale, cellaspect,
				0.0f, 0.0f, 0.0f,	// grid center
				beginfan_vtp, fanvertex_vtp, NULL,
				getelevation_vtp2,
				objref);
	}
	else
	{
		m_bFloat = false;
		short *image = NULL;
		m_pMini = new ministub(image,
				&size, &dim, m_fMaximumScale, cellaspect,
				0.0f, 0.0f, 0.0f,	// grid center
				beginfan_vtp, fanvertex_vtp, NULL,
				getelevation_vtp1,
				objref);
	}
	m_pMini->setrelscale(m_fDrawScale);

	m_iDrawnTriangles = -1;
	m_iBlockSize = m_iSize.x / 4;

	return DTErr_OK;
}

DTErr SRTerrain::ReInit(const vtElevationGrid *pGrid)
{
	int size = m_iSize.x;
	float dim = m_fStep.x;
	const float cellaspect = m_fStep.y / m_fStep.x;

	delete m_pMini;
	void *objref = (void *) pGrid;
	if (pGrid->IsFloatMode())
	{
		float *image = NULL;
		m_pMini = new ministub(image,
				&size, &dim, m_fMaximumScale, cellaspect,
				0.0f, 0.0f, 0.0f,	// grid center
				beginfan_vtp, fanvertex_vtp, NULL,
				getelevation_vtp2,
				objref);
	}
	else
	{
		short *image = NULL;
		m_pMini = new ministub(image,
				&size, &dim, m_fMaximumScale, cellaspect,
				0.0f, 0.0f, 0.0f,	// grid center
				beginfan_vtp, fanvertex_vtp, NULL,
				getelevation_vtp1,
				objref);
	}
	m_pMini->setrelscale(m_fDrawScale);

	return DTErr_OK;
}

void SRTerrain::SetVerticalExag(float fExag)
{
	m_fHeightScale = fExag;

	// safety check
	if (m_fHeightScale > m_fMaximumScale)
		m_fHeightScale = m_fMaximumScale;

	m_fDrawScale = m_fHeightScale / m_fMaximumScale;
	m_pMini->setrelscale(m_fDrawScale);
}


//
// This will be called once per frame, during the culling pass.
//
// However, libMini does not allow you to call the culling pass
// independently of the rendering pass, so we cannot do culling here.
// Instead, just store the values for later use.
//
void SRTerrain::DoCulling(const vtCamera *pCam)
{
	// Grab necessary values from the VTP Scene framework, store for later
	m_eyepos_ogl = pCam->GetTrans();
	m_window_size = vtGetScene()->GetWindowSize();
	m_fAspect = (float)m_window_size.x / m_window_size.y;
	m_fNear = pCam->GetHither();
	m_fFar = pCam->GetYon();

	// Get up vector and direction vector from camera matrix
	FMatrix4 mat;
	pCam->GetTransform(mat);
	FPoint3 up(0.0f, 1.0f, 0.0f);
	mat.TransformVector(up, eye_up);

	FPoint3 forward(0.0f, 0.0f, -1.0f);
	mat.TransformVector(forward, eye_forward);

	if (pCam->IsOrtho())
	{
		// libMini supports orthographic viewing as of libMini 5.0.
		// A negative FOV value indicates to the library that the FOV is
		//  actually the orthographic height of the camera.
		m_fFOVY = pCam->GetWidth() / m_fAspect;
		m_fFOVY = -m_fFOVY;
	}
	else
	{
		float fov = pCam->GetFOV();
		float fov_y2 = atan(tan (fov/2) / m_fAspect);
		m_fFOVY = fov_y2 * 2.0f * 180 / PIf;
	}
}

void SRTerrain::DoRender()
{
	// Prepare the render state for our OpenGL usage
	PreRender();

	// Render the triangles
	RenderSurface();

	// Clean up
	PostRender();
}

void SRTerrain::LoadSingleMaterial()
{
	// single texture for the whole terrain
	vtMaterial *pMat = GetMaterial(0);
	if (pMat)
	{
		ApplyMaterial(pMat);
		SetupTexGen(1.0f);
	}
}

void SRTerrain::RenderSurface()
{
	s_pSRTerrain = this;

	LoadSingleMaterial();

	RenderPass();

	DisableTexGen();
}

void SRTerrain::RenderPass()
{
	float ex = m_eyepos_ogl.x;
	float ey = m_eyepos_ogl.y;
	float ez = m_eyepos_ogl.z;

	const float fov = m_fFOVY;

	const float ux = eye_up.x;
	const float uy = eye_up.y;
	const float uz = eye_up.z;

	const float dx = eye_forward.x;
	const float dy = eye_forward.y;
	const float dz = eye_forward.z;

	myfancnt = 0;
	m_iDrawnTriangles = 0;

	// Convert the eye location to the unusual coordinate scheme of libMini.
	ex -= (m_iSize.x/2)*m_fStep.x;
	ez += (m_iSize.y/2)*m_fStep.y;

	m_pMini->draw(m_fResolution,
				ex, ey, ez,
				dx, dy, dz,
				ux, uy, uz,
				fov, m_fAspect,
				m_fNear, m_fFar);

	if (myfancnt>0) glEnd();

	// adaptively adjust resolution threshold up or down to attain
	// the desired polygon (vertex) count target
	int diff = m_iDrawnTriangles - m_iPolygonTarget;
	int iRange = m_iPolygonTarget / 10;		// ensure within 10%

	// If we aren't within the triangle count range adjust the input resolution
	// like a binary search
	if (diff < -iRange || diff > iRange)
	{
//		VTLOG("diff %d, ", diff);
		if (diff < -iRange)
		{
			m_fLResolution = m_fResolution;

			// if the high end isn't high enough, double it
			if (m_fLResolution + 25 >= m_fHResolution)
			{
//				VTLOG("increase HRes, ");
				m_fHResolution *= 4;
			}
		}
		else
		{
			m_fHResolution = m_fResolution;
			if (m_fLResolution + 25 >= m_fHResolution)
			{
//				VTLOG("decrease LRes, ");
				m_fLResolution /= 4;
			}
		}

		m_fResolution = m_fLResolution + (m_fHResolution - m_fLResolution) / 2;
//		VTLOG("rez: [%.1f, %.1f, %.1f] (%d/%d)\n", m_fLResolution, m_fResolution, m_fHResolution, m_iDrawnTriangles, m_iPolygonTarget);

		// keep the resolution within reasonable bounds
		if (m_fResolution < 5.0f)
			m_fResolution = 5.0f;
		if (m_fResolution > 4E7)
			m_fResolution = 4E7;
	}
}

//
// These methods are called when the framework needs to know the surface
// position of the terrain at a given grid point.  Supply the height
// value from our own data structures.
//
float SRTerrain::GetElevation(int iX, int iZ, bool bTrue) const
{
	if (iX<0 || iX>m_iSize.x-1 || iZ<0 || iZ>m_iSize.y-1)
		return 0.0f;

	const float height = m_pMini->getheight(iX, iZ);

	if (bTrue)
		// convert stored value to true value
		return height / m_fDrawScale / m_fMaximumScale;
	else
		// convert stored value to drawn value
		return height;
}

void SRTerrain::SetElevation(int iX, int iZ, float fValue, bool bTrue)
{
	if (iX<0 || iX>m_iSize.x-1 || iZ<0 || iZ>m_iSize.y-1)
		return;

	if (bTrue)
		m_pMini->setrealheight(iX, iZ, fValue * m_fDrawScale * m_fMaximumScale);
	else
		m_pMini->setrealheight(iX, iZ, fValue);
}

void SRTerrain::GetWorldLocation(int i, int j, FPoint3 &p, bool bTrue) const
{
	if (i<0 || i>m_iSize.x-1 || j<0 || j>m_iSize.y-1)
	{
		p.Set(0, INVALID_ELEVATION, 0);
		return;
	}

	float height = m_pMini->getheight(i, j);

	if (bTrue)
		// convert stored value to true value
		height = height / m_fDrawScale / m_fMaximumScale;

	p.Set(m_fXLookup[i], height, m_fZLookup[j]);
}

void SRTerrain::SetPolygonTarget(int iPolygonCount)
{
	vtDynTerrainGeom::SetPolygonTarget(iPolygonCount);
	//m_fResolution = iPolygonCount * 5;
	//m_fHResolution = m_fResolution * 1.2f;
	//m_fLResolution = m_fResolution / 1.2f;
}


