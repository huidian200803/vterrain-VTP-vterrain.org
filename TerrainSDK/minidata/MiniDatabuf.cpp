//
// MiniDatabuf.cpp
//
// Copyright (c) 2006-2008 Virtual Terrain Project and Stefan Roettger
// Free for all uses, see license.txt for details.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

// Headers for JPEG support, which uses the library "libjpeg"
extern "C" {
#include "jpeglib.h"
}

#include "MiniDatabuf.h"
#include "vtdata/vtLog.h"
#include "vtdata/vtCRS.h"
#include "vtdata/MathTypes.h"
#include "vtdata/FilePath.h"

#define ERRORMSG() VTLOG1("Error!")

#if !USE_LIBMINI_DATABUF

// magic identifier history
// MAGIC1=12640; // original magic identifier of DB version 1
// MAGIC2=13048; // backwards compatibility for DB version 2
// MAGIC3=13091; // backwards compatibility for DB version 3
// MAGIC4=13269; // backwards compatibility for DB version 4
// MAGIC5=13398; // actual magic identifier of DB version 5

int MiniDatabuf::MAGIC=13269;

unsigned short int MiniDatabuf::INTEL_CHECK=1;

// default constructor
MiniDatabuf::MiniDatabuf()
{
	xsize=ysize=zsize=0;
	tsteps=0;
	type=0;

	swx=swy=0.0f;
	nwx=nwy=0.0f;
	nex=ney=0.0f;
	sex=sey=0.0f;
	h0=dh=0.0f;
	t0=dt=0.0f;

	scaling=1.0f;
	bias=0.0f;

	data=NULL;
	bytes=0;
}

MiniDatabuf::~MiniDatabuf()
{
	release();
}

// set the geographic extents
void MiniDatabuf::set_extents(float left, float right, float bottom, float top)
{
	swx = left;
	swy = bottom;
	nwx = left;
	nwy = top;
	sex = right;
	sey = bottom;
	nex = right;
	ney = top;
}

// set LLWGS84 corners
void MiniDatabuf::set_LLWGS84corners(float sw_corner_x,float sw_corner_y,
									 float se_corner_x,float se_corner_y,
									 float nw_corner_x,float nw_corner_y,
									 float ne_corner_x,float ne_corner_y)
{
	LLWGS84_swx=sw_corner_x;
	LLWGS84_swy=sw_corner_y;

	LLWGS84_sex=se_corner_x;
	LLWGS84_sey=se_corner_y;

	LLWGS84_nwx=nw_corner_x;
	LLWGS84_nwy=nw_corner_y;

	LLWGS84_nex=ne_corner_x;
	LLWGS84_ney=ne_corner_y;
}

// A useful method to set the extents (in local CRS) and the corners
//  (in Geo WGS84) at the same time.
bool MiniDatabuf::SetBounds(const vtCRS &crs, const DRECT &extents)
{
	// First, set the extent rectangle
	set_extents((float)extents.left, (float)extents.right, (float)extents.bottom, (float)extents.top);

	// Create transform from local to Geo-WGS84
	vtCRS geo;
	geo.SetWellKnownGeogCS("WGS84");
	ScopedOCTransform trans(CreateCoordTransform(&crs, &geo));

	if (!trans)
		return false;

	// Convert each corner as a point
	DPoint2 sw_corner, se_corner, nw_corner, ne_corner;

	sw_corner.Set(extents.left, extents.bottom);
	trans->Transform(1, &sw_corner.x, &sw_corner.y);

	se_corner.Set(extents.right, extents.bottom);
	trans->Transform(1, &se_corner.x, &se_corner.y);

	nw_corner.Set(extents.left, extents.top);
	trans->Transform(1, &nw_corner.x, &nw_corner.y);

	ne_corner.Set(extents.right, extents.top);
	trans->Transform(1, &ne_corner.x, &ne_corner.y);

	set_LLWGS84corners((float)sw_corner.x, (float)sw_corner.y,
                       (float)se_corner.x, (float)se_corner.y,
                       (float)nw_corner.x, (float)nw_corner.y,
                       (float)ne_corner.x, (float)ne_corner.y);
	return true;
}

// allocate a new memory chunk
void MiniDatabuf::alloc(unsigned int xs,unsigned int ys,unsigned int zs,unsigned int ts,unsigned int ty)
{
	unsigned int bs,cs;

	unsigned char *byteptr;
	short int *shortptr;
	float *floatptr;

	unsigned int count;

	if (ty==0) bs=1;
	else if (ty==1) bs=2;
	else if (ty==2) bs=4;
	else if (ty==3) bs=3;
	else if (ty==4) bs=3;
	else if (ty==5) bs=4;
	else if (ty==6) bs=4;
	else ERRORMSG();

	cs=xs*ys*zs*ts;
	bs*=cs;

	if (ty==4 || ty==6) bs/=6;

	if ((data=malloc(bs))==NULL) ERRORMSG();

	if (ty==1)
		for (shortptr=(short int *)data,count=0; count<cs; count++) *shortptr++=0;
	else if (ty==2)
		for (floatptr=(float *)data,count=0; count<cs; count++) *floatptr++=0.0f;
	else
		for (byteptr=(unsigned char *)data,count=0; count<bs; count++) *byteptr++=0;

	bytes=bs;

	xsize=xs;
	ysize=ys;
	zsize=zs;
	tsteps=ts;
	type=ty;
}

