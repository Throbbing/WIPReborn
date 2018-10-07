
#include "Sprite.h"
#include "Camera.h"
#include <algorithm>
#include "Scene.h"



WIPScene* WIPScene::instance()
{
	static WIPScene* _instance;
	if (!_instance)
		_instance = new WIPScene();
	return _instance;
}

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
	world_depth = max_depth;
	quad_tree = new WIPQuadTree(world_sizex, world_sizey, max_depth);
	quad_tree->build_empty();
	initilized = true;



  

}

void WIPScene::add_sprite(WIPSprite* sprite)
{
	if(!initilized)
		return;
	sprite->add_to_scene(this);
	objects.push_back(sprite);
	quad_tree->insert(*sprite);
}

void WIPScene::load_level(int id,const RBVector2& pos)
{
	if (!loader)
		return;
	loader->load_level(id, pos);
}

void WIPScene::clear(bool destroy_persistent)
{
	if (!initilized)
		return;
	std::vector<TRefCountPtr<WIPSprite>>::iterator  it;
	int i = 0;
	for (it = objects.begin(); it != objects.end(); ++it)
	{
    (*it)->end_components();
    (*it)->leave_scene(this);
		if (!destroy_persistent&&(*it)->get_persistent())
		{
      //cache *it
      //it->clear()
      _objects_cache.push_back(*it);
      
			continue;
		}
    
		
		WIPSprite::destroy(*it);
	}

	objects.clear();
	delete quad_tree;
	quad_tree = new WIPQuadTree(world_size_x, world_size_y, world_depth);
  
	quad_tree->build_empty();
	/*
	for (auto i : cameras)
	{
		delete i;
	}
	*/
  
	cameras.clear();
  
}

void WIPScene::remove_sprite(WIPSprite*  sptrite, bool op_related_scene)
{
	if(!initilized)
		return;
	std::vector<TRefCountPtr<WIPSprite> >::iterator  it;
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

void WIPScene::update_sprite(WIPSprite*  sprite)
{
	//todo:must avoid every-frame remove-insert!!
	quad_tree->remove_change(*sprite);
	quad_tree->insert(*sprite);

}

void WIPScene::load_sprite(WIPSprite* sprite)
{
	if (sprite)
		loaded_objects.push_back(sprite);
}
void WIPScene::load_camera(WIPCamera* camera)
{
	if (camera)
		loaded_cameras.push_back(camera);
}

void WIPScene::add_camera(WIPCamera* camera)
{
	cameras.push_back(camera);
}

WIPCamera* WIPScene::create_camera(f32 w, f32 h, f32 sx, f32 sy, f32 sw, f32 sh, int iww, int iwh)
{
	WIPCamera* ret =  new WIPCamera(w, h,sx,sy, sw, sh,iww,iwh);
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

void WIPScene::sort_by_y(std::vector<WIPSprite* > v)
{
	std::sort(objects.begin(), objects.end(), less_y);
}

void WIPScene::update_zorder_by_type_tag(std::string type_tag, f32 min_z, f32 max_z)
{
	std::vector<WIPSprite* > charecters;
	for (int i = 0; i < objects.size(); ++i)
	{
		if (objects[i]->_type_tag == type_tag)
			charecters.push_back(objects[i]);
	}
	if (charecters.empty())
		return;
	sort_by_y(charecters);
	int size = charecters.size();

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


void WIPScene::update_zorder_by_type_tag(const WIPSprite*  s, std::string type_tag, f32 min_z, f32 max_z)
{
	std::vector<const WIPSprite*  > out_objects;
	std::vector<const WIPSprite*  > out_objects_filted;
	quad_tree->get_near_node(*s,out_objects);
	std::sort(out_objects.begin(), out_objects.end());
	int n = 0;
	const WIPSprite*  pre = nullptr;
	for (int i = 0; i < out_objects.size(); ++i)
	{
		const WIPSprite*  s = out_objects[i];
		if (s != pre)
		{
			out_objects_filted.push_back(s);
		}
	}
	//problem:we may trouble the intersected object's order with other objects which are not in our consideration if only consider the intersected objects.
}


WIPCamera* WIPScene::get_camera(const std::string& name) const
{
	for (auto i : cameras)
	{
		if (i->name == name)
		{
			return i;
		}
	}
	return nullptr;
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
void WIPScene::cancel_object_change()
{
	_deleting_objects.clear();
	for (auto i : _creating_objects)
	{
		WIPSprite::destroy(i);

	}
	_creating_objects.clear();
}
void WIPScene::submit_object_change()
{
	for (auto i : _deleting_objects)
	{
		WIPSprite::destroy(i);

	}
	_deleting_objects.clear();
	for (auto i : _creating_objects)
	{
		add_sprite(i);

	}
	//must init after all the objects is in.
	for (auto i : _creating_objects)
	{
		i->init_components();

	}
	_creating_objects.clear();
}
//creating or deleting objects will happened at the end of a frame!
void WIPScene::pending_objects(WIPSprite* s)
{
	_creating_objects.push_back(s);
}

//add object to scene in case of destory iterator.
void WIPScene::creating_object(WIPSprite* s)
{
	_deleting_objects.push_back(s);
}
//issue : scene only do add_sprite / remove_sprite(cannot delete continuously when iter objects) or clear
void WIPScene::submit_level_change()
{
	if (level_change_state & 2)
	{
		//unload
		clear();
	}
	if (level_change_state & 1)
	{
		//load
		for (auto i : loaded_objects)
		{
			add_sprite(i);
		}

    //restore cached persistent object
    for (auto i : _objects_cache)
    {
      //_objects_cache[0]->init_components();
      add_sprite(i);
    }

		for (auto i : loaded_cameras)
		{
			add_camera(i);
		}
    
    for (auto i : loaded_objects)
		{
      //init just for loaded objects
			i->init_components();
		}
    for (auto i : objects)
    {
      i->start_components();
    }
		loaded_cameras.clear();
		loaded_objects.clear();
    _objects_cache.clear();
	}
	reset_level_load_state();
}
void WIPScene::render_world()
{
	if (!world_renderer)
		return;
	for (auto i : cameras)
	{
		g_rhi->change_viewport(i->viewport);
		world_renderer->render(i);
	}
}
void WIPScene::render_ui()
{
	g_temp_uisys->render();
}

