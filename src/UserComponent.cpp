#include "UIWrap.h"
#include "UserComponent.h"
#include "imgui.h"

//object should belongs to a world not scene
#include "GLFWApp.h"

void regist_user_component()
{
	MapComponent::register_tick_component();
	NPCComponent::register_tick_component();
  TransformComponent::register_tick_component();
	//EnemeyComponent::register_tick_component();
	//PlayerComponent::register_tick_component();
	//BulletComponent::register_tick_component();
	//BulletComponent1::register_tick_component();
}

void WIPScriptComponent::init()
{
	g_script_manager->call_table_function(_name.data(), "init",nullptr);
}

void WIPScriptComponent::on_begin_contact(const WIPSprite* s)
{
	g_script_manager->call_table_function(_name.data(), "on_begin_contact", nullptr);
}
void WIPScriptComponent::on_contact(const WIPSprite* s, float dt)
{
	g_script_manager->call_table_function(_name.data(), "on_contact", nullptr);
}
void WIPScriptComponent::on_end_contact(const WIPSprite* s)
{
	g_script_manager->call_table_function(_name.data(), "on_end_contact", nullptr);
}

void WIPScriptComponent::update(f32 dt)
{
	g_script_manager->call_table_function(_name.data(), "update", "d",dt);
}
void WIPScriptComponent::fix_update(f32 dt)
{
	g_script_manager->call_table_function(_name.data(), "fix_update","d",dt);
}
void WIPScriptComponent::destroy()
{
	g_script_manager->call_table_function(_name.data(), "destroy", nullptr);
}

//level start/end
void WIPScriptComponent::start()
{
	g_script_manager->call_table_function(_name.data(), "start", nullptr);
}
void WIPScriptComponent::end()
{
	g_script_manager->call_table_function(_name.data(), "end", nullptr);
}

void UTF_8ToUnicode(wchar_t* pOut, char *pText)
{
	char* uchar = (char *)pOut;
	uchar[1] = ((pText[0] & 0x0F) << 4) + ((pText[1] >> 2) & 0x0F);
	uchar[0] = ((pText[1] & 0x03) << 6) + (pText[2] & 0x3F);
	return;
}

void UnicodeToUTF_8(char* pOut, wchar_t* pText)
{
	char* pchar = (char *)pText;
	pOut[0] = (0xE0 | ((pchar[1] & 0xF0) >> 4));
	pOut[1] = (0x80 | ((pchar[1] & 0x0F) << 2)) + ((pchar[0] & 0xC0) >> 6);
	pOut[2] = (0x80 | (pchar[0] & 0x3F));
	return;
}

char* get_utf8(wchar_t* text, char* buf)
{
	for (size_t i = 0; i < wcslen(text); ++i)
	{
		UnicodeToUTF_8(&buf[i * 3], &text[i]);
	}
	return buf;
}


bool imgui_button_short(wchar_t* text)
{
	char t[15];
	memset(t, 0, 15);
	return ImGui::Button(get_utf8(text, t));
}

bool imgui_button_long(wchar_t* text)
{
	char t[512];
	memset(t, 0, 512);
	return ImGui::Button(get_utf8(text, t));
}

void imgui_label_short(wchar_t* text)
{
	char t[15];
	memset(t, 0, 15);
	ImGui::Text(get_utf8(text, t));
}

void imgui_lable_long(wchar_t* text)
{
	char t[512];
	memset(t, 0, 512);
	ImGui::Text(get_utf8(text, t));
}

void MapComponent::change_to_talk(string_hash tp, void* data)
{
	cur_npc_ui = (NPCDisplayData*)data;
	game_state = GameState::E_TALK;
}

void MapComponent::change_to_player(string_hash tp, void* data)
{
	g_temp_uisys->clear();
	cur_npc_ui = nullptr;
	game_state = GameState::E_PLAYER_CONTROLL;
}

//actions
struct MovetoPlayer :public Ac
{
  MovetoPlayer(WIPSprite* s, const std::string& ani, float speed);
  bool run(float dt)
  {
    if (!mover) return true;
    if (end)
      return true;
    if (!begin)
    {
      init(dt);
      begin = true;
    }
    /*
    RBVector2 target(s->_transform->world_x, s->_transform->world_y);
    RBVector2 mp(mover->_transform->world_x, mover->_transform->world_y);
    RBVector2 dis(target - mp);
    dis.normalize();
    dis = dis*dt*speed;
    if ((dis + mp - target).squared_size()<1.5f)
    {
      mover->_animation->play_name("stand_down", true);
      end = true;
      return true;
    }
    mover->translate(dis.x, dis.y);
    */
    if (doing(dt))
    {
      end = true;
      return true;
    }
    return false;
  }
  virtual bool doing(float dt)
  {
    RBVector2 target(s->_transform->world_x, s->_transform->world_y);
    RBVector2 mp(mover->_transform->world_x, mover->_transform->world_y);
    RBVector2 dis(target - mp);
    dis.normalize();
    dis = dis*dt*speed;
    if ((dis + mp - target).squared_size()<1.5f)
    {
      mover->_animation->play_name("stand_down", true);
      end = true;
      return true;
    }
    mover->translate(dis.x, dis.y);
    time += dt;
    return false;
  }
  void init(float dt)
  {
    mover->_animation->play_name(ani_name, true);
    s = g_scene->get_sprite_by_tag("man");
  }
  WIPSprite* mover = 0;
  std::string ani_name;
  WIPSprite* s = 0;
  float speed;
};

struct Moveto :public Ac
{
  Moveto(WIPSprite* s, const std::string& ani, float speed, const RBVector2& pos);
  bool run(float dt)
  {
    if (!mover) return true;
    if (end)
      return true;
    if (!begin)
    {
      init(dt);
      begin = true;
    }
    if (doing(dt))
    {
      end = true;
      return true;
    }
    return false;
  }
  bool doing(float dt)
  {
    RBVector2 target(tar);
    RBVector2 mp(mover->_transform->world_x, mover->_transform->world_y);
    RBVector2 dis(target - mp);
    dis.normalize();
    dis = dis*dt*speed;
    mover->_animation->play_name(ani_name, true);
    if ((dis + mp - target).squared_size()<0.005f)
    {
      mover->_animation->play_name("stand_down", false);
      
      return true;
    }
    mover->translate(dis.x, dis.y);
    time += dt;
    return false;
  }
  void init(float dt)
  {
    mover->_animation->play_name(ani_name, true);
  }
  WIPSprite* mover = 0;
  std::string ani_name;
  float speed;
  RBVector2 tar;
};

struct TurnoffBGM : public Ac
{
  TurnoffBGM(const std::string& name){ event_name = name; }
  bool run(float dt)
  {
    if (event_name.empty()) return true;
    if (end)
      return true;
    if (!begin)
    {
      g_audio_manager->Stop(event_name);
      begin = true;
    }
    end = true;
    return true;
  }
  std::string event_name;
};

struct PlayBGM :public Ac
{
  PlayBGM(const std::string& name){ event_name = name; }
  bool run(float dt)
  {
    if (event_name.empty()) return true;
    if (end)
      return true;
    if (!begin)
    {
      g_audio_manager->Play(event_name);
      begin = true;
    }
    end = true;
    return true;
  }
  std::string event_name;
};

