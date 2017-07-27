#pragma once
#include "QuadTree.h"
#include <string>

class WIPSprite;
class WIPCamera;
class WIPScene
{
public:
	WIPScene():initilized(false)
	{
	}
	~WIPScene();

	void init(float world_sizex,float world_sizey,int max_depth)
	{
		world_size_x = world_sizex;
		world_size_y = world_sizey;
		quad_tree = new WIPQuadTree(world_sizex,world_sizey,max_depth);
		quad_tree->build_empty();
		initilized = true;
	}
	void add_sprite(WIPSprite* sprite);
	void remove_sprite(WIPSprite* sptrite);
	WIPCamera* create_camera(f32 w, f32 h, f32 sw, f32 sh, int iww, int iwh);
	void update_sprite(WIPSprite* sprite);
	void sort_by_y(std::vector<WIPSprite*> v);
	void update_zorder_by_type_tag(std::string type_tag,f32 min_z=0.2f,f32 max_z=0.8f);
	void update_zorder_by_type_tag(const WIPSprite* s, std::string type_tag, f32 min_z = 0.2f, f32 max_z = 0.8f);
	void update(f32 dt);
	void fix_update(f32 dt);
	void init_components();
	void destroy_components();

	void clear();
	//todo
	void change_level(){}
	bool initilized;
	float world_size_x,world_size_y;
	WIPQuadTree* quad_tree;
	std::vector<WIPSprite*> objects;
	std::vector<WIPCamera*> cameras;
};