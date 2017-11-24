//
// vtui Helper.cpp: Some useful standalone functions for use with wxWidgets.
//
// Copyright (c) 2002-2009 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/image.h"
#include "wx/progdlg.h"

#include "DoubleProgDlg.h"

#include "vtdata/FilePath.h"	// for dir_iter
#include "vtdata/GDALWrapper.h"
#include "vtdata/vtCRS.h"
#include "vtdata/vtLog.h"
#include "vtdata/vtDIB.h"
#include "Helper.h"

bool IsGUIApp()
{
	// Don't try to pop up a message box or progress dialog if called within a
	//  wx console app.  This function determines if we are a real GUI app.
	wxAppConsole *pAppCon = wxApp::GetInstance();
	wxApp *pApp = dynamic_cast<wxApp *>(pAppCon);
	return pApp != NULL;
}

/**
  Make a wxBitmap of a given pixel size, filled with a single color.
  
  The caller is responsible for deleting the bitmap later.
 */
wxBitmap *MakeColorBitmap(int xsize, int ysize, wxColour color)
{
	wxImage pImage(xsize, ysize);
	int i, j;
	for (i = 0; i < xsize; i++)
		for (j = 0; j < ysize; j++)
		{
			pImage.SetRGB(i, j, color.Red(), color.Green(), color.Blue());
		}

	bool bOk = pImage.Ok();

	wxBitmap *pBitmap = new wxBitmap(pImage);
	return pBitmap;
}

wxBitmap *DibToBitmap(vtDIB *dib)
{
	int xsize = dib->GetWidth();
	int ysize = dib->GetHeight();
	wxImage image(xsize, ysize);
	int i, j;
	RGBi rgb;
	for (i = 0; i < xsize; i++)
		for (j = 0; j < ysize; j++)
		{
			dib->GetPixel24(i, j, rgb);
			image.SetRGB(i, j, rgb.r, rgb.g, rgb.b);
		}
	return new wxBitmap(image);
}

void FillWithColorSize(wxStaticBitmap *pStaticBitmap, int w, int h, const wxColour &color)
{
	wxBitmap *pNewBitmap = MakeColorBitmap(w, h, color);
	pStaticBitmap->SetBitmap(*pNewBitmap);
	delete pNewBitmap;
}

void FillWithColorSize(wxBitmapButton *pBitmapButton, int w, int h, const wxColour &color)
{
	wxBitmap *pNewBitmap = MakeColorBitmap(w, h, color);
	pBitmapButton->SetBitmapLabel(*pNewBitmap);
	delete pNewBitmap;
}

void FillWithColor(wxStaticBitmap *pStaticBitmap, const wxColour &color)
{
	const wxBitmap &bm = pStaticBitmap->GetBitmap();
	FillWithColorSize(pStaticBitmap, bm.GetWidth(), bm.GetHeight(), color);
}

void FillWithColor(wxBitmapButton *pBitmapButton, const wxColour &color)
{
	const wxBitmap &bm = pBitmapButton->GetBitmapLabel();
	FillWithColorSize(pBitmapButton, bm.GetWidth(), bm.GetHeight(), color);
}

void FillWithColor(wxStaticBitmap *pStaticBitmap, const RGBi &color)
{
	FillWithColor(pStaticBitmap, wxColour(color.r, color.g, color.b));
}

void FillWithColor(wxBitmapButton *pBitmapButton, const RGBi &color)
{
	FillWithColor(pBitmapButton, wxColour(color.r, color.g, color.b));
}

void FillWithColorSize(wxStaticBitmap *pStaticBitmap, int w, int h, const RGBi &color)
{
	FillWithColorSize(pStaticBitmap, w, h, wxColour(color.r, color.g, color.b));
}

void FillWithColorSize(wxBitmapButton *pBitmapButton, int w, int h, const RGBi &color)
{
	FillWithColorSize(pBitmapButton, w, h, wxColour(color.r, color.g, color.b));
}

/**
 * This function is used to find all files in a given directory,
 * and if they match a wildcard, add them to a combo box.
 *
 * The wildcard comparison is case-insensitive, so for example "*.vtst"
 * will match "Foo.vtst" and "BAR.VTST".
 */
