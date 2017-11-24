//
// ByteOrder.cpp
//
// Copyright (c) 2001-2004 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#include "ByteOrder.h"

static int GetDataTypeSize( DataType type )
{
	switch ( type )
	{
		case DT_SHORT:	return sizeof(short);	break;
		case DT_INT:	return sizeof(int);		break;
		case DT_LONG:	return sizeof(long);	break;
		case DT_FLOAT:	return sizeof(float);	break;
		case DT_DOUBLE:	return sizeof(double);	break;
	}
	assert(false);
	return 1;
}


/**
 * If the byte orders differ, swap bytes; if not, don't; return the result.
 * This is the memory buffer version of the SwapBytes() macros, and as such
 * supports an array of data.  It also parametizes the element type to avoid
 * function explosion.
 * \param items the data items to order
 * \param type the type of each item in the array
 * \param nitems the number if items in the array
 * \param data_order the byte order of the data
 * \param desired_order the desired byte ordering
 *
 */
void SwapMemBytes( void *items, DataType type, size_t nitems,
				   ByteOrder data_order, ByteOrder desired_order )
{
	if ( data_order	== BO_MACHINE ) data_order	= NativeByteOrder();
	if ( desired_order == BO_MACHINE ) desired_order = NativeByteOrder();
	if ( data_order == desired_order )
		return;

	size_t tsize = GetDataTypeSize( type );
	char  *base = (char *) items, *p;

	switch ( type )
	{
	case DT_SHORT  :
		for ( p = base + (nitems-1) * tsize; p >= base; p -= tsize )
			*(short *)p = SwapShort( *(short *)p );
		break;
	case DT_INT	:
		for ( p = base + (nitems-1) * tsize; p >= base; p -= tsize )
			*(int   *)p = SwapInt( *(int   *)p );
		break;
	case DT_LONG   :
		for ( p = base + (nitems-1) * tsize; p >= base; p -= tsize )
			*(long  *)p = SwapLong( *(long  *)p );
		break;
	case DT_FLOAT  :
		for ( p = base + (nitems-1) * tsize; p >= base; p -= tsize )
			*(float *)p = SwapFloat( *(float *)p );
		break;
	case DT_DOUBLE :
		for ( p = base + (nitems-1) * tsize; p >= base; p -= tsize )
			*(double *)p = SwapDouble( *(double *)p );
		break;
	default:
		assert(false);
	}
}


/**
 * Like stdio's fread(), but adds an optional byte swapping phase for
 * the data read.
 * \param ptr data buffer to read items into
 * \param type the data type of items to be read
 * \param nitems the number of items to read
 * \param stream the stdio stream open for read
 * \param file_order the byte ordering of data read from the file
 * \param desired_order the desired byte ordering
 * \return fread() return value (num items read, or negative for error)
 *
 */
size_t FRead( void *ptr, DataType type, size_t nitems, FILE *stream,
			  ByteOrder file_order, ByteOrder desired_order )
{
	int tsize  = GetDataTypeSize( type );

	// returned value is the number of "items" read
	size_t ret = fread( ptr, tsize, nitems, stream );

	if ( (int)ret >= 0 )
		SwapMemBytes( ptr, type, ret, file_order, desired_order );
	return ret;
}


/**
 * Like stdio's fread(), but adds an optional byte swapping phase for
 * the data read. File access is done via zlib's gzip IO routines to
 * be compatible with gzopen(), etc.
 * \param ptr data buffer to read items into
 * \param type the data type of items to be read
 * \param nitems the number of items to read
 * \param stream the stdio stream open for read
 * \param file_order the byte ordering of data read from the file
 * \param desired_order the desired byte ordering
 * \return fread() return value (Number of items read, or negative for error)
 *
 */
size_t GZFRead( void *ptr, DataType type, size_t nitems, gzFile gzstream,
			  ByteOrder file_order, ByteOrder desired_order )
{
	int tsize  = GetDataTypeSize( type );

	// returned value is the number of "items" read
	size_t ret = gzread(gzstream, ptr, tsize * nitems) / tsize;

	if ( (int)ret >= 0 )
		SwapMemBytes( ptr, type, ret, file_order, desired_order );
	return ret;
}

/**
 * Just like stdio's fwrite(), but adds an optional byte swapping phase for
 * the data write.
 * \param ptr data buffer to write items from
 * \param type the data type of items to be written
 * \param nitems the number of items to written
 * \param stream the stdio stream open for writing
 * \param file_order the byte ordering of data written to the file
 * \param data_order the byte ordering of data in the buffer
 * \return fwrite() return value (Number of items read, or negative for error)
 *
 */
size_t FWrite( void *ptr, DataType type, size_t nitems, FILE *stream,
			  ByteOrder file_order, ByteOrder data_order )
{
	// If we need to swap bytes, we have two options:
	//  1. create a temporary buffer with the swapped values, write it
	//  2. swap, write, swap back again
	// The second approach is implemented here

	SwapMemBytes( ptr, type, nitems, file_order, data_order );

	int tsize  = GetDataTypeSize( type );
	size_t ret = fwrite(ptr, tsize, nitems, stream);

	SwapMemBytes( ptr, type, nitems, file_order, data_order );

	return ret;
}

/**
 * Just like stdio's fwrite(), but adds an optional byte swapping phase for
 * the data write. File access is done via zlib's gzip IO routines to
 * be compatible with gzopen(), etc.
 * \param ptr data buffer to write items from
 * \param type the data type of items to be written
 * \param nitems the number of items to written
 * \param stream the stdio stream open for writing
 * \param file_order the byte ordering of data written to the file
 * \param data_order the byte ordering of data in the buffer
 * \return fwrite() return value (num items written, or negative for error)
 *
 */
size_t GZFWrite( void *ptr, DataType type, size_t nitems, gzFile gzstream,
			  ByteOrder file_order, ByteOrder data_order )
{
	// If we need to swap bytes, we have two options:
	//  1. create a temporary buffer with the swapped values, write it
	//  2. swap, write, swap back again
	// The second approach is implemented here

	SwapMemBytes( ptr, type, nitems, file_order, data_order );

	int tsize  = GetDataTypeSize( type );
	size_t ret = gzwrite(gzstream, ptr, tsize * nitems);

	SwapMemBytes( ptr, type, nitems, file_order, data_order );

	return ret;
}

