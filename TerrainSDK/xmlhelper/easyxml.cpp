// easyxml.cpp - implementation of EasyXML interfaces.

#include <string.h>		// strcmp()
#include <fstream>
using namespace std;

#if WIN32
#include <io.h>
#define lseek _lseek
#endif

#include "easyxml.hpp"
#include "xmlparse.h"
#include "widestring.h"

////////////////////////////////////////////////////////////////////////
// Implementation of XMLAttributes.
////////////////////////////////////////////////////////////////////////

XMLAttributes::XMLAttributes ()
{
}

XMLAttributes::~XMLAttributes ()
{
}

int XMLAttributes::findAttribute (const char * name) const
{
	int s = size();
	for (int i = 0; i < s; i++) {
		if (strcmp(name, getName(i)) == 0)
			return i;
	}
	return -1;
}

bool XMLAttributes::hasAttribute (const char * name) const
{
	return (findAttribute(name) != -1);
}

const char *XMLAttributes::getValue (const char * name) const
{
	int pos = findAttribute(name);
	if (pos >= 0)
		return getValue(pos);
	else
		return 0;
}

////////////////////////////////////////////////////////////////////////
// Implementation of XMLAttributesDefault.
////////////////////////////////////////////////////////////////////////

XMLAttributesDefault::XMLAttributesDefault ()
{
}

XMLAttributesDefault::XMLAttributesDefault (const XMLAttributes &atts)
{
	int s = atts.size();
	for (int i = 0; i < s; i++)
		addAttribute(atts.getName(i), atts.getValue(i));
}

XMLAttributesDefault::~XMLAttributesDefault ()
{
}

int XMLAttributesDefault::size () const
{
	return _atts.size() / 2;
}

const char *XMLAttributesDefault::getName (int i) const
{
	return _atts[i*2].c_str();
}

const char *XMLAttributesDefault::getValue (int i) const
{
	return _atts[i*2+1].c_str();
}

void XMLAttributesDefault::addAttribute (const char * name, const char * value)
{
	_atts.push_back(name);
	_atts.push_back(value);
}

void XMLAttributesDefault::setName (int i, const char * name)
{
	_atts[i*2] = name;
}

void XMLAttributesDefault::setValue (int i, const char * name)
{
	_atts[i*2+1] = name;
}

void XMLAttributesDefault::setValue (const char * name, const char * value)
{
	int pos = findAttribute(name);
	if (pos >= 0) {
		setName(pos, name);
		setValue(pos, value);
	} else {
		addAttribute(name, value);
	}
}


////////////////////////////////////////////////////////////////////////
// Attribute list wrapper for Expat.
////////////////////////////////////////////////////////////////////////

class ExpatAtts : public XMLAttributes
{
public:
	ExpatAtts (const char ** atts) : _atts(atts) {}

	virtual int size () const;
	virtual const char * getName (int i) const;
	virtual const char * getValue (int i) const;

private:
	const char ** _atts;
};

int ExpatAtts::size () const
{
	int s = 0;
	for (int i = 0; _atts[i] != 0; i += 2)
		s++;
	return s;
}

const char *ExpatAtts::getName (int i) const
{
	return _atts[i*2];
}

const char *ExpatAtts::getValue (int i) const
{
	return _atts[i*2+1];
}


////////////////////////////////////////////////////////////////////////
// Static callback functions for Expat.
////////////////////////////////////////////////////////////////////////

#define VISITOR (*((XMLVisitor *)userData))

static void start_element (void * userData, const char * name, const char ** atts)
{
	VISITOR.startElement(name, ExpatAtts(atts));
}

static void end_element (void * userData, const char * name)
{
	VISITOR.endElement(name);
}

static void character_data (void * userData, const char * s, int len)
{
	VISITOR.data(s, len);
}

static void processing_instruction (void * userData,
			const char * target,
			const char * data)
{
	VISITOR.pi(target, data);
}

#undef VISITOR


////////////////////////////////////////////////////////////////////////
// Implementation of XMLReader.
////////////////////////////////////////////////////////////////////////

/**
 * Read and parse the XML from an istream.
 */
void readXML (istream &input, XMLVisitor &visitor, const string &path,
			  bool progress_callback(int))
{
	XML_Parser parser = XML_ParserCreate(0);
	XML_SetUserData(parser, &visitor);
	XML_SetElementHandler(parser, start_element, end_element);
	XML_SetCharacterDataHandler(parser, character_data);
	XML_SetProcessingInstructionHandler(parser, processing_instruction);

	visitor.startXML();

	int progress = 0;
	char buf[16384];
	while (!input.eof())
	{
		// FIXME: get proper error string from system
		if (!input.good())
		{
			int line = XML_GetCurrentLineNumber(parser);
			int col = XML_GetCurrentColumnNumber(parser);
			XML_ParserFree(parser);
			throw xh_io_exception("Problem reading file",
					xh_location(path, line, col), "XML Parser");
		}

		input.read(buf,16384);
		if (!XML_Parse(parser, buf, (int) input.gcount(), false))
		{
			const XML_LChar *message = XML_ErrorString(XML_GetErrorCode(parser));
			int line = XML_GetCurrentLineNumber(parser);
			int col = XML_GetCurrentColumnNumber(parser);
			XML_ParserFree(parser);
			throw xh_io_exception(message, xh_location(path, line, col),
					"XML Parser");
		}
		if (progress_callback != NULL)
		{
			progress++;
			if (progress == 400)
				progress = 0;
			progress_callback(progress/4);
		}
	}

	// Verify end of document.
	if (!XML_Parse(parser, buf, 0, true))
	{
		int line = XML_GetCurrentLineNumber(parser);
		int col = XML_GetCurrentColumnNumber(parser);
		XML_ParserFree(parser);
		throw xh_io_exception(XML_ErrorString(XML_GetErrorCode(parser)),
			xh_location(path, line, col), "XML Parser");
	}

	XML_ParserFree(parser);
}

