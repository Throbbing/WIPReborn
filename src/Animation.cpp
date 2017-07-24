#if 0
#include "Animation.h"
#include "AnimationClip.h"
#include "SpriteFrame.h"
#include "AnimationManager.h"
#include "Logger.h"

WIPAnimation::WIPAnimation(WIPSpriteFrame* frame_ref)
{
	_frame_ref = frame_ref;
	_speed = 1.f;
}

WIPAnimation::WIPAnimation()
{
	_frame_ref = 0;
	_speed = 1.f;
}

void WIPAnimation::add_clip(WIPAnimationClip* clip,std::string name)
{
	
	_internal_clip_queue[name] = clip;
}

void WIPAnimation::remove_clip(WIPAnimationClip* clip)
{
	g_logger->debug_print(WIP_WARNING,"This function is not in use!");
}
void WIPAnimation::remove_clip(std::string name)
{
	_internal_clip_queue.erase(name);
}
void WIPAnimation::play(WIPAnimationClip* clip)
{
	if(!clip)
		return;
	if(clip->bplaying)
		return;

	_frame_ref->texture = clip->_textures;
	clip->_framebox_ref = _frame_ref->framebox;

	g_animation_manager->add_clip(clip);
}
void WIPAnimation::play(std::string name)
{
	WIPAnimationClip* clip = _internal_clip_queue[name];
	if(clip)
		play(clip);
}
void WIPAnimation::play()
{
	WIPAnimationClip* clip = (_internal_clip_queue.begin()->second);
	if(clip)
		play(clip);
}
void WIPAnimation::stop(WIPAnimationClip* clip)
{
	if(!clip)
		return;
	clip->_will_stop = true;
}
void WIPAnimation::stop(std::string name)
{
	WIPAnimationClip* clip = _internal_clip_queue[name];
	if(clip)
		stop(clip);
}
void WIPAnimation::stop()
{
	WIPAnimationClip* clip = (_internal_clip_queue.begin()->second);
	if(clip)
		stop(clip);
}
void WIPAnimation::rewind(WIPAnimationClip* clip)
{
	if(!clip)
		return;
	stop(clip);
	play(clip);

}
void WIPAnimation::rewind(std::string name)
{
	WIPAnimationClip* clip = _internal_clip_queue[name];
	rewind(clip);
}
void WIPAnimation::rewind()
{
	WIPAnimationClip* clip = (_internal_clip_queue.begin()->second);
	rewind(clip);
}
void WIPAnimation::playeQueue(WIPAnimationQueue* queue)
{
	g_logger->debug_print(WIP_WARNING,"This function is not in use!");
}
#endif