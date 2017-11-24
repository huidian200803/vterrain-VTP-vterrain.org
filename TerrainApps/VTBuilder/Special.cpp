//
//  The Special menu functions of the VTBuilder application.
//
// Copyright (c) 2003-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/progdlg.h>

#include "vtdata/vtDIB.h"
#include "vtdata/vtLog.h"
#include "vtdata/ElevationGrid.h"
#include "vtdata/FileFilters.h"
#include "vtdata/Icosa.h"
#include "vtui/Helper.h"	// for ProgressDialog
#include "vtui/SizeDlg.h"

#include "Frame.h"
#include "ElevLayer.h"

bool ProcessBillboardTexture(const char *fname_in, const char *fname_out,
							 const RGBi &bg, bool progress_callback(int) = NULL)
{
	float blend_factor;
	vtDIB dib1, dib2, dib3;
	if (!dib1.ReadPNG(fname_in))
	{
		DisplayAndLog("Couldn't read input file.");
		return false;
	}
	const IPoint2 size = dib1.GetSize();

	// First pass: restore color of edge texels by guessing correct
	//  non-background color.
	RGBAi c, res, diff;
	dib2.Allocate(size, 32);
	for (int i = 0; i < size.x; i++)
	{
		progress_callback(i * 100 / size.x);
		for (int j = 0; j < size.y; j++)
		{
			dib1.GetPixel32(i, j, c);
			if (c.a == 0)
			{
				res = bg;
				res.a = 0;
			}
			else if (c.a == 255)
			{
				res = c;
			}
			else
			{
				blend_factor = c.a / 255.0f;

				diff = c - bg;
				res = bg + (diff * (1.0f / blend_factor));
				res.Crop();
				res.a = c.a;
			}
			dib2.SetPixel32(i, j, res);
		}
	}

	// Now make many passes over the bitmap, filling in areas of alpha==0
	//  with values from the nearest pixels.
	dib3.Allocate(size, 24);
	int filled_in = 1;
	int progress_target = -1;
	while (filled_in)
	{
		filled_in = 0;
		dib3.SetColor(RGBi(0,0,0));

		RGBi sum;
		int surround;
		for (int i = 0; i < size.x; i++)
		{
			for (int j = 0; j < size.y; j++)
			{
				dib2.GetPixel32(i, j, c);
				if (c.a != 0)
					continue;

				// collect surrounding values
				sum.Set(0,0,0);
				surround = 0;
				for (int x = -1; x <= 1; x++)
				for (int y = -1; y <= 1; y++)
				{
					if (x == 0 && y == 0) continue;
					if (i + x < 0) continue;
					if (i + x > size.x - 1) continue;
					if (j + y < 0) continue;
					if (j + y > size.y - 1) continue;
					dib2.GetPixel32(i+x, j+y, c);
					if (c.a != 0)
					{
						sum += c;
						surround++;
					}
				}
				if (surround > 2)
				{
					sum /= (float) surround;
					dib3.SetPixel24(i, j, sum);
				}
			}
		}
		for (int i = 0; i < size.x; i++)
		{
			for (int j = 0; j < size.y; j++)
			{
				dib2.GetPixel32(i, j, c);
				if (c.a == 0)
				{
					dib3.GetPixel24(i, j, sum);
					if (sum.r != 0 || sum.g != 0 || sum.b != 0)
					{
						c = sum;
						c.a = 1;
						dib2.SetPixel32(i, j, c);
						filled_in++;
					}
				}
			}
		}
		if (progress_target == -1 && filled_in > 0)
			progress_target = filled_in * 2 / 3;
		progress_callback((progress_target - filled_in) * 100 / progress_target);
	}
	// One final pass: changed the regions with alpha==1 to 0
	// (we were just using the value as a flag)
	for (int i = 0; i < size.x; i++)
	{
		progress_callback(i * 100 / size.x);
		for (int j = 0; j < size.y; j++)
		{
			dib2.GetPixel32(i, j, c);
			if (c.a == 1)
				c.a = 0;
			dib2.SetPixel32(i, j, c);
		}
	}

	if (dib2.WritePNG(fname_out))
		DisplayAndLog("Successful.");
	else
		DisplayAndLog("Unsuccessful.");
	return true;
}


