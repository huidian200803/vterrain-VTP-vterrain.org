//
// BuilderTest: test.cpp
//
// Examples of how to call the Builder library from a console application.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/ElevationGrid.h"

#include "Builder.h"
#include "Options.h"
#include "ElevLayer.h"
#include "ImageLayer.h"
#include "RenderOptions.h"
#include "vtImage.h"

void TestProjectLoad(Builder &bld)
{
	bool success = bld.LoadProject("./Data/vtb_workspace.vtb");

	if (success)
		VTLOG("Loaded %d layers.\n", bld.NumLayers());
	else
		VTLOG("Couldn't load project.\n");

	// Clean up anything that was loaded
	bld.DeleteContents();
}

void TestDEMToBT(Builder &bld)
{
	// Load an elevation layer from USGS DEM, save it to BT
	vtLayer *lay = bld.ImportDataFromFile(LT_ELEVATION,
		_T("./Data/30.dem"), false, false);
	if (!lay)
		return;

	bool success = lay->SaveAs(_T("./Data/30.bt"));

	delete lay;
}

void TestElevationReproject(Builder &bld)
{
	// Load an elevation layer, reproject from UTM to lat-lon
	vtElevLayer *elay = new vtElevLayer;
	bool success = elay->Load(_T("./Data/30.bt"));
	if (!success)
		return;

	// Make a lat-lon CS with the same datum as the UTM input
	vtProjection source, dest;
	elay->GetProjection(source);
	int datum = source.GetDatum();
	dest.SetGeogCSFromDatum(datum);

	// Do the transformation
	elay->TransformCoords(dest);

	// Indicate that we prefer a gzipped output file
	elay->SetPreferGZip(true);

	success = elay->SaveAs(_T("./Data/30_latlon.bt.gz"));

	delete elay;
}

void TestReplaceInvalid(Builder &bld)
{
	// Load an elevation layer
	vtElevLayer *elay = new vtElevLayer;
	bool success = elay->Load(_T("./Data/30.bt"));
	if (!success)
		return;

	if (1)
	{
		// We could either call the fill-gaps functionality
		elay->m_pGrid->FillGaps();
		success = elay->SaveAs(_T("./Data/30_gaps_filled.bt"));
	}
	else
	{
		// Or just replace all the gaps with a fixed value
		int count = elay->SetUnknown(1.0f);
		if (count)
			printf("Set %d heixels.\n", count);
		success = elay->SaveAs(_T("./Data/30_gaps_set.bt"));
	}
	delete elay;
}

void TestRemoveRange(Builder &bld)
{
	// Load an elevation layer
	vtElevLayer *elay = new vtElevLayer;
	bool success = elay->Load(_T("./Data/30.bt"));
	if (!success)
		return;

	// Simulate long-term global warming: remove terrain up to 10m
	float zmin = 4000;
	float zmax = 5000;
	int count = elay->RemoveElevRange(zmin, zmax);
	if (count)
		printf("Removed %d heixels.\n", count);
	success = elay->SaveAs(_T("./Data/30_range_removed.bt"));
	delete elay;
}

void TestRenderToBitmap(Builder &bld)
{
	// Load an elevation layer
	vtElevLayer *elay = new vtElevLayer;
	bool success = elay->Load(_T("./Data/30.bt"));
	if (!success)
		return;

	RenderOptions ropt;

	ropt.m_iSizeX = 200;
	ropt.m_iSizeY = 200;
	ropt.m_strColorMap = _T("VTBuilder.cmt");
	ropt.m_bToFile = true;
	ropt.m_ColorNODATA.Set(255,0,0);	// red
	ropt.m_bShading = true;
	ropt.m_strToFile = _T("./Data/30_rendered.tif");
	ropt.m_bJPEG = false;

	// Set draw options or leave defaults alone
	vtElevLayer::m_draw.m_bShadingQuick = false;
	vtElevLayer::m_draw.m_bShadingDot = true;
	vtElevLayer::m_draw.m_fGamma = 1.1f;
	vtElevLayer::m_draw.m_iCastAngle = 30;

	bld.ExportBitmap(elay, ropt);
	delete elay;
}

void TestReplaceRGB(Builder &bld)
{
	vtImageLayer *ilay = new vtImageLayer;
	bool success = ilay->Load(_T("./Data/30_rendered.tif"));
	if (!success)
		return;

	// Replace all the red pixels with black
	ilay->ReplaceColor(RGBi(255,0,0), RGBi(0,0,0));

	success = ilay->SaveAs(_T("./Data/30_rendered_black.tif"));
	delete ilay;
}

