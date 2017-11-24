//
// Header for Unarchive.cpp
//

#ifndef UNARCHIVE_H
#define UNARCHIVE_H

int ExpandTGZ(const char *archive_fname, const char *prepend_path);
int ExpandZip(const char *archive_fname, const char *prepend_path,
			  bool progress_callback(int) = NULL);

#endif

