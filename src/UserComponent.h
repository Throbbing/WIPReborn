#pragma once
#include "Sprite.h"
#include "Camera.h"
#include "Input.h"
#include "Scene.h"
#include "Render.h"
#include <fstream>
#include "AudioManager.h"
#include "Sound.h"

class MapGrid
{
public:
	MapGrid(WIPSprite* sp,int sub)
	{
		bg = sp;
		w = bg->_render->mesh.get_witdh();
		h = bg->_render->mesh.get_height();
		bitmap = new char[sub*sub];
		memset(bitmap, 0, sub*sub);
		this->sub = sub;
	}
	void load_mask_data(const char* filename)
	{
		std::ifstream fin(filename, std::ios::binary);
		fin.read(bitmap, sub*sub);
		fin.close();
	}
	void save_mask_data(const char* filename)
	{
		std::ofstream fout(filename, std::ios::binary);
		fout.write(bitmap, sub*sub);
		fout.close();
	}
	unsigned char get_position_state(f32 wx,f32 wy)
	{
		RBVector2 v[4];
		bg->get_world_position(v);
		RBVector2 lb = v[1];
		f32 lx = wx - lb.x;
		f32 ly = wy - lb.y;
		f32 sx = lx / w;
		f32 sy = ly / h;
		int idx = sx*sub + int(sy*sub)*sub;
		return bitmap[idx];
	}
	void set_debug_tag(f32 wx,f32 wy, char tag)
	{
		RBVector2 v[4];
		bg->get_world_position(v);
		RBVector2 lb = v[1];
		f32 lx = wx - lb.x;
		f32 ly = wy - lb.y;
		f32 sx = lx / w;
		f32 sy = ly / h;
		int idx = sx*sub + int(sy*sub)*sub;
		bitmap[idx] = tag;
	}
	void clear_data()
	{
		memset(bitmap, 0, sub*sub);
	}
	void debug_draw(const WIPCamera* cam, char tag)
	{
		f32 dx = w / sub;
		f32 dy = h / sub;
		RBVector2 v[4];
		bg->get_world_position(v);
		RBVector2 lb = v[1];
		g_rhi->begin_debug_context();
		g_rhi->change_debug_color(RBColorf::blue);

		for (int i = 0; i < sub*sub; ++i)
		{
			if (bitmap[i] == tag)
			{
				f32 y = i / sub*dy + lb.y;
				f32 x = i % sub*dx + lb.x;
				g_rhi->debug_draw_aabb2d(RBVector2(x, y), RBVector2(x + dx, y + dy), cam);
			}
		}
		g_rhi->debug_submit();

		g_rhi->end_debug_context();
	}
	char* bitmap;
	WIPSprite* bg;
	f32 w, h;
	int sub;

};

class MapComponent : public WIPTickComponent
{
public:
	enum class ManState
	{
		E_LEFT,
		E_RIGHT,
		E_UP, E_DOWN
	};

