#include "AnimationQueue.h"
#include "Logger.h"

void WIPAnimationQueue::add_clip(WIPAnimationClip* clip)
{
	_internal_clips.push_back(clip);
}

void WIPAnimationQueue::remove_clip(WIPAnimationClip* clip)
{
	_ClipList::iterator it;
	for(it = _internal_clips.begin();it!=_internal_clips.end();++it)
	{
		if((*it)==clip)
		{
			_internal_clips.erase(it);
			break;
		}
	}
#ifdef _DEBUG
	if(it==_internal_clips.end())
	{
		g_logger->debug_print(WIP_NOTE,"There is no clip:%d",clip);
	}
#endif
}

void WIPAnimationQueue::clear()
{
	_internal_clips.clear();
}
