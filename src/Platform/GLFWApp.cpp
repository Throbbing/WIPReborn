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

void open_explorer(const wchar_t* path)
{
#ifdef WIN32
	ShellExecuteW(0, L"open", 0, 0, path, SW_SHOWNORMAL);
#endif
}

std::wstring string_to_wstring(const std::string &str)
{
	std::wstring wstr(str.length(), L' ');
	std::copy(str.begin(), str.end(), wstr.begin());
	return wstr;
}

//只拷贝低字节至string中
std::string wstring_to_string(const std::wstring &wstr)
{
	std::string str(wstr.length(), ' ');
	std::copy(wstr.begin(), wstr.end(), str.begin());
	return str;
}

void GLFWApp::pending_objects(TRefCountPtr<WIPSprite> s)
{
	deleting_objects.push_back(s);
}

void GLFWApp::creating_object(TRefCountPtr<WIPSprite> s)
{
	creating_objects.push_back(s);
}

f32 GLFWApp::get_cur_time() const
{
	return timer->get_time_ms();
}

void GLFWApp::init_tank_demo()
{
	g_audio_manager->LoadBank("./audio/Desktop/master.bank", false);
	g_audio_manager->LoadBank("./audio/Desktop/master.strings.bank", false);
	StudioSound* sound_fire = g_audio_manager->CreateSound("event:/fire");
	StudioSound* sound_blast = g_audio_manager->CreateSound("event:/blast");
	StudioSound* sound_death_enemy = g_audio_manager->CreateSound("event:/Death 1");
	StudioSound* sound_death = g_audio_manager->CreateSound("event:/Death 2");
	StudioSound* sound_start = g_audio_manager->CreateSound("event:/start");

	cameras.push_back(scene->create_camera(20, 20, window_w, window_h, window_w, window_h));
	cameras[0]->move_to(5, 5);

	ui_renderer = new UIRender();
	ui_renderer->init(cameras[0]);

	g_physics_manager->set_debug_camera(cameras[0]);

	render_texture2d = g_rhi->RHICreateRenderTexture2D(200, 200, RBColorf::black);
	/*
	class WIPTexture2D* enemy_texture;
	class WIPTexture2D* player_texture;
	class WIPTexture2D* block_texture;
	class WIPTexture2D* pop_texture;
	class WIPTexture2D* bullet_texture;

	class WIPAnimationClip* enemy_clip;
	class WIPAnimationClip* player_clip;
	class WIPAnimationClip* pop_clip;

	#define ANIMYNUM 7
	TRefCountPtr<WIPSprite> enemy[ANIMYNUM];
	TRefCountPtr<WIPSprite> block;
	TRefCountPtr<WIPSprite> player;
	TRefCountPtr<WIPSprite> pop[ANIMYNUM];
	TRefCountPtr<WIPSprite> bullet_texture[ANIMYNUM];
	*/
	enemy_clip = WIPAnimationClip::create_with_atlas("enemy_run", "./clips/tank/tank.clip");

	player_clip = WIPAnimationClip::create_with_atlas("player_run", "./clips/tank/tank.clip");

	pop_clip = WIPAnimationClip::create_with_atlas("pop", "./clips/tank/pop.clip");
	pop_clip->_speed = 5;

	auto res_handle1 = g_res_manager->load_resource("./pic/tank/enemy.png", WIPResourceType::TEXTURE);
	int ww = ((TextureData *)(res_handle1->extra))->width;
	int hh = ((TextureData *)(res_handle1->extra))->height;
	auto res_handle2 = g_res_manager->load_resource("./pic/tank/player.png", WIPResourceType::TEXTURE);
	int ww1 = ((TextureData *)(res_handle2->extra))->width;
	int hh1 = ((TextureData *)(res_handle2->extra))->height;
	auto res_handle1mask = g_res_manager->load_resource("./pic/tank/block.png", WIPResourceType::TEXTURE);
	int ww1block = ((TextureData *)(res_handle1mask->extra))->width;
	int hh1block = ((TextureData *)(res_handle1mask->extra))->height;
	auto res_handle1fog = g_res_manager->load_resource("./pic/tank/pop.png", WIPResourceType::TEXTURE);
	int ww1pop = ((TextureData *)(res_handle1fog->extra))->width;
	int hh1pop = ((TextureData *)(res_handle1fog->extra))->height;
	auto res_handlebullet = g_res_manager->load_resource("./pic/tank/bullet.png", WIPResourceType::TEXTURE);
	int ww1blt = ((TextureData *)(res_handlebullet->extra))->width;
	int hh1blt = ((TextureData *)(res_handlebullet->extra))->height;


	enemy_texture = g_rhi->RHICreateTexture2D(ww, hh, res_handle1->ptr);
	player_texture = g_rhi->RHICreateTexture2D(ww1, hh1, res_handle2->ptr);
	block_texture = g_rhi->RHICreateTexture2D(ww1block, hh1block, res_handle1mask->ptr);
	pop_texture = g_rhi->RHICreateTexture2D(ww1pop, hh1pop, res_handle1fog->ptr);
	bullet_texture = g_rhi->RHICreateTexture2D(ww1blt, hh1blt, res_handlebullet->ptr);

	TRefCountPtr<WIPSprite> splayer = nullptr;
	{
		WIPSpriteCreator ctor_man1(1.8f, 1.8f, WIPMaterialType::E_TRANSLUCENT);
		ctor_man1.texture = player_texture;
		ctor_man1.world_render = world_renderer;
		ctor_man1.body_tp = WIPCollider::_CollisionTypes::E_RIGIDBODY;
		ctor_man1.collider_sx = 1.f;
		ctor_man1.collider_sy = 1.f;
		TRefCountPtr<WIPSprite> sp = WIPSpriteFactory::create_sprite(ctor_man1);
		splayer = sp;
		sp->_animation->add_clip(player_clip, player_clip->name);
		sp->set_tag("player");

		PlayerComponent* pcc = new PlayerComponent(sp);
		pcc->sound_death = sound_death;
		pcc->sound_start = sound_start;
		pcc->sound = sound_fire;
		pcc->clip = player_clip;
		sp->add_tick_component(pcc);
		scene->add_sprite(sp);
		pcc->text_renderer = text_renderer;
		pcc->cam = cameras[0];

		pcc->enemy_texture = enemy_texture;
		pcc->enemy_clip = enemy_clip;
		pcc->pop_clip = pop_clip;
		pcc->pop_texture = pop_texture;
		pcc->bullet_texture = bullet_texture;
		pcc->scene = scene;
		pcc->world_renderer = world_renderer;
		pcc->sound_fire = sound_fire;
		pcc->sound_blast = sound_blast;
		pcc->sound_death_enemy = sound_death_enemy;
		pcc->player_clip = player_clip;

		WIPSpriteCreator ctor_blt(0.3f, 0.3f, WIPMaterialType::E_TRANSLUCENT);
		ctor_blt.texture = bullet_texture;
		ctor_blt.world_render = world_renderer;
		ctor_blt.body_tp = WIPCollider::_CollisionTypes::E_RIGIDBODY;
		ctor_blt.collider_sx = 1.f;
		ctor_blt.collider_sy = 1.f;
		TRefCountPtr<WIPSprite> spblt = WIPSpriteFactory::create_sprite(ctor_blt);
		spblt->_animation->add_clip(player_clip, player_clip->name);
		spblt->_animation->play_name(player_clip->name, false);
		spblt->_render->is_visible = false;
		BulletComponent* pcc1 = new BulletComponent(spblt);
		pcc1->sound = sound_blast;
		spblt->add_tick_component(pcc1);
		spblt->set_tag("bullet");
		pcc->blt = spblt;
		spblt->translate_to(21, 21);
		pcc1->pos = RBVector2(21, 21);
		scene->add_sprite(spblt);

		WIPSpriteCreator ctor_pop(2.f, 2.f, WIPMaterialType::E_TRANSLUCENT);
		ctor_pop.texture = pop_texture;
		ctor_pop.world_render = world_renderer;
		ctor_pop.body_tp = WIPCollider::_CollisionTypes::E_NO_PHYSICS;
		ctor_pop.collider_sx = 1.f;
		ctor_pop.collider_sy = 1.f;
		TRefCountPtr<WIPSprite> sp_pop = WIPSpriteFactory::create_sprite(ctor_pop);
		sp_pop->set_z_order(-0.1f);
		sp_pop->_animation->add_clip(pop_clip, pop_clip->name);
		//sp_pop->_animation->play_name(pop_clip->name, false);
		sp_pop->_render->is_visible = false;
		sp_pop->set_tag("player_pop");
		auto cb = [](void* s)->void
		{
			((WIPSprite*)s)->_render->is_visible = false;
		};
		sp_pop->_animation->add_clip_callback(pop_clip->name, cb, (WIPSprite*)sp_pop);
		scene->add_sprite(sp_pop);

		pcc1->pop_obj = sp_pop;
	}
#if 1
	for (int i = 0; i < ANIMYNUM; i++)
	{
		WIPSpriteCreator ctor_man(1.8f, 1.8f, WIPMaterialType::E_TRANSLUCENT);
		ctor_man.texture = enemy_texture;
		ctor_man.world_render = world_renderer;
		ctor_man.body_tp = WIPCollider::_CollisionTypes::E_RIGIDBODY;
		ctor_man.collider_sx = 1.f;
		ctor_man.collider_sy = 1.f;
		TRefCountPtr<WIPSprite> sp = WIPSpriteFactory::create_sprite(ctor_man);
		sp->_animation->add_clip(enemy_clip, enemy_clip->name);
		sp->translate_to(RBMath::get_rand_range_f(-20, 20), RBMath::get_rand_range_f(-20, 20));
		sp->set_tag("enemy");
		EnemeyComponent* pcc = new EnemeyComponent(sp);
		pcc->player_ref = splayer;
		pcc->sound = sound_fire;
		pcc->sound_death = sound_death_enemy;
		pcc->clip = enemy_clip;
		sp->add_tick_component(pcc);
		scene->add_sprite(sp);

		WIPSpriteCreator ctor_blt(0.3f, 0.3f, WIPMaterialType::E_TRANSLUCENT);
		ctor_blt.texture = bullet_texture;
		ctor_blt.world_render = world_renderer;
		ctor_blt.body_tp = WIPCollider::_CollisionTypes::E_RIGIDBODY;
		ctor_blt.collider_sx = 1.f;
		ctor_blt.collider_sy = 1.f;
		TRefCountPtr<WIPSprite> spblt = WIPSpriteFactory::create_sprite(ctor_blt);
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
			spblt->translate_to(25, 22 - 0.6*i);
			pcc1->pos = RBVector2(25, 22 - 0.6*i);
			scene->add_sprite(spblt);

			WIPSpriteCreator ctor_pop(2.f, 2.f, WIPMaterialType::E_TRANSLUCENT);
			ctor_pop.texture = pop_texture;
			ctor_pop.world_render = world_renderer;
			ctor_pop.body_tp = WIPCollider::_CollisionTypes::E_NO_PHYSICS;
			ctor_pop.collider_sx = 1.f;
			ctor_pop.collider_sy = 1.f;
			TRefCountPtr<WIPSprite> sp_pop = WIPSpriteFactory::create_sprite(ctor_pop);
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
			scene->add_sprite(sp_pop);

			pcc1->pop_obj = sp_pop;
		}
		else
		{
			BulletComponent1* pcc1 = new BulletComponent1(spblt);
			pcc1->sound = sound_blast;
			spblt->add_tick_component(pcc1);
			spblt->set_tag("bullet_enemy");
			pcc->blt = spblt;
			spblt->translate_to(25, 22 - 0.6*i);
			pcc1->pos = RBVector2(25, 22 - 0.6*i);
			scene->add_sprite(spblt);

			WIPSpriteCreator ctor_pop(2.f, 2.f, WIPMaterialType::E_TRANSLUCENT);
			ctor_pop.texture = pop_texture;
			ctor_pop.world_render = world_renderer;
			ctor_pop.body_tp = WIPCollider::_CollisionTypes::E_NO_PHYSICS;
			ctor_pop.collider_sx = 1.f;
			ctor_pop.collider_sy = 1.f;
			TRefCountPtr<WIPSprite> sp_pop = WIPSpriteFactory::create_sprite(ctor_pop);
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
			scene->add_sprite(sp_pop);

			pcc1->pop_obj = sp_pop;
		}
		
		
	}



	//block
	{
		for (int i = 0; i < 10; ++i)
		{
			WIPSpriteCreator ctor_man1(1.8f, 1.8f, WIPMaterialType::E_OPAQUE);
			ctor_man1.texture = block_texture;
			ctor_man1.world_render = world_renderer;
			ctor_man1.body_tp = WIPCollider::_CollisionTypes::E_STATIC_RIGIDBODY;
			ctor_man1.collider_sx = 1.f;
			ctor_man1.collider_sy = 1.f;
			TRefCountPtr<WIPSprite> sp = WIPSpriteFactory::create_sprite(ctor_man1);
			sp->set_tag("block");
			scene->add_sprite(sp);
			sp->translate_to(0, 8 + i*1.8);
		}

	}

	{
		for (int i = 0; i < 10; ++i)
		{
			WIPSpriteCreator ctor_man1(1.8f, 1.8f, WIPMaterialType::E_OPAQUE);
			ctor_man1.texture = block_texture;
			ctor_man1.world_render = world_renderer;
			ctor_man1.body_tp = WIPCollider::_CollisionTypes::E_STATIC_RIGIDBODY;
			ctor_man1.collider_sx = 1.f;
			ctor_man1.collider_sy = 1.f;
			TRefCountPtr<WIPSprite> sp = WIPSpriteFactory::create_sprite(ctor_man1);
			sp->set_tag("block");

			scene->add_sprite(sp);
			sp->translate_to(0 - 12, -9 + i*1.8);
		}
	}
	{
		WIPSpriteCreator ctor_man1(1.8f, 1.8f, WIPMaterialType::E_OPAQUE);
		ctor_man1.texture = block_texture;
		ctor_man1.world_render = world_renderer;
		ctor_man1.body_tp = WIPCollider::_CollisionTypes::E_STATIC_RIGIDBODY;
		ctor_man1.collider_sx = 1.f;
		ctor_man1.collider_sy = 1.f;
		TRefCountPtr<WIPSprite> sp = WIPSpriteFactory::create_sprite(ctor_man1);
		sp->set_tag("block");

		scene->add_sprite(sp);
		sp->translate_to(-20, 8);
	}
	{
		WIPSpriteCreator ctor_man1(1.8f, 1.8f, WIPMaterialType::E_OPAQUE);
		ctor_man1.texture = block_texture;
		ctor_man1.world_render = world_renderer;
		ctor_man1.body_tp = WIPCollider::_CollisionTypes::E_STATIC_RIGIDBODY;
		ctor_man1.collider_sx = 1.f;
		ctor_man1.collider_sy = 1.f;
		TRefCountPtr<WIPSprite> sp = WIPSpriteFactory::create_sprite(ctor_man1);
		sp->set_tag("block");

		scene->add_sprite(sp);
		sp->translate_to(0, 0);
	}
	{
		WIPSpriteCreator ctor_man1(1.8f, 1.8f, WIPMaterialType::E_OPAQUE);
		ctor_man1.texture = block_texture;
		ctor_man1.world_render = world_renderer;
		ctor_man1.body_tp = WIPCollider::_CollisionTypes::E_STATIC_RIGIDBODY;
		ctor_man1.collider_sx = 1.f;
		ctor_man1.collider_sy = 1.f;
		TRefCountPtr<WIPSprite> sp = WIPSpriteFactory::create_sprite(ctor_man1);
		sp->set_tag("block");

		scene->add_sprite(sp);
		sp->translate_to(10, 10);
	}

	{
		for (int i = 0; i < 8; ++i)
		{
			WIPSpriteCreator ctor_man1(1.8f, 1.8f, WIPMaterialType::E_OPAQUE);
			ctor_man1.texture = block_texture;
			ctor_man1.world_render = world_renderer;
			ctor_man1.body_tp = WIPCollider::_CollisionTypes::E_STATIC_RIGIDBODY;
			ctor_man1.collider_sx = 1.f;
			ctor_man1.collider_sy = 1.f;
			TRefCountPtr<WIPSprite> sp = WIPSpriteFactory::create_sprite(ctor_man1);
			sp->set_tag("block");

			scene->add_sprite(sp);
			sp->translate_to(2 + i*1.8, -12);
		}

	}

	{
		for (int i = 0; i < 23; ++i)
		{
			WIPSpriteCreator ctor_man1(1.8f, 1.8f, WIPMaterialType::E_OPAQUE);
			ctor_man1.texture = block_texture;
			ctor_man1.world_render = world_renderer;
			ctor_man1.body_tp = WIPCollider::_CollisionTypes::E_STATIC_RIGIDBODY;
			ctor_man1.collider_sx = 1.f;
			ctor_man1.collider_sy = 1.f;
			TRefCountPtr<WIPSprite> sp = WIPSpriteFactory::create_sprite(ctor_man1);
			sp->set_tag("block");

			scene->add_sprite(sp);
			sp->translate_to(-20 + i*1.8, -20);
		}

		for (int i = 0; i < 23; ++i)
		{
			WIPSpriteCreator ctor_man1(1.8f, 1.8f, WIPMaterialType::E_OPAQUE);
			ctor_man1.texture = block_texture;
			ctor_man1.world_render = world_renderer;
			ctor_man1.body_tp = WIPCollider::_CollisionTypes::E_STATIC_RIGIDBODY;
			ctor_man1.collider_sx = 1.f;
			ctor_man1.collider_sy = 1.f;
			TRefCountPtr<WIPSprite> sp = WIPSpriteFactory::create_sprite(ctor_man1);
			sp->set_tag("block");

			scene->add_sprite(sp);
			sp->translate_to(-20 + i*1.8, 20);
		}
		for (int i = 0; i < 23; ++i)
		{
			WIPSpriteCreator ctor_man1(1.8f, 1.8f, WIPMaterialType::E_OPAQUE);
			ctor_man1.texture = block_texture;
			ctor_man1.world_render = world_renderer;
			ctor_man1.body_tp = WIPCollider::_CollisionTypes::E_STATIC_RIGIDBODY;
			ctor_man1.collider_sx = 1.f;
			ctor_man1.collider_sy = 1.f;
			TRefCountPtr<WIPSprite> sp = WIPSpriteFactory::create_sprite(ctor_man1);
			sp->set_tag("block");

			scene->add_sprite(sp);
			sp->translate_to(-20 , -20+i*1.8);
		}
		for (int i = 0; i < 23; ++i)
		{
			WIPSpriteCreator ctor_man1(1.8f, 1.8f, WIPMaterialType::E_OPAQUE);
			ctor_man1.texture = block_texture;
			ctor_man1.world_render = world_renderer;
			ctor_man1.body_tp = WIPCollider::_CollisionTypes::E_STATIC_RIGIDBODY;
			ctor_man1.collider_sx = 1.f;
			ctor_man1.collider_sy = 1.f;
			TRefCountPtr<WIPSprite> sp = WIPSpriteFactory::create_sprite(ctor_man1);
			sp->set_tag("block");

			scene->add_sprite(sp);
			sp->translate_to(20, -20 + i*1.8);
		}
	}
