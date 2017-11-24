//
// Name: FeatInfoDlg.h
//
// Copyright (c) 2002-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __FeatInfoDlg_H__
#define __FeatInfoDlg_H__

#include "vtui/FeatureTableDlg.h"
#include "RawLayer.h"

class BuilderView;

class FeatInfoDlg: public FeatureTableDlg
{
public:
	// constructors and destructors
	FeatInfoDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void SetLayer(vtLayer *pLayer) { m_pLayer = pLayer; }
	void SetView(BuilderView *pView) { m_pView = pView; }
	vtLayer *GetLayer() { return m_pLayer; }

	virtual void OnModified();
	virtual void RefreshViz();

protected:
	BuilderView *m_pView;
	vtLayer *m_pLayer;
};

#endif	// __FeatInfoDlg_H__

