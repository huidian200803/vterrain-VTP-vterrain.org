//
// Name: TileDlg.cpp
//
// Copyright (c) 2005-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/FileFilters.h"	// for FSTRING_INI
#include "vtdata/vtLog.h"
#include "vtui/AutoDialog.h"

#include "TileDlg.h"
#include "BuilderView.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// TileDlg
//----------------------------------------------------------------------------

// WDR: event table for TileDlg

BEGIN_EVENT_TABLE(TileDlg,TileDlgBase)
	EVT_BUTTON( ID_DOTDOTDOT, TileDlg::OnDotDotDot )
	EVT_TEXT( ID_COLUMNS, TileDlg::OnSize )
	EVT_TEXT( ID_ROWS, TileDlg::OnSize )
	EVT_TEXT( ID_TEXT_TO_FOLDER, TileDlg::OnFilename )
	EVT_CHOICE( ID_CHOICE_LOD0_SIZE, TileDlg::OnLODSize )
END_EVENT_TABLE()

TileDlg::TileDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	TileDlgBase( parent, id, title, position, size, style )
{
	m_fEstX = -1;
	m_fEstY = -1;
	m_iColumns = 1;
	m_iRows = 1;

	m_bSetting = false;
	m_pView = NULL;
	m_bElev = false;

	m_bCompressNone = true;
	m_bCompressOGL = false;
	m_bCompressSquishFast = false;
	m_bCompressSquishSlow = false;
	m_bCompressJPEG = false;

	AddValidator(this, ID_TEXT_TO_FOLDER, &m_strToFile);
	AddNumValidator(this, ID_COLUMNS, &m_iColumns);
	AddNumValidator(this, ID_ROWS, &m_iRows);
	AddValidator(this, ID_CHOICE_LOD0_SIZE, &m_iLODChoice);
	AddValidator(this, ID_SPIN_NUM_LODS, &m_iNumLODs);

	// informations
	AddNumValidator(this, ID_TOTALX, &m_iTotalX);
	AddNumValidator(this, ID_TOTALY, &m_iTotalY);

	AddNumValidator(this, ID_AREAX, &m_fAreaX);
	AddNumValidator(this, ID_AREAY, &m_fAreaY);

	AddNumValidator(this, ID_ESTX, &m_fEstX);
	AddNumValidator(this, ID_ESTY, &m_fEstY);

	AddNumValidator(this, ID_CURX, &m_fCurX);
	AddNumValidator(this, ID_CURY, &m_fCurY);

	AddValidator(this, ID_OMIT_FLAT, &m_bOmitFlatTiles);
	AddValidator(this, ID_MASK_UNKNOWN, &m_bMaskUnknown);
	AddValidator(this, ID_TEXTURE_ALPHA, &m_bImageAlpha);

	AddValidator(this, ID_TC_NONE, &m_bCompressNone);
	AddValidator(this, ID_TC_OGL, &m_bCompressOGL);
	AddValidator(this, ID_TC_SQUISH_FAST, &m_bCompressSquishFast);
	AddValidator(this, ID_TC_SQUISH_SLOW, &m_bCompressSquishSlow);
	AddValidator(this, ID_TC_JPEG, &m_bCompressJPEG);

	UpdateEnables();

	TransferDataToWindow();

	GetSizer()->SetSizeHints(this);
}

void TileDlg::SetElevation(bool bElev)
{
	// Elevation is handled as grid corners, not center, so grid size are differnt
	m_bElev = bElev;

	GetChoiceLod0Size()->Clear();
	if (m_bElev)
	{
		GetChoiceLod0Size()->Append(_T("32 + 1"));
		GetChoiceLod0Size()->Append(_T("64 + 1"));
		GetChoiceLod0Size()->Append(_T("128 + 1"));
		GetChoiceLod0Size()->Append(_T("256 + 1"));
		GetChoiceLod0Size()->Append(_T("512 + 1"));
		GetChoiceLod0Size()->Append(_T("1024 + 1"));
		GetChoiceLod0Size()->Append(_T("2048 + 1"));
		GetChoiceLod0Size()->Append(_T("4096 + 1"));
	}
	else
	{
		GetChoiceLod0Size()->Append(_T("32"));
		GetChoiceLod0Size()->Append(_T("64"));
		GetChoiceLod0Size()->Append(_T("128"));
		GetChoiceLod0Size()->Append(_T("256"));
		GetChoiceLod0Size()->Append(_T("512"));
		GetChoiceLod0Size()->Append(_T("1024"));
		GetChoiceLod0Size()->Append(_T("2048"));
		GetChoiceLod0Size()->Append(_T("4096"));
	}
}

