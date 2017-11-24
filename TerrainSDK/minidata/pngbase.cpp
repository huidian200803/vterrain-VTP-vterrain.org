// (c) by Stefan Roettger

#include <stdio.h>

extern "C"
   {
   #include <png.h>
   }

// compatibility with libpng 1.4
#if ((PNG_LIBPNG_VER_MAJOR==1 && PNG_LIBPNG_VER_MINOR>=4) || PNG_LIBPNG_VER_MAJOR>1)
#define int_p_NULL NULL
#define png_voidp_NULL NULL
#define png_infopp_NULL NULL
#define png_error_ptr_NULL NULL
#define png_set_gray_1_2_4_to_8 png_set_expand_gray_1_2_4_to_8
#endif

#include <mini/minibase.h>

#include "pngbase.h"

namespace pngbase {

struct PNG_USER_READ_STRUCT
   {
   unsigned char *data;
   unsigned int bytes;
   unsigned int ptr;
   };

typedef PNG_USER_READ_STRUCT PNG_USER_READ_PARAMS;

// user read function
void user_read_fn(png_structp png_ptr,png_bytep ptr,png_size_t size)
   {
   PNG_USER_READ_PARAMS *params=(PNG_USER_READ_PARAMS *)png_get_io_ptr(png_ptr);

   if (params->ptr+size>params->bytes) png_error(png_ptr,"read error");
   else
      {
      memcpy(ptr,&params->data[params->ptr],size);
      params->ptr+=(unsigned int)size;
      }
   }

// decompress PNG image
unsigned char *decompressPNGimage(unsigned char *data,unsigned int bytes,int *width,int *height,int *components)
   {
   unsigned int i;

   // declare PNG pointers
   png_structp png_ptr;
   png_infop info_ptr;

   // declare PNG image parameters
   png_uint_32 image_width,image_height,image_channels;
   int bit_depth,color_type,interlace_type;
   double gamma;

   // declare PNG image chunk
   unsigned char *image_data;

   // declare PNG row pointers
   unsigned char **row_pointers;
   unsigned int row_bytes;

   // declare PNG user read parameters
   PNG_USER_READ_PARAMS png_user_read_params;

   // check PNG signature
   if (bytes<8) return(NULL);
   if (png_sig_cmp(data,0,8)) return(NULL);

   // create PNG read struct
   png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING,png_voidp_NULL,png_error_ptr_NULL,png_error_ptr_NULL);
   if (png_ptr==NULL) return(NULL);

   // create PNG info struct
   info_ptr=png_create_info_struct(png_ptr);
   if (info_ptr==NULL)
      {
      png_destroy_read_struct(&png_ptr,png_infopp_NULL,png_infopp_NULL);
      return(NULL);
      }

   // set PNG error handler
   if (setjmp(png_jmpbuf(png_ptr)))
      {
      png_destroy_read_struct(&png_ptr,&info_ptr,png_infopp_NULL);
      return(NULL);
      }

   // initialize PNG user read parameters
   png_user_read_params.data=data;
   png_user_read_params.bytes=bytes;
   png_user_read_params.ptr=8;

   // set PNG user read function
   png_set_read_fn(png_ptr,(void *)&png_user_read_params,user_read_fn);

   // tell PNG that the header has been read
   png_set_sig_bytes(png_ptr,8);

   // ask PNG to read the image header
   png_read_info(png_ptr,info_ptr);

   // get PNG image header
   png_get_IHDR(png_ptr,info_ptr,
                &image_width,&image_height,&bit_depth,&color_type,&interlace_type,
                int_p_NULL,int_p_NULL);

   // ask PNG to strip 16 bit colors down to 8 bits
   png_set_strip_16(png_ptr);

   // ask PNG to extract multiple pixels into separate bytes
   png_set_packing(png_ptr);

   // ask PNG to expand paletted colors into true RGB triplets
   if (color_type==PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);

   // ask PNG to expand gray-scale images to full 8 bits
   if (color_type==PNG_COLOR_TYPE_GRAY && bit_depth<8) png_set_gray_1_2_4_to_8(png_ptr);

   // ask PNG to expand paletted or RGB images with transparency to full alpha channels
   if (png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);

   // set PNG gamma value
   if (!png_get_gAMA(png_ptr,info_ptr,&gamma)) gamma=1.0/2.2; // assume neutral gamma
   png_set_gamma(png_ptr,2.2,gamma);

   // update PNG info struct
   png_read_update_info(png_ptr,info_ptr);

   // get PNG number of channels
   image_channels=png_get_channels(png_ptr,info_ptr);

   // get PNG bytes per row
   row_bytes=png_get_rowbytes(png_ptr,info_ptr);

   // allocate the memory to hold the image
   if ((image_data=(unsigned char *)malloc(row_bytes*image_height))==NULL)
      {
      png_destroy_read_struct(&png_ptr,&info_ptr,png_infopp_NULL);
      return(NULL);
      }

   // allocate PNG row pointers
   if ((row_pointers=(unsigned char **)malloc(image_height*sizeof(unsigned char *)))==NULL)
      {
      png_destroy_read_struct(&png_ptr,&info_ptr,png_infopp_NULL);
      return(NULL);
      }

   // prepare PNG row pointers
   for (i=0; i<image_height; i++) row_pointers[i]=image_data+i*row_bytes;

   // finally read the PNG image
   png_read_image(png_ptr,row_pointers);

