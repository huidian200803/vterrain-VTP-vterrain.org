//
// DataPath.cpp
//
// Copyright (c) 2007-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "DataPath.h"
#include "FilePath.h"
#include "vtLog.h"

// globals
static vtStringArray s_datapath;
static vtString s_configfile;

///////////////////////////////////////////////////////////////////////
// XML format

#include "xmlhelper/easyxml.hpp"
#define STR_DATAPATH "DataPath"

class PathsVisitor : public XMLVisitor
{
public:
	PathsVisitor() {}
	void startElement(const char *name, const XMLAttributes &atts) { m_data = ""; }
	void endElement (const char *name)
	{
		const char *str = m_data.c_str();
		if (strcmp(name, STR_DATAPATH) == 0)
			s_datapath.push_back(str);
	}
	void data(const char *s, int length) { m_data.append(string(s, length)); }

protected:
	std::string m_data;
};


////////////////////////////////////////////////

void vtSetDataPath(const vtStringArray &paths)
{
	s_datapath = paths;
}

vtStringArray &vtGetDataPath()
{
	return s_datapath;
}

bool vtLoadDataPath(const char *user_config_dir, const char *config_dir)
{
	// Look for the config file which has the datapaths.
	// There are two supported places for it.
	//  1. In the same directory as the executable.
	//  2. On Windows, in the user's "Application Data" folder.
	vtString BaseName = "vtp.xml";
	vtString FileName = BaseName;

	vtString AppDataUser(user_config_dir ? user_config_dir : "");
	vtString AppDataCommon(config_dir ? config_dir : "");

	VTLOG("Looking for '%s'\n", (const char *) FileName);
	bool bFound = vtFileExists(FileName);
	if (!bFound)
	{
		VTLOG1("Not found.\n");
		FileName = AppDataUser + "/" + BaseName;
		VTLOG("Looking for '%s'\n", (const char *) FileName);
		bFound = vtFileExists(FileName);
	}
	if (!bFound)
	{
		VTLOG1("Not found.\n");
		FileName = AppDataCommon + "/" + BaseName;
		VTLOG("Looking for '%s'\n", (const char *) FileName);
		bFound = vtFileExists(FileName);
	}

	// Not found anywhere.  Default to default data path
	if (!bFound)
	{
		VTLOG1("Not found.\n");
		return false;
	}

	VTLOG("\tReading datapaths from '%s'\n", (const char*)FileName);

	PathsVisitor visitor;
	try
	{
		VTLOG1("\tA. convert filename to std::string\n");
		std::string fname2(FileName);

		VTLOG1("\tB. calling readXML\n");
		readXML(fname2, visitor);
	}
	catch (xh_io_exception &ex)
	{
		VTLOG1("\tC. caught xh_io_exception\n");
		const string msg = ex.getFormattedMessage();
		VTLOG(" XML problem: %s\n", msg.c_str());
		return false;
	}
	catch (xh_throwable &t)
	{
		VTLOG1("\tD. caught xh_throwable\n");
		const string msg = t.getFormattedMessage();
		VTLOG(" XML problem: %s\n", msg.c_str());
		return false;
	}
	VTLOG1("\tSuccessfully read.\n");

	// Remember where we loaded it from
	s_configfile = FileName;

	// Supply the special symbols {appdata} and {appdatacommon}
	for (uint i = 0; i < s_datapath.size(); i++)
	{
		s_datapath[i].Replace("{appdata}", AppDataUser);
		s_datapath[i].Replace("{appdatacommon}", AppDataCommon);
	}

	VTLOG1("Loaded Datapaths:\n");
	uint n = s_datapath.size();
	if (n == 0)
		VTLOG("   none.\n");
	for (uint i = 0; i < n; i++)
		VTLOG("   %s\n", (const char *) s_datapath[i]);
	VTLOG1("\n");
	return true;
}

bool vtSaveDataPath(const char *fname)
{
	vtString filename;
	if (fname)
	{
		filename = fname;

		// Remember where we're saving it to
		s_configfile = fname;
	}
	else
		filename = s_configfile;

	VTLOG("Writing datapaths to '%s'\n", (const char *) filename);

	FILE *fp = vtFileOpen(filename, "wb");
	if (!fp)
	{
		vtString msg;
		msg = "Couldn't write settings to file \"";
		msg += filename;
		msg += "\"\nPlease make sure it's writable.\n";
		VTLOG(msg);
		return false;
	}

	// write to file
	fprintf(fp, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	fprintf(fp, "<VTP>\n");
	for (uint i = 0; i < s_datapath.size(); i++)
	{
		fprintf(fp, "\t<" STR_DATAPATH ">");
		fputs(s_datapath[i], fp);
		fprintf(fp, "</" STR_DATAPATH ">\n");
	}
	fprintf(fp, "</VTP>\n");
	fclose(fp);
	return true;
}

/**
 Takes an absolute path to a file.  Determines if that file is on the data
 path, and if so it shortens the filename and returns true.

 \param fname The absolute path to a file.
 \param folder The folder within the datapath to look.
 \return true if the file was found and the filename was shortened.
 */
bool MakeRelativeToDataPath(vtString &fname, const vtString &folder)
{
	vtString file = StartOfFilename(fname);

	vtString search = file;
	if (folder != "")
		search = folder + "/" + file;

	const vtStringArray &paths = vtGetDataPath();

	vtString full = FindFileOnPaths(paths, search);
	if (full != "")
	{
		fname = file;
		return true;
	}

	// Also check if the fname contains any of the data paths.
	// Convert slashes for consistent comparison, and ignore case.
	vtString fname_lower = fname;
	fname_lower.MakeLower();
	fname_lower.Replace('\\', '/');
	for (uint i = 0; i < paths.size(); i++)
	{
		vtString path_lower = paths[i];
		path_lower.MakeLower();
		path_lower.Replace('\\', '/');

		const int length = path_lower.GetLength();
		if (fname_lower.Left(length) == path_lower)
		{
			fname = fname.Mid(length);
			return true;
		}
	}

	return false;
}

