//
// Name: SceneGraphDlg.cpp
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/treectrl.h"
#include "wx/image.h"

#include "vtlib/vtlib.h"
#include "vtlib/core/Engine.h"
#include "SceneGraphDlg.h"

// We use xpm on all platforms
#  include "icons/camera.xpm"
#  include "icons/engine.xpm"
#  include "icons/geode.xpm"
#  include "icons/group.xpm"
#  include "icons/light.xpm"
#  include "icons/lod.xpm"
#  include "icons/mesh.xpm"
#  include "icons/top.xpm"
#  include "icons/unknown.xpm"
#  include "icons/xform.xpm"
#  include "icons/fog.xpm"
#  include "icons/shadow.xpm"
#  include "icons/hud.xpm"
#  include "icons/dyngeom.xpm"

#define LocalIcon(X) wxIcon( (const char**) X##_xpm )


/////////////////////////////

class MyTreeItemData : public wxTreeItemData
{
public:
	MyTreeItemData(osg::Node *pNode, vtEngine *pEngine)
	{
		m_pNode = pNode;
		m_pEngine = pEngine;
	}
	osg::Node *m_pNode;
	vtEngine *m_pEngine;
};


//----------------------------------------------------------------------------
// SceneGraphDlg
//----------------------------------------------------------------------------

// WDR: event table for SceneGraphDlg

BEGIN_EVENT_TABLE(SceneGraphDlg, SceneGraphDlgBase)
	EVT_INIT_DIALOG (SceneGraphDlg::OnInitDialog)
	EVT_TREE_SEL_CHANGED( ID_SCENETREE, SceneGraphDlg::OnTreeSelChanged )
	EVT_CHECKBOX( ID_ENABLED, SceneGraphDlg::OnEnabled )
	EVT_BUTTON( ID_ZOOMTO, SceneGraphDlg::OnZoomTo )
	EVT_BUTTON( ID_REFRESH, SceneGraphDlg::OnRefresh )
	EVT_BUTTON( ID_LOG, SceneGraphDlg::OnLog )
	EVT_CHAR( SceneGraphDlg::OnChar )
END_EVENT_TABLE()

SceneGraphDlg::SceneGraphDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	SceneGraphDlgBase( parent, id, title, position, size, style )
{
	// Work around wxFormDesigner's lack of support for limiting to smallest size
	GetSizer()->SetSizeHints(this);

	m_pZoomTo = GetZoomto();
	m_pEnabled = GetEnabled();
	m_pTree = GetScenetree();

	m_pZoomTo->Enable(false);

	m_imageListNormal = NULL;
	CreateImageList(16);
}

SceneGraphDlg::~SceneGraphDlg()
{
	delete m_imageListNormal;
}


///////////

void SceneGraphDlg::CreateImageList(int size)
{
	delete m_imageListNormal;

	if ( size == -1 )
	{
		m_imageListNormal = NULL;
		return;
	}
	// Make an image list containing small icons
	m_imageListNormal = new wxImageList(size, size, TRUE);

	wxIcon icons[14];
	icons[0] = LocalIcon(camera);	// camera
	icons[1] = LocalIcon(engine);	// engine
	icons[2] = LocalIcon(geode);	// geom
	icons[3] = LocalIcon(group);	// group
	icons[4] = LocalIcon(light);	// light
	icons[5] = LocalIcon(lod);		// lod
	icons[6] = LocalIcon(mesh);		// mesh
	icons[7] = LocalIcon(top);		// top
	icons[8] = LocalIcon(unknown);	// unknown
	icons[9] = LocalIcon(xform);	// xform
	icons[10] = LocalIcon(fog);		// fog
	icons[11] = LocalIcon(shadow);	// shadow
	icons[12] = LocalIcon(hud);		// hud
	icons[13] = LocalIcon(dyngeom);	// dyngeom

	int sizeOrig = icons[0].GetWidth();
	for ( size_t i = 0; i < WXSIZEOF(icons); i++ )
	{
		if ( size == sizeOrig )
			m_imageListNormal->Add(icons[i]);
		else
			m_imageListNormal->Add(wxBitmap(wxBitmap(icons[i]).ConvertToImage().Rescale(size, size)));
	}
	m_pTree->SetImageList(m_imageListNormal);
}


