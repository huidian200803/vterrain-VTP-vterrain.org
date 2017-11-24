//
// SkyDome - a simple day/night skydome, should be replaced with a
// more realistic version at some point.
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/vtLog.h"
#include "vtdata/SPA.h"
#include "vtdata/FilePath.h"
#include "SkyDome.h"
#include "GeomUtil.h"	// for CreateBoundSphereGeode

// minimum and maximum ambient light values
const float MIN_AMB = 0.0f;
const float MAX_AMB = 1.0f;
const float AMB_RANGE = MAX_AMB-MIN_AMB;

// maximum directional light intensity
const float MAX_INT = 1.0f;

// only show stars up to this magnitude
const float MAX_MAGNITUDE = 5.8f;

#define SKYDOME_SMOOTHNESS 16

/**
 * Sunrise and sunset cause a warm-colored circular glow at the point where
 * the sun is touching the horizon.  This function sets the radius of that
 * circle.
 *
 * The radius angle in radians.  A typical value is 0.5 (around 28 degrees).
 */
const float MaxSunsetAngle = 0.5f;

// Night isn't completely black
const float NITE_GLO = 0.15f;

// Radians <-> Degrees Macros, single angle
inline float RAD_TO_DEG(float x){return (x * (180.0f/PIf));}
inline float DEG_TO_RAD(float x){return (x * (PIf/180.0f));}

// Coordinate change macros for a point class
// Here x = rho, y = theta, z = phi
#define rho		x
#define theta	y
#define phi		z

inline void PT_CART_TO_SPHERE(const FPoint3& A, FPoint3& B) {
	B.rho = A.Length();
	B.theta = atan2f(-A.z, A.x);
	B.phi = atan2f(A.y, sqrtf(A.x*A.x + A.z*A.z));
}

inline void PT_SPHERE_TO_CART(const FPoint3& A, FPoint3& B) {
	B.x = A.rho * cosf(A.phi) * sinf(A.theta);
	B.y = A.rho * cosf(A.phi) * cosf(A.theta);
	B.z = -A.rho * sinf(A.phi);
}


// Helpers

vtTransform *CreateMarker(vtMaterialArray *pMats, const RGBf &color)
{
	// Now make a green marker, directly north
	int matidx = pMats->AddRGBMaterial(color, false, false, true);
	FPoint3 p;
	vtMesh *mesh = new vtMesh(osg::PrimitiveSet::LINES, 0, 500);
	mesh->AddVertex(FPoint3(-0.07, 0, -0.94));
	mesh->AddVertex(FPoint3( 0.07, 0, -0.94));
	mesh->AddVertex(FPoint3(0, -0.07, -0.94));
	mesh->AddVertex(FPoint3(0,  0.07, -0.94));
	mesh->AddLine(0, 1);
	mesh->AddLine(2, 3);
	vtGeode *geode = new vtGeode;
	geode->setName("sky marker");
	geode->SetMaterials(pMats);
	geode->AddMesh(mesh, matidx);
	vtTransform *trans = new vtTransform;
	trans->addChild(geode);
	return trans;
}
void PlaceMarker(vtTransform *trans, float alt, float azi)
{
	trans->Identity();
	trans->Rotate(FPoint3(1,0,0), DEG_TO_RAD(alt));
	trans->RotateParent(FPoint3(0,1,0), DEG_TO_RAD(-azi));
}


///////////////////////////////////////////////////////////////////////////
// SkyDome
//

vtSkyDome::vtSkyDome()
{
	m_pStarDome = NULL;
	m_pSunLight = NULL;
	m_pSunLightSource = NULL;

	m_pMats = NULL;
	m_pMat = NULL;
	m_pTextureMat = NULL;
	m_pDomeGeom = NULL;
	m_pDomeMesh = NULL;
	m_pSunGeom = NULL;
	m_pSunMat = NULL;
	m_pSunImage = NULL;
	SphVertices = NULL;
	m_bHasTexture = false;

	m_fStarAltitude = -5;	// sun is well below the horizon
}

