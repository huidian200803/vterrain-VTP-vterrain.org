//
// Implements the following classes:
//
// AutoDialog - An improvement to wxDialog which makes validation easier.
//
// AutoPanel - An improvement to wxPanel which makes validation easier.
//
// wxNumericValidator - A validator capable of transfering numeric values.
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtdata/vtString.h"
#include "AutoDialog.h"

/////////////////////////////////////////////////
//

wxNumericValidator::wxNumericValidator(short *val) : wxValidator()
{
	Initialize();
	m_pValShort = val;
}

wxNumericValidator::wxNumericValidator(int *val) : wxValidator()
{
	Initialize();
	m_pValInt = val;
}

wxNumericValidator::wxNumericValidator(float *val, int digits) : wxValidator()
{
	Initialize();
	m_pValFloat = val;
	m_iDigits = digits;
}

wxNumericValidator::wxNumericValidator(double *val, int digits) : wxValidator()
{
	Initialize();
	m_pValDouble = val;
	m_iDigits = digits;
}

wxNumericValidator::wxNumericValidator(const wxNumericValidator& val)
{
	Initialize();
	Copy(val);
}

/*
  Called by constructors to initialize ALL data members
*/
void wxNumericValidator::Initialize()
{
	m_pValShort = NULL;
	m_pValInt = NULL;
	m_pValFloat = NULL;
	m_pValDouble = NULL;
	m_iDigits = 0;
	m_bEnabled = true;
}

bool wxNumericValidator::Copy(const wxNumericValidator& val)
{
	wxValidator::Copy(val);

	m_pValShort = val.m_pValShort;
	m_pValInt = val.m_pValInt;
	m_pValFloat = val.m_pValFloat;
	m_pValDouble = val.m_pValDouble;
	m_iDigits = val.m_iDigits;

	return true;
}

// Called to transfer data to the window
bool wxNumericValidator::TransferToWindow()
{
	if ( !m_validatorWindow )
		return false;

	if ( !m_bEnabled )
		return true;

	// Compare existing string to new string; don't update unless the actual
	//  numeric value has changed.
	wxString OldString;
	if (m_validatorWindow->IsKindOf(CLASSINFO(wxStaticText)) )
	{
		wxStaticText* pControl = (wxStaticText*) m_validatorWindow;
		OldString = pControl->GetLabel();
	}
	else if (m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)) )
	{
		wxTextCtrl* pControl = (wxTextCtrl*) m_validatorWindow;
		OldString = pControl->GetValue();
	}
	else
		return false;

	wxString str, format;
	long OldIntValue;
	double OldDoubleValue;
	if (m_pValShort)
	{
		if (OldString != _T(""))
		{
			OldString.ToLong(&OldIntValue);
			if (OldIntValue == (long)*m_pValShort)
				return true;
		}

		str.Printf(_T("%d"), *m_pValShort);
	}
	else if (m_pValInt)
	{
		if (OldString != _T(""))
		{
			OldString.ToLong(&OldIntValue);
			if (OldIntValue == (long)*m_pValInt)
				return true;
		}

		str.Printf(_T("%d"), *m_pValInt);
	}
	else if (m_pValFloat)
	{
		if (OldString != _T(""))
		{
			OldString.ToDouble(&OldDoubleValue);
			if (OldDoubleValue == (double)*m_pValFloat)
				return true;
		}

		if (m_iDigits != -1)
		{
			format.Printf(_T("%%.%df"), m_iDigits);
			str.Printf(format, *m_pValFloat);
		}
		else
			str.Printf(_T("%.8g"), *m_pValFloat);	// 8 significant digits
	}
	else if (m_pValDouble)
	{
		if (OldString != _T(""))
		{
			OldString.ToDouble(&OldDoubleValue);
			if (OldDoubleValue == *m_pValDouble)
				return true;
		}

		if (m_iDigits != -1)
		{
			format.Printf(_T("%%.%dlf"), m_iDigits);
			str.Printf(format, *m_pValDouble);
		}
		else
			str.Printf(_T("%.16lg"), *m_pValDouble);	// 16 significant digits
	}

	if (m_validatorWindow->IsKindOf(CLASSINFO(wxStaticText)) )
	{
		wxStaticText* pControl = (wxStaticText*) m_validatorWindow;
		if (pControl)
		{
			pControl->SetLabel(str);
			return true;
		}
	}
	else
	if (m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)) )
	{
		wxTextCtrl* pControl = (wxTextCtrl*) m_validatorWindow;
		if (pControl)
		{
			pControl->SetValue(str);
			return true;
		}
	}

	// unrecognized control, or bad pointer
	return false;
}