void TileDlg::SetTilingOptions(TilingOptions &opt)
{
	m_strToFile = wxString(opt.fname, wxConvUTF8);
	m_iColumns = opt.cols;
	m_iRows = opt.rows;
	m_iLOD0Size = opt.lod0size;
	m_iNumLODs = opt.numlods;
	m_bOmitFlatTiles = opt.bOmitFlatTiles;
	m_bMaskUnknown = opt.bMaskUnknownAreas;
	m_bImageAlpha = opt.bImageAlpha;

	m_bCompressNone = !opt.bUseTextureCompression;
	if (opt.bUseTextureCompression)
	{
		m_bCompressOGL = (opt.eCompressionType == TC_OPENGL);
		m_bCompressSquishFast = (opt.eCompressionType == TC_SQUISH_FAST);
		m_bCompressSquishSlow = (opt.eCompressionType == TC_SQUISH_SLOW);
		m_bCompressJPEG = (opt.eCompressionType == TC_JPEG);
	}

	m_iLODChoice = vt_log2(m_iLOD0Size)-5;

	UpdateInfo();
}

void TileDlg::GetTilingOptions(TilingOptions &opt) const
{
	opt.cols = m_iColumns;
	opt.rows = m_iRows;
	opt.lod0size = m_iLOD0Size;
	opt.numlods = m_iNumLODs;
	opt.fname = m_strToFile.mb_str(wxConvUTF8);
	opt.bOmitFlatTiles = m_bOmitFlatTiles;
	opt.bMaskUnknownAreas = m_bMaskUnknown;
	opt.bImageAlpha = m_bImageAlpha;

	opt.bUseTextureCompression = !m_bCompressNone;
	if (m_bCompressOGL) opt.eCompressionType = TC_OPENGL;
	if (m_bCompressSquishFast) opt.eCompressionType = TC_SQUISH_FAST;
	if (m_bCompressSquishSlow) opt.eCompressionType = TC_SQUISH_SLOW;
	if (m_bCompressJPEG) opt.eCompressionType = TC_JPEG;
}

void TileDlg::SetArea(const DRECT &area)
{
	m_area = area;

	UpdateInfo();
}

void TileDlg::UpdateInfo()
{
	m_iTotalX = m_iLOD0Size * m_iColumns;
	m_iTotalY = m_iLOD0Size * m_iRows;
	if (m_bElev)
	{
		// Elevation is handled as grid corners, imagery is handled as
		//  centers, so grid sizes are differnt
		m_iTotalX ++;
		m_iTotalY ++;
	}

	m_fAreaX = m_area.Width();
	m_fAreaY = m_area.Height();

	if (m_bElev)
	{
		m_fCurX = m_fAreaX / (m_iTotalX - 1);
		m_fCurY = m_fAreaY / (m_iTotalY - 1);
	}
	else
	{
		m_fCurX = m_fAreaX / m_iTotalX;
		m_fCurY = m_fAreaY / m_iTotalY;
	}

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void TileDlg::UpdateEnables()
{
	FindWindow(wxID_OK)->Enable(m_strToFile != _T(""));

	FindWindow(ID_OMIT_FLAT)->Enable(m_bElev);
	FindWindow(ID_MASK_UNKNOWN)->Enable(m_bElev);

	// We actually need to leave the compression options enabled even if we're
	//  using this dialog for elevation, because they might want to write
	//  derived textures separately, and this is the only place they can
	//  indicate whether to compress.
	FindWindow(ID_TC_NONE)->Enable(true);

#if USE_OPENGL
	FindWindow(ID_TC_OGL)->Enable(true);
#else
	FindWindow(ID_TC_OGL)->Enable(false);
#endif

#if SUPPORT_SQUISH
	FindWindow(ID_TC_SQUISH_FAST)->Enable(true);
	FindWindow(ID_TC_SQUISH_SLOW)->Enable(true);
#else
	FindWindow(ID_TC_SQUISH_FAST)->Enable(false);
	FindWindow(ID_TC_SQUISH_SLOW)->Enable(false);
#endif
}

// WDR: handler implementations for TileDlg

void TileDlg::OnFilename( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	UpdateEnables();
}

void TileDlg::OnLODSize( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	m_iLOD0Size = 1 << (m_iLODChoice + 5);
	UpdateInfo();
}

void TileDlg::OnSize( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	UpdateInfo();

	if (m_pView)
		m_pView->SetGridMarks(m_area, m_iColumns, m_iRows, -1, -1);
}

void TileDlg::OnDotDotDot( wxCommandEvent &event )
{
	// ask the user for a filename
	wxString filter;
	filter += FSTRING_INI;
	wxFileDialog saveFile(NULL, _(".Ini file"), _T(""), _T(""), filter, wxFD_SAVE);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	// update controls
	m_strToFile = saveFile.GetPath();

	TransferDataToWindow();
	UpdateEnables();
}