int AddFilenamesToChoice(wxChoice *choice, const char *directory,
	const char *wildcard, int omit_chars)
{
//  VTLOG(" AddFilenamesToChoice '%s', '%s':", directory, wildcard);

	int entries = 0, matches = 0;

	wxString wildstr(wildcard, wxConvUTF8);
	wildstr.LowerCase();

	for (dir_iter it((const char *)directory); it != dir_iter(); ++it)
	{
		entries++;
		std::string name1 = it.filename();
		//	VTLOG("   entry: '%s'", name1.c_str());
		if (it.is_hidden() || it.is_directory())
			continue;

		wxString name(name1.c_str(), wxConvUTF8);
		wxString name_lower = name;
		name_lower.LowerCase();

		if (name_lower.Matches(wildstr))
		{
			if (omit_chars)
				choice->Append(name.Left(name.Length()-omit_chars));
			else
				choice->Append(name);
			matches++;
		}
	}
//  VTLOG(" %d entries, %d matches\n", entries, matches);
	return matches;
}

/**
 * This function is used to find all files in a given directory,
 * and if they match a wildcard, add them to a combo box.
 *
 * The wildcard comparison is case-insensitive, so for example "*.vtst"
 * will match "Foo.vtst" and "BAR.VTST".
 */
int AddFilenamesToComboBox(wxComboBox *box, const char *directory,
	const char *wildcard, int omit_chars)
{
	// safety check
	if (!box)
		return 0;

//  VTLOG(" AddFilenamesToComboBox '%s', '%s':", directory, wildcard);

	// This does not work on all platforms, because wxComboBox is only a subclass
	//  of wxChoice on some wx flavors.
	//return AddFilenamesToChoice(box, directory, wildcard, omit_chars);

	// Instead, we need the same implementation
	int entries = 0, matches = 0;

	wxString wildstr(wildcard, wxConvUTF8);
	wildstr.LowerCase();

	for (dir_iter it((const char *)directory); it != dir_iter(); ++it)
	{
		entries++;
		std::string name1 = it.filename();
		//	VTLOG("   entry: '%s'", name1.c_str());
		if (it.is_hidden() || it.is_directory())
			continue;

		wxString name(name1.c_str(), wxConvUTF8);
		wxString name_lower = name;
		name_lower.LowerCase();

		if (name_lower.Matches(wildstr))
		{
			if (omit_chars)
				box->Append(name.Left(name.Length()-omit_chars));
			else
				box->Append(name);
			matches++;
		}
	}
//  VTLOG(" %d entries, %d matches\n", entries, matches);
	return matches;
}

/**
 * Read a directory and add all the filenames which match a given wildcard
 * to a string array.
 */
int AddFilenamesToStringArray(vtStringArray &array, const char *directory,
	const char *wildcard, int omit_chars)
{
	int matches = 0;
	for (dir_iter it(directory); it != dir_iter(); ++it)
	{
		if (it.is_hidden() || it.is_directory())
			continue;

		vtString name = it.filename().c_str();
		if (name.Matches(wildcard))
		{
			if (omit_chars)
				array.push_back(name.Left(name.GetLength()-omit_chars));
			else
				array.push_back(name);
			matches++;
		}
	}
	return matches;
}

//////////////////////////////////////

/**
 Add a file format type to a directory dialog filter string.
 
 For example, to ask the user for a BT or JPEG file:
 \code
	wxString filter = _("All Formats|");
	AddType(filter, _T("BT Files (*.bt)|*.bt"));
	AddType(filter, _T("JPEG Files (*.jpg;*.jpeg)|*.jpg;*.jpeg"));
	wxFileDialog loadFile(NULL, _("Open file"), _T(""), _T(""), filter, wxFD_OPEN);
 \endcode
 It is even easier if you use the standard strings from vtdata/FileFilters.h:
 \code
	AddType(filter, FSTRING_BT);
	AddType(filter, FSTRING_JPEG);
 \endcode
 */
void AddType(wxString &str, const wxString &filter)
{
	// Chop up the input string.  Expected form is "str1|str2|str3"
	wxString str1 = str.BeforeFirst('|');

	wxString str2 = str.AfterFirst('|');
	str2 = str2.BeforeFirst('|');

	wxString str3 = str.AfterFirst('|');
	str3 = str3.AfterFirst('|');

	// Chop up the filter string.  str4 is the wildcard part.
	wxString str4 = filter.AfterFirst('|');

	// Now rebuild the string, with the filter added
	wxString output = str1 + _T("|");
	output += str2;
	if (str2.Len() > 1)
		output += _T(";");
	output += str4;
	output += _T("|");
	if (str3.Len() > 1)
	{
		output += str3;
		output += _T("|");
	}
	output += filter;

	str = output;
}

