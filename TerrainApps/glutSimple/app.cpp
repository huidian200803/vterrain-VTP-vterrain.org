//
// Name:     app.cpp
// Purpose:  Example GLUT/vtlib application.
//
// Copyright (c) 2001-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __FreeBSD__
#  include <ieeefp.h>
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/TerrainScene.h"
#include "vtlib/core/NavEngines.h"
#include "vtdata/DataPath.h"
#include "vtdata/vtLog.h"

#ifdef __DARWIN_OSX__
 #import <GLUT/glut.h>
#else
 #include <GL/glut.h>
#endif

// Enable bits of example code
#define EXAMPLE_SNOW		0
#define EXAMPLE_PLANTS		0
#define EXAMPLE_GEOMETRY	0

#if EXAMPLE_SNOW
#include <osgParticle/PrecipitationEffect>
#pragma comment(lib, "osgParticled.lib")
#endif

void redraw();
void reshape(int w, int h);
void keyboard(uchar key, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);

/*
  The GLUT window reshape event
*/
void reshape(int w, int h )
{
	glViewport(0, 0, w, h);
	vtGetScene()->SetWindowSize(w, h);
}

/*
  The GLUT keyboard event
*/
void keyboard(uchar key, int x, int y)
{
	if ( key == 27 /* ESC */ || key == 'q' || key == 'Q' )
		exit(0);

	vtGetScene()->OnKey(key, 0);
}

#ifndef GLUT_LEFT_BUTTON
 /* Mouse buttons. */
 #define GLUT_LEFT_BUTTON		0
 #define GLUT_MIDDLE_BUTTON		1
 #define GLUT_RIGHT_BUTTON		2

 /* Mouse button  state. */
 #define GLUT_DOWN			0
 #define GLUT_UP				1
#endif

int last_button_state;

/*
  The GLUT mouse event
*/
void mouse(int button, int state, int x, int y)
{
	// turn GLUT mouse event into a VT mouse event
	vtMouseEvent event;
	event.type = (state == GLUT_DOWN) ? VT_DOWN : VT_UP;

	if (button == GLUT_LEFT_BUTTON)
		event.button = VT_LEFT;
	else if (button == GLUT_MIDDLE_BUTTON)
		event.button = VT_MIDDLE;
	else if (button == GLUT_RIGHT_BUTTON)
		event.button = VT_RIGHT;

	event.flags = 0;
	event.pos.Set(x, y);

	vtGetScene()->OnMouse(event);
}

void motion(int x, int y)
{
	vtMouseEvent event;
	event.type = VT_MOVE;
	event.button = VT_NONE;
	event.flags = 0;
	event.pos.Set(x, y);

	vtGetScene()->OnMouse(event);
}


/*
  The GLUT redraw event
*/
void redraw()
{
	vtGetScene()->DoUpdate();

	glutPostRedisplay();
	glutSwapBuffers();
}