	MapComponent(WIPSprite* s) :WIPTickComponent(s)
	{

	}
	~MapComponent(){}
	void load_collision_mask()
	{

	}
	void fix_sprite_position(WIPSprite* sprite)
	{
		if (grid->get_position_state(man->_transform->world_x, man->_transform->world_y))
		{
			man->translate_to(old_pos.x, old_pos.y);
		}
	}
	void init()
	{
		
		pre_clip = nullptr;
		bg = host_object;
		old_pos = RBVector2::zero_vector;
		grid = new MapGrid(bg, 100);
		grid->load_mask_data("./a.mask");
		draw_debug = false;
		newpx = 0;
		newpy = 0;
		fog_dir = RBVector2(1.f, 1.f);
		fog_dir.normalize();

		g_audio_manager->LoadBank("./audio/Desktop/master.bank", false);
		g_audio_manager->LoadBank("./audio/Desktop/master.strings.bank", false);
		sound = g_audio_manager->CreateSound("event:/bgm");
		g_audio_manager->Play(sound);

		
	}
	void destroy()
	{
		delete grid;
	}
	void update(f32 dt)
	{
		
		

		//man->translate(dt*-0.1, 0);
		float speed = 3.2f;
		if (Input::get_key_pressed(WIP_W))
		{
			man->translate(0, speed*dt);
			if (man->_animation->play_name("walk_up", false))
			{
				pre_clip = clip3;
			}
			//cam->move(0,speed*dt);
			man_state = ManState::E_UP;
		}
		else if (Input::get_key_pressed(WIP_A))
		{
			man->translate(-speed*dt, 0);
			if (man->_animation->play_name("walk_left", false))
			{
				pre_clip = clip1;
			}
			man_state = ManState::E_LEFT;

			//cam->move(-speed*dt, 0);
		}
		else if (Input::get_key_pressed(WIP_S))
		{
			man->translate(0, -speed*dt);
			if (man->_animation->play_name("walk_down", false))
			{
				pre_clip = clip;
			}
			man_state = ManState::E_DOWN;

			//cam->move(0,-speed*dt);
		}
		else if (Input::get_key_pressed(WIP_D))
		{
			man->translate(speed*dt, 0);
			if (man->_animation->play_name("walk_right", false))
			{
				pre_clip = clip2;
			}
			man_state = ManState::E_RIGHT;

			//cam->move(speed*dt, 0);
		}
		else
		{
			switch (man_state)
			{
			case ManState::E_DOWN:
				man->_animation->play_name("stand_down",false);
				break;
			case ManState::E_LEFT:
				man->_animation->play_name("stand_left", false);
				break;
			case ManState::E_RIGHT:
				man->_animation->play_name("stand_right", false);
				break;
			case ManState::E_UP:
				man->_animation->play_name("stand_up", false);
				break;
			}
			
		}
		if (Input::get_sys_key_pressed(WIP_MOUSE_LBUTTON))
		{
			 RBVector2 v = cam->screen_to_world(RBVector2I( Input::get_mouse_x(), Input::get_mouse_y()));
			 grid->set_debug_tag(v.x, v.y,1);
		}
		if (Input::get_sys_key_pressed(WIP_MOUSE_RBUTTON))
		{
			RBVector2 v = cam->screen_to_world(RBVector2I(Input::get_mouse_x(), Input::get_mouse_y()));
			grid->set_debug_tag(v.x, v.y, 0);
		}
		if (Input::get_sys_key_up(WIP_MOUSE_MBUTTON))
		{
			//grid->save_mask_data("./a.mask");
			//LOG_INFO("Mask saved!");
		}
		if (Input::get_key_up(WIP_Z))
		{
			//draw_debug = !draw_debug;
		}
		if (Input::get_key_up(WIP_X))
		{
			//grid->clear_data();
		}
		fix_sprite_position(bg);
		RBVector2 manpos(man->_transform->world_x, man->_transform->world_y);
		RBVector2 campos(cam->world_x, cam->world_y);
		RBVector2 d(manpos - campos);

		RBVector2 v[4];
		bg->get_world_position(v);
		RBVector2 lb = v[1];
		RBVector2 rt = v[3];
		cam->zoomin(Input::get_mouse_scroller()*0.1);
		RBVector2 daabb(cam->world_w*cam->_zoom*0.5f, cam->world_h*cam->_zoom*0.5f);
		RBAABB2D bg_bound(lb , rt );
		RBAABB2D cam_aabb(campos - daabb, campos + daabb);

		
		if (!bg_bound.is_contain(cam_aabb))
			cam->zoomout(Input::get_mouse_scroller()*0.1);


		
		if (d.size() > 0.1f)
		{
			d.normalize();
			f32 move_speed = 2.5f;
			d *= (dt * move_speed);
			newpx = cam->world_x + d.x;
			newpy = cam->world_y + d.y;
			newpx = RBMath::clamp(newpx, lb.x + daabb.x, rt.x - daabb.x);
			newpy = RBMath::clamp(newpy, lb.y + daabb.y, rt.y - daabb.y);

			cam->move_to(newpx, newpy);
			
		}
		old_pos.x = man->_transform->world_x;
		old_pos.y = man->_transform->world_y;
		//grid->set_debug_tag(old_pos.x, old_pos.y);
		f32 fog_speed = 1.f;
		fogs->translate(dt*fog_speed*fog_dir.x, dt*fog_speed*fog_dir.y);
		RBVector2 pos = fogs->get_anchor_world_position();
		RBAABB2D bg_bound_ex(bg_bound);
		RBVector2 v11[4];
		fogs->get_world_position(v11);
		RBVector2 lb1 = v11[1];
		RBVector2 rt1 = v11[3];
		bg_bound_ex.min.x -= (rt1.x - lb1.x)*0.6f;
		bg_bound_ex.min.y -= (rt1.y - lb1.y)*0.6f;
		bg_bound_ex.max.x += (rt1.x - lb1.x)*0.6f;
		bg_bound_ex.max.y += (rt1.y - lb1.y)*0.6f;

		if (!bg_bound_ex.is_contain(pos))
		{
			int side = RBMath::get_rand_range_i(1, 4);
			f32 px,py;
			f32 s;
			switch (side)
			{
			case 1:
				px = RBMath::get_rand_range_f(bg_bound.min.x,bg_bound.max.x);
				py = bg_bound_ex.max.y - 0.1f;
				s = RBMath::get_rand_range_f(PI, PI*2.f);
				fog_dir.x = RBMath::sin(s);
				fog_dir.y = RBMath::cos(s);
				break;
			case 2:
				px = RBMath::get_rand_range_f(bg_bound.min.x, bg_bound.max.x);
				py = bg_bound_ex.min.y + 0.1f;
				s = RBMath::get_rand_range_f(0.f, PI);
				fog_dir.x = RBMath::sin(s);
				fog_dir.y = RBMath::cos(s);
				break;
			case 3:
				px = bg_bound_ex.min.x + 0.1f;
				py = RBMath::get_rand_range_f(bg_bound.min.y, bg_bound.max.y);
				s = RBMath::get_rand_range_f(HALF_PI, PI+HALF_PI);
				fog_dir.x = RBMath::sin(s);
				fog_dir.y = RBMath::cos(s);
				break;
			case 4:
				py = RBMath::get_rand_range_f(bg_bound.min.y, bg_bound.max.y);
				px = bg_bound_ex.max.x - 0.1f;
				s = RBMath::get_rand_range_f(HALF_PI, PI + HALF_PI);
				fog_dir.x = -RBMath::sin(s);
				fog_dir.y = -RBMath::cos(s);
				break;
			}
			
			fogs->translate_to(px, py);

			
		}

		if (draw_debug)
			grid->debug_draw(cam,1);

		scene->update_zorder_by_type_tag("character");

		
		//grid->clear_data();
	}
	void fix_update(f32 dt)
	{

	}

	

	WIPSprite* man;
	WIPSprite* fogs;
	RBVector2 fog_dir;
	WIPSprite* bg;

	WIPCamera* cam;
	WIPScene* scene;
	ManState man_state;
	WIPAnimationClip* clip;
	WIPAnimationClip* clip1;
	WIPAnimationClip* clip2;
	WIPAnimationClip* clip3;
	WIPAnimationClip* clip_s;
	WIPAnimationClip* clip1_s;
	WIPAnimationClip* clip2_s;
	WIPAnimationClip* clip3_s;
	WIPAnimationClip* pre_clip;

	RBVector2 old_pos;
	MapGrid* grid;
	bool draw_debug;
	f32 newpx;
	f32 newpy;

	StudioSound* sound;

	

	UIRender* ui_renderer;
};