//
// vtTin.cpp
//
// Class which represents a Triangulated Irregular Network.
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <algorithm>	// for std::min/max

#include "vtTin.h"
#include "vtLog.h"
#include "DxfParser.h"
#include "FilePath.h"
#include "ByteOrder.h"


vtTin::vtTin()
{
	m_trianglebins = NULL;
}

vtTin::~vtTin()
{
	FreeData();
}

void vtTin::AddVert(const DPoint2 &p, float z)
{
	m_vert.Append(p);
	m_z.push_back(z);
}

// Add vertex with vertex normal
void vtTin::AddVert(const DPoint2 &p, float z, FPoint3 &normal)
{
	m_vert.Append(p);
	m_z.push_back(z);
	m_vert_normal.Append(normal);
}

void vtTin::AddTri(int i1, int i2, int i3, int surface_type)
{
	m_tri.push_back(i1);
	m_tri.push_back(i2);
	m_tri.push_back(i3);
	if (surface_type != -1)
		m_surfidx.push_back(surface_type);
}

void vtTin::RemVert(int v)
{
	// safety check
	if (v < 0 || v >= (int) m_vert.GetSize())
		return;
	m_vert.RemoveAt(v);
	m_z.erase(m_z.begin() + v);
	m_vert_normal.RemoveAt(v);

	// Re-index the triangles
	for (uint i = 0; i < m_tri.size()/3; i++)
	{
		// Remove any triangles which referenced this vertex
		if (m_tri[i*3 + 0] == v ||
			m_tri[i*3 + 1] == v ||
			m_tri[i*3 + 2] == v)
		{
			m_tri.erase(m_tri.begin() + i*3, m_tri.begin() + (i*3) + 3);
			i--;
			continue;
		}
		// For all other triangles, adjust the indices to reflect the removal
		if (m_tri[i*3 + 0] > v) m_tri[i*3 + 0] = m_tri[i*3 + 0] - 1;
		if (m_tri[i*3 + 1] > v) m_tri[i*3 + 1] = m_tri[i*3 + 1] - 1;
		if (m_tri[i*3 + 2] > v) m_tri[i*3 + 2] = m_tri[i*3 + 2] - 1;
	}
}

void vtTin::RemTri(int t)
{
	// safety check
	if (t < 0 || t >= (int) m_tri.size())
		return;
	m_tri.erase(m_tri.begin() + t*3, m_tri.begin() + t*3 + 3);
}

uint vtTin::AddSurfaceType(const vtString &surface_texture, float fTiling)
{
	m_surftypes.push_back(surface_texture);
	m_surftype_tiling.push_back(fTiling);
	return m_surftypes.size()-1;
}

void vtTin::SetSurfaceType(int iTri, int surface_type)
{
	if (m_surfidx.size() != m_tri.size()/3)
		m_surfidx.resize(m_tri.size()/3);

	m_surfidx[iTri] = surface_type;
}

bool vtTin::_ReadTinOld(FILE *fp)
{
	int i, num;
	FPoint3 f;
	DPoint2 p;

	fread(&num, 1, sizeof(int), fp);
	m_vert.SetMaxSize(num);
	for (i = 0; i < num; i++)
	{
		fread(&f.x, 3, sizeof(float), fp);

		p.Set(f.x, f.y);
		AddVert(p, f.z);
	}
	for (i = 0; i < num/3; i++)
	{
		AddTri(i*3, i*3+1, i*3+2);
	}
	return true;
}

bool vtTin::_ReadTin(FILE *fp, bool progress_callback(int))
{
	if (!_ReadTinHeader(fp))
		return false;
	if (!_ReadTinBody(fp, progress_callback))
		return false;
	return true;
}

bool vtTin::_ReadTinHeader(FILE *fp)
{
	int crs_length;

	char marker[5];
	fread(marker, 5, 1, fp);
	if (strncmp(marker, "tin", 3))
		return false;	// Not a Tin
	int version = marker[4] - '0';

	fread(&m_file_verts, 4, 1, fp);
	fread(&m_file_tris, 4, 1, fp);
	fread(&m_file_data_start, 4, 1, fp);
	fread(&crs_length, 4, 1, fp);
	if (crs_length > 2000)
		return false;

	if (crs_length)
	{
		char wkt_buf[2000], *wkt = wkt_buf;
		fread(wkt, crs_length, 1, fp);
		wkt_buf[crs_length] = 0;

		OGRErr err = m_crs.importFromWkt((char **) &wkt);
		if (err != OGRERR_NONE)
			return false;
	}

	if (version > 1)
	{
		// version 2 of the format has extents: left, top, right, bottom, min z, max h
		fread(&m_EarthExtents.left, sizeof(double), 4, fp);
		fread(&m_fMinHeight, sizeof(float), 1, fp);
		fread(&m_fMaxHeight, sizeof(float), 1, fp);
	}

	return true;
}

bool vtTin::_ReadTinBody(FILE *fp, bool progress_callback(int))
{
	fseek(fp, m_file_data_start, SEEK_SET);

	// pre-allocate for efficiency
	m_vert.SetMaxSize(m_file_verts);
	m_tri.reserve(m_file_tris * 3);

	// read verts
	DPoint2 p;
	float z;
	for (int i = 0; i < m_file_verts; i++)
	{
		if (progress_callback != NULL && (i % 1024) == 0)
			progress_callback(i * 49 / m_file_verts);

		fread(&p.x, 8, 2, fp);	// 2 doubles
		fread(&z, 4, 1, fp);	// 1 float
		AddVert(p, z);
	}
	// read tris
	int tribuf[3];
	for (int i = 0; i < m_file_tris; i++)
	{
		if (progress_callback != NULL && (i % 1024) == 0)
			progress_callback(50 + i * 49 / m_file_tris);

		fread(tribuf, 4, 3, fp);	// 3 ints
		AddTri(tribuf[0], tribuf[1], tribuf[2]);
	}
	return true;
}

/**
 * Read the TIN from a native TIN format (.itf) file.
 */
bool vtTin::Read(const char *fname, bool progress_callback(int))
{
	// first read the point from the .tin file
	FILE *fp = vtFileOpen(fname, "rb");
	if (!fp)
		return false;

	bool success = _ReadTin(fp, progress_callback);
	fclose(fp);

	if (!success)
		return false;

	ComputeExtents();
	return true;
}

/**
 * Read the TIN header from a native TIN format (.itf).  Reading the header is
 * quick and lets you query properties (NumVerts, NumTris, GetEarthExtents)
 * before loading the rest of the file.
 */
bool vtTin::ReadHeader(const char *fname)
{
	// first read the point from the .tin file
	FILE *fp = vtFileOpen(fname, "rb");
	if (!fp)
		return false;

	bool success = _ReadTinHeader(fp);
	fclose(fp);

	if (!success)
		return false;

	return true;
}

/**
 * Read the TIN body from a native TIN format (.itf) file.  You should
 * first call ReadHeader() if you are doing a two-part read.
 */
bool vtTin::ReadBody(const char *fname, bool progress_callback(int))
{
	// first read the point from the .tin file
	FILE *fp = vtFileOpen(fname, "rb");
	if (!fp)
		return false;

	bool success = _ReadTinBody(fp, progress_callback);
	fclose(fp);

	if (!success)
		return false;

	return true;
}

/**
 * Attempt to read TIN data from a DXF file.
 */
