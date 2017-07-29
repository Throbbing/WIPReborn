#include "ButtonWidget.h"
#include "../wip/Input.h"
#include "../wip/Renderer.h"

ButtonWidget::ButtonWidget()
{
	_binit = false;
	_callback_holder = nullptr;
	call = false;
}

ButtonWidget::~ButtonWidget()
{
	delete _image_down;
	delete _image_normal;
	delete _image_move;
}

void ButtonWidget::set_callback_holder(ofApp* holder)
{
	_callback_holder = holder;
}

ofApp* ButtonWidget::get_callback_holder()
{
	return _callback_holder;
}

void ButtonWidget::init_button(std::string pic_path_down,std::string pic_path_move,std::string pic_path_normal,void (*button_event)(ButtonWidget*))
{
	_button_event = button_event;

	_image_down = new WIPTexture(pic_path_down.c_str());
	_image_move = new WIPTexture(pic_path_move.c_str());
	_image_normal = new WIPTexture(pic_path_normal.c_str());


	_mouse_state = 0;//normal
	_button_state = 0;//normal
	_mx = _my = 0;
	_x = _y = 0;
	_gh = _image_normal->get_height();
	_gw = _image_normal->get_width();
	_binit = true;
	_width = _gw;
	_height = _gh;
	_last_mouse_state = 0;

	_fbo->resize(_width,_height);
}

void ButtonWidget::set_size(int width,int height)
{
	_width = _gw = width;
	_height = _gh = height;
}

void ButtonWidget::update_button(int __x,int __y)
{
	_mx = __x;
	_my = __y;
	//static bool _bpressed = false;

	if(_mx>=_x&&_mx<=_x+_gw&&_my>=_y&&_my<=_y+_gh)
	{
		/*鼠标状态 _mouse_state:
		 *0：已经弹起；
		 *1：正在按下；
		 *2：正在弹起
		 *3：已经按下
		 ------------
		 *按钮状态 _button_state:
		 *0:Nomal
		 *1:Move
		 *2:Down
		 */
		if(_mouse_state==1)//pressing
		{
			_button_state = 2;//down
			_mouse_state = 3;//pressed
		}
		else if(_mouse_state==2)
		{
			if(_button_event)
				_button_event(this);
			call = true;
			_button_state = 1;//move
			_mouse_state = 0;
		}
		else
			if(_mouse_state!=3)
				_button_state = 1;
	}
	else
	{
		_button_state = 0;
		_mouse_state = 0;
		
	}
	
}

/*
void ButtonWidget::draw()
{
	//未初始化返回
	if(!_binit)
		return;

	switch(_button_state)
	{
	case 0:
		_image_normal.draw(_x,_y,_gw,_gh);
		break;
	case 1:
		_image_move.draw(_x,_y,_gw,_gh);
		break;
	case 2:
		_image_down.draw(_x,_y,_gw,_gh);
		break;
	}
}
*/

void ButtonWidget::set_position(int __x,int __y)
{
	_x = __x;
	_y = __y;
}

void ButtonWidget::update(float dt)
{
	ActiveDrawableRect::update(dt);
	int mouse_x = Input::get_mouse_x();
	int mouse_y = Input::get_mouse_y();
	//if left button is pressed state.Ture for pressed.
	bool bpressed = Input::get_sys_key_pressed(WIP_MOUSE_LBUTTON);
	if(_last_mouse_state==0&&bpressed==1)
		set_mouse_state(mouse_x,mouse_y,1 );
	else if(_last_mouse_state==1&&bpressed==0)
		set_mouse_state(mouse_x,mouse_y,2 );
	update_button(mouse_x,mouse_y);
	//set_mouse_state(mouse_x,mouse_y,bpressed? 1 : 2 );
	
	draw_internal();
	_last_mouse_state = bpressed;
}

void ButtonWidget::draw_internal(int x,int y)
{
		//未初始化返回
	if(!_binit)
		return;
	int w = _fbo->get_width();
	int h = _fbo->get_height();
	_fbo->begin();
	_fbo->clear();
	switch(_button_state)
	{
	case 0:
		g_renderer->render(_image_normal->get_texture_id(),0,0,w,h);
		break;
	case 1:
		g_renderer->render(_image_move->get_texture_id(),0,0,w,h);
		break;
	case 2:
		g_renderer->render(_image_down->get_texture_id(),0,0,w,h);
		break;
	}
	_fbo->end();
}

void ButtonWidget::draw(int x,int y)
{
	BaseWidget::draw(x,y);
}