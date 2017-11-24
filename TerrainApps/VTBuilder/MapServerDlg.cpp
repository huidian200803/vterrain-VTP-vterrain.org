//
// Name: MapServerDlg.cpp
//
// Copyright (c) 2003-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "MapServerDlg.h"
#if SUPPORT_CURL
#include "vtdata/WFSClient.h"
#endif
#include "vtdata/FileFilters.h"	// for FSTRING filters
#include "vtdata/vtLog.h"
#include "vtui/AutoDialog.h"
#include "vtui/Helper.h"	// for progress dialog

// WDR: class implementations

//----------------------------------------------------------------------------
// MapServerDlg.cpp
//----------------------------------------------------------------------------

// WDR: event table for MapServerDlg.cpp

BEGIN_EVENT_TABLE(MapServerDlg, MapServerDlgBase)
	EVT_INIT_DIALOG (MapServerDlg::OnInitDialog)
	EVT_COMBOBOX( ID_BASE_URL, MapServerDlg::OnServer )
	EVT_TEXT( ID_BASE_URL, MapServerDlg::OnBaseUrlText )
	EVT_TEXT( ID_WIDTH, MapServerDlg::OnSize )
	EVT_TEXT( ID_HEIGHT, MapServerDlg::OnSize )
	EVT_CHOICE( ID_CHOICE_FORMAT, MapServerDlg::OnFormat )
	EVT_LISTBOX( ID_LIST_LAYERS, MapServerDlg::OnLayer )
	EVT_BUTTON( ID_QUERY_LAYERS, MapServerDlg::OnQueryLayers )
	EVT_RADIOBUTTON( ID_RADIO_CREATE_NEW, MapServerDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO_TO_FILE, MapServerDlg::OnRadio )
	EVT_BUTTON( ID_DOTDOTDOT, MapServerDlg::OnDotDotDot )
	EVT_TEXT( ID_TEXT_TO_FILE, MapServerDlg::OnTextToFile )
END_EVENT_TABLE()

MapServerDlg::MapServerDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	MapServerDlgBase( parent, id, title, position, size, style )
{
	m_bSetting = false;
	m_iXSize = 1024;
	m_iYSize = 1024;
	m_iServer = -1;
	m_iLayer = -1;
	m_iStyle = -1;
	m_iFormat = 1;
	m_bNewLayer = true;
	m_bToFile = false;

	AddValidator(this, ID_RADIO_CREATE_NEW, &m_bNewLayer);
	AddValidator(this, ID_RADIO_TO_FILE, &m_bToFile);

	AddNumValidator(this, ID_WIDTH, &m_iXSize);
	AddNumValidator(this, ID_HEIGHT, &m_iYSize);

	AddValidator(this, ID_QUERY, &m_strQueryURL);   // query url string
	AddValidator(this, ID_CHOICE_FORMAT, &m_iFormat);
	AddValidator(this, ID_TEXT_TO_FILE, &m_strToFile);

	GetListLayers()->Clear();
	GetListLayers()->Append(_T("<none>"));
	GetListLayers()->SetSelection(0);

	GetFormat()->Clear();
	GetFormat()->Append(_T("JPEG"));
	GetFormat()->Append(_T("PNG"));
	GetFormat()->Append(_T("GeoTIFF"));
	GetFormat()->SetSelection(0);

	GetSizer()->SetSizeHints(this);
}

void MapServerDlg::SetServerArray(OGCServerArray &array)
{
	m_pServers = &array;

	// default to first server
	if (m_pServers->size() > 0)
		m_iServer = 0;
}

void MapServerDlg::UpdateEnabling()
{
	GetTextToFile()->Enable(m_bToFile);
	GetDotdotdot()->Enable(m_bToFile);

	// For now, going straight to memory must be PNG
	if (!m_bToFile)
		GetFormat()->SetSelection(1);
	GetFormat()->Enable(m_bToFile);
}


// WDR: handler implementations for MapServerDlg.cpp

void MapServerDlg::OnTextToFile( wxCommandEvent &event )
{
	TransferDataFromWindow();
}

