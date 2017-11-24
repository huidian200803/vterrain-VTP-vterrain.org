//
// SMTerrain class : An implementation a terrain rendering engine
//		based on the ideas and input of Seumas McNally.
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "SMTerrain.h"
#include "FP8.h"

#define USE_VERTEX_BUFFER	0
#define VERTEX_BUFFER_SIZE	960

#define DEFAULT_POLYGON_TARGET	10000
#define ADAPTION_SPEED		.00003f
#define QUALITYCONSTANT_MIN	0.002f

#define ASSUME_LOWEST_LEVEL	0	// Saves memory and increases strip length

#if INTEGER_HEIGHT
#define PACK_SCALE	4.0f	// use .25 meter spacing
#else
#define PACK_SCALE	1.0f
#endif

#if USE_VERTEX_BUFFER
// vertex buffer
static GLfloat g_vbuf_base[VERTEX_BUFFER_SIZE*3], *g_vbuf = g_vbuf_base;
static int verts_in_buffer = 0;
#endif

//
// Macro used to determine the index of a vertex (element of the height
// field) given it's (x,y) location in the grid
//
// This assumes a square grid size of (1 << m_n) + 1
//
#define offset(x, y)  (((y) << (m_n)) + (y) + (x))

// how to generate vertex locations from a heightfield index
#define LOCX(index) m_fXLookup[index % m_iDim]
#define LOCY(index)	m_pData[index]*m_fZScale
#define LOCZ(index) m_fZLookup[index / m_iDim]

#define MAKE_XYZ(index) LOCX(index), LOCY(index), LOCZ(index)
#define MAKE_XYZ2(x,y) m_fXLookup[x], m_pData[offset(x,y)]*m_fZScale, m_fZLookup[y]
#define MAKE_XYZ2_TRUE(x,y) m_fXLookup[x], (float)m_pData[offset(x,y)]/PACK_SCALE, m_fZLookup[y]

/*
Seumas said:
  "An implicit tree means that the structure of the tree is implied, and
  you can instantly jump to any node by using shifts and adds. An explicit
  tree has its structure explicitly defined through pointers, which means
  it's slower to use, but its structure is also dynamic at run time.

  The size of a full binary tree should be ((1 << Levels) - 1) nodes, which
  means if you're storing an implicit tree of bytes, you need that many bytes
  for the array.

  Each "level" of the tree is represented by a contiguous set of array
  elements starting at a specific index. That starting index is computed
  with levelindex = ((1 << level) - 1), where level is 0 for the root node.
  There will be (1 << level) nodes at that level, as array entries
  progressing from (and including) levelindex.

  So to find any node given its level and its position in that level, you
  can use something like:

  #define node(l, i) (((1 << (l)) - 1) + (i))

  To go to a node's left child, increment l and double i. For the right
  child, add another 1 to i.

  Now there's another way to access the tree, which is better if you're
  doing a lot of jumping around. If you index the tree from 1, rather than 0
  (as arrays normally are), it's really easy to jump around the tree. Just
  use a global node index G as the 1-based index into the array for this
  node. To go to the left child, double G. For the right child, double and
  add 1. To go back to the parent, divide by 2 (which is right shifting by 1).

  If you're using an implicit tree to store a hierarchical set of multi-
  resolution information, such as the variance levels of triangles in a
  binary triangle tree, it is very much like a a 1-dimensional mip-mapped
  texture, as you might use for 3D rendering.

  If you draw the tree on a piece of paper, it becomes pretty clear. Draw
  one box, which is the root. Then to the right draw 2 more boxes, and to
  the right of those 4 more boxes, and then 8 more. Each set of boxes is
  a level in the tree."

  "I store an Balanced Implicit Binary Tree, with one byte per node, for each
  root binary triangle on the map. Since the maps are 1024x1024 and when
  rendered are broken into 64x64 blocks, that means there are 512 root
  binary triangles (256 diamonds of 2) in the map, and thus 512 separate
  pre-computed variance trees. These trees store variance for binary
  triangles, and not vertices. They are also totally separate from the
  dynamic binary triangle trees which are created for the actual rendering.
  They correspond directly to the binary triangles created in the dynamic
  trees, though, so it is easy and quick to determine the variance of a
  triangle in the dynamic tree when deciding whether it should be split
  or not."

  "Since I don't work with odd-sized maps or patches, in my case [...]
  an 8x8 patch, and a binary triangle tree for one diagonal half of it
  would have a total of 7 levels to represent the patch to maximum
  tessellation, assuming the first triangle is called level 1. So in this
  contrived case my implicit variance tree would only go to level 4, since
  I just revisited the code and realized that I actually only don't bother
  with the lowest three levels in the implicit trees.

  I don't store any quadtrees, or any vertex data structures. All vertex
  coordinates come through a combination of being passed down through the
  recursive functions, and for height, sampling into the height map."
*/


