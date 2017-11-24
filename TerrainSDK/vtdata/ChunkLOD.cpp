//
// ChunkLOD.cpp
//
// Function which takes heightfield input and generate a chunked-lod
// structure as output.  Uses quadtree decomposition, with a
// Lindstrom-Koller-ROAM-esque decimation algorithm used to decimate
// the individual chunks.
//
// Based on public-domain code by Thatcher Ulrich <tu@tulrich.com> 2001
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

#include "ChunkLOD.h"
#include "ChunkUtil.h"

#include "vtdata/ElevationGrid.h"

struct chunkstats g_chunkstats;
static bool (*s_progress_callback) (int) = NULL;

// Returns the bit position of the lowest 1 bit in the given value.
// If x == 0, returns the number of bits in an integer.
//
// E.g. lowest_one(1) == 0; lowest_one(16) == 4; lowest_one(5) == 0;
static int	lowest_one(int x)
{
	int	intbits = sizeof(x) * 8;
	int	i;
	for (i = 0; i < intbits; i++, x = x >> 1) {
		if (x & 1) break;
	}
	return i;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

struct heightfield {
	int	m_size;
	int	m_log_size;	// size == (1 << log_size) + 1
	int	root_level;	// level of the root chunk (TODO: reverse the meaning of 'level' to be more intuitive).
	float	sample_spacing;
	float	vertical_scale;	// scales the units stored in heightfield_elem's.  meters == stored_short * vertical_scale
	float	input_vertical_scale;	// scale factor to apply to input data.
	const vtElevationGrid *m_bt;
	mmap_array<uchar>*	m_level;

	heightfield(float initial_vertical_scale, float input_scale) {
		m_size = 0;
		m_log_size = 0;
		sample_spacing = 1.0f;
		vertical_scale = initial_vertical_scale;
		input_vertical_scale = input_scale;
		m_bt = NULL;
		m_level = NULL;
	}

	~heightfield() {
		clear();
	}

	// Frees any allocated data and resets our members.
	void	clear()
	{
		if (m_level) {
			delete m_level;
			m_level = 0;
		}

		m_size = 0;
		m_log_size = 0;
	}

	// Return a (writable) reference to the height element at (x, z).
	short	height(int x, int z)
	{
		assert(m_bt);
		// Note that we flip the Z coordinate here; ChunkLOD seems to want it
		//  reversed from how elevationgrid stores it.
		return (short) (m_bt->GetFValue(x, m_size-1-z) * input_vertical_scale / vertical_scale);
	}

	// Return the activation level at (x, z)
	int	get_level(int x, int z)
	{
		assert(m_level);
		int val = m_level->get(z, x >> 1); // swap indices for VM performance -- .bt is column major
		if (x & 1) {
			val = val >> 4;
		}
		val &= 0x0F;
		if (val == 0x0F) return -1;
		else return val;
	}

	void	set_level(int x, int z, int lev)
	{
		assert(lev >= -1 && lev < 15);
		lev &= 0x0F;
		uchar	current = m_level->get(z, x >> 1); // swap indices for VM performance -- .bt is column major
		if (x & 1) {
			current = (current & 0x0F) | (lev << 4);
		} else {
			current = (current & 0xF0) | (lev);
		}
		m_level->get(z, x >> 1) = current;
	}

	// Sets the activation_level to the given level, if it's greater than
	// the vert's current activation level.
	void	activate(int x, int z, int lev)
	{
		assert(lev < 15);	// 15 is our flag value.
		int	current_level = get_level(x, z);
		if (lev > current_level) {
			if (current_level == -1) {
				g_chunkstats.output_vertices++;
			}
			set_level(x, z, lev);
		}
	}

	// Given the coordinates of the center of a quadtree node, this
	// function returns its node index.  The node index is essentially
	// the node's rank in a breadth-first quadtree traversal.  Assumes
	// a [nw, ne, sw, se] traversal order.
	//
	// If the coordinates don't specify a valid node (e.g. if the coords
	// are outside the heightfield) then returns -1.
	int	node_index(int x, int z)
	{
		if (x < 0 || x >= m_size || z < 0 || z >= m_size) {
			return -1;
		}

		int	l1 = lowest_one(x | z);
		int	depth = m_log_size - l1 - 1;

		int	base = 0x55555555 & ((1 << depth*2) - 1);	// total node count in all levels above ours.
		int	shift = l1 + 1;

		// Effective coords within this node's level.
		int	col = x >> shift;
		int	row = z >> shift;

		return base + (row << depth) + col;
	}


	// Given an x or z coordinate, along which an edge runs, this
	// function returns the lowest LOD level that the edge divides.
	//
	// (This is useful for determining how conservative we need to be
	// with edge skirts.)
	int	minimum_edge_lod(int coord)
	{
		int	l1 = lowest_one(coord);
		int	depth = (m_log_size - l1 - 1);

		return iclamp(root_level - depth, 0, root_level);	// TODO: reverse direction of level

//		depth = iclamp(depth, 0, root_level);
//		if (depth < 0) depth = 0;
//
//		return depth;
	}


	// Use the specified .BT format heightfield data file as our height input.
	//
	// Return true on success, false on failure.
	bool init_bt(const vtElevationGrid *grid)
	{
		clear();

		m_bt = grid;

		int btwidth, btheight;
		m_bt->GetDimensions(btwidth, btheight);
		m_size = std::max(btwidth, btheight);

		// Compute the log_size and make sure the size is 2^N + 1
		m_log_size = (int) (vt_log2f((float) m_size - 1) + 0.5);
		if (m_size != (1 << m_log_size) + 1) {
			if (m_size < 0 || m_size > (1 << 20)) {
				printf("invalid heightfield dimensions -- size from file = %d\n", m_size);
				return false;
			}

			printf("Warning: data is not (2^N + 1) x (2^N + 1); will extend to make it the correct size.\n");

			// Expand log_size until it contains size.
			while (m_size > (1 << m_log_size) + 1) {
				m_log_size++;
			}
			m_size = (1 << m_log_size) + 1;
		}

		DPoint2 spacing = m_bt->GetSpacing();
		sample_spacing = (float) spacing.x;
		printf("sample_spacing = %f\n", sample_spacing);

		// Allocate storage for vertex activation levels.
		m_level = new mmap_array<uchar>(m_size, (m_size + 1) >> 1, true);	// swap height/width; .bt is column-major
		assert(m_level);

		// Initialize level array.
		for (int i = 0; i < m_size; i++) {
			for (int j = 0; j < m_size; j++) {
				m_level->get(m_size - 1 - j, i >> 1) = 255;	// swap height/width -- (to match .bt order; good idea??)
			}
		}
		return true;
	}
};
#endif	// DOXYGEN_SHOULD_SKIP_THIS

void	update(heightfield& hf, float base_max_error, int ax, int az, int rx, int rz, int lx, int lz);
void	propagate_activation_level(heightfield& hf, int cx, int cz, int level, int target_level);
int		check_propagation(heightfield& hf, int cx, int cz, int level);
void	generate_empty_TOC(FILE* rw, int level);
bool	generate_node_data(FILE* rw, heightfield& hf, int x0, int z0, int log_size, int level);

const char*	spinner = "-\\|/";


/** Generate LOD chunks from the given heightfield.
 *
 * tree_depth determines the depth of the chunk quadtree.
 *
 * base_max_error specifies the maximum allowed geometric vertex error,
 * at the finest level of detail.
 *
 * Spacing determines the horizontal sample spacing for bitmap
 * heightfields only.
 */
bool HeightfieldChunker::ProcessGrid(const vtElevationGrid *grid, FILE *out,
	int tree_depth, float base_max_error, float vertical_scale,
	float input_vertical_scale, bool progress_callback(int))
{
	heightfield	hf(vertical_scale, input_vertical_scale);

	if (!hf.init_bt(grid))
	{
		printf("Failed to initialize heightfield.\n");
		return false;
	}

	hf.root_level = tree_depth - 1;

	g_chunkstats.clear();
	g_chunkstats.input_vertices = hf.m_size * hf.m_size;
	g_chunkstats.total_chunks = quadtree_node_count(tree_depth);
	s_progress_callback = progress_callback;

	printf("updating...");

	// Run a view-independent L-K style BTT update on the heightfield, to generate
	// error and activation_level values for each element.
	update(hf, base_max_error, 0, hf.m_size - 1, hf.m_size - 1, hf.m_size - 1, 0, 0);	// sw half of the square
	update(hf, base_max_error, hf.m_size - 1, 0, 0, 0, hf.m_size - 1, hf.m_size - 1);	// ne half of the square

	printf("done.\n");

	printf("propagating....");

	// Propagate the activation_level values of verts to their
	// parent verts, quadtree LOD style.  Gives same result as L-K.
	for (int i = 0; i < hf.m_log_size; i++) {
		propagate_activation_level(hf, hf.m_size >> 1, hf.m_size >> 1, hf.m_log_size - 1, i);
		propagate_activation_level(hf, hf.m_size >> 1, hf.m_size >> 1, hf.m_log_size - 1, i);
//		printf("\b%c", spinner[(spin_count++)&3]);
	}

//	check_propagation(hf, hf.size >> 1, hf.size >> 1, hf.log_size - 1);//xxxxx

	printf("done\n");

	// Write a .chu header for the output file.
	WriteUint32(out, ('C') | ('H' << 8) | ('U' << 16));	// four byte "CHU\0" tag
	WriteUint16(out, 9);	// file format version.
	WriteUint16(out, tree_depth);	// depth of the chunk quadtree.
	WriteFloat32(out, base_max_error);	// max geometric error at base level mesh.
	WriteFloat32(out, vertical_scale);	// meters / unit of vertical measurement.
	WriteFloat32(out, (1 << (hf.m_log_size - (tree_depth - 1))) * hf.sample_spacing);	// x/z dimension, in meters, of highest LOD chunks.
	WriteUint32(out, 0x55555555 & ((1 << (tree_depth*2)) - 1));	// Chunk count.  Fully populated quadtree.

	printf("meshing....");

	// Make space for our chunk table-of-contents.  Fixed-size
	// chunk headers will go in this space; the vertex/index data
	// for chunks gets appended to the end of the stream.
	generate_empty_TOC(out, hf.root_level);

	// Write out the node data for the entire chunk tree.
	if (!generate_node_data(out, hf, 0, 0, hf.m_log_size, hf.root_level))
		return false;

	printf("done\n");

	return true;
}


// Given the triangle, computes an error value and activation level
// for its base vertex, and recurses to child triangles.
void	update(heightfield& hf, float base_max_error, int ax, int az, int rx, int rz, int lx, int lz)
{
	// Compute the coordinates of this triangle's base vertex.
	int	dx = lx - rx;
	int	dz = lz - rz;
	if (iabs(dx) <= 1 && iabs(dz) <= 1) {
		// We've reached the base level.  There's no base vertex
		// to update, and no child triangles to recurse to.
		return;
	}

	// base vert is midway between left and right verts.
	int	bx = rx + (dx >> 1);
	int	bz = rz + (dz >> 1);

	float	error = fabsf((hf.height(bx, bz) - (hf.height(lx, lz) + hf.height(rx, rz)) / 2.f) * hf.vertical_scale);
	assert(error >= 0);
	if (error >= base_max_error) {
		// Compute the mesh level above which this vertex
		// needs to be included in LOD meshes.
		int	activation_level = (int) floor(vt_log2f(error / base_max_error) + 0.5f);

		// Force the base vert to at least this activation level.
		hf.activate(bx, bz, activation_level);
	}

	// Recurse to child triangles.
	update(hf, base_max_error, bx, bz, ax, az, rx, rz);	// base, apex, right
	update(hf, base_max_error, bx, bz, lx, lz, ax, az);	// base, left, apex
}


const float	SQRT_2 = sqrtf(2);


// Returns the height of the query point (x,z) within the triangle (a, r, l),
// as tesselated to the specified LOD.
//
// Return value is in heightfield discrete coords.  To get meters,
// multiply by hf.vertical_scale.
short	height_query(heightfield& hf, int level, int x, int z, int ax, int az, int rx, int rz, int lx, int lz)
{
	// If the query is on one of our verts, return that vert's height.
	if ((x == ax && z == az)
	 || (x == rx && z == rz)
	 || (x == lx && z == lz))
	{
		return hf.height(x, z);
	}

	// Compute the coordinates of this triangle's base vertex.
	int	dx = lx - rx;
	int	dz = lz - rz;
	if (iabs(dx) <= 1 && iabs(dz) <= 1) {
		// We've reached the base level.  This is an error condition; we should
		// have gotten a successful test earlier.

		// assert(0);
		printf("Error: height_query hit base of heightfield.\n");

		return hf.height(ax, az);
	}

	// base vert is midway between left and right verts.
	int	bx = rx + (dx >> 1);
	int	bz = rz + (dz >> 1);

	// compute the length of a side edge.
	float	edge_length_squared = (dx * dx + dz * dz) / 2.f;

	float	sr, sl;	// barycentric coords w/r/t the right and left edges.
	sr = ((x - ax) * (rx - ax) + (z - az) * (rz - az)) / edge_length_squared;
	sl = ((x - ax) * (lx - ax) + (z - az) * (lz - az)) / edge_length_squared;

	int	base_vert_level = hf.get_level(bx, bz);
	if (base_vert_level >= level){
		// The mesh is more tesselated at the desired LOD.  Recurse.
		if (sr >= sl) {
			// Query is in right child triangle.
			return height_query(hf, level, x, z, bx, bz, ax, az, rx, rz);	// base, apex, right
		} else {
			// Query is in left child triangle.
			return height_query(hf, level, x, z, bx, bz, lx, lz, ax, az);	// base, left, apex
		}
	}

	short	ay = hf.height(ax, az);
	short	dr = hf.height(rx, rz) - ay;
	short	dl = hf.height(lx, lz) - ay;

	// This triangle is as far as the desired LOD goes.  Compute the
	// query's height on the triangle.
	return (short) floor(ay + sl * dl + sr * dr + 0.5);
}


// Returns the height of the mesh as simplified to the specified level
// of detail.
short	get_height_at_LOD(heightfield& hf, int level, int x, int z)
{
	if (z > x) {
		// Query in SW quadrant.
		return height_query(hf, level, x, z, 0, hf.m_size - 1, hf.m_size - 1, hf.m_size - 1, 0, 0);	// sw half of the square

	} else {	// query in NW quadrant
		return height_query(hf, level, x, z, hf.m_size - 1, 0, 0, 0, hf.m_size - 1, hf.m_size - 1);	// ne half of the square
	}
}


// Does a quadtree descent through the heightfield, in the square with
// center at (cx, cz) and size of (2 ^ (level + 1) + 1).  Descends
// until level == target_level, and then propagates this square's
// child center verts to the corresponding edge vert, and the edge
// verts to the center.  Essentially the quadtree meshing update
// dependency graph as in my Gamasutra article.  Must call this with
// successively increasing target_level to get correct propagation.
void	propagate_activation_level(heightfield& hf, int cx, int cz, int level, int target_level)
{
	int	half_size = 1 << level;
	int	quarter_size = half_size >> 1;

	if (level > target_level) {
		// Recurse to children.
		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < 2; i++) {
				propagate_activation_level(hf,
							   cx - quarter_size + half_size * i,
							   cz - quarter_size + half_size * j,
							   level - 1, target_level);
			}
		}
		return;
	}

	// We're at the target level.  Do the propagation on this
	// square.

	if (level > 0) {
		// Propagate child verts to edge verts.
		int	lev = hf.get_level(cx + quarter_size, cz - quarter_size);	// ne
		hf.activate(cx + half_size, cz, lev);
		hf.activate(cx, cz - half_size, lev);

		lev = hf.get_level(cx - quarter_size, cz - quarter_size);	// nw
		hf.activate(cx, cz - half_size, lev);
		hf.activate(cx - half_size, cz, lev);

		lev = hf.get_level(cx - quarter_size, cz + quarter_size);	// sw
		hf.activate(cx - half_size, cz, lev);
		hf.activate(cx, cz + half_size, lev);

		lev = hf.get_level(cx + quarter_size, cz + quarter_size);	// se
		hf.activate(cx, cz + half_size, lev);
		hf.activate(cx + half_size, cz, lev);
	}

	// Propagate edge verts to center.
	hf.activate(cx, cz, hf.get_level(cx + half_size, cz));
	hf.activate(cx, cz, hf.get_level(cx, cz - half_size));
	hf.activate(cx, cz, hf.get_level(cx, cz + half_size));
	hf.activate(cx, cz, hf.get_level(cx - half_size, cz));
}


