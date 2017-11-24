//
// Name:		ProjectionDlg.cpp
//
// Copyright (c) 2002-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/numdlg.h>

#include "ProjectionDlg.h"
#include "StatePlaneDlg.h"
#include "Helper.h"			// for GuessZoneFromGeo

#include "vtdata/vtLog.h"
#include "vtdata/FileFilters.h"

//
// Must offset the values we use for Datum because a Choice
// control cannot handle "client data" values less than 0.
//
#define CHOICE_OFFSET 10

// WDR: class implementations

//---------------------------------------------------------------------------
// ProjectionDlg
//---------------------------------------------------------------------------

// WDR: event table for ProjectionDlg

BEGIN_EVENT_TABLE(ProjectionDlg, ProjectionDlgBase)
	EVT_INIT_DIALOG (ProjectionDlg::OnInitDialog)
	EVT_CHOICE( ID_PROJ, ProjectionDlg::OnProjChoice )
	EVT_BUTTON( ID_STATEPLANE, ProjectionDlg::OnSetStatePlane )
	EVT_BUTTON( ID_SET_EPSG, ProjectionDlg::OnSetEPSG )
	EVT_CHOICE( ID_ZONE, ProjectionDlg::OnZone )
	EVT_CHOICE( ID_HORUNITS, ProjectionDlg::OnHorizUnits )
	EVT_LIST_ITEM_RIGHT_CLICK( ID_PROJPARAM, ProjectionDlg::OnItemRightClick )
	EVT_CHOICE( ID_DATUM, ProjectionDlg::OnDatum )
	EVT_CHECKBOX( ID_SHOW_ALL_DATUMS, ProjectionDlg::OnShowAllDatums )
	EVT_BUTTON( ID_PROJ_LOAD, ProjectionDlg::OnProjLoad )
	EVT_BUTTON( ID_PROJ_SAVE, ProjectionDlg::OnProjSave )
END_EVENT_TABLE()

ProjectionDlg::ProjectionDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	ProjectionDlgBase( parent, id, title, position, size, style | wxRESIZE_BORDER )
{
	m_bInitializedUI = false;
	m_bShowAllDatums = false;

	m_GeoRefPoint.Set(0,0);

	m_pParamCtrl = GetProjparam();
	m_pZoneCtrl = GetZonechoice();
	m_pHorizCtrl = GetHorizchoice();
	m_pDatumCtrl = GetDatumchoice();
	m_pProjCtrl = GetProjchoice();

	AddValidator(this, ID_PROJ, &m_iProj);
	AddValidator(this, ID_ZONE, &m_iZone);
	AddValidator(this, ID_SHOW_ALL_DATUMS, &m_bShowAllDatums);

	// Work around wxFormDesigner's lack of support for limiting to smallest size
	GetSizer()->SetSizeHints(this);

	// The order here must match the ProjType enum!
	m_pProjCtrl->Append(_("Albers Equal Area Conic"));
	m_pProjCtrl->Append(_("Geographic"));
	m_pProjCtrl->Append(_("Hotine Oblique Mercator"));
	m_pProjCtrl->Append(_("Krovak"));
	m_pProjCtrl->Append(_("Lambert Azimuthal Equal-Area"));
	m_pProjCtrl->Append(_("Lambert Conformal Conic"));
	m_pProjCtrl->Append(_("Lambert Conformal Conic 1 SP"));
	m_pProjCtrl->Append(_("New Zealand Map Grid"));
	m_pProjCtrl->Append(_("Oblique Stereographic"));
	m_pProjCtrl->Append(_("Polar Stereographic"));
	m_pProjCtrl->Append(_("Sinusoidal"));
	m_pProjCtrl->Append(_("Stereographic"));
	m_pProjCtrl->Append(_("Mercator"));
	m_pProjCtrl->Append(_("Transverse Mercator"));
	m_pProjCtrl->Append(_("UTM"));
	m_pProjCtrl->Append(_("Dymaxion"));

	// Fill in choices for Datum
	RefreshDatums();

	m_pParamCtrl->ClearAll();
	m_pParamCtrl->InsertColumn(0, _("Attribute"));
	m_pParamCtrl->SetColumnWidth(0, 130);
	m_pParamCtrl->InsertColumn(1, _("Value"));
	m_pParamCtrl->SetColumnWidth(1, 85);

	m_bInitializedUI = true;
}

