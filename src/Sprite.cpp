#include "Sprite.h"
#include "Scene.h"

#include "AnimationQueue.h"

#include "AnimationManager.h"

WIPMaterial::WIPMaterial() :texture(nullptr), material_normal(nullptr), shader(nullptr)
{
	vert_color[0] = RBColorf::white;
	vert_color[1] = RBColorf::white;
	vert_color[2] = RBColorf::white;
	vert_color[3] = RBColorf::white;
	material_type = WIPMaterialType::E_OTHER;
}



WIPMesh::WIPMesh(f32 w, f32 h)
{
	set_quickly(-w / 2, h / 2, -w / 2, -h / 2, w / 2, h / 2, w / 2, -h / 2);
}
WIPMesh::WIPMesh()
{
	f32 w = 1;
	f32 h = 1;
	set_quickly(-w / 2, h / 2, -w / 2, -h / 2, w / 2, h / 2, w / 2, -h / 2);
}

float WIPMesh::get_witdh()
{
	return RBMath::abs(rt.x - lt.x);
}
float WIPMesh::get_height()
{
	return RBMath::abs(rt.y - rb.y);
}

void WIPMesh::set_quickly
(
float lt_x1, float lt_y1,
float lb_x2, float lb_y2,
float rb_x3, float rb_y3,
float rt_x4, float rt_y4
)
{
	lt = RBVector2(lt_x1, lt_y1);
	lb = RBVector2(lb_x2, lb_y2);
	rb = RBVector2(rb_x3, rb_y3);
	rt = RBVector2(rt_x4, rt_y4);
}


WIPComponent::WIPComponent() :host_object(nullptr){}
void WIPComponent::set_host(TRefCountPtr<WIPSprite> ho)
{
	host_object = ho.GetReference();
}

WIPComponent::WIPComponent(WIPSprite* s) :host_object(s){}

//every subclass call base destructor so this must be defined.
WIPComponent::~WIPComponent(){}

WIPTickComponent::WIPTickComponent(TRefCountPtr<WIPSprite> host) : WIPComponent(host){}

WIPTickComponent::~WIPTickComponent(){}


WIPAnimation::WIPAnimation() : _speed(1.f), _last_clip(nullptr){}
WIPAnimation::~WIPAnimation()
{
	if (_internal_clip_queue.empty())
		return;
	_Internal_clip_queue::iterator it = _internal_clip_queue.begin();
	for (; it != _internal_clip_queue.end(); ++it)
	{
		g_animation_manager->remove_clip(it->second);
		delete it->second;
	}
	_internal_clip_queue.clear();
	//_internal_clip_queue.swap(_Internal_clip_queue());
}
void WIPAnimation::add_clip(WIPAnimationClip* clip, std::string name)
{
	_internal_clip_queue[name] = new WIPClipInstance(&_framebox_ref, clip);
}
void WIPAnimation::add_clip_callback(std::string name, WIPClipInstance::clip_callback_t cb, void* o)
{
	_internal_clip_queue[name]->cb = cb;
	_internal_clip_queue[name]->obj_ref = o;
}
void WIPAnimation::clear_clip_callback(std::string name)
{
	_internal_clip_queue[name]->cb = nullptr;
	_internal_clip_queue[name]->obj_ref = nullptr;
}
//not use
void WIPAnimation::remove_clip(WIPAnimationClip* clip)
{
	g_logger->debug_print(WIP_WARNING, "This function is not in use!");
}
void WIPAnimation::remove_clip(std::string name)
{
	_internal_clip_queue.erase(name);
}
void WIPAnimation::set_clip_instance_speed(std::string name, f32 speed)
{
	_internal_clip_queue[name]->speed = speed;
}
bool WIPAnimation::play(WIPClipInstance* clip_instance, bool loop)
{
	if (clip_instance->bplaying)
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
bool WIPAnimation::play(WIPAnimationClip* clip, bool loop)
{
	if (!clip)
		return false;

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
		return false;
	}

	if (it->second->bplaying)
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
bool WIPAnimation::play_name(std::string name, bool loop)
{
	_Internal_clip_queue::iterator it = _internal_clip_queue.find(name);
	if (it != _internal_clip_queue.end())
		return play(it->second, loop);
	return false;
}
bool WIPAnimation::play(bool loop)
{
	if (_internal_clip_queue.empty())
		return false;
	WIPClipInstance* clip = (_internal_clip_queue.begin()->second);
	if (clip)
		return play(clip, loop);
	return false;
}
void WIPAnimation::stop(WIPClipInstance* clip_ins)
{
	clip_ins->will_stop = true;
}
void WIPAnimation::stop(WIPAnimationClip* clip)
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
	it->second->will_stop = true;
}
void WIPAnimation::stop(std::string name)
{
	_Internal_clip_queue::iterator it = _internal_clip_queue.find(name);
	if (it != _internal_clip_queue.end())
		stop(it->second);
}
void WIPAnimation::stop()
{
	if (_internal_clip_queue.empty())
		return;
	WIPClipInstance* clip = (_internal_clip_queue.begin()->second);
	if (clip)
		stop(clip);
}

