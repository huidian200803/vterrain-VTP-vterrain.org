//
// Name: FeatureTableDlg3d.h
//
// Copyright (c) 2002-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __FeatureTableDlg3d_H__
#define __FeatureTableDlg3d_H__

#include "vtui/FeatureTableDlg.h"

class vtAbstractLayer;

class FeatureTableDlg3d: public FeatureTableDlg
{
public:
	// constructors and destructors
	FeatureTableDlg3d( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void SetLayer(vtAbstractLayer *pLayer) { m_pLayer = pLayer; }
	vtAbstractLayer *GetLayer() { return m_pLayer; }

	virtual void OnModified();
	virtual void RefreshViz();
	virtual void OnFeatureDelete(vtFeature *f);
	virtual void OnFieldEdited(uint iIndex);
	virtual void OnEditEnd();

protected:
	vtAbstractLayer *m_pLayer;
};

#endif	// __FeatureTableDlg3d_H__

