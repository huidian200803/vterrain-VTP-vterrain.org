//
// DxfParser.cpp
//
// Class for parsing a DXF File.
//
// Copyright (c) 2004-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "DxfParser.h"
#include "vtLog.h"
#include "FilePath.h"

using std::vector;
using std::set;

/**
 * Constructor.  Pass in the filename to parse.
 */
DxfParser::DxfParser(const vtString &sFileName,
					   std::vector<DxfEntity> &entities,
					   std::vector<DxfLayer> &layers) :
	m_entities(entities),
	m_layers(layers)
{
	m_sFileName = sFileName;
}

/**
 * This method will parse the entire file, collecting all entities
 *  that it finds.
 *
 * \param progress_callback	Pass a function to receive progress notification
 *		(values of 0 to 100) if desired.  Optional.
 *
 * \returns Success.  If failure, then call GetLastError() to get an
 *		informative error message.
 */
bool DxfParser::RetrieveEntities(bool progress_callback(int))
{
	try
	{
		DxfCodeValue pair;

		if (m_sFileName.IsEmpty())
			return false;

		m_iLine = 0;
		m_pFile = vtFileOpen(m_sFileName, "rb");
		if (m_pFile == NULL)
		{
			return false;
		}
		m_iCounter = 0;
		if (fseek(m_pFile, 0, SEEK_END) != 0)
		{
			fclose(m_pFile);
			return false;
		}
		m_iEndPosition = ftell(m_pFile);
		if (m_iEndPosition < 1)
		{
			fclose(m_pFile);
			return false;
		}
		rewind(m_pFile);

		// Process each section
		while (ReadCodeValue(pair))
		{
			if (pair.m_iCode == 0 && pair.m_sValue == "SECTION")
			{
				if (!ReadCodeValue(pair))
					throw "Unexpected end of file found.";

				if (pair.m_iCode != 2)
					throw "Expecting section type, but none encountered.";

				if (pair.m_sValue == "TABLES")
					ReadTableSection(progress_callback);
				else if (pair.m_sValue == "ENTITIES")
					ReadEntitySection(progress_callback);
				else
					SkipSection();

			}
			else if (pair.m_iCode == 0 && pair.m_sValue == "EOF")
			{
				break;
			}
			else
			{
				throw "Expecting section, but none encountered.";
			}
		}
	}
	catch (const char *msg)
	{
		m_strMessage.Format("DXF Parse Error: %s\nLine = %d\n", msg, m_iLine);

		fclose(m_pFile);
		return false;
	}
	catch (...)
	{
		m_strMessage.Format("DXF Parse Error: Unknown parse error.\nLine = %d\n", m_iLine);

		fclose(m_pFile);
		return false;
	}

	fclose(m_pFile);
	return true;
}

inline bool DxfParser::ReadCodeValue(DxfCodeValue & pair)
{
	// Read the code.
	char buf[1024];
	if (fgets(buf, 1024, m_pFile) == NULL)
		return false;
	vtString sVal = buf;
	sVal.TrimRight();
	sVal.TrimLeft();
	if (!sVal.IsNumber())
		throw "Non-numeric code encountered.";
	pair.m_iCode = atoi(sVal);

	// Read the value.
	if (fgets(buf, 1024, m_pFile) == NULL)
		throw "Unexpected end of line encountered.";
	sVal = buf;
	sVal.TrimRight();
	sVal.TrimLeft();
	pair.m_sValue = sVal;

	m_iLine += 2;

	return true;
}

void DxfParser::SkipSection()
{
	DxfCodeValue pair;
	bool bFoundEnd = false;
	while (ReadCodeValue(pair))
	{
		if (pair.m_iCode == 0 && pair.m_sValue == "ENDSEC")
		{
			bFoundEnd = true;
			break;
		}
	}
	if (!bFoundEnd)
		throw "Unable to find end of section.";
}

void DxfParser::ReadTableSection(bool progress_callback(int))
{
	DxfCodeValue pair;
	bool bFoundEnd = false;
	while (ReadCodeValue(pair))
	{
		if (pair.m_iCode == 0 && pair.m_sValue == "ENDSEC")
		{
			bFoundEnd = true;
			break;
		}
		if (pair.m_iCode == 0 && pair.m_sValue == "LAYER")
		{
			ReadLayer();
		}
	}
}