//
// Construct/destruct
//
SMTerrain::SMTerrain() : vtDynTerrainGeom()
{
	m_pData = NULL;
	m_pVariance = NULL;
	m_TriPool = NULL;
	m_HypoLength = NULL;
	m_pBlockArray = NULL;
}

SMTerrain::~SMTerrain()
{
	delete m_pData;
	delete m_pVariance;
	delete m_TriPool;
	delete m_HypoLength;

	if (m_pBlockArray)
	{
		for (int i = 0; i < m_iBlockArrayDim; i++)
			delete m_pBlockArray[i];
		delete m_pBlockArray;
	}
}


//
// Initialize the terrain LOD structures
//
// Allocates the height/variance arrays
// Fills in the arrays with initial values
//
// fZScale converts from height values (meters) to world coordinates
//
DTErr SMTerrain::Init(const vtElevationGrid *pGrid, float fZScale)
{
	DTErr err = BasicInit(pGrid);
	if (err != DTErr_OK)
		return err;

	if (m_iSize.x != m_iSize.y)
		return DTErr_NOTSQUARE;

	// get size of array
	m_iDim = m_iSize.x;

	// compute n (log2 of grid size)
	m_n = vt_log2(m_iDim - 1);

	// ensure that the grid is size (1 << n) + 1
	int required_size = (1<<m_n) + 1;
	if (m_iSize.x != required_size || m_iSize.y != required_size)
		return DTErr_NOTPOWER2;

	// the triangle bintree will have (2n + 2) levels
	// these levels are numbered with 1-based numbering, (1 2 3...)
	m_iLevels = 2 * m_n + 2;

	// Default: create 8x8 blocks (eg. 1024 -> 128*128 (actually, 129))
	m_iBlockN = m_n - 3;	// 8x8 blocks
	// safety check
	if (m_iBlockN < 0) m_iBlockN = 0;

	m_iBlockLevel = 2 * (m_n - m_iBlockN) + 1;
	m_iBlockCutoff = 1 << (2 * (m_n - m_iBlockN));
	m_iBlockArrayDim = 1 << (m_n - m_iBlockN);

	// Allocate a 2D array of blocks
	int i, j;
	m_pBlockArray = new BlockPtr[m_iBlockArrayDim];
	for (i = 0; i < m_iBlockArrayDim; i++)
		m_pBlockArray[i] = new Block[m_iBlockArrayDim];

	// the triangle bintree will have ((1 << levels) - 1) nodes
	m_iNodes = (1 << m_iLevels) - 1;

	// the bottom level of the bintree cannot be split
	m_iSplitCutoff = (1 << (m_iLevels-2));

#if !ASSUME_LOWEST_LEVEL
	// however, we don't need to store variance for the bottom-most nodes
	// of the tree, so use one less level, and add 1 for 1-based numbering
	m_iUsedNodes = (1 << (m_iLevels-1));

#else
// APPROXIMATION
	// Seumas says he gets away with levels-4 !! Let's try a less
	// radical approximation: levels-2
	m_iUsedNodes = (1 << (m_iLevels-2));
#endif

	// allocate arrays
	m_pData = new HeightType[m_iSize.x * m_iSize.y];

	// this is potentially a big chunk of memory, so it may fail
	if (!m_pData)
		return DTErr_NOMEM;

	// copy data from supplied elevation grid
	float elev;
	for (i = 0; i < m_iSize.x; i++)
	{
		for (j = 0; j < m_iSize.y; j++)
		{
			elev = pGrid->GetFValue(i, j);
			m_pData[offset(i,j)] = (HeightType)(PACK_SCALE * elev);
		}
	}
	m_fZScale = fZScale / PACK_SCALE;

	// find indices of corner vertices
	m_sw = offset(0, 0);
	m_nw = offset(0, m_iSize.y-1);
	m_ne = offset(m_iSize.x-1, m_iSize.y-1);
	m_se = offset(m_iSize.x-1, 0);

	m_iPolygonTarget = DEFAULT_POLYGON_TARGET;
	m_fQualityConstant= 0.1f;		// safe initial value
	m_iDrawnTriangles = -1;

	return DTErr_OK;
}


