//
// vtui Helper.h
//  Some useful standalone functions for use with wxWidgets.
//
// Copyright (c) 2002-2009 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
/** \file Helper.h */

#include "vtdata/vtString.h"
#include "vtdata/MathTypes.h"

bool IsGUIApp();

class vtDIB;
class wxBitmap;
class wxColour;
class wxStaticBitmap;
class wxBitmapButton;
class wxComboBox;
class wxChoice;

//////

wxBitmap *MakeColorBitmap(int xsize, int ysize, wxColour color);
wxBitmap *DibToBitmap(vtDIB *dib);

void FillWithColor(wxStaticBitmap *pStaticBitmap, const wxColour &color);
void FillWithColor(wxBitmapButton *pBitmapButton, const wxColour &color);
void FillWithColorSize(wxStaticBitmap *pStaticBitmap, int w, int h, const wxColour &color);
void FillWithColorSize(wxBitmapButton *pBitmapButton, int w, int h, const wxColour &color);

void FillWithColor(wxStaticBitmap *pStaticBitmap, const RGBi &color);
void FillWithColor(wxBitmapButton *pBitmapButton, const RGBi &color);
void FillWithColorSize(wxStaticBitmap *pStaticBitmap, int w, int h, const RGBi &color);
void FillWithColorSize(wxBitmapButton *pBitmapButton, int w, int h, const RGBi &color);

int AddFilenamesToComboBox(wxComboBox *box, const char *directory,
	const char *wildcard, int omit_chars = 0);
int AddFilenamesToChoice(wxChoice *choice, const char *directory,
	const char *wildcard, int omit_chars = 0);
int AddFilenamesToStringArray(vtStringArray &array, const char *directory,
	const char *wildcard, int omit_chars = 0);
void AddType(wxString &str, const wxString &filter);

/////

void IncreaseRect(wxRect &rect, int adjust);
void DrawRectangle(wxDC *pDC, const wxRect &rect, bool bCrossed = false);

/////

#if WIN32
/**
 * Win32 allows us to do a real StretchBlt operation, although it still won't
 * do a StretchBlt with a mask.
 */
class wxDC2 : public wxDC
{
public:
	void StretchBlit(const wxBitmap &bmp, wxCoord x, wxCoord y,
		wxCoord width, wxCoord height, wxCoord src_x, wxCoord src_y,
		wxCoord src_width, wxCoord src_height);
};
#endif	// WIN32

bool LogWindowsVersion();
vtString FormatCoord(bool bGeo, double val, bool minsec = false);
enum wxLanguage GetLangFromName(const wxString &name);

///////////
// Helper methods which provide a progress dialog.

extern bool progress_callback(int amount);
void OpenProgressDialog(const wxString &title, const wxString &message,
	bool bCancellable = false, wxWindow *pParent = NULL);
void SetProgressDialogParent(wxWindow *pParent);
void CloseProgressDialog();
bool UpdateProgressDialog(int amount, const wxString& newmsg = wxT(""));
void ResumeProgressDialog();

///////////
// Helper methods which provide a 'double' progress dialog.

extern bool progress_callback2(int amount1, int amount2);
extern bool progress_callback_major(int amount);
extern bool progress_callback_minor(int amount);
void OpenProgressDialog2(const wxString &title, bool bCancellable = false, wxWindow *pParent = NULL);
void SetProgressDialog2Width(int width);
void CloseProgressDialog2();
bool UpdateProgressDialog2(int amount1, int amount2, const wxString& newmsg = wxT(""));
void ResumeProgressDialog2();

/////

int GuessZoneFromGeo(const DPoint2 &pos);

/////

vtString MakeFilenameDB(const vtString &base, int col, int row,
						int relative_lod);

/////
wxString StartOfFilenameWX(const wxString &strFullPath);
wxString ToBackslash(const wxString &path);
void RemoveFileExtensions(wxString &fname, bool bAll = true);

/////

// Initialize GDAL/OGR.  If there are problems, report them with a message box and VTLOG.
void CheckForGDALAndWarn();

// Display a message to the user, and also send it to the log file.
void DisplayAndLog(const char *pFormat, ...);
void DisplayAndLog(const wchar_t *pFormat, ...);
void DisplayAndLog(const wxString &msg);	// Non-variable-argument

void LaunchAppDocumentation(const vtString &appname,
							const vtString &local_lang_code);

void ConvertArgcArgv(int wxArgc, wxChar** wxArgv, int* pArgc, char*** pArgv);
