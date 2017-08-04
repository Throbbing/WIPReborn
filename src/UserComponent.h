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
	void resize(int sub)
	{
		delete[] bitmap;
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
	void debug_draw(const WIPCamera* cam, const RBVector2& minp1)
	{
		f32 dx = w / sub;
		f32 dy = h / sub;
		RBVector2 v[4];
		bg->get_world_position(v);
		RBVector2 lb = v[1];
		RBVector2 minp;

		minp.y = minp1.y - lb.y;
		minp.x = minp1.x - lb.x;

		minp.y = ((int)(minp.y / dy))*dy + lb.y;
		minp.x = ((int)(minp.x / dx))*dx + lb.x;
		g_rhi->begin_debug_context();
		g_rhi->change_debug_color(RBColorf::blue);
		g_rhi->debug_draw_aabb2d(minp, minp+RBVector2(dx,dy), cam);
		g_rhi->debug_submit();
		g_rhi->end_debug_context();
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

		edit_mode = false;
		gsize = 0;
		
	}
	void destroy()
	{
		delete grid;
	}
	void update(f32 dt);
	
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
#define Text1
#ifdef Text1
	class LargeTexture_TextRender* text_renderer;
#else
	class TextRender* text_renderer;
#endif

	bool edit_mode;

	int gsize;
};