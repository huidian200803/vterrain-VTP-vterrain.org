#pragma once

#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>

template<class CViewer>
class CGLThread : public OpenThreads::Thread, public OpenThreads::Mutex
{
public:
	CGLThread(CViewer* viewer) : Thread(), _viewer(viewer), _resize(false) {}

	void resizeViewport(const long w, const long h)
	{
		_w = w;
		_h = h;
		_resize = true;
	}

	void run()
	{
		while(_viewer->rendering())
		{
			if(_resize)
			{
				_viewer->resize(_w, _h);
				_resize = false;
			}
			_viewer->frame();
		}
	}

private:
	CViewer* _viewer;
	bool _rendering;
	bool _resize;

	int _w, _h;
};
