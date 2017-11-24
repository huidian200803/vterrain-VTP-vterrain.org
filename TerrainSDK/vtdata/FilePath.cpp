//
// FilePath.cpp
//
// Functions for helping with management of files, paths and directories.
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdio.h>
#include <errno.h>

#include "FilePath.h"
#include "vtLog.h"

#ifdef VTUNIX
# include <unistd.h>
# include <sys/stat.h>
#if __GNUC__ == 4 && __GNUC_MINOR__ >= 3
  #include <cstdlib>
#endif
#else
# include <direct.h>
# include <io.h>
 #ifdef _MSC_VER	// MSVC also has sys/stat
 # include <sys/stat.h>
 #endif
#endif

#if WIN32
#  ifdef _MSC_VER
#	undef mkdir		// replace the one in direct.h that takes 1 param
#	define mkdir(dirname,mode)	_mkdir(dirname)
#	define rmdir(dirname)		_rmdir(dirname)
#	define strdup(str)			_strdup(str)
#	define unlink(fname)		_unlink(fname)
#	define access(path,mode)	_access(path,mode)
#	define vsnprintf			_vsnprintf
#   define stat					_stat
#  else
#	define mkdir(dirname,mode) _mkdir(dirname)
#  endif
#else
#  include <utime.h>
#endif

#if SUPPORT_BZIP2
  #include "bzlib.h"
#endif

/**
 * The dir_iter class provides a cross-platform way to read directories.
 * It is addapted from the 'boost' library, without encurring the huge overhead
 * of adding boost as a dependency.
 */
#if WIN32

dir_iter::dir_iter()
{
	m_handle = -1;
}

dir_iter::dir_iter(std::string const &dirname)
{
#if SUPPORT_WSTRING
	wstring2 name;
	name.from_utf8(dirname.c_str());
	name += L"\\*";
	m_handle = _wfindfirst((wchar_t *)name.c_str(), &m_data);
#else
	std::string name;
	name = dirname;
	name += "\\*";
	m_handle = _findfirst(name.c_str(), &m_data);
#endif
}

dir_iter::~dir_iter()
{
	if (m_handle != -1)
		_findclose(m_handle);
}

bool dir_iter::is_directory()
{
	return (m_data.attrib & _A_SUBDIR) != 0;
}

bool dir_iter::is_hidden()
{
	return (m_data.attrib & _A_HIDDEN) != 0;
}

std::string dir_iter::filename()
{
#if SUPPORT_WSTRING
	wstring2 name = m_data.name;
	return name.to_utf8();
#else
	std::string name = m_data.name;
	return name;
#endif
}

void dir_iter::operator++()
{
	if (m_handle != -1)
	{
#if SUPPORT_WSTRING
		if (_wfindnext(m_handle, &m_data) == -1)
#else
		if (_findnext(m_handle, &m_data) == -1)
#endif
		{
			_findclose(m_handle);
			m_handle = -1;
		}
	}
}

bool dir_iter::operator!=(const dir_iter &it)
{
	return (m_handle == -1) != (it.m_handle == -1);
}

///////////////////////////////////////////////////////////////////////
#else	// non-WIN32 platforms, i.e. generally Unix

dir_iter::dir_iter()
{
	m_handle = 0;
	m_stat_p = false;
}

dir_iter::dir_iter(std::string const &dirname)
{
	m_dirname = dirname;
	if (m_dirname[m_dirname.length()-1] != '/')
		m_dirname += "/";

	m_handle = opendir(dirname.c_str());
	m_stat_p = false;
	operator++ ();
}

dir_iter::~dir_iter()
{
	if (m_handle)
		closedir(m_handle);
}

bool dir_iter::is_directory()
{
	return S_ISDIR(get_stat().st_mode);
}

bool dir_iter::is_hidden()
{
	return (m_current[0] == '.');
}

std::string dir_iter::filename()
{
	return m_current;
}

void dir_iter::operator++()
{
	if (!m_handle)
		return;

	m_stat_p = false;
	dirent *rc = readdir(m_handle);
	if (rc != 0)
		m_current = rc->d_name;
	else
	{
		m_current = "";
		closedir(m_handle);
		m_handle = 0;
	}
}

bool dir_iter::operator!=(const dir_iter &it)
{
	return (m_handle == 0) != (it.m_handle == 0);
}