bool vtTin::ReadDXF(const char *fname, bool progress_callback(int))
{
	VTLOG("vtTin::ReadDXF():\n");

	std::vector<DxfEntity> entities;
	std::vector<DxfLayer> layers;

	DxfParser parser(fname, entities, layers);
	bool bSuccess = parser.RetrieveEntities(progress_callback);
	if (!bSuccess)
	{
		VTLOG(parser.GetLastError());
		return false;
	}

	int vtx = 0;
	int found = 0;
	for (uint i = 0; i < entities.size(); i++)
	{
		const DxfEntity &ent = entities[i];
		if (ent.m_iType == DET_3DFace || ent.m_iType == DET_Polygon)
		{
			int NumVerts = ent.m_points.size();
			if (NumVerts == 3)
			{
				for (int j = 0; j < 3; j++)
				{
					DPoint2 p(ent.m_points[j].x, ent.m_points[j].y);
					float z = (float) ent.m_points[j].z;

					AddVert(p, z);
				}
				AddTri(vtx, vtx+1, vtx+2);
				vtx += 3;
				found ++;
			}
		}
	}
	VTLOG(" Found %d triangle entities, of type 3DFace or Polygon.\n", found);

	// If we didn't find any surfaces, we haven't got a TIN
	if (found == 0)
		return false;

	// Test each triangle for clockwisdom, fix if needed
	CleanupClockwisdom();

	ComputeExtents();
	return true;
}

bool vtTin::ReadADF(const char *fname, bool progress_callback(int))
{
	const vtString tnxy_name = fname;
	if (tnxy_name.Right(6) != "xy.adf")
		return false;

	vtString base = tnxy_name.Left(tnxy_name.GetLength()-6);
	vtString tnz_name = base + "z.adf";
	vtString tnod_name = base + "od.adf";

	FILE *fp1 = vtFileOpen(tnxy_name, "rb");
	FILE *fp2 = vtFileOpen(tnz_name, "rb");
	FILE *fp3 = vtFileOpen(tnod_name, "rb");
	if (!fp1 || !fp2 || !fp3)
		return false;

	fseek(fp1, 0, SEEK_END);
	const int length_xy = ftell(fp1);
	rewind(fp1);	// go back again
	uint num_points = length_xy / 16;	// X and Y, each 8 byte doubles

	fseek(fp2, 0, SEEK_END);
	const int length_z = ftell(fp2);
	rewind(fp2);	// go back again
	uint num_heights = length_z / 4;		// Z is a 4 byte float

	DPoint2 p;
	float z;
	for (uint i = 0; i < num_points; i++)
	{
		if ((i%200) == 0 && progress_callback != NULL)
			progress_callback(i * 40 / num_points);

		FRead(&p.x, DT_DOUBLE, 2, fp1, BO_BIG_ENDIAN, BO_LITTLE_ENDIAN);
		FRead(&z, DT_FLOAT, 1, fp2, BO_BIG_ENDIAN, BO_LITTLE_ENDIAN);
		AddVert(p, z);
	}

	fseek(fp3, 0, SEEK_END);
	const int length_od = ftell(fp3);
	rewind(fp3);	// go back again
	const uint num_faces = length_od / 12;		// A B C as 4-byte ints

	int v[3];
	for (uint i = 0; i < num_faces; i++)
	{
		if ((i%200) == 0 && progress_callback != NULL)
			progress_callback(40 + i * 40 / num_faces);

		FRead(v, DT_INT, 3, fp3, BO_BIG_ENDIAN, BO_LITTLE_ENDIAN);
		AddTri(v[0]-1, v[1]-1, v[2]-1);
	}

	fclose(fp1);
	fclose(fp2);
	fclose(fp3);

	// Cleanup: the ESRI TIN contains four "boundary" point far outside the
	//  extents (directly North, South, East, and West).  We should ignore
	//  those four points and the triangles connected to them.
	// It seems we can assume the four 'extra' vertices are the first four.
	m_vert.RemoveAt(0, 4);
	m_z.erase(m_z.begin(), m_z.begin() + 4);
	m_vert_normal.RemoveAt(0, 4);

	// Re-index the triangles
	uint total = m_tri.size()/3;
	for (uint i = 0; i < total; i++)
	{
		if ((i%200) == 0 && progress_callback != NULL)
			progress_callback(80 + i * 20 / total);

		// Remove any triangles which referenced this vertex
		if (m_tri[i*3 + 0] < 4 ||
			m_tri[i*3 + 1] < 4 ||
			m_tri[i*3 + 2] < 4)
		{
			m_tri.erase(m_tri.begin() + i*3, m_tri.begin() + i*3 + 3);
			i--;
			total--;
			continue;
		}
	}
	// For all other triangles, adjust the indices to reflect the removal
	for (uint i = 0; i < m_tri.size(); i++)
		m_tri[i] = m_tri[i] - 4;

	// Test each triangle for clockwisdom, fix if needed
	CleanupClockwisdom();

	ComputeExtents();

	return true;
}

/**
 * Write the TIN to the Aquaveo GMS format.
 */
bool vtTin::ReadGMS(const char *fname, bool progress_callback(int))
{
	FILE *fp = vtFileOpen(fname, "rb");
	if (!fp)
		return false;

	char buf[256];
	vtString tin_name;
	int material_id;
	int num_points;

	// first line is file identifier
	if (fgets(buf, 256, fp) == NULL)
		return false;

	if (strncmp(buf, "TIN", 3) != 0)
		return false;

	while (1)
	{
		if (fgets(buf, 256, fp) == NULL)
			break;

		// trim trailing EOL characters
		vtString vstr = buf;
		vstr.Remove('\r');
		vstr.Remove('\n');
		const char *str = (const char *)vstr;

		if (!strncmp(str, "BEGT", 4))	// beginning of TIN block
			continue;

		if (!strncmp(str, "ID", 2))	// material ID
		{
			sscanf(str, "ID %d", &material_id);
		}
		else if (!strncmp(str, "MAT", 3))	// material ID
		{
			sscanf(str, "MAT %d", &material_id);
		}
		else if (!strncmp(str, "TCOL", 4))	// material ID
		{
			sscanf(str, "TCOL %d", &material_id);
		}
		else if (!strncmp(str, "TNAM", 4))	// TIN name
		{
			tin_name = str + 5;
		}
		else if (!strncmp(str, "VERT", 4))	// Beginning of vertices
		{
			sscanf(buf, "VERT %d\n", &num_points);
			DPoint2 p;
			float z;
			int optional;
			for (int i = 0; i < num_points; i++)
			{
				if (fgets(buf, 256, fp) == NULL)
					break;

				// First three are X, Y, Z.  Optional fourth is "ID" or "locked".
				sscanf(buf, "%lf %lf %f %d", &p.x, &p.y, &z, &optional);
#if 0
				// Some files have Y/-Z flipped (but they are non-standard)
				double temp = p.y; p.y = -z; z = temp;
#endif

				AddVert(p, z);

				if ((i%200) == 0 && progress_callback != NULL)
				{
					if (progress_callback(i * 49 / num_points))
					{
						fclose(fp);
						return false;	// user cancelled
					}
				}
			}
		}
		else if (!strncmp(str, "TRI", 3))	// Beginning of triangles
		{
			int num_faces;
			sscanf(str, "TRI %d\n", &num_faces);
			int v[3];
			for (int i = 0; i < num_faces; i++)
			{
				fscanf(fp, "%d %d %d\n", v, v+2, v+1);
				// the indices in the file are 1-based, so subtract 1
				AddTri(v[0]-1, v[1]-1, v[2]-1);

				if ((i%200) == 0 && progress_callback != NULL)
				{
					if (progress_callback(49 + i * 50 / num_faces))
					{
						fclose(fp);
						return false;	// user cancelled
					}
				}
			}
		}
	}
	fclose(fp);
	ComputeExtents();
	return true;
}