// Debugging function -- verifies that activation level dependencies
// are correct throughout the tree.
int	check_propagation(heightfield& hf, int cx, int cz, int level)
{
	int	half_size = 1 << level;
	int	quarter_size = half_size >> 1;

	int	max_act = -1;

	// cne = ne child, cnw = nw child, etc.
	int	cne = -1;
	int	cnw = -1;
	int	csw = -1;
	int	cse = -1;
	if (level > 0) {
		// Recurse to children.
		cne = check_propagation(hf, cx + quarter_size, cz - quarter_size, level - 1);
		cnw = check_propagation(hf, cx - quarter_size, cz - quarter_size, level - 1);
		csw = check_propagation(hf, cx - quarter_size, cz + quarter_size, level - 1);
		cse = check_propagation(hf, cx + quarter_size, cz + quarter_size, level - 1);
	}

	// ee == east edge, en = north edge, etc
	int	ee = hf.get_level(cx + half_size, cz);
	int	en = hf.get_level(cx, cz - half_size);
	int	ew = hf.get_level(cx - half_size, cz);
	int	es = hf.get_level(cx, cz + half_size);

	if (level > 0) {
		// Check child verts against edge verts.
		if (cne > ee || cse > ee) {
			printf("cp error! ee! lev = %d, cx = %d, cz = %d, alev = %d\n", level, cx, cz, ee);	//xxxxx
		}

		if (cne > en || cnw > en) {
			printf("cp error! en! lev = %d, cx = %d, cz = %d, alev = %d\n", level, cx, cz, en);	//xxxxx
		}

		if (cnw > ew || csw > ew) {
			printf("cp error! ew! lev = %d, cx = %d, cz = %d, alev = %d\n", level, cx, cz, ew);	//xxxxx
		}

		if (csw > es || cse > es) {
			printf("cp error! es! lev = %d, cx = %d, cz = %d, alev = %d\n", level, cx, cz, es);	//xxxxx
		}
	}

	// Check level of edge verts against center.
	int	c = hf.get_level(cx, cz);
	max_act = std::max(max_act, ee);
	max_act = std::max(max_act, en);
	max_act = std::max(max_act, es);
	max_act = std::max(max_act, ew);

	if (max_act > c) {
		printf("cp error! center! lev = %d, cx = %d, cz = %d, alev = %d, max_act = %d ee = %d en = %d ew = %d es = %d\n", level, cx, cz, c, max_act, ee, en, ew, es);	//xxxxx
	}

	return std::max(max_act, c);
}


