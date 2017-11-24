//
// StructLayer.cpp
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/DLG.h"
#include "vtdata/Fence.h"
#include "vtdata/FileFilters.h"
#include "vtdata/ElevationGrid.h"
#include "vtdata/FilePath.h"
#include "vtdata/Triangulate.h"
#include "ogrsf_frmts.h"

#include "Builder.h"
#include "StructLayer.h"
#include "ElevLayer.h"
#include "BuilderView.h"
#include "vtui/BuildingDlg.h"
#include "vtui/Helper.h"
#include "vtui/InstanceDlg.h"
#include "ImportStructDlg.h"

wxPen orangePen;
wxPen yellowPen;
wxPen thickPen;
static bool g_bInitializedPens = false;

//////////////////////////////////////////////////////////////////////////

vtStructureLayer::vtStructureLayer() : vtLayer(LT_STRUCTURE)
{
	wxString name = _("Untitled");
	name += _T(".vtst");
	SetLayerFilename(name);

	m_bPreferGZip = false;

	if (!g_bInitializedPens)
	{
		g_bInitializedPens = true;

		orangePen.SetColour(255,128,0);
		yellowPen.SetColour(255,255,0);

		thickPen.SetColour(255,255,255);
		thickPen.SetWidth(3);
	}
	m_pLastView = NULL;
}

bool vtStructureLayer::GetExtent(DRECT &rect)
{
	if (IsEmpty())
		return false;

	GetExtents(rect);

	// expand by 10 meters (converted to appropriate units)
	DPoint2 offset(10, 10);
	if (m_crs.IsGeographic())
	{
		DPoint2 center;
		rect.GetCenter(center);
		double fMetersPerLongitude = EstimateDegreesToMeters(center.y);
		offset.x /= fMetersPerLongitude;
		offset.y /= METERS_PER_LATITUDE;
	}
	else
	{
		double fMetersPerUnit = GetMetersPerUnit(m_crs.GetUnits());
		offset.x /= fMetersPerUnit;
		offset.y /= fMetersPerUnit;
	}
	rect.Grow(offset.x, offset.y);

	return true;
}

void vtStructureLayer::DrawLayer(vtScaledView *pView, UIContext &ui)
{
	uint structs = size();
	pView->SetColor(RGBi(255, 128, 0));
	DrawStructures(pView, false);	// unselected

	pView->SetColor(RGBi(255, 255, 0));
	DrawStructures(pView, true);	// selected

	DrawBuildingHighlight(pView);

#if 0 // TODO
	// Rubber-banding
	if (ui.m_bLMouseButton && ui.mode == LB_BldEdit && ui.m_bRubber)
	{
		DrawBuilding(pView, &ui.m_EditBuilding);
	}
	if (ui.mode == LB_EditLinear && ui.m_bRubber)
	{
		DrawLinear(pView, &ui.m_EditLinear);
	}
#endif

	// Remember for next time
	m_pLastView = pView;
}

void vtStructureLayer::DrawStructures(vtScaledView *pView, bool bOnlySelected)
{
	uint structs = size();
	for (unsigned i = 0; i < structs; i++)
	{
		// draw each building
		vtStructure *str = at(i);
		if (bOnlySelected && !str->IsSelected())
			continue;

		vtBuilding *bld = str->GetBuilding();
		if (bld)
			DrawBuilding(pView, bld);

		vtFence *fen = str->GetFence();
		if (fen)
			DrawLinear(pView, fen);

		vtStructInstance *inst = str->GetInstance();
		if (inst)
		{
			const DPoint2 &origin = inst->GetPoint();

			pView->DrawXHair(origin, 3);

			if (inst->GetItem())
			{
				FRECT ext = inst->GetItem()->m_extents;
				if (!ext.IsEmpty())
				{
					// TODO
				}
			}
		}
	}
}

void vtStructureLayer::DrawBuildingHighlight(vtScaledView *pView)
{
	if (m_pEditBuilding)
	{
		glLogicOp(GL_INVERT);
		glLineWidth(3.0f);

		const DPolygon2 &dl = m_pEditBuilding->GetFootprint(m_iEditLevel);
		int j = m_iEditEdge;
		int ring = dl.WhichRing(j);
		const DLine2 &dline = dl[ring];
		int sides = dline.GetSize();
		pView->DrawLine(dline[j], dline[(j + 1) % sides]);

		glLogicOp(GL_COPY);
	}
}

