//
// RoadLayer.cpp
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "RoadLayer.h"
#include "NodeDlg.h"
#include "RoadDlg.h"
#include "Builder.h"
#include "BuilderView.h"
#include "vtui/Helper.h"

bool vtRoadLayer::m_bDrawNodes = false;
bool vtRoadLayer::m_bShowDirection = false;
bool vtRoadLayer::m_bDrawWidth = false;

//////////////////////////////////////////////////////////////////////

vtRoadLayer::vtRoadLayer() : vtLayer(LT_ROAD)
{
	wxString name = _("Untitled");
	name += _T(".rmf");
	SetLayerFilename(name);
}

vtRoadLayer::~vtRoadLayer()
{
}

///////////////////////////////////////////////////////////////////////
// Standard layer overrides
//

bool vtRoadLayer::GetExtent(DRECT &rect)
{
	if (NumNodes() == 0 && NumLinks() == 0)
		return false;

	rect = GetMapExtent();
	return true;
}

void vtRoadLayer::DrawLayer(vtScaledView *pView, UIContext &ui)
{
	Draw(pView, m_bDrawNodes);
}

bool vtRoadLayer::OnSave(bool progress_callback(int))
{
	return WriteRMF(GetLayerFilename().mb_str(wxConvUTF8));
}

bool vtRoadLayer::OnLoad()
{
	bool success = ReadRMF(GetLayerFilename().mb_str(wxConvUTF8));
	if (!success)
		return false;

	// Set visual properties
	for (NodeEdit *pN = GetFirstNode(); pN; pN = pN->GetNext())
	{
		pN->DetermineVisualFromLinks();
	}

	// Pre-process some link attributes
	for (LinkEdit *pL = GetFirstLink(); pL; pL = pL->GetNext())
	{
		pL->m_fLength = pL->Length();

		//set the bounding box for the link
		pL->Dirtied();

		// clean up link direction info
		if ((pL->m_iFlags & (RF_FORWARD|RF_REVERSE)) == 0)
			pL->m_iFlags |= (RF_FORWARD|RF_REVERSE);
	}
	return true;
}

void vtRoadLayer::GetCRS(vtCRS &crs)
{
	crs = vtRoadMap::GetAtCRS();
}

bool vtRoadLayer::AppendDataFrom(vtLayer *pL)
{
	// safety check
	if (pL->GetType() != LT_ROAD)
		return false;

	vtRoadLayer *pFrom = (vtRoadLayer *)pL;

	// add nodes to our list
	TNode *n = pFrom->GetFirstNode();
	while (n)
	{
		TNode *next = n->GetNext();
		n->SetNext(m_pFirstNode);
		m_pFirstNode = n;
		n = next;
	}
	// add links to our list
	TLink *r = pFrom->GetFirstLink();
	while (r)
	{
		TLink *next = r->GetNext();
		r->SetNext(m_pFirstLink);
		m_pFirstLink = r;
		r = next;
	}
	pFrom->m_pFirstLink = NULL;
	pFrom->m_pFirstNode = NULL;

	ComputeExtents();

	return true;
}

///////////////////////////////////////////////////////////////////////

int vtRoadLayer::GetSelectedNodes()
{
	int count = 0;
	for (NodeEdit *n = GetFirstNode(); n; n = n->GetNext())
		if (n->IsSelected()) count++;
	return count;
}

int vtRoadLayer::GetSelectedLinks()
{
	int count = 0;
	for (LinkEdit *n = GetFirstLink(); n; n = n->GetNext())
		if (n->IsSelected()) count++;
	return count;
}

void vtRoadLayer::ToggleLinkDirection(LinkEdit *pLink)
{
	switch (pLink->m_iFlags & (RF_FORWARD|RF_REVERSE))
	{
		case RF_FORWARD:
			pLink->m_iFlags &= ~RF_FORWARD;
			pLink->m_iFlags |= RF_REVERSE;
			break;
		case RF_REVERSE:
			pLink->m_iFlags |= RF_FORWARD;
			break;
		case (RF_FORWARD|RF_REVERSE):
			pLink->m_iFlags &= ~RF_REVERSE;
			break;
	}
	SetModified(true);
}

void vtRoadLayer::MoveSelectedNodes(const DPoint2 &offset)
{
	for (NodeEdit *n = GetFirstNode(); n; n=n->GetNext())
	{
		if (n->IsSelected())
			n->Translate(offset);
	}
}