namespace mesh {
	void	clear();
	void	emit_vertex(heightfield& hf, int ax, int az);	// call this in strip order.
	void	emit_previous_vertex();	// for ending a strip and starting another.
	int	get_index_count();	// helpful for determining whether we need an extra degenerate for correct winding order.

	// used for edge "skirt" vertices, which (can be) displaced below
	// the corresponding heightfield verts.
	void	emit_special_vertex(heightfield& hf, int ax, short y, int az);

	int	lookup_index(int x, int z);

	bool	write(FILE* rw, heightfield& hf, int activation_level);
};


void	generate_edge_data(FILE* out, heightfield& hf, int dir, int x0, int z0, int x1, int z1, int level);


// Manually synced!!!  (@@ should use a fixed-size struct, to be
// safer, although that ruins endian safety.)  If you change the chunk
// header contents, you must keep this constant in sync.  In DEBUG
// builds, there's an assert that should catch discrepancies, but be
// careful.
const int	CHUNK_HEADER_BYTES = 4 + 4*4 + 1 + 2 + 2 + 2*2 + 4;


// Append an empty table-of-contents for a fully-populated quadtree,
// and rewind the stream to the start of the contents.  Use this to
// make room for TOC at the beginning of the file, while bulk
// vert/index data gets appended after the TOC.
void	generate_empty_TOC(FILE* rw, int root_level)
{
	uchar	buf[CHUNK_HEADER_BYTES];	// dummy chunk header.
	memset(buf, 0, sizeof(buf));

	int	start_pos = ftell(rw);

	int	chunk_count = quadtree_node_count(root_level + 1);	// tqt has a handy function to compute # of nodes in a quadtree
	for (int i = 0; i < chunk_count; i++)
	{
		fwrite(buf, sizeof(buf), 1, rw);
	}

	// Rewind, so caller can start writing real TOC data.
	fseek(rw, start_pos, SEEK_SET);
}


