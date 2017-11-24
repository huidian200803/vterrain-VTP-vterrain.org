//
// Name:     app.cpp
// Purpose:  The application class for a wxWindows application.
//
// Copyright (c) 2001-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
#include <Python/Python.h>

// For compilers that support precompilation, includes "wx.h".
//#include "wx/wxprec.h"

//#ifndef WX_PRECOMP
//#include "wx/wx.h"
//#endif

// Header for the vtlib library
#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/TerrainScene.h"
#include "vtlib/core/NavEngines.h"
#include "vtdata/vtLog.h"

// Constants
vtTerrainScene* m_pTerrainScene = NULL;
bool m_pbKeyState[512];
int m_iConsecutiveMousemoves;
bool m_bPainting = false;

// Methods
static
PyObject* vtpSimple_init(PyObject* self, PyObject* args) {
  // Would be usefule to include a parameter for the type of flyer
  const char *sourcefile, *datadir, *debugfile;
  if (!PyArg_ParseTuple(args, "sss", &sourcefile, &datadir, &debugfile))
	return NULL;


  m_pTerrainScene = NULL;
  vtGetScene()->Init();
  
  VTSTARTLOG(debugfile);
  
  // Get a handle to the vtScene - one is already created for you
  vtScene *pScene = vtGetScene();
  
  // Look up the camera
  vtCamera *pCamera = pScene->GetCamera();
  pCamera->SetHither(10);
  pCamera->SetYon(100000);
  
  // The  terrain scene will contain all the terrains that are created.
  m_pTerrainScene = new vtTerrainScene;
  
  // Set the global data path
  vtStringArray paths;
  paths.push_back(vtString(datadir));
  pScene->SetDataPath(paths);
  
  // Begin creating the scene, including the sun and sky
  vtGroup *pTopGroup = m_pTerrainScene->BeginTerrainScene();
  
  // Tell the scene graph to point to this terrain scene
  pScene->SetRoot(pTopGroup);
  
  // Create a new vtTerrain, read its paramters from a file
  vtTerrain *pTerr = new vtTerrain;
  pTerr->SetParamFile(sourcefile);
  pTerr->LoadParams();
  
  // Add the terrain to the scene, and contruct it
  m_pTerrainScene->AppendTerrain(pTerr);
  if (!m_pTerrainScene->BuildTerrain(pTerr))
	{
	  printf("Couldn't create the terrain.  Perhaps the elevation data file isn't in the expected location?\n");
	  Py_INCREF(Py_False);
	  return Py_False;

	}
  m_pTerrainScene->SetCurrentTerrain(pTerr);
  
  // Create a navigation engine to move around on the terrain
  // Get flight speed from terrain parameters
  float fSpeed = pTerr->GetParams().GetValueFloat(STR_NAVSPEED);
  
  vtTerrainFlyer *pFlyer = new vtTerrainFlyer(fSpeed);
  pFlyer->SetTarget(pCamera);
  pFlyer->SetHeightField(pTerr->GetHeightField());
  pScene->AddEngine(pFlyer);
  
  // Minimum height over terrain is 100 m
  vtHeightConstrain *pConstrain = new vtHeightConstrain(100);
  pConstrain->SetTarget(pCamera);
  pConstrain->SetHeightField(pTerr->GetHeightField());
  pScene->AddEngine(pConstrain);

  for (int i = 0; i < 512; i++)
	m_pbKeyState[i] = false;
  vtGetScene()->SetKeyStates(m_pbKeyState);
  
  
  printf("Done creating scene.\n");
  Py_INCREF(Py_True);
  return Py_True;
}