void WIPAnimation::stop_now(WIPClipInstance* clip_ins)
{
	clip_ins->stop_now = true;
}
void WIPAnimation::stop_now(WIPAnimationClip* clip)
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
void WIPAnimation::stop_now(std::string name)
{
	_Internal_clip_queue::iterator it = _internal_clip_queue.find(name);
	if (it != _internal_clip_queue.end())
		stop_now(it->second);
}
void WIPAnimation::stop_now()
{
	if (_internal_clip_queue.empty())
		return;
	WIPClipInstance* clip = (_internal_clip_queue.begin()->second);
	if (clip)
		stop_now(clip);
}

void WIPAnimation::rewind(WIPClipInstance* clip_ins)
{
	stop(clip_ins);
	play(clip_ins);
}
void WIPAnimation::rewind(WIPAnimationClip* clip)
{
	if (!clip)
		return;
	stop(clip);
	play(clip);
}
void WIPAnimation::rewind(std::string name)
{
	_Internal_clip_queue::iterator it = _internal_clip_queue.find(name);
	if (it != _internal_clip_queue.end())
		rewind(it->second);
}
void WIPAnimation::rewind()
{
	WIPClipInstance* clip = (_internal_clip_queue.begin()->second);
	rewind(clip);
}
void WIPAnimation::playeQueue(WIPAnimationQueue* queue)
{
	g_logger->debug_print(WIP_WARNING, "This function is not in use!");
}



WIPSprite::~WIPSprite()
{
	//LOG_NOTE("ssss");
	//删除组件前移除所有依赖！！
	delete _transform;
	delete _collider;
	delete _render;
	delete _animation;
	for (size_t i = 0; i < components.size(); ++i)
	{
		delete components[i];
	}
	for (size_t i = 0; i < tick_components.size(); ++i)
	{
		delete tick_components[i];
	}
}

void WIPSprite::destroy(TRefCountPtr<WIPSprite> s)
{
	for (auto i : s->related_scenes)
	{
		i->remove_sprite(s, false);
	}
	s->related_scenes.clear();
	s->_collider->destroy();
	//删除组件前移除所有依赖！！
	//delete s->_transform;
	//delete s->_collider;
	//delete s->_render;
	//delete s->_animation;
	for (size_t i = 0; i < s->components.size(); ++i)
	{
		s->destroy_components();
		//delete s->components[i];
	}
	for (size_t i = 0; i < s->tick_components.size(); ++i)
	{
		s->destroy_components();
		//delete s->tick_components[i];
	}
	ids.push_back(s->key);
	//delete s;
}

void WIPSprite::add_to_scene(WIPScene* scene)
{
	related_scenes.push_back(scene);
}

void WIPSprite::update_world()
{
	cache_world_position();
	for (size_t i = 0; i < related_scenes.size(); ++i)
	{
		related_scenes[i]->update_sprite(this);
	}
}

void WIPSprite::leave_scene(WIPScene* scene)
{
	std::vector<WIPScene*>::iterator  it;
	for (it = related_scenes.begin(); it != related_scenes.end(); ++it)
	{
		if (*it == scene)
		{
			break;
		}
	}
	if (it != related_scenes.end())
	{
		related_scenes.erase(it);
	}
	return;
}



TRefCountPtr<WIPSprite> WIPSpriteFactory::create_sprite(const WIPSpriteCreator& creator)
{
	TRefCountPtr<WIPSprite> s = WIPSprite::create(creator.w, creator.h, creator.body_tp, creator.collider_sx, creator.collider_sy);
	s->_render->material.material_type = creator.mt;
	switch (creator.mt)
	{
	case WIPMaterialType::E_OPAQUE:
		s->_render->material.shader = creator.world_render->bound_shader_opaque;

		break;
	case WIPMaterialType::E_TRANSLUCENT:
		s->_render->material.shader = creator.world_render->bound_shader_translucent;

		break;
	case WIPMaterialType::E_ALPHAMASK:
		s->_render->material.shader = creator.world_render->bound_shader_mask;

		break;
	default:

		break;
	}
	s->_render->material.texture = creator.texture;
	return s;
}

void WIPSprite::destroy_self()
{


}

int WIPSprite::cur_id=0;
std::vector<int> WIPSprite::ids;

