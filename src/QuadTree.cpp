#include "QuadTree.h"
#include "Scene.h"


WIPQuadTree::WIPQuadTree(float ws, float hs, int dp , const WIPScene* in_scene ) :
root(), depth(dp), scene_ref(in_scene)
{
	float a = ws*0.5f;
	float b = hs*0.5f;
	world_bound.reset();
	world_bound.include(RBVector2(-a, -b));
	world_bound.include(RBVector2(a, b));

}

WIPQuadTree::WIPQuadTree(int dp, const WIPScene* in_scene) :
root(), depth(dp), scene_ref(in_scene)
{
	world_bound.reset();
}

void WIPQuadTree::compress_world_bound()
{
	if (!scene_ref)
	{
		LOG_INFO("Can't compress world bound because there is no scene_ref!");
		return;
	}
	world_bound.reset();
	RBVector2 vert[4];
	
	for (int i = 0; i < scene_ref->objects.size(); ++i)
	{
		scene_ref->objects[i]->get_world_position(vert);
		world_bound.include(vert[0]);
		world_bound.include(vert[1]);
		world_bound.include(vert[2]);
		world_bound.include(vert[3]);

	}
}