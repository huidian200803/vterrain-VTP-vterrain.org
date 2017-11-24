//
// Import.cpp - MainFrame methods for importing data
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <memory>	// for auto_ptr

#include "vtdata/Building.h"
#include "vtdata/DLG.h"
#include "vtdata/DxfParser.h"
#include "vtdata/ElevationGrid.h"
#include "vtdata/FileFilters.h"
#include "vtdata/FilePath.h"
#include "vtdata/GDALWrapper.h"
#include "vtdata/LULC.h"
#include "vtdata/Unarchive.h"
#include "vtdata/vtLog.h"
#include "vtui/Helper.h"
#include "vtui/ProjectionDlg.h"

#include "Builder.h"
#include "Tin2d.h"
// Layers
#include "StructLayer.h"
#include "WaterLayer.h"
#include "ElevLayer.h"
#include "ImageLayer.h"
#include "RawLayer.h"
#include "RoadLayer.h"
#include "VegLayer.h"
#include "UtilityLayer.h"
// Dialogs
#include "ImportVegDlg.h"
#include "VegFieldsDlg.h"
#include "ImportStructDlgOGR.h"
#include "ImportPointDlg.h"

#include "ogrsf_frmts.h"

//
// remember a set of directories, one for each layer type
//
wxString ImportDirectory[LAYER_TYPES];


// Helper
wxString GetTempFolderName(const char *base)
{
	// first determine where to put our temporary directory
	vtString path;

	const char *temp = getenv("TEMP");
	if (temp)
		path = temp;
	else
#if WIN32
		path = "C:/TEMP";
#else
		path = "/tmp";
#endif
	path += "/";

	// the create a folder named after the file in the full path "base"
	vtString base2 = StartOfFilename(base);
	path += base2;

	// appended with the word _temp
	path += "_temp";

	return wxString(path, wxConvUTF8);
}