void ProjectionDlg::SetCRS(const vtCRS &crs)
{
	m_crs = crs;
	SetUIFromProjection();
}

void ProjectionDlg::OnInitDialog(wxInitDialogEvent& event)
{
	SetUIFromProjection();
	wxDialog::OnInitDialog(event);  // calls TransferDataToWindow()
}

void ProjectionDlg::RefreshDatums()
{
	m_pDatumCtrl->Clear();

	m_pDatumCtrl->Append(_("Unknown"), (void *) (-1+CHOICE_OFFSET));
	for (uint i = 0; i < g_EPSGDatums.size(); i++)
	{
		int code = g_EPSGDatums[i].iCode;
		wxString str(g_EPSGDatums[i].szName, wxConvUTF8);
		if (!m_bShowAllDatums)
		{
			if (!g_EPSGDatums[i].bCommon)
				continue;
		}
		void *clientData = (void *) (code+CHOICE_OFFSET);
		m_pDatumCtrl->Append(str, clientData);
	}
}


// Re-arrange the UI for a given CRS
void ProjectionDlg::SetProjectionUI(ProjType type)
{
	m_eProj = type;
	m_iProj = m_eProj;

	if (m_bInitializedUI)
		UpdateControlStatus();
}

void ProjectionDlg::UpdateControlStatus()
{
	int i, pos = 0;
	int real_zone;
	wxString str;

	m_pZoneCtrl->Clear();
	switch (m_eProj)
	{
	case PT_GEO:
		m_pParamCtrl->Enable(false);
		m_pZoneCtrl->Enable(false);
		break;
	case PT_UTM:
		m_pParamCtrl->Enable(false);
		m_pZoneCtrl->Enable(true);
		real_zone = m_crs.GetUTMZone();
		for (i = -60; i <= -1; i++)
		{
			str.Printf(_T("Zone %d"), i);
			m_pZoneCtrl->Append(str, (void *) (i+100));
			if (i == real_zone)
				m_iZone = pos;
			pos++;
		}
		for (i = 1; i <= 60; i++)
		{
			str.Printf(_T("Zone %d"), i);
			m_pZoneCtrl->Append(str, (void *) (i+100));
			if (i == real_zone)
				m_iZone = pos;
			pos++;
		}
		break;
	case PT_ALBERS:
	case PT_HOM:
	case PT_HOMAC:
	case PT_KROVAK:
	case PT_LCC:
	case PT_LCC1SP:
	case PT_LAEA:
	case PT_NZMG:
	case PT_MERC:
	case PT_TM:
	case PT_SINUS:
	case PT_STEREO:
	case PT_OS:
	case PT_PS:
		m_pParamCtrl->Enable(true);
		m_pZoneCtrl->Enable(false);
		break;
	case PT_DYMAX:
		m_pParamCtrl->Enable(false);
		m_pZoneCtrl->Enable(false);
		break;
	}
	m_iDatum = m_crs.GetDatum();
	UpdateDatumStatus();
	UpdateEllipsoid();

	// Do horizontal units ("linear units")
	m_pHorizCtrl->Clear();
	if (m_eProj == PT_GEO)
		m_pHorizCtrl->Append(_("Degrees"), (void *) LU_DEGREES);
	if (m_eProj != PT_GEO && m_eProj != PT_DYMAX)
		m_pHorizCtrl->Append(_("Meters"), (void *) LU_METERS);
	if (m_eProj != PT_GEO && m_eProj != PT_UTM && m_eProj != PT_DYMAX)
	{
		m_pHorizCtrl->Append(_("Feet (International)"), (void *) LU_FEET_INT);
		m_pHorizCtrl->Append(_("Feet (U.S. Survey)"), (void *) LU_FEET_US);
	}
	if (m_eProj == PT_DYMAX)
		m_pHorizCtrl->Append(_("Unit Edges"), (void *) LU_UNITEDGE);
	// manually transfer value
	for (uint j = 0; j < m_pHorizCtrl->GetCount(); j++)
	{
		if ((long int) m_pHorizCtrl->GetClientData(j) == m_crs.GetUnits())
			m_pHorizCtrl->SetSelection(j);
	}

	DisplayProjectionSpecificParams();

	TransferDataToWindow();
}

