/*
	LevellerTag.cpp
	Tag storage support.
	Copyright 2002-2006 Daylon Graphics Ltd. /  Virtual Terrain Project
	____________________________________________________________________________
	Please use a tab setting of 4 to view this file.

	Revision history:
	rcg		aug 15/02	Created.
	rcg		jul 20/05	Renamed from util_tag, namespace changes.
	rcg		oct 25/05	Portability fixes.
						Moved to daylon namespace from leveller.
*/

#include <assert.h>
#define myassert assert
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "vtLog.h"

#include "LevellerTag.h"

using namespace daylon;

#define daylon_assert assert

enum
{
	kClosed,
	kReading,
	kWriting
};

const char kSep = '/';

#define TAG_THROW	{ daylon_assert(false); throw 0; }

size_t daylon::CRootTag::GetValSize(int valKind) const
{
	size_t n;

	switch(valKind)
	{
		case VALKIND_NONE:
			n = 0;
			break;

		case VALKIND_CHAR:
		case VALKIND_BYTE:
			n = 1;
			break;

		case VALKIND_INT16:
		case VALKIND_UINT16:
			n = 2;
			break;

		case VALKIND_INT32:
		case VALKIND_UINT32:
		case VALKIND_FLOAT:
			n = 4;
			break;

		case VALKIND_DOUBLE:
			n = 8;
			break;

		default:
			daylon_assert(false);
			throw 0;
			break;
	}
	return n;
}


size_t daylon::CRootTag::CalcNormalStorage
(
	size_t numTags,
	int valKind,
	size_t arraySize /* = 1 */
) const
{
	return (numTags *
			(sizeof(TAGHEADER) +
			arraySize * this->GetValSize(valKind) )
			);
}


size_t daylon::CRootTag::CalcBinaryTagStorage(size_t binSize) const
{
	return (sizeof(TAGHEADER) + binSize);
}


void daylon::CRootTag::Open(const char* pszMode)
{
	if(m_openMode != kClosed)
		TAG_THROW;

	switch(*pszMode)
	{
		case 'r': m_openMode = kReading; break;
		case 'w': m_openMode = kWriting; break;
		default: TAG_THROW; break;
	}
	m_mark = 0; // rewind
}


void daylon::CRootTag::Close(void)
{
	m_openMode = kClosed;
}


daylon::uint32 daylon::CRootTag::ReadUINT32
(
	const char* pszPath,
	daylon::uint32 _default
)
{
	void* pv;
	size_t size = this->Read(pszPath, &pv);

	if(pv == NULL)
		return _default;

	if(size != this->GetValSize(VALKIND_UINT32))
		TAG_THROW;

	return *((daylon::uint32*)pv);
}


double daylon::CRootTag::ReadDouble
(
	const char* pszPath,
	double _default
)
{
	void* pv;
	size_t size = this->Read(pszPath, &pv);

	if(pv == NULL)
		return _default;

	if(size != this->GetValSize(VALKIND_DOUBLE))
		TAG_THROW;

	return (double) *((daylon::float64*)pv);
}


size_t daylon::CRootTag::Read
(
	const char* pszPath,
	void**		ppvData
)
{
	// Read a tag given its full pathspec.

	m_mark = 0; // Start from the start.
	return this->ReadTag(pszPath, ppvData);
	// Note: it doesn't matter that m_mark is
	// not zero here, because it will be if
	// we Read again.
}


void daylon::CRootTag::WasEndReached(void)
{
	// If we've reached the end of our data, throw.
	if(m_mark >= m_size)
		throw 0;
}

// Actual recursive routine.
size_t daylon::CRootTag::ReadTag
(
	const char* pszPath,
	void**		ppvData
)
{
	// Advance down to the tag whose full pathspec
	// matches pszPath. Return zero and put NULL
	// into *ppvData on failure.

	if(pszPath == NULL ||
		ppvData == NULL ||
		*pszPath == 0 ||
		*pszPath == kSep)
		TAG_THROW;

	// Get the path's highestmost tagname.
	char sz[MAX_TAGNAMELEN + 1];

	const char* psz = pszPath;
	char* psz2 = sz;

	for(; *psz != 0 && *psz != kSep;)
		*psz2++ = *psz++;

	*psz2 = 0;

	int bAtEnd = (*psz == 0);

	const char* pszRest = psz + 1;

	bool bHasSibling;

	do
	{

		// When returning bin data, put a void* into *pvData.
		if(m_mark >= m_size)
		{
			// We reached the end, and found nothing.
			*ppvData = NULL;
			return 0;
		}

		TAG* pTag = (TAG*)(m_pRoot + m_mark);

		int bHasKids = (TAGRELATION_CHILD ==
						(TAGRELATION_CHILD & pTag->header.relationFlags));

		bHasSibling = (TAGRELATION_SIBLING ==
						(TAGRELATION_SIBLING & pTag->header.relationFlags));

		int bMatch = (strcmp(sz, pTag->header.szName) == 0);


		// Advance read mark to next tag.
		m_mark += sizeof(TAGHEADER) + pTag->header.valueSize;

		if(bMatch)
		{
			// We can't be a parent if we're at the end of the
			// path, and vice versa.
			if(bAtEnd == bHasKids)
			{
				*ppvData = NULL;
				return 0;
			}

			if(bAtEnd)
			{
				// We are a leaf.
				*ppvData = &(pTag->value.n);
				return pTag->header.valueSize;
			}
			else
			{
				// We are a parent. The search goes on.
				return this->ReadTag(pszRest, ppvData);
			}
		}


		// No match. Maybe the next tag.
		// We have to skip children and try siblings.

		if(bHasKids)
		{
			// We have to skip over our child
			// and all of its siblings.
			while(this->SkipTag()) {}
		}
	} while(bHasSibling);

	// We didn't match, and we don't have any siblings,
	// so return failure. Maybe one of the other children
	// in the parent tag will match.
	*ppvData = NULL;
	return 0;
}


