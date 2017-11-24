//
// SMTerrain class : Dynamically rendering terrain
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef SMTERRAINH
#define SMTERRAINH

#include <limits.h>
#include "DynTerrain.h"

/** \addtogroup dynterr */
/*@{*/

// Configure the implementation with the following definitions:

// Converts all elevation values to signed short integers, such that each
// unit = 0.25 meters.  The range of a short (*/-32768) allows an elevation
// range of +/- 8000 meters, which is enough for all the topography on Earth.
#define INTEGER_HEIGHT		1

// Store the frustum state (out, part in, all in) which each triangle so
// that they can be drawn with special colors to aid in debugging.
#define STORE_FRUSTUM		0

// Use quick (macro, no assert) triangle stack, may give performance increase
// (not compatible with STORE_FRUSTUM)
#define FAST_ALLOC			0

// Pack the variable values into a single byte, using the FP8 macros.
// This saves memory by 1 byte per heixel.
#define USE_FP8				1


/*
Seumas says:

  "I use bytes for my variance info, since my vertical height map
  resolution is 25cm. Since the variance tree can get pretty big,
  I'd suggest using bytes anyway, perhaps with a non-linear scaling
  if you need larger dynamic range."

  "I believe my implicit variance tree is max-4 levels deep, so imagine
  the finest tessellation possible, then step back 4 levels from there."
*/

#if INTEGER_HEIGHT
  typedef short HeightType;
  #if USE_FP8
	typedef uchar VarianceType;
  #else
	typedef unsigned short VarianceType;
  #endif
  typedef int MathType;		// use this type to perform arithmetic on
							// the heights and variances
#else
  typedef float HeightType;
  typedef float VarianceType;
  typedef float MathType;
#endif

#if STORE_FRUSTUM
  #define SetFrust(t, i) (t)->m_frust = i
#else
  #define SetFrust(t, i)
#endif

#define FRUST_OUT		0
#define FRUST_PARTIN	1
#define FRUST_ALLIN		2

class BinTri
{
public:
	BinTri *LeftChild;
	BinTri *RightChild;
	BinTri *LeftNeighbor;
	BinTri *RightNeighbor;
	BinTri *BottomNeighbor;
	// to add: some goop for geomorphs
#if STORE_FRUSTUM
	int m_frust;
#endif
};

class Block
{
public:
	// the two root triangles of the block
	BinTri *root[2];
	int num[2];
	int v0[2];
	int v1[2];
	int va[2];
};
typedef Block *BlockPtr;


/*!
	\par
	The SMTerrain class implements Seumas McNally's algorithm for regular-grid
	terrain LOD.  It was implemented directly from correspondence from the
	<a href="http://www.gamedev.net/community/memorial/seumas/">dearly missed</a>
	Seumas, and is highly similar to the implementation in his game
	<a href="http://www.treadmarks.com/">Treadmarks</a>.
	\par
	This implementation requires that the input heightfield is a square regular
	grid of dimension 2^n+1, e.g.. 1025, 2049, or 4097.  Storage requirements
	are around 3-5 bytes/vertex.  The data is represented in memory by a
	regular grid of height values, an implicit binary tree of variance values,
	and an explicit binary tree of visible faces.
	\par
	This is the fastest and most memory-efficient algorithm currently known for
	terrain rendering with a long viewing distance and an unconstrained viewpoint.
	\par
	<h4>Usage</h4>
	 -# Create a new SMTerrain object
	 -# Initialize it with a LocalGrid to provide height values
	 -# add it to your scene graph
	 -# set the desired level detail with SetPolygonTarget()
	 -# let the scene graph API render the terrain for you each frame
 */
class SMTerrain : public vtDynTerrainGeom
{
public:
	SMTerrain();

	/// initialization
	DTErr Init(const vtElevationGrid *pGrid, float fZScale);
	static int MemoryRequired(int iDimension);

	// overrides
	void DoRender();
	void DoCulling(const vtCamera *pCam);
	float GetElevation(int iX, int iZ, bool bTrue = false) const;
	void GetWorldLocation(int iX, int iZ, FPoint3 &p, bool bTrue = false) const;
	void SetVerticalExag(float fExag);
	float GetVerticalExag() const;

	float GetQualityConstant() const { return m_fQualityConstant; }

protected:
	// initialization
	void Init2();
	void AllocatePool();
	void ComputeVariances();
	MathType ComputeTriangleVariance(int num, int v0, int v1, int va, int level);
	void SetupBlocks();

	// per-frame
	void AdjustQualityConstant();
	void SplitToBlocks(int num, BinTri *tri, int v0, int v1, int va);
	void SplitIfNeeded(int num, BinTri *tri, int v0, int v1, int va, bool bEntirelyInFrustum, int level);
	void Split(BinTri *tri);
	void Split2(BinTri *tri);
#if !FAST_ALLOC
	inline BinTri *AllocateBinTri();
#endif

	// rendering
	void RenderSurface();
	void RenderBlock(BlockPtr block);
	bool BlockIsVisible(BlockPtr block);
	void LoadSingleMaterial();
	void render_triangle_as_fan(BinTri *pTri, int v0, int v1, int va,
		bool even, bool right);
	void render_triangle_single(BinTri *pTri, int v0, int v1, int va);
	void flush_buffer(int type);
	inline float DistanceToTriangle(int num);

	// cleanup
	virtual ~SMTerrain();

private:
	HeightType *m_pData;		// the elevation height array
	VarianceType *m_pVariance;	// the variance implicit binary tree
	float *m_HypoLength;		// table of hypotenuse lengths

	// indices of the corners of the height array
	int m_sw, m_nw, m_ne, m_se;

	int m_iDim;				// dimension: same as width
	int m_n;				// the power-of-2 which defines the grid
	int m_iLevels;			// triangle tree's number of levels deep
	int m_iNodes;			// triangle tree's number of nodes
	int m_iUsedNodes;		// number of nodes needed for variance tree
	int m_iSplitCutoff;
	FPoint3 m_Eyepos;		// world position of the viewpoint
	float m_fZScale;		// converts from values in height field to world scale

	float m_fQualityConstant;

	BlockPtr *m_pBlockArray;
	int m_iBlockN;
	int m_iBlockLevel;
	int m_iBlockCutoff;
	int m_iBlockPoolEnd;
	int m_iBlockArrayDim;

	// The triangle pool use to hold rendered triangles
	int		m_iTriPoolSize;
	BinTri *m_TriPool;
	int		m_NextBinTriPool;

	// the two root triangles of the binary triangle trees
	BinTri *m_pNWTri;
	BinTri *m_pSETri;
};

/*@}*/	// Group dynterr

#endif
