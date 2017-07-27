#pragma once

#ifndef USE

#include "Render.h"
#include <string>
#include <map>
#include "Box2D/Box2D.h"

//message dispatch
class WIPObject
{
public:

};


enum class NormalType
{
	E_NOT_USE,
	E_VERTEX_NORMAL,
	E_TEXTURE_NORMAL
};

class WIPMaterialNormal
{
public:
	WIPMaterialNormal():tp(NormalType::E_NOT_USE){}
	NormalType tp;
};

class WIPVertexMaterialNormal : public WIPMaterialNormal
{

};

class WIPTextureMaterialNormal : public WIPMaterialNormal
{

};

enum class WIPMaterialType
{
	E_OPAQUE,
	E_TRANSLUCENT,
	E_ALPHAMASK,
	E_OTHER
};

//vertex color——material
//vertex uv —— framebox
//texture —— material
//normal —— material 

//same alpha->same texture(cpu transform vertices make a total buffer to render!)
class WIPMaterial 
{
public:
	WIPMaterial():texture(nullptr),material_normal(nullptr),shader(nullptr)
	{
		vert_color[0] = RBColorf::white;
		vert_color[1] = RBColorf::white;
		vert_color[2] = RBColorf::white;
		vert_color[3] = RBColorf::white;
		material_type = WIPMaterialType::E_OTHER;
	}
	WIPTexture2D* texture;
	WIPMaterialNormal* material_normal;
	RBColorf vert_color[4];
	WIPBoundShader* shader;

	WIPMaterialType material_type;
	
};


class WIPMaterialManager
{
public:
	void load_default_shaders(const char* shader_path)
	{
		
	}
	std::vector<WIPBoundShader*> shaders;

};


#include "RBMath/Inc/Vector2.h"



//object coordinates
//Anchor is on geometric center
class WIPMesh
{
public:
	WIPMesh(f32 w,f32 h)
	{
		set_quickly(-w/2,h/2,-w/2,-h/2,w/2,h/2,w/2,-h/2);
	}
	WIPMesh()
	{
		f32 w = 1;
		f32 h = 1;
		set_quickly(-w/2,h/2,-w/2,-h/2,w/2,h/2,w/2,-h/2);		
	}
	//WIPMesh(const WIPMesh&);
	RBVector2 lt,lb,rb,rt;
	float get_witdh()
	{
		return RBMath::abs(rt.x  - lt.x);
	}
	float get_height()
	{
		return RBMath::abs(rt.y-rb.y);
	}
	//copy
	//these will be remove
	/*
	RBVector2 ltc,lbc,rbc,rtc;
	RBVector2 anchorc;
	*/
	void set_quickly
		(
		float lt_x1,float lt_y1,
		float lb_x2,float lb_y2,
		float rb_x3,float rb_y3,
		float rt_x4,float rt_y4
		)
		{
			lt = RBVector2(lt_x1,lt_y1);
			lb = RBVector2(lb_x2,lb_y2);
			rb = RBVector2(rb_x3,rb_y3);
			rt = RBVector2(rt_x4,rt_y4);
		}


private:
	
};

//system component
class WIPComponent : public WIPObject
{
public:
	WIPComponent(class WIPSprite* s);
	WIPComponent();
	virtual ~WIPComponent() = 0;
	class WIPSprite* host_object;
	void set_host(class WIPSprite* ho);

};

//user component
class WIPTickComponent : public WIPComponent
{
public:
	WIPTickComponent(class WIPSprite* host);
	virtual ~WIPTickComponent() = 0;
	virtual void init() = 0;
	virtual void update(f32 dt)=0;
	virtual void fix_update(f32 dt)=0;
	virtual void destroy() = 0;
};

class WIPRenderComponent : public WIPComponent
{
public:
	~WIPRenderComponent(){}
	WIPRenderComponent(float w,float h):mesh(w,h){}
	WIPMaterial material;
	WIPMesh mesh;
};

#include "AnimationClip.h"
#include "AnimationQueue.h"
#include "FrameBox.h"
#include "AnimationManager.h"

