//
// SkyDome.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTLIB_SKYDOMEH
#define VTLIB_SKYDOMEH

#include <vtdata/vtTime.h>

/** \addtogroup terrain */
/*@{*/

/**
 * A StarDome is a sphere of stars, based on real star data, implemented as
 * Points.  The intended use is to make it large and far away from the camera,
 * so that it is always behind all the terrain and objects in the world.
 * It also contains an image of the Moon (as a texture billboard).  The Moon
 * and Stars are moved appropriately for the time (set with SetTime).
 */
class vtStarDome : public vtGroup
{
public:
	vtStarDome();
	~vtStarDome();

	void	Create(const char *starfile, float brightness,
					const char *moon_texture = NULL);

	void	SetTime(const vtTime &time);
	void	SetStarFile(char *starpath);

	struct Star
	{
		float	ra;		// Right Ascension
		float	dec;	// Declination
		float	mag;	// Magnitude
		float	relmag;
		FPoint3 cartpt;	// Cartesian coordinate
	};

private:
	vtGeode		*m_pStarGeom;
	vtMesh		*m_pStarMesh;
	vtMovGeode	*m_pMoonGeom;
	vtMaterialArrayPtr m_pMats;
	ImagePtr	 m_pMoonImage;

	vtTime	m_time;
	float	RelativeBrightness;

	/// Star array and the number of stars in the array
	int		NumStars;
	Star	*Starfield;
	char	StarFile[100];
	float	HighMag, LowMag, MagRange;

	/// Read star data from file - returns number of stars read into array
	bool	ReadStarData(const char *starfile);
	void	ConvertStarCoord(Star *star);

	void	AddStars(vtMesh *geo);
	void	AddConstellation(vtMesh *geo);
	void	FadeStars();

};

/**
 * A SkyDome is a hemisphere which is colored according to the time of day:
 * shades of blue, plus yellow-orange at dawn and dusk.  The intended use is
 * to make it large and far away from the camera, so that it is always behind
 * all the terrain and objects in the world.  It also contains an image of
 * the Sun (as a texture billboard).  The Sun is moved and sky is colored
 * appropriately for the time of day (set with SetTime).  It also supplies a
 * real Light (vtLightSource) which approximates the actual color, direction and
 * intensity of sunlight.
 */
class vtSkyDome : public vtTransform
{
public:
	vtSkyDome();
	~vtSkyDome();

	void	Create(const char *starfile, int depth, float radius,
					const char *sun_texture = NULL, const char *moon_texture = NULL);

	void	SetGeoLocation(const DPoint2 &geo) { m_geo = geo; }
	void	SetTime(const vtTime &time);
	void	SetDayColors(const RGBf &horizon, const RGBf &azimuth);
	void	SetSunsetColor(const RGBf &sunset);
	void	SetInterpCutoff(float cutoff);
	void	SetSunLight(vtTransform *light) { m_pSunLight = light; }
	void	SetSunLightSource(vtLightSource *ls) { m_pSunLightSource = ls; }
	bool	SetTexture(const char *filename);
	void	SetStarAltitude(float fDegrees) { m_fStarAltitude = fDegrees; }
	void	RefreshCelestialObjects();

	void	ShowMarkers(bool bShow);
	bool	MarkersShown();

protected:
	void	CreateMarkers();
	void	UpdateSunLight();
	void	ApplyDomeColors();
	void	ConvertVertices();

	DPoint2		m_geo;		// The earth location in lon-lat
	vtTime		m_time;		// Local time at this location

	float	m_fSunAlt, m_fSunAzi;	// in Degrees
	float	m_fStarAltitude;	// Cutoff for when stars should be displayed

	vtStarDome	*m_pStarDome;
	vtTransform	*m_pSunLight;
	vtLightSource *m_pSunLightSource;

	// day dome colors (when not textured)
	RGBf	DayHorizonCol, DayAzimuthCol, SunsetCol;
	float	Cutoff;

	int		NumVertices;
	FPoint3	*SphVertices;

	vtTransform		*m_pCelestial;
	vtGeode			*m_pDomeGeom;

	vtMaterialArrayPtr m_pMats;
	vtMaterial		*m_pMat;
	vtMaterial		*m_pTextureMat;
	vtMesh			*m_pDomeMesh;

	vtMovGeode		*m_pSunGeom;
	vtMaterial		*m_pSunMat;
	ImagePtr		 m_pSunImage;

	bool		m_bHasTexture;

	// Test markers:
	vtGeode			*m_pTicks;
	vtGeode			*m_pWireSphere;
	vtTransform		*m_pGreenMarker;
	vtTransform		*m_pRedMarker;
};

// Helper: creates a movable cross-hair for pointing out something on the sky
vtTransform *CreateMarker(vtMaterialArray *pMats, const RGBf &color);

/*@}*/  // terrain

#endif	// VTLIB_SKYDOMEH

