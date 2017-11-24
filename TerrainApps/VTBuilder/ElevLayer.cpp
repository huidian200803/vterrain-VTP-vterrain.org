//
// ElevLayer.cpp
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/file.h>

#include "vtdata/config_vtdata.h"
#include "vtdata/DataPath.h"
#include "vtdata/ElevationGrid.h"
#include "vtdata/FileFilters.h"
#include "vtdata/FilePath.h"
#include "vtdata/vtDIB.h"
#include "vtdata/vtLog.h"
#include "vtui/Helper.h"	// for FormatCoord

#include "minidata/LocalDatabuf.h"

#include "Builder.h"
#include "BuilderView.h"	// For grid marks
#include "ElevLayer.h"
#include "ExtentDlg.h"
#include "ImageGLCanvas.h"
#include "Options.h"
#include "RawDlg.h"
#include "Tin2d.h"

#if SUPPORT_QUIKGRID
  #include "vtdata/QuikGrid.h"
  // QuikGrid includes
  #include "scatdata.h"
  #include "xpand.h"
#endif


////////////////////////////////////////////////////////////////////
// statics

ElevDrawOptions vtElevLayer::m_draw;
bool vtElevLayer::m_bDefaultGZip = false;
int vtElevLayer::m_iElevMemLimit = -1;


////////////////////////////////////////////////////////////////////

vtElevLayer::vtElevLayer() : vtLayer(LT_ELEVATION)
{
	SetupDefaults();
	m_pGrid = NULL;
	m_pTin = NULL;
}

vtElevLayer::vtElevLayer(const DRECT &area, const IPoint2 &size,
	bool bFloats, float fScale, const vtCRS &crs) : vtLayer(LT_ELEVATION)
{
	SetupDefaults();

	VTLOG(" Constructing vtElevLayer of size %d x %d, floats %d\n",
		size.x, size.y, bFloats);

	m_pTin = NULL;
	m_pGrid = new vtElevationGrid(area, size, bFloats, crs);
	if (!m_pGrid->HasData())
		VTLOG1(" Grid allocation failed.\n");

	m_pGrid->SetScale(fScale);
}

vtElevLayer::vtElevLayer(vtElevationGrid *grid) : vtLayer(LT_ELEVATION)
{
	SetupDefaults();

	m_pTin = NULL;
	m_pGrid = grid;
	m_pGrid->SetupLocalCS(1.0f);
}

vtElevLayer::~vtElevLayer()
{
	delete m_pGrid;
	delete m_pTin;

	// Make sure we don't have it in the cache
	ElevCacheRemove(this);
}

bool vtElevLayer::OnSave(bool progress_callback(int))
{
	vtString fname = (const char *) GetLayerFilename().mb_str(wxConvUTF8);
	if (m_pGrid)
		return m_pGrid->SaveToBT(fname, progress_callback, m_bPreferGZip);
	if (m_pTin)
		return m_pTin->Write(fname, progress_callback);
	return false;
}

bool vtElevLayer::OnLoad()
{
	bool success = false;
	vtElevError err;

	wxString fname = GetLayerFilename();
	vtString fname_utf8 = (const char *) fname.mb_str(wxConvUTF8);

	if (!fname.Right(3).CmpNoCase(_T(".bt")) ||
		!fname.Right(6).CmpNoCase(_T(".bt.gz")))
	{
		// remember whether this layer was read from a compressed file
		if (!fname.Right(6).CmpNoCase(_T(".bt.gz")))
			m_bPreferGZip = true;

		m_pGrid = new vtElevationGrid;

		success = ElevCacheOpen(this, fname_utf8, &err);
		if (success)
		{
			m_pGrid->SetupLocalCS(1.0f);
		}
		else
		{
			wxString str;
			if (err.message != "")
				str = wxString::FromUTF8((const char *) err.message);
			else
				str = wxString::Format(_("Couldn't load elevation"));
			wxMessageBox(str);
		}
	}
	else if (!fname.Right(4).CmpNoCase(_T(".tin")) ||
			 !fname.Right(4).CmpNoCase(_T(".itf")))
	{
		m_pTin = new vtTin2d;
		success = ElevCacheOpen(this, fname_utf8, &err);
	}
	if (!success && err.type == vtElevError::READ_CRS)
	{
		// Missing prj file
		wxString str = _("CRS file");
		str += _T(" (");
		RemoveFileExtensions(fname);
		str += fname;
		str += _T(".prj) ");
		str += _("is missing or unreadable.\n");
		wxMessageBox(str);
	}

	return success;
}

bool vtElevLayer::TransformCoords(vtCRS &crs_new)
{
	VTLOG("vtElevLayer::TransformCoords\n");

	vtCRS crs_old;
	GetCRS(crs_old);

	if (crs_old == crs_new)
		return true;		// No conversion necessary

	bool success = false;
	if (m_pGrid)
	{
		// Check to see if the projections differ *only* by datum
		vtCRS test = crs_old;
		test.SetDatum(crs_new.GetDatum());
		if (test == crs_new)
		{
			success = m_pGrid->ReprojectExtents(crs_new);
		}
		else
		{
			bool bUpgradeToFloat = false;

			if (!m_pGrid->IsFloatMode())
			{
				if (g_Options.GetValueBool(TAG_REPRO_TO_FLOAT_NEVER))
					bUpgradeToFloat = false;
				else if (g_Options.GetValueBool(TAG_REPRO_TO_FLOAT_ALWAYS))
					bUpgradeToFloat = true;
				else if (!IsGUIApp())
				{
					// Be sure not to ask, if this is not a GUI app
					bUpgradeToFloat = false;
				}
				else
				{
					// Ask
					int res = wxMessageBox(_("Input grid is integer.  Use floating-point values in reprojected grid?"),
						_("query"), wxYES_NO);
					if (res == wxYES)
						bUpgradeToFloat = true;
				}
			}

			// actually re-project the grid elements
			vtElevationGrid *grid_new = new vtElevationGrid;

			vtElevError err;
			success = grid_new->ConvertCRS(m_pGrid, crs_new, bUpgradeToFloat,
				progress_callback, &err);

			if (success)
			{
				delete m_pGrid;
				m_pGrid = grid_new;
				ReImage();
			}
			else
			{
				wxString msg((const char *) err.message, wxConvUTF8);
				wxMessageBox(msg, _("Error"));
				delete grid_new;
			}
		}
	}
	if (m_pTin)
	{
		success = m_pTin->ConvertCRS(crs_new);
	}
	SetModified(true);

	return success;
}

bool vtElevLayer::NeedsDraw()
{
	if (m_pGrid != NULL && m_pGrid->HasData() == false)
		return false;
	if (m_bNeedsDraw)
		return true;
	if (m_Bitmap.IsAllocated() && m_draw.m_bShowElevation && !m_bBitmapRendered)
		return true;
	return false;
}

void vtElevLayer::DrawLayer(vtScaledView *pView, UIContext &ui)
{
	if (m_pGrid)
	{
		if (m_draw.m_bShowElevation)
			DrawLayerBitmap(pView);
		else
			DrawLayerOutline(pView);
	}
	if (m_pTin)
	{
		if (m_pTin->NumTris() > 0)
			m_pTin->DrawTin(pView);
		else
			// If we have no data, just draw an outline
			DrawLayerOutline(pView);
	}
	if (ui.m_bRubber)
	{
		pView->DrawLine(mTrim1, mTrim2);
	}
}

bool vtElevLayer::GetAreaExtent(DRECT &rect)
{
	if (m_pGrid)
	{
		rect = m_pGrid->GetAreaExtents();
		return true;
	}
	else if (m_pTin)
	{
		// for a TIN, area extents are the same as normal extents
		rect = m_pTin->GetEarthExtents();
		return true;
	}
	return false;
}