#endif
	show_text = false;

}

void GLFWApp::update_tank_demo()
{
	if (show_text)
	{
		wchar_t words1[] = L"死了!";
		//std::wstring wbuf = string_to_wstring(buf);
		
		text_renderer->render_text(200, 400, words1, wcslen(words1), window_w, cameras[0]);
		text_renderer->render(cameras[0]);
	}
}


void GLFWApp::update_rpg_demo()
{
	ui_renderer->render_pic(0, 0, render_texture2d->get_width(), render_texture2d->get_height(), render_texture2d);
	g_rhi->set_back_buffer(render_texture2d);
	g_rhi->clear_back_buffer();
	g_rhi->set_main_back_buffer();
	//ui_renderer->render_box(0, 0, 0, 0, RBColorf(0.3, 0.3, 0.5, 0.5));
	//ui_renderer->render_pic(700, 50, face->get_width(), face->get_height(), face);
	//wchar_t words1[] = L"趙靈兒:\n好……放了他，我就跟你們走……";
	//text_renderer->render_text(100, 200, words1, wcslen(words1), window_w, cameras[0]);
	//text_renderer->render(cameras[0]);
	
}

void GLFWApp::init_rpg_demo()
{
	cameras.push_back(scene->create_camera(20.f, 20.f, window_w, window_h, window_w, window_h));
	cameras[0]->move_to(5.f, 5.f);

	ui_renderer = new UIRender();
	ui_renderer->init(cameras[0]);


	g_physics_manager->set_debug_camera(cameras[0]);

	render_texture2d = g_rhi->RHICreateRenderTexture2D(window_w, window_h, RBColorf::black);


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
	tex2d1mask = g_rhi->RHICreateTexture2D(ww1, hh1, res_handle1mask->ptr, 0, 0, 0, 1);

	tex2d_fog = g_rhi->RHICreateTexture2D(ww1fog, hh1fog, res_handle1fog->ptr);

	tex2d_lixiaoyao = g_rhi->RHICreateTexture2D(wli, hli, res_lixiaoyao->ptr);
	tex2d_zaji1 = g_rhi->RHICreateTexture2D(wzaji1, hzaji1, res_zaji1->ptr);
	tex2d_zaji2 = g_rhi->RHICreateTexture2D(wzaji2, hzaji2, res_zaji2->ptr);
	tex2d_crowd = g_rhi->RHICreateTexture2D(wcrow, hcrow, res_crowd->ptr);

	auto res_face = g_res_manager->load_resource("./pic/face.png", WIPResourceType::TEXTURE);
	int fw = ((TextureData *)(res_face->extra))->width;
	int fh = ((TextureData *)(res_face->extra))->height;
	face = g_rhi->RHICreateTexture2D(fw, fh, res_face->ptr);

	auto res_face_miaoren = g_res_manager->load_resource("./pic/miaoren.png", WIPResourceType::TEXTURE);
	fw = ((TextureData *)(res_face_miaoren->extra))->width;
	fh = ((TextureData *)(res_face_miaoren->extra))->height;
	WIPTexture2D* face_miaoren = g_rhi->RHICreateTexture2D(fw, fh, res_face_miaoren->ptr);

	auto res_face_251 = g_res_manager->load_resource("./pic/25-1.png", WIPResourceType::TEXTURE);
	fw = ((TextureData *)(res_face_251->extra))->width;
	fh = ((TextureData *)(res_face_251->extra))->height;
	WIPTexture2D* face_25_1 = g_rhi->RHICreateTexture2D(fw, fh, res_face_251->ptr);

	WIPSpriteCreator ctor_man(3.6f*0.5, 3.6f, WIPMaterialType::E_TRANSLUCENT);
	ctor_man.texture = tex2d;
	ctor_man.world_render = world_renderer;
	ctor_man.body_tp = WIPCollider::_CollisionTypes::E_RIGIDBODY;
	ctor_man.collider_sx = 0.5f;
	ctor_man.collider_sy = 0.2f;

	WIPSpriteCreator ctor_mask(40.f, 30.f, WIPMaterialType::E_TRANSLUCENT);
	ctor_mask.texture = tex2d1mask;
	ctor_mask.world_render = world_renderer;
	ctor_mask.body_tp = WIPCollider::_CollisionTypes::E_NO_PHYSICS;

	WIPSpriteCreator ctor_bg(40.f, 30.f, WIPMaterialType::E_TRANSLUCENT);
	ctor_bg.texture = tex2d1;
	ctor_bg.world_render = world_renderer;
	ctor_bg.body_tp = WIPCollider::_CollisionTypes::E_NO_PHYSICS;


	WIPSpriteCreator ctor_fog(20.f, 20.f, WIPMaterialType::E_TRANSLUCENT);
	ctor_fog.texture = tex2d_fog;
	ctor_fog.world_render = world_renderer;
	ctor_fog.body_tp = WIPCollider::_CollisionTypes::E_NO_PHYSICS;


	WIPSpriteCreator ctor_li(3.f*rotli, 3.5f, WIPMaterialType::E_TRANSLUCENT);
	ctor_li.texture = tex2d_lixiaoyao;
	ctor_li.world_render = world_renderer;
	//ctor_li.body_tp = WIPCollider::_CollisionTypes::E_GHOST;
	ctor_li.collider_sx = 0.5f;
	ctor_li.collider_sy = 0.2f;

	WIPSpriteCreator ctor_zaji1(1.2f*2.5f*1.2f, 3.6f*1.2f, WIPMaterialType::E_TRANSLUCENT);
	ctor_zaji1.texture = tex2d_zaji1;
	ctor_zaji1.world_render = world_renderer;
	ctor_zaji1.collider_sx = 0.5f;
	ctor_zaji1.collider_sy = 0.2f;

	WIPSpriteCreator ctor_zaji2(1.8f*1.2f, 3.4f, WIPMaterialType::E_TRANSLUCENT);
	ctor_zaji2.texture = tex2d_zaji2;
	ctor_zaji2.world_render = world_renderer;
	ctor_zaji2.collider_sx = 0.5f;
	ctor_zaji2.collider_sy = 0.2f;

	WIPSpriteCreator ctor_crowd(6 * 1.2f, 8.f, WIPMaterialType::E_TRANSLUCENT);
	ctor_crowd.texture = tex2d_crowd;
	ctor_crowd.world_render = world_renderer;
	ctor_crowd.collider_sx = 0.85f;
	ctor_crowd.collider_sy = 0.4f;



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
	man_lixiaoyao->set_anchor(0.5f, 0);
	NPCComponent* npcc = new NPCComponent(man_lixiaoyao);
	npcc->words[0].push(L"......");
	npcc->add_faces("dft", face_miaoren);
	man_lixiaoyao->add_tick_component(npcc);

	zaji1 = WIPSpriteFactory::create_sprite(ctor_zaji1);
	zaji1->_animation->add_clip(clip, clip->name);
	zaji1->_animation->set_clip_instance_speed(clip->name
		, 0.2f);
	zaji1->_animation->play(clip, true);
	zaji1->set_anchor(0.5f, 0);
	NPCComponent* npcc1 = new NPCComponent(zaji1);
	npcc1->words[0].push(L"还是不要去打扰他们了...");
	npcc1->add_faces("dft", face);
	zaji1->add_tick_component(npcc1);

	zaji2 = WIPSpriteFactory::create_sprite(ctor_zaji2);
	zaji2->_animation->add_clip(clip1_s, clip1_s->name);
	zaji2->_animation->play(clip1_s, true);
	zaji2->set_anchor(0.5f, 0);
	NPCComponent* npcc2 = new NPCComponent(zaji2);
	npcc2->add_faces("dft", face_25_1);
	npcc2->words[0].push(L"真是可怜的孩子...");
	npcc2->words[0].push(L"想当年，南诏国大水，灵儿她娘也是这么死的...");
	npcc2->words[0].push(L"如今，想不到这种事情也会发生在灵儿身上...");
	zaji2->add_tick_component(npcc2);


	crowd = WIPSpriteFactory::create_sprite(ctor_crowd);
	crowd->_animation->add_clip(clip_s, clip_s->name);
	crowd->_animation->play(clip_s);
	crowd->set_anchor(0.4f, 0.2f);
	NPCComponent* npcc3 = new NPCComponent(crowd);
	npcc3->words[0].push(L"尔等凡人，速速退散！");
	crowd->add_tick_component(npcc3);

	bg = WIPSpriteFactory::create_sprite(ctor_bg);
	MapComponent* mc = new MapComponent(bg);
	bg->add_tick_component(mc);
	mc->cam = cameras[0];
	mc->scene = scene;
	mc->render_texture2d = render_texture2d;
	bg_mask = WIPSpriteFactory::create_sprite(ctor_mask);
	man = WIPSpriteFactory::create_sprite(ctor_man);
	man->_animation->add_clip(clip, clip->name);
	man->_animation->add_clip(clip1, clip1->name);
	man->_animation->add_clip(clip2, clip2->name);
	man->_animation->add_clip(clip3, clip3->name);
	man->_animation->add_clip(clip_s, clip_s->name);
	man->_animation->add_clip(clip1_s, clip1_s->name);
	man->_animation->add_clip(clip2_s, clip2_s->name);
	man->_animation->add_clip(clip3_s, clip3_s->name);
	mc->man = man;

	man->_animation->play(clip_s);
	man->set_anchor(0.5f, 0);

	mc->ui_renderer = ui_renderer;
	mc->text_renderer = text_renderer;

	mc->subscribe_event(npcc, get_string_hash("npc talk"), WIP_EVENT_HANDLER_OUT(MapComponent, change_to_talk, mc));
	mc->subscribe_event(npcc, get_string_hash("player"), WIP_EVENT_HANDLER_OUT(MapComponent, change_to_player, mc));
	mc->subscribe_event(npcc1, get_string_hash("npc talk"), WIP_EVENT_HANDLER_OUT(MapComponent, change_to_talk, mc));
	mc->subscribe_event(npcc1, get_string_hash("player"), WIP_EVENT_HANDLER_OUT(MapComponent, change_to_player, mc));
	mc->subscribe_event(npcc2, get_string_hash("npc talk"), WIP_EVENT_HANDLER_OUT(MapComponent, change_to_talk, mc));
	mc->subscribe_event(npcc2, get_string_hash("player"), WIP_EVENT_HANDLER_OUT(MapComponent, change_to_player, mc));
	mc->subscribe_event(npcc3, get_string_hash("npc talk"), WIP_EVENT_HANDLER_OUT(MapComponent, change_to_talk, mc));
	mc->subscribe_event(npcc3, get_string_hash("player"), WIP_EVENT_HANDLER_OUT(MapComponent, change_to_player, mc));


	bg->set_tag("bg");
	bg_mask->set_tag("mask");
	man->set_tag("man");
	man_lixiaoyao->set_tag("lixiaoyao");
	zaji1->set_tag("zaji1");
	zaji2->set_tag("zaji2");
	crowd->set_tag("crowd");


	bg->set_type_tag("scene");
	bg_mask->set_type_tag("scene");

	man->set_type_tag("character");
	man_lixiaoyao->set_type_tag("character");
	zaji1->set_type_tag("character");
	zaji2->set_type_tag("character");
	crowd->set_type_tag("character");

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
	std::vector<TRefCountPtr<const WIPSprite>> sp;
	scene->quad_tree->get_all_nodes(sp);
	sp.clear();

	man_lixiaoyao->translate_to(-3, 1);
	scene->quad_tree->get_all_nodes(sp);
	sp.clear();
	zaji1->translate_to(-5, 0);
	scene->quad_tree->get_all_nodes(sp);
	sp.clear();
	zaji2->translate_to(-4, -2);
	scene->quad_tree->get_all_nodes(sp);
	sp.clear();
	crowd->translate_to(-8, 3);
	scene->quad_tree->get_all_nodes(sp);
	sp.clear();




	fogs = WIPSpriteFactory::create_sprite(ctor_fog);
	scene->add_sprite(fogs);
	fogs->set_type_tag("scene");
	fogs->translate_to(0.f, 0.f);
	fogs->set_z_order(0.05f);
	mc->fogs = fogs;



	bg->translate_to(0.f, 0.f);
	bg_mask->translate_to(0.f, 0.f);
	man->translate_to(8.f, 1.f);
	bg->set_z_order(0.9f);
	man->set_z_order(0.4f);
	bg_mask->set_z_order(0.1f);

	pre_clip = nullptr;
}

