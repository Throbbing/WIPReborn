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

void WIPScene::init(float world_sizex, float world_sizey, int max_depth)
{
	world_size_x = world_sizex;
	world_size_y = world_sizey;
	quad_tree = new WIPQuadTree(world_sizex, world_sizey, max_depth);
	quad_tree->build_empty();
	initilized = true;
}

void WIPScene::add_sprite(TRefCountPtr<WIPSprite> sprite)
{
	if(!initilized)
		return;
	sprite->add_to_scene(this);
	objects.push_back(sprite);
	quad_tree->insert(*sprite);
}

void WIPScene::clear()
{

	objects.clear();
}

void WIPScene::remove_sprite(TRefCountPtr<WIPSprite> sptrite, bool op_related_scene)
{
	if(!initilized)
		return;
	std::vector<TRefCountPtr<WIPSprite>>::iterator  it;
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
		if (op_related_scene)
			sptrite->leave_scene(this);
	}

}

void WIPScene::update_sprite(TRefCountPtr<WIPSprite> sprite)
{
	//todo:must avoid every-frame remove-insert!!
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

bool less_y(TRefCountPtr< WIPSprite> s1, TRefCountPtr< WIPSprite> s2)
{
	return (s1->_transform->world_y < s2->_transform->world_y);
}

void WIPScene::sort_by_y(std::vector<TRefCountPtr<WIPSprite>> v)
{
	std::sort(objects.begin(), objects.end(), less_y);
}

void WIPScene::update_zorder_by_type_tag(std::string type_tag, f32 min_z, f32 max_z)
{
	std::vector<TRefCountPtr<WIPSprite>> charecters;
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


void WIPScene::update_zorder_by_type_tag(TRefCountPtr<const WIPSprite> s, std::string type_tag, f32 min_z, f32 max_z)
{
	std::vector<TRefCountPtr<const WIPSprite> > out_objects;
	std::vector<TRefCountPtr<const WIPSprite> > out_objects_filted;
	quad_tree->get_near_node(*s,out_objects);
	std::sort(out_objects.begin(), out_objects.end());
	int n = 0;
	TRefCountPtr<const WIPSprite> pre = nullptr;
	for (int i = 0; i < out_objects.size(); ++i)
	{
		TRefCountPtr<const WIPSprite> s = out_objects[i];
		if (s != pre)
		{
			out_objects_filted.push_back(s);
		}
	}
	//problem:we may trouble the intersected object's order with other objects which are not in our consideration if only consider the intersected objects.
}

WIPSprite* WIPScene::get_sprite_by_tag(const std::string& name) const
{
	for (auto i : objects)
	{
		if (i->get_tag() == name)
		{
			return i.GetReference();
		}
	}
	return nullptr;
}