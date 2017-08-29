#include "UIWrap.h"
#include "UserComponent.h"
#include "imgui.h"




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

char* get_utf8(wchar_t* text,char* buf)
{
	for (int i = 0; i < wcslen(text);++i)
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
  a = new A();
  b = new B();
  a->subscribe_event(imbt, get_string_hash("button_push"), WIP_EVENT_HANDLER_OUT(A, push, a));
  b->subscribe_event(imbt, get_string_hash("button_push"), WIP_EVENT_HANDLER_OUT(B, push1, b));
}

void MapComponent::update(f32 dt)
{

  imbt->update();

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
		ImGui::Text(get_utf8(L"鼠标左键绘制碰撞，鼠标右键擦除碰撞",text));
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

	
	for (int i = 0,j=0; i < strlen(buf); i+=3,j++)
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