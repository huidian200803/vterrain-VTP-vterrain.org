//
// Name: ProfileDlg.h
//
// Copyright (c) 2004-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __ProfileDlg_H__
#define __ProfileDlg_H__

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "vtui_UI.h"
#include "AutoDialog.h"
#include "vtdata/HeightField.h"
#include "vtdata/vtCRS.h"

class ProfileCallback
{
public:
	virtual void Begin() {}
	virtual float GetElevation(const DPoint2 &p) = 0;
	virtual float GetCultureHeight(const DPoint2 &p) = 0;
	virtual bool HasCulture() { return false; }
};

// WDR: class declarations

//----------------------------------------------------------------------------
// ProfileDlg
//----------------------------------------------------------------------------

class ProfileDlg: public ProfileDlgBase
{
public:
	// constructors and destructors
	ProfileDlg( wxWindow *parent, wxWindowID id,
		const wxString& title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for ProfileDlg
	wxTextCtrl* GetRF()  { return (wxTextCtrl*) FindWindow( ID_RF ); }
	wxChoice* GetCurvature()  { return (wxChoice*) FindWindow( ID_CURVATURE ); }
	wxCheckBox* GetFresnel()  { return (wxCheckBox*) FindWindow( ID_FRESNEL ); }
	wxCheckBox* GetEffective()  { return (wxCheckBox*) FindWindow( ID_USE_EFFECTIVE ); }
	wxTextCtrl* GetHeight2()  { return (wxTextCtrl*) FindWindow( ID_HEIGHT2 ); }
	wxTextCtrl* GetHeight1()  { return (wxTextCtrl*) FindWindow( ID_HEIGHT1 ); }
	wxTextCtrl* GetText()  { return (wxTextCtrl*) FindWindow( ID_STATUS_TEXT ); }
	wxCheckBox* GetLineOfSight()  { return (wxCheckBox*) FindWindow( ID_LINE_OF_SIGHT ); }
	wxCheckBox* GetVisibility()  { return (wxCheckBox*) FindWindow( ID_VISIBILITY ); }
	wxButton* GetShowCulture()  { return (wxButton*) FindWindow( ID_SHOW_CULTURE ); }

	void MakePoint(wxPoint &p, int i, float value);
	void DrawChart(wxDC& dc);
	void UpdateMessageText();
	void UpdateEnabling();

	void SetCRS(const vtCRS &crs);
	void SetPath(const DLine2 &path);
	void SetPoints(const DPoint2 &p1, const DPoint2 &p2);
	void SetCallback(ProfileCallback *callback);

	void GetValues();
	void Analyze();
	void ComputeLineOfSight();
	void ComputeVisibility();
	float ComputeFresnelRadius(float dist, float freq, int zone);
	void ComputeFirstFresnel();
	void ComputeGeoidSurface();
	float ApplyGeoid(float h, int i, char t);
	void ComputeSignalLoss(float dist, float freq);
	float TotalHeightAt(int i);

	void WriteProfileToDXF(const char *filename);
	void WriteTraceToDXF(const char *filename);
	void WriteProfileToCSV(const char *filename);

private:
	// WDR: member variable declarations for ProfileDlg
	ProfileCallback *m_callback;
	std::vector<float> m_values;
	std::vector<float> m_values_culture;
	std::vector<bool> m_visible;
	vtCRS    m_crs;
	DPoint2 m_p1, m_p2;
	DLine2 m_path;
	wxSize m_clientsize;
	int m_xrange, m_yrange;
	float m_fMin, m_fMax;
	float m_fMinDist, m_fMaxDist, m_fTotalDist;
	float m_fDrawMin, m_fDrawMax, m_fDrawRange;
	bool m_bHavePoints, m_bHavePath, m_bHaveValues;
	bool m_bHaveValidData, m_bHaveInvalid;
	bool m_bMouseOnLine;
	float m_fMouse, m_fMouseDist;
	int m_iMin, m_iMax, m_iMouse;
	wxPoint m_base;
	bool m_bHaveSlope;
	float m_fSlope;
	bool m_bLeftButton;
	bool m_bValidStart, m_bValidLine;
	float m_fHeightAtStart, m_fHeightAtEnd;
	bool m_bIntersectsGround;
	bool m_bIntersectsCulture;
	float m_fIntersectHeight;
	float m_fIntersectDistance;
	int m_iIntersectIndex;

	float m_fGeodesicDistance;
	bool  m_bHaveFresnel;
	bool  m_bHaveLOS;
	bool  m_bHaveGeoidSurface;
	std::vector<float> m_FirstFresnel;
	std::vector<float> m_LineOfSight;
	std::vector<float> m_GeoidSurface;
	std::vector<bool> m_rvisible;
	float m_fGeoidCurvature;
	float m_fMouseFresnel;
	float m_fMouseLOS;
	std::vector<double> m_fGeoDistAtPoint;

	// these values are exposed directly in the GUI
	float m_fHeight1, m_fHeight2;
	float m_fRadioFrequency;
	int m_iCurvature;	// 0=none, 1=terrain, 2=line of sight
	bool m_bGetCulture;
	bool m_bHaveCulture;

	// these values are retreived as needed from the GUI
	bool m_bLineOfSight, m_bVisibility;
	bool m_bUseFresnel, m_bUseEffectiveRadius;

	DPoint2 m_DrawOrg, m_DrawScale;
	void MakePoint(FPoint2 &p, int i, float value);
	void MakePoint(const DPoint2 &p_in, DPoint2 &p_out);
	void DrawProfileToDXF(FILE *fp);
	void DrawTraceToDXF(FILE *fp);

private:
	// WDR: handler declarations for ProfileDlg
	void OnExportCSV( wxCommandEvent &event );
	void OnExportTrace( wxCommandEvent &event );
	void OnExportDXF( wxCommandEvent &event );
	void OnCurvature( wxCommandEvent &event );
	void OnRF( wxCommandEvent &event );
	void OnUseEffective( wxCommandEvent &event );
	void OnFresnel( wxCommandEvent &event );
	void OnHeight2( wxCommandEvent &event );
	void OnHeight1( wxCommandEvent &event );
	void OnLineOfSight( wxCommandEvent &event );
	void OnShowCulture( wxCommandEvent &event );
	void OnVisibility( wxCommandEvent &event );
	void OnPaint(wxPaintEvent &event);
	void OnDraw(wxDC& dc); // overridden to draw this view
	void OnSize(wxSizeEvent& event);
	void OnLeftDown(wxMouseEvent& event);
	void OnLeftUp(wxMouseEvent& event);
	void OnMouseMove(wxMouseEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __ProfileDlg_H__

