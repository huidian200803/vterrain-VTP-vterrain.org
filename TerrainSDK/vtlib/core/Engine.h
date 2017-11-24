//
// Engine.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

#include "vtdata/vtString.h"
#include "Event.h"

class vtWindow;

/** \defgroup eng Engines and Events
 * These classes are used for per-frame simulation and user interaction.
 * See the <a href="group__nav.html">Navigation</a> module for engines
 * which are specifically for navigation.
 */
/*@{*/

/**
 * This class simply provides the ability to store whether an object is
 * "enabled" or not.  This is generally useful, such as for Nodes or Engines
 * which can be turned on and off.
 */
class vtEnabledBase
{
public:
	vtEnabledBase() { m_bEnabled = true; }

	virtual void SetEnabled(bool bOn) { m_bEnabled = bOn; }
	bool GetEnabled() { return m_bEnabled; }

protected:
	bool m_bEnabled;
};

typedef osg::ref_ptr<class vtEngine> vtEnginePtr;
typedef osg::Referenced* ReferencePtr;

/**
 * The vtEngine class represents an Engine, which is a convenient way to
 * encapsulate an application behavior which occurs every frame.  An Engine
 * typically has a target, which is another object which it affects.  The
 * Engine's method Eval() is called each frame, at which point it can do
 * whatever simulation or computation it desires, and then update its target.
 * \par
 * For example, you may have a node in your scene graph which represents a fish.
 * You could then create a Engine class (e.g. FishEngine, subclass of vtEngine)
 * which simulates the behavior of a fish.  For each fish you create, you would
 * also create a FishEngine and set the Engine's target to be the fish.
 */
class vtEngine : public vtEnabledBase, public osg::Referenced
{
public:
	vtEngine();

	/**
	 * Get a target from the engine.  Most engines will have only a single
	 * target.
	 * \param which The number of the target (default 0).
	 */
	osg::Referenced *GetTarget(uint which = 0);
	/** Adds a target to the engine. */
	void AddTarget(osg::Referenced *ptr) { m_Targets.push_back(ptr); }
	/** Removes a target from the engine. */
	void RemoveTarget(osg::Referenced *ptr);

	/// Return the number of targets for this engine.
	uint NumTargets() { return m_Targets.size(); }

	void setName(const char *str) { m_strName = str; }
	const char *getName() { return m_strName; }

	/// Virtual handler, to catch mouse events, can be overridden by your engine class.
	virtual void OnMouse(vtMouseEvent &event);

	/// Virtual handler, to catch keyboard events, can be overridden by your engine class.
	virtual void OnKey(int key, int flags);

	/// Virtual handler, to catch resize events, can be overridden by your engine class.
	virtual void OnWindowSize(int width, int height);

	/** Virtual handler, will be called every frame to do the work of the engine.  You
	 * must override this if you want your engine to be useful.
	 */
	virtual void Eval();

	// an engine may be associate with a window
	void SetWindow(vtWindow *pWin) { m_pWindow = pWin; }
	vtWindow *GetWindow() { return m_pWindow; }

	// Engine tree methods
	void AddChild(vtEngine *pEngine) { m_Children.push_back(pEngine); }
	void RemoveChild(vtEngine *pEngine);
	vtEngine *GetChild(uint i) { return m_Children[i].get(); }
	uint NumChildren() { return m_Children.size(); }

	void AddChildrenToList(class vtEngineArray &list, bool bEnabledOnly);

protected:
	std::vector<ReferencePtr> m_Targets;
	std::vector<vtEnginePtr> m_Children;
	vtString		 m_strName;
	vtWindow		*m_pWindow;

protected:
	~vtEngine() {}
};


class vtEngineArray : public std::vector<vtEngine*>
{
public:
	vtEngineArray(vtEngine *pTop, bool bEnabledOnly = true)
	{
		if (pTop)
			pTop->AddChildrenToList(*this, bEnabledOnly);
	}
};


/**
 * This simple engine extends the base class vtEngine with the ability to
 * remember the last state of the mouse.  This is useful for engines which
 * simply need to know mouse state (cursor position and button state).  They
 * can simply subclass from vtMouseStateEngine and use the inherited fields rather
 * than having to catch mouse events.
 */
class vtMouseStateEngine : public vtEngine
{
public:
	vtMouseStateEngine();

	void OnMouse(vtMouseEvent &event);

	void GetNormalizedMouseCoords(float &mx, float &my);

	/** The state of the mouse buttons */
	int m_buttons;

	/** The location of the mouse cursor */
	IPoint2 m_pos;

	/** Last known state of the modifier flags */
	int m_flags;

protected:
	virtual ~vtMouseStateEngine() {}
};

/**
 * A simple "Billboard" engine which turns its target to face the
 * camera each frame.
 */
class vtSimpleBillboardEngine : public vtEngine
{
public:
	vtSimpleBillboardEngine(float fAngleOffset = 0.0f);
	virtual ~vtSimpleBillboardEngine() {}

	void SetPitch(bool bFlag) { m_bPitch = bFlag; }

	void Eval();

	bool m_bPitch;
	float m_fAngleOffset;
};

/*@}*/	// Group eng

