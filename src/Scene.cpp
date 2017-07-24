#include "Scene.h"
#include "Sprite.h"
#include "Camera.h"

void WIPScene::add_sprite(WIPSprite* sprite)
{
	if(!initilized)
		return;
	sprite->add_to_scene(this);
	objects.push_back(sprite);
	quad_tree->insert(*sprite);
}

void WIPScene::remove_sprite(WIPSprite* sptrite)
{
	if(!initilized)
		return;
	std::vector<WIPSprite*>::iterator  it;
	int i = 0;
	for( it=objects.begin();it!=objects.end();++it)
	{
		if(*it==sptrite)
		{
			break;
		}
		i++;
	}
	if(it==objects.end())
	{
		LOG_NOTE("No such a node");
	}
	else
	{
		quad_tree->remove_change(*sptrite);
		objects.erase(it);
		sptrite->leave_scene(this);
	}

}

void WIPScene::update_sprite(WIPSprite* sprite)
{
	quad_tree->remove_change(*sprite);
	quad_tree->insert(*sprite);

}

WIPCamera* WIPScene::create_camera(f32 w, f32 h, f32 sw, f32 sh)
{
	WIPCamera* ret =  new WIPCamera(w, h, sw, sh);
	cameras.push_back(ret);
	return ret;
}

WIPScene::~WIPScene()
{
	delete quad_tree;
	for (auto i : cameras)
	{
		delete i;
	}
	cameras.clear();
}