void MainFrame::DoProcessBillboard()
{
	wxTextEntryDialog dlg1(this,
		_T("This feature allows you to process a billboard texture to remove\n")
		_T("unwanted background effects on its edges.  The file should be in\n")
		_T("PNG format, 24-bit color plus 8-bit alpha.  To begin, specify the\n")
		_T("current background color of the image to process.  Enter the color\n")
		_T("as R G B, e.g. black is 0 0 0 and white is 255 255 255."), _T("Wizard"));
	if (dlg1.ShowModal() == wxID_CANCEL)
		return;
	RGBi bg;
	wxString str = dlg1.GetValue();
	vtString color = (const char *) str.mb_str(wxConvUTF8);
	int res = sscanf(color, "%hd %hd %hd", &bg.r, &bg.g, &bg.b);
	if (res != 3)
	{
		DisplayAndLog("Couldn't parse color.");
		return;
	}
	wxFileDialog dlg2(this, _T("Choose input texture file"), _T(""), _T(""), _T("*.png"));
	if (dlg2.ShowModal() == wxID_CANCEL)
		return;
	str = dlg2.GetPath();
	vtString fname_in = (const char *) str.mb_str(wxConvUTF8);

	wxFileDialog dlg3(this, _T("Choose output texture file"), _T(""), _T(""),
		_T("*.png"), wxFD_SAVE);
	if (dlg3.ShowModal() == wxID_CANCEL)
		return;
	str = dlg3.GetPath();
	vtString fname_out = (const char *) str.mb_str(wxConvUTF8);

	OpenProgressDialog(_T("Processing"), _T(""));

	ProcessBillboardTexture(fname_in, fname_out, bg, progress_callback);

	CloseProgressDialog();
}

void MainFrame::DoElevCopy()
{
#if WIN32
	int opened = ::OpenClipboard((HWND)GetHandle());
	if (opened)
	{
		if (!::EmptyClipboard())
		{
			::CloseClipboard();
			wxMessageBox(_T("Cannot empty clipboard."));
			return;
		}

		ElevCopy();
		::CloseClipboard();
	}
#endif	// WIN32
}

void MainFrame::DoElevPasteNew()
{
#if WIN32
	int opened = ::OpenClipboard((HWND)GetHandle());
	if (opened)
	{
		ElevPasteNew();
		::CloseClipboard();
	}
#endif	// WIN32
}

#if WIN32
#include "vtdata/LevellerTag.h"
static bool VerifyHFclip(BYTE* pMem, size_t size)
{
	// Verify the signature and endianness of received hf clipping.
	if (size < sizeof(daylon::TAG))
		return false;

	// The first two bytes must be 'hf'.
	if (::memcmp(pMem, "hf", 2) != 0)
		return false;

	// THe next two must be an int16 that equals 0x3031.
	unsigned __int16 endian = *((unsigned __int16*)(pMem+2));
	return endian == 0x3031;
}

#endif