void vtStructureLayer::DrawBuilding(vtScaledView *pView, vtBuilding *bld)
{
	DPoint2 center;

	bld->GetBaseLevelCenter(center);

	// crosshair at building center
	pView->DrawXHair(center, 5);

	// draw building footprint for all levels
	int levs = bld->NumLevels();

	for (int i = 0; i < levs; i++)
	{
		const DPolygon2 &dp = bld->GetFootprint(i);
		pView->DrawDPolygon2(dp, false, true);	// no fill, yes circles
	}
}

void vtStructureLayer::DrawLinear(vtScaledView *pView, vtFence *fen)
{
	uint j;

	// draw the line itself
	DLine2 &pts = fen->GetFencePoints();
	pView->DrawPolyLine(pts, false);

	// draw a small crosshair on each vertex
	for (j = 0; j < pts.GetSize(); j++)
	{
		pView->DrawXHair(pts[j], 2);
	}
}

bool vtStructureLayer::OnSave(bool progress_callback(int))
{
	return WriteXML(GetFilename(), m_bPreferGZip);
}

bool vtStructureLayer::OnLoad()
{
	vtString fname = GetFilename();

	// remember whether this layer was read from a compressed file
	if (GetExtension(fname, true).CompareNoCase(".vtst.gz") == 0)
		m_bPreferGZip = true;

	// check file size, show progressm dialog for big files
	int size = GetFileSize(fname);
	bool bShowProgress = (size > 1024*1024);	// 1 MB
	if (m_bPreferGZip)
		bShowProgress = (size > 1024*50);	// 50 KB

	if (bShowProgress)
		OpenProgressDialog(_("Loading Structures"), wxString::FromUTF8((const char *) fname), false);

	bool success = ReadXML(fname, progress_callback);

	if (bShowProgress)
		CloseProgressDialog();

	if (!success)
		return false;

	ResolveInstancesOfItems();
	return true;
}

void vtStructureLayer::ResolveInstancesOfItems()
{
	uint structs = size();
	for (uint i = 0; i < structs; i++)
	{
		vtStructure *str = at(i);
		vtStructInstance *inst = str->GetInstance();
		if (!inst)
			continue;
		g_bld->ResolveInstanceItem(inst);
	}
}

void vtStructureLayer::CleanFootprints(double epsilon, int &degenerate, int &overlapping)
{
	degenerate = 0;
	overlapping = 0;
	for (uint i = 0; i < size(); i++)
	{
		vtStructure *pStructure = at(i);
		vtBuilding *bld = pStructure->GetBuilding();
		if (!bld)
			continue;
		for (uint j = 0; j < bld->NumLevels(); j++)
		{
			vtLevel *lev = bld->GetLevel(j);
			DPolygon2 &dp = lev->GetFootprint();
			int rem = dp.RemoveDegeneratePoints(epsilon);
			degenerate += rem;

			// Also try to catch the case of the polygon looping around
			// over the same points more than once.
			for (uint r = 0; r < dp.size(); r++)
			{
				DLine2 &ring = dp[r];
				for (uint k2 = 1; k2 < ring.GetSize(); k2++)
				{
					const DPoint2 &p2 = ring[k2];
					for (uint k1 = 0; k1 < k2; k1++)
					{
						const DPoint2 &p1 = ring[k1];
						const DPoint2 diff = p1 - p2;
						if (fabs(diff.x) < epsilon && fabs(diff.y) < epsilon)
						{
							ring.RemoveAt(k2);
							k2--;
							overlapping++;
							rem++;
							break;
						}
					}
				}
			}
			if (rem)
			{
				// Must size down the edge arrays
				lev->ResizeEdgesToMatchFootprint();
			}
		}
	}
}

void vtStructureLayer::GetCRS(vtCRS &crs)
{
	crs = m_crs;
}

void vtStructureLayer::SetCRS(const vtCRS &crs)
{
	if (m_crs == crs)
		return;

	m_crs = crs;
	SetModified(true);
}

