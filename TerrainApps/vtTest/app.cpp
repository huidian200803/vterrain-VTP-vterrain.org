//
// Name:	 vtTest/app.cpp
// Purpose:  Test code for vtlib, using OSG as a framework.
//
// Copyright (c) 2007-2015 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/NavEngines.h"
#include "vtlib/core/GeomUtil.h"
#include "vtlib/vtosg/OSGEventHandler.h"
#include "vtlib/vtosg/MultiTexture.h"
#include "vtdata/vtLog.h"

class Orbit : public vtEngine
{
public:
	Orbit() { theta = 0.0f; radius = 2.2f; }
	void Eval()
	{
		theta += 0.006f;
		if (theta > PI2f)
			theta -= PI2f;
		for (uint i = 0; i < NumTargets(); i++)
		{
			osg::Referenced *t = GetTarget(i);
			vtTransform *x = dynamic_cast<vtTransform*>(t);
			if (axis == 0)
				x->SetTrans(FPoint3(radius * cos(theta), 0.0f, radius * sin(theta)));
			else if (axis == 1)
				x->SetTrans(FPoint3(radius * cos(theta), radius * sin(theta), 0.0f));
			x->PointTowards(FPoint3(0,0,0));
		}
	}
	float radius;
	float theta;
	int axis;

protected:
	~Orbit() {}
};

class App : public vtEngine
{
public:
	App() { m_pCamera = NULL; }

	int main(int argc, char **argv);

	bool CreateScene();
	void OnKey(int key, int flags);
	void SetTest(int test);

	vtGroup *MakeTestGroup(int number)
	{
		vtGroup *grp = new vtGroup;
		m_Test.push_back(grp);
		char name[80];
		sprintf(name, "Test %d", number);
		grp->setName(name);

		// Initially, not shown.
		grp->setNodeMask(0);

		m_pRoot->addChild(grp);
		return grp;
	}

	void MakeTest0();
	void MakeTest1();
	void MakeTest2();
	void MakeTest3();
	void MakeTest4();
	void MakeTest5();
	void MakeTest6();
	void MakeTest7();
	void MakeTest8();
	void MakeTest9();
	void MakeTest10();

	void MeshAndOBJTest();

public:
	vtScene *m_pScene;
	vtCamera *m_pCamera;
	vtGroup *m_pRoot;
	vtMaterialArrayPtr m_Materials;
	vtGeode *m_BlockGeode;
	int m_iTest;

	// Component nodes
	vtLightSource *m_pLight;
	vtLightSource *m_pLight2;
	vtTransform *m_pLightTrans;
	vtTransform *m_pLightTrans2;
	vtGeode *m_pBall;
	vtGeode *m_pCrosshair;
	vtTransform *m_pMovingBox;
	vtTransform *m_pMovingBox2;
	vtLOD *m_pLOD;
	vtGroup *m_pLODs;

	// Collections of nodes into individual tests
	std::vector<vtGroup*> m_Test;

	// Engines
	vtTrackball *m_tball;
	Orbit *orbit;
	Orbit *orbit2;
};


//--------------------------------------------------------------------------
// Test cases:
// 0. RGB wireframe crosshair
// 1. yellow sphere, one directional white light
// 2. yellow sphere, one directional white light moving around it in a circle
// 3. red light, orbiting, box to show its location
// 4. second orbiting light
// 5. a LOD object
// 6. many LOD objects
// 7. Shader

