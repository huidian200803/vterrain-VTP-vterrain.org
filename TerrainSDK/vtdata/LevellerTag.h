#ifndef __LEVELLER_TAG_H__
#define __LEVELLER_TAG_H__
/*
	LevellerTag.h

	Tag storage support.
	Copyright 2002-2005 Daylon Graphics Ltd. /  Virtual Terrain Project
	____________________________________________________________________________
	Please use a tab setting of 4 to view this file.
	____________________________________________________________________________

	This file came from part of the Leveller (tm) Software Developer's Kit.
	It is the reference C++ code used to implement the hierarchical tag	storage
	system used by the public heightfield clipboard format (and possibly
	elsewhere).

	This code is licenced under the LGPL (Lesser GNU Public Licence).
	Basically, you can do whatever you want with it, including integrating
	it with commercial software.

	Note: One-byte structure member alignment used.

	Tags are data storage entities that exist in a hierarchical relationship
	within a root "tag", which is just an area of memory manipulated by the
	CRootTag class.

	CRootTag always iterates through the entire block when searching for a tag.
	This is not considered to be a problem since a) the number of tags is
	small, b) the data is in memory, and c) once a match on the head of the
	pathspec occurs, the search rapidly narrows. If the search time becomes
	problematic, you can always use the tag's tagSize fields to store
	aggregate size info in each tag, allowing you to skip a tag and all its
	children in one shot. Calculating the values of this field is not
	currently implemented.

	Note: this module doesn't know anything about the clipboard, since that
	tends to be a platform-specific feature. Daylon Graphics may have
	reference code for clipboard usage elsewhere in the Leveller SDK.
*/

#pragma pack(push, 1)

namespace daylon {

typedef uchar	uint8, uchar, byte;
typedef char			int8;
typedef short			int16;
typedef unsigned short	uint16, word;
typedef int				int32;
typedef uint	uint32, dword;
typedef float			float32;
typedef double			float64;

typedef void* (*_MALLOCFUNC)(size_t);
typedef void (*_FREEFUNC)(void*);
typedef void* (*_MEMCOPYFUNC)(void*, const void*, size_t);


const size_t MAX_TAGNAMELEN = 15;

enum
{
	VALKIND_NONE	= 0,
	VALKIND_BINARY	= 1,
	VALKIND_CHAR	= 2,
	VALKIND_BYTE	= 3,
	VALKIND_INT16	= 4,
	VALKIND_UINT16	= 5,
	VALKIND_INT32	= 6,
	VALKIND_UINT32	= 7,
	VALKIND_FLOAT	= 8,
	VALKIND_DOUBLE	= 9
};

// Relation flags; OR together.
enum
{
	TAGRELATION_CHILD	= 1,
	TAGRELATION_SIBLING	= 2
};


typedef struct
{
	char	szName[MAX_TAGNAMELEN + 1];

	daylon::uint32	valueSize;
	daylon::uint32	reserved;

	// To make seeking more efficient, one can,
	// after writing all the tags, work through them,
	// compute each one's full compound size, and
	// store them here.
	daylon::uint32	tagSize;
	daylon::uint32	reserved2;

	daylon::int16	relationFlags;
	daylon::int16	valueKind;
} TAGHEADER; // 36 bytes

typedef union
{
	daylon::int32			n;
	daylon::float64			d;
	daylon::byte			c[1];
} TAGVALUE; // 8 bytes

// A tag takes sizeof(TAGHEADER) + header.valueSize bytes.
// If a tag is storing a scalar type and header.valueSize
// is greater than the scalar's size, it means an array
// of scalar values is being stored, the number of which
// is valueSize / sizeof(scalar).
typedef struct
{
	TAGHEADER	header;
	TAGVALUE	value;
} TAG;

#pragma pack(pop)


class CRootTag
{
	// Implements a C++ interface to a
	// virtual root tag. Call CalcNormalStorage
	// to compute the storage for all the normal tags
	// of each type you need,
	// and CalcBinaryTagStorage for each binary tag.
	// Sum the results together, allocate a block
	// that size, and call SetStorage.
	// Call Open to read or write tags.
	// If you're writing a root tag, call Write
	// to append each tag. If you're reading, call ReadTag.

	public:
		CRootTag() : m_pRoot(NULL), m_mark(0), m_size(0),
					m_openMode(0) {}
		virtual ~CRootTag() {}

		size_t GetValSize(int) const;
		size_t CalcNormalStorage(size_t numTags, int kind, size_t arraySize = 1) const;
		size_t CalcBinaryTagStorage(size_t binSize) const;

		void SetStorage(void* p, size_t n)
			{ m_pRoot = (uchar*)p;
			m_mark = 0; m_size = n; m_openMode = 0; }

		void Open(const char* pszMode);
		void Close(void);

		// WriteParent() lets you write a parent tag.
		void WriteParent(const char* pszName, bool bHasSibling)
			{ this->Write(pszName,
						TAGRELATION_CHILD |
						(bHasSibling ? TAGRELATION_SIBLING : 0));
			}

		// Write() is the general-purpose tag appender.
		void Write(
			const char* pszName, int relationFlags,
			 int valueKind = VALKIND_NONE,
			 const void* pvData = NULL,
			 size_t datasize = 0);

		void Write(const char* pszName,
					daylon::uint32 value,
					bool bHasSibling);

		void Write(const char* pszName,
					double value,
					bool bHasSibling);


		// WriteBinary() must be used if you are dynamically
		// generating the tag's value, or if you want to
		// save memory and create the value data inside
		// the tag's memory area.
		void* WriteBinary(
			const char* pszName, bool bHasSibling,
			 size_t datasize);

		// Read() is the generic tag reader.
		// Full pathspecs must always be given.
		size_t Read(const char*, void**);
		daylon::uint32 ReadUINT32(const char*, daylon::uint32 _default);
		double ReadDouble(const char*, double _default);

		size_t MemSize(void) const
			{ return sizeof(*this) + m_size; }
		void CopyFrom(const CRootTag&, _MALLOCFUNC, _MEMCOPYFUNC);
		void Destroy(_FREEFUNC);

#if VTDEBUG
		void Dump(void);
		void DumpTag(const char*);
		static const char* TypeString(int);
		static char* ValueString(const TAG&, char*);
#endif

	private:
		uchar*	m_pRoot;
		size_t			m_mark;
		size_t			m_size;
		int				m_openMode;

		void WasEndReached(void);
		int SkipTag(void);
		size_t ReadTag(const char*, void**);
		size_t ArraySize(const TAG&) const;

}; // CRootTag

} // namespace daylon

#endif // __LEVELLER_TAG_H__