//
// Create a new elevation layer by pasting data from the clipboard, using the
//  Daylon Leveller clipboard format for heightfields.
//
void Builder::ElevCopy()
{
#if WIN32
	UINT eFormat = ::RegisterClipboardFormat(_T("daylon_elev"));
	if (eFormat == 0)
	{
		wxMessageBox(_("Can't register clipboard format"));
		return;
	}

	vtElevLayer *pEL = GetActiveElevLayer();
	if (!pEL)
		return;
	vtElevationGrid *grid = pEL->GetGrid();
	if (!grid)
		return;

	int cw, cb;
	grid->GetDimensions(cw, cb);

	/* We're going to make the following tags:

		hf01 : nil

		header
			version: ui32(0)

		body
			heixels
				extents
					width	ui32
					breadth	ui32

				format
					depth		ui32(32)
					fp			ui32(1)

				data		b()

			coordsys
				geometry    ui32    (0=flat, 1=earth)
				projection
					format	ui32	(0=wkt if geometry=earth)
					data	b()		(the proj. string)
				pixelmapping			(raster-to-proj mapping)
					transform				(affine matrix)
						origin
							x	d
							z	d
						scale
							x	d
							z	d
				altitude			(assumes raw elevs are zero-based)
					units	ui32	(EPSG measure code; 9001=m, 9002=ft, 9003=sft, etc.)
					scale	d		(raw-to-units scaling)
					offset	d		(raw-to-units base)
			[
			alpha
				format
					depth	ui32

				data		b()
			]

		  5-7 parent tags  +7 (12-14)
			1 nil tag
		  5-6 ui32 tags		+3 (8-9)
			0 double tags   +6
		  1-2 binary tags   +1 (2-3)

	*/
	size_t nParentTags = 5+7;
	size_t nIntTags = 5+3;
	size_t nDblTags = 0+6;

	// Determine alpha tags needed.
	bool bAlpha = false;
	for (int i = 0; i < cw; i++)
	{
		for (int j = 0; j < cb; j++)
		{
			if (grid->GetFValue(i, j) == INVALID_ELEVATION)
			{
				bAlpha = true;
				break;
			}
		}
	}

	if (bAlpha)
	{
		// There were void pixels.
		nParentTags++;	// "body/alpha"
		nParentTags++;	// "body/alpha/format"
		nIntTags++;		// "body/alpha/format/depth"
	}

	size_t clipSize = 0;

	daylon::CRootTag clip;

	clipSize += clip.CalcNormalStorage(1, daylon::VALKIND_NONE);

	clipSize += clip.CalcNormalStorage(nParentTags, daylon::VALKIND_NONE);
	clipSize += clip.CalcNormalStorage(nIntTags, daylon::VALKIND_UINT32);
	clipSize += clip.CalcNormalStorage(nDblTags, daylon::VALKIND_DOUBLE);
	// HF data.
	clipSize += clip.CalcBinaryTagStorage(cw * cb * sizeof(float));

	// Void data.
	if (bAlpha)
		clipSize += clip.CalcBinaryTagStorage(cw * cb * sizeof(uchar));

	// Projection string.
	char *wkt = NULL;
	m_crs.exportToWkt( &wkt );
	grid->GetCRS().exportToWkt(&wkt);
	vtString wkt_str = wkt;
	CPLFree(wkt);
	clipSize += clip.CalcBinaryTagStorage(wkt_str.GetLength());

	// Allocate.
	HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, clipSize);
	BYTE *pMem = (BYTE*)::GlobalLock(hMem);

	// Write data to clipboard.
	try
	{
		clip.SetStorage(pMem, clipSize);

		clip.Open("w");

		// Write ID and endianness.
		// This works because tag names are at the
		// top of the TAG structure, hence they
		// start the entire memory block.
		unsigned __int16 endian = 0x3031; //'01'
		char szEnd[] = { 'h', 'f', ' ', ' ', 0 };
		::memcpy(szEnd+2, &endian, sizeof(endian));
		clip.Write(szEnd, daylon::TAGRELATION_SIBLING);

		clip.WriteParent("header", true);
		clip.Write("version", (unsigned __int32)0, false);

		clip.WriteParent("body", false);
		clip.WriteParent("heixels", true);
		clip.WriteParent("extents", true);
		clip.Write("width", (daylon::uint32) cw, true);
		clip.Write("breadth", (daylon::uint32) cb, false);

		clip.WriteParent("format", true);
		clip.Write("depth", (unsigned __int32)(sizeof(float)*8), true);
		clip.Write("fp", (unsigned __int32)1, false);
		//clip.Dump();

		float *phv = (float*)
			clip.WriteBinary("data", false,
			cw * cb * sizeof(float));

		// Transfer selected heightfield pixels to clipboard.
		int x, z;
		for(z = 0; z < cb; z++)
		{
			for(x = 0; x < cw; x++)
			{
				*phv = grid->GetFValue(x, cb-1-z);
				phv++;
			}
		}

		// Tack on coordsys tag.
		clip.WriteParent("coordsys", bAlpha);
			clip.Write("geometry", (unsigned __int32)1, true);
			clip.WriteParent("projection", true);
				clip.Write("format", (unsigned __int32)0, true);
				uchar *pstrproj =
					(uchar*)clip.WriteBinary(
						"data", false,
					wkt_str.GetLength());
				::memcpy(pstrproj, (const char *)wkt_str, wkt_str.GetLength());

			DRECT &ext = grid->GetEarthExtents();
			double xp = ext.left;
			double yp = ext.top;
			double xscale = ext.Width() / (cw - 1);
			double zscale = -ext.Height() / (cb - 1);
			clip.WriteParent("pixelmapping", true);
				clip.WriteParent("transform", false);
					clip.WriteParent("origin", true);
						clip.Write("x", xp, true);
						clip.Write("z", yp, false);
					clip.WriteParent("scale", false);
						clip.Write("x", xscale, true);
						clip.Write("z", zscale, false);

			double fElevScale = grid->GetScale();
			double fOffset = 0.0f;
			clip.WriteParent("altitude", false);
				clip.Write("units", (daylon::uint32) 9001, true);
				clip.Write("scale", fElevScale, true);
				clip.Write("offset", fOffset, false);

		// Transfer any mask pixels to clipboard.
		if (bAlpha)
		{
			clip.WriteParent("alpha", false);

			clip.WriteParent("format", true);
			clip.Write("depth", (unsigned __int32)(sizeof(uchar)*8), false);

			uchar *pa = (uchar*)
				clip.WriteBinary("data", false,
					cw * cb * sizeof(uchar));

			for(z = 0; z < cb; z++)
			{
				for(x = 0; x < cw; x++)
				{
					*pa++ = (grid->GetFValue(x, cb-1-z)==INVALID_ELEVATION ? 0 : 255);
				}
			}
		}
	}
	catch(...)
	{
		wxMessageBox(_T("Cannot place data on clipboard"));
	}
	clip.Close();