bool vtRoadLayer::TransformCoords(vtCRS &crs_new)
{
	// Create conversion object
	vtCRS Source;
	GetCRS(Source);

	ScopedOCTransform trans(CreateCoordTransform(&Source, &crs_new));
	if (!trans)
		return false;		// inconvertible coordinate systems

	LinkEdit *l;
	NodeEdit *n;
	for (l = GetFirstLink(); l; l=l->GetNext())
	{
		for (uint i = 0; i < l->GetSize(); i++)
			trans->Transform(1, &(l->GetAt(i).x), &(l->GetAt(i).y));
		l->Dirtied();
	}
	for (n = GetFirstNode(); n; n=n->GetNext())
		trans->Transform(1, &(n->Pos().x), &(n->Pos().y));

	// set the vtRoadMap CRS
	m_crs = crs_new;
	SetModified(true);

	m_bValidExtents = false;
	return true;
}

void vtRoadLayer::SetCRS(const vtCRS &crs)
{
	if (m_crs == crs)
		return;

	m_crs = crs;

	// Extents are still valid, but we should recompute things like displayed
	// link widths, which may have different values in another projection.
	for (LinkEdit *link = GetFirstLink(); link; link = link->GetNext())
		link->m_bSidesComputed = false;

	SetModified(true);
}

void vtRoadLayer::Offset(const DPoint2 &p)
{
	bool bSelLinks = (NumSelectedLinks() > 0);
	bool bSelNodes = (NumSelectedNodes() > 0);
	bool bSelected = bSelLinks || bSelNodes;
	for (LinkEdit *link = GetFirstLink(); link; link=link->GetNext())
	{
		for (uint i = 0; i < link->GetSize(); i++)
		{
			if (bSelected && !link->IsSelected())
				continue;
			link->GetAt(i) += p;
		}
		link->m_bSidesComputed = false;
		if (bSelLinks && !bSelNodes)
		{
			link->GetNode(0)->Pos() += p;
			link->GetNode(1)->Pos() += p;
		}
	}
	for (NodeEdit *node = GetFirstNode(); node; node=node->GetNext())
	{
		if (bSelected && !node->IsSelected())
			continue;
		node->Pos() += p;
		if (!bSelLinks && bSelNodes)
		{
			for (int i = 0; i < node->NumLinks(); i++)
			{
				TLink *l1 = node->GetLink(i);
				if (l1->GetNode(0) == node)
					l1->SetAt(0, node->Pos());
				else
					l1->SetAt(l1->GetSize()-1, node->Pos());
			}
		}
	}

	// recompute link extents
	for (LinkEdit *r2 = GetFirstLink(); r2; r2=r2->GetNext())
		r2->Dirtied();

	m_bValidExtents = false;
}

void vtRoadLayer::GetPropertyText(wxString &strIn)
{
	strIn += _("Network of links.\n");

	wxString str;
	str.Printf(_("Nodes: %d, selected: %d\n"), NumNodes(), NumSelectedNodes());
	strIn += str;
	str.Printf(_("Links: %d, selected: %d\n"), NumLinks(), NumSelectedLinks());
	strIn += str;
}

void vtRoadLayer::OnLeftDown(BuilderView *pView, UIContext &ui)
{
	if (ui.mode == LB_LinkEdit && ui.m_pEditingRoad)
	{
		int closest_i = -1;
		double dist;
		ui.m_pEditingRoad->NearestPoint(ui.m_DownLocation, closest_i, dist);
		if (dist < pView->PixelsToWorld(8).x)
		{
			// begin dragging point
			ui.m_iEditingPoint = closest_i;
			return;
		}
		else
			ui.m_iEditingPoint = -1;
	}
	if (ui.mode == LB_Dir)
	{
		LinkEdit *pLink = FindLink(ui.m_DownLocation, pView->PixelsToWorld(5).x);
		if (pLink)
		{
			ToggleLinkDirection(pLink);
			pView->Refresh();
		}
	}
	if (ui.mode == LB_LinkEdit)
	{
		// see if there is a link or node at m_DownPoint
		const float epsilon = pView->PixelsToWorld(5).x;

		LinkEdit *pLink = FindLink(ui.m_DownLocation, epsilon);
		if (pLink != ui.m_pEditingRoad)
		{
			if (ui.m_pEditingRoad)	// Un-highlight previously hightlighted
			{
				pView->Refresh();
				ui.m_pEditingRoad->m_bDrawPoints = false;
			}
			ui.m_pEditingRoad = pLink;
			if (ui.m_pEditingRoad)	// Highlight the currently hightlighted
			{
				pView->Refresh();
				ui.m_pEditingRoad->m_bDrawPoints = true;
			}
		}
	}
	if (ui.mode == LB_LinkExtend)
	{
		pView->OnLButtonClickElement(this);
	}
}