void MapServerDlg::OnDotDotDot( wxCommandEvent &event )
{
	wxString filter = _("All Files|*.*");
	if (m_iFormat == 0)
		AddType(filter, FSTRING_JPEG);
	if (m_iFormat == 1)
		AddType(filter, FSTRING_PNG);
	if (m_iFormat == 2)
		AddType(filter, FSTRING_TIF);

	// ask the user for a filename
	wxFileDialog saveFile(NULL, _("Save Imagery"), _T(""), _T(""), filter, wxFD_SAVE);
	saveFile.SetFilterIndex(1);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	m_strToFile = saveFile.GetPath();

	// update controls
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void MapServerDlg::OnRadio( wxCommandEvent &event )
{
	TransferDataFromWindow();
	UpdateEnabling();
}

void MapServerDlg::OnQueryLayers( wxCommandEvent &event )
{
#if SUPPORT_CURL
	VTLOG1("OnQueryLayers\n");

	wxString val = GetBaseUrl()->GetValue();
	vtString url = (const char *) val.mb_str(wxConvUTF8);
	
	OpenProgressDialog(_("Querying server..."), val, false, this);

	VTLOG("  from base URL: %s\n", (const char *)url);

	vtString msg;
	bool success = GetLayersFromWMS(url, m_pServers->at(m_iServer).m_layers,
		msg, progress_callback);

	CloseProgressDialog();

	if (success)
	{
		UpdateLayerList();
		UpdateLayerDescription();
		UpdateURL();
	}
	else
	{
		VTLOG("Error: '%s'\n", (const char *)msg);
		wxString str(msg, wxConvUTF8);
		wxMessageBox(str);
	}
#endif // SUPPORT_CURL
}

void MapServerDlg::OnServer( wxCommandEvent &event )
{
	m_iServer = GetBaseUrl()->GetSelection();
	m_pServers->m_iSelected = m_iServer;
	UpdateLayerList();
	UpdateLayerDescription();
	UpdateURL();
}

void MapServerDlg::OnBaseUrlText( wxCommandEvent &event )
{
	TransferDataFromWindow();
	wxString val = GetBaseUrl()->GetValue();

	// Remove "GetCaps" so we have the base URL
	vtString url = (const char *) val.mb_str(wxConvUTF8);

	// If it already has the "GetCapabilties", remove it
	int getcaps = url.Find("&request=GetCapabilities");
	if (getcaps == -1)
		getcaps = url.Find("?request=GetCapabilities");
	if (getcaps != -1)
	{
		url.Delete(getcaps, strlen("&request=GetCapabilities"));
		GetBaseUrl()->SetValue(wxString(url, wxConvUTF8));
	}

	m_pServers->at(m_iServer).m_url = url;
	UpdateURL();
}

void MapServerDlg::OnLayer( wxCommandEvent &event )
{
	if (m_iServer == -1)
		return;

	OGCLayerArray &layers = m_pServers->at(m_iServer).m_layers;
	int num = layers.size();
	if (num == 0)
		return;

	m_iLayer = GetListLayers()->GetSelection();
	layers.m_iSelected = m_iLayer;
	UpdateLayerDescription();
	UpdateURL();
}

void MapServerDlg::OnFormat( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	UpdateURL();
}

void MapServerDlg::OnInitDialog(wxInitDialogEvent& event)
{
	GetBaseUrl()->Clear();
	int numservers = m_pServers->size();
	for (int i = 0; i < numservers; i++)
	{
		wxString str(m_pServers->at(i).m_url, wxConvUTF8);
		GetBaseUrl()->Append(str);
	}
	// If no selection, pick the first server
	if (m_pServers->m_iSelected == -1 && numservers > 0)
		m_pServers->m_iSelected = 0;
	// Keep selection in range
	if (m_pServers->m_iSelected >= numservers)
		m_pServers->m_iSelected = numservers-1;
	// Apply selection
	if (m_pServers->m_iSelected != -1)
		GetBaseUrl()->SetSelection(m_pServers->m_iSelected);
	m_iServer = m_pServers->m_iSelected;

	UpdateEnabling();
	UpdateLayerList();
	UpdateLayerDescription();

	m_bSetting = true;
	wxWindow::OnInitDialog(event);
	m_bSetting = false;
}

void MapServerDlg::OnSize( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	UpdateURL();
}

void MapServerDlg::UpdateLayerList()
{
	GetListLayers()->Clear();
	if (m_iServer == -1)
		return;

	OGCLayerArray &layers = m_pServers->at(m_iServer).m_layers;
	int num = layers.size();
	if (num == 0)
	{
		GetListLayers()->Append(_("<none>"));
		m_iLayer = -1;
	}
	else
	{
		for (int i = 0; i < num; i++)
		{
			vtString str;
			vtTagArray *tags = layers[i];
			vtTag *tag = tags->FindTag("Name");
			if (tag)
				GetListLayers()->Append(wxString(tag->value, wxConvUTF8));
		}
	}
	// If no selection, pick the first layer
	if (layers.m_iSelected == -1 && num > 0)
		layers.m_iSelected = 0;
	// Keep selection in range
	if (layers.m_iSelected >= num)
		layers.m_iSelected = num-1;
	// Apply selection
	if (layers.m_iSelected != -1)
		GetListLayers()->SetSelection(layers.m_iSelected);
	m_iLayer = layers.m_iSelected;
}

void MapServerDlg::UpdateLayerDescription()
{
	GetLayerDesc()->Clear();
	if (m_iServer == -1 || m_iLayer == -1)
		return;

	wxString str;
	vtTag *tag;
	tag = m_pServers->at(m_iServer).m_layers.at(m_iLayer)->FindTag("Title");
	if (tag)
	{
		str += _("Title: ");
		str += wxString(tag->value, wxConvUTF8);
		str += _T("\n");
	}
	tag = m_pServers->at(m_iServer).m_layers.at(m_iLayer)->FindTag("Abstract");
	if (tag)
	{
		str += _("Abstract: ");
		str += wxString(tag->value, wxConvUTF8);
		str += _T("\n");
	}
	GetLayerDesc()->SetValue(str);
}

void MapServerDlg::UpdateURL()
{
	if (m_iServer == -1)
		return;

	// Avoid problem with european decimal punctuation; the BBOX string in
	//  particular must have coords formatted as X.Y not X,Y
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	OGCServer &server = m_pServers->at(m_iServer);
	vtString url = server.m_url;

	int has_qmark = (url.Find("?") != -1);

	if (has_qmark)
		url += "&REQUEST=GetMap";
	else
		url += "?REQUEST=GetMap";

	// Some servers seem to insist on a VERSION element
	url += "&VERSION=1.1.0";

	// some severs need the following to clarify we want WMS
	url += "&SERVICE=WMS";

	url += "&LAYERS=";  // required, even if left blank
	if (m_iLayer != -1)
	{
		vtTagArray *layer = server.m_layers.at(m_iLayer);
		vtString layername = layer->GetValueString("Name");
		url += layername;

		url += "&STYLES=";  // required, even if left blank
		if (m_iStyle != -1)
		{
			// TODO
		}
	}
	int epsg = m_crs.GuessEPSGCode();
	if (epsg == -1)
		epsg = 4326;		// 4326 = WGS84
	vtString str;
	str.Format("&SRS=EPSG:%d", epsg);
	url += str;

	str.Format("&BBOX=%lf,%lf,%lf,%lf", m_area.left, m_area.bottom, m_area.right, m_area.top);
	url += str;

	str.Format("&WIDTH=%d&HEIGHT=%d", m_iXSize, m_iYSize);
	url += str;
	if (m_iFormat == 0) url += "&FORMAT=image/jpeg";
	if (m_iFormat == 1) url += "&FORMAT=image/png";
	if (m_iFormat == 2) url += "&FORMAT=image/geotiff";

	url += "&TRANSPARENT=TRUE&EXCEPTIONS=WMS_XML&";

	m_strQueryURL = wxString(url, wxConvUTF8);

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

