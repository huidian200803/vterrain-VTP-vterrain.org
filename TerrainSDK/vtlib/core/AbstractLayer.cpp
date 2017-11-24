//
// AbstractLayer.cpp
//
// Copyright (c) 2006-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/vtosg/MultiTexture.h"
#include "TParams.h"
#include "GeomFactory.h"

#include "AbstractLayer.h"

#include "vtdata/DataPath.h"
#include "vtdata/Features.h"	// for vtFeatureSet
#include "vtdata/vtLog.h"

vtAbstractLayer::vtAbstractLayer() : vtLayer(LT_RAW)
{
	m_pSet = NULL;
	pContainer = NULL;
	pGeomGroup = NULL;
	pLabelGroup = NULL;
	pMultiTexture = NULL;
	m_pHeightField = NULL;

	m_Props.SetValueString("Type", TERR_LTYPE_ABSTRACT);

	m_bNeedRebuild = false;
}

vtAbstractLayer::~vtAbstractLayer()
{
	ReleaseGeometry();
	if (pContainer)
	{
		pContainer->getParent(0)->removeChild(pContainer);
		pContainer = NULL;
	}
	delete pMultiTexture;
	delete m_pSet;
}

bool vtAbstractLayer::Load(const vtCRS &crs, vtFeatureLoader *loader,
	bool progress_callback(int))
{
	// Load the features: use the loader we are provided, or the default
	vtFeatureSet *feat = NULL;
	vtString fname = m_Props.GetValueString("Filename");
	vtString path = FindFileOnPaths(vtGetDataPath(), fname);
	if (path == "")
	{
		// For historical reasons, also search a "PointData" folder on the data path
		vtString prefix = "PointData/";
		path = FindFileOnPaths(vtGetDataPath(), prefix+fname);
	}
	if (path == "")
	{
		// If it's not a file, perhaps it's a virtual data source
		if (loader)
			feat = loader->LoadFrom(fname);
		if (!feat)
		{
			VTLOG("Couldn't read features from '%s'\n", (const char *) fname);
			return false;
		}
	}
	if (!feat)
	{
		// Use the regular feature loader, for regular disk files.
		vtFeatureLoader loader;
		feat = loader.LoadFrom(path);
	}
	if (!feat)
	{
		VTLOG("Couldn't read features from file '%s'\n", (const char *) path);
		return false;
	}
	VTLOG("Successfully read features from file '%s'\n", (const char *) path);

	SetFeatureSet(feat);

	// We may need to convert from the CRS of the featureset to the CRS of the
	//  terrain (before converting from terrain to world coordinates)
	vtCRS &crs_feat = m_pSet->GetAtCRS();

	VTLOG("  features: CRS has root: %p\n", crs_feat.GetRoot());
	char type[7], value[4000];
	if (crs_feat.GetRoot())
	{
		crs_feat.GetTextDescription(type, value);
		VTLOG("   (%s: %s)\n", type, value);
	}
	VTLOG("   terrain: CRS has root: %p\n", crs.GetRoot());
	if (crs.GetRoot())
	{
		crs.GetTextDescription(type, value);
		VTLOG("   (%s: %s)\n", type, value);
	}

	// If we have two valid CRSs, and they are not the same, then we need a transform
	if (crs_feat.GetRoot() && crs.GetRoot())
	{
		VTLOG1("  Testing if they are the same: ");
		if (!crs_feat.IsSame(&crs))
		{
			VTLOG1("CRS is different, making a transform.\n");
			m_pOCTransform.reset(CreateCoordTransform(&crs_feat, &crs, true));
		}
		else
			VTLOG1("Yes. No transform needed.\n");
	}
	VTLOG1("vtAbstractLayer::Load finished\n");
	return true;
}

void vtAbstractLayer::SetLayerName(const vtString &fname)
{
	if (m_pSet)
		m_pSet->SetFilename(fname);

	// Keep properties in sync
	m_Props.SetValueString("Filename", fname);
}

vtString vtAbstractLayer::GetLayerName()
{
	if (m_pSet)
		return m_pSet->GetFilename();

	return m_Props.GetValueString("Filename");
}