#if VTDEBUG
	VTLOG1("Copying grid to clipboard: ");
	clip.Dump();
#endif

	::GlobalUnlock(hMem);

	if (::SetClipboardData(eFormat, hMem) == NULL)
	{
		DWORD err = ::GetLastError();
		VTLOG("Cannot put data on clipboard. Error %d", (int)err);
	}
	// Undo our allocation
	::GlobalFree(hMem);

#endif	// WIN32
}

//
// Create a new elevation layer by pasting data from the clipboard, using the
//  Daylone Leveller clipboard format for heightfields.
//
void Builder::ElevPasteNew()
{
#if WIN32
	UINT eFormat = ::RegisterClipboardFormat(_T("daylon_elev"));
	if (eFormat == 0)
	{
		wxMessageBox(_("Can't register clipboard format"));
		return;
	}
	// Get handle to clipboard data.
	HANDLE hMem = ::GetClipboardData(eFormat);
	if (hMem == NULL)
		return;
	void *pMem = (void*)::GlobalLock(hMem);
	if (pMem == NULL)
		return;
	size_t nbytes = ::GlobalSize(hMem);

	daylon::CRootTag clip;

	BYTE *pData = ((BYTE*)pMem) /*+ sizeof(kPublicHFclipID)*/;
	clip.SetStorage(pData, nbytes /*-
				sizeof(kPublicHFclipID)*/);
	if (!VerifyHFclip((BYTE*)pMem, nbytes))
		return;

	clip.Open("r");
	const int width = clip.ReadUINT32("body/heixels/extents/width", 0);
	const int breadth = clip.ReadUINT32("body/heixels/extents/breadth", 0);
	// See what format the elevations are in.
	// Lev 2.5, and DEMEdit support floating-point for now.
	const int bpp = clip.ReadUINT32("body/heixels/format/depth", 0);
	const bool bFP = (0 != clip.ReadUINT32("body/heixels/format/fp", 0));

	// Get any coordsys info. Ignore geometries other
	// than code 1 (planetary body, Earth). Ignore projection
	// formats other than WKT.

	vtCRS crs;
	DRECT area;
	float fElevScale = 1.0;

	const int geomcode =
		clip.ReadUINT32("body/coordsys/geometry", 0);
	const int projFmt =
		clip.ReadUINT32("body/coordsys/projection/format", 0);
	if (geomcode == 1 && projFmt == 0)
	{
		void *pv = NULL;
		size_t n =
			clip.Read("body/coordsys/projection/data", &pv);

		if (n != 0)
		{
			char *psz = new char[n + 1];
			memcpy(psz, pv, n);
			psz[n] = 0;
			//m_georef_info.set_projection(psz);
			char *wkt = psz;
			crs.importFromWkt(&wkt);
			delete psz;
		}

		// Read extents
		double d0 = clip.ReadDouble("body/coordsys/pixelmapping/transform/origin/x", 0.0);
		double d3 = clip.ReadDouble("body/coordsys/pixelmapping/transform/origin/z", 0.0);
		double d1 = clip.ReadDouble("body/coordsys/pixelmapping/transform/scale/x", 1.0);
		double d5 = clip.ReadDouble("body/coordsys/pixelmapping/transform/scale/z", 1.0);
		area.left = d0;
		area.right = d0 + (width-1) * d1;
		area.top = d3;
		area.bottom = d3 + (breadth-1) * d5;

		// Read vertical units and scale
		UINT units = clip.ReadUINT32("body/coordsys/altitude/units", 9001);
		fElevScale = clip.ReadDouble("body/coordsys/altitude/scale", 1.0);
		double offset = clip.ReadDouble("body/coordsys/altitude/offset", 0.0);
		switch (units)
		{
		case 9001: break;	// meter
		case 9002: fElevScale *= 0.3048f; break;	// foot
		case 9003: fElevScale *= (1200.0f/3937.0f); break; // U.S. survey foot
		}
	}

	// Create new layer
	vtElevLayer *pEL = new vtElevLayer(area, IPoint2(width, breadth), bFP, 1.0f, crs);

	// Copy the elevations.
	// Require packed pixel storage.
	float *pElevs;
	size_t n = clip.Read("body/heixels/data", (void**)&pElevs);
	int i = 0;
	for (int z = 0; z < breadth; z++)
	{
		for(int x = 0; x < width; x++, i++)
			pEL->GetGrid()->SetFValue(x, breadth-1-z, pElevs[i] * fElevScale);
	}
	pEL->GetGrid()->ComputeHeightExtents();
	g_bld->AddLayerWithCheck(pEL);

	::GlobalUnlock(hMem);
#endif	// WIN32
}