void SceneGraphDlg::RefreshTreeContents()
{
	vtScene* scene = vtGetScene();
	if (!scene)
		return;

	// start with a blank slate
	m_pTree->DeleteAllItems();

	// Fill in the tree with nodes
	m_bFirst = true;
	vtGroup *pRoot = scene->GetRoot();
	if (pRoot) AddNodeItemsRecursively(wxTreeItemId(), pRoot, 0);

	wxTreeItemId hRoot = m_pTree->GetRootItem();
	wxTreeItemId hEngRoot = m_pTree->AppendItem(hRoot, _("Engines"), 7, 7);

	vtEngine *pEngine = scene->GetRootEngine();
	if (pEngine) AddEnginesRecursively(hEngRoot, pEngine, 0);
	m_pTree->Expand(hEngRoot);

	m_pSelectedEngine = NULL;
	m_pSelectedNode = NULL;
}


void SceneGraphDlg::AddNodeItemsRecursively(wxTreeItemId hParentItem,
											osg::Node *pNode, int depth)
{
	wxString str;
	int nImage;
	wxTreeItemId hNewItem;

	if (!pNode) return;

	if (dynamic_cast<vtLightSource*>(pNode))
	{
		str = _("vtLightSource");
		nImage = 4;
	}
	else if (dynamic_cast<vtDynGeom*>(pNode))
	{
		str = _("vtDynGeom");
		nImage = 13;
	}
	else if (dynamic_cast<vtGeode*>(pNode))
	{
		str = _("vtGeode");
		nImage = 2;
	}
	else if (dynamic_cast<vtLOD*>(pNode))
	{
		str = _T("vtLOD");
		nImage = 5;
	}
	else if (dynamic_cast<vtTransform*>(pNode))
	{
		str = _T("vtTransform");
		nImage = 9;
	}
	else if (dynamic_cast<vtFog*>(pNode))
	{
		str = _("vtFog");
		nImage = 10;
	}
	else if (dynamic_cast<vtShadow*>(pNode))
	{
		str = _("vtShadow");
		nImage = 11;
	}
	else if (dynamic_cast<vtHUD*>(pNode))
	{
		str = _("vtHUD");
		nImage = 12;
	}
	else if (dynamic_cast<vtGroup*>(pNode))
	{
		// must be just a group for grouping's sake
		str = _("vtGroup");
		nImage = 3;
	}
	// Or, raw OSG nodes
	else if (dynamic_cast<osg::MatrixTransform*>(pNode))
	{
		str = _("MatrixTransform");
		nImage = 9;
	}
	else if (dynamic_cast<osg::LOD*>(pNode))
	{
		str = _("LOD");
		nImage = 5;
	}
	else if (dynamic_cast<osg::Group*>(pNode))
	{
		str = _("Group");
		nImage = 3;
	}
	else if (dynamic_cast<osg::Geode*>(pNode))
	{
		str = _("Geode");
		nImage = 2;
	}
	else
	{
		// must be something else
		str = _("Other");
		nImage = 8;
	}
	if (pNode->getName() != "")
	{
		str += _T(" \"");
		str += wxString::FromUTF8(pNode->getName().c_str());
		str += _T("\"");
	}

	if (m_bFirst)
	{
		hNewItem = m_pTree->AddRoot(str);
		m_bFirst = false;
	}
	else
		hNewItem = m_pTree->AppendItem(hParentItem, str, nImage, nImage);

	vtGeode *pGeode = dynamic_cast<vtGeode*>(pNode);
	osg::Geode *geode = dynamic_cast<osg::Geode*>(pNode);
	if (pGeode)
	{
		int num_mesh = pGeode->NumMeshes();
		wxTreeItemId	hGeomItem;

		for (int i = 0; i < num_mesh; i++)
		{
			osg::Drawable *draw = pGeode->getDrawable(i);
			vtMesh *pMesh = pGeode->GetMesh(i);
			vtTextMesh *pTextMesh = pGeode->GetTextMesh(i);
			osg::Geometry *geom = dynamic_cast<osg::Geometry*>(draw);

			if (pMesh)
			{
				int iNumPrim = pMesh->NumPrims();
				int iNumVert = pMesh->NumVertices();

				vtMesh::PrimType ptype = pMesh->getPrimType();
				const char *mtype="";
				switch (ptype)
				{
				case osg::PrimitiveSet::POINTS: mtype = "Points"; break;
				case osg::PrimitiveSet::LINES: mtype = "Lines"; break;
				case osg::PrimitiveSet::LINE_STRIP: mtype = "LineStrip"; break;
				case osg::PrimitiveSet::TRIANGLES: mtype = "Triangles"; break;
				case osg::PrimitiveSet::TRIANGLE_STRIP: mtype = "TriStrip"; break;
				case osg::PrimitiveSet::TRIANGLE_FAN: mtype = "TriFan"; break;
				case osg::PrimitiveSet::QUADS: mtype = "Quads"; break;
				case osg::PrimitiveSet::QUAD_STRIP: mtype = "QuadStrip"; break;
				case osg::PrimitiveSet::POLYGON: mtype = "Polygon"; break;
				default:	// Keep picky compilers quiet.
					break;
				}
				str.Printf(_("%d: Mesh, %hs, %d verts, %d prims"), i, mtype, iNumVert, iNumPrim);
				hGeomItem = m_pTree->AppendItem(hNewItem, str, 6, 6);
			}
			else if (pTextMesh)
				hGeomItem = m_pTree->AppendItem(hNewItem, _("Text Mesh"), 6, 6);
			else if (geom)
			{
				int iNumVert = geom->getVertexArray()->getNumElements();
				str.Printf(_("%d: Drawable, %d verts"), i, iNumVert);
				hGeomItem = m_pTree->AppendItem(hNewItem, str, 6, 6);
			}
			else if (draw)
			{
				str = _("Drawable");
				hGeomItem = m_pTree->AppendItem(hNewItem, str, 6, 6);
			}
		}
	}
	else if (geode)
	{
		int num_draw = geode->getNumDrawables();
		wxTreeItemId	hGeomItem;

		for (int i = 0; i < num_draw; i++)
		{
			osg::Geometry *geom = dynamic_cast<osg::Geometry*>(geode->getDrawable(i));
			if (geom)
			{
				int iNumVert = geom->getVertexArray()->getNumElements();
				str.Printf(_("%d: %d verts"), i, iNumVert);
				hGeomItem = m_pTree->AppendItem(hNewItem, str, 6, 6);
			}
			else
			{
				str.Printf(_("%d: drawable"), i);
				hGeomItem = m_pTree->AppendItem(hNewItem, str, 6, 6);
			}
		}
	}

	m_pTree->SetItemData(hNewItem, new MyTreeItemData(pNode, NULL));

	wxTreeItemId hSubItem;
	osg::Group *group = dynamic_cast<osg::Group*>(pNode);
	if (group)
	{
		int num_children = group->getNumChildren();
		if (num_children > 200)
		{
			str.Printf(_("(%d children)"), num_children);
			hSubItem = m_pTree->AppendItem(hNewItem, str, 8, 8);
		}
		else
		{
			for (int i = 0; i < num_children; i++)
			{
				osg::Node *pChild = group->getChild(i);
				if (pChild)
					AddNodeItemsRecursively(hNewItem, pChild, depth+1);
				else
					hSubItem = m_pTree->AppendItem(hNewItem, _("(internal node)"), 8, 8);
			}
		}
	}
	// expand a bit so that the tree is initially partially exposed
	if (depth < 2)
		m_pTree->Expand(hNewItem);
}

