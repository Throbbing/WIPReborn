#include "GLFWApp.h"
#include "../AnimationClip.h"
#include "../AnimationManager.h"
#include "../FileSystem.h"
#include "../IniHelper.h"
#include "../Logger.h"
#include "../QuadTree.h"
#include "../Render.h"
#include "../ResourceManager.h"
#include "../Scene.h"
#include "../ScriptManager.h"
#include "../Sprite.h"
#include "../WIPTime.h"
#include "../InputManager.h"
#include "../Input.h"
#include "../Camera.h"
#ifndef _WIN32
#include "unistd.h"
#endif


int GLFWApp::pack_sprite(const WIPSprite **sprite, void *mem, int n, int offset_n)
{
	float *p = (float*)mem;
	float s[sizeof(float) * 44];
	int k = -1 + offset_n;
	while (n--)
	{
		k++;
		float w1 = ofGetWindowWidth() / 2;
		float w2 = ofGetWindowHeight() / 2;
		//03
		//12
		// todo:padding 1 float

		if (!sprite[k])
			continue;
		/*
		if(sprite[k]->handled)
		continue;
		*/
		RBVector2 vert[4];
		sprite[k]->get_world_position(vert);
		s[0] = vert[0].x / w1*zoom;
		s[1] = vert[0].y / w2*zoom;
		s[2] = sprite[k]->_transform->z_order;
		s[3] = sprite[k]->_render->material.vert_color[0].r;
		s[4] = sprite[k]->_render->material.vert_color[0].g;
		s[5] = sprite[k]->_render->material.vert_color[0].b;
		s[6] = 0;
		s[7] = 0;
		s[8] = 1;
		// from framebox
		s[9] = sprite[k]->_animation->_framebox_ref.lb.x;//0
		s[10] = sprite[k]->_animation->_framebox_ref.lb.y;//0

		s[11] = vert[1].x / w1*zoom;
		s[12] = vert[1].y / w2*zoom;
		s[13] = sprite[k]->_transform->z_order;
		s[14] = sprite[k]->_render->material.vert_color[0].r;
		s[15] = sprite[k]->_render->material.vert_color[0].g;
		s[16] = sprite[k]->_render->material.vert_color[0].b;
		s[17] = 0;
		s[18] = 0;
		s[19] = 1;
		// from framebox
		s[20] = sprite[k]->_animation->_framebox_ref.lt.x;//0
		s[21] = sprite[k]->_animation->_framebox_ref.lt.y;//1

		s[22] = vert[2].x / w1*zoom;
		s[23] = vert[2].y / w2*zoom;
		s[24] = sprite[k]->_transform->z_order;
		s[25] = sprite[k]->_render->material.vert_color[0].r;
		s[26] = sprite[k]->_render->material.vert_color[0].g;
		s[27] = sprite[k]->_render->material.vert_color[0].b;
		s[28] = 0;
		s[29] = 0;
		s[30] = 1;
		// from framebox
		s[31] = sprite[k]->_animation->_framebox_ref.rt.x;//1
		s[32] = sprite[k]->_animation->_framebox_ref.rt.y;//1

		s[33] = vert[3].x / w1*zoom;
		s[34] = vert[3].y / w2*zoom;
		s[35] = sprite[k]->_transform->z_order;
		s[36] = sprite[k]->_render->material.vert_color[0].r;
		s[37] = sprite[k]->_render->material.vert_color[0].g;
		s[38] = sprite[k]->_render->material.vert_color[0].b;
		s[39] = 0;
		s[40] = 0;
		s[41] = 1;
		// from framebox
		s[42] = sprite[k]->_animation->_framebox_ref.rb.x;//1
		s[43] = sprite[k]->_animation->_framebox_ref.rb.y;//1
		memcpy(p, s, sizeof(float) * 44);
		p += 44;
		//LOG_WARN("%d",p-mem);
		if ((int)(p + 44 - (float*)mem) >= MEMSIZE / sizeof(float))
		{
			//LOG_WARN("Copy overflow!Break!");
			return k + 1;
		}
	}
	//draw done!
	return -1;
}