// reset buffer
void MiniDatabuf::reset()
{
	data=NULL;
	bytes=0;
}

// release buffer
void MiniDatabuf::release()
{
	if (data!=NULL) free(data);

	data=NULL;
	bytes=0;
}

// data is saved in MSB format
void MiniDatabuf::savedata(const char *filename)
{
	FILE *file;

	if (data==NULL) return;

	if (bytes==0) ERRORMSG();

	if ((file=vtFileOpen(filename,"wb"))==NULL) ERRORMSG();

	// save magic identifier
	fprintf(file,"MAGIC=%d\n",MAGIC);

	// save mandatory metadata
	fprintf(file,"xsize=%d\n",xsize);
	fprintf(file,"ysize=%d\n",ysize);
	fprintf(file,"zsize=%d\n",zsize);
	fprintf(file,"tsteps=%d\n",tsteps);
	fprintf(file,"type=%d\n",type);

	// save optional metadata
	fprintf(file,"swx=%f\n",swx);
	fprintf(file,"swy=%f\n",swy);
	fprintf(file,"nwx=%f\n",nwx);
	fprintf(file,"nwy=%f\n",nwy);
	fprintf(file,"nex=%f\n",nex);
	fprintf(file,"ney=%f\n",ney);
	fprintf(file,"sex=%f\n",sex);
	fprintf(file,"sey=%f\n",sey);
	fprintf(file,"h0=%f\n",h0);
	fprintf(file,"dh=%f\n",dh);
	fprintf(file,"t0=%f\n",t0);
	fprintf(file,"dt=%f\n",dt);

	// save optional scaling
	fprintf(file,"scaling=%f\n",scaling);
	fprintf(file,"bias=%f\n",bias);

	// libmini format stuff
	fprintf(file,"extformat=0\n");
	fprintf(file,"implformat=0\n");

	// save optional corner points in Lat/Lon
	fprintf(file,"LLWGS84_swx=%f\n",LLWGS84_swx);
	fprintf(file,"LLWGS84_swy=%f\n",LLWGS84_swy);
	fprintf(file,"LLWGS84_nwx=%f\n",LLWGS84_nwx);
	fprintf(file,"LLWGS84_nwy=%f\n",LLWGS84_nwy);
	fprintf(file,"LLWGS84_nex=%f\n",LLWGS84_nex);
	fprintf(file,"LLWGS84_ney=%f\n",LLWGS84_ney);
	fprintf(file,"LLWGS84_sex=%f\n",LLWGS84_sex);
	fprintf(file,"LLWGS84_sey=%f\n",LLWGS84_sey);

	// save length of data chunk
	fprintf(file,"bytes=%u\n",bytes);

	// write zero byte
	fputc(0, file);

	// save data chunk
	if (*((unsigned char *)(&INTEL_CHECK))==0)
	{
		if (fwrite(data,bytes,1,file)!=1) ERRORMSG();
		fclose(file);
	}
	else
	{
		swapbytes();
		if (fwrite(data,bytes,1,file)!=1) ERRORMSG();
		swapbytes();

		fclose(file);
	}
}

//
// data is saved in JPEG format
// \param quality	JPEG quality in the range of 0..100.
//
bool MiniDatabuf::savedataJPEG(const char *filename, int quality)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	FILE *outfile = vtFileOpen(filename, "wb");
	if (outfile == NULL)
		return false;

	/* Initialize the JPEG decompression object with default error handling. */
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	/* Specify data source for decompression */
	jpeg_stdio_dest(&cinfo, outfile);

	// set parameters for compression
	cinfo.image_width = xsize;	/* image width and height, in pixels */
	cinfo.image_height = ysize;
	cinfo.input_components = 3;	/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB;	/* colorspace of input image */

	// Now use the library's routine to set default compression parameters.
	jpeg_set_defaults(&cinfo);

	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

	/* Start compressor */
	jpeg_start_compress(&cinfo, TRUE);

	/* Process each scanline */
	int row_stride = cinfo.image_width * cinfo.input_components;
	while (cinfo.next_scanline < cinfo.image_height)
	{
		JSAMPROW row_buffer = (JSAMPROW)data + cinfo.next_scanline * row_stride;
		jpeg_write_scanlines(&cinfo, &row_buffer, 1);
	}
	jpeg_finish_compress(&cinfo);

	/* After finish_compress, we can close the output file. */
	fclose(outfile);

	jpeg_destroy_compress(&cinfo);

	return true;
}

// swap byte ordering between MSB and LSB
void MiniDatabuf::swapbytes()
{
	unsigned int i,b;

	unsigned char *ptr,tmp;

	if (type==0 || (type>=3 && type<=6)) return;

	if (type==1) b=2;
	else if (type==2) b=4;
	else ERRORMSG();

	if (bytes==0 || bytes%b!=0) ERRORMSG();

	ptr=(unsigned char *)data+bytes;

	while (ptr!=data)
	{
		ptr-=b;

		for (i=0; i<(b>>1); i++)
		{
			tmp=ptr[i];
			ptr[i]=ptr[b-1-i];
			ptr[b-1-i]=tmp;
		}
	}
}