bool vtStructureLayer::TransformCoords(vtCRS &crs)
{
	if (crs == m_crs)
		return true;

	// Create conversion object.
	vtCRS Source;
	GetCRS(Source);
	ScopedOCTransform trans(CreateCoordTransform(&Source, &crs));
	if (!trans)
		return false;		// Inconvertible coordinate systems.

	DPoint2 loc;
	uint i, j;
	uint count = size();
	for (i = 0; i < count; i++)
	{
		vtStructure *str = at(i);
		vtBuilding *bld = str->GetBuilding();
		if (bld)
			bld->TransformCoords(trans);

		vtFence *fen = str->GetFence();
		if (fen)
		{
			DLine2 line = fen->GetFencePoints();
			for (j = 0; j < line.GetSize(); j++)
				trans->Transform(1, &line[j].x, &line[j].y);
			fen->SetFencePoints(line);
		}

		vtStructInstance *inst = str->GetInstance();
		if (inst)
		{
			loc = inst->GetPoint();
			trans->Transform(1, &loc.x, &loc.y);
			inst->SetPoint(loc);
		}
	}

	// Set the CRS.
	m_crs = crs;
	SetModified(true);

	return true;
}

bool vtStructureLayer::AppendDataFrom(vtLayer *pL)
{
	// safety check
	if (pL->GetType() != LT_STRUCTURE)
		return false;

	vtStructureLayer *pFrom = (vtStructureLayer *)pL;

	int count = pFrom->size();
	for (int i = 0; i < count; i++)
	{
		vtStructure *str = pFrom->at(i);
		push_back(str);
	}
	// tell the source layer that it has no structures (we have taken them)
	pFrom->clear();

	return true;
}

void vtStructureLayer::Offset(const DPoint2 &delta)
{
	vtStructureArray::Offset(delta);
}

void vtStructureLayer::GetPropertyText(wxString &strIn)
{
	strIn += _("Filename: ");
	strIn += GetLayerFilename();
	strIn += _T("\n");

	wxString str;
	str.Printf(_("Number of structures: %d\n"), size());
	strIn += str;

	int bld = 0, lin = 0, ins = 0;
	for (uint i = 0; i < size(); i++)
	{
		const vtStructure *sp = at(i);
		if (sp->GetType() == ST_BUILDING) bld++;
		else if (sp->GetType() == ST_LINEAR) lin++;
		else if (sp->GetType() ==ST_INSTANCE) ins++;
	}
	str.Printf(_("\t %d Buildings (procedural)\n"), bld);
	strIn += str;
	str.Printf(_("\t %d Linears (fences/walls)\n"), lin);
	strIn += str;
	str.Printf(_("\t %d Instances (external 3D models)\n"), ins);
	strIn += str;

	str.Printf(_("Number of selected structures: %d\n"), NumSelected());
	strIn += str;
}

void vtStructureLayer::OnLeftDown(BuilderView *pView, UIContext &ui)
{
	switch (ui.mode)
	{
	case LB_AddInstance:
		if (g_bld->m_pInstanceDlg)
		{
			vtTagArray *tags = g_bld->m_pInstanceDlg->GetTagArray();
			OnLeftDownAddInstance(pView, ui, tags);
		}
		break;
	case LB_AddLinear:
		if (ui.m_pCurLinear == NULL)
		{
			ui.m_pCurLinear = AddNewFence();
			ui.m_pCurLinear->SetParams(g_bld->m_LSOptions);
			ui.m_bRubber = true;
		}
		ui.m_pCurLinear->AddPoint(ui.m_CurLocation);
		pView->Refresh(true);
		break;
	case LB_BldEdit:
		OnLeftDownEditBuilding(pView, ui);
		break;
	case LB_BldAddPoints:
		OnLeftDownBldAddPoints(pView, ui);
		break;
	case LB_BldDeletePoints:
		OnLeftDownBldDeletePoints(pView, ui);
		break;
	case LB_EditLinear:
		OnLeftDownEditLinear(pView, ui);
		// TODO - find nearest point on nearest linear
		break;
	default:
		break;
	}
}

void vtStructureLayer::OnLeftUp(BuilderView *pView, UIContext &ui)
{
	if (ui.mode == LB_BldEdit && ui.m_bRubber)
	{
		pView->Refresh();

		// copy back from temp building to real building
		*ui.m_pCurBuilding = ui.m_EditBuilding;
		ui.m_bRubber = false;
		g_bld->GetActiveLayer()->SetModified(true);
		ui.m_pCurBuilding = NULL;
	}
	if (ui.mode == LB_EditLinear && ui.m_bRubber)
	{
		pView->Refresh();

		// copy back from temp building to real building
		*ui.m_pCurLinear = ui.m_EditLinear;
		ui.m_bRubber = false;
		g_bld->GetActiveLayer()->SetModified(true);
		ui.m_pCurLinear = NULL;
	}
}