struct gen_state;
void	generate_block(heightfield& hf, int level, int log_size, int cx, int cz);
void	generate_quadrant(heightfield& hf, gen_state* s, int lx, int lz, int tx, int tz, int rx, int rz, int level);

int s_nodes_generated  = 0;

// Given a square of data, with northwest corner at (x0, z0) and
// comprising ((1<<log_size)+1) verts along each axis, this function
// generates the mesh using verts which are active at the given level.
//
// If we're not at the base level (level > 0), then also recurses to
// quadtree child nodes and generates their data.
bool generate_node_data(FILE* out, heightfield& hf, int x0, int z0, int log_size, int level)
{
	int	start_pos = ftell(out);	// use this to verify the value of CHUNK_HEADER_BYTES

	g_chunkstats.output_chunks++;

	if (s_progress_callback != NULL)
		s_progress_callback(g_chunkstats.output_chunks * 100 / g_chunkstats.total_chunks);

	int	size = (1 << log_size);
	int	half_size = size >> 1;
	int	cx = x0 + half_size;
	int	cz = z0 + half_size;

	// Assign a label to this chunk, so edges can reference the chunks.
	int	chunk_label = hf.node_index(cx, cz);

	// Write our label.
	WriteUint32(out, chunk_label);

	// Write the labels of our neighbors.
	WriteUint32(out, hf.node_index(cx + size, cz));	// EAST
	WriteUint32(out, hf.node_index(cx, cz - size));	// NORTH
	WriteUint32(out, hf.node_index(cx - size, cz));	// WEST
	WriteUint32(out, hf.node_index(cx, cz + size));	// SOUTH

	// Chunk address.
	int	LOD_level = hf.root_level - level;
	assert(LOD_level >= 0 && LOD_level < 256);
	WriteByte(out, LOD_level);
	WriteUint16(out, x0 >> log_size);
	WriteUint16(out, z0 >> log_size);

	// Start making the mesh.
	mesh::clear();

	// !!! This needs to be done in propagate, or something (too late now) !!!
	// Make sure our corner verts are activated on this level.
	hf.activate(x0 + size, z0, level);
	hf.activate(x0, z0, level);
	hf.activate(x0, z0 + size, level);
	hf.activate(x0 + size, z0 + size, level);

	// Generate the mesh.
	generate_block(hf, level, log_size, x0 + half_size, z0 + half_size);

//	printf("\b%c", spinner[(spin_count++)&3]);

//	// Print some interesting info.
//	printf("chunk: (%d, %d) size = %d\n", x0, z0, size);

	// Generate data for our edge skirts.  Go counterclockwise around
	// the outside (ensures correct winding).
	generate_edge_data(out, hf, 0, cx + half_size, cz + half_size, cx + half_size, cz - half_size, level);	// east
	generate_edge_data(out, hf, 1, cx + half_size, cz - half_size, cx - half_size, cz - half_size, level);	// north
	generate_edge_data(out, hf, 2, cx - half_size, cz - half_size, cx - half_size, cz + half_size, level);	// west
	generate_edge_data(out, hf, 3, cx - half_size, cz + half_size, cx + half_size, cz + half_size, level);	// south

	// Finish writing our data.
	if (!mesh::write(out, hf, level))
		return false;

	int	header_bytes_written = ftell(out) - start_pos;
	assert(header_bytes_written == CHUNK_HEADER_BYTES);

	// recurse to child regions, to generate child chunks.
	if (level > 0)
	{
		int	half_size = (1 << (log_size-1));
		bool res;

		res = generate_node_data(out, hf, x0, z0, log_size-1, level-1);	// nw
		if (!res) return false;

		res = generate_node_data(out, hf, x0 + half_size, z0, log_size-1, level-1);	// ne
		if (!res) return false;

		res = generate_node_data(out, hf, x0, z0 + half_size, log_size-1, level-1);	// sw
		if (!res) return false;

		res = generate_node_data(out, hf, x0 + half_size, z0 + half_size, log_size-1, level-1);	// se
		if (!res) return false;
	}
	return true;
}