int daylon::CRootTag::SkipTag(void)
{
	// Skip a tag and all its children.
	// Stop at whatever tag follows (or EOF).
	// Return true if the following tag is a sibling.

	this->WasEndReached();

	TAGHEADER*	pTag = (TAGHEADER*)(m_pRoot + m_mark);

	m_mark += sizeof(*pTag) + pTag->valueSize;

	// Skip child and all of its siblings.
	if(TAGRELATION_CHILD == (pTag->relationFlags & TAGRELATION_CHILD))
		while(this->SkipTag()) {}

	return (TAGRELATION_SIBLING == (pTag->relationFlags &
			TAGRELATION_SIBLING));
}


size_t daylon::CRootTag::ArraySize(const TAG& tag) const
{
	// Get the array size of a tag's value.

	return (tag.header.valueSize /
				this->GetValSize(tag.header.valueKind));

}


void daylon::CRootTag::Write
(
	const char* pszName,
	int			relationFlags,
	int			valueKind /* = VALKIND_NONE*/,
	const void*	pvData /* = NULL*/,
	size_t		datasize /* = 0 */
)
{
	// Append a tag.

	// To store an array of scalar values,
	// set valueKind to the scaler's type, put
	// the array to start at pvData, and make
	// datasize the byte size of the array.
	// Otherwise, leave datasize at zero.

	this->WasEndReached();

	TAG* pTag = (TAG*)(m_pRoot + m_mark);

	daylon_assert(pszName != NULL);
	daylon_assert(strlen(pszName) <= MAX_TAGNAMELEN);
	daylon_assert(relationFlags <= (TAGRELATION_SIBLING | TAGRELATION_CHILD));

	for(const uchar* psz = (const uchar*)pszName;
		*psz != 0; psz++)
	{
		if(*psz < ' ' || *psz > 0x7F)
		{
			daylon_assert(false);
			throw 0;
		}
	}

	strcpy(pTag->header.szName, pszName);
	pTag->header.relationFlags = relationFlags;
	pTag->header.valueKind = valueKind;
	pTag->header.reserved = 0;
	pTag->header.tagSize = 0;
	pTag->header.reserved2 = 0;

	if(TAGRELATION_CHILD == (relationFlags & TAGRELATION_CHILD))
	{
		// Parent tag; force valuesize to zero,
		// and set no value type.
		pTag->header.valueSize = 0;
		pTag->header.valueKind = VALKIND_NONE;
	}
	else
	{
		// Leaf tag; add value part.
		if(datasize != 0)
		{
			daylon_assert(datasize != 0);
			if(datasize == 0)
				throw 0;
			pTag->header.valueSize = datasize;
		}
		else
			pTag->header.valueSize = this->GetValSize(valueKind);

		// Transfer data.
		memcpy(&pTag->value.n, pvData, pTag->header.valueSize);
	}

	// Advance write mark.
	m_mark += this->CalcBinaryTagStorage(pTag->header.valueSize);
}


void* daylon::CRootTag::WriteBinary
(
	const char* pszName,
	bool bHasSibling,
	size_t datasize
)
{
	// Write a binary value tag, but leave the
	// filling in of the value portion up to the caller.
	// Return a pointer to the start of the memory block
	// where it should write.

	this->WasEndReached();

	TAG* pTag = (TAG*)(m_pRoot + m_mark);

	daylon_assert(pszName != NULL);
	daylon_assert(datasize != 0);
	daylon_assert(strlen(pszName) <= MAX_TAGNAMELEN);

	for(const char* psz = pszName; *psz != 0; psz++)
	{
		if(*psz < ' ')
		{
			daylon_assert(false);
			throw 0;
		}
	}

	strcpy(pTag->header.szName, pszName);
	pTag->header.relationFlags = bHasSibling ?
		TAGRELATION_SIBLING : 0;
	pTag->header.valueKind = VALKIND_BINARY;
	pTag->header.reserved = 0;
	pTag->header.valueSize = datasize;

	// Advance write mark.
	m_mark += this->CalcBinaryTagStorage(pTag->header.valueSize);

	return &pTag->value.n;
}