//make a clip instance according a given clip
class WIPAnimation : public WIPComponent
{
public:
	WIPAnimation() :_speed(1.f), _last_clip(nullptr){}
	~WIPAnimation()
	{
		if(_internal_clip_queue.empty())
			return;
		_Internal_clip_queue::iterator it = _internal_clip_queue.begin();
		for(;it!=_internal_clip_queue.end();++it)
		{
			delete it->second;
		}
		_internal_clip_queue.clear();
		//_internal_clip_queue.swap(_Internal_clip_queue());
	}
	inline void add_clip(WIPAnimationClip* clip,std::string name)
	{
		_internal_clip_queue[name] = new WIPClipInstance(&_framebox_ref,clip);
	}
	//not use
	void remove_clip(WIPAnimationClip* clip)
	{
		g_logger->debug_print(WIP_WARNING,"This function is not in use!");
	}
	void remove_clip(std::string name)
	{
		_internal_clip_queue.erase(name);
	}
	inline bool play(WIPClipInstance* clip_instance,bool loop=false)
	{
		if(clip_instance->bplaying)
			return false;
		if (_last_clip)
			_last_clip->stop_now = true;
		clip_instance->bplaying = false;
		clip_instance->stop_now = false;
		clip_instance->will_stop = false;
		clip_instance->bloop = loop;
		g_animation_manager->add_clip(clip_instance);
		_last_clip = clip_instance;
		return true;
	}
	bool play(WIPAnimationClip* clip,bool loop=false)
	{
		if(!clip)
			return false;
		
		_Internal_clip_queue::iterator it = _internal_clip_queue.begin();
		for(;it!=_internal_clip_queue.end();++it)
		{
			if(it->second->clip_ref==clip)
			{
				break;
			}
		}
		if(it==_internal_clip_queue.end())
		{
			return false;
		}

		if(it->second->bplaying)
			return false;
		if (_last_clip)
			_last_clip->stop_now = true;
		it->second->bplaying = false;
		it->second->stop_now = false;
		it->second->will_stop = false;
		it->second->bloop = loop;
		g_animation_manager->add_clip(it->second);
		_last_clip = it->second;
		return true;
	}
	bool play_name(std::string name, bool loop )
	{
		WIPClipInstance* clip = _internal_clip_queue[name];
		if(clip)
			return play(clip,loop);
		return false;
	}
	bool play(bool loop)
	{
		if (_internal_clip_queue.empty())
			return false;
		WIPClipInstance* clip = (_internal_clip_queue.begin()->second);
		if(clip)
			return play(clip,loop);
		return false;
	}
	inline void stop(WIPClipInstance* clip_ins)
	{
		clip_ins->will_stop = true;
	}
	void stop(WIPAnimationClip* clip)
	{
		_Internal_clip_queue::iterator it = _internal_clip_queue.begin();
		for(;it!=_internal_clip_queue.end();++it)
		{
			if(it->second->clip_ref==clip)
			{
				break;
			}
		}
		if(it==_internal_clip_queue.end())
		{
			return;
		}
		it->second->will_stop = true;
	}
	void stop(std::string name)
	{
		WIPClipInstance* clip = _internal_clip_queue[name];
		if(clip)
			stop(clip);
	}
	void stop()
	{
		if(_internal_clip_queue.empty())
			return;
		WIPClipInstance* clip = (_internal_clip_queue.begin()->second);
		if(clip)
			stop(clip);
	}



	inline void stop_now(WIPClipInstance* clip_ins)
	{
		clip_ins->stop_now = true;
	}
	void stop_now(WIPAnimationClip* clip)
	{
		_Internal_clip_queue::iterator it = _internal_clip_queue.begin();
		for (; it != _internal_clip_queue.end(); ++it)
		{
			if (it->second->clip_ref == clip)
			{
				break;
			}
		}
		if (it == _internal_clip_queue.end())
		{
			return;
		}
		it->second->stop_now = true;
	}
	void stop_now(std::string name)
	{
		WIPClipInstance* clip = _internal_clip_queue[name];
		if (clip)
			stop_now(clip);
	}
	void stop_now()
	{
		if (_internal_clip_queue.empty())
			return;
		WIPClipInstance* clip = (_internal_clip_queue.begin()->second);
		if (clip)
			stop_now(clip);
	}