void ProjectionDlg::UpdateDatumStatus()
{
	bool bIsCommon = false;
	for (uint i = 0; i < g_EPSGDatums.size(); i++)
	{
		if (g_EPSGDatums[i].iCode == m_iDatum)
			bIsCommon = g_EPSGDatums[i].bCommon;
	}
	// If we've got a rare datum, but the UI is set to show the short list,
	//  change it to show the full list instead.
	if (!bIsCommon && !m_bShowAllDatums)
	{
		m_bShowAllDatums = true;
		RefreshDatums();
		TransferDataToWindow();
	}
	wxString str(DatumToString(m_iDatum), wxConvUTF8);
	m_pDatumCtrl->SetStringSelection(str);

	UpdateEllipsoid();
}

void ProjectionDlg::UpdateEllipsoid()
{
	wxString str;
	const char *ellip = m_crs.GetAttrValue("SPHEROID");
	if (ellip)
		str = wxString(ellip, wxConvUTF8);

	GetEllipsoid()->SetValue(str);
}

void ProjectionDlg::DisplayProjectionSpecificParams()
{
	m_pParamCtrl->DeleteAllItems();

	OGR_SRSNode *root = m_crs.GetRoot();
	if (!root)
	{
		m_pParamCtrl->InsertItem(0, _("(Invalid projection)"));
		return;	 // bogus projection
	}

	OGR_SRSNode *node, *par1, *par2;
	const char *value;
	int children = root->GetChildCount();
	int i, item = 0;

	wxString str;
	for (i = 0; i < children; i++)
	{
		node = root->GetChild(i);
		value = node->GetValue();
		if (!strcmp(value, "PARAMETER"))
		{
			par1 = node->GetChild(0);
			value = par1->GetValue();
			str = wxString(value, wxConvUTF8);
			item = m_pParamCtrl->InsertItem(item, str);

			par2 = node->GetChild(1);
			value = par2->GetValue();
			str = wxString(value, wxConvUTF8);
			m_pParamCtrl->SetItem(item, 1, str);
			item++;
		}
	}
}

