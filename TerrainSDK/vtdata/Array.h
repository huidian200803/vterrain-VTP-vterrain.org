//
// Array.h
//
// Copyright (c) 2001-2015 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
/** \file Array.h */

#pragma once

#ifdef _MSC_VER
	#if _DEBUG
	  #define _CRTDBG_MAP_ALLOC
	  #include <crtdbg.h>
	#endif
#endif

#include <stdlib.h>	// for free()
#include <memory.h>	// for memcpy()
#include <assert.h>	// for assert()

/**
 * An Array template which automatically grows as you add or set
 * entities.
 *
 * Note that construction and destruction is not done automatically
 * if the entities are class objects.  You can provide this destruction
 * yourself by overriding the DestructItems method, but it is easier to use
 * this template for objects with simple value semantics such as basic types
 * (int, float..), structs, and pointers.  If you do create a subclass like
 * this:
\code
	class MyArray : public vtArray<MyObject *> {};
\endcode
 * note that you will need to provide not only a DestructItems()
 * implementation, but also a destructor.  This is because the default
 * vtArray destructor is not smart enough to call your DestructItems()
 * method (it will call the base DestructItems() instead, which does
 * nothing).
 *
 * A full working example is:
\code
	class MyArray : public vtArray<MyObject *>
	{
		virtual ~MyArray() { Clear(); free(m_Data); m_Data = NULL; m_MaxSize = 0; }
		virtual	void DestructItems(uint first, uint last)
		{
			for (uint i = first; i <= last; i++)
				delete GetAt(i);
		}
	};
\endcode
 */
template <class E> class vtArray
{
public:
	vtArray(uint size = 0);
	vtArray(const vtArray<E>&);
	virtual ~vtArray();

	uint	GetSize() const;
	uint	GetMaxSize() const;
	bool	SetSize(uint);
	bool	SetMaxSize(uint);
	uint	GetElemSize() const;
	E*		GetData() const;
	void	FreeData();
	bool	IsEmpty() const;
	E&		GetAt(uint i) const;
	bool	SetAt(uint i, E);

	vtArray<E>& operator=(const vtArray<E>&);
	E&			operator[](uint i);
	const E&	operator[](uint i) const;
	void		Clear();
	bool		RemoveAt(uint i, int n = 1);
	int			Append(const E&);
	int			Append(const vtArray<E>&);
	int			Find(const E&) const;

protected:
	//	Internal functions
	virtual bool	Grow(uint);
	virtual	void	DestructItems(uint first, uint last);

	//	Data members
	uint	m_Size;		// number of elements added so far
	uint	m_MaxSize;	// maximum number of elements we have room for
	E*		m_Data;		// data area for array
};


/**
 * Creates and initializes an empty array (array with no elements).
 *
 * \param size	number of elements data area should initially make
 *			room for (initial value of [MaxSize]).
 *			If zero, little initial space is allocated but the
 *			array will grow dynamically as space is needed.
 *
 * \sa vtArray::SetMaxSize vtArray::SetSize
 *
 * \par Example:
\code
	vtArray<void*> foo;		// empty array of pointers
\endcode
 *
 */
template <class E> vtArray<E>::vtArray(uint size)
{
	m_Size = 0;				// empty to start
	m_MaxSize = 0;			// remember the size
	m_Data = NULL;
	if (size > 0)			// make room for <size> elements
		Grow(size);
}

/**
 * Complete free the data held by this array's data.
 */
template <class E> void vtArray<E>::FreeData()
{
	Clear();
	if (m_Data)
		free(m_Data);
	m_Data = NULL;
	m_MaxSize = 0;
}

/**
 * Creates and initializes an array from another (of the same type).
 *
 * \param a	An array to copy from.
 */
template <class E> vtArray<E>::vtArray(const vtArray<E>& a)
{
	if( this == &a )
		return;

	m_Size = 0;		// empty to start
	m_MaxSize = 0;	// remember the size
	m_Data = NULL;
	Append(a);		// copy each element from the given array
}


/**
 *	Called by the array implementation when array items are deleted.
 *	The default implementation does _not_ call the destructors for the
 *	array items. This will not work if your array elements do memory
 *	deallocation in their destructors.
 *
 *	Override this function to explicitly call the destructors properly if
 *	you need this functionality.
 *
 *	\param first	Index of first element to destroy
 *	\param last		Index of last element to destroy
 *
 * \par Example:
\code
	// Overrides DestructItems to call constructors
	inline void MyArray::DestructItems(uint first, uint last)
	{
		for (uint i = first; i <= last; ++i)
		   delete GetAt(i);
	}
\endcode
 */
template <class E> inline void vtArray<E>::DestructItems(uint first, uint last)
{
	//VTLOG("base DestructItems, %d %d\n", start, nitems);
}


/**
 *	Destructor for array class.
 */
template <class E> inline vtArray<E>::~vtArray()
{
	//VTLOG("~vtArray, size %d, max %d\n", m_Size, m_MaxSize);
	Clear();
	free(m_Data);
	m_Data = NULL;
	m_MaxSize = 0;
}

