//
// ByteOrder.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef BYTEORDERH
#define BYTEORDERH

#include <stdio.h>
#include <assert.h>
#include <zlib.h>

enum ByteOrder
{
	BO_LITTLE_ENDIAN,		/*  Intel  */
	BO_BIG_ENDIAN,			/*  Motorola, MIPS, Sun, etc.  */
	BO_MACHINE,
	BO_LE  = BO_LITTLE_ENDIAN,
	BO_BE  = BO_BIG_ENDIAN,
	BO_CPU = BO_MACHINE
};

enum DataType
{
	DT_SHORT, DT_INT, DT_LONG, DT_FLOAT, DT_DOUBLE
};

/**
 * Determines the native byte order of the host machine.
 */
inline ByteOrder NativeByteOrder( void )
{
	static int iByteOrderTest = 0x1;

	if ( *((char *) &iByteOrderTest) == 0x1 )
		return BO_LITTLE_ENDIAN;
	else
		return BO_BIG_ENDIAN;
}

/*
 * Various macros that always swap byte order.  Expanded in-line for
 *   maximum speed when processing arrays.
 *
 */
inline short SwapShort( short data )
{
	return (data << 8) | ((data >> 8) & 0x00ff);
}
inline long SwapLong( long data )
{
	assert( sizeof(long) == 4 );
	return ( data << 24 )				 | (( data << 8 ) & 0x00FF0000 ) |
		   (( data >> 24 ) & 0x000000FF ) | (( data >> 8 ) & 0x0000FF00 );
}
inline int SwapInt( int data )
{
	// we don't need to support any ancient compilers where int==short
//	assert( sizeof(int) == sizeof(short) || sizeof(int) == sizeof(long) );
//	if ( sizeof(int) == sizeof(short) )
//		return SwapShort( (short)data );
//	else
		return SwapLong( (long)data );
}
inline float SwapFloat( float data )
{
	assert( sizeof(float) == sizeof(long) );
	long s = SwapLong( *((long *)&data) );
	return *((float *)&s);
}
inline double SwapDouble( double data )
{
	assert( sizeof(double) == 2*sizeof(long) );
	char *p  = (char *)&data;
	long  l1 = *((long *)( p + 0			)),
		  l2 = *((long *)( p + sizeof(long) ));
	*((long *)( p + 0			)) = SwapLong( l2 );
	*((long *)( p + sizeof(long) )) = SwapLong( l1 );
	return data;
}

/**
 * If the byte orders differ, swap bytes; if not, don't; return the result.
 * \param data the data to order
 * \param data_order the order of the data
 * \param desired_order the desired byte ordering
 * \return data, potentially byte swapped
 *
 */
inline short SwapBytes( short	 data,
						ByteOrder data_order,
						ByteOrder desired_order )
{
	if ( data_order	== BO_MACHINE ) data_order	= NativeByteOrder();
	if ( desired_order == BO_MACHINE ) desired_order = NativeByteOrder();
	if ( data_order == desired_order )
		return data;
	else
		return SwapShort( data );
}

inline long SwapBytes( long	  data,
					   ByteOrder data_order,
					   ByteOrder desired_order )
{
	if ( data_order	== BO_MACHINE ) data_order	= NativeByteOrder();
	if ( desired_order == BO_MACHINE ) desired_order = NativeByteOrder();
	if ( data_order == desired_order )
		return data;
	else
		return SwapLong( data );
}

inline int SwapBytes( int		data,
					  ByteOrder data_order,
					  ByteOrder desired_order )
{
	if ( data_order	== BO_MACHINE ) data_order	= NativeByteOrder();
	if ( desired_order == BO_MACHINE ) desired_order = NativeByteOrder();
	if ( data_order == desired_order )
		return data;
	else
		return SwapInt( data );
}

inline float SwapBytes( float	 data,
						ByteOrder data_order,
						ByteOrder desired_order )
{
	if ( data_order	== BO_MACHINE ) data_order	= NativeByteOrder();
	if ( desired_order == BO_MACHINE ) desired_order = NativeByteOrder();
	if ( data_order == desired_order )
		return data;
	else
		return SwapFloat( data );
}

inline double SwapBytes( double	data,
						 ByteOrder data_order,
						 ByteOrder desired_order )
{
	if ( data_order	== BO_MACHINE ) data_order	= NativeByteOrder();
	if ( desired_order == BO_MACHINE ) desired_order = NativeByteOrder();
	if ( data_order == desired_order )
		return data;
	else
		return SwapDouble( data );
}

void SwapMemBytes( void *items, DataType type, size_t count,
				   ByteOrder file_order, ByteOrder desired_order );

size_t FRead( void *ptr, DataType type, size_t nitems, FILE *stream,
			  ByteOrder file_order, ByteOrder desired_order = BO_MACHINE );

size_t GZFRead( void *ptr, DataType type, size_t nitems, gzFile gzstream,
			  ByteOrder file_order, ByteOrder desired_order = BO_MACHINE );

size_t FWrite( void *ptr, DataType type, size_t nitems, FILE *stream,
			  ByteOrder file_order, ByteOrder data_order = BO_MACHINE );

size_t GZFWrite( void *ptr, DataType type, size_t nitems, gzFile gzstream,
			  ByteOrder file_order, ByteOrder data_order = BO_MACHINE );

#endif
