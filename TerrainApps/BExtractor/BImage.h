
#ifndef CBIMAGE_H
#define CBIMAGE_H

class CDib;
class OGRSpatialReference;

class CBImage
{
public:
	CBImage();
	~CBImage();

	bool LoadFromFile(const char *szPathName, CDC *pDC, HDRAWDIB hdd);
	bool LoadTFW(const char *szPathName);

private:
	bool LoadGDAL(const char *szPathName, CDC *pDC, HDRAWDIB hdd);

public:
	OGRSpatialReference *m_pSpatialReference;
	CDib	*m_pSourceDIB, *m_pMonoDIB, *m_pCurrentDIB;
	CBitmap m_bmp;
	bool	m_initialized;

	CPoint	m_PixelSize;			// size of bitmap from the file
	double	m_xUTMoffset;			//data obtained from image world file (.tfw)
	double	m_yUTMoffset;
	double	m_fImageWidth;		// in UTM meters
	double	m_fImageHeight;		// same
	double	m_xMetersPerPixel;
	double	m_yMetersPerPixel;
};

#endif CBIMAGE_H