//
// Ask the user for a filename, and import data from it.
//
void Builder::ImportData(LayerType ltype)
{
	// make a string which contains filters for the appropriate file types
	wxString filter = GetImportFilterString(ltype);

	// ask the user for a filename
	// default the same directory they used last time for a layer of this type
	wxFileDialog loadFile(NULL, _("Import Data"), ImportDirectory[ltype],
		_T(""), filter, wxFD_OPEN | wxFD_MULTIPLE);
	bool bResult = (loadFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	//multiple selection
	wxArrayString strFileNameArray;
	loadFile.GetPaths(strFileNameArray);

	// remember the directory they used
	ImportDirectory[ltype] = loadFile.GetDirectory();

	// TESTING code here
//	ImportDataFromS57(strFileName);

	for (uint i=0; i<strFileNameArray.GetCount(); ++i)
	{
		const wxString &fname = strFileNameArray.Item(i);
		if (ImportDataFromArchive(ltype, fname, true) != 0)
		{
			// succeeded, so add to the MRU
			AddToMRU(m_ImportFiles, (const char *) fname.mb_str(wxConvUTF8));
		}
	}
}

/**
 * Import data of a given type from a file, which can potentially be an
 * archive file.  If it's an archive, it will be unarchived to a temporary
 * folder, and the contents will be imported.
 *
 * \return Number of layers created during the import.
 */
int Builder::ImportDataFromArchive(LayerType ltype, const wxString &fname_in,
								   bool bRefresh)
{
	VTLOG("ImportDataFromArchive(type %d, '%s'\n", ltype, (const char *)fname_in.mb_str(wxConvUTF8));

	// check file extension
	wxString fname = fname_in;
	wxString ext = fname.AfterLast('.');

	// check if it's an archive
	bool bGZip = false;
	bool bTGZip = false;
	bool bZip = false;

	if (ext.CmpNoCase(_T("gz")) == 0 || ext.CmpNoCase(_T("bz2")) == 0)
	{
		// We could expand .gz and .bz2 files into a temporary folder, but it
		//  would be inefficient as many of the file readers used gzopen etc.
		//  hence they already support gzipped input efficiently.
		bGZip = true;
	}
	if (ext.CmpNoCase(_T("tgz")) == 0 || ext.CmpNoCase(_T("tar")) == 0 ||
		fname.Right(7).CmpNoCase(_T(".tar.gz")) == 0)
		bTGZip = true;

	if (ext.CmpNoCase(_T("zip")) == 0)
		bZip = true;

	if (!bTGZip && !bZip)
	{
		LayerArray layers;
		layers.SetOwnership(false);
		bool got = ImportLayersFromFile(ltype, fname, layers, bRefresh, true);
		if (!got)
			return 0;	// no layers created

		int num_imported = 0;
		for (uint i = 0; i < layers.size(); i++)
		{
			if (AddLayerWithCheck(layers[i], true))
				num_imported++;		// Layer accepted
			else
				delete layers[i];	// Discard the failed layer
		}
		return num_imported;
	}

	// try to uncompress
	wxString path, prepend_path;
	path = GetTempFolderName(fname_in.mb_str(wxConvUTF8));

	VTLOG("Creating temp dir at '%s'\n", (const char *)path.mb_str(wxConvUTF8));
	bool created = vtCreateDir(path.mb_str(wxConvUTF8));
	if (!created && errno != EEXIST)
	{
		DisplayAndLog("Couldn't create temporary directory to hold contents of archive.");
		return 0;	// no layers created
	}
	prepend_path = path + _T("/");

	vtString str1 = (const char *) fname_in.mb_str(wxConvUTF8);
	vtString str2 = (const char *) prepend_path.mb_str(wxConvUTF8);

	OpenProgressDialog(_("Expanding archive"), wxString::FromUTF8((const char *) str1),
		false, m_pParentWindow);
	int num_files;
	if (bTGZip)
		num_files = ExpandTGZ(str1, str2);
	if (bZip)
		num_files = ExpandZip(str1, str2, progress_callback);
	CloseProgressDialog();

	int layer_count = 0;
	VTLOG(" Unarchived %d files.\n", num_files);
	if (num_files < 1)
	{
		DisplayAndLog("Couldn't expand archive.");
	}
	else if (num_files == 1)
	{
		// the archive contained a single file
		std::string pathname = (const char *) prepend_path.mb_str(wxConvUTF8);
		wxString internal_name;
		for (dir_iter it(pathname); it != dir_iter(); ++it)
		{
			if (it.is_directory())
				continue;
			std::string name1 = it.filename();
			fname = prepend_path;
			internal_name = wxString(name1.c_str(), wxConvUTF8);
			fname += internal_name;
			break;
		}

		// try to load, or import it
		Builder::LoadResult result = LoadLayer(fname);
		if (result == Builder::LOADED)
			layer_count = 1;
		else if (result == Builder::NOT_NATIVE)
		{
			// Otherwise, try importing
			LayerArray layers;
			layers.SetOwnership(false);
			if (ImportLayersFromFile(ltype, fname, layers, bRefresh, true))
			{
				int num_imported = 0;
				for (uint i = 0; i < layers.size(); i++)
				{
					if (AddLayerWithCheck(layers[i], true))
					{
						num_imported++;
						// use the internal filename, not the archive filename which is temporary
						layers[i]->SetLayerFilename(internal_name);
						layers[i]->SetImportedFrom(fname_in);
					}
					else
						delete layers[i];
				}
				layer_count = num_imported;
			}
		}
		else if (result == CANCELLED || result == FAILED)
			layer_count = 0;
	}
	else if (num_files > 1)
	{
		vtArray<vtLayer *> LoadedLayers;
		vtLayer *pLayer;

		// probably SDTS
		// try to guess layer type from original file name
		if (fname.Contains(_T(".hy")) || fname.Contains(_T(".HY")))
			ltype = LT_WATER;
		if (fname.Contains(_T(".rd")) || fname.Contains(_T(".RD")))
			ltype = LT_ROAD;
		if (fname.Contains(_T(".dem")) || fname.Contains(_T(".DEM")))
			ltype = LT_ELEVATION;
		if (fname.Contains(_T(".ms")) || fname.Contains(_T(".MS")))
			ltype = LT_STRUCTURE;

		// look for an SDTS catalog file
		wxString fname_cat;
		wxString fname_hdr;
		bool found_cat = false;
		bool found_hdr = false;
		bool found_rt1 = false;
		bool found_dem = false;
		bool found_bil = false;

		std::string pathname = (const char *) path.mb_str(wxConvUTF8);
		VTLOG(" Looking at contents of folder: '%s'\n", pathname.c_str());

		for (dir_iter it(pathname); it != dir_iter(); ++it)
		{
			if (it.is_directory()) continue;
			wxString fname2 = wxString(it.filename().c_str(), wxConvUTF8);

			if (fname2.Right(8).CmpNoCase(_T("catd.ddf")) == 0)
			{
				fname_cat = prepend_path;
				fname_cat += fname2;
				found_cat = true;
				break;
			}
			if (fname2.Right(4).CmpNoCase(_T(".hdr")) == 0)
			{
				ltype = LT_ELEVATION;
				fname_hdr = prepend_path;
				fname_hdr += fname2;
				found_hdr = true;
				break;
			}
			if (fname2.Right(4).CmpNoCase(_T(".dem")) == 0)
				found_dem = true;
			if (fname2.Right(4).CmpNoCase(_T(".bil")) == 0)
				found_bil = true;
			if (fname2.Right(4).CmpNoCase(_T(".rt1")) == 0)
				found_rt1 = true;
		}
		wxString single_file_import;
		if (found_cat)
			single_file_import = fname_cat;
		if (found_hdr && found_dem)
			single_file_import = fname_hdr;
		if (found_hdr && found_bil)
			single_file_import = fname_hdr;
		if (single_file_import != _T(""))
		{
			// We expect a single layer from SDTS or BIL/HDR or DEM/HDR (GTOPO30)
			pLayer = ImportLayerFromFile(ltype, single_file_import, bRefresh, true);
			if (pLayer)
			{
				bool success = AddLayerWithCheck(pLayer, true);
				if (!success)
				{
					delete pLayer;
					return 0;	// no layers created
				}
				pLayer->SetLayerFilename(single_file_import);
				LoadedLayers.Append(pLayer);
				layer_count++;
			}
		}
		else if (found_rt1)
		{
			layer_count = ImportDataFromTIGER(path);
		}
		else
		{
			// Look through archive for individual files (like .dem)
			std::string path = (const char *) prepend_path.mb_str(wxConvUTF8);
			for (dir_iter it(path); it != dir_iter(); ++it)
			{
				if (it.is_directory()) continue;
				wxString fname2 = wxString(it.filename().c_str(), wxConvUTF8);

				fname = prepend_path;
				fname += fname2;

				// Try importing w/o warning on failure, since it could just
				// be some harmless files in there.
				pLayer = ImportLayerFromFile(ltype, fname, bRefresh, false);
				if (pLayer)
				{
					bool success = AddLayerWithCheck(pLayer, true);
					if (!success)
					{
						delete pLayer;
						return 0;	// no layers loaded
					}
					pLayer->SetLayerFilename(fname2);
					LoadedLayers.Append(pLayer);
					layer_count++;
				}
			}
		}
		if (layer_count == 0)
			DisplayAndLog("Don't know what to do with contents of archive.");

		// set the original imported filename
		for (uint i = 0; i < LoadedLayers.GetSize(); i++)
			LoadedLayers[i]->SetImportedFrom(fname_in);
	}

	// clean up after ourselves
	prepend_path = GetTempFolderName(fname_in.mb_str(wxConvUTF8));
	vtDestroyDir(prepend_path.mb_str(wxConvUTF8));

	return layer_count;
}

/**
 * ImportLayersFromFile: the main import method.
 *
 * \param strFileName	The filename.
 * \param layers		A container by reference, to hold any resulting layers.
 * \param bRefresh		True if the GUI should be refreshed after import.
 * \param bWarn			True if the GUI should be warned on failure.
 *
 * \return	True if any layers were successfully imported.
 */
bool Builder::ImportLayersFromFile(LayerType ltype, const wxString &strFileName,
	LayerArray &layers, bool bRefresh, bool bWarn)
{
	// check the file extension
	wxString strExt = strFileName.AfterLast('.');

	// Is it a kind of file we might get multiple layers from?
	if (!strExt.CmpNoCase(_T("osm")))
	{
		OpenProgressDialog(_("Importing from OpenStreetMap"), strFileName,
			false, m_pParentWindow);
		UpdateProgressDialog(0, strFileName);
		ImportDataFromOSM(strFileName, layers, progress_callback);
		CloseProgressDialog();
	}
	else if (!strExt.CmpNoCase(_T("ntf")))
	{
		ImportDataFromNTF(strFileName, layers);
	}
	else
	{
		// We only expect at most one layer.
		vtLayer *layer = ImportLayerFromFile(ltype, strFileName,
			bRefresh, bWarn);
		if (layer)
			layers.push_back(layer);
	}
	return (layers.size() > 0);
}

/**
 * ImportLayerFromFile: a main import method.
 *
 * \param ltype			The Layer type suspected.
 * \param strFileName	The filename.
 * \param bRefresh		True if the GUI should be refreshed after import.
 * \param bWarn			True if the GUI should be warned on failure.
 *
 * \return	The layer imported, or NULL if we got nothing.
 */
vtLayer *Builder::ImportLayerFromFile(LayerType ltype, const wxString &strFileName,
									  bool bRefresh, bool bWarn)
{
	VTLOG1("ImportDataFromFile '");
	VTLOG1(strFileName.ToUTF8());
	VTLOG1("', type '");
	VTLOG1(GetLayerTypeName(ltype).ToUTF8());
	VTLOG1("'\n");

	// check the file extension
	wxString strExt = strFileName.AfterLast('.');

	// check to see if the file is readable
	vtString fname = (const char *) strFileName.mb_str(wxConvUTF8);
	FILE *fp = vtFileOpen(fname, "rb");
	if (!fp)
	{
		// Cannot Open File
		VTLOG("Couldn't open file %s\n", (const char *) fname);
		return NULL;
	}
	bool bIsDB = (strExt.Len() == 2 && !strExt.Left(2).CmpNoCase(_T("db")));
	if (bIsDB)
	{
		// Get type from DB file
		for (int i = 0; i < 10; i++)
		{
			char buf[80];
			if (fgets(buf, 80, fp) != NULL)
			{
				if (!strncmp(buf, "type", 4))
				{
					int type;
					sscanf(buf, "type=%d", &type);

					// 0 = unsigned byte, 1 = signed short, 2 = float, 3 = RGB,
					// 4 = RGBA, 5 = compressed RGB, 6 = compressed RGBA
					if (type == 1 || type == 2)
						ltype = LT_ELEVATION;
					else if (type == 3 || type == 4)
						ltype = LT_IMAGE;
					break;
				}
			}
		}
	}
	fclose(fp);

	if (m_pParentWindow)
		OpenProgressDialog(_("Importing Data"), strFileName, true, m_pParentWindow);

	// call the appropriate reader
	vtLayerPtr pLayer = NULL;
	switch (ltype)
	{
	case LT_ELEVATION:
		pLayer = ImportElevation(strFileName, bWarn);
		break;
	case LT_IMAGE:
		pLayer = ImportImage(strFileName);
		break;
	case LT_ROAD:
	case LT_WATER:
		if (!strExt.CmpNoCase(_T("dlg")))
		{
			pLayer = ImportFromDLG(strFileName, ltype);
		}
		else if (!strExt.CmpNoCase(_T("shp")))
		{
			pLayer = ImportFromSHP(strFileName, ltype);
		}
		else if (!strFileName.Right(8).CmpNoCase(_T("catd.ddf")) ||
				 !strExt.CmpNoCase(_T("mif")) ||
				 !strExt.CmpNoCase(_T("tab")))
		{
			pLayer = ImportVectorsWithOGR(strFileName, ltype);
		}
		break;
	case LT_STRUCTURE:
		if (!strExt.CmpNoCase(_T("shp")))
		{
			pLayer = ImportFromSHP(strFileName, ltype);
//			pLayer = ImportVectorsWithOGR(strFileName, ltype);
		}
		else if (!strExt.CmpNoCase(_T("gml")))
		{
			pLayer = ImportVectorsWithOGR(strFileName, ltype);
		}
		else if (!strExt.CmpNoCase(_T("bcf")))
		{
			pLayer = ImportFromBCF(strFileName);
		}
		else if (!strExt.CmpNoCase(_T("dlg")))
		{
			pLayer = ImportFromDLG(strFileName, ltype);
		}
		else if (!strFileName.Right(8).CmpNoCase(_T("catd.ddf")))
		{
			pLayer = ImportVectorsWithOGR(strFileName, ltype);
		}
		break;
	case LT_VEG:
		if (!strExt.CmpNoCase(_T("gir")))
		{
			pLayer = ImportFromLULC(strFileName, ltype);
		}
		if (!strExt.CmpNoCase(_T("shp")))
		{
			pLayer = ImportFromSHP(strFileName, ltype);
		}
		break;
	case LT_UNKNOWN:
		if (!strExt.CmpNoCase(_T("gir")))
		{
			pLayer = ImportFromLULC(strFileName, ltype);
		}
		else if (!strExt.CmpNoCase(_T("bcf")))
		{
			pLayer = ImportFromBCF(strFileName);
		}
		else if (!strExt.CmpNoCase(_T("dlg")))
		{
			pLayer = ImportFromDLG(strFileName, ltype);
		}
		else if (!strFileName.Right(8).CmpNoCase(_T("catd.ddf")))
		{
			// SDTS file: might be Elevation or Vector (SDTS-DEM or SDTS-DLG)
			// To try to distinguish, look for a file called xxxxrsdf.ddf
			// which would indicate that it is a raster.
			bool bRaster = false;
			int len = (int)strFileName.Length();
			wxString strFileName2 = strFileName.Left(len - 8);
			wxString strFileName3 = strFileName2 + _T("rsdf.ddf");
			FILE *fp;
			fp = vtFileOpen(strFileName3.mb_str(wxConvUTF8), "rb");
			if (fp != NULL)
			{
				bRaster = true;
				fclose(fp);
			}
			else
			{
				// also try with upper-case (for Unix)
				strFileName3 = strFileName2 + _T("RSDF.DDF");
				fp = vtFileOpen(strFileName3.mb_str(wxConvUTF8), "rb");
				if (fp != NULL)
				{
					bRaster = true;
					fclose(fp);
				}
			}
			if (bRaster)
				pLayer = ImportElevation(strFileName, bWarn);
			else
				pLayer = ImportVectorsWithOGR(strFileName, ltype);
		}
		else if (!strExt.CmpNoCase(_T("shp")) ||
				 !strExt.CmpNoCase(_T("igc")))
		{
			pLayer = new vtRawLayer;
			pLayer->SetLayerFilename(strFileName);
			if (!pLayer->OnLoad())
			{
				delete pLayer;
				pLayer = NULL;
			}
		}
		else if (!strExt.CmpNoCase(_T("xyz")) ||
				 !strExt.CmpNoCase(_T("enzi")))
		{
			vtFeatureSet *pSet = ImportPointsFromXYZ(fname, progress_callback);
			if (pSet)
			{
				vtRawLayer *pRaw = new vtRawLayer;
				pRaw->SetLayerFilename(strFileName);
				pRaw->SetFeatureSet(pSet);
				// Adopt existing CRS
				pRaw->SetCRS(m_crs);
				pLayer = pRaw;
			}
		}
		else if (!strExt.CmpNoCase(_T("bcf")))
		{
			pLayer = ImportFromBCF(strFileName);
		}
		else if (!strExt.CmpNoCase(_T("jpg")))
		{
			pLayer = ImportImage(strFileName);
		}
		else if (!strExt.Left(3).CmpNoCase(_T("ppm")) ||
				 !strExt.Left(2).CmpNoCase(_T("db")))
		{
			pLayer = ImportImage(strFileName);
		}
		else if (!strExt.CmpNoCase(_T("mif")) ||
				 !strExt.CmpNoCase(_T("tab")))
		{
			pLayer = ImportRawFromOGR(strFileName);
		}
		else
		{
			// Many other Elevation formats are supported
			pLayer = ImportElevation(strFileName, bWarn);
		}
		break;
	case LT_UTILITY:
		if(!strExt.CmpNoCase(_T("shp")))
			pLayer = ImportFromSHP(strFileName, ltype);
		break;
	case LT_RAW:
		if (!strExt.CmpNoCase(_T("shp")))
			pLayer = ImportFromSHP(strFileName, ltype);
		else if (!strExt.CmpNoCase(_T("dxf")))
			pLayer = ImportFromDXF(strFileName, ltype);
		else if (!strExt.CmpNoCase(_T("igc")))
		{
			vtRawLayer *pRL = new vtRawLayer;
			if (pRL->Load(wxString(fname, wxConvUTF8)))
				pLayer = pRL;
		}
		else
		{
			pLayer = ImportRawFromOGR(strFileName);
		}
		break;
	default:	// Keep picky compilers quiet.
		break;
	}
	if (bIsDB && pLayer)
	{
		// Adopt existing CRS
		pLayer->SetCRS(m_crs);
	}

	CloseProgressDialog();

	if (!pLayer)
	{
		// import failed
		VTLOG("  import failed/cancelled.\n");
		if (bWarn)
			wxMessageBox(_("Did not import any data from that file."));
		return NULL;
	}
	VTLOG("  import succeeded.\n");

	wxString layer_fname = pLayer->GetLayerFilename();
	if (layer_fname.IsEmpty() || !layer_fname.Cmp(_("Untitled")))
		pLayer->SetLayerFilename(strFileName);

	return pLayer;
}

//
// type to guess layer type from a DLG file
//
LayerType Builder::GuessLayerTypeFromDLG(vtDLGFile *pDLG)
{
	LayerType ltype = LT_UNKNOWN;
	DLGType dtype = pDLG->GuessFileType();

	// convert the DLG type to one of our layer types
	switch (dtype)
	{
	case DLG_HYPSO:		ltype = LT_RAW; break;
	case DLG_HYDRO:		ltype = LT_WATER; break;
	case DLG_VEG:		ltype = LT_RAW; break;
	case DLG_NONVEG:	ltype = LT_RAW; break;
	case DLG_BOUNDARIES:ltype = LT_RAW; break;
	case DLG_MARKERS:	ltype = LT_RAW; break;
	case DLG_ROAD:		ltype = LT_ROAD; break;
	case DLG_RAIL:		ltype = LT_ROAD; break;
	case DLG_MTF:		ltype = LT_RAW; break;
	case DLG_MANMADE:	ltype = LT_STRUCTURE; break;
	case DLG_UNKNOWN:
		{
			// if we can't tell from the DLG, ask the user
			ltype = AskLayerType();
		}
		break;
	}
	return ltype;
}

//
// based on the type of layer, choose which file
// types (file extensions) to allow for import
//
wxString GetImportFilterString(LayerType ltype)
{
	wxString filter = _T("All Known ");
	filter += vtLayer::LayerTypeNames[ltype];
	filter += _T(" Formats|");

	switch (ltype)
	{
	case LT_RAW:
		// abstract GIS data
		AddType(filter, FSTRING_DXF);
		AddType(filter, FSTRING_IGC);
		AddType(filter, FSTRING_MI);
		AddType(filter, FSTRING_NTF);
		AddType(filter, FSTRING_SHP);
		break;
	case LT_ELEVATION:
		// dem, etc.
		AddType(filter, FSTRING_3TX);
		AddType(filter, FSTRING_ADF);
		AddType(filter, FSTRING_ASC);
		AddType(filter, FSTRING_BIL);
		AddType(filter, FSTRING_CDF);
		AddType(filter, FSTRING_DEM);
		AddType(filter, FSTRING_DTED);
		AddType(filter, FSTRING_DXF);
		AddType(filter, FSTRING_GMS);
		AddType(filter, FSTRING_GTOPO);
		AddType(filter, FSTRING_HGT);
		AddType(filter, FSTRING_IMG);
		AddType(filter, FSTRING_MEM);
		AddType(filter, FSTRING_NTF);
		AddType(filter, FSTRING_PGM);
		AddType(filter, FSTRING_PNG);
		AddType(filter, FSTRING_RAW);
		AddType(filter, FSTRING_SDTS);
		AddType(filter, FSTRING_Surfer);
		AddType(filter, FSTRING_TER);
		AddType(filter, FSTRING_TIF);
		AddType(filter, FSTRING_ECW);
		AddType(filter, FSTRING_TXT);
		AddType(filter, FSTRING_XYZ);
		AddType(filter, FSTRING_COMP);
		break;
	case LT_IMAGE:
		// bmp, doq, img, png, ppm, tif
		AddType(filter, FSTRING_BMP);
		AddType(filter, FSTRING_DOQ);
		AddType(filter, FSTRING_IMG);
		AddType(filter, FSTRING_PNG);
		AddType(filter, FSTRING_PPM);
		AddType(filter, FSTRING_TIF);
		AddType(filter, FSTRING_ECW);
		break;
	case LT_ROAD:
		// dlg, shp, sdts-dlg, osm
		AddType(filter, FSTRING_COMP);
		AddType(filter, FSTRING_DLG);
		AddType(filter, FSTRING_MI);
		AddType(filter, FSTRING_OSM);
		AddType(filter, FSTRING_SDTS);
		AddType(filter, FSTRING_SHP);
		break;
	case LT_STRUCTURE:
		// dlg, shp, bcf, sdts-dlg
		AddType(filter, FSTRING_GML);
		AddType(filter, FSTRING_DLG);
		AddType(filter, FSTRING_SHP);
		AddType(filter, FSTRING_BCF);
		AddType(filter, FSTRING_SDTS);
		AddType(filter, FSTRING_COMP);
		break;
	case LT_WATER:
		// dlg, shp, sdts-dlg
		AddType(filter, FSTRING_DLG);
		AddType(filter, FSTRING_SHP);
		AddType(filter, FSTRING_SDTS);
		AddType(filter, FSTRING_COMP);
		break;
	case LT_VEG:
		// lulc, shp, sdts
		AddType(filter, FSTRING_LULC);
		AddType(filter, FSTRING_SHP);
		AddType(filter, FSTRING_SDTS);
		AddType(filter, FSTRING_COMP);
		break;
	case LT_UTILITY:
		AddType(filter, FSTRING_SHP);
		break;
	default:	// Keep picky compilers quiet.
		break;
	}
	return filter;
}


vtLayerPtr Builder::ImportFromDLG(const wxString &fname_in, LayerType ltype)
{
	vtDLGFile *pDLG = new vtDLGFile;
	bool success = pDLG->Read(fname_in.mb_str(wxConvUTF8), progress_callback);
	if (!success)
	{
		DisplayAndLog(pDLG->GetErrorMessage());
		delete pDLG;
		return NULL;
	}

	// try to guess what kind of data it is by asking the DLG object
	// to look at its attributes
	if (ltype == LT_UNKNOWN)
		ltype = GuessLayerTypeFromDLG(pDLG);

	// create the new layer
	vtLayerPtr pLayer = vtLayer::CreateNewLayer(ltype);

	// read the DLG data into the layer
	if (ltype == LT_ROAD)
	{
		vtRoadLayer *pRL = (vtRoadLayer *)pLayer;
		pRL->AddElementsFromDLG(pDLG);
		pRL->RemoveUnusedNodes();
	}
	if (ltype == LT_WATER)
	{
		vtWaterLayer *pWL = (vtWaterLayer *)pLayer;
		pWL->AddElementsFromDLG(pDLG);
	}
	if (ltype == LT_STRUCTURE)
	{
		vtStructureLayer *pSL = (vtStructureLayer *)pLayer;
		pSL->AddElementsFromDLG(pDLG);
	}
/*	if (ltype == LT_RAW)
	{
		vtRawLayer *pRL = (vtRawLayer *)pLayer;
		pRL->AddElementsFromDLG(pDLG);
	}
*/
	// now we no longer need the DLG object
	delete pDLG;

	return pLayer;
}

vtLayerPtr Builder::ImportFromSHP(const wxString &strFileName, LayerType ltype)
{
	bool success;
	int nShapeType;

	// SHPOpen doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(strFileName.mb_str(wxConvUTF8));

	SHPHandle hSHP = SHPOpen(fname_local, "rb");
	if (hSHP == NULL)
	{
		wxMessageBox(_("Couldn't read that Shape file.  Perhaps it is\nmissing its corresponding .dbf and .shx files."));
		return NULL;
	}
	else
	{
		// Get type of data
		SHPGetInfo(hSHP, NULL, &nShapeType, NULL, NULL);

		// Check Shape Type, Veg Layer should be Poly data
		SHPClose(hSHP);
	}

	// if layer type unknown, ask user input
	if (ltype == LT_UNKNOWN)
	{
		ltype = AskLayerType();
		if (ltype == LT_UNKNOWN)	// User cancelled the operation
			return NULL;
	}

	// Create the new layer
	vtLayerPtr pLayer = vtLayer::CreateNewLayer(ltype);

	// Does SHP already have a CRS?
	vtCRS crs;
	if (crs.ReadProjFile(strFileName.mb_str(wxConvUTF8)))
	{
		// OK, we'll use it
	}
	else
	{
		// Ask user for a CRS
		ProjectionDlg dlg(NULL, -1, _("Please indicate coordinate system"));
		dlg.SetCRS(m_crs);

		if (dlg.ShowModal() == wxID_CANCEL)
			return NULL;
		dlg.GetCRS(crs);
	}

	// read SHP data into the layer
	if (ltype == LT_ROAD)
	{
		vtRoadLayer *pRL = (vtRoadLayer *)pLayer;
		pRL->AddElementsFromSHP(strFileName, crs, progress_callback);
		pRL->RemoveUnusedNodes();
	}

	// read vegetation SHP data into the layer
	if (ltype == LT_VEG)
	{
		if (nShapeType != SHPT_POLYGON && nShapeType != SHPT_POINT)
		{
			wxMessageBox(_("The Shapefile must have either point features\n(for individual plants) or polygon features\n (for plant distribution areas)."));
			return NULL;
		}

		vtVegLayer *pVL = (vtVegLayer *)pLayer;
		if (nShapeType == SHPT_POLYGON)
		{
			ImportVegDlg dlg(m_pParentWindow, -1, _("Import Vegetation Information"));
			dlg.SetShapefileName(strFileName);
			if (dlg.ShowModal() == wxID_CANCEL)
				return NULL;
			success = pVL->AddElementsFromSHP_Polys(strFileName, crs,
				dlg.m_fieldindex, dlg.m_datatype);
			if (!success)
				return NULL;
		}
		if (nShapeType == SHPT_POINT)
		{
			VegFieldsDlg dlg(m_pParentWindow, -1, _("Map fields to attributes"));
			dlg.SetShapefileName(strFileName);
			dlg.SetVegLayer(pVL);
			if (dlg.ShowModal() == wxID_CANCEL)
				return NULL;
			success = pVL->AddElementsFromSHP_Points(strFileName, crs, dlg.m_options);
			if (!success)
				return NULL;
		}
	}

	if (ltype == LT_WATER)
	{
		vtWaterLayer *pWL = (vtWaterLayer *)pLayer;
		pWL->AddElementsFromSHP(strFileName, crs);
	}

	if (ltype == LT_STRUCTURE)
	{
		vtStructureLayer *pSL = (vtStructureLayer *)pLayer;
		success = pSL->AddElementsFromSHP(strFileName, crs, m_area);
		if (!success)
			return NULL;
	}

	if (ltype == LT_RAW)
	{
		pLayer->SetLayerFilename(strFileName);
		if (pLayer->OnLoad())
			pLayer->SetCRS(crs);
		else
		{
			delete pLayer;
			pLayer = NULL;
		}
	}
	return pLayer;
}

vtLayerPtr Builder::ImportFromDXF(const wxString &strFileName, LayerType ltype)
{
	if (ltype == LT_ELEVATION)
	{
		vtElevError err;
		vtElevLayer *pEL = new vtElevLayer;
		if (pEL->ImportFromFile(strFileName, NULL, &err))
			return pEL;
		else
			return NULL;
	}
	if (ltype == LT_RAW)
	{
		vtFeatureLoader loader;
		vtFeatureSet *pSet = loader.LoadFromDXF(strFileName.mb_str(wxConvUTF8));
		if (!pSet)
			return NULL;

		// We should ask for a CRS
		vtCRS &Crs = pSet->GetAtCRS();
		if (!g_bld->ConfirmValidCRS(&Crs))
		{
			delete pSet;
			return NULL;
		}
		vtRawLayer *pRL = new vtRawLayer;
		pRL->SetFeatureSet(pSet);
		return pRL;
	}
	return NULL;
}

vtLayerPtr Builder::ImportElevation(const wxString &strFileName, bool bWarn)
{
	vtElevLayer *pElev = new vtElevLayer;

	vtElevError err;
	bool success = pElev->ImportFromFile(strFileName, progress_callback, &err);

	if (success)
		return pElev;
	else
	{
		if (bWarn)
		{
			// Try getting descriptive message from the grid
			vtString msg = err.message;
			if (msg == "")
				msg = "Couldn't import data from that file.";
			DisplayAndLog(msg);
		}
		delete pElev;
		return NULL;
	}
}

vtLayerPtr Builder::ImportImage(const wxString &strFileName)
{
	vtImageLayer *pLayer = new vtImageLayer;

	bool success = pLayer->ImportFromFile(strFileName);

	if (success)
		return pLayer;
	else
	{
		delete pLayer;
		return NULL;
	}
}

vtLayerPtr Builder::ImportFromLULC(const wxString &strFileName, LayerType ltype)
{
	// Read LULC file, check for errors
	vtLULCFile *pLULC = new vtLULCFile(strFileName.mb_str(wxConvUTF8));
	if (pLULC->m_iError)
	{
		wxString msg(pLULC->GetErrorMessage(), wxConvUTF8);
		wxMessageBox(msg);
		delete pLULC;
		return NULL;
	}

	// If layer type unknown, assume it's veg type
	if (ltype == LT_UNKNOWN)
		ltype=LT_VEG;

	pLULC->ProcessLULCPolys();

	// Create New Layer
	vtLayerPtr pLayer = vtLayer::CreateNewLayer(ltype);

	// Read LULC data into the new Veg layer
	vtVegLayer *pVL = (vtVegLayer *)pLayer;

	pVL->AddElementsFromLULC(pLULC);

	// Now we no longer need the LULC object
	delete pLULC;

	return pLayer;
}

vtStructureLayer *Builder::ImportFromBCF(const wxString &strFileName)
{
	vtStructureLayer *pSL = new vtStructureLayer;
	if (pSL->ReadBCF(strFileName.mb_str(wxConvUTF8)))
		return pSL;
	else
	{
		delete pSL;
		return NULL;
	}
}

//
// Import from a Garmin MapSource GPS export file (.txt)
//
void Builder::ImportFromMapSource(const char *fname)
{
	FILE *fp = vtFileOpen(fname, "r");
	if (!fp)
		return;

	vtArray<vtRawLayer *> layers;
	char buf[200];
	bool bUTM = false;
	bool bGotSRS = false;
	vtCRS crs;

	if (fgets(buf, 200, fp) == NULL)
		return;
	if (!strncmp(buf+5, "UTM", 3))
		bUTM = true;
	if (fgets(buf, 200, fp) == NULL) // assume "Datum   WGS 84"
		return;

	char ch;
	int i;
	vtRawLayer *pRL=NULL;

	while (fgets(buf, 200, fp))	// get a line
	{
		if (!strncmp(buf, "Track\t", 6))
		{
			pRL = new vtRawLayer;
			pRL->SetGeomType(wkbPoint);
			layers.Append(pRL);
			bGotSRS = false;

			// parse name
			char name[40];
			for (i = 6; ; i++)
			{
				ch = buf[i];
				if (ch == '\t' || ch == 0)
					break;
				name[i-6] = ch;
			}
			name[i-6] = 0;
			pRL->SetLayerFilename(wxString(name, wxConvUTF8));
		}
		if (!strncmp(buf, "Trackpoint", 10))
		{
			DPoint2 p;
			if (bUTM)
			{
				int zone;
				sscanf(buf+10, "%d %c %lf %lf", &zone, &ch, &p.x, &p.y);

				if (!bGotSRS)
				{
					crs.SetWellKnownGeogCS("WGS84");
					if (bUTM)
						crs.SetUTMZone(zone);
					pRL->SetCRS(crs);
					bGotSRS = true;
				}
			}
			else
			{
				// Example: N20 04.319 W155 27.902
				char east_hemi;
				int east_degree;
				double east_minute;
				char nord_hemi;
				int nord_degree;
				double nord_minute;
				sscanf(buf+11, "%c%d %lf %c%d %lf ",
					&nord_hemi, &nord_degree, &nord_minute,
					&east_hemi, &east_degree, &east_minute);
				p.x = (east_hemi=='E'?1:-1) * (east_degree + east_minute/60.0);
				p.y = (nord_hemi=='N'?1:-1) * (nord_degree + nord_minute/60.0);

				if (!bGotSRS)
				{
					crs.SetWellKnownGeogCS("WGS84");
					pRL->SetCRS(crs);
					bGotSRS = true;
				}
			}
			pRL->AddPoint(p);
		}
	}

	// Display the list of imported tracks to the user
	int n = layers.GetSize();
	wxString *choices = new wxString[n];
	wxArrayInt selections;
	wxString str;
	for (i = 0; i < n; i++)
	{
		choices[i] = layers[i]->GetLayerFilename();

		choices[i] += _T(" (");
		if (bUTM)
		{
			layers[i]->GetCRS(crs);
			str.Printf(_T("zone %d, "), crs.GetUTMZone());
			choices[i] += str;
		}
		str.Printf(_T("points %d"), layers[i]->GetFeatureSet()->NumEntities());
		choices[i] += str;
		choices[i] += _T(")");
	}

#if wxVERSION_NUMBER > 2900		// 2.9.0
	int nsel = wxGetSelectedChoices(selections, _("Which layers to import?"),
		_("Import Tracks"), n, choices);
#else
	// Older API
	int nsel = (int)wxGetMultipleChoices(selections, _("Which layers to import?"),
		_("Import Tracks"), n, choices);
#endif

	// for each of the layers the user wants, add them to our project
	for (i = 0; i < nsel; i++)
	{
		int sel = selections[i];
		AddLayerWithCheck(layers[sel]);
		layers[sel]->SetModified(false);
	}
	// for all the rest, delete 'em
	for (i = 0; i < n; i++)
	{
		if (layers[i]->GetModified())
			delete layers[i];
	}
	delete [] choices;
}

// Helper for following methods
double ExtractValueFromString(const char *string, int iStyle, bool bEasting,
							  bool bFlipEasting)
{
	if (iStyle == 0)	// decimal
	{
		return atof(string);
	}
	else if (iStyle == 1)	// packed DMS
	{
		int deg, min, sec, frac;
		if (bEasting)
		{
			deg  = GetIntFromString(string, 3);
			min  = GetIntFromString(string+3, 2);
			sec  = GetIntFromString(string+5, 2);
			frac = GetIntFromString(string+7, 2);
			if (deg > 180)
			{
				deg  = GetIntFromString(string, 2);
				min  = GetIntFromString(string+2, 2);
				sec  = GetIntFromString(string+4, 2);
				frac = 0;
			}
		}
		else
		{
			deg  = GetIntFromString(string, 2);
			min  = GetIntFromString(string+2, 2);
			sec  = GetIntFromString(string+4, 2);
			frac = GetIntFromString(string+6, 2);
		}
		double secs = sec + (frac/100.0);
		double val = deg + (min/60.0) + (secs/3600.0);
		if (bFlipEasting)
			val = -val;
		return val;
	}
	else if (iStyle == 2)	// HMD: Hemisphere, Degrees, Minutes
	{
		// Example string: "E144 53.621"
		char hemi = string[0];
		int degrees;
		double minutes;
		sscanf(string+1, "%d %lf", &degrees, &minutes);
		double val = degrees + (minutes/60.0);

		// southern and western hemispheres: negative values
		if (hemi == 'W' || hemi == 'S')
			val = -val;

		return val;
	}
	return 0.0;
}

// Helper for following methods
double ExtractValue(DBFHandle db, int iRec, int iField, DBFFieldType ftype,
					int iStyle, bool bEasting, bool bFlipEasting)
{
	const char *string;
	switch (ftype)
	{
	case FTString:
		string = DBFReadStringAttribute(db, iRec, iField);
		return ExtractValueFromString(string, iStyle, bEasting, bFlipEasting);
	case FTInteger:
		return DBFReadIntegerAttribute(db, iRec, iField);
	case FTDouble:
		return DBFReadDoubleAttribute(db, iRec, iField);
	default:
		return 0.0;
	}
	return 0.0;
}

vtFeatureSetPoint2D *Builder::ImportPointsFromDBF(const char *fname)
{
	// DBFOpen doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(fname);

	// Open DBF File
	DBFHandle db = DBFOpen(fname_local, "rb");
	if (db == NULL)
		return NULL;

	ImportPointDlg dlg(m_pParentWindow, -1, _("Point Data Import"));

	// default to the current CRS
	dlg.SetCRS(m_crs);

	// Fill the DBF field names into the "Use Field" controls
	int *pnWidth = 0, *pnDecimals = 0;
	char pszFieldName[32];
	int iFields = DBFGetFieldCount(db);
	int i;
	vtArray<DBFFieldType> m_fieldtypes;
	for (i = 0; i < iFields; i++)
	{
		DBFFieldType fieldtype = DBFGetFieldInfo(db, i, pszFieldName,
			pnWidth, pnDecimals );
		wxString str(pszFieldName, wxConvUTF8);

		dlg.GetEasting()->Append(str);
		dlg.GetNorthing()->Append(str);
		dlg.GetElevation()->Append(str);
		dlg.GetImportField()->Append(str);

		m_fieldtypes.Append(fieldtype);
		//if (fieldtype == FTString)
		//	GetChoiceFileField()->Append(str);
		//if (fieldtype == FTInteger || fieldtype == FTDouble)
		//	GetChoiceHeightField()->Append(str);
	}
	if (dlg.ShowModal() != wxID_OK)
	{
		DBFClose(db);
		return NULL;
	}
	int iEast = dlg.m_iEasting;
	int iNorth = dlg.m_iNorthing;
	int iStyle;
	if (dlg.m_bFormat1) iStyle = 0;
	if (dlg.m_bFormat2) iStyle = 1;
	if (dlg.m_bFormat3) iStyle = 2;

	// Now import
	vtFeatureSetPoint2D *pSet = new vtFeatureSetPoint2D;
	pSet->SetCRS(dlg.m_crs);

	int iRecords = DBFGetRecordCount(db);
	for (i = 0; i < iRecords; i++)
	{
		DPoint2 p;
		p.x = ExtractValue(db, i, iEast, m_fieldtypes[iEast], iStyle, true, dlg.m_bLongitudeWest);
		p.y = ExtractValue(db, i, iNorth, m_fieldtypes[iNorth], iStyle, false, false);
		pSet->AddPoint(p);
	}
	DBFClose(db);

	// Also copy along the corresponding DBF data into the new featureset
	pSet->SetFilename(fname);
	pSet->LoadDataFromDBF(fname);

	return pSet;
}

void Tokenize(char *buf, const char *delim, vtStringArray &tokens)
{
	char *p = NULL;
	p = strtok(buf, delim);
	while (p != NULL)
	{
		tokens.push_back(vtString(p));
		p = strtok(NULL, delim);
	}
}

int ReadLine(char *buf, int max_count, FILE *fp, bool bExpectCR, bool bExpectLF)
{
	int count = 0;
	while (1)
	{
		char ch = fgetc(fp);
		if (ch == EOF)
			break;
		else if (ch == 10 || ch == 13)
		{
			if (bExpectCR && !bExpectLF && ch == 13)
				break;
			if (bExpectLF && ch == 10)
				break;
		}
		else
		{
			buf[count++] = ch;
			if (count == max_count)
				break;
		}
	}
	buf[count] = 0;	// terminate
	return count;
}

vtFeatureSet *Builder::ImportPointsFromCSV(const char *fname)
{
	FILE *fp = vtFileOpen(fname, "rb");
	if (fp == NULL)
		return NULL;

	// First, try to guess the line ending.
	// It is probably LF, or CR LR, but it might just be CR.
	bool bHaveCR = false, bHaveLF = false;
	char buf[4096];
	size_t count = fread(buf, 1, 4096, fp);
	for (size_t i = 0; i < count; i++)
	{
		if (buf[i] == 10) bHaveLF = true;
		if (buf[i] == 13) bHaveCR = true;
	}
	rewind(fp);

	// Assume that the first line contains field names
	vtStringArray fieldnames;
	if (ReadLine(buf, 4096, fp, bHaveCR, bHaveLF) == 0)
		return NULL;
	Tokenize(buf, ",", fieldnames);
	int iFields = (int)fieldnames.size();
	if (iFields == 0)
	{
		fclose(fp);
		return NULL;
	}

	ImportPointDlg dlg(m_pParentWindow, -1, _("Point Data Import"));
	dlg.m_bElevation = false;

	// default to the current CRS
	dlg.SetCRS(m_crs);

	// Fill the field names into the "Use Field" controls
	for (int i = 0; i < iFields; i++)
	{
		wxString str;
		str.sprintf(_T("%d"), i);
		str += _T(" (");
		str += wxString(fieldnames[i], wxConvUTF8);
		str += _T(")");

		dlg.GetEasting()->Append(str);
		dlg.GetNorthing()->Append(str);
		dlg.GetElevation()->Append(str);
		dlg.GetImportField()->Append(str);
	}
	if (dlg.ShowModal() != wxID_OK)
	{
		fclose(fp);
		return NULL;
	}
	int iEast = dlg.m_iEasting;
	int iNorth = dlg.m_iNorthing;
	int iElev = dlg.m_iElevation;
	int iStyle;
	if (dlg.m_bFormat1) iStyle = 0;
	if (dlg.m_bFormat2) iStyle = 1;
	if (dlg.m_bFormat3) iStyle = 2;

	// Now import
	if (dlg.m_bElevation)
	{
		vtFeatureSetPoint3D *pSet = new vtFeatureSetPoint3D;
		pSet->SetCRS(dlg.m_crs);

		if (dlg.m_bImportField)
		{
			pSet->AddField(fieldnames[dlg.m_iImportField], FT_String, 40);
		}

		while (ReadLine(buf, 4096, fp, bHaveCR, bHaveLF))
		{
			vtStringArray values;
			Tokenize(buf, ",", values);

			DPoint3 p;
			p.x = ExtractValueFromString(values[iEast], iStyle, true, dlg.m_bLongitudeWest);
			p.y = ExtractValueFromString(values[iNorth], iStyle, false, false);
			p.z = atof(values[iElev]);
			int record = pSet->AddPoint(p);

			if (dlg.m_bImportField)
			{
				pSet->SetValueFromString(record, 0, values[dlg.m_iImportField]);
			}
		}
		pSet->SetFilename(fname);
		return pSet;
	}
	else
	{
		vtFeatureSetPoint2D *pSet = new vtFeatureSetPoint2D;
		pSet->SetCRS(dlg.m_crs);

		while (ReadLine(buf, 4096, fp, bHaveCR, bHaveLF))
		{
			vtStringArray values;
			Tokenize(buf, ",", values);

			DPoint2 p;
			p.x = ExtractValueFromString(values[iEast], iStyle, true, dlg.m_bLongitudeWest);
			p.y = ExtractValueFromString(values[iNorth], iStyle, false, false);
			pSet->AddPoint(p);
		}
		pSet->SetFilename(fname);
		return pSet;
	}
}

vtFeatureSet *Builder::ImportPointsFromXYZ(const char *fname, bool progress_callback(int))
{
	FILE *fp = vtFileOpen(fname, "rb");
	if (fp == NULL)
		return NULL;

	int line = 0, count = 0;
	char buf[4096];

	// Now import
	vtFeatureSetPoint3D *pSet = new vtFeatureSetPoint3D;

	vtStringArray values;
	while (fgets(buf, 4096, fp))
	{
		line++;
		if ((line % 1024) == 0)
		{
			progress_callback(count);
			if (++count > 99) count = 0;
		}

		values.clear();
		Tokenize(buf, " ", values);

		DPoint3 p;
		p.x = atof(values[0]);
		p.y = atof(values[1]);
		p.z = atof(values[2]);
		int record = pSet->AddPoint(p);
	}
	fclose(fp);
	pSet->SetFilename(fname);
	return pSet;
}

//
// Import point data from a tabular data source such as a .dbf or .csv
//
void Builder::ImportDataPointsFromTable(const char *fname, bool progress_callback(int))
{
	vtFeatureSet *pSet = NULL;

	vtString ext = GetExtension(fname);
	if (!ext.CompareNoCase(".dbf"))
		pSet = ImportPointsFromDBF(fname);
	else if (!ext.CompareNoCase(".csv"))
		pSet = ImportPointsFromCSV(fname);
	else if (!ext.CompareNoCase(".xyz") || !ext.CompareNoCase(".enzi"))
		pSet = ImportPointsFromXYZ(fname, progress_callback);
	else
		return;

	if (pSet)
	{
		vtRawLayer *pRaw = new vtRawLayer;
		pRaw->SetFeatureSet(pSet);
		AddLayerWithCheck(pRaw);
	}
}

vtLayerPtr Builder::ImportRawFromOGR(const wxString &strFileName)
{
	// create the new layer
	vtRawLayer *pRL = new vtRawLayer;
	bool success = pRL->LoadWithOGR(strFileName.mb_str(wxConvUTF8), progress_callback);

	if (success)
		return pRL;
	else
	{
		delete pRL;
		return NULL;
	}
}

vtLayerPtr Builder::ImportVectorsWithOGR(const wxString &strFileName, LayerType ltype)
{
	vtCRS crs;

	g_GDALWrapper.RequestOGRFormats();

	// OGR doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(strFileName.mb_str(wxConvUTF8));

	GDALDataset *datasource = (GDALDataset *)GDALOpen(fname_local, GA_ReadOnly);
	if (!datasource)
		return NULL;

	if (ltype == LT_UNKNOWN)
	{
		// TODO: Try to guess the layer type from the file
		// For now, just assume it's transportation
		ltype = LT_ROAD;
	}

	// create the new layer
	vtLayerPtr pLayer = vtLayer::CreateNewLayer(ltype);

	// read the OGR data into the layer
	if (ltype == LT_ROAD)
	{
		vtRoadLayer *pRL = (vtRoadLayer *)pLayer;
		pRL->AddElementsFromOGR(datasource, progress_callback);
//		pRL->RemoveUnusedNodes();
	}
	if (ltype == LT_WATER)
	{
		vtWaterLayer *pWL = (vtWaterLayer *)pLayer;
		pWL->AddElementsFromOGR(datasource, progress_callback);
	}
	if (ltype == LT_STRUCTURE)
	{
		ImportStructOGRDlg ImportDialog(g_bld->m_pParentWindow, -1, _("Import Structures"));

		ImportDialog.SetDatasource(datasource);

		if (ImportDialog.ShowModal() != wxID_OK)
			return NULL;

		if (ImportDialog.m_iType == 0)
			ImportDialog.m_opt.type = ST_BUILDING;
		if (ImportDialog.m_iType == 1)
			ImportDialog.m_opt.type = ST_BUILDING;
		if (ImportDialog.m_iType == 2)
			ImportDialog.m_opt.type = ST_LINEAR;
		if (ImportDialog.m_iType == 3)
			ImportDialog.m_opt.type = ST_INSTANCE;

		ImportDialog.m_opt.rect = m_area;

		vtStructureLayer *pSL = (vtStructureLayer *)pLayer;

		if (NULL != GetActiveElevLayer())
			ImportDialog.m_opt.pHeightField = GetActiveElevLayer()->GetHeightField();
		else if (NULL != FindLayerOfType(LT_ELEVATION))
			ImportDialog.m_opt.pHeightField = ((vtElevLayer*)FindLayerOfType(LT_ELEVATION))->GetHeightField();
		else
			ImportDialog.m_opt.pHeightField = NULL;
		pSL->AddElementsFromOGR(datasource, ImportDialog.m_opt, progress_callback);

		pSL->GetCRS(crs);
		if (OGRERR_NONE != crs.Validate())
		{
			// Get a CRS
			ProjectionDlg dlg(g_bld->m_pParentWindow, -1, _("Please indicate coodinate system"));
			dlg.SetCRS(m_crs);

			if (dlg.ShowModal() == wxID_CANCEL)
			{
				delete pSL;
				return NULL;
			}
			dlg.GetCRS(crs);
			pSL->SetCRS(crs);
		}
	}

	delete datasource;

	return pLayer;
}


//
//Import from TIGER, returns number of layers imported
//
int Builder::ImportDataFromTIGER(const wxString &strDirName)
{
	g_GDALWrapper.RequestOGRFormats();

	// OGR doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(strDirName.mb_str(wxConvUTF8));

	GDALDataset *pDatasource = (GDALDataset *)GDALOpen(fname_local, GA_ReadOnly);
	if (!pDatasource)
		return 0;

#if 0	// TODO: port this to GDAL 2.0
	int i, j, feature_count;
	OGRLayer		*pOGRLayer;
	OGRFeature		*pFeature;
	OGRGeometry		*pGeom;
	OGRLineString   *pLineString;
	vtWaterFeature	wfeat;

	// Assume that this data source is a TIGER/Line file
	//
	// Iterate through the layers looking for the ones we care about
	//
	int num_layers = pDatasource->GetLayerCount();
	vtString layername = pDatasource->GetName();

	// create the new layers
	vtWaterLayer *pWL = new vtWaterLayer;
	pWL->SetLayerFilename(wxString(layername + "_water", wxConvUTF8));
	pWL->SetModified(true);

	vtRoadLayer *pRL = new vtRoadLayer;
	pRL->SetLayerFilename(wxString(layername + "_roads", wxConvUTF8));
	pRL->SetModified(true);

	for (i = 0; i < num_layers; i++)
	{
		pOGRLayer = pDatasource->GetLayer(i);
		if (!pOGRLayer)
			continue;

		feature_count = pOGRLayer->GetFeatureCount();
		pOGRLayer->ResetReading();
		OGRFeatureDefn *defn = pOGRLayer->GetLayerDefn();
		if (!defn)
			continue;

#if VTDEBUG
		VTLOG("Layer %d/%d, '%s'\n", i, num_layers, defn->GetName());

		// Debug: iterate through the fields
		int field_count1 = defn->GetFieldCount();
		for (j = 0; j < field_count1; j++)
		{
			OGRFieldDefn *field_def1 = defn->GetFieldDefn(j);
			if (field_def1)
			{
				const char *fnameref = field_def1->GetNameRef();
				OGRFieldType ftype = field_def1->GetType();
				VTLOG("  field '%s' type %d\n", fnameref, ftype);
			}
		}
#endif

		// ignore all layers other than CompleteChain
		const char *layer_name = defn->GetName();
		if (strcmp(layer_name, "CompleteChain"))
			continue;

		// Get the projection (SpatialReference) from this layer
		OGRSpatialReference *pSpatialRef = pOGRLayer->GetSpatialRef();
		if (pSpatialRef)
		{
			vtCRS crs;
			crs.SetSpatialReference(pSpatialRef);
			pWL->SetCRS(crs);
			pRL->SetCRS(crs);
		}

		// Progress Dialog
		OpenProgressDialog(_("Importing from TIGER..."), strDirName);

		int index_cfcc = defn->GetFieldIndex("CFCC");
		int fcount = 0;
		while( (pFeature = pOGRLayer->GetNextFeature()) != NULL )
		{
			// make sure we delete the feature no matter how the loop exits
			std::auto_ptr<OGRFeature> ensure_deletion(pFeature);

			UpdateProgressDialog(100 * fcount / feature_count);

			pGeom = pFeature->GetGeometryRef();
			if (!pGeom) continue;

			if (!pFeature->IsFieldSet(index_cfcc))
				continue;

			const char *cfcc = pFeature->GetFieldAsString(index_cfcc);

			pLineString = (OGRLineString *) pGeom;
			int num_points = pLineString->getNumPoints();

			if (!strncmp(cfcc, "A", 1))
			{
				// Road: implicit nodes at start and end
				LinkEdit *r = pRL->AddNewLink();
				bool bReject = pRL->ApplyCFCC((LinkEdit *)r, cfcc);
				if (bReject)
				{
					delete r;
					continue;
				}
				for (j = 0; j < num_points; j++)
				{
					r->Append(DPoint2(pLineString->getX(j),
						pLineString->getY(j)));
				}
				TNode *n1 = pRL->AddNewNode();
				n1->SetPos(pLineString->getX(0), pLineString->getY(0));

				TNode *n2 = pRL->AddNewNode();
				n2->SetPos(pLineString->getX(num_points-1), pLineString->getY(num_points-1));

				r->ConnectNodes(n1, n2);

				//set bounding box for the road
				r->Dirtied();
			}

			if (!strncmp(cfcc, "H", 1))
			{
				// Hydrography
				int num = atoi(cfcc+1);
				bool bSkip = true;
				switch (num)
				{
				case 1:		// Shoreline of perennial water feature
				case 2:		// Shoreline of intermittent water feature
					break;
				case 11:	// Perennial stream or river
				case 12:	// Intermittent stream, river, or wash
				case 13:	// Braided stream or river
					wfeat.m_bIsBody = false;
					bSkip = false;
					break;
				case 30:	// Lake or pond
				case 31:	// Perennial lake or pond
				case 32:	// Intermittent lake or pond
				case 40:	// Reservoir
				case 41:	// Perennial reservoir
				case 42:	// Intermittent reservoir
				case 50:	// Bay, estuary, gulf, sound, sea, or ocean
				case 51:	// Bay, estuary, gulf, or sound
				case 52:	// Sea or ocean
					wfeat.m_bIsBody = true;
					bSkip = false;
					break;
				}
				if (!bSkip)
				{
					wfeat.SetSize(num_points);
					for (j = 0; j < num_points; j++)
					{
						wfeat.SetAt(j, DPoint2(pLineString->getX(j),
							pLineString->getY(j)));
					}
					pWL->AddFeature(wfeat);
				}
			}

			fcount++;
		}
		CloseProgressDialog();
	}

	delete pDatasource;

	// Merge nodes
//	OpenProgressDialog("Removing redundant nodes...");
//	pRL->MergeRedundantNodes(true, progress_callback);

	// Set visual properties
	for (NodeEdit *pN = pRL->GetFirstNode(); pN; pN = pN->GetNext())
	{
		pN->DetermineVisualFromLinks();
	}

	int layer_count = 0;
	bool success;
	success = AddLayerWithCheck(pWL, true);
	if (!success)
		delete pWL;
	else
		layer_count++;

	success = AddLayerWithCheck(pRL, true);
	if (!success)
		delete pRL;
	else
		layer_count++;

	return layer_count;
#else
	return 0;
#endif
}

void Builder::ImportDataFromNTF(const wxString &strFileName, LayerArray &layers)
{
	g_GDALWrapper.RequestOGRFormats();

	// OGR doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(strFileName.mb_str(wxConvUTF8));

	GDALDataset *pDatasource = (GDALDataset *)GDALOpen(fname_local, GA_ReadOnly);
	if (!pDatasource)
		return;

	// Progress Dialog
	OpenProgressDialog(_("Importing from NTF..."), strFileName);

	OGRFeature *pFeature;
	OGRGeometry		*pGeom;
	OGRLineString   *pLineString;
	OGRSpatialReference *pSpatialRef = NULL;

	// create the (potential) new layers
	vtRoadLayer *pRL = new vtRoadLayer;
	pRL->SetLayerFilename(strFileName + _T(";roads"));
	pRL->SetModified(true);

	vtStructureLayer *pSL = new vtStructureLayer;
	pSL->SetLayerFilename(strFileName + _T(";structures"));
	pSL->SetModified(true);

	// Iterate through the layers looking for the ones we care about?
	//
	int i, num_layers = pDatasource->GetLayerCount();
	for (i = 0; i < num_layers; i++)
	{
		OGRLayer *pOGRLayer = pDatasource->GetLayer(i);
		if (!pOGRLayer)
			continue;

		if (pSpatialRef == NULL)
		{
			pSpatialRef = pOGRLayer->GetSpatialRef();
			if (pSpatialRef)
			{
				vtCRS crs;
				crs.SetSpatialReference(pSpatialRef);
				pRL->SetCRS(crs);
				pSL->SetCRS(crs);
			}
		}
#if 0
		// Simply create a raw layer from each OGR layer
		vtRawLayer *pRL = new vtRawLayer;
		if (pRL->CreateFromOGRLayer(pOGRLayer))
		{
			wxString layname = strFileName;
			layname += wxString::Format(_T(";%d"), i);
			pRL->SetLayerFilename(layname);

			bool success = AddLayerWithCheck(pRL, true);
		}
		else
			delete pRL;
#else
		int feature_count = pOGRLayer->GetFeatureCount();
		pOGRLayer->ResetReading();
		OGRFeatureDefn *defn = pOGRLayer->GetLayerDefn();
		if (!defn)
			continue;
		vtString layer_name = defn->GetName();

		// We depend on feature codes
		int index_fc = defn->GetFieldIndex("FEAT_CODE");
		if (index_fc == -1)
			continue;

		// Points
		if (layer_name == "LANDLINE_POINT" || layer_name == "LANDLINE99_POINT")
		{
		}
		// Lines
		if (layer_name == "LANDLINE_LINE" || layer_name == "LANDLINE99_LINE")
		{
			int fcount = 0;
			while( (pFeature = pOGRLayer->GetNextFeature()) != NULL )
			{
				// make sure we delete the feature no matter how the loop exits
				std::auto_ptr<OGRFeature> ensure_deletion(pFeature);

				UpdateProgressDialog(100 * fcount / feature_count);
				fcount++;

				pGeom = pFeature->GetGeometryRef();
				if (!pGeom) continue;

				if (!pFeature->IsFieldSet(index_fc))
					continue;

				vtString fc = pFeature->GetFieldAsString(index_fc);

				pLineString = (OGRLineString *) pGeom;

				if (fc == "0001")	// Building outline
				{
					vtBuilding *bld = pSL->AddBuildingFromLineString(pLineString);
					if (bld)
					{
						vtBuilding *pDefBld = GetClosestDefault(bld);
						if (pDefBld)
							bld->CopyStyleFrom(pDefBld, true);
						else
						{
							bld->SetNumStories(1);
							bld->SetRoofType(ROOF_FLAT);
						}
					}
				}
				if (fc == "0098")	// Road centerline
				{
					LinkEdit *pLE = pRL->AddRoadSegment(pLineString);
					// some defaults..
					pLE->m_iLanes = 2;
					pLE->m_fLaneWidth = 3.0f;
					pLE->SetFlag(RF_MARGIN, true);
				}
			}
		}
		// Names
		if (layer_name == "LANDLINE_NAME" || layer_name == "LANDLINE99_NAME")
		{
		}
#endif
	}

	layers.push_back(pRL);
	layers.push_back(pSL);

	delete pDatasource;

	CloseProgressDialog();
}


void Builder::ImportDataFromS57(const wxString &strDirName)
{
	g_GDALWrapper.RequestOGRFormats();

	// OGR doesn't yet support utf-8 or wide filenames, so convert
	vtString fname_local = UTF8ToLocal(strDirName.mb_str(wxConvUTF8));

	GDALDataset *pDatasource = (GDALDataset *)GDALOpen(fname_local, GA_ReadOnly);
	if (!pDatasource)
		return;

	// create the new layers
	vtWaterLayer *pWL = new vtWaterLayer;
	pWL->SetLayerFilename(strDirName + _T("/water"));
	pWL->SetModified(true);

	int i, j, feature_count;
	OGRLayer		*pOGRLayer;
	OGRFeature		*pFeature;
	OGRGeometry		*pGeom;
	OGRLineString   *pLineString;

	vtWaterFeature	wfeat;

	// Assume that this data source is a S57 file
	//
	// Iterate through the layers looking for the ones we care about
	//
	int num_layers = pDatasource->GetLayerCount();
	for (i = 0; i < num_layers; i++)
	{
		pOGRLayer = pDatasource->GetLayer(i);
		if (!pOGRLayer)
			continue;

		feature_count = pOGRLayer->GetFeatureCount();
		pOGRLayer->ResetReading();
		OGRFeatureDefn *defn = pOGRLayer->GetLayerDefn();
		if (!defn)
			continue;

#if 0
		//Debug: interate throught the fields
		int field_count1 = defn->GetFieldCount();
		for (j = 0; j < field_count1; j++)
		{
			OGRFieldDefn *field_def1 = defn->GetFieldDefn(j);
			if (field_def1)
			{
				const char *fnameref = field_def1->GetNameRef();
				OGRFieldType ftype = field_def1->GetType();
			}
		}
#endif

		// Get the projection (SpatialReference) from this layer
		OGRSpatialReference *pSpatialRef = pOGRLayer->GetSpatialRef();
		if (pSpatialRef)
		{
			vtCRS crs;
			crs.SetSpatialReference(pSpatialRef);
			pWL->SetCRS(crs);
		}

		// Progress Dialog
		OpenProgressDialog(_("Importing from S-57..."), strDirName);

		// Get line features
		const char *layer_name = defn->GetName();
		if (strcmp(layer_name, "Line"))
			continue;

		int fcount = 0;
		while( (pFeature = pOGRLayer->GetNextFeature()) != NULL )
		{
			// make sure we delete the feature no matter how the loop exits
			std::unique_ptr<OGRFeature> ensure_deletion(pFeature);

			UpdateProgressDialog(100 * fcount / feature_count);

			pGeom = pFeature->GetGeometryRef();
			if (!pGeom) continue;

			pLineString = (OGRLineString *) pGeom;
			int num_points = pLineString->getNumPoints();

			if (!strcmp(layer_name, "Line"))
			{
				// Hydrography
				wfeat.SetSize(num_points);
				for (j = 0; j < num_points; j++)
				{
					wfeat.SetAt(j, DPoint2(pLineString->getX(j),
						pLineString->getY(j)));
				}
				pWL->AddFeature(wfeat);
			}

			fcount++;
		}
		CloseProgressDialog();
	}
	delete pDatasource;

	bool success;
	success = AddLayerWithCheck(pWL, true);
	if (!success)
		delete pWL;
}


//
//Import from SCC Viewer Export Format
//
int Builder::ImportDataFromSCC(const char *filename)
{
	FILE *fp = vtFileOpen(filename, "rb");
	if (!fp)
		return 0;

	vtString shortname = StartOfFilename(filename);
	RemoveFileExtensions(shortname);

	vtCRS crs;
	crs.SetGeogCSFromDatum(EPSG_DATUM_WGS84);
	crs.SetUTM(1);

	// create the new layers
	vtElevLayer *pEL = new vtElevLayer;
	pEL->SetLayerFilename(wxString(shortname + "_tin", wxConvUTF8));
	pEL->SetModified(true);

	vtStructureLayer *pSL = new vtStructureLayer;
	pSL->SetLayerFilename(wxString(shortname + "_structures", wxConvUTF8));
	pSL->SetModified(true);
	pSL->SetCRS(crs);

	vtVegLayer *pVL = new vtVegLayer;
	pVL->SetModified(true);
	pVL->SetVegType(VLT_Instances);
	pVL->SetLayerFilename(wxString(shortname + "_vegetation", wxConvUTF8));
	pVL->SetCRS(crs);
	vtPlantInstanceArray *pia = pVL->GetPIA();
	pia->SetSpeciesList(&m_SpeciesList);
	int id = m_SpeciesList.GetSpeciesIdByCommonName("Ponderosa Pine");
	vtPlantSpecies *ps = m_SpeciesList.GetSpecies(id);

	// Progress Dialog
	OpenProgressDialog(_("Importing from SCC..."), wxString::FromUTF8((const char *) filename));

	vtTin2d *tin = new vtTin2d;

	int state = 0;
	int num_mesh, num_tri, color, v, vtx = 0, mesh = 0;
	char buf[400];
	vtString object_type;
	float height;
	vtFence *linear;

	while (fgets(buf, 400, fp) != NULL)
	{
		if (state == 0)	// start of file: number of TIN meshes
		{
			sscanf(buf, "%d", &num_mesh);
			state = 1;
		}
		else if (state == 1)	// header line of mesh
		{
			// Each mesh starts with a header giving the mesh name, number of
			//  triangles in the mesh, mesh color, and mesh texture name
			const char *word = strtok(buf, ",");
			vtString name = word;
			word = strtok(NULL, ",");
			sscanf(word, "%d", &num_tri);
			word = strtok(NULL, ",");
			sscanf(word, "%d", &color);
			word = strtok(NULL, ",");
			state = 2;
			v = 0;
		}
		else if (state == 2)	// vertex of a TIN
		{
			// A vertex comprises of X,Y,Z ordinates, and X, Y, and Z vertex
			//  normals.
			DPoint2 p;
			float z;
			sscanf(buf, "%lf,%lf,%f", &p.x, &p.y, &z);
			tin->AddVert(p, z);
			if ((vtx%3)==2)
				tin->AddTri(vtx-2, vtx-1, vtx);
			vtx++;
			v++;

			UpdateProgressDialog(100 * v / (num_tri*3));

			if (v == num_tri*3)	// last vtx of this mesh
			{
				state = 1;
				mesh++;
				if (mesh == num_mesh)
					state = 4;
			}
		}
		else if (state == 4)	// CONTOUR etc.
		{
			if (!strncmp(buf, "CONTOUR", 7))
			{
				state = 5;
			}
			else if (!strncmp(buf, "OBJECTS", 7))
			{
				// The header line contains the objects layer name, the object
				//  name, the color, and the objects type description.
				const char *word = strtok(buf, ",");
				// first: OBJECTS

				word = strtok(NULL, ",");
				vtString layer_name = word;

				word = strtok(NULL, ",");
				vtString object_name = word;

				word = strtok(NULL, ",");
				sscanf(word, "%d", &color);

				word = strtok(NULL, ",");
				object_type = word;

				state = 6;
			}
			else if (!strncmp(buf, "OBJLINE", 7))
			{
				// the objects layer name, the object name, the color, the
				// objects size in X, Y and Z, and the objects type description.
				const char *word = strtok(buf, ",");
				// first: OBJLINE

				word = strtok(NULL, ",");
				vtString layer_name = word;

				word = strtok(NULL, ",");
				vtString object_name = word;

				word = strtok(NULL, ",");
				sscanf(word, "%d", &color);

				FPoint2 size;
				word = strtok(NULL, ",");
				sscanf(word, "%f", &size.x);
				word = strtok(NULL, ",");
				sscanf(word, "%f", &size.y);
				word = strtok(NULL, ",");
				sscanf(word, "%f", &height);

				word = strtok(NULL, ",");
				object_type = word;

				// Begin a new linear structure
				linear = pSL->AddNewFence();
				if (object_type.Left(5) == "Fence")
					linear->ApplyStyle(FS_WOOD_POSTS_WIRE);
				else if (object_type.Left(5) == "Hedge")
					linear->ApplyStyle(FS_PRIVET);
				else
					// unknown type; use a railing as a placeholder
					linear->ApplyStyle(FS_RAILING_ROW);

				// Apply height and spacing
				vtLinearParams &params = linear->GetParams();
				params.m_fPostHeight = height;
				params.m_fPostSpacing = size.y;

				state = 7;
			}
		}
		else if (state == 5)	// CONTOUR
		{
			if (!strncmp(buf, "ENDCONTOUR", 10))
				state = 4;
		}
		else if (state == 6)	// OBJECTS
		{
			if (!strncmp(buf, "ENDOBJECTS", 10))
				state = 4;
			else
			{
				// X,Y,Z insertion point, followed by the objects size in X, Y
				//  and Z, and object orientation in X,Y, and Z axes.
				DPoint2 p;
				float z;
				FPoint3 size;
				sscanf(buf, "%lf,%lf,%f,%f,%f,%f", &p.x, &p.y, &z,
					&size.x, &size.y, &size.z);

				if (object_type.Left(4) == "Tree")
				{
					pia->AddPlant(p, size.z, ps);
				}
			}
		}
		else if (state == 7)	// OBJLINE
		{
			if (!strncmp(buf, "ENDOBJLINE", 10))
			{
				// Close linear structure
				state = 4;
			}
			else
			{
				// Add X,Y,Z point to linear structure
				DPoint2 p;
				float z;
				sscanf(buf, "%lf,%lf,%f", &p.x, &p.y, &z);
				if (linear)
					linear->AddPoint(p);
			}
		}
	}
	fclose(fp);
	CloseProgressDialog();

	tin->ComputeExtents();
	tin->CleanupClockwisdom();
	pEL->SetTin(tin);
	pEL->SetCRS(crs);

	int layer_count = 0;
	bool success;
	success = AddLayerWithCheck(pEL);
	if (!success)
		delete pEL;
	else
		layer_count++;

	success = AddLayerWithCheck(pSL);
	if (!success)
		delete pSL;
	else
		layer_count++;

	success = AddLayerWithCheck(pVL);
	if (!success)
		delete pVL;
	else
		layer_count++;

	return layer_count;
}

bool Builder::ImportDataFromDXF(const char *filename)
{
	VTLOG1("ImportDataFromDXF():\n");

	std::vector<DxfEntity> entities;
	std::vector<DxfLayer> layers;

	OpenProgressDialog(_("Parsing DXF"), wxString::FromUTF8((const char *) filename),
		true, m_pParentWindow);
	DxfParser parser(filename, entities, layers);
	bool bSuccess = parser.RetrieveEntities(progress_callback);
	CloseProgressDialog();
	if (!bSuccess)
	{
		VTLOG1(parser.GetLastError());
		return false;
	}

	// We could just use vtFeatureLoader::LoadFromDXF, but that only produces
	//  a single feature set.  A DXF might have points, polylines and polygons
	//  in it, so that should produce multiple featuresets
	vtFeatureSetPoint2D *fs_points = new vtFeatureSetPoint2D;
	int f_layer = fs_points->AddField("Layer", FT_String, 40);
	int f_color = fs_points->AddField("Color", FT_String, 16);
	int f_label = fs_points->AddField("Label", FT_String, 80);

	vtFeatureSetLineString *fs_polylines = new vtFeatureSetLineString;
	fs_polylines->AddField("Layer", FT_String, 40);
	fs_polylines->AddField("Color", FT_String, 16);

	vtFeatureSet *set;
	for (uint i = 0; i < entities.size(); i++)
	{
		const DxfEntity &ent = entities[i];
		int record = -1;
		if (ent.m_iType == DET_Point)
		{
			set = fs_points;
			record = fs_points->AddPoint(DPoint2(ent.m_points[0].x, ent.m_points[0].y));
		}
		if (ent.m_iType == DET_Polyline)
		{
			DLine2 dline;
			int NumVerts = (int)ent.m_points.size();
			dline.SetSize(NumVerts);
			for (int j = 0; j < NumVerts; j++)
			{
				dline[j].Set(ent.m_points[j].x, ent.m_points[j].y);
			}
			set = fs_polylines;
			record = fs_polylines->AddPolyLine(dline);
		}
		if (record != -1)
		{
			DxfLayer &dlay = layers[ent.m_iLayer];
			set->SetValue(record, f_layer, dlay.m_name);

			RGBf c = dlay.m_color;
			vtString str;
			str.Format("%.2f %.2f %.2f", c.r, c.g, c.b);
			set->SetValue(record, f_color, str);

			if (ent.m_label != "")
				set->SetValue(record, f_label, ent.m_label);
		}
	}

	if (fs_points->NumEntities() > 0)
	{
		vtRawLayer *pRL = new vtRawLayer;
		pRL->SetFeatureSet(fs_points);
		pRL->SetLayerFilename(_T("points.shp"));
		pRL->SetModified(true);
		// Assume existing CRS
		pRL->SetCRS(m_crs);
		if (!AddLayerWithCheck(pRL, true))
			delete pRL;
	}
	else
		delete fs_points;

	if (fs_polylines->NumEntities() > 0)
	{
		vtRawLayer *pRL = new vtRawLayer;
		pRL->SetFeatureSet(fs_polylines);
		pRL->SetLayerFilename(_T("polylines.shp"));
		pRL->SetModified(true);
		// Assume existing CRS
		pRL->SetCRS(m_crs);
		if (!AddLayerWithCheck(pRL, true))
			delete pRL;
	}
	else
		delete fs_polylines;

	return true;
}