int vtElevLayer::GetMemoryUsed() const
{
	if (m_pGrid)
		return m_pGrid->MemoryUsed();
	else if (m_pTin)
		return m_pTin->GetMemoryUsed();

	return 0;
}

int vtElevLayer::MemoryNeededToLoad() const
{
	if (m_pGrid)
		return m_pGrid->MemoryNeededToLoad();
	else if (m_pTin)
		return m_pTin->MemoryNeededToLoad();

	return 0;
}

void vtElevLayer::FreeData()
{
	if (m_pGrid)
		return m_pGrid->FreeData();
	else if (m_pTin)
		return m_pTin->FreeData();
}

/**
 * If an elevation layer is waiting to be paged in, then it has some basic information
 * like extents, but no data yet.
 */
bool vtElevLayer::HasData()
{
	if (m_pGrid)
		return m_pGrid->HasData();
	else if (m_pTin)
		return (m_pTin->NumTris() > 0);
	return false;
}

void vtElevLayer::OnLeftDown(BuilderView *pView, UIContext &ui)
{
	if (ui.mode == LB_TrimTIN)
	{
		mTrim1 = ui.m_DownLocation;
		mTrim2 = ui.m_CurLocation;
		ui.m_bRubber = true;
		pView->Refresh();
	}
}

void vtElevLayer::OnMouseMove(BuilderView *pView, UIContext &ui)
{
	if (ui.mode == LB_TrimTIN && ui.m_bRubber)
	{
		mTrim2 = ui.m_CurLocation;
		pView->Refresh();
	}
}

void vtElevLayer::OnLeftUp(BuilderView *pView, UIContext &ui)
{
	if (ui.mode == LB_TrimTIN && ui.m_bRubber)
	{
		ui.m_bRubber = false;

		int num = m_pTin->RemoveTrianglesBySegment(ui.m_DownLocation, ui.m_CurLocation);
		if (num)
		{
			SetModified(true);
			pView->Refresh();
		}
	}
}

bool vtElevLayer::GetExtent(DRECT &rect)
{
	if (m_pGrid)
	{
		rect = m_pGrid->GetEarthExtents();
		return true;
	}
	if (m_pTin)
	{
		rect = m_pTin->GetEarthExtents();
		return true;
	}
	return false;
}

bool vtElevLayer::AppendDataFrom(vtLayer *pL)
{
	if (pL->GetType() != LT_ELEVATION)
		return false;

	vtElevLayer *pEL = (vtElevLayer *)pL;
	if (m_pGrid)
	{
		// Deliberately unimplemented - it does not make sense to do this
		// operation with grids
	}
	if (m_pTin && pEL->m_pTin)
	{
		// In some cases, it does make sense to merge TINs
		m_pTin->AppendFrom(pEL->m_pTin);
		return true;
	}
	return false;
}


////////////////////////////////////////////////////////////////////

void vtElevLayer::SetupDefaults()
{
	m_bNeedsDraw = false;
	m_bBitmapRendered = false;
	SetLayerFilename(_("Untitled"));
	m_bPreferGZip = false;

	m_ImageSize.x = 0;
	m_ImageSize.y = 0;
	m_fSpacing = 0.0f;
}


void vtElevLayer::SetupBitmap()
{
	int cols, rows;
	m_pGrid->GetDimensions(cols, rows);
	m_ImageSize.x = cols;
	m_ImageSize.y = rows;

	int iMax = g_Options.GetValueInt(TAG_ELEV_MAX_SIZE);

	int div = 1;
	while (m_ImageSize.x * m_ImageSize.y > 4096*4096 ||
		m_ImageSize.x > iMax || m_ImageSize.y > iMax)
	{
		// bitmap is too big, chop it down
		div++;
		m_ImageSize.x = cols / div;
		m_ImageSize.y = rows / div;
	}

	if (!m_Bitmap.Allocate(m_ImageSize, 32))
	{
		DisplayAndLog(_("Couldn't create bitmap, probably too large."));
	}

	// And also the texture
	m_Texture.CreateFromBitmap(&m_Bitmap);

	m_bNeedsDraw = true;
}

void vtElevLayer::RenderBitmap()
{
	if (!m_pGrid)
		return;

	// flag as being rendered
	m_bNeedsDraw = false;

	// safety check
	if (m_ImageSize.x == 0 || m_ImageSize.y == 0)
		return;

	DetermineMeterSpacing();

	clock_t tm1 = clock();

#if 0
	// TODO: re-enable this friendly cancel behavior
	if (UpdateProgressDialog(j * 100 / m_ImageSize.y))
	{
		wxString msg = _("Turn off displayed elevation for elevation layers?");
		if (wxMessageBox(msg, _T(""), wxYES_NO) == wxYES)
		{
			m_draw.m_bShowElevation = false;
			CloseProgressDialog();
			return;
		}
		else
			ResumeProgressDialog();
	}
#endif
	ColorMap cmap;
	vtString cmap_fname = m_draw.m_strColorMapFile;
	vtString cmap_path = FindFileOnPaths(vtGetDataPath(), "GeoTypical/" + cmap_fname);
	bool bLoaded = false;
	if (cmap_path != "")
	{
		if (cmap.Load(cmap_path))
			bLoaded = true;
	}
	if (!bLoaded)
		SetupDefaultColors(cmap);

	bool has_invalid = m_pGrid->ColorDibFromElevation(&m_Bitmap, &cmap,
		8000, RGBAi(255, 0, 0, 0), progress_callback_minor);

	if (m_draw.m_bShadingQuick)
		m_pGrid->ShadeQuick(&m_Bitmap, SHADING_BIAS, true, progress_callback_minor);
	else if (m_draw.m_bShadingDot)
	{
		// Quick and simple sunlight vector
		FPoint3 light_dir = LightDirection(m_draw.m_iCastAngle, m_draw.m_iCastDirection);

		if (m_draw.m_bCastShadows)
			m_pGrid->ShadowCastDib(&m_Bitmap, light_dir, 1.0f,
				m_draw.m_fAmbient, progress_callback_minor);
		else
			m_pGrid->ShadeDibFromElevation(&m_Bitmap, light_dir, 1.0f,
				m_draw.m_fAmbient, m_draw.m_fGamma, true, progress_callback_minor);
	}

	clock_t tm2 = clock();
	float time = ((float)tm2 - tm1) / CLOCKS_PER_SEC;
	VTLOG("RenderBitmap: %.3f seconds.\n", time);

	// Copy to texture
	m_Texture.CopyImageFromBitmap(&m_Bitmap);

	m_bBitmapRendered = true;
}

void vtElevLayer::DrawLayerBitmap(vtScaledView *pView)
{
	if (!m_pGrid)
		return;

	// If we have grid data, but we don't yet have a bitmap to render it, then allocate
	if (m_pGrid->HasData() && !m_Bitmap.IsAllocated())
		SetupBitmap();

	if (!m_Bitmap.IsAllocated() || !m_bBitmapRendered)
	{
		DrawLayerOutline(pView);
		return;
	}

	m_Texture.Draw(pView, m_pGrid->GetAreaExtents());
}

void vtElevLayer::DrawLayerOutline(vtScaledView *pView)
{
	DRECT rect;
	GetExtent(rect);

	if (m_pGrid && !m_pGrid->HasData())
	{
		// draw darker, dotted lines for a grid not in memory
		glColor3f(0.0f, 0.25f, 0.0f);
	}
	else
	{
		// draw a simple crossed box with green lines
		glColor3f(0.0f, 0.5f, 0.0f);
	}
	pView->DrawRectangle(rect);
}