void vtRoadLayer::OnLeftUp(BuilderView *pView, UIContext &ui)
{
	if (ui.mode != LB_LinkEdit)
		return;

	if (ui.m_pEditingRoad != NULL && ui.m_iEditingPoint >= 0)
	{
		LinkEdit *le = ui.m_pEditingRoad;
		DPoint2 p = le->GetAt(ui.m_iEditingPoint);
		p += (ui.m_CurLocation - ui.m_DownLocation);
		le->SetAt(ui.m_iEditingPoint, p);
		le->Dirtied();

		pView->Refresh();

		// see if we changed the first or last point, affects some node
		int num_points = le->GetSize();
		NodeEdit *node = NULL;
		if (ui.m_iEditingPoint == 0)
			node = le->GetNode(0);
		if (ui.m_iEditingPoint == num_points-1)
			node = le->GetNode(1);
		if (node)
		{
			node->SetPos(p);
			for (int i = 0; i < node->NumLinks(); i++)
			{
				LinkEdit *link = node->GetLink(i);
				if (link->GetNode(0) == node)
					link->SetAt(0, p);
				if (link->GetNode(1) == node)
					link->SetAt(link->GetSize()-1, p);
				link->Dirtied();
			}
			pView->Refresh();
		}
		// We have changed the layer
		SetModified(true);
	}
	ui.m_iEditingPoint = -1;
}

void vtRoadLayer::OnRightUp(BuilderView *pView, UIContext &ui)
{
	//if we are not clicked close to a single item, edit all selected items.
	bool status;
	if (ui.mode == LB_Node)
		status = EditNodesProperties(pView);
	else
		status = EditLinksProperties(pView);
	if (status)
	{
		SetModified(true);
		pView->Refresh();
		g_bld->RefreshTreeStatus();
	}
}

void vtRoadLayer::OnLeftDoubleClick(BuilderView *pView, UIContext &ui)
{
	DRECT world_bound, bound2;

	// epsilon is how close to the link/node can we be off by?
	double epsilon = pView->PixelsToWorld(5).x;
	bool bRefresh = false;

	if (ui.mode == LB_Node)
	{
		SelectNode(ui.m_DownLocation, epsilon, bound2);
		EditNodeProperties(pView, ui.m_DownLocation, epsilon, world_bound);
		bRefresh = true;
	}
	else if (ui.mode == LB_Link)
	{
		SelectLink(ui.m_DownLocation, epsilon, bound2);
		EditLinkProperties(ui.m_DownLocation, epsilon, world_bound);
		bRefresh = true;
	}
	if (bRefresh)
	{
		pView->Refresh();
	}
}

void vtRoadLayer::OnMouseMove(BuilderView *pView, UIContext &ui)
{
	if (ui.mode == LB_LinkEdit)
	{
		if (ui.m_pEditingRoad)
		{
			// see if there is a node nearby
			int previous = ui.m_pEditingRoad->m_iHighlightPoint;
			int closest_i = -1;
			double dist;
			ui.m_pEditingRoad->NearestPoint(ui.m_CurLocation, closest_i, dist);

			if (dist < pView->PixelsToWorld(8).x)
				ui.m_pEditingRoad->m_iHighlightPoint = closest_i;
			else
				ui.m_pEditingRoad->m_iHighlightPoint = -1;

			if (previous != ui.m_pEditingRoad->m_iHighlightPoint)
				pView->Refresh();
		}
	}
}

bool vtRoadLayer::EditNodeProperties(BuilderView *pView, const DPoint2 &point, float epsilon,
									 DRECT &bound)
{
	NodeEdit *node = (NodeEdit *) FindNodeAtPoint(point, epsilon);
	if (node)
	{
		DPoint2 p = node->Pos();
		bound.SetRect(p.x-epsilon, p.y+epsilon, p.x+epsilon, p.y-epsilon);
		return node->EditProperties(pView, this);
	}
	return false;
}

bool vtRoadLayer::EditLinkProperties(const DPoint2 &point, float error,
									 DRECT &bound)
{
	LinkEdit* bestRSoFar = NULL;
	double dist = error;
	double b;
	bool RFound = false;

	DRECT target(point.x-error, point.y+error, point.x+error, point.y-error);

	for (LinkEdit* curLink = GetFirstLink(); curLink; curLink = curLink->GetNext())
	{
		if (curLink->OverlapsExtent(target))
		{
			b = curLink->DistanceToPoint(point);
			if (b < dist)
			{
				bestRSoFar = curLink;
				dist = b;
				RFound = true;
			}
		}
	}

	if (RFound)
	{
		bound = bestRSoFar->m_extent;
		return bestRSoFar->EditProperties(this);
	}

	return false;
}