void vtAbstractLayer::SetVisible(bool bVis)
{
	if (pContainer != NULL)
		pContainer->SetEnabled(bVis);

	if (pMultiTexture)
		pMultiTexture->Enable(bVis);

	vtLayerBase::SetVisible(bVis);
}

void vtAbstractLayer::SetFeatureSet(vtFeatureSet *pFeatureSet)
{
	m_pSet = pFeatureSet;

	// Handy pointers to disambiguate m_pSet
	m_pSetP2 = dynamic_cast<vtFeatureSetPoint2D*>(m_pSet);
	m_pSetP3 = dynamic_cast<vtFeatureSetPoint3D*>(m_pSet);
	m_pSetLS2 = dynamic_cast<vtFeatureSetLineString*>(m_pSet);
	m_pSetLS3 = dynamic_cast<vtFeatureSetLineString3D*>(m_pSet);
	m_pSetPoly = dynamic_cast<vtFeatureSetPolygon*>(m_pSet);
}


// Helper for the CreateFeature methods
bool GetColorField(const vtFeatureSet &feat, int iRecord, int iField, RGBAf &rgba)
{
	vtString str;
	float r, g, b;
	feat.GetValueAsString(iRecord, iField, str);
	if (sscanf((const char *)str, "%f %f %f", &r, &g, &b) != 3)
		return false;
	rgba.Set(r, g, b, 1);
	return true;
}

void vtAbstractLayer::CreateContainer(osg::Group *pParent)
{
	// first time
	pContainer = new vtGroup;
	pContainer->setName("Abstract Layer");

	pParent->addChild(pContainer);
}

bool vtAbstractLayer::EarthExtents(DRECT &ext)
{
	if (m_pSet)
		return m_pSet->EarthExtents(ext);
	return false;
}

/**
 * Given a featureset, create the geometry and place it
 * on the terrain.
 */
void vtAbstractLayer::CreateFeatureVisuals(osg::Group *pParent, vtHeightField3d *pHF,
	float fSpacing, bool progress_callback(int))
{
	VTLOG1("CreateFeatureVisuals\n");

	if (!pContainer)
		CreateContainer(pParent);

	SetHeightfield(pHF);
	m_fSpacing = fSpacing;

	RecreateFeatureVisuals(progress_callback);
}

void vtAbstractLayer::RecreateFeatureVisuals(bool progress_callback(int))
{
	uint entities = m_pSet->NumEntities();
	VTLOG("  Creating %d entities.. ", entities);

	for (uint i = 0; i < entities; i++)
	{
		CreateFeatureVisual(i);
		if (progress_callback != NULL)
			progress_callback(i * 100 / entities);
	}

	// A few types of visuals are not strictly per-feature; they must be
	//  created at once from all the features:
	//
	// 1. A line going through a point set.
	// 2. A TextureOverlay which rasterizes all the featues.

	if (m_Props.GetValueBool("LineGeometry") && m_pSetP3 != NULL)
		CreateLineGeometryForPoints();

	VTLOG1("Done.\n");
}

void vtAbstractLayer::CreateFeatureVisual(int iIndex)
{
	if (m_Props.GetValueBool("ObjectGeometry"))
		CreateObjectGeometry(iIndex);

	if (m_Props.GetValueBool("LineGeometry"))
		CreateLineGeometry(iIndex);

	if (m_Props.GetValueBool("Labels"))
		CreateFeatureLabel(iIndex);
}

void vtAbstractLayer::CreateGeomGroup()
{
	// create geometry group to contain all the meshes
	pGeomGroup = new vtGroup;
	pGeomGroup->setName("Geometry");
	pContainer->addChild(pGeomGroup);

	// Create materials.
	pGeomMats = new vtMaterialArray;

	// common colors
	RGBi color;

	color = m_Props.GetValueRGBi("ObjectGeomColor");
	material_index_object = pGeomMats->AddRGBMaterial(color, true, true);

	color = m_Props.GetValueRGBi("LineGeomColor");
	material_index_line = pGeomMats->AddRGBMaterial(color, false, false);

	// There is always a yellow highlight material
	material_index_yellow = pGeomMats->AddRGBMaterial(RGBf(1,1,0), false, false);

	pGeodeObject = new vtGeode;
	pGeodeObject->setName("Objects");
	pGeodeObject->SetMaterials(pGeomMats);
	pGeomGroup->addChild(pGeodeObject);

	pGeodeLine = new vtGeode;
	pGeodeLine->setName("Lines");
	pGeodeLine->SetMaterials(pGeomMats);
	pGeomGroup->addChild(pGeodeLine);
}