	inline void rewind(WIPClipInstance* clip_ins)
	{
		stop(clip_ins);
		play(clip_ins);
	}
	void rewind(WIPAnimationClip* clip)
	{
		if(!clip)
			return;
		stop(clip);
		play(clip);
	}
	void rewind(std::string name)
	{
		WIPClipInstance* clip = _internal_clip_queue[name];
		rewind(clip);
	}
	void rewind()
	{
		WIPClipInstance* clip = (_internal_clip_queue.begin()->second);
		rewind(clip);
	}
	void playeQueue(WIPAnimationQueue* queue)
	{
		g_logger->debug_print(WIP_WARNING,"This function is not in use!");
	}

private:
	//--- 1 for normal,can be negtive
	f32 _speed;
public:
	WIPFrameBox _framebox_ref;
private:
	typedef std::map<std::string,WIPClipInstance*> _Internal_clip_queue;
	_Internal_clip_queue _internal_clip_queue;

	WIPClipInstance* _last_clip;

};

class WIPPhysicsManager;
//todo 独立的碰撞mesh
class WIPCollider:public WIPComponent
{
public:
	enum _FrictionTypes
	{
		E_DYNAMIC = 1,
		E_STATIC,
		E_FRICTION_COUNT,
	};

	enum _ShapeTypes
	{
		E_POLYGON = 1,
		E_CIRCLE,
		E_SHAPE_COUNT,
	};

	enum _CollisionTypes
	{
		//
		E_STATIC_RIGIDBODY = 0,
		E_GHOST,
		E_RIGIDBODY,
		E_NOTUSE,
		//参加碰撞检测但是没有物理
		E_NO_PHYSICS,
		//不参加碰撞检测没有物理	
		E_COLIISION_COUNT,
	};

	//now noly support polygon shape
	static WIPCollider* create_collider(WIPSprite* m, WIPCollider::_CollisionTypes tp);
	static WIPCollider* create_collider(std::vector<RBVector2>& poly);
	WIPCollider();
	~WIPCollider();
	//n is vertex number v contains 2*v elements
	void reset_polygon_vertices(WIPSprite* v, i32 n = 4);
	void reset_polygon_position(f32 x, f32 y);
	void reset_polygon_density(f32 density);

	void reset_body_rad(f32 rad);
	void reset_body_restitution(f32 restitution);
	void reset_body_friction(f32 v, _FrictionTypes type);

	void set_mesh_box(f32 sx, f32 sy)
	{
		_cb_scale_x = sx;
		_cb_scale_y = sy;
	}
	void reset_body_type(_CollisionTypes type);

	void add_force(f32 x, f32 y);

	void update_out(WIPMesh& mesh, f32& rad, f32& x, f32 &y);
	void update_in(WIPMesh* mesh, f32 rad, f32 x, f32 y);

	i32 get_collision_list_size();

	b2Body* get_collision_index_body(i32 i);

	bool is_collision_list_empty();

	void set_density(f32 density);

	f32 get_speed_x();
	f32 get_speed_y();
	void set_active(bool v);
	void set_sprite(class WIPSprite* sprite);
protected:


private:
	friend WIPPhysicsManager;
	friend WIPSprite;
	b2PolygonShape* _polygon_shape;
	b2Body* _body;
	bool _active;
	f32 _cb_scale_x;
	f32 _cb_scale_y;
};


class WIPTransform : public WIPComponent
{
public:
	~WIPTransform(){}
	f32 world_x,world_y;
	f32 z_order;
	//CCW is positive
	//rad
	f32 rotation;
	f32 scale_x,scale_y;
	f32 anchor_x;
	f32 anchor_y;
	WIPTransform():world_x(0.f),world_y(0.f),z_order(0),rotation(0.f),
	scale_x(1.f),scale_y(1.f),anchor_x(0.5f),anchor_y(0.5f){}
};

class WIPScene;

