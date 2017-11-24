#pragma once

#include <osg/Node>


// CSrcProperty command target

class CSrcProperty : public CCachedDataPathProperty
{
public:
	CSrcProperty();
	virtual ~CSrcProperty();

	virtual void Close();

	osg::ref_ptr<osg::Node> getNode();

protected:
	bool _modelLoaded;
	osg::ref_ptr<osg::Node> _node;

	virtual void OnDataAvailable(DWORD dwSize, DWORD bscfFlag);
	virtual void OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCTSTR szStatusText);
};