/**
 * Write the TIN to the GMS format.  Historically GMS stood for 'Groundwater
 * Modeling System' from the EMS-I company, now called Aquaveo.
 */
bool vtTin::WriteGMS(const char *fname, bool progress_callback(int)) const
{
	FILE *fp = vtFileOpen(fname, "wb");
	if (!fp)
		return false;

	// first line is file identifier
	fprintf(fp, "TIN\n");
	fprintf(fp, "BEGT\n");
	fprintf(fp, "ID 1\n");			// Indices start at 1
	//fprintf(fp, "TNAM tin\n");	// "name" of the TIN; optional
	//fprintf(fp, "MAT 1\n");		// "TIN material ID"; optional

	int count = 0;
	const int verts = NumVerts();
	const int tris = NumTris();
	const int total = verts + tris;

	// write verts
	fprintf(fp, "VERT %d\n", verts);
	for (int i = 0; i < verts; i++)
	{
		fprintf(fp, "%lf %lf %f\n", m_vert[i].x, m_vert[i].y, m_z[i]);

		if (progress_callback && (++count % 200) == 0)
			progress_callback(count * 99 / total);
	}
	// write tris
	fprintf(fp, "TRI %d\n", tris);
	for (int i = 0; i < tris; i++)
	{
		// the indices in the file are 1-based, so add 1
		fprintf(fp, "%d %d %d\n", m_tri[i*3+0]+1, m_tri[i*3+1]+1, m_tri[i*3+2]+1);

		if (progress_callback && (++count % 200) == 0)
			progress_callback(count * 99 / total);
	}
	fprintf(fp, "ENDT\n");
	fclose(fp);
	return true;
}

bool vtTin::ReadPLY(const char *fname, bool progress_callback(int))
{
	FILE *fp = vtFileOpen(fname, "rb");
	if (!fp)
		return false;

	VTLOG("ReadPLY '%s'\n", fname);

	char buf[256];
	int material_id;
	int num_points;
	int num_faces;

	// first line is file identifier
	if (fgets(buf, 256, fp) == NULL)
		return false;

	if (strncmp(buf, "ply", 3) != 0)
		return false;

	while (fgets(buf, 256, fp) != NULL)
	{
		// trim trailing EOL characters
		vtString vstr = buf;
		vstr.Remove('\r');
		vstr.Remove('\n');
		const char *str = (const char *)vstr;

		if (!strncmp(str, "format", 6))	// beginning of TIN block
			continue;

		if (!strncmp(str, "ID", 2))	// material ID
		{
			sscanf(str, "ID %d", &material_id);
		}
		else if (!strncmp(str, "MAT", 3))	// material ID
		{
			sscanf(str, "MAT %d", &material_id);
		}
		else if (!strncmp(str, "TCOL", 4))	// material ID
		{
			sscanf(str, "TCOL %d", &material_id);
		}
 		else if (!strncmp(str, "element vertex", 14))	// Number of vertices
		{
			sscanf(buf, "element vertex %d\n", &num_points);
		}
 		else if (!strncmp(str, "element face", 12))	// Number of triangles
		{
			sscanf(buf, "element face %d\n", &num_faces);
		}
		else if (!strncmp(str, "end_header", 10))
		{
			DPoint2 p;
			float z;
			int optional;

			VTLOG("ReadPLY num_points %d\n", num_points);

			for (int i = 0; i < num_points; i++)
			{
				if (fgets(buf, 256, fp) == NULL)
					break;

				// First three are X, Y, Z.  Optional fourth is "ID" or "locked".
				sscanf(buf, "%lf %lf %f %d", &p.x, &p.y, &z, &optional);
#if 0
				// Some files have Y/-Z flipped (but they are non-standard)
				double temp = p.y; p.y = -z; z = temp;
#endif
				AddVert(p, z);

				if ((i%200) == 0 && progress_callback != NULL)
				{
					if (progress_callback(i * 49 / num_points))
					{
						fclose(fp);
						return false;	// user cancelled
					}
				}
			}

			// Then read the triangles
			VTLOG("ReadPLY num_faces %d\n", num_faces);

			int inu, a, b, c;
			for (int i = 0; i < num_faces; i++)
			{
				if (fgets(buf, 256, fp) == NULL)
					break;

				sscanf(buf, "%d %d %d %d\n", &inu, &a, &b, &c);
				AddTri(a, b, c);

				if ((i%200) == 0 && progress_callback != NULL)
				{
					if (progress_callback(49 + i * 50 / num_faces))
					{
						fclose(fp);
						return false;	// user cancelled
					}
				}
			}
		}
	}
	fclose(fp);
	ComputeExtents();
	return true;
}

/**
 * Write the TIN to a Collada (.dae) file.  Note that we write X and Y as
 * geographic coordinates, but DAE only supports single-precision floating
 * point values, so it may lose some precision.
 */