/*
03
12
*/
void GLFWApp::pack_index(void *mem, int n)
{
	unsigned int* p = (unsigned int*)mem;
	unsigned int s[6];
	int k = -1;
	int off = 0;
	while (n--)
	{
		++k;
		s[0] = 0 + off;
		s[1] = 1 + off;
		s[2] = 3 + off;
		s[3] = 1 + off;
		s[4] = 2 + off;
		s[5] = 3 + off;
		off += 4;
		memcpy(p, s, 6 * sizeof(unsigned int));
		p += 6;
	}
}


bool GLFWApp::init()
{
	window_w = 1024;
	window_h = 768;
	bool ret = create_window("test");

	WIPFileSystem &fs = *g_filesystem;
	std::string cur_path = fs.get_current_dir();

	fs.set_current_dir("./data/");
	LOG_NOTE("cur path:%s", fs.get_current_dir().c_str());

	std::string lua_project_path;
	std::string lua_ini_path;
	std::string log_path;
	f32 fps;

	std::string path = "WIPCFG.ini";
	if (WIPIniHelper::reset_ini_file(path.data()))
	{
		WIPIniHelper::get_float("Common", "fps", fps);
		WIPIniHelper::get_string("Common", "project", lua_project_path);
		WIPIniHelper::get_string("Common", "script_init", lua_ini_path);
		WIPIniHelper::get_string("Common", "log", log_path);
		WIPIniHelper::close();
	}
	else
	{
		LOG_ERROR("Read ini failed!\n");
		require_exit();
	}

	g_logger->startup(log_path.c_str());
	g_logger->new_log();

	g_script_manager->startup();
	g_script_manager->load_file(lua_project_path.c_str());

	times = new TimeSource();
	RBClock::init(times);
	clock = RBClock::Instance();

	timer = new RBTimerBase(*clock);
	clock->set_filtering(10, 1.f / fps);
	lastTime = timer->get_time();
	clock->update();

	_frame = 1.f / fps;

	g_rhi->init();


	scene = new WIPScene();
	scene->init(1, 1, 6);


	world_renderer = new WorldRender();
	world_renderer->init();
	world_renderer->set_world(scene);

	

	clip = WIPAnimationClip::create_with_atlas("walk_down", "./clips/1.clip");
	clip1 = WIPAnimationClip::create_with_atlas("walk_left", "./clips/2.clip");
	clip2 = WIPAnimationClip::create_with_atlas("walk_right", "./clips/3.clip");
	clip3 = WIPAnimationClip::create_with_atlas("walk_up", "./clips/4.clip");
	clip_s = WIPAnimationClip::create_with_atlas("stand_down", "./clips/1s.clip");
	clip1_s = WIPAnimationClip::create_with_atlas("stand_left", "./clips/2s.clip");
	clip2_s = WIPAnimationClip::create_with_atlas("stand_right", "./clips/3s.clip");
	clip3_s = WIPAnimationClip::create_with_atlas("stand_up", "./clips/4s.clip");

	auto res_handle1 = g_res_manager->load_resource("./pic/zhaolinger_1_8.png", WIPResourceType::TEXTURE);
	int ww = ((TextureData *)(res_handle1->extra))->width;
	int hh = ((TextureData *)(res_handle1->extra))->height;
	auto res_handle2 = g_res_manager->load_resource("./pic/xianling_5.jpg", WIPResourceType::TEXTURE);
	int ww1 = ((TextureData *)(res_handle2->extra))->width;
	int hh1 = ((TextureData *)(res_handle2->extra))->height;
	auto res_handle1mask = g_res_manager->load_resource("./pic/xianling_5_2.png", WIPResourceType::TEXTURE);
	int ww1mask = ((TextureData *)(res_handle1mask->extra))->width;
	int hh1mask = ((TextureData *)(res_handle1mask->extra))->height;
	auto res_handle1fog = g_res_manager->load_resource("./pic/fog.png", WIPResourceType::TEXTURE);
	int ww1fog = ((TextureData *)(res_handle1fog->extra))->width;
	int hh1fog = ((TextureData *)(res_handle1fog->extra))->height;

	float rot = ww / (float)hh;



	auto res_lixiaoyao = g_res_manager->load_resource("./pic/lixiaoyao_orijan.png", WIPResourceType::TEXTURE);
	int wli = ((TextureData *)(res_lixiaoyao->extra))->width;
	int hli = ((TextureData *)(res_lixiaoyao->extra))->height;
	auto res_zaji1 = g_res_manager->load_resource("./pic/suzhou_people.png", WIPResourceType::TEXTURE);
	int wzaji1 = ((TextureData *)(res_zaji1->extra))->width;
	int hzaji1 = ((TextureData *)(res_zaji1->extra))->height;
	auto res_zaji2 = g_res_manager->load_resource("./pic/suzhou_people2.png", WIPResourceType::TEXTURE);
	int wzaji2 = ((TextureData *)(res_zaji2->extra))->width;
	int hzaji2 = ((TextureData *)(res_zaji2->extra))->height;
	auto res_crowd = g_res_manager->load_resource("./pic/crowd.png", WIPResourceType::TEXTURE);
	int wcrow = ((TextureData *)(res_crowd->extra))->width;
	int hcrow = ((TextureData *)(res_crowd->extra))->height;

	float rotli = wli / (float)hli;

	tex2d = g_rhi->RHICreateTexture2D(ww, hh, res_handle1->ptr);
	tex2d1 = g_rhi->RHICreateTexture2D(ww1, hh1, res_handle2->ptr);
	//issue:texture boarder
	tex2d1mask = g_rhi->RHICreateTexture2D(ww1, hh1, res_handle1mask->ptr , 0, 0, 0, 1);

	tex2d_fog = g_rhi->RHICreateTexture2D(ww1fog, hh1fog, res_handle1fog->ptr);

	tex2d_lixiaoyao = g_rhi->RHICreateTexture2D(wli, hli, res_lixiaoyao->ptr,0,0,0,1);
	tex2d_zaji1 = g_rhi->RHICreateTexture2D(wzaji1, hzaji1, res_zaji1->ptr);
	tex2d_zaji2 = g_rhi->RHICreateTexture2D(wzaji2, hzaji2, res_zaji2->ptr);
	tex2d_crowd = g_rhi->RHICreateTexture2D(wcrow, hcrow, res_crowd->ptr);




	WIPSpriteCreator ctor_man(3.f*rot, 3.f, WIPMaterialType::E_TRANSLUCENT);
	ctor_man.texture = tex2d;
	ctor_man.world_render = world_renderer;

	WIPSpriteCreator ctor_mask(40.f, 30.f, WIPMaterialType::E_TRANSLUCENT);
	ctor_mask.texture = tex2d1mask;
	ctor_mask.world_render = world_renderer;


	WIPSpriteCreator ctor_bg(40.f, 30.f, WIPMaterialType::E_TRANSLUCENT);
	ctor_bg.texture = tex2d1;
	ctor_bg.world_render = world_renderer;

	WIPSpriteCreator ctor_fog(20.f, 20.f, WIPMaterialType::E_TRANSLUCENT);
	ctor_fog.texture = tex2d_fog;
	ctor_fog.world_render = world_renderer;

	WIPSpriteCreator ctor_li(3.f*rotli, 3.5f, WIPMaterialType::E_TRANSLUCENT);
	ctor_li.texture = tex2d_lixiaoyao;
	ctor_li.world_render = world_renderer;

	WIPSpriteCreator ctor_zaji1(1.2f*1.5f, 3.6f, WIPMaterialType::E_TRANSLUCENT);
	ctor_zaji1.texture = tex2d_zaji1;
	ctor_zaji1.world_render = world_renderer;

	WIPSpriteCreator ctor_zaji2(1.8f*1.2f, 2.4f, WIPMaterialType::E_TRANSLUCENT);
	ctor_zaji2.texture = tex2d_zaji2;
	ctor_zaji2.world_render = world_renderer;

	WIPSpriteCreator ctor_crowd(6*1.2f, 6.f, WIPMaterialType::E_TRANSLUCENT);
	ctor_crowd.texture = tex2d_crowd;
	ctor_crowd.world_render = world_renderer;

	man_lixiaoyao = WIPSpriteFactory::create_sprite(ctor_li);
	man_lixiaoyao->_animation->add_clip(clip, clip->name);
	man_lixiaoyao->_animation->add_clip(clip1, clip1->name);
	man_lixiaoyao->_animation->add_clip(clip2, clip2->name);
	man_lixiaoyao->_animation->add_clip(clip3, clip3->name);
	man_lixiaoyao->_animation->add_clip(clip_s, clip_s->name);
	man_lixiaoyao->_animation->add_clip(clip1_s, clip1_s->name);
	man_lixiaoyao->_animation->add_clip(clip2_s, clip2_s->name);
	man_lixiaoyao->_animation->add_clip(clip3_s, clip3_s->name);
	man_lixiaoyao->_animation->play(clip1_s);

	zaji1 = WIPSpriteFactory::create_sprite(ctor_zaji1);
	zaji1->_animation->add_clip(clip, clip->name);
	zaji1->_animation->play(clip, true);

	zaji2 = WIPSpriteFactory::create_sprite(ctor_zaji2);
	zaji2->_animation->add_clip(clip, clip->name);
	zaji2->_animation->play(clip, true);


	crowd = WIPSpriteFactory::create_sprite(ctor_crowd);
	crowd->_animation->add_clip(clip_s, clip_s->name);
	crowd->_animation->play(clip_s);


	bg = WIPSpriteFactory::create_sprite(ctor_bg);
	bg_mask = WIPSpriteFactory::create_sprite(ctor_mask);
	man = WIPSpriteFactory::create_sprite(ctor_man);
	//clip->bloop = true;
	man->_animation->add_clip(clip, clip->name);
	//clip1->bloop = true;

	man->_animation->add_clip(clip1, clip1->name);
	//clip2->bloop = true;

	man->_animation->add_clip(clip2, clip2->name);
	//clip3->bloop = true;

	man->_animation->add_clip(clip3, clip3->name);


	man->_animation->add_clip(clip_s, clip_s->name);
	//clip1->bloop = true;

	man->_animation->add_clip(clip1_s, clip1_s->name);
	//clip2->bloop = true;

	man->_animation->add_clip(clip2_s, clip2_s->name);
	//clip3->bloop = true;

	man->_animation->add_clip(clip3_s, clip3_s->name);

	man->_animation->play(clip_s);
	bg->set_tag("bg");
	bg_mask->set_tag("mask");
	man->set_tag("man");
	man_lixiaoyao->set_tag("lixiaoyao");
	zaji1->set_tag("zaji1");
	zaji2->set_tag("zaji2");
	crowd->set_tag("crowd");




	scene->add_sprite(bg);
	scene->add_sprite(bg_mask);
	scene->add_sprite(man);
	scene->add_sprite(man_lixiaoyao);
	scene->add_sprite(zaji1);
	scene->add_sprite(zaji2);
	scene->add_sprite(crowd);

	man_lixiaoyao->set_z_order(0.4f);
	zaji1->set_z_order(0.4f);
	zaji2->set_z_order(0.4f);
	crowd->set_z_order(0.4f);

	man_lixiaoyao->translate_to(5, 2);
	zaji1->translate_to(-3, 3);
	zaji2->translate_to(-1,3);
	crowd->translate_to(-2, -2);


	fogs = WIPSpriteFactory::create_sprite(ctor_fog);
	scene->add_sprite(fogs);

	fogs->translate_to(0.f, 0.f);
	fogs->set_z_order(0.05);

	bg->translate_to(0.f, 0.f);
	bg_mask->translate_to(0.f, 0.f);
	man->translate_to(5.f, 5.f);
	bg->set_z_order(0.5f);
	man->set_z_order(0.4f);
	bg_mask->set_z_order(0.1f);

	cameras.push_back(scene->create_camera(20.f, 20.f, window_w, window_h));
	cameras[0]->move_to(5.f, 5.f);

	//g_res_manager->free(res_handle1, res_handle1->size);
	g_input_manager->startup("");
	g_animation_manager->startup(0.15);

	pre_clip = nullptr;
	return ret;
}

