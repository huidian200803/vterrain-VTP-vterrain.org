//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#pragma once

#include "vtdata/vtLog.h"

/** Catch wxWidgets logging message, direct them to the vtLog */
class LogCatcher : public wxLog
{
	void DoLogText(const wxString& msg)
	{
		VTLOG1(" wxLog: ");
		VTLOG1(msg.ToUTF8());
		VTLOG1("\n");
	}

	virtual void DoLogRecord(wxLogLevel level,
		const wxString& msg,
		const wxLogRecordInfo& info)
	{
	    wxString message(msg);
	    DoLogText(message);
	}
};
