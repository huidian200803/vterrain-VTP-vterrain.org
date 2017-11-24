//
// StructureLayer.h
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef STRUCTLAYER_H
#define STRUCTLAYER_H

#include "vtdata/shapelib/shapefil.h"
#include "vtdata/Features.h"
#include "vtdata/StructArray.h"
#include "vtdata/Features.h"
#include "Layer.h"

class vtDLGFile;
class vtElevLayer;
class vtScaledView;

//////////////////////////////////////////////////////////

class vtStructureLayer : public vtLayer, public vtStructureArray
{
public:
	vtStructureLayer();

	// Implement vtLayer methods
	bool GetExtent(DRECT &rect);
	void DrawLayer(vtScaledView *pView, UIContext &ui);
	bool TransformCoords(vtCRS &crs);
	bool OnSave(bool progress_callback(int) = NULL);
	bool OnLoad();
	void GetCRS(vtCRS &crs);
	void SetCRS(const vtCRS &crs);
	bool AppendDataFrom(vtLayer *pL);
	void Offset(const DPoint2 &p);
	void GetPropertyText(wxString &str);
	bool AskForSaveFilename();

	wxString GetLayerFilename() { return wxString(GetFilename(), wxConvUTF8); }
	void SetLayerFilename(const wxString &fname)
	{
		SetFilename((const char *) fname.mb_str(wxConvUTF8));
		vtLayer::SetLayerFilename(fname);
	}

	// UI event handlers
	void OnLeftDown(BuilderView *pView, UIContext &ui);
	void OnLeftUp(BuilderView *pView, UIContext &ui);
	void OnRightDown(BuilderView *pView, UIContext &ui);
	void OnMouseMove(BuilderView *pView, UIContext &ui);

	void OnLeftDownEditBuilding(BuilderView *pView, UIContext &ui);
	void OnLeftDownBldAddPoints(BuilderView *pView, UIContext &ui);
	void OnLeftDownBldDeletePoints(BuilderView *pView, UIContext &ui);
	void OnLeftDownEditLinear(BuilderView *pView, UIContext &ui);
	void OnLeftDownAddInstance(BuilderView *pView, UIContext &ui, vtTagArray *tags);
	void UpdateMove(UIContext &ui);
	void UpdateRotate(UIContext &ui);
	void UpdateResizeScale(BuilderView *pView, UIContext &ui);

	void DrawBuildingHighlight(vtScaledView *pView);
	bool AddElementsFromSHP(const wxString &filename, const vtCRS &crs, DRECT rect);
	void AddElementsFromDLG(vtDLGFile *pDlg);

	bool EditBuildingProperties();
	void AddFoundations(vtElevLayer *pEL);

	void DrawBuilding(vtScaledView *pView, vtBuilding *bld);
	void DrawLinear(vtScaledView *pView, vtFence *fen);

	// inverts selection values on all structures.
	void InvertSelection();
	void DeselectAll();
	int DoBoxSelect(const DRECT &rect, SelectionType st);

	// override to catch edit hightlighting
	virtual void SetEditedEdge(vtBuilding *bld, int lev, int edge);

	void ResolveInstancesOfItems();
	void CleanFootprints(double epsilon, int &degenerate, int &overlapping);

protected:
	void DrawStructures(vtScaledView *pView, bool bOnlySelected);

	bool	m_bPreferGZip;	// user wants their elevation treated as a .gz file};
	vtScaledView *m_pLastView;
};

#endif	// STRUCTLAYER_H

