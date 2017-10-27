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
  void init();
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
//#define Text1
#ifdef Text1
	class LargeTexture_TextRender* text_renderer;
#else
	class TextRender* text_renderer;
#endif

	bool edit_mode;

	int gsize;

  //test event system
  class IMButton* imbt;
  class A* a;
  class A* a1;
  class B* b;
  class IMMainMenuBar* main_bar;
  class IMCheckBox* cb;

  string_hash component_update;
};

class EnemeyComponent : public WIPTickComponent
{
public:
	enum class ManState
	{
		E_LEFT,
		E_RIGHT,
		E_UP, E_DOWN
	};
	WIPOBJECT(EnemeyComponent, WIPTickComponent);
	EnemeyComponent(WIPSprite* s) :WIPTickComponent(s)
	{
		pre_clip = nullptr;
		clip = nullptr;
		blt = nullptr;
		player_ref = nullptr;
	}
	~EnemeyComponent(){}
	virtual void init();
	virtual void update(f32 dt);
	virtual void fix_update(f32 dt)
	{

	}
	virtual void destroy()
	{

	}
	void des_player(string_hash tp, void* ud);
	ManState man_state;
	WIPAnimationClip* pre_clip;
	WIPAnimationClip* clip;

	f32 acc_t;
	int cur_direction;

	WIPSprite* blt;
	WIPSprite* player_ref;

	StudioSound* sound;
	StudioSound* sound_death;

};

class PlayerComponent : public WIPTickComponent
{
public:
	enum class ManState
	{
		E_LEFT,
		E_RIGHT,
		E_UP, E_DOWN
	};
	ManState man_state;
	WIPOBJECT(PlayerComponent, WIPTickComponent);
	PlayerComponent(WIPSprite* s) :WIPTickComponent(s)
	{
		pre_clip = nullptr;
		clip = nullptr;
		cam = nullptr;
		blt = nullptr;
	}
	void add_hp(string_hash tp, void* ud);
	~PlayerComponent(){}
	virtual void init();
	virtual void update(f32 dt);

	virtual void fix_update(f32 dt)
	{

	}
	virtual void destroy()
	{

	}
	WIPAnimationClip* pre_clip;
	WIPAnimationClip* clip;
	WIPCamera* cam;

	WIPSprite* blt;

	StudioSound* sound;
	StudioSound* sound_start;
	StudioSound* sound_death;

	float acc = 100;
	f32 hp = 100;
	int killed = 0;
	class TextRender* text_renderer;


	class WIPTexture2D* enemy_texture;
	class WIPTexture2D* player_texture;
	class WIPTexture2D* block_texture;
	class WIPTexture2D* pop_texture;
	class WIPTexture2D* bullet_texture;

	class WIPAnimationClip* enemy_clip;
	class WIPAnimationClip* player_clip;


	class WIPAnimationClip* pop_clip;
	class WIPScene* scene;
	class WorldRender* world_renderer;

	class StudioSound* sound_fire;
	class StudioSound* sound_blast;
	class StudioSound* sound_death_enemy;

};

class BulletComponent : public WIPTickComponent
{
public:
	WIPOBJECT(BulletComponent, WIPTickComponent);
	BulletComponent(WIPSprite* s) :WIPTickComponent(s)
	{
	}

	~BulletComponent(){}
	virtual void init()
	{
	}
	virtual void update(f32 dt);

	virtual void fix_update(f32 dt)
	{

	}
	virtual void destroy()
	{

	}
	RBVector2 v;
	WIPSprite* pop_obj;
	StudioSound* sound;
	RBVector2 pos;
	f32 s = 0;
	int main_axis = -1;
};

class BulletComponent1 : public WIPTickComponent
{
public:
	WIPOBJECT(BulletComponent1, WIPTickComponent);
	BulletComponent1(WIPSprite* s) :WIPTickComponent(s)
	{
	}

	~BulletComponent1(){}
	virtual void init()
	{
	}
	virtual void update(f32 dt);

	virtual void fix_update(f32 dt)
	{

	}
	virtual void destroy()
	{

	}
	RBVector2 v;
	WIPSprite* pop_obj;
	StudioSound* sound;
	RBVector2 pos;
};