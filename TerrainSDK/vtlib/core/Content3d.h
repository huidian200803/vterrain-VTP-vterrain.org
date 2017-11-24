//
// Content3d.h
//
// Header for the Content Management classes.
//
// Copyright (c) 2003-2011 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#ifndef VTLIB_CONTENT3DH
#define VTLIB_CONTENT3DH

/** \defgroup content Content Management
 * These classes are used for managing 3D model content - generally,
 * collections of 3D models with metadata.
 */
/*@{*/

#include "vtdata/Content.h"

/**
 * Subclass of vtItem which is capable of instantiating the corresponding
 * 3D geometry for the item.
 */
class vtItem3d : public vtItem
{
public:
	bool LoadModels();
	void UpdateExtents();

	NodePtr m_pNode;
};

/**
 * Subclass of vtContentManager which is capable of instantiating the
 * corresponding 3D geometry for the content items.
 */
class vtContentManager3d : public vtContentManager
{
public:
	vtContentManager3d();
	~vtContentManager3d();

	// implementation
	osg::Node *CreateNodeFromItemname(const char *itemname);
	virtual vtItem *NewItem() { return new vtItem3d; }
	void ReleaseContents();

	vtGroupPtr m_pGroup;

	// Static: handle to the singleton
	static vtContentManager3d *s_pContent;
};

void vtSetGlobalContent(vtContentManager3d &cm3d);
vtContentManager3d &vtGetContent();

/*@}*/	// group content

#endif // VTLIB_CONTENT3DH