vtSkyDome::~vtSkyDome()
{
	if (SphVertices)
		delete[] SphVertices;
}

/**
 * Creates a complete SkyDome, which includes a StarDome.
 */
void vtSkyDome::Create(const char *starfile, int depth, float radius,
					 const char *sun_texture, const char *moon_texture)
{
	VTLOG("  vtSkyDome::Create\n");
	setName("SkyDome");

	VTLOG("   Creating Dome Nodes\n");
	m_pCelestial = new vtTransform;
	m_pCelestial->setName("Celestial Sphere");
	addChild(m_pCelestial);

	m_pDomeGeom = new vtGeode;
	m_pDomeGeom->setName("SkyDomeGeom");
	addChild(m_pDomeGeom);		// dome geometry does not rotate

	VTLOG("   Creating Dome Materials\n");
	m_pMats = new vtMaterialArray;
	m_pDomeGeom->SetMaterials(m_pMats);

	// Only a single material is needed for the untextured dome, since vertex
	//  colors are used to change the color of the sky.
	m_pMat = new vtMaterial;
	m_pMat->SetLighting(false);
	m_pMat->SetCulling(false);	// visible from the inside as well as outside
	m_pMats->push_back(m_pMat);

	// Create the geometry of the dome itself
	VTLOG("   Creating Dome Mesh\n");
	int res = SKYDOME_SMOOTHNESS;
	int vertices = (res*2+1)*(res/2+1);

	m_pDomeMesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_STRIP, VT_Colors | VT_TexCoords, vertices);

	// Face normals point in, not out
	bool bNormalsIn = false;
	bool bHemisphere = true;

	m_pDomeMesh->CreateEllipsoid(FPoint3(0,0,0), FPoint3(1.0f, 1.0f, 1.0f),
		res, bHemisphere, bNormalsIn);

	m_pDomeGeom->AddMesh(m_pDomeMesh, 0);

	// Extra graphics on the dome, to help with development and testing.
	VTLOG("   Creating Markers\n");
	CreateMarkers();
	ShowMarkers(false);

	NumVertices = m_pDomeMesh->NumVertices();
	SphVertices = new FPoint3[NumVertices];
	ConvertVertices();

	Scale(radius);

	// Set default horizon, azimuth and sunset colors
	DayHorizonCol.Set(0.5f, 1.0f, 1.0f);
	DayAzimuthCol.Set(0.3f, 0.3f, 0.9f);
	SunsetCol.Set(1.0f, 0.55f, 0.0f);

	// Set default Interpolation cutoff
	SetInterpCutoff(0.3f);

	if (sun_texture && *sun_texture)
	{
		VTLOG1("   Loading Sun Image.. ");
		m_pSunImage = osgDB::readImageFile(sun_texture);
		if (m_pSunImage.valid()) {
			VTLOG("succeeded.\n");

			int idx = m_pMats->AddTextureMaterial(m_pSunImage,
								 false, false,	// culling, lighting
								 true, true,	// transp, additive
								 1.0f,			// diffuse
								 1.0f, 1.0f);	// alpha, emmisive

			// Create sun
			m_pSunMat = m_pMats->at(idx);

			VTLOG("   Creating Sun Geom\n");
			vtGeode *pGeode = new vtGeode;
			pGeode->setName("Sun geom");
			m_pSunGeom = new vtMovGeode(pGeode);
			m_pSunGeom->setName("Sun xform");

			VTLOG("   Creating Sun Mesh\n");
			vtMesh *SunMesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_FAN, VT_TexCoords, 4);

			SunMesh->AddRectangleXZ(0.50f, 0.50f);
			pGeode->SetMaterials(m_pMats);
			pGeode->AddMesh(SunMesh, idx);

			// Z translation, to face us at the topographic north (horizon)
			FMatrix4 trans;
			trans.Identity();
			trans.Translate(FPoint3(0.0f, 0.90f, 0.0f));
			SunMesh->TransformVertices(trans);
			trans.Identity();
			trans.AxisAngle(FPoint3(1,0,0), -PID2f);
			SunMesh->TransformVertices(trans);

			// The sun is attached to the celestial sphere which rotates
			m_pCelestial->addChild(m_pSunGeom);
		}
		else
			VTLOG("failed.\n");
	}

	// Create the vtStarDome
	if (starfile && *starfile)
	{
		m_pStarDome = new vtStarDome;
		m_pStarDome->Create(starfile, 2.0f, moon_texture);
		m_pStarDome->setName("StarDome");
		m_pCelestial->addChild(m_pStarDome);
	}

	// Default untextured sky colors
	RGBf horizon_color(0.70f, 0.85f, 1.0f);
	RGBf azimuth_color(0.12f, 0.32f, 0.70f);
	SetDayColors(horizon_color, azimuth_color);
}