void MainFrame::DoDymaxTexture()
{
	int face;
	DPoint3 uvw;
	uvw.z = 0.0f;
	double u, v;
	double lat, lon;

	wxFileDialog dlg(this, _("Choose input file"), _T(""), _T(""), _T("*.bmp;*.png"));
	if (dlg.ShowModal() == wxID_CANCEL)
		return;

	wxString choices[6];
	choices[0] = _T("128");
	choices[1] = _T("256");
	choices[2] = _T("512");
	choices[3] = _T("1024");
	choices[4] = _T("2048");
	choices[5] = _T("4096");
	wxSingleChoiceDialog dlg2(this, _("Size of each output tile?"),
		_("Query"), 6, choices);
	if (dlg2.ShowModal() == wxID_CANCEL)
		return;
	int sel = dlg2.GetSelection();
	int output_size = 1 << (7+sel);

	wxTextEntryDialog dlg3(this, _("Prefix for output filenames?"), _("Query"));
	if (dlg3.ShowModal() == wxID_CANCEL)
		return;
	wxString prefix = dlg3.GetValue();

	// ask the user for a directory
	static wxString default_path = wxEmptyString;
	wxDirDialog getDir(this, _("Output Directory"), default_path, wxDD_DEFAULT_STYLE);
	getDir.SetWindowStyle(getDir.GetWindowStyle() | wxDD_NEW_DIR_BUTTON);
	if (getDir.ShowModal() != wxID_OK)
		return;
	wxString strDirName = getDir.GetPath();
	default_path = strDirName;	// save it for next time
	vtString DirName = (const char*)strDirName.mb_str(wxConvUTF8);

	// TODO? change this code to use vtBitmap instead of vtDIB?

	wxProgressDialog prog(_("Processing"), _("Loading source bitmap.."), 100);
	prog.Show(TRUE);

	// read texture
	vtDIB img;
	wxString path = dlg.GetPath();

	OpenProgressDialog(_T("Reading file"), path, false, this);
	bool success = img.Read(path.mb_str(wxConvUTF8), progress_callback);
	CloseProgressDialog();
	if (!success)
	{
		DisplayAndLog("File read failed");
		return;
	}
	int input_x = img.GetWidth();
	int input_y = img.GetHeight();
	int depth = img.GetDepth();

	DymaxIcosa ico;

	uchar value;
	RGBi rgb;
	RGBAi rgba;
	for (int i = 0; i < 10; i++)
	{
		vtDIB out;
		out.Allocate(IPoint2(output_size, output_size), depth);

		wxString msg;
		msg.Printf(_("Creating tile %d ..."), i+1);
		prog.Update((i+1)*10, msg);

		for (int x = 0; x < output_size; x++)
		{
			for (int y = 0; y < output_size; y++)
			{
				if (y < output_size-1-x)
				{
					face = icosa_face_pairs[i][0];
					u = (double)x / output_size;
					v = (double)y / output_size;
				}
				else
				{
					face = icosa_face_pairs[i][1];
					u = (double)(output_size-1-x) / output_size;
					v = (double)(output_size-1-y) / output_size;
				}
				uvw.x = u;
				uvw.y = v;
				ico.FaceUVToGeo(face, uvw, lon, lat);

				int source_x = (int) (lon / PI2d * input_x);
				int source_y = (int) (lat / PId * input_y);

				if (depth == 8)
				{
					value = img.GetPixel8(source_x, source_y);
					out.SetPixel8(x, output_size-1-y, value);
				}
				else if (depth == 24)
				{
					img.GetPixel24(source_x, source_y, rgb);
					out.SetPixel24(x, output_size-1-y, rgb);
				}
				else if (depth == 32)
				{
					img.GetPixel32(source_x, source_y, rgba);
					out.SetPixel32(x, output_size-1-y, rgba);
				}
			}
		}
		vtString name;
		name.Format("%s_%02d%02d.png", (const char *) prefix.mb_str(wxConvUTF8),
			icosa_face_pairs[i][0]+1, icosa_face_pairs[i][1]+1);
		success = out.WritePNG(DirName + "/" + name);
		if (!success)
		{
			DisplayAndLog("Failed to write file %s.", (const char *) name);
			return;
		}
	}

	DisplayAndLog("Successful.");
}

