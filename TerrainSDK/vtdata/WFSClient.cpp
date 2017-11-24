//
// WFSClient.cpp
//
// Web Feature Server Client
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "config_vtdata.h"
#include "Features.h"

// The dependency on curl for http is optional.  If not desired, skip this file.
#if SUPPORT_CURL

#include "FilePath.h"
#include "vtLog.h"
#include "TripDub.h"
#include "WFSClient.h"
#include "xmlhelper/easyxml.hpp"


/////////////////////////////////////////////////////////////////////////////
// vtFeatures methods
//

vtFeatureSet *vtFeatureLoader::ReadFeaturesFromWFS(const char *szServerURL, const char *layername)
{
	vtString url = szServerURL;
	url += "GetFeature?typeName=";
	url += layername;

	vtString str;
	ReqContext cl;
	cl.GetURL(url, str);
	if (str == "")
		return NULL;

	const char *temp_fname = "C:/temp/gml_temp.gml";
	FILE *fp = vtFileOpen(temp_fname, "wb");
	if (!fp)
		return NULL;

	fwrite((const char *)str, 1, str.GetLength(), fp);
	fclose(fp);

	return LoadWithOGR(temp_fname);
}


////////////////////////////////////////////////////////////////////////
// Visitor class, for XML parsing of WFS Layer List files.
//

class LayerListVisitor : public XMLVisitor
{
public:
	LayerListVisitor(OGCLayerArray &pLayers) : m_Layers(pLayers)
	{
	}

	virtual ~LayerListVisitor () {}

	void startXML() { _level = 0; m_pCurrent = NULL; }
	void endXML() { _level = 0; }
	void startElement (const char * name, const XMLAttributes &atts);
	void endElement (const char * name);
	void data (const char * s, int length);

private:
	string _data;
	int _level;

	OGCLayerArray &m_Layers;
	OGCLayerArray m_Stack;
	vtTagArray *m_pCurrent;
};

void LayerListVisitor::startElement (const char * name, const XMLAttributes &atts)
{
	if (string(name) == "Layer")
	{
		_level++;
		m_pCurrent = new vtTagArray;
		m_Stack.push_back(m_pCurrent);
	}
	_data = "";
}

void LayerListVisitor::endElement(const char *name)
{
	if (string(name) == "Layer")
	{
		_level--;

		vtTagArray *newlayer = m_Stack[m_Stack.size()-1];

		// only add layers that have a "Name"
		if (newlayer->FindTag("Name"))
			m_Layers.push_back(newlayer);
		else
			delete newlayer;

		m_Stack.pop_back();

		int num = m_Stack.size();
		if (num)
			m_pCurrent = m_Stack[num-1];
		else
			m_pCurrent = NULL;
	}
	else if (m_pCurrent != NULL)
	{
		const char *value = _data.c_str();
		m_pCurrent->AddTag(name, value);
	}
}

void LayerListVisitor::data(const char *s, int length)
{
	_data.append(string(s, length));
}

//
//
//
bool GetLayersFromWFS(const char *szServerURL, OGCLayerArray &layers)
{
	vtString url = szServerURL;
	url += "GetCapabilities?version=0.0.14";

	vtString str;
	ReqContext cl;
	cl.GetURL(url, str);
	if (str == "")
		return false;

	const char *temp_fname = "C:/temp/layers_temp.xml";
	FILE *fp = vtFileOpen(temp_fname, "wb");
	if (!fp)
		return false;
	fwrite((const char *)str, 1, str.GetLength(), fp);
	fclose(fp);

	LayerListVisitor visitor(layers);
	try
	{
		readXML(temp_fname, visitor);
	}
	catch (xh_exception &)
	{
		// TODO: would be good to pass back the error message.
		return false;
	}
	return true;
}


#include <sstream>

//
// for now, handle WMS in this module as well
//
bool GetLayersFromWMS(const char *szServerURL, OGCLayerArray &layers,
					  vtString &msg, bool (*progress_callback)(int))
{
	vtString url = szServerURL;

	int has_qmark = (url.Find("?") != -1);

	if (has_qmark)
		url += "&REQUEST=GetCapabilities";
	else
		url += "?REQUEST=GetCapabilities";

	// some severs need the following to clarify we want WMS
	url += "&SERVICE=WMS";

	VTLOG1("GetLayersFromWMS, URL: ");
	VTLOG1(url);
	VTLOG1("\n");

	ReqContext cl;
	cl.SetProgressCallback(progress_callback);

	vtString str;
	if (!cl.GetURL(url, str))
	{
		// there was an error
		msg = cl.GetErrorMsg();
		return false;
	}

#if 0
	// write to file for debugging
	const char *temp_fname = "C:/temp/wms.xml";
	FILE *fp = vtFileOpen(temp_fname, "wb");
	if (!fp)
		return false;
	fwrite((const char *)str, 1, str.GetLength(), fp);
	fclose(fp);
#endif

	std::istringstream buf((const char *)str);
	LayerListVisitor visitor(layers);
	try
	{
		readXML(buf, visitor);
	}
	catch (xh_exception &ex)
	{
		// TODO: would be good to pass back the error message.
		msg = ex.getFormattedMessage().c_str();
		return false;
	}
	return true;
}

#else
// no support for HTTP
vtFeatureSet *vtFeatureLoader::ReadFeaturesFromWFS(const char *szServerURL, const char *layername)
{
	return NULL;
}
#endif	// SUPPORT_CURL
