//
// AbstractLayer.h
//
// Copyright (c) 2006-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ABSTRACTLAYERH
#define ABSTRACTLAYERH

#include "TerrainLayers.h"
#include "vtdata/Features.h"

#include <memory>	// for auto_ptr

class vtTerrain;

class vtVisual
{
public:
	vtVisual() : m_xform(NULL) {}
	std::vector<vtMesh*> m_meshes;
	vtTransform *m_xform;
};

typedef std::map<vtFeature*,vtVisual*> VizMap;

/**
 * An abstract layer is a traditional GIS-style set of geometry features with
 * attributes.  It can be shown on the terrain in a variety of ways (styles).
 * Style is described with a set of properties, encoded as a vtTagArray.
 * These are the supported style properties, all of which are optional:
	- "Filename": If the layer should be loaded from disk, this is the filename
		to load from.  It can be a relative path anywhere on the VTP data paths,
		or an absolute path.

	- "ObjectGeometry": true to show geometry for each point of the features.
	 - "ObjectGeomColor": The color of each geometry (R,G,B as float 0..1).
		Default is white.
	 - "ObjectColorFieldIndex": The 0-based index of the field which contains the
		color of each geometry.
	 - "ObjectGeomHeight": The height in meters of each geometry above the ground.
	 - "ObjectGeomSize": The size in meters of each geometry.

	- "LineGeometry": true to show lines running though each point of the features.
	 - "LineGeomColor": The color of each line (R,G,B as float 0..1).
		Default is white.
	 - "LineColorFieldIndex": The 0-based index of the field which contains the
		color of each line.
	 - "LineGeomHeight": The height in meters of each line above the ground.
	 - "LineWidth": Pixel width of the 3D lines to create.  Default it 1.
	 - "Tessellate": true to tesslate the geometry of each feature before draping
		it on the ground.  This can produce a smoother result.

	- "Labels": true to show floating text labels for the features.
	 - "LabelColor": The color of each label (R,G,B as float 0..1)
		Default is white.
	 - "TextColorFieldIndex": The 0-based index of the field which contains the
		color of each label.
	 - "TextFieldIndex": The 0-based index of the field which contains the text you
		want to use for the label.  For example, if you have several fields called
		"ID", "Name", and "Address", and you want to show "Name", use the value 1.
	 - "LabelHeight": Height in meters above the ground.  This is the distance
		from the ground surface to the lower edge of the text label.
		Default is 0.
	 - "LabelSize": Size (vertical extent) of the text labels in meters.
		Default is 18.
	 - "Font": Filename (may include path) of the font to use for text labels.
		Example: "Arial.ttf"
	 - "LabelOutline": true to put a dark outline around the font to improve its
		readability against most backgrounds.

 When a terrain description (TParams) contains an abstract layer, these same
 style properties are encoded.  On disk, they are stored as XML elements.
 */
class vtAbstractLayer : public vtLayer
{
public:
	vtAbstractLayer();
	~vtAbstractLayer();

	bool Load(const vtCRS &crs, vtFeatureLoader *loader = NULL,
		bool progress_callback(int) = NULL);
	void SetLayerName(const vtString &fname);
	vtString GetLayerName();
	void SetVisible(bool vis);

	void SetHeightfield(vtHeightField3d *pHF) { m_pHeightField = pHF; }
	void SetFeatureSet(vtFeatureSet *pSet);
	vtFeatureSet *GetFeatureSet() const { return m_pSet; }
	vtGroup *GetLabelGroup() const { return pLabelGroup; }
	vtGroup *GetContainer() const { return pContainer.get(); }
	vtVisual *GetViz(vtFeature *feat);
	vtMultiTexture *GetMultiTexture() const { return pMultiTexture; }
	void CreateContainer(osg::Group *pParent);
	bool EarthExtents(DRECT &ext);

	// Create for all features
	void CreateFeatureVisuals(osg::Group *pParent, vtHeightField3d *pHF,
		float fSpacing, bool progress_callback(int) = NULL);
	void RecreateFeatureVisuals(bool progress_callback(int) = NULL);
	void CreateLineGeometryForPoints();

	// Create for a single feature
	void CreateFeatureVisual(int iIndex);
	void CreateObjectGeometry(uint iIndex);
	void CreateLineGeometry(uint iIndex);
	void CreateFeatureLabel(uint iIndex);

	void ReleaseGeometry();
	void ReleaseFeatureGeometry(vtFeature *f);

	// When the underlying feature changes, we need to rebuild the visual
	void RefreshFeatureVisuals(bool progress_callback(int) = NULL);
	void RefreshFeature(uint iIndex);
	void UpdateVisualSelection();
	void Reload();

	// To make sure all edits are fully reflected in the visual, call these
	//  methods around any editing of style or geometry.
	void EditBegin();
	void EditEnd();
	void DeleteFeature(vtFeature *f);

protected:
	void CreateGeomGroup();
	void CreateLabelGroup();
	int GetObjectMaterialIndex(vtTagArray &style, uint iIndex);

	/// This is the set of features which the layer contains.
	vtFeatureSet *m_pSet;
	vtGroupPtr pContainer;
	vtGroup *pGeomGroup;
	vtGroup *pLabelGroup;
	vtMultiTexture *pMultiTexture;

	// Handy pointers to disambiguate pSet
	vtFeatureSetPoint2D *m_pSetP2;
	vtFeatureSetPoint3D *m_pSetP3;
	vtFeatureSetLineString   *m_pSetLS2;
	vtFeatureSetLineString3D *m_pSetLS3;
	vtFeatureSetPolygon *m_pSetPoly;

	// For draped features, the heightfield to drape on.
	vtHeightField3d *m_pHeightField;
	float m_fSpacing;		// The horizontal spacing for draping vectors.

	// Used to create the visual features
	vtFontPtr m_pFont;
	vtMaterialArrayPtr pGeomMats;

	int material_index_object;
	int material_index_line;
	int material_index_yellow;
	vtGeode *pGeodeObject;
	vtGeode *pGeodeLine;

	VizMap m_Map;

	// A transform from the CRS of the featureset to the CRS of the scene they are shown in.
	std::auto_ptr<OCTransform> m_pOCTransform;

	// Edit tracking
	bool CreateAtOnce();
	bool m_bNeedRebuild;
};

#endif // ABSTRACTLAYERH