void GLFWApp::run() {
	while (!glfwWindowShouldClose(window))
	{
		if (_exit_requist)
			break;
		curTime = times->get_time();
		if (curTime - lastTime >= _frame)
		{
			//////////////////////////////////////////////////////////////////////////
			// only this order!!!!!
			// only to put input handle here to keep everything right
			{

				int bits = g_input_manager->get_last_key_info()->key_bit;
				int bits_c = g_input_manager->get_last_key_info()[1].key_bit;

				if (lmouse_keep_going) {
					bits |= WIP_MOUSE_LBUTTON;
				}
				if (rmouse_keep_going)
					bits |= WIP_MOUSE_RBUTTON;
				if (mmouse_keep_going)
					bits |= WIP_MOUSE_MBUTTON;
				g_input_manager->update(bits_c, bits);
			}

			glfwPollEvents();
			//////////////////////////////////////////////////////////////////////////
			//LOG_NOTE("%f",clock->get_frame_time());
			g_animation_manager->update(clock->get_frame_time());
			// update physics
			g_script_manager->call("main_logic");

			//glDepthMask(GL_TRUE);
			glViewport(0, 0, window_w, window_h);
			glClearColor(0.85, 0.85, 0.85, 1);
			glClearDepth(1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


			//g_rhi->set_shader(0);
			g_rhi->begin_debug_context();
			g_rhi->change_debug_color(RBColorf::red);
			//quad_tree->debug_draw();
			g_rhi->debug_draw_aabb2d(RBVector2(-1, -1), RBVector2(0.99f, 0.99f), 1, 1);
			g_rhi->debug_submit();
			g_rhi->end_debug_context();

			cameras[0]->zoomin(Input::get_mouse_scroller()*0.1);


			f32 dt = clock->get_frame_time();

			fogs->translate(dt*0.2, dt*0.2);

			//man->translate(dt*-0.1, 0);
			float speed = 3.2f;
			if (Input::get_key_pressed(WIP_W))
			{
				man->translate(0, speed*dt);
				if (man->_animation->play(clip3))
				{
					pre_clip = clip3;
				}
				//cameras[0]->move(0,speed*dt);
				man_state = ManState::E_UP;
			}
			else if (Input::get_key_pressed(WIP_A))
			{
				man->translate( -speed*dt,0);
				if(man->_animation->play(clip1))
				{
					pre_clip = clip1;
				}
				man_state = ManState::E_LEFT;

				//cameras[0]->move(-speed*dt, 0);
			}
			else if (Input::get_key_pressed(WIP_S))
			{
				man->translate(0, -speed*dt);
				if(man->_animation->play(clip))
				{
					pre_clip = clip;
				}
				man_state = ManState::E_DOWN;

				//cameras[0]->move(0,-speed*dt);
			}
			else if (Input::get_key_pressed(WIP_D))
			{
				man->translate(speed*dt, 0);
				if(man->_animation->play(clip2))
				{
					pre_clip = clip2;
				}
				man_state = ManState::E_RIGHT;

				//cameras[0]->move(speed*dt, 0);
			}
			else
			{
				switch (man_state)
				{
				case ManState::E_DOWN:
					man->_animation->play(clip_s);
					break;
				case ManState::E_LEFT:
					man->_animation->play(clip1_s);
					break;
				case ManState::E_RIGHT:
					man->_animation->play(clip2_s);
					break;
				case ManState::E_UP:
					man->_animation->play(clip3_s);
					break;
				}
				//man->_animation->play(clip2);
			}


			for (auto i : cameras)
				world_renderer->render(i);


			// g_script_manager->call("debug_draw");

			lastTime = curTime;
			clock->update();
			// g_sound_palyer->update();

			glfwSwapBuffers(window);
			g_input_manager->clear_states();
			// for glfw
			g_input_manager->clear_scroller();
		}
		else {
#ifdef _WIN32
			Sleep(0);
#else
			usleep(0);
#endif
		}
	}
}

GLFWApp::~GLFWApp()
{
	g_script_manager->shutdown();
	g_logger->shutdown();
	glfwDestroyWindow(window);
	glfwTerminate();
}