struct ShowTexture :public Ac
{
  ShowTexture(MapComponent* tex){ t=tex; }
  bool run(float dt)
  {
    if (!t) return true;
    if (end)
      return true;
    if (!begin)
    {
      t->game_state = MapComponent::GameState::E_END;
      //g_temp_uisys->draw_picture(0, 0, t->get_width(), t->get_height(), t);
      begin = true;
    }
    end = true;
    return true;
  }
  MapComponent* t;
};

struct CameraZoom :public Ac
{
  CameraZoom(float tar, WIPCamera* c, float sp){ target = tar; cam = c; speed = sp; }
  bool run(float dt)
  {
    if (end)
      return true;
    if (!begin)
    {
      init(dt);
      begin = true;
    }
    if (doing(dt))
    {
      end = true;
      return true;
    }
    return false;
  }
  void init(float dt)
  {
    
  }
  bool doing(float dt)
  {
    float d = dt*(target - cam->_zoom)*speed;
    if (RBMath::abs(cam->_zoom+d-target)<0.05f)
    {
      return true;
    }
    cam->_zoom += d;
    return false;
  }
  float target;
  float speed;
  WIPCamera* cam;
};

//if you want use combine action you must implemente the doing function and init function
struct CombineAc2 :public Ac
{
  CombineAc2(Ac* ac1,Ac* ac2)
  {
    a = ac1;
    b = ac2;
  }
  bool run(float dt)
  {
    if (!a || !b) return true;

    bool ad = a->run(dt);
    bool bd = b->run(dt);
    return ad&&bd;
    if (end)
      return true;
    if (!begin)
    {
      init(dt);
      begin = true;
    }
    
    if (doing(dt))
    {
      end = true;
      return true;
    }
    return false;
  }
  void init(float dt)
  {
    a->init(dt);
    b->init(dt);
  }
  bool doing(float dt)
  {
    bool ad = a->doing(dt);
    bool bd = b->doing(dt);
    return ad&&bd;
  }
  Ac* a;
  Ac* b;
};

void MapComponent::init()
{
	cam = g_scene->get_camera("MainCam");
	scene = g_scene;
	man = g_scene->get_sprite_by_tag("man");
  woman = g_scene->get_sprite_by_tag("npc1");
	fogs = g_scene->get_sprite_by_tag("fog");

	pre_clip = nullptr;
	bg = host_object;
	old_pos = RBVector2::zero_vector;
	grid = new MapGrid(bg, 100);
	grid->load_mask_data(mask_path.c_str());
	draw_debug = false;
	newpx = 0;
	newpy = 0;
  float s = RBMath::get_rand_range_f(PI, PI*2.f);
  fog_dir.x = RBMath::sin(s);
  fog_dir.y = RBMath::cos(s);
	fog_dir.normalize();

	
	//sound = g_audio_manager->CreateSound("event:/bgm");
	sound_t = g_audio_manager->CreateSound("event:/bgm1");
	g_audio_manager->Play(sound_t);

	edit_mode = false;
	gsize = 0;


	//test event
	imbt = new IMButton("caonim");
	cb = new IMCheckBox("nimei", true);
	a = new A();
	a1 = new A();
	b = new B();
	main_bar = new IMMainMenuBar();
	main_bar->add_menu_item("File", "Open..");
	main_bar->add_menu_item("File", "Save..");
	main_bar->add_menu_item("File", "Import..");
	main_bar->add_menu_item("Tools", "Material Editor");
	main_bar->add_menu_item("Tools", "Rendering Setting");
	main_bar->add_menu_item("Tools", "Camera Setting");
	main_bar->add_menu_item("Help", "Help");
	main_bar->add_menu_item("Debug", "Debug Tools");

	a->subscribe_event(cb, get_string_hash("check_box_change"), WIP_EVENT_HANDLER_OUT(A, check_box, a));
	cb->signal_for_init();
	a->subscribe_event(imbt, get_string_hash("button_push"), WIP_EVENT_HANDLER_OUT(A, push, a), 2);
	a1->subscribe_event(imbt, get_string_hash("button_push"), WIP_EVENT_HANDLER_OUT(A, push, a1), 2);
	b->subscribe_event(imbt, get_string_hash("button_push"), WIP_EVENT_HANDLER_OUT(B, push1, b), 1);

	b->subscribe_event(main_bar, get_string_hash("File.Open.."), WIP_EVENT_HANDLER_OUT(B, handle_menu, b));
	b->subscribe_event(main_bar, get_string_hash("File.Save.."), WIP_EVENT_HANDLER_OUT(B, handle_menu, b));
	b->subscribe_event(main_bar, get_string_hash("File.Import.."), WIP_EVENT_HANDLER_OUT(B, handle_menu, b));
	b->subscribe_event(main_bar, get_string_hash("Tools.Rendering Setting"), WIP_EVENT_HANDLER_OUT(B, handle_menu, b));

	component_update = get_string_hash("MapComponent Update");

	imbt->subscribe_event(this, component_update, WIP_EVENT_HANDLER_OUT(IMButton, update, imbt));
	main_bar->subscribe_event(this, component_update, WIP_EVENT_HANDLER_OUT(IMMainMenuBar, update, main_bar));
	cb->subscribe_event(this, component_update, WIP_EVENT_HANDLER_OUT(IMCheckBox, update, cb));



	{
		auto res_extbt = g_res_manager->load_resource("./pic/exit-2.png", WIPResourceType::TEXTURE);
		int w = ((TextureData *)(res_extbt->extra))->width;
		int h = ((TextureData *)(res_extbt->extra))->height;

		ext_bt = g_rhi->RHICreateTexture2D(w, h, res_extbt->ptr);
	}
	{
		auto res_extbt = g_res_manager->load_resource("./pic/continue-2.png", WIPResourceType::TEXTURE);
		int w = ((TextureData *)(res_extbt->extra))->width;
		int h = ((TextureData *)(res_extbt->extra))->height;

		ctn_bt = g_rhi->RHICreateTexture2D(w, h, res_extbt->ptr);
	}
	{
		auto res_extbt = g_res_manager->load_resource("./pic/fd/end_words.png", WIPResourceType::TEXTURE);
		int w = ((TextureData *)(res_extbt->extra))->width;
		int h = ((TextureData *)(res_extbt->extra))->height;

		stt_bt = g_rhi->RHICreateTexture2D(w, h, res_extbt->ptr);
	}
	{
		auto res_extbt = g_res_manager->load_resource("./pic/fd/title.png", WIPResourceType::TEXTURE);
		int w = ((TextureData *)(res_extbt->extra))->width;
		int h = ((TextureData *)(res_extbt->extra))->height;

		t_bg = g_rhi->RHICreateTexture2D(w, h, res_extbt->ptr);
	}
	{
		auto res_extbt = g_res_manager->load_resource("./pic/fd/title_words.png", WIPResourceType::TEXTURE);
		int w = ((TextureData *)(res_extbt->extra))->width;
		int h = ((TextureData *)(res_extbt->extra))->height;

		title = g_rhi->RHICreateTexture2D(w, h, res_extbt->ptr);
	}
  {
    auto res_extbt = g_res_manager->load_resource("./pic/fd/Black background2.png", WIPResourceType::TEXTURE);
    int w = ((TextureData *)(res_extbt->extra))->width;
    int h = ((TextureData *)(res_extbt->extra))->height;

    move_bar = g_rhi->RHICreateTexture2D(w, h, res_extbt->ptr);
  }

  {
    auto res_extbt = g_res_manager->load_resource("./pic/fd/zhaougouqd.png", WIPResourceType::TEXTURE);
    int w = ((TextureData *)(res_extbt->extra))->width;
    int h = ((TextureData *)(res_extbt->extra))->height;

    end_tex = g_rhi->RHICreateTexture2D(w, h, res_extbt->ptr);
  }
  //actions.push_back(new CameraZoom(cam->_zoom - 0.2f, cam, 1.5f));

  //todo:write a debugger!
  actions.push_back(new CombineAc2(new Moveto(man, "walk_down", 1.f, RBVector2(3.3f, 8.2f)), new MovetoPlayer(woman, "walk_up", 1.f)));

  actions.push_back(new TurnoffBGM("event:/bgm1"));
  actions.push_back(new PlayBGM("event:/grass_bgm"));
  actions.push_back(new Moveto(man, "walk_down", 1.5f,RBVector2(3.3f,11.2f)));
  actions.push_back(new PlayBGM("event:/fly_cry"));
  actions.push_back(new Moveto(woman, "stand_up", 0.5f, RBVector2(5.f, 4.3f)));
  actions.push_back(new PlayBGM("event:/last_cry"));
  actions.push_back(new CombineAc2(new MovetoPlayer(woman, "walk_up", 8.f), new CameraZoom(cam->_zoom - 0.7f, cam, 8.5f)));
  actions.push_back(new ShowTexture(this));
}
void MapComponent::destroy()
{
	g_audio_manager->StopAll();
	delete sound;// = g_audio_manager->CreateSound("event:/bgm");
	delete sound_t;// = g_audio_manager->CreateSound("event:/bgm1");
	delete grid;
  for (int i = 0; i < actions.size(); ++i)
    delete actions[i];
}

