//
// Layer.cpp
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <algorithm>

#include "vtdata/vtLog.h"
#include "vtui/Helper.h"
#include "Builder.h"

// Layer headers
#include "WaterLayer.h"
#include "ElevLayer.h"
#include "ImageLayer.h"
#include "RoadLayer.h"
#include "StructLayer.h"
#include "VegLayer.h"
#include "UtilityLayer.h"
#include "RawLayer.h"

wxArrayString vtLayer::LayerTypeNames;
const wxChar *vtLayer::LayerFileExtension[LAYER_TYPES] =
{
	_T(".shp"),
	_T(".bt"),
	_T(".tif"),
	_T(".rmf"),
	_T(".vtst"),
	_T(".hyd"),
	_T(".vf"),
	_T(".osm")
};


wxString GetLayerTypeName(const LayerType &ltype)
{
	if (ltype == LT_UNKNOWN)
		return wxString(_("Unknown"));
	else
		return vtLayer::LayerTypeNames[ltype];
}

//////////////////////////////////////////////////////////

vtLayer::vtLayer(LayerType type) : vtLayerBase(type)
{
	m_type = type;
	m_bVisible = true;
	m_bModified = false;
	m_bSticky = false;
	m_bNative = false;
}

vtLayer::~vtLayer()
{
}

bool vtLayer::Save(bool progress_callback(int))
{
	if (!m_bNative)
	{
		if (!AskForSaveFilename())
			return false;
	}
	VTLOG("Saving data...");
	bool success = OnSave(progress_callback);
	if (success)
	{
		VTLOG("OK.\n");
		m_bNative = true;
		SetModified(false);
	}
	else
		VTLOG("Failed.\n");

	return success;
}

bool vtLayer::SaveAs(const wxString &filename, bool progress_callback(int))
{
	SetLayerFilename(filename);
	m_bNative = true;
	return Save(progress_callback);
}

bool vtLayer::Load(const wxString &filename)
{
	if (filename != _T(""))
		SetLayerFilename(filename);
	bool success = OnLoad();
	if (success)
		m_bNative = true;
	return success;
}

void vtLayer::OnModifiedChange()
{
	g_bld->RefreshTreeStatus();
}

void vtLayer::SetLayerFilename(const wxString &fname)
{
	VTLOG("Setting layer filename to '%s'\n", (const char *) fname.mb_str(wxConvUTF8));
	bool bNeedRefresh = (m_wsFilename.Cmp(fname) != 0);
	m_wsFilename = fname;
	if (bNeedRefresh)
		g_bld->RefreshTreeStatus();
}

wxString vtLayer::GetSaveFileDialogFilter()
{
	wxString str;

	str = LayerTypeNames[m_type];
	str += _(" Files");
	str += _T(" (*");

	wxString ext = GetFileExtension();
	str += ext;
	str += _T(")|*");
	str += ext;
	str += _("|All Files|*.*");

	return str;
}

bool vtLayer::AskForSaveFilename()
{
	wxString filter = GetSaveFileDialogFilter();
	wxString fname = GetLayerFilename();
#if WIN32
	// The dumb Microsoft file dialog can have trouble with forward slashes.
	fname = ToBackslash(fname);
#endif
	wxFileDialog saveFile(NULL, _("Save Layer"), _T(""), fname,
		filter, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	VTLOG("Asking user for file name\n");
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return false;

	fname = saveFile.GetPath();
	VTLOG("Got filename: '%s'\n", (const char *) fname.mb_str(wxConvUTF8));

	// Add file extension if user didn't specify it
	wxString ext = GetFileExtension();
	if (fname.Len() < ext.Len() || fname.Right(ext.Len()) != ext)
	{
		fname += ext;
	}
	SetLayerFilename(fname);
	m_bNative = true;
	return true;
}

vtString vtLayer::GetExportFilename(const wxString &format_filter) const
{
	wxString filter = _("All Files|*.*");
	AddType(filter, format_filter);

	wxString defaultFile = GetLayerFilename();
	RemoveFileExtensions(defaultFile);

	// ask the user for a filename
	wxFileDialog saveFile(NULL, _("Export"), _T(""), defaultFile,
		filter, wxFD_SAVE);
	saveFile.SetFilterIndex(1);
	if (saveFile.ShowModal() != wxID_OK)
		return vtString("");
	wxString path = saveFile.GetPath();
	vtString path2 = (const char *) path.mb_str(wxConvUTF8);
	return path2;
}

wxString vtLayer::GetFileExtension()
{
	return (wxString) LayerFileExtension[GetType()];
}

void vtLayer::SetMessageText(const wxString &msg)
{
	if (g_bld->m_pParentWindow)
		g_bld->m_pParentWindow->SetStatusText(msg);
}

void vtLayer::GetCRS(vtCRS &crs)
{
	// shouldn't get here.
}

void vtLayer::Offset(const DPoint2 &p)
{
	// shouldn't get here unless a layer is incapable of offset
}

/////////////////////////////////////////////////

// static
//
vtLayer *vtLayer::CreateNewLayer(LayerType ltype)
{
	vtLayer *pLayer = NULL;
	switch (ltype)
	{
	case LT_RAW:
		pLayer = new vtRawLayer;
		break;
	case LT_ELEVATION:
		pLayer = new vtElevLayer;
		break;
	case LT_IMAGE:
		pLayer = new vtImageLayer;
		break;
	case LT_ROAD:
		pLayer = new vtRoadLayer;
		break;
	case LT_STRUCTURE:
		pLayer = new vtStructureLayer;
		break;
	case LT_UTILITY:
		pLayer = new vtUtilityLayer;
		break;
	case LT_WATER:
		pLayer = new vtWaterLayer;
		break;
	case LT_VEG:
		pLayer = new vtVegLayer;
		break;
	default:	// Keep picky compilers quiet.
		break;
	}
	return pLayer;
}

////////////////////////////////////////////////

LayerArray::~LayerArray()
{
	if (m_bOwnLayers)
		DeleteLayers();
}

void LayerArray::Remove(vtLayer *lay)
{
	iterator it = std::find(begin(), end(), lay);
	if (it != end())
		erase(it);
}

void LayerArray::DeleteLayers()
{
	for (uint i = 0; i < size(); ++i)
	{
		vtLayerPtr lp = at(i);

		// safety check
		assert(lp->GetType() >= LT_UNKNOWN && lp->GetType() < LAYER_TYPES);

		delete lp;
	}
	clear();
}

vtLayer *LayerArray::FindByFilename(const wxString &name)
{
	for (uint i = 0; i < size(); i++)
	{
		if (at(i)->GetLayerFilename() == name)
			return at(i);
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////

DrawStyle::DrawStyle()
{
	m_LineColor.Set(0,0,0);
	m_FillColor.Set(255,255,255);
	m_bFill = false;
}