/*void ResolveClosest(bool &valid1, bool &valid2, bool &valid3,
					double dist1, double dist2, double dist3)
{
	if (valid1 && valid2)
	{
		if (dist1 < dist2)
			valid2 = false;
		else
			valid1 = false;
	}
	if (valid1 && valid3)
	{
		if (dist1 < dist3)
			valid3 = false;
		else
			valid1 = false;
	}
	if (valid2 && valid3)
	{
		if (dist2 < dist3)
			valid3 = false;
		else
			valid2 = false;
	}
}*/

void vtStructureLayer::OnLeftDownEditBuilding(BuilderView *pView, UIContext &ui)
{
	double epsilon = pView->PixelsToWorld(6).x;  // 6 pixels as world coord

	int building1, building2,  corner;
	double dist1, dist2;

	bool found1 = FindClosestBuildingCenter(ui.m_DownLocation, epsilon, building1, dist1);
	bool found2 = FindClosestBuildingCorner(ui.m_DownLocation, epsilon, building2, corner, dist2);

	if (found1 && found2)
	{
		// which was closer?
		if (dist1 < dist2)
			found2 = false;
		else
			found1 = false;
	}
	if (found1)
	{
		// closest point is a building center
		ui.m_pCurBuilding = at(building1)->GetBuilding();
		ui.m_bDragCenter = true;
	}
	if (found2)
	{
		// closest point is a building corner
		ui.m_pCurBuilding = at(building2)->GetBuilding();
		ui.m_bDragCenter = false;
		ui.m_iCurCorner = corner;
		ui.m_bRotate = ui.m_bControl;
	}
	if (found1 || found2)
	{
		ui.m_bRubber = true;

		// make a copy of the building, to edit and display while dragging
		ui.m_EditBuilding = *ui.m_pCurBuilding;
	}
}

void vtStructureLayer::OnLeftDownBldAddPoints(BuilderView *pView, UIContext &ui)
{
	double dEpsilon = pView->PixelsToWorld(6).x;  // 6 pixels as world coord
	double dClosest;
	int iStructure;

	if (!FindClosestBuilding(ui.m_DownLocation, dEpsilon, iStructure, dClosest))
		return;

	vtBuilding *pBuilding = at(iStructure)->GetBuilding();

	// Variables for levels and edges
	vtLevel *pLevel;
	int i, iIndex, iLevel, iNumLevels;
	DPoint2 Intersection;

	// Since we don't yet do editing of level footprints other than 0,
	//  and editing it affects all levels, it's only useful to do all levels
	iLevel = -1;

	if (-1 == iLevel)
	{
		// Add in all levels
		iNumLevels = pBuilding->NumLevels();
		for (i = 0; i < iNumLevels; i++)
		{
			pLevel = pBuilding->GetLevel(i);
			if (pLevel->GetFootprint().NearestSegment(ui.m_DownLocation,
				iIndex, dClosest, Intersection))
			{
				pLevel->AddEdge(iIndex, Intersection);
			}
		}
	}
	else
	{
		// Add in specified level
		pLevel = pBuilding->GetLevel(iLevel);
		if (pLevel->GetFootprint().NearestSegment(ui.m_DownLocation,
			iIndex, dClosest, Intersection))
		{
			pLevel->AddEdge(iIndex, Intersection);
		}
	}

	pView->Refresh();
}

void vtStructureLayer::OnLeftDownBldDeletePoints(BuilderView *pView, UIContext &ui)
{
	double dEpsilon = pView->PixelsToWorld(6).x;  // 6 pixels as world coord
	double dClosest;
	int iStructure;

	if (!FindClosestBuilding(ui.m_DownLocation, dEpsilon, iStructure, dClosest))
		return;

	vtBuilding *pBuilding = at(iStructure)->GetBuilding();

	// Find extent of building before any point removal
	DRECT Extent;
	pBuilding->GetExtents(Extent);

	vtLevel *pLevel;
	int i, iIndex, iLevel, iNumLevels;

	// Since we don't yet do editing of level footprints other than 0,
	//  and editing it affects all levels, it's only useful to do all levels
	iLevel = -1;

	if (-1 == iLevel)
	{
		// Remove in all levels
		iNumLevels = pBuilding->NumLevels();
		for (i = 0; i <iNumLevels; i++)
		{
			pLevel = pBuilding->GetLevel(i);
			pLevel->GetFootprint().NearestPoint(ui.m_DownLocation, iIndex, dClosest);
			pLevel->DeleteEdge(iIndex);
		}
	}
	else
	{
		// Remove in specified level
		pLevel = pBuilding->GetLevel(iLevel);
		pLevel->GetFootprint().NearestPoint(ui.m_DownLocation, iIndex, dClosest);
		pLevel->DeleteEdge(iIndex);
	}

	pView->Refresh();
}