void vtAbstractLayer::CreateLabelGroup()
{
	pLabelGroup = new vtGroup;
	pLabelGroup->setName("Labels");
	pContainer->addChild(pLabelGroup);

	// If they specified a font name, use it
	vtString fontfile;
	if (!m_Props.GetValueString("Font", fontfile))
	{
		// otherwise, use the default
		fontfile = "Arial.ttf";
	}
	VTLOG("Attempting to load font from '%s'\n", (const char *)fontfile);

	// First, let the underlying scenegraph library try to find the font
	m_pFont = osgText::readFontFile((const char *)fontfile);
	if (m_pFont.valid())
	{
		VTLOG("Successfully read font from '%s'\n", (const char *)fontfile);
		return;
	}
	else
	{
		// look on VTP data paths
		vtString vtname = "Fonts/" + fontfile;
		vtString fontfile2 = FindFileOnPaths(vtGetDataPath(), vtname);
		if (fontfile2 != "")
		{
			m_pFont = osgText::readFontFile((const char *)fontfile2);
			if (m_pFont.valid())
			{
				VTLOG("Successfully read font from '%s'\n", (const char *)fontfile2);
				return;
			}
		}
	}
	VTLOG("Couldn't load font from '%s', not creating labels.\n", (const char *) fontfile);
}

int vtAbstractLayer::GetObjectMaterialIndex(vtTagArray &style, uint iIndex)
{
	int result;
	int color_field_index;
	if (style.GetValueInt("ObjectColorFieldIndex", color_field_index))
	{
		RGBAf rgba;
		if (GetColorField(*m_pSet, iIndex, color_field_index, rgba))
		{
			result = pGeomMats->FindByDiffuse(rgba);
			if (result == -1)
			{
				RGBf rgb = (RGBf) rgba;
				result = pGeomMats->AddRGBMaterial(rgb, true, true);
			}
		}
		else
			result = material_index_object;
	}
	else
		result = material_index_object;
	return result;
}