class WIPSprite : public WIPObject
{
public:
	static WIPSprite* create(f32 width, f32 height, WIPCollider::_CollisionTypes tp = WIPCollider::_CollisionTypes::E_STATIC_RIGIDBODY,f32 sx=1.f,f32 sy=1.f)
	{
		WIPSprite* ret = new WIPSprite();
		ret->_transform = new WIPTransform();
		ret->_render = new WIPRenderComponent(width,height);
		ret->_animation = new WIPAnimation();
		if (tp != 4)
		{
			ret->_collider = WIPCollider::create_collider(ret, tp);
			ret->_collider->set_sprite(ret);
			ret->_collider->set_mesh_box(sx,sy);
		}
		else
		{
			ret->_collider = nullptr;
		}
		ret->_tag = "NONAME";
		ret->_type_tag = "NOTAG";
		return ret;
	}
	//use for lua
	static WIPSprite* create()
	{
		return nullptr;
	}
	static void destroy(WIPSprite* sprite)
	{
		delete sprite->_transform;
		delete sprite->_render;
		delete sprite->_animation;
		delete sprite->_collider;
		delete sprite;
	}
private:
	WIPSprite()
	{
	}
	~WIPSprite();

public:
	
	void set_z_order(float z)
	{
		_transform->z_order = z;
	}
	void set_anchor(f32 percentx,f32 percenty)
	{
		_transform->anchor_x = percentx;
		_transform->anchor_y = percenty;
	}
	//force set a texture to _frame and at the same time set the full texture as the render result
	void set_texture(WIPTexture2D* texture)
	{
		_render->material.texture = texture;
	}

	//the anchor position is also the world 

	//Transform functions
	void rotate(f32 drad)
	{
		_transform->rotation += drad;
		update_world();
	}
	void rotate_to(f32 rad)
	{
		_transform->rotation = rad;
		update_world();
	}

	void translate(f32 dx,f32 dy)
	{
		_transform->world_x += dx;
		_transform->world_y += dy;
		update_world();
	}
	void translate_to(f32 x,f32 y)
	{
		_transform->world_x = x;
		_transform->world_y = y;		
		update_world();
	}
	//scale with anchor
	void scale(f32 dsx, f32 dsy)
	{
		_transform->scale_x += dsx;
		_transform->scale_y += dsy;
		update_world();
	}
	void scale_to(f32 sx,f32 sy)
	{
		_transform->scale_x = sx;
		_transform->scale_y = sy;
		update_world();
	}

	void update_world();


