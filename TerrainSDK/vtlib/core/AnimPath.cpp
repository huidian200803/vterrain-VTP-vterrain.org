//
// AnimPath.cpp
//
// Implementation animation path capabilities.
//
// Copyright (c) 2004-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "AnimPath.h"
#include "vtdata/vtLog.h"
#include "vtdata/FilePath.h"


void ControlPoint::Interpolate(double ratio, const ControlPoint &first, const ControlPoint &second)
{
	double one_minus_ratio = 1.0 - ratio;
	m_Position = first.m_Position*one_minus_ratio + second.m_Position*ratio;
	m_Rotation.Slerp(first.m_Rotation, second.m_Rotation, ratio);
}

void ControlPoint::GetMatrix(FMatrix4 &matrix, bool bPosOnly) const
{
	matrix.Identity();

	if (!bPosOnly)
	{
		FMatrix3 m3;
		m_Rotation.GetMatrix(m3);

		FMatrix4 m4;
		m4.SetFromMatrix3(m3);
		matrix.PostMult(m4);
	}

	matrix.Translate(m_Position);
}


///////////////////////////////////////////////////////////////////////

vtAnimPath::vtAnimPath() :
	m_InterpMode(LINEAR),
	m_bLoop(false),
	m_fLoopSegmentTime(0.0f)
{
	m_pConvertToWGS = NULL;
	m_pConvertFromWGS = NULL;
}

vtAnimPath::vtAnimPath(const vtAnimPath &ap) :
	m_TimeControlPointMap(ap.m_TimeControlPointMap),
	m_InterpMode(ap.m_InterpMode),
	m_bLoop(ap.m_bLoop),
	m_fLoopSegmentTime(0.0f)
{
	m_pConvertToWGS = NULL;
	m_pConvertFromWGS = NULL;
}

vtAnimPath::~vtAnimPath()
{
	delete m_pConvertToWGS;
	delete m_pConvertFromWGS;
}

bool vtAnimPath::SetCRS(const vtCRS &crs, const LocalCS &conv)
{
	m_crs = crs;
	m_conv = conv;

	// Prepare to convert from local to global CRS
	vtCRS global_crs;
	OGRErr err = global_crs.SetGeogCSFromDatum(EPSG_DATUM_WGS84);
	if (err != OGRERR_NONE)
		return false;

	delete m_pConvertToWGS;
	delete m_pConvertFromWGS;

	m_pConvertToWGS = CreateCoordTransform(&m_crs, &global_crs, true);
	m_pConvertFromWGS = CreateCoordTransform(&global_crs, &m_crs, true);

	return true;
}

void vtAnimPath::Insert(double time,const ControlPoint &controlPoint)
{
	m_TimeControlPointMap[time] = controlPoint;
}

void vtAnimPath::RemovePoint(int index)
{
	std::map<double,ControlPoint> :: iterator iter;
	int count = 0;
	for (iter = m_TimeControlPointMap.begin();
		iter != m_TimeControlPointMap.end();
		iter++)
	{
		if (count == index)
		{
			m_TimeControlPointMap.erase(iter);
			return;
		}
		count++;
	}
}

double vtAnimPath::GetFirstTime() const
{
	if (m_TimeControlPointMap.empty())
		return 0.0;
	return m_TimeControlPointMap.begin()->first;
}

double vtAnimPath::GetLastTime() const
{
	if (m_TimeControlPointMap.empty())
		return 0.0;
	if (m_bLoop && m_TimeControlPointMap.size() >= 2)
		return m_fLoopSegmentTime;
	else
		return m_TimeControlPointMap.rbegin()->first;
}

void vtAnimPath::SetLoop(bool bFlag)
{
	m_bLoop = bFlag;

	// must reprocess with smoothing of the beginning and end
	if (m_TimeControlPointMap.size() >= 2)
		ProcessPoints();
}

void vtAnimPath::SetTimeFromLinearDistance()
{
	FPoint3 pos, last;
	float fTime=0;

	TimeControlPointMap newmap;

	for (TimeControlPointMap::iterator it = m_TimeControlPointMap.begin();
		it != m_TimeControlPointMap.end(); it++)
	{
		pos = it->second.m_Position;
		if (it != m_TimeControlPointMap.begin())
		{
			float dist = (pos - last).Length();
			fTime += dist;
		}
		last = pos;

		newmap[fTime] = it->second;
	}
	// Now switch to the new map
	m_TimeControlPointMap = newmap;
}