/**
 * Enlarge the array to accomodate <i>growto</i> elements. If the array
 * can already hold this many elements, nothing is done. Otherwise,
 * the data area of the array is enlarged (reallocating if necessary)
 * so that <i>growto</i> contiguous elements can be stored.
 *
 * \param growto	Number of elements the array should be able to
 *					hold after it has grown
 *
 * \return True if array was successfully grown, else false.
 *
 * \sa vtArray::SetData vtArray::SetMaxSize
 */
template <class E> bool vtArray<E>::Grow(uint growto)
{
	int	n = growto - m_MaxSize;
	E*		old_data;

	if (n >= 0)
	{
		if (m_Data == NULL)					// make new data area?
		{
//			assert(m_Size == 0);
			m_Data = (E*) malloc(sizeof(E) * growto);
			if (m_Data == NULL)				// could not enlarge?
//				ERROR(("vtArray::Grow: out of memory"), false);
				return false;
		}
		else
		{
			if (growto < 4) growto = 4;	// minimum growth
			old_data = m_Data;
			m_Data = (E*) malloc(sizeof(E) * (growto));
			memcpy(m_Data, old_data, sizeof(E) * m_Size);
			free(old_data);
			if (m_Data == NULL)
//				ERROR(("vtArray::Grow: out of memory"), false);
				return false;
		}
		m_MaxSize = growto;					// remember new size
	}
	return true;
}

template <class E> inline E* vtArray<E>::GetData() const
{
	return m_Data;
}

/**
 * If the array is user-managed, MaxSize establishes the
 * maximum number of elements that can be stored. If the array
 * is dynamically managed by the system, setting the maximum size
 * enlarges its data area to accomodate the required number of elements.
 *
 * \param s	Current maximum size of array (number of elements its
 *			data area can hold)
 *
 * \return bool, true if maximum size successfully changed, else false.
 *
 * \sa vtArray::SetData vtArray::SetSize vtArray::GetElemSize vtArray::Grow
 */
template <class E> bool vtArray<E>::SetMaxSize(uint s)
{
	if (s > m_MaxSize)			// enlarge array
		return Grow(s);			// if we can
	return true;
}

template <class E> inline uint	vtArray<E>::GetMaxSize() const
	{ return m_MaxSize; }

template <class E> inline uint	vtArray<E>::GetElemSize() const
	{ return sizeof(E); }

/**
 * Set the current array size. If the array is dynamically managed,
 * it will be enlarged to accomodate the new size. If not,
 * the array size cannot be set beyond the current maximum size.
 * When the array size is enlarged, we should call constructors
 * for the new empty elements. We don't do that yet.
 *
 * \param s		Current number of elements contained in array
 *
 * \return bool	True if size successfully changed, else false.
 *
 * \sa vtArray::SetData vtArray::Grow vtArray::SetMaxSize
 *
 * \par Examples:
\code
	vtArray<RGBi> cols(256);	  // room for 256 colors
	int ncols = cols.GetSize();	// will be zero
	ncols = cols.GetMaxSize();	// will be 256
	cols.SetSize(ncols);		// calls 256 Color4 constructors NOT
\endcode
 *
 */
template <class E> bool inline vtArray<E>::SetSize(uint s)
{
	assert(s >= 0);
	if (s > m_MaxSize)
		if (!Grow(s))
			return false;
	m_Size = s;
	return true;
}

template <class E> inline uint vtArray<E>::GetSize() const
	{ return m_Size; }

/**
 *	Sets the i'th element of the array to the given value.
 *	The number of bytes copied is determined by the element size
 *	of the array specified at initialization time.
 *	If the array is not large enough, it is extended to become
 *	1 1/2 times its current size.
 *
 * \param i		Index of new element (0 based)
 * \param val	Value of the new element.
 *
 * \return int index of element added or -1 if out of memory
 *
 * \par Examples:
\code
	vtArray<RGBi> cols(16);			// room for 16 colors
	cols.SetAt(0, RGBi(1,1,1));		// first color white
	cols.SetAt(15, RGBi(1,0,0));	// last color red
									// makes Colors 1-14, too
	cols.SetAt(17, RGBi(0,1,1));	// causes array growth
\endcode
 */
template <class E> bool vtArray<E>::SetAt(uint i, E val)
{
	if (i >= m_MaxSize)			// need to extend array?
	   {
		uint n = m_MaxSize;
		n += (n >> 1);			// grow to 1 1/2 times current size
		if (n <= i) n = i + 1;	// unless user wants more
		if (!Grow(n))			// extend failure
			return false;
	   }
	m_Data[i] = val;
	if (i >= m_Size)			// enlarge array size if at end
		m_Size = i + 1;
	return true;
}

/**
 *	Gets the i'th element of the array.
 *
 * \param i	0 based index of element to get. Like C++ arrays,
 *			these arrays do not check the range of the index so your
 *			program will crash if you supply a number less than zero
 *			or greater than the array size as an index.
 *
 * \return element accessed
 */