//////////////////////////////////

void IncreaseRect(wxRect &rect, int adjust)
{
	rect.y -= adjust;
	rect.height += (adjust<<1);
	rect.x -= adjust;
	rect.width += (adjust<<1);
}

void DrawRectangle(wxDC *pDC, const wxRect &rect, bool bCrossed)
{
	int left = rect.x;
	int right = rect.x + rect.GetWidth();
	int top = rect.y;
	int bottom = rect.y + rect.GetHeight();
	wxPoint p[5];
	p[0].x = left;
	p[0].y = bottom;

	p[1].x = left;
	p[1].y = top;

	p[2].x = right;
	p[2].y = top;

	p[3].x = right;
	p[3].y = bottom;

	p[4].x = left;
	p[4].y = bottom;
	pDC->DrawLines(5, p);

	if (bCrossed)
	{
		pDC->DrawLine(left, bottom, right, top);
		pDC->DrawLine(left, top, right, bottom);
	}
}


//////////////////////////////////////

#if WIN32
//
// Win32 allows us to do a real StretchBlt operation directly from a bitmap
// (which is not a function that wxWidgets exposes) although it still won't
// do a StretchBlt with a mask, so this is no-mask only.
//
void wxDC2::StretchBlit(const wxBitmap &bmp,
						wxCoord x, wxCoord y,
						wxCoord width, wxCoord height,
						wxCoord src_x, wxCoord src_y,
						wxCoord src_width, wxCoord src_height)
{
	wxCHECK_RET( bmp.Ok(), _T("invalid bitmap in wxDC::DrawBitmap") );

	HDC cdc = ((HDC)GetHDC());
	HDC memdc = ::CreateCompatibleDC( cdc );
	HBITMAP hbitmap = (HBITMAP) bmp.GetHBITMAP( );

	HGDIOBJ hOldBitmap = ::SelectObject( memdc, hbitmap );

//	int bwidth = bmp.GetWidth(), bheight = bmp.GetHeight();
	::StretchBlt( cdc, x, y, width, height, memdc, src_x, src_y, src_width, src_height, SRCCOPY);

	::SelectObject( memdc, hOldBitmap );
	::DeleteDC( memdc );
}
#endif // WIN32

///////////////////////////////////////////////////////////////////////

#if WIN32

#pragma comment(lib, "Version.lib")

//
// This code comes from Microsoft; it's the gnarly way of finding out
//  at runtime exactly what version of Windows we are running on.
//
bool LogWindowsVersion()
{
	WCHAR path[_MAX_PATH];
	if (!GetSystemDirectoryW(path, _MAX_PATH))
		return false;

	wcscat_s(path, L"\\kernel32.dll");

	//
	// Based on example code from this article
	// http://support.microsoft.com/kb/167597
	//

	DWORD handle;
#if (_WIN32_WINNT >= _WIN32_WINNT_VISTA)
	DWORD len = GetFileVersionInfoSizeExW(FILE_VER_GET_NEUTRAL, path, &handle);
#else
	DWORD len = GetFileVersionInfoSizeW(path, &handle);
#endif
	if (!len)
		return false;

	uint8_t buff[2048];

#if (_WIN32_WINNT >= _WIN32_WINNT_VISTA)
	if (!GetFileVersionInfoExW(FILE_VER_GET_NEUTRAL, path, 0, len, buff))
#else
	if (!GetFileVersionInfoW(path, 0, len, buff.get()))
#endif
		return false;

	VS_FIXEDFILEINFO *vInfo = nullptr;
	UINT infoSize;

	if (!VerQueryValueW(buff, L"\\", reinterpret_cast<LPVOID*>(&vInfo), &infoSize))
		return false;

	if (!infoSize)
		return false;

	VTLOG("Windows %u.%u.%u.%u\n",
		HIWORD(vInfo->dwFileVersionMS),
		LOWORD(vInfo->dwFileVersionMS),
		HIWORD(vInfo->dwFileVersionLS),
		LOWORD(vInfo->dwFileVersionLS));

	return true;
}

