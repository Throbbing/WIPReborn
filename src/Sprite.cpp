#include "Sprite.h"
#include "Scene.h"

WIPComponent::WIPComponent() :host_object(nullptr){}
void WIPComponent::set_host(WIPSprite* ho)
{
	host_object = ho;
}

WIPComponent::WIPComponent(WIPSprite* s) :host_object(s){}

//every subclass call base destructor so this must be defined.
WIPComponent::~WIPComponent(){}

WIPTickComponent::WIPTickComponent(WIPSprite* host) :WIPComponent(host){}

WIPTickComponent::~WIPTickComponent(){}


WIPSprite::~WIPSprite()
{

}

void WIPSprite::destroy(WIPSprite* s)
{
	for (auto i : s->related_scenes)
	{
		i->remove_sprite(s, false);
	}
	s->related_scenes.clear();
	//删除组件前移除所有依赖！！
	delete s->_transform;
	delete s->_collider;
	delete s->_render;
	delete s->_animation;
	for (int i = 0; i < s->components.size(); ++i)
	{
		delete s->components[i];
	}
	for (int i = 0; i < s->tick_components.size(); ++i)
	{
		delete s->tick_components[i];
	}
	delete s;
}

void WIPSprite::add_to_scene(WIPScene* scene)
{
	related_scenes.push_back(scene);
}

void WIPSprite::update_world()
{
	for (int i = 0; i < related_scenes.size(); ++i)
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



WIPSprite* WIPSpriteFactory::create_sprite(const WIPSpriteCreator& creator)
{
	WIPSprite* s = WIPSprite::create(creator.w,creator.h,creator.body_tp,creator.collider_sx,creator.collider_sy);
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