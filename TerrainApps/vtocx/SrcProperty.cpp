// SrcProperty.cpp : implementation file
//

#include "stdafx.h"
#include "vtocx.h"
#include "SrcProperty.h"
#include "vtocxCtrl.h"

#include <fstream>
#include <strstream>
#include <string>
//#include <ios>

#include <osgDB/ReadFile>

// CSrcProperty

CSrcProperty::CSrcProperty()
{
}

CSrcProperty::~CSrcProperty()
{
}

void CSrcProperty::Close()
{
	// Add your specialized code here and/or call the base class
	CCachedDataPathProperty::Close();
}

osg::ref_ptr<osg::Node> CSrcProperty::getNode()
{
	return _node;
}

// CSrcProperty member functions

AFX_STATIC inline DWORD __AfxTransferFileContent(CFile* pFrom, CFile* pTo)
{
	BYTE buff[512*1024]; // enlarging the buffer speeds up loading significantly !
	DWORD dwRead = 0;
	DWORD dwActual;
	do
	{
		dwActual = pFrom->Read(buff, 512*1024);
		pTo->Write(buff, dwActual);

		dwRead += dwActual;
	}
	while (dwActual > 0);
	return dwRead;
}

void CSrcProperty::OnDataAvailable(DWORD dwSize, DWORD bscfFlag)
{
//	CCachedDataPathProperty::OnDataAvailable(dwSize, bscfFlag);
	UNUSED_ALWAYS(bscfFlag);
	UNUSED_ALWAYS(dwSize);
	ULONGLONG dwPos = m_Cache.GetPosition();
	TRY
	{
		// Cache the data in our mem file.
		m_Cache.SeekToEnd();
		__AfxTransferFileContent(this, &m_Cache);
	}
	CATCH_ALL(e)
	{
		m_Cache.Seek(dwPos, CFile::begin);
		THROW_LAST();
	}
	END_CATCH_ALL
	m_Cache.Seek(dwPos, CFile::begin);

	if(bscfFlag & BSCF_LASTDATANOTIFICATION)
	{
		m_Cache.SeekToBegin();

		// get the length (bytes) of the memory file
		long memLen = m_Cache.GetLength();

		// detach the buffer and close the file
		char* memData = (char*) m_Cache.Detach();

		std::istrstream istream(memData, memLen);
		CString extension = GetPath();
		int pos = extension.ReverseFind('.');
		if(++pos) extension = extension.Right(extension.GetLength() - pos); // remove path and filename from extension

		// (Load data from stream, if desired)

		free(memData);
//		m_Cache.Attach((BYTE*)memData, memLen);
	}
}

void CSrcProperty::OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCTSTR szStatusText)
{
	// Add your specialized code here and/or call the base class
	// add code that make a progressbar available in the OpenGL window...
	CCachedDataPathProperty::OnProgress(ulProgress, ulProgressMax, ulStatusCode, szStatusText);
}