void vtSkyDome::CreateMarkers()
{
	// First create some 5-degree tic marks.
	int yellow = m_pMats->AddRGBMaterial(RGBf(1,1,0), false, false, true);
	FPoint3 p;
	vtMesh *tics = new vtMesh(osg::PrimitiveSet::LINES, 0, (36+1)*2*2);
	for (float t = 0; t < PId; t += (PId / 36))	// 5 degree increment
	{
		float sint = sinf(t) * 0.95f;
		float cost = cosf(t) * 0.95f;
		tics->AddLine(FPoint3(-0.05, sint, cost), FPoint3(0.05, sint, cost));
		tics->AddLine(FPoint3(cost, sint, -0.05), FPoint3(cost, sint, 0.05));
	}
	m_pTicks = new vtGeode;
	m_pTicks->setName("Ticks");
	m_pTicks->SetMaterials(m_pMats);
	m_pTicks->AddMesh(tics, yellow);
	addChild(m_pTicks);

	// Put green marker on alt-axi location of sun.
	m_pGreenMarker = CreateMarker(m_pMats, RGBf(0,1,0));
	m_pGreenMarker->setName("Green Marker");
	addChild(m_pGreenMarker);

	// Put red marker on the sun's position on the celestial sphere.
	m_pRedMarker = CreateMarker(m_pMats, RGBf(1,0,0));
	m_pRedMarker->setName("Red Marker");
	m_pCelestial->addChild(m_pRedMarker);

	// Create celestial sphere wifreframe, to aid in development and testing
	FSphere sph(FPoint3(0,0,0), 0.99);
	m_pWireSphere = CreateBoundSphereGeode(sph, 60);
	m_pWireSphere->setName("Celestial Sphere wireframe");
	m_pCelestial->addChild(m_pWireSphere);
}

void vtSkyDome::ShowMarkers(bool bShow)
{
	VTLOG("SkyDome ShowMarkers(%d)\n", bShow);
	m_pTicks->SetEnabled(bShow);
	m_pGreenMarker->SetEnabled(bShow);
	m_pRedMarker->SetEnabled(bShow);
	m_pWireSphere->SetEnabled(bShow);
}

bool vtSkyDome::MarkersShown()
{
	return m_pTicks->GetEnabled();
}


/**
 * Sets the time of day (or night).
 * \param time			Time in seconds since midnight.
 */
void vtSkyDome::SetTime(const vtTime &time)
{
	m_time = time;

	// Pass along time to the vtStarDome, for it to position the moon
	if (m_pStarDome)
		m_pStarDome->SetTime(time);

	RefreshCelestialObjects();
}

