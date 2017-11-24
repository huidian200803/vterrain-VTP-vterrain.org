//
// Name:		FeatureTableDlg.cpp
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <algorithm>	// For std::min/max

#include "FeatureTableDlg.h"
#include "vtdata/vtLog.h"
#include "vtdata/Features.h"
#include "Helper.h"		// for ProgressDialog


// WDR: class implementations

//---------------------------------------------------------------------------
// FeatInfoDlg
//---------------------------------------------------------------------------

// WDR: event table for FeatInfoDlg

BEGIN_EVENT_TABLE(FeatureTableDlg, FeatureTableDlgBase)
	EVT_INIT_DIALOG (FeatureTableDlg::OnInitDialog)
	EVT_LIST_ITEM_SELECTED( ID_LIST, FeatureTableDlg::OnItemSelected )
	EVT_LIST_ITEM_DESELECTED( ID_LIST, FeatureTableDlg::OnItemSelected )
	EVT_LIST_ITEM_RIGHT_CLICK( ID_LIST, FeatureTableDlg::OnListRightClick )
	EVT_CHOICE( ID_CHOICE_SHOW, FeatureTableDlg::OnChoiceShow )
	EVT_CHOICE( ID_CHOICE_VERTICAL, FeatureTableDlg::OnChoiceVertical )
	EVT_BUTTON( ID_DEL_HIGH, FeatureTableDlg::OnDeleteHighlighted )
	EVT_LEFT_DCLICK( FeatureTableDlg::OnLeftDClick )
END_EVENT_TABLE()

FeatureTableDlg::FeatureTableDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	FeatureTableDlgBase( parent, id, title, position, size, style )
{
	m_iShow = 1;
	m_iVUnits = 0;
	m_pFeatures = NULL;

	AddValidator(this, ID_CHOICE_SHOW, &m_iShow);
	AddValidator(this, ID_CHOICE_VERTICAL, &m_iVUnits);

	UpdateTitle();
}

void FeatureTableDlg::SetFeatureSet(vtFeatureSet *pFeatures)
{
	if (m_pFeatures == pFeatures)
		return;

	m_pFeatures = pFeatures;

	GetList()->ClearAll();  // clears all items and columns
	m_iCoordColumns = 0;

	if (!m_pFeatures)
	{
		// No feature set, disable controls
		GetList()->Enable(false);
		GetChoiceShow()->Enable(false);
		GetDelHigh()->Enable(false);
		GetTextVertical()->Enable(false);
		GetChoiceVertical()->Enable(false);
		return;
	}

	vtCRS &crs = pFeatures->GetAtCRS();
	m_bGeo = (crs.IsGeographic() != 0);

	GetList()->Enable(true);
	GetChoiceShow()->Enable(true);

	int field = 0;
	OGRwkbGeometryType type = m_pFeatures->GetGeomType();
	if (type == wkbPoint || type == wkbPoint25D)
	{
		GetList()->InsertColumn(field++, _T("X"), wxLIST_FORMAT_LEFT, m_bGeo ? 90 : 60);
		GetList()->InsertColumn(field++, _T("Y"), wxLIST_FORMAT_LEFT, m_bGeo ? 90 : 60);
		m_iCoordColumns = 2;
	}
	if (type == wkbPoint25D)
	{
		GetList()->InsertColumn(field++, _T("Z"), wxLIST_FORMAT_LEFT, 70);
		m_iCoordColumns = 3;
	}

	GetTextVertical()->Enable(type == SHPT_POINTZ);
	GetChoiceVertical()->Enable(type == SHPT_POINTZ);

	uint i;
	for (i = 0; i < m_pFeatures->NumFields(); i++)
	{
		Field *pField = m_pFeatures->GetField(i);
		wxString name(pField->m_name, wxConvUTF8);
		int width1 = pField->m_width * 6;
		int width2 = name.Length() * 8;
		int width = std::max(width1, width2);
		if (width < 20)
			width = 20;
		GetList()->InsertColumn(field++, name, wxLIST_FORMAT_LEFT, width);
	}

	UpdateTitle();
}

void FeatureTableDlg::UpdateTitle()
{
	wxString title = _("Feature Info");
	if (m_pFeatures)
	{
		title += _T(": ");
		title += wxString(m_pFeatures->GetFilename(), wxConvUTF8);
	}
	SetTitle(title);
}

void FeatureTableDlg::Clear()
{
	GetList()->DeleteAllItems();
	GetDelHigh()->Enable(false);
}

