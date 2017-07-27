#include "Scene.h"
#include "Sprite.h"
#include "Camera.h"
#include <algorithm>

void WIPScene::update(f32 dt)
{
	for (auto i:objects)
	{
		i->update(dt);
	}
}

void WIPScene::fix_update(f32 dt)
{
	for (auto i : objects)
	{
		i->fix_update(dt);
	}
}

void WIPScene::init_components()
{
	for (auto i : objects)
	{
		i->init_components();
	}
}

void WIPScene::destroy_components()
{
	for (auto i : objects)
	{
		i->destroy_components();
	}
}

void WIPScene::add_sprite(WIPSprite* sprite)
{
	if(!initilized)
		return;
	sprite->add_to_scene(this);
	objects.push_back(sprite);
	quad_tree->insert(*sprite);
}

void WIPScene::clear()
{

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

WIPCamera* WIPScene::create_camera(f32 w, f32 h, f32 sw, f32 sh, int iww, int iwh)
{
	WIPCamera* ret =  new WIPCamera(w, h, sw, sh,iww,iwh);
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

bool less_y(const WIPSprite* s1, const WIPSprite* s2)
{
	return (s1->_transform->world_y < s2->_transform->world_y);
}

void WIPScene::sort_by_y(std::vector<WIPSprite*> v)
{
	std::sort(objects.begin(), objects.end(), less_y);
}

void WIPScene::update_zorder_by_type_tag(std::string type_tag, f32 min_z, f32 max_z)
{
	std::vector<WIPSprite*> charecters;
	for (int i = 0; i < objects.size(); ++i)
	{
		if (objects[i]->_type_tag == type_tag)
			charecters.push_back(objects[i]);
	}
	if (charecters.empty())
		return;
	sort_by_y(charecters);
	int size = charecters.size()-1;

	f32 inter = (max_z - min_z)*0.9f / size;
	//f32 ybase = charecters[0]->_transform->world_y;
	//f32 spanz = max_z - min_z;
	/*
	for (int i = 0; i < charecters.size(); ++i)
	{
		(charecters[i]->_transform->world_y - ybase);
	}
	*/
	int k = 0;
	for (auto i : charecters)
	{
		i->set_z_order(min_z + inter*k);
		k++;
	}

}


void WIPScene::update_zorder_by_type_tag(const WIPSprite* s, std::string type_tag, f32 min_z, f32 max_z)
{
	std::vector<const WIPSprite* > out_objects;
	std::vector<const WIPSprite* > out_objects_filted;
	quad_tree->get_near_node(*s,out_objects);
	std::sort(out_objects.begin(), out_objects.end());
	int n = 0;
	const WIPSprite* pre = nullptr;
	for (int i = 0; i < out_objects.size(); ++i)
	{
		const WIPSprite* s = out_objects[i];
		if (s != pre)
		{
			out_objects_filted.push_back(s);
		}
	}
	//problem:we may trouble the intersected object's order with other objects which are not in our consideration if only consider the intersected objects.
}