/**
	Given a featureset and style description, create a geometry object (such as
	spheres) and place it on the terrain.
	If 2D, they will be draped on the terrain.
*/
void vtAbstractLayer::CreateObjectGeometry(uint iIndex)
{
	if (!pGeomGroup)
		CreateGeomGroup();

	// for GetValueFloat below
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	// We support geometry for 2D and 3D points, and 2D and 3D polylines
	if (!m_pSetP2 && !m_pSetP3 && !m_pSetLS2 && !m_pSetLS3)
		return;

	// Determine color and material index
	int material_index = GetObjectMaterialIndex(m_Props, iIndex);

	// Determine geometry size and placement
	float fHeight = 0.0f;
	if (m_pSetP2 || m_pSetLS2)
		m_Props.GetValueFloat("ObjectGeomHeight", fHeight);

	float fRadius;
	if (!m_Props.GetValueFloat("ObjectGeomSize", fRadius))
		fRadius = 1;

	int res = 3;
	FPoint3 p3;

	// Track what is created
	vtVisual *viz = GetViz(m_pSet->GetFeature(iIndex));

	if (m_pSetP2)
	{
		const DPoint2 &epos = m_pSetP2->GetPoint(iIndex);
		m_pHeightField->ConvertEarthToSurfacePoint(epos, p3, 0, true);	// use true elev
		p3.y += fHeight;

		vtMesh *mesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_STRIP, VT_Normals, res*res*2);
		mesh->CreateEllipsoid(p3, FPoint3(fRadius, fRadius, fRadius), res);

		pGeodeObject->AddMesh(mesh, material_index);

		// Track
		if (viz) viz->m_meshes.push_back(mesh);
	}
	else if (m_pSetP3)
	{
		const DPoint3 &epos = m_pSetP3->GetPoint(iIndex);
		float original_z = (float) epos.z;
		m_pHeightField->m_LocalCS.EarthToLocal(epos, p3);

		// If a large number of entities, make as simple geometry as possible
		bool bTetrahedra = (m_pSet->NumEntities() > 10000);

		bool bShaded = true;
		vtMesh *mesh;
		if (bTetrahedra)
		{
			mesh = new vtMesh(osg::PrimitiveSet::TRIANGLES, bShaded ? VT_Normals : 0, 12);
			mesh->CreateTetrahedron(p3, fRadius);
			if (bShaded)
				mesh->SetNormalsFromPrimitives();
		}
		else
		{
			mesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_STRIP, bShaded ? VT_Normals : 0, res*res*2);
			mesh->CreateEllipsoid(p3, FPoint3(fRadius, fRadius, fRadius), res);
		}

		pGeodeObject->AddMesh(mesh, material_index);

		// Track
		if (viz) viz->m_meshes.push_back(mesh);
	}
	else if (m_pSetLS2)
	{
		const DLine2 &dline = m_pSetLS2->GetPolyLine(iIndex);
		for (uint j = 0; j < dline.GetSize(); j++)
		{
			// preserve 3D point's elevation: don't drape
			m_pHeightField->ConvertEarthToSurfacePoint(dline[j], p3);
			p3.y += fHeight;

			vtMesh *mesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_STRIP, VT_Normals, res*res*2);
			mesh->CreateEllipsoid(p3, FPoint3(fRadius, fRadius, fRadius), res);

			pGeodeObject->AddMesh(mesh, material_index);

			// Track
			if (viz) viz->m_meshes.push_back(mesh);
		}
	}
	else if (m_pSetLS3)
	{
		const DLine3 &dline = m_pSetLS3->GetPolyLine(iIndex);
		for (uint j = 0; j < dline.GetSize(); j++)
		{
			// preserve 3D point's elevation: don't drape
			m_pHeightField->m_LocalCS.EarthToLocal(dline[j], p3);

			vtMesh *mesh = new vtMesh(osg::PrimitiveSet::TRIANGLE_STRIP, VT_Normals, res*res*2);
			mesh->CreateEllipsoid(p3, FPoint3(fRadius, fRadius, fRadius), res);

			pGeodeObject->AddMesh(mesh, material_index);

			// Track
			if (viz) viz->m_meshes.push_back(mesh);
		}
	}
}