/*
{
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;

	// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
	// If that fails, try using the OSVERSIONINFO structure.
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
	{
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) )
			return false;
	}

	switch (osvi.dwPlatformId)
	{
		// Test for the Windows NT product family.
		case VER_PLATFORM_WIN32_NT:
			// Test for the specific product family.
			if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
				VTLOG("Windows Server&nbsp;2003 family, ");

			if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
				VTLOG("Windows XP ");

			if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
				VTLOG("Windows 2000 ");

			if ( osvi.dwMajorVersion <= 4 )
				VTLOG("Windows NT ");

			// Test for specific product on Windows NT 4.0 SP6 and later.
			if( bOsVersionInfoEx )
			{
				// Test for the workstation type.
				if ( osvi.wProductType == VER_NT_WORKSTATION )
				{
					if( osvi.dwMajorVersion == 4 )
						VTLOG("Workstation 4.0 ");
					else if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
						VTLOG("Home Edition ");
					else
						VTLOG("Professional ");
				}

				// Test for the server type.
				else if ( osvi.wProductType == VER_NT_SERVER )
				{
					if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
					{
						if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
							VTLOG("Datacenter Edition ");
						else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
							VTLOG("Enterprise Edition ");
						else if ( osvi.wSuiteMask == VER_SUITE_BLADE )
							VTLOG("Web Edition ");
						else
							VTLOG("Standard Edition ");
					}

					else if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
					{
						if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
							VTLOG("Datacenter Server ");
						else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
							VTLOG("Advanced Server ");
						else
							VTLOG("Server ");
					}

					else  // Windows NT 4.0
					{
						if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
							VTLOG("Server 4.0, Enterprise Edition ");
						else
							VTLOG("Server 4.0 ");
					}
				}
			}
			else  // Test for specific product on Windows NT 4.0 SP5 and earlier
			{
				HKEY hKey;
				TCHAR szProductType[BUFSIZE];
				DWORD dwBufLen=BUFSIZE;
				LONG lRet;

				lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
					_T("SYSTEM\\CurrentControlSet\\Control\\ProductOptions"),
					0, KEY_QUERY_VALUE, &hKey );
				if( lRet != ERROR_SUCCESS )
					return false;

				lRet = RegQueryValueEx( hKey, _T("ProductType"), NULL, NULL,
					(LPBYTE) szProductType, &dwBufLen);
				if( (lRet != ERROR_SUCCESS) || (dwBufLen > BUFSIZE) )
					return false;

				RegCloseKey( hKey );

				if ( lstrcmpi( _T("WINNT"), szProductType) == 0 )
					VTLOG( "Workstation ");
				if ( lstrcmpi( _T("LANMANNT"), szProductType) == 0 )
					VTLOG( "Server ");
				if ( lstrcmpi( _T("SERVERNT"), szProductType) == 0 )
					VTLOG( "Advanced Server ");

				VTLOG( "%d.%d ", osvi.dwMajorVersion, osvi.dwMinorVersion );
			}

			// Display service pack (if any) and build number.
			if (osvi.dwMajorVersion == 4 &&
				 lstrcmpi( osvi.szCSDVersion, _T("Service Pack 6") ) == 0 )
			{
				HKEY hKey;
				LONG lRet;

				// Test for SP6 versus SP6a.
				lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
					_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q246009"),
					0, KEY_QUERY_VALUE, &hKey );
				if( lRet == ERROR_SUCCESS )
					VTLOG( "Service Pack 6a (Build %d)\n", osvi.dwBuildNumber & 0xFFFF );
				else // Windows NT 4.0 prior to SP6a
				{
					VTLOG( "%s (Build %d)\n",
						osvi.szCSDVersion,
						osvi.dwBuildNumber & 0xFFFF);
				}

				RegCloseKey( hKey );
			}
			else // Windows NT 3.51 and earlier or Windows 2000 and later
			{
				VTLOG( "%s (Build %d)\n",
					osvi.szCSDVersion,
					osvi.dwBuildNumber & 0xFFFF);
			}
			break;

		// Test for the Windows 95 product family.
		case VER_PLATFORM_WIN32_WINDOWS:
			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
			{
				 VTLOG("Windows 95 ");
				 if ( osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B' )
					 VTLOG("OSR2 ");
			}
			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
			{
				 VTLOG("Windows 98 ");
				 if ( osvi.szCSDVersion[1] == 'A' )
					 VTLOG("SE ");
			}
			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
			{
				 VTLOG("Windows Millennium Edition\n");
			}
			break;

		case VER_PLATFORM_WIN32s:
			VTLOG("Win32s\n");
			break;
	}
	return true;
}
*/
#endif // WIN32

