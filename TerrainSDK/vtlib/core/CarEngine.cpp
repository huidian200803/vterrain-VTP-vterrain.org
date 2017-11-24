//
// CarEngine.cpp
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "CarEngine.h"
#include "vtdata/vtLog.h"

using namespace std;

// acceleration in meters per second^2 (for the car.)
#define ACCELERATION 4.0f
#define ACCEL (ACCELERATION*1000/60/60)

static FPoint3 XAXIS = FPoint3(1, 0, 0);
static FPoint3 YAXIS = FPoint3(0, 1, 0);


///////////////////////////////////////////////////////////////////////
// Helpers

// Adjust angle (in radians) so that return value will be between PI and -PI.
float angleNormal(float val)
{
	while (val > PIf)
		val -= PI2f;
	while (val < -PIf)
		val += PI2f;
	return val;
}


///////////////////////////////////////////////////////////////////////
// CarEngine class

/**
 Setup engine to drive freely.

 \param vehicle	The vehicle this engine will control.
 \param hf		The surface to drive on.
 */
CarEngine::CarEngine(Vehicle *vehicle, vtHeightField3d *hf)
{
	m_pVehicle = vehicle;
	AddTarget(vehicle);

	m_pHeightField = hf;

	m_fSpeed = 0;

	m_fSteeringAngle = 0.0f;
	m_fFriction = 1.0f;
	m_fCurRotation = PID2f;
	m_fCurPitch = 0;
	m_vAxis = XAXIS;

	m_bFirstTime = true;
	m_bCameraFollow = false;
	m_bCameraDistance = 20.0f;

	m_fPrevTime = 0.0f;		// we will use engine time (t) from now on

	m_vCurPos = vehicle->GetTrans();
	m_eMode = JUST_DRIVE;
}

// Evaluate the car engine.
void CarEngine::Eval()
{
	float t = vtGetTime();
	float fDeltaTime = t - m_fPrevTime;

	// Don't get too jumpy on low framerate, such as when the program is paused
	if (fDeltaTime > 1.0f)
		fDeltaTime = 1.0f;

	//find Next location
	FPoint3 vNext;
	switch (m_eMode)
	{
	case NONE:	//go nowhere.
		vNext = m_vCurPos;
		MoveCarTo(vNext);
		break;
	case JUST_DRIVE:
		// apply friction
		m_fSpeed *= m_fFriction;

		// move in the direction we're facing
		vNext.x = m_vCurPos.x + fDeltaTime*m_fSpeed*cosf(m_fCurRotation);
		vNext.y = m_vCurPos.y;
		vNext.z = m_vCurPos.z - fDeltaTime*m_fSpeed*sinf(m_fCurRotation);
		MoveCarTo(vNext);
		break;
	case FOLLOW_ROAD:
	case FOLLOW_PATH:
		break;
	}
	// spin the wheels, adjusted for speed.
	SpinWheels(fDeltaTime*m_fSpeed/m_pVehicle->GetWheelRadius());

	m_fPrevTime = t;
}

void CarEngine::IgnoreElapsedTime()
{
	m_fPrevTime = vtGetTime();
}

void CarEngine::SetSpeed(float fMetersPerSec)
{
	m_fSpeed = fMetersPerSec;
}

DPoint2 CarEngine::GetEarthPos(const LocalCS &conv)
{
	// convert terrain to earth coords
	DPoint3 d3;
	conv.LocalToEarth(m_vCurPos, d3);
	return DPoint2(d3.x, d3.y);
}

void CarEngine::SetEarthPos(const LocalCS &conv, const DPoint2 &pos)
{
	// convert earth to terrain coords
	DPoint3 d3(pos.x, pos.y, 0);
	conv.EarthToLocal(d3, m_vCurPos);

	ApplyCurrentLocation(true);
}

void CarEngine::SetRotation(float fRot)
{
	m_fCurRotation = fRot;
	ApplyCurrentLocation(true);
}

void CarEngine::ApplyCurrentLocation(bool bAlignOnGround)
{
	vtTransform *car = dynamic_cast<vtTransform*> (GetTarget());
	if (!car)
		return;

	// re-settle vehicle on terrain surface
	car->Identity();

	car->SetTrans(m_vCurPos);

	// Angle is measure from +X, but our car's "forward" is -Z.  That's a difference
	//  in angle of PI/2 between them.
	car->RotateLocal(FPoint3(0,1,0), m_fCurRotation-PID2f);

	if (bAlignOnGround)
	{
		m_vCurPos.y = SetPitch();
		car->SetTrans(m_vCurPos);
	}
}