void SMTerrain::Init2()
{
	// allocate and compute variance array
	m_pVariance = new VarianceType[m_iUsedNodes];
	ComputeVariances();

	m_TriPool = NULL;
	AllocatePool();

	m_HypoLength = new float[m_iLevels];
	m_HypoLength[2] = m_fDiagonalLength;
	for (int i = 3; i < m_iLevels; i++)
	{
		m_HypoLength[i] = m_HypoLength[i-1] / sqrtf(2.0f);
	}
}


void SMTerrain::AllocatePool()
{
	delete m_TriPool;
	m_iTriPoolSize = m_iPolygonTarget * 3;
	m_TriPool = new BinTri[m_iTriPoolSize];
}


//
// return memory required, in K, for a given grid size
//
int SMTerrain::MemoryRequired(int iDimension)
{
	int k = 0;

	int n = vt_log2(iDimension - 1);
	int iLevels = 2 * n + 2;

#if ASSUME_LOWEST_LEVEL
	int iUsedNodes = (1 << (iLevels-2));
#else
	int iUsedNodes = (1 << (iLevels-1));
#endif

	k += (iDimension * iDimension * sizeof(HeightType)) / 1024;	// heightfield
	k += (iUsedNodes * sizeof(VarianceType)) / 1024;			// variance
	k += (DEFAULT_POLYGON_TARGET * 3 * sizeof(BinTri)) / 1024;

	return k;
}


/*
  Seumas says:

 "My method is to recursively descend the tree, at each stage noting the
  difference between the triangle's interpolated center of hypotenuse height
  and the sampled height at the center of the hypotenuse.
  The maximum of this value and the values returned by the left and right
  children is passed back up as the return value from the function, and is
  also what is actually written to the variance tree."

  "This difference roughly records how "wrong" the terrain will be if this
  particular triangle is left solid, and not split into two. The value stored
  for each triangle in the implicit tree is the maximum of the value for that
  tri, and the value for all its children. Thus a tri will never have a lower
  stored variance value than any of its children."
*/
void SMTerrain::ComputeVariances()
{
	// Using 1-based numbering for levels of the bintree, level 1 is
	// the parent quad, and level 2 is each of the 2 main triangles.

	// Using 1-based numbering for the nodes, the parent quad is 1.
	// To find any nodes left and right children, use:
	//   left child: (1 << node)
	//   right child: (1 << node) + 1

	// Level1  Level2  Level3
	// +-----+ +-----+ +-----+
	// |     | |    /| |\ 5 /|
	// |     | | 2 / | | \ / |
	// |  1  | |  /  | |4 X 6|
	// |     | | / 3 | | / \ |
	// |     | |/    | |/ 7 \|
	// +-----+ +-----+ +-----+

	ComputeTriangleVariance(2, m_sw, m_ne, m_nw, 2);
	ComputeTriangleVariance(3, m_ne, m_sw, m_se, 2);
}

int stat_total = 0, stat_256 = 0;

//
// compute variance that this triangle would cause, if not split
//
MathType SMTerrain::ComputeTriangleVariance(int num, int v0, int v1, int va, int level)
{
	// find the center vertex
	// whether the array is 0-base or 1-base, this works:
	int vc = (v0 + v1) >> 1;

	// height at the center of the triangle, if split
	MathType height0 = m_pData[vc];

	// height at the center of the triangle, if not split
	MathType height1;

	MathType variance;
#if INTEGER_HEIGHT
	height1 = ((MathType)m_pData[v0] + (MathType)m_pData[v1]) >> 1;
	variance = abs(height1 - height0);
#else
	height1 = (m_pData[v0] + m_pData[v1]) / 2.0f;
	variance = fabsf(height1 - height0);
#endif

	// if there's another level below us that can be split, descend
#if ASSUME_LOWEST_LEVEL
	if (level < m_iLevels-2)
#else
	if (level < m_iLevels-1)
#endif
	{
		MathType child_var;

		// descend to compute variance of the left child triangle
		child_var = ComputeTriangleVariance((num << 1), va, v0, vc, level+1);
		if (child_var > variance) variance = child_var;

		// descend to compute variance of the right child triangle
		child_var = ComputeTriangleVariance((num << 1) + 1, v1, va, vc, level+1);
		if (child_var > variance) variance = child_var;
	}

	// store in the variance implicit tree
#if USE_FP8
	m_pVariance[num] = (VarianceType) EncodeFP8(variance);
#else
	m_pVariance[num] = (VarianceType) variance;
#endif

	// return maximum variance
	return variance;
}


//
// Allocate memory for a new binary tree triangle
//
#if FAST_ALLOC
#define AllocateBinTri() (m_TriPool + (m_NextBinTriPool++))
#else
inline BinTri *SMTerrain::AllocateBinTri()
{
	assert(m_NextBinTriPool < m_iTriPoolSize);
	register BinTri *tri = m_TriPool + (m_NextBinTriPool++);
	SetFrust(tri, -1);
	return tri;
}
#endif