//
// Create the 3d scene: prepare for user interaction.
//
bool App::CreateScene()
{
	m_iTest = -1;

	// Get a handle to the vtScene - one is already created for you
	m_pScene = vtGetScene();

	// Tell the scene graph to point to our root
	m_pRoot = new vtGroup;;
	m_pScene->SetRoot(m_pRoot);

	// Look up the camera
	m_pCamera = m_pScene->GetCamera();
	m_pCamera->SetHither(1);
	m_pCamera->SetYon(100);

	// Make two lights, and transforms to move them
	m_pLight = new vtLightSource(0);
	m_pLight->SetAmbient(RGBf(0,0,0));
	m_pLight->SetDiffuse(RGBf(1,1,1));	// white
	m_pLight->SetSpecular(RGBf(1,1,1));
	m_pLightTrans = new vtTransform;
	m_pLightTrans->addChild(m_pLight);
	m_pRoot->addChild(m_pLightTrans);

	m_pLight2 = new vtLightSource(1);
	m_pLight2->SetAmbient(RGBf(0,0,0));
	m_pLight2->SetDiffuse(RGBf(1,1,1));
	m_pLight2->SetSpecular(RGBf(1,1,1));
	m_pLightTrans2 = new vtTransform;
	m_pLightTrans2->addChild(m_pLight2);
	m_pRoot->addChild(m_pLightTrans2);

	m_pLightTrans->SetTrans(FPoint3(5,0,0));
	m_pLightTrans->PointTowards(FPoint3(0,0,0));

	m_pLightTrans2->SetTrans(FPoint3(5,0,0));
	m_pLightTrans2->PointTowards(FPoint3(0,0,0));

	// Trackball to move camera around the center
	m_tball = new vtTrackball(5.0f);
	m_tball->AddTarget(m_pCamera);
	m_tball->SetRotateButton(VT_LEFT);
	m_tball->SetZoomButton(VT_RIGHT);
	m_tball->SetZoomScale(2.0f);
	this->AddChild(m_tball);

	// we, the app, are the root engine
	m_pScene->SetRootEngine(this);

	m_Materials = new vtMaterialArray;
	m_Materials->AddRGBMaterial(RGBf(1,1,0));
	m_Materials->AddRGBMaterial(RGBf(1,0,0),true,false,true);	// wire
	m_Materials->AddRGBMaterial(RGBf(0,1,0),true,false,true);	// wire
	m_Materials->AddRGBMaterial(RGBf(0,0,1),true,false,true);	// wire
	m_Materials->AddRGBMaterial(RGBf(0,0,1),true,true,false,0.6f,0.5f);
	m_Materials->AddRGBMaterial(RGBf(1,1,1),true,false,true);	// wire

	// Reference grid (always there)
	vtGeode *pGeode = CreateLineGridGeom(m_Materials, 5,
					   FPoint3(-10,-1,-10), FPoint3(10,-1,10), 32);
	m_pRoot->addChild(pGeode);

	m_BlockGeode = CreateBlockGeom(m_Materials, 4, FPoint3(0.1f, 0.1f, 0.6f));

	MakeTest0();
	MakeTest1();
	MakeTest2();
	MakeTest3();
	MakeTest4();
	MakeTest5();
	MakeTest6();
	MakeTest7();
	MakeTest8();
	MakeTest9();
	MakeTest10();

	SetTest(0);

	//vtLogGraph(m_pRoot);

	return true;
}

void App::MakeTest0()
{
	// Test 0
	vtGroup *grp = MakeTestGroup(0);

	m_pCrosshair = new vtGeode;
	m_pCrosshair->SetMaterials(m_Materials);
	AddLineMesh(m_pCrosshair, 1, FPoint3(-1, 0, 0), FPoint3(1, 0, 0));
	AddLineMesh(m_pCrosshair, 2, FPoint3(0, -1, 0), FPoint3(0, 1, 0));
	AddLineMesh(m_pCrosshair, 3, FPoint3(0, 0, -1), FPoint3(0, 0, 1));
	grp->addChild(m_pCrosshair);
}

void App::MakeTest1()
{
	// Test 1
	vtGroup *grp = MakeTestGroup(1);

	m_pBall = CreateSphereGeom(m_Materials, 0, VT_Normals, 1.0f, 32);	// Unit radius
	grp->addChild(m_pBall);
}

void App::MakeTest2()
{
	// Test 2: light, orbiting, box to show its location
	vtGroup *grp = MakeTestGroup(2);

	m_pMovingBox = new vtTransform;
	m_pMovingBox->addChild(m_BlockGeode);
	grp->addChild(m_pBall);
	grp->addChild(m_pMovingBox);

	orbit = new Orbit;
	orbit->axis = 0;
	orbit->AddTarget(m_pLightTrans);
	orbit->AddTarget(m_pMovingBox);
	this->AddChild(orbit);
}