void vtStructureLayer::OnLeftDownEditLinear(BuilderView *pView, UIContext &ui)
{
	double epsilon = pView->PixelsToWorld(6).x;  // 6 pixels as world coord

	int structure, corner;
	double dist1;

	bool found1 = FindClosestLinearCorner(ui.m_DownLocation, epsilon,
		structure, corner, dist1);

	if (found1)
	{
		// closest point is a building center
		ui.m_pCurLinear = at(structure)->GetFence();
		ui.m_iCurCorner = corner;
		ui.m_bRubber = true;

		// make a copy of the linear, to edit and display while dragging
		ui.m_EditLinear = *ui.m_pCurLinear;
	}
}

void vtStructureLayer::OnLeftDownAddInstance(BuilderView *pView, UIContext &ui, vtTagArray *tags)
{
	vtStructInstance *inst = AddNewInstance();

	inst->SetPoint(ui.m_DownLocation);
	inst->CopyTagsFrom(*tags);

	g_bld->ResolveInstanceItem(inst);

	SetModified(true);
	pView->Refresh();
}

void vtStructureLayer::OnRightDown(BuilderView *pView, UIContext &ui)
{
	if (ui.mode == LB_AddLinear && ui.m_pCurLinear != NULL)
	{
		ui.m_pCurLinear->AddPoint(ui.m_CurLocation);
		pView->Refresh(true);
		ui.m_pCurLinear = NULL;
		ui.m_bRubber = false;
		SetModified(true);
	}
}

void vtStructureLayer::OnMouseMove(BuilderView *pView, UIContext &ui)
{
	if (ui.m_bLMouseButton && ui.mode == LB_BldEdit && ui.m_bRubber)
	{
		// rubber-band a building
		if (ui.m_bDragCenter)
			UpdateMove(ui);
		else if (ui.m_bRotate)
			UpdateRotate(ui);
		else
			UpdateResizeScale(pView, ui);
		pView->Refresh();
	}
	if (ui.mode == LB_AddLinear && ui.m_bRubber)
	{
		//dc.DrawLine(pts[pts.GetSize() - 1], ui.m_CurPoint);
	}
	if (ui.mode == LB_EditLinear && ui.m_bRubber)
	{
		// rubber-band a linear
		ui.m_EditLinear.GetFencePoints().SetAt(ui.m_iCurCorner, ui.m_CurLocation);
		//DrawLinear(pView, &ui.m_EditLinear);
	}
}

void vtStructureLayer::UpdateMove(UIContext &ui)
{
	DPoint2 p;
	DPoint2 moved_by = ui.m_CurLocation - ui.m_DownLocation;

	int i, levs = ui.m_pCurBuilding->NumLevels();
	for (i = 0; i < levs; i++)
	{
		DPolygon2 dl = ui.m_pCurBuilding->GetFootprint(i);
		dl.Add(moved_by);
		ui.m_EditBuilding.SetFootprint(i, dl);
	}
}

void vtStructureLayer::UpdateRotate(UIContext &ui)
{
	DPoint2 origin;
	ui.m_pCurBuilding->GetBaseLevelCenter(origin);

	DPoint2 original_vector = ui.m_DownLocation - origin;
	double angle1 = atan2(original_vector.y, original_vector.x);

	DPoint2 cur_vector = ui.m_CurLocation - origin;
	double angle2 = atan2(cur_vector.y, cur_vector.x);

	double angle_diff = angle2 - angle1;

	DPoint2 p;
	uint i, j, r, levs = ui.m_pCurBuilding->NumLevels();
	for (i = 0; i < levs; i++)
	{
		DPolygon2 foot = ui.m_pCurBuilding->GetFootprint(i);
		for (r = 0; r < foot.size(); r++)
		{
			DLine2 &dl = foot[r];
			for (j = 0; j < dl.GetSize(); j++)
			{
				p = dl[j];
				p -= origin;
				p.Rotate(angle_diff);
				p += origin;
				dl.SetAt(j, p);
			}
		}
		ui.m_EditBuilding.SetFootprint(i, foot);
	}
}