void SceneGraphDlg::AddEnginesRecursively(wxTreeItemId hParentItem,
										vtEngine *pEng, int depth)
{
	wxTreeItemId hNewItem;

	if (!pEng) return;

	wxString str(pEng->getName(), wxConvUTF8);
	if (str == _T(""))
		str = _("unnamed");

	int targets = pEng->NumTargets();
	osg::Referenced *target = pEng->GetTarget();
	if (target)
	{
		str += _T(" -> ");
		osg::Node *node = dynamic_cast<osg::Node*>(target);
		if (node)
		{
			str += _T("\"");
			wxString str2(node->getName().c_str(), wxConvUTF8);
			str += str2;
			str += _T("\"");
		}
		else
			str += _("(non-node)");
	}
	if (targets > 1)
	{
		wxString plus;
		plus.Printf(_(" (%d targets total)"), targets);
		str += plus;
	}

	hNewItem = m_pTree->AppendItem(hParentItem, str, 1, 1);
	m_pTree->SetItemData(hNewItem, new MyTreeItemData(NULL, pEng));

	for (uint i = 0; i < pEng->NumChildren(); i++)
	{
		vtEngine *pChild = pEng->GetChild(i);
		AddEnginesRecursively(hNewItem, pChild, depth+1);
	}

	// always expand engines
	m_pTree->Expand(hNewItem);
}