bool vtRoadLayer::EditNodesProperties(BuilderView *pView)
{
	int count = 0;
	NodeEdit *node=NULL;

	for (NodeEdit* n = GetFirstNode(); n; n = n->GetNext())
	{
		if (!n->IsSelected())
			continue;
		count++;
		node = n;
	}
	if (count == 0)
		return false;

	NodeDlg dlg(NULL, -1, _("Node Properties"));
	if (count == 1)
		dlg.SetNode(node, this);
	else
		dlg.SetNode(NULL, this);

	return (dlg.ShowModal() == wxID_OK);
}

bool vtRoadLayer::EditLinksProperties(BuilderView *pView)
{
	int count = 0;
	LinkEdit *link=NULL;

	//only bring up dialog is there is a selected link.
	for (LinkEdit* r = GetFirstLink(); r; r = r->GetNext())
	{
		if (!r->IsSelected())
			continue;
		count++;
		link = r;
	}
	if (count == 0)
		return false;

	RoadDlg dlg(NULL, -1, _("Link Properties"));
	if (count == 1)
		dlg.SetRoad(link, this);	//only one link found
	else
		dlg.SetRoad(NULL, this);
	return (dlg.ShowModal() == wxID_OK);
}

/** Selected everything in the box, return true if anything was selected. 
 */
bool vtRoadLayer::SelectArea(const DRECT &box, bool nodemode, bool crossSelect)
{
	bool ret = false;
	int selected;
	if (nodemode)
	{
		selected = SelectNodes(box, true);
		wxString str = wxString::Format(_("Selected %d nodes"), selected);
		if (selected) SetMessageText(str);
		ret = (selected != 0);
	}
	else
	{
		if (crossSelect)
			selected = CrossSelectLinks(box, true);
		else
			selected = SelectLinks(box, true);

		wxString str = wxString::Format(_("Selected %d links"), selected);
		if (selected) SetMessageText(str);
		ret = (selected != 0);
	}
	return ret;
}

void vtRoadLayer::DoClean(double epsilon)
{
	// Check CRS.
	vtCRS crs;
	GetCRS(crs);
	bool bDegrees = (crs.IsGeographic() != 0);

	int count;
	OpenProgressDialog(_("Cleaning RoadMap"), _T(""));

	UpdateProgressDialog(10, _("Removing unused nodes"));
	count = RemoveUnusedNodes();
	if (count)
	{
		DisplayAndLog("Removed %i nodes", count);
		SetModified(true);
	}

	UpdateProgressDialog(20, _("Merging redundant nodes"));
	// potentially takes a long time...
	count = MergeRedundantNodes(bDegrees, progress_callback);
	if (count)
	{
		DisplayAndLog("Merged %d redundant roads", count);
		SetModified(true);
	}

	UpdateProgressDialog(30, _("Cleaning link points"));
	count = CleanLinkPoints(epsilon);
	if (count)
	{
		DisplayAndLog("Cleaned %d link points", count);
		SetModified(true);
	}

	UpdateProgressDialog(40, _T("Removing degenerate links"));
	count = RemoveDegenerateLinks();
	if (count)
	{
		DisplayAndLog("Removed %d degenerate links", count);
		SetModified(true);
	}

#if 0
	// The following cleanup operations are disabled until they are proven safe!

	UpdateProgressDialog(40, _T("Removing unnecessary nodes"));
	count = RemoveUnnecessaryNodes();
	if (count)
	{
		DisplayAndLog("Removed %d unnecessary nodes", count);
	}

	UpdateProgressDialog(60, _T("Removing dangling links"));
	count = DeleteDanglingRoads();
	if (count)
	{
		DisplayAndLog("Removed %i dangling links", count);
	}

	UpdateProgressDialog(70, _T("Fixing overlapped roads"));
	count = FixOverlappedRoads(bDegrees);
	if (count)
	{
		DisplayAndLog("Fixed %i overlapped roads", count);
	}

	UpdateProgressDialog(80, _T("Fixing extraneous parallels"));
	count = FixExtraneousParallels();
	if (count)
	{
		DisplayAndLog("Fixed %i extraneous parallels", count);
	}

	UpdateProgressDialog(90, _T("Splitting looping roads"));
	count = SplitLoopingRoads();
	if (count)
	{
		DisplayAndLog("Split %d looping roads", count);
	}
#endif

	CloseProgressDialog();
	ComputeExtents();
}