void daylon::CRootTag::Write
(
	const char* pszName,
	daylon::uint32 value,
	bool bHasSibling
)
{
	this->Write(
		pszName,
		bHasSibling ? TAGRELATION_SIBLING : 0,
		VALKIND_UINT32,
		&value,
		this->GetValSize(VALKIND_UINT32));
}

void daylon::CRootTag::Write
(
	const char* pszName,
	double value,
	bool bHasSibling
)
{
	const daylon::float64 f64 = (daylon::float64)value;
	this->Write(
		pszName,
		bHasSibling ? TAGRELATION_SIBLING : 0,
		VALKIND_DOUBLE,
		&f64,
		this->GetValSize(VALKIND_DOUBLE));
}


void daylon::CRootTag::CopyFrom(const daylon::CRootTag& tag, _MALLOCFUNC fnMalloc, _MEMCOPYFUNC fnMemCopy)
{
	assert(m_pRoot == NULL);
	assert(fnMalloc != NULL);
	assert(fnMemCopy != NULL);
	this->SetStorage(fnMalloc(tag.m_size), tag.m_size);
	fnMemCopy(m_pRoot, tag.m_pRoot, m_size);
	m_mark = tag.m_mark;
	m_openMode = tag.m_openMode;
}

void daylon::CRootTag::Destroy(_FREEFUNC fnFree)
{
	assert(m_pRoot != NULL);
	assert(fnFree != NULL);
	fnFree(m_pRoot);
}


#if VTDEBUG

// Utility routines for verifying clip tree.

void daylon::CRootTag::Dump(void)
{
	// Preserve mark for those dumping the tree
	// in the middle of I/O operations.
	size_t oldmark = m_mark;

	m_mark = 0;
	VTLOG("\nDump of clip tree structure:\n\n");
	VTLOG("Tag                        Type         Size                 Value\n");
	VTLOG("---------------------------------------------------------------------\n");

	this->DumpTag("");
	VTLOG("\n");

	m_mark = oldmark;
}

void daylon::CRootTag::DumpTag(const char* pszIndent)
{
	// Actual recursive routine.

	bool bHasSibling = true;

	while(bHasSibling && m_mark < m_size)
	{
		TAG* pTag = (TAG*)(m_pRoot + m_mark);

		bool bHasKids = (TAGRELATION_CHILD ==
						(TAGRELATION_CHILD & pTag->header.relationFlags));

		bHasSibling = (TAGRELATION_SIBLING ==
						(TAGRELATION_SIBLING & pTag->header.relationFlags));

		// Advance read mark to next tag.
		m_mark += sizeof(TAGHEADER) + pTag->header.valueSize;

		// Dump current node.
		char szColumn[100];
		size_t colwidth = 24 - (strlen(pTag->header.szName) + strlen(pszIndent));
		for(size_t i = 0; i < colwidth; i++)
			szColumn[i] = ' ';
		szColumn[colwidth] = 0;
		char sz[50];

		VTLOG("%s%s%s%8s %9d bytes %20s\n", pszIndent,
			pTag->header.szName,
			szColumn,
			CRootTag::TypeString(pTag->header.valueKind),
			pTag->header.valueSize,
			CRootTag::ValueString(*pTag, sz)
			);

		// Dump any children.
		if(bHasKids)
		{
			char sz[50];
			sprintf(sz, "%s  ", pszIndent);
			this->DumpTag(sz);
		}
	}
}

//static
const char* daylon::CRootTag::TypeString(int n)
{
	const char* kTypeStrs[] =
		{ "void", "binary", "char", "byte",
			"int16", "uint16", "int32", "uint32",
			"float32", "float64" };

	daylon_assert(n >= 0 &&
		n < sizeof(kTypeStrs) / sizeof(kTypeStrs[0]));

	return kTypeStrs[n];
}

//static
char* daylon::CRootTag::ValueString(const TAG& tag, char* psz)
{
	daylon_assert(psz != NULL);

	switch(tag.header.valueKind)
	{
		case VALKIND_NONE:
			psz[0] = 0;
			break;

		case VALKIND_BINARY:
			strcpy(psz, "(bytestream)");
			break;

		case VALKIND_CHAR:
			sprintf(psz, "%d", (int)(char)tag.value.c[0]);
			break;

		case VALKIND_BYTE:
			sprintf(psz, "%d", (int)tag.value.c[0]);
			break;

		case VALKIND_INT16:
			sprintf(psz, "%d", (int)*((daylon::int16*)&tag.value));
			break;

		case VALKIND_UINT16:
			sprintf(psz, "%d", (int)*((daylon::uint16*)&tag.value));
			break;

		case VALKIND_INT32:
			sprintf(psz, "%d", (int)*((daylon::int32*)&tag.value));
			break;

		case VALKIND_UINT32:
			sprintf(psz, "%u", (uint)*((daylon::uint32*)&tag.value));
			break;

		case VALKIND_FLOAT:
			sprintf(psz, "%f", *((daylon::float32*)&tag.value));
			break;

		case VALKIND_DOUBLE:
			sprintf(psz, "%f", *((daylon::float64*)&tag.value));
			break;

	}
	return psz;
}


#endif	// _DEBUG