void vtElevLayer::SetupDefaultColors(ColorMap &cmap)
{
	cmap.m_bRelative = false;
	cmap.Add(-400*17, RGBi(255, 255, 255));
	cmap.Add(-400*16, RGBi(20, 20, 30	));
	cmap.Add(-400*15, RGBi(60, 60, 70	));
	cmap.Add(-400*14, RGBi(120, 120, 130));
	cmap.Add(-400*13, RGBi(180, 185, 190));
	cmap.Add(-400*12, RGBi(160, 80, 0	));
	cmap.Add(-400*11, RGBi(128, 128, 0	));
	cmap.Add(-400*10, RGBi(160, 0, 160	));
	cmap.Add(-400* 9, RGBi(144, 64, 144	));
	cmap.Add(-400* 8, RGBi(128, 128, 128));
	cmap.Add(-400* 7, RGBi(64, 128, 60	));
	cmap.Add(-400* 6, RGBi(0, 128, 0	));
	cmap.Add(-400* 5, RGBi(0, 128, 128	));
	cmap.Add(-400* 4, RGBi(0, 0, 160	));
	cmap.Add(-400* 3, RGBi(43, 90, 142	));
	cmap.Add(-400* 2, RGBi(81, 121, 172	));
	cmap.Add(-400* 1, RGBi(108, 156, 195));
	cmap.Add(-1.0f,  RGBi(182, 228, 255));
	cmap.Add(0, RGBi(0, 0, 0xee));
	cmap.Add( 0.1f,  RGBi(40, 224, 40	));
	cmap.Add( 450* 1, RGBi(0, 128, 0	));
	cmap.Add( 450* 2, RGBi(100, 144, 76	));
	cmap.Add( 450* 3, RGBi(204, 170, 136));
	cmap.Add( 450* 4, RGBi(136, 100, 70	));
	cmap.Add( 450* 5, RGBi(128, 128, 128));
	cmap.Add( 450* 6, RGBi(180, 128, 64	));
	cmap.Add( 450* 7, RGBi(255, 144, 32	));
	cmap.Add( 450* 8, RGBi(200, 110, 80	));
	cmap.Add( 450* 9, RGBi(160, 80, 160	));
	cmap.Add( 450*10, RGBi(144, 40, 128	));
	cmap.Add( 450*11, RGBi(128, 128, 128));
	cmap.Add( 450*12, RGBi(255, 255, 255));
	cmap.Add( 450*13, RGBi(255, 255, 128));
	cmap.Add( 450*14, RGBi(255, 128, 0));
	cmap.Add( 450*15, RGBi(0, 128, 0));
}

void vtElevLayer::ReRender()
{
	if (IsGrid())
	{
		m_bBitmapRendered = false;
		m_bNeedsDraw = true;
	}
}

void vtElevLayer::ReImage()
{
	m_bBitmapRendered = false;
}


/**
 * Determine the approximate spacing, in meters, between each grid cell, in the X
 * direction.  The result is placed in the m_fSpacing member for use in shading.
 */
void vtElevLayer::DetermineMeterSpacing()
{
	vtCRS &crs = m_pGrid->GetCRS();
	if (crs.IsGeographic())
	{
		const DRECT &area = m_pGrid->GetEarthExtents();

		const double fToMeters = EstimateDegreesToMeters((area.bottom + area.top)/2);
		m_fSpacing = (float) (area.Width()) * fToMeters / (m_pGrid->NumColumns() - 1);
	}
	else
	{
		// Linear units-based projections are much simpler
		const DPoint2 &spacing = m_pGrid->GetSpacing();
		m_fSpacing = spacing.x * GetMetersPerUnit(crs.GetUnits());
	}
}

void vtElevLayer::Offset(const DPoint2 &p)
{
	if (m_pGrid)
		m_pGrid->Offset(p);

	if (m_pTin)
		m_pTin->Offset(p);
}

vtHeightField *vtElevLayer::GetHeightField()
{
	if (m_pGrid)
		return m_pGrid;
	if (m_pTin)
		return m_pTin;
	return NULL;
}

float vtElevLayer::GetElevation(const DPoint2 &p)
{
	if (m_pGrid)
	{
		if (m_pGrid->HasData())
			return m_pGrid->GetFilteredValue(p);
	}
	if (m_pTin)
	{
		float fAltitude;
		if (m_pTin->FindAltitudeOnEarth(p, fAltitude))
			return fAltitude;
	}
	return INVALID_ELEVATION;
}

bool vtElevLayer::GetHeightExtents(float &fMinHeight, float &fMaxHeight) const
{
	if (m_pGrid)
		m_pGrid->GetHeightExtents(fMinHeight, fMaxHeight);
	else if (m_pTin)
		m_pTin->GetHeightExtents(fMinHeight, fMaxHeight);
	else
		return false;
	return true;
}

void vtElevLayer::GetCRS(vtCRS &crs)
{
	if (m_pGrid)
		crs = m_pGrid->GetCRS();
	else if (m_pTin)
		crs = m_pTin->m_crs;
}

void vtElevLayer::SetCRS(const vtCRS &crs)
{
	if (m_pGrid)
	{
		const vtCRS &current = m_pGrid->GetCRS();
		if (crs != current)
			SetModified(true);

		// if units change, meter extents of grid (and the shading which is
		//  derived from them) need to be recomputed
		LinearUnits oldunits = current.GetUnits();
		m_pGrid->SetCRS(crs);
		if (crs.GetUnits() != oldunits)
			ReRender();
	}
	if (m_pTin)
	{
		const vtCRS &current = m_pTin->m_crs;
		if (crs != current)
			SetModified(true);

		m_pTin->m_crs = crs;
	}
}

