//
// QuikGrid.cpp
//
// This code provides an interface to the QuikGrid LGPL library
//  (http://www.perspectiveedge.com/source.html)
//
// In particular, it solves the problem that QuikGrid can only link to
//  a single callback function for contour output.
//
// Copyright (c) 2007-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdio.h>

#include "QuikGrid.h"

#if SUPPORT_QUIKGRID

static ContourCallback s_pLineCallbackFunction = NULL;
static void *s_pContext = NULL;

//
// This globally-scoped method is found at link time by the QuikGrid
//  library, which uses it as a callback.
//
void DoLineTo( float x, float y, int drawtype )
{
	if (s_pLineCallbackFunction != NULL)
		s_pLineCallbackFunction(s_pContext, x, y, drawtype == 0);
}

void SetQuikGridCallbackFunction(ContourCallback fn, void *context)
{
	s_pLineCallbackFunction = fn;
	s_pContext = context;
}

//
// This callback function will receive points output from QuikGrid.
//
void ReceiveContourPoint1(void *context, float x, float y, bool bStart)
{
	ContourConverter *cc = (ContourConverter *) context;
	cc->Coord(x, y, bStart);
}


/////////////////////////////////////////////////////////////////////////////
// class ContourConverter

ContourConverter::ContourConverter()
{
	m_pGrid = NULL;
	m_pLS = NULL;
}

ContourConverter::~ContourConverter()
{
	delete m_pGrid;
}

bool ContourConverter::SetupGrid(vtHeightFieldGrid3d *pHGGrid)
{
	// Make a note of this terrain and its attributes
	m_pHF = pHGGrid;
	m_ext = m_pHF->GetEarthExtents();
	m_spacing = m_pHF->GetSpacing();

	// copy data from our grid to a QuikGrid object
	int nx, ny;
	m_pHF->GetDimensions(nx, ny);

	m_pGrid = new SurfaceGrid(nx,ny);
	int i, j;
	for (i = 0; i < nx; i++)
	{
		for (j = 0; j < ny; j++)
		{
			// use the true elevation, for true contours
			m_pGrid->zset(i, j, m_pHF->GetElevation(i, j, true));
		}
	}
	return true;
}

/**
 * Set up the class to create line features on a terrain.
 *
 * \param pHFGrid The heightfield you will generate the contour lines on.
 * \param fset The featureset to receive the polylines.
 * \return True if successful.
 */
bool ContourConverter::Setup(vtHeightFieldGrid3d *pHFGrid, vtFeatureSetLineString *fset)
{
	if (!pHFGrid)
		return false;

	if (!SetupGrid(pHFGrid))
		return false;

	m_pLS = fset;
	return true;
}


/**
 * Generate a contour line to be draped on the terrain.
 *
 * \param fAlt The altitude (elevation) of the line to be generated.
 */
void ContourConverter::GenerateContour(float fAlt)
{
	SetQuikGridCallbackFunction(ReceiveContourPoint1, this);
	m_fAltitude = fAlt;
	Contour(*m_pGrid, fAlt);
}

/**
 * Generate a set of contour lines to be draped on the terrain.
 *
 * \param fInterval  The vertical spacing between the contours.  For example,
 *		if the elevation range of your data is from 50 to 350 meters, then
 *		an fIterval of 100 will place contour bands at 100,200,300 meters.
 */
void ContourConverter::GenerateContours(float fInterval)
{
	float fMin, fMax;
	vtHeightField *hf = m_pHF;

	hf->GetHeightExtents(fMin, fMax);
	int start = (int) (fMin / fInterval) + 1;
	int stop = (int) (fMax / fInterval);

	SetQuikGridCallbackFunction(ReceiveContourPoint1, this);
	for (int i = start; i <= stop; i++)
	{
		m_fAltitude = i * fInterval;
		Contour(*m_pGrid, i * fInterval);
	}
}

void ContourConverter::Coord(float x, float y, bool bStart)
{
	if (bStart)
		Flush();

	DPoint2 p2;
	p2.x = m_ext.left + x * m_spacing.x;
	p2.y = m_ext.bottom + y * m_spacing.y;
	m_line.Append(p2);
}

/**
 * Finishes the contour generation process.  Call once when you are done
 * using the class to generate contours.
 */
void ContourConverter::Finish()
{
	Flush();
}

void ContourConverter::Flush()
{
	// we may have some degenerate geometry; we need at least three points
	if (m_line.GetSize() > 2)
	{
		// confirm they are not all the same
		DPoint2 p2 = m_line[0];
		bool same = true;
		for (uint i = 1; i < m_line.GetSize(); i++)
		{
			if (m_line[i] != p2)
			{
				same = false;
				break;
			}
		}
		if (!same)
		{
			int record = m_pLS->AddPolyLine(m_line);
			m_pLS->SetValue(record, 0, m_fAltitude);
		}
	}
	m_line.Clear();
}

#endif // SUPPORT_QUIKGRID
