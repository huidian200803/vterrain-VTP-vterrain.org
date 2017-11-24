/*
 * Unarchive.cpp - Extract files from a gzipped TAR file
 * adapted from some 'libpng' sample code
//
// Copyright (c) 2003-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef VTUNIX
# include <unistd.h>
# include <utime.h>
#endif

#include "config_vtdata.h"
#include "FilePath.h"
#include "Unarchive.h"
#include "vtUnzip.h"

#include "zlib.h"

#ifdef WIN32
#  ifdef _MSC_VER
#	define unlink(fn)		  _unlink(fn)
#  endif
#endif


/* Values used in typeflag field.  */
#define REGTYPE		'0'		/* regular file */
#define AREGTYPE	'\0'	/* regular file */
#define DIRTYPE		'5'		/* directory */

#define BLOCKSIZE 512

struct tar_header
{						/* byte offset */
	char name[100];		/*	0 */
	char mode[8];		/* 100 */
	char uid[8];		/* 108 */
	char gid[8];		/* 116 */
	char size[12];		/* 124 */
	char mtime[12];		/* 136 */
	char chksum[8];		/* 148 */
	char typeflag;		/* 156 */
	char linkname[100];	/* 157 */
	char magic[6];		/* 257 */
	char version[2];	/* 263 */
	char uname[32];		/* 265 */
	char gname[32];		/* 297 */
	char devmajor[8];	/* 329 */
	char devminor[8];	/* 337 */
	char prefix[155];	/* 345 */
						/* 500 */
};

union tar_buffer {
	char				buffer[BLOCKSIZE];
	struct tar_header	header;
};

/* helper functions */
int getoct(char *p,int width)
{
	int result = 0;
	char c;

	while (width --)
	{
		c = *p++;
		if (c == ' ')
			continue;
		if (c == 0)
			break;
		result = result * 8 + (c - '0');
	}
	return result;
}

/**
 * Unarchives the indicated tarred, gzipped, or gzipped tar file.
 * Each directory and file in the archive is created.
 *
 * \param prepend_path A string to be prepended to all output filenames.
 *
 * \return -1 on error, otherwise the number of files the archive contained.
 */
int ExpandTGZ(const char *archive_fname, const char *prepend_path)
{
	gzFile	in;
	union	tar_buffer buffer;
	int		len;
	int		getheader = 1;
	int		remaining = 0;
	FILE	*outfile = NULL;
	char	fname[BLOCKSIZE];
	char	fullname[1024];
	time_t	tartime;
	int		files_encountered = 0;

	/*
	 *  Process the TGZ file
	 */
	in = gzopen(archive_fname, "rb");
	if (in == NULL)
	{
//		fprintf(stderr,"%s: Couldn't gzopen %s\n", prog, TGZfile);
		return -1;
	}

	while (1)
	{
		len = gzread(in, &buffer, BLOCKSIZE);
		if (len < 0)
		{
			// error (gzerror(in, &err));
			return -1;
		}
		/*
		* Always expect complete blocks to process
		* the tar information.
		*/
		if (len != BLOCKSIZE)
		{
			// error("gzread: incomplete block read");
			gzclose(in);
			return -1;
		}

		/*
		* If we have to get a tar header
		*/
		if (getheader == 1)
		{
			/*
			 * if we met the end of the tar
			 * or the end-of-tar block,
			 * we are done
			 */
			if ((len == 0) || (buffer.header.name[0]== 0)) break;

			tartime = (time_t)getoct(buffer.header.mtime, 12);
			strncpy(fname, buffer.header.name, BLOCKSIZE);

			strcpy(fullname, prepend_path);
			strcat(fullname, fname);

			switch (buffer.header.typeflag)
			{
			case DIRTYPE:
				vtCreateDir(fullname);
				break;
			case REGTYPE:
			case AREGTYPE:
				remaining = getoct(buffer.header.size, 12);
				if (remaining)
				{
					outfile = vtFileOpen(fullname,"wb");
					if (outfile == NULL)
					{
						/* try creating directory */
						char *p = strrchr(fullname, '/');
						if (p != NULL)
						{
							*p = '\0';
							vtCreateDir(fullname);
							*p = '/';
							outfile = vtFileOpen(fullname,"wb");
						}
					}
//					fprintf(stderr, "%s %s\n", (outfile) ? "Extracting" : "Couldn't create", fname);
					files_encountered++;
				}
				else
					outfile = NULL;
				/*
				 * could have no contents
				 */
				getheader = (remaining) ? 0 : 1;
				break;
			default:
				break;
			}
		}
		else
		{
			uint bytes = (remaining > BLOCKSIZE) ? BLOCKSIZE : remaining;

			if (outfile != NULL)
			{
				if (fwrite(&buffer,sizeof(char),bytes,outfile) != bytes)
				{
//					fprintf(stderr,"%s : error writing %s skipping...\n",prog,fname);
					fclose(outfile);
					unlink(fname);
				}
			}
			remaining -= bytes;
			if (remaining == 0)
			{
				getheader = 1;
				if (outfile != NULL)
				{
#ifdef WIN32
					fclose(outfile);

					outfile = NULL;
#else
					struct utimbuf settime;

					settime.actime = settime.modtime = tartime;

					fclose(outfile);
					outfile = NULL;
					utime(fname,&settime);
#endif
				}
			}
		}
	}

	if (gzclose(in) != Z_OK)
	{
		// error("failed gzclose");
		return -1;
	}

	return files_encountered;
}


/**
 * Unarchives the indicated zipped file.
 * Each directory and file in the archive is created.
 *
 * \param prepend_path A string to be prepended to all output filenames.
 *
 * \return -1 on error, otherwise the number of files the archive contained.
 */
int ExpandZip(const char *archive_fname, const char *prepend_path,
			  bool progress_callback(int))
{
	// vtUnzip doesn't handle utf8 paths, so convert to local
	vtString local_fname = UTF8ToLocal(archive_fname);
	vtUnzip uz;
	bool success = uz.Open(local_fname);
	if (!success)
		return -1;
	return uz.Extract(true, true, prepend_path, progress_callback);
}