/////////////////////////////////

vtString FormatCoord(bool bGeo, double val, bool minsec)
{
	vtString str;
	if (bGeo)
	{
		if (minsec)
		{
			// show minutes and seconds
			double degree = val;
			double min = (degree - (int)degree) * 60.0f;
			double sec = (min - (int)min) * 60.0f;

			str.Format("%d° %d' %.1f\"", (int)degree, (int)min, sec);
		}
		else
			str.Format("%3.6lf", val);	// decimal degrees
	}
	else
		str.Format("%.2lf", val);	// meters-based
	return str;
}

/////////////////////////////////

//
// Given a string name of a language like "de" or "German", return the
// enumerated language id, e.g. wxLANGUAGE_GERMAN.
//
enum wxLanguage GetLangFromName(const wxString &name)
{
	int lang;
	for (lang = wxLANGUAGE_ABKHAZIAN; lang < wxLANGUAGE_USER_DEFINED; lang++)
	{
		const wxLanguageInfo *info = wxLocale::GetLanguageInfo(lang);

		// safety check
		if (!info)
			continue;

		if (name.CmpNoCase(info->Description) == 0)
			return (enum wxLanguage) lang;
		if (name.Length() == 2)
		{
			wxString shortname = info->CanonicalName.Left(2);
			if (name.CmpNoCase(shortname) == 0)
				return (enum wxLanguage) lang;
		}
		else if (name.Length() == 5 && name[2] == '_')
		{
			if (name.CmpNoCase(info->CanonicalName) == 0)
				return (enum wxLanguage) lang;
		}
		else if (name.CmpNoCase(info->Description) == 0)
			return (enum wxLanguage) lang;
	}
	return wxLANGUAGE_UNKNOWN;
}


///////////////////////////////////////////////////////////////////////
// Shared Progress Dialog Functionality
//

static bool s_bOpen = false, s_bShown = false;
wxProgressDialog *g_pProg = NULL;
wxWindow *g_pProgParent = NULL;

void YieldForIdle()
{
	// In some cases (including wxMSW 2.9.x) the progress dialog seems to
	// starve the other windows of idle and redraw events.  We can
	// explicitly ask wx to make those happen.
#if wxCHECK_VERSION(2, 9, 0)
	wxApp::GetInstance()->ProcessIdle();
#endif
#ifdef __WXGTK__
	// Do we need to do something with GTK? wxYield on wxgtk2.8 seems to hang.
#else
	wxYield();
#endif
}

bool progress_callback(int amount)
{
	bool value = false;
	if (g_pProg)
	{
		if (!s_bShown && amount > 0)
		{
			g_pProg->Show(true);
			s_bShown = true;
		}
		if (s_bShown)
		{
			// Update() returns false if the Cancel button has been pushed
			// but this functions return _true_ if user wants to cancel
			value = (g_pProg->Update(amount) == false);
			//YieldForIdle();
		}
	}
	return value;
}

void SetProgressDialogParent(wxWindow *pParent)
{
	g_pProgParent = pParent;
}

void OpenProgressDialog(const wxString &title, const wxString &message,
	bool bCancellable, wxWindow *pParent)
{
	VTLOG1("OpenProgressDialog: ");
	if (s_bOpen)
	{
		VTLOG1("already open, returning\n");
		return;
	}
	else
		VTLOG1("opening new dialog\n");
	if (!IsGUIApp())
		return;
	if (!pParent)
		pParent = g_pProgParent;

	wxString msg = message;
	if (msg == _T(""))
	{
		// force the window to be wider by giving a dummy string
		msg = _T("___________________________________");
	}
	int style = wxPD_AUTO_HIDE | wxPD_APP_MODAL;
	if (bCancellable)
		style |= wxPD_CAN_ABORT;
	style |= wxPD_SMOOTH;

	s_bOpen = true;
	s_bShown = false;
	g_pProg = new wxProgressDialog(title, msg, 100, pParent, style);
	//g_pProg->Show(true);
	//g_pProg->Update(0, _T(" "));
}

void CloseProgressDialog()
{
	VTLOG1("CloseProgressDialog\n");
	if (g_pProg)
	{
		g_pProg->Destroy();
		g_pProg = NULL;
		s_bOpen = false;
		s_bShown = false;
	}
}