// Called to transfer data from the window
bool wxNumericValidator::TransferFromWindow()
{
	if ( !m_validatorWindow )
		return false;

	if ( !m_bEnabled )
		return true;

	// string controls
	wxString str;
	if (m_validatorWindow->IsKindOf(CLASSINFO(wxStaticText)) )
	{
		wxStaticText* pControl = (wxStaticText*) m_validatorWindow;
		if (pControl)
			str = pControl->GetLabel();
	}
	else if (m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)) )
	{
		wxTextCtrl* pControl = (wxTextCtrl*) m_validatorWindow;
		if (pControl)
			str = pControl->GetValue();
	}
	else // unrecognized control, or bad pointer
		return false;

	if (str != _T(""))
	{
		vtString ccs = (const char *) str.mb_str(*wxConvCurrent);
		if (m_pValShort)
			sscanf(ccs, "%hd", m_pValShort);
		if (m_pValInt)
			sscanf(ccs, "%d", m_pValInt);
		if (m_pValFloat)
			sscanf(ccs, "%f", m_pValFloat);
		if (m_pValDouble)
			sscanf(ccs, "%lf", m_pValDouble);
		return true;
	}
	return false;
}

/////////////////////////////////////////////////
//

void AutoDialog::AddValidator(long id, wxString *sptr)
{
	wxWindow *pWin = FindWindow(id);
	if (!pWin) return;
	pWin->SetValidator(wxGenericValidator(sptr));	// actually clones the one we pass in
}

void AutoDialog::AddValidator(long id, bool *bptr)
{
	wxWindow *pWin = FindWindow(id);
	if (!pWin) return;
	pWin->SetValidator(wxGenericValidator(bptr));	// actually clones the one we pass in
}

void AutoDialog::AddValidator(long id, int *iptr)
{
	wxWindow *pWin = FindWindow(id);
	if (!pWin) return;
	pWin->SetValidator(wxGenericValidator(iptr));	// actually clones the one we pass in
}

wxNumericValidator *AutoDialog::AddNumValidator(long id, short *sptr)
{
	wxWindow *pWin = FindWindow(id);
	if (!pWin) return NULL;

	// actually clones the one we pass in
	pWin->SetValidator(wxNumericValidator(sptr));
	return (wxNumericValidator*) pWin->GetValidator();
}

wxNumericValidator *AutoDialog::AddNumValidator(long id, int *iptr)
{
	wxWindow *pWin = FindWindow(id);
	if (!pWin) return NULL;

	// actually clones the one we pass in
	pWin->SetValidator(wxNumericValidator(iptr));
	return (wxNumericValidator*) pWin->GetValidator();
}

wxNumericValidator *AutoDialog::AddNumValidator(long id, float *fptr, int digits)
{
	wxWindow *pWin = FindWindow(id);
	if (!pWin) return NULL;

	// actually clones the one we pass in
	pWin->SetValidator(wxNumericValidator(fptr, digits));
	return (wxNumericValidator*) pWin->GetValidator();
}

wxNumericValidator *AutoDialog::AddNumValidator(long id, double *dptr, int digits)
{
	wxWindow *pWin = FindWindow(id);
	if (!pWin) return NULL;

	// actually clones the one we pass in
	pWin->SetValidator(wxNumericValidator(dptr, digits));
	return (wxNumericValidator*) pWin->GetValidator();
}


/////////////////////////////////////////////////
//

void AutoPanel::AddValidator(long id, wxString *sptr)
{
	wxWindow *pWin = FindWindow(id);
	if (!pWin) return;
	pWin->SetValidator(wxGenericValidator(sptr));	// actually clones the one we pass in
}

void AutoPanel::AddValidator(long id, bool *bptr)
{
	wxWindow *pWin = FindWindow(id);
	if (!pWin) return;
	pWin->SetValidator(wxGenericValidator(bptr));	// actually clones the one we pass in
}

void AutoPanel::AddValidator(long id, int *iptr)
{
	wxWindow *pWin = FindWindow(id);
	if (!pWin) return;
	pWin->SetValidator(wxGenericValidator(iptr));	// actually clones the one we pass in
}

wxNumericValidator *AutoPanel::AddNumValidator(long id, int *iptr)
{
	wxWindow *pWin = FindWindow(id);
	if (!pWin) return NULL;

	// actually clones the one we pass in
	pWin->SetValidator(wxNumericValidator(iptr));
	return (wxNumericValidator *) pWin->GetValidator();
}

