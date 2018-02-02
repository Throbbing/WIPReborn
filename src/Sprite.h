#pragma once

#ifndef USE

#include "Render.h"
#include <string>
#include <map>
#include "Box2D/Box2D.h"
#include "Object.h"
#include "RefCountPtr.h"
//message dispatch
/*
class WIPObject1
{
public:

};
*/



enum class NormalType
{
	E_NOT_USE,
	E_VERTEX_NORMAL,
	E_TEXTURE_NORMAL
};

class WIPMaterialNormal : public FRefCountedObject
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
class WIPMaterial : public FRefCountedObject
{
public:
	WIPMaterial();
	WIPTexture2D* texture;
	WIPMaterialNormal* material_normal;
	RBColorf vert_color[4];
	WIPBoundShader* shader;

	WIPMaterialType material_type;
	
};


class WIPMaterialManager : public FRefCountedObject
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
class WIPMesh : public FRefCountedObject
{
public:
	WIPMesh(f32 w, f32 h);
	WIPMesh();
	//WIPMesh(const WIPMesh&);
	RBVector2 lt,lb,rb,rt;
	float get_witdh();
	float get_height();
	//copy
	//these will be remove
	/*
	RBVector2 ltc,lbc,rbc,rtc;
	RBVector2 anchorc;
	*/
	void set_quickly
		(
		float lt_x1, float lt_y1,
		float lb_x2, float lb_y2,
		float rb_x3, float rb_y3,
		float rt_x4, float rt_y4
		);


private:
	
};

//对基类重载new对子类依然生效，尺寸正确，但是此系统每个类都有一个header，大小不一样，不能分配到基类同一个连表上
//system component
class WIPComponent : public WIPObject
{
public:
  WIPOBJECT(WIPComponent, WIPObject);
	WIPComponent(WIPSprite* s);
	WIPComponent();
	virtual ~WIPComponent() = 0;
	//this is a weak ref!
	WIPSprite* host_object;
	void set_host(TRefCountPtr<WIPSprite> ho);
};

//user component
class WIPTickComponent : public WIPComponent
{
public:
	WIPOBJECT(WIPTickComponent, WIPComponent);
	WIPTickComponent(TRefCountPtr<WIPSprite> host);
	virtual ~WIPTickComponent() = 0;
	virtual void init() = 0;
	virtual void on_begin_contact(const WIPSprite* s) {}
	virtual void on_contact(const WIPSprite* s) {}
	virtual void on_end_contact(const WIPSprite* s) {}

	virtual void update(f32 dt)=0;
	virtual void fix_update(f32 dt)=0;
	virtual void destroy() = 0;

	//level start/end
	virtual void start(){}
	virtual void end(){}
};

class WIPRenderComponent : public WIPComponent
{
public:
  WIPOBJECT(WIPRenderComponent, WIPComponent);
  WIP_MEM(WIPRenderComponent);

	~WIPRenderComponent(){}
	WIPRenderComponent(float w, float h) :mesh(w, h), is_visible(true){}
	WIPMaterial material;
	WIPMesh mesh;
	bool is_visible;
};


#include "AnimationClip.h"
#include "FrameBox.h"
//make a clip instance according a given clip
class WIPAnimation : public WIPComponent
{
public:
  WIPOBJECT(WIPAnimation, WIPComponent);
  WIP_MEM(WIPAnimation);
  WIPAnimation();
	~WIPAnimation();
	void add_clip(WIPAnimationClip* clip, std::string name);
	void add_clip_callback(std::string name, WIPClipInstance::clip_callback_t cb, void* o);
	void clear_clip_callback(std::string name);
	//not use
	void remove_clip(WIPAnimationClip* clip);
	void remove_clip(std::string name);
	void set_clip_instance_speed(std::string name, f32 speed);
	bool play(WIPClipInstance* clip_instance, bool loop = false);
	bool play(WIPAnimationClip* clip, bool loop = false);
	bool play_name(std::string name, bool loop);
	bool play(bool loop);
	void stop(WIPClipInstance* clip_ins);
	void stop(WIPAnimationClip* clip);
	void stop(std::string name);
	void stop();
	void stop_now(WIPClipInstance* clip_ins);
	void stop_now(WIPAnimationClip* clip);
	void stop_now(std::string name);
	void stop_now();
	void rewind(WIPClipInstance* clip_ins);
	void rewind(WIPAnimationClip* clip);
	void rewind(std::string name);
	void rewind();
	void playeQueue(class WIPAnimationQueue* queue);

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
  WIPOBJECT(WIPCollider, WIPComponent);
  WIP_MEM(WIPCollider);

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
		E_NO_PHYSICS,
		E_COLIISION_COUNT,
	};

	//now noly support polygon shape
	static WIPCollider* create_collider(TRefCountPtr<WIPSprite> m, WIPCollider::_CollisionTypes tp,f32 sx=1.f,f32 sy=1.f);
	static WIPCollider* create_collider(std::vector<RBVector2>& poly);
	WIPCollider();
	~WIPCollider();
	void destroy();
	//n is vertex number v contains 2*v elements
	void reset_polygon_vertices(TRefCountPtr<WIPSprite> v, i32 n = 4);
	void reset_polygon_position(f32 x, f32 y);
	void reset_polygon_density(f32 density);
	void recreate_fixture();

	void reset_body_rad(f32 rad);
	void reset_body_restitution(f32 restitution);
	void reset_body_friction(f32 v, _FrictionTypes type);
	//set collider scale(collider_scale = sprite_scale*cb_scale)
	inline void set_mesh_box(f32 sx, f32 sy)
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
	void set_sprite( TRefCountPtr<WIPSprite> sprite);

	FORCEINLINE b2Body* get_body(){ return _body; }
	//these functions can be called saveral times!
	void on_begin_contact(const WIPSprite* contact);
	void on_end_contact(const WIPSprite* contact);
	