void FeatureTableDlg::ShowSelected()
{
	if (!m_pFeatures)
		return;

	m_iShow = 0;
	TransferDataToWindow();
	Clear();

	int selected = m_pFeatures->NumSelected();
	if (selected > 2000)
	{
		wxString msg;
		msg.Printf(_("There are %d selected features.  Are you sure you\n want to display them all in the table view?"), selected);
		if (wxMessageBox(msg, _("Warning"), wxYES_NO) == wxNO)
			return;
	}
	bool bProgress = (selected > 500);
	if (bProgress)
		OpenProgressDialog(_("Populating table"), _T(""), false, this);
	int i, num = m_pFeatures->NumEntities();
	for (i = 0; i < num; i++)
	{
		if (bProgress && (i%20)==0)
			UpdateProgressDialog(i * 99 / num);
		if (m_pFeatures->IsSelected(i))
			ShowFeature(i);
	}
	if (bProgress)
		CloseProgressDialog();
}

void FeatureTableDlg::ShowPicked()
{
	if (!m_pFeatures)
		return;

	m_iShow = 1;
	TransferDataToWindow();
	Clear();

	int i, num = m_pFeatures->NumEntities();

	for (i = 0; i < num; i++)
	{
		if (m_pFeatures->IsPicked(i))
			ShowFeature(i);
	}
}

void FeatureTableDlg::ShowAll()
{
	if (!m_pFeatures)
		return;

	m_iShow = 2;
	TransferDataToWindow();
	Clear();

	int i, num = m_pFeatures->NumEntities();
	if (num > 2000)
	{
		wxString msg;
		msg.Printf(_("There are %d features in this layer.  Are you sure\n you want to display them all in the table view?"), num);
		if (wxMessageBox(msg, _("Warning"), wxYES_NO) == wxNO)
			return;
	}
	bool bProgress = (num > 500);
	if (bProgress)
		OpenProgressDialog(_("Populating table"), _T(""), false, this);
	for (i = 0; i < num; i++)
	{
		if (bProgress && (i%20)==0)
			UpdateProgressDialog(i * 99 / num);
		ShowFeature(i);
	}
	if (bProgress)
		CloseProgressDialog();
}

void FeatureTableDlg::ShowFeature(int iFeat)
{
	int next = GetList()->GetItemCount();
	GetList()->InsertItem(next, _T("temp"));
	GetList()->SetItemData(next, iFeat);

	UpdateFeatureText(next, iFeat);
}

void FeatureTableDlg::UpdateFeatureText(int iItem, int iFeat)
{
	int field = 0;

	wxString strFormat;
	if (m_bGeo)
		strFormat = _T("%.8lf");
	else
		strFormat = _T("%.2lf");

	wxString str;
	OGRwkbGeometryType type = m_pFeatures->GetGeomType();
	if (type == wkbPoint)
	{
		DPoint2 p2;
		((vtFeatureSetPoint2D *)m_pFeatures)->GetPoint(iFeat, p2);

		str.Printf(strFormat, p2.x);
		GetList()->SetItem(iItem, field++, str);
		str.Printf(strFormat, p2.y);
		GetList()->SetItem(iItem, field++, str);
	}
	if (type == wkbPoint25D)
	{
		DPoint3 p3;
		((vtFeatureSetPoint3D *)m_pFeatures)->GetPoint(iFeat, p3);

		str.Printf(strFormat, p3.x);
		GetList()->SetItem(iItem, field++, str);
		str.Printf(strFormat, p3.y);
		GetList()->SetItem(iItem, field++, str);

		double scale = GetMetersPerUnit((LinearUnits) (m_iVUnits+1));
		str.Printf(_T("%.2lf"), p3.z / scale);
		GetList()->SetItem(iItem, field++, str);
	}

	for (uint i = 0; i < m_pFeatures->NumFields(); i++)
	{
		vtString vs;
		m_pFeatures->GetValueAsString(iFeat, i, vs);
		wxString wide(vs, wxConvUTF8);
		GetList()->SetItem(iItem, field++, wide);
	}
}

void FeatureTableDlg::RefreshItems()
{
	if (m_iShow == 0)
		ShowSelected();
	else if (m_iShow == 1)
		ShowPicked();
	else if (m_iShow == 2)
		ShowAll();
}