void ProjectionDlg::SetUIFromProjection()
{
	if (m_crs.IsDymaxion())
		SetProjectionUI(PT_DYMAX);
	else if (m_crs.IsGeographic())
		SetProjectionUI(PT_GEO);
	else
	{
		const char *crs_string = m_crs.GetName();

		if (!strcmp(crs_string, SRS_PT_TRANSVERSE_MERCATOR))
		{
			if (m_crs.GetUTMZone() != 0)
				SetProjectionUI(PT_UTM);
			else
				SetProjectionUI(PT_TM);
		}

		// Supposedly, Gauss-Kruger is just a form of Transverse Mercator
		else if (!strcmp(crs_string, "Gauss_Kruger"))
			SetProjectionUI(PT_TM);

		else if (!strcmp(crs_string, SRS_PT_ALBERS_CONIC_EQUAL_AREA))
			SetProjectionUI(PT_ALBERS);

		else if (!strcmp(crs_string, SRS_PT_MERCATOR_1SP))
			SetProjectionUI(PT_MERC);

		else if (!strcmp(crs_string, SRS_PT_HOTINE_OBLIQUE_MERCATOR))
			SetProjectionUI(PT_HOM);

		else if (!strcmp(crs_string, SRS_PT_HOTINE_OBLIQUE_MERCATOR_AZIMUTH_CENTER))
			SetProjectionUI(PT_HOMAC);

		else if (!strcmp(crs_string, SRS_PT_LAMBERT_CONFORMAL_CONIC_1SP))
			SetProjectionUI(PT_LCC1SP);

		else if (!strcmp(crs_string, SRS_PT_LAMBERT_CONFORMAL_CONIC_2SP))
			SetProjectionUI(PT_LCC);

		else if (!strcmp(crs_string, SRS_PT_LAMBERT_AZIMUTHAL_EQUAL_AREA))
			SetProjectionUI(PT_LAEA);

		else if (!strcmp(crs_string, SRS_PT_NEW_ZEALAND_MAP_GRID))
			SetProjectionUI(PT_NZMG);

		else if (!strcmp(crs_string, SRS_PT_SINUSOIDAL))
			SetProjectionUI(PT_SINUS);

		else if (!strcmp(crs_string, SRS_PT_STEREOGRAPHIC))
			SetProjectionUI(PT_STEREO);

		else if (!strcmp(crs_string, SRS_PT_OBLIQUE_STEREOGRAPHIC))
			SetProjectionUI(PT_OS);

		else if (!strcmp(crs_string, SRS_PT_POLAR_STEREOGRAPHIC))
			SetProjectionUI(PT_PS);

		else if (!strcmp(crs_string, SRS_PT_KROVAK))
			SetProjectionUI(PT_KROVAK);

		// I've seen a .prj file for Stereo70 which refers to the projection
		//  as "Double_Stereographic", but this is unknown by OGR.  We do know
		//  about Oblique Stereographic, which is what i believe is meant.
		else if (!strcmp(crs_string, "Double_Stereographic"))
		{
			OGR_SRSNode *node = m_crs.GetAttrNode("PROJECTION");
			node = node->GetChild(0);
			node->SetValue(SRS_PT_OBLIQUE_STEREOGRAPHIC);
			SetProjectionUI(PT_OS);
		}
		else
		{
			wxString str = _("Unknown projection: ");
			str += wxString(crs_string, wxConvUTF8);
			wxMessageBox(str);
		}
	}
}

void ProjectionDlg::GetCRS(vtCRS &crs)
{
	crs = m_crs;
}


// WDR: handler implementations for ProjectionDlg

