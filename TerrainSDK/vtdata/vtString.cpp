//
// vtString.cpp
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtString.h"
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <wchar.h>
#include <algorithm>	// for std::min/max

#define InterlockInc(i) (*(i))++
#define InterlockDec(i) --(*(i))

/////////////////////////////////////////////////////////////////////////////
// static class data, special inlines

// vtChNil is left for backward compatibility
static char vtChNil = '\0';

// For an empty string, m_pchData will point here
// (note: avoids special case of checking for NULL m_pchData)
// empty string data (and locked)
static int _vtInitData[] = { -1, 0, 0, 0 };
static vtStringData* _vtDataNil = (vtStringData*)&_vtInitData;
pcchar _vtPchNil = (pcchar)(((uchar*)&_vtInitData)+sizeof(vtStringData));


//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction

vtString::vtString(const vtString& stringSrc)
{
	if (stringSrc.GetData()->nRefs >= 0)
	{
		m_pchData = stringSrc.m_pchData;
		InterlockInc(&GetData()->nRefs);
	}
	else
	{
		Init();
		*this = stringSrc.m_pchData;
	}
}

// construct from subset of characters from an ANSI string (converts to char)
vtString::vtString(pcchar lpch, int nLength)
{
	Init();
	if (nLength != 0)
	{
		AllocBuffer(nLength);
		memcpy(m_pchData, lpch, nLength*sizeof(char));
	}
}

void vtString::AllocBuffer(int nLen)
// always allocate one extra character for '\0' termination
// assumes [optimistically] that data length will equal allocation length
{
	if (nLen == 0)
		Init();
	else
	{
		vtStringData* pData;
		pData = (vtStringData*)
			new uchar[sizeof(vtStringData) + (nLen+1)*sizeof(char)];
		pData->nAllocLength = nLen;
		pData->nRefs = 1;
		pData->data()[nLen] = '\0';
		pData->nDataLength = nLen;
		m_pchData = pData->data();
	}
}

void vtString::FreeData(vtStringData* pData)
{
	delete[] (uchar*)pData;
}

void vtString::Release()
{
	if (GetData() != _vtDataNil)
	{
		if (InterlockDec(&GetData()->nRefs) <= 0)
			FreeData(GetData());
		Init();
	}
}

void vtString::Release(vtStringData* pData)
{
	if (pData != _vtDataNil)
	{
		if (InterlockDec(&pData->nRefs) <= 0)
			FreeData(pData);
	}
}

void vtString::Clear()
{
	if (GetData()->nDataLength == 0)
		return;
	if (GetData()->nRefs >= 0)
		Release();
	else
		*this = &vtChNil;
}

void vtString::CopyBeforeWrite()
{
	if (GetData()->nRefs > 1)
	{
		vtStringData* pData = GetData();
		Release();
		AllocBuffer(pData->nDataLength);
		memcpy(m_pchData, pData->data(), (pData->nDataLength+1)*sizeof(char));
	}
}

void vtString::AllocBeforeWrite(int nLen)
{
	if (GetData()->nRefs > 1 || nLen > GetData()->nAllocLength)
	{
		Release();
		AllocBuffer(nLen);
	}
}

//
//  free any attached data
//
vtString::~vtString()
{
	if (GetData() != _vtDataNil) {
		if (InterlockDec(&GetData()->nRefs) <= 0)
			FreeData(GetData());
	}
}

//////////////////////////////////////////////////////////////////////////////
// Helpers for the rest of the implementation

void vtString::AllocCopy(vtString& dest, int nCopyLen, int nCopyIndex,
	 int nExtraLen) const
{
	// will clone the data attached to this string
	// allocating 'nExtraLen' characters
	// Places results in uninitialized string 'dest'
	// Will copy the part or all of original data to start of new string

	int nNewLen = nCopyLen + nExtraLen;
	if (nNewLen == 0)
	{
		dest.Init();
	}
	else
	{
		dest.AllocBuffer(nNewLen);
		memcpy(dest.m_pchData, m_pchData+nCopyIndex, nCopyLen*sizeof(char));
	}
}

//////////////////////////////////////////////////////////////////////////////
// More sophisticated construction

vtString::vtString(pcchar szStr)
{
	Init();
	int nLen = SafeStrlen(szStr);
	if (nLen != 0)
	{
		AllocBuffer(nLen);
		memcpy(m_pchData, szStr, nLen*sizeof(char));
	}
}


//////////////////////////////////////////////////////////////////////////////
// Assignment operators
//  All assign a new value to the string
//	  (a) first see if the buffer is big enough
//	  (b) if enough room, copy on top of old buffer, set size and type
//	  (c) otherwise free old string data, and create a new one
//
//  All routines return the new string (but as a 'const vtString&' so that
//	  assigning it again will cause a copy, eg: s1 = s2 = "hi there".
//