struct gen_state {
	int	my_buffer[2][2];	// x,z coords of the last two vertices emitted by the generate_ functions.
	int	activation_level;	// for determining whether a vertex is enabled in the block we're working on
	int	ptr;	// indexes my_buffer.
	int	previous_level;	// for keeping track of level changes during recursion.

	// Returns true if the specified vertex is in my_buffer.
	bool	in_my_buffer(int x, int z)
	{
		return ((x == my_buffer[0][0]) && (z == my_buffer[0][1]))
			|| ((x == my_buffer[1][0]) && (z == my_buffer[1][1]));
	}

	// Sets the current my_buffer entry to (x,z)
	void	set_my_buffer(int x, int z)
	{
		my_buffer[ptr][0] = x;
		my_buffer[ptr][1] = z;
	}
};


// Generate the mesh for the specified square with the given center.
// This is paraphrased directly out of Lindstrom et al, SIGGRAPH '96.
// It generates a square mesh by walking counterclockwise around four
// triangular quadrants.  The resulting mesh is composed of a single
// continuous triangle strip, with a few corners turned via degenerate
// tris where necessary.
void	generate_block(heightfield& hf, int activation_level, int log_size, int cx, int cz)
{
	// quadrant corner coordinates.
	int	hs = 1 << (log_size - 1);
	int	q[4][2] = {
		{ cx + hs, cz + hs },	// se
		{ cx + hs, cz - hs },	// ne
		{ cx - hs, cz - hs },	// nw
		{ cx - hs, cz + hs },	// sw
	};

	// Init state for generating mesh.
	gen_state	state;
	state.ptr = 0;
	state.previous_level = 0;
	state.activation_level = activation_level;
	for (int i = 0; i < 4; i++) {
		state.my_buffer[i>>1][i&1] = -1;
	}

	mesh::emit_vertex(hf, q[0][0], q[0][1]);
	state.set_my_buffer(q[0][0], q[0][1]);

	{for (int i = 0; i < 4; i++) {
		if ((state.previous_level & 1) == 0) {
			// tulrich: turn a corner?
			state.ptr ^= 1;
		} else {
			// tulrich: jump via degenerate?
			int	x = state.my_buffer[1 - state.ptr][0];
			int	z = state.my_buffer[1 - state.ptr][1];
			mesh::emit_vertex(hf, x, z);	// or, emit vertex(last - 1);
		}

		// Initial vertex of quadrant.
		mesh::emit_vertex(hf, q[i][0], q[i][1]);
		state.set_my_buffer(q[i][0], q[i][1]);
		state.previous_level = 2 * log_size + 1;

		generate_quadrant(hf,
				  &state,
				  q[i][0], q[i][1],	// q[i][l]
				  cx, cz,	// q[i][t]
				  q[(i+1)&3][0], q[(i+1)&3][1],	// q[i][r]
				  2 * log_size
			);
	}}
	if (state.in_my_buffer(q[0][0], q[0][1]) == false) {
		// finish off the strip.  @@ may not be necessary?
		mesh::emit_vertex(hf, q[0][0], q[0][1]);
	}
}


