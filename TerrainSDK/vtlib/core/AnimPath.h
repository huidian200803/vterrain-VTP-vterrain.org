//
// AnimPath.h
//
// Implementation animation path capabilities.
//
// Copyright (c) 2004-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ANIMPATHH
#define ANIMPATHH

#include "Engine.h"
#include "vtdata/CubicSpline.h"
#include "vtdata/Features.h"
#include "vtdata/LocalCS.h"

/** \addtogroup nav */
/*@{*/

/**
 * This class describes a single location, including both a position and
 * orientation, in world coordinates, which is used to define a path.
 */
struct ControlPoint
{
	ControlPoint() {}

	/// Construct from position.  Rotation is default (down -Z axis).
	ControlPoint(const FPoint3 &position):
		m_Position(position),
		m_Rotation() {}

	/// Construct from position and rotation.
	ControlPoint(const FPoint3 &position, const FQuat &rotation):
		m_Position(position),
		m_Rotation(rotation) {}

	void Interpolate(double ratio,const ControlPoint &first, const ControlPoint &second);
	void GetMatrix(FMatrix4 &matrix, bool bPosOnly = false) const;

	FPoint3 m_Position;
	FQuat m_Rotation;
	int m_iIndex;
};

/**
 * An animation path is a series of locations (class ControlPoint) which
 * define a path through space in world coordinates.  It is useful to use
 * in conjunction with the vtAnimPathEngine class, which can move any
 * transform (such as scene graph object or camera) along the path.
 *
 * When this object is serialized to an XML file, the points are transformed
 * to earth coordinates in GCS(WGS84) so that the result is interoperable and
 * terrain-independent.
 */
class vtAnimPath : public osg::Referenced
{
public:
	vtAnimPath();
	vtAnimPath(const vtAnimPath &ap);
	virtual ~vtAnimPath();

	/// Must tell the AnimPath what CRS its points are in, for serialization.
	bool SetCRS(const vtCRS &crs, const LocalCS &conv);

	/// Get the transformation matrix for a point in time.
	bool GetMatrix(double time, FMatrix4 &matrix, bool bPosOnly) const
	{
		ControlPoint cp;
		if (!GetInterpolatedControlPoint(time,cp))
			return false;
		cp.GetMatrix(matrix, bPosOnly);
		return true;
	}

	/// Get the local ControlPoint frame for a point in time.
	virtual bool GetInterpolatedControlPoint(double time, ControlPoint &controlPoint) const;

	/// Insert a control point on this path.
	void Insert(double time, const ControlPoint &controlPoint);
	/// Remove a control point from this path, by index.
	void RemovePoint(int index);
	/// Return number of control points in this path.
	uint NumPoints() { return m_TimeControlPointMap.size(); }

	void SetTimeFromLinearDistance();
	void ProcessPoints();

	double GetFirstTime() const;
	double GetLastTime() const;
	double GetPeriod() const { return GetLastTime()-GetFirstTime();}
	float GetTotalTime();

	enum InterpMode
	{
		LINEAR,
		CUBIC_SPLINE
	};
	/** Set the interpolation mode, either vtAnimPath::LINEAR or
	 * vtAnimPath::CUBIC_SPLINE.
	 * - LINEAR: the position is interpolated in a straight line between
	 *   each of the control points.
	 * - CUBIC_SPLINE: the position is interpolated using a cubic spline,
	 *   a smooth path which passes directly through each control point.
	 */
	void SetInterpMode(InterpMode mode) { m_InterpMode = mode; }
	/// Returns the interpolation mode, either LINEAR or CUBIC_SPLINE
	InterpMode GetInterpMode() const { return m_InterpMode; }

	typedef std::map<double,ControlPoint> TimeControlPointMap;

	TimeControlPointMap &GetTimeControlPointMap() { return m_TimeControlPointMap; }
	const TimeControlPointMap &GetTimeControlPointMap() const { return m_TimeControlPointMap; }

	void Clear() { m_TimeControlPointMap.clear(); }
	bool IsEmpty() const { return m_TimeControlPointMap.empty(); }

	/** If Loop is true, interpolation of the control points will loop back
	 *  to the first point.  Where N is the number of points, normal
	 *  interpolation is 0,1,2,...,N-1.  Loop interpolation is 0,1,2,...,N-1,0.
	 *  This is like the difference between an open polyline and a closed
	 *  polygon.
	 */
	void SetLoop(bool bFlag);
	/// Return true if interpolation is set to loop back to the first point.
	bool GetLoop() const { return m_bLoop; }

	bool Write(const char *fname);
	bool Read(const char *fname);

