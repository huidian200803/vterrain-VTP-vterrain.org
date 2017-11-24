//
// GDALWrapper.cpp
//

#include "GDALWrapper.h"

#include "FilePath.h"	// for FindFileOnPaths
#include "MathTypes.h"	// for ScopedLocale
#include "vtString.h"
#include "vtLog.h"
#include "vtCRS.h"	// For testing vtCRS

// GDAL
#include "gdal_priv.h"
#include "cpl_multiproc.h"	// for CPLCleanupTLS

// OGR
#include <ogrsf_frmts.h>

// sends all GDAL/OGR/PROJ.4 messages to the VTP log output
void CPL_STDCALL cpl_error_handler(CPLErr eErrClass, int err_no, const char *msg)
{
	if (eErrClass == CE_Debug)
		VTLOG1("CPL Debug: ");
	else if (eErrClass == CE_Warning)
		VTLOG("CPL Warning %d: ", err_no);
	else
		VTLOG("CPL Error %d: ", err_no);
	VTLOG1(msg);
	VTLOG1("\n");
}

// A singleton for this class
GDALWrapper g_GDALWrapper;

GDALWrapper::GDALWrapper()
{
	m_bGDALFormatsRegistered = false;
	m_bOGRFormatsRegistered = false;

	// send all GDAL/OGR/PROJ.4 messages to the VTP log output
	CPLPushErrorHandler(cpl_error_handler);

	// tell it to send all debugging messages
	CPLSetConfigOption("CPL_DEBUG", "ON");
}

GDALWrapper::~GDALWrapper()
{
	// this must be called before CPLCleanupTLS
	CPLPopErrorHandler();

	// Destroying the regsitered format drivers only needs to be done
	// once at exit.
	if (m_bGDALFormatsRegistered)
	{
		GDALDestroyDriverManager();
	}

	// Call OSRCleanup just in case, because any usage of OGR (e.g.
	// might have caused stuff to be loaded that's not unloaded unless
	// OSRCleanup is called, which is only done in the GeoTIFF driver
	// unregistering.  So this handles the situation where we've use OGR
	// but not GDAL.
	OSRCleanup();
	CPLFinderClean();
#if GDAL_VERSION_NUM >= 1310
	CPLCleanupTLS();	// this function was added in GDAL 1.3.1
	VSICleanupFileManager();
#endif

	if (m_bOGRFormatsRegistered)
	{
		// TODO: still necessary to clean up memory with GDAL 2.0?
		//OGRSFDriverRegistrar *reg = OGRSFDriverRegistrar::GetRegistrar();
		//delete reg;
	}
}

void GDALWrapper::RequestGDALFormats()
{
	if (!m_bGDALFormatsRegistered)
	{
		GDALAllRegister();
		m_bGDALFormatsRegistered = true;
	}
}

void GDALWrapper::RequestOGRFormats()
{
	if (!m_bOGRFormatsRegistered)
	{
		OGRRegisterAll();
		m_bOGRFormatsRegistered = true;
	}
}

bool GDALWrapper::Init()
{
	// check for correctly set up environment variables and locatable files
	m_initResult.hasGDAL_DATA = FindGDALData();
	m_initResult.hasPROJ_LIB = FindPROJ4Data();
	m_initResult.hasPROJSO = FindPROJ4SO();
	VTLOG("test results: GDAL_DATA %d, PROJ_LIB %d, PROJSO %d\n",
		m_initResult.hasGDAL_DATA,
		m_initResult.hasPROJ_LIB,
		m_initResult.hasPROJSO);

	return m_initResult.Success();
}

