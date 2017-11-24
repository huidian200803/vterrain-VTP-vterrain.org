//
// GDALWrapper.h
//

#pragma once

class GDALInitResult
{
public:
	GDALInitResult() { hasGDAL_DATA = false; hasPROJ_LIB = false; hasPROJSO = false; }

	bool hasGDAL_DATA;
	bool hasPROJ_LIB;
	bool hasPROJSO;

	bool Success() { return hasGDAL_DATA && hasPROJ_LIB && hasPROJSO; }
};

class GDALWrapper
{
public:
	GDALWrapper();
	~GDALWrapper();

	void RequestGDALFormats();
	void RequestOGRFormats();
	bool Init();
	GDALInitResult* GetInitResult() { return &m_initResult; }
	bool TestPROJ4();

protected:
	bool FindGDALData();
	bool FindPROJ4Data();
	bool FindPROJ4SO();

	bool m_bGDALFormatsRegistered;
	bool m_bOGRFormatsRegistered;
	GDALInitResult m_initResult;
};

extern GDALWrapper g_GDALWrapper;

#ifdef WIN32
#define DEFAULT_LOCATION_GDAL_DATA "../../GDAL-data/"
#define DEFAULT_LOCATION_PROJ_LIB "../../PROJ4-data/"
#elif __APPLE__
#define DEFAULT_LOCATION_GDAL_DATA "Shared/share/gdal/"
#define DEFAULT_LOCATION_PROJ_LIB "Shared/share/proj/"
#define DEFAULT_LOCATION_PROJSO "Shared/lib/"
#else // other unixes
#define DEFAULT_LOCATION_GDAL_DATA "/usr/local/share/gdal/"
#define DEFAULT_LOCATION_PROJ_LIB "/usr/local/share/proj/"
#define DEFAULT_LOCATION_PROJSO "/usr/local/lib/"
#endif