void DxfParser::ReadEntitySection(bool progress_callback(int))
{
	DxfCodeValue pair;
	bool bFoundEnd = false;
	while (ReadCodeValue(pair))
	{
		if (pair.m_iCode == 0)
		{
			if (pair.m_sValue == "ENDSEC")
			{
				bFoundEnd = true;
				break;
			}
			else if (pair.m_sValue == "POLYLINE")
				ReadPolyline();
			else if (pair.m_sValue == "POINT")
				ReadPoint();
			else if (pair.m_sValue == "LINE")
				ReadLine();
			else if (pair.m_sValue == "LWPOLYLINE")
				ReadLWPolyline();
			else if (pair.m_sValue == "3DFACE")
				Read3DFace();
			else if (pair.m_sValue == "TEXT")
				ReadText();
			else
				VTLOG("Unknown ent type: %s\n", (const char *) pair.m_sValue);
		}

		m_iCounter++;
		if (m_iCounter == 200)
		{
			long current = ftell(m_pFile);
			int iProgress =  (int) ((double)current / m_iEndPosition * 100);
			if (progress_callback(iProgress))
			{
				// user cancelled
				throw "Cancelled by user.";
			}
			m_iCounter = 0;
		}
	}
	if (!bFoundEnd)
		throw "Unable to find end of section.";
}

void DxfParser::ReadLayer()
{
	DxfCodeValue pair;

	DxfLayer layer;
	long oldPos = ftell(m_pFile);

	while (ReadCodeValue(pair))
	{
		if (pair.m_iCode == 0)
		{
			// save the layer.
			m_layers.push_back(layer);

			// back that pointer up.
			fseek(m_pFile, oldPos, SEEK_SET);
			break;
		}
		else if (pair.m_iCode == 2)		// name
		{
			layer.m_name = pair.m_sValue;
		}
		else if (pair.m_iCode == 62)	// color
		{
			//ACAD Color Name
			//1 RED
			//2 YELLOW
			//3 GREEN
			//4 CYAN
			//5 BLUE
			//6 MAGENTA
			//7 BLACK/WHITE
			//21 = another cyan? 131 = light red?
			switch (atoi(pair.m_sValue))
			{
			case 1: layer.m_color = RGBi(255,0,0); break;
			case 2: layer.m_color = RGBi(255,255,0); break;
			case 3: layer.m_color = RGBi(0,255,0); break;
			case 4: layer.m_color = RGBi(0,255,255); break;
			case 5: layer.m_color = RGBi(0,0,255); break;
			case 6: layer.m_color = RGBi(255,0,255); break;
			case 7: layer.m_color = RGBi(255,255,255); break;
			default:layer.m_color = RGBi(128,128,128); break;	// grey
			}
		}
		oldPos = ftell(m_pFile);
	}
}

void DxfParser::ReadPoint()
{
	DxfCodeValue pair;
	DxfEntity entity;

	bool bFoundEnd = false, bFoundLayer = false, bFoundX = false;
	bool bFoundY = false, bFoundZ = false;
	DPoint3 pt;

	long oldPos = ftell(m_pFile);

	while (ReadCodeValue(pair))
	{
		if (pair.m_iCode == 0)
		{
			bFoundEnd = true;

			// save the entity off if it has everything.
			if (bFoundLayer && bFoundX && bFoundY)
			{
				if (!bFoundZ)
					pt.z = 0.0;
				entity.m_points.push_back(pt);
				entity.m_iType = DET_Point;
				m_entities.push_back(entity);
			}

			// back that pointer up.
			// don't worry, we won't be here if current line is less than 4
			fseek(m_pFile, oldPos, SEEK_SET);
			break;
		}
		else if (pair.m_iCode == 8)
		{
			entity.m_iLayer = GetLayerIndex(pair.m_sValue);
			bFoundLayer = true;
		}
		else if (pair.m_iCode == 10)
		{
			pt.x = strtod(pair.m_sValue, NULL);
			bFoundX = true;
		}
		else if (pair.m_iCode == 20)
		{
			pt.y = strtod(pair.m_sValue, NULL);
			bFoundY = true;
		}
		else if (pair.m_iCode == 30)
		{
			pt.z = strtod(pair.m_sValue, NULL);
			bFoundZ = true;
		}
		oldPos = ftell(m_pFile);
	}
	if (!bFoundEnd)
		throw "Unable to find end of point entity.";
}