bool vtTin::WriteDAE(const char *fname, bool progress_callback(int)) const
{
	FILE *fp = vtFileOpen(fname, "wb");
	if (!fp)
		return false;

	// first line is file identifier
	fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n");
	fprintf(fp, "<COLLADA xmlns=\"http://www.collada.org/2005/11/COLLADASchema\" version=\"1.4.1\">\n");
	fprintf(fp, "  <asset>\n");
	fprintf(fp, "    <contributor>\n");
	fprintf(fp, "      <authoring_tool>VTBuilder</authoring_tool>\n");
	fprintf(fp, "    </contributor>\n");
//	fprintf(fp, "    <created>2012-01-09T14:26:45Z</created>\n");
//	fprintf(fp, "    <modified>2012-01-09T14:26:45Z</modified>\n");
//	fprintf(fp, "    <unit meter=\"0.02539999969303608\" name=\"inch\" />\n");
	fprintf(fp, "    <up_axis>Z_UP</up_axis>\n");
	fprintf(fp, "  </asset>\n");
	fprintf(fp, "  <library_visual_scenes>\n");
	fprintf(fp, "    <visual_scene id=\"ID1\">\n");
	fprintf(fp, "      <node name=\"VTBuilder\">\n");
	fprintf(fp, "        <node id=\"ID2\" name=\"Earth_Terrain\">\n");
	fprintf(fp, "          <matrix>1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1</matrix>\n");
	fprintf(fp, "          <instance_geometry url=\"#ID3\">\n");
	fprintf(fp, "            <bind_material>\n");
	fprintf(fp, "              <technique_common>\n");
	fprintf(fp, "                <instance_material symbol=\"Material2\" target=\"#ID4\">\n");
	fprintf(fp, "                  <bind_vertex_input semantic=\"UVSET0\" input_semantic=\"TEXCOORD\" input_set=\"0\" />\n");
	fprintf(fp, "                </instance_material>\n");
	fprintf(fp, "              </technique_common>\n");
	fprintf(fp, "            </bind_material>\n");
	fprintf(fp, "          </instance_geometry>\n");
	fprintf(fp, "        </node>\n");
	fprintf(fp, "      </node>\n");
	fprintf(fp, "    </visual_scene>\n");
	fprintf(fp, "  </library_visual_scenes>\n");
	fprintf(fp, "  <library_geometries>\n");
	fprintf(fp, "    <geometry id=\"ID3\">\n");
	fprintf(fp, "      <mesh>\n");
	fprintf(fp, "        <source id=\"ID6\">\n");

	int count = 0;
	const int verts = NumVerts();
	const int tris = NumTris();
	const int total = verts + tris;

	// Here are:   Count   and   Coordinates X Y Z...
	fprintf(fp, "          <float_array id=\"ID10\" count=\"%d\">\n",verts);

	// write verts
	for (int i = 0; i < verts; i++)
	{
		fprintf(fp, "%lf %lf %f\n", m_vert[i].x, m_vert[i].y, m_z[i]);

		if (progress_callback && (++count % 200) == 0)
			progress_callback(count * 99 / total);
	}
	fprintf(fp, "          </float_array>\n");

	fprintf(fp, "          <technique_common>\n");
	fprintf(fp, "            <accessor count=\"222\" source=\"#ID10\" stride=\"3\">\n");
	fprintf(fp, "              <param name=\"X\" type=\"float\" />\n");
	fprintf(fp, "              <param name=\"Y\" type=\"float\" />\n");
	fprintf(fp, "              <param name=\"Z\" type=\"float\" />\n");
	fprintf(fp, "            </accessor>\n");
	fprintf(fp, "          </technique_common>\n");
	fprintf(fp, "        </source>\n");
	fprintf(fp, "\n");
	fprintf(fp, "        <source id=\"ID8\">\n");
	fprintf(fp, "          <Name_array id=\"ID12\" count=\"0\" />\n");
	fprintf(fp, "          <technique_common>\n");
	fprintf(fp, "            <accessor count=\"0\" source=\"#ID12\" stride=\"1\">\n");
	fprintf(fp, "              <param name=\"skp_material\" type=\"Name\" />\n");
	fprintf(fp, "            </accessor>\n");
	fprintf(fp, "          </technique_common>\n");
	fprintf(fp, "        </source>\n");
	fprintf(fp, "        <vertices id=\"ID9\">\n");
	fprintf(fp, "          <input semantic=\"POSITION\" source=\"#ID6\" />\n");
	fprintf(fp, "          <input semantic=\"NORMAL\" source=\"#ID7\" />\n");
	fprintf(fp, "        </vertices>\n");

	// Here is triangles Count
	fprintf(fp, "        <triangles count=\"%d\" material=\"Material2\">\n", tris);
	fprintf(fp, "          <input offset=\"0\" semantic=\"VERTEX\" source=\"#ID9\" />\n");
	fprintf(fp, "          <p>\n");

	// write tris
	//	fprintf(fp, "TRI %d\n", tris);
	for (int i = 0; i < tris; i++)
	{
		// Here is     triangle definition (zero based)  A B C ...
		// the indices in the file are 1-based, so add 1
		fprintf(fp, "%d %d %d\n", m_tri[i*3+0], m_tri[i*3+1], m_tri[i*3+2]);

		if (progress_callback && (++count % 200) == 0)
			progress_callback(count * 99 / total);
	}

	fprintf(fp, "</p>\n");

	fprintf(fp, "        </triangles>\n");
	fprintf(fp, "      </mesh>\n");
	fprintf(fp, "    </geometry>\n");
	fprintf(fp, "  </library_geometries>\n");
	fprintf(fp, "  <library_materials>\n");
	fprintf(fp, "    <material id=\"ID4\" name=\"Google_Earth_Snapshot\">\n");
	fprintf(fp, "      <instance_effect url=\"#ID5\" />\n");
	fprintf(fp, "    </material>\n");
	fprintf(fp, "  </library_materials>\n");
	fprintf(fp, "  <library_effects>\n");
	fprintf(fp, "    <effect id=\"ID5\">\n");
	fprintf(fp, "      <profile_COMMON>\n");
	fprintf(fp, "        <technique sid=\"COMMON\">\n");
	fprintf(fp, "          <lambert>\n");
	fprintf(fp, "            <diffuse>\n");

	// Here is the color definition of the surface
	fprintf(fp, "              <color>0.3411764705882353 0.392156862745098 0.3411764705882353 1</color>\n");

	fprintf(fp, "            </diffuse>\n");
	fprintf(fp, "          </lambert>\n");
	fprintf(fp, "        </technique>\n");
	fprintf(fp, "      </profile_COMMON>\n");
	fprintf(fp, "    </effect>\n");
	fprintf(fp, "  </library_effects>\n");
	fprintf(fp, "  <scene>\n");
	fprintf(fp, "    <instance_visual_scene url=\"#ID1\" />\n");
	fprintf(fp, "  </scene>\n");
	fprintf(fp, "</COLLADA>\n");

	fclose(fp);
	return true;
}

/**
 * Write the TIN to a VRML (.wrl) file as an IndexedFaceSet.  Note that we
 * write X and Y as geographic coordinates, but VRML only supports
 * single-precision floating point values, so it may lose some precision.
 */
bool vtTin::WriteWRL(const char *fname, bool progress_callback(int)) const
{
	FILE *fp = vtFileOpen(fname, "wb");
	if (!fp)
		return false;

	fprintf(fp, "#VRML V2.0 utf8\n");
	fprintf(fp, "\n");
	fprintf(fp, "WorldInfo\n");
	fprintf(fp, "  {\n");
	fprintf(fp, "  info\n");
	fprintf(fp, "    [\n");
	fprintf(fp, "    \"Generated by VTBuilder\"\n");
	fprintf(fp, "    ]\n");
	fprintf(fp, "  title \"TIN VRML Model\"\n");
	fprintf(fp, "  }\n");
	fprintf(fp, "\n");
	fprintf(fp, "# TIN---------\n");
	fprintf(fp, "Transform\n");
	fprintf(fp, "  {\n");
	fprintf(fp, "  children\n");
	fprintf(fp, "    [\n");
	fprintf(fp, "    Shape\n");
	fprintf(fp, "      {\n");
	fprintf(fp, "      appearance Appearance\n");
	fprintf(fp, "	{\n");
	fprintf(fp, "	material Material\n");
	fprintf(fp, "	  {\n");
	fprintf(fp, "	  }\n");
	fprintf(fp, "	texture ImageTexture\n");
	fprintf(fp, "	  {\n");
	fprintf(fp, "	  url\n");
	fprintf(fp, "	    [\n");
	fprintf(fp, "	    \"OrtoImage.jpg\"\n");
	fprintf(fp, "	    ]\n");
	fprintf(fp, "	  }\n");
	fprintf(fp, "	}\n");
	fprintf(fp, "      geometry      IndexedFaceSet {\n");
	fprintf(fp, "              ccw FALSE\n");
	fprintf(fp, "              solid FALSE\n");
	fprintf(fp, "              creaseAngle 1.396263\n");
	fprintf(fp, "coord DEF Kxzy Coordinate {\n");
	fprintf(fp, "                                     point [\n");

	int i, count = 0;
	const int verts = NumVerts();
	const int tris = NumTris();
	const int total = verts + tris;

	// write verts
	//	fprintf(fp, "VERT %d\n", verts);
	for (i = 0; i < verts; i++)
	{
		fprintf(fp, "%lf %lf %f\n", m_vert[i].x, m_vert[i].y, m_z[i]);

		if (progress_callback && (++count % 200) == 0)
			progress_callback(count * 99 / total);
	}

	fprintf(fp, "	    ]\n");
	fprintf(fp, "	  }\n");
	fprintf(fp, "      	coordIndex \n");
	fprintf(fp, "                  [\n");

	// write tris
	for (i = 0; i < tris; i++)
	{
		// Here is  triangle definition (zero based)  A B C -1...
		// the indices in the file are 1-based, so add 1
		fprintf(fp, "%d %d %d -1\n", m_tri[i*3+0], m_tri[i*3+1], m_tri[i*3+2]);

		if (progress_callback && (++count % 200) == 0)
			progress_callback(count * 99 / total);
	}
	fprintf(fp, "				  ]\n");
	fprintf(fp, "				}\n");
	fprintf(fp, "			      }\n");
	fprintf(fp, "			    ]\n");
	fprintf(fp, "			  }\n");
	fclose(fp);
	return true;
}