// WDR: handler implementations for SceneGraphDlg

void SceneGraphDlg::OnRefresh( wxCommandEvent &event )
{
	RefreshTreeContents();
}

void SceneGraphDlg::OnZoomTo( wxCommandEvent &event )
{
	if (m_pSelectedNode)
	{
		FSphere sph;
		GetBoundSphere(m_pSelectedNode, sph, true);	// global bounds

		// a bit back to make sure whole volume of bounding sphere is in view
		vtCamera *pCam = vtGetScene()->GetCamera();
		float smallest = std::min(pCam->GetFOV(), pCam->GetVertFOV());
		float alpha = smallest / 2.0f;
		float distance = sph.radius / tanf(alpha);
		sph.radius = distance;

		pCam->ZoomToSphere(sph);
	}
}

void SceneGraphDlg::OnEnabled( wxCommandEvent &event )
{
	if (m_pSelectedEngine)
		m_pSelectedEngine->SetEnabled(m_pEnabled->GetValue());
	if (m_pSelectedNode)
		SetEnabled(m_pSelectedNode, m_pEnabled->GetValue());
}

void SceneGraphDlg::OnTreeSelChanged( wxTreeEvent &event )
{
	wxTreeItemId item = event.GetItem();
	MyTreeItemData *data = (MyTreeItemData *)m_pTree->GetItemData(item);

	m_pEnabled->Enable(data != NULL);

	m_pSelectedEngine = NULL;
	m_pSelectedNode = NULL;

	if (data && data->m_pEngine)
	{
		m_pSelectedEngine = data->m_pEngine;
		m_pEnabled->SetValue(m_pSelectedEngine->GetEnabled());
	}
	if (data && data->m_pNode)
	{
		m_pSelectedNode = data->m_pNode;
		m_pEnabled->SetValue(NodeIsEnabled(m_pSelectedNode));
		m_pZoomTo->Enable(true);
	}
	else
		m_pZoomTo->Enable(false);
}

void SceneGraphDlg::OnInitDialog(wxInitDialogEvent& event)
{
	RefreshTreeContents();

	wxWindow::OnInitDialog(event);
}

void SceneGraphDlg::OnChar(wxKeyEvent& event)
{
	long key = event.GetKeyCode();

	if (key == 'd' && m_pSelectedNode)
		vtLogGraph(m_pSelectedNode);

	// Allow wxWidgets to pass the event along to other code
	event.Skip();
}

void SceneGraphDlg::OnLog( wxCommandEvent &event )
{
	if (!m_pSelectedNode)
		return;

	wxArrayString choices;
	choices.Add(_("The standard log file (debug.txt)"));
	choices.Add(_("A dot file (scene.dot)"));
	int index = wxGetSingleChoiceIndex(_("Log the scene graph to:"), _T(""), choices, this);
	if (index == 0)
	{
		vtLogGraph(m_pSelectedNode);
	}
	else if (index == 1)
	{
		osg::Group *group = dynamic_cast<osg::Group*>(m_pSelectedNode);
		if (group)
			WriteDotFile(group, "scene.dot");
	}
}