// Auxiliary function for generate_block().  Generates a mesh from a
// triangular quadrant of a square heightfield block.  Paraphrased
// directly out of Lindstrom et al, SIGGRAPH '96.
void	generate_quadrant(heightfield& hf, gen_state* s, int lx, int lz, int tx, int tz, int rx, int rz, int recursion_level)
{
	if (recursion_level <= 0) return;

	if (hf.get_level(tx, tz) >= s->activation_level) {
		// Find base vertex.
		int	bx = (lx + rx) >> 1;
		int	bz = (lz + rz) >> 1;

		generate_quadrant(hf, s, lx, lz, bx, bz, tx, tz, recursion_level - 1);	// left half of quadrant

		if (s->in_my_buffer(tx,tz) == false) {
			if ((recursion_level + s->previous_level) & 1) {
				s->ptr ^= 1;
			} else {
				int	x = s->my_buffer[1 - s->ptr][0];
				int	z = s->my_buffer[1 - s->ptr][1];
				mesh::emit_vertex(hf, x, z);	// or, emit vertex(last - 1);
			}
			mesh::emit_vertex(hf, tx, tz);
			s->set_my_buffer(tx, tz);
			s->previous_level = recursion_level;
		}

		generate_quadrant(hf, s, tx, tz, bx, bz, rx, rz, recursion_level - 1);
	}
}


// Write out the data for an edge of the chunk that was just generated.
// (x0,z0) - (x1,z1) defines the extent of the edge in the heightfield.
// level determines which vertices in the mesh are active.
//
// Generates a "skirt" mesh which ensures that this mesh always covers
// the space between our simplified edge and the full-LOD edge.
void	generate_edge_data(FILE* out, heightfield& hf, int dir, int x0, int z0, int x1, int z1, int level)
{
	// We're going to write a list of vertices comprising the
	// edge.
	//
	// We're also going to write the index (in this list) of the
	// midpoint vertex of the edge, so the renderer can join
	// t-junctions.

	//
	// Vertices.
	//

	// Scan the edge, looking for the minimum height at each vert,
	// taking into account the full LOD mesh, plus all meshes up to
	// two levels above our own.


	array<int>	vert_minimums;

	// Step along the edge.
	int	dx, dz, steps;
	if (x0 < x1) {
		assert(z0 == z1);
		dx = 1;
		dz = 0;
		steps = x1 - x0 + 1;
	} else if (x0 > x1) {
		assert(z0 == z1);
		dx = -1;
		dz = 0;
		steps = x0 - x1 + 1;
	} else if (z0 < z1) {
		assert(x0 == x1);
		dx = 0;
		dz = 1;
		steps = z1 - z0 + 1;
	} else if (z0 > z1) {
		assert(x0 == x1);
		dx = 0;
		dz = -1;
		steps = z0 - z1 + 1;
	} else {
		assert(0);	// edge must have non-zero length...
	}

	int	current_min = hf.height(x0, z0);
	for (int i = 0, x = x0, z = z0; i < steps; i++, x += dx, z += dz)
	{
		current_min = std::min(current_min, (int) hf.height(x, z));

		if (hf.get_level(x, z) >= level) {
			// This is an active vert at this level of detail.

			// TODO: activation level & chunk level are consistent
			// with each other, but they go in the unintuitive
			// direction: level 0 is the *highest* (i.e. most
			// detailed) "level of detail".  Should reverse this to be
			// less confusing.

			// Check height of lower LODs.  The rule is, the renderer
			// allows a certain level of difference in LOD between
			// neighboring chunks, so we want to ensure we can cover
			// cracks that encompass gaps between our mesh and the
			// minimum LOD of our neighbor.
			//
			// The more levels of difference allowed, the less
			// constrained the renderer can be about paging.  On the
			// other hand, the more levels of difference allowed, the
			// more conservative we have to be with our skirts.
			//
			// Also, every edge is *interior* to some minimum LOD
			// chunk.  So we don't have to look any higher than that
			// LOD.

			int	major_coord = x0;
			if (dz == 0) {
				major_coord = z0;
			}
			int	minimum_edge_lod = hf.minimum_edge_lod(major_coord);	// lod of the least-detailed chunk bordering this edge.

			const int MAXIMUM_ALLOWED_NEIGHBOR_DIFFERENCE = 2;	// A parameter in the renderer as well -- keep in sync.

			int	level_diff = std::min(minimum_edge_lod + 1, hf.root_level) - level;
			level_diff = std::min(level_diff, MAXIMUM_ALLOWED_NEIGHBOR_DIFFERENCE);

			for (int lod = level; lod <= level + level_diff; lod++)
			{
				short	lod_height = get_height_at_LOD(hf, lod, x, z);
				current_min = std::min(current_min, (int) lod_height);
			}

			// Remember the minimum height of the edge for this
			// segment.

			if (current_min > -32768) {
				current_min -= 1;	// be slightly conservative here.
			}
			vert_minimums.push_back(current_min);
			current_min = hf.height(x, z);
		}
	}

	// Generate a "skirt" which runs around our outside edges and
	// makes sure we don't leave any visible cracks between our
	// (simplified) edges and the true shape of the mesh along our
	// edges.

	mesh::emit_previous_vertex();	// end the previous strip; starting a new one.
	if ((mesh::get_index_count() & 1) == 0) {
		// even number of verts, which means current winding order
		// will be backwards, after we add the degenerate to start the
		// strip.  Emit an extra degenerate vert to restore the normal
		// winding order.
		mesh::emit_previous_vertex();
	}

	int	vert_index = 0;
	{for (int i = 0, x = x0, z = z0; i < steps; i++, x += dx, z += dz)
	{
		if (hf.get_level(x, z) >= level)
		{
			// Put down a vertex which is at the minimum of our
			// previous and next segments.
			int	min_height = vert_minimums[vert_index];
			if (vert_minimums.size() > vert_index + 1)
			{
				min_height = std::min(min_height, vert_minimums[vert_index + 1]);
			}

			mesh::emit_vertex(hf, x, z);
			if (i == 0) {
				mesh::emit_previous_vertex();	// starting a new strip.
			}
			mesh::emit_special_vertex(hf, x, min_height, z);

			vert_index++;
		}
	}}
}