void vtStructureLayer::UpdateResizeScale(BuilderView *pView, UIContext &ui)
{
	DPoint2 moved_by = ui.m_CurLocation - ui.m_DownLocation;

	DPoint2 origin;
	ui.m_pCurBuilding->GetBaseLevelCenter(origin);

	DPoint2 diff1 = ui.m_DownLocation - origin;
	DPoint2 diff2 = ui.m_CurLocation - origin;
	float fScale = diff2.Length() / diff1.Length();

	uint i, j, r;
	DPoint2 p;
	if (ui.m_bShift)
	{
		// Scale evenly
		uint levs = ui.m_pCurBuilding->NumLevels();
		for (i = 0; i < levs; i++)
		{
			DPolygon2 foot = ui.m_pCurBuilding->GetFootprint(i);
			for (r = 0; r < foot.size(); r++)
			{
				DLine2 &dl = foot[r];
				for (j = 0; j < dl.GetSize(); j++)
				{
					p = dl[j];
					p -= origin;
					p *= fScale;
					p += origin;
					dl.SetAt(j, p);
				}
			}
			ui.m_EditBuilding.SetFootprint(i, foot);
		}
	}
	else
	{
		// drag individual corner points
		DPolygon2 footprint = ui.m_pCurBuilding->GetFootprint(0);

		int vert = ui.m_iCurCorner;
		int ring = footprint.WhichRing(vert);
		DLine2 &foot = footprint[ring];
		p = foot[vert];

		int points = foot.GetSize();
		if (pView->m_bConstrain && points > 3)
		{
			// Maintain angles
			DPoint2 p0 = foot.GetSafePoint(vert - 1);
			DPoint2 p1 = foot.GetSafePoint(vert + 1);
			DPoint2 vec0 = (p - p0).Normalize();
			DPoint2 vec1 = (p - p1).Normalize();
			DPoint2 vec2 = vec0;
			vec2.Rotate(PID2d);
			DPoint2 vec3 = vec1;
			vec3.Rotate(PID2d);

			p += moved_by;
			double a;

			a = (p - p0).Dot(vec2);
			foot.SetSafePoint(vert - 1, p0 + (vec2 * a));
			a = (p - p1).Dot(vec3);
			foot.SetSafePoint(vert + 1, p1 + (vec3 * a));
			foot.SetAt(vert, p);
		}
		else
		{
			p += moved_by;
			foot.SetAt(vert, p);
		}
		// Changing only the lowest level is near useless.  For the great
		//  majority of cases, the user will want the footprints for all
		//  levels to remain in sync.
		for (i = 0; i < ui.m_EditBuilding.NumLevels(); i++)
			ui.m_EditBuilding.SetFootprint(i, footprint);

		// A better guess might be to offset the footprint points of each
		// level by the delta change; e.g. that would preserve roof overhangs.
	}
}


/////////////////////////////////////////////////////////////////////////////

bool vtStructureLayer::EditBuildingProperties()
{
	// Look for the first selected building, and count how many are selected
	int count = 0;
	vtBuilding *bld_selected=NULL;

	for (uint i = 0; i < size(); i++)
	{
		vtStructure *str = at(i);
		vtBuilding *bld = str->GetBuilding();
		if (bld && str->IsSelected())
		{
			count++;
			bld_selected = bld;
		}
	}
	// We don't support editing more than 1 building at a time
	if (count != 1)
		return false;

	// for now, assume they will use the dialog to change something about
	// the building (pessimistic)
	SetModified(true);

	BuildingDlg dlg(NULL, -1, _("Building Properties"));
	dlg.Setup(this, bld_selected);

	dlg.ShowModal();

	return true;
}

