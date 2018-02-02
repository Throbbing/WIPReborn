#pragma once
#include <vector>
#include <hash_map>
#include "Sprite.h"
#include "RBMath/Inc/AABB.h"
#include "RefCount.h"

class WIPScene;
//#define W
class WIPQuadTreeNode
{
public:
	WIPQuadTreeNode(bool lf=false,bool rt=true):leaf(lf),root(rt)
	{
		child[0] = nullptr;
		child[1] = nullptr;
		child[2] = nullptr;
		child[3] = nullptr;
	}
	~WIPQuadTreeNode()
	{
		//index.swap(std::vector<int>());
	}
#ifdef W
	typedef std::hash_map< TRefCountPtr<const WIPSprite>, TRefCountPtr<const WIPSprite>> object_list_t;
#else
	typedef HashLink<WIPSprite> object_list_t;
#endif
	object_list_t index;
	bool leaf;
	bool root;
	//ltlbrtrb
	WIPQuadTreeNode* child[4];
};

/*
TODO:
use aabb2d.is_contain(aabb)
the object belongs to the node when the object intersect with its two children or more.
or
push the object to children just when the children totally contain it.
if we do like above, objects can store just only once in the tree.
*/
class WIPQuadTree : public FRefCountedObject
{
public:
	//give the max care world range
	WIPQuadTree(float ws, float hs, int dp = 8, const WIPScene* in_scene = 0);
	WIPQuadTree(int dp = 8, const WIPScene* in_scene = 0);

	void build_empty()
	{
		if(depth==1)
			root.leaf = true;
		_build_empty(&root,depth);
	}
	