namespace mesh {
// mini module for building up mesh data.

//data:
	struct vert_info {
		short	x, z;
		short	y;
		bool	special;

		// hash function, for hash<>.
		static int	compute(const vert_info& data)
		{
			return data.x + (data.y << 5) * 101 + (data.z << 10) * 101 + data.special;
		}


		vert_info() : x(-1), z(-1), y(0), special(false) {}
		vert_info(int vx, int vz) : x(vx), z(vz), y(0), special(false) {}

		// A "special" vert is not on the heightfield, but has its own Y value.
		vert_info(int vx, int vy, int vz)
			: x(vx),
			  z(vz),
			  y(vy),
			  special(true)
		{}

		bool	operator==(const vert_info& v) { return x == v.x && z == v.z; }
	};

	array<vert_info>	vertices;
	array<int>	vertex_indices;
	hash<vert_info, int, vert_info>	index_table;	// to accelerate get_vertex_index()

	array<int>	edge_strip[4];
	array<vert_info>	edge_lo[4];
	array<vert_info>	edge_hi[4][2];

	FPoint3	min, max;	// for bounding box.

	short	min_y, max_y;


//code:
	// Return the index of the specified vert.  If the vert isn't
	// in our vertex list, then add it to the end.
	int	get_vertex_index(int x, int z)
	{
		int	index = lookup_index(x, z);

		if (index != -1) {
			// We already have that vert.
			return index;
		}

		index = vertices.size();
		vert_info	v(x, z);
		vertices.push_back(v);
		index_table.add(v, index);

		return index;
	}


	// Add a "special" vertex; i.e. a vert that is not on the
	// heightfield.
	int	special_vertex_index(int x, short y, int z)
	{
		int	index = vertices.size();
		vert_info	v(x, y, z);
		vertices.push_back(v);

		return index;
	}


	// Return the index in the current vertex array of the specified
	// vertex.  If the vertex can't be found in the current array,
	// then returns -1.
	int	lookup_index(int x, int z)
	{
		int	index;
		if (index_table.get(vert_info(x, z), &index)) {
			// Found it.
			return index;
		}
		// Didn't find it.
		return -1;
	}


	// Reset and empty all our containers, to start a fresh mesh.
	void	clear()
	{
		vertices.clear();
		vertex_indices.clear();
		index_table.clear();
		index_table.resize(4096);

		for (int i = 0; i < 4; i++) {
			edge_strip[i].clear();
			edge_lo[i].clear();
			edge_hi[i][0].clear();
			edge_hi[i][1].clear();
		}

		min = FPoint3(1000000, 1000000, 1000000);
		max = FPoint3(-1000000, -1000000, -1000000);

		min_y = (short) 0x7FFF;
		max_y = (short) 0x8000;
	}

	// Utility function, to output the quantized data for a vertex.
	static void	write_vertex(FILE* rw, heightfield& hf,
					 int level, const FPoint3& box_center, const FPoint3& compress_factor,
					 const vert_info& v)
	{
		short	x, y, z;

		x = (int) floor(((v.x * hf.sample_spacing - box_center.x) * compress_factor.x) + 0.5);
		if (v.special) {
			y = v.y;
		} else {
			y = hf.height(v.x, v.z);
		}
		z = (int) floor(((v.z * hf.sample_spacing - box_center.z) * compress_factor.z) + 0.5);

		WriteUint16(rw, x);
		WriteUint16(rw, y);
		WriteUint16(rw, z);

		// Morph info.  Calculate the difference between the
		// vert height, and the height of the same spot in the
		// next lower-LOD mesh.
		short	lerped_height;
		if (v.special) {
			lerped_height = y;	// special verts don't morph.
		} else {
			lerped_height = get_height_at_LOD(hf, level + 1, v.x, v.z);
		}
		int	morph_delta = (lerped_height - y);
		WriteUint16(rw, (short) morph_delta);
		assert(morph_delta == (short) morph_delta);	// Watch out for overflow.
	}


