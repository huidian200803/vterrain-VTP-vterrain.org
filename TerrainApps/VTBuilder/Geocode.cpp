//
//  The geocoding functions of the VTBuilder application.
//
// Copyright (c) 2001-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/FileFilters.h"
#include "vtdata/GEOnet.h"
#include "vtdata/TripDub.h"
#include "vtui/Helper.h"

#include "Frame.h"
// Dialogs
#include "GeocodeDlg.h"

#if SUPPORT_WSTRING

bool FindGeoPointInBuffer(const char *buffer, DPoint2 &p)
{
	const char *lon = strstr(buffer, "<geo:long>");
	const char *lat = strstr(buffer, "<geo:lat>");
	if (lon && lat)
	{
		sscanf(lon+10, "%lf", &p.x);
		sscanf(lat+9, "%lf", &p.y);
		return true;
	}
	return false;
}

bool FindWithGeocoderUS(ReqContext &webcontext,
		const vtString &strStreet, const vtString &strCity,
		const vtString &strState, DPoint2 &p)
{
	bool bFound = false;

	// Must have street address; a city or zipcode won't suffice.
	if (strStreet == "")
		return false;

	if (!strStreet.Left(6).CompareNoCase("po box") ||
		!strStreet.Left(8).CompareNoCase("p.o. box"))
	{
		// forget it, geocoder.us doesn't do PO boxes
		return false;
	}
	vtString url;
	vtString result;
	int offset, chop;

	// Clean up the street address for geocoder.us
	vtString street = strStreet;

	// First, is this a multi-line street address?  If so, it's likely that only
	//  one of the lines is the proper street that TIGER knows.
	offset = street.Find('\n');
	if (offset != -1)
	{
		// chop it and examine each part
		vtString part1 = street.Left(offset);
		vtString part2 = street.Mid(offset+1);
		if (isdigit(part1[0]))
			street = part1;
		else if (isdigit(part2[0]))
			street = part2;
		else
		{
			int foo = 1;
		}
	}

	static const char *snames[21] = {
		"street ", "st. ", "avenue ", "ave. ",
		"drive ", "drive, ", "dr. ", " dr ", "dr., ",
		"boulevard ", "blvd. ", "bl ", "blvd, ",
		"place ", "pl. ",
		"road ", "rd. ", "road, ", "rd., ",
		"way ", "lane "};

	// Look for apartment numbers, suite etc. (they usually follow
	//  the "St." designation) and get rid of them.
	vtString lower = street;
	lower.MakeLower();
	int n;
	for (n = 0; n < (sizeof(snames)/sizeof(char*)); n++)
	{
		offset = lower.Find(snames[n]);
		if (offset != -1)
			break;
	}
	if (offset != -1)
	{
		chop = offset + strlen(snames[n]) - 1;
		street = street.Left(chop);
	}

	url = "http://rpc.geocoder.us/service/rest?address=";
	if (street != "")
	{
		url += street.FormatForURL();
		url += "+";
	}
	if (strCity != "")
	{
		url += strCity.FormatForURL();
		url += "+";
	}
	if (strState != "")
	{
		url += strState.FormatForURL();
	}
	if (webcontext.GetURL(url, result))
	{
		bFound = FindGeoPointInBuffer(result, p);
	}
	return bFound;
}

