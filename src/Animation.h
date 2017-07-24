#if 0
#pragma once
#include "RBMath/Inc/Platform/RBBasedata.h"
#include <string>
#include <map>

class WIPSpriteFrame;
class WIPAnimationClip;
class WIPAnimationQueue;



class WIPAnimation
{
public:
	WIPAnimation(WIPSpriteFrame* frame_ref);
	//for lua
	WIPAnimation();
	void add_clip(WIPAnimationClip* clip,std::string name);
	//not use
	void remove_clip(WIPAnimationClip* clip);
	void remove_clip(std::string name);
	void play(WIPAnimationClip* clip);
	void play(std::string name);
	void play();
	void stop(WIPAnimationClip* clip);
	void stop(std::string name);
	void stop();
	void rewind(WIPAnimationClip* clip);
	void rewind(std::string name);
	void rewind();
	void playeQueue(WIPAnimationQueue* queue);

	INLINE void reset_frame_ref(WIPSpriteFrame* sf)
	{
		_frame_ref = sf;
	}
private:
	//--- 1 for normal,can be negtive
	f32 _speed;
	WIPSpriteFrame* _frame_ref;
	typedef std::map<std::string,WIPAnimationClip*> _Internal_clip_queue;
	_Internal_clip_queue _internal_clip_queue;
};
#endif