void vtSkyDome::RefreshCelestialObjects()
{
	DPoint2 geo = m_geo;

#if 0
	// TEST with fake time and place
	m_time.SetDate(2000, 3, 21);	// roughly vernal equinox
	m_time.SetTimeOfDay(12, 0, 0);	// high noon
	geo.Set(15, 45);	 // On the prime meridian, just west of Bordeaux France
#endif

	// Determine the Sun's location in the celestial sphere
	int year, month, day, hour, minute, second;
	m_time.GetDate(year, month, day);
	m_time.GetTimeOfDay(hour, minute, second);

	// Timezone: for now, just convert degrees to hours.  This gives 'correct'
	//  results, but is actually "Local Time", as opposed to what most people
	//  probably expect, which is "Standard Time", usually defined by an
	//  integral number of hours offset from universal time.  We would have to
	//  either ask the user for this, or guess it using a complicated political
	//  map of time zones in the world.
	float timezone = (float)geo.x / 15;

	// sealevel for now
	float elevation = 0;

	spa_data spa;
	SetCommonValues(spa, (float) geo.x, (float) geo.y,
			   year, month, day, hour, minute, second,
			   timezone, elevation);

	// sun location relative to this spot on earth ("topocentric")
	m_fSunAlt = (float) spa.altitude;
	m_fSunAzi = (float) spa.azimuth;

	// Sun location in the celestial sphere ("geocentric")
	float ra = (float) spa.alpha;
	float dec = (float) spa.delta;

	// Set the correct transformation of the celestial sphere for the
	//  location on earth and current time.
	m_pCelestial->Identity();

	float latitude_in_radians = (float) (geo.y / 180.0f * PIf);
	m_pCelestial->RotateLocal(FPoint3(1,0,0), latitude_in_radians);

	float gst = (float) spa.nu;		// GST = Greenwich Sidereal Time, degrees
	float lst = gst + (float) geo.x;// LST = Local Sideral Time

//	VTLOG("YMD %d %d %d, HMS %02d:%02d:%02d, RA/DEC %.0f %.0f, GST %.0f, LST %.0f\n",
//		year, month, day, hour, minute, second, ra, dec, gst, lst);

	float dec_of_sphere = DEG_TO_RAD(lst);
	m_pCelestial->RotateLocal(FPoint3(0,0,1), dec_of_sphere);

	// Place the sun geometry on the celestial sphere
	if (m_pSunGeom)
	{
		m_pSunGeom->Identity();
		m_pSunGeom->Rotate(FPoint3(1,0,0), DEG_TO_RAD(90 - dec));
		m_pSunGeom->RotateParent(FPoint3(0,0,1), DEG_TO_RAD(-ra));
	}

	// Determine if the stardome is active according to time of day
	if (m_pStarDome)
	{
		if (m_fSunAlt < m_fStarAltitude)
			m_pStarDome->SetEnabled(true);
		else
			m_pStarDome->SetEnabled(false);
	}

	// set the direction and intensity of the sunlight
	if (m_pSunLight != NULL)
		UpdateSunLight();

	ApplyDomeColors();

	// Markers:
	// Put green marker where SPA tells us the alt-azi sun should go
	PlaceMarker(m_pGreenMarker, m_fSunAlt, m_fSunAzi);

	// Put red marker where SPA tells us the ra-dec sun should go
	m_pRedMarker->Identity();
	m_pRedMarker->Rotate(FPoint3(1,0,0), DEG_TO_RAD(90 - dec));
	m_pRedMarker->RotateParent(FPoint3(0,0,1), DEG_TO_RAD(-ra));
}


