//
// Layer.h
//
// Copyright (c) 2001-2015 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

#include "vtdata/MathTypes.h"
#include "vtdata/vtString.h"
#include "vtdata/LayerBase.h"

class BuilderView;
class vtScaledView;
class vtCRS;
struct UIContext;

/**
 * A VTBuilder layer is a single blob of data, generally stored in memory and drawn
 * in the main view.  Each layer has a type (Raw, Elevation, Image, etc.) and a CRS.
 * vtLayer is an abstract base class for all the layer classes.
 * It defines a set of operations which each layer class may implement.
 */
class vtLayer : public vtLayerBase
{
public:
	vtLayer(LayerType type);
	virtual ~vtLayer();

	void SetSticky(bool bSticky) { m_bSticky = bSticky; }
	bool GetSticky() { return m_bSticky; }
	bool IsNative() { return m_bNative; }

	// Implement LayerBase method
	virtual void OnModifiedChange();

	wxString GetImportedFrom() { return m_wsImportedFrom; }
	void SetImportedFrom(const wxString &fname) { m_wsImportedFrom = fname; }

	// operations
	static vtLayer *CreateNewLayer(LayerType ltype);
	bool Save(bool progress_callback(int) = NULL);
	bool SaveAs(const wxString &filename = _T(""), bool progress_callback(int) = NULL);
	bool Load(const wxString &filename = _T(""));

	// these must be implemented:
	/// Get the extents
	virtual bool GetExtent(DRECT &rect) = 0;
	virtual void DrawLayer(vtScaledView *pView, UIContext &ui) = 0;
	/// Transform the coordinates into another CRS
	virtual bool TransformCoords(vtCRS &crs) = 0;
	virtual bool OnSave(bool progress_callback(int) = NULL) = 0;
	virtual bool OnLoad() = 0;
	/// Merge the contents of another layer (of the same type)
	virtual bool AppendDataFrom(vtLayer *pL) = 0;
	/// Get the CRS
	virtual void GetCRS(vtCRS &crs) = 0;
	/// Set the CRS, which does not reproject
	virtual void SetCRS(const vtCRS &crs) = 0;
	/// Shift all the coordinates by a horizontal offset
	virtual void Offset(const DPoint2 &delta);

	// these may be optionally implemented:
	virtual bool SetExtent(const DRECT &rect) { return false; }
	virtual void GetPropertyText(wxString &str) {}
	virtual wxString GetFileExtension();
	virtual bool CanBeSaved() { return true; }
	virtual wxString GetLayerFilename() const { return m_wsFilename; }
	virtual void SetLayerFilename(const wxString &fname);
	virtual bool AskForSaveFilename();
	virtual bool GetAreaExtent(DRECT &rect) { return GetExtent(rect); }

	// UI event handlers which can be implemented if desired
	virtual void OnLeftDown(BuilderView *pView, UIContext &ui) {}
	virtual void OnLeftUp(BuilderView *pView, UIContext &ui) {}
	virtual void OnRightDown(BuilderView *pView, UIContext &ui) {}
	virtual void OnRightUp(BuilderView *pView, UIContext &ui) {}
	virtual void OnLeftDoubleClick(BuilderView *pView, UIContext &ui) {}
	virtual void OnMouseMove(BuilderView *pView, UIContext &ui) {}

	vtString GetExportFilename(const wxString &format_filter) const;

	static wxArrayString LayerTypeNames;
	static const wxChar *LayerFileExtension[];

protected:
	wxString GetSaveFileDialogFilter();
	void SetMessageText(const wxString &msg);

	// this filename is only used if the layer subclass doesn't have its own
	wxString	m_wsFilename;

	// remember what file this layer was imported from
	wxString	m_wsImportedFrom;

	bool		m_bNative;
	bool		m_bSticky;		// If sticky, don't page out the layer
};

typedef vtLayer *vtLayerPtr;


//
// Name: LayerArray
// An array of layer objects.
//
class LayerArray : public std::vector<vtLayerPtr>
{
public:
	LayerArray() : m_bOwnLayers(true) {}
	~LayerArray();

	void Remove(vtLayer *lay);
	void DeleteLayers();
	vtLayer *FindByFilename(const wxString &name);

	/** If true, this array owns the layers it contains, so it will delete them
	    when the array is deleted. Default is true. */
	void SetOwnership(bool bOwn) { m_bOwnLayers = bOwn; }

private:
	bool	m_bOwnLayers;
};

class DrawStyle
{
public:
	DrawStyle();

	RGBi m_LineColor;
	RGBi m_FillColor;
	bool m_bFill;
	//int m_MarkerShape;	// 0 = circle, 1 = crosshair, this should be an enum
	int m_MarkerSize;	// in pixels
};

////////////////////
// Helpers

wxString GetLayerTypeName(const LayerType &lype);

