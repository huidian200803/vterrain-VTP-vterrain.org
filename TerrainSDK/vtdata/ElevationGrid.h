//
// vtElevationGrid.h
//
// Copyright (c) 2001-2013 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#ifndef ELEVATIONGRIDH
#define ELEVATIONGRIDH

#include "MathTypes.h"
#include "vtCRS.h"
#include "LocalCS.h"
#include "HeightField.h"
#include "vtString.h"

class GDALDataset;

/**
 * The vtElevationGrid class represents a generic grid of elevation data.
 * It supports reading and writing the data from many file formats, testing
 * the height at any given point, reprojecting the grid between coordinate
 * systems, and many other operations.
 *
 * Height elements ("heixels") can be either integer (2 bytes) or floating
 * point (4 bytes).  Heixel values are always in meters.  Areas of unknown
 * elevation are represented by the value INVALID_ELEVATION.
 *
 * To load a grid from a file, first create an empty grid, then call the
 * appropriated Load method.
 */
class vtElevationGrid : public vtHeightFieldGrid3d
{
public:
	vtElevationGrid();
	vtElevationGrid(const vtElevationGrid &rhs);
	vtElevationGrid(const DRECT &area, const IPoint2 &size, bool bFloat,
		const vtCRS &crs);
	virtual ~vtElevationGrid();

	vtElevationGrid &operator=(const vtElevationGrid &rhs);

	bool CopyFrom(const vtElevationGrid &rhs);
	bool CopyHeaderFrom(const vtElevationGrid &rhs);
	bool CopyDataFrom(const vtElevationGrid &rhs);

	bool Create(const DRECT &area, const IPoint2 &size, bool bFloat,
		const vtCRS &crs, vtElevError *err = NULL);
	void FreeData();

	void Clear();
	void Invalidate();
	bool ConvertCRS(vtElevationGrid *pOld, const vtCRS &NewProj,
		float bUpgradeToFloat, bool progress_callback(int) = NULL,
		vtElevError *err = NULL);
	bool ReprojectExtents(const vtCRS &crs_new);
	void Scale(float fScale, bool bDirect, bool bRecomputeExtents = true);
	void VertOffset(float fAmount);
	void ComputeHeightExtents();
	void Offset(const DPoint2 &delta);
	int ReplaceValue(float value1, float value2);
	bool FillGaps(DRECT *area = NULL, bool progress_callback(int) = NULL);
	bool FillGapsSmooth(DRECT *area = NULL, bool progress_callback(int) = NULL);

	int FillGapsByRegionGrowing(int radius_start=2, int radius_stop=5, bool progress_callback(int) = NULL);
	int FillGapsByRegionGrowing(int radius, bool progress_callback(int) = NULL);

	// Load from unknown file format
	bool LoadFromFile( const char *szFileName, bool progress_callback(int) = NULL,
		vtElevError *err = NULL);