/**
 * Write the TIN to a Wavefront OBJ file.  Note that we write X and Y as
 * geographic coordinates, but OBJ only supports single-precision floating
 * point values, so it may lose some precision.
 */
bool vtTin::WriteOBJ(const char *fname, bool progress_callback(int)) const
{
	FILE *fp = vtFileOpen(fname, "wb");
	if (!fp)
		return false;

	int i, count = 0;
	const int verts = NumVerts();
	const int tris = NumTris();
	const int total = verts + tris;

	fprintf(fp, "####\n");
	fprintf(fp, "#\n");
	fprintf(fp, "# OBJ File Generated by VTBuilder\n");
	fprintf(fp, "#\n");
	fprintf(fp, "####\n");
	fprintf(fp, "# Object %s\n", fname);
	fprintf(fp, "#\n");
	fprintf(fp, "# Vertices: %d\n", verts);
	fprintf(fp, "# Faces: %d\n", tris);
	fprintf(fp, "#\n");
	fprintf(fp, "####\n");

	// write verts
	for (i = 0; i < verts; i++)
	{
		fprintf(fp, "v %lf %lf %f\n", m_vert[i].x, m_vert[i].y, m_z[i]);

		if (progress_callback && (++count % 200) == 0)
			progress_callback(count * 99 / total);
	}

	fprintf(fp, "# %d vertices, 0 vertices normals\n", verts);
	fprintf(fp, "\n");

	// write tris
	for (i = 0; i < tris; i++)
	{
		// Here is     triangle definition (zero based)  A B C ...
		// the indices in the file are 1-based, so add 1
		fprintf(fp, "f %d %d %d\n", m_tri[i*3+0]+1, m_tri[i*3+1]+1, m_tri[i*3+2]+1);

		if (progress_callback && (++count % 200) == 0)
			progress_callback(count * 99 / total);
	}
	fprintf(fp, "# %d faces, 0 coords texture\n", tris);
	fprintf(fp, "\n");
	fprintf(fp, "# End of File\n");
	fclose(fp);
	return true;
}

/**
 * Write the TIN to a Stanford Polygon File Format (PLY),
 * http://en.wikipedia.org/wiki/PLY_(file_format)
 */
bool vtTin::WritePLY(const char *fname, bool progress_callback(int)) const
{
	FILE *fp = vtFileOpen(fname, "wb");
	if (!fp)
		return false;

	int i, count = 0;
	int verts = NumVerts();
	int tris = NumTris();
	int total = verts + tris;

	fprintf(fp, "ply\n");
	fprintf(fp, "format ascii 1.0\n");
	fprintf(fp, "comment VTBuilder generated\n");
	fprintf(fp, "element vertex %d\n", verts);
	fprintf(fp, "property float x\n");
	fprintf(fp, "property float y\n");
	fprintf(fp, "property float z\n");
	fprintf(fp, "element face %d\n", tris);
	fprintf(fp, "property list uchar int vertex_indices\n");
	fprintf(fp, "end_header\n");

	// write verts
	for (i = 0; i < verts; i++)
	{
		fprintf(fp, "%lf %lf %f\n", m_vert[i].x, m_vert[i].y, m_z[i]);

		if (progress_callback && (++count % 200) == 0)
			progress_callback(count * 99 / total);
	}

	// write tris
	for (i = 0; i < tris; i++)
	{
		// Here is  triangle definition (zero based)  A B C ...
		fprintf(fp, "3 %d %d %d\n", m_tri[i*3+0], m_tri[i*3+1], m_tri[i*3+2]);

		if (progress_callback && (++count % 200) == 0)
			progress_callback(count * 99 / total);
	}

	fclose(fp);
	return true;
}

/**
 * Write the TIN to a AutoCAD DXF file using 3DFACE entities.
 */
bool vtTin::WriteDXF(const char *fname, bool progress_callback(int)) const
{
	FILE *fp = vtFileOpen(fname, "wb");
	if (!fp)
		return false;

	// Header
	fprintf(fp, "  0\nSECTION\n");
	fprintf(fp, "  2\nHEADER\n  9\n$ACADVER\n  1\nAC1006\n");
	fprintf(fp, "  9\n$EXTMIN\n");
	fprintf(fp, " 10\n%lf\n", m_EarthExtents.left);
	fprintf(fp, " 20\n%lf\n", m_EarthExtents.bottom);
	fprintf(fp, "  9\n$EXTMAX\n");
	fprintf(fp, " 10\n%lf\n", m_EarthExtents.right);
	fprintf(fp, " 20\n%lf\n", m_EarthExtents.top);
	fprintf(fp, "  0\nENDSEC\n");

	// Tables section
	fprintf(fp, "  0\nSECTION\n");
	fprintf(fp, "  2\nTABLES\n");

	// ------------------------------------
	// Table of Layers
	fprintf(fp, "  0\nTABLE\n");
	fprintf(fp, "  2\nLAYER\n");
	fprintf(fp, " 70\n1\n");	// max number of layers which follow

	// A layer
	fprintf(fp, "  0\nLAYER\n");
	fprintf(fp, "  2\nPEN1\n");	// layer name
	fprintf(fp, " 70\n0\n");	// layer flags
	fprintf(fp, " 62\n3\n");	// color number 3 = green
	fprintf(fp, "  6\nCONTINUOUS\n");	// linetype name

	// end tables layer
	fprintf(fp, "  0\nENDTAB\n");

	// ------------------------------------
	// end tables section
	fprintf(fp, "  0\nENDSEC\n");

	// Entities
	fprintf(fp, "  0\nSECTION\n");
	fprintf(fp, "  2\nENTITIES\n");

	// write tris
	int i, count = 0;
	int verts = NumVerts();
	int tris = NumTris();
	int total = verts + tris;
	const char *layer = "PEN1";
	for (i = 0; i < tris; i++)
	{
		const int v0 = m_tri[i*3+0];
		const int v1 = m_tri[i*3+1];
		const int v2 = m_tri[i*3+2];

		fprintf(fp, "  0\n3DFACE\n");
		fprintf(fp, "  8\n%s\n", layer);
		fprintf(fp, " 62\n     3\n");	// color number
		fprintf(fp, " 10\n%lf\n", m_vert[v0].x);
		fprintf(fp, " 20\n%lf\n", m_vert[v0].y);
		fprintf(fp, " 30\n%f\n", m_z[v0]);
		fprintf(fp, " 11\n%lf\n", m_vert[v1].x);
		fprintf(fp, " 21\n%lf\n", m_vert[v1].y);
		fprintf(fp, " 31\n%f\n", m_z[v1]);
		fprintf(fp, " 12\n%lf\n", m_vert[v2].x);
		fprintf(fp, " 22\n%lf\n", m_vert[v2].y);
		fprintf(fp, " 32\n%f\n", m_z[v2]);
		// DXF wants the last point duplicated to make 4 points.
		fprintf(fp, " 13\n%lf\n", m_vert[v2].x);
		fprintf(fp, " 23\n%lf\n", m_vert[v2].y);
		fprintf(fp, " 33\n%f\n", m_z[v2]);

		if (progress_callback && (++count % 200) == 0)
			progress_callback(count * 99 / total);
	}
	fprintf(fp, "  0\nENDSEC\n");
	fprintf(fp, "  0\nEOF\n");
	fclose(fp);
	return true;
}

