//
// Unzip.h
//
// Copyright (c) 2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef UNZIP_H
#define UNZIP_H

#include <stdio.h>
#include "unzip/unzip.h"	// Lower-level 'unzip' library

class vtUnzip
{
public:
	// Construction
	vtUnzip();

	// Operations
	bool Open(const char *lpszFilePath, zlib_filefunc_def *p = NULL);
	bool IsOpen(void) const;

	bool GetGlobalInfo(unz_global_info *) const;
	int GetGlobalCount(void) const;
	bool GetGlobalComment(char *szComment, uLong uSizeBuf) const;

	bool GoToFirstFile(void);
	bool GoToNextFile(void);
	bool GetCurrentFileInfo(unz_file_info *pfile_info,
		char *szFileName = NULL,
		uLong fileNameBufferSize = 0,
		void *extraField = NULL,
		uLong extraFieldBufferSize = 0,
		char *szComment = NULL,
		uLong commentBufferSize = 0) const;

	bool OpenCurrentFile(const char *password = NULL);
	int ReadCurrentFile(voidp buf, size_t len);
	bool ExtractCurrentFile(FILE *file, void *buf, size_t buf_size);
	bool CloseCurrentFile();

	bool ExtractAccept(const char *write_filename, bool bOverwrite);
	int Extract(bool bFullPath, bool bOverwrite, const char *lpszDst,
		bool progress_callback(int) = NULL);

	bool	Attach(unzFile);
	unzFile Detach(void);

	operator unzFile(void);

	static bool change_file_date(const char *filename,uLong dosdate,tm_unz tmu_date);

public:
	// Implementation
	virtual ~vtUnzip();
	virtual void Close();
	int  m_error_count;
	virtual void OnError(const char *lpszFilePath) {}

public:
	// Attributes
	unzFile m_handle;
};

#endif // UNZIP_H