	// Load from a specific kind of file
	bool LoadFrom3TX(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadFromASC(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadFromDEM(const char *szFileName, bool progress_callback(int) = NULL, vtElevError *err = NULL);
	bool LoadFromTerragen(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadFromDTED(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadFromGTOPO30(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadFromGLOBE(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadFromGRD(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadFromDSAA(const char* szFileName, bool progress_callback(int) = NULL);
	bool LoadFromPGM(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadFromRAW(const char *szFileName, int width,	int height,
		int bytes_per_element, float vertical_units, bool bBigEndian,
		bool progress_callback(int));
	bool LoadFromMicroDEM(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadFromXYZ(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadFromXYZ(FILE *fp, const char *format, bool progress_callback(int) = NULL);
	bool LoadFromHGT(const char *szFileName, bool progress_callback(int) = NULL);
	bool LoadFromBT(const char *szFileName, bool progress_callback(int) = NULL,
		vtElevError *err = NULL);
	bool LoadBTHeader(const char *szFileName, vtElevError *err = NULL);
	bool LoadBTData(const char *szFileName, bool progress_callback(int) = NULL,
		vtElevError *err = NULL);

	// Use GDAL to read a file
	bool LoadWithGDAL(const char *szFileName, bool progress_callback(int) = NULL,
		vtElevError *err = NULL);

	// Use OGR to read a file
	bool LoadFromNTF5(const char *szFileName, bool progress_callback(int) = NULL);

	// Save
	bool SaveTo3TX(const char *szFileName, bool progress_callback(int) = NULL) const;
	bool SaveToGeoTIFF(const char *szFileName) const;
	bool SaveToTerragen(const char *szFileName) const;
	bool SaveToBT(const char *szFileName, bool progress_callback(int) = NULL, bool bGZip = false);
	bool SaveToSTM(const char *szFileName, bool progress_callback(int) = NULL);
	bool SaveToPlanet(const char *szDirName, bool progress_callback(int) = NULL);
	bool SaveToASC(const char *szFileName, bool progress_callback(int) = NULL) const;
	bool SaveToVRML(const char *szFileName, bool progress_callback(int) = NULL) const;
	bool SaveToXYZ(const char *szFileName, bool progress_callback(int) = NULL) const;
	bool SaveToRAWINF(const char *szFileName, bool progress_callback(int) = NULL) const;
	bool SaveToRAW_Unity(const char *szFileName, bool progress_callback(int) = NULL) const;
	bool SaveToPNG16(const char *fname);

	// Set/Get height values
	void  SetFValue(int i, int j, float value);
	void  SetValue(int i, int j, short value);
	short GetShortValue(int i, int j) const;	// returns height value as a short integer
	float GetFValue(int i, int j) const;		// returns height value as a float
	float GetFValueSafe(int i, int j) const;

	float GetClosestValue(const DPoint2 &p) const;
	float GetFilteredValue(const DPoint2 &p) const;

	// Accessors
	/** Return the embedded name of the DEM is it has one */
	const char *GetDEMName()	{ return m_strOriginalDEMName; }

	/** Returns the geographic extents of the *area* covered by grid. */
	DRECT GetAreaExtents() const;

	/** Get the data size of the grid heixels (height elements): \c true if
	 * floating point (4-byte), \c false if integer (2-byte).
	 */
	bool  IsFloatMode()	const { return m_bFloatMode; }

	void FillWithSingleValue(float fValue);
	void GetEarthPoint(int i, int j, DPoint2 &p) const;
	void GetEarthLocation(int i, int j, DPoint3 &loc) const;

	vtCRS &GetCRS() { return m_crs; }
	const vtCRS &GetCRS() const { return m_crs; }
	void SetCRS(const vtCRS &crs);

	bool GetCorners(DLine2 &line, bool bGeo) const;
	void SetCorners(const DLine2 &line);

	short *GetData() { return m_pData; }
	float *GetFloatData() { return m_pFData; }

	const short *GetData()	  const { return m_pData;  }
	const float *GetFloatData() const { return m_pFData; }

	void SetScale(float sc) { m_fVMeters = sc; }
	float GetScale() const { return m_fVMeters; }

	bool HasData() const { return (m_pData != NULL || m_pFData != NULL); }
	int MemoryNeededToLoad() const { return m_iSize.x * m_iSize.x * (m_bFloatMode ? 4 : 2); }
	int MemoryUsed() const { if (m_pData) return m_iSize.x * m_iSize.x * 2;
						 else if (m_pFData) return m_iSize.x * m_iSize.x * 4;
						 else return 0; }

	// Implement vtHeightField methods
	bool FindAltitudeOnEarth(const DPoint2 &p, float &fAltitude, bool bTrue = false) const;

	// Implement vtHeightField3d methods
	virtual float GetElevation(int iX, int iZ, bool bTrue = false) const;
	virtual void GetWorldLocation(int i, int j, FPoint3 &loc, bool bTrue = false) const;

	// methods that deal with world coordinates
	void SetupLocalCS(float fVerticalExag = 1.0f);
	float GetWorldValue(int i, int j, bool bTrue = false) const;
	bool FindAltitudeAtPoint(const FPoint3 &p3, float &fAltitude,
		bool bTrue = false, int iCultureFlags = 0,
		FPoint3 *vNormal = NULL) const;

protected:
	bool	m_bFloatMode;
	short	*m_pData;
	float	*m_pFData;
	float	m_fVMeters;	// scale factor to convert stored heights to meters
	float	m_fVerticalScale;

	void SetupMembers();
	void ComputeExtentsFromCorners();
	void ComputeCornersFromExtents();
	bool ParseNTF5(GDALDataset *pDatasource, vtString &msg, bool progress_callback(int));
	bool GetXYZLine(const char *buf, const char *pattern, const char *format,
					int components, double *x, double *y, double *z);
	void SetError(vtElevError *err, vtElevError::ErrorType type, const char *szFormat, ...);

	DPoint2	m_Corners[4];	// data corners, in the CRS of this terrain
	vtCRS	m_crs;		// a grid always has some CRS

	bool	AllocateGrid(vtElevError *err = NULL);
	vtString	m_strOriginalDEMName;
};

#endif	// ELEVATIONGRIDH