void vtSkyDome::UpdateSunLight()
{
	// Point the actual sun light, such that is it coming from the sun
	//  that we draw in the sky.
	m_pSunLight->Identity();

	// First rotate by 180 degrees because OpenGL lights default to
	//  facing 'north', but alt-azi here assumes the default position is
	//  _from_ the north at the horizon, facing us.
	m_pSunLight->Rotate(FPoint3(1,0,0), PId);
	m_pSunLight->Rotate(FPoint3(1,0,0), DEG_TO_RAD(m_fSunAlt));
	m_pSunLight->Rotate(FPoint3(0,1,0), DEG_TO_RAD(-m_fSunAzi));

//	FPoint3 dir = m_pSunLight->GetDirection();
//	VTLOG("  Alt-Azi %.3f %.3f, Light dir: %.3f %.3f %.3f\n", m_fSunAlt, m_fSunAzi, dir.x, dir.y, dir.z);

	float ambient = 0.0f;

	// set intensity of ambient light based on time of day
	if (m_fSunAlt < -5)
	{
		// night
		ambient = MIN_AMB;
	}
	else if (m_fSunAlt >= -5 && m_fSunAlt <= 5)
	{
		// dawn / dusk
		ambient = MIN_AMB + AMB_RANGE * (m_fSunAlt + 5) / (10);
	}
	else if (m_fSunAlt >= 5)
	{
		// day
		ambient = MAX_AMB;
	}

	float intensity = 0.0f;

	RGBf white(1.0f, 1.0f, 1.0f);
	RGBf yellow(1.0f, 0.6f, 0.4f);
	RGBf color;
	float fraction;

	// set intensity of sunlight based on whether it is over the horizon
	if (m_fSunAlt < -2)
	{
		// night
		color = yellow;
		intensity = 0.0f;
	}
	else if (m_fSunAlt >= -2 && m_fSunAlt < 2)
	{
		// dawn / dusk
		fraction = (m_fSunAlt + 2) / 4;
		color = yellow + ((white - yellow) * fraction);
		intensity = MAX_INT * fraction;
	}
	else if (m_fSunAlt >= 2)
	{
		// day
		color = white;
		intensity = MAX_INT;
	}

	color *= intensity;

	m_pSunLightSource->SetDiffuse(color);
	m_pSunLightSource->SetAmbient(RGBf(ambient, ambient, ambient));
	m_pSunLightSource->SetSpecular(color);

	// Don't actually color the sun geometry, because we use a sun texture now.
	// if (m_pSunMat) m_pSunMat->SetDiffuse(color);
}

//
//
void vtSkyDome::ConvertVertices()
{
	FPoint3 p, psph;

	int num = m_pDomeMesh->NumVertices();
	for (int i = 0; i < num; i++)
	{
		p = m_pDomeMesh->GetVtxPos(i);
		PT_CART_TO_SPHERE(p, psph);
		SphVertices[i] = psph;
	}
}

//
//
void vtSkyDome::SetDayColors(const RGBf &horizon, const RGBf &azimuth)
{
	DayHorizonCol = horizon;
	DayAzimuthCol = azimuth;

	ApplyDomeColors();
}

//
//
void vtSkyDome::SetSunsetColor(const RGBf &sunset)
{
	SunsetCol = sunset;
}

//
//
void vtSkyDome::SetInterpCutoff(float cutoff)
{
	Cutoff = cutoff;
}

//
//
bool vtSkyDome::SetTexture(const char *filename)
{
	if (m_pTextureMat)
	{
		// if it hasn't changed, return
		osg::Image *image = m_pTextureMat->GetTextureImage();
		if (filename && image->getFileName() == filename)
			return true;

		// Already textured; remove previous material
		m_pMats->RemoveMaterial(m_pTextureMat);
		m_pTextureMat = NULL;
	}

	if (!filename)
	{
		if (m_bHasTexture)
		{
			VTLOG("   SkyDome: Setting to no Texture.\n");
			// Go back to vertex-coloured dome
			m_bHasTexture = false;

			int index = m_pMats->Find(m_pMat);
			m_pDomeGeom->SetMeshMatIndex(m_pDomeMesh, index);
			ApplyDomeColors();
		}
		return true;
	}

	VTLOG("   SkyDome: Set Texture to '%s'.. ", filename);
	ImagePtr pImage = osgDB::readImageFile(filename);
	if (!pImage.valid())
	{
		VTLOG("failed.\n");
		return false;
	}
	VTLOG("loaded OK.\n");

	VTLOG("    Image is %d x %d, depth %d\n", GetWidth(pImage),
		GetHeight(pImage), GetDepth(pImage));

	// create and apply the texture material
	int index = m_pMats->AddTextureMaterial(pImage, false, false);

	m_pTextureMat = m_pMats->at(index);

	// set the vertices to initially white
	int verts = m_pDomeMesh->NumVertices();
	for (int i = 0; i < verts; i++)
		m_pDomeMesh->SetVtxColor(i, RGBf(1,1,1));	// all white vertices

	m_pDomeGeom->SetMeshMatIndex(m_pDomeMesh, index);
	m_pDomeMesh->ReOptimize();
	m_bHasTexture = true;

	return true;
}