template <class E> inline E& vtArray<E>::GetAt(uint i) const
	{ return m_Data[i]; }

template <class E> inline const E& vtArray<E>::operator[](uint i) const
	{ return m_Data[i]; }

template <class E> inline E& vtArray<E>::operator[](uint i)
	{ return m_Data[i]; }

/**
 * Compares each element of the array to the input element and
 * returns the index of the first one that matches. Comparison
 * of elements is done using operator== (which must be defined
 * for your element class if you want to use Find).
 *
 * \param
 *	<elem>	value of the element to match
 *
 * \return int
 *	index of matching array element or -1 if not found
 *
 * \sa vtArray::SetAt
 *
 * \par Examples:
\code
	vtArray<int> foo;		 // define integer array
	foo.Append(5);		 // first element is 5
	foo.Append(6);		 // second element is 5
	int t = foo.Find(7);  // returns -1
	t = foo.Find(6);	  // returns 1
\endcode
 *
 */
template <class E> int vtArray<E>::Find(const E& elem) const
{
	const E* p = m_Data;

	for (uint i = 0; i < m_Size; ++i)	// look for matching element
		if (*p++ == elem)
			return i;					// found it
	return -1;
}

/**
 * Appends one element onto the end of the array.
 * If the array is dynamically managed, it is enlarged
 * to accomodate another element if necessary.
 *
 * \param
 *	<v>	 value of the new element
 *
 * \return void*
 *	-> element added (within array) or NULL if out of memory
 *
 * \sa vtArray::SetAt vtArray::SetSize vtArray::RemoveAt
 *
 * \par Examples:
\code
	vtArray<float>	vals;
	vals.Append(1.0f);	  // first element
	vals.Append(2.0f);	  // second element
	vals.SetAt(5, 6.0f);  // sixth element
	vals.Append(7.0f);	  // seventh element
\endcode
 */
template <class E> int inline vtArray<E>::Append(const E& v)
{
	int	index = m_Size;

	if (!SetAt(index, v))
		return -1;
	return index;
}

/**
 *	Removes the i'th element of the array. The following
 *	elements are shuffled up to eliminate the unused space.
 *
 * \param	i	Index of element to remove (0 based)
 * \param	n	Number of elements to remove (default 1)
 *
 * \return bool True if element was successfully removed, else false.
 *
 * \sa vtArray::Append vtArray::SetAt vtArray::SetSize
 *
 * \par Examples:
\code
	vtArray<int16> zot(8); // room for 8 shorts
	zot.SetSize(8);		 // now has 8 zeros
	zot[1] = 1;			 // second element
	zot[2] = 2;			 // third element
	zot.RemoveAt(0);	 // remove first element
	zot.RemoveAt(-1);	 // returns false
\endcode
 */
template <class E> bool vtArray<E>::RemoveAt(uint i, int n)
{
	E*	elem;
	int	shuffle;

	if (i >= m_Size)				// element out of range?
		return false;
	if (n == 0) n = 1;				// default is one element
	shuffle = m_Size - (i + n);		// number to shuffle up
	elem = m_Data + i;
	memcpy(elem, elem + n, sizeof(E) * shuffle);
	m_Size -= n;
	return true;
}

/**
 * Concatenates the contents of the source array into the destination array.
 * The destination array is enlarged if possible. When an object array
 * is appended, the objects are multiply referenced (not duplicated).
 *
 * \param <src>	Source array containing elements to be appended
 *
 * \return index of last element successfully added or -1 on error
 *
 * \sa vtArray::Append vtArray::SetAt
 *
 * \par Examples:
\code
	int	zap[3] = { 1, 2, 3 };
	vtArray<int> zip(3, &zap);	// user managed
	vtArray<int> zot;				// dynamic
	zot.Append(zip);			// adds 1 2 3
	zot.Append(zip);			// adds 1 2 3 again
\endcode
 */
template <class E> int vtArray<E>::Append(const vtArray<E>& src)
{
	int	n = m_Size + src.m_Size;

	if (!Grow(n))
		return -1;
	for (uint i = 0; i < src.m_Size; ++i)
		m_Data[m_Size + i] = src.m_Data[i];
	m_MaxSize = n;
	m_Size += src.m_Size;
	return n - 1;
}

/**
 * Removes the elements in the array but not the array data area.
 * An array is considered empty if it has no elements.
 *
 * \sa vtArray::SetSize vtArray::IsEmpty
 */
template <class E> void vtArray<E>::Clear()
{
	//VTLOG("Empty, size %d\n", m_Size);
	if (m_Size > 0)
		DestructItems(0, m_Size - 1);
	m_Size = 0;
}

/**
 * Determines whether an array has elements or not
 *
 * \return True if array contains no elements, else false.
 *
 * \sa vtArray::SetSize vtArray::Empty
 */
template <class E> inline bool vtArray<E>::IsEmpty() const
	{ return m_Size == 0; }