struct stat &dir_iter::get_stat()
{
	//VTLOG1("get_stat");
	if (!m_stat_p)
	{
		//VTLOG(" calling lstat on '%s':", (m_dirname + m_current).c_str());
		lstat((m_dirname + m_current).c_str(), &m_stat);
		//VTLOG(" %d", m_stat.st_mode);
		m_stat_p = true;
	}
	//VTLOG1("\n");
	return m_stat;
}

#endif	// !WIN32


#define LOG_FIND 0
#if LOG_FIND
#define LOGFIND VTLOG
#else
#define LOGFIND
#endif

/**
 * This function will search for a given file on the given paths, returning
 * the full path to the first file which is found (file exists and can be
 * read from).
 *
 * \param paths An array of strings containing the directories to search.
 * Each directory should end with a the trailing slash ("/" or "\")
 *
 * \param filename A filename, which can optionally contain a partial path
 * as well.  Examples: "foo.txt" or "Stuff/foo.txt"
 * Note: encoding of this filename is assumed to be UTF-8!
 */
vtString FindFileOnPaths(const vtStringArray &paths, const char *filename)
{
	if (!strcmp(filename, ""))
		return vtString("");

	// it's possible that the filename is already resolvable without
	// searching the data paths
	LOGFIND("Searching for file... '%s'\n", filename);
	FILE *fp = vtFileOpen(filename, "r");
	if (fp != NULL)
	{
		fclose(fp);
		LOGFIND("File exists: %s\n", filename);
		return vtString(filename);
	}

	for (uint i = 0; i < paths.size(); i++)
	{
		vtString fname = paths[i];
		LOGFIND("... looking in '%s'\n", (const char*) fname);
		fname += filename;
		fp = vtFileOpen((const char *)fname, "r");
		if (fp != NULL)
		{
			fclose(fp);
			LOGFIND("Resolved file: '%s'\n", (const char*)fname);
			return fname;
		}
	}
	return vtString("");
}

int vtMkdir(const char *dirname)
{
#ifdef _MSC_VER
	// convert utf-8 to widechar
#if SUPPORT_WSTRING
	wstring2 name;
	name.from_utf8(dirname);
	return _wmkdir(name.c_str());
#else
	return _mkdir(dirname);
#endif
#else
	return mkdir(dirname, 0775);
#endif
}

/**
 * Recursive make directory.
 * Aborts if there is an ENOENT error somewhere in the middle.
 *
 * \param dirname The full page to directory to be created, in utf-8 encoding.
 * \return true if OK, false on error
 */
bool vtCreateDir(const char *dirname)
{
	char *buffer = strdup(dirname);
	char *p;
	int  len = strlen(buffer);

	if (len <= 0) {
		free(buffer);
		return false;
	}
	if (buffer[len-1] == '/') {
		buffer[len-1] = '\0';
	}
	if (vtMkdir(buffer) == 0)	// 0 means success
	{
		free(buffer);
		return true;
	}

	p = buffer+1;
	while (1)
	{
		char hold;

		while(*p && *p != '\\' && *p != '/')
			p++;
		hold = *p;
		*p = 0;
		if ((vtMkdir(buffer) == -1) && (errno == ENOENT))
		{
			VTLOG("Error: couldn't create directory '%s'\n", buffer);
			free(buffer);
			return false;
		}
		if (hold == 0)
			break;
		*p++ = hold;
	}
	free(buffer);
	return true;
}

/**
 * Destroy a directory and all its contents (recusively if needed).
 */
void vtDestroyDir(const char *dirname)
{
	int result;

	vtStringArray con;

	char fullname[1024];
	dir_iter it(dirname);
	for (; it != dir_iter(); ++it)
	{
		std::string name1 = it.filename();
		if (name1 == "." || name1 == "..")
			continue;
		con.push_back(vtString(name1.c_str()));
	}

	for (uint i = 0; i < con.size(); i++)
	{
		vtString item = con[i];

		strcpy(fullname, dirname);
		strcat(fullname, "/");
		strcat(fullname, (const char *) item );

		if (it.is_directory())
		{
			vtDestroyDir(fullname);
		}
		else
		{
			result = unlink(fullname);
			if (result == -1)
			{
				// failed
				if (errno == ENOENT)	// not found
					result = 0;
				if (errno == EACCES)	// found but can't delete
					result = 0;
			}
		}
	}
	rmdir(dirname);
}

