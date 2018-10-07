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
#include "../PhysicsManager.h"
#ifndef _WIN32
#include "unistd.h"
#endif

#include "UserComponent.h"
#include "AudioManager.h"
#include "RemoteryProfiler.h"

#ifdef _WIN32
#include "windows.h"
#endif // _WIN32

GLFWApp* g_app = nullptr;

f32 GLFWApp::get_cur_time() const
{
	return timer->get_time_ms();
}

bool GLFWApp::init()
{

	if (!g_pool_allocator->init())
		LOG_ERROR("Pool allocator init failed!");

	//WIPSprite::init_mem();
	//WIPAnimation::init_mem();
	

	WIPFileSystem &fs = *g_filesystem;
	std::string cur_path = fs.get_current_dir();

	fs.set_current_dir("./data/");
	LOG_NOTE("cur path:%s", fs.get_current_dir().c_str());

	fs.get_relative_path(cur_path);

	std::string lua_project_path;
	std::string lua_ini_path;
	std::string log_path;
	f32 fps;

	std::string apath;

	std::string path = "WIPCFG.ini";
	if (WIPIniHelper::reset_ini_file(path.data()))
	{
		WIPIniHelper::get_float("Common", "fps", fps);
		WIPIniHelper::get_string("Common", "project", lua_project_path);
		WIPIniHelper::get_string("Common", "script_init", lua_ini_path);
		WIPIniHelper::get_string("Common", "log", log_path);


		WIPIniHelper::close();
		LOG_INFO("Reading ini file...");
	}
	else
	{
		LOG_ERROR("Read ini failed!\n");
		require_exit();
	}

	LOG_INFO("Logger start up...");
	g_logger->startup(log_path.c_str());

	g_logger->new_log();

	LOG_INFO("Logger start up success...");


	g_script_manager->startup();
	g_script_manager->load_file(lua_project_path.c_str());
	LOG_INFO("Script start up...");


	times = new TimeSource();
	RBClock::init(times);
	clock = RBClock::Instance();

	timer = new RBTimerBase(*clock);
	clock->set_filtering(10, 1.f / fps);
	lastTime = timer->get_time();
	clock->update();
	LOG_INFO("Time start up...");


	_frame = 1.f / fps;

	g_rhi->init();
	LOG_INFO("RHI start up...");
	imgui_renderer = new GlfwImguiRender();
	imgui_renderer->imgui_init(window, "./font/simkai.ttf", 19);
	LOG_INFO("IMGUI start up 200M+ memory used...");
	g_scene->init(1, 1, 4);
	LOG_INFO("Creating scene...");


	


	g_physics_manager->startup();
	LOG_INFO("Physics start up...");


	g_input_manager->startup("");
	LOG_INFO("Input start up...");

	g_animation_manager->startup(0.15f);
	LOG_INFO("Animation start up...");

	RemoteryProfiler::startup();
	regist_user_component();

	g_temp_uisys->startup();

	

	return true;
}

void GLFWApp::run()
{

	while (((!glfwWindowShouldClose(window)) && (!_exit_requist)))
	{

		if (_exit_requist)
			break;
		curTime = times->get_time();
		if (curTime - lastTime >= _frame)
		{
			rmt_BeginCPUSample(frame_time_avaliable, 0);

			//////////////////////////////////////////////////////////////////////////
			// only this order!!!!!
			// only to put input handle here to keep everything right
			{
				rmt_BeginCPUSample(handle_input, RMTSF_Aggregate);

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

				rmt_EndCPUSample();
			}

			rmt_BeginCPUSample(glfw_poll_event, 0);
			glfwPollEvents();
			rmt_EndCPUSample();
			//////////////////////////////////////////////////////////////////////////
			imgui_renderer->imgui_new_frame();

			rmt_BeginCPUSample(script_main, 0);
			g_script_manager->call("main_logic");
			rmt_EndCPUSample();

			//todo:move to camera::clear add rhi!
			
			g_rhi->clear_back_buffer(RBColorf::black);
			//glViewport(0, 0, window_w, window_h);
			//glClearColor(0.85, 0.85, 0.85, 1);
			glClearDepth(1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


			//g_rhi->set_shader(0);
#ifdef DEBUGDRAW
			rmt_BeginCPUSample(debug_draw_cpu, 0);
			rmt_BeginOpenGLSample(debug_draw);
			g_rhi->begin_debug_context();
			g_rhi->change_debug_color(RBColorf::red);
			scene->quad_tree->debug_draw(cameras[0]);
			//g_rhi->debug_draw_aabb2d(RBVector2(-0.5f, -0.5f), RBVector2(0.5f, 0.5f), cameras[0]);
			g_rhi->debug_submit();
			g_rhi->end_debug_context();
			rmt_EndOpenGLSample();
			rmt_EndCPUSample();
#endif
			rmt_BeginCPUSample(get_time, RMTSF_Aggregate);
			f32 dt = clock->get_frame_time();
			rmt_EndCPUSample();



			

			rmt_BeginCPUSample(animation_update, 0);
			g_animation_manager->update(clock->get_frame_time());
			rmt_EndCPUSample();
			rmt_BeginCPUSample(physics_update, 0);
			g_physics_manager->update(g_scene, dt);
			rmt_EndCPUSample();

			rmt_BeginCPUSample(scene_update, 0);
			g_scene->update(dt);

			g_scene->fix_update(dt);
			rmt_EndCPUSample();

			rmt_BeginCPUSample(begin_render, 0);
			g_scene->render_world();
			rmt_EndCPUSample();

			//invoke after world renderer for draw UI
			g_scene->render_ui();
			//update_tank_demo();

			// g_script_manager->call("debug_draw");
			rmt_BeginCPUSample(render_imgui, 0);
			ImGui::Render();
			rmt_EndCPUSample();

			rmt_BeginCPUSample(audio_update, 0);
			g_audio_manager->Update();
			rmt_EndCPUSample();

			lastTime = curTime;
			rmt_BeginCPUSample(get_time, RMTSF_Aggregate);
			clock->update();
			rmt_EndCPUSample();
			// g_sound_palyer->update();

			rmt_BeginCPUSample(swap_buffer_cpu, 0);
			rmt_BeginOpenGLSample(swap_buffer_gpu);
			glfwSwapBuffers(window);
			rmt_EndOpenGLSample();
			rmt_EndCPUSample();

			rmt_BeginCPUSample(handle_input, RMTSF_Aggregate);
			g_input_manager->clear_states();
			// for glfw
			g_input_manager->clear_scroller();
			rmt_EndCPUSample();
			rmt_EndCPUSample();

			//we must change the object layout at the end of a frame in case of some iter trouble
			//update create/remove object event
			g_scene->submit_object_change();
			//update scene load
			g_scene->submit_level_change();

			

		}
		else
		{
#ifdef _WIN32
			Sleep(0);
#else
			usleep(0);
#endif
		}


	}
	g_scene->clear();
	
}

GLFWApp::~GLFWApp()
{
	RemoteryProfiler::shutdown();
	imgui_renderer->imgui_shutdown();
	g_physics_manager->shutdown();
	g_script_manager->shutdown();
	g_logger->shutdown();
	g_res_manager->shutdown();
	glfwDestroyWindow(window);
	glfwTerminate();
	g_pool_allocator->shutdown();
}