TRefCountPtr<WIPSprite> WIPSprite::create(f32 width, f32 height, WIPCollider::_CollisionTypes tp, f32 sx, f32 sy)
{
	TRefCountPtr<WIPSprite> ret = new WIPSprite();
	ret->_transform = new WIPTransform();
	ret->_render = new WIPRenderComponent(width, height);
	ret->_animation = new WIPAnimation();
	if (tp != 4)
	{
		ret->_collider = WIPCollider::create_collider(ret, tp,sx,sy);
		ret->_collider->set_sprite(ret);
		
	}
	else
	{
		ret->_collider = nullptr;
	}
	ret->_tag = "NONAME";
	ret->_type_tag = "NOTAG";
	//assign an id
	int id = -1;
	if (ids.size() > 0)
	{
		id = ids.back();
		ids.pop_back();
	}
	else
	{
		id = cur_id++;
	}
	ret->key = id;
	return ret;
}

void WIPSprite::update_collider()
{
	_collider->reset_polygon_vertices(this);
	_collider->recreate_fixture();
}

void WIPSprite::get_anchor_vertices(RBVector2* vertices) const
{
	if (!vertices)
		return;

	f32 ax = (_transform->anchor_x - 0.5f)*_render->mesh.get_witdh();
	f32 ay = (_transform->anchor_y - 0.5f)*_render->mesh.get_height();
	RBVector2 lb1 = _render->mesh.lb - RBVector2(ax, ay);
	RBVector2 lt1 = _render->mesh.lt - RBVector2(ax, ay);
	RBVector2 rb1 = _render->mesh.rb - RBVector2(ax, ay);
	RBVector2 rt1 = _render->mesh.rt - RBVector2(ax, ay);

	vertices[0] = lt1;
	vertices[1] = lb1;
	vertices[2] = rt1;
	vertices[3] = rb1;
}

WIPComponent* WIPSprite::get_component_by_name(const char* name) const
{
	int name_hash = get_string_hash(name);
	for (auto i : tick_components)
	{
		if (i->get_type() == name_hash)
		{
			return i;
		}
	}
	for (auto i : components)
	{
		if (i->get_type() == name_hash)
		{
			return i;
		}
	}
	return nullptr;
}



void WIPSprite::cache_world_position()
{
	
	f32 scale_x = _transform->scale_x;
	f32 scale_y = _transform->scale_y;
	f32 tworld_x = _transform->world_x;//-  _transform->anchor_x*_render->mesh.get_witdh();
	f32 tworld_y = _transform->world_y;//-  _transform->anchor_y*_render->mesh.get_height();
	f32 rotation = _transform->rotation;

	f32 ax = (_transform->anchor_x - 0.5f)*_render->mesh.get_witdh();
	f32 ay = (_transform->anchor_y - 0.5f)*_render->mesh.get_height();
	RBVector2 lb1 = _render->mesh.lb - RBVector2(ax, ay);
	RBVector2 lt1 = _render->mesh.lt - RBVector2(ax, ay);
	RBVector2 rb1 = _render->mesh.rb - RBVector2(ax, ay);
	RBVector2 rt1 = _render->mesh.rt - RBVector2(ax, ay);

	RBVector2 lb1r, lt1r, rb1r, rt1r;
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

	lt1r += RBVector2(tworld_x, tworld_y);
	lb1r += RBVector2(tworld_x, tworld_y);
	rt1r += RBVector2(tworld_x, tworld_y);
	rb1r += RBVector2(tworld_x, tworld_y);

	_transform->vertices_cache[0] = lt1r;
	_transform->vertices_cache[1] = lb1r;
	_transform->vertices_cache[2] = rt1r;
	_transform->vertices_cache[3] = rb1r;
}

void WIPSprite::get_world_position(RBVector2* vertices) const
{
	if (!vertices)
		return;
	memcpy(vertices, _transform->vertices_cache, sizeof(RBVector2) * 4);
}


void WIPSprite::on_contact()
{
	auto* it = _collider->_contact_objects.head();
	while (it)
	{
		for (auto j : tick_components)
		{
			j->on_contact(it->data);
		}
		it = it->next;
	}
}

void WIPSprite::update(f32 dt)
{
	if (_collider&&!_collider->_contact_objects.empty())
		on_contact();
	for (auto i : tick_components)
	{
		i->update(dt);
	}
}
void WIPSprite::fix_update(f32 dt)
{
	for (auto i : tick_components)
	{
		i->fix_update(dt);
	}
}

void WIPSprite::init_components()
{
	for (auto i : tick_components)
	{
		i->init();
	}
}

void WIPSprite::start_components()
{
	for (auto i : tick_components)
	{
		i->start();
	}
}

void WIPSprite::end_components()
{
	for (auto i : tick_components)
	{
		i->end();
	}
}

void WIPSprite::destroy_components()
{
	for (auto i : tick_components)
	{
		i->destroy();
	}
}

#include "ScriptManager.h"

void WIPSprite::link_lua_object(const char* tname)
{
	CHECK(_lua_name == "");
	_lua_name = tname;
}


void WIPSpriteunlink_lua_object()
{

}