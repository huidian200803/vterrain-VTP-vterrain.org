// (c) by Stefan Roettger

#ifndef JPEGBASE_H
#define JPEGBASE_H

namespace jpegbase {

unsigned char *decompressJPEGimage(unsigned char *data,unsigned int bytes,int *width,int *height,int *components);
void compressJPEGimage(unsigned char *image,int width,int height,int components,float quality,unsigned char **data,unsigned int *bytes);

}

#endif