//
//
void vtSkyDome::ApplyDomeColors()
{
	vtMesh *mesh = m_pDomeMesh;
	RGBf vtxcol;
	FPoint3 psph;
	float phipct, phipct_cut, thetapct;
	float sunpct;
	float fademod;

	if (m_fSunAlt >= -5 && m_fSunAlt <= 5)
	{
		// dawn
		fademod = NITE_GLO + (1.0f - NITE_GLO) * (float)(m_fSunAlt + 5)/10;
	}
	else if (m_fSunAlt < -5)
	{
		fademod = NITE_GLO;	// night
	}
	else
		fademod = 1.0f;	// day

	// Don't actually change the dome color if it already has a texture
	if (m_bHasTexture)
		return;

	// Set day colors
	for (uint i = 0; i < mesh->NumVertices(); i++)
	{
		FPoint3 p = mesh->GetVtxPos(i);
		psph = SphVertices[i];

		phipct = fabsf(PID2f - fabsf(psph.phi))/PID2f;
		thetapct = (PID2f - fabsf(psph.theta))/PID2f;

		// Ordering of color layering is important!!!
		// Color is interpolated along phi from 0 to Cutoff
		if ((1.0f - phipct) <= Cutoff)
		{
			phipct_cut = (1.0f - phipct)/Cutoff;
			vtxcol = (DayHorizonCol * (1.0f - phipct_cut)) + (DayAzimuthCol * phipct_cut);
		}
		else
		{
			vtxcol = DayAzimuthCol;
		}
		vtxcol *= fademod;
#if 1
		// Sunrise/sunset glow
		if (m_fSunAlt >= -5 && m_fSunAlt <= 5)
		{
			float midseqpct = fabsf(m_fSunAlt)/5;

			// Hack
			if (m_fSunAzi > 180)
			{
				// sunset
				if (((1.0f - phipct) <= MaxSunsetAngle) && (fabsf(psph.theta) > PID2f))
				{
					phipct_cut = (1.0f - phipct)/MaxSunsetAngle;
					sunpct = (1.0f - phipct_cut) * (1.0f - midseqpct) * (-thetapct);
					vtxcol = (vtxcol * (1.0f - sunpct)) + (SunsetCol * sunpct);
				}
			}
			else
			{
				// sunrise
				if (((1.0f - phipct) <= MaxSunsetAngle) && (fabsf(psph.theta) <= PID2f))
				{
					phipct_cut = (1.0f - phipct)/MaxSunsetAngle;
					sunpct = (1.0f - phipct_cut) * (1.0f - midseqpct) * thetapct;
					vtxcol = (vtxcol * (1.0f - sunpct)) + (SunsetCol * sunpct);
				}
			}
		}
#endif
		mesh->SetVtxColor(i, vtxcol);
	}

	mesh->ReOptimize();
}


///////////////////////////////////////////////////////////////////////
//
// vtStarDome
//
vtStarDome::vtStarDome()
{
	m_pMats = NULL;
	Starfield = NULL;
	m_pMoonImage = NULL;
}

vtStarDome::~vtStarDome()
{
	if (Starfield)
		delete[] Starfield;
}