WIPSprite* GLFWApp::get_by_tag(std::string name) const
{
	return scene->get_sprite_by_tag(name);
}

bool GLFWApp::init()
{

	if (!g_pool_allocator->init())
		LOG_ERROR("Pool allocator init failed!");

	//WIPSprite::init_mem();
	//WIPAnimation::init_mem();


	scoller_y = 0;
	window_w = 800;
	window_h = 600;
	bool ret = create_window("Demo");

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

	struct data_pak_t
	{
		RBVector2 player_pos;
		RBVector2 enemey_pos;
		RBVector2 player_bullet_pos;
		RBVector2 enemey_bullet_pos;
		int is_shotting;
		int is_dying;
		f32 cur_time_ms;
	};



	imgui_renderer = new GlfwImguiRender();
	imgui_renderer->imgui_init(window, "./font/simkai.ttf", 19);

	scene = new WIPScene();
	scene->init(1, 1, 4);
	LOG_INFO("Creating scene...");


	world_renderer = new WorldRender();
	world_renderer->init();
	world_renderer->set_world(scene);
	LOG_INFO("Renderer start up...");
#ifdef Text1
	text_renderer = new LargeTexture_TextRender(2048, 2048);
#else
	text_renderer = new TextRender(512, 512);
#endif
	text_renderer->init();
	text_renderer->load_font("./font/simkai.ttf", 24, 24);

	g_physics_manager->startup();
	LOG_INFO("Physics start up...");


	g_input_manager->startup("");
	LOG_INFO("Input start up...");

	g_animation_manager->startup(0.15f);
	LOG_INFO("Animation start up...");

	RemoteryProfiler::startup();


	init_rpg_demo();
	//init_tank_demo();


	//use a big delta time to play first frame
	g_animation_manager->update(1);


	scene->init_components();
	return ret;
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
			g_rhi->change_viewport(cameras[0]->viewport);
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
			g_physics_manager->update(scene, dt);
			rmt_EndCPUSample();

			rmt_BeginCPUSample(scene_update, 0);
			scene->update(dt);

			scene->fix_update(dt);
			rmt_EndCPUSample();

			rmt_BeginCPUSample(begin_render, 0);
			for (auto i : cameras)
				world_renderer->render(i);
			rmt_EndCPUSample();

			/*
			rmt_BeginCPUSample(begin_render_texture,0);
			g_rhi->set_back_buffer(render_texture2d);
			WIPViewPort vp(0, 0, render_texture2d->get_width(), render_texture2d->get_height());
			g_rhi->change_viewport(&vp);


			for (auto i : cameras)
			world_renderer->render(i);

			g_rhi->set_main_back_buffer();
			g_rhi->change_viewport(cameras[0]->viewport);
			rmt_EndCPUSample();
			*/

			update_rpg_demo();
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

			for (auto i : deleting_objects)
			{
				if (i->get_tag() == "player")
				{
					show_text = true;
				}
				WIPSprite::destroy(i);
				
			}
			deleting_objects.clear();

			for (auto i : creating_objects)
			{
				i->init_components();
				scene->add_sprite(i);

			}
			creating_objects.clear();
			rmt_EndCPUSample();

			

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
	scene->destroy_components();
	scene->clear();
	delete scene;

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