	void set_tag(std::string s)
	{
		_tag = s;
	}
	inline std::string get_tag()
	{
		return _tag;
	}
	void set_type_tag(std::string s)
	{
		_type_tag = s;
	}
	inline std::string get_type_tag()
	{
		return _type_tag;
	}
	RBVector2 get_anchor_world_position()
	{
		return RBVector2(_transform->world_x,_transform->world_y);
	}
	//lt lb rt rb,regard anchor as center
	//mesh regard 0.5,0.5 as center
	void get_anchor_vertices(RBVector2* vertices) const
	{
		if (!vertices)
			return;

		f32 ax = (_transform->anchor_x - 0.5f)*_render->mesh.get_witdh();
		f32 ay = (_transform->anchor_y - 0.5f)*_render->mesh.get_height();
		RBVector2 lb1 = _render->mesh.lb -RBVector2(ax, ay);
		RBVector2 lt1 = _render->mesh.lt -RBVector2(ax, ay);
		RBVector2 rb1 = _render->mesh.rb -RBVector2(ax, ay);
		RBVector2 rt1 = _render->mesh.rt -RBVector2(ax, ay);

		vertices[0] = lt1;
		vertices[1] = lb1;
		vertices[2] = rt1;
		vertices[3] = rb1;
	}
	//lt lb rb rt
	void get_world_position(RBVector2* vertices) const
	{
		if(!vertices)
			return;
		f32 scale_x = _transform->scale_x;
		f32 scale_y = _transform->scale_y;
		f32 tworld_x = _transform->world_x ;//-  _transform->anchor_x*_render->mesh.get_witdh();
		f32 tworld_y = _transform->world_y ;//-  _transform->anchor_y*_render->mesh.get_height();
		f32 rotation = _transform->rotation;

		f32 ax = (_transform->anchor_x-0.5f)*_render->mesh.get_witdh();
		f32 ay = (_transform->anchor_y-0.5f)*_render->mesh.get_height();
		RBVector2 lb1 = _render->mesh.lb - RBVector2(ax,ay);
		RBVector2 lt1 = _render->mesh.lt - RBVector2(ax,ay);
		RBVector2 rb1 = _render->mesh.rb - RBVector2(ax,ay);
		RBVector2 rt1 = _render->mesh.rt - RBVector2(ax,ay);

		RBVector2 lb1r,lt1r,rb1r,rt1r;
		#define _cos RBMath::cos
		#define _sin RBMath::sin
		lt1r.y = lt1.x*_sin(rotation) + lt1.y*_cos(rotation);
		lt1r.x = lt1.x*_cos(rotation) - lt1.y*_sin(rotation);

		lb1r.y = lb1.x*_sin(rotation) + lb1.y*_cos(rotation);
		lb1r.x = lb1.x*_cos(rotation) - lb1.y*_sin(rotation);

		rb1r.y = rb1.x*_sin(rotation) + rb1.y*_cos(rotation);
		rb1r.x = rb1.x*_cos(rotation) - rb1.y*_sin(rotation);

		rt1r.y = rt1.x*_sin(rotation) + rt1.y*_cos(rotation);
		rt1r.x = rt1.x*_cos(rotation) - rt1.y*_sin(rotation);
		#undef _cos
		#undef _sin

		lt1r.x *= scale_x;
		lb1r.x *= scale_x;
		rt1r.x *= scale_x;
		rb1r.x *= scale_x;
		lt1r.y *= scale_y;
		lb1r.y *= scale_y;
		rt1r.y *= scale_y;
		rb1r.y *= scale_y;

		lt1r+=RBVector2(tworld_x,tworld_y);
		lb1r+=RBVector2(tworld_x,tworld_y);
		rt1r+=RBVector2(tworld_x,tworld_y);
		rb1r+=RBVector2(tworld_x,tworld_y);

		vertices[0] = lt1r;
		vertices[1] = lb1r;
		vertices[2] = rt1r;
		vertices[3] = rb1r;

	}
	void add_to_scene(WIPScene* scene);
	void leave_scene(WIPScene* scene);
	void add_component(WIPComponent* c)
	{
		components.push_back(c);
		c->set_host(this);
	}
	void add_tick_component(WIPTickComponent* tc)
	{
		tick_components.push_back(tc);
		tc->set_host(this);
	}


	void update(f32 dt)
	{
		for (auto i : tick_components)
		{
			i->update(dt);
		}
	}
	void fix_update(f32 dt)
	{
		for (auto i : tick_components)
		{
			i->fix_update(dt);
		}
	}

	void init_components()
	{
		for (auto i : tick_components)
		{
			i->init();
		}
	}

	void destroy_components()
	{
		for (auto i : tick_components)
		{
			i->destroy();
		}
	}

public:
	WIPTransform* _transform;
	WIPRenderComponent* _render;
	WIPAnimation* _animation;
	WIPCollider* _collider;

	std::string _tag;
	std::vector<WIPScene*> related_scenes;
	//todo:hash
	std::string _type_tag;

	std::vector<WIPComponent* > components;
	std::vector<WIPTickComponent* > tick_components;

};

struct WIPSpriteCreator
{
	WIPSpriteCreator(f32 iw,f32 ih,WIPMaterialType mtp = WIPMaterialType::E_OPAQUE)
		:mt(mtp), world_render(nullptr), texture(nullptr), w(iw), h(ih), body_tp(WIPCollider::_CollisionTypes::E_STATIC_RIGIDBODY),
		collider_sx(1.f), collider_sy(1.f)
	{}
	WIPMaterialType mt;
	WorldRender* world_render;
	WIPTexture2D* texture;
	WIPCollider::_CollisionTypes body_tp;
	f32 collider_sx;
	f32 collider_sy;
	float w, h;
};

class WIPSpriteFactory
{
public:
	static WIPSprite* create_sprite(const WIPSpriteCreator& creator);

};
#endif