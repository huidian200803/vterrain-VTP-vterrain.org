//
// Joystick.h - provide access to wxWidget's joystick capabilities for Win32
//  if it was not compiled into the main library.
//
// Copyright (c) 2006-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if WIN32

#if wxUSE_JOYSTICK
#include "wx/joystick.h"
#else

class wxJoystick
{
 public:
  /*
   * Public interface
   */
  wxJoystick(int joystick = wxJOYSTICK1);

  // Attributes
  ////////////////////////////////////////////////////////////////////////////

  wxPoint GetPosition(void) const;
  int GetZPosition(void) const;
  int GetButtonState(void) const;
  int GetPOVPosition(void) const;
  int GetPOVCTSPosition(void) const;
  int GetRudderPosition(void) const;
  int GetUPosition(void) const;
  int GetVPosition(void) const;
  int GetMovementThreshold(void) const;
  void SetMovementThreshold(int threshold) ;

  // Capabilities
  ////////////////////////////////////////////////////////////////////////////

  static int GetNumberJoysticks(void);

  bool IsOk(void) const; // Checks that the joystick is functioning
  int GetXMin(void) const;
  int GetYMin(void) const;
  int GetZMin(void) const;
  int GetXMax(void) const;
  int GetYMax(void) const;
  int GetZMax(void) const;
  int GetNumberButtons(void) const;
  int GetNumberAxes(void) const;
  int GetMaxButtons(void) const;
  int GetMaxAxes(void) const;
  int GetPollingMin(void) const;
  int GetPollingMax(void) const;
  int GetRudderMin(void) const;
  int GetRudderMax(void) const;

  bool HasRudder(void) const;
  bool HasZ(void) const;
  bool HasPOV(void) const;
  bool HasPOV4Dir(void) const;
  bool HasPOVCTS(void) const;

  // Operations
  ////////////////////////////////////////////////////////////////////////////

  // pollingFreq = 0 means that movement events are sent when above the threshold.
  // If pollingFreq > 0, events are received every this many milliseconds.
  bool SetCapture(wxWindow *win, int pollingFreq = 0);
  bool ReleaseCapture(void);

protected:
  int       m_joystick;
};

#endif // !wxUSE_JOYSTICK

#endif // WIN32