bool vtElevLayer::ImportFromFile(const wxString &strFileName,
	bool progress_callback(int), vtElevError *err)
{
	// Avoid trouble with '.' and ',' in Europe - all the file readers assume
	//  the default "C" locale.
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	wxString strExt = strFileName.AfterLast('.');
	vtString fname = (const char *) strFileName.mb_str(wxConvUTF8);

	VTLOG("ImportFromFile '%s'\n", (const char *) fname);

	if (!strExt.CmpNoCase(_T("gz")))
	{
		// ignore .gz, look at extension under it
		wxString dropped = strFileName.Left(strFileName.Len()-3);
		strExt = dropped.AfterLast('.');
	}

	if (!strExt.CmpNoCase(_T("bz2")))
	{
		// ignore .bz2, look at extension under it
		wxString dropped = strFileName.Left(strFileName.Len()-4);
		strExt = dropped.AfterLast('.');
	}

	// The first character in the file is useful for telling which format
	// the file really is.
	FILE *fp = vtFileOpen(fname, "rb");
	char first = fgetc(fp);
	fclose(fp);

	bool success = false;

	if (!strExt.CmpNoCase(_T("dxf")))
	{
		m_pTin = new vtTin2d;
		success = m_pTin->ReadDXF(fname, progress_callback);
	}
	else
	if (!strFileName.Right(6).CmpNoCase(_T("xy.adf")))
	{
		m_pTin = new vtTin2d;
		success = m_pTin->ReadADF(fname, progress_callback);
	}
	else
	if (!strFileName.Right(4).CmpNoCase(_T(".tin")))
	{
		m_pTin = new vtTin2d;
		success = m_pTin->ReadGMS(fname, progress_callback);
	}
	else
	if (!strFileName.Right(4).CmpNoCase(_T(".ply")))
	{
		m_pTin = new vtTin2d;
		success = m_pTin->ReadPLY(fname, progress_callback);
	}
	else
	{
		if (m_pGrid == NULL)
			m_pGrid = new vtElevationGrid;
	}

	if (!strExt.CmpNoCase(_T("3tx")))
	{
		success = m_pGrid->LoadFrom3TX(fname, progress_callback);
	}
	else if (!strExt.CmpNoCase(_T("dem")))
	{
		// If there is a .hdr file in the same place, it is most likely
		//  a GTOPO30/SRTM30 file
		vtString hdr_fname = ChangeFileExtension(fname, ".hdr");
		if (vtFileExists(hdr_fname))
			success = m_pGrid->LoadFromGTOPO30(hdr_fname, progress_callback);
		else
		{
			if (first == '*')
				success = m_pGrid->LoadFromMicroDEM(fname, progress_callback);
			else
				success = m_pGrid->LoadFromDEM(fname, progress_callback, err);
		}
	}
	else if (!strExt.CmpNoCase(_T("asc")))
	{
		success = m_pGrid->LoadFromASC(fname, progress_callback);
		// vtElevationGrid does have its own ASC reader, but use GDAL instead
//		success = m_pGrid->LoadWithGDAL(strFileName.mb_str(wxConvUTF8), progress_callback, err);
	}
	else if (!strExt.CmpNoCase(_T("bil")))
	{
		success = m_pGrid->LoadWithGDAL(fname, progress_callback, err);
	}
	else if (!strExt.CmpNoCase(_T("mem")))
	{
		success = m_pGrid->LoadWithGDAL(fname, progress_callback, err);
	}
	else if (!strExt.CmpNoCase(_T("ter")))
	{
		success = m_pGrid->LoadFromTerragen(fname, progress_callback);
	}
	else if (!strExt.CmpNoCase(_T("cdf")))
	{
		success = m_pGrid->LoadWithGDAL(fname, progress_callback, err);
	}
	else if (!strExt.CmpNoCase(_T("hdr")))
	{
		success = m_pGrid->LoadFromGTOPO30(fname, progress_callback);
		if (!success)
			success = m_pGrid->LoadFromGLOBE(fname, progress_callback);
	}
	else if (!strExt.CmpNoCase(_T("dte")) ||
			!strExt.CmpNoCase(_T("dt0")) ||
			!strExt.CmpNoCase(_T("dt1")) ||
			!strExt.CmpNoCase(_T("dt2")))
	{
		success = m_pGrid->LoadFromDTED(fname, progress_callback);
	}
	else if (!strExt.Left(3).CmpNoCase(_T("pgm")))
	{
		success = m_pGrid->LoadFromPGM(fname, progress_callback);
	}
	else if (!strExt.CmpNoCase(_T("grd")))
	{
		// might by CDF, might be Surfer GRD
		if (first == 'D')
		{
			VTLOG("First character is 'D', attempting load as a Surfer Grid file.\n");
			success = m_pGrid->LoadFromGRD(fname, progress_callback);
		}
		else
		{
			VTLOG("First character is not 'D', attempting load as a netCDF file.\n");
			success = m_pGrid->LoadWithGDAL(fname, progress_callback, err);
		}
		if (!success)
		{
			VTLOG("Didn't load successfully, attempting load with GDAL.\n");
			// Might be 'Arc Binary Grid', try GDAL
			success = m_pGrid->LoadWithGDAL(fname, progress_callback, err);
		}
	}
	else if (!strFileName.Right(8).CmpNoCase(_T("catd.ddf")) ||
			!strExt.Left(3).CmpNoCase(_T("tif")) ||
			!strExt.Left(3).CmpNoCase(_T("png")) ||
			!strExt.Left(3).CmpNoCase(_T("img")) ||
			!strExt.CmpNoCase(_T("adf")))
	{
		if (m_pGrid)
			success = m_pGrid->LoadWithGDAL(fname, progress_callback, err);
	}
	else if (!strExt.CmpNoCase(_T("raw")))
	{
		RawDlg dlg(NULL, -1, _("Raw Elevation File"));

		dlg.m_iBytes = 2;
		dlg.m_iWidth = 100;
		dlg.m_iHeight = 100;
		dlg.m_fVUnits = 1.0f;
		dlg.m_fSpacing = 30.0f;
		dlg.m_bBigEndian = false;
		dlg.m_extents.SetToZero();
		g_bld->GetCRS(dlg.m_original);

		if (dlg.ShowModal() == wxID_OK)
		{
			success = m_pGrid->LoadFromRAW(fname, dlg.m_iWidth,
					dlg.m_iHeight, dlg.m_iBytes, dlg.m_fVUnits, dlg.m_bBigEndian,
					progress_callback);
		}
		if (success)
		{
			m_pGrid->SetEarthExtents(dlg.m_extents);
			m_pGrid->SetCRS(dlg.m_crs);
		}
	}
	else if (!strExt.CmpNoCase(_T("ntf")))
	{
		success = m_pGrid->LoadFromNTF5(fname, progress_callback);
	}
	else if (!strExt.CmpNoCase(_T("txt")) ||
		!strExt.CmpNoCase(_T("xyz")))
	{
		success = m_pGrid->LoadFromXYZ(fname, progress_callback);
	}
	else if (!strExt.CmpNoCase(_T("hgt")))
	{
		success = m_pGrid->LoadFromHGT(fname, progress_callback);
	}
	else if (!strExt.Left(2).CmpNoCase(_T("db")))
	{
		success = ImportFromDB(fname, progress_callback);
	}
	if (!success)
		return false;

	vtCRS *pProj;
	if (m_pGrid)
		pProj = &m_pGrid->GetCRS();
	else
		pProj = &m_pTin->m_crs;

	// We should ask for a CRS before asking for extents
	if (!g_bld->ConfirmValidCRS(pProj))
	{
		if (err)
		{
			err->type = vtElevError::CANCELLED;
			err->message = "Cancelled";
		}
		return false;
	}

	if (m_pGrid != NULL)
	{
		if (m_pGrid->GetEarthExtents().IsEmpty())
		{
			// No extents.
			wxString msg = _("File lacks geographic location (extents). Would you like to specify extents?\n Yes - specify extents\n No - use some default values\n");
			int res = wxMessageBox(msg, _("Elevation Import"), wxYES_NO | wxCANCEL);
			if (res == wxYES)
			{
				DRECT ext;
				ext.SetToZero();
				ExtentDlg dlg(NULL, -1, _("Elevation Grid Extents"));
				dlg.SetArea(ext, (pProj->IsGeographic() != 0));
				if (dlg.ShowModal() == wxID_OK)
					m_pGrid->SetEarthExtents(dlg.m_area);
				else
					return false;
			}
			if (res == wxNO)
			{
				// Just make up some fake extents, assuming a regular even grid
				int xsize, ysize;
				m_pGrid->GetDimensions(xsize, ysize);

				DRECT ext;
				ext.left = ext.bottom = 0;
				if (pProj->IsGeographic())
				{
					ext.right = xsize * (1.0/3600);	// arc second
					ext.top = ysize * (1.0/3600);
				}
				else
				{
					ext.right = xsize * 10;	// 10 linear units (meters, feet..)
					ext.top = ysize * 10;
				}
				m_pGrid->SetEarthExtents(ext);
			}
			if (res == wxCANCEL)
			{
				if (err)
				{
					err->type = vtElevError::CANCELLED;
					err->message = "Cancelled";
				}
				return false;
			}
		}
		m_pGrid->SetupLocalCS(1.0f);
	}
	return true;
}

