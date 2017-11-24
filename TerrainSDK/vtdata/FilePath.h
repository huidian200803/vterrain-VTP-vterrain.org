//
// FilePath.h
//
// Copyright (c) 2002-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
/** \file FilePath.h */

#ifndef FILEPATHH
#define FILEPATHH

#include "vtString.h"

#include "zlib.h"

#if SUPPORT_BZIP2
  #ifndef _BZLIB_H
	typedef void BZFILE;
  #endif
#endif

#include <fstream>

#ifdef WIN32
  #include <io.h>
#else
  #include <sys/stat.h>
  #include <dirent.h>
  #include <unistd.h>
  #include <pwd.h>
  #include <grp.h>
#endif

/**
 * A portable class for reading directory contents.
 *
 * Example of use:
\code
	for (dir_iter it("C:/temp"); it != dir_iter(); ++it)
	{
		if (it.is_hidden())
			continue;
		if (it.is_directory())
			printf("Directory: '%s'\n", it.filename().c_str());
		else
			printf("File: '%s'\n", it.filename().c_str());
	}
\endcode
 */
class dir_iter
{
public:
	dir_iter();
	dir_iter(std::string const &dirname);
	~dir_iter();

	/// Returns true if the current object is a directory.
	bool is_directory();

	/// Returns true if the current object is hidden.
	bool is_hidden();

	/// Get the filename fo the current object.
	std::string filename();

	// Iterate the object to the next file/directory.
	void operator++();

	// Test for inequality useful to test when iteration is finished.
	bool operator!=(const dir_iter &it);

private:
#ifdef WIN32
#if SUPPORT_WSTRING
	struct _wfinddata_t m_data;
#else
	struct _finddata_t m_data;
#endif
	long               m_handle;
#else
	DIR         *m_handle;
	std::string m_dirname;
	std::string m_current;
	struct stat m_stat;
	bool        m_stat_p;
	struct stat &get_stat();
#endif
};

vtString FindFileOnPaths(const vtStringArray &paths, const char *filename);
bool vtCreateDir(const char *dirname);
void vtDestroyDir(const char *dirname);
void vtDeleteFile(const char *filename);
const char *StartOfFilename(const char *szFullPath);
vtString ExtractPath(const char *szFullPath, bool bTrailingSlash);
bool PathIsAbsolute(const char *szPath);
vtString PathLevelUp(const char *src);
vtString get_line_from_stream(std::ifstream &input);
void RemoveFileExtensions(vtString &fname, bool bAll = true);
vtString GetExtension(const vtString &fname, bool bFull = true);
vtString ChangeFileExtension(const char *input, const char *extension);
bool vtFileExists(const char *fname);
int GetFileSize(const char *fname);

void SetEnvironmentVar(const vtString &var, const vtString &value);


// Encapsulation for Zlib's gzip output functions.
class GZOutput
{
public:
	GZOutput(bool bCompressed);
	bool bGZip;
	FILE *fp;
	gzFile gfp;
};
bool gfopen(GZOutput &out, const char *fname);
int gfprintf(GZOutput &out, const char *pFormat, ...);
void gfclose(GZOutput &out);

// Excapsulation of Zlib's gzip input functions
// adds support for utf-8 filenames

gzFile vtGZOpen(const char *path, const char *mode);


// This class encapsulate reading from a file which may be compressed
//  with gzip, compressed with bzip2, or not compressed.  It automatically
//  recognizes the compression so the caller doesn't have to check.
class vtCompressionReader
{
public:
	vtCompressionReader();
	~vtCompressionReader();

	bool open(const char *fname);
	size_t read(void *buf, size_t size);
	void close();

protected:
	FILE *fp;
	gzFile gfp;
#if SUPPORT_BZIP2
	BZFILE *bfp;
#endif
};


/////////////////////////////////////////////
// Open a file using a UTF-8 or wide character filename.

FILE *vtFileOpen(const char *fname_utf8, const char *mode);
FILE *vtFileOpen(wchar_t *fname_wide, const char *mode);

#if SUPPORT_WSTRING
FILE *vtFileOpen(const std::wstring &fname_ws, const char *mode);
#endif

#endif // FILEPATHH

