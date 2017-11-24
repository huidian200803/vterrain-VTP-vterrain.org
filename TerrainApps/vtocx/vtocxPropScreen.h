#pragma once



// CvtocxPropScreen : Property page dialog

class CvtocxPropScreen : public COlePropertyPage
{
	DECLARE_DYNCREATE(CvtocxPropScreen)
	DECLARE_OLECREATE_EX(CvtocxPropScreen)

// Constructors
public:
	CvtocxPropScreen();

// Dialog Data
	enum { IDD = IDD_VTOCXPROPSCREEN };

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support

// Message maps
protected:
	DECLARE_MESSAGE_MAP()
};