//
// Use the QuikGrid library to generate a grid from a set of 3D points.
//
bool vtElevLayer::CreateFromPoints(vtFeatureSet *set, const IPoint2 &size,
								   float fDistanceRatio)
{
#if SUPPORT_QUIKGRID
	vtFeatureSetPoint3D *fsp3 = dynamic_cast<vtFeatureSetPoint3D *>(set);
	if (!fsp3)
		return false;

	DRECT extent;
	fsp3->ComputeExtent(extent);

	int iMaxSize = fsp3->NumEntities();
	ScatData sdata(iMaxSize);
	DPoint3 p;
	for (int i = 0; i < iMaxSize; i++)
	{
		fsp3->GetPoint(i, p);
		sdata.SetNext(p.x, p.y, p.z);
	}

	// Make a SurfaceGrid to hold the results
	DPoint2 spacing(extent.Width() / (size.x-1), extent.Height() / (size.y-1));

	SurfaceGrid Zgrid(size.x, size.y);
	for (int x = 0; x < size.x; x++)
		Zgrid.xset(x, extent.left + spacing.x * x);
	for (int y = 0; y < size.y; y++)
		Zgrid.yset(y, extent.bottom + spacing.y * y);

	// "When any new points will not contributed more than 1/(scan bandwidth cutoff)
	//   towards the value of a grid intersection scanning will cease in that
	//   direction. "
	int x1 = XpandScanRatio();		// default 16, valid values 1..100

	// "The Distance cutoff specifies a percent of the Density Distance"
	int x2 = XpandDensityRatio();	// default 150, valid values 1..10000
	int x3 = XpandEdgeFactor();		// default 100, valid values 1..10000
	float x4 = XpandUndefinedZ();
	long x5 = XpandSample();

	XpandDensityRatio((int) (fDistanceRatio * 100));

	// Do the expand operation, gradually so we get progress
	XpandInit(Zgrid, sdata);
	int count = 0, total = size.x * size.y;
	while (XpandPoint( Zgrid, sdata))
	{
		if ((count % 100) == 0)
		{
			if (progress_callback(count * 99 / total))
			{
				// user cancelled
				return false;
			}
		}
		count++;
	}

	// copy the result to a ElevationGrid
	m_pGrid = new vtElevationGrid(extent, size, true, set->GetAtCRS());

	for (int x = 0; x < size.x; x++)
		for (int y = 0; y < size.y; y++)
		{
			float value = Zgrid.z(x,y);
			if (value == -99999)
				m_pGrid->SetFValue(x, y, INVALID_ELEVATION);
			else
				m_pGrid->SetFValue(x, y, value);
		}

	m_pGrid->ComputeHeightExtents();
	m_pGrid->SetupLocalCS();

	return true;
#else
	// No QuikGrid
	return false;
#endif
}

void vtElevLayer::SetGrid(vtElevationGrid *grid)
{
	delete m_pTin;
	delete m_pGrid;
	m_pTin = NULL;
	m_pGrid = grid;
	m_pGrid->SetupLocalCS(1.0f);
}

int vtElevLayer::RemoveElevRange(float zmin, float zmax, const DRECT *area)
{
	if (!m_pGrid)
		return 0;

	const DRECT &ext = m_pGrid->GetEarthExtents();
	const DPoint2 &step = m_pGrid->GetSpacing();
	const IPoint2 &Size = m_pGrid->GetDimensions();

	DPoint2 p;
	int count = 0;
	for (int i = 0; i < Size.x; i++)
	{
		for (int j = 0; j < Size.y; j++)
		{
			if (area)
			{
				p.x = ext.left + (i * step.x);
				p.y = ext.bottom + (j * step.y);
				if (!area->ContainsPoint(p))
					continue;
			}

			const float val = m_pGrid->GetFValue(i, j);
			if (val >= zmin && val <= zmax)
			{
				m_pGrid->SetFValue(i, j, INVALID_ELEVATION);
				count++;
			}
		}
	}
	return count;
}

int vtElevLayer::SetUnknown(float fValue, const DRECT *area)
{
	if (!m_pGrid)
		return 0;

	const IPoint2 &Size = m_pGrid->GetDimensions();
	const DRECT &ext = m_pGrid->GetEarthExtents();
	const DPoint2 &step = m_pGrid->GetSpacing();

	int count = 0;
	DPoint2 p;

	bool bUseArea = (area && !area->IsEmpty());

	for (int i = 0; i < Size.x; i++)
	{
		p.x = ext.left + (i * step.x);
		for (int j = 0; j < Size.y; j++)
		{
			p.y = ext.bottom + (j * step.y);
			// If an area was passed, restrict ourselves to use it
			if (bUseArea)
			{
				if (!area->ContainsPoint(p))
					continue;
			}
			if (j == 0) VTLOG(" val %f ", m_pGrid->GetFValue(i, j));
			if (m_pGrid->GetFValue(i, j) == INVALID_ELEVATION)
			{
				m_pGrid->SetFValue(i, j, fValue);
				count++;
			}
		}
	}
	return count;
}

void vtElevLayer::SetTin(vtTin2d *pTin)
{
	m_pTin = pTin;
	m_bNative = true;
}

void vtElevLayer::MergeSharedVerts(bool bSilent)
{
	if (!m_pTin)
		return;

	OpenProgressDialog(_("Merging shared vertices"), _T(""));

	int before = m_pTin->NumVerts();
	m_pTin->MergeSharedVerts(progress_callback);
	int after = m_pTin->NumVerts();

	if (after != before)
		SetModified(true);

	CloseProgressDialog();

	if (!bSilent)
	{
		if (after < before)
			DisplayAndLog((const wchar_t *) _("Reduced vertices from %d to %d"), before, after);
		else
			DisplayAndLog((const wchar_t *) _("There are %d vertices, unable to merge any."), before);
	}
}

void vtElevLayer::SetupTinTriangleBins(int target_triangles_per_bin)
{
	if (m_pTin && m_pTin->NumTris())
	{
		int tris = m_pTin->NumTris();
		// Aim for a given number of triangles in a bin
		int bins = (int) sqrt((double) tris / target_triangles_per_bin);
		if (bins < 10)
			bins = 10;
		m_pTin->SetupTriangleBins(bins, progress_callback);
	}
}

bool vtElevLayer::SetExtent(const DRECT &rect)
{
	if (m_pGrid)
	{
		m_pGrid->SetEarthExtents(rect);
		return true;
	}
	return false;
}