void vtStructureLayer::AddFoundations(vtElevLayer *pEL)
{
	// TODO: ask user if they want concrete, the same material as the
	//	building wall, or what.
	// TODO: ask user exactly how much slope (or depth) to tolerate
	//	without building a foundation.

#if 1
	int built = vtStructureArray::AddFoundations(pEL->GetHeightField());
#else
	vtLevel *pLev, *pNewLev;
	int i, j, pts, built = 0;
	float fElev;

	int selected = NumSelected();
	int size = GetSize();
	for (i = 0; i < size; i++)
	{
		vtStructure *str = GetAt(i);
		vtBuilding *bld = str->GetBuilding();
		if (!bld)
			continue;
		if (selected > 0 && !str->IsSelected())
			continue;

		// Get the footprint of the lowest level
		pLev = bld->GetLevel(0);
		const DLine2 &foot = pLev->GetFootprint();
		pts = foot.GetSize();

		float fMin = 1E9, fMax = -1E9;
		for (j = 0; j < pts; j++)
		{
			fElev = pEL->GetElevation(foot[j]);
			if (fElev == INVALID_ELEVATION)
				continue;

			if (fElev < fMin) fMin = fElev;
			if (fElev > fMax) fMax = fElev;
		}
		float fDiff = fMax - fMin;

		// if there's less than 50cm of depth, don't bother building
		// a foundation
		if (fDiff < 0.5f)
			continue;

		// Create and add a foundation level
		pNewLev = new vtLevel;
		pNewLev->m_iStories = 1;
		pNewLev->m_fStoryHeight = fDiff;
		bld->InsertLevel(0, pNewLev);
		bld->SetFootprint(0, foot);
		pNewLev->SetEdgeMaterial(BMAT_NAME_CEMENT);
		pNewLev->SetEdgeColor(RGBi(255, 255, 255));
		built++;
	}
#endif
	DisplayAndLog("Added a foundation level to %d buildings.", built);
}

void vtStructureLayer::InvertSelection()
{
	for (uint i = 0; i < size(); i++)
	{
		vtStructure *str = at(i);
		str->Select(!str->IsSelected());
	}
}

void vtStructureLayer::DeselectAll()
{
	for (uint i = 0; i < size(); i++)
	{
		vtStructure *str = at(i);
		str->Select(false);
	}
}

int vtStructureLayer::DoBoxSelect(const DRECT &rect, SelectionType st)
{
	int affected = 0;
	bool bWas;

	for (uint i = 0; i < size(); i++)
	{
		vtStructure *str = at(i);

		bWas = str->IsSelected();
		if (st == ST_NORMAL)
			str->Select(false);

		if (!str->IsContainedBy(rect))
			continue;

		switch (st)
		{
		case ST_NORMAL:
			str->Select(true);
			affected++;
			break;
		case ST_ADD:
			str->Select(true);
			if (!bWas) affected++;
			break;
		case ST_SUBTRACT:
			str->Select(false);
			if (bWas) affected++;
			break;
		case ST_TOGGLE:
			str->Select(!bWas);
			affected++;
			break;
		}
	}
	return affected;
}

void vtStructureLayer::SetEditedEdge(vtBuilding *bld, int lev, int edge)
{
	vtStructureArray::SetEditedEdge(bld, lev, edge);
}


/////////////////////////////////////////////////////////////////////////////
// Import methods

bool vtStructureLayer::AddElementsFromSHP(const wxString &filename,
										  const vtCRS &crs, DRECT rect)
{
	ImportStructDlg dlg(NULL, -1, _("Import Structures"));

	dlg.SetFileName(filename);
	if (dlg.ShowModal() != wxID_OK)
		return false;

	dlg.m_opt.rect = rect;

	bool success = ReadSHP(filename.mb_str(wxConvUTF8), dlg.m_opt, progress_callback);
	if (!success)
		return false;

	m_crs = crs;
	return true;
}

void vtStructureLayer::AddElementsFromDLG(vtDLGFile *pDlg)
{
	m_crs = pDlg->GetCRS();

/*	TODO: similar code to import from SDTS-DLG
	NEEDED: an actual sample file containing building data in this format.
*/
}

bool vtStructureLayer::AskForSaveFilename()
{
	wxString filter = FSTRING_VTST;
	AddType(filter, FSTRING_VTSTGZ);

	wxFileDialog saveFile(NULL, _("Save Layer"), _T(""), GetLayerFilename(),
		filter, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	saveFile.SetFilterIndex( m_bPreferGZip ? 1 : 0);

	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return false;

	wxString fname = saveFile.GetPath();
	m_bPreferGZip = (saveFile.GetFilterIndex() == 1);

	// work around incorrect extension(s) that wxFileDialog added
	RemoveFileExtensions(fname);
	if (m_bPreferGZip)
		fname += _T(".vtst.gz");
	else
		fname += _T(".vtst");

	SetLayerFilename(fname);
	m_bNative = true;
	return true;
}