void DxfParser::ReadText()
{
	// A "TEXT" entity in DXF is basically a label with a point location,
	//  treat is as a point entity with label field.
	DxfCodeValue pair;
	DxfEntity entity;

	bool bFoundEnd = false, bFoundLayer = false, bFoundX = false;
	bool bFoundY = false, bFoundZ = false;
	DPoint3 pt;

	long oldPos = ftell(m_pFile);

	while (ReadCodeValue(pair))
	{
		if (pair.m_iCode == 0)
		{
			bFoundEnd = true;

			// save the entity off if it has everything.
			if (bFoundLayer && bFoundX && bFoundY)
			{
				if (!bFoundZ)
					pt.z = 0.0;
				entity.m_points.push_back(pt);
				entity.m_iType = DET_Point;
				m_entities.push_back(entity);
			}

			// back that pointer up.
			// don't worry, we won't be here if current line is less than 4
			fseek(m_pFile, oldPos, SEEK_SET);
			break;
		}
		else if (pair.m_iCode == 8)
		{
			entity.m_iLayer = GetLayerIndex(pair.m_sValue);
			bFoundLayer = true;
		}
		else if (pair.m_iCode == 10)
		{
			pt.x = strtod(pair.m_sValue, NULL);
			bFoundX = true;
		}
		else if (pair.m_iCode == 20)
		{
			pt.y = strtod(pair.m_sValue, NULL);
			bFoundY = true;
		}
		else if (pair.m_iCode == 30)
		{
			pt.z = strtod(pair.m_sValue, NULL);
			bFoundZ = true;
		}
		else if (pair.m_iCode == 1)
		{
			entity.m_label = pair.m_sValue;
		}
		oldPos = ftell(m_pFile);
	}
	if (!bFoundEnd)
		throw "Unable to find end of point entity.";
}

void DxfParser::ReadLine()
{
	DxfCodeValue pair;
	DxfEntity entity;
	entity.m_iType = DET_Polyline;
	DPoint3 startPt, endPt;
	bool bFoundEnd = false;
	bool bFoundLayer = false;
	bool bFoundStartX = false;
	bool bFoundStartY = false;
	bool bFoundStartZ = false;
	bool bFoundEndX = false;
	bool bFoundEndY = false;
	bool bFoundEndZ = false;

	long oldPos = ftell(m_pFile);

	while (ReadCodeValue(pair))
	{
		if (pair.m_iCode == 0)
		{
			bFoundEnd = true;

			// back that pointer up.
			// don't worry, we won't be here if current line is less than 4
			fseek(m_pFile, oldPos, SEEK_SET);
			entity.m_points.push_back(endPt);
			entity.m_points.push_back(startPt);

			// save the entity off if it has everything.
			if (bFoundLayer && bFoundStartX && bFoundStartY && bFoundStartZ &&
				bFoundEndX && bFoundEndY && bFoundEndZ && entity.m_points.size() > 0)
			{
				m_entities.push_back(entity);
			}
			break;
		}
		else if (pair.m_iCode == 8)
		{
			entity.m_iLayer = GetLayerIndex(pair.m_sValue);
			bFoundLayer = true;
		}
		else if (pair.m_iCode == 10)
		{
			startPt.x = strtod(pair.m_sValue, NULL);
			bFoundStartX = true;
		}
		else if (pair.m_iCode == 20)
		{
			startPt.y = strtod(pair.m_sValue, NULL);
			bFoundStartY = true;
		}
		else if (pair.m_iCode == 30)
		{
			startPt.z = strtod(pair.m_sValue, NULL);
			bFoundStartZ = true;
		}
		else if (pair.m_iCode == 11)
		{
			endPt.x = strtod(pair.m_sValue, NULL);
			bFoundEndX = true;
		}
		else if (pair.m_iCode == 21)
		{
			endPt.y = strtod(pair.m_sValue, NULL);
			bFoundEndY = true;
		}
		else if (pair.m_iCode == 31)
		{
			endPt.z = strtod(pair.m_sValue, NULL);
			bFoundEndZ = true;
		}
		oldPos = ftell(m_pFile);
	}
	if (!bFoundEnd)
		throw "Unable to find end of polyline entity.";
}