void MainFrame::DoDymaxMap()
{
	VTLOG1("DoDymaxMap\n");

	DPoint3 uvw;
	uvw.z = 0.0f;

	wxString filter = _("Image Formats|");
	AddType(filter, FSTRING_BMP);
	AddType(filter, FSTRING_PNG);
	AddType(filter, FSTRING_JPEG);

	wxFileDialog dlg(this, _("Choose input file"), _T(""), _T(""), filter);
	if (dlg.ShowModal() == wxID_CANCEL)
		return;
	wxString path = dlg.GetPath();

	wxFileDialog dlg2(this, _("Choose output file"), _T(""), _T(""), _T("*.png"), wxFD_SAVE);
	if (dlg2.ShowModal() == wxID_CANCEL)
		return;

	int style = wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_CAN_ABORT | wxPD_SMOOTH;
	wxString title = _("Processing");
	title += _T(" ");
	title += path;
	wxString message = _("Loading");
	message += _T(" ");
	message += path;
	wxProgressDialog prog(title, message, 100, this, style);
	prog.Show(TRUE);

	// read texture
	vtDIB img;
	VTLOG("Reading Input: %s\n", (const char *) path.mb_str());

	OpenProgressDialog(_T("Reading file"), path, false, this);
	bool success = img.Read(path.mb_str(wxConvUTF8), progress_callback);
	CloseProgressDialog();
	if (!success)
	{
		DisplayAndLog("File read failed");
		return;
	}
	int input_x = img.GetWidth();
	int input_y = img.GetHeight();
	int depth = img.GetDepth();
	VTLOG("input: size %d %d, depth %d\n", input_x, input_y, depth);

	int output_x = (input_x * 0.85);	// reduce slightly, to avoid gaps on forward projection
	int output_y = (output_x / 5.5 * 2.6);

	SizeDlg dlg3(this, -1, _("Output size"));
	dlg3.SetBase(IPoint2(output_x/4, output_y/4));
	dlg3.SetRatioRange(1.0f, 4.0f);
	dlg3.GetTextCtrl()->SetValue(_T("The size of the rendered image should be, at most, slightly less than the input."));
	if (dlg3.ShowModal() != wxID_OK)
		return;

	output_x = dlg3.m_Current.x;
    output_y = dlg3.m_Current.y;

	// Make output
	vtDIB out;
	VTLOG("output: size %d %d\n", output_x, output_y);
	if (!out.Allocate(IPoint2(output_x, output_y), depth))
	{
		vtString msg;
		msg.Format("Could not allocate DIB of size %d x %d (%.1f MB)", output_x, output_y,
			(float) output_x * output_y * depth / 8 / 1024.0f / 1024.0f);
		DisplayAndLog(msg);
		return;
	}

	DymaxIcosa ico;

	DPoint2 p;
	DPoint2 dmp;
	uchar value;
	RGBi rgb;
	RGBAi rgba;
	wxString msg;

	prog.Update(1, _T(""));

	for (int x = 0; x < input_x; x++)
	{
		msg.Printf(_T("Projecting %d/%d"), x, input_x);
		if (prog.Update(x * 99 / input_x, msg) == false)
		{
			// cancelled
			DisplayAndLog("Cancelled.");
			return;
		}

		p.x = (((double)x / (input_x-1)) * 360.0) - 180.0;
		for (int y = 0; y < input_y; y++)
		{
			p.y = -((((double)y / (input_y-1)) * 180.0) - 90.0);

			ico.GeoToDymax(p, dmp);

			int target_x = (int) (dmp.x * output_x / 5.5);
			int target_y = (int) (dmp.y * output_y / 2.6);

			if (target_x < 0 || target_x >= output_x)
				continue;
			if (target_y < 0 || target_y >= output_y)
				continue;

			switch (depth)
			{
			case 8:
				value = img.GetPixel8(x, y);
				out.SetPixel8(target_x, output_y-1-target_y, value);
				break;
			case 24:
				img.GetPixel24(x, y, rgb);
				out.SetPixel24(target_x, output_y-1-target_y, rgb);
				break;
			case 32:
				img.GetPixel32(x, y, rgba);
				out.SetPixel32(target_x, output_y-1-target_y, rgba);
				break;
			}
		}
	}
	wxString path2 = dlg2.GetPath();
	msg = _("Saving to ");
	msg += path2;
	prog.Update(99, msg);

	vtString fname = (const char *) path2.mb_str(wxConvUTF8);
	success = out.WritePNG(fname);
	if (!success)
	{
		DisplayAndLog("Failed to write file %s.", (const char *) fname);
		return;
	}

	DisplayAndLog("Successful.");
}

