// (c) by Stefan Roettger

#include <stdio.h>

extern "C"
   {
   #include <zlib.h>
   }

#include <mini/minibase.h>

#include "zlibbase.h"

namespace zlibbase {

// decompress with ZLIB
unsigned char *decompressZLIB(unsigned char *chunk,unsigned int chunklen,unsigned int *bytes)
   {
   int i;

   Bytef *mem;
   uLongf len;

   int status;

   len=4*chunklen;

   for (i=0; i<10; i++)
      {
      if ((mem=(Bytef *)malloc(len))==NULL) break;

      if ((status=uncompress(mem,&len,chunk,chunklen))==Z_OK)
         {
         if ((mem=(Bytef *)realloc(mem,len))==NULL) break;
         *bytes=len;
         return(mem);
         }

      if (status!=Z_BUF_ERROR) break;

      free(mem);
      len*=2;
      }

   return(NULL);
   }

// compress with ZLIB
void compressZLIB(unsigned char *data,unsigned int bytes,unsigned char **chunk,unsigned int *chunklen,int level)
   {
   int i;

   Bytef *mem;
   uLongf len;

   int status;

   len=2*bytes+12;

   for (i=0; i<10; i++)
      {
      if ((mem=(Bytef *)malloc(len))==NULL) break;

      if ((status=compress2(mem,&len,data,bytes,level))==Z_OK)
         {
         if (((*chunk)=(unsigned char *)realloc(mem,len))==NULL) break;
         *chunklen=len;
         return;
         }

      if (status!=Z_BUF_ERROR) break;

      free(mem);
      len*=2;
      }

   *chunk=NULL;
   *chunklen=0;
   }

}