void App::MakeTest3()
{
	// Test 3: red light, orbiting, box to show its location
	vtGroup *grp = MakeTestGroup(3);

	grp->addChild(m_pBall);
	grp->addChild(m_pMovingBox);
}

void App::MakeTest4()
{
	// Test 4: second orbiting light
	vtGroup *grp = MakeTestGroup(4);

	m_pMovingBox2 = new vtTransform;
	m_pMovingBox2->addChild(m_BlockGeode);
	grp->addChild(m_pBall);
	grp->addChild(m_pMovingBox);
	grp->addChild(m_pMovingBox2);

	orbit2 = new Orbit;
	orbit2->axis = 1;
	orbit2->AddTarget(m_pLightTrans2);
	orbit2->AddTarget(m_pMovingBox2);
	this->AddChild(orbit2);
}

void App::MakeTest5()
{
	// Test 5: LOD
	vtGroup *grp = MakeTestGroup(5);

	vtGeode *g1 = CreateSphereGeom(m_Materials, 0, VT_Normals, 0.6f, 32);
	vtGeode *g2 = CreateSphereGeom(m_Materials, 1, VT_Normals, 0.7f, 32);
	vtGeode *g3 = CreateSphereGeom(m_Materials, 2, VT_Normals, 0.8f, 32);
	m_pLOD = new vtLOD;
	m_pLOD->addChild(g1);
	m_pLOD->addChild(g2);
	m_pLOD->addChild(g3);

	m_pLOD->setRange(0, 0.0f, 6.0f);
	m_pLOD->setRange(1, 6.0f, 12.0f);
	m_pLOD->setRange(2, 12.0f, 22.0f);
	m_pLOD->SetCenter(FPoint3(0,0,0));
	grp->addChild(m_pLOD);
}

void App::MakeTest6()
{
	// Test 6: many LOD objects
	vtGroup *grp = MakeTestGroup(6);

	m_pLODs = new vtGroup;
	grp->addChild(m_pLODs);
	for (int i = -3; i <= 3; i++)
	for (int j = -3; j <= 3; j++)
	{
		if (i == 0 && j == 0)
			continue;
		vtTransform *trans = new vtTransform;
		trans->SetTrans(FPoint3(i*2, 0, j*2));
		m_pLODs->addChild(trans);
		trans->addChild(m_pLOD);
	}
}

void App::MakeTest7()
{
	// Test 7: Shader
	vtGroup *grp = MakeTestGroup(7);

	vtGeode *ball2 = CreateSphereGeom(m_Materials, 0, VT_Normals, 1.0f, 32);	// Unit radius
	ball2->setName("ball2");
	grp->addChild(ball2);

	const std::string TheShaderSource =
	   "uniform vec3 rgb;\n"
	   "void main()\n"
	   "{\n"
	   "   gl_FragColor = vec4(rgb, 1.0);\n"
	   "}\n";

	static const char *microshaderVertSource = {
		"// microshader - colors a fragment based on its position\n"
		"varying vec4 color;\n"
		"void main(void)\n"
		"{\n"
		"    color = gl_Vertex;\n"
		"    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	//	"    color = gl_ProjectionMatrix * gl_Vertex;\n"
	//	"    gl_Position = gl_Vertex;\n"
		"}\n"
	};

	static const char *microshaderFragSource = {
		"varying vec4 color;\n"
		"void main(void)\n"
		"{\n"
		"	int band = int((cos(color.r*3) + sin(color.g*2.5) - sin(color.b*4)) * 10);\n"
		"	if (band & 1)\n"
		"    gl_FragColor = vec4(1,1,1,1);\n"
		"   else\n"
		"    gl_FragColor = clamp( color, 0.0, 1.0 );\n"
//		"    gl_FragColor = clamp( color, 0.0, 1.0 );\n"
		"}\n"
	};

//	osg::ref_ptr<osg::Shader> shader(new osg::Shader(osg::Shader::FRAGMENT));
//	shader->setShaderSource(TheShaderSource);

	osg::ref_ptr<osg::Program> program(new osg::Program());
//	program->addShader(shader);
    program->addShader( new osg::Shader( osg::Shader::VERTEX, microshaderVertSource ) );
    program->addShader( new osg::Shader( osg::Shader::FRAGMENT, microshaderFragSource ) );

	osg::ref_ptr<osg::StateSet> ss = ball2->getOrCreateStateSet();
	ss->setAttribute(program);

	osg::ref_ptr<osg::Uniform> rgbUniform(
		new osg::Uniform("rgb", osg::Vec3(0.2, 0.2, 1.0)));
	ss->addUniform(rgbUniform);
}