void vtAnimPath::ProcessPoints()
{
	m_Spline.Cleanup();

	if (m_TimeControlPointMap.size() < 2)
	{
		m_fLoopSegmentTime = 0.0f;
		return;
	}

	TimeControlPointMap::iterator first = m_TimeControlPointMap.begin();
	TimeControlPointMap::iterator last = m_TimeControlPointMap.end();
	last--;

	if (m_bLoop)
	{
		// Estimate time for last segment to make it loop
		TimeControlPointMap::const_iterator it0 = m_TimeControlPointMap.begin();
		TimeControlPointMap::const_iterator it1 = it0;
		it1++;
		TimeControlPointMap::const_reverse_iterator it2 = m_TimeControlPointMap.rbegin();

		// consider first two points
		double time_diff = (it1->first - it0->first);
		float pos_diff = (it1->second.m_Position - it0->second.m_Position).Length();
		double speed = pos_diff / time_diff;

		// then do the same for the last two
		pos_diff = (it0->second.m_Position - it2->second.m_Position).Length();
		m_fLoopSegmentTime = it2->first + (pos_diff/speed);

		m_LoopControlPoint = it0->second;
	}

	TimeControlPointMap::iterator it;

	if (m_bLoop)
	{
		// one more at the beginning to wraparound-loop smoothly
		m_Spline.AddPoint(last->second.m_Position);
	}

	it = first;
	int i = 0;
	while (it != m_TimeControlPointMap.end())
	{
		it->second.m_iIndex = i;
		m_Spline.AddPoint(it->second.m_Position);
		it++;
		i++;
	}

	if (m_bLoop)
	{
		// two more at the end to wraparound-loop smoothly
		it = first;
		m_Spline.AddPoint(it->second.m_Position);
		it++;
		m_Spline.AddPoint(it->second.m_Position);
	}

	// now create smooth curve, in case it's needed later
	m_Spline.Generate();
}

void vtAnimPath::InterpolateControlPoints(TimeControlPointMap::const_iterator &a,
										  TimeControlPointMap::const_iterator &b,
										  double time,
										  ControlPoint &result) const
{
	double delta_time = b->first - a->first;
	if (delta_time == 0.0)
		result = a->second;
	else
	{
		double elapsed = time - a->first;
		double ratio = elapsed/delta_time;

		result.Interpolate(ratio, a->second, b->second);
		if (m_InterpMode == CUBIC_SPLINE)
		{
			// Don't use that linear position.
			// Find the position on the spline.
			int num = a->second.m_iIndex;

			if (m_bLoop)	// indices are +1 for loop spline
				num++;

			DPoint3 dpos;
			m_Spline.Interpolate(num + ratio, &dpos);
			result.m_Position = dpos;
		}
	}
}

bool vtAnimPath::GetInterpolatedControlPoint(double time, ControlPoint &controlPoint) const
{
	if (m_TimeControlPointMap.empty())
		return false;

	TimeControlPointMap::const_iterator second = m_TimeControlPointMap.lower_bound(time);
	if (second==m_TimeControlPointMap.begin())
	{
		controlPoint = second->second;
	}
	else if (second != m_TimeControlPointMap.end())
	{
		TimeControlPointMap::const_iterator first = second;
		--first;
		// we have both a lower bound and the next item.
		InterpolateControlPoints(first, second, time, controlPoint);
	}
	else // (second==_timeControlPointMap.end())
	{
		if (m_bLoop && m_TimeControlPointMap.size() >= 2)
		{
			TimeControlPointMap dummy;
			dummy[m_fLoopSegmentTime] = m_LoopControlPoint;
			TimeControlPointMap::const_iterator it2 = dummy.begin();

			second--;
			InterpolateControlPoints(second, it2, time, controlPoint);
		}
		else
			controlPoint = m_TimeControlPointMap.rbegin()->second;
	}
	return true;
}

float vtAnimPath::GetTotalTime()
{
	if (m_bLoop)
	{
	}
	return 0;
}


/**
 * Write the animation path to a .vtap file, a simple XML file format.
 */