void ResumeProgressDialog()
{
	if (g_pProg)
		g_pProg->Resume();
}

//
// returns true if the user pressed the "Cancel" button
//
bool UpdateProgressDialog(int amount, const wxString& newmsg)
{
	bool value = false;
	if (g_pProg)
	{
		value = (g_pProg->Update(amount, newmsg) == false);
		YieldForIdle();
	}
	return value;
}


///////////////////////////////////////////////////////////////////////
// Shared Double Progress Dialog Functionality
//

static bool s_bOpen2 = false;
DoubleProgressDialog *g_pProg2 = NULL;

bool progress_callback2(int amount1, int amount2)
{
	bool value = false;
	// Update() returns false if the Cancel button has been pushed
	// but this functions return _true_ if user wants to cancel
	if (g_pProg2)
	{
		value = (g_pProg2->Update(amount1, amount2) == false);
		YieldForIdle();
	}
	return value;
}

bool progress_callback_major(int amount)
{
	bool value = false;
	// Update() returns false if the Cancel button has been pushed
	// but this functions return _true_ if user wants to cancel
	if (g_pProg2)
	{
		value = (g_pProg2->Update(amount, -1) == false);
		YieldForIdle();
	}
	return value;
}

bool progress_callback_minor(int amount)
{
	bool value = false;
	// Update() returns false if the Cancel button has been pushed
	// but this functions return _true_ if user wants to cancel
	if (g_pProg2)
	{
		value = (g_pProg2->Update(-1, amount) == false);
		YieldForIdle();
	}
	return value;
}

void OpenProgressDialog2(const wxString &title, bool bCancellable, wxWindow *pParent)
{
	if (s_bOpen2)
		return;
	if (!pParent)
		pParent = g_pProgParent;

	// force the window to be wider by giving a dummy string
	wxString message = _T("___________________________________");
	int style = wxPD_AUTO_HIDE | wxPD_APP_MODAL;
	if (bCancellable)
		style |= wxPD_CAN_ABORT;
	style |= wxPD_SMOOTH;

	s_bOpen2 = true;
	g_pProg2 = new DoubleProgressDialog(title, message, pParent, style);
	g_pProg2->Show(true);
	g_pProg2->Update(0, 0, _T(" "));
}

void SetProgressDialog2Width(int width)
{
	if (g_pProg2)
	{
		wxSize s = g_pProg2->GetSize();
		s.SetWidth(width);
		g_pProg2->SetSize(s);
	}
}

void CloseProgressDialog2()
{
	if (g_pProg2)
	{
		g_pProg2->Destroy();
		g_pProg2 = NULL;
		s_bOpen2 = false;
	}
}

void ResumeProgressDialog2()
{
	if (g_pProg2)
		g_pProg2->Resume();
}

//
// returns true if the user pressed the "Cancel" button
//
bool UpdateProgressDialog2(int amount1, int amount2, const wxString& newmsg)
{
	bool value = false;
	if (g_pProg2)
	{
		value = (g_pProg2->Update(amount1, amount2, newmsg) == false);
		YieldForIdle();
	}
	return value;
}


//////////////////////////////////////

int GuessZoneFromGeo(const DPoint2 &pos)
{
	int zone = (int) (((pos.x + 180.0) / 6.0) + 1.0);
	if (pos.y < 0)
		zone = -zone;
	return zone;
}

//////////////////////////////////////

// Assemble the filepath for a libMini .db file
vtString MakeFilenameDB(const vtString &base, int col, int row,
						int relative_lod)
{
	vtString fname = base, str;
	fname += '/';
	if (relative_lod == 0)
		str.Format("tile.%d-%d.db", col, row);
	else
		str.Format("tile.%d-%d.db%d", col, row, relative_lod);

	fname += str;
	return fname;
}

/**
 * Given a full path containing a filename, return a pointer to
 * the filename portion of the string.
 */
wxString StartOfFilenameWX(const wxString &strFullPath)
{
	int index = 0;

	int tmp1 = strFullPath.Find('/', true);
	if (tmp1 > index)
		index = tmp1+1;
	int tmp2 = strFullPath.Find('\\', true);
	if (tmp2 > index)
		index = tmp2+1;
	int tmp3 = strFullPath.Find(':', true);
	if (tmp3 > index)
		index = tmp3+1;

	return strFullPath.Mid(index);
}