void vtTin::FreeData()
{
	m_vert.FreeData();
	m_tri.clear();

	// The bins must be cleared when the triangles are freed
	delete m_trianglebins;
	m_trianglebins = NULL;
}

/**
 * Write the TIN to a TIN (.itf) file (VTP-defined format).
 */
bool vtTin::Write(const char *fname, bool progress_callback(int)) const
{
	FILE *fp = vtFileOpen(fname, "wb");
	if (!fp)
		return false;

	char *wkt;
	OGRErr err = m_crs.exportToWkt(&wkt);
	if (err != OGRERR_NONE)
	{
		fclose(fp);
		return false;
	}
	int crs_length = strlen(wkt);
	int data_start = 5 + 4 + 4 + 4 + + 4 + crs_length + 32 + 4 + 4;

	int i;
	int verts = NumVerts();
	int tris = NumTris();

	fwrite("tin02", 5, 1, fp);	// version 2
	fwrite(&verts, 4, 1, fp);
	fwrite(&tris, 4, 1, fp);
	fwrite(&data_start, 4, 1, fp);
	fwrite(&crs_length, 4, 1, fp);
	fwrite(wkt, crs_length, 1, fp);
	OGRFree(wkt);

	// version 2 of the format has extents: left, top, right, bottom, min z, max h
	fwrite(&m_EarthExtents.left, sizeof(double), 4, fp);
	fwrite(&m_fMinHeight, sizeof(float), 1, fp);
	fwrite(&m_fMaxHeight, sizeof(float), 1, fp);

	// room for future extention: you can add fields here, as long as you
	// increase the data_start offset above accordingly

	// count progress
	int count = 0, total = verts + tris;

	// write verts
	for (i = 0; i < verts; i++)
	{
		fwrite(&m_vert[i].x, 8, 2, fp);	// 2 doubles
		fwrite(&m_z[i], 4, 1, fp);		// 1 float

		if (progress_callback && (++count % 100) == 0)
			progress_callback(count * 99 / total);
	}
	// write tris
	for (i = 0; i < tris; i++)
	{
		fwrite(&m_tri[i*3], 4, 3, fp);	// 3 ints

		if (progress_callback && (++count % 100) == 0)
			progress_callback(count * 99 / total);
	}

	fclose(fp);
	return true;
}

bool vtTin::ComputeExtents()
{
	const int size = NumVerts();
	if (size == 0)
		return false;

	m_EarthExtents.SetInsideOut();
	m_fMinHeight = 1E9;
	m_fMaxHeight = -1E9;

	for (int j = 0; j < size; j++)
	{
		m_EarthExtents.GrowToContainPoint(m_vert[j]);

		float z = m_z[j];
		if (z > m_fMaxHeight)
			m_fMaxHeight = z;
		if (z < m_fMinHeight)
			m_fMinHeight = z;
	}
	return true;
}

void vtTin::Offset(const DPoint2 &p)
{
	const uint size = m_vert.GetSize();
	for (uint j = 0; j < size; j++)
		m_vert[j] += p;
	ComputeExtents();
}

void vtTin::Scale(float fFactor)
{
	const uint size = m_z.size();
	for (uint j = 0; j < size; j++)
		m_z[j] *= fFactor;
	ComputeExtents();
}

void vtTin::VertOffset(float fAmount)
{
	const uint size = m_z.size();
	for (uint j = 0; j < size; j++)
		m_z[j] += fAmount;
	ComputeExtents();
}

/**
 * Test if a given 2D point (x, y) is inside a triangle of this TIN (given
 * by index).  If so, return true and give the elevation value by reference.
 */
bool vtTin::TestTriangle(int tri, const DPoint2 &p, float &fAltitude) const
{
	// get points
	const int v0 = m_tri[tri*3];
	const int v1 = m_tri[tri*3+1];
	const int v2 = m_tri[tri*3+2];
	const DPoint2 &p1 = m_vert[v0];
	const DPoint2 &p2 = m_vert[v1];
	const DPoint2 &p3 = m_vert[v2];

	// First try to identify which triangle
	if (PointInTriangle(p, p1, p2, p3))
	{
		double bary[3];
		if (BarycentricCoords(p1, p2, p3, p, bary))
		{
			// compute barycentric combination of function values at vertices
			const double val = bary[0] * m_z[v0] +
							   bary[1] * m_z[v1] +
							   bary[2] * m_z[v2];
			fAltitude = (float) val;
			return true;
		}
	}
	return false;
}

/**
 * If you are going to do a large number of height-testing of this TIN
 * (with FindAltitudeOnEarth), call this method once first to set up a
 * series of indexing bins which greatly speed up testing.
 *
 * \param bins Number of bins per dimension, e.g. a value of 50 produces
 *		50*50=2500 bins.  More bins produces faster height-testing with
 *		the only tradeoff being a small amount of RAM per bin.
 * \param progress_callback If supplied, this function will be called back
 *		with a value of 0 to 100 as the operation progresses.
 */
void vtTin::SetupTriangleBins(int bins, bool progress_callback(int))
{
	DRECT rect = m_EarthExtents;
	m_BinSize.x = rect.Width() / bins;
	m_BinSize.y = rect.Height() / bins;

	delete m_trianglebins;
	m_trianglebins = new BinArray(bins, bins);

	uint tris = NumTris();
	for (uint i = 0; i < tris; i++)
	{
		if ((i%100)==0 && progress_callback)
			progress_callback(i * 100 / tris);

		// get 2D points
		const DPoint2 &p1 = m_vert[m_tri[i*3]];
		const DPoint2 &p2 = m_vert[m_tri[i*3+1]];
		const DPoint2 &p3 = m_vert[m_tri[i*3+2]];

		// find the correct range of bins, and add the index of this index to it
		DPoint2 fminrange, fmaxrange;

		fminrange.x = std::min(std::min(p1.x, p2.x), p3.x);
		fmaxrange.x = std::max(std::max(p1.x, p2.x), p3.x);

		fminrange.y = std::min(std::min(p1.y, p2.y), p3.y);
		fmaxrange.y = std::max(std::max(p1.y, p2.y), p3.y);

		IPoint2 bin_start, bin_end;

		bin_start.x = (uint) ((fminrange.x-rect.left) / m_BinSize.x);
		bin_end.x = (uint)	 ((fmaxrange.x-rect.left) / m_BinSize.x);

		bin_start.y = (uint) ((fminrange.y-rect.bottom) / m_BinSize.y);
		bin_end.y = (uint)	 ((fmaxrange.y-rect.bottom) / m_BinSize.y);

		for (int j = bin_start.x; j <= bin_end.x; j++)
		{
			for (int k = bin_start.y; k <= bin_end.y; k++)
			{
				Bin *bin = m_trianglebins->GetBin(j, k);
				if (bin)
					bin->push_back(i);
			}
		}
	}
}

int vtTin::MemoryNeededToLoad() const
{
	int bytes = m_file_verts * sizeof(DPoint2);	// xy
	bytes += m_file_verts * sizeof(float);		// z
	bytes += sizeof(int) * 3 * m_file_tris;		// triangles
	return bytes;
}

double vtTin::GetArea2D()
{
	double area = 0.0;
	uint tris = NumTris();
	for (uint i = 0; i < tris; i++)
	{
		const DPoint2 &p1 = m_vert[m_tri[i*3]];
		const DPoint2 &p2 = m_vert[m_tri[i*3+1]];
		const DPoint2 &p3 = m_vert[m_tri[i*3+2]];
		area += AreaOfTriangle(p1, p2, p3);
	}
	return area;
}