bool vtAnimPath::Write(const char *fname)
{
	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	FILE *fp = vtFileOpen(fname, "wb");
	if (!fp) return false;

	fprintf(fp, "<?xml version=\"1.0\"?>\n");
	fprintf(fp, "<animation-path file-format-version=\"1.0\">\n");

	FMatrix3 m3;
	FPoint3 vector;
	FPoint3 world_p1, world_p2;
	DPoint3 earth_p1, earth_p2;

	TimeControlPointMap::iterator it = m_TimeControlPointMap.begin();
	while (it != m_TimeControlPointMap.end())
	{
		const ControlPoint &point = it->second;

		point.m_Rotation.GetMatrix(m3);
		m3.Transform(FPoint3(0,0,-1), vector);

		world_p1 = point.m_Position;
		world_p2 = world_p1 + vector;
		m_conv.LocalToEarth(world_p1, earth_p1);
		m_conv.LocalToEarth(world_p2, earth_p2);

		int result = 0;
		result += m_pConvertToWGS->Transform(1, &earth_p1.x, &earth_p1.y);
		result += m_pConvertToWGS->Transform(1, &earth_p2.x, &earth_p2.y);

		fprintf(fp, "\t<location");

		fprintf(fp, " p1=\"%.12lf,%.12lf,%.2lf\"",
			earth_p1.x, earth_p1.y, earth_p1.z);
		fprintf(fp, " p2=\"%.12lf,%.12lf,%.2lf\"",
			earth_p2.x, earth_p2.y, earth_p2.z);
		fprintf(fp, " time=\"%.2f\"", it->first);

		// Attributes that might be added later include:
		// 1. roll
		// 2. camera parameters (fov, orthographic, etc.) although those
		// don't apply to the locations of non-camera objects.

		fprintf(fp, " />\n");
		it++;
	}
	fprintf(fp, "</animation-path>\n");
	fclose(fp);
	return true;
}

/////////////////////////////////////////////////////////////////////////////
/**
 * Internal class used by the vtAnimPath class to read .vtap XML files.
 */
class AnimPathVisitor : public XMLVisitor
{
public:
	AnimPathVisitor(vtAnimPath *path) { m_path = path; }
	void startXML() {}
	void endXML() {}
	void startElement(const char *name, const XMLAttributes &atts);
	void endElement(const char *name) {}
	void data(const char *s, int length) {}
	LocalCS m_conv;
private:
	vtAnimPath *m_path;
};

void AnimPathVisitor::startElement(const char *name, const XMLAttributes &atts)
{
	const char *attval;

	if (!strcmp(name, "location"))
	{
		DPoint3 earth_p1, earth_p2;
		FPoint3 world_p1, world_p2, vector;
		float time;

		attval = atts.getValue("p1");
		if (attval)
			sscanf(attval, "%lf,%lf,%lf", &earth_p1.x, &earth_p1.y, &earth_p1.z);
		attval = atts.getValue("p2");
		if (attval)
			sscanf(attval, "%lf,%lf,%lf", &earth_p2.x, &earth_p2.y, &earth_p2.z);
		attval = atts.getValue("time");
		if (attval)
			sscanf(attval, "%f", &time);

		int result = 0;
		result += m_path->m_pConvertFromWGS->Transform(1, &earth_p1.x, &earth_p1.y);
		result += m_path->m_pConvertFromWGS->Transform(1, &earth_p2.x, &earth_p2.y);

		m_conv.EarthToLocal(earth_p1, world_p1);
		m_conv.EarthToLocal(earth_p2, world_p2);

		ControlPoint point;
		point.m_Position = world_p1;
		vector = world_p2 - world_p1;
		point.m_Rotation.SetFromVector(vector);

		m_path->Insert(time, point);
	}
}

/////////////////////////////////////////////

/**
 * Read the animation path from a .vtap file, a simple XML file format.
 */
bool vtAnimPath::Read(const char *fname)
{
	// Avoid trouble with '.' and ',' in Europe
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	// Clear before loading
	Clear();

	AnimPathVisitor visitor(this);
	visitor.m_conv = m_conv;
	try
	{
		readXML(fname, visitor);
	}
	catch (xh_io_exception &exp)
	{
		// TODO: would be good to pass back the error message.
		VTLOG("XML parsing error: %s\n", exp.getFormattedMessage().c_str());
		return false;
	}
	return true;
}


///////////////////////////////////////////////////////////////////////

