//
// vtUnzip.cpp
//
// Adapted from chunks of code found online, cleaned up and made portable.
//
// Copyright (c) 2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef _WIN32
#include "windows.h"
#endif

#include "vtUnzip.h"
#include "FilePath.h"
#include "vtLog.h"
#include <assert.h>

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

///////////////////////////////////////////////////////////////////////

vtUnzip::vtUnzip() : m_handle(NULL)
{
	m_error_count = 0;
}

bool vtUnzip::Open(const char* lpszFilePath, zlib_filefunc_def* def)
{
	assert(m_handle == NULL);
	m_handle = ::unzOpen2(lpszFilePath, def);
	return (m_handle != NULL);
}

bool vtUnzip::IsOpen(void) const
{
	return (m_handle != NULL);
}

vtUnzip::~vtUnzip()
{
	if (IsOpen())
		Close();
}

void vtUnzip::Close()
{
	assert(m_handle != NULL);
	::unzClose(m_handle);
	m_handle = NULL;
}

bool vtUnzip::GetGlobalComment(char *szComment, uLong uSizeBuf) const
{
	assert(m_handle != NULL);
	return (UNZ_OK == ::unzGetGlobalComment(m_handle, szComment, uSizeBuf));
}

bool vtUnzip::GetGlobalInfo(unz_global_info * p) const
{
	assert(m_handle != NULL);
	return (UNZ_OK == ::unzGetGlobalInfo(m_handle, p));
}

int vtUnzip::GetGlobalCount(void) const
{
	assert(m_handle != NULL);
	unz_global_info info;
	return (::unzGetGlobalInfo(m_handle, &info) == UNZ_OK) ? info.number_entry : 0;
}

bool vtUnzip::GetCurrentFileInfo(unz_file_info *pfile_info,
								 char *szFileName,
								 uLong fileNameBufferSize,
								 void *extraField,
								 uLong extraFieldBufferSize,
								 char *szComment,
								 uLong commentBufferSize) const
{
	assert(m_handle != NULL);
	return (UNZ_OK == ::unzGetCurrentFileInfo(m_handle,
		pfile_info,
		szFileName,
		fileNameBufferSize,
		extraField,
		extraFieldBufferSize,
		szComment,
		commentBufferSize));
}

bool vtUnzip::GoToNextFile(void)
{
	assert(m_handle != NULL);
	return (UNZ_OK == ::unzGoToNextFile(m_handle));
}

bool vtUnzip::GoToFirstFile(void)
{
	return (UNZ_OK == ::unzGoToFirstFile(m_handle));
}

bool vtUnzip::CloseCurrentFile()
{
	assert(m_handle != NULL);
	return (UNZ_OK == ::unzCloseCurrentFile(m_handle));
}

int vtUnzip::ReadCurrentFile(voidp buf, size_t len)
{
	assert(m_handle != NULL);
	return ::unzReadCurrentFile(m_handle, buf, len);
}

bool vtUnzip::OpenCurrentFile(const char* password)
{
	assert(m_handle != NULL);
	return (UNZ_OK == ::unzOpenCurrentFilePassword(m_handle, password));
}

bool vtUnzip::change_file_date(const char *filename, uLong dosdate, tm_unz tmu_date)
{
	bool bOK = false;
#ifdef _WIN32
#if SUPPORT_WSTRING
	wstring2 ws;
	ws.from_utf8(filename);
	HANDLE hFile = CreateFileW(ws.c_str(), GENERIC_READ | GENERIC_WRITE, 0,NULL,
		OPEN_EXISTING,0,NULL);
#else
	HANDLE hFile = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, 0,NULL,
		OPEN_EXISTING,0,NULL);
#endif
	bOK = (hFile != INVALID_HANDLE_VALUE);
	if (bOK)
	{
		FILETIME ftm,ftLocal,ftLastAcc;
		BOOL bBOK = GetFileTime(hFile,NULL,&ftLastAcc,NULL);
		bOK = (bBOK == TRUE);
		DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
		LocalFileTimeToFileTime(&ftLocal,&ftm);
		SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
		CloseHandle(hFile);
	}
