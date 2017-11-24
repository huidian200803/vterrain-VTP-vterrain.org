//
// Name: frame.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef FRAMEH
#define FRAMEH

#include <wx/dnd.h>			// for wxFileDropTarget
#include <wx/splitter.h>	// for wxSplitterWindow
#include "vtlib/core/Content3d.h"
#include "vtdata/FilePath.h"
#include <map>

class MyTreeCtrl;
class vtTransform;
class SceneGraphDlg;
class PropPanel;
class ModelPanel;
class LightDlg;
class vtGroup;
class vtLOD;
class vtGeode;
class vtTransform;
class ItemGroup;

class vtFrame: public wxFrame
{
public:
	vtFrame(wxFrame *frame, const wxString& title, const wxPoint& pos, const wxSize& size,
		long style = wxDEFAULT_FRAME_STYLE);
	~vtFrame();

protected:
	void CreateMenus();
	void CreateToolbar();

	void ReadDataPath();

	// command handlers
	void OnExit(wxCommandEvent& event);
	void OnClose(wxCloseEvent &event);
	void OnIdle(wxIdleEvent& event);

	void OnOpen(wxCommandEvent& event);
	void OnSave(wxCommandEvent& event);
	void OnTestXML(wxCommandEvent& event);
	void OnItemNew(wxCommandEvent& event);
	void OnItemDelete(wxCommandEvent& event);
	void OnItemAddModel(wxCommandEvent& event);
	void OnItemRemoveModel(wxCommandEvent& event);
	void OnItemModelProps(wxCommandEvent& event);
	void OnItemRotModel(wxCommandEvent& event);
	void OnItemSetAmbient(wxCommandEvent& event);
	void OnItemSmoothing(wxCommandEvent& event);
	void OnItemSave(wxCommandEvent& event);
	void OnSceneGraph(wxCommandEvent& event);
	void OnViewOrigin(wxCommandEvent& event);
	void OnUpdateViewOrigin(wxUpdateUIEvent& event);
	void OnViewRulers(wxCommandEvent& event);
	void OnUpdateViewRulers(wxUpdateUIEvent& event);
	void OnViewWireframe(wxCommandEvent& event);
	void OnUpdateViewWireframe(wxUpdateUIEvent& event);
	void OnViewStats(wxCommandEvent& event);
	void OnViewLights(wxCommandEvent& event);
	void OnHelpAbout(wxCommandEvent& event);

	void OnUpdateItemAddModel(wxUpdateUIEvent& event);
	void OnUpdateItemModelExists(wxUpdateUIEvent& event);

	void SaveContentsFile(const wxString &fname);
	void FreeContents();

	void DisplayMessageBox(const wxString &str);
	void AddTool(int id, const wxBitmap &bmp, const wxString &tooltip, bool tog) {
		m_pToolbar->AddTool(id, wxEmptyString, bmp, wxNullBitmap,
			tog ? wxITEM_CHECK : wxITEM_NORMAL, tooltip, tooltip, NULL);
	}

public:
	class vtGLCanvas *m_canvas;
	wxToolBar		*m_pToolbar;

	wxSplitterWindow *m_splitter;
	wxSplitterWindow *m_splitter2;
	wxWindow		*m_blank;
	MyTreeCtrl		*m_pTree;		// left child of splitter

	// Modeless dialogs
	SceneGraphDlg	*m_pSceneGraphDlg;
	PropPanel		*m_pPropDlg;
	ModelPanel		*m_pModelDlg;
	LightDlg		*m_pLightDlg;

public:
	void RenderingPause();
	void RenderingResume();
	void AddModelFromFile(const wxString &fname);
	void ModelNameChanged(vtModel *model);
	int GetModelTriCount(vtModel *model);
	void OnChar(wxKeyEvent& event);
	void UseLight(vtTransform *pLight);
	void LoadContentsFile(const wxString &fname);
	void UpdateStatusText();

public:
	void		UpdateCurrentModelLOD();
	void		UpdateScale(vtModel *model);
	void		UpdateTransform(vtModel *model);
	void		RefreshTreeItems();
	void		SetItemName(vtItem *item, const vtString &name);

	// Models
	void		SetCurrentItemAndModel(vtItem *item, vtModel *model);
	void		SetCurrentItem(vtItem *item);
	void		SetCurrentModel(vtModel *mod);
	vtModel		*AddModel(const wxString &fname);
	vtTransform	*AttemptLoad(vtModel *model);
	ItemGroup	*GetItemGroup(vtItem *item);
	void		UpdateItemGroup(vtItem *item);
	void		ShowItemGroupLOD(bool bTrue);
	void		AddNewItem();
	void		DisplayCurrentModel();
	void		ZoomToCurrentModel();
	void		ZoomToModel(vtModel *model);
	void		UpdateWidgets();

	void		DisplayCurrentItem();
	void		ZoomToCurrentItem();

	vtContentManager3d	m_Man;
	vtItem				*m_pCurrentItem;
	vtModel				*m_pCurrentModel;
	vtFontPtr			 m_pFont;

	std::map<vtItem *, ItemGroup *> m_itemmap;
	std::map<vtModel *, vtTransformPtr> m_nodemap;

	bool m_bShowOrigin;
	bool m_bShowRulers;
	bool m_bWireframe;

	bool m_bCloseOnIdle;

	DECLARE_EVENT_TABLE()
};

class DnDFile : public wxFileDropTarget
{
public:
	virtual bool OnDropFiles(wxCoord x, wxCoord y,
		const wxArrayString &filenames);
};

// Helper
extern vtFrame *GetMainFrame();

#endif	// FRAMEH