/**
 * Read and parse the XML from a file.
 */
void readXML (const string &path_utf8, XMLVisitor &visitor,
			  bool progress_callback(int))
{
	FILE *fp;
#if WIN32
	// For Windows we need to convert the utf-8 path to either multi-byte
	//  (which might fail in some cases) or wide characters (wchar_t).
	// The latter is better.
	widestring fn;
	fn.from_utf8(path_utf8.c_str());
	fp = _wfopen(fn.c_str(), L"rb");
#elif __DARWIN_OSX__
	// Mac OS X already likes utf-8.
	fp = fopen(path_utf8.c_str(), "rb");
#else
	// some other Unix flavor
	widestring fn;
	fn.from_utf8(path_utf8.c_str());
	fp = fopen(fn.mb_str(), "rb");
#endif
	if (!fp)
		throw xh_io_exception("Failed to open file", xh_location(path_utf8),
					"XML Parser");

	// If it is not compressed, we can get the length of the file
	int iFileLength = -1;
	if (progress_callback != NULL)
	{
		const char *path_cstr = path_utf8.c_str();

		// Not compressed if: filename does not end with "gz"
		if (strncmp(path_cstr+path_utf8.length()-2, "gz", 2))
		{
			fseek(fp, 0, SEEK_END);
			iFileLength = ftell(fp);
		}
	}

	// From the zlib documentation:
	// "The duplicated descriptor should be saved to avoid a leak, since gzdopen
	// does not close fd if it fails. If you are using fileno() to get the file
	// descriptor from a FILE *, then you will have to use dup() to avoid double-
	// close()ing the file descriptor."
#ifdef _MSC_VER
	int fd = _fileno(fp);
	int fd_copy = _dup(fd);
#else
	int fd = fileno(fp);
	int fd_copy = dup(fd);
#endif

	// Ensure that underlying file descriptor (fd) is rewound
	lseek(fd_copy, 0, SEEK_SET);

	gzFile gfp = gzdopen(fd_copy, "rb");

	if (!gfp)
	{
		fclose(fp);
		throw xh_io_exception("Failed to open file", xh_location(path_utf8),
					"XML Parser");
	}
	try
	{
		readCompressedXML(gfp, visitor, path_utf8, iFileLength, progress_callback);
	}
	catch (xh_io_exception &e)
	{
			gzclose(gfp);
			fclose(fp);
			throw e;
	}
	catch (xh_throwable &t)
	{
			gzclose(gfp);
			fclose(fp);
			throw t;
	}
	// If it gets here, it succeeded
	gzclose(gfp);
	fclose(fp);
}

void readCompressedXML (gzFile fp, XMLVisitor &visitor, const string& path,
						int iFileLength, bool progress_callback(int))
{
	XML_Parser parser = XML_ParserCreate(0);
	XML_SetUserData(parser, &visitor);
	XML_SetElementHandler(parser, start_element, end_element);
	XML_SetCharacterDataHandler(parser, character_data);
	XML_SetProcessingInstructionHandler(parser, processing_instruction);

	visitor.startXML();

	const int BUFSIZE = 8192;
	int progress = 0;
	char buf[8192];
	while (!gzeof(fp))
	{
		int iError;
		gzerror(fp, &iError);
		int iCount = gzread(fp, buf, BUFSIZE);
		gzerror(fp, &iError);
		if (iCount > 0)
		{
			if (!XML_Parse(parser, buf, iCount, false))
			{
				const XML_LChar *message = XML_ErrorString(XML_GetErrorCode(parser));
				int line = XML_GetCurrentLineNumber(parser);
				int col = XML_GetCurrentColumnNumber(parser);
				XML_ParserFree(parser);
				throw xh_io_exception(message, xh_location(path, line, col),
					"XML Parser");
			}
			if (progress_callback != NULL)
			{
				if (iFileLength != -1)
				{
					// We know the length, so we can estimate total progress
					progress += iCount;
					// beware integer overflow, use doubles
					double prog = (float)progress * 99.0 / iFileLength;
					progress_callback((int) prog);
				}
				else
				{
					// Give a gradually advancing progress that wraps around
					progress++;
					if (progress == 400)
						progress = 0;
					progress_callback(progress/4);
				}
			}
		}
		else if (iCount < 0)
		{
			int line = XML_GetCurrentLineNumber(parser);
			int col = XML_GetCurrentColumnNumber(parser);
			XML_ParserFree(parser);
			throw xh_io_exception("Problem reading file",
					xh_location(path, line, col), "XML Parser");
		}
	}

	// Verify end of document.
	if (!XML_Parse(parser, buf, 0, true))
	{
		XML_Error errcode = XML_GetErrorCode(parser);
		const XML_LChar *errstr = XML_ErrorString(errcode);
		int line = XML_GetCurrentLineNumber(parser);
		int column = XML_GetCurrentColumnNumber(parser);

		XML_ParserFree(parser);

		throw xh_io_exception(errstr,
			xh_location(path, line, column), "XML Parser");
	}

	XML_ParserFree(parser);
}