void vtString::AssignCopy(int nSrcLen, pcchar szSrcData)
{
	AllocBeforeWrite(nSrcLen);
	memcpy(m_pchData, szSrcData, nSrcLen*sizeof(char));
	GetData()->nDataLength = nSrcLen;
	m_pchData[nSrcLen] = '\0';
}

const vtString& vtString::operator=(const vtString& stringSrc)
{
	if (m_pchData != stringSrc.m_pchData)
	{
		if ((GetData()->nRefs < 0 && GetData() != _vtDataNil) ||
			stringSrc.GetData()->nRefs < 0)
		{
			// actual copy necessary since one of the strings is locked
			AssignCopy(stringSrc.GetData()->nDataLength, stringSrc.m_pchData);
		}
		else
		{
			// can just copy references around
			Release();
			m_pchData = stringSrc.m_pchData;
			InterlockInc(&GetData()->nRefs);
		}
	}
	return *this;
}

const vtString& vtString::operator=(pcchar lpsz)
{
	AssignCopy(SafeStrlen(lpsz), lpsz);
	return *this;
}


//////////////////////////////////////////////////////////////////////////////
// concatenation

// NOTE: "operator+" is done as friend functions for simplicity
//	  There are three variants:
//		  vtString + vtString
// and for ? = char, pcchar
//		  vtString + ?
//		  ? + vtString

void vtString::ConcatCopy(int nSrc1Len, pcchar szSrc1Data,
	int nSrc2Len, pcchar szSrc2Data)
{
  // -- master concatenation routine
  // Concatenate two sources
  // -- assume that 'this' is a new vtString object

	int nNewLen = nSrc1Len + nSrc2Len;
	if (nNewLen != 0)
	{
		AllocBuffer(nNewLen);
		memcpy(m_pchData, szSrc1Data, nSrc1Len*sizeof(char));
		memcpy(m_pchData+nSrc1Len, szSrc2Data, nSrc2Len*sizeof(char));
	}
}

vtString operator+(const vtString& string1, const vtString& string2) {
	vtString s;
	s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData,
		string2.GetData()->nDataLength, string2.m_pchData);
	return s;
}

vtString operator+(const vtString& string, pcchar lpsz) {
	vtString s;
	s.ConcatCopy(string.GetData()->nDataLength, string.m_pchData,
		vtString::SafeStrlen(lpsz), lpsz);
	return s;
}

vtString operator+(pcchar lpsz, const vtString& string) {
	vtString s;
	s.ConcatCopy(vtString::SafeStrlen(lpsz), lpsz, string.GetData()->nDataLength,
		string.m_pchData);
	return s;
}

//////////////////////////////////////////////////////////////////////////////
// concatenate in place

void vtString::ConcatInPlace(int nSrcLen, pcchar szSrcData)
{
	//  -- the main routine for += operators

	// concatenating an empty string is a no-op!
	if (nSrcLen == 0)
		return;

	// if the buffer is too small, or we have a width mis-match, just
	//   allocate a new buffer (slow but sure)
	if (GetData()->nRefs > 1 || GetData()->nDataLength + nSrcLen > GetData()->nAllocLength)
	{
		// we have to grow the buffer, use the ConcatCopy routine
		vtStringData* pOldData = GetData();
		ConcatCopy(GetData()->nDataLength, m_pchData, nSrcLen, szSrcData);
		vtString::Release(pOldData);
	}
	else
	{
		// fast concatenation when buffer big enough
		memcpy(m_pchData+GetData()->nDataLength, szSrcData, nSrcLen*sizeof(char));
		GetData()->nDataLength += nSrcLen;
		m_pchData[GetData()->nDataLength] = '\0';
	}
}

const vtString& vtString::operator+=(pcchar lpsz)
{
	ConcatInPlace(SafeStrlen(lpsz), lpsz);
	return *this;
}

const vtString& vtString::operator+=(const vtString& string)
{
	ConcatInPlace(string.GetData()->nDataLength, string.m_pchData);
	return *this;
}

const vtString& vtString::operator+=(char ch)
{
	ConcatInPlace(1, &ch);
	return *this;
}

void vtString::Concat(pcchar buffer, size_t length)
{
	ConcatInPlace(length, buffer);
}

//////////////////////////////////////////////////////////////////////////////
// less common string expressions

vtString operator+(const vtString& string1, char ch)
{
	vtString s;
	s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData, 1, &ch);
	return s;
}

