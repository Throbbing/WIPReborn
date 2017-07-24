#include "Sprite.h"
#ifdef USE
#include "FrameBox.h"
#include "SpriteFrame.h"
#include "Mesh.h"
#include "Animation.h"
#include "Texture.h"
#include "Collider.h"
#include "Animator.h"
#include "RenderTexture.h"
#include "Assertion.h"

WIPSprite *WIPSprite::create(f32 width,f32 height)
{
	WIPSprite* ret = new WIPSprite();
	ret->_render = new WIPRenderComponent();
	ret->_transform = new WIPTransform();
	//ret->_frame = new WIPSpriteFrame();
	//ret->_frame->framebox = new WIPFrameBox();
	//ret->_frame->texture = 0;
	ret->_animation = new WIPAnimation(ret->_frame);
	ret ->_collider = 0;
	ret->_animator = NULL;


	ret->translate(0.f,0.f);
	ret->set_anchor(0.f,0.f);
	return ret;
	
}

WIPSprite *WIPSprite::create()
{
	WIPSprite* ret = new WIPSprite();
	ret->_origin_mesh = 0;
	ret->_copy_mesh = 0;
	ret->_frame = new WIPSpriteFrame();
	ret->_frame->framebox = new WIPFrameBox();
	ret->_frame->texture = 0;
	ret->_animation_controller = 0;
	ret->_collider = 0;

	ret->_animator = 0;


	ret->translate(0.f,0.f);
	ret->set_anchor(0.f,0.f);
	ret->_pak.framebox = ret->_frame->framebox;
	ret->_pak.z_order = ret->z_order;
	ret->_pak.tex_id = 0;
	return ret;
}

void WIPSprite::destroy(WIPSprite* sprite)
{

}

WIPSprite::WIPSprite()
{
	_transform = nullptr;
	_render = nullptr;
	_animation = nullptr;
	_collider = nullptr;
	_tag = "NONAME";
}

WIPSprite::~WIPSprite()
{

}

void WIPSprite::set_z_order(i32 z)
{
	transform->z_order = z;
}

void WIPSprite::set_anchor(f32 percentx,f32 percenty)
{
	f32 w = _origin_mesh->rb.x - _origin_mesh->lt.x;
	f32 h = _origin_mesh->rb.y - _origin_mesh->lt.y;
	f32 x = w * percentx;
	f32 y = h * percenty;
	_copy_mesh->anchor.x = x - w*0.5f;
	_copy_mesh->anchor.y = y - h*0.5;
}

void WIPSprite::set_texture(WIPTexture* texture)
{
	_frame->texture = texture;
	_default_texture = texture;
	_pak.tex_id = texture->get_texture_id();
	fill_static_texture_uv();
}

void WIPSprite::fill_static_texture_uv()
{
	_pak.framebox->set_quickly(0.f,1.f,0.f,0.f,1.f,0.f,1.f,1.f,1.f,1.f);
}

void WIPSprite::rotate(f32 rad)
{
	rotation += rad;
	if(RBMath::is_nearly_equal(rotation,2*PI))
		rotation = 0.f;
	if(rotation>=2*PI)
		rotation -= 2*PI;
}

void WIPSprite::rotate_to(f32 rad)
{
	rotation = rad;
	if(rotation>=2*PI)
		rotation -= 2*PI;
}

void WIPSprite::translate(f32 x,f32 y)
{
	world_x += x;
	world_y += y;

}

void WIPSprite::translate_to(f32 x,f32 y)
{
	world_x = x;
	world_y = y;
}

void WIPSprite::scale(f32 sx,f32 sy)
{
	scale_x *= sx;
	scale_y *= sy;
}

void WIPSprite::scale_to(f32 sx,f32 sy)
{
	scale_x = sx;
	scale_y = sy;
}

void WIPSprite::reset_mesh()
{
	*_copy_mesh = *_origin_mesh;

}

void WIPSprite::reset_origin_mesh(WIPMesh* mesh)
{
	CHECK(mesh!=0);
	_origin_mesh = mesh;
	if(_copy_mesh)
		delete _copy_mesh;
	_copy_mesh = new WIPMesh(*_origin_mesh);

	if(_animator)
		_animator->get_buffer()->resize(mesh->rb.x-mesh->lb.x,mesh->rt.y-mesh->rb.y);
}

void WIPSprite::reset_animaton(WIPAnimation* ani)
{
	if(_animation_controller)
		delete _animation_controller;
	_animation_controller = ani;
	ani->reset_frame_ref(_frame);
}

void WIPSprite::reset_collider(WIPCollider* collider)
{
	if(_collider)
		delete _collider;
	_collider = collider;
	_collider->reset_polygon_vertices(this->_copy_mesh);
}

void WIPSprite::set_tag(std::string s)
{
	_tag = s;
}

std::string WIPSprite::get_tag()
{
	return _tag;
}
#endif

#include "Scene.h"

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
	WIPSprite* s = WIPSprite::create(creator.w,creator.h);
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