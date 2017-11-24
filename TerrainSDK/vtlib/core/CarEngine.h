//
// CarEngine.h
// header file for CarEngine.cpp
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef CARENGINEH
#define CARENGINEH

#include "vtdata/HeightField.h"
#include "Vehicles.h"

enum CarEngineMode { NONE, JUST_DRIVE, FOLLOW_ROAD, FOLLOW_PATH };

class CarEngine : public vtEngine
{
public:
	CarEngine(Vehicle *vehicle, vtHeightField3d *hf);

	bool Valid();
	void Eval();
	void IgnoreElapsedTime();

	FPoint3 GetCurPos() { return m_vCurPos; }
	void SetEarthPos(const LocalCS &conv, const DPoint2 &pos);
	DPoint2 GetEarthPos(const LocalCS &conv);

	void SetSpeed(float fMeterPerSec);

	void SetRotation(float fRot);
	float GetRotation() { return m_fCurRotation; }

	void SetSteeringAngle(float fRadians) { m_fSteeringAngle = fRadians; }
	void SetFriction(float factor) { m_fFriction = factor; }

	void SetCameraFollow(bool bOn) { m_bCameraFollow = bOn; }
	void SetCameraDistance(float fMeters) { m_bCameraDistance = fMeters; }

protected:
	void ApplyCurrentLocation(bool bAlignOnGround);

	float SetPitch();  //returns new height for car.

	//move car to the given vector
	//change rotation about Y axis based on current and next position.
	//height is average of tire points
	void MoveCarTo(const FPoint3 &pos);

	//spin all wheels for distance traveled.
	void SpinWheels(float radians);

	vtHeightField3d *m_pHeightField;

	//what mode the engine is operating at.
	CarEngineMode m_eMode;

	FPoint3 m_vCurPos;		// current position
	float m_fCurRotation;	// rotation around Y axis(Yaw)
	float m_fCurPitch;		// rotation around orientation axis (Pitch)
	FPoint3 m_vAxis;		// orientation axis, determined by where wheels are w/respect to ground

	float m_fSpeed;			// speed in meters per second
	float m_fTargetSpeed;	// target speed
	float m_fPrevTime;		// last time that eval was run on.

	//tires
	vtTransform *FrontLeft() { return m_pVehicle->m_pFrontLeft; }
	vtTransform *FrontRight() { return m_pVehicle->m_pFrontRight; }
	vtTransform *RearLeft() { return m_pVehicle->m_pRearLeft; }
	vtTransform *RearRight() { return m_pVehicle->m_pRearRight; }

	float m_fSteeringAngle;
	float m_fFriction;

	//a setup flag - first eval doesn't run right...
	bool m_bFirstTime;

	bool m_bCameraFollow;
	float m_bCameraDistance;

	Vehicle *m_pVehicle;
};
typedef osg::ref_ptr<CarEngine> CarEnginePtr;


#endif // CARENGINEH