/**
	Given a featureset and style description, create line geometry.
	If 2D, it will be draped on the terrain. Polygon features
	(vtFeatureSetPolygon) will also be created as line geometry
	(unfilled polygons) and draped on the ground.
*/
void vtAbstractLayer::CreateLineGeometry(uint iIndex)
{
	// for GetValueFloat below
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	// We support geometry for 2D and 3D polylines, and 2D polygons
	if (!m_pSetLS2 && !m_pSetLS3 && !m_pSetPoly)
		return;

	// geometry group to contain all the meshes
	if (!pGeomGroup)
		CreateGeomGroup();

	// Determine color and material index
	int color_field_index;
	int material_index;
	if (m_Props.GetValueInt("LineColorFieldIndex", color_field_index))
	{
		RGBAf rgba;
		if (GetColorField(*m_pSet, iIndex, color_field_index, rgba))
		{
			material_index = pGeomMats->FindByDiffuse(rgba);
			if (material_index == -1)
			{
				RGBf rgb = (RGBf) rgba;
				material_index = pGeomMats->AddRGBMaterial(rgb, false, false, true);
			}
		}
		else
			material_index = material_index_line;
	}
	else
		material_index = material_index_line;

	// Estimate number of mesh vertices we'll have
	int iEstimatedVerts = 0;
	if (m_pSetLS2)
	{
		const DLine2 &dline = m_pSetLS2->GetPolyLine(iIndex);
		iEstimatedVerts = dline.GetSize();
	}
	else if (m_pSetLS3)
	{
		const DLine3 &dline = m_pSetLS3->GetPolyLine(iIndex);
		iEstimatedVerts = dline.GetSize();
	}
	else if (m_pSetPoly)
	{
		const DPolygon2 &dpoly = m_pSetPoly->GetPolygon(iIndex);
		for (uint k = 0; k < dpoly.size(); k++)
		{
			const DLine2 &dline = dpoly[k];
			iEstimatedVerts += dline.GetSize();
			iEstimatedVerts ++;		// close polygon
		}
	}

	vtGeomFactory mf(pGeodeLine, osg::PrimitiveSet::LINE_STRIP, 0, 3000, material_index,
		iEstimatedVerts);

	float fHeight = 0.0f;
	if (m_pSetLS2 || m_pSetPoly)
	{
		if (!m_Props.GetValueFloat("LineGeomHeight", fHeight))
			fHeight = 1.0f;
	}
	bool bTessellate = m_Props.GetValueBool("Tessellate");
	bool bCurve = false;

	FPoint3 f3;
	uint size;
	if (m_pSetLS2)
	{
		const DLine2 &dline = m_pSetLS2->GetPolyLine(iIndex);

		if (m_pOCTransform.get())
		{
			// Make a copy and transform it
			DLine2 copy = dline;
			TransformInPlace(m_pOCTransform.get(), copy);
			mf.AddSurfaceLineToMesh(m_pHeightField, copy, m_fSpacing, fHeight, bTessellate, bCurve, true);
		}
		else
			mf.AddSurfaceLineToMesh(m_pHeightField, dline, m_fSpacing, fHeight, bTessellate, bCurve, true);
	}
	else if (m_pSetLS3)
	{
		mf.PrimStart();
		const DLine3 &dline = m_pSetLS3->GetPolyLine(iIndex);
		size = dline.GetSize();
		for (uint j = 0; j < size; j++)
		{
			// preserve 3D point's elevation: don't drape
			DPoint3 p = dline[j];
			if (m_pOCTransform.get())
				m_pOCTransform->Transform(1, &p.x, &p.y);
			m_pHeightField->m_LocalCS.EarthToLocal(p, f3);
			mf.AddVertex(f3);
		}
		mf.PrimEnd();
	}
	else if (m_pSetPoly)
	{
		const DPolygon2 &dpoly = m_pSetPoly->GetPolygon(iIndex);
		for (uint k = 0; k < dpoly.size(); k++)
		{
			// This would be the efficient way
//				const DLine2 &dline = dpoly[k];

			// but we must copy each polyline in order to close it
			DLine2 dline = dpoly[k];
			dline.Append(dline[0]);

			if (m_pOCTransform.get())
				TransformInPlace(m_pOCTransform.get(), dline);

			mf.AddSurfaceLineToMesh(m_pHeightField, dline, m_fSpacing, fHeight, bTessellate, bCurve, true);
		}
	}

	// If the user specified a line width, apply it now
	bool bWidth = false;
	float fWidth;
	if (m_Props.GetValueFloat("LineWidth", fWidth) && fWidth != 1.0f)
		bWidth = true;

	// Track what was created
	vtVisual *viz = GetViz(m_pSet->GetFeature(iIndex));
	for (uint i = 0; i < mf.Meshes(); i++)
	{
		vtMesh *mesh = mf.Mesh(i);

		if (bWidth)
			mesh->SetLineWidth(fWidth);

		// Track
		if (viz) viz->m_meshes.push_back(mesh);
	}
}

/**
	Given a featureset and style description, create line geometry that
	goes through all the points.
*/
void vtAbstractLayer::CreateLineGeometryForPoints()
{
	// for GetValueFloat below
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	// We support geometry for 3D point sets (line through the points),
	if (!m_pSetP3)
		return;

	// geometry group to contain all the meshes
	if (!pGeomGroup)
		CreateGeomGroup();

	int material_index = material_index_line;

	// Estimate number of mesh vertices we'll have
	const DLine3 &dline = m_pSetP3->GetAllPoints();
	uint size = dline.GetSize();
	int iEstimatedVerts = size;

	vtGeomFactory mf(pGeodeLine, osg::PrimitiveSet::LINE_STRIP, 0, 30000, material_index,
		iEstimatedVerts);

	FPoint3 f3;
	mf.PrimStart();
	for (uint j = 0; j < size; j++)
	{
		// preserve 3D point's elevation: don't drape
		m_pHeightField->m_LocalCS.EarthToLocal(dline[j], f3);
		mf.AddVertex(f3);
	}
	mf.PrimEnd();

	// If the user specified a line width, apply it now
	float fWidth;
	if (m_Props.GetValueFloat("LineWidth", fWidth) && fWidth != 1.0f)
	{
		for (uint i = 0; i < mf.Meshes(); i++)
		{
			vtMesh *mesh = mf.Mesh(i);
			mesh->SetLineWidth(fWidth);
		}
	}
}