wxNumericValidator *AutoPanel::AddNumValidator(long id, float *fptr, int digits)
{
	wxWindow *pWin = FindWindow(id);
	if (!pWin) return NULL;

	// actually clones the one we pass in
	pWin->SetValidator(wxNumericValidator(fptr, digits));
	return (wxNumericValidator *) pWin->GetValidator();
}

wxNumericValidator *AutoPanel::AddNumValidator(long id, double *dptr, int digits)
{
	wxWindow *pWin = FindWindow(id);
	if (!pWin) return NULL;

	pWin->SetValidator(wxNumericValidator(dptr, digits));
	// actually clones the one we pass in
	return (wxNumericValidator *) pWin->GetValidator();
}


/////////////////////////////////////////////////
//

// And forms of the methods which don't require subclassing from AutoDialog or AutoPanel
void AddValidator(wxWindow *parent, long id, wxString *sptr)
{
	wxWindow *pWin = parent->FindWindow(id);
	if (!pWin) return;
	pWin->SetValidator(wxGenericValidator(sptr));	// actually clones the one we pass in
}

void AddValidator(wxWindow *parent, long id, bool *bptr)
{
	wxWindow *pWin = parent->FindWindow(id);
	if (!pWin) return;
	pWin->SetValidator(wxGenericValidator(bptr));	// actually clones the one we pass in
}

void AddValidator(wxWindow *parent, long id, int *iptr)
{
	wxWindow *pWin = parent->FindWindow(id);
	if (!pWin) return;
	pWin->SetValidator(wxGenericValidator(iptr));	// actually clones the one we pass in
}

wxNumericValidator *AddNumValidator(wxWindow *parent, long id, short *sptr)
{
	wxWindow *pWin = parent->FindWindow(id);
	if (!pWin) return NULL;

	// actually clones the one we pass in
	pWin->SetValidator(wxNumericValidator(sptr));
	return (wxNumericValidator*) pWin->GetValidator();
}

wxNumericValidator *AddNumValidator(wxWindow *parent, long id, int *iptr)
{
	wxWindow *pWin = parent->FindWindow(id);
	if (!pWin) return NULL;

	// actually clones the one we pass in
	pWin->SetValidator(wxNumericValidator(iptr));
	return (wxNumericValidator*) pWin->GetValidator();
}

wxNumericValidator *AddNumValidator(wxWindow *parent, long id, float *fptr, int digits)
{
	wxWindow *pWin = parent->FindWindow(id);
	if (!pWin) return NULL;

	// actually clones the one we pass in
	pWin->SetValidator(wxNumericValidator(fptr, digits));
	return (wxNumericValidator*) pWin->GetValidator();
}

wxNumericValidator *AddNumValidator(wxWindow *parent, long id, double *dptr, int digits)
{
	wxWindow *pWin = parent->FindWindow(id);
	if (!pWin) return NULL;

	// actually clones the one we pass in
	pWin->SetValidator(wxNumericValidator(dptr, digits));
	return (wxNumericValidator*) pWin->GetValidator();
}


/////////////////////////////////////////////////
//

// And forms of the methods which take a pointer directly
void AddValidator(wxWindow *win, wxString *sptr)
{
	win->SetValidator(wxGenericValidator(sptr));	// actually clones the one we pass in
}

void AddValidator(wxWindow *win, bool *bptr)
{
	win->SetValidator(wxGenericValidator(bptr));	// actually clones the one we pass in
}

void AddValidator(wxWindow *win, int *iptr)
{
	win->SetValidator(wxGenericValidator(iptr));	// actually clones the one we pass in
}

wxNumericValidator *AddNumValidator(wxWindow *win, short *sptr)
{
	// actually clones the one we pass in
	win->SetValidator(wxNumericValidator(sptr));
	return (wxNumericValidator*) win->GetValidator();
}

wxNumericValidator *AddNumValidator(wxWindow *win, int *iptr)
{
	// actually clones the one we pass in
	win->SetValidator(wxNumericValidator(iptr));
	return (wxNumericValidator*) win->GetValidator();
}

wxNumericValidator *AddNumValidator(wxWindow *win, float *fptr, int digits)
{
	// actually clones the one we pass in
	win->SetValidator(wxNumericValidator(fptr, digits));
	return (wxNumericValidator*) win->GetValidator();
}

wxNumericValidator *AddNumValidator(wxWindow *win, double *dptr, int digits)
{
	// actually clones the one we pass in
	win->SetValidator(wxNumericValidator(dptr, digits));
	return (wxNumericValidator*) win->GetValidator();
}