void vtStarDome::Create(const char *starfile, float brightness,
					  const char *moon_texture)
{
	VTLOG("  vtStarDome::Create\n");

	setName("StarDome");
	m_pStarGeom = new vtGeode;
	m_pStarGeom->setName("StarDomeGeom");

	NumStars = 0;
	RelativeBrightness = brightness;

	// Read in the star data
	ReadStarData(starfile);

	m_pMats = new vtMaterialArray;
	int star_mat = m_pMats->AddRGBMaterial(RGBf(0,0,0), false, false);
	vtMaterial *pMat = m_pMats->at(star_mat);
	pMat->SetTransparent(true, true);

	// Need a material?
	m_pStarGeom->SetMaterials(m_pMats);

	m_pStarMesh = new vtMesh(osg::PrimitiveSet::POINTS, VT_Colors, NumStars);
	AddStars(m_pStarMesh);
	m_pStarGeom->AddMesh(m_pStarMesh, star_mat);
	addChild(m_pStarGeom);

	if (moon_texture && *moon_texture)
	{
		int idx = -1;
		m_pMoonImage = osgDB::readImageFile(moon_texture);
		if (m_pMoonImage->valid())
		{
			idx = m_pMats->AddTextureMaterial(m_pMoonImage,
								 false, false,	// culling, lighting
								 true, true,	// transparent, additive
								 1.0f,			// diffuse
								 1.0f, 1.0f);	// alpha, emmisive
		}
		if (idx == -1)
			return;		// could not load texture, cannot have sun

		// Create moon
		vtGeode *pGeode = new vtGeode;
		pGeode->setName("Moon geom");
		m_pMoonGeom = new vtMovGeode(pGeode);
		m_pMoonGeom->setName("Moon xform");

		vtMesh *MoonMesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_FAN, VT_TexCoords, 4);

		MoonMesh->AddRectangleXZ(0.1f, 0.1f);
		pGeode->SetMaterials(m_pMats);
		pGeode->AddMesh(MoonMesh, idx);

		// Y translation
		FMatrix4 trans;
		trans.Identity();
		trans.Translate(FPoint3(0.0f, 0.90f, 0.0f));
		MoonMesh->TransformVertices(trans);

		addChild(m_pMoonGeom);
	}
}

/**
 * Sets the time of day (or night).
 * \param time			Time in seconds since midnight.
 */
void vtStarDome::SetTime(const vtTime &time)
{
	m_time = time;

	// TODO: Put the moon in the correct place on the celestial sphere

	FadeStars();
}

// Helper function to compare stars by magnitude
int	compare_mag(const void *star1, const void *star2)
{
	vtStarDome::Star *pstar1 = (vtStarDome::Star *)star1;
	vtStarDome::Star *pstar2 = (vtStarDome::Star *)star2;

	float magdiff = pstar1->mag - pstar2->mag;
	if (magdiff < 0) return -1;
	else if (magdiff > 0) return 1;
	else return 0;
}


//
// Add Stars to GeoMesh Vertex List ordered by magnitude
//
void vtStarDome::AddStars(vtMesh *mesh)
{
	FPoint3	temppt;
	RGBf	tempcol, white(1.0f, 1.0f, 1.0f);

	// Add each star to the GeoMesh's vertex array
	for (int i = 0; i < NumStars; i++)
	{
		mesh->AddVertex(Starfield[i].cartpt);
		tempcol = (Starfield[i].relmag > 1.0f) ? white : white * Starfield[i].relmag;
#if 0
		TRACE("Star #%d %f RGB(%f,%f,%f)\n", i, Starfield[i].relmag,
			tempcol.r, tempcol.g, tempcol.b);
#endif
		mesh->SetVtxColor(i, tempcol);
	}
}


/**
 * Add Constellations to GeoMesh Vertex List
 */
void vtStarDome::AddConstellation(vtMesh *mesh)
{
	// to do
}

/**
 * Determines how many vertices are need to be included in the mesh, based
 * on the time of day and when dawn and dusk are supposed to be.  This makes
 * the stars 'wink out' nicely.  (It is currently not used.)
 */
