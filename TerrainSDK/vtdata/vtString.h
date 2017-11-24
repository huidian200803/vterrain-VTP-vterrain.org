//
// vtString.h
//
// Copyright (c) 2001-2009 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTSTRINGH
#define VTSTRINGH

#include <string.h>
#include <stdarg.h>
#include <string>
#include <vector>

#include "config_vtdata.h"

#if SUPPORT_WSTRING
class wstring2;
#endif

#ifdef WIN32
#  define WIN_UNIX_CDECL __cdecl
#else
   /*  UNIX doesn't have competing calling conventions to deal with  */
#  define WIN_UNIX_CDECL
#endif

#ifdef WIN32
#  define stricmp _stricmp   // MBCS/Unicode aware
#else
#  define stricmp strcasecmp
#endif

// pointer to const char
typedef char *pchar;
typedef const char *pcchar;
typedef const uchar *pcuchar;

struct vtStringData
{
	int nRefs;			// reference count
	int nDataLength;	// length of data (including terminator)
	int nAllocLength;	// length of allocation

	char* data()		// char* to managed data
		{ return (char*)(this+1); }
};

/**
 * This class encapsulates a simple string, much like the 'CString' of MFC,
 * or the 'wxString' of wxWidgets.
 */
class vtString
{
public:
// Constructors

	// constructs empty vtString
	vtString();
	// copy constructor
	vtString(const vtString& stringSrc);
	// from an ANSI string (converts to char)
	vtString(pcchar lpsz);
	// from ucharacters
	vtString(const uchar* psz);
	// subset of characters from an ANSI string (converts to char)
	vtString(pcchar lpch, int nLength);

	// Attributes & Operations

	// get data length
	int GetLength() const;
	// TRUE if zero length
	bool IsEmpty() const;
	// clear contents to empty
	void Clear();

	// return single character at zero-based index
	char GetAt(int nIndex) const;
	// return single character at zero-based index
	const char& operator[](int nIndex) const;
	// access single character at zero-based index
	char& operator[](int nIndex);
	// set a single character at zero-based index
	void SetAt(int nIndex, char ch);
	// return pointer to const string
	operator pcchar() const;

	// overloaded assignment

	// ref-counted copy from another vtString
	const vtString& operator=(const vtString& stringSrc);
	// copy string content from ANSI string (converts to char)
	const vtString& operator=(pcchar lpsz);
	// copy string content from uchars
	const vtString& operator=(const uchar* psz);

	// string concatenation

	// concatenate a ANSI string
	const vtString& operator+=(pcchar lpsz);
	// concatenate from another vtString
	const vtString& operator+=(const vtString& string);
	// concatenate a single character
	const vtString& operator+=(char ch);
	// concatenate from a buffer with length
	void Concat(pcchar buffer, size_t length);

	friend vtString operator+(const vtString& string1, const vtString& string2);
	friend vtString operator+(const vtString& string, char ch);
	friend vtString operator+(char ch, const vtString& string);
	friend vtString operator+(const vtString& string, pcchar lpsz);
	friend vtString operator+(pcchar lpsz, const vtString& string);

	// string comparison

	// straight character comparison
	int Compare(pcchar lpsz) const;
	// compare ignoring case
	int CompareNoCase(pcchar lpsz) const;
	// test if the string is an integer (with possible sign)
	bool IsNumber() const;
	// test if it matches a pattern which can include * and ?
	bool Matches(pcchar lpsz) const;

	// simple sub-string extraction

	// return nCount characters starting at zero-based nFirst
	vtString Mid(int nFirst, int nCount) const;
	// return all characters starting at zero-based nFirst
	vtString Mid(int nFirst) const;
	// return first nCount characters in string
	vtString Left(int nCount) const;
	// return nCount characters from end of string
	vtString Right(int nCount) const;

	// upper/lower/reverse conversion

	// NLS aware conversion to uppercase
	void MakeUpper();
	// NLS aware conversion to lowercase
	void MakeLower();
	// reverse string right-to-left
	void MakeReverse();

	// trimming whitespace (either side)

	// remove whitespace starting from right edge
	void TrimRight();
	// remove whitespace starting from left side
	void TrimLeft();

	// searching

	// find character starting at left, -1 if not found
	int Find(char ch) const;
	// find character starting at right
	int ReverseFind(char ch) const;
	// find character starting at zero-based index and going right
	int Find(char ch, int nStart) const;
	// find first instance of any character in passed string
	int FindOneOf(pcchar lpszCharSet) const;
	// find first instance of substring
	int Find(pcchar szSub) const;
	// find first instance of substring starting at zero-based index
	int Find(pcchar szSub, int nStart) const;

	// advanced manipulation

	// replace occurrences of chOld with chNew
	int Replace(char chOld, char chNew);
	// replace occurrences of strOld with strNew
	int Replace(const char *strOld, const char *strNew, bool bReplaceAll = false);

