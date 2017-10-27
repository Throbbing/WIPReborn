#include "UIWrap.h"
#include "UserComponent.h"
#include "imgui.h"

//object should belongs to a world not scene
#include "GLFWApp.h"




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
	for (int i = 0; i < wcslen(text); ++i)
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

void MapComponent::init()
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

}

void MapComponent::update(f32 dt)
{
	send_event(component_update);

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
		memset(text, 0, 256);
		ImGui::Text(get_utf8(L"鼠标左键绘制碰撞，鼠标右键擦除碰撞", text));
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
	cam->zoomin(Input::get_mouse_scroller()*0.1);
	RBVector2 daabb(cam->world_w*cam->_zoom*0.5f, cam->world_h*cam->_zoom*0.5f);
	RBAABB2D bg_bound(lb, rt);
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


	wchar_t wbuf[3] = L"确定";

	//this is utf-8 encode
	static char buf[1024] = "\xe4\xb8\xa5 \xe8\x9b\x99 \xe7\x8e\x8b \xe5\xad\x90";
	UnicodeToUTF_8(buf, wbuf);
	int k = strlen(buf);
	wchar_t wbuf_out[1024];
	memset(wbuf_out, 0, 1024 * sizeof(wchar_t));


	ImGui::InputText("UTF-8 input", buf, 1024);


	for (int i = 0, j = 0; i < strlen(buf); i += 3, j++)
	{
		UTF_8ToUnicode(&wbuf_out[j], &buf[i]);
	}




	text_renderer->render_text(0, 700, wbuf_out, wcslen(wbuf_out), 200, cam);

	char t[9];
	memset(t, 0, 9);




	if (imgui_button_short(L"编辑"))
	{
		edit_mode = !edit_mode;
	}
	//grid->clear_data();
}

void PlayerComponent::init()
{
	man_state = ManState::E_DOWN;
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
	if (killed % 2 == 0)
	{
		if (RBMath::get_rand_i(900) > 450)
		{
			for (int i = 0; i < RBMath::get_rand_range_i(1, 5); i++)
			{

				WIPSpriteCreator ctor_man(1.8f, 1.8f, WIPMaterialType::E_TRANSLUCENT);
				ctor_man.texture = enemy_texture;
				ctor_man.world_render = world_renderer;
				ctor_man.body_tp = WIPCollider::_CollisionTypes::E_RIGIDBODY;
				ctor_man.collider_sx = 1.f;
				ctor_man.collider_sy = 1.f;
				WIPSprite* sp = WIPSpriteFactory::create_sprite(ctor_man);
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
				WIPSprite* spblt = WIPSpriteFactory::create_sprite(ctor_blt);
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
					WIPSprite* sp_pop = WIPSpriteFactory::create_sprite(ctor_pop);
					sp_pop->_animation->add_clip(pop_clip, pop_clip->name);
					//sp_pop->_animation->play_name(pop_clip->name, false);
					sp_pop->set_z_order(-0.1);
					sp_pop->set_tag("pop_enemy");
					sp_pop->_render->is_visible = false;
					auto cb = [](void* s)->void
					{
						((WIPSprite*)s)->_render->is_visible = false;
					};
					sp_pop->_animation->add_clip_callback(pop_clip->name, cb, sp_pop);
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
					WIPSprite* sp_pop = WIPSpriteFactory::create_sprite(ctor_pop);
					sp_pop->_animation->add_clip(pop_clip, pop_clip->name);
					//sp_pop->_animation->play_name(pop_clip->name, false);
					sp_pop->set_z_order(-0.1);
					sp_pop->set_tag("pop_enemy");
					sp_pop->_render->is_visible = false;
					auto cb = [](void* s)->void
					{
						((WIPSprite*)s)->_render->is_visible = false;
					};
					sp_pop->_animation->add_clip_callback(pop_clip->name, cb, sp_pop);
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
			acc += 0.1;
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
	
	for (b2ContactEdge* ce = host_object->_collider->get_body()->GetContactList(); ce; ce = ce->next)
	{
		b2Contact* c = ce->contact;
		WIPSprite* s2 = (WIPSprite*)c->GetFixtureB()->GetBody()->GetUserData();
		if (s2->get_tag() == "bullet_enemy" || s2->get_tag() == "bullet")
		{
			if (!Input::get_key_pressed(WIP_K) || acc <= 0.1f)
			hp -= 17;
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
			hp -= 16;
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
}

void EnemeyComponent::des_player(string_hash tp, void* ud)
{
	player_ref = nullptr;
}

void EnemeyComponent::update(f32 dt)
{
	f32 dd = 1.f;
	f32 dx = 0, dy = 0;
	float speed = 3.2f;
	acc_t += dt;
	static f32 fixt = 1.5f ;
	int r = 0;
	ImGui::SliderFloat("", &fixt, 1.5f, 5.5f);
	f32 tf = fixt+RBMath::get_rand_range_f(-1.2, 1.2);

	if (acc_t > tf)
	{
		cur_direction = RBMath::get_rand_range_i(0, 3);
		r = RBMath::get_rand_i(8);
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
				return;
			}
			s2 = (WIPSprite*)c->GetFixtureA()->GetBody()->GetUserData();
			if (s2->get_tag() == "bullet")
			{
				host_object->send_event(get_string_hash("add_hp"));
				g_audio_manager->Play(sound_death);
				unsubscribe_all_events();
				g_app->pending_objects(host_object);
				return;
			}
		}
}

void BulletComponent::update(f32 dt)
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