void App::MakeTest8()
{
	// Test 8: Single texture
	vtGroup *grp = MakeTestGroup(8);

	osg::Image *image = LoadOsgImage("G:/Data-Testing/Culture/modulate.jpg");
	int matidx = m_Materials->AddTextureMaterial(image, true, false);

	vtGeode *ball3 = CreateSphereGeom(m_Materials, matidx, VT_Normals | VT_TexCoords, 1.0f, 32);

	grp->addChild(ball3);
}

void App::MakeTest9()
{
	// Test 9: Single texture, using TexGen
	vtGroup *grp = MakeTestGroup(9);

	vtImageGeo *image = new vtImageGeo;
	image->ReadTIF("G:/Data-USA/Data-Hawaii/GeoSpecific/waimea_subset_2048.tif");

	// Force the internal format to RGBA
	image->setInternalTextureFormat(GL_RGBA);

	vtMaterial *pMat = new vtMaterial;
	pMat->SetAmbient(1,1,1);
	pMat->SetTexture2D(image);
	pMat->SetCulling(true);
	pMat->SetLighting(false);

	FPoint2 scale(1, 1), offset(0.5, 0.5);
	pMat->SetTexGen2D(scale, offset);
	pMat->SetTextureMode(GL_DECAL);
	pMat->SetClamp(true);

	int matidx = m_Materials->AppendMaterial(pMat);

	vtGeode *ball4 = CreateSphereGeom(m_Materials, matidx, VT_Normals, 1.0f, 32);

	grp->addChild(ball4);
}

void App::MakeTest10()
{
	// Test 10: Two textures
	vtGroup *grp = MakeTestGroup(10);

//	osg::Image *image1 = osgDB::readImageFile("G:/Data-Testing/Culture/drystone_wall_512.jpg");
	osg::Image *image1 = osgDB::readImageFile("G:/Data-Testing/Culture/modulate.jpg");

	vtMaterial *pMat = new vtMaterial;
	pMat->SetTexture2D(image1, 0);
	pMat->SetCulling(true);
	pMat->SetLighting(false);

//	osg::Image *image2 = osgDB::readImageFile("G:/Data-Testing/Culture/modulate.jpg");
//	osg::Image *image2 = osgDB::readImageFile("G:/Data-USA/Data-Hawaii/GeoSpecific/waimea_subset_2048.tif");

	vtImageGeo *image2 = new vtImageGeo;
	image2->ReadTIF("G:/Data-USA/Data-Hawaii/GeoSpecific/waimea_subset_2048.tif");

	FPoint2 scale(1, -1), offset(-0.5, -0.5);
#if 0
	pMat->SetTexture(image2, 1);

	// Force the internal format to RGBA
	image2->setInternalTextureFormat(GL_RGBA);

	pMat->SetTexGen2D(scale, offset, GL_DECAL, 1);	// Texture unit 1
	pMat->SetClamp(true, 1);
#endif

	int matidx = m_Materials->AppendMaterial(pMat);

	vtGeode *ball5 = CreateSphereGeom(m_Materials, matidx, VT_Normals | VT_TexCoords, 1.0f, 32);

	vtMultiTexture *mt = new vtMultiTexture;
	mt->Create(ball5, image2, scale, offset, 1, GL_DECAL);

	grp->addChild(ball5);
}