	// remove occurrences of chRemove
	int Remove(char chRemove);
	// insert character at zero-based index; concatenates
	// if index is past end of string
	int Insert(int nIndex, char ch);
	// insert substring at zero-based index; concatenates
	// if index is past end of string
	int Insert(int nIndex, pcchar pstr);
	// delete nCount characters starting at zero-based index
	int Delete(int nIndex, int nCount = 1);

	// simple formatting
	// printf-like formatting using passed string
	void WIN_UNIX_CDECL Format(pcchar lpszFormat, ...);
	// printf-like formatting using variable arguments parameter
	void FormatV(pcchar lpszFormat, va_list argList);
	// produce a string suitable for passing in a URL
	void FormatForURL(const char *szInput);
	vtString FormatForURL() const;

	// formatting for localization (uses FormatMessage API)
	// format using FormatMessage API on passed string
	void WIN_UNIX_CDECL FormatMessage(pcchar lpszFormat, ...);

	// Access to string implementation buffer as "C" character array
	// get pointer to modifiable buffer at least as long as nMinBufLength
	pchar GetBuffer(int nMinBufLength);
	// release buffer, setting length to nNewLength (or to first nul if -1)
	void ReleaseBuffer(int nNewLength = -1);
	// get pointer to modifiable buffer exactly as long as nNewLength
	pchar GetBufferSetLength(int nNewLength);
	// release memory allocated to but unused by string
	void FreeExtra();

	// Use LockBuffer/UnlockBuffer to turn refcounting off

	// turn refcounting back on
	pchar LockBuffer();
	// turn refcounting off
	void UnlockBuffer();

#if SUPPORT_WSTRING
	wstring2 UTF8ToWideString();
#endif
	vtString UTF8ToLocal();

// Implementation
public:
	~vtString();
	int GetAllocLength() const;

protected:
	pchar m_pchData;   // pointer to ref counted string data