/*
	Seumas says:

	"In my engine, "resetting the state" entails setting the NextBinTriPool
	index variable to zero, and deleting the handful of heap allocated "patch"
	structures which contain worldspace coordinates for their location, and
	pointers to the root nodes of two pool-allocated binary triangle trees.
	Which basically means that resetting state takes an insignificant amount
	of time."

	"I allocate "patch" structures dynamically since with the way the map
	wraps, you could theoretically have a large enough view distance to be
	able to see the entire map 2 or 3 times off in the distance, and
	obviously you wouldn't want those wrapped areas to be tessellated the
	very same. Thus it wouldn't work to pre-allocate patch structures only
	covering the physical extents of the height field.

	Oh, and I don't store vertex coordinates or indices in my binary triangle
	structures, just child and neighbor pointers and some goop for geomorphs.
	Coordinates are passed and calculated on the stack in the recursive
	functions, with heights pulled out of the height field when needed."

	"Currently I am passing down coordinates during my split step, though
	without height values, so only 2 numbers (and integers) per vertex of
	the triangle.
	My recursive functions look something like this:

	int BinTri::F(int x1, int y1, int x2, int y2, int x3, int y3)
	{
		int xc = (x1 + x2) >>1;
		int yc = (y1 + y2) >>1;

		LeftChild->F(x2, y2, x3, y3, xc, yc);
		RightChild->F(x3, y3, x1, y1, xc, yc);
	}
*/


void SMTerrain::SetupBlocks()
{
	// reset the state, empty the pool
	m_NextBinTriPool = 0;

	// make two fresh triangles for the top of the tree
	m_pNWTri = AllocateBinTri();
	m_pSETri = AllocateBinTri();

	// fill out the structures
	m_pNWTri->LeftChild = NULL;
	m_pNWTri->RightChild = NULL;
	m_pNWTri->LeftNeighbor = NULL;
	m_pNWTri->RightNeighbor = NULL;
	m_pNWTri->BottomNeighbor = m_pSETri;

	m_pSETri->LeftChild = NULL;
	m_pSETri->RightChild = NULL;
	m_pSETri->LeftNeighbor = NULL;
	m_pSETri->RightNeighbor = NULL;
	m_pSETri->BottomNeighbor = m_pNWTri;

	// reset the block root node pointers
	int a, b;
	for (a = 0; a < m_iBlockArrayDim; a++)
		for (b = 0; b < m_iBlockArrayDim; b++)
		{
			m_pBlockArray[a][b].root[0] = NULL;
			m_pBlockArray[a][b].root[1] = NULL;
		}

	// split down to the level of the blocks' root nodes
	SplitToBlocks(2, m_pNWTri, m_sw, m_ne, m_nw);
	SplitToBlocks(3, m_pSETri, m_ne, m_sw, m_se);
}


void SMTerrain::SplitToBlocks(int num, BinTri *tri, int v0, int v1, int va)
{
	int vc = (v0 + v1) >> 1;
	if (num >= (m_iBlockCutoff << 1))
	{
		int x = vc % m_iDim;
		int y = vc / m_iDim;
		int a = x >> m_iBlockN;
		int b = y >> m_iBlockN;

		// terminal node: root of a block
		// which block?
		int empty = (m_pBlockArray[a][b].root[0] != NULL) ? 1 : 0;
		m_pBlockArray[a][b].root[empty] = tri;
		m_pBlockArray[a][b].num[empty] = num;
		m_pBlockArray[a][b].v0[empty] = v0;
		m_pBlockArray[a][b].v1[empty] = v1;
		m_pBlockArray[a][b].va[empty] = va;
		return;
	}

	if (!tri->LeftChild)
		Split(tri);

	if (num < (m_iBlockCutoff << 1))
	{
		SplitToBlocks((num << 1), tri->LeftChild, va, v0, vc);
		SplitToBlocks((num << 1)+1, tri->RightChild, v1, va, vc);
	}
}