/**
 * Return a copy of the string that has forward slashes converted to backslashes.
 * This is useful for passing paths and filenames to the file dialog on WIN32.
 */
wxString ToBackslash(const wxString &path)
{
	wxString	result;
	size_t	i, len = path.length();
	result.reserve(len);
	for ( i = 0; i < len; i++ )
	{
		wxChar ch = path.GetChar(i);
		switch ( ch )
		{
		case _T('/'):
			ch = _T('\\');	// convert to backslash
			// fall through
		default:
			result += ch;	// normal char
		}
	}
	return result;
}

/**
 * Given a filename (which may include a path), remove any file extension(s)
 * which it may have.
 */
void RemoveFileExtensions(wxString &fname, bool bAll)
{
	for (int i = fname.Length()-1; i >= 0; i--)
	{
		wxChar ch = fname[i];

		// If we hit a path divider, stop
		if (ch == ':' || ch == '\\' || ch == '/')
			break;

		// If we hit a period which indicates an extension, snip
		if (ch == '.')
		{
			fname = fname.Left(i);

			// if we're not snipping all the extensions, stop now
			if (!bAll)
				return;
		}
	}
}

/////////////////////////////////////////////////////

//
// Initialize GDAL/OGR.  If there are problems, report them with a message box and VTLOG.
//
void CheckForGDALAndWarn()
{
	// check for correctly set up environment variables and locatable files
	g_GDALWrapper.Init();
	GDALInitResult *gr = g_GDALWrapper.GetInitResult();

	if (!gr->hasGDAL_DATA)
	{
		vtString msg = "Unable to locate the necessary GDAL files for full coordinate\n"
			" system support. Without these files, many operations won't work.\n";
		DisplayAndLog(msg);
	}
	if (!gr->hasPROJ_LIB)
	{
		vtString msg = "Unable to locate the necessary PROJ.4 files for full coordinate\n"
			" system support. Without these files, many operations won't work.\n";
		DisplayAndLog(msg);
	}
	if (!gr->hasPROJSO)
	{
		vtString msg = "Unable to locate the PROJ.4 shared library for full coordinate\n"
			" system support. Without the file, many operations won't work.\n";
		DisplayAndLog(msg);
	}
	else
	{
		// Test that PROJ4 is actually working.
		if (!g_GDALWrapper.TestPROJ4())
			DisplayAndLog("Unable to transform coordinates.  This may be because the shared\n"
				"library for PROJ.4 is not found.  Without this, many operations won't work.");
	}
}


//
// Display a message to the user, and also send it to the log file.
//
void DisplayAndLog(const char *pFormat, ...)
{
	va_list va;
	va_start(va, pFormat);

	char ach[2048];
	vsprintf(ach, pFormat, va);

	wxString msg(ach, wxConvUTF8);

	// Careful here: Don't try to pop up a message box if called within a
	//  wx console app.  wxMessageBox only works if it is a full wxApp.
	if (IsGUIApp())
		wxMessageBox(msg);

	strcat(ach, "\n");
	VTLOG1(ach);
}

//
// Also wide-character version of the same function.
//
void DisplayAndLog(const wchar_t *pFormat, ...)
{
//#ifdef UNICODE
//	// Try to translate the string
//	wxString trans = wxGetTranslation(pFormat);
//	pFormat = trans.c_str();
//#endif

	va_list va;
	va_start(va, pFormat);

	// Use wide characters
	wchar_t ach[2048];
#ifdef _MSC_VER
	vswprintf(ach, pFormat, va);
#else
	// apparently on non-MSVC platforms this takes 4 arguments (safer)
	vswprintf(ach, 2048, pFormat, va);
#endif

	wxString msg(ach);

	// Careful here: Don't try to pop up a message box if called within a
	//  wx console app.  wxMessageBox only works if it is a full wxApp.
	if (IsGUIApp())
		wxMessageBox(msg);

	VTLOG1(msg.ToUTF8());
	VTLOG1("\n");
}