void vtElevLayer::GetPropertyText(wxString &strIn)
{
	wxString result = strIn, str;

	if (m_pGrid)
	{
		int cols, rows;
		m_pGrid->GetDimensions(cols, rows);
		str.Printf(_("Grid size: %d x %d\n"), cols, rows);
		result += str;

		bool bGeo = (m_pGrid->GetCRS().IsGeographic() != 0);
		result += _("Grid spacing: ");
		const DPoint2 &spacing = m_pGrid->GetSpacing();
		result += wxString(FormatCoord(bGeo, spacing.x), wxConvUTF8);
		result += _T(" x ");
		result += wxString(FormatCoord(bGeo, spacing.y), wxConvUTF8);
		result += _T("\n");

		if (m_pGrid->IsFloatMode())
			str.Printf(_("Floating point: Yes\n"));
		else
			str.Printf(_("Floating point: No\n"));
		result += str;

		if (m_pGrid->HasData())
		{
			m_pGrid->ComputeHeightExtents();
			float fMin, fMax;
			m_pGrid->GetHeightExtents(fMin, fMax);
			str.Printf(_("Minimum elevation: %.2f\n"), fMin);
			result += str;
			str.Printf(_("Maximum elevation: %.2f\n"), fMax);
			result += str;

			int num_unknown = m_pGrid->FindNumUnknown();
			if (num_unknown != 0)
			{
				str.Printf(_("Number of unknown heixels: %d (%.2f%%)\n"),
					num_unknown, num_unknown * 100.0f / (cols*rows));
				result += str;
			}
			int mem = m_pGrid->MemoryUsed();
			str.Printf(_("Size in memory: %d bytes (%.1f MB)\n"),
				mem, (float)mem / 1024 / 1024);
			result += str;
		}
		else
		{
			result += _("Not in memory.\n");
		}

		str.Printf(_("Height scale (meters per vertical unit): %f\n"), m_pGrid->GetScale());
		result += str;

		const char *dem_name = m_pGrid->GetDEMName();
		if (*dem_name)
		{
			str.Printf(_("Original DEM name: \"%hs\"\n"), dem_name);
			result += str;
		}
	}
	if (m_pTin)
	{
		int verts = m_pTin->NumVerts();
		int tris = m_pTin->NumTris();
		str.Printf(_("TIN\nVertices: %d\nTriangles: %d\n"), verts, tris);
		result += str;

		result += _("Min/max elevation: ");
		float minh, maxh;
		m_pTin->GetHeightExtents(minh, maxh);
		if (minh == INVALID_ELEVATION)
			str = _("None\n");
		else
			str.Printf(_T("%.2f, %.2f\n"), minh, maxh);
		result += str;

		int mem_bytes = m_pTin->GetMemoryUsed();
		str.Printf(_T("Size in memory: %d bytes (%0.1f Kb, %0.1f Mb)\n"),
			mem_bytes, (float)mem_bytes/1024, (float)mem_bytes/1024/1024);
		result += str;

		LinearUnits units = m_pTin->m_crs.GetUnits();
		vtString unit_name = GetLinearUnitName(units);
		str.Printf(_T("Surface area (2D): %g Square %s\n"),
			m_pTin->GetArea2D(), (const char *) unit_name);
		result += str;
		str.Printf(_T("Surface area (3D): %g Square %s\n"),
			m_pTin->GetArea3D(), (const char *) unit_name);
		result += str;

	}
	strIn = result;
}

wxString vtElevLayer::GetFileExtension()
{
	if (m_pTin)
		return _T(".itf");
	else
	{
		if (m_bPreferGZip)
			return _T(".bt.gz");
		else
			return _T(".bt");
	}
}