/*
Seumas says:
	"The difficult part is correlating a certain variance threshold with
	a certain triangle count, since the variance threshold needed to achieve
	a certain tri count will change with the structure of the terrain that
	is visible in the current frustum.  That's where the "Black Magic" comes
	in as one smoothly scales the variance threshold to bring the resultant
	triangle count in line with the	desired triangle count."
*/
void SMTerrain::AdjustQualityConstant()
{
	// do we need to expand our triangle pool?
	if (m_iPolygonTarget * 3 > m_iTriPoolSize)
		AllocatePool();

	// The number of split triangles in the triangle pool will be roughly
	// 2x the number of drawn polygons, and provides a more robust metric
	// than using the polygon count itself.
	int diff = (m_NextBinTriPool/2) - m_iPolygonTarget;

	// adjust if necessary
	int iRange = m_iPolygonTarget / 20;
	float adjust = 1.0f;
	if (diff < -iRange) adjust = 1.0f + ((diff+iRange) * ADAPTION_SPEED);
	if (diff > iRange) adjust = 1.0f + ((diff-iRange) * ADAPTION_SPEED);

	m_fQualityConstant *= adjust;
	if (m_fQualityConstant < QUALITYCONSTANT_MIN)
		m_fQualityConstant = QUALITYCONSTANT_MIN;
}


void SMTerrain::DoCulling(const vtCamera *pCam)
{
	m_Eyepos = pCam->GetTrans();

	if (m_iDrawnTriangles != -1)
		AdjustQualityConstant();

	// reset state
	SetupBlocks();
#if 1
	// Do splitting on the whole tree
	SplitIfNeeded(2, m_pNWTri, m_sw, m_ne, m_nw, false, 2);
	SplitIfNeeded(3, m_pSETri, m_ne, m_sw, m_se, false, 2);
#else
	// Do splitting on each block individually.  For some reason,
	// this is much less robust.  The feedback loop of the QualityConstant
	// does not operate in a stable fashion unless the terrain is split
	// in a well-distributed fashion, and going by block is apparently
	// too linear and fragile.
	int a, b;
	for (a = 0; a < m_iBlockArrayDim; a++)
		for (b = 0; b < m_iBlockArrayDim; b++)
		{
			// split them as needed
			BlockPtr block = m_pBlockArray[a]+b;
			SplitIfNeeded(block->num[0], block->root[0],
				block->v0[0], block->v1[0], block->va[0], false);
			SplitIfNeeded(block->num[1], block->root[1],
				block->v0[1], block->v1[1], block->va[1], false);
		}
#endif
}


inline float SMTerrain::DistanceToTriangle(int num)
{
	float diffx = m_Eyepos.x - LOCX(num);
	float diffy = m_Eyepos.y - LOCY(num);
	float diffz = m_Eyepos.z - LOCZ(num);
	float TriZDist2 = (diffx * diffx) + (diffy * diffy) + (diffz * diffz);

	// However, it isn't that expensive to do a sqrt
	return (float)sqrt(TriZDist2);
}


