#pragma once
#include "RBMath/Inc/Platform/RBBasedata.h"
#include <list>
#include "RefCount.h"

class WIPAnimationClip;

class WIPAnimationQueue : public FRefCountedObject
{
public:
	void add_clip(WIPAnimationClip* clip);
	void remove_clip(WIPAnimationClip* clip);
	void clear();

private:
	typedef std::list<WIPAnimationClip*> _ClipList;
	_ClipList _internal_clips;
};