/**
 * Delete a file.
 */
void vtDeleteFile(const char *filename)
{
	unlink(filename);
}


/**
 * Given a full path containing a filename, return a pointer to
 * the filename portion of the string.
 */
const char *StartOfFilename(const char *szFullPath)
{
	const char *tmp = szFullPath;
	const char *tmp1 = strrchr(szFullPath, '/');
	if (tmp1)
		tmp = tmp1+1;
	const char *tmp2 = strrchr(szFullPath, '\\');
	if (tmp2 && tmp2 > tmp)
		tmp = tmp2+1;
	const char *tmp3 = strrchr(szFullPath, ':');
	if (tmp3 && tmp3 > tmp)
		tmp = tmp3+1;
	return tmp;
}

/**
 * Given a full path containing a filename, return a string containing
 * just the path portion of the string.
 *
 * \param szFullPath The full path.
 * \param bTrailingSlash If true, include the trailing slash, if present.
 *		Example Input: "/usr/data/file.txt"
 *		Output: /usr/data/" with trailing slash, "/usr/data" without.
 */
vtString ExtractPath(const char *szFullPath, bool bTrailingSlash)
{
	const char *fname = StartOfFilename(szFullPath);
	int len = fname - szFullPath;
	if (!bTrailingSlash  && len > 0)
	{
		if (szFullPath[len-1] == '/' || szFullPath[len-1] == '\\')
			len--;
	}
	return vtString(szFullPath, len);
}

/**
 * Return whether a path is absolute or relative.
 */
bool PathIsAbsolute(const char *szPath)
{
	int len = strlen(szPath);
	if (len >= 2 && szPath[1] == ':')
		return true;
	if (len >= 1 && (szPath[0] == '/' || szPath[0] == '\\'))
		return true;
	return false;
}

/** Given a path like "C:/bar/foo", returns the path a level up, like "C:/bar"
 */
vtString PathLevelUp(const char *src)
{
	vtString up = src;

	const char *tmp = NULL;
	const char *tmp1 = strrchr(src, '/');
	if (tmp1)
		tmp = tmp1;
	const char *tmp2 = strrchr(src, '\\');
	if (tmp2 && tmp2 > tmp)
		tmp = tmp2;
	const char *tmp3 = strrchr(src, ':');
	if (tmp3 && tmp3 > tmp)
		tmp = tmp3;
	if (tmp)
		return vtString(src, (tmp-src));
	else
		return vtString("");
}

/**
 * Given a filename (which may include a path), remove any file extension(s)
 * which it may have.
 */
void RemoveFileExtensions(vtString &fname, bool bAll)
{
	for (int i = fname.GetLength()-1; i >= 0; i--)
	{
		char ch = fname[i];

		// If we hit a path divider, stop
		if (ch == ':' || ch == '\\' || ch == '/')
			break;

		// If we hit a period which indicates an extension, snip
		if (ch == '.')
		{
			fname = fname.Left(i);

			// if we're not snipping all the extensions, stop now
			if (!bAll)
				return;
		}
	}
}

/**
 * Get the full file extension(s) from a filename.
 */
vtString GetExtension(const vtString &fname, bool bFull)
{
	int chop = -1;
	for (int i = fname.GetLength()-1; i >= 0; i--)
	{
		char ch = fname[i];

		// If we hit a path divider, stop
		if (ch == ':' || ch == '\\' || ch == '/')
			break;

		// If we hit a period which indicates an extension, note it.
		if (ch == '.')
		{
			chop = i;
			if (!bFull)
				break;
		}
	}
	if (chop == -1)
		return vtString("");
	else
		return fname.Right(fname.GetLength() - chop);
}

vtString ChangeFileExtension(const char *input, const char *extension)
{
	vtString result = input;
	int index = result.ReverseFind('.');
	if (index != -1)
		result = result.Left(index);
	result += extension;
	return result;
}

