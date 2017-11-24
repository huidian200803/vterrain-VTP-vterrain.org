//
// DoubleProgressDialog class
//
// Copyright (c) 2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/defs.h"
#include "wx/progdlg.h"
#include "wx/dialog.h"

class DoubleProgressDialog : public wxDialog
{
	DECLARE_DYNAMIC_CLASS(DoubleProgressDialog)
public:
	/* Creates and displays dialog, disables event handling for other
	frames or parent frame to avoid recursion problems.
		@param title title for window
		@param message message to display in window
		@param maximum value for status bar, if <= 0, no bar is shown
		@param parent window or NULL
		@param style is the bit mask of wxPD_XXX constants from wx/defs.h
	*/
	DoubleProgressDialog(const wxString &title, wxString const &message,
		wxWindow *parent = NULL,
		int style = wxPD_APP_MODAL | wxPD_AUTO_HIDE);
	/* Destructor.
	Re-enables event handling for other windows.
	*/
	virtual ~DoubleProgressDialog();

	/* Update the status bar to the new value.
		@param value new value
		@param newmsg if used, new message to display
		@returns true if ABORT button has not been pressed
	*/
	virtual bool Update(int value1, int value2, const wxString& newmsg = wxEmptyString);

	// Must provide overload to avoid hiding it (and warnings about it)
	virtual void Update() { wxDialog::Update(); }

	/* Can be called to continue after the cancel button has been pressed, but
	   the program decided to continue the operation (e.g., user didn't
	   confirm it) */
	void Resume();

	virtual bool Show( bool show = true );

protected:
	// callback for optional abort button
	void OnCancel(wxCommandEvent& event);

	// callback to disable "hard" window closing
	void OnClose(wxCloseEvent& event);

	// must be called to reenable the other windows temporarily disabled while
	// the dialog was shown
	void ReenableOtherWindows();

private:
	// create the label with given text and another one to show the time nearby
	// as the next windows in the sizer, returns the created control
	wxStaticText *CreateLabel(const wxString& text, wxSizer *sizer);

	// updates the label message
	void UpdateMessage(const wxString &newmsg);

	// part of Update() returns true if not cancelled
	bool DoAfterUpdate();

	// shortcuts for enabling buttons
	void EnableClose();
	void EnableAbort(bool enable=true);
	inline void DisableAbort() { EnableAbort(false); }

	// Yield to other parts of the GUI
	void DoYield();

	// the status bars
	wxGauge *m_gauge1;
	wxGauge *m_gauge2;
	// the message displayed
	wxStaticText *m_msg;

	// parent top level window (may be NULL)
	wxWindow *m_parentTop;

	// continue processing or not (return value for Update())
	enum
	{
		Uncancelable = -1,   // dialog can't be canceled
		Canceled,            // can be cancelled and, in fact, was
		Continue,            // can be cancelled but wasn't
		Finished             // finished, waiting to be removed from screen
	} m_state;

	// the abort buttons (or NULL if none)
	wxButton *m_btnAbort;

	// the maximum value
	int m_maximum;

	// tells how often a change of the estimated time has to be confirmed
	// before it is actually displayed - this reduces the frequence of updates
	// of estimated and remaining time
	const int m_delay;
	// counts the confirmations
	int m_ctdelay;
	unsigned long m_display_estimated;

	bool m_hasAbortButton;

#if defined(__WXMSW__ ) || defined(__WXPM__)
	// the factor we use to always keep the value in 16 bit range as the native
	// control only supports ranges from 0 to 65,535
	size_t m_factor;
#endif // __WXMSW__

	// for wxPD_APP_MODAL case
	class WXDLLEXPORT wxWindowDisabler *m_winDisabler;

	DECLARE_EVENT_TABLE()
	//    DECLARE_NO_COPY_CLASS(DoubleProgressDialog)
};

