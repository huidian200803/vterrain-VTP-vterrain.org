//
// VTConcert.cpp
//
// A very simply command-line tool to convert from any VTP-supported elevation
// format to a BT file.
//
// Copyright (c) 2003-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtdata/ElevationGrid.h"
#include "vtdata/FilePath.h"

void print_help()
{
	printf("VTConvert, a command-line tool for converting geodata.\n");
	printf("Currently, it just converts elevation data, from any format, to the BT format.\n");
	printf(" Build: ");
#if VTDEBUG
	printf("Debug");
#else
	printf("Release");
#endif
	printf(", date: %s\n\n", __DATE__);

	printf("Command-line options:\n");
	printf("  -in infile       Indicates the input file.\n");
	printf("  -out outfile     Indicates the output file.\n");
	printf("  (or)\n");
	printf("  -indir in        Indicates the input directory.\n");
	printf("  -outdir out      Indicates the output directory.\n");
	printf("  -gzip            Write output directly to a .gz file\n");
	printf("\n");
	printf("If outfile is not specified, it is derived from infile.\n");
	printf("If outfile has a trailing slash, it is assumed to be a\n"
		" directory, the output has it's name derived from infile,\n"
		" and is written into that directory.\n");
	printf("\n");
}

void Convert(vtString &fname_in, vtString &fname_out, bool bGZip)
{
	// Add extension, if not present
	if (bGZip)
	{
		if (fname_out.Right(6).CompareNoCase(".bt.gz"))
			fname_out += ".bt.gz";
	}
	else
	{
		if (fname_out.Right(3).CompareNoCase(".bt"))
			fname_out += ".bt";
	}

	vtElevationGrid grid;
	if (grid.LoadFromFile(fname_in))
	{
		if (grid.SaveToBT(fname_out, NULL, bGZip))
		{
			int col, row;
			grid.GetDimensions(col, row);
			printf("Successfully wrote elevation, grid size %d x %d.\n", col, row);
		}
		else
			printf("Failed to write output file.\n");
	}
	else
	{
		printf("Failed to read elevation data from %s\n", (const char *) fname_in);
	}
}

int main(int argc, char **argv)
{
	vtString str, fname_in, fname_out, dirname_in, dirname_out;
	bool bGZip = false;

	for (int i = 0; i < argc; i++)
	{
		str = argv[i];
		if (str == "-in")
		{
			fname_in = argv[i+1];
			i++;
		}
		else if (str == "-out")
		{
			fname_out = argv[i+1];
			i++;
		}
		else if (str == "-indir")
		{
			dirname_in = argv[i+1];
			i++;
		}
		else if (str == "-outdir")
		{
			dirname_out = argv[i+1];
			i++;
		}
		else if (str.Left(2) == "-h")
		{
			print_help();
			return 0;
		}
		else if (str == "-gzip")
		{
			bGZip = true;
		}
	}
	if (fname_in == "" && dirname_in == "")
	{
		printf("Didn't get an input.  Try -h for help.\n");
		return 0;
	}

	// Check if output is a directory
	vtString last = fname_out.Right(1);
	if (last == "/" || last == "\\")
	{
		// Use it as the base of the output path
		fname_out += fname_in;
		RemoveFileExtensions(fname_out);
	}
	else if (fname_in != "" && fname_out == "")
	{
		// Derive output name, if not given, from input.
		fname_out = fname_in;
		RemoveFileExtensions(fname_out);
	}

	if (dirname_in != "")
	{
		vtString end1 = dirname_in.Right(1);
		if (end1 != "/" || end1 != "\\") dirname_in += "/";

		vtString end2 = dirname_out.Right(1);
		if (end2 != "/" || end2 != "\\") dirname_out += "/";

		for (dir_iter it((const char *)dirname_in); it != dir_iter(); ++it)
		{
			if (it.is_hidden() || it.is_directory())
				continue;

			std::string name1 = it.filename();
			vtString fname_in = name1.c_str();

			// Ignore some extension which we know aren't terrain
			vtString ext = GetExtension(fname_in, false);
			if (ext.CompareNoCase(".tfw") == 0 || ext.CompareNoCase(".xml") == 0)
				continue;

			fname_out = fname_in;
			RemoveFileExtensions(fname_out);

			vtString TempIn = dirname_in + fname_in;
			vtString TempOut = dirname_out + fname_out;

			Convert(TempIn, TempOut, bGZip);
		}
	}
	else
		// Simple: just one file
		Convert(fname_in, fname_out, bGZip);

	return 0;
}