static
PyObject *vtpSimple_updateScene(PyObject* self, PyObject* args)
{
  if (!m_bPainting) {
	m_bPainting = true;
	vtGetScene()->DoUpdate();
	m_iConsecutiveMousemoves = 0;
	m_bPainting = false;
  }

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *vtpSimple_keyEvent(PyObject* self, PyObject* args, PyObject* kw)
{
  int key;
  long flag = 0;
  bool alt = false, shift = false, control = false;
  static char* keywords[] = {"code", "alt","shift","control", NULL};
  if (!PyArg_ParseTupleAndKeywords(args, kw, "i|bbb", keywords, &key, &alt, &shift, &control))
	return NULL;

  if (alt) flag |= VT_CONTROL;
  if (shift) flag |= VT_SHIFT;
  if (control) flag |= VT_CONTROL;

  // pass the char to the vtlib Scene
  vtGetScene()->OnKey(key, flag);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *vtpSimple_setKeyState(PyObject* self, PyObject* args)
{
  int key;
  bool state;
  if (!PyArg_ParseTuple(args, "ib", &key, &state))
	return NULL;

  if (key < 512) {
	m_pbKeyState[key] = state;
  }

  Py_INCREF(Py_None);
  return Py_None;

}

static
PyObject *vtpSimple_mouseButtonEvent(PyObject* self, PyObject* args, PyObject* kw)
{
  // 1 = left, 2 = right, 3 = middle
  int buttonNums[] = {VT_LEFT, VT_RIGHT, VT_MIDDLE};
  int button; 
  long xpos, ypos;
  bool down;
  vtMouseEvent event;

  bool alt = false, shift = false, control = false;
  char* keywords[] = {"x", "y", "button", "down", "alt","shift","control", NULL};
  if (!PyArg_ParseTupleAndKeywords(args, kw, "llib|bbb", keywords, 
								   &xpos, &ypos, &button, &down, 
								   &alt, &shift, &control))
	return NULL;

  button--;

  if (button < 0 || button >= 3) {
	Py_INCREF(Py_None);
	return Py_None;
  }
  event.button = buttonNums[button];

  if (down) {
	event.type = VT_DOWN;
  } else {
	event.type = VT_UP;
  }

  event.pos.Set(xpos, ypos);
  event.flags = 0;
  if (alt) event.flags |= VT_CONTROL;
  if (shift) event.flags |= VT_SHIFT;
  if (control) event.flags |= VT_CONTROL;
  
  vtGetScene()->OnMouse(event);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *vtpSimple_mouseMoveEvent(PyObject* self, PyObject* args, PyObject* kw)
{
  long xpos, ypos;
  vtMouseEvent event;

  if (m_iConsecutiveMousemoves > 1) {
	Py_INCREF(Py_None);
	return Py_None;
  }

  bool alt = false, shift = false, control = false;
  char* keywords[] = {"x", "y", "alt","shift","control", NULL};
  if (!PyArg_ParseTupleAndKeywords(args, kw, "ll|bbb", keywords, 
								   &xpos, &ypos, 
								   &alt, &shift, &control))
	return NULL;

  event.type = VT_MOVE;
  event.button = VT_NONE;
  event.pos.Set(xpos, ypos);

  m_iConsecutiveMousemoves++;		// Increment
  
  event.flags = 0;
  if (alt) event.flags |= VT_CONTROL;
  if (shift) event.flags |= VT_SHIFT;
  if (control) event.flags |= VT_CONTROL;
  
  vtGetScene()->OnMouse(event);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *vtpSimple_setCameraPosition(PyObject* self, PyObject* args)
{
  int width, height;
  if (!PyArg_ParseTuple(args, "ii", &width, &height))
	return NULL;
  //printf("window: %i %i\n", width, height);
  vtGetScene()->SetWindowSize(width, height);

  Py_INCREF(Py_None);
  return Py_None;
}


static
PyObject *vtpSimple_setCanvasSize(PyObject* self, PyObject* args)
{
  int width, height;
  if (!PyArg_ParseTuple(args, "ii", &width, &height))
	return NULL;
  //printf("window: %i %i\n", width, height);
  vtGetScene()->SetWindowSize(width, height);

  Py_INCREF(Py_None);
  return Py_None;
}

static
PyObject *vtpSimple_shutdown(PyObject* self, PyObject* args)
{
  if (m_pTerrainScene)
	{
	  // Clean up the scene
	  vtGetScene()->SetRoot(NULL);
	  m_pTerrainScene->CleanupScene();
	  delete m_pTerrainScene;
	  vtGetScene()->Shutdown();
	}
  Py_INCREF(Py_None);
  return Py_None;
}



// Python Module stuff
static PyMethodDef vtpSimpleMethods[] = {
    {"init",  vtpSimple_init, METH_VARARGS,
     "Initialize the vtScene."},
	{"shutdown", vtpSimple_shutdown, METH_VARARGS,
	 "Shutdown the scene."},
	{"updateScene", vtpSimple_updateScene, METH_VARARGS,
	 "Update/redraw the scene"},
	{"setCanvasSize", vtpSimple_setCanvasSize, METH_VARARGS,
	 "Set the GL canvas size."},

	{"keyEvent", (PyCFunction)vtpSimple_keyEvent, METH_VARARGS | METH_KEYWORDS,
	 "Send a key event to the scene."},
	{"mouseButtonEvent", (PyCFunction)vtpSimple_mouseButtonEvent, METH_VARARGS | METH_KEYWORDS,
	 "Send a key event to the scene."},
	{"mouseMoveEvent", (PyCFunction)vtpSimple_mouseMoveEvent, METH_VARARGS | METH_KEYWORDS,
	 "Send a key event to the scene."},
	{"setKeyState", (PyCFunction)vtpSimple_setKeyState, METH_KEYWORDS,
	 "Sets the state of a key from the keyboard as on/off."},
	/*
	{"create_canvas",vtpSimple_create_canvas, METH_VARARGS,
	 "Create a canvas."},
	*/
    {NULL, NULL, 0, NULL}        /* Sentinel */
};


PyMODINIT_FUNC
initvtpSimple(void)
{
    PyObject *m;

    m = Py_InitModule("vtpSimple", vtpSimpleMethods);
    if (m == NULL)
        return;
}