	bool CreateFromLineString(const vtCRS &crs, vtFeatureSet *pSet);

protected:
	void InterpolateControlPoints(TimeControlPointMap::const_iterator &a,
								  TimeControlPointMap::const_iterator &b,
								  double time,
								  ControlPoint &c) const;
	TimeControlPointMap m_TimeControlPointMap;

	InterpMode		m_InterpMode;
	CubicSpline		m_Spline;
	bool			m_bLoop;
	double			m_fLoopSegmentTime;
	ControlPoint	m_LoopControlPoint;

	// For dealing with the global CRS.
	vtCRS	m_crs;
	OCTransform		*m_pConvertToWGS;
	OCTransform		*m_pConvertFromWGS;
	LocalCS m_conv;
	friend class AnimPathVisitor;
};
typedef osg::ref_ptr<vtAnimPath> vtAnimPathPtr;

/**
 * This class connects a path (vtAnimPath) to any number of targets.  The
 * targets can be any transform (such as scene graph object or camera)
 * which will be moved along the path based on time.
 *
 * Speed is relative.  For example, calling SetSpeed(2.0) means that an
 * animation path with control points from 0 to 10 seconds will be played
 * in 5 seconds.  Default speed is 1.0.
 *
 * Other aspects of playback you can control include continuous play
 * (SetContinuous) and position only (SetPosOnly) which is useful if you
 * want to move a camera along a path but allow the user to freely look
 * around.
 */
class vtAnimPathEngine : public vtEngine
{
public:
	vtAnimPathEngine():
		m_pAnimationPath(NULL),
		m_bContinuous(false),
		m_fLastTime(FLT_MAX),
		m_fTime(0.0f),
		m_bPosOnly(false),
		m_fSpeed(1.0f) {}

	vtAnimPathEngine(vtAnimPath *ap):
		m_pAnimationPath(ap),
		m_bContinuous(false),
		m_fLastTime(FLT_MAX),
		m_fTime(0.0f),
		m_bPosOnly(false),
		m_fSpeed(1.0f) {}

	/// Set the animation path for this engine to use.
	void SetAnimationPath(vtAnimPath* path) { m_pAnimationPath = path; }
	/// Get the animation path associated with this engine.
	vtAnimPath* GetAnimationPath() { return m_pAnimationPath.get(); }
	/// Get the animation path associated with this engine.
	const vtAnimPath* GetAnimationPath() const { return m_pAnimationPath; }

	void UpdateTargets();
	void Reset();

	/// Set the play speed, in time units per second.  Default is 1.0.
	void SetSpeed(float fSpeed) { m_fSpeed = fSpeed; }
	/// Get the play speed.
	float GetSpeed() const { return m_fSpeed; }

	/** Set to true for continuous play, meaning that each time it reaches
	 *  the end of the animation path, it continues from the beginning without
	 *  stopping. */
	void SetContinuous(bool bFlag) { m_bContinuous = bFlag; }
	/// Return true if set for continuous play.
	bool GetContinuous() const { return m_bContinuous; }

	/** Set to true for play to ignore the orientation of the animation path,
	 *  so that only the position of the targets is affected. */
	void SetPosOnly(bool bFlag) { m_bPosOnly = bFlag; }
	/// Return true if set to use position only.
	bool GetPosOnly() const { return m_bPosOnly; }

	/// Virtual handler, will be called every frame to do the work of the engine.
	virtual void Eval();
	virtual void SetEnabled(bool bOn);

	/** Set the play time, which should be in the range of 0.0 to the length
	 *  of the animation path.  This will affect the targets the next time
	 *  Eval() or UpdateTargets() is called. */
	void SetTime(float fTime) { m_fTime = fTime; }
	/** Set the play time, which will be in the range of 0.0 to the length of
	 *  the animation path. */
	float GetTime() { return m_fTime; }

public:
	vtAnimPathPtr m_pAnimationPath;		// the engine owns the animpath
	bool	m_bContinuous;
	float	m_fLastTime;
	float	m_fTime;
	bool	m_bPosOnly;
	float	m_fSpeed;
};

/* Convenience classes for organizing a set of animation paths with their engines. */
struct vtAnimEntry
{
	vtAnimPath *m_pAnim;
	vtAnimPathEngine *m_pEngine;
	vtString m_Name;
};

class vtAnimContainer : public std::vector<vtAnimEntry>
{
public:
	void SetEngineContainer(vtEngine *pContainer)
	{
		m_pParentEngine = pContainer;
	}
	void AppendEntry(const vtAnimEntry &pEntry);

protected:
	vtEnginePtr m_pParentEngine;
};

/*@}*/  // nav

#endif // ANIMPATHH