void App::MeshAndOBJTest()
{
	// Make a minimal shape, 6 vertices, 2 rectangles side by side, 3 strips.
	vtMesh *mesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_STRIP, 0, 6);
	mesh->AddVertex(FPoint3(0, 0, 0));
	mesh->AddVertex(FPoint3(1, 0, 0));
	mesh->AddVertex(FPoint3(2, 0, 0));
	mesh->AddVertex(FPoint3(0, 1, 0));
	mesh->AddVertex(FPoint3(1, 1, 0));
	mesh->AddVertex(FPoint3(2, 1, 0));
	mesh->AddFan(1, 0, 4, 3);
	mesh->AddFan(2, 1, 5, 4);
	mesh->AddFan(3, 0, 4, 1, 5, 2);

	// And one with fans.
	vtMesh *mesh2 = new vtMesh(osg::PrimitiveSet::TRIANGLE_FAN, 0, 6);
	mesh2->AddVertex(FPoint3(0, 0, 0));
	mesh2->AddVertex(FPoint3(1, 0, 0));
	mesh2->AddVertex(FPoint3(2, 0, 0));
	mesh2->AddVertex(FPoint3(0, 1, 0));
	mesh2->AddVertex(FPoint3(1, 1, 0));
	mesh2->AddVertex(FPoint3(2, 1, 0));
	mesh2->AddFan(0, 1, 4, 3);
	mesh2->AddFan(1, 2, 5, 4);

	osg::ref_ptr<vtGeode> geode(new vtGeode);
	geode->addDrawable(mesh);
	geode->addDrawable(mesh2);

	WriteGeodeToOBJ(geode, "C:/temp/mesh_test.obj");
}

void App::SetTest(int test)
{
	m_iTest = test;
	VTLOG("Test %d\n", test);

	for (size_t i = 0; i < m_Test.size(); i++)
		m_Test[i]->SetEnabled(i == test);

	switch (test)
	{
	case 2:
		orbit->SetEnabled(true);
		break;
	case 3:
		// red light
		m_pLight->SetDiffuse(RGBf(1,0,0));
		break;
	case 4:
		// red light, green light
		orbit->SetEnabled(true);
		orbit2->SetEnabled(true);
		m_pLight2->SetEnabled(true);
		m_pLight->SetDiffuse(RGBf(1,0,0));
		m_pLight2->SetDiffuse(RGBf(0,1,0));
		break;
	default:
		// no orbiting, single white light
		orbit->SetEnabled(false);
		orbit2->SetEnabled(false);
		m_pLight->SetDiffuse(RGBf(1,1,1));
		m_pLight->SetEnabled(true);
		m_pLight2->SetEnabled(false);
		break;
	}
}

void App::OnKey(int key, int flags)
{
	m_iTest++;
	if (m_iTest >= (int) m_Test.size())
		m_iTest = 0;
	SetTest(m_iTest);
}

void print_engine_tree(vtEngine *eng, int indent = 0)
{
	for (int i = 0; i < indent; i++) VTLOG1(" ");
	VTLOG("eng %lx\n", eng);
	for (uint i = 0; i < eng->NumChildren(); i++)
		print_engine_tree(eng->GetChild(i), indent+1);
}

/*
  The works.
*/
int App::main(int argc, char **argv)
{
	VTSTARTLOG("debug.txt");
	VTLOG("vtTest\n");

	// Make a scene and a viewer:
	vtGetScene()->Init(argc, argv);
	osgViewer::Viewer *viewer = vtGetScene()->getViewer();

	// Add a handler for GUI events
	osg::ref_ptr<vtOSGEventHandler> pHandler = new vtOSGEventHandler;
	viewer->addEventHandler(pHandler);

	// Basic test of the mesh and OBJ functionality.
	MeshAndOBJTest();
	return 0;

	VTLOG1("Creating the terrain..\n");
	if (!CreateScene())
		return 0;

	// Check engine tree
	//print_engine_tree(vtGetScene()->GetRootEngine());

	// We must call realize to be certain that a display/context is set up
	viewer->realize();

	// Only then can we safely get window size
	vtGetScene()->GetWindowSizeFromOSG();

	VTLOG1("Running..\n");
	while (!viewer->done())
		vtGetScene()->DoUpdate();		// calls viewer::frame

	VTLOG1("Cleaning up..\n");
	m_pScene->SetRoot(NULL);
	m_pScene->SetRootEngine(NULL);
	m_pScene->Shutdown();

	return 0;
}

int main(int argc, char **argv)
{
#if WIN32 && defined(_MSC_VER) && VTDEBUG
	// sometimes, MSVC seems to need to be told to show unfreed memory on exit
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	osg::ref_ptr<App> app = new App;
	return app->main(argc, argv);
}
