#pragma once



// CvtocxPropNavigation : Property page dialog

class CvtocxPropNavigation : public COlePropertyPage
{
	DECLARE_DYNCREATE(CvtocxPropNavigation)
	DECLARE_OLECREATE_EX(CvtocxPropNavigation)

// Constructors
public:
	CvtocxPropNavigation();

// Dialog Data
	enum { IDD = IDD_PROPNAVIGATION_VTOCX };

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support

// Message maps
protected:
	DECLARE_MESSAGE_MAP()
public:
	float m_Velocity;
	float m_Rotation;
	float m_Pan;
};
