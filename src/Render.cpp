#include "Render.h"

#ifdef USE_D3D11
#include "D3D11/D3D11RHI.h"
#else
#include "OpenGL/OpenGLRHI.h"
#endif

WIPDynamicRHI *g_rhi = WIPDynamicRHI::get_rhi();

WIPDynamicRHI *WIPDynamicRHI::get_rhi() {
#ifndef USE_D3D11
  return new GLDynamicRHI();
#else
  return new D3D11DynamicRHI();
#endif
}

WIPVertexShader *WIPDynamicRHI::get_vertex_shader(const string &name) {
  auto s = _vertex_shaders.find(name);
  if (s == _vertex_shaders.end()) {
    LOG_WARN("Can't find vertex shader [%s]", name.c_str());
    return nullptr;
  }
  return s->second;
}
WIPPixelShader *WIPDynamicRHI::get_pixel_shader(const string &name) {
  auto s = _pixel_shaders.find(name);
  if (s == _pixel_shaders.end()) {
    LOG_WARN("Can't find pixel shader [%s]", name.c_str());
    return nullptr;
  }
  return s->second;
}

#include "Sprite.h"
#include "Camera.h"
#include  "Scene.h"
#include "./RBMath/Inc/AABB.h"
#include <algorithm>


void WorldRender::init()
{
	scene_ref = nullptr;
	vertex_buffer_size = 1024 * 1024;

	vertex_format = g_rhi->RHICreateVertexFormat();
	vertex_format->add_float_vertex_attribute(3);
	vertex_format->add_float_vertex_attribute(3);
	vertex_format->add_float_vertex_attribute(3);
	vertex_format->add_float_vertex_attribute(2);

	unsigned int indices[] = { 0, 1, 3, 1, 2, 3 };
	index_buffer = g_rhi->RHICreateIndexBuffer(
		6 * sizeof(unsigned int), indices,
		BufferType::E_STATIC_DRAW);
	vertex_buffer = g_rhi->RHICreateVertexBuffer(
		vertex_buffer_size, 0, BufferType::E_DYNAMIC_DRAW);

	cpu_vertex_buffer = new unsigned char[vertex_buffer_size];

	pack_mem = new float[sizeof(float) * 44];

	auto wvs = g_rhi->get_vertex_shader("simple_vs");
	auto wps1 = g_rhi->get_pixel_shader("opaque");
	auto wps2 = g_rhi->get_pixel_shader("translucent");
	auto wps3 = g_rhi->get_pixel_shader("alpha_mask");

	bound_shader_opaque = g_rhi->RHICreateBoundShader(wvs, wps1);
	bound_shader_translucent = g_rhi->RHICreateBoundShader(wvs, wps2);
	bound_shader_mask = g_rhi->RHICreateBoundShader(wvs, wps3);

}
void WorldRender::set_world(const WIPScene* scene)
{
	scene_ref = scene;
}
void WorldRender::render(WIPCamera* cam)
{
	if (!scene_ref)
		return;
	g_rhi->set_vertex_buffer(vertex_buffer);
	g_rhi->set_index_buffer(index_buffer);
	g_rhi->set_vertex_format(vertex_format);
	_pre_texture = nullptr;
	culling(cam);
	sort_by_texture();
	sort_by_zorder();
	g_rhi->enable_depth_test();
	//render opaque
	if (!opaque_objects.empty())
	{
		g_rhi->set_shader(bound_shader_opaque);
		bool change_t = false;
		int res = 0;
		while (res != opaque_objects.size())
		{
			void *p = g_rhi->lock_vertex_buffer(vertex_buffer);
			int old_res = res;
			res = _pack_sprites_opaque(p, opaque_objects.size() - res,
				res, *cam, change_t);
			g_rhi->unlock_vertex_buffer(vertex_buffer);
			
			if (res - old_res > 0)
			{
				p = g_rhi->lock_index_buffer(index_buffer);
				_pack_index(p, res - old_res);
				g_rhi->unlock_index_buffer(index_buffer);
				g_rhi->draw_triangles((res - old_res) * 6, 0);
			}
			if (change_t)
			{
				g_rhi->set_uniform_texture("in_texture", 0, _pre_texture);
				change_t = false;
			}
		}
	}
	g_rhi->enable_blend();
	g_rhi->set_blend_function();
	g_rhi->set_depth_write(false);
	_pre_texture = nullptr;
	//render alpha
	if (!blend_objects.empty())
	{
		g_rhi->set_shader(bound_shader_translucent);
		bool change_t = false;
		int res = 0;
		while (res != blend_objects.size())
		{
			void *p = g_rhi->lock_vertex_buffer(vertex_buffer);
			int old_res = res;
			res = _pack_sprites_blend(p, blend_objects.size() - res,
				res, *cam, change_t);
			g_rhi->unlock_vertex_buffer(vertex_buffer);
			if (res - old_res > 0)
			{
				p = g_rhi->lock_index_buffer(index_buffer);
				_pack_index(p, res - old_res);
				g_rhi->unlock_index_buffer(index_buffer);
				g_rhi->draw_triangles((res - old_res) * 6, 0);
			}
			if (change_t)
			{
				g_rhi->set_uniform_texture("in_texture", 0, _pre_texture);
				change_t = false;
			}
		}
	}
	g_rhi->set_depth_write(true);
	opaque_objects.clear();
	blend_objects.clear();
}
void WorldRender::destroy()
{
	delete[] cpu_vertex_buffer;
	delete[] pack_mem;
}
void WorldRender::culling(const WIPCamera* cam)
{
	RBAABB2D cam_bound;
	cam_bound.reset();
	float hw = cam->world_w*0.5f*cam->_zoom;
	float hh = cam->world_w*0.5f*cam->_zoom;

	cam_bound.include(RBVector2(cam->world_x - hw, cam->world_y - hh));
	cam_bound.include(RBVector2(cam->world_x + hw, cam->world_y + hh));

	vector<const WIPSprite*> out_index;
	scene_ref->quad_tree->get_intersected_node(cam_bound, out_index);
	sort(out_index.begin(), out_index.end());
	const WIPSprite* pre = nullptr;
	for (int i = 0; i < out_index.size(); ++i)
	{
		const WIPSprite* s = out_index[i];
		if (s != pre)
		{
			if (s->_render->material.material_type == WIPMaterialType::E_OPAQUE)
			{
				opaque_objects.push_back(s);
			}
			else if (
				s->_render->material.material_type == WIPMaterialType::E_TRANSLUCENT
				)
			{
				blend_objects.push_back(s);
			}
			else
			{
				blend_objects.push_back(s);
				//LOG_WARN("Can't render this material type!");
			}
			pre = out_index[i];
		}

	}


}
int WorldRender::_pack_sprites_blend(void *mem, int n, int offset_n, const WIPCamera& camera, bool& change_texture)
{
	const WIPCamera* cam = &camera;
	float *p = (float*)mem;
	float *s = pack_mem;
	int k = -1 + offset_n;
	while (n--)
	{
		k++;

		//03
		//12
		// todo:padding 1 float

		if (!blend_objects[k])
			continue;
		if (blend_objects[k]->_render->material.texture != _pre_texture)
		{
			_pre_texture = blend_objects[k]->_render->material.texture;
			change_texture = true;
			return k;
		}
		/*
		if(sprite[k]->handled)
		continue;
		*/
		float zoom = cam->_zoom;
		RBVector2 vert[4];
		blend_objects[k]->get_world_position(vert);
		RBVector2 cam_pos(cam->world_x, cam->world_y);
		vert[0] = vert[0] - cam_pos;
		vert[1] = vert[1] - cam_pos;
		vert[2] = vert[2] - cam_pos;
		vert[3] = vert[3] - cam_pos;

#ifdef USE_D3D
		//transform for d3d11
#else
		float w1 = 1.f/(cam->world_w*0.5f*zoom);
		float w2 = 1.f/(cam->world_h*0.5f*zoom);
#endif

		s[0] = vert[0].x * w1;
		s[1] = vert[0].y * w2;
		s[2] = blend_objects[k]->_transform->z_order;
		s[3] = blend_objects[k]->_render->material.vert_color[0].r;
		s[4] = blend_objects[k]->_render->material.vert_color[0].g;
		s[5] = blend_objects[k]->_render->material.vert_color[0].b;
		s[6] = 0;
		s[7] = 0;
		s[8] = 1;
		// from framebox
		s[9] = blend_objects[k]->_animation->_framebox_ref.lb.x;//0
		s[10] = blend_objects[k]->_animation->_framebox_ref.lb.y;//0

		s[11] = vert[1].x * w1;
		s[12] = vert[1].y * w2;
		s[13] = blend_objects[k]->_transform->z_order;
		s[14] = blend_objects[k]->_render->material.vert_color[1].r;
		s[15] = blend_objects[k]->_render->material.vert_color[1].g;
		s[16] = blend_objects[k]->_render->material.vert_color[1].b;
		s[17] = 0;
		s[18] = 0;
		s[19] = 1;
		// from framebox
		s[20] = blend_objects[k]->_animation->_framebox_ref.lt.x;//0
		s[21] = blend_objects[k]->_animation->_framebox_ref.lt.y;//1

		s[22] = vert[2].x * w1;
		s[23] = vert[2].y * w2;
		s[24] = blend_objects[k]->_transform->z_order;
		s[25] = blend_objects[k]->_render->material.vert_color[2].r;
		s[26] = blend_objects[k]->_render->material.vert_color[2].g;
		s[27] = blend_objects[k]->_render->material.vert_color[2].b;
		s[28] = 0;
		s[29] = 0;
		s[30] = 1;
		// from framebox
		s[31] = blend_objects[k]->_animation->_framebox_ref.rt.x;//1
		s[32] = blend_objects[k]->_animation->_framebox_ref.rt.y;//1

		s[33] = vert[3].x * w1;
		s[34] = vert[3].y * w2;
		s[35] = blend_objects[k]->_transform->z_order;
		s[36] = blend_objects[k]->_render->material.vert_color[3].r;
		s[37] = blend_objects[k]->_render->material.vert_color[3].g;
		s[38] = blend_objects[k]->_render->material.vert_color[3].b;
		s[39] = 0;
		s[40] = 0;
		s[41] = 1;
		// from framebox
		s[42] = blend_objects[k]->_animation->_framebox_ref.rb.x;//1
		s[43] = blend_objects[k]->_animation->_framebox_ref.rb.y;//1
		memcpy(p, s, sizeof(float) * 44);
		p += 44;
		//LOG_WARN("%d",p-mem);
		if ((int)(p + 44 - (float*)mem) >= vertex_buffer_size / sizeof(float))
		{
			//LOG_WARN("Copy overflow!Break!");
			change_texture = false;
			return k + 1;
		}
	}
	//pack done!
	return blend_objects.size();
}
int WorldRender::_pack_sprites_opaque(void *mem, int n, int offset_n, const WIPCamera& camera, bool& change_texture)
{
	const WIPCamera* cam = &camera;
	float *p = (float*)mem;
	float *s = pack_mem;
	int k = -1 + offset_n;
	while (n--)
	{
		k++;

		//03
		//12
		// todo:padding 1 float

		if (!opaque_objects[k])
			continue;
		if (opaque_objects[k]->_render->material.texture != _pre_texture)
		{
			_pre_texture = opaque_objects[k]->_render->material.texture;
			change_texture = true;
			return k;
		}
		/*
		if(sprite[k]->handled)
		continue;
		*/
		float zoom = cam->_zoom;
		RBVector2 vert[4];
		opaque_objects[k]->get_world_position(vert);
		RBVector2 cam_pos(cam->world_x, cam->world_y);
		vert[0] = vert[0] - cam_pos;
		vert[1] = vert[1] - cam_pos;
		vert[2] = vert[2] - cam_pos;
		vert[3] = vert[3] - cam_pos;

#ifdef USE_D3D
		//transform for d3d11
#else
		float w1 = 1.f/(cam->world_w*0.5f*zoom);
		float w2 = 1.f/(cam->world_h*0.5f*zoom);
#endif

		s[0] = vert[0].x * w1;
		s[1] = vert[0].y * w2;
		s[2] = opaque_objects[k]->_transform->z_order;
		s[3] = opaque_objects[k]->_render->material.vert_color[0].r;
		s[4] = opaque_objects[k]->_render->material.vert_color[0].g;
		s[5] = opaque_objects[k]->_render->material.vert_color[0].b;
		s[6] = 0;
		s[7] = 0;
		s[8] = 1;
		// from framebox
		s[9] = opaque_objects[k]->_animation->_framebox_ref.lb.x;//0
		s[10] = opaque_objects[k]->_animation->_framebox_ref.lb.y;//0

		s[11] = vert[1].x * w1;
		s[12] = vert[1].y * w2;
		s[13] = opaque_objects[k]->_transform->z_order;
		s[14] = opaque_objects[k]->_render->material.vert_color[1].r;
		s[15] = opaque_objects[k]->_render->material.vert_color[1].g;
		s[16] = opaque_objects[k]->_render->material.vert_color[1].b;
		s[17] = 0;
		s[18] = 0;
		s[19] = 1;
		// from framebox
		s[20] = opaque_objects[k]->_animation->_framebox_ref.lt.x;//0
		s[21] = opaque_objects[k]->_animation->_framebox_ref.lt.y;//1

		s[22] = vert[2].x * w1;
		s[23] = vert[2].y * w2;
		s[24] = opaque_objects[k]->_transform->z_order;
		s[25] = opaque_objects[k]->_render->material.vert_color[2].r;
		s[26] = opaque_objects[k]->_render->material.vert_color[2].g;
		s[27] = opaque_objects[k]->_render->material.vert_color[2].b;
		s[28] = 0;
		s[29] = 0;
		s[30] = 1;
		// from framebox
		s[31] = opaque_objects[k]->_animation->_framebox_ref.rt.x;//1
		s[32] = opaque_objects[k]->_animation->_framebox_ref.rt.y;//1

		s[33] = vert[3].x * w1;
		s[34] = vert[3].y * w2;
		s[35] = opaque_objects[k]->_transform->z_order;
		s[36] = opaque_objects[k]->_render->material.vert_color[3].r;
		s[37] = opaque_objects[k]->_render->material.vert_color[3].g;
		s[38] = opaque_objects[k]->_render->material.vert_color[3].b;
		s[39] = 0;
		s[40] = 0;
		s[41] = 1;
		// from framebox
		s[42] = opaque_objects[k]->_animation->_framebox_ref.rb.x;//1
		s[43] = opaque_objects[k]->_animation->_framebox_ref.rb.y;//1
		memcpy(p, s, sizeof(float) * 44);
		p += 44;
		//LOG_WARN("%d",p-mem);
		if ((int)(p + 44 - (float*)mem) >= vertex_buffer_size / sizeof(float))
		{
			//LOG_WARN("Copy overflow!Break!");
			change_texture = false;
			return k + 1;
		}
	}
	//pack done!
	return opaque_objects.size();
}
bool WorldRender::comp_less(const WIPSprite* lhs, const WIPSprite* rhs)
{
	return (lhs->_render->material.texture) < (rhs->_render->material.texture);
}
bool WorldRender::comp_greater(const WIPSprite* lhs, const WIPSprite* rhs)
{
	return (lhs->_transform->z_order) > (rhs->_transform->z_order);
}
void WorldRender::sort_by_texture()
{
	std::sort(opaque_objects.begin(), opaque_objects.end(), comp_less);
	//std::sort(blend_objects.begin(), blend_objects.end(), comp_less);
}
void WorldRender::sort_by_zorder()
{
	std::sort(blend_objects.begin(), blend_objects.end(), comp_greater);
}