/*
  Seumas says:

  "First, I believe the following to be true:  With all normal projective
  frustum transforms, for a given camera space point P, when the Z distance
  doubles, the screen space X and Y projected coordinates halve, and likewise
  when the Z halves, the screen space coordinates double.  This relative
  relationship between Z and projected coordinates should be true regardless
  of the field of view angles of the frustum, as long as the frustum has
  the same aspect ratio as the display screen (an equal sided box in 3D is
  a square in screen pixels).

  Noting the above, my split test is basically as follows:

  TriZDist = the camera space Z distance of the center of the triangle's
  hypotenuse (I pick this rather than the tri center since the hypotenuse
  center must be computed to find the coordinates of the children, if
  passing triangle vertices on the stack in a recursive function).

  TriVariance = the pre-computed variance (what ROAM calls "wedgie thickness",
  but I have another cheap and fast way of approximating this as well) for
  the binary triangle in question.

  QualityConstant = a magic number derived each frame through witchcraft
  and the real-time ritual sacrifice of three chickens per frame which
  controls to what general quality level the terrain is refined.

  The per-triangle test then becomes:

  if (TriVariance > TriZDist * QualityConstant) SplitTri();

  Without a divide or square root (or even many multiplies) in sight.
  I admit though that this simplification lends itself primarily to
  horizontal views of terrain, but then that is usually the preferred and/or
  primary camera orientation used when viewing terrain interactively.  As an
  aside, I've recently added the ability to freely look up and down by as
  much as 45 degrees each way to Tread Marks, and haven't seen the need to
  switch to a more complicated test.

  The way this works is that instead of dividing the world space variance
  value (or wedgie thickness) by the triangle's Z, we multiply the acceptable
  variance constant (QualityConstant above) by the triangle's Z.  Due to the
  relationship between camera Z and projected screen space coordinates, I
  submit that this produces the same end result, but without any costly
  operations such as divides or square roots."
*/
void SMTerrain::SplitIfNeeded(int num, BinTri *tri,
							  int v0, int v1, int va,
							  bool bEntirelyInFrustum, int level)
{
/*
	Cull against view volume.
	Seumas says: "For each bin tri I first use a recursive function which tests
	whether the tri is in the frustum.  Totally out tris are dropped, totally
	in tris switch over to a culling-free recursive function, and partway tris
	continue with the frustum testing function."
*/
	int vc = (v0 + v1) >> 1;

	if (!bEntirelyInFrustum)
	{
		const FPoint3 p1(MAKE_XYZ(vc));
		int ret = IsVisible(p1, m_HypoLength[level]/2.0f);
		if (ret == VT_AllVisible)
		{
			bEntirelyInFrustum = true;
			SetFrust(tri, FRUST_ALLIN);
#if STORE_FRUSTUM
			if (tri->LeftChild) SetFrust(tri->LeftChild, FRUST_ALLIN);
			if (tri->RightChild) SetFrust(tri->RightChild, FRUST_ALLIN);
#endif
		}
		else if (ret == VT_Visible)
		{
			// possibly visible: bEntirelyInFrustum stays false
			SetFrust(tri, FRUST_PARTIN);
		}
		else if (ret == 0)
		{
			// entirely out of frustum
			SetFrust(tri, FRUST_OUT);
#if STORE_FRUSTUM
			if (tri->LeftChild) SetFrust(tri->LeftChild, FRUST_OUT);
			if (tri->RightChild) SetFrust(tri->RightChild, FRUST_OUT);
#endif
			return;
		}
	}
	else
	{
		SetFrust(tri, FRUST_ALLIN);
	}

	// if unsplit and variance is too high, split
	if (!tri->LeftChild &&
		m_NextBinTriPool < m_iTriPoolSize-50)	// safety check!  don't overflow
	{
#if ASSUME_LOWEST_LEVEL
		// one level above bottom, no stored variance, so always split
		if (num >= m_iUsedNodes)
		{
			if (m_NextBinTriPool < m_iTriPoolSize-50)	// safety check!  don't overflow
				Split(tri);
			return;
		}
#endif
		// do the correct split test
		float distance = DistanceToTriangle(vc);

#if USE_FP8
		float variance = DecodeFP8(m_pVariance[num]);
#else
		float variance = m_pVariance[num];
#endif
		if (variance / m_fQualityConstant > distance - m_HypoLength[level])
		{
			Split(tri);
		}
	}

	// if now split, descend the tree
	if (tri->LeftChild &&
		num < m_iSplitCutoff)
	{
		SplitIfNeeded((num << 1), tri->LeftChild, va, v0, vc, bEntirelyInFrustum, level+1);
		SplitIfNeeded((num << 1)+1, tri->RightChild, v1, va, vc, bEntirelyInFrustum, level+1);
	}
}


//
// The following triangle-splitting code is straight out of Seumas's
// description.
//
void SMTerrain::Split(BinTri *tri)
{
	if (tri->BottomNeighbor != NULL)
	{
		if (tri->BottomNeighbor->BottomNeighbor != tri)
		{
			Split(tri->BottomNeighbor);
		}
		Split2(tri);
		Split2(tri->BottomNeighbor);
		tri->LeftChild->RightNeighbor = tri->BottomNeighbor->RightChild;
		tri->RightChild->LeftNeighbor = tri->BottomNeighbor->LeftChild;
		tri->BottomNeighbor->LeftChild->RightNeighbor = tri->RightChild;
		tri->BottomNeighbor->RightChild->LeftNeighbor = tri->LeftChild;
	}
	else
	{
		Split2(tri);
		tri->LeftChild->RightNeighbor = NULL;
		tri->RightChild->LeftNeighbor = NULL;
	}
}

void SMTerrain::Split2(BinTri *tri)
{
	tri->LeftChild = AllocateBinTri();
	tri->RightChild = AllocateBinTri();
	tri->LeftChild->LeftNeighbor = tri->RightChild;
	tri->RightChild->RightNeighbor = tri->LeftChild;
	tri->LeftChild->BottomNeighbor = tri->LeftNeighbor;

	// Simplified implementation by Andreas Ogren (omits 2 ifs)
	if (tri->LeftNeighbor != NULL)
	{
		if (tri->LeftNeighbor->BottomNeighbor == tri)
			tri->LeftNeighbor->BottomNeighbor = tri->LeftChild;
		else
			tri->LeftNeighbor->RightNeighbor = tri->LeftChild;
	}
	tri->RightChild->BottomNeighbor = tri->RightNeighbor;
	if (tri->RightNeighbor != NULL)
	{
		if (tri->RightNeighbor->BottomNeighbor == tri)
			tri->RightNeighbor->BottomNeighbor = tri->RightChild;
		else
			tri->RightNeighbor->LeftNeighbor = tri->RightChild;
	}

	tri->LeftChild->LeftChild = NULL;
	tri->LeftChild->RightChild = NULL;
	tri->RightChild->LeftChild = NULL;
	tri->RightChild->RightChild = NULL;

#if STORE_FRUSTUM
	if (tri->m_frust != VT_Visible)
	{
		SetFrust(tri->LeftChild, tri->m_frust);
		SetFrust(tri->RightChild, tri->m_frust);
	}
#endif
}