//
// Elevations are slightly more complicated than other layers, because there
// are two formats allowed for saving.  This gets a bit messy, especially since
// wxWidgets does not support our double extension (.bt.gz) syntax.
//
bool vtElevLayer::AskForSaveFilename()
{
	wxString filter;

	if (m_pTin)
		filter = FSTRING_TIN;
	else
		filter = FSTRING_BT _T("|") FSTRING_BTGZ;

	wxString defaultFilename = GetLayerFilename();

	// Beware: if the default filename is on a path that is no longer valid,
	//  then (at leats on Win32) the file dialog won't open.  Avoid this by
	//  checking for validity first, and strip the path if needed.
	bool valid = wxFile::Access(defaultFilename, wxFile::read);
	if (!valid)
		defaultFilename = StartOfFilenameWX(defaultFilename);

	wxFileDialog saveFile(NULL, _("Save Layer"), _T(""), defaultFilename,
		filter, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	// If user always wants to default to compressed, overwrite
	if (m_bDefaultGZip)
		m_bPreferGZip = true;
	saveFile.SetFilterIndex(m_pGrid && m_bPreferGZip ? 1 : 0);

	VTLOG("Asking user for elevation file name\n");
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return false;

	vtString fname = (const char *) saveFile.GetPath().mb_str(wxConvUTF8);
	VTLOG("Got filename: '%s'\n", (const char *) fname);

	if (m_pGrid)
	{
		m_bPreferGZip = (saveFile.GetFilterIndex() == 1);

		// work around incorrect extension(s) that wxFileDialog added
		RemoveFileExtensions(fname);
		if (m_bPreferGZip)
			fname += ".bt.gz";
		else
			fname += ".bt";
	}

	SetLayerFilename(wxString(fname, wxConvUTF8));
	m_bNative = true;
	return true;
}

// Helper

FPoint3 LightDirection(float angle, float direction)
{
	float phi = angle / 180.0f * PIf;
	float theta = direction / 180.0f * PIf;
	FPoint3 light_dir;
	light_dir.x = (-sin(theta)*cos(phi));
	light_dir.z = (-cos(theta)*cos(phi));
	light_dir.y = -sin(phi);
	return light_dir;
}

bool vtElevLayer::WriteElevationTileset(TilingOptions &opts, BuilderView *pView)
{
	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	// Check that options are valid
	CheckCompressionMethod(opts);

	// grid size
	int base_tilesize = opts.lod0size;

	int gridcols, gridrows;
	m_pGrid->GetDimensions(gridcols, gridrows);

	DRECT area = m_pGrid->GetEarthExtents();
	DPoint2 tile_dim(area.Width()/opts.cols, area.Height()/opts.rows);
	DPoint2 cell_size = tile_dim / base_tilesize;

	const vtCRS &crs = m_pGrid->GetCRS();

	// Try to create directory to hold the tiles
	vtString dirname = opts.fname;
	RemoveFileExtensions(dirname);
	if (!vtCreateDir(dirname))
		return false;

	// We won't know the exact height extents until the tiles have generated,
	//  so gather extents as we produce the tiles and write the INI later.
	float minheight = 1E9, maxheight = -1E9;

	ColorMap cmap;
	vtElevLayer::SetupDefaultColors(cmap);	// defaults
	vtString dirname_image = opts.fname_images;
	RemoveFileExtensions(dirname_image);
	if (opts.bCreateDerivedImages)
	{
		if (!vtCreateDir(dirname_image))
			return false;

		vtString cmap_fname = opts.draw.m_strColorMapFile;
		vtString cmap_path = FindFileOnPaths(vtGetDataPath(), "GeoTypical/" + cmap_fname);
		if (cmap_path == "")
			DisplayAndLog("Couldn't find color map.");
		else
		{
			if (!cmap.Load(cmap_path))
				DisplayAndLog("Couldn't load color map.");
		}
	}

	ImageGLCanvas *pCanvas = NULL;
#if USE_OPENGL
	wxFrame *frame = new wxFrame;
	if (opts.bCreateDerivedImages && opts.bUseTextureCompression && opts.eCompressionType == TC_OPENGL)
	{
		frame->Create(g_bld->m_pParentWindow, -1, _T("Texture Compression OpenGL Context"),
			wxPoint(100,400), wxSize(280, 300), wxCAPTION | wxCLIP_CHILDREN);
		pCanvas = new ImageGLCanvas(frame);
	}
#endif

	// make a note of which lods exist
	LODMap lod_existence_map(opts.cols, opts.rows);

	bool bFloat = m_pGrid->IsFloatMode();
	bool bJPEG = (opts.bUseTextureCompression && opts.eCompressionType == TC_JPEG);

	int i, j, lod;
	int total = opts.rows * opts.cols, done = 0;
	for (j = 0; j < opts.rows; j++)
	{
		for (i = 0; i < opts.cols; i++)
		{
			// We might want to skip certain tiles
			if (opts.iMinRow != -1 &&
				(i < opts.iMinCol || i > opts.iMaxCol ||
				 j < opts.iMinRow || j > opts.iMaxRow))
				continue;

			DRECT tile_area;
			tile_area.left = area.left + tile_dim.x * i;
			tile_area.right = area.left + tile_dim.x * (i+1);
			tile_area.bottom = area.bottom + tile_dim.y * j;
			tile_area.top = area.bottom + tile_dim.y * (j+1);

			int col = i;
			int row = opts.rows-1-j;

			// draw our progress in the main view
			if (pView)
				pView->SetGridMarks(area, opts.cols, opts.rows, col, opts.rows-1-row);

			// Extract the highest LOD we need
			vtElevationGrid base_lod(tile_area, IPoint2(base_tilesize+1, base_tilesize+1),
				bFloat, crs);

			bool bAllInvalid = true;
			bool bAllZero = true;
			int iNumInvalid = 0;
			DPoint2 p;
			int x, y;
			for (y = base_tilesize; y >= 0; y--)
			{
				p.y = area.bottom + (j*tile_dim.y) + ((double)y / base_tilesize * tile_dim.y);
				for (x = 0; x <= base_tilesize; x++)
				{
					p.x = area.left + (i*tile_dim.x) + ((double)x / base_tilesize * tile_dim.x);

					float fvalue = m_pGrid->GetFilteredValue(p);
					base_lod.SetFValue(x, y, fvalue);

					if (fvalue == INVALID_ELEVATION)
						iNumInvalid++;
					else
					{
						bAllInvalid = false;

						// Gather height extents
						if (fvalue < minheight)
							minheight = fvalue;
						if (fvalue > maxheight)
							maxheight = fvalue;
					}
					if (fvalue != 0)
						bAllZero = false;
				}
			}
			// Increment whether we omit or not
			done++;

			// If there is no real data there, omit this tile
			if (bAllInvalid)
				continue;

			// Omit all-zero tiles (flat sea-level) if desired
			if (opts.bOmitFlatTiles && bAllZero)
				continue;

			// Now we know this tile will be included, so note the LODs present
			int base_tile_exponent = vt_log2(base_tilesize);
			lod_existence_map.set(i, j, base_tile_exponent, base_tile_exponent-(opts.numlods-1));

			if (iNumInvalid > 0)
			{
				UpdateProgressDialog2(done*99/total, 0, _("Filling gaps"));

				bool bGood;
				int method = g_Options.GetValueInt(TAG_GAP_FILL_METHOD);
				if (method == 1)
					bGood = base_lod.FillGaps(NULL, progress_callback_minor);
				else if (method == 2)
					bGood = base_lod.FillGapsSmooth(NULL, progress_callback_minor);
				else if (method == 3)
					bGood = (base_lod.FillGapsByRegionGrowing(2, 5, progress_callback_minor) != -1);
				if (!bGood)
					return false;

				opts.iNoDataFilled += iNumInvalid;
			}

			// Create a matching derived texture tileset
			if (opts.bCreateDerivedImages)
			{
				// Create a matching derived texture tileset
				vtDIB dib;
				base_lod.ComputeHeightExtents();

				if (opts.bImageAlpha)
				{
					dib.Allocate(IPoint2(base_tilesize, base_tilesize), 32);
					base_lod.ColorDibFromElevation(&dib, &cmap, 4000, RGBAi(0,0,0,0));
				}
				else
				{
					dib.Allocate(IPoint2(base_tilesize, base_tilesize), 24);
					base_lod.ColorDibFromElevation(&dib, &cmap, 4000, RGBi(255,0,0));
				}

				if (opts.draw.m_bShadingQuick)
					base_lod.ShadeQuick(&dib, SHADING_BIAS, true);
				else if (opts.draw.m_bShadingDot)
				{
					FPoint3 light_dir = LightDirection(opts.draw.m_iCastAngle,
						opts.draw.m_iCastDirection);

					// Don't cast shadows for tileset; they won't cast
					//  correctly from one tile to the next.
					base_lod.ShadeDibFromElevation(&dib, light_dir, 1.0f,
						opts.draw.m_fAmbient, opts.draw.m_fGamma, true);
				}

				for (int k = 0; k < opts.numlods; k++)
				{
					vtString fname = MakeFilenameDB(dirname_image, col, row, k);

					int tilesize = base_tilesize >> k;

					vtMiniDatabuf output_buf;
					output_buf.xsize = tilesize;
					output_buf.ysize = tilesize;
					output_buf.zsize = 1;
					output_buf.tsteps = 1;
					output_buf.SetBounds(crs, tile_area);

					int depth = dib.GetDepth() / 8;
					int iUncompressedSize = tilesize * tilesize * depth;
					uchar *rgb_bytes = (uchar *) malloc(iUncompressedSize);

					uchar *dst = rgb_bytes;
					if (opts.bImageAlpha)
					{
						RGBAi rgba;
						for (int ro = 0; ro < base_tilesize; ro += (1<<k))
							for (int co = 0; co < base_tilesize; co += (1<<k))
							{
								dib.GetPixel32(co, ro, rgba);
								*dst++ = rgba.r;
								*dst++ = rgba.g;
								*dst++ = rgba.b;
								*dst++ = rgba.a;
							}
					}
					else
					{
						RGBi rgb;
						for (int ro = 0; ro < base_tilesize; ro += (1<<k))
							for (int co = 0; co < base_tilesize; co += (1<<k))
							{
								dib.GetPixel24(co, ro, rgb);
								*dst++ = rgb.r;
								*dst++ = rgb.g;
								*dst++ = rgb.b;
							}
					}

					// Write and optionally compress the image
					WriteMiniImage(fname, opts, rgb_bytes, output_buf,
						iUncompressedSize, pCanvas);

					// Free the uncompressed image
					free(rgb_bytes);
				}
			}

			for (lod = 0; lod < opts.numlods; lod++)
			{
				int tilesize = base_tilesize >> lod;

				vtString fname = MakeFilenameDB(dirname, col, row, lod);

				// make a message for the progress dialog
				wxString msg;
				msg.Printf(_("Writing tile '%hs', size %dx%d"),
					(const char *)fname, tilesize, tilesize);
				UpdateProgressDialog2(done*99/total, 0, msg);

				vtMiniDatabuf buf;
				buf.SetBounds(crs, tile_area);
				buf.alloc(tilesize+1, tilesize+1, 1, 1, bFloat ? 2 : 1);
				float *fdata = (float *) buf.data;
				short *sdata = (short *) buf.data;

				DPoint2 p;
				for (int y = base_tilesize; y >= 0; y -= (1<<lod))
				{
					p.y = area.bottom + (j*tile_dim.y) + ((double)y / base_tilesize * tile_dim.y);
					for (int x = 0; x <= base_tilesize; x += (1<<lod))
					{
						p.x = area.left + (i*tile_dim.x) + ((double)x / base_tilesize * tile_dim.x);

						if (bFloat)
						{
							*fdata = base_lod.GetFilteredValue(p);
							fdata++;
						}
						else
						{
							*sdata = (short) base_lod.GetFilteredValue(p);
							sdata++;
						}
					}
				}
				if (buf.savedata(fname) == 0)
				{
					// what should we do if writing a tile fails?
				}
			}
		}
	}

	// Write .ini file
	if (!WriteTilesetHeader(opts.fname, opts.cols, opts.rows, opts.lod0size,
		area, crs, minheight, maxheight, &lod_existence_map, false))
	{
		vtDestroyDir(dirname);
		return false;
	}

	if (opts.bCreateDerivedImages)
	{
		// Write .ini file for images
		WriteTilesetHeader(opts.fname_images, opts.cols, opts.rows,
			opts.lod0size, area, crs, INVALID_ELEVATION, INVALID_ELEVATION,
			&lod_existence_map, bJPEG);
	}

#if USE_OPENGL
	if (frame)
	{
		frame->Close();
		delete frame;
	}
#endif

	return true;
}

/**
 * Loads from a "DB" file, which is the format of libMini tilesets tiles.
 *
 * \returns \c true if the file was successfully opened and read.
 */
bool vtElevLayer::ImportFromDB(const char *szFileName, bool progress_callback(int))
{
#if USE_LIBMINI_DATABUF
	DRECT area;
	bool bFloat;
	vtCRS crs;	// CRS is always unknown

	vtMiniDatabuf dbuf;
	dbuf.loaddata(szFileName);

	if (dbuf.type == 2)
		bFloat = true;	// float
	else if (dbuf.type == 1)
		bFloat = false;	// signed short
	else
		return false;

	area.SetRect(dbuf.nwx, dbuf.nwy, dbuf.sex, dbuf.sey);

	if (!m_pGrid->Create(area, IPoint2(dbuf.xsize, dbuf.ysize), bFloat, crs))
		return false;

	int i, j;
	for (j = 0; j < (int)dbuf.ysize; j++)
	{
		//if (progress_callback != NULL)
		//	progress_callback(j * 100 / dbuf.ysize);

		for (i = 0; i < (int)dbuf.xsize; i++)
		{
			if (bFloat)
			{
				float val = dbuf.getval(i, j, 0);
				m_pGrid->SetFValue(i, dbuf.xsize-1-j, val);
			}
			else
			{
				short val = dbuf.getval(i, j, 0);
				m_pGrid->SetValue(i, dbuf.xsize-1-j, val);
			}
		}
	}
	return true;
#else
	return false;
#endif
}


/**
 * Compute a good tiling for a given area and resolution.
 *
 * Based on Tile LOD0 Size and resolution, estimate how closely a set of
 * tiles can match a given area.  This is affected by whether the area can
 * increase or decrease slightly to be an even number of tiles.
 *
 * \param original_area Input.
 * \param resolution Input: desired x and y resolution, e.g. 30 meters.
 * \param iTileSize Input: Tile LOD0 size, e.g 512.
 * \param bGrow Input: Allow the resulting area to be slightly larger than the original.
 * \param bShrink Input: Allow the resulting area to be slightly smaller than the original.
 *		Note: you must pass true for at least one of Grow and Shrink.
 * \param new_area Output: The resulting area.
 * \param tiling Output: N x M tiling.
 */
bool MatchTilingToResolution(const DRECT &original_area, const DPoint2 &resolution,
							int &iTileSize, bool bGrow, bool bShrink, DRECT &new_area,
							IPoint2 &tiling)
{
	DPoint2 tilearea;
	bool go = true;
	double estx, esty;
	while (go)
	{
		tilearea = resolution * iTileSize;

		// How many tiles would fit in the original area?
		estx = original_area.Width() / tilearea.x;
		esty = original_area.Height() / tilearea.y;

		if (estx < 1.0 || esty < 1.0)
		{
			// Tiles would not fit at all, so force the tile size smaller
			iTileSize >>= 1;
			if (iTileSize == 1)
				go = false;
		}
		else
			go = false;
	}
	if (bGrow && bShrink)
	{
		// round to closest
		tiling.x = (int) (estx + 0.5);
		tiling.y = (int) (esty + 0.5);
	}
	else if (bGrow)
	{
		// grow but not shrink: round up
		tiling.x = (int) (estx + 0.99999);
		tiling.y = (int) (esty + 0.99999);
	}
	else if (bShrink)
	{
		// shrink but not grow: round down
		tiling.x = (int) estx;
		tiling.y = (int) esty;
	}
	else
		return false;

	// Now that we know the tile size, we can compute the new area
	DPoint2 center = original_area.GetCenter();
	DPoint2 new_size(tiling.x * tilearea.x, tiling.y * tilearea.y);
	new_area.left   = center.x - 0.5 * new_size.x;
	new_area.right  = center.x + 0.5 * new_size.x;
	new_area.bottom = center.y - 0.5 * new_size.y;
	new_area.top	= center.y + 0.5 * new_size.y;

	return true;
}

// Pool of most-recently-used elevation layers, to keep in memory when paging
std::vector<vtElevLayer*> g_ElevMRU;

bool ElevCacheOpen(vtElevLayer *pLayer, const char *fname, vtElevError *err)
{
	if (vtElevLayer::m_iElevMemLimit != -1)
	{
		// Limit ourselves to a fixed number of elevation files loaded, deferred
		//  until needed.
		if (pLayer->GetGrid())
			return pLayer->GetGrid()->LoadBTHeader(fname, err);
		else
			return pLayer->GetTin()->ReadHeader(fname);
	}
	else
	{
		bool success;
		if (pLayer->GetGrid())
		{
			OpenProgressDialog(_("Loading Elevation Grid"), wxString::FromUTF8(fname));
			success = pLayer->GetGrid()->LoadFromBT(fname, progress_callback, err);
			CloseProgressDialog();
		}
		else
		{
			OpenProgressDialog(_("Loading TIN"), wxString::FromUTF8(fname));
			success = pLayer->GetTin()->Read(fname, progress_callback);
			CloseProgressDialog();
		}
		return success;
	}
}

bool ElevCacheLoadData(vtElevLayer *elev)
{
	wxString fname = elev->GetLayerFilename();
	vtString fname_utf8 = (const char *)fname.mb_str(wxConvUTF8);

	VTLOG("ElevCache needs '%s':\n", StartOfFilename(fname_utf8));

	size_t num_loaded = g_ElevMRU.size();

	int mem = 0;
	for (size_t i = 0; i < num_loaded; i++)
		mem += g_ElevMRU[i]->GetMemoryUsed();

	// Consider memory needs of new layer's data
	mem += elev->MemoryNeededToLoad();

	VTLOG("  ElevCache needs %d bytes (%.1f MB, limit is %d MB)\n", mem,
		(float)mem / (1024*1024), vtElevLayer::m_iElevMemLimit);

	bool bGo = true;
	while (bGo && mem > (vtElevLayer::m_iElevMemLimit * 1024 * 1024))
	{
		// Look for a layer we can unload, starting with the least recently
		// used (LRU) at the start of list
		size_t i;
		for (i = 0; i < g_ElevMRU.size(); i++)
		{
			vtElevLayer *elay = g_ElevMRU[i];
			if (!elay->GetSticky())
			{
				// Found one
				mem -= elay->GetMemoryUsed();

				VTLOG("  Freeing '%s', Need %d bytes (%.1f MB)\n",
					(const char *) StartOfFilenameWX(elay->GetLayerFilename()).ToAscii(),
					mem, (float)mem / (1024*1024));

				elay->FreeData();
				g_ElevMRU.erase(g_ElevMRU.begin() + i);
				break;
			}
		}
		// If we went all the way through the list without finding a layer
		//  we can unload, then give up
		if (i == g_ElevMRU.size())
		{
			VTLOG(" No more unloadable layers, will now exceed cache size.\n");
			bGo = false;
		}
	}

	VTLOG1("  ElevCache loading.\n");
	if (elev->GetGrid())
	{
		if (!elev->GetGrid()->LoadBTData(fname_utf8, progress_callback))
		{
			VTLOG("Major error!  Couldn't load file '%s' in the elevation cache.\n", (const char *)fname_utf8);
			return false;
		}
	}
	else if (elev->GetTin())
	{
		if (!elev->GetTin()->ReadBody(fname_utf8))
		{
			VTLOG("Major error!  Couldn't load file '%s' in the elevation cache.\n", (const char *)fname_utf8);
			return false;
		}
	}

	// most recently used goes to the end of the list
	//  (to be precise, it is the most recently loaded)
	g_ElevMRU.push_back(elev);

	return true;
}

void ElevCacheRemove(vtElevLayer *elev)
{
	for (size_t i = 0; i < g_ElevMRU.size(); i++)
	{
		if (g_ElevMRU[i] == elev)
		{
			g_ElevMRU.erase(g_ElevMRU.begin() + i);
			return;
		}
	}
}