void InitGLUT()
{
	int   fake_argc = 1;
	char *fake_argv[3] = { (char *)"glutSimple", (char *)"Example Program", NULL };

	/*
	Initialise GLUT
	*/
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(800, 600);
	glutInit          (&fake_argc, fake_argv);
	glutInitDisplayMode (GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow  (fake_argv[1]);
	glutDisplayFunc   (redraw);
	glutReshapeFunc   (reshape);
	glutKeyboardFunc  (keyboard);
	glutMouseFunc     (mouse);
	glutMotionFunc    (motion);
}


//
// Create the 3d scene
//
bool CreateScene()
{
	// Get a handle to the vtScene - one is already created for you
	vtScene *pScene = vtGetScene();

	// Log messages to make troubleshooting easier
	VTSTARTLOG("debug.txt");
	VTLOG("glutSimple\n");

	// Look up the camera
	vtCamera *pCamera = pScene->GetCamera();
	pCamera->SetHither(10);
	pCamera->SetYon(100000);

	// The  terrain scene will contain all the terrains that are created.
	vtTerrainScene *ts = new vtTerrainScene;

	// Set the global data path
	vtStringArray paths;
	paths.push_back(vtString("../Data/"));
	paths.push_back(vtString("Data/"));
	vtSetDataPath(paths);

	// Begin creating the scene, including the sun and sky
	vtGroup *pTopGroup = ts->BeginTerrainScene();

#if EXAMPLE_SNOW
	osgParticle::PrecipitationEffect *precipitationEffect = new osgParticle::PrecipitationEffect;
	precipitationEffect->snow(1);
	//precipitationEffect->rain(1);
	vtTransform *xform = new vtTransform;
	xform->Rotate2(FPoint3(1,0,0), -PID2f);
	pTopGroup->addChild(xform);
	xform->GetOsgGroup()->addChild(precipitationEffect);
#endif

	// Tell the scene graph to point to this terrain scene
	pScene->SetRoot(pTopGroup);

	// Create a new vtTerrain, read its parameters from a file
	vtTerrain *pTerr = new vtTerrain;
	pTerr->SetParamFile("Data/Simple.xml");
	pTerr->LoadParams();

#if EXAMPLE_PLANTS
	//Set species list
	vtString species = FindFileOnPaths(paths, "PlantData/species.xml");
	if (species != "")
	{
		vtSpeciesList3d *pList = new vtSpeciesList3d;
		if (pList->ReadXML(species))
			pTerr->SetPlantList(pList);
	}
#endif

	// Add the terrain to the scene, and contruct it
	ts->AppendTerrain(pTerr);
	if (!ts->BuildTerrain(pTerr))
	{
		printf("Terrain creation failed: %s\n",
			(const char *)pTerr->GetLastError());
		return false;
	}
	ts->SetCurrentTerrain(pTerr);

	// Create a navigation engine to move around on the terrain
	// Get flight speed from terrain parameters
	float fSpeed = pTerr->GetParams().GetValueFloat(STR_NAVSPEED);

	vtTerrainFlyer *pFlyer = new vtTerrainFlyer(fSpeed);
	pFlyer->AddTarget(pCamera);
	pFlyer->SetHeightField(pTerr->GetHeightField());
	pScene->AddEngine(pFlyer);

	// Minimum height over terrain is 100 m
	vtHeightConstrain *pConstrain = new vtHeightConstrain(100);
	pConstrain->AddTarget(pCamera);
	pConstrain->SetHeightField(pTerr->GetHeightField());
	pScene->AddEngine(pConstrain);

#if EXAMPLE_GEOMETRY
	// This example code demonstrates how to create colored lines, and it's
	//  also useful for debugging vtlib ports to new scenegraphs.
	vtHeightFieldGrid3d *grid = pTerr->GetHeightFieldGrid3d();
	FRECT ext = grid->m_WorldExtents;
	float minh, maxh;
	grid->GetHeightExtents(minh, maxh);
	FBox3 box(0,				   minh, ext.top,
		ext.right, minh, ext.bottom);

	vtMaterialArray *pMats = new vtMaterialArray;
	pMats->AddRGBMaterial1(RGBf(1.0f, 0.0f, 0.0f), true, false, true);
	pMats->AddRGBMaterial1(RGBf(0.0f, 1.0f, 0.0f), true, false, true);
	pMats->AddRGBMaterial1(RGBf(0.0f, 0.0f, 1.0f), true, false, true);
	vtGeode *pGeode = CreateLineGridGeom(pMats, 0,
						   box.min, box.max, 32);
	vtMesh *mesh1 = new vtMesh(osg::PrimitiveSet::LINES, 0, 6);
	mesh1->AddLine(FPoint3(0,0,0), FPoint3(0,maxh,0));
	pGeode->AddMesh(mesh1, 1);
	vtMesh *mesh2 = new vtMesh(osg::PrimitiveSet::LINES, 0, 6);
	mesh2->AddLine(FPoint3(-1000,0,0), FPoint3(1000,0,0));
	pGeode->AddMesh(mesh2, 2);
	pTopGroup->addChild(pGeode);
#endif

	return true;
}


/*
  The works.
*/
int main(int argc, char ** argv)
{
#ifdef __FreeBSD__
	/*  FreeBSD is more stringent with FP ops by default, and OSG is doing  */
	/*    silly things sqrt(Inf) (computing lengths of MAXFLOAT and NaN     */
	/*    Vec3's).   This turns off FP bug core dumps, ignoring the error   */
	/*    like most platforms do by default.                                */
	fpsetmask(0);
#endif

	printf("Initializing GLUT..\n");
	InitGLUT();

	vtGetScene()->Init(argc, argv);
    vtGetScene()->SetGraphicsContext(new osgViewer::GraphicsWindowEmbedded(0, 0, 800, 600));

	printf("Creating the terrain..\n");
	CreateScene();

	printf("Running..\n");
	glutMainLoop();

	return 0;
}