vtString operator+(char ch, const vtString& string)
{
	vtString s;
	s.ConcatCopy(1, &ch, string.GetData()->nDataLength, string.m_pchData);
	return s;
}


///////////////////////////////////////////////////////////////////////////////
// String comparison and Testing

bool vtString::IsNumber() const
{
	const char *s = (const char *) m_pchData;

	if ((s[0] == '-') || (s[0] == '+'))
		s++;
	while (*s)
	{
		if (!isdigit((uchar) (*s))) return false;
			s++;
	}
	return true;
}

bool vtString::Matches(pcchar lpsz) const
{
	const char *string = (const char *) m_pchData;
	const char *wild = lpsz;

	const char *cp=0, *mp=0;	// set to 0 to avoid compiler warning

	while ((*string) && (*wild != '*'))
	{
		if ((*wild != *string) && (*wild != '?'))
			return false;

		wild++;
		string++;
	}
	while (*string)
	{
		if (*wild == '*')
		{
			if (!*++wild)
				return true;

			mp = wild;
			cp = string+1;
		}
		else if ((*wild == *string) || (*wild == '?'))
		{
			wild++;
			string++;
		}
		else
		{
			wild = mp;
			string = cp++;
		}
	}
	while (*wild == '*')
		wild++;

	return !*wild;
}

///////////////////////////////////////////////////////////////////////////////
// Advanced direct buffer access

pchar vtString::GetBuffer(int nMinBufLength)
{
	if (GetData()->nRefs > 1 || nMinBufLength > GetData()->nAllocLength)
	{
		// we have to grow the buffer
		vtStringData* pOldData = GetData();
		int nOldLen = GetData()->nDataLength;   // AllocBuffer will tromp it
		if (nMinBufLength < nOldLen)
			nMinBufLength = nOldLen;
		AllocBuffer(nMinBufLength);
		memcpy(m_pchData, pOldData->data(), (nOldLen+1)*sizeof(char));
		GetData()->nDataLength = nOldLen;
		vtString::Release(pOldData);
	}

	// return a pointer to the character storage for this string
	return m_pchData;
}

void vtString::ReleaseBuffer(int nNewLength)
{
	CopyBeforeWrite();  // just in case GetBuffer was not called

	if (nNewLength == -1)
		nNewLength = strlen(m_pchData); // zero terminated

	GetData()->nDataLength = nNewLength;
	m_pchData[nNewLength] = '\0';
}

pchar vtString::GetBufferSetLength(int nNewLength)
{
	GetBuffer(nNewLength);
	GetData()->nDataLength = nNewLength;
	m_pchData[nNewLength] = '\0';
	return m_pchData;
}

void vtString::FreeExtra()
{
	if (GetData()->nDataLength != GetData()->nAllocLength)
	{
		vtStringData* pOldData = GetData();
		AllocBuffer(GetData()->nDataLength);
		memcpy(m_pchData, pOldData->data(), pOldData->nDataLength*sizeof(char));
		vtString::Release(pOldData);
	}
}

pchar vtString::LockBuffer()
{
	pchar lpsz = GetBuffer(0);
	GetData()->nRefs = -1;
	return lpsz;
}

void vtString::UnlockBuffer()
{
	if (GetData() != _vtDataNil)
		GetData()->nRefs = 1;
}

///////////////////////////////////////////////////////////////////////////////
// Commonly used routines (rarely used routines in STREX.CPP)

int vtString::Find(char ch) const
{
	return Find(ch, 0);
}

