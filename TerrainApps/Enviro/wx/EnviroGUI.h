//
// EnviroGUI.h
// GUI-specific functionality of the Enviro class
//
// Copyright (c) 2003-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "../Enviro.h"

class vtJoystickEngine : public vtEngine
{
public:
	vtJoystickEngine();
	void Eval();

	void SetSpeed(float f) { m_fSpeed = f; };
	float GetSpeed() { return m_fSpeed; }

	class wxJoystick *m_pStick;
	float m_fSpeed, m_fLastTime;
};

/**
 The core GUI class of the application.  Handles GUI implementations of all the
 functionality of the Enviro class.
 */
class EnviroGUI : public Enviro
{
public:
	EnviroGUI();
	~EnviroGUI();

	// The following are overrides, to handle situations in which the
	//  GUI must be informed of what happens in the Enviro object.
	virtual void ShowPopupMenu(const IPoint2 &pos);
	virtual void SetTerrainToGUI(vtTerrain *pTerrain);
	virtual void ShowLayerView();
	virtual void RefreshLayerView();
	virtual void UpdateLayerView();
	virtual void CameraChanged();
	virtual void EarthPosUpdated();
	virtual void ShowDistance(const DPoint2 &p1, const DPoint2 &p2,
		double fGround, double fVertical);
	virtual void ShowDistance(const DLine2 &path,
		double fGround, double fVertical);
	virtual vtTagArray *GetInstanceFromGUI();
	virtual bool OnMouseEvent(vtMouseEvent &event);
	virtual void SetupScene3();
	virtual void SetTimeEngineToGUI(class vtTimeEngine *pEngine);
	virtual bool IsAcceptable(vtTerrain *pTerr);
	virtual void OnCreateTerrain(vtTerrain *pTerr);
	virtual void ShowMessage(const vtString &str);
	virtual void SetFlightSpeed(float speed);
	virtual void SetState(AppState s);
	virtual vtString GetStringFromUser(const vtString &title, const vtString &msg);
	virtual void ShowProgress(bool bShow);
	virtual void SetProgressTerrain(vtTerrain *pTerr);
	virtual void UpdateProgress(const char *msg1, const char *msg2, int amount1, int amount2);
	virtual void AddVehicle(CarEngine *eng);
	virtual void RemoveVehicle(CarEngine *eng);

	void SetScenario(int num);

	// The following are useful methods
	bool SaveVegetation(bool bAskFilename);
	bool SaveStructures(bool bAskFilename);
	void OnSetDelete(vtFeatureSet *set);

	// navigation engines
	vtJoystickEngine	*m_pJFlyer;
};

// Helper
vtAbstractLayer *CreateNewAbstractPointLayer(vtTerrain *pTerr, bool bAskStyle = true);
vtAbstractLayer *CreateNewAbstractLineLayer(vtTerrain *pTerr, bool bAskStyle = true);

/** global singleton */
extern EnviroGUI g_App;

