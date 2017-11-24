/**
\mainpage Virtual Terrain Builder: Developer Documentation

\section overview Overview
VTBuilder is part of the <a href="http://vterrain.org/">Virtual Terrain Project</a>
and distributed under a completely free <a href="../license.txt">open source license</a>.
Although it is generally an end-user GUI application, it contains powerful methods
which can be called by any C++ programmer.  This developer documentation highlights
some of the classes and methods available.

\section top1 Notable
All of the classes are documented under the 'Classes' tab above, but here are
a few of the most potentially useful classes and methods:
	- Builder
		- Builder::ImportDataFromFile
		- Builder::AddLayer
		- Builder::RemoveLayer
		- Builder::DoSampleElevationToTilePyramids
		- Builder::DoSampleImageryToTilePyramids
	- vtLayer
		- vtLayer::GetExtent
		- vtLayer::GetProjection
		- vtLayer::TransformCoords
	- TilingOptions

\section top2 Functions
	- MatchTilingToResolution()

\section top3 Options
	The global object <b>g_Options</b> contains all the user-level options.
	It is of type <a href="http://vterrain.org/Doc/vtdata/classvtTagArray.html">vtTagArray</a>.

	The following values may be especially useful to set.  They are true/false,
	default false, except as noted.

	<table style="border-collapse: collapse;">
	<tr>
		<th>Option</th><th>Meaning</th>
	</tr>
	<tr>
		<td>TAG_USE_CURRENT_CRS</td>
		<td>If a layer is loaded and added to the Builder object, and it lacks a
			CRS, then the Builder's current CRS is assumed.  Recommended value for
			non-GUI usage: true.</td>
	</tr>
	<tr>
		<td>TAG_LOAD_IMAGES_ALWAYS</td>
		<td>Always load images into memory.</td>
	</tr>
	<tr>
		<td>TAG_LOAD_IMAGES_NEVER</td>
		<td>Always load images out-of-core.</td>
	</tr>
	<tr>
		<td>TAG_REPRO_TO_FLOAT_ALWAYS</td>
		<td>Always upgrade elevation layers with integer height values to floating point upon reprojection.</td>
	</tr>
	<tr>
		<td>TAG_REPRO_TO_FLOAT_NEVER</td>
		<td>Never upgrade elevation layers with integer height values to floating point upon reprojection.</td>
	</tr>
	<tr>
		<td>TAG_SLOW_FILL_GAPS</td>
		<td>Use the slower and smoother algorithm to fill gaps in elevation grids.</td>
	</tr>
	<tr>
		<td>TAG_SAMPLING_N</td>
		<td>Set to N for NxN multisampling for imagery.</td>
	</tr>
	<tr>
		<td>TAG_BLACK_TRANSP</td>
		<td>Treat black pixels in image layers as transparent, when sampling.</td>
	</tr>
	<tr>
		<td>TAG_TIFF_COMPRESS</td>
		<td>Write TIFF files with DEFLATE compression.</td>
	</tr>
	</table>

	<b>Example:</b>
	<p><code>g_Options.SetValueBool(TAG_REPRO_TO_FLOAT_ALWAYS, true);</code></p>
	<p><code>g_Options.SetValueInt(TAG_SAMPLING_N, 3, true);</code></p>
 */*/