int vtString::Find(char ch, int nStart) const
{
	int nLength = GetData()->nDataLength;
	if (nStart >= nLength)
		return -1;

	// find first single character
	pchar lpsz = strchr(m_pchData + nStart, (uchar)ch);

	// return -1 if not found and index otherwise
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

int vtString::FindOneOf(pcchar lpszCharSet) const
{
	pchar lpsz = strpbrk(m_pchData, lpszCharSet);
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

void vtString::MakeUpper()
{
	CopyBeforeWrite();
#ifdef WIN32
	_strupr(m_pchData);
#else
	for (char *p = m_pchData; *p; p++ )
		*p = toupper(*p);
#endif
}

void vtString::MakeLower()
{
	CopyBeforeWrite();
#ifdef WIN32
	_strlwr(m_pchData);
#else
	for (char *p = m_pchData; *p; p++ )
		*p = tolower(*p);
#endif
}

void vtString::MakeReverse()
{
	CopyBeforeWrite();
#ifdef WIN32
	_strrev(m_pchData);
#else
	{
		int len = strlen( m_pchData );
		for ( int i = len / 2 - 1; i >= 0; i-- ) {
			char tmp = m_pchData[i];
			m_pchData[  i  ] = m_pchData[len-1-i];
			m_pchData[len-1-i] = tmp;
		}
	}
#endif
}


void vtString::SetAt(int nIndex, char ch)
{
	CopyBeforeWrite();
	m_pchData[nIndex] = ch;
}

// formatting (using wsprintf style formatting)
void WIN_UNIX_CDECL vtString::Format(pcchar lpszFormat, ...) {
	va_list argList;
	va_start(argList, lpszFormat);
	FormatV(lpszFormat, argList);
	va_end(argList);
}

void vtString::FormatV(pcchar lpszFormat, va_list argList)
{
#ifdef _MSC_VER
	va_list argListSave = argList;
#else
	// probably gcc, which has the newer standard "va_copy" macro
	va_list argListSave;
	va_copy(argListSave, argList);
#endif

	// make a guess at the maximum length of the resulting string
	int nMaxLen = 0;
	for (pcchar lpsz = lpszFormat; *lpsz != '\0'; lpsz++)
	{
		// handle '%' character, but watch out for '%%'
		if (*lpsz != '%' || *(++lpsz) == '%')
		{
			nMaxLen++;
			continue;
		}

		int nItemLen = 0;

		// handle '%' character with format
		int nWidth = 0;
		for (; *lpsz != '\0'; lpsz++)
		{
			// check for valid flags
			if (*lpsz == '#')
				nMaxLen += 2;   // for '0x'
			else if (*lpsz == '*')
				nWidth = va_arg(argList, int);
			else if (*lpsz == '-' || *lpsz == '+' || *lpsz == '0' ||
				*lpsz == ' ')
				;
			else // hit non-flag character
				break;
		}
		// get width and skip it
		if (nWidth == 0)
		{
			// width indicated by
			nWidth = atoi(lpsz);
			for (; *lpsz != '\0' && isdigit(*lpsz); lpsz++)
				;
		}
		int nPrecision = 0;
		if (*lpsz == '.')
		{
			// skip past '.' separator (width.precision)
			lpsz++;

			// get precision and skip it
			if (*lpsz == '*')
			{
				nPrecision = va_arg(argList, int);
				lpsz++;
			}
			else
			{
				nPrecision = atoi(lpsz);
				for (; *lpsz != '\0' && isdigit(*lpsz); lpsz++)
					;
			}
		}

		// should be on type modifier or specifier
		if (strncmp(lpsz, "I64", 3) == 0)
		{
			lpsz += 3;
		}
		else
		{
			switch (*lpsz)
			{
			// modifiers that affect size
			case 'h':
				lpsz++;
				break;
			case 'l':
				lpsz++;
				break;

			// modifiers that do not affect size
			case 'F':
			case 'N':
			case 'L':
				lpsz++;
				break;
			}
		}

		// now should be on specifier
		switch (*lpsz)
		{
		// single characters
		case 'c':
		case 'C':
			nItemLen = 2;
			va_arg(argList, int);
			break;
		// strings
		  case 's':
			{
				const char *pstrNextArg = va_arg(argList, pcchar);
				if (pstrNextArg == NULL)
				   nItemLen = 6;  // "(null)"
				else
				{
				   nItemLen = strlen(pstrNextArg);
				   nItemLen = std::max(1, nItemLen);
				}
			}
			break;
		  case 'S':
				/*  FIXME:  This case should do wchar_t *'s  */
				const char *pstrNextArg = va_arg(argList, pcchar);
				if (pstrNextArg == NULL)
				   nItemLen = 6; // "(null)"
				else
				{
				   nItemLen = strlen(pstrNextArg);
				   nItemLen = std::max(1, nItemLen);
				}
			break;
		}

		// adjust nItemLen for strings
		if (nItemLen != 0) {
			if (nPrecision != 0)
				nItemLen = std::min(nItemLen, nPrecision);
			nItemLen = std::max(nItemLen, nWidth);
		}
		else
		{
			switch (*lpsz)
			{
			// integers
			case 'd':
			case 'i':
			case 'u':
			case 'x':
			case 'X':
			case 'o':
				va_arg(argList, int);
				nItemLen = 32;
				nItemLen = std::max(nItemLen, nWidth+nPrecision);
				break;

			case 'e':
			case 'g':
			case 'G':
				va_arg(argList, double);
				nItemLen = 128;
				nItemLen = std::max(nItemLen, nWidth+nPrecision);
				break;

			case 'f':
				va_arg(argList, double);
				nItemLen = 128; // width isn't truncated
				// 312 == strlen("-1+(309 zeroes).")
				// 309 zeroes == max precision of a double
				nItemLen = std::max(nItemLen, 312+nPrecision);
				break;

			case 'p':
				va_arg(argList, void*);
				nItemLen = 32;
				nItemLen = std::max(nItemLen, nWidth+nPrecision);
				break;

			// no output
			case 'n':
				va_arg(argList, int*);
				break;

			default:
				;
			}
		}

		// adjust nMaxLen for output nItemLen
		nMaxLen += nItemLen;
	}

	GetBuffer(nMaxLen);
	vsprintf(m_pchData, lpszFormat, argListSave);
	ReleaseBuffer();

	va_end(argListSave);
}

void vtString::TrimRight()
{
	// find beginning of trailing spaces by starting at beginning
	CopyBeforeWrite();

	// beware trouble with signed characters being cast to negative ints
	uchar *lpsz = (uchar *) m_pchData;
	uchar *lpszLast = NULL;

	while (*lpsz != '\0')
	{
		if (isspace(*lpsz))
		{
			if (lpszLast == NULL)
				lpszLast = lpsz;
		}
		else
			lpszLast = NULL;
		lpsz++;
	}

	if (lpszLast != NULL)
	{
		// truncate at trailing space start
		*lpszLast = '\0';
		GetData()->nDataLength = lpszLast - (uchar *) m_pchData;
	}
}


void vtString::TrimLeft()
{
	// find first non-space character
	CopyBeforeWrite();
	char *lpsz = m_pchData;

	while (isspace(*lpsz))
		lpsz++;

	if (lpsz != m_pchData)
	{
		// fix up data and length
		int nDataLength = GetData()->nDataLength - (lpsz - m_pchData);
		memmove(m_pchData, lpsz, (nDataLength+1)*sizeof(char));
		GetData()->nDataLength = nDataLength;
	}
}

//////////////////////////////////////////////////////////////////////////////
// Very simple sub-string extraction

vtString vtString::Mid(int nFirst) const
{
	return Mid(nFirst, GetData()->nDataLength - nFirst);
}

vtString vtString::Mid(int nFirst, int nCount) const
{
	// out-of-bounds requests return sensible things
	if (nFirst < 0)
		nFirst = 0;
	if (nCount < 0)
		nCount = 0;

	if (nFirst + nCount > GetData()->nDataLength)
		nCount = GetData()->nDataLength - nFirst;
	if (nFirst > GetData()->nDataLength)
		nCount = 0;

	// optimize case of returning entire string
	if (nFirst == 0 && nFirst + nCount == GetData()->nDataLength)
		return *this;

	vtString dest;
	AllocCopy(dest, nCount, nFirst, 0);
	return dest;
}

vtString vtString::Right(int nCount) const
{
	if (nCount < 0)
		nCount = 0;
	if (nCount >= GetData()->nDataLength)
		return *this;

	vtString dest;
	AllocCopy(dest, nCount, GetData()->nDataLength-nCount, 0);
	return dest;
}

vtString vtString::Left(int nCount) const
{
	if (nCount < 0)
		nCount = 0;
	if (nCount >= GetData()->nDataLength)
		return *this;

	vtString dest;
	AllocCopy(dest, nCount, 0, 0);
	return dest;
}


//////////////////////////////////////////////////////////////////////////////
// Finding

int vtString::ReverseFind(char ch) const
{
	// find last single character
	const char *lpsz = strrchr(m_pchData, ch);

	// return -1 if not found, distance from beginning otherwise
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

// find a sub-string (like strstr)
int vtString::Find(pcchar szSub) const
{
	return Find(szSub, 0);
}

int vtString::Find(pcchar szSub, int nStart) const
{
	int nLength = GetData()->nDataLength;
	if (nStart > nLength)
		return -1;

	// find first matching substring
	const char *lpsz = strstr(m_pchData + nStart, szSub);

	// return -1 for not found, distance from beginning otherwise
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}


// advanced manipulation

// replace occurrences of chOld with chNew
int vtString::Replace(char chOld, char chNew)
{
	int nCount = 0;

	// short-circuit the nop case
	if (chOld != chNew)
	{
		char *pszBuffer = m_pchData;

		// otherwise modify each character that matches in the string
		bool bCopied = false;
		int nLength = GetLength();
		;
		for (int iChar = 0; iChar < nLength; iChar ++)
		{
			// replace instances of the specified character only
			if( pszBuffer[iChar] == chOld )
			{
				if( !bCopied )
				{
					bCopied = true;
					CopyBeforeWrite();
				}
				pszBuffer[iChar] = chNew;
				nCount++;
			}
		}
	}
	return nCount;
}

// replace occurrences of strOld with strNew
int vtString::Replace(const char *strOld, const char *strNew, bool bReplaceAll)
{
	int iCount = 0;   // count of replacements made

	char *pszBuffer = m_pchData;

	size_t uiOldLen = strlen(strOld);
	size_t uiNewLen = strlen(strNew);

	size_t dwPos = 0;

	while ( pszBuffer[dwPos] != 0 )
	{
		char *result = strstr(pszBuffer + dwPos, strOld);
		if ( result == NULL )
			break;                  // exit the loop
		else
		{
			dwPos = result - pszBuffer;

			//replace this occurance of the old string with the new one
			*this = Left(dwPos) + strNew + Right(GetLength() - dwPos - uiOldLen);

			//move up pos past the string that was replaced
			dwPos += uiNewLen;

			pszBuffer = m_pchData;

			//increase replace count
			++iCount;

			// stop now?
			if ( !bReplaceAll )
				break;                  // exit the loop
		}
	}

	return iCount;
}

// remove occurrences of chRemove
int vtString::Remove(char chRemove)
{
	CopyBeforeWrite();

	char *pstrSource = m_pchData;
	char *pstrDest = m_pchData;
	char *pstrEnd = m_pchData + GetData()->nDataLength;

	while (pstrSource < pstrEnd)
	{
		if (*pstrSource != chRemove)
		{
			*pstrDest = *pstrSource;
			pstrDest++;
		}
		pstrSource++;
	}
	*pstrDest = '\0';
	int nCount = pstrSource - pstrDest;
	GetData()->nDataLength -= nCount;

	return nCount;
}

#if 0
// insert character at zero-based index; concatenates
// if index is past end of string
int vtString::Insert(int nIndex, char ch)
{
	int nCount = 0;
	return nCount;
}

// insert substring at zero-based index; concatenates
// if index is past end of string
int vtString::Insert(int nIndex, pcchar pstr)
{
	int nCount = 0;
	return nCount;
}
#endif

// delete nCount characters starting at zero-based index
int vtString::Delete(int iIndex, int nCount)
{
	// check bounds first
	if (iIndex < 0)
		iIndex = 0;

	if (nCount < 0)
		nCount = 0;

	int nLength = GetLength();
	if ((nCount+iIndex) > nLength)
	{
		nCount = nLength-iIndex;
	}

	// now actually remove characters
	if (nCount > 0)
	{
		CopyBeforeWrite();

		int nCharsToCopy = nLength-(iIndex+nCount)+1;
		memmove( m_pchData+iIndex, m_pchData+iIndex+nCount, nCharsToCopy );

		// BD added to fix a bug where the end of string is moved in, but vtString
		//  still thinks the string is full length.
		GetData()->nDataLength -= nCount;
	}
	return GetLength();
}

void vtString::FormatForURL(const char *szInput)
{
	*this = "";
	for (const char *c = szInput; *c; c++)
	{
		switch (*c)
		{
		case '\r':
			break;
		case '\n':
			*this += '+';
			break;
		case ' ':
			*this += "%20";
			break;
		case ',':
			*this += "%2C";
			break;
		case '#':
			*this += "%23";
			break;
		//case ':':
		//	*this += "%3A";
		//	break;
		case '\\':
			*this += '/';
			break;
		default:
			*this += *c;
		}
	}
}

vtString vtString::FormatForURL() const
{
	vtString str;
	str.FormatForURL(m_pchData);
	return str;
}


//////////////////////////////////////////////////////////////////////////////
// Unicode / UTF8 support

#if SUPPORT_WSTRING

wstring2 vtString::UTF8ToWideString()
{
	wstring2 ws;
	ws.from_utf8(m_pchData);
	return ws;
}

/**
 * For a vtString which contains a UTF-8 encoded string, attempt to convert it
 * to a string in the local (current locale) character set.
 */
vtString vtString::UTF8ToLocal()
{
	// first make wide string
	wstring2 ws;
	ws.from_utf8(m_pchData);

	// get ready for conversion
	int len = ws.length();
	const wchar_t *cstr = ws.c_str();
	mbstate_t       mbstate;
	memset(&mbstate, '\0', sizeof (mbstate));

	// then convert it to a (local encoding) multi-byte string
	vtString str;
	char *target = str.GetBufferSetLength(len);
	int count = wcsrtombs(target, &cstr, len+1, &mbstate);
	return str;
}

vtString UTF8ToLocal(const char *string_utf8)
{
	// Safety check
	if (!string_utf8)
		return vtString("");

	// First make wide string
	wstring2 ws;
	ws.from_utf8(string_utf8);

	// Then use mb_str to convert to local encoding
	vtString str = ws.mb_str();

#if 0
	// In theory, this code should be better, because it does not rely on
	//  the fixed-size static buffer in wstring2::mb_str(), but wcsrtombs
	//  does not behave well.
	int len = ws.length();
	const wchar_t *cstr = ws.c_str();
	mbstate_t       mbstate;
	memset(&mbstate, '\0', sizeof (mbstate));

	// then convert it to a (local encoding) multi-byte string
	vtString str;
	char *target = str.GetBufferSetLength(len);
	int count = wcsrtombs(target, &cstr, len+1, &mbstate);
#endif
	return str;
}

#else
// Fallback for non-WSTRING case: we just hope our string is UTF8 compatible!
vtString vtString::UTF8ToLocal()
{
	return *this;
}
vtString UTF8ToLocal(const char *string_utf8)
{
	return vtString(string_utf8);
}
#endif	// SUPPORT_WSTRING


/////////////////////////////////////////////////////////////////////////////

vtString EscapeStringForXML(const char *input)
{
	vtString output;
	const char *p1 = input;
	for (; ('\0' != *p1); p1++)
	{
		switch (*p1)
		{
		case '<':
			output += "&lt;";
			break;
		case '&':
			output += "&amp;";
			break;
		case '>':
			output += "&gt;";
			break;
		case '"':
			output += "&quot;";
			break;
		case '\'':
			output += "&apos;";
			break;
		default:
			output += *p1;
		}
	}
	return output;
}

void EscapeStringForXML(const std::string &input, std::string &output)
{
	output = "";
	const char *p1 = input.c_str();
	for (; ('\0' != *p1); p1++)
	{
		switch (*p1)
		{
		case '<':
			output += "&lt;";
			break;
		case '&':
			output += "&amp;";
			break;
		case '>':
			output += "&gt;";
			break;
		case '"':
			output += "&quot;";
			break;
		case '\'':
			output += "&apos;";
			break;
		default:
			output += *p1;
		}
	}
}

#if SUPPORT_WSTRING

void EscapeStringForXML(const std::wstring &input, std::string &output)
{
	output = "";
	char cbuf[3];
	for (const wchar_t *p1 = input.c_str(); *p1 != L'\0'; p1++)
	{
		switch (*p1)
		{
		case L'<':
			output += "&lt;";
			break;
		case L'&':
			output += "&amp;";
			break;
		case L'>':
			output += "&gt;";
			break;
		case L'"':
			output += "&quot;";
			break;
		case L'\'':
			output += "&apos;";
			break;
		default:
			// wide character to multi-byte
			wctomb(cbuf, *p1);
			output += *cbuf;
		}
	}
}

void EscapeStringForXML(const std::wstring &input, std::wstring &output)
{
	output = L"";
	const wchar_t *p1 = input.c_str();
	for (; (L'\0' != *p1); p1++)
	{
		switch (*p1)
		{
		case L'<':
			output += L"&lt;";
			break;
		case L'&':
			output += L"&amp;";
			break;
		case L'>':
			output += L"&gt;";
			break;
		case L'"':
			output += L"&quot;";
			break;
		case L'\'':
			output += L"&apos;";
			break;
		default:
			output += *p1;
		}
	}
}

#endif // SUPPORT_WSTRING


/////////////////////////////////////////////////////////////////////////////
// wstring2

#if SUPPORT_WSTRING

using namespace std;

char wstring2::s_buffer[MAX_WSTRING2_SIZE];

wstring2::wstring2() : wstring()
{
}

wstring2::wstring2(const wchar_t *__s) : wstring(__s)
{
}

wstring2::wstring2(const char *__s)
{
	int len = strlen(__s);
	wchar_t *tmp = new wchar_t[len*2+1];
	mbstowcs(tmp, __s, len+1);
	// now copy the result into our own storage
	*((wstring*)this) = tmp;
	// and get rid of the temporary buffer
	delete [] tmp;
}

const char *wstring2::mb_str() const
{
	const wchar_t *guts = c_str();
	size_t result = wcstombs(s_buffer, guts, MAX_WSTRING2_SIZE);
	return s_buffer;
}

#define WC_UTF16

static size_t encode_utf16(uint input, wchar_t *output)
{
	if (input<=0xffff)
	{
		if (output) *output++ = (wchar_t) input;
		return 1;
	}
	else if (input>=0x110000)
	{
		return (size_t)-1;
	}
	else
	{
		if (output)
		{
			*output++ = (wchar_t) ((input >> 10)+0xd7c0);
			*output++ = (wchar_t) ((input&0x3ff)+0xdc00);
		}
		return 2;
	}
}

static size_t decode_utf16(const wchar_t *input, uint &output)
{
	if ((*input<0xd800) || (*input>0xdfff))
	{
		output = *input;
		return 1;
	}
	else if ((input[1]<0xdc00) || (input[1]>=0xdfff))
	{
		output = *input;
		return (size_t)-1;
	}
	else
	{
		output = ((input[0] - 0xd7c0) << 10) + (input[1] - 0xdc00);
		return 2;
	}
}

static uint utf8_max[]=
	{ 0x7f, 0x7ff, 0xffff, 0x1fffff, 0x3ffffff, 0x7fffffff, 0xffffffff };

size_t wstring2::from_utf8(const char *psz)
{
	size_t safe_length = strlen(psz)+1;
	wchar_t *tmp = new wchar_t[safe_length*2];
	wchar_t *buf = tmp;
	size_t n = safe_length;

	size_t len = 0;

	while (*psz && ((!buf) || (len < n)))
	{
		uchar cc = *psz++, fc = cc;
		unsigned cnt;
		for (cnt = 0; fc & 0x80; cnt++)
			fc <<= 1;
		if (!cnt)
		{
			// plain ASCII char
			if (buf)
				*buf++ = cc;
			len++;
		}
		else
		{
			cnt--;
			if (!cnt)
			{
				// invalid UTF-8 sequence
				return (size_t)-1;
			}
			else
			{
				unsigned ocnt = cnt - 1;
				uint res = cc & (0x3f >> cnt);
				while (cnt--)
				{
					cc = *psz++;
					if ((cc & 0xC0) != 0x80)
					{
						// invalid UTF-8 sequence
						return (size_t)-1;
					}
					res = (res << 6) | (cc & 0x3f);
				}
				if (res <= utf8_max[ocnt])
				{
					// illegal UTF-8 encoding
					return (size_t)-1;
				}
				size_t pa = encode_utf16(res, buf);
				if (pa == (size_t)-1)
				  return (size_t)-1;
				if (buf)
					buf += pa;
				len += pa;
			}
		}
	}
	if (buf && (len < n))
		*buf = 0;

	// now copy the result into our own storage
	*((wstring*)this) = tmp;
	// and get rid of the temporary buffer
	delete [] tmp;

	return len;
}

const char *wstring2::to_utf8() const
{
	char *buf = s_buffer;
	size_t len = 0;
	size_t n = MAX_WSTRING2_SIZE;
	const wchar_t *psz = c_str();

	while (*psz && ((!buf) || (len < n)))
	{
		uint cc;
#ifdef WC_UTF16
		size_t pa = decode_utf16(psz, cc);
		psz += (pa == (size_t)-1) ? 1 : pa;
#else
		cc=(*psz++) & 0x7fffffff;
#endif
		unsigned cnt;
		for (cnt = 0; cc > utf8_max[cnt]; cnt++) {}
		if (!cnt)
		{
			// plain ASCII char
			if (buf)
				*buf++ = (char) cc;
			len++;
		}
		else
		{
			len += cnt + 1;
			if (buf)
			{
				*buf++ = (char) ((-128 >> cnt) | ((cc >> (cnt * 6)) & (0x3f >> cnt)));
				while (cnt--)
					*buf++ = (char) (0x80 | ((cc >> (cnt * 6)) & 0x3f));
			}
		}
	}

	if (buf && (len<n)) *buf = 0;
	return s_buffer;
}

#endif // SUPPORT_WSTRING

/** Useful function which wraps the C standard library's strtok */
void vtTokenize(char *buf, const char *delim, vtStringArray &tokens)
{
	char *p = NULL;
	p = strtok(buf, delim);
	while (p != NULL)
	{
		tokens.push_back(vtString(p));
		p = strtok(NULL, delim);
	}
}

int vtFindString(const vtStringArray &arr, const char *find)
{
	for (size_t i = 0; i < arr.size(); i++)
	{
		if (arr[i].Compare(find) == 0)
			return (int) i;
	}
	return -1;
}

vtString vtConcatArray(const vtStringArray &arr, const char delim)
{
	vtString cat;
	for (size_t i = 0; i < arr.size(); i++)
	{
		cat += arr[i];
		if (i < arr.size()-1)
			cat += delim;
	}
	return cat;
}

/** Extract a string array (ie. tokenize) with the messy non-const of strtok */
void vtExtractArray(const vtString &input, vtStringArray &arr, const char delim)
{
	int curr = 0, next, len = input.GetLength();
	while ((next = input.Find(delim, curr)) != -1)
	{
		arr.push_back(vtString((const char *)input+curr, next-curr));
		curr = next+1;
	}
	// and remainder after last delim
	if (curr != len)
		arr.push_back(vtString((const char *)input+curr, len-curr));
}