/**
 * Given a featureset and style description, create a labels and place it
 * on the terrain.
 *
 * If the features are 2D or 3D points (vtFeatureSetPoint2D or
 * vtFeatureSetPoint3D) then the labels will be placed at those points.  If
 * the features are 2D polygons (vtFeatureSetPolygon) then the point used is
 * the centroid of the polygon.
 */
void vtAbstractLayer::CreateFeatureLabel(uint iIndex)
{
	// We support text labels for 2D and 3D points, and 2D polygons
	if (!m_pSetP2 && !m_pSetP3 && !m_pSetPoly)
		return;

	// create group
	if (!pLabelGroup)
		CreateLabelGroup();

	// Must have a font to make a label
	if (!m_pFont.valid())
		return;

	// for GetValueFloat below
	ScopedLocale normal_numbers(LC_NUMERIC, "C");

	// Get the earth location of the label
	DPoint2 p2;
	DPoint3 p3;
	if (m_pSetP2)
		p2 = m_pSetP2->GetPoint(iIndex);
	else if (m_pSetP3)
	{
		p3 = m_pSetP3->GetPoint(iIndex);
		p2.Set(p3.x, p3.y);
	}
	else if (m_pSetPoly)
	{
		const DPolygon2 &dp = m_pSetPoly->GetPolygon(iIndex);
		p2 = dp[0].Centroid();
	}

	// Don't drape on culture, but do use true elevation
	FPoint3 fp3;
	if (!m_pHeightField->ConvertEarthToSurfacePoint(p2, fp3, 0, true))
		return;

	float label_elevation;
	if (!m_Props.GetValueFloat("LabelHeight", label_elevation))
		label_elevation = 0.0f;

	// Elevate the location by the desired vertical offset
	fp3.y += label_elevation;

	// If we have a 3D point, we can use the Z component of the point
	//  to further affect the elevation.
	if (m_pSetP3)
		fp3.y += p3.z;

	float label_size;
	if (!m_Props.GetValueFloat("LabelSize", label_size))
		label_size = 18;

	// Create the vtTextMesh
	vtTextMesh *text = new vtTextMesh(m_pFont, label_size, true);	// center

	// Get the label text
	int text_field_index;
	if (!m_Props.GetValueInt("TextFieldIndex", text_field_index))
		text_field_index = -1;
	vtString str;
	m_pSet->GetValueAsString(iIndex, text_field_index, str);

#if SUPPORT_WSTRING
	// Text will be UTF-8
	wstring2 wide_string;
	wide_string.from_utf8(str);
	text->SetText(wide_string);
#else
	// Hope that it isn't
	text->SetText(str);
#endif

	// Create the vtGeode object to contain the vtTextMesh
	vtGeode *geode = new vtGeode;
	geode->setName(str);

	// Determine feature color
	bool bGotColor = false;
	int color_field_index;
	if (m_Props.GetValueInt("TextColorFieldIndex", color_field_index))
	{
		RGBAf rgba;
		if (GetColorField(*m_pSet, iIndex, color_field_index, rgba))
		{
			text->SetColor(rgba);
			bGotColor = true;
		}
	}
	if (!bGotColor)
	{
		RGBf rgb = m_Props.GetValueRGBi("LabelColor");
		text->SetColor(rgb);
	}

	bool bOutline = m_Props.GetValueBool("LabelOutline");

	// Labels will automatically turn to face the user because that's vtlib's
	// default behavior now.
	geode->AddTextMesh(text, -1, bOutline);

	// Transform to position it, add to the label group.
	vtTransform *bb = new vtTransform;
	bb->addChild(geode);
	bb->SetTrans(fp3);
	pLabelGroup->addChild(bb);

	// Track what was created
	vtVisual *viz = GetViz(m_pSet->GetFeature(iIndex));
	if (viz) viz->m_xform = bb;
}

/**
 * Release all the 3D stuff created for the layer (including geometry and labels).
 */