void SMTerrain::DoRender()
{
	// Prepare the render state for our OpenGL usage
	PreRender();

	// Render the triangles
	RenderSurface();

	// Clean up
	PostRender();
}


/*
	Seumas says:

	"Once the splitting is done, rendering is accomplished by recursively
	diving the trees again, passing down vertex coordinates on the stack,
	and batching up triangle fans when the leaves are processed."

	"I'm fanning using a small fan-buffer, which I start filling with
	triangles as the first leaf node is encountered during rendering,
	and continue filling it until the current leaf can't be made part
	of the fan, at which point the fan buffer is flushed out to OpenGL
	and the current leaf triangle becomes the start of the buffer again.
	By alternating child descent orders (left first then right, and at
	the next level down, right first then left, alternating at each level
	of the tree) I get triangles in an order that's very conducive to
	fanning."
	"I usually get between 3 and 4 triangles per fan."

	"The trick to fans is swapping left/right vs right/left child descent
	order at each level down in your recursive function, then you visit
	leaf nodes in almost perfect fan order."
*/
void SMTerrain::flush_buffer(int type)
{
#if USE_VERTEX_BUFFER
	glDrawArrays(type, 0, verts_in_buffer);

	g_vbuf = g_vbuf_base;
	verts_in_buffer = 0;
#endif
}

#if USE_VERTEX_BUFFER	// put each vertex into a vertex buffer
	#define send_vertex(index) \
/*		*g_vbuf++ = LOCX(index); \
		*g_vbuf++ = LOCY(index); \
		*g_vbuf++ = LOCZ(index); \
*/		*g_vbuf++ = (float)(index%m_iDim); \
		*g_vbuf++ = m_pData[index]; \
		*g_vbuf++ = (float)(index / m_iDim); \
		verts_in_buffer++;
	#define Begin(x)
	#define End()
#else		// send each vertex individually
	#define send_vertex(index) glVertex3f((float)(index%m_iDim), m_pData[index], (float)(index / m_iDim))
	#define Begin(x) glBegin(x)
	#define End() glEnd()
#endif

int fan_start;
int fan_last = 0;
int fan_count;

void SMTerrain::render_triangle_as_fan(BinTri *pTri, int v0, int v1, int va,
									   bool even, bool right)
{
	// if there are children, descend
	if (pTri->LeftChild)
	{
		int vc = (v0 + v1) >> 1;
		// at each level of the bintree, alternate left vs. right-traversal
		if (even)
		{
			render_triangle_as_fan(pTri->RightChild, v1, va, vc, !even, true);
			render_triangle_as_fan(pTri->LeftChild, va, v0, vc, !even, false);
		}
		else
		{
			render_triangle_as_fan(pTri->LeftChild, va, v0, vc, !even, false);
			render_triangle_as_fan(pTri->RightChild, v1, va, vc, !even, true);
		}
		return;
	}
	// at this point, we know this is a leaf node
	// can it be added to the fan?
	if (va == fan_start && v0 == fan_last)
	{
		// yes!
		send_vertex(v1);
		m_iDrawnTriangles++;
		fan_last = v1;
		return;
	}
	// can we be added to the fan? another way
	if (v0 == fan_start && v1 == fan_last)
	{
		// yes!
		send_vertex(va);
		m_iDrawnTriangles++;
		fan_last = va;
		return;
	}
	// can we be added to the fan? another way
	if (v1 == fan_start && va == fan_last)
	{
		// yes!
		send_vertex(v0);
		m_iDrawnTriangles++;
		fan_last = v0;
		return;
	}
	// if there was a previous fan
	if (fan_start != -1)
	{
		// finish previous fan
		End();
		fan_count++;
	}

	// start a fan
	Begin(GL_TRIANGLE_FAN);
	if (right)
	{
		send_vertex(va);
		send_vertex(v0);
		send_vertex(v1);
		fan_start = va;
		fan_last = v1;
	}
	else
	{
		send_vertex(v0);
		send_vertex(v1);
		send_vertex(va);
		fan_start = v0;
		fan_last = va;
	}
	m_iDrawnTriangles++;
}