void TestDEMToTileset(Builder &bld)
{
	// Load two layers, one high resolution and one low resolution.
	//  Reproject them to the same CRS.
	//  Create a tileset from them, which will contain detail only where necessary.
	vtLayer *lay1 = bld.ImportDataFromFile(LT_ELEVATION,
		_T("./Data/hawaii-c.dem"), false, false);
	if (!lay1)
		return;
	bld.AddLayer(lay1);

	vtLayer *lay2 = bld.ImportDataFromFile(LT_ELEVATION,
		_T("./Data/30.dem"), false, false);
	if (!lay2)
		return;

	// Make sure the CRS is the same, reproject if necessary
	vtProjection proj1, proj2;
	lay1->GetProjection(proj1);
	lay2->GetProjection(proj2);
	if (proj1 != proj2)
		lay2->TransformCoords(proj1);
	bld.AddLayer(lay2);

	// Set the overall CRS
	bld.SetProjection(proj1);

	// Indicate sampling area: the extents of the larger layer
	DRECT area;
	lay1->GetExtent(area);
	bld.SetArea(area);

	// Or, this is how to set the area to full extents of all the data loaded:
	// bld.SetArea(bld.GetExtents());

	// Set all the desired sampling parameters
	// ---------------------------------------

	// Sample floats; set to false to sample short integers.
	bool bFloat = true;

	TilingOptions tileopts;
	tileopts.cols = 3;
	tileopts.rows = 3;
	tileopts.lod0size = 1024;
	tileopts.numlods = 4;
	tileopts.fname = "./Data/output_elev.ini";
	tileopts.bCreateDerivedImages = true;
	tileopts.fname_images = "./Data/output_img.ini";
	tileopts.draw.m_bShadingQuick = true;
	tileopts.draw.m_strColorMapFile = "VTBuilder.cmt";
	tileopts.draw.m_fAmbient = 0.2f;
	tileopts.bOmitFlatTiles = false;
	tileopts.bUseTextureCompression = true;
	tileopts.eCompressionType = TC_SQUISH_FAST;		// Must be Squish; no OpenGL.
	tileopts.iNoDataFilled = 0;

	// Now start the sampling
	bool success = bld.DoSampleElevationToTilePyramids(NULL, tileopts, bFloat, false);

	if (success)
	{
		VTLOG("Success.  %d nodata heixels were filled.\n", tileopts.iNoDataFilled);
	}

	// Clean up
	bld.RemoveLayer(lay1);
	bld.RemoveLayer(lay2);
}

void TestOverviews(Builder &bld)
{
	vtImageLayer *ilay = new vtImageLayer;
	bool success = ilay->Load(_T("./Data/30_rendered.tif"));
	if (!success)
		return;

	// If the image does not already have overviews, add them
	vtImage *im = ilay->GetImage();
	if (im->NumBitmapsOnDisk() == 1)
	{
		bool success = im->CreateOverviews();
	}
}

void main(int argc, char **argv)
{
	// Any program that links with wxWidgets must call this
	wxInitialize();

	// The builder object contains all the layers and methods we need
	Builder bld;

	// Set to upgrade to floating point upon reprojection
	g_Options.SetValueBool(TAG_REPRO_TO_FLOAT_ALWAYS, true, true);

	// Set TAG_LOAD_IMAGES_ALWAYS always load images into memory, or
	//  set TAG_LOAD_IMAGES_NEVER to always access them out-of-core.
	//g_Options.SetValueBool(TAG_LOAD_IMAGES_ALWAYS, true, true);
	g_Options.SetValueBool(TAG_LOAD_IMAGES_NEVER, true, true);

	// Turn on NxN multisampling for imagery
	g_Options.SetValueInt(TAG_SAMPLING_N, 3, true);

	// Set to treat-black-as-transparent for imagery
	g_Options.SetValueBool(TAG_BLACK_TRANSP, true, true);

	// Call each test
	//TestProjectLoad(bld);
	//TestDEMToBT(bld);
	//TestElevationReproject(bld);
	//TestReplaceInvalid(bld);
	//TestRemoveRange(bld);
	//TestRenderToBitmap(bld);
	//TestReplaceRGB(bld);
	//TestDEMToTileset(bld);
	TestOverviews(bld);

	wxUninitialize();
}

