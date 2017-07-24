#pragma once
#include "RBMath/Inc/Platform/RBBasedata.h"
#include <list>
#include "LinkList.h"
#include <list>
#include <vector>


using std::list;
using std::vector;

#define WIP_ANIMATION_DELTA_T 1/24

class WIPAnimationClip;
class WIPClipInstance;

class WIPAnimationManager
{
public:
	static WIPAnimationManager* instance();

	bool startup(f32 delta = 1/24);
	void shutdown();
	void update(f32 dt);

	void add_clip(WIPClipInstance* clip);
	void add_clip_back(WIPClipInstance* clip);
	void add_clip_front(WIPClipInstance* clip);

protected:
	WIPAnimationManager();
	~WIPAnimationManager();

private:

	

	//typedef LinkList<WIPClipInstance*> _ClipQueue;
	//_ClipQueue _clip_queue;
	//i32 _queue_szie;

	//f32 _cur_dt;
	f32 _delta_t;

	list<WIPClipInstance*> _clip_queue;
	vector<WIPClipInstance*> _remove_list;
};

extern WIPAnimationManager* g_animation_manager;