double vtTin::GetArea3D()
{
	double area = 0.0;
	uint tris = NumTris();

	for (uint i = 0; i < tris; i++)
	{
		const int v0 = m_tri[i*3];
		const int v1 = m_tri[i*3+1];
		const int v2 = m_tri[i*3+2];
		const DPoint2 &p1 = m_vert[v0];
		const DPoint2 &p2 = m_vert[v1];
		const DPoint2 &p3 = m_vert[v2];
		DPoint3 dp1(p1.x, p1.y, m_z[v0]);
		DPoint3 dp2(p2.x, p2.y, m_z[v1]);
		DPoint3 dp3(p3.x, p3.y, m_z[v2]);

		area += AreaOfTriangle(dp1, dp2, dp3);
	}

	return area;
}

bool vtTin::FindAltitudeOnEarth(const DPoint2 &p, float &fAltitude, bool bTrue) const
{
	int unused_triangle;
	return FindTriangleOnEarth(p, fAltitude, unused_triangle, bTrue);
}

bool vtTin::FindTriangleOnEarth(const DPoint2 &p, float &fAltitude, int &iTriangle,
	bool bTrue) const
{
	uint tris = NumTris();

	// If we have some triangle bins, they can be used for a much faster test
	if (m_trianglebins != NULL)
	{
		int col = (int) ((p.x - m_EarthExtents.left) / m_BinSize.x);
		int row = (int) ((p.y - m_EarthExtents.bottom) / m_BinSize.y);
		Bin *bin = m_trianglebins->GetBin(col, row);
		if (!bin)
			return false;

		for (uint i = 0; i < bin->size(); i++)
		{
			if (TestTriangle(bin->at(i), p, fAltitude))
			{
				iTriangle = bin->at(i);
				return true;
			}
		}
		// If it was not in any of these bins, then it did not hit anything
		return false;
	}
	// If no bins, we do a naive slow search.
	for (uint i = 0; i < tris; i++)
	{
		if (TestTriangle(i, p, fAltitude))
		{
			iTriangle = i;
			return true;
		}
	}
	return false;
}

bool vtTin::FindAltitudeAtPoint(const FPoint3 &p3, float &fAltitude,
		bool bTrue, int iCultureFlags, FPoint3 *vNormal) const
{
	// Convert to 2D earth point, and test vs. TIN triangles
	DPoint3 earth;
	m_LocalCS.LocalToEarth(p3, earth);

	// If we need to provide a normal, do a separate test that gets the triangle
	if (vNormal != NULL)
	{
		int iTriangle;
		const bool hit = FindTriangleOnEarth(DPoint2(earth.x, earth.y),
			fAltitude, iTriangle, bTrue);
		if (hit)
			*vNormal = GetTriangleNormal(iTriangle);
		return hit;
	}
	else
		return FindAltitudeOnEarth(DPoint2(earth.x, earth.y), fAltitude, bTrue);
}

FPoint3 vtTin::GetTriangleNormal(int iTriangle) const
{
	FPoint3 wp0, wp1, wp2;
	_GetLocalTrianglePoints(iTriangle, wp0, wp1, wp2);
	FPoint3 norm = (wp1 - wp0).Cross(wp2 - wp0);
	norm.Normalize();
	return norm;
}

void vtTin::_GetLocalTrianglePoints(int iTriangle, FPoint3 &wp0, FPoint3 &wp1, FPoint3 &wp2) const
{
	const int v0 = m_tri[iTriangle*3];
	const int v1 = m_tri[iTriangle*3+1];
	const int v2 = m_tri[iTriangle*3+2];
	const DPoint2 &p1 = m_vert[v0];
	const DPoint2 &p2 = m_vert[v1];
	const DPoint2 &p3 = m_vert[v2];
	m_LocalCS.EarthToLocal(DPoint3(p1.x, p1.y, m_z[v0]), wp0);
	m_LocalCS.EarthToLocal(DPoint3(p2.x, p2.y, m_z[v1]), wp1);
	m_LocalCS.EarthToLocal(DPoint3(p3.x, p3.y, m_z[v2]), wp2);
}

bool vtTin::ConvertCRS(const vtCRS &crs_new)
{
	// Create conversion object
	ScopedOCTransform trans(CreateCoordTransform(&m_crs, &crs_new));
	if (!trans)
		return false;		// inconvertible coordinate systems

	int size = NumVerts();
	for (int i = 0; i < size; i++)
	{
		DPoint2 &p = m_vert[i];
		trans->Transform(1, &p.x, &p.y);
	}

	// adopt new coordinate system
	m_crs = crs_new;

	return true;
}

/**
 * Test each triangle for clockwisdom, fix if needed.  The result should
 *  be a TIN with consistent vertex ordering, such that all face normals
 *  point up rather than down, that is, counter-clockwise.
 */
void vtTin::CleanupClockwisdom()
{
	int v0, v1, v2;
	uint tris = NumTris();
	for (uint i = 0; i < tris; i++)
	{
		v0 = m_tri[i*3];
		v1 = m_tri[i*3+1];
		v2 = m_tri[i*3+2];
		// get 2D points
		const DPoint2 &p1 = m_vert[v0];
		const DPoint2 &p2 = m_vert[v1];
		const DPoint2 &p3 = m_vert[v2];

		// The so-called 2D cross product
		double cross2d = (p2-p1).Cross(p3-p1);
		if (cross2d < 0)
		{
			// flip
			m_tri[i*3+1] = v2;
			m_tri[i*3+2] = v1;
		}
	}
}

/**
 * Because the TIN triangles refer to their vertices by index, it's possible
 * to have some vertices which are not referenced.  Find and remove those
 * vertices.
 * \return The number of unused vertices removed.
 */
int vtTin::RemoveUnusedVertices()
{
	size_t verts = NumVerts();
	std::vector<bool> used;
	used.resize(verts, false);

	// Flag all the vertices that are used
	size_t tris = NumTris();
	for (size_t i = 0; i < tris; i++)
	{
		used[m_tri[i*3]] = true;
		used[m_tri[i*3+1]] = true;
		used[m_tri[i*3+2]] = true;
	}

	// Remove all the vertices that weren't flagged
	int count = 0;
	for (size_t i = 0; i < verts;)
	{
		if (!used[i])
		{
			// Remove vertex
			RemVert(i);
			used.erase(used.begin()+i);
			verts--;
			count++;
		}
		else
			i++;
	}
	return count;
}

/**
 Copy all the vertices and triangle of another TIN to this one.
 This is a simple join.  No attempt is made to share vertices or any other integration.
 It is further assumed that the two TINs have compatible coordinate systems.
 */
void vtTin::AppendFrom(const vtTin *pTin)
{
	const size_t verts = pTin->NumVerts();
	const size_t tris = pTin->NumTris();

	// Preallocate (for efficiency)
	m_vert.SetMaxSize(m_vert.GetSize() + verts + 1);
	m_z.reserve(m_vert.GetSize() + verts + 1);
	m_tri.reserve(m_tri.size() + (tris*3) + 1);

	// Remember the starting point for vertex indices
	const int base = NumVerts();

	// Simple, naive copy of vertices and triangles
	DPoint2 p;
	float z;
	for (size_t i = 0; i < verts; i++)
	{
		pTin->GetVert(i, p, z);
		AddVert(p, z);
	}
	for (size_t i = 0; i < tris; i++)
	{
		const int *tri = pTin->GetAtTri(i);
		AddTri(base+tri[0], base+tri[1], base+tri[2]);
	}
	ComputeExtents();
}