//
// Determines the pitch of the car, based on tire positions.
// Returns height of the center of the car.
//
float CarEngine::SetPitch()
{
	vtTransform *car = dynamic_cast<vtTransform*> (GetTarget());
	if (!car)
		return 1.0f;

	FSphere fL, fR, rL, rR;

	// find tire locations, given the current pose of the vehicle
	FrontLeft()->GetBoundSphere(fL, true);
	FrontRight()->GetBoundSphere(fR, true);
	RearLeft()->GetBoundSphere(rL, true);
	RearRight()->GetBoundSphere(rR, true);

	// get the four points on the ground under the tires
	m_pHeightField->FindAltitudeAtPoint(fL.center, fL.center.y, false, CE_ROADS);
	m_pHeightField->FindAltitudeAtPoint(fR.center, fR.center.y, false, CE_ROADS);
	m_pHeightField->FindAltitudeAtPoint(rL.center, rL.center.y, false, CE_ROADS);
	m_pHeightField->FindAltitudeAtPoint(rR.center, rR.center.y, false, CE_ROADS);

	// find midpoints between the tires.
	FPoint3 rM, fM;
	rM = (rL.center + rR.center)/2;
	fM = (fL.center + fR.center)/2;

	FPoint3 back_side = rR.center - rL.center;
	FPoint3 left_side = fL.center - rL.center;
	back_side.Normalize();
	left_side.Normalize();

	// vNormal the upwards vector
	FPoint3 vNormal = back_side.Cross(left_side);
	vNormal.Normalize();

	// new pitch
	FPoint3 horiz = left_side;
	horiz.y = 0;
	float xz = horiz.Length();

	// tan(pitch) = y / xz, so pitch = atan(y/xz);
	m_fCurPitch = atanf(left_side.y / xz);

	car->RotateLocal(FPoint3(1,0,0), m_fCurPitch);

	// new roll
	horiz = back_side;
	horiz.y = 0;
	xz = horiz.Length();

	// tan(roll) = y / xz, so roll = atan(y/xz);
	float fRoll = atanf(back_side.y / xz);

	car->RotateLocal(FPoint3(0,0,1), fRoll);

	// return height of midpoint of all wheels.
	return (fM.y+rM.y)/2;
}

/**
 Rotate (yaw) body to head toward next location, pitch it based on tire
  locations, and determine overall height of vehicle.
 Steering of wheels is done in here too.
 */
void CarEngine::MoveCarTo(const FPoint3 &next_pos)
{
	vtTransform *carxform = dynamic_cast<vtTransform*> (GetTarget());
	if (!carxform)
		return;

	// Place the car in the XZ plane
	carxform->Identity();
	carxform->SetTrans(next_pos);

	FPoint3 delta = next_pos - m_vCurPos;

	if (delta.x != 0.0f || delta.z != 0.0f)
	{
		float distance_traveled = delta.Length();

		// Turn car based on how the steering wheel is turned
		m_fCurRotation += (m_fSteeringAngle * distance_traveled);
		m_fCurRotation = angleNormal(m_fCurRotation);

		//VTLOG("Wheel: %-f\t m_CurRot: %f \n", m_fSteeringAngle, m_fCurRotation);
#if 0
		// Yaw the wheel objects to show wheel angle?  Don't bother for now,
		//  it's too subtle an effect.
		FPoint3 trans;
		trans = m_pFrontLeft->GetTrans();
		m_pFrontLeft->Identity();
		m_pFrontLeft->Rotate(YAXIS, m_fSteeringAngle);
		m_pFrontLeft->Translate(trans);

		trans = m_pFrontRight->GetTrans();
		m_pFrontRight->Identity();
		m_pFrontRight->Rotate(YAXIS, m_fSteeringAngle);
		m_pFrontRight->Translate(trans);
#endif
	}
	// Angle is measure from +X, but our car's "forward" is -Z.  That's a difference
	//  in angle of PI/2 between them.
	carxform->RotateLocal(FPoint3(0,1,0), m_fCurRotation-PID2f);

	// Modify the orientation of the car to match the terrain
	//  points of the tires
	float elev;
	if (m_bFirstTime)
	{
		m_pHeightField->FindAltitudeAtPoint(m_vCurPos, elev, false, CE_ROADS);
		m_bFirstTime = false;
	}
	else
		elev = SetPitch();

	// New position is now current
	m_vCurPos = next_pos;
	m_vCurPos.y = elev;
	carxform->SetTrans(m_vCurPos);

	// If desired, we can put the camera in a position to observe the car;
	//  this should be a separate engine, but it was quick and easy to add this:
	if (m_bCameraFollow)
	{
		FPoint3 direction = carxform->GetDirection();
		FPoint3 offset = direction * -1.0f * m_bCameraDistance;
		offset.y += (m_bCameraDistance / 2.0f);
		FPoint3 from = m_vCurPos + offset;

		vtGetScene()->GetCamera()->SetTrans(from);
		vtGetScene()->GetCamera()->PointTowards(m_vCurPos);
	}
}

//spin the wheels base on how much we've driven
void CarEngine::SpinWheels(float radians)
{
	FrontLeft()->RotateLocal(XAXIS, -radians);
	FrontRight()->RotateLocal(XAXIS, -radians);
	RearLeft()->RotateLocal(XAXIS, -radians);
	RearRight()->RotateLocal(XAXIS, -radians);
}

bool CarEngine::Valid()
{
	return (FrontLeft() != NULL && FrontRight() != NULL &&
			RearLeft() != NULL && RearRight() != NULL);
}