#endif // !USE_LIBMINI_DATABUF

int mapCRS2MINI(const vtCRS &crs)
{
    const char *crsname = crs.GetAttrValue("PROJECTION");

    if (crs.IsGeographic()) return(1);
    else if (crs.GetUTMZone()!=0) return(2);
    else if (EQUAL(crsname,SRS_PT_MERCATOR_1SP)) return(3);

    return(0);
}

int mapEPSG2MINI(int epsgdatum)
{
	static const int NAD27=1;
	static const int WGS72=2;
	static const int WGS84=3;
	static const int NAD83=4;
	static const int SPHERE=5;
	static const int ED50=6;
	static const int ED79=7;
	static const int OHD=8;

	switch (epsgdatum)
	{
	case 6608: return(NAD27);
	case 6322: return(WGS72);
	case 6326: return(WGS84);
	case 6269: return(NAD83);
	case 6230: return(ED50);
	case 6231: return(ED79);
	case 6135: return(OHD);
	}

	return(SPHERE);
}

bool WriteTilesetHeader(const char *filename, int cols, int rows, int lod0size,
						const DRECT &area, const vtCRS &crs,
						float minheight, float maxheight,
						LODMap *lodmap, bool bJPEG)
{
	FILE *fp = vtFileOpen(filename, "wb");
	if (!fp)
		return false;

	fprintf(fp, "[TilesetDescription]\n");
	fprintf(fp, "Columns=%d\n", cols);
	fprintf(fp, "Rows=%d\n", rows);
	fprintf(fp, "LOD0_Size=%d\n", lod0size);
	fprintf(fp, "Extent_Left=%.16lg\n", area.left);
	fprintf(fp, "Extent_Right=%.16lg\n", area.right);
	fprintf(fp, "Extent_Bottom=%.16lg\n", area.bottom);
	fprintf(fp, "Extent_Top=%.16lg\n", area.top);
	// write CRS, but pretty it up a bit
	OGRSpatialReference *poSimpleClone = crs.Clone();
	poSimpleClone->GetRoot()->StripNodes( "AXIS" );
	poSimpleClone->GetRoot()->StripNodes( "AUTHORITY" );
	char *wkt;
	poSimpleClone->exportToWkt(&wkt);
	fprintf(fp, "CRS=%s\n", wkt);
	OGRFree(wkt);	// Free CRS
	delete poSimpleClone;

	// For elevation tilesets, also write vertical extents
	if (minheight != INVALID_ELEVATION)
	{
		fprintf(fp, "Elevation_Min=%.f\n", minheight);
		fprintf(fp, "Elevation_Max=%.f\n", maxheight);
	}

	if (lodmap != NULL)
	{
		int mmin, mmax;
		for (int i = 0; i < rows; i++)
		{
			fprintf(fp, "RowLODs %2d:", i);
			for (int j = 0; j < cols; j++)
			{
				lodmap->get(j, i, mmin, mmax);
				fprintf(fp, " %d/%d", mmin, mmax);
			}
			fprintf(fp, "\n");
		}
	}

	// Create a transformation that will map from the current CRS to Lat/Lon WGS84.
	vtCRS crs_llwgs84;
	crs_llwgs84.SetWellKnownGeogCS("WGS84");
	OCTransform *LLWGS84transform = CreateCoordTransform(&crs, &crs_llwgs84);

	// write center point of the tileset in Lat/Lon WGS84
	// this is helpful for libMini to compute an approximate translation
	double cx=(area.left+area.right)/2;
	double cy=(area.bottom+area.top)/2;
	if (LLWGS84transform->Transform(1,&cx,&cy)==1)
		fprintf(fp, "CenterPoint_LLWGS84=(%.16lg,%.16lg)\n",cx,cy);

	// write north point of the tileset in Lat/Lon WGS84
	// this is helpful for libMini to compute an approximate rotation
	double nx=(area.left+area.right)/2;
	double ny=area.top;
	if (LLWGS84transform->Transform(1,&nx,&ny)==1)
		fprintf(fp, "NorthPoint_LLWGS84=(%.16lg,%.16lg)\n",nx,ny);

	// delete Lat/Lon WGS84 transformation
	delete LLWGS84transform;

	// write CRS info
	// this is helpful for libMini to easily identify the coordinate reference system
	// supported CRS are: Geographic, UTM, Mercator
	const int crsnum=mapCRS2MINI(crs);
	const int datum=mapEPSG2MINI(crs.GetDatum());
	const int utmzone= crs.GetUTMZone();
	fprintf(fp, "CoordSys=(%d,%d,%d)\n", crsnum,datum,utmzone);

	if (bJPEG)
		fprintf(fp, "Format=JPEG\n");
	else
		fprintf(fp, "Format=DB\n");

	fclose(fp);

	return true;
}
