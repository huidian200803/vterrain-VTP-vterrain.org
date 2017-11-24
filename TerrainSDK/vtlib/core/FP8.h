/* homegrown floating point: 3 bits exponent, 8 bit mantissa
   maps
   0.. 31  ->     0..  31
  32.. 63  ->    32..  63
  64.. 95  ->    64.. 126, step 2
  96..127  ->   128.. 252, step 4
 128..159  ->   256.. 504, step 8
 160..191  ->   512..1008, step 16
 192..223  ->  1024..2016, step 32
 224..255  ->  2048..4032, step 64
*/

#ifdef _MSC_VER
#  define INLINE __forceinline
#else
#  define INLINE inline
#endif

INLINE uchar EncodeFP8( unsigned short nVal )
{
	if( nVal < 64   ) return (uchar) nVal;
	if( nVal < 128  ) return (0x40 | (nVal-  64)/2);
	if( nVal < 256  ) return (0x60 | (nVal- 128)/4);
	if( nVal < 512  ) return (0x80 | (nVal- 256)/8);
	if( nVal < 1024 ) return (0xa0 | (nVal- 512)/16);
	if( nVal < 2048 ) return (0xc0 | (nVal-1024)/32);
	if( nVal < 4096 ) return (0xe0 | (nVal-2048)/64);
					  return  0xff;
}

INLINE unsigned short DecodeFP8( uchar nVal )
{
	int a = nVal >> 5;
	int b = nVal & 31;

	if( a < 2 ) {
		return b;
	} else {
		return (b + 32) << (a-1);
	}
}


