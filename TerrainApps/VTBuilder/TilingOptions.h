//
// TilingOptions.h
//
// Copyright (c) 2006-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TilingOptions_H
#define TilingOptions_H

#include "ElevDrawOptions.h"

enum TextureCompressionType { TC_OPENGL, TC_SQUISH_FAST, TC_SQUISH_SLOW, TC_JPEG };

/**
 * All the options needed to describe how to create a tileset.
 */
class TilingOptions
{
public:
	TilingOptions()
	{
		bCreateDerivedImages = false;
		bMaskUnknownAreas = false;
		bImageAlpha = false;
		bOmitFlatTiles = false;
		bUseTextureCompression = false;
		eCompressionType = TC_OPENGL;
		iNoDataFilled = 0;
		iMinCol = -1;
		iMaxCol = -1;
		iMinRow = -1;
		iMaxRow = -1;
	}

	int cols, rows;
	int lod0size;
	int numlods;
	vtString fname;

	// If this is an elevation tileset, then create a corresponding derived
	//  image tileset.
	bool bCreateDerivedImages;

	// If creating a derived image tileset, then use alpha values to mask where
	//  the elevation has NODATA.
	bool bMaskUnknownAreas;

	// If creating imagery tiles, use alpha values. Implied by bMaskUnknownAreas.
	bool bImageAlpha;

	vtString fname_images;
	ElevDrawOptions draw;

	// If elevation, we can omit flat (sea-level) tiles
	bool bOmitFlatTiles;

	// If imagery (or deriving imagery from elevation) then it can be compressed
	bool bUseTextureCompression;
	TextureCompressionType eCompressionType;

	// These can be set to restrict the output to certain (tile) rows only
	int iMinCol, iMaxCol;
	int iMinRow, iMaxRow;

	// after the sampling, will contain the number of NODATA heixels filled in
	int iNoDataFilled;
};

#endif // TilingOptions_H

