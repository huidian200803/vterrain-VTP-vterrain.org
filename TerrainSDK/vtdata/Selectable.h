//
// Simple class to represent selectability
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef SELECTEABLEH
#define SELECTEABLEH

class Selectable
{
public:
	Selectable() { m_bSelect = false; }

	void Select(bool b) { m_bSelect = b; }
	bool IsSelected() const { return m_bSelect; }
	void ToggleSelect() { m_bSelect = !m_bSelect; }

protected:
	bool	m_bSelect;	//true if node is selected
};

#endif