#else
	// TODO: Unix implementation, if needed.  Do we care about file dates?
	bOK = true;
#endif
	return bOK;
}

bool vtUnzip::ExtractCurrentFile(FILE* file, void* buf, size_t size_buf)
{
	bool bOK = true;
	while (bOK)
	{
		int bytes = ReadCurrentFile(buf,size_buf);
		if (bytes < 0)
		{
			VTLOG("error with zipfile in ReadCurrentFile\n");
			bOK = false;
		}
		else if (bytes > 0)
		{
			if (fwrite(buf,bytes,1,file)!=1)
			{
				VTLOG("error in writing extracted file\n");
				bOK = false;
			}
		}
		else
			break;
	}
	return bOK;
}

bool vtUnzip::Attach(unzFile handle)
{
	assert(m_handle == NULL);
	m_handle = handle;
	return (handle != NULL);
}

unzFile vtUnzip::Detach(void)
{
	assert(m_handle != NULL);
	unzFile handle = m_handle;
	m_handle = NULL;
	return handle;
}

vtUnzip::operator unzFile(void)
{
	return m_handle;
}

///////////////////////////////////////////////////////////////////////

bool vtUnzip::ExtractAccept(const char *write_filename, bool bOverwrite)
{
	bool bOK = true;
	if (!bOverwrite)
	{
		FILE* file = vtFileOpen(write_filename, "rb");
		bool bExisting = (file != NULL);
		if (file != NULL) fclose(file);
		bOK = !bExisting;
	}
	return bOK;
}

int vtUnzip::Extract(bool bFullPath, bool bOverwrite, const char *lpszDst,
					 bool progress_callback(int))
{
	int iCount = 0;
	int iTotal = GetGlobalCount();

	bool bOK = true;
	for (bool bContinue = GoToFirstFile(); bContinue; bContinue = GoToNextFile())
	{
		char szFileName[MAX_PATH];
		unz_file_info info;
		bOK = GetCurrentFileInfo(&info, szFileName, MAX_PATH);
		if (!bOK)
			break;

		vtString src_filename = szFileName;

		const char *short_fname = (const char *)src_filename;
		for (const char *p = short_fname;
			(*p) != '\0';
			p++)
		{
			if (((*p)=='/') || ((*p)=='\\'))
			{
				short_fname = p+1;
			}
		}
		vtString short_filename = short_fname;

		if ((*short_filename)=='\0')
		{
			if (bFullPath)
			{
				VTLOG("creating directory: %s\n", (const char *)src_filename);
				vtCreateDir(src_filename);
			}
		}
		else
		{
			bOK = OpenCurrentFile();
			if (bOK)
			{
				vtString write_filename;
				write_filename = vtString(lpszDst) + (bFullPath ? src_filename : short_filename);

				vtString strResult;

				VTLOG("Extracting %s ...", (const char *) write_filename);
				if (ExtractAccept(write_filename, bOverwrite))
				{
					FILE* file = vtFileOpen(write_filename, "wb");

					bool bWrite = (file != NULL);
					if (bWrite)
					{
						char buf[4096];
						bWrite = ExtractCurrentFile(file, buf, 4096);
						fclose(file);
					}
					if (bWrite)
					{
						vtUnzip::change_file_date(write_filename,info.dosDate, info.tmu_date);
						iCount++;
						if (progress_callback != NULL)
						{
							progress_callback(iCount * 99 / iTotal);
						}
					}
					else
					{
						m_error_count++;
						OnError(write_filename);
					}
					strResult = bWrite ? "ok" : "failed";
				}
				else
				{
					strResult = "skipped";
				}
				VTLOG(" %s\n", (const char *) strResult);
				CloseCurrentFile();
			}
		}
	}
	if (bOK)
		return iCount;
	else
		return -1;
}

