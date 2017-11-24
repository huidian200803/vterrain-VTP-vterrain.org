//
// WFSClient.h
//
// Web Feature Server Client
//
// Copyright (c) 2003-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTDATA_WFSCLIENT_H
#define VTDATA_WFSCLIENT_H

#include "Content.h"

class OGCLayerArray : public std::vector<vtTagArray *>
{
public:
	OGCLayerArray() { m_iSelected = -1; }
	~OGCLayerArray() {
		for (uint i = 0; i < size(); i++)
			delete at(i);
	}
	// remember which layer the user last expressed interest in
	int m_iSelected;
};

struct OGCServer
{
	vtString m_url;
	OGCLayerArray m_layers;
};

class OGCServerArray : public std::vector<OGCServer>
{
public:
	OGCServerArray() { m_iSelected = -1; }
	// remember which server the user last expressed interest in
	int m_iSelected;
};

bool GetLayersFromWFS(const char *szServerURL, OGCLayerArray &layers);

// for now, handle WMS here as well
bool GetLayersFromWMS(const char *szServerURL, OGCLayerArray &layers,
					  vtString &msg, bool (*progress_callback)(int) = NULL);

#endif // VTDATA_WFSCLIENT_H

