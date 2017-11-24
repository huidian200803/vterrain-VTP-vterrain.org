// (c) by Stefan Roettger

#include <stdio.h>

extern "C"
   {
   #include <jpeglib.h>
   }

#include <mini/minibase.h>

#include "jpegbase.h"

namespace jpegbase {

typedef struct
   {
   struct jpeg_source_mgr pub;
   }
my_source_mgr;

typedef my_source_mgr *my_src_ptr;

void my_init_source(j_decompress_ptr cinfo)
   {if (cinfo==NULL) ERRORMSG();}

boolean my_fill_input_buffer(j_decompress_ptr cinfo)
   {
   if (cinfo==NULL) ERRORMSG();

   ERRORMSG();
   return(TRUE);
   }

void my_skip_input_data(j_decompress_ptr cinfo,long num_bytes)
   {
   my_src_ptr src=(my_src_ptr)cinfo->src;

   src->pub.next_input_byte+=(size_t)num_bytes;
   src->pub.bytes_in_buffer-=(size_t)num_bytes;
   }

void my_term_source(j_decompress_ptr cinfo)
   {if (cinfo==NULL) ERRORMSG();}

void jpeg_mem_src(j_decompress_ptr cinfo,unsigned char *data,unsigned int bytes)
   {
   my_src_ptr src;

   if (cinfo->src==NULL)
      cinfo->src=(struct jpeg_source_mgr *)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo,JPOOL_PERMANENT,sizeof(my_source_mgr));

   src=(my_src_ptr)cinfo->src;

   src->pub.init_source=my_init_source;
   src->pub.fill_input_buffer=my_fill_input_buffer;
   src->pub.skip_input_data=my_skip_input_data;
   src->pub.resync_to_restart=jpeg_resync_to_restart;
   src->pub.term_source=my_term_source;

   src->pub.bytes_in_buffer=bytes;
   src->pub.next_input_byte=data;
   }

unsigned char *decompressJPEGimage(unsigned char *data,unsigned int bytes,int *width,int *height,int *components)
   {
   int i;

   unsigned char *image,**ptr;

   struct jpeg_error_mgr error;
   struct jpeg_decompress_struct info;

   info.err=jpeg_std_error(&error);
   jpeg_create_decompress(&info);
   jpeg_mem_src(&info,data,bytes);

   if (jpeg_read_header(&info,TRUE)!=JPEG_HEADER_OK) return(NULL);

   *width=info.image_width;
   *height=info.image_height;
   *components=info.num_components;

   if ((image=(unsigned char *)malloc((*width)*(*height)*(*components)))==NULL) MEMERROR();
   if ((ptr=(unsigned char **)malloc((*height)*sizeof(unsigned char *)))==NULL) MEMERROR();
   for (i=0; i<*height; i++) ptr[i]=&image[(*width)*i*(*components)];

   jpeg_start_decompress(&info);
   while (info.output_scanline<info.output_height)
      jpeg_read_scanlines(&info,&ptr[info.output_scanline],1);

   jpeg_finish_decompress(&info);
   jpeg_destroy_decompress(&info);
   if (error.num_warnings!=0) ERRORMSG();

   free(ptr);

   return(image);
   }

typedef struct
   {
   struct jpeg_destination_mgr pub;

   unsigned char **data;
   unsigned int *bytes;

   unsigned int size;
   }
my_destination_mgr;

typedef my_destination_mgr *my_dest_ptr;

void init_destination(j_compress_ptr cinfo)
   {
   const unsigned int basesize=10000;

   my_dest_ptr dest=(my_dest_ptr)cinfo->dest;

   if (dest->size==0)
      {
      dest->size=basesize;
      if ((*(dest->data)=(unsigned char *)malloc(dest->size))==NULL) MEMERROR();
      }

   dest->pub.next_output_byte=*(dest->data);
   dest->pub.free_in_buffer=basesize;
   }

boolean empty_output_buffer(j_compress_ptr cinfo)
   {
   my_dest_ptr dest=(my_dest_ptr)cinfo->dest;

   if ((*(dest->data)=(unsigned char *)realloc(*(dest->data),2*dest->size))==NULL) MEMERROR();

   dest->pub.next_output_byte=(*(dest->data))+dest->size;
   dest->pub.free_in_buffer=dest->size;

   dest->size*=2;

   return(TRUE);
   }

void term_destination(j_compress_ptr cinfo)
   {
   my_dest_ptr dest=(my_dest_ptr)cinfo->dest;

   *(dest->bytes)=dest->size-dest->pub.free_in_buffer;
   if ((*(dest->data)=(unsigned char *)realloc(*(dest->data),*(dest->bytes)))==NULL) MEMERROR();
   }

void jpeg_mem_dest(j_compress_ptr cinfo,unsigned char **data,unsigned int *bytes)
   {
   my_dest_ptr dest;

   if (cinfo->dest==NULL)
      cinfo->dest=(struct jpeg_destination_mgr *)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo,JPOOL_PERMANENT,sizeof(my_destination_mgr));

   dest=(my_dest_ptr)cinfo->dest;

   dest->pub.init_destination=init_destination;
   dest->pub.empty_output_buffer=empty_output_buffer;
   dest->pub.term_destination=term_destination;

   dest->data=data;
   dest->bytes=bytes;
   dest->size=0;
   }

void compressJPEGimage(unsigned char *image,int width,int height,int components,float quality,unsigned char **data,unsigned int *bytes)
   {
   int i;

   unsigned char **ptr;

   struct jpeg_error_mgr error;
   struct jpeg_compress_struct info;

   info.err=jpeg_std_error(&error);
   jpeg_create_compress(&info);
   jpeg_mem_dest(&info,data,bytes);

   if ((ptr=(unsigned char **)malloc(height*sizeof(unsigned char *)))==NULL) MEMERROR();
   for (i=0; i<height; i++) ptr[i]=&image[width*i*components];

   info.image_width=width;
   info.image_height=height;
   info.input_components=components;

   if (components==1) info.in_color_space=JCS_GRAYSCALE;
   else if (components==3) info.in_color_space=JCS_RGB;
   else if (components==4) ERRORMSG();

   jpeg_set_defaults(&info);
   jpeg_set_quality(&info,ftrc(100*quality+0.5f),TRUE);

   jpeg_start_compress(&info,TRUE);

   while (info.next_scanline<info.image_height)
      jpeg_write_scanlines(&info,&ptr[info.next_scanline],1);

   jpeg_finish_compress(&info);
   jpeg_destroy_compress(&info);
   if (error.num_warnings!=0) ERRORMSG();

   free(ptr);
   }

}