	// implementation helpers
	vtStringData* GetData() const;
	void Init();
	void AllocCopy(vtString& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;
	void AllocBuffer(int nLen);
	void AssignCopy(int nSrcLen, pcchar szSrcData);
	void ConcatCopy(int nSrc1Len, pcchar szSrc1Data, int nSrc2Len, pcchar szSrc2Data);
	void ConcatInPlace(int nSrcLen, pcchar szSrcData);
	void CopyBeforeWrite();
	void AllocBeforeWrite(int nLen);
	void Release();
	static void Release(vtStringData* pData);
	static size_t SafeStrlen(pcchar lpsz);
	static void FreeData(vtStringData* pData);
	};

// Compare helpers
bool operator==(const vtString& s1, const vtString& s2);
bool operator==(const vtString& s1, pcchar s2);
bool operator==(pcchar s1, const vtString& s2);
bool operator!=(const vtString& s1, const vtString& s2);
bool operator!=(const vtString& s1, pcchar s2);
bool operator!=(pcchar s1, const vtString& s2);
bool operator<(const vtString& s1, const vtString& s2);
bool operator<(const vtString& s1, pcchar s2);
bool operator<(pcchar s1, const vtString& s2);
bool operator>(const vtString& s1, const vtString& s2);
bool operator>(const vtString& s1, pcchar s2);
bool operator>(pcchar s1, const vtString& s2);
bool operator<=(const vtString& s1, const vtString& s2);
bool operator<=(const vtString& s1, pcchar s2);
bool operator<=(pcchar s1, const vtString& s2);
bool operator>=(const vtString& s1, const vtString& s2);
bool operator>=(const vtString& s1, pcchar s2);
bool operator>=(pcchar s1, const vtString& s2);

// Globals
extern pcchar _vtPchNil;
#define vtEmptyString ((vtString&)*(vtString*)&_vtPchNil)

//
// vtString inlines
//
inline vtStringData* vtString::GetData() const
	{ return ((vtStringData*)m_pchData)-1; }
inline void vtString::Init()
	{ m_pchData = vtEmptyString.m_pchData; }
inline vtString::vtString()
	{ m_pchData = vtEmptyString.m_pchData; }
inline vtString::vtString(const uchar* lpsz)
	{ Init(); *this = (pcchar)lpsz; }
inline const vtString& vtString::operator=(const uchar* lpsz)
	{ *this = (pcchar)lpsz; return *this; }

inline int vtString::GetLength() const
	{ return GetData()->nDataLength; }
inline int vtString::GetAllocLength() const
	{ return GetData()->nAllocLength; }
inline bool vtString::IsEmpty() const
	{ return GetData()->nDataLength == 0; }
inline vtString::operator pcchar() const
	{ return m_pchData; }
inline size_t vtString::SafeStrlen(pcchar lpsz)
	{ return (lpsz == NULL) ? 0 : strlen(lpsz); }

// string comparison
inline int vtString::Compare(pcchar lpsz) const
	{ return strcmp(m_pchData, lpsz); }	// MBCS/Unicode aware
inline int vtString::CompareNoCase(pcchar lpsz) const
	{ return stricmp(m_pchData, lpsz); }

inline char vtString::GetAt(int nIndex) const
{
	return m_pchData[nIndex];
}
inline const char& vtString::operator[](int nIndex) const
{
	// same as GetAt
	return m_pchData[nIndex];
}
inline char& vtString::operator[](int nIndex)
{
	// same as GetAt
	return m_pchData[nIndex];
}
inline bool operator==(const vtString& s1, const vtString& s2)
	{ return s1.Compare(s2) == 0; }
inline bool operator==(const vtString& s1, pcchar s2)
	{ return s1.Compare(s2) == 0; }
inline bool operator==(pcchar s1, const vtString& s2)
	{ return s2.Compare(s1) == 0; }
inline bool operator!=(const vtString& s1, const vtString& s2)
	{ return s1.Compare(s2) != 0; }
inline bool operator!=(const vtString& s1, pcchar s2)
	{ return s1.Compare(s2) != 0; }
inline bool operator!=(pcchar s1, const vtString& s2)
	{ return s2.Compare(s1) != 0; }
inline bool operator<(const vtString& s1, const vtString& s2)
	{ return s1.Compare(s2) < 0; }
inline bool operator<(const vtString& s1, pcchar s2)
	{ return s1.Compare(s2) < 0; }
inline bool operator<(pcchar s1, const vtString& s2)
	{ return s2.Compare(s1) > 0; }
inline bool operator>(const vtString& s1, const vtString& s2)
	{ return s1.Compare(s2) > 0; }
inline bool operator>(const vtString& s1, pcchar s2)
	{ return s1.Compare(s2) > 0; }
inline bool operator>(pcchar s1, const vtString& s2)
	{ return s2.Compare(s1) < 0; }
inline bool operator<=(const vtString& s1, const vtString& s2)
	{ return s1.Compare(s2) <= 0; }
inline bool operator<=(const vtString& s1, pcchar s2)
	{ return s1.Compare(s2) <= 0; }
inline bool operator<=(pcchar s1, const vtString& s2)
	{ return s2.Compare(s1) >= 0; }
inline bool operator>=(const vtString& s1, const vtString& s2)
	{ return s1.Compare(s2) >= 0; }
inline bool operator>=(const vtString& s1, pcchar s2)
	{ return s1.Compare(s2) >= 0; }
inline bool operator>=(pcchar s1, const vtString& s2)
	{ return s2.Compare(s1) <= 0; }

// helpers
vtString EscapeStringForXML(const char *input);
void EscapeStringForXML(const std::string &input, std::string &output);

#if SUPPORT_WSTRING
void EscapeStringForXML(const std::wstring &input, std::string &output);
void EscapeStringForXML(const std::wstring &input, std::wstring &output);
#endif

vtString UTF8ToLocal(const char *string_utf8);


/////////////////////////////////////////////////////////////////////////////
// wstring2

#if SUPPORT_WSTRING

#define MAX_WSTRING2_SIZE 4096

/**
 * Another string class.  This one always stores string internally as 16-bit
 * wide characters, and can convert to and from other representations as
 * desired.
 *
 * Unlike the C++ standard "wstring" class, on which it is based, this class
 * can convert to and from UTF8 and default encoded 8-bit character strings.
 *
 * Unlike wxString, this class always uses wide characters, so it does not
 * need to be compiled in two flavors.  It also avoids a dependency on all
 * of wxWidgets as well..
 *
 * Unlike MFC's CString, this class actually stores wide characters rather
 * than messy, multi-byte variable encodings.
 */
class wstring2 : public std::wstring
{
public:
	wstring2();
	wstring2(const wchar_t *__s);
	wstring2(const char *__s);
	const char *mb_str() const;		// multibyte string in local encoding

	size_t from_utf8(const char *in);
	const char *to_utf8() const;

private:
	static char s_buffer[MAX_WSTRING2_SIZE];
};

#endif // SUPPORT_WSTRING


/**
 * vtStringArray class: an array of vtString objects.
 */
#define vtStringArray std::vector<vtString>

/** Useful function which wraps the C standard library's strtok */
void vtTokenize(char *buf, const char *delim, vtStringArray &tokens);

/** Find a string in a string array, return index or -1 if not found */
int vtFindString(const vtStringArray &arr, const char *find);

/** Concatenate a string array into a string, with a divider character */
vtString vtConcatArray(const vtStringArray &arr, const char delim);

/** Extract a string array from a string by divider (ie. tokenize) */
void vtExtractArray(const vtString &input, vtStringArray &arr, const char delim);

#endif	// VTSTRINGH