//
// non-strip logic
//
void SMTerrain::render_triangle_single(BinTri *pTri, int v0, int v1, int va)
{
	// if there are children, descend
	if (pTri->LeftChild)
	{
		int vc = (v0 + v1) >> 1;
		render_triangle_single(pTri->RightChild, v1, va, vc);
		render_triangle_single(pTri->LeftChild, va, v0, vc);
		return;
	}

	// at this point, we know this is a leaf node
#if STORE_FRUSTUM
	switch (pTri->m_frust)
	{
	case FRUST_OUT:		glColor3f(0.0f, 0.0f, 0.0f); break;
	case FRUST_PARTIN:	glColor3f(1.0f, 0.0f, 0.0f); break;
	case FRUST_ALLIN:	glColor3f(1.0f, 1.0f, 1.0f); break;
	default:			glColor3f(0.0f, 1.0f, 0.0f); break;
	}
#endif
	Begin(GL_TRIANGLES);

	send_vertex(v0);
	send_vertex(v1);
	send_vertex(va);

	End();
#if USE_VERTEX_BUFFER
	if (verts_in_buffer == VERTEX_BUFFER_SIZE)
		flush_buffer(GL_TRIANGLES);
#endif
	m_iDrawnTriangles++;
}


void SMTerrain::RenderBlock(BlockPtr block)
{
#if 1
	fan_start = -1;
	render_triangle_as_fan(block->root[0],	block->v0[0],
		block->v1[0], block->va[0], true, false);
	render_triangle_as_fan(block->root[1],	block->v0[1],
		block->v1[1], block->va[1], true, false);
	End();
	fan_count++;

#else // The non-triangle-fan way of doing things.
#if USE_VERTEX_BUFFER
	glVertexPointer(3, GL_FLOAT, 0, g_vbuf_base);
#endif
	render_triangle_single(block->root[0],	block->v0[0],
		block->v1[0], block->va[0]);
	render_triangle_single(block->root[1],	block->v0[1],
		block->v1[1], block->va[1]);
#if USE_VERTEX_BUFFER
	if (verts_in_buffer)
		flush_buffer(GL_TRIANGLES);
#endif
#endif
}

bool SMTerrain::BlockIsVisible(BlockPtr block)
{
	// just test one of the triangles, using the point-radius test
	int vc = (block->v0[0] + block->v1[0]) >> 1;
	FPoint3 p1(MAKE_XYZ(vc));
	int ret = IsVisible(p1, m_HypoLength[m_iBlockLevel]/2.0f);

	return (ret > 0);
}

void SMTerrain::LoadSingleMaterial()
{
	// single texture for the whole terrain
	vtMaterial *pMat = GetMaterial(0);
	if (pMat)
	{
		ApplyMaterial(pMat);
		SetupTexGen(1.0f);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
}

void SMTerrain::RenderSurface()
{
	fan_count = 0;	// for statistics

	glPushMatrix();
	glScalef(1, 1/PACK_SCALE, 1);

#if 0
	// If we had no blocks, it would be this simple:
	// Start with the two top triangles
	render_triangle(m_pNWTri, m_sw, m_ne, m_nw, true, false);
	render_triangle(m_pSETri, m_ne, m_sw, m_se, true, false);
#endif
	// render each block
	int a, b;
	BlockPtr block;
	for (a = 0; a < m_iBlockArrayDim; a++)
	{
		for (b = 0; b < m_iBlockArrayDim; b++)
		{
			block = m_pBlockArray[a]+b;

			// do block-level culling
			if (!BlockIsVisible(block)) continue;

			LoadSingleMaterial();

			RenderBlock(block);
		}
	}
	DisableTexGen();
	glPopMatrix();
}


float SMTerrain::GetElevation(int iX, int iZ, bool bTrue) const
{
	if (bTrue)
		return ((float) m_pData[offset(iX,iZ)]) / PACK_SCALE;
	else
		return m_pData[offset(iX,iZ)]*m_fZScale;
}

void SMTerrain::GetWorldLocation(int iX, int iZ, FPoint3 &p, bool bTrue) const
{
	if (bTrue)
		p.Set(MAKE_XYZ2_TRUE(iX, iZ));
	else
		p.Set(MAKE_XYZ2(iX, iZ));
}

void SMTerrain::SetVerticalExag(float fExag)
{
	m_fZScale = fExag / PACK_SCALE;
}

float SMTerrain::GetVerticalExag() const
{
	return m_fZScale * PACK_SCALE;
}