MovetoPlayer::MovetoPlayer(WIPSprite* s, const std::string& ani, float inspeed) : Ac()
{
  mover = s;
  ani_name = ani;
  speed = inspeed;
}



Moveto::Moveto(WIPSprite* s, const std::string& ani, float inspeed,const RBVector2& pos) : Ac()
{
  mover = s;
  ani_name = ani;
  speed = inspeed;
  tar = pos;
}

#include <stack>

struct EditorData
{
  EditorData()
  {
    reload_level_file("LevelCache.h");
    init_trigger();
  }
  ~EditorData()
  {
    close_level();
  }
  //trigger data
  void init_trigger()
  {
    trigger_size[0] = trigger_size[1] = 1.f;
    is_visible = false;
    memset(texture_name, 0, 100);
    char s[32] = "noname\0";
    memcpy(event_name,s,32);
    std::string sc = "[](void* data, const WIPSprite* s, TransformComponent* t)->void\n{\n//add your code \n//you can also edit your code in level file. \n}\0";
    memcpy(script[BEGIN_EVENT], sc.data(), sc.size());
    sc = "[](void* data, const WIPSprite* s, TransformComponent* t)->void\n{\n//add your code \n}\0";
    memcpy(script[END_EVENT], sc.data(), sc.size());
    sc = "[](void* data, const WIPSprite* s, TransformComponent* t)->void\n{\n//add your code \n}\0";
    memcpy(script[CONTACT_EVENT], sc.data(), sc.size());
    sc = "[](void* data, TransformComponent* t)->void\n{\n//add your code \n}\0";
    memcpy(script[LEVEL_START], sc.data(), sc.size());
    sc = "[](void* data, TransformComponent* t)->void\n{\n//add your code \n}\0";
    memcpy(script[LEVEL_END], sc.data(), sc.size());
    sc = "[](void* data, float dt, TransformComponent* t)->void\n{\n//add your code \n}\0";
    memcpy(script[UPDATE_EVENT], sc.data(), sc.size());
    position = RBVector2::zero_vector;
    rotation = RBVector2::zero_vector;
  }


  void add_trigger()
  {
    char out1[10240] =
      "{\n"
      "WIPSpriteCreator ctor_trs1(%f, %f, WIPMaterialType::E_OTHER);\n"
      "ctor_trs1.texture = 0;\n"
      "ctor_trs1.world_render = 0;\n"
      "ctor_trs1.body_tp = WIPCollider::_CollisionTypes::E_GHOST;\n"
      "ctor_trs1.collider_sx = 1.0f;\n"
      "ctor_trs1.collider_sy = 1.0f;\n"
      "auto* man_trans1 = WIPSpriteFactory::create_sprite(ctor_trs1);\n"
      "man_trans1->set_anchor(0.f, 0.f);\n"
      "man_trans1->set_tag(\"%s\");\n"
      "man_trans1->set_type_tag(\"trigger\");\n"
      "TransformComponent* tc1 = (TransformComponent*)WIPObject::create_tick_component(\"TransformComponent\", man_trans1);\n"
      "tc1->func_begin = %s;\n"
      "tc1->func_end = %s;\n"
      "tc1->func_update = %s;\n"
      "tc1->func_contact = %s;\n"
      "tc1->func_level_start = %s;\n"
      "tc1->func_level_end = %s;\n"
      "man_trans1->add_tick_component(tc1);\n"
      "scene->load_sprite(man_trans1);\n"
      "man_trans1->translate_to(%f, %f);\n"
      "man_trans1->_render->is_visible = false\n;"
      "man_trans1->rotate_to(%f);}\n\0";
    char out[10240];
    sprintf(out, out1, trigger_size[0], trigger_size[1], event_name, script[0], script[1], script[2], script[3], script[4], script[5], position.x, position.y,rotation.x);
    
    level_file << out;
    level_file.flush();

    {
      WIPSpriteCreator ctor_trs1(trigger_size[0], trigger_size[1], WIPMaterialType::E_OTHER);
      ctor_trs1.texture = 0;
      ctor_trs1.world_render = 0;
      ctor_trs1.body_tp = WIPCollider::_CollisionTypes::E_GHOST;
      ctor_trs1.collider_sx = 1.0f;
      ctor_trs1.collider_sy = 1.0f;
      auto* man_trans1 = WIPSpriteFactory::create_sprite(ctor_trs1);
      man_trans1->_render->is_visible = false;
      man_trans1->set_anchor(0.f, 0.f);
      man_trans1->set_tag(event_name);
      man_trans1->set_type_tag("trigger");
      TransformComponent* tc1 = (TransformComponent*)WIPObject::create_tick_component("TransformComponent", man_trans1);
      man_trans1->add_tick_component(tc1);
      g_scene->load_sprite(man_trans1);
      man_trans1->translate_to(position.x, position.y);
      man_trans1->rotate_to(rotation.x);

      added_sprites.push_back(man_trans1);

    }
  }

  float trigger_size[2];
  bool is_visible;
  char texture_name[100];
  char event_name[32];
  char script[6][2048];
  RBVector2 position;
  RBVector2 rotation;
  
  

  void reload_level_file(const char* name)
  {
    if (level_file.is_open())
      level_file.close();
    level_file.open(name, std::ios::app);

  }

  void close_level()
  {
    level_file.close();
  }

  void remove_sprite(int id)
  {
    if (added_sprites[id] == nullptr)
      return;
    g_scene->remove_sprite(added_sprites[id]);
    bak_id.push(id);
  }
  
  std::fstream level_file;
  std::vector<WIPSprite*> added_sprites;
  std::stack<int> bak_id;


} g_eddata;

