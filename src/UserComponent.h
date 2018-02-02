#pragma once
#include "Sprite.h"
#include "Camera.h"
#include "Input.h"
#include "Scene.h"
#include "Render.h"
#include <fstream>
#include "AudioManager.h"
#include "Sound.h"
#include "ScriptManager.h"

//Lua逻辑的组件,lua内所有组件都继承与一个抽象的lua逻辑组件，以保证素有函数都有实现
//创建代码入口都是从lua开始，首先创建Sprite，使用一个全局的子表来保存创建的Spritelua对象，还要把这个名称传回宿主对象，宿主对象通过这个名称查找到对应的lua对象（对象销毁时要注意销毁次索引项）
//ScriptComponentlua创建后称为对应Sprite的component域（脚本名字作为key），此处调用lua脚本功能时候，在宿主对象中通过名称找到对应的lua对象，然后通过lua对象的使用本类的脚本名字索引到对应的lua component，然后调用
//此方案性能不明！
class WIPScriptComponent:public WIPTickComponent
{
public:
	WIPOBJECT(WIPScriptComponent, WIPTickComponent);
	WIP_MEM(WIPScriptComponent);
	WIPScriptComponent(WIPSprite* s, const char* script_path) :
		WIPTickComponent(s), _missing(false)
	{
		_missing = !g_script_manager->load_file(script_path);
		std::string name(g_filesystem->get_filename(script_path));
		std::string str(name);
		char tmp[8] = {0};
		itoa(s->key, tmp, 10);
		str += tmp;
		_name = str;
		str += "=" + name + ":new();";
		g_script_manager->do_string(str.data());
		
		
	}
	virtual void init();
	virtual void on_begin_contact(const WIPSprite* s);
	virtual void on_contact(const WIPSprite* s);
	virtual void on_end_contact(const WIPSprite* s);

	virtual void update(f32 dt);
	virtual void fix_update(f32 dt);
	virtual void destroy();

	//level start/end
	virtual void start();
	virtual void end();
	~WIPScriptComponent()
	{
		//remove from lua
		g_script_manager->remove_global_var(_name.data());
	}

	bool _missing;
	//the lua name of this component
	std::string _name;
};

#include "./tiny_dnn/tiny_dnn.h"

using namespace tiny_dnn;
using namespace tiny_dnn::activation;
using namespace tiny_dnn::layers;

class MapGrid
{
public:
	MapGrid(TRefCountPtr<WIPSprite> sp,int sub)
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
	TRefCountPtr<WIPSprite> bg;
	f32 w, h;
	int sub;

};

struct NPCDisplayData
{
	wchar_t* words;
	WIPTexture2D* face;
};

struct NPCMental
{

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
	enum class GameState
	{
		E_TITLE,
		E_TALK,
		E_PLAYER_CONTROLL,
		E_TOTAL
	};
	WIPOBJECT(MapComponent, WIPTickComponent);
	WIP_MEM(MapComponent);

	MapComponent(TRefCountPtr<WIPSprite> s) :WIPTickComponent(s)
	{

	}
	~MapComponent(){}
	void load_collision_mask()
	{

	}
	void change_to_talk(string_hash tp, void* data);
	void change_to_player(string_hash tp, void* data);
	void fix_sprite_position(TRefCountPtr<WIPSprite> sprite)
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

	

	TRefCountPtr<WIPSprite> man;
	TRefCountPtr<WIPSprite> fogs;
	RBVector2 fog_dir;
	TRefCountPtr<WIPSprite> bg;

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
	StudioSound* sound_t;
	

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


  GameState game_state = GameState::E_TITLE;
  const NPCDisplayData* cur_npc_ui = nullptr;

  WIPRenderTexture2D* render_texture2d;

  //title resouces
  WIPTexture2D* ext_bt;
  WIPTexture2D* ctn_bt;
  WIPTexture2D* stt_bt;
  WIPTexture2D* t_bg;
  WIPTexture2D* title;
  int title_state = 0;

};

class NPCComponent : public WIPTickComponent
{
public:
	WIPOBJECT(NPCComponent, WIPTickComponent);
	WIP_MEM(NPCComponent);
	NPCComponent(TRefCountPtr<WIPSprite> s);
	~NPCComponent();
	virtual void init();
	virtual void update(f32 dt);
	virtual void fix_update(f32 dt)
	{

	}
	virtual void destroy();
	void add_faces(std::string name,WIPTexture2D* tex);
	void set_face(std::string name);
	void set_no_face();
	void set_default_face();
	void on_begin_contact(const WIPSprite* s);
	void on_end_contact(const WIPSprite* s);
	void on_contact(const WIPSprite* s);
	NPCDisplayData data;
	std::queue<wchar_t*> words[2];
	std::map<int, WIPTexture2D*> npc_faces;
	MapComponent* map_component=nullptr;
};

class EnemeyComponent : public WIPTickComponent
{
public:

	struct data_pak_t
	{
		data_pak_t(RBVector2 ppos, RBVector2 epos, RBVector2 pbpos, RBVector2 ebpos, int shot, int dying, f32 time) :
			player_pos(ppos), enemey_pos(epos), player_bullet_pos(pbpos), enemey_bullet_pos(epos), is_shotting(shot), is_dying(dying), cur_time_ms(time){}
		RBVector2 player_pos;
		RBVector2 enemey_pos;
		RBVector2 player_bullet_pos;
		RBVector2 enemey_bullet_pos;
		int is_shotting;
		int is_dying;
		f32 cur_time_ms;
	};

	enum class ManState
	{
		E_LEFT,
		E_RIGHT,
		E_UP, E_DOWN
	};
	WIPOBJECT(EnemeyComponent, WIPTickComponent);
	WIP_MEM(EnemeyComponent);

	EnemeyComponent(TRefCountPtr<WIPSprite> s) :WIPTickComponent(s)
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
	virtual void destroy();
	void des_player(string_hash tp, void* ud);
	ManState man_state;
	WIPAnimationClip* pre_clip;
	WIPAnimationClip* clip;

	f32 acc_t;
	int cur_direction;

	TRefCountPtr<WIPSprite> blt;
	TRefCountPtr<WIPSprite> player_ref;

	StudioSound* sound;
	StudioSound* sound_death;

	std::vector<data_pak_t> data_out;
	int cur_size=0;

	f32 start_time = 0.f;
	f32 last_time = 0.f;

	std::ofstream fout;

	network<sequential> nn;

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
	WIP_MEM(PlayerComponent);

	PlayerComponent(TRefCountPtr<WIPSprite> s) :WIPTickComponent(s)
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

	TRefCountPtr<WIPSprite> blt;

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
	WIP_MEM(BulletComponent);

	BulletComponent(TRefCountPtr<WIPSprite> s) :WIPTickComponent(s)
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
	TRefCountPtr<WIPSprite> pop_obj;
	StudioSound* sound;
	RBVector2 pos;
	f32 s = 0;
	int main_axis = -1;
};

class BulletComponent1 : public WIPTickComponent
{
public:
	WIPOBJECT(BulletComponent1, WIPTickComponent);
	WIP_MEM(BulletComponent1);

	BulletComponent1(TRefCountPtr<WIPSprite> s) :WIPTickComponent(s)
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
	TRefCountPtr<WIPSprite> pop_obj;
	StudioSound* sound;
	RBVector2 pos;
};