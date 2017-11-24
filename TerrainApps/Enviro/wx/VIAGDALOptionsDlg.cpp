/////////////////////////////////////////////////////////////////////////////
// Name:        VIAGDALOptionsDlg.cpp
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
    #pragma implementation "VIAGDALOptionsDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "VIAGDALOptionsDlg.h"
#include <gdal_priv.h>
#include "vtlib/vtlib.h"
#include "vtdata/vtLog.h"
#include <wx/filename.h>
#include <wx/stdpaths.h>

// WDR: class implementations

//----------------------------------------------------------------------------
// CVIAGDALOptionsDlg
//----------------------------------------------------------------------------

CVIAGDALOptionsDlg::CVIAGDALOptionsDlg( wxWindow *parent) : VIAGDALOptionsDlgBase(parent)
{
    m_ScaleFactor = wxString::Format(_("%0.2f"), 1.0);
	m_XSampleInterval = wxString::Format(_("%0.2f"), 10.0);
	m_YSampleInterval = wxString::Format(_("%0.2f"), 10.0);
}

void CVIAGDALOptionsDlg::Setup(int iDriverIndex)
{
    GDALDriver *pDriver = GetGDALDriverManager()->GetDriver(iDriverIndex);
    wxString DataTypes(pDriver->GetMetadataItem(GDAL_DMD_CREATIONDATATYPES), wxConvUTF8);
	wxFileName HelpTopic(wxStandardPaths::Get().GetConfigDir(), wxEmptyString);
#ifdef __WXMSW__
	HelpTopic.RemoveLastDir(); // Strip off the app name appended by wxWidgets
#endif
	HelpTopic.AppendDir(wxT("VTP"));
	HelpTopic.AppendDir(wxT("CommonData"));
    HelpTopic.AppendDir(wxT("frmts"));
    HelpTopic.SetFullName(wxString(pDriver->GetMetadataItem(GDAL_DMD_HELPTOPIC), wxConvUTF8));
    char *pToken;
	int iPos;

	if (HelpTopic.FileExists())
		m_pHtmlWindow->LoadPage(HelpTopic.GetFullPath());
    else
        VTLOG(_T("Cannot open gdal help file %s\n"), (const char *)HelpTopic.GetFullPath().mb_str(wxConvUTF8));

	char *datatypes = strdup((const char *) DataTypes.To8BitData());
    pToken = strtok(datatypes, " ");
	free(datatypes);
    while( pToken != NULL )
    {
        m_pDataTypeChoice->Append(wxString(pToken, wxConvUTF8));
        pToken = strtok( NULL, " ");
    }
	if (-1 != (iPos = m_pDataTypeChoice->FindString(wxT("Float32"))))
		m_pDataTypeChoice->SetSelection(iPos);
	else
		m_pDataTypeChoice->SetSelection(0);
}