void DxfParser::ReadPolyline()
{
	DxfCodeValue pair;
	DxfEntity entity;
	bool bFoundEnd = false;
	bool bFoundLayer = false;

	// Default type is polyline, unless a code 70 says it's actually a polygon
	entity.m_iType = DET_Polyline;

	while (ReadCodeValue(pair))
	{
		if (pair.m_iCode == 0 && pair.m_sValue == "SEQEND")
		{
			bFoundEnd = true;

			// save the entity off if it has everything.
			if (bFoundLayer && entity.m_points.size() > 0)
			{
				m_entities.push_back(entity);
			}
			break;
		}
		else if (pair.m_iCode == 8)
		{
			entity.m_iLayer = GetLayerIndex(pair.m_sValue);
			bFoundLayer = true;
		}
		else if (pair.m_iCode == 70)
		{
			if (atoi(pair.m_sValue) & 1)
				entity.m_iType = DET_Polygon;
		}
		else if (pair.m_iCode == 0 && pair.m_sValue == "VERTEX")
		{
			ReadVertex(entity.m_points);
		}
	}
	if (!bFoundEnd)
		throw "Unable to find end of polyline entity.";
}

void DxfParser::ReadVertex(std::vector<DPoint3> & points)
{
	DxfCodeValue pair;

	bool bFoundEnd = false, bFoundX = false;
	bool bFoundY = false, bFoundZ = false;
	DPoint3 pt;

	long oldPos = ftell(m_pFile);
	while (ReadCodeValue(pair))
	{
		if (pair.m_iCode == 0)
		{
			bFoundEnd = true;

			// save the entity off if it has everything.
			if (bFoundX && bFoundY)
			{
				if (!bFoundZ)
					pt.z = 0.0;
				points.push_back(pt);
			}

			// back that pointer up.
			// don't worry, we won't be here if current line is less than 4
			fseek(m_pFile, oldPos, SEEK_SET);
			break;
		}
		else if (pair.m_iCode == 10)
		{
			pt.x = strtod(pair.m_sValue, NULL);
			bFoundX = true;
		}
		else if (pair.m_iCode == 20)
		{
			pt.y = strtod(pair.m_sValue, NULL);
			bFoundY = true;
		}
		else if (pair.m_iCode == 30)
		{
			pt.z = strtod(pair.m_sValue, NULL);
			bFoundZ = true;
		}
		oldPos = ftell(m_pFile);
	}
	if (!bFoundEnd)
		throw "Unable to find end of vertex entity.";
}

int DxfParser::GetLayerIndex(const vtString &sLayer)
{
	uint iLayers = m_layers.size();
	for (uint i = 0; i < iLayers; ++i)
	{
		if (sLayer == m_layers[i].m_name)
			return i;
	}
	// not found, add it
	DxfLayer lay;
	lay.m_name = sLayer;
	lay.m_color.Set(240, 200, 220);		// mauve
	m_layers.push_back(lay);
	return m_layers.size()-1;
}


