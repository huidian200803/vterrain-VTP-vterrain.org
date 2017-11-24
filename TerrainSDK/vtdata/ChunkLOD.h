#include "vtdata/ElevationGrid.h"

// struct to hold statistics.
struct chunkstats {
	int	input_vertices;
	int	output_vertices;
	int	output_real_triangles;
	int	output_degenerate_triangles;
	int	output_chunks;
	int	output_size;
	int output_most_vertices_per_chunk;
	int	total_chunks;

	chunkstats() { clear(); }
	void clear() {
		input_vertices = 0;
		output_vertices = 0;
		output_real_triangles = 0;
		output_degenerate_triangles = 0;
		output_chunks = 0;
		output_size = 0;
		output_most_vertices_per_chunk = 0;
		total_chunks = 0;
	}
};

/**
 * Functionality for dealing with ChunkLOD files.
 */
class HeightfieldChunker
{
public:
	bool ProcessGrid(const vtElevationGrid *grid, FILE* out,
		int tree_depth,	float base_max_error,
		float vertical_scale, float input_vertical_scale,
		bool progress_callback(int) = NULL);
};

extern struct chunkstats g_chunkstats;
