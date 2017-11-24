//
// Name:		FeatureTableDlg3d.cpp
//
// Copyright (c) 2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "FeatureTableDlg3d.h"
#include "vtlib/vtlib.h"
#include "vtlib/core/AbstractLayer.h"


FeatureTableDlg3d::FeatureTableDlg3d( wxWindow *parent, wxWindowID id,
	const wxString &title, const wxPoint &position, const wxSize& size, long style ) :
	FeatureTableDlg(parent, id, title, position, size, style)
{
	m_pLayer = NULL;
}

void FeatureTableDlg3d::OnModified()
{
	//m_pLayer->SetModified(true);
}
void FeatureTableDlg3d::RefreshViz()
{
	m_pLayer->RefreshFeatureVisuals();
}

void FeatureTableDlg3d::OnFeatureDelete(vtFeature *f)
{
	//vtFeature *f = m_pFeatures->GetFeature(iIndex);
	if (!m_pLayer)
		return;

	m_pLayer->DeleteFeature(f);
}

void FeatureTableDlg3d::OnFieldEdited(uint iIndex)
{
	m_pLayer->RefreshFeature(iIndex);
}

void FeatureTableDlg3d::OnEditEnd()
{
	if (!m_pLayer)
		return;

	m_pLayer->EditEnd();
}

