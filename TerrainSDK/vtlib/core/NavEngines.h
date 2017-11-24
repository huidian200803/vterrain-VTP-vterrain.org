//
// NavEngines.h
//
// Navigation Engines, generally for moving the camera with mouse input.
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef NAVENGINESH
#define NAVENGINESH

#include "vtdata/HeightField.h"
#include "Engine.h"
#include "Event.h"

/** \defgroup nav Navigation
 * These classes are used for navigation: moving a camera or similar object
 * based on user input.
 */
/*@{*/

///////////////////////////////////////////////////

/**
 *  This engine flies the viewpoint around using the mouse position.
 *
	- Left button: forward-backward, yaw.
	- Right button: up-down, left-right.
	- Both buttons: pitch, roll.
 *
 *  Mouse position is considered relative to the center of the window.
 */
class vtFlyer : public vtMouseStateEngine
{
public:
	vtFlyer(float fSpeed = 1.0f, bool bAllowRoll = false);

	/** Set the speed of navigation, in meters per second, at maximum mouse
		displacement. */
	void SetSpeed(float fSpeed) { m_fSpeed = fSpeed; }
	/** Get the speed of navigation.  This is the speed at which the
		target will more at maximum mouse displacement. */
	float GetSpeed() { return m_fSpeed; }

	void SetAlwaysMove(bool bMove);
	void SetMultiplier(float fMult) { m_fMult = fMult; }

	void OnMouse(vtMouseEvent &event);
	void Eval();

	enum DOF { DOF_X = 0, DOF_Y, DOF_Z, DOF_PITCH, DOF_YAW, DOF_ROLL };
	void SetDOF(DOF dof, bool bTrue) { m_bDOF[dof] = bTrue; }
	bool GetDOF(DOF dof) { return m_bDOF[dof]; }

	/// Current speed (meters/second as of the most recent frame)
	float GetCurrentSpeed() { return m_fCurrentSpeed; }

protected:
	void DoKeyNavigation();

	float	m_fSpeed;		// max units per frame of movement
	float	m_fMult;		// multiply speed by this factor
	bool	m_bDOF[6];		// six degrees of freedom
	bool	m_bAlwaysMove;
	float	m_fCurrentSpeed; // current speed (most recent frame)

protected:
	~vtFlyer() {}
};

/**
 * This engine extends vtFlyer for navigation of an orthographic
 * camera (usually, top-down).
 *
	- Left button: forward-backward (implemented as zoom), yaw
	- Right button: up-down, left-right
 *
 *  Position is considered relative to the center of the window.
 */
class vtOrthoFlyer : public vtFlyer
{
public:
	vtOrthoFlyer(float fSpeed);
	void Eval();
};

/**
 * This engine extends vtFlyer with the ability to scale speed with
 * height above the terrain.
 */
class vtTerrainFlyer : public vtFlyer
{
public:
	vtTerrainFlyer(float fSpeed, bool bAllowRoll = false);

	/// Set the heightfield on which to do the terrain following.
	void SetHeightField(const vtHeightField3d *pHF) { m_pHeightField = pHF; }

	// If true, exaggerate the speed of the view by height above ground
	void SetExag(bool bDo) { m_bExag = bDo; }
	bool GetExag() { return m_bExag; }

	// override
	void Eval();

protected:
	const vtHeightField3d *m_pHeightField;
	bool	m_bExag;		// exaggerate speed based on height

protected:
	~vtTerrainFlyer() {}
};


/**
 *  This engine looks the viewpoint around using the mouse position.
 *  Movement is only done when left mouse button is pressed.
 *  Right mouse button is a holdover from vtFlyer.
 *
	- No buttons: pitch, yaw
	- Left button: forward
	- Right button: up/dn, left/right
 *
 *  Position is considered relative to the center of the window.
 */
class vtPanoFlyer : public vtTerrainFlyer
{
public:
	vtPanoFlyer(float fSpeed);

	void Eval();

protected:
	float	m_Velocity;
};


class vtTin3d;

/**
 * This engine has the ability to do usefully constrained navigation over
 * a (generally sparse) TIN terrain.
 */
class vtTinFlyer : public vtMouseStateEngine
{
public:
	vtTinFlyer(float fSpeed);

	void SetSpeed(float fSpeed) { m_fSpeed = fSpeed; }
	float GetSpeed() { return m_fSpeed; }

	/// Set the TIN heightfield to which to constrain
	void SetTin(vtTin3d *pTin);

	// implementation override
	void Eval();

protected:
	float	m_fSpeed;	// max units per frame of movement
	vtTin3d	*m_pTin;
	float	m_fPitch;
};


class vtHeightConstrain;

/**
 * Similar to vtTerrainFlyer, but a velocity (with damping) is maintained.
 * Viewpoint moves even after mouse button is released.
 */
class VFlyer : public vtTerrainFlyer
{
public:
	VFlyer(float scale);

	void SetVerticalVelocity(float velocity);

	/** Set the downward acceleration, in meter per second squared.
		For example, a value of 9.81 would be normal gravity at the
		earth's surface. */
	void SetGravity(float grav) { m_fGravity = grav; }