bool GDALWrapper::FindGDALData()
{
	vtStringArray dpg;

	const char *gdalenv = getenv("GDAL_DATA");
	VTLOG("getenv GDAL_DATA: '%s'\n", gdalenv ? gdalenv : "NULL");
	if (gdalenv != NULL)
		dpg.push_back(vtString(gdalenv) + "/");

	dpg.push_back(vtString(DEFAULT_LOCATION_GDAL_DATA));
#if VTUNIX
	// add the usual unix paths
	dpg.push_back(vtString("/usr/local/share/gdal/"));
#endif

	vtString pcsPath = FindFileOnPaths(dpg, "pcs.csv");
	vtString datumPath = FindFileOnPaths(dpg, "gdal_datum.csv");
	dpg.pop_back();
	if (pcsPath == vtEmptyString || datumPath == vtEmptyString)
		return false;

	if (ExtractPath(pcsPath, false) != ExtractPath(datumPath, false))
		VTLOG("Warning: multiple versions of GDAL data installed: %s and %s.\n", (const char*)pcsPath, (const char*)datumPath);

	vtString newpath = ExtractPath(datumPath, false);
	if (gdalenv == NULL || newpath != gdalenv)
		SetEnvironmentVar("GDAL_DATA", newpath);
	return true;
}

bool GDALWrapper::FindPROJ4Data()
{
	vtStringArray dpp;

	const char *proj4 = getenv("PROJ_LIB");
	VTLOG("getenv PROJ_LIB: '%s'\n", proj4 ? proj4 : "NULL");
	if (proj4 != NULL)
		dpp.push_back(vtString(proj4) + "/");

	dpp.push_back(vtString(DEFAULT_LOCATION_PROJ_LIB));
#if VTUNIX
	// add the usual unix paths
	dpp.push_back(vtString("/usr/local/share/proj/"));
#endif

	vtString fname = FindFileOnPaths(dpp, "nad83");
	FILE *fp = (fname != "") ? vtFileOpen((const char *)fname, "rb") : NULL;
	if (fp == NULL)
		return false;
	else
		fclose(fp);

	vtString newpath = ExtractPath(fname, false);
	if (proj4 == NULL || newpath != proj4)
		SetEnvironmentVar("PROJ_LIB", newpath);
	return true;
}

bool GDALWrapper::FindPROJ4SO()
{
#ifndef WIN32
	vtStringArray dpso;
    const char *proj4so = getenv("PROJSO");
    VTLOG("getenv PROJSO: '%s'\n", proj4so ? proj4so : "NULL");
    if (proj4so != NULL)
        dpso.push_back(vtString(proj4so) + "/");

	// On 64-bit Linux, 64-bit libs are sometimes in a "64" folder
#ifdef _LP64
	dpso.push_back(vtString("/usr/local/lib/x86_64-linux-gnu/"));
	dpso.push_back(vtString("/usr/local/lib64/"));
	dpso.push_back(vtString("/usr/lib/x86_64-linux-gnu/"));
	dpso.push_back(vtString("/usr/lib64/"));
#else
	dpso.push_back(vtString("/usr/local/lib/i386-linux-gnu/"));
	dpso.push_back(vtString("/usr/local/lib32/"));
	dpso.push_back(vtString("/usr/lib/i386-linux-gnu/"));
	dpso.push_back(vtString("/usr/lib32/"));
#endif
	dpso.push_back(vtString(DEFAULT_LOCATION_PROJSO));
	dpso.push_back(vtString("/usr/local/lib/"));
	dpso.push_back(vtString("/usr/lib/"));

	// On non-Windows platform, we have to look for the library itself
	vtString soExtension = ".unknown"; // for no platform.
	vtString soName = "libproj";
#if __APPLE__
	soExtension = ".dylib";
#else // other unixes
    soExtension = ".so";
#endif
	soName += soExtension;

	VTLOG("Looking for '%s' on paths, ", (const char *)soName);
	vtString fname = FindFileOnPaths(dpso, soName);
	VTLOG("found: '%s', ", (fname != "") ? (const char *)fname : "");
	FILE *fp = (fname != "") ? vtFileOpen((const char *)fname, "rb") : NULL;
	if (fp == NULL)
	{
		VTLOG("not found.\n");
		return false;
	}
	else
	{
		VTLOG("found.\n");
		fclose(fp);
	}

	CPLSetConfigOption("PROJSO", fname);
#endif	// WIN32
	return true;
}

bool GDALWrapper::TestPROJ4()
{
	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	// Now test that PROJ4 is working.
	VTLOG1("Testing ability to create coordinate transforms.\n");
	vtCRS crs1, crs2;
	crs1.SetUTM(1);
	crs2.SetUTM(2);
	ScopedOCTransform trans(CreateCoordTransform(&crs1, &crs2));
	if (trans.get())
		return true;

	return false;
}