void vtAbstractLayer::ReleaseGeometry()
{
	// Safety check
	if (!m_pSet)
		return;

	for (int i = m_pSet->NumEntities()-1; i >= 0; i--)
	{
		vtFeature *f = m_pSet->GetFeature(i);
		ReleaseFeatureGeometry(f);
	}
	if (pGeomGroup)
	{
		pContainer->removeChild(pGeomGroup);
		pGeomGroup = NULL;
	}
	if (pLabelGroup)
	{
		pContainer->removeChild(pLabelGroup);
		pLabelGroup = NULL;
	}
}

/**
 * Release all the 3D stuff created for a given feature.
 */
void vtAbstractLayer::ReleaseFeatureGeometry(vtFeature *f)
{
	vtVisual *v = GetViz(f);

	for (uint m = 0; m < v->m_meshes.size(); m++)
	{
		vtMesh *mesh = v->m_meshes[m];

		pGeodeObject->RemoveMesh(mesh);
		pGeodeLine->RemoveMesh(mesh);
	}
	if (v->m_xform)
		pLabelGroup->removeChild(v->m_xform);
	delete v;
	m_Map.erase(f);
}

void vtAbstractLayer::DeleteFeature(vtFeature *f)
{
	// Check if we need to rebuild the whole thing
	if (CreateAtOnce())
		m_bNeedRebuild = true;
	else
		ReleaseFeatureGeometry(f);
}

void vtAbstractLayer::RefreshFeatureVisuals(bool progress_callback(int))
{
	ReleaseGeometry();
	RecreateFeatureVisuals(progress_callback);
}

// When the underlying feature changes, we need to rebuild the visual
void vtAbstractLayer::RefreshFeature(uint iIndex)
{
	// If we're not doing a full rebuild, we can create individual items
	if (!m_bNeedRebuild)
	{
		vtFeature *f = m_pSet->GetFeature(iIndex);
		ReleaseFeatureGeometry(f);
		CreateFeatureVisual(iIndex);
	}
}

void vtAbstractLayer::UpdateVisualSelection()
{
	// use SetMeshMatIndex to make the meshes of selected features yellow
	for (uint j = 0; j < m_pSet->NumEntities(); j++)
	{
		vtFeature *feat = m_pSet->GetFeature(j);
		vtVisual *viz = GetViz(feat);
		if (viz)
		{
			int material_index;
			if (m_pSet->IsSelected(j))
				material_index = material_index_yellow;
			else
				material_index = GetObjectMaterialIndex(m_Props, j);

			for (uint k = 0; k < viz->m_meshes.size(); k++)
			{
				vtMesh *mesh = viz->m_meshes[k];
				pGeodeObject->SetMeshMatIndex(mesh, material_index);
			}
		}
	}
}

// When the feature set changes (on disk), we can reload it and rebuild the visual
void vtAbstractLayer::Reload()
{
	// We must release the geometry before changing the featureset
	ReleaseGeometry();

	vtString fname = m_pSet->GetFilename();

	// Now we can remove the previous featureset
	delete m_pSet;

	vtFeatureLoader loader;
	vtFeatureSet *newset = loader.LoadFrom(fname);
	if (!newset)
	{
		VTLOG("Couldn't read features from file '%s'\n", (const char *)fname);
		return;
	}
	VTLOG("Successfully read features from file '%s'\n", (const char *)fname);
	SetFeatureSet(newset);

	RefreshFeatureVisuals();
}

// To make sure all edits are fully reflected in the visual, call these
//  methods around any editing of style or geometry.
void vtAbstractLayer::EditBegin()
{
}

void vtAbstractLayer::EditEnd()
{
	if (m_bNeedRebuild)
	{
		m_bNeedRebuild = false;
		RefreshFeatureVisuals();
	}
}

vtVisual *vtAbstractLayer::GetViz(vtFeature *feat)
{
#if 0
	return NULL;
#else
	vtVisual *v = m_Map[feat];
	if (!v)
	{
		v = new vtVisual;
		m_Map[feat] = v;
	}
	return v;
#endif
}

// A few types of visuals are not strictly per-feature; they must be
//  created at once from all the features.
bool vtAbstractLayer::CreateAtOnce()
{
	if (m_Props.GetValueBool("LineGeometry") && m_pSetP3 != NULL)
		return true;
	return false;
}