/**
 * Return the length of the longest edge of a specific triangle.
 */
double vtTin::GetTriMaxEdgeLength(int iTri) const
{
	const int tris = NumTris();
	if (iTri < 0 || iTri >= tris)
		return 0.0;

	// get points
	const int v0 = m_tri[iTri*3];
	const int v1 = m_tri[iTri*3+1];
	const int v2 = m_tri[iTri*3+2];
	const DPoint2 &p1 = m_vert[v0];
	const DPoint2 &p2 = m_vert[v1];
	const DPoint2 &p3 = m_vert[v2];

	// check lengths
	double len1 = (p2 - p1).Length();
	double len2 = (p3 - p2).Length();
	double len3 = (p1 - p3).Length();
	return len1 > len2 ?
		(len1 > len3 ? len1 : len3) :
	(len2 > len3 ? len2 : len3);
}


// Number of bins used by the merge algorithm.  Time is roughly proportional
// to N*N/BINS, where N is the number of vertices, so increase BINS for speed.
//
#define BINS	4000

/**
 * Combine all vertices which are at the same location.  By removing these
 * redundant vertices, the mesh will consume less space in memory and on disk.
 */
void vtTin::MergeSharedVerts(bool progress_callback(int))
{
	uint verts = NumVerts();

	uint i, j;
	int bin;

	DRECT rect = m_EarthExtents;
	double width = rect.Width();

	// make it slightly larger avoid edge condition
	rect.left -= 0.000001;
	width += 0.000002;

	m_bReplace = new int[verts];
	m_vertbin = new Bin[BINS];
	m_tribin = new Bin[BINS];

	// sort the vertices into bins
	for (i = 0; i < verts; i++)
	{
		// flag all vertices initially not to remove
		m_bReplace[i] = -1;

		// find the correct bin, and add the index of this vertex to it
		bin = (int) (BINS * (m_vert[i].x - rect.left) / width);
		m_vertbin[bin].push_back(i);
	}
	uint trisize = m_tri.size();
	for (i = 0; i < trisize; i++)
	{
		// find the correct bin, and add the index of this index to it
		bin = (int) (BINS * (m_vert[m_tri[i]].x - rect.left) / width);
		m_tribin[bin].push_back(i);
	}

	// compare within each bin, and between each adjacent bin,
	// looking for matching vertices to flag for removal
	for (bin = 0; bin < BINS; bin++)
	{
		if (progress_callback != NULL)
			progress_callback(bin * 100 / BINS);

		_CompareBins(bin, bin);
		if (bin < BINS-1)
			_CompareBins(bin, bin+1);
	}
	// now update each triangle index to point to the merge result
	for (bin = 0; bin < BINS; bin++)
	{
		if (progress_callback != NULL)
			progress_callback(bin * 100 / BINS);

		_UpdateIndicesInInBin(bin);
	}

	// now compact the vertex bins into a single array

	// make a copy to copy from
	DLine2 *vertcopy = new DLine2(m_vert);
	float *zcopy = new float[m_z.size()];
	for (i = 0; i < m_z.size(); i++)
		zcopy[i] = m_z[i];

	int inew = 0;	// index into brand new array (actually re-using old)

	for (bin = 0; bin < BINS; bin++)
	{
		if (progress_callback != NULL)
			progress_callback(bin * 100 / BINS);

		uint binverts = m_vertbin[bin].size();
		for (i = 0; i < binverts; i++)
		{
			int v_old = m_vertbin[bin].at(i);
			if (m_bReplace[v_old] != -1)
				continue;

			int v_new = inew;

			// copy old to new
			m_vert[v_new] = vertcopy->GetAt(v_old);
			m_z[v_new] = zcopy[v_old];

			uint bintris = m_tribin[bin].size();
			for (j = 0; j < bintris; j++)
			{
				int trindx = m_tribin[bin].at(j);
				if (m_tri[trindx] == v_old)
					m_tri[trindx] = v_new;
			}
			inew++;
		}
	}

	// our original array containers now hold the compacted result
	int newsize = inew;
	m_vert.SetSize(newsize);
	m_z.resize(newsize);

	// free up all the stuff we allocated
	delete [] m_bReplace;
	delete [] m_vertbin;
	delete [] m_tribin;
	delete vertcopy;
	delete [] zcopy;
}

void vtTin::_UpdateIndicesInInBin(int bin)
{
	int i, j;

	int binverts = m_vertbin[bin].size();
	for (i = 0; i < binverts; i++)
	{
		int v_before = m_vertbin[bin].at(i);
		int v_after = m_bReplace[v_before];

		if (v_after == -1)
			continue;

		int bintris = m_tribin[bin].size();
		for (j = 0; j < bintris; j++)
		{
			int trindx = m_tribin[bin].at(j);
			if (m_tri[trindx] == v_before)
				m_tri[trindx] = v_after;
		}
	}
}

void vtTin::_CompareBins(int bin1, int bin2)
{
	int i, j;
	int ix1, ix2;
	int start;

	int size1 = m_vertbin[bin1].size();
	int size2 = m_vertbin[bin2].size();
	for (i = 0; i < size1; i++)
	{
		ix1 = m_vertbin[bin1].at(i);

		// within a bin, we can do N*N/2 instead of N*N compares
		// i.e. size1*size1/2, instead of size1*size2
		if (bin1 == bin2)
			start = i+1;
		else
			start = 0;

		for (j = start; j < size2; j++)
		{
			ix2 = m_vertbin[bin2].at(j);

			// don't compare against itself
			if (ix1 == ix2)
				continue;

			if (m_vert[ix1] == m_vert[ix2])
			{
				// ensure that one of them is flagged
				if (m_bReplace[ix1] == -1)
				{
					if (m_bReplace[ix2] == -1)
						m_bReplace[ix1] = ix2;
					else if (m_bReplace[ix2] != ix1)
						m_bReplace[ix1] = m_bReplace[ix2];
				}
				if (m_bReplace[ix2] == -1)
				{
					if (m_bReplace[ix1] == -1)
						m_bReplace[ix2] = ix1;
					else if (m_bReplace[ix1] != ix2)
						m_bReplace[ix2] = m_bReplace[ix1];
				}
			}
		}
	}
}

/**
 * Remove all the triangles of this TIN which intersect a given line segment.
 *
 * \param ep1, ep2 The endpoints of the line segment.
 * \return The number of triangles removed.
 */
int vtTin::RemoveTrianglesBySegment(const DPoint2 &ep1, const DPoint2 &ep2)
{
	int count = 0;

	uint tris = NumTris();
	for (uint i = 0; i < tris; i++)
	{
		// get 2D points
		const int v0 = m_tri[i*3];
		const int v1 = m_tri[i*3+1];
		const int v2 = m_tri[i*3+2];
		const DPoint2 &p1 = m_vert[v0];
		const DPoint2 &p2 = m_vert[v1];
		const DPoint2 &p3 = m_vert[v2];

		if (LineSegmentsIntersect(ep1, ep2, p1, p2) ||
			LineSegmentsIntersect(ep1, ep2, p2, p3) ||
			LineSegmentsIntersect(ep1, ep2, p3, p1))
		{
			m_tri.erase(m_tri.begin() + i*3, m_tri.begin() + i*3 + 3);
			i--;
			tris--;
			count++;
		}
	}
	if (count > 0)
	{
		RemoveUnusedVertices();
		ComputeExtents();
	}
	return count;
}