void vtStarDome::FadeStars()
{
#if 0
	int numvalidstars = 0;
	vtMesh *mesh = m_pStarMesh;

	float duskpct = (float)(m_iTimeOfDay - DuskStartTime)/DuskDuration;
	float dawnpct = (float)(DawnEndTime - m_iTimeOfDay)/DawnDuration;

	float magmod;
	if (m_iTimeOfDay >= DuskStartTime && m_iTimeOfDay <= DuskEndTime)
		magmod = duskpct;
	else if (m_iTimeOfDay >= DawnStartTime && m_iTimeOfDay <= DawnEndTime)
		magmod = dawnpct;
	else magmod = 1.00f;

	for (int i = 0; i < NumStars; i++)
	{
		numvalidstars++;
		if (Starfield[i].mag >= (HighMag + MagRange * magmod)) break;
	}
//	mesh->SetNumVertices(numvalidstars);		// TODO
#endif
}

//
// Set Star File
//
void vtStarDome::SetStarFile(char *starpath)
{
	strcpy(StarFile, starpath);
}


/**
 * Read the star data files and throw into array structures.
 */
bool vtStarDome::ReadStarData(const char *starfile)
{
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	int n, numstars = 0, num_file_stars = 0;
	float himag, lomag;
	int ra_h, ra_m, dec_d, dec_m, dec_s;
	float ra_s;

	FILE *starfp = NULL;
	Star *curstar;

	starfp = vtFileOpen(starfile, "r");
	if (!starfp)
	{
		VTLOG("Couldn't open %s\n", starfile);
		return false;
	}

	if ((n = fscanf(starfp, "%d %f %f\n", &num_file_stars,
					  &himag, &lomag)) == EOF)
	{
		VTLOG("Couldn't read number of stars from %s\n", starfile);
		return false;
	}

	Starfield = new Star[num_file_stars];

	while (numstars++ != num_file_stars)
	{
		curstar =  &Starfield[NumStars];
		n = fscanf(starfp, "%d %d %f %d %d %d %f\n", &ra_h,
			&ra_m, &ra_s, &dec_d, &dec_m, &dec_s, &curstar->mag);

		curstar->ra = (float)ra_h + (float)ra_m/60.0f + (float)ra_s/3600.0f;
		curstar->dec = (float)dec_d + (float)dec_m/60.0f + (float)dec_s/3600.0f;

		if (n == EOF)
			return false;

		ConvertStarCoord(curstar);

//		if (curstar->mag <= 3.35)	// convenient for finding the big dipper
		if (curstar->mag <= MAX_MAGNITUDE)
			NumStars++;
	}
	lomag = MAX_MAGNITUDE;

	fclose(starfp);

	// sort the stars by magnitude
	qsort((void *)Starfield, NumStars, sizeof(Star), compare_mag);

	HighMag = himag;
	LowMag = lomag;
	MagRange = fabsf(HighMag) + LowMag;

	// Pre calculate the relative magnitude percentage of each star
	for (int i = 0; i < NumStars; i++)
	{
		Starfield[i].relmag = (MagRange - (Starfield[i].mag + fabsf(HighMag)))
			/(MagRange/RelativeBrightness);
	}
	return true;
}


/**
 * Convert star coordinate information to x,y,z coordinates.
 */
void vtStarDome::ConvertStarCoord(Star *star)
{
	FPoint3 spherept;

	// Make sure the stars are drawn inside of the sky dome
	spherept.rho = 0.95f; // calculated for a unit  sphere

	// First off convert from sexagesimal to spherical notation for RA
	// Right Ascension corresponds to theta  Range: 0 - 24 hours
	spherept.theta = star->ra * (PI2f/24.0f);

	// Declination corresponds to phi Range: -90 to 90 degrees
	spherept.phi = DEG_TO_RAD(star->dec);

	PT_SPHERE_TO_CART(spherept, star->cartpt);
}