	/** In order for gravity to cease when the target is prevented
		from falling, e.g. when it is sitting on the ground, supply
		the engine you are using to keep the target above the ground. */
	void SetGroundTester(vtHeightConstrain *pEng) { m_pConstrain = pEng; }

	/** Set the damping of the velocity, per second. A typical range is from
		1 (very little damping) to 10 (a lot of damping). */
	void SetDamping(float factor) { m_fDamping = factor; }
	float GetDamping() { return m_fDamping; }

	void OnMouse(vtMouseEvent &event);
	void Eval();	// overrides

protected:
	FPoint3	m_Velocity;
	float	m_last_time;
	float	m_fGravity;
	float	m_fDamping;
	vtHeightConstrain *m_pConstrain;
};


/**
 * Implementation of a Quake-like navigation engine.  Mouse position
 * rotates the view direction, and keyboard input moves the view
 * position.
 */
class QuakeFlyer : public vtTerrainFlyer
{
public:
	QuakeFlyer(float scale);
	void Eval();

	// override
	void OnKey(int key, int flags);

protected:
	IPoint2	m_LastMousePt;
	float	m_flastmovex, m_flastmovey;
	short	m_sWrap;
	bool	m_bNavEnable;
};


/**
 * Constrain the height of a target over a heightfield.
 */
class vtHeightConstrain : public vtEngine
{
public:
	vtHeightConstrain(float fMinHeight);

	/// Set the heightfield on which to do the terrain following.
	void SetHeightField(const vtHeightField3d *pHF) { m_pHF = pHF; }

	/// Set the height above the terrain to allow.
	void SetMinGroundOffset(float fMeters) { m_fMinGroundOffset = fMeters; }

	/// Get the height above the terrain to allow.
	float GetMinGroundOffset() { return m_fMinGroundOffset; }

	/// Set whether to use the height of culture for terrain following. Default is false.
	void SetUseCulture(bool set) { m_bUseCulture = set; }

	/// Get whether to use the height of culture for terrain following.
	bool GetUseCulture() { return m_bUseCulture; }

	/// If true, the current height above the terrain is maintained.  Default is false.
	void SetMaintain(bool bMaintain) { m_bMaintain = bMaintain; }
	bool GetMaintain() { return m_bMaintain; }

	/// Set the height which should be maintained.
	void SetMaintainHeight(float fHeight) { m_fMaintainHeight = fHeight; }

	bool IsVerticallyMobile();

	void Eval();

protected:
	bool	m_bMaintain;
	float	m_fMaintainHeight;
	const vtHeightField3d *m_pHF;
	float m_fMinGroundOffset;
	bool	m_bUseCulture;
	bool	m_bOnGround;

protected:
	~vtHeightConstrain() {}
};


//////////////////////////////////////////////////

/**
 * A Trackball is a navigation engine which can move a Camera (or any
 * other Transform target) and move it around a center point based on
 * mouse input.
 *
 * You can set which combinations of mouse buttons and modifier keys
 * will rotate, zoom and translate the target.  The default is:
	- Middle button: Rotate.
	- Middle button with shift key: Zoom.
 */
class vtTrackball : public vtMouseStateEngine
{
public:
	vtTrackball(float fRadius);

	void SetRadius(float fRadius);
	float GetRadius() const;

	void SetPivotPoint(const FPoint3 &pos) { m_Pivot = pos; }
	FPoint3 GetPivotPoint() const { return m_Pivot; }

	void SetRotateButton(int button, int modifier = 0, bool bExact = true);
	void SetZoomButton(int button, int modifier = 0, bool bExact = true);
	void SetTranslateButton(int button, int modifier = 0, bool bExact = true);

	void SetZoomScale(float s);
	void SetRotScale(float s);
	void SetTransScale(float s);

	void SetDirection(float fTheta, float fPhi);
	void MoveDirection(float fTheta, float fPhi);

	void SetTrans(const FPoint3 &trans) { m_Trans = trans; }
	void Translate(const FPoint3 &trans) { m_Trans += trans; }
	FPoint3 GetTrans() const { return m_Trans; }

	void GetState(FPoint3 *state);
	void SetState(const FPoint3 *state);

	void OnMouse(vtMouseEvent &event);
	void Eval();

	void LimitPos(const FPoint3 &min, const FPoint3 &max);
	void LimitTrans(const FPoint3 &min, const FPoint3 &max);

protected:
	bool _IsRotate();
	bool _IsZoom();
	bool _IsTranslate();
	void _ApplyLimits();

	FPoint3	m_Pos, m_Start, m_Trans, m_Pivot;
	bool	m_bRotate;
	bool	m_bZoom;
	bool	m_bTrans;

	IPoint2	m_MouseStart;
	float	m_fZoomScale;
	float	m_fRotScale;
	float	m_fTransScale;

	int		m_rotate_button;
	int		m_rotate_modifier;
	bool	m_rotate_exact;
	int		m_zoom_button;
	int		m_zoom_modifier;
	bool	m_zoom_exact;
	int		m_trans_button;
	int		m_trans_modifier;
	bool	m_trans_exact;

	// Limits
	FPoint3 m_pos_min, m_pos_max;
	FPoint3 m_trans_min, m_trans_max;

protected:
	~vtTrackball() {}
};

/*@}*/	// Group nav

#endif