bool vtFileExists(const char *fname)
{
#if VTDEBUG
	VTLOG("FileExists(%s):", fname);
#endif
	FILE *fp = vtFileOpen(fname, "r");
	if (!fp)
	{
#if VTDEBUG
		VTLOG1("false.\n");
#endif
		return false;
	}
	fclose(fp);
#if VTDEBUG
	VTLOG1("true.\n");
#endif
	return true;
}

int GetFileSize(const char *fname)
{
	struct stat buf;
	int result;

	/* Get data associated with "crt_stat.c": */
	result = stat(fname, &buf );

	/* Check if statistics are valid: */
	if (result != 0)
		return 0;
	return buf.st_size;
}

void SetEnvironmentVar(const vtString &var, const vtString &value)
{
#if VTUNIX
	setenv(var, value, 1);	// 1 means overwrite
#elif WIN32
	vtString msg = var + "=" + value;
	_putenv(msg);
#endif
	VTLOG1("setenv ");
	VTLOG1(var);
	VTLOG1(" = ");
	VTLOG1(value);
	VTLOG1("\n");
}


#include <fstream>
using namespace std;
//
// helper
//
vtString get_line_from_stream(ifstream &input)
{
	char buf[256];
	input.getline(buf, 256);
	int len = strlen(buf);

	// trim trailing CR and LF characters
	while (len > 0 && (buf[len-1] == '\r' || buf[len-1] == '\n'))
	{
		buf[len-1] = '\0';
		len--;
	}
	return vtString(buf);
}

/* alternate version
vtString get_line_from_stream(ifstream &input)
{
	char buf[80];
	// eat leading LF
	if (input.peek() == '\n') {
		input.ignore();
		buf[0] = '\0';
	} else {
		input >> buf;
	}
	return vtString(buf);
}
*/


//
// Encapsulation for Zlib's gzip output functions.  These wrappers allow
//  you to do stdio file output to a compressed _or_ uncompressed file
//  with one set of functions.
//
GZOutput::GZOutput(bool bCompressed)
{
	fp = NULL;
	gfp = NULL;
	bGZip = bCompressed;
}

bool gfopen(GZOutput &out, const char *fname)
{
	if (out.bGZip)
	{
		out.gfp = vtGZOpen(fname, "wb");
		return (out.gfp != NULL);
	}
	else
	{
		out.fp = vtFileOpen(fname, "wb");
		return out.fp != NULL;
	}
}

int gfprintf(GZOutput &out, const char *pFormat, ...)
{
	va_list va;
	va_start(va, pFormat);

	if (out.bGZip)
	{
		// For unknown reasons, gzprintf sometimes fails to write strings,
		//  instead writing a handful of bogus bytes.
//		return gzprintf(out.gfp, pFormat, va);

		// Work around the mysterious failure in gzprintf
		char buf[4096];
		int chars = vsnprintf(buf, 4096, pFormat, va);
		gzwrite(out.gfp, buf, chars);
		return chars;
	}
	else
		return vfprintf(out.fp, pFormat, va);
}

void gfclose(GZOutput &out)
{
	if (out.bGZip)
		gzclose(out.gfp);
	else
		fclose(out.fp);
}


/////////////////////////////////////////////////////////////////////////////
// vtCompressionReader class

vtCompressionReader::vtCompressionReader()
{
	fp = NULL;
	gfp = NULL;
#if SUPPORT_BZIP2
	bfp = NULL;
#endif
}

vtCompressionReader::~vtCompressionReader()
{
	close();
}

bool vtCompressionReader::open(const char *fname)
{
	fp = vtFileOpen(fname, "rb");
	if (!fp)
	{
		fp = NULL;
		return false;
	}
	uchar buf[3];
	if (fread(buf, 3, 1, fp) != 1)
	{
		fp = NULL;
		fclose(fp);
		return false;
	}
	// Gzip signature: 1f 8b 08
	if (buf[0] == 0x1f && buf[1] == 0x8b && buf[2] == 0x08)
	{
		fclose(fp);
		fp = NULL;
		gfp = gzopen(fname, "rb");
		if (gfp == NULL)
			return false;
	}
#if SUPPORT_BZIP2
	// BZip2 signature: 5a 42 31 ("BZh")
	else if (buf[0] == 'B' && buf[1] == 'Z' && buf[2] == 'h')
	{
		fclose(fp);
		fp = NULL;
		bfp = BZ2_bzopen(fname, "rb");
		if (bfp == NULL)
			return false;
	}
#endif
	else
	{
		// plain stdio file IO; nothing to do but rewind to the beginning
		rewind(fp);
	}
	return true;
}