	void _build_empty(WIPQuadTreeNode* root,int cd)
	{
		if(cd==1)
		{
			return;
		}
		root->child[0] = new WIPQuadTreeNode(cd==2,false);
		root->child[1] = new WIPQuadTreeNode(cd==2,false);
		root->child[2] = new WIPQuadTreeNode(cd==2,false);
		root->child[3] = new WIPQuadTreeNode(cd==2,false);
		cd--;
		_build_empty(root->child[0],cd);
		_build_empty(root->child[1],cd);
		_build_empty(root->child[2],cd);
		_build_empty(root->child[3],cd);
	}
	void insert(const WIPSprite&  sprite)
	{
		RBAABB2D& world_aabb = world_bound;
		RBAABB2D sprite_bound;
		RBVector2 vert[4];
		sprite.get_world_position(vert);
		sprite_bound.include(vert);
		//if (!sprite_bound.intersection(world_aabb))
		{
			world_bound.include(sprite_bound.min);
			world_bound.include(sprite_bound.max);
		}
		_insert(&root, sprite_bound, world_aabb, &sprite);
	}
	void _insert(WIPQuadTreeNode* root,const RBAABB2D& sprite_bound,const RBAABB2D& aabb,TRefCountPtr<const WIPSprite> id)
	{
		if(root->leaf)
		{
#ifdef W
			root->index[id]=(id);
#else
			root->index.insert(id);
#endif
			return;
		}
		RBVector2 c = aabb.min+(aabb.max-aabb.min)*0.5f;
		/*
		//push to parent node?
		bool in_cur_bound = sprite_bound.is_contain(c);
		if(in_cur_bound)
		{
			root->index.push_back(id);
			return;
		}
		*/
		RBAABB2D lbb(aabb.min.x,aabb.min.y,c.x,c.y);
		if(lbb.intersection(sprite_bound))
		{
			_insert(root->child[0],sprite_bound,lbb,id);
		}
		RBAABB2D ltb(aabb.min.x,c.y,c.x,aabb.max.y);
		if(ltb.intersection(sprite_bound))
		{
			_insert(root->child[1],sprite_bound,ltb,id);
		}
		RBAABB2D rtb(c.x,c.y,aabb.max.x,aabb.max.y);
		if(rtb.intersection(sprite_bound))
		{
			_insert(root->child[2],sprite_bound,rtb,id);
		}
		RBAABB2D rbb(c.x,aabb.min.y,aabb.max.x,c.y);
		if(rbb.intersection(sprite_bound))
		{
			_insert(root->child[3],sprite_bound,rbb,id);
		}
		

	}
	void get_intersected_node(const RBAABB2D& aabb,std::vector< TRefCountPtr<const WIPSprite>>& out_index) const
	{
		RBAABB2D world_aabb(world_bound);
		if (world_aabb.intersection(aabb))
			_get_intersected_node(&root,aabb,world_aabb,out_index);
	}
	void _get_intersected_node(const WIPQuadTreeNode* root, const RBAABB2D& sprite_bound, const RBAABB2D& aabb, std::vector< TRefCountPtr<const WIPSprite> >& out_index) const
	{
		if(root->leaf)
		{
#ifdef W
			WIPQuadTreeNode::object_list_t::const_iterator it;
			for (it = root->index.begin(); it != root->index.end(); ++it)
			{
				out_index.push_back(it->second);
			}
#else
			auto* it = root->index.head();
			while (it)
			{
				out_index.push_back(it->data);
				it = it->next;
			}
#endif
			return;
		}
		RBVector2 c = aabb.min+(aabb.max-aabb.min)*0.5f;
		RBAABB2D lbb(aabb.min.x,aabb.min.y,c.x,c.y);
		if(lbb.intersection(sprite_bound))
		{
			_get_intersected_node(root->child[0],sprite_bound,lbb,out_index);
		}
		RBAABB2D ltb(aabb.min.x,c.y,c.x,aabb.max.y);
		if(ltb.intersection(sprite_bound))
		{
			_get_intersected_node(root->child[1],sprite_bound,ltb,out_index);
		}
		RBAABB2D rtb(c.x,c.y,aabb.max.x,aabb.max.y);
		if(rtb.intersection(sprite_bound))
		{
			_get_intersected_node(root->child[2],sprite_bound,rtb,out_index);
		}
		RBAABB2D rbb(c.x,aabb.min.y,aabb.max.x,c.y);
		if(rbb.intersection(sprite_bound))
		{
			_get_intersected_node(root->child[3],sprite_bound,rbb,out_index);
		}
	}
	void get_near_node(const WIPSprite&  sprite, std::vector<TRefCountPtr<const WIPSprite>>& out_index) const
	{
		RBAABB2D world_aabb(world_bound);
		RBAABB2D sprite_bound;
		RBVector2 vert[4];
		sprite.get_world_position(vert);
		sprite_bound.include(vert);
		_get_near_node(&root,sprite_bound,world_aabb,&sprite,out_index);
	}
	void get_near_node(const RBVector2& pos, std::vector< TRefCountPtr<const WIPSprite>>& out_index) const
	{
		RBAABB2D world_aabb(world_bound);
		_get_near_node_point(&root,pos,world_aabb,out_index);
	}
	void _get_near_node_point(const WIPQuadTreeNode* root, const RBVector2& pos, const RBAABB2D& aabb, std::vector< TRefCountPtr<const WIPSprite>>& out_index) const
	{
		if(root->leaf)
		{
#ifdef W
			WIPQuadTreeNode::object_list_t::const_iterator it;
			for (it = root->index.begin(); it != root->index.end(); ++it)
			{
				out_index.push_back(it->second);
			}
#else
			auto* it = root->index.head();
			while (it)
			{
				out_index.push_back(it->data);
				it = it->next;
			}
#endif
			return;
		}
		RBVector2 c = aabb.min+(aabb.max-aabb.min)*0.5f;
		RBAABB2D lbb(aabb.min.x,aabb.min.y,c.x,c.y);
		if(lbb.is_contain(pos))
		{
			_get_near_node_point(root->child[0],pos,lbb,out_index);
		}
		RBAABB2D ltb(aabb.min.x,c.y,c.x,aabb.max.y);
		if(ltb.is_contain(pos))
		{
			_get_near_node_point(root->child[1],pos,ltb,out_index);
		}
		RBAABB2D rtb(c.x,c.y,aabb.max.x,aabb.max.y);
		if(rtb.is_contain(pos))
		{
			_get_near_node_point(root->child[2],pos,rtb,out_index);
		}
		RBAABB2D rbb(c.x,aabb.min.y,aabb.max.x,c.y);
		if(rbb.is_contain(pos))
		{
			_get_near_node_point(root->child[3],pos,rbb,out_index);
		}

	}
	void _get_near_node(const WIPQuadTreeNode* root, const RBAABB2D& sprite_bound, const RBAABB2D& aabb, TRefCountPtr<const WIPSprite> id, std::vector<TRefCountPtr<const WIPSprite>>& out_index) const
	{
		if(root->leaf)
		{
#ifdef W
			WIPQuadTreeNode::object_list_t::const_iterator it;
			for (it = root->index.begin(); it != root->index.end(); ++it)
			{
				out_index.push_back(it->second);
			}
#else
			auto* it = root->index.head();
			while (it)
			{
				out_index.push_back(it->data);
				it = it->next;
			}
#endif
			return;
		}
		RBVector2 c = aabb.min+(aabb.max-aabb.min)*0.5f;
		RBAABB2D lbb(aabb.min.x,aabb.min.y,c.x,c.y);
		if(lbb.intersection(sprite_bound))
		{
			_get_near_node(root->child[0],sprite_bound,lbb,id,out_index);
		}
		RBAABB2D ltb(aabb.min.x,c.y,c.x,aabb.max.y);
		if(ltb.intersection(sprite_bound))
		{
			_get_near_node(root->child[1],sprite_bound,ltb,id,out_index);
		}
		RBAABB2D rtb(c.x,c.y,aabb.max.x,aabb.max.y);
		if(rtb.intersection(sprite_bound))
		{
			_get_near_node(root->child[2],sprite_bound,rtb,id,out_index);
		}
		RBAABB2D rbb(c.x,aabb.min.y,aabb.max.x,c.y);
		if(rbb.intersection(sprite_bound))
		{
			_get_near_node(root->child[3],sprite_bound,rbb,id,out_index);
		}
	}
	void debug_draw(const WIPCamera* cam)
	{
		RBAABB2D world_aabb(world_bound);
		//ofPushStyle();
		//ofSetColor(ofColor::blue);
		_debug_draw(&root,world_aabb,cam);
		//ofPopStyle();
	}
	void _debug_draw(WIPQuadTreeNode* root,const RBAABB2D& aabb,const WIPCamera*cam)
	{
		if(root->leaf)
		{
			RBVector2 c = aabb.min+(aabb.max-aabb.min)*0.5f;

			g_rhi->debug_draw_aabb2d(aabb.min,aabb.max,cam);
			
			return;
		}
		RBVector2 c = aabb.min+(aabb.max-aabb.min)*0.5f;
		RBAABB2D lbb(aabb.min.x,aabb.min.y,c.x,c.y);
		
		RBAABB2D ltb(aabb.min.x,c.y,c.x,aabb.max.y);
		RBAABB2D rtb(c.x,c.y,aabb.max.x,aabb.max.y);
		RBAABB2D rbb(c.x,aabb.min.y,aabb.max.x,c.y);
		_debug_draw(root->child[0],lbb,cam);
		_debug_draw(root->child[1],ltb,cam);
		_debug_draw(root->child[2],rtb,cam);
		_debug_draw(root->child[3],rbb,cam);

	}
	void remove_change(const WIPSprite& sprite)
	{
		//sprite bound changed,we have to search the whole tree
		_remove_change(&root, world_bound, &sprite);
	}
	void _remove_change(WIPQuadTreeNode* root, const RBAABB2D& aabb,  TRefCountPtr<const WIPSprite> id)
	{
		if (root->leaf)
		{
#ifdef W
			if (root->index.find(id) != root->index.end())
				root->index.erase(id);
#else
			root->index.remove(id);
#endif
			return;
		}
		RBVector2 c = aabb.min + (aabb.max - aabb.min)*0.5f;
		RBAABB2D lbb(aabb.min.x, aabb.min.y, c.x, c.y);
		_remove_change(root->child[0], lbb, id);
		RBAABB2D ltb(aabb.min.x, c.y, c.x, aabb.max.y);
		_remove_change(root->child[1], ltb, id);
		RBAABB2D rtb(c.x, c.y, aabb.max.x, aabb.max.y);
		_remove_change(root->child[2], rtb, id);
		RBAABB2D rbb(c.x, aabb.min.y, aabb.max.x, c.y);
		_remove_change(root->child[3], rbb, id);
	}
	void remove(const WIPSprite&  sprite)
	{
		RBAABB2D world_aabb(world_bound);
		RBAABB2D sprite_bound;
		RBVector2 vert[4];
		sprite.get_world_position(vert);
		sprite_bound.include(vert);
		_remove(&root,sprite_bound,world_aabb,&sprite);
	}
	
	
	void _remove(WIPQuadTreeNode* root, const RBAABB2D& sprite_bound, const RBAABB2D& aabb,  TRefCountPtr<const WIPSprite> id)
	{
		if(root->leaf)
		{
#ifdef W
			if (root->index.find(id) != root->index.end())
				root->index.erase(id);
#else
			root->index.remove(id);
#endif
			return;
		}
		RBVector2 c = aabb.min+(aabb.max-aabb.min)*0.5f;
		RBAABB2D lbb(aabb.min.x,aabb.min.y,c.x,c.y);
		if(lbb.intersection(sprite_bound))
		{
			_remove(root->child[0],sprite_bound,lbb,id);
		}
		RBAABB2D ltb(aabb.min.x,c.y,c.x,aabb.max.y);
		if(ltb.intersection(sprite_bound))
		{
			_remove(root->child[1],sprite_bound,ltb,id);
		}
		RBAABB2D rtb(c.x,c.y,aabb.max.x,aabb.max.y);
		if(rtb.intersection(sprite_bound))
		{
			_remove(root->child[2],sprite_bound,rtb,id);
		}
		RBAABB2D rbb(c.x,aabb.min.y,aabb.max.x,c.y);
		if(rbb.intersection(sprite_bound))
		{
			_remove(root->child[3],sprite_bound,rbb,id);
		}
	}
	void get_all_nodes(std::vector< TRefCountPtr<const WIPSprite>>& out_index) const
	{
		_get_all_nodes(&root,out_index);
	}
	void _get_all_nodes(const WIPQuadTreeNode* root, std::vector< TRefCountPtr<const WIPSprite>>& out_index) const
	{
		if(root->leaf)
		{
#ifdef W
			WIPQuadTreeNode::object_list_t::const_iterator it;
			for (it = root->index.begin(); it != root->index.end(); ++it)
			{
				out_index.push_back(it->second);
			}
#else
			auto* it = root->index.head();
			while (it)
			{
				out_index.push_back(it->data);
				it = it->next;
			}
#endif
			return;
		}
		_get_all_nodes(root->child[0],out_index);
		_get_all_nodes(root->child[1],out_index);
		_get_all_nodes(root->child[2],out_index);
		_get_all_nodes(root->child[3],out_index);

	}
	void _release(const WIPQuadTreeNode* root)
	{
		if(!root)
			return;
		_release(root->child[0]);
		_release(root->child[1]);
		_release(root->child[2]);
		_release(root->child[3]);
		if (root!=&this->root)
			delete root;
		
	}

	void compress_world_bound();

	~WIPQuadTree()
	{
		_release(&root);
	}
	WIPQuadTreeNode root;
	int depth;
	RBAABB2D world_bound;
	const WIPScene* scene_ref;
};