void MainFrame::DoGeocode()
{
	GeocodeDlg dlg(this, -1, _("Geocode"));
	dlg.m_bGeocodeUS = false;
	dlg.m_bGazetteer = false;
	dlg.m_bGNS = false;
	if (dlg.ShowModal() == wxID_CANCEL)
		return;

	wxString fname = dlg.m_strData;
	bool success;
	vtFeatureSetPoint2D feat;
	const DPoint2 zero(0,0);

	wxString shpname;
	if (!fname.Right(3).CmpNoCase(_T("shp")))
	{
		shpname = fname;
		OpenProgressDialog(_T("Reading"), fname, false);
		success = feat.LoadFromSHP(shpname.mb_str(wxConvUTF8), progress_callback);
		CloseProgressDialog();
	}
	else if (!fname.Right(3).CmpNoCase(_T("dbf")))
	{
		// DBFOpen doesn't yet support utf-8 or wide filenames, so convert
		vtString fname_local = UTF8ToLocal(fname.mb_str(wxConvUTF8));

		DBFHandle hDBF = DBFOpen(fname_local, "rb");
		if (!hDBF)
			return;
		int iDBFRecords = DBFGetRecordCount(hDBF);
		DBFClose(hDBF);

		vtFeatureSetPoint2D newfeat;
		newfeat.SetNumEntities(iDBFRecords);
		for (int i = 0; i < iDBFRecords; i++)
			newfeat.SetPoint(i, zero);

		shpname = fname.Left(fname.Length()-3);
		shpname += _T("shp");
		newfeat.SaveToSHP(shpname.mb_str(wxConvUTF8));

		// now re-open
		success = feat.LoadFromSHP(shpname.mb_str(wxConvUTF8));
	}
	else if (!fname.Right(3).CmpNoCase(_T("csv")))
	{
		success = feat.LoadDataFromCSV(fname.mb_str(wxConvUTF8));
		if (success)
		{
			// ensure the points are initialized to zero
			for (uint i = 0; i < feat.NumEntities(); i++)
				feat.SetPoint(i, zero);
		}
	}

	if (!success)
		return;
	int iRecords = feat.NumEntities();
	vtString strStreet;
	vtString strCity;
	vtString strState;
	vtString strCode;
	vtString strCountry;

	int rec, iKnown = 0, iFound = 0; // How many are already known
	DPoint2 p;

	// Used for geocode.us requests (when available)
	ReqContext webcontext;

	// Used by the Census Bureau Gazetteer
	Gazetteer gaz;
	bool bHaveGaz = false;
	bool bHaveZip = false;
	if (dlg.m_bGazetteer)
	{
		bHaveGaz = gaz.ReadPlaces(dlg.m_strGaz.mb_str(wxConvUTF8));
		bHaveZip = gaz.ReadZips(dlg.m_strZip.mb_str(wxConvUTF8));
	}

	// Use by the GEOnet Name Server (GNS) data files
	Countries countries;
	bool bHaveGNS = false;
	if (dlg.m_bGNS)
	{
		OpenProgressDialog(_T("Reading GNS file..."), fname, false);
		bHaveGNS = countries.ReadGCF(dlg.m_strGNS.mb_str(wxConvUTF8), progress_callback);
		CloseProgressDialog();
	}

	OpenProgressDialog(_T("Geocoding"), fname, true);
	bool bFound;
	for (rec = 0; rec < iRecords; rec++)
	{
		feat.GetPoint(rec, p);
		if (p != zero)
		{
			iKnown++;
			continue;
		}

		bFound = false;
		feat.GetValueAsString(rec, 7, strStreet);
		feat.GetValueAsString(rec, 8, strCity);
		feat.GetValueAsString(rec, 9, strState);
		feat.GetValueAsString(rec, 10, strCode);
		feat.GetValueAsString(rec, 11, strCountry);

		// show what we're working on in the progress dialog
		wxString msg;
		msg.Printf(_T("%d/%d: '%hs', '%hs'"), rec, iRecords,
			(const char *)strCity, (const char *)strCountry);
		if (UpdateProgressDialog(rec*100/iRecords, msg))
			break;

		// Try geocode.us first; it has the most detail
		if (!bFound && dlg.m_bGeocodeUS &&
			!strCountry.CompareNoCase("United States of America"))
		{
			bFound = FindWithGeocoderUS(webcontext, strStreet, strCity,
				strState, p);

			// pause to be nice to the server?
		}

		// Then (for US addresses) the gazetteer can look up a point for a
		//  zip code or city.
		if (!bFound && dlg.m_bGazetteer &&
			!strCountry.CompareNoCase("United States of America"))
		{
			// USA: Use zip code, if we have it
			if (bHaveZip && strCode != "")
			{
				// We only use the 5-digit code
				vtString five = strCode.Left(5);
				int zip = atoi(five);
				bFound = gaz.FindZip(zip, p);
			}
			if (!bFound && bHaveGaz)
			{
				// Use city/place name
				bFound = gaz.FindPlace(strState, strCity, p);
			}
		}

		// Then (for International addresses) GNS can get a point for a city
		if (!bFound && bHaveGNS &&
			strCountry.CompareNoCase("United States of America") != 0)
		{
			bool bUTF = true;	// !!
			bFound = countries.FindPlaceWithGuess(strCountry, strCity, p, bUTF);
		}

		if (bFound)
		{
			feat.SetPoint(rec, p);
			iFound++;
		}
	}
	CloseProgressDialog();

	wxString str;
	str.Printf(_T("%d records, %d already known\n%d/%d resolved, %d remain unknown"),
		iRecords, iKnown, iFound, iRecords-iKnown, iRecords-iKnown-iFound);
	wxMessageBox(str, _T("Results"));

	if (iFound != 0)
	{
		// Save to SHP
		wxFileDialog saveFile(NULL, _T("Save to SHP"), _T(""), _T(""),
			FSTRING_SHP, wxFD_SAVE);
		if (saveFile.ShowModal() == wxID_OK)
		{
			shpname = saveFile.GetPath();
			OpenProgressDialog(_T("Saving"), shpname, false);
			feat.SaveToSHP(shpname.mb_str(wxConvUTF8), progress_callback);
			CloseProgressDialog();
		}
	}

	if (bHaveGNS)
	{
		OpenProgressDialog(_T("Freeing GNS data..."), _T(""), false);
		countries.Free(progress_callback);
		CloseProgressDialog();
	}
}

#else
void MainFrame::DoGeocode()
{
}
#endif // SUPPORT_WSTRING