size_t vtCompressionReader::read(void *buf, size_t size)
{
	if (gfp)
		return gzread(gfp, buf, size);
#if SUPPORT_BZIP2
	else if (bfp)
		return BZ2_bzread(bfp, buf, size);
#endif
	else if (fp)
		return fread(buf, size, 1, fp);
	return 0;
}

void vtCompressionReader::close()
{
	if (gfp)
	{
		gzclose(gfp);
		gfp = NULL;
	}
#if SUPPORT_BZIP2
	else if (bfp)
	{
		BZ2_bzclose(bfp);
		bfp = NULL;
	}
#endif
	else if (fp)
	{
		fclose(fp);
		fp = NULL;
	}
}


///////////////////////////////////////////////////////////////////////
// Excapsulation of Zlib's gzip input functions
//
gzFile vtGZOpen(const char *path, const char *mode)
{
	return gzopen(path, mode);
}

///////////////////////////////////////////////////////////////////////
//

/**
 * Open a file using a UTF-8 encoded filename.
 *
 * Parameters are the same as fopen().  The only difference is that
 * instead of being limited to multi-byte local charset, it is UTF-8
 * which supports all languages.
 */
FILE *vtFileOpen(const char *fname_utf8, const char *mode)
{
	// A conversion might be needed for file names on the systems where
	//  they're not Unicode (basically anything except Windows)
	//
	// By default, it's UTF-8 under Mac OS X and Libc elsewhere.  Windows
	//  has wfopen which takes wide characters naturally.

#if WIN32
	// Windows
#if SUPPORT_WSTRING
	wstring2 fn, mo(mode);
	fn.from_utf8(fname_utf8);
	FILE *fp = _wfopen(fn.c_str(), mo.c_str());
#else
	FILE *fp = fopen(fname_utf8, mode);
#endif
	if (!fp)
	{
		if (errno != ENOENT)
			VTLOG("_wfopen failed, errno is %d\n", errno);
		return NULL;
	}
	return fp;
#elif __DARWIN_OSX__
	// Mac OS X
	return fopen(fname_utf8, mode);
#else
	// some other Unix flavor
  #if SUPPORT_WSTRING
	wstring2 fn, mo(mode);
	fn.from_utf8(fname_utf8);
	return fopen(fn.mb_str(), mode);
  #else
	// hope for the best
	return fopen(fname_utf8, mode);
  #endif
#endif
}

/**
 * Open a file using a wide-character (i.e. Unicode) filename.
 *
 * Parameters are the same as fopen().  The only difference is that
 * instead of being limited to multi-byte local charset, it is Unicode
 * which supports all languages.
 */
FILE *vtFileOpen(wchar_t *fname_wide, const char *mode)
{
  #if SUPPORT_WSTRING

#if WIN32
	// Windows
	wstring2 mo(mode);
	return _wfopen(fname_wide, mo.c_str());
#elif __DARWIN_OSX__
	// Mac OS X
	wstring2 fn(fname_wide);
	return fopen(fn.to_utf8(), mode);
#else
	// some other Unix flavor
	wstring2 fn(fname_wide);
	return fopen(fn.mb_str(), mode);
#endif

  #else
	return NULL;	// Are there Unix platforms without wstring?
  #endif
}

#if SUPPORT_WSTRING

/**
 * Open a file using a UTF-8 encoded filename.
 *
 * Parameters are the same as fopen().  The only difference is that
 * instead of being limited to multi-byte local charset, it is Unicode
 * which supports all languages.
 */
FILE *vtFileOpen(const wstring &fname_ws, const char *mode)
{
#if WIN32
	wstring2 mo(mode);
	return _wfopen(fname_ws.c_str(), mo.c_str());
	// Windows
#elif __DARWIN_OSX__
	// Mac OS X
	const wstring2 fn(fname_ws.c_str());
	return fopen(fn.to_utf8(), mode);
#else
	// some other Unix flavor
	const wstring2 fn(fname_ws.c_str());
	return fopen(fn.mb_str(), mode);
#endif
}

#endif // SUPPORT_WSTRING
