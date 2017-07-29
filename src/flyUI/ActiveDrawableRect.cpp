#include "ActiveDrawableRect.h"
#include "../wip/Input.h"
#include "../wip/Renderer.h"
#include <iostream>


ActiveDrawableRect::ActiveDrawableRect(void)
{
	_bmouse_position = false;
	_last_mouse = false;
	_all_num = 0;
	bdraw = true;
	_fbo = WIPRenderTexture::create(1,1);
}


ActiveDrawableRect::~ActiveDrawableRect(void)
{
	delete _fbo;
}

void ActiveDrawableRect::set_position(int x,int y)
{
	_x = x;
	_y = y;
}

void ActiveDrawableRect::update(float dt)
{
	if(!_bactive) 
		return;
	if(_bmouse_position)
	{
		set_position(Input::get_mouse_x(),Input::get_mouse_y());
		update_editor();
	}
}

void ActiveDrawableRect::draw(int x,int y)
{
	if(x<0||y<0)
		g_renderer->render(_fbo->get_texture_id(),_x,_y,_width,_height);
	else
		g_renderer->render(_fbo->get_texture_id(),x,y,_width,_height);
}

void ActiveDrawableRect::draw_internal(int x,int y)
{

}

void ActiveDrawableRect::set_id(int id)
{
	_id = id;
}

int ActiveDrawableRect::get_id()
{
	return _id;
}

void ActiveDrawableRect::set_name(std::string name)
{
	_name = name;
}

std::string ActiveDrawableRect::get_name()
{
	return _name;
}

void ActiveDrawableRect::reshape(int width,int height,int x,int y)
{
	
	if(x>=0&&y>=0)
	{
		_x = x;
		_y = y;
	}
	
	_width = width;
	_height = height;
}

WIPRenderTexture* ActiveDrawableRect::get_fbo()
{
	return _fbo;
}


//editor
void ActiveDrawableRect::set_mouse_position()
{
	_bmouse_position = true;
}

void ActiveDrawableRect::cancel_mouse_position()
{
	_bmouse_position = false;
}

bool ActiveDrawableRect::is_mouse_position()
{
	return _bmouse_position;
}

void ActiveDrawableRect::update_editor()
{
	bool m = Input::get_sys_key_pressed(WIP_MOUSE_LBUTTON);
	if(m&&!_last_mouse)
	{
		std::cout<<"("<<Input::get_mouse_x()<<","<<Input::get_mouse_y()<<")"<<std::endl;
		_x = Input::get_mouse_x();
		_y = Input::get_mouse_y();
		cancel_mouse_position();
	}
	_last_mouse = m;

}

ActiveDrawableRect** ActiveDrawableRect::get_all_contents()
{
	return _contents;
}

int ActiveDrawableRect::get_all_num()
{
	return _all_num;
}

void ActiveDrawableRect::set_activate(bool val)
{
	_bactive = val;
}

bool ActiveDrawableRect::is_active()
{
	return _bactive;
}