void MapComponent::update(f32 dt)
{

	//send_event(component_update);
	switch (game_state)
	{
	case GameState::E_TITLE:
	{
		g_temp_uisys->begin();
		g_temp_uisys->draw_picture(1, 0, t_bg->get_width(), t_bg->get_height(), t_bg);
    if (alpha >= 1.f || alpha <= 0.f)
      alpha_s *= -1.f;
    alpha += dt*alpha_s*RBMath::get_rand_range_f(0.1f,0.5f);
    g_temp_uisys->draw_picture(1, 0, t_bg->get_width(), t_bg->get_height(), title,RBColorf(1,1,1,alpha));
#if 0
		if (Input::get_key_down(WIP_W))
		{
			title_state--;
			if (title_state < 0) title_state = 2;
			title_state %= 3;
		}
		if (Input::get_key_down(WIP_S))
		{
			title_state++;
			title_state %= 3;
		}

		switch (title_state)
		{
		case 0:
		{
			g_temp_uisys->draw_picture(330, 600 - 391, stt_bt->get_width(), stt_bt->get_height(), stt_bt);
		}
		break;
		case 1:
		{
			g_temp_uisys->draw_picture(330, 600 - 450, ctn_bt->get_width(), ctn_bt->get_height(), ctn_bt);

		}
		break;
		case 2:
		{
			g_temp_uisys->draw_picture(330, 600 - 520, ext_bt->get_width(), ext_bt->get_height(), ext_bt);

		}
		break;
		default:
			break;
		}
#endif
		g_temp_uisys->end();

		if (Input::get_sys_key_down(WIP_SPACE))
		{
      /*
			switch (title_state)
			{
			case 0:
			{
				g_audio_manager->StopAll(FMOD_STUDIO_STOP_ALLOWFADEOUT);
				g_audio_manager->Play(sound);
				change_to_player(0, 0);
			}
			break;
			case 1:
			{

			}
			break;
			case 2:
			{
				g_app->require_exit();

			}
			break;
			}
      */
      g_scene->game_varible["title"].number.integer = 0;
      change_to_player(0, 0);
		}

		return;
	}
	break;
	case GameState::E_PLAYER_CONTROLL:
	{
    
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
				man->_animation->play_name("stand_down", false);
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
	}
	break;
	case GameState::E_TALK:
	{
		if (cur_npc_ui)
		{
      
			g_temp_uisys->begin();
			g_temp_uisys->draw_box(0, 0, 0, 0, RBColorf(0.f, 0.f, 0.f, 0.5f));
			if (cur_npc_ui->face)
				g_temp_uisys->draw_picture(600, 50, cur_npc_ui->face->get_width(), cur_npc_ui->face->get_height(), cur_npc_ui->face);
			wchar_t *words1 = cur_npc_ui->words;
			g_temp_uisys->draw_text(100,48, words1, wcslen(words1), g_app->window_w);
			g_temp_uisys->end();
		}
	}
	break;
  case GameState::E_ACTION:
  {
    man->_animation->play_name("stand_down", false);
    g_temp_uisys->begin();
    g_temp_uisys->draw_picture(1, 0, move_bar->get_width(), move_bar->get_height(), move_bar);
    g_temp_uisys->end();
    for (int i = 0; i < actions.size(); ++i)
    {
      bool r = actions[i]->run(dt);
      if (!r)
        break;
    }

    /*
    
    woman->_animation->play_name("walk_up", true);
    g_temp_uisys->begin();
    g_temp_uisys->draw_picture(1, 0, move_bar->get_width(), move_bar->get_height(), move_bar);
    g_temp_uisys->end();
    RBVector2 wp(woman->_transform->world_x, woman->_transform->world_y);
    RBVector2 mp(man->_transform->world_x, man->_transform->world_y);
    if (!action_bg)
    {
      action_bg = true;
      trans_target = wp + 0.8f*(mp - wp);
    }
    RBVector2 dis(trans_target - wp);
    dis.normalize();
    dis = dis*dt;
    woman->translate(dis.x, dis.y);
    */
  }
  break;
  case GameState::E_END:
  {
    g_temp_uisys->begin();
    g_temp_uisys->draw_picture(1, 0, end_tex->get_width(), end_tex->get_height(), end_tex);
    if (alpha >= 1.f || alpha <= 0.f)
      alpha_s *= -1.f;
    alpha += dt*alpha_s*RBMath::get_rand_range_f(0.1f, 0.5f);
    g_temp_uisys->draw_picture(1, 0, t_bg->get_width(), t_bg->get_height(), stt_bt, RBColorf(1, 0.5, 0.5, alpha));
    g_temp_uisys->end();
    if (Input::get_sys_key_down(WIP_SPACE))
    {
      //g_scene->game_varible["already pass"] = Game_Varible(0);
      //g_scene->game_varible["title"] = Game_Varible(1);
      g_scene->loader->init_game();
      g_scene->load_level(1, RBVector2(6.6f, -12.6f));
    }
  }
  break;
	default:
		break;
	}

	if (edit_mode)
	{
		if (!ImGui::IsMouseHoveringAnyWindow())
		{
			if (Input::get_sys_key_pressed(WIP_MOUSE_LBUTTON))
			{
				RBVector2 v = cam->screen_to_world(RBVector2I(Input::get_mouse_x(), Input::get_mouse_y()));
				grid->set_debug_tag(v.x, v.y, 1);
			}
			if (Input::get_sys_key_pressed(WIP_MOUSE_RBUTTON))
			{
				RBVector2 v = cam->screen_to_world(RBVector2I(Input::get_mouse_x(), Input::get_mouse_y()));
				grid->set_debug_tag(v.x, v.y, 0);
			}
			RBVector2 v = cam->screen_to_world(RBVector2I(Input::get_mouse_x(), Input::get_mouse_y()));
			grid->debug_draw(cam, v);
		}
		ImGui::Begin("Editor");
		ImGui::SliderInt("Size", &gsize, 1, 300);
		if (imgui_button_short(L"重置尺寸"))
		{
			grid->resize(gsize);
		}
		if (imgui_button_short(L"储存碰撞"))
		{
			grid->save_mask_data("./a.mask");
			//LOG_INFO("Mask saved!");
		}
		if (imgui_button_short(L"清除碰撞"))
		{
			grid->clear_data();
		}
		char text[256];
		::memset(text, 0, 256);
		ImGui::Text(get_utf8(L"鼠标左键绘制碰撞，鼠标右键擦除碰撞", text));
    ImGui::Text("%f,%f", man->_transform->world_x, man->_transform->world_y);
		ImGui::End();

    ImGui::Begin("Level Editor");
    ImGui::BeginGroup();
    ImGui::CollapsingHeader("Trigger");

    ImGui::InputFloat2("Size", g_eddata.trigger_size);
    ImGui::InputFloat2("Position", (float*)&g_eddata.position);
    ImGui::InputFloat2("Rotation", (float*)&g_eddata.rotation);
    ImGui::InputText("Event name", g_eddata.event_name,32);
    ImGui::InputTextMultiline("Script 0", g_eddata.script[0], 2048);
    ImGui::InputTextMultiline("Script 1", g_eddata.script[1], 2048);
    ImGui::InputTextMultiline("Script 2", g_eddata.script[2], 2048);
    ImGui::InputTextMultiline("Script 3", g_eddata.script[3], 2048);
    ImGui::InputTextMultiline("Script 4", g_eddata.script[4], 2048);
    ImGui::InputTextMultiline("Script 5", g_eddata.script[5], 2048);





    if(ImGui::Button("Add Trigger"))
    {
      g_eddata.add_trigger();
    }

    ImGui::EndGroup();
    ImGui::End();
	}
	fix_sprite_position(bg);
	RBVector2 manpos(man->_transform->world_x, man->_transform->world_y);
	RBVector2 campos(cam->world_x, cam->world_y);
	RBVector2 d(manpos - campos);

	RBVector2 v[4];
	bg->get_world_position(v);
	RBVector2 lb = v[1];
	RBVector2 rt = v[3];
  if (edit_mode)
	cam->zoomin(Input::get_mouse_scroller()*0.1);
	RBVector2 daabb(cam->world_w*cam->_zoom*0.5f, cam->world_h*cam->_zoom*0.5f);
	RBAABB2D bg_bound(lb, rt);
	RBAABB2D cam_aabb(campos - daabb, campos + daabb);

  if (edit_mode)
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
		f32 px, py;
		f32 s;
		switch (side)
		{
		case 1:
			px = RBMath::get_rand_range_f(bg_bound.min.x, bg_bound.max.x);
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
			s = RBMath::get_rand_range_f(HALF_PI, PI + HALF_PI);
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

	if (edit_mode)
		grid->debug_draw(cam, 1);

	scene->update_zorder_by_type_tag("character");

#if 1

	wchar_t wbuf[3] = L"确定";

	//this is utf-8 encode
	static char buf[1024] = "\xe4\xb8\xa5\xe8\x9b\x99\xe7\x8e\x8b\xe5\xad\x90";
	UnicodeToUTF_8(buf, wbuf);
	int k = strlen(buf);
	wchar_t wbuf_out[1024];
	memset(wbuf_out, 0, 1024 * sizeof(wchar_t));


	ImGui::InputText("UTF-8 input", buf, 1024);


	for (size_t i = 0, j = 0; i < strlen(buf); i += 3, j++)
	{
		UTF_8ToUnicode(&wbuf_out[j], &buf[i]);
	}




	//text_renderer->render_text(0, 700, wbuf_out, wcslen(wbuf_out), 200, cam);


	char t[9];
	memset(t, 0, 9);




	if (imgui_button_short(L"编辑"))
	{
		edit_mode = !edit_mode;
	}
	//grid->clear_data();

	if (ImGui::Button("Scale Character"))
	{
		man->scale(1.2f, 1.2f);
	}

	ImGui::Begin("Make Clip");
	static int wh[2] = {0};
	static char cname[64];
	ImGui::InputText("File name",cname,64);
	ImGui::InputInt2("W-H", wh);
	if (imgui_button_short(L"生成"))
	{
		f32 intervalx = 1.f / (f32)wh[0];
		f32 intervaly = 1.f / (f32)wh[1];

		{
			
			f32 ybase = 0.f;
			int i = 0;
			while (ybase + 0.001f < 1.f)
			{
				std::string sname = cname;
				sname += "_";
				char tmp[8];
				itoa(i++, tmp, 10);
				sname += tmp;
				sname += ".clip";
				std::ofstream fout(sname.data(), std::ios::ate);

				fout << "[head]\ntexture = " << cname << ".png\ntotal_frame = " << wh[0] << std::endl;
				char outs[128] = "lb_x = %f\nlb_y = %f\nlt_x = %f\nlt_y = %f\nrt_x = %f\nrt_y = %f\nrb_x = %f\nrb_y = %f\n\0";
				char outs1[256] = { 0 };
				f32 xbase = 0.f;
				u32 frame_number = 1;
				while (xbase + 0.001f < 1.f)
				{
					fout << "[" << frame_number++ << "]" << std::endl;
					sprintf(outs1, outs,
						xbase,
						ybase,
						xbase,
						ybase + intervaly,
						xbase + intervalx,
						ybase + intervaly,
						xbase + intervalx,
						ybase);
					xbase += intervalx;
					fout << outs1;
				}
				ybase += intervaly;
				fout.close();
			}

		}
	}
	ImGui::End();


	if (Input::get_key_up(WIP_R))
	{
		g_scene->reload_level();
	}
	if (Input::get_key_up(WIP_L))
	{
		g_scene->load_level();
	}
#endif
}

#if 0
void PlayerComponent::init()
{
	man_state = ManState::E_UP;
	cam->zoomout(1.0);
	host_object->translate_to(0, 0);
	g_audio_manager->Play(sound_start);

	subscribe_event(get_string_hash("add_hp"), WIP_EVENT_HANDLER(PlayerComponent, add_hp));


}

void PlayerComponent::add_hp(string_hash tp, void* ud)
{
	hp+=hp*0.1;
	if (hp > 100)
		hp = 100;
	acc += 1;
	if (acc > 100)
		acc = 100;
	killed++;
	static int created = 0;
	if (true)//killed % 2 == 0)
	{
		if (true)//RBMath::get_rand_i(900) > 450)
		{
			for (int i = 0; i < 1/*RBMath::get_rand_range_i(1, 5)*/; i++)
			{

				WIPSpriteCreator ctor_man(1.8f, 1.8f, WIPMaterialType::E_TRANSLUCENT);
				ctor_man.texture = enemy_texture;
				ctor_man.world_render = world_renderer;
				ctor_man.body_tp = WIPCollider::_CollisionTypes::E_RIGIDBODY;
				ctor_man.collider_sx = 1.f;
				ctor_man.collider_sy = 1.f;
				WIPSprite*  sp = WIPSpriteFactory::create_sprite(ctor_man);
				sp->_animation->add_clip(enemy_clip, enemy_clip->name);
				sp->translate_to(RBMath::get_rand_range_f(-20, 20), RBMath::get_rand_range_f(-20, 20));
				sp->set_tag("enemy");
				EnemeyComponent* pcc = new EnemeyComponent(sp);
				pcc->player_ref = host_object;
				pcc->sound = sound_fire;
				pcc->sound_death = sound_death_enemy;
				pcc->clip = enemy_clip;
				sp->add_tick_component(pcc);
				g_app->creating_object(sp);
				//scene->add_sprite(sp);

				WIPSpriteCreator ctor_blt(0.3f, 0.3f, WIPMaterialType::E_TRANSLUCENT);
				ctor_blt.texture = bullet_texture;
				ctor_blt.world_render = world_renderer;
				ctor_blt.body_tp = WIPCollider::_CollisionTypes::E_RIGIDBODY;
				ctor_blt.collider_sx = 1.f;
				ctor_blt.collider_sy = 1.f;
				WIPSprite*  spblt = WIPSpriteFactory::create_sprite(ctor_blt);
				spblt->_animation->add_clip(player_clip, player_clip->name);
				spblt->_animation->play_name(player_clip->name, false);
				spblt->_render->is_visible = false;
				int r = RBMath::get_rand_i(800);
				if (r < 0)
				{
					BulletComponent* pcc1 = new BulletComponent(spblt);
					pcc1->sound = sound_blast;
					spblt->add_tick_component(pcc1);
					spblt->set_tag("bullet_enemy");
					pcc->blt = spblt;
					spblt->translate_to(-28, 22 - created);
					pcc1->pos = RBVector2(-28, 22 - created);
					g_app->creating_object(spblt);
					//scene->add_sprite(spblt);

					WIPSpriteCreator ctor_pop(2.f, 2.f, WIPMaterialType::E_TRANSLUCENT);
					ctor_pop.texture = pop_texture;
					ctor_pop.world_render = world_renderer;
					ctor_pop.body_tp = WIPCollider::_CollisionTypes::E_NO_PHYSICS;
					ctor_pop.collider_sx = 1.f;
					ctor_pop.collider_sy = 1.f;
					WIPSprite*  sp_pop = WIPSpriteFactory::create_sprite(ctor_pop);
					sp_pop->_animation->add_clip(pop_clip, pop_clip->name);
					//sp_pop->_animation->play_name(pop_clip->name, false);
					sp_pop->set_z_order(-0.1f);
					sp_pop->set_tag("pop_enemy");
					sp_pop->_render->is_visible = false;
					auto cb = [](void* s)->void
					{
						((WIPSprite*)s)->_render->is_visible = false;
					};
					sp_pop->_animation->add_clip_callback(pop_clip->name, cb, (WIPSprite *)sp_pop);
					g_app->creating_object(sp_pop);
					//scene->add_sprite(sp_pop);

					pcc1->pop_obj = sp_pop;
				}
				else
				{
					BulletComponent1* pcc1 = new BulletComponent1(spblt);
					pcc1->sound = sound_blast;
					spblt->add_tick_component(pcc1);
					spblt->set_tag("bullet_enemy");
					pcc->blt = spblt;
					spblt->translate_to(-28, 22 - created);
					pcc1->pos = RBVector2(-28, 22 -  created );
					g_app->creating_object(spblt);
					//scene->add_sprite(spblt);

					WIPSpriteCreator ctor_pop(2.f, 2.f, WIPMaterialType::E_TRANSLUCENT);
					ctor_pop.texture = pop_texture;
					ctor_pop.world_render = world_renderer;
					ctor_pop.body_tp = WIPCollider::_CollisionTypes::E_NO_PHYSICS;
					ctor_pop.collider_sx = 1.f;
					ctor_pop.collider_sy = 1.f;
					WIPSprite*  sp_pop = WIPSpriteFactory::create_sprite(ctor_pop);
					sp_pop->_animation->add_clip(pop_clip, pop_clip->name);
					//sp_pop->_animation->play_name(pop_clip->name, false);
					sp_pop->set_z_order(-0.1f);
					sp_pop->set_tag("pop_enemy");
					sp_pop->_render->is_visible = false;
					auto cb = [](void* s)->void
					{
						((WIPSprite*)s)->_render->is_visible = false;
					};
					sp_pop->_animation->add_clip_callback(pop_clip->name, cb, (WIPSprite *)sp_pop);
					g_app->creating_object(sp_pop);
					//scene->add_sprite(sp_pop);

					pcc1->pop_obj = sp_pop;
				}
				created++;


			}
		}

	}
}

void PlayerComponent::update(f32 dt)
{
	static bool accing = false;
	float speed = 5.2f;

	
	
		if (acc <= 99)
		{
			acc += 0.1f;
		}
		if (Input::get_key_pressed(WIP_K))
		{
			if (acc > 0.5)
			{
				speed *= 2;
				acc -= 1;
			}
			/*
			if (acc < 0.5)
				accing = true;
			*/
		}
	

	if (Input::get_key_pressed(WIP_W))
	{
		host_object->translate(0, speed*dt);
		man_state = ManState::E_UP;
	}
	else if (Input::get_key_pressed(WIP_A))
	{
		host_object->translate(-speed*dt, 0);
		man_state = ManState::E_LEFT;
	}
	else if (Input::get_key_pressed(WIP_S))
	{
		host_object->translate(0, -speed*dt);
		man_state = ManState::E_DOWN;
	}
	else if (Input::get_key_pressed(WIP_D))
	{
		host_object->translate(speed*dt, 0);
		man_state = ManState::E_RIGHT;
	}
	f32 dd = 1.f;
	f32 dx = 0, dy = 0;
	{
		switch (man_state)
		{
		case ManState::E_DOWN:
			host_object->_animation->play_name("player_run", false);
			host_object->rotate_to(DEG2RAD(180));
			dy = -dd;
			break;
		case ManState::E_LEFT:
			host_object->_animation->play_name("player_run", false);
			host_object->rotate_to(DEG2RAD(90));
			dx = -dd;
			break;
		case ManState::E_RIGHT:
			host_object->_animation->play_name("player_run", false);
			host_object->rotate_to(DEG2RAD(270));
			dx = dd;
			break;
		case ManState::E_UP:
			host_object->_animation->play_name("player_run", false);
			host_object->rotate_to(DEG2RAD(0));
			dy = dd;
			break;
		}

	}

	cam->move_to(host_object->_transform->world_x, host_object->_transform->world_y);

	cam->zoomout(Input::get_mouse_scroller()*0.1f);

	static RBVector2 blt_d = RBVector2::zero_vector;
	if (blt->_render->is_visible == false)
	{
		//blt->_render->is_visible = false;
		if (Input::get_key_pressed(WIP_J))
		{
			g_audio_manager->Play(sound);
			blt->translate_to(host_object->_transform->world_x + dx * 1.5, host_object->_transform->world_y + dy * 1.5);
			blt_d.x = dx;
			blt_d.y = dy;
			((BulletComponent*)blt->tick_components[0])->v = blt_d;
			blt->_render->is_visible = true;

		}
	}
	else
	{

	}
	
	f32 dhp = 0;//16

	for (b2ContactEdge* ce = host_object->_collider->get_body()->GetContactList(); ce; ce = ce->next)
	{
		b2Contact* c = ce->contact;
		WIPSprite* s2 = (WIPSprite*)c->GetFixtureB()->GetBody()->GetUserData();
		if (s2->get_tag() == "bullet_enemy" || s2->get_tag() == "bullet")
		{
			if (!Input::get_key_pressed(WIP_K) || acc <= 0.1f)
				hp -= dhp;
			if (hp <= 0)
			{
				unsubscribe_all_events();
				g_audio_manager->Play(sound_death);
				g_app->pending_objects(host_object);
				host_object->send_event(get_string_hash("destory"));
				return;
			}
		}
		s2 = (WIPSprite*)c->GetFixtureA()->GetBody()->GetUserData();
		if (s2->get_tag() == "bullet_enemy" || s2->get_tag() == "bullet")
		{
			if (!Input::get_key_pressed(WIP_K)||acc<=0.1f)
				hp -= dhp;
			if (hp <= 0)
			{
				unsubscribe_all_events();
				g_audio_manager->Play(sound_death);
				g_app->pending_objects(host_object);
				host_object->send_event(get_string_hash("destory"));
				return;
			}
		}
	}
	//blt->_animation->play_name("player_run", false);

	//wchar_t words1[8]={0};
	//wsprintfW(words1, L"HP:%d\n", hp);
	//text_renderer->render_text(20, 600, words1, wcslen(words1), 800, cam);
	//text_renderer->render(cam);
	if (Input::get_key_down(WIP_P))
	{
		g_mem_manager->report();
	}
	ImGui::SetNextWindowPos(ImVec2(20, 20));
	ImGui::Begin("");
	imgui_label_short(L"血量：");
	ImGui::SameLine();
	ImGui::ProgressBar(hp / 100.f, ImVec2(100, 20));
	imgui_label_short(L"氮气：");
	ImGui::SameLine();
	ImGui::ProgressBar(acc / 100.f, ImVec2(100, 20));
	imgui_label_short(L"击杀：");
	ImGui::SameLine();
	ImGui::Text("%d", killed);
	ImGui::End();
}

void EnemeyComponent::init()
{
	man_state = ManState::E_DOWN;
	acc_t = 0.f;
	cur_direction = RBMath::get_rand_range_i(0, 3);
	subscribe_event(player_ref, get_string_hash("destory"), WIP_EVENT_HANDLER(EnemeyComponent,des_player));


	last_time = start_time = g_app->get_cur_time();

	fout.open("d1.data", std::ios::binary | std::ios::app);

	nn.load("d0.model");


}

void EnemeyComponent::des_player(string_hash tp, void* ud)
{
	player_ref = nullptr;
}



void EnemeyComponent::update(f32 dt)
{
	bool changing = false;
	int is_shotting = 0;
	int is_dying = 0;
	f32 dd = 1.f;
	f32 dx = 0, dy = 0;
	float speed = 3.2f;
	acc_t += dt;
	static f32 fixt = 1.5f ;
	int r = 0;
	ImGui::SliderFloat("", &fixt, 1.5f, 5.5f);
	f32 tf = fixt;// +RBMath::get_rand_range_f(-1.2, 1.2);

	data_pak_t df(RBVector2(player_ref->_transform->world_x, player_ref->_transform->world_y),
		RBVector2(host_object->_transform->world_x, host_object->_transform->world_y),
		RBVector2(((PlayerComponent*)(player_ref->tick_components[0]))->blt->_transform->world_x,
		((PlayerComponent*)(player_ref->tick_components[0]))->blt->_transform->world_y),
		RBVector2(blt->_transform->world_x, blt->_transform->world_y),
		is_shotting,
		is_dying,
		g_app->get_cur_time() - start_time
		);
	vec_t data = { df.player_pos.x, df.player_pos.y, df.enemey_pos.x, df.enemey_pos.y,
		df.player_bullet_pos.x, df.player_bullet_pos.y };
	auto res = nn.predict_label(data);

	if (res<3||acc_t > tf)
	{
		changing = true;
		cur_direction = RBMath::get_rand_range_i(0, 3);
		
		if (res < 3)
		{
			PlayerComponent::ManState state = ((PlayerComponent*)(player_ref->tick_components[0]))->man_state;
			switch (state)
			{
			case PlayerComponent::ManState::E_LEFT:
				cur_direction = 1;
				break;
			case PlayerComponent::ManState::E_RIGHT:
				cur_direction = 3;

				break;
			case PlayerComponent::ManState::E_UP:
				cur_direction = 0;

				break;
			case PlayerComponent::ManState::E_DOWN:
				cur_direction = 2;

				break;
			default:
				break;
			}
		}
		r = RBMath::get_rand_i(8);
		if(acc_t > tf)
		acc_t = 0;
	}
	else
	{
		if (host_object->_transform->world_x > 20
			)
		{
			host_object->translate_to(19, host_object->_transform->world_y);
			cur_direction = RBMath::get_rand_range_i(0, 3);
		}
		if (host_object->_transform->world_x < -20)
		{
			host_object->translate_to(-19, host_object->_transform->world_y);
			cur_direction = RBMath::get_rand_range_i(0, 3);
		}
		if (host_object->_transform->world_y > 20
			)
		{
			host_object->translate_to(host_object->_transform->world_x, 19);
			cur_direction = RBMath::get_rand_range_i(0, 3);
		}
		if (host_object->_transform->world_y < -20)
		{
			host_object->translate_to(host_object->_transform->world_x, -19);
			cur_direction = RBMath::get_rand_range_i(0, 3);
		}
	}

	int d = cur_direction;
	if (d == 0)
	{
		host_object->rotate_to(DEG2RAD(0));
		host_object->translate(0, speed*dt);

		//cam->move(0,speed*dt);
		dy = dd;
		man_state = ManState::E_UP;
	}
	else if (d == 1)
	{
		host_object->rotate_to(DEG2RAD(90));
		host_object->translate(-speed*dt, 0);

		man_state = ManState::E_LEFT;
		dx = -dd;
		//cam->move(-speed*dt, 0);
	}
	else if (d == 2)
	{
		host_object->rotate_to(DEG2RAD(180));
		host_object->translate(0, -speed*dt);

		dy = -dd;
		man_state = ManState::E_DOWN;

		//cam->move(0,-speed*dt);
	}
	else if (d == 3)
	{
		host_object->rotate_to(DEG2RAD(270));
		host_object->translate(speed*dt, 0);

		dx = dd;
		man_state = ManState::E_RIGHT;

		//cam->move(speed*dt, 0);
	}
	
	if (player_ref&&r >= 4)
	{
		//shot player
		f32 a = (player_ref->_transform->world_x - host_object->_transform->world_x);
		if (a > 0)
		{
			dx = dd;
			man_state = ManState::E_RIGHT;
		}
		else
		{
			dx = -dd;
			man_state = ManState::E_LEFT;
		}
		f32 b = (player_ref->_transform->world_y - host_object->_transform->world_y);
		if (b > 0)
		{
			dy = dd;
			man_state = ManState::E_UP;
		}
		else
		{
			dy = -dd;
			man_state = ManState::E_DOWN;
		}
	}
	
		static RBVector2 blt_d = RBVector2::zero_vector;
		if (blt->_render->is_visible == false)
		{

			if (player_ref&& r >= 4)
			{
				//shot
				f32 a = (player_ref->_transform->world_x - host_object->_transform->world_x);
				f32 b = (player_ref->_transform->world_y - host_object->_transform->world_y);

				if (
					
					(a > 0 && man_state == ManState::E_RIGHT&&RBMath::abs(b)<8) ||
					(a < 0 && man_state == ManState::E_LEFT&&RBMath::abs(b)<8) ||
					(b>0 && man_state == ManState::E_UP&&RBMath::abs(a)<8) ||
					(b < 0 && man_state == ManState::E_DOWN&&RBMath::abs(a)<8)
					
					)
				{
					//g_audio_manager->Play(sound);
					blt->translate_to(host_object->_transform->world_x + dx * 1.5, host_object->_transform->world_y + dy * 1.5);

					blt_d.x = dx;
					blt_d.y = dy;
					((BulletComponent*)blt->tick_components[0])->v = blt_d;
					blt->_render->is_visible = true;
					is_shotting = 1;
				}
			}

		}
		for (b2ContactEdge* ce = host_object->_collider->get_body()->GetContactList(); ce; ce = ce->next)
		{
			b2Contact* c = ce->contact;
			WIPSprite* s2 = (WIPSprite*)c->GetFixtureB()->GetBody()->GetUserData();
			if (s2->get_tag() == "bullet")
			{
				host_object->send_event(get_string_hash("add_hp"));
				g_audio_manager->Play(sound_death);
				unsubscribe_all_events();
				g_app->pending_objects(host_object);
				is_dying = 1;
				//return;
			}
			s2 = (WIPSprite*)c->GetFixtureA()->GetBody()->GetUserData();
			if (s2->get_tag() == "bullet")
			{
				host_object->send_event(get_string_hash("add_hp"));
				g_audio_manager->Play(sound_death);
				unsubscribe_all_events();
				g_app->pending_objects(host_object);
				is_dying = 1;
				//return;
			}
		}

		//ms
		f32 interval = 500;
		f32 ct = g_app->get_cur_time();
		//if (is_dying||changing)
		{
			RBVector2 player_blt_pos(((PlayerComponent*)(player_ref->tick_components[0]))->blt->_transform->world_x,
				((PlayerComponent*)(player_ref->tick_components[0]))->blt->_transform->world_y);
			last_time = g_app->get_cur_time();
			data_pak_t d(RBVector2(player_ref->_transform->world_x, player_ref->_transform->world_y),
				RBVector2(host_object->_transform->world_x, host_object->_transform->world_y),
				player_blt_pos,
				RBVector2(blt->_transform->world_x, blt->_transform->world_y),
				is_shotting,
				is_dying,
				g_app->get_cur_time() - start_time
				);
			data_out.push_back(d);

		}
		if (is_dying)
		{
			//write data
			/*
			for (auto i : data_out)
			{
			fout << i.player_pos.x << ',' << i.player_pos.y << '|'
			<< i.enemey_pos.x << ',' << i.enemey_pos.y << '|'
			<< i.player_bullet_pos.x << ',' << i.player_bullet_pos.y << '|'
			<< i.enemey_bullet_pos.x << ',' << i.enemey_bullet_pos.y << '|'
			<< is_shotting << '|' << is_dying << '|' << i.cur_time_ms<<'|' << std::endl;
			}
			*/
			fout.write((const char*)data_out.data(), data_out.size()*sizeof(data_pak_t));
			data_out.clear();
		}
		
		//last_time = g_app->get_cur_time();
}

void EnemeyComponent::destroy()
{
	fout.close();
}

void BulletComponent::update(f32 dt)
{
	
	int i = host_object->_collider->get_collision_list_size();
	if ( i > 0)
	{
		g_audio_manager->Play(sound);
		pop_obj->_render->is_visible = true;
		pop_obj->translate_to(host_object->_transform->world_x, host_object->_transform->world_y);
		pop_obj->_animation->play_name("pop", false);

		host_object->_render->is_visible = false;
		v = RBVector2::zero_vector;
		host_object->translate_to(pos.x, pos.y);
	}
	else
		host_object->_render->is_visible = true;

	if (host_object->_transform->world_x < -20 || host_object->_transform->world_x>20 || host_object->_transform->world_y > 20 || host_object->_transform->world_y < -20)
	{
		host_object->_render->is_visible = false;
		host_object->translate_to(pos.x, pos.y);
		s = 0;
		main_axis = -1;
	}
	else
	{
		//host_object->_render->is_visible = true;

		f32 blt_speed = 16.5f;
		//v.y -= 1.8*dt;

		if (main_axis == -1)
		{
			if (RBMath::abs(v.x) < 0.1)
			{
				main_axis = 0;
				
			}
			else if (RBMath::abs(v.y) < 0.1)
			{
				main_axis = 1;
				
			}
		}
		if (main_axis == 1)
		{
			v.y = 1*RBMath::sin(s*20);
			s += v.x*dt;
		}
		else if (main_axis == 0)
		{
			v.x = 1*RBMath::sin(s*20 );
			s += v.y*dt;
		}

		host_object->translate(v.x*blt_speed*dt, v.y*blt_speed*dt);
	}
}

void BulletComponent1::update(f32 dt)
{
	int i = host_object->_collider->get_collision_list_size();
	if (i > 0)
	{
		g_audio_manager->Play(sound);
		
		pop_obj->_render->is_visible = true;
		pop_obj->translate_to(host_object->_transform->world_x, host_object->_transform->world_y);
		pop_obj->_animation->play_name("pop", false);

		host_object->_render->is_visible = false;
		v = RBVector2::zero_vector;
		host_object->translate_to(pos.x, pos.y);
	}
	else
		host_object->_render->is_visible = true;

	if (host_object->_transform->world_x < -20 || host_object->_transform->world_x>20 || host_object->_transform->world_y > 20 || host_object->_transform->world_y < -20)
	{
		host_object->_render->is_visible = false;
		host_object->translate_to(pos.x, pos.y);
	}
	else
	{
		//host_object->_render->is_visible = true;

		f32 blt_speed = 10.5f;


		host_object->translate(v.x*blt_speed*dt, v.y*blt_speed*dt);
	}
}
#endif

NPCComponent::NPCComponent(WIPSprite*  s) :WIPTickComponent(s)
{

}
NPCComponent::~NPCComponent()
{

}
void NPCComponent::init()
{
	//pre:on load
	data.face = nullptr;
	data.words = nullptr;
	set_default_face();
	map_component = (MapComponent*)g_scene->get_sprite_by_tag("bg")->get_component<MapComponent>();
}
void NPCComponent::update(f32 dt)
{
	/*
	WIPTickComponent* tc = WIPObject::create_tick_component("MapComponent", this->host_object);
	tc->init();
	tc->update(1);
	*/
	return;
}
void NPCComponent::destroy()
{

}

void NPCComponent::on_begin_contact(const WIPSprite* s)
{
  
	//LOG_INFO("begin contact:%s", s->get_tag().data());
}

void NPCComponent::on_end_contact(const WIPSprite* s)
{
	send_event(get_string_hash("player"));
	//LOG_INFO("end contact:%s", s->get_tag().data());
}

void NPCComponent::on_contact(const WIPSprite* s, float dt)
{
	if (s->get_tag() == "man")
	{
		if (map_component->cur_npc_ui != nullptr&&map_component->cur_npc_ui != &data)
			return;
		if (Input::get_sys_key_down(WIP_SPACE))
		{
			if (words[0].empty())
			{
				/*
				while (!words[1].empty())
				{
					words[0].push(words[1].front());
					words[1].pop();
				}
				*/
				words[0].swap(words[1]);
				send_event(get_string_hash("player"));
				return;
			}
      g_audio_manager->Play("event:/talk_se");
			data.words = words[0].front();
			send_event(get_string_hash("npc talk"), &data);
			words[1].push(words[0].front());
			words[0].pop();

		}
	}
	//LOG_INFO("contacting:%s", s->get_tag().data());
}

void NPCComponent::add_faces(std::string name, WIPTexture2D* tex)
{
	npc_faces[get_string_hash(name.data())] = tex;
}
void NPCComponent::set_face(std::string name)
{
	data.face = npc_faces[get_string_hash(name.data())];
}
void NPCComponent::set_no_face()
{
	data.face = nullptr;
}

void NPCComponent::set_default_face()
{
	if (npc_faces.empty())
	{
		set_no_face();
		return;
	}
	if (npc_faces.find(get_string_hash("dft"))==npc_faces.end())
	{
		set_no_face();
		return;
	}
	set_face("dft");
}



TransformComponent::TransformComponent(WIPSprite*  s) :WIPTickComponent(s)
{
  call_data[0] = 0;
  call_data[1] = 0;
  call_data[2] = 0;
  call_data[3] = 0;
}
TransformComponent::~TransformComponent()
{

}
void TransformComponent::init()
{
  auto* s = g_scene->get_sprite_by_tag("bg");
    if (s)
      map_component = (MapComponent*)s->get_component<MapComponent>();
 

}
void TransformComponent::update(f32 dt)
{
  if (func_update)
    func_update(call_data[0], dt,this);
  return;
}
void TransformComponent::destroy()
{
  //delete call_data[0];
  //delete call_data[1];
  //delete call_data[2];
  //delete call_data[3];
  //parameters should be deleted by level
}

void TransformComponent::on_begin_contact(const WIPSprite* s)
{
  //此处处于物理模拟中不能在此创建对象
  //TODO：任何创建销毁都应该被推迟到帧尾
  if (func_begin)
  func_begin(call_data[1], s,this);

}

void TransformComponent::on_end_contact(const WIPSprite* s)
{
  if (func_end)
  func_end(call_data[3], s,this);
}

void TransformComponent::on_contact(const WIPSprite* s, float dt)
{
  //g_scene->load_level(id,pos);
  if (func_contact)
  func_contact(call_data[2], s,dt,this);
  return;
}


void TransformComponent::start()
{
  if (func_level_start)
    func_level_start(call_data[LEVEL_START], this);
}

void TransformComponent::end()
{
  if (func_level_end)
    func_level_end(call_data[LEVEL_END], this);
}