#if 0
// A wxString-taking version of the function, to make it perfectly clear
//  to the compiler which overloaded function to use.
//
// This method produces incorrect results, at least with wx2.9.4 and VC10.
// Passing an object (like wxString) instead of a char* for the format seems to
// confused the va_list methods.
//
void DisplayAndLog(const wxString &wxformat, ...)
{
	va_list va;
	va_start(va, wxformat);

	char ach[2048];
	vsprintf(ach, wxformat.ToUTF8(), va);

	wxString msg(ach, wxConvUTF8);

	// Careful here: Don't try to pop up a message box if called within a
	//  wx console app.  wxMessageBox only works if it is a full wxApp.
	if (IsGUIApp())
		wxMessageBox(msg);

	strcat(ach, "\n");
	VTLOG1(ach);
}
#endif

//
// A wxString version of the function, which takes a single string.
//
void DisplayAndLog(const wxString &msg)
{
	// Careful here: Don't try to pop up a message box if called within a
	//  wx console app.  wxMessageBox only works if it is a full wxApp.
	if (IsGUIApp())
		wxMessageBox(msg);

	vtString msg2 = (const char *) msg.ToUTF8();
	VTLOG1(msg2);
	VTLOG1("\n");
}

/**
 * Example: to launch the enviro documentation in Italian:
 * LaunchAppDocumentation("Enviro", "it");
 */
void LaunchAppDocumentation(const vtString &appname,
							const vtString &local_lang_code)
{
	// Launch default web browser with documentation pages
	wxString wxcwd = wxGetCwd();
	vtString cwd = (const char *) wxcwd.mb_str(wxConvUTF8);

	VTLOG("LaunchAppDocumentation: cwd is '%s'\n", (const char *) cwd);
	vtString cwd_up = PathLevelUp(cwd);

#if VTDEBUG && WIN32
	// During development, we are in a folder like /vtp/vc10/TerrainApps/App,
	// so need to go over to the source branch to find the Docs folder.
	cwd_up = PathLevelUp(cwd_up);
	cwd_up = PathLevelUp(cwd_up);
	cwd_up += "/TerrainApps/";
	cwd_up += appname;
#endif

	vtStringArray paths;

	// If the app is using a language other than English, look for docs
	//  in that language first.
	if (local_lang_code != "en")
		paths.push_back(cwd_up + "/Docs/" + local_lang_code + "/");
	if (local_lang_code != "en")
		paths.push_back(cwd + "/Docs/" + local_lang_code + "/");

	// Always fall back on English if the other language isn't found
	paths.push_back(cwd_up + "/Docs/en/");
	paths.push_back(cwd + "/Docs/en/");

	VTLOG1("Looking for index.html on paths:\n");
	for (size_t i = 0; i < paths.size(); i++)
	{
		VTLOG1("\t");
		VTLOG1(paths[i]);
		VTLOG1("\n");
	}

	vtString result = FindFileOnPaths(paths, "index.html");
	if (result == "")
	{
		DisplayAndLog("Couldn't find local documentation files");
		return;
	}
	vtString url;
	url.FormatForURL(result);
	url = "file:///" + url;
	VTLOG("Launching URL: %s\n", (const char *) url);
	wxLaunchDefaultBrowser(wxString(url, wxConvUTF8));
}

// Convert wxWidgets argc argv to char format
class MyLocalArgs
{
public:
	MyLocalArgs()
	{
		m_Argv = NULL;
		m_Argc = 0;
	}
	~MyLocalArgs()
	{
		Cleanup();
	}
	void Cleanup()
	{
		if (NULL != m_Argv)
		{
			for (int i = 0; NULL != m_Argv[i]; i++)
				delete[] m_Argv[i];
			delete[] m_Argv;
		}
	}
	void ConvertArgcArgv(int wxArgc, wxChar** wxArgv, int* pArgc, char*** pArgv)
	{
		Cleanup();
		*pArgc = m_Argc = wxArgc;
		*pArgv = m_Argv = new char*[m_Argc + 1];
		m_Argv[m_Argc] = NULL;
		for (int i = 0; i < m_Argc; i++)
		{
			const wxWX2MBbuf tmp_buf = wxConvCurrent->cWX2MB(wxArgv[i]);
			m_Argv[i] = new char[strlen((const char*)tmp_buf) + 1];
			strcpy(m_Argv[i], (const char*)tmp_buf);
		}
	}
private:
	char** m_Argv;
	int m_Argc;
};

static MyLocalArgs s_LocalArgs;

void ConvertArgcArgv(int wxArgc, wxChar** wxArgv, int* pArgc, char*** pArgv)
{
	return s_LocalArgs.ConvertArgcArgv(wxArgc, wxArgv, pArgc, pArgv);
}