void ProjectionDlg::OnProjSave( wxCommandEvent &event )
{
	wxFileDialog saveFile(NULL, _("Save Coordinate System to File"), _T(""), _T(""),
		FSTRING_PRJ, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (saveFile.ShowModal() == wxID_CANCEL)
		return;
	wxString strPathName = saveFile.GetPath();
	m_crs.WriteProjFile(strPathName.mb_str(wxConvUTF8));
}

void ProjectionDlg::OnProjLoad( wxCommandEvent &event )
{
	wxFileDialog loadFile(NULL, _("Load Coordinate System from File"), _T(""), _T(""),
		FSTRING_PRJ, wxFD_OPEN);
	if (loadFile.ShowModal() != wxID_OK)
		return;
	wxString strPathName = loadFile.GetPath();
	if (m_crs.ReadProjFile(strPathName.mb_str(wxConvUTF8)))
		SetUIFromProjection();
	else
		wxMessageBox(_("Couldn't load coordinate system from that file.\n"));
}

void ProjectionDlg::OnDatum( wxCommandEvent &event )
{
	// operate on a copy for safety
	vtCRS copy = m_crs;
	int sel = event.GetInt();
	int datum_new = ((long int) m_pDatumCtrl->GetClientData(sel)) - CHOICE_OFFSET;
	OGRErr err = copy.SetDatum(datum_new);
	if (err == OGRERR_NONE)
	{
		// succeeded
		m_crs = copy;
		m_iDatum = datum_new;
	}
	else
	{
		// Failed.  The OGR error message has already gone to the debug log
		//  via CPL.  We just need to inform the user via the GUI.
		wxMessageBox(_("Couldn't set that Datum.  Perhaps the EPSG\n tables could not be located.  Check that your\n GEOTIFF_CSV environment variable is set."));
		SetUIFromProjection();
	}
	UpdateEllipsoid();
}

void ProjectionDlg::OnItemRightClick( wxListEvent &event )
{
	int item_clicked = event.GetIndex();

	OGR_SRSNode *root = m_crs.GetRoot();
	OGR_SRSNode *node, *par1, *par2;
	const char *value;
	int children = root->GetChildCount();
	int i, item = 0;
	wxString str;

	for (i = 0; i < children; i++)
	{
		node = root->GetChild(i);
		value = node->GetValue();
		if (strcmp(value, "PARAMETER"))
			continue;
		par1 = node->GetChild(0);
		par2 = node->GetChild(1);
		value = par2->GetValue();
		if (item == item_clicked)
		{
			wxString caption = _("Value for ");
			str = wxString(par1->GetValue(), wxConvUTF8);
			caption += str;
			str = wxString(value, wxConvUTF8);
			wxString result = wxGetTextFromUser(caption, _("Enter new value"),
				str, this);
			if (result != _T(""))
			{
//			  double newval = atof((const char *)result);
				par2->SetValue(result.mb_str(wxConvUTF8));
				DisplayProjectionSpecificParams();
				return;
			}
		}
		item++;
	}
}

void ProjectionDlg::OnHorizUnits( wxCommandEvent &event )
{
	TransferDataFromWindow();

	m_iUnits = m_pHorizCtrl->GetSelection();

	LinearUnits iUnits = (LinearUnits) (long int) m_pHorizCtrl->GetClientData(m_iUnits);

	if (iUnits == LU_METERS)
	{
		m_crs.SetLinearUnits(SRS_UL_METER, 1.0);
	}
	if (iUnits == LU_FEET_INT)
	{
		m_crs.SetLinearUnits(SRS_UL_FOOT, GetMetersPerUnit(iUnits));
	}
	if (iUnits == LU_FEET_US)
	{
		m_crs.SetLinearUnits(SRS_UL_US_FOOT, GetMetersPerUnit(iUnits));
	}

	TransferDataToWindow();
	UpdateControlStatus();
}

void ProjectionDlg::OnZone( wxCommandEvent &event )
{
	TransferDataFromWindow();

	void *vval = m_pZoneCtrl->GetClientData(m_iZone);
	long int val = (long int) vval - 100;
	m_crs.SetUTMZone(val);

	UpdateControlStatus();
}

void ProjectionDlg::OnSetStatePlane( wxCommandEvent &event )
{
	AskStatePlane();
}

void ProjectionDlg::OnSetEPSG( wxCommandEvent &event )
{
	// 4001 - 4904 for GCS values, 2000 - 3993 or 20004 - 32766 for PCS
	// Get an integer
	int value = m_crs.GuessEPSGCode();
	int minv = 2000;
	int maxv = 32766;
	value = wxGetNumberFromUser(_T(""), _("Enter EPSG code:"), _("Input"),
		value, minv, maxv);
	if (value < 0)
		return;

	OGRErr result = m_crs.importFromEPSG(value);
	if (result == OGRERR_FAILURE)
		wxMessageBox(_("Couldn't set EPSG coordinate system."));
	else
		SetUIFromProjection();
}

void ProjectionDlg::OnProjChoice( wxCommandEvent &event )
{
	TransferDataFromWindow();

	// Even lightweight tasks can runs into trouble with the Locale ./, issue.
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	m_crs.SetGeogCSFromDatum(m_iDatum);

	m_eProj = (ProjType) m_iProj;

	if (m_eProj != PT_DYMAX)
		m_crs.SetDymaxion(false);

	switch (m_eProj)
	{
	case PT_GEO:
		// nothing more to do
		break;
	case PT_UTM:
		// To be polite, suggest a UTM zone based roughly on where the user
		//  might have some data.
		m_iZone = GuessZoneFromGeo(m_GeoRefPoint);
		m_crs.SetUTMZone(m_iZone);
		break;
	case PT_ALBERS:
		// Put in some default values
		m_crs.SetACEA( 60.0, 68.0, 59.0, -132.5, 500000, 500000 );
		break;
	case PT_HOM:
		// Put in some default values; these are for Alaska Zone 1
		m_crs.SetHOM(57, -133.66666666666666,
			323.13010236111114, 323.13010236111114,
			0.9999, 5000000, -5000000);
		break;
	case PT_HOMAC:
		// Put in some default values; these are for Swiss CH-LV03 (EPSG 21781)
		m_crs.SetHOMAC(46.95240555555556, 7.439583333333333,
			90, 90, 1, 600000, 200000);
		break;
	case PT_KROVAK:
		// Put in some default values
		m_crs.SetKrovak(49.5, 24.83333333333333,
			30.28813975277778, 78.5, 0.9999, 0, 0);
		break;
	case PT_LAEA:
		// Put in some default values
		m_crs.SetLAEA( 51, -150, 1000000, 0 );
		break;
	case PT_LCC:
		// Put in some default values
		m_crs.SetLCC( 10, 20, 0, 15, 0, 0 );
		break;
	case PT_LCC1SP:
		// Put in some default values
		m_crs.SetLCC1SP( 30, 10, 1.0, 0, 0 );
		break;
	case PT_NZMG:
		// Put in some default values
		m_crs.SetNZMG( 41, 173, 2510000, 6023150 );
		break;
	case PT_MERC:
		// Put in some default values
		m_crs.SetMercator(0.0, 0.0, 1.0, 0, 0);
		break;
	case PT_TM:
		// Put in some default values
		// These are for the OSGB projection, a common case
		m_crs.SetTM(49.0, -2.0, 0.999601272, 400000, -100000);
		break;
	case PT_SINUS:
		// Put in some default values
		m_crs.SetSinusoidal(0, 0, 0); // dfCenterLong, dfFalseEasting, dfFalseNorthing
		break;
	case PT_STEREO:
		// Put in some default values
		m_crs.SetStereographic( 0.0, 0.0, 1.0, 0.0, 0.0);
		break;
	case PT_OS:
		// Put in some default values
		// These are for Stereo70 (Romania)
		m_crs.SetOS(45.0, 25.0, 0.999750,500000, 500000);
		break;
	case PT_PS:
		// Put in some default values
		// These are for the IBCAO polar bathymetry
		m_crs.SetPS(90.0, 0.0, 1.0, 0.0, 0.0);
		break;
	case PT_DYMAX:
		m_crs.SetDymaxion(true);
		break;
	}

	SetProjectionUI( (ProjType) m_iProj );
}

void ProjectionDlg::OnShowAllDatums( wxCommandEvent &event )
{
	TransferDataFromWindow();
	RefreshDatums();
	UpdateDatumStatus();
}

void ProjectionDlg::AskStatePlane()
{
	// Pop up choices for State Plane
	StatePlaneDlg dialog(this, 201, _("Select State Plane"));
	if (dialog.ShowModal() != wxID_OK)
		return;

	StatePlaneInfo *plane_info = GetStatePlaneTable();
	int selection = dialog.m_iStatePlane;
	int bNAD83 = !dialog.m_bNAD27;
	int usgs_code = plane_info[selection].usgs_code;

	OGRErr result;
	if (dialog.m_bFeet)
		result = m_crs.SetStatePlane(usgs_code, bNAD83, SRS_UL_FOOT, GetMetersPerUnit(LU_FEET_INT));
	else if (dialog.m_bFeetUS)
		result = m_crs.SetStatePlane(usgs_code, bNAD83, SRS_UL_US_FOOT, GetMetersPerUnit(LU_FEET_US));
	else
		result = m_crs.SetStatePlane(usgs_code, bNAD83);

	if (result == OGRERR_FAILURE)
	{
		wxMessageBox(_("Couldn't set state plane projection.  Perhaps the\n\
 EPSG tables could not be located.  Check that your\n\
 GEOTIFF_CSV environment variable is set."));
	}
	else
	{
		SetUIFromProjection();
	}
}