void DxfParser::ReadLWPolyline()
{
	long oldPos = ftell(m_pFile);

	// we assume that the x,y values come in that order (i.e. 10 20 10 20 etc.)
	DxfCodeValue pair;
	DxfEntity entity;
	double dElevation = 0.0;
	bool bFoundElevation = false;
	bool bFoundEnd = false;
	bool bFoundLayer = false;
	bool bFoundType = false;
	int iCurrIndex = 0;

	while (ReadCodeValue(pair))
	{
		if (pair.m_iCode == 0)
		{
			bFoundEnd = true;

			// save the entity off if it has everything.
			if (bFoundElevation && bFoundLayer && bFoundType && entity.m_points.size() > 0)
			{
				for (int i = 0; i < iCurrIndex; ++i)
					entity.m_points[i].z = dElevation;
				m_entities.push_back(entity);
			}

			// back that pointer up.
			// don't worry, we won't be here if current line is less than 4
			fseek(m_pFile, oldPos, SEEK_SET);
			break;
		}
		else if (pair.m_iCode == 8)
		{
			entity.m_iLayer = GetLayerIndex(pair.m_sValue);
			bFoundLayer = true;
		}
		else if (pair.m_iCode == 10)
		{
			DPoint3 pt;
			pt.x = strtod(pair.m_sValue, NULL);
			entity.m_points.push_back(pt);
		}
		else if (pair.m_iCode == 20)
		{
			entity.m_points[iCurrIndex].y = strtod(pair.m_sValue, NULL);
			++iCurrIndex;
		}
		else if (pair.m_iCode == 38)
		{
			dElevation = strtod(pair.m_sValue, NULL);
			bFoundElevation = true;
		}
		else if (pair.m_iCode == 70)
		{
			if (atoi(pair.m_sValue) & 1)
				entity.m_iType = DET_Polygon;
			else
				entity.m_iType = DET_Polyline;
			bFoundType = true;
		}
		oldPos = ftell(m_pFile);
	}
	if (!bFoundEnd)
		throw "Unable to find end of lwpolyline entity.";
}

void DxfParser::Read3DFace()
{
	DxfCodeValue pair;
	DxfEntity entity;
	bool bFoundEnd = false;
	bool bFoundLayer = false;

	entity.m_iType = DET_3DFace;

	// There are always four points in a 3DFACE...
	DPoint3 pt1, pt2, pt3, pt4;

	long oldPos = ftell(m_pFile);
	while (ReadCodeValue(pair))
	{
		if (pair.m_iCode == 0)
		{
			bFoundEnd = true;

			// back that pointer up.
			// don't worry, we won't be here if current line is less than 4
			fseek(m_pFile, oldPos, SEEK_SET);

			// save the entity off if it has everything.
			break;
		}
		else if (pair.m_iCode == 8)
		{
			entity.m_iLayer = GetLayerIndex(pair.m_sValue);
			bFoundLayer = true;
		}

		else if (pair.m_iCode == 10)
			pt1.x = strtod(pair.m_sValue, NULL);
		else if (pair.m_iCode == 20)
			pt1.y = strtod(pair.m_sValue, NULL);
		else if (pair.m_iCode == 30)
			pt1.z = strtod(pair.m_sValue, NULL);

		else if (pair.m_iCode == 11)
			pt2.x = strtod(pair.m_sValue, NULL);
		else if (pair.m_iCode == 21)
			pt2.y = strtod(pair.m_sValue, NULL);
		else if (pair.m_iCode == 31)
			pt2.z = strtod(pair.m_sValue, NULL);

		else if (pair.m_iCode == 12)
			pt3.x = strtod(pair.m_sValue, NULL);
		else if (pair.m_iCode == 22)
			pt3.y = strtod(pair.m_sValue, NULL);
		else if (pair.m_iCode == 32)
			pt3.z = strtod(pair.m_sValue, NULL);

		else if (pair.m_iCode == 13)
			pt4.x = strtod(pair.m_sValue, NULL);
		else if (pair.m_iCode == 23)
			pt4.y = strtod(pair.m_sValue, NULL);
		else if (pair.m_iCode == 33)
			pt4.z = strtod(pair.m_sValue, NULL);
		else if (pair.m_iCode == 70)
		{
			// Invisible edge flags.  Ignore these?
			//  Bit 1,2,3,4 means 1st,2nd,3rd,4th edge is invisible
			// int mask = atoi(pair.m_sValue);
		}
		oldPos = ftell(m_pFile);
	}

	if (!bFoundEnd)
		throw "Unable to find end of polyline entity.";

	// ...although the forth point may be the same as the third,
	//  in which case ignore it and treat this as a triangle.
	entity.m_points.push_back(pt1);
	entity.m_points.push_back(pt2);
	entity.m_points.push_back(pt3);
	if (pt4 != pt3)
		entity.m_points.push_back(pt4);

	if (bFoundLayer)
		m_entities.push_back(entity);
}