   // free PNG row pointers
   free(row_pointers);

   // read the rest of the PNG image
   png_read_end(png_ptr,info_ptr);

   // clean up and free any memory allocated
   png_destroy_read_struct(&png_ptr,&info_ptr,png_infopp_NULL);

   // return image parameters
   *width=image_width;
   *height=image_height;
   *components=image_channels;

   // return decompressed image
   return(image_data);
   }

struct PNG_USER_WRITE_STRUCT
   {
   unsigned char *data;
   unsigned int bytes;
   unsigned int chunk;
   unsigned int inc;
   };

typedef PNG_USER_WRITE_STRUCT PNG_USER_WRITE_PARAMS;

// user write function
void user_write_fn(png_structp png_ptr,png_bytep ptr,png_size_t size)
   {
   PNG_USER_WRITE_PARAMS *params=(PNG_USER_WRITE_PARAMS *)png_get_io_ptr(png_ptr);

   if (params->bytes+size>params->chunk)
      {
      while (params->chunk<params->bytes+size) params->chunk+=params->inc;

      if (params->data==NULL)
         {
         params->data=(unsigned char *)malloc(params->chunk);
         if (params->data==NULL) png_error(png_ptr,"write error");
         }
      else
         {
         params->data=(unsigned char *)realloc(params->data,params->chunk);
         if (params->data==NULL) png_error(png_ptr,"write error");
         }
      }

   memcpy(&params->data[params->bytes],ptr,size);
   params->bytes+=(unsigned int)size;
   }

// user flush function
void user_flush_fn(png_structp png_ptr)
   {
   PNG_USER_READ_PARAMS *params=(PNG_USER_READ_PARAMS *)png_get_io_ptr(png_ptr);

   params->data=(unsigned char *)realloc(params->data,params->bytes);
   if (params->data==NULL) png_error(png_ptr,"write error");
   }

// compress PNG image
void compressPNGimage(unsigned char *image,int width,int height,int components,unsigned char **data,unsigned int *bytes,float gamma,int level)
   {
   int i;

   // declare PNG pointers
   png_structp png_ptr;
   png_infop info_ptr;

   // declare PNG image parameters
   int color_type,sample_depth,interlace_type;

   // declare PNG row pointers
   unsigned char **row_pointers;
   unsigned int row_bytes;

   // declare PNG user write parameters
   PNG_USER_WRITE_PARAMS png_user_write_params;

   *data=NULL;
   *bytes=0;

   // create PNG write struct
   png_ptr=png_create_write_struct(PNG_LIBPNG_VER_STRING,png_voidp_NULL,png_error_ptr_NULL,png_error_ptr_NULL);
   if (png_ptr==NULL) return;

   // create PNG info struct
   info_ptr=png_create_info_struct(png_ptr);
   if (info_ptr==NULL)
      {
      png_destroy_write_struct(&png_ptr,png_infopp_NULL);
      return;
      }

   // set PNG error handler
   if (setjmp(png_jmpbuf(png_ptr)))
      {
      png_destroy_write_struct(&png_ptr,&info_ptr);
      return;
      }

   // initialize PNG user write parameters
   png_user_write_params.data=NULL;
   png_user_write_params.bytes=0;
   png_user_write_params.chunk=0;
   png_user_write_params.inc=100000;

   // set PNG user write function
   png_set_write_fn(png_ptr,(void *)&png_user_write_params,user_write_fn,user_flush_fn);

   // set PNG gamma value
   if (gamma<=0.0f) gamma=2.2f; // assume neutral gamma
   png_set_gamma(png_ptr,gamma,1.0/2.2);

   // set PNG compression level
   png_set_compression_level(png_ptr,level);

   // set PNG image type
   if (components==1 || components==2) color_type=PNG_COLOR_TYPE_GRAY;
   else if (components==3) color_type=PNG_COLOR_TYPE_RGB;
   else if (components==4) color_type=PNG_COLOR_TYPE_RGB_ALPHA;
   else
      {
      png_destroy_write_struct(&png_ptr,&info_ptr);
      return;
      }

   // set PNG sample depth
   if (components==2) sample_depth=16;
   else sample_depth=8;

   // set PNG interlace type
   interlace_type=PNG_INTERLACE_NONE;

   // set PNG image type
   png_set_IHDR(png_ptr,info_ptr,
                width,height,sample_depth,color_type,interlace_type,
                PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);

   // write PNG info struct
   png_write_info(png_ptr,info_ptr);

   // ask PNG to pack separate bytes into multiple pixels
   png_set_packing(png_ptr);

   // get PNG bytes per row
   row_bytes=png_get_rowbytes(png_ptr,info_ptr);

   // allocate PNG row pointers
   if ((row_pointers=(unsigned char **)malloc(height*sizeof(unsigned char *)))==NULL)
      {
      png_destroy_write_struct(&png_ptr,&info_ptr);
      return;
      }

   // prepare PNG row pointers
   for (i=0; i<height; i++) row_pointers[i]=image+i*row_bytes;

   // finally write the PNG image
   png_write_image(png_ptr,row_pointers);

   // free PNG row pointers
   free(row_pointers);

   // write the rest of the PNG image
   png_write_end(png_ptr,info_ptr);

   // clean up and free any memory allocated
   png_destroy_write_struct(&png_ptr,&info_ptr);

   // return compressed image
   *data=png_user_write_params.data;
   *bytes=png_user_write_params.bytes;
   }

}