protected:


private:
	
	HashLink<WIPSprite> _contact_objects;
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
	WIPOBJECT(WIPTransform, WIPComponent);
	~WIPTransform(){}
	f32 world_x, world_y;
	f32 z_order;
	//CCW is positive
	//rad
	f32 rotation;
	f32 scale_x, scale_y;
	f32 anchor_x;
	f32 anchor_y;
	RBVector2 vertices_cache[4];
	WIPTransform() :world_x(0.f), world_y(0.f), z_order(0), rotation(0.f),
		scale_x(1.f), scale_y(1.f), anchor_x(0.5f), anchor_y(0.5f){}
};

class WIPScene;
//#include "RefCountPtr.h"
class WIPSprite : public WIPObject
{
public:
  WIPOBJECT(WIPSprite,WIPObject);
  WIP_MEM(WIPSprite)
  static TRefCountPtr<WIPSprite> create(f32 width, f32 height, WIPCollider::_CollisionTypes tp = WIPCollider::_CollisionTypes::E_STATIC_RIGIDBODY, f32 sx = 1.f, f32 sy = 1.f);
	static void destroy(TRefCountPtr<WIPSprite> s);
	//use for lua
	static TRefCountPtr<WIPSprite> create()
	{
		return nullptr;
	}
	//连接到lua对象，以便可以从C++快速访问到对应的lua对象
	void link_lua_object(const char* tname);
	//销毁时务必先解除link，因为没有使用弱引用
	void unlink_lua_object();
	int key;
	static int cur_id;
	static std::vector<int> ids;
private:

	

	WIPSprite(){}
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
		if (_collider)
			update_collider();
	}
	void scale_to(f32 sx,f32 sy)
	{
		_transform->scale_x = sx;
		_transform->scale_y = sy;
		update_world();
		if (_collider)
			update_collider();
	}

	void update_world();
	//if scale you must scale collider on the same time
	void update_collider();

	void set_tag(std::string s)
	{
		_tag = s;
	}
	inline const std::string& get_tag() const
	{
		return _tag;
	}
	void set_type_tag(std::string s)
	{
		_type_tag = s;
	}
	inline const std::string& get_type_tag() const
	{
		return _type_tag;
	}
	RBVector2 get_anchor_world_position()
	{
		return RBVector2(_transform->world_x,_transform->world_y);
	}
	//lt lb rt rb,regard anchor as center
	//mesh regard 0.5,0.5 as center
	void get_anchor_vertices(RBVector2* vertices) const;
	//lt lb rb rt
	void get_world_position(RBVector2* vertices) const;
	void cache_world_position();
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

	
	void on_contact();

	void update(f32 dt);
	void fix_update(f32 dt);
	//create
	void init_components();
	//level start
	void start_components();
	//level end
	void end_components();
	//life end
	void destroy_components();

	void destroy_self();

	WIPComponent* get_component_by_name(const char* name) const;
	template<class T>
	WIPComponent* get_component() const
	{
		for (auto i : tick_components)
		{
			if (i->is_type_of<T>())
			{
				return i;
			}
		}
		for (auto i : components)
		{
			if (i->is_type_of<T>())
			{
				return i;
			}
		}
		return nullptr;
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

	std::string _lua_name="";
};

struct WIPSpriteCreator
{
	WIPSpriteCreator(f32 iw,f32 ih,WIPMaterialType mtp = WIPMaterialType::E_OPAQUE)
		:mt(mtp), world_render(nullptr), texture(nullptr), w(iw), h(ih), body_tp(WIPCollider::_CollisionTypes::E_GHOST),
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
	static TRefCountPtr<WIPSprite> create_sprite(const WIPSpriteCreator& creator);

};


#endif