	// Write out the current chunk.
	bool	write(FILE* rw, heightfield& hf, int level)
	{
		// Write min & max y values.  This can be used to reconstruct
		// the bounding box.
		WriteUint16(rw, min_y);
		WriteUint16(rw, max_y);

		// Write a placeholder for the mesh data file pos.
		int	current_pos = ftell(rw);
		WriteUint32(rw, 0);

		// write out the vertex data at the *end* of the file.
		fseek(rw, 0, SEEK_END);
		int	mesh_pos = ftell(rw);

		// Compute bounding box.  Determines the scale and offset for
		// quantizing the verts.
		FPoint3	box_center = (min + max) * 0.5f;
		FPoint3	box_extent = (max - min) * 0.5f;

		// Use (1 << 14) values in both positive and negative
		// directions.  Wastes just under 1 bit, but the total range
		// is [-2^14, 2^14], which is 2^15+1 values.  This is good --
		// fits nicely w/ 2^N+1 dimensions of binary-triangle-tree
		// vertex locations.

		FPoint3	compress_factor;
		for (int i = 0; i < 3; i++) {
			compress_factor[i] = (1 << 14) / std::max(1.0f, box_extent[i]);
		}

		// Make sure the vertex buffer is not too big.
		int numverts = vertices.size();
		if (numverts > g_chunkstats.output_most_vertices_per_chunk)
			g_chunkstats.output_most_vertices_per_chunk = numverts;
		if (numverts >= (1 << 16)) {
			//printf("error: chunk contains > 64K vertices.  Try processing again, but use\n"
			//       "a deeper chunk tree.\n");
			return false;
		}

		// Write vertices.  All verts contain morph info.
		WriteUint16(rw, vertices.size());
		for (int i = 0; i < vertices.size(); i++) {
			write_vertex(rw, hf, level, box_center, compress_factor, vertices[i]);
		}

		// Write triangle-strip vertex indices.
		WriteUint32(rw, vertex_indices.size());
		for (int i = 0; i < vertex_indices.size(); i++) {
			WriteUint16(rw, vertex_indices[i]);
		}

		// Count the real triangles in the main chunk.
		int	tris = 0;
		for (int i = 0; i < vertex_indices.size() - 2; i++) {
			if (vertex_indices[i] != vertex_indices[i+1]
				&& vertex_indices[i] != vertex_indices[i+2])
			{
				// Real triangle.
				tris++;
			}
		}

		g_chunkstats.output_real_triangles += tris;
		g_chunkstats.output_degenerate_triangles += (vertex_indices.size() - 2) - tris;

		// Write real triangle count.
		WriteUint32(rw, tris);

		// Rewind, and fill in the mesh data file pos.
		fseek(rw, current_pos, SEEK_SET);
		WriteUint32(rw, mesh_pos);

		return true;
	}


	// Update our bounding box given the specified newly added vertex.
	void	update_bounds(heightfield& hf, const FPoint3& v, short y)
	{
		for (int i = 0; i < 3; i++) {
			if (v[i] < min[i]) {
				min[i] = v[i];
			}
			if (v[i] > max[i]) {
				max[i] = v[i];
			}
		}

		// Update min/max y.
		if (y < min_y) {
			min_y = y;
		}
		if (y > max_y) {
			max_y = y;
		}

		// Peephole optimization: if the strip begins with three of
		// the same vert in a row, as generate_block() often causes,
		// then the first two are a no-op, so strip them away.
		if (vertex_indices.size() == 3
			&& vertex_indices[0] == vertex_indices[1]
			&& vertex_indices[0] == vertex_indices[2])
		{
			vertex_indices.resize(1);
		}
	}


	// Call this in strip order.  Inserts the given vert into the current strip.
	void	emit_vertex(heightfield& hf, int x, int z)
	{
		int	index = get_vertex_index(x, z);
		vertex_indices.push_back(index);

		// Check coordinates and update bounding box.
		short	y = hf.height(x, z);
		FPoint3	v(x * hf.sample_spacing, y * hf.vertical_scale, z * hf.sample_spacing);
		update_bounds(hf, v, y);
	}


	// Emit a vertex that's not on the heightfield (and doesn't have vertex sharing).
	void	emit_special_vertex(heightfield& hf, int x, short y, int z)
	{
		int	index = special_vertex_index(x, y, z);
		vertex_indices.push_back(index);

		// Check coordinates and update bounding box.
		FPoint3	v(x * hf.sample_spacing, y * hf.vertical_scale, z * hf.sample_spacing);
		update_bounds(hf, v, y);
	}


	// Emit the last vertex index again.  This creates a degenerate
	// triangle, useful for ending a strip and starting a new strip
	// elsewhere.
	void	emit_previous_vertex()
	{
		assert(vertex_indices.size() > 0);
		int	last_index = vertex_indices[vertex_indices.size() - 1];
		vertex_indices.push_back(last_index);
	}


	// Return the current count of strip indices.  This is helpful for
	// determining whether we need an extra degenerate triangle to get
	// the correct winding order for a new strip.
	int	get_index_count()
	{
		return vertex_indices.size();
	}
};

