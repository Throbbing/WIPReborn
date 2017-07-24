#pragma once
#include "QuadTree.h"

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
	WIPCamera* create_camera(f32 w, f32 h, f32 sw, f32 sh);
	void update_sprite(WIPSprite* sprite);
	bool initilized;
	float world_size_x,world_size_y;
	WIPQuadTree* quad_tree;
	std::vector<WIPSprite*> objects;
	std::vector<WIPCamera*> cameras;
};