bool FeatureTableDlg::EditValue(int iFeature, int iColumn)
{
	if (iColumn < m_iCoordColumns)
	{
		// TODO: allow numeric entry of point coordinates
	}
	else
	{
		int iField = iColumn - m_iCoordColumns;
		vtString vs;
		m_pFeatures->GetValueAsString(iFeature, iField, vs);

		wxString previous(vs, wxConvUTF8);

		Field *pField = m_pFeatures->GetField(iField);

		wxString message, caption;
		message.Printf(_("Enter a new value for field '%hs'"),
			(const char *) pField->m_name);
		caption.Printf(_("Text entry"));
		wxString str = wxGetTextFromUser(message, caption, previous, this);
		if (str != _T(""))
		{
			vs = str.mb_str(wxConvUTF8);
			m_pFeatures->SetValueFromString(iFeature, iField, vs);
			return true;
		}
	}
	return false;
}


// WDR: handler implementations for FeatureTableDlg

void FeatureTableDlg::OnLeftDClick( wxMouseEvent &event )
{
}

void FeatureTableDlg::OnInitDialog(wxInitDialogEvent& event)
{
	GetChoiceShow()->Clear();
	GetChoiceShow()->Append(_("Selected"));
	GetChoiceShow()->Append(_("Picked"));
	GetChoiceShow()->Append(_("All"));

	GetChoiceVertical()->Clear();
	GetChoiceVertical()->Append(_("Meter"));
	GetChoiceVertical()->Append(_("Foot"));
	GetChoiceVertical()->Append(_("Foot (US)"));

	GetDelHigh()->Enable(false);

	wxDialog::OnInitDialog(event);
}


void FeatureTableDlg::OnDeleteHighlighted( wxCommandEvent &event )
{
	int iDeleted = 0;
	int iFeat;
	int item = -1;
	for ( ;; )
	{
		item = GetList()->GetNextItem(item, wxLIST_NEXT_ALL,
									 wxLIST_STATE_SELECTED);
		if ( item == -1 )
			break;
		iFeat = (int) GetList()->GetItemData(item);
		m_pFeatures->SetToDelete(iFeat);
		iDeleted++;
	}
	if (iDeleted > 0)
	{
		VTLOG("Set %d items to delete, removing visuals..\n", iDeleted);

		// Delete high-level features first
		for (uint i = 0; i < m_pFeatures->NumEntities(); i++)
		{
			if (m_pFeatures->IsDeleted(i))
			{
				vtFeature *f = m_pFeatures->GetFeature(i);
				OnFeatureDelete(f);
			}
		}
		// Then low-level
		m_pFeatures->ApplyDeletion();

		// Finish
		OnEditEnd();

		OnModified();
		RefreshItems();
		RefreshViz();
	}
}

void FeatureTableDlg::OnChoiceVertical( wxCommandEvent &event )
{
	TransferDataFromWindow();
	RefreshItems();
}

void FeatureTableDlg::OnChoiceShow( wxCommandEvent &event )
{
	TransferDataFromWindow();
	RefreshItems();
}

void FeatureTableDlg::OnListRightClick( wxListEvent &event )
{
/*	int iFeat;
	int item = -1;

	item = GetList()->GetNextItem(item, wxLIST_NEXT_ALL,
								  wxLIST_STATE_SELECTED);
	if ( item == -1 )
		return; */
	int iItem = event.GetIndex();
	int iFeat = (int) GetList()->GetItemData(iItem);
/*	for (int i = 0; i < m_pFeatures->NumFields(); i++)
	{
		vtString vs;
		m_pFeatures->GetValueAsString(iFeat, i, vs);
		wstring2 wide;
		wide.from_utf8(vs);
		VTLOG(_T("Field %d, Value '%ls'\n"), i, wide.c_str());
	} */
	wxPoint mouse = event.GetPoint();
	int x = 0;
	int columns = GetList()->GetColumnCount();
	for (int i = 0; i < columns; i++)
	{
		int width = GetList()->GetColumnWidth(i);
		if (x < mouse.x && mouse.x < x+width)
		{
			// Found it;
			VTLOG("Clicked column %d\n", i);
			if (EditValue(iFeat, i))
			{
				// Refresh the display
				OnFieldEdited(iFeat);
				OnModified();
				UpdateFeatureText(iItem, iFeat);
			}
			return;
		}
		x += width;
	}
}

void FeatureTableDlg::OnItemSelected( wxListEvent &event )
{
	int count = 0;
	int item = -1;
	for ( ;; )
	{
		item = GetList()->GetNextItem(item, wxLIST_NEXT_ALL,
									 wxLIST_STATE_SELECTED);
		if ( item == -1 )
			break;
		count++;
	}
	GetDelHigh()->Enable(count > 0);
}