bool vtAnimPath::CreateFromLineString(const vtCRS &crs,
										vtFeatureSet *pSet)
{
	vtFeatureSetLineString *pSetLS2 = dynamic_cast<vtFeatureSetLineString *>(pSet);
	vtFeatureSetLineString3D *pSetLS3 = dynamic_cast<vtFeatureSetLineString3D *>(pSet);
	if (!pSetLS2 && !pSetLS3)
		return false;

	// Clear our control points because we're going to fill it
	m_TimeControlPointMap.clear();

	ScopedOCTransform trans;
	vtCRS &line_crs = pSet->GetAtCRS();
	if (!crs.IsSame(&line_crs))
	{
		// need transformation from feature CRS to terrain CRS
		trans.set(CreateCoordTransform(&line_crs, &crs, true));
	}

	FPoint3 pos;
	uint i = 0, j;	// only first entity

	FLine3 fline;
	if (pSetLS2)
	{
		DPoint2 current, previous(1E9,1E9);
		const DLine2 &dline2 = pSetLS2->GetPolyLine(i);
		for (j = 0; j < dline2.GetSize(); j++)
		{
			current = dline2[j];

			// Must skip redundant points, smooth (spline) paths don't like them
			if (current == previous)
				continue;

			// Transform 1: feature CRS to terrain CRS
			if (trans)
				trans->Transform(1, &current.x, &current.y);

			// Transform 2: earth CRS to world CRS
			m_conv.EarthToLocal(current.x, current.y, pos.x, pos.z);
			pos.y = 0;

			fline.Append(pos);
			previous = current;
		}
	}
	else if (pSetLS3)
	{
		DPoint3 current, previous(1E9,1E9,1E9);
		const DLine3 &dline3 = pSetLS3->GetPolyLine(i);
		for (j = 0; j < dline3.GetSize(); j++)
		{
			current = dline3[j];

			// Must skip redundant points, smooth (spline) paths don't like them
			if (current == previous)
				continue;

			// Transform 1: feature CRS to terrain CRS
			if (trans)
				trans->Transform(1, &current.x, &current.y);

			// Transform 2: earth CRS to world CRS
			m_conv.EarthToLocal(current.x, current.y, pos.x, pos.z);
			pos.y = (float) current.z;

			fline.Append(pos);
			previous = current;
		}
	}
	double time = 0;
	for (j = 0; j < fline.GetSize(); j++)
	{
		// SHP file has no orientations, so derive from positions
		FPoint3 diff;
		if (j == 0)
			diff = (fline[j+1] - fline[j]);
		else
			diff = (fline[j] - fline[j-1]);

		if (j > 0)
		{
			// estimate time based on linear distance
			float distance = diff.Length();

			// default correlation: 1 approximate meter per second
			time += distance;
		}
		FMatrix3 m3;
		m3.MakeOrientation(diff, true);
		FQuat quat;
		quat.SetFromMatrix(m3);

		// Add as a control point
		Insert(time, ControlPoint(fline[j], quat));
	}

	// Set up spline, in case they want smooth motion
	ProcessPoints();

	return true;
}

/* Convenience classes for organizing a set of animation paths. */
void vtAnimContainer::AppendEntry(const vtAnimEntry &entry)
{
	m_pParentEngine->AddChild(entry.m_pEngine);
	push_back(entry);
}


///////////////////////////////////////////////////////////////////////
// class implementation for vtAnimPathEngine

void vtAnimPathEngine::SetEnabled(bool bOn)
{
	bool bWas = m_bEnabled;
	vtEnabledBase::SetEnabled(bOn);
	if (!bWas && bOn)
	{
		// turning this engine on
		m_fLastTime = vtGetTime();
	}
}

void vtAnimPathEngine::Eval()
{
	if (m_pAnimationPath == NULL)
		return;

	float fNow = vtGetTime();
	if (m_fLastTime==FLT_MAX)
		m_fLastTime = fNow;

	float fElapsed = fNow - m_fLastTime;
	m_fTime += fElapsed * m_fSpeed;

	if (m_fTime > m_pAnimationPath->GetLastTime())
	{
		if (m_bContinuous)
		{
			// wrap around
			m_fTime -= (float) m_pAnimationPath->GetPeriod();
		}
		else
		{
			// stop at the end
			m_fTime = (float) m_pAnimationPath->GetLastTime();
			SetEnabled(false);
		}
	}

	UpdateTargets();

	m_fLastTime = fNow;
}

void vtAnimPathEngine::UpdateTargets()
{
	for (uint i = 0; i < NumTargets(); i++)
	{
		osg::Referenced *target = GetTarget(i);
		vtTransform *tr = dynamic_cast<vtTransform*>(target);
		if (!tr)
			continue;

		ControlPoint cp;
		if (m_pAnimationPath->GetInterpolatedControlPoint(m_fTime, cp))
		{
			FMatrix4 matrix;
			cp.GetMatrix(matrix, m_bPosOnly);
			if (m_bPosOnly)
			{
				// Only copy position
				FPoint3 pos = matrix.GetTrans();
				tr->SetTrans(pos);
			}
			else
				tr->SetTransform(matrix);
		}
	}
}

/**
 * Set the engine back to the beginning of its path (time = 0).  This will
 * affect the targets the next time Eval() or UpdateTargets() is called.
 */
void vtAnimPathEngine::Reset()
{
